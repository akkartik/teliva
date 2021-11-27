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


#define BG(i) (COLOR_PAIR((i)+8))
#define FG(i) (COLOR_PAIR(i))


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
  printw(
  "usage: %s [options] [script [args]].\n"
  "Available options are:\n"
  "  -e stat  execute string " LUA_QL("stat") "\n"
  "  -l name  require library " LUA_QL("name") "\n"
  "  -i       enter interactive mode after executing " LUA_QL("script") "\n"
  "  -v       show version information\n"
  "  --       stop handling options\n"
  ,
  progname);
  refresh();
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


static void print_version (void) {
  l_message(NULL, LUA_RELEASE "  " LUA_COPYRIGHT);
}


/* pushes commandline args to the stack, then an array of all commandline args */
static int getargs (lua_State *L, char **argv, int n) {
  int narg;
  int i;
  int argc = 0;
  while (argv[argc]) argc++;  /* count total number of arguments */
  narg = argc - (n + 1);  /* number of arguments to the script */
  luaL_checkstack(L, narg + 3, "too many arguments to script");
  for (i=n+1; i < argc; i++)
    lua_pushstring(L, argv[i]);
  lua_createtable(L, narg, n + 1);
  for (i=0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - n);
  }
  return narg;
}


static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}


static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getfield(L, LUA_GLOBALSINDEX, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  lua_pop(L, 1);  /* remove global */
  return p;
}


static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    const char *tp = msg + lmsg - (sizeof(LUA_QL("<eof>")) - 1);
    if (strstr(msg, LUA_QL("<eof>")) == tp) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  addstr(prmt);
  refresh();
  getnstr(b, LUA_MAXINPUT);
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[l-1] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* first line starts with `=' ? */
    lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
  else
    lua_pushstring(L, b);
  return 1;
}


static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  for (;;) {  /* repeat until gets a complete line */
    status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_strlen(L, 1), "=keyboard");
    if (!incomplete(L, status)) break;  /* cannot try to add lines? */
    if (!pushline(L, 0))  /* no more input? */
      return -1;
    lua_pushliteral(L, "\n");  /* add a new line... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
  lua_remove(L, 1);  /* remove line */
  return status;
}


static void dotty (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;
  while ((status = loadline(L)) != -1) {
    if (status == 0) status = docall(L, 0, 0);
    report(L, status);
    if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
      lua_getglobal(L, "print");
      lua_insert(L, 1);
      if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
        l_message(progname, lua_pushfstring(L,
                               "error calling " LUA_QL("print") " (%s)",
                               lua_tostring(L, -1)));
    }
  }
  lua_settop(L, 0);  /* clear stack */
  refresh();
  progname = oldprogname;
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


static const char *look_up_definition (lua_State *L, const char *name) {
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
        return lua_tostring(L, -1);
    }
  }
  lua_pop(L, 1);
  return NULL;
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
      if (status != 0) return report(L, status);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
  return 0;
}


char *Image_name = NULL;
static int handle_image (lua_State *L, char **argv, int n) {
  int status;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  /* parse and load file contents (teliva_program array) */
  Image_name = argv[n];
  status = luaL_loadfile(L, Image_name);
  lua_insert(L, -(narg+1));
  if (status != 0) return status;
  status = docall(L, narg, 0);
  if (status != 0) return report(L, status);
  status = load_definitions(L);
  if (status != 0) return 0;
  /* call main() */
  lua_getglobal(L, "main");
  status = docall(L, 0, 1);
  if (status != 0) return report(L, status);
  return 0;
}


#define CURRENT_DEFINITION_LEN 256
char Current_definition[CURRENT_DEFINITION_LEN+1] = {0};
void save_to_current_definition_and_editor_buffer (lua_State *L, const char *definition) {
  strncpy(Current_definition, definition, CURRENT_DEFINITION_LEN);
  const char *contents = look_up_definition(L, Current_definition);
  FILE *out = fopen("teliva_editbuffer", "w");
  if (contents != NULL)
    fprintf(out, "%s", contents);
  fclose(out);
  lua_settop(L, 0);  /* reclaim contents */
}


static void read_editor_buffer (char *out) {
  FILE *in = fopen("teliva_editbuffer", "r");
  fread(out, 8190, 1, in);  /* TODO: handle overly large file */
  fclose(in);
}


static void update_definition (lua_State *L, const char *name, char *new_contents) {
  assert(lua_gettop(L) == 0);
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  assert(history_array == 1);
  /* create a new table containing a single binding */
  lua_createtable(L, /*number of fields per mutation*/2, 0);
  lua_pushstring(L, new_contents);
  assert(strlen(name) > 0);
  lua_setfield(L, -2, name);
  /* include timestamp at which binding was created */
  time_t t;
  time(&t);
  char time_string[50] = {0};
  ctime_r(&t, time_string);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  lua_settop(L, 0);
}


static void save_image (lua_State *L) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  int history_array_size = luaL_getn(L, history_array);
  FILE* out = fopen(Image_name, "w");
  fprintf(out, "teliva_program = {\n");
  for (int i = 1;  i <= history_array_size; ++i) {
    lua_rawgeti(L, history_array, i);
    int table = lua_gettop(L);
    fprintf(out, "  {\n");
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      const char* key = lua_tostring(L, -2);
      if (strcmp(key, "__teliva_undo") == 0) {
        fprintf(out, "    %s = %ld,\n", key, lua_tointeger(L, -1));
        continue;
      }
      const char* value = lua_tostring(L, -1);
      fprintf(out, "    %s = [==[\n", key);
      fprintf(out, "%s", value);
      fprintf(out, "]==],\n");
    }
    fprintf(out, "  },\n");
    lua_pop(L, 1);
  }
  fprintf(out, "}\n");
  fclose(out);
  lua_pop(L, 1);
}


/* death and rebirth */
char **Argv = NULL;
extern int edit (lua_State *L, char *filename, const char *message);
extern void clearEditor (void);
extern int editorOpen (char *filename);
int edit_buffer (lua_State *L, const char *message) {
  return edit(L, "teliva_editbuffer", message);
}
void editor_refresh_buffer (void) {
  clearEditor();
  editorOpen("teliva_editbuffer");
}


int load_editor_buffer_to_current_definition_in_image(lua_State *L) {
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents);
  update_definition(L, Current_definition, new_contents);
  save_image(L);
  /* reload binding */
  return luaL_loadbuffer(L, new_contents, strlen(new_contents), Current_definition)
          || docall(L, 0, 1);
}


/* return true if user chose to back into the big picture view */
/* But only if there are no errors. Otherwise things can get confusing. */
const char *Previous_error = NULL;
extern int resumeEdit (lua_State *L);
int edit_image (lua_State *L, const char *definition) {
  save_to_current_definition_and_editor_buffer(L, definition);
  int back_to_big_picture = edit_buffer(L, /*status message*/ "");
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
  draw_menu_item("Esc", "go back");
  /* draw_menu_item("↓/space", "older"); */
  attroff(A_REVERSE);
  mvaddstr(LINES-1, menu_column, " ↓/space ");
  menu_column += 9;  /* strlen isn't sufficient */
  attron(A_REVERSE);
  draw_string_on_menu("older");
  /* draw_menu_item("↑/backspace", "newer"); */
  attroff(A_REVERSE);
  mvaddstr(LINES-1, menu_column, " ↑/backspace ");
  menu_column += 13;  /* strlen isn't sufficient */
  attron(A_REVERSE);
  draw_string_on_menu("newer");
  draw_menu_item("^e", "edit/add note");
  if (cursor < history_array_size)
    draw_menu_item("^u", "undo everything after this");
  attrset(A_NORMAL);
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
        attron(FG(7));
        printw("  %s", lua_tostring(L, -1));
        attroff(FG(7));
      }
      lua_pop(L, 1);
      lua_getfield(L, t, "__teliva_note");
      if (!lua_isnil(L, -1)) {
        attron(FG(6));
        printw("  -- %s", lua_tostring(L, -1));
        attroff(FG(6));
      }
      lua_pop(L, 1);
      y++;
      const char *definition_contents = lua_tostring(L, -1);
      int x = 0;
      move(y, x);
      for (int j = 0; j < strlen(definition_contents); ++j) {
        char c = definition_contents[j];
        if (c == '-' && j+1 < strlen(definition_contents) && definition_contents[j+1] == '-')
          attron(FG(6));
        if (c != '\n') {
          addch(definition_contents[j]);
          ++x;
        }
        else {
          /* newline */
          ++y;
          x = 0;
          move(y, x);
          attroff(FG(6));
        }
      }
      y++;
    }
    lua_pop(L, 1);  // history element
    y++;
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
  char time_string[50] = {0};
  ctime_r(&t, time_string);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  /* clean up */
  lua_pop(L, 1);
}


/* precondition: teliva_program is at top of stack */
void save_note_to_editor_buffer (lua_State *L, int cursor) {
  lua_rawgeti(L, -1, cursor);
  lua_getfield(L, -1, "__teliva_note");
  const char *contents = lua_tostring(L, -1);
  FILE *out = fopen("teliva_editbuffer", "w");
  if (contents != NULL)
    fprintf(out, "%s", contents);
  fclose(out);
  lua_pop(L, 2);  /* contents, table at cursor */
}


/* precondition: teliva_program is at top of stack */
void load_note_from_editor_buffer (lua_State *L, int cursor) {
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents);
  lua_rawgeti(L, -1, cursor);
  lua_pushstring(L, new_contents);
  lua_setfield(L, -2, "__teliva_note");
  lua_pop(L, 1);  /* table at cursor */
}


void recent_changes (lua_State *L) {
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
    assert(history_array == 1);
    history_array_size = luaL_getn(L, history_array);
    render_recent_changes(L, history_array, cursor, history_array_size);
    int c = getch();
    switch (c) {
      case ESC:
        quit = 1;
        break;
      case KEY_DOWN:
      case ' ':
        if (cursor > 1) --cursor;
        break;
      case KEY_UP:
      case KEY_BACKSPACE:
        if (cursor < history_array_size) ++cursor;
        break;
      case CTRL_E:
        save_note_to_editor_buffer(L, cursor);
        edit_buffer(L, "");
        load_note_from_editor_buffer(L, cursor);
        save_image(L);
        break;
      case CTRL_U:
        if (cursor < history_array_size) {
          add_undo_event(L, cursor);
          save_image(L);
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
  draw_menu_item("Esc", "go back");
  draw_menu_item("Enter", "submit");
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
  attron(BG(7));
  addstr(definition_name);
  attrset(A_NORMAL);
  addstr("  ");
}

/* return true if submitted */
void big_picture (lua_State *L) {
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
  for (int level = 1; level < 5; ++level) {
    mvaddstr(y, 0, "                ");
    for (int i = history_array_size; i > 0; --i) {
      lua_rawgeti(L, history_array, i);
      int t = lua_gettop(L);
      for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
        const char* definition_name = lua_tostring(L, -2);
        if (is_special_history_key(definition_name)) continue;
        lua_getfield(L, cgt, definition_name);
        int depth = lua_tointeger(L, -1);
        if (depth == level)
          if (is_current_definition(L, definition_name, i, history_array, history_array_size))
            draw_definition_name(definition_name);
        lua_pop(L, 1);  // depth of value
      }
      lua_pop(L, 1);  // history element
    }
    y += 2;
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

  char query[CURRENT_DEFINITION_LEN+1] = {0};
  int qlen = 0;
  while (1) {
    big_picture_menu();
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-2, x, ' ');
    mvprintw(LINES-2, 0, "Edit: %s", query);
    int c = getch();
    if (c == KEY_BACKSPACE) {
      if (qlen != 0) query[--qlen] = '\0';
    } else if (c == ESC) {
      return;
    } else if (c == ENTER) {
      int back_to_big_picture = edit_image(L, query);
      if (back_to_big_picture) goto restart;
      return;
    } else if (c == CTRL_U) {
      qlen = 0;
      query[qlen] = '\0';
    } else if (c == CTRL_R) {
      recent_changes(L);
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


extern void cleanup_curses (void);
void switch_to_editor (lua_State *L) {
  /* clobber the app's ncurses colors; we'll restart the app when we rerun it. */
  for (int i = 0; i < 8; ++i)
    init_pair(i, i, -1);
  for (int i = 0; i < 8; ++i)
    init_pair(i+8, -1, i);
  nodelay(stdscr, 0);
  big_picture(L);
  cleanup_curses();
  execv(Argv[0], Argv);
  /* never returns */
}


/* check that argument has no extra characters at the end */
#define notail(x)	{if ((x)[2] != '\0') return -1;}


static int collectargs (char **argv, int *pi, int *pv, int *pe) {
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    if (argv[i][0] != '-')  /* not an option? */
        return i;
    switch (argv[i][1]) {  /* option */
      case '-':
        notail(argv[i]);
        return (argv[i+1] != NULL ? i+1 : 0);
      case '\0':
        return i;
      case 'i':
        notail(argv[i]);
        *pi = 1;  /* fall through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* fall through */
      case 'l':
        if (argv[i][2] == '\0') {
          i++;
          if (argv[i] == NULL) return -1;
        }
        break;
      default: return -1;  /* invalid option */
    }
  }
  return 0;
}


static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    if (argv[i] == NULL) continue;
    lua_assert(argv[i][0] == '-');
    switch (argv[i][1]) {  /* option */
      case 'e': {
        const char *chunk = argv[i] + 2;
        if (*chunk == '\0') chunk = argv[++i];
        lua_assert(chunk != NULL);
        if (dostring(L, chunk, "=(command line)") != 0)
          return 1;
        break;
      }
      case 'l': {
        const char *filename = argv[i] + 2;
        if (*filename == '\0') filename = argv[++i];
        lua_assert(filename != NULL);
        if (dolibrary(L, filename))
          return 1;  /* stop if file fails */
        break;
      }
      default: break;
    }
  }
  return 0;
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
  if (status != 0) return report(L, status);
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


static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int status;
  int image;
  int has_i = 0, has_v = 0, has_e = 0;
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
  image = collectargs(argv, &has_i, &has_v, &has_e);
  if (image < 0) {  /* invalid args? */
    print_usage();
    getch();
    s->status = 1;
    return 0;
  }
  if (has_v) print_version();
  s->status = runargs(L, argv, (image > 0) ? image : s->argc);
  if (s->status != 0) return 0;
  if (image)
    s->status = handle_image(L, argv, image);
  if (s->status != 0) return 0;
  if (has_i)
    dotty(L);
  else if (image == 0 && !has_e && !has_v) {
    print_version();
    dotty(L);
  }
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
  setlocale(LC_ALL, "");
  initscr();
  keypad(stdscr, 1);
  start_color();
  use_default_colors();
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

