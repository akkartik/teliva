/*
** $Id: lua.c,v 1.160.1.2 2007/12/28 15:32:23 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>
#include <ncurses.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define lua_c

#include "lua.h"
//? #include "lstate.h"
#include "teliva.h"

#include "lauxlib.h"
#include "lualib.h"


static lua_State *globalL = NULL;

static const char *progname = LUA_PROGNAME;



static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (void) {
  printf("usage: %s ___.tlv [args]\n", progname);
}


static void l_message (const char *pname, const char *msg) {
  if (!stdscr || isendwin()) {
    printf("%s: %s\n", pname, msg);
    exit(1);
  }
  if (pname) mvprintw(LINES-2, 0, "%s: ", pname);
  printw(msg);
  mvprintw(LINES-1, 0, "sorry, you'll need to edit the image directly. press any key to exit.");
  refresh();
  nodelay(stdscr, 0);  /* make getch() block */
  getch();
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
  return status;
}


/* return final y containing text */
static int render_wrapped_text (int y, int xmin, int xmax, const char *text) {
  int x = xmin;
  move(y, x);
  for (int j = 0; j < strlen(text); ++j) {
    char c = text[j];
    if (c != '\n') {
      addch(text[j]);
      ++x;
      if (x >= xmax) {
        ++y;
        x = xmin;
        move(y, x);
      }
    }
    else {
      /* newline */
      ++y;
      x = xmin;
      move(y, x);
    }
  }
  return y;
}


const char *Previous_error = NULL;
void render_previous_error (void) {
  if (!Previous_error) return;
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  attron(COLOR_PAIR(COLOR_PAIR_ERROR));
  render_wrapped_text(LINES-10, COLS/2, COLS, Previous_error);
  attroff(COLOR_PAIR(COLOR_PAIR_ERROR));
}


extern char *strdup(const char *s);
extern void developer_mode (lua_State *L);
static int report_in_developer_mode (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    Previous_error = strdup(lua_tostring(L, -1));  /* memory leak */
    if (Previous_error == NULL) Previous_error = "(error object is not a string)";
    lua_pop(L, 1);
    for (int x = 0; x < COLS; ++x) {
      mvaddch(LINES-2, x, ' ');
      mvaddch(LINES-1, x, ' ');
    }
    render_previous_error();
    mvaddstr(LINES-1, 0, "press any key to continue");
    getch();
    developer_mode(L);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


/* initialize global binding "args" for commandline args */
static void set_args (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  lua_newtable(L);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  lua_setglobal(L, "arg");
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report_in_developer_mode(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report_in_developer_mode(L, status);
}


void stack_dump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  int y = 1;
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        mvprintw(y, 30, "`%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        mvprintw(y, 30, lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        mvprintw(y, 30, "%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        mvprintw(y, 30, "%s", lua_typename(L, t));
        break;

    }
    y++;
    mvprintw(y, 30, "  ");  /* put a separator */
    y++;
  }
  mvprintw(y, 30, "\n");  /* end the listing */
  y++;
}


static int binding_exists (lua_State *L, const char *name) {
  int result = 0;
  lua_getglobal(L, name);
  result = !lua_isnil(L, -1);
  lua_pop(L, 1);
  return result;
}


static const char *special_history_keys[] = {
  "__teliva_timestamp",
  "__teliva_undo",
  "__teliva_note",
  NULL,
};

int is_special_history_key(const char *key) {
  for (const char **curr = special_history_keys; *curr != NULL; ++curr) {
    if (strcmp(*curr, key) == 0)
      return 1;
  }
  return 0;
}


/* when found, return 1 and leave string on top of stack
 * when not found, return 0
 * caller is responsible for cleaning up the stack. */
static int look_up_definition (lua_State *L, const char *name) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  /* iterate over mutations in teliva_program history in reverse order */
  int history_array_size = luaL_getn(L, history_array);
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int table = lua_gettop(L);
    /* iterate over bindings */
    /* really we expect only one */
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      const char* key = lua_tostring(L, -2);
      if (strcmp(key, "__teliva_undo") == 0) {
        int next_i = lua_tointeger(L, -1);
        assert(next_i < i);
        i = next_i + 1;  /* account for decrement */
        lua_pop(L, 1);
        break;
      }
      if (is_special_history_key(key)) continue;
      if (strcmp(key, name) == 0)
        return 1;
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  return 0;
}


int load_definitions(lua_State *L) {
  int status;
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  /* iterate over mutations in teliva_program history in reverse order */
  int history_array_size = luaL_getn(L, history_array);
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int table = lua_gettop(L);
    /* iterate over bindings */
    /* really we expect only one */
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      const char* key = lua_tostring(L, -2);
      if (strcmp(key, "__teliva_undo") == 0) {
        int next_i = lua_tointeger(L, -1);
        assert(next_i < i);
        i = next_i + 1;  /* account for decrement */
        lua_pop(L, 1);
        break;
      }
      if (is_special_history_key(key)) continue;
      if (binding_exists(L, key))
        continue;  // most recent binding trumps older ones
      const char* value = lua_tostring(L, -1);
      status = dostring(L, value, key);
      if (status != 0) return report_in_developer_mode(L, status);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  return 0;
}


char *Image_name = NULL;
extern void load_tlv (lua_State *L, char *filename);
static int handle_image (lua_State *L, char **argv, int n) {
  int status;
  set_args(L, argv, n);
  /* parse and load file contents (teliva_program array) */
  Image_name = argv[n];
  load_tlv(L, Image_name);
//?   save_tlv(L, Image_name);  // manual test; should always return identical result, modulo key order
//?   exit(1);
  status = load_definitions(L);
  if (status != 0) return 0;
  /* call main() */
  lua_getglobal(L, "main");
  status = docall(L, 0, 1);
  if (status != 0) return report_in_developer_mode(L, status);
  return 0;
}


#define CURRENT_DEFINITION_LEN 256
char Current_definition[CURRENT_DEFINITION_LEN+1] = {0};

void save_editor_state (int rowoff, int coloff, int cy, int cx) {
  if (strlen(Current_definition) == 0) return;
  FILE *out = fopen("teliva_editor_state", "w");
  fprintf(out, "__teliva_editor_state = {\n");
  fprintf(out, "  image = \"%s\", definition = \"%s\",\n", Image_name, Current_definition);
  fprintf(out, "  rowoff = %d, coloff = %d,\n", rowoff, coloff);
  fprintf(out, "  cy = %d, cx = %d,\n", cy, cx);
  fprintf(out, "}\n");
  fclose(out);
}

void save_to_current_definition_and_editor_buffer (lua_State *L, const char *definition) {
  int current_stack_index = lua_gettop(L);
  strncpy(Current_definition, definition, CURRENT_DEFINITION_LEN);
  int status = look_up_definition(L, Current_definition);
  FILE *out = fopen("teliva_editor_buffer", "w");
  if (status)
    fprintf(out, "%s", lua_tostring(L, -1));
  fclose(out);
  lua_settop(L, current_stack_index);
}


static void read_editor_buffer (char *out) {
  FILE *in = fopen("teliva_editor_buffer", "r");
  fread(out, 8190, 1, in);  /* TODO: handle overly large file */
  fclose(in);
}


static void update_definition (lua_State *L, const char *name, char *new_contents) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  /* create a new table containing a single binding */
  lua_createtable(L, /*number of fields per mutation*/2, 0);
  lua_pushstring(L, new_contents);
  assert(strlen(name) > 0);
  lua_setfield(L, -2, name);
  /* include timestamp at which binding was created */
  time_t t;
  time(&t);
  char *time_string = ctime(&t);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  lua_settop(L, history_array);
}


extern void save_tlv (lua_State *L, char *filename);
int load_editor_buffer_to_current_definition_in_image(lua_State *L) {
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents);
  update_definition(L, Current_definition, new_contents);
  save_tlv(L, Image_name);
  /* reload binding */
  return luaL_loadbuffer(L, new_contents, strlen(new_contents), Current_definition)
          || docall(L, 0, 1);
}


/* return true if user chose to back into the big picture view */
/* But only if there are no errors. Otherwise things can get confusing. */
extern int edit (lua_State *L, char *filename);
extern int resumeEdit (lua_State *L);
int edit_current_definition (lua_State *L) {
  int back_to_big_picture = edit(L, "teliva_editor_buffer");
  // error handling
  while (1) {
    int status;
    status = load_editor_buffer_to_current_definition_in_image(L);
    if (status == 0 || lua_isnil(L, -1))
      break;
    Previous_error = lua_tostring(L, -1);
    if (Previous_error == NULL) Previous_error = "(error object is not a string)";
    back_to_big_picture = resumeEdit(L);
    lua_pop(L, 1);
  }
  return back_to_big_picture;
}


extern void draw_menu_item (const char* key, const char* name);
extern void draw_string_on_menu (const char* s);
static void recent_changes_menu (int cursor, int history_array_size) {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
  extern int menu_column;
  menu_column = 2;
  draw_menu_item("^x", "go back");
  /* draw_menu_item("↓|space", "older"); */
  attroff(A_REVERSE);
  mvaddstr(LINES-1, menu_column, " ↓");
  attron(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("|");
  attroff(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("space ");
  menu_column += 9;  /* strlen isn't sufficient */
  attron(A_REVERSE);
  draw_string_on_menu("older");
  /* draw_menu_item("↑|backspace|delete|^h", "newer"); */
  attroff(A_REVERSE);
  mvaddstr(LINES-1, menu_column, " ↑");
  attron(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("|");
  attroff(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("backspace");
  attron(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("|");
  attroff(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("delete");
  attron(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("|");
  attroff(COLOR_PAIR(COLOR_PAIR_MENU_ALTERNATE));
  addstr("^h ");
  menu_column += 23;
  attron(A_REVERSE);
  draw_string_on_menu("newer");
  draw_menu_item("^e", "edit note");
  if (cursor < history_array_size)
    draw_menu_item("^u", "undo everything after this");
  attrset(A_NORMAL);
}


/* return final y containing text */
static int render_wrapped_lua_text (int y, int xmin, int xmax, const char *text) {
  int x = xmin;
  move(y, x);
  for (int j = 0; j < strlen(text); ++j) {
    char c = text[j];
    if (c == '-' && j+1 < strlen(text) && text[j+1] == '-')
      attron(COLOR_PAIR(COLOR_PAIR_LUA_COMMENT));
    if (c != '\n') {
      addch(text[j]);
      ++x;
      if (x >= xmax) {
        ++y;
        x = xmin;
        move(y, x);
      }
    }
    else {
      /* newline */
      ++y;
      x = xmin;
      move(y, x);
      attroff(COLOR_PAIR(COLOR_PAIR_LUA_COMMENT));
    }
  }
  return y;
}


void render_recent_changes (lua_State *L, int history_array, int start_index, int history_array_size) {
  clear();
  attrset(A_BOLD);
  mvaddstr(1, 0, "Recent changes");
  attrset(A_NORMAL);
  int y = 3;
  attron(A_REVERSE);
  for (int i = start_index; i > 0; --i) {
    attron(A_BOLD);
    mvprintw(y, 0, "%3d. ", i);
    attrset(A_NORMAL);
    lua_rawgeti(L, history_array, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
      if (strcmp(lua_tostring(L, -2), "__teliva_undo") == 0) {
        addstr("undo to ");
        attron(A_BOLD);
        printw("%d", lua_tointeger(L, -1));
        attroff(A_BOLD);
        y++;
        continue;
      }
      const char *definition_name = lua_tostring(L, -2);
      if (is_special_history_key(definition_name)) continue;
      addstr(definition_name);
      /* save timestamp of binding if available */
      lua_getfield(L, t, "__teliva_timestamp");
      if (!lua_isnil(L, -1)) {
        attron(COLOR_PAIR(COLOR_PAIR_FADE));
        printw("  %s", lua_tostring(L, -1));
        attroff(COLOR_PAIR(COLOR_PAIR_FADE));
      }
      lua_pop(L, 1);
      lua_getfield(L, t, "__teliva_note");
      if (!lua_isnil(L, -1)) {
        attron(COLOR_PAIR(COLOR_PAIR_FADE));
        printw("  -- %s", lua_tostring(L, -1));
        attroff(COLOR_PAIR(COLOR_PAIR_FADE));
      }
      lua_pop(L, 1);
      y++;
      const char *definition_contents = lua_tostring(L, -1);
      y = render_wrapped_lua_text(y, 0, COLS, definition_contents);
      y++;
      if (y >= LINES-1) break;
    }
    lua_pop(L, 1);  // history element
    y++;
    if (y >= LINES-1) break;
  }
  recent_changes_menu(start_index, history_array_size);
  refresh();
}


void add_undo_event(lua_State *L, int cursor) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  /* create a new table containing the undo event */
  lua_createtable(L, /*number of fields per mutation*/2, 0);
  lua_pushinteger(L, cursor);
  lua_setfield(L, -2, "__teliva_undo");
  /* include timestamp at which event was created */
  time_t t;
  time(&t);
  char *time_string = ctime(&t);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  /* clean up */
  lua_pop(L, 1);
}


void save_note_to_editor_buffer (lua_State *L, int cursor) {
  lua_getglobal(L, "teliva_program");
  lua_rawgeti(L, -1, cursor);
  lua_getfield(L, -1, "__teliva_note");
  const char *contents = lua_tostring(L, -1);
  FILE *out = fopen("teliva_editor_buffer", "w");
  if (contents != NULL)
    fprintf(out, "%s", contents);
  fclose(out);
  lua_pop(L, 3);  /* contents, table at cursor, teliva_program */
}


void load_note_from_editor_buffer (lua_State *L, int cursor) {
  lua_getglobal(L, "teliva_program");
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents);
  lua_rawgeti(L, -1, cursor);
  lua_pushstring(L, new_contents);
  lua_setfield(L, -2, "__teliva_note");
  lua_pop(L, 2);  /* table at cursor, teliva_program */
}


void recent_changes_view (lua_State *L) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  assert(history_array == 1);
  int history_array_size = luaL_getn(L, history_array);
  int cursor = history_array_size;
  lua_pop(L, 1);
  int quit = 0;
  while (!quit) {
    /* refresh state after each operation so we pick up modifications */
    lua_getglobal(L, "teliva_program");
    history_array = lua_gettop(L);
    history_array_size = luaL_getn(L, history_array);
    render_recent_changes(L, history_array, cursor, history_array_size);
    int c = getch();
    switch (c) {
      case CTRL_X:
        quit = 1;
        break;
      case KEY_DOWN:
      case ' ':
        if (cursor > 1) --cursor;
        break;
      case KEY_UP:
      case KEY_BACKSPACE:
      case DELETE:
      case CTRL_H:
        if (cursor < history_array_size) ++cursor;
        break;
      case CTRL_E:
        save_note_to_editor_buffer(L, cursor);
        /* big picture hotkey unnecessarily available here */
        /* TODO: go hotkey is misleading. edits will not be persisted until you return to recent changes */
        edit(L, "teliva_editor_buffer");
        load_note_from_editor_buffer(L, cursor);
        save_tlv(L, Image_name);
        break;
      case CTRL_U:
        if (cursor < history_array_size) {
          add_undo_event(L, cursor);
          save_tlv(L, Image_name);
        }
        break;
    }
    lua_pop(L, 1);
  }
}


extern void draw_menu_item (const char* key, const char* name);
static void big_picture_menu (void) {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
  extern int menu_column;
  menu_column = 2;
  draw_menu_item("^x", "go back");
  draw_menu_item("Enter", "submit");
  draw_menu_item("^h", "backspace");
  draw_menu_item("^u", "clear");
  draw_menu_item("^r", "recent changes");
  attrset(A_NORMAL);
}


static int is_current_definition(lua_State *L, const char *definition_name, int current_history_array_index, int history_array_location, int history_array_size) {
  /* Sequentially scan back through history_array until current_history_array_index.
   * Is there an earlier definition of definition_name? */
//?   int oldsize = L->top-L->stack;
  int found = 0;
  for (int i = history_array_size; i > current_history_array_index; --i) {
    lua_rawgeti(L, history_array_location, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0;) {
      lua_pop(L, 1);  // value
      const char *curr = lua_tostring(L, -1);
      if (strcmp(curr, definition_name) == 0) {
        found = 1;
        lua_pop(L, 1); // key
        break;
      }
      // leave key on stack for next iteration
    }
    lua_pop(L, 1);  // history element
    if (found)
      break;
  }
//?   if(oldsize != L->top-L->stack) {
//?     endwin();
//?     printf("%d %d\n", oldsize, L->top-L->stack);
//?     exit(1);
//?   }
  return !found;
}


void draw_definition_name (const char *definition_name) {
  attron(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
  addstr(" ");
  addstr(definition_name);
  addstr(" ");
  attroff(COLOR_PAIR(COLOR_PAIR_HIGHLIGHT));
  addstr("  ");
}

/* return true if submitted */
void big_picture_view (lua_State *L) {
restart:
  clear();
  luaL_newmetatable(L, "__teliva_call_graph_depth");
  int cgt = lua_gettop(L);
  // special-case: we don't instrument the call to main, but it's always at depth 1
  lua_pushinteger(L, 1);
  lua_setfield(L, cgt, "main");
  // segment definitions by depth
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  int history_array_size = luaL_getn(L, history_array);

  int y = 1;
  attrset(A_BOLD);
  mvaddstr(y, 0, "Big picture");
  attrset(A_NORMAL);
  y += 2;
  mvaddstr(y, 0, "data:           ");
  // first: data (non-functions) that's not the Teliva menu or curses variables
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
      const char *definition_name = lua_tostring(L, -2);
      if (is_special_history_key(definition_name)) continue;
      lua_getglobal(L, definition_name);
      int is_userdata = lua_isuserdata(L, -1);
      int is_function = lua_isfunction(L, -1);
      lua_pop(L, 1);
      if (strcmp(definition_name, "menu") != 0  // required by all Teliva programs
          && !is_function  // functions are not data
          && !is_userdata  // including curses window objects
                           // (unlikely to have an interesting definition)
      ) {
        if (is_current_definition(L, definition_name, i, history_array, history_array_size))
          draw_definition_name(definition_name);
      }
    }
    lua_pop(L, 1);  // history element
  }

  // second: menu and other userdata
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
      const char* definition_name = lua_tostring(L, -2);
      if (is_special_history_key(definition_name)) continue;
      lua_getglobal(L, definition_name);
      int is_userdata = lua_isuserdata(L, -1);
      lua_pop(L, 1);
      if (strcmp(definition_name, "menu") == 0
          || is_userdata  // including curses window objects
      ) {
        if (is_current_definition(L, definition_name, i, history_array, history_array_size))
          draw_definition_name(definition_name);
      }
    }
    lua_pop(L, 1);  // history element
  }

  // functions by level
  y += 2;
  mvprintw(y, 0, "functions: ");
  y++;
  for (int level = 1; ; ++level) {
    mvaddstr(y, 0, "                ");
    bool drew_anything = false;
    for (int i = history_array_size; i > 0; --i) {
      lua_rawgeti(L, history_array, i);
      int t = lua_gettop(L);
      for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
        const char* definition_name = lua_tostring(L, -2);
        if (is_special_history_key(definition_name)) continue;
        lua_getfield(L, cgt, definition_name);
        int depth = lua_tointeger(L, -1);
        if (depth == level) {
          if (is_current_definition(L, definition_name, i, history_array, history_array_size))
            draw_definition_name(definition_name);
          drew_anything = true;
        }
        lua_pop(L, 1);  // depth of value
      }
      lua_pop(L, 1);  // history element
    }
    y += 2;
    if (!drew_anything) break;
  }

  // unused functions
  mvaddstr(y, 0, "                ");
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
      const char* definition_name = lua_tostring(L, -2);
      if (is_special_history_key(definition_name)) continue;
      lua_getglobal(L, definition_name);
      int is_function = lua_isfunction(L, -1);
      lua_pop(L, 1);
      lua_getfield(L, cgt, definition_name);
      if (is_function && lua_isnoneornil(L, -1))
        if (is_current_definition(L, definition_name, i, history_array, history_array_size))
          draw_definition_name(definition_name);
      lua_pop(L, 1);  // depth of value
    }
    lua_pop(L, 1);  // history element
  }

  lua_settop(L, 0);
  render_previous_error();

  char query[CURRENT_DEFINITION_LEN+1] = {0};
  int qlen = 0;
  while (1) {
    big_picture_menu();
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-2, x, ' ');
    mvprintw(LINES-2, 0, "Edit: %s", query);
    int c = getch();
    if (c == KEY_BACKSPACE || c == DELETE || c == CTRL_H) {
      if (qlen != 0) query[--qlen] = '\0';
    } else if (c == CTRL_X) {
      return;
    } else if (c == ENTER) {
      save_to_current_definition_and_editor_buffer(L, query);
      int back_to_big_picture = edit_current_definition(L);
      if (back_to_big_picture) goto restart;
      return;
    } else if (c == CTRL_U) {
      qlen = 0;
      query[qlen] = '\0';
    } else if (c == CTRL_R) {
      recent_changes_view(L);
      goto restart;
    } else if (isprint(c)) {
      if (qlen < CURRENT_DEFINITION_LEN) {
          query[qlen++] = c;
          query[qlen] = '\0';
      }
    }
  }
  /* never gets here */
}


/* return true if:
 *  - editor_state exists, and
 *  - editor_state is applicable to the current image */
int editor_view_in_progress (lua_State *L) {
  int status;
  status = luaL_loadfile(L, "teliva_editor_state");
  if (status != 0) return 0;
  status = docall(L, 0, 0);
  if (status != 0) return 0;
  lua_getglobal(L, "__teliva_editor_state");
  int editor_state_index = lua_gettop(L);
  lua_getfield(L, editor_state_index, "image");
  const char *image_name = lua_tostring(L, -1);
  const int result = (strcmp(image_name, Image_name) == 0);
  lua_settop(L, editor_state_index);
  return result;
}

extern int edit_from(lua_State *L, char *filename, int rowoff, int coloff, int cy, int cx);
int restore_editor_view (lua_State *L) {
  lua_getglobal(L, "__teliva_editor_state");
  int editor_state_index = lua_gettop(L);
  lua_getfield(L, editor_state_index, "definition");
  const char *definition = lua_tostring(L, -1);
  save_to_current_definition_and_editor_buffer(L, definition);
  lua_getfield(L, editor_state_index, "rowoff");
  int rowoff = lua_tointeger(L, -1);
  lua_getfield(L, editor_state_index, "coloff");
  int coloff = lua_tointeger(L, -1);
  lua_getfield(L, editor_state_index, "cy");
  int cy = lua_tointeger(L, -1);
  lua_getfield(L, editor_state_index, "cx");
  int cx = lua_tointeger(L, -1);
  lua_settop(L, editor_state_index);
  int back_to_big_picture = edit_from(L, "teliva_editor_buffer", rowoff, coloff, cy, cx);
  // error handling
  while (1) {
    int status;
    status = load_editor_buffer_to_current_definition_in_image(L);
    if (status == 0 || lua_isnil(L, -1))
      break;
    Previous_error = strdup(lua_tostring(L, -1));  /* memory leak */
    if (Previous_error == NULL) Previous_error = "(error object is not a string)";
    lua_pop(L, 1);
    back_to_big_picture = resumeEdit(L);
  }
  return back_to_big_picture;
}


char **Argv = NULL;
extern void cleanup_curses (void);
void developer_mode (lua_State *L) {
  /* clobber the app's ncurses colors; we'll restart the app when we rerun it. */
  assume_default_colors(COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_NORMAL, COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_HIGHLIGHT, COLOR_HIGHLIGHT_FOREGROUND, COLOR_HIGHLIGHT_BACKGROUND);
  init_pair(COLOR_PAIR_FADE, COLOR_FADE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MENU_ALTERNATE, COLOR_MENU_ALTERNATE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_COMMENT, COLOR_LUA_COMMENT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_KEYWORD, COLOR_LUA_KEYWORD, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_CONSTANT, COLOR_LUA_CONSTANT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MATCH, COLOR_MATCH_FOREGROUND, COLOR_MATCH_BACKGROUND);
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  nodelay(stdscr, 0);  /* make getch() block */
  int switch_to_big_picture_view = 1;
  if (editor_view_in_progress(L))
    switch_to_big_picture_view = restore_editor_view(L);
  if (switch_to_big_picture_view)
    big_picture_view(L);
  cleanup_curses();
  execv(Argv[0], Argv);
  /* never returns */
}


static int handle_luainit (lua_State *L) {
  const char *init = getenv(LUA_INIT);
  if (init == NULL) return 0;  /* status OK */
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, "=" LUA_INIT);
}


/* roughly equivalent to:
 *  globalname = require(filename) */
static int dorequire (lua_State *L, const char *filename, const char *globalname) {
  int status = luaL_loadfile(L, filename) || docall(L, /*nargs*/0, /*don't clean up stack*/0);
  if (status != 0) return report_in_developer_mode(L, status);
  if (lua_isnil(L, -1)) {
    endwin();
    printf("%s didn't return a module\n", filename);
    exit(1);
  }
  lua_setglobal(L, globalname);
  return 0;
}


struct Smain {
  int argc;
  char **argv;
  int status;
};


/* does its own error handling, always returns 0 to prevent duplicate messages */
static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int status;
  globalL = L;
  if (argv[0] && argv[0][0]) progname = argv[0];
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
  status = dorequire(L, "src/lcurses/curses.lua", "curses");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/socket.lua", "socket");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/url.lua", "url");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/ltn12.lua", "ltn12");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/mime.lua", "mime");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/headers.lua", "headers");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasocket/http.lua", "http");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasec/ssl.lua", "ssl");
  if (status != 0) return 0;
  status = dorequire(L, "src/luasec/https.lua", "https");
  if (status != 0) return 0;
  status = dorequire(L, "src/json.lua", "json");
  if (status != 0) return 0;
  lua_gc(L, LUA_GCRESTART, 0);
  s->status = handle_luainit(L);
  if (s->status != 0) return 0;
  s->status = handle_image(L, argv, 1);
  if (s->status != 0) return 0;
  return 0;
}


extern void draw_menu (lua_State *);


int main (int argc, char **argv) {
  int status;
  struct Smain s;
  lua_State *L = luaL_newstate();
  if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
  if (argc == 1) {
    print_usage();
    exit(1);
  }
  setlocale(LC_ALL, "");
  initscr();
  keypad(stdscr, 1);
  start_color();
  assume_default_colors(COLOR_FOREGROUND, COLOR_BACKGROUND);
  draw_menu(L);
  echo();
  s.argc = argc;
  s.argv = argv;
  Argv = argv;
  status = lua_cpcall(L, &pmain, &s);
  report(L, status);
  lua_close(L);
  cleanup_curses();
  return (status || s.status) ? EXIT_FAILURE : EXIT_SUCCESS;
}

