#include <assert.h>
#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "lua.h"
#include "lauxlib.h"
#include "teliva.h"
#include "tlv.h"

/*** Standard UI elements */

int menu_column = 0;

void draw_string_on_menu(const char* s) {
  mvaddstr(LINES-1, menu_column, " ");
  ++menu_column;
  mvaddstr(LINES-1, menu_column, s);
  menu_column += strlen(s);
  mvaddstr(LINES-1, menu_column, " ");
  ++menu_column;
}

void draw_menu_item(const char* key, const char* name) {
  attroff(A_REVERSE);
  draw_string_on_menu(key);
  attron(A_REVERSE);
  draw_string_on_menu(name);
}

static void render_permissions(lua_State* L);
char* Previous_message;
static void draw_menu(lua_State* L) {
  attron(A_BOLD|A_REVERSE);
  color_set(COLOR_PAIR_MENU, NULL);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  menu_column = 2;
  draw_menu_item("^x", "exit");
  draw_menu_item("^e", "edit");
  draw_menu_item("^p", "perms");

  /* if app ran successfully, render any app-specific items */
  if (Previous_message == NULL) {
    lua_getglobal(L, "menu");
    int table = lua_gettop(L);
    if (lua_istable(L, -1)) {
      for (int i = 1; i <= luaL_getn(L, table); ++i) {
        lua_rawgeti(L, table, i);
        int menu_item = lua_gettop(L);
        lua_rawgeti(L, menu_item, 1);  /* key */
        lua_rawgeti(L, menu_item, 2);  /* value */
        draw_menu_item(lua_tostring(L, -2), lua_tostring(L, -1));
        lua_pop(L, 3);
      }
    }
    lua_pop(L, 1);
  }
  else {
    /* otherwise render the flash message */
    attron(COLOR_PAIR(COLOR_PAIR_ERROR));
    addstr(" ");
    addstr(Previous_message);
    addstr(" ");
    attroff(COLOR_PAIR(COLOR_PAIR_ERROR));
  }

  /* render app permissions on the right */
  render_permissions(L);

  attrset(A_NORMAL);
}

static void render_permissions(lua_State* L) {
  attrset(A_NORMAL);
  mvaddstr(LINES-1, COLS-12, "");
  int file_colors = file_operations_allowed ? COLOR_PAIR_WARN : COLOR_PAIR_SAFE;
  int net_colors = net_operations_allowed ? COLOR_PAIR_WARN : COLOR_PAIR_SAFE;
  if (file_operations_allowed && net_operations_allowed) {
    file_colors = net_colors = COLOR_PAIR_RISK;
  }

  attron(COLOR_PAIR(file_colors));
  addstr("file ");
  attron(A_REVERSE);
  addstr(" ");
  attroff(COLOR_PAIR(file_colors));

  attron(COLOR_PAIR(net_colors));
  addstr(" ");
  attroff(A_REVERSE);
  addstr(" net");
  attroff(COLOR_PAIR(net_colors));
}

void render_trusted_teliva_data(lua_State* L) {
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  init_pair(COLOR_PAIR_MENU, COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_SAFE, COLOR_SAFE_REVERSE, COLOR_FOREGROUND);
  init_pair(COLOR_PAIR_WARN, COLOR_WARN_REVERSE, COLOR_FOREGROUND);
  init_pair(COLOR_PAIR_RISK, COLOR_RISK_REVERSE, COLOR_FOREGROUND);
  draw_menu(L);
}

/*** Error reporting */

const char* Previous_error = NULL;

/* return final y containing text */
static int render_wrapped_text(int y, int xmin, int xmax, const char* text) {
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

void render_previous_error(void) {
  if (!Previous_error) return;
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  attron(COLOR_PAIR(COLOR_PAIR_ERROR));
  render_wrapped_text(LINES-10, COLS/2, COLS, Previous_error);
  attroff(COLOR_PAIR(COLOR_PAIR_ERROR));
}

extern char* strdup(const char* s);
int report_in_developer_mode(lua_State* L, int status) {
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

/*** Developer mode, big picture view */

#define CURRENT_DEFINITION_LEN 256

static void big_picture_menu(void) {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
  menu_column = 2;
  draw_menu_item("^x", "go back");
  draw_menu_item("^g", "go to highlight");
  draw_menu_item("Enter", "submit");
  draw_menu_item("^h", "backspace");
  draw_menu_item("^u", "clear");
  draw_menu_item("^r", "recent changes");
  attrset(A_NORMAL);
}

static int is_current_definition(lua_State* L, const char* definition_name, int current_history_array_index, int history_array_location, int history_array_size) {
  /* Sequentially scan back through history_array until current_history_array_index.
   * Is there an earlier definition of definition_name? */
  int oldtop = lua_gettop(L);
  int found = 0;
  for (int i = history_array_size; i > current_history_array_index; --i) {
    lua_rawgeti(L, history_array_location, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0;) {
      lua_pop(L, 1);  // value
      const char* curr = lua_tostring(L, -1);
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
  if(oldtop != lua_gettop(L)) {
    endwin();
    printf("%d %d\n", oldtop, lua_gettop(L));
    exit(1);
  }
  return !found;
}

void draw_definition_name(const char* definition_name) {
  attron(COLOR_PAIR(COLOR_PAIR_SELECTABLE));
  addstr(" ");
  addstr(definition_name);
  addstr(" ");
  attroff(COLOR_PAIR(COLOR_PAIR_SELECTABLE));
  addstr("  ");
}

void draw_highlighted_definition_name(const char* definition_name) {
  attron(A_REVERSE);
  addstr(" ");
  addstr(definition_name);
  addstr(" ");
  attroff(A_REVERSE);
  addstr("  ");
}

/* return true if submitted */
static int edit_current_definition(lua_State* L);
static void recent_changes_view(lua_State* L);
void big_picture_view(lua_State* L) {
  /* Without any intervening edits, big_picture_view always stably renders
   * definitions in exactly the same spatial order, both in levels from top to
   * bottom and in indexes within each level from left to right. */
  int highlight_level = 0;
  int highlight_index_within_level = 0;
  int level_size[30] = {0};  /* number of indexes within each level */
  char highlight[CURRENT_DEFINITION_LEN+1] = {0};
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
  if (highlight_level < 0) highlight_level = 0;
  int level = 0;
  int index_within_level = 0;
  for (int i = history_array_size; i > 0; --i) {
    lua_rawgeti(L, history_array, i);
    int t = lua_gettop(L);
    for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
      const char* definition_name = lua_tostring(L, -2);
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
        if (is_current_definition(L, definition_name, i, history_array, history_array_size)) {
          if (level == highlight_level && index_within_level == highlight_index_within_level) {
            draw_highlighted_definition_name(definition_name);
            strncpy(highlight, definition_name, CURRENT_DEFINITION_LEN);
          } else {
            draw_definition_name(definition_name);
          }
          ++index_within_level;
        }
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
        if (is_current_definition(L, definition_name, i, history_array, history_array_size)) {
          if (level == highlight_level && index_within_level == highlight_index_within_level) {
            draw_highlighted_definition_name(definition_name);
            strncpy(highlight, definition_name, CURRENT_DEFINITION_LEN);
          } else {
            draw_definition_name(definition_name);
          }
          ++index_within_level;
        }
      }
    }
    lua_pop(L, 1);  // history element
  }
  level_size[level] = index_within_level;
  level++;

  // functions by level
  y += 2;
  mvprintw(y, 0, "functions: ");
  y++;
  for (int depth = 1; ; ++depth) {
    mvaddstr(y, 0, "                ");
    bool drew_anything = false;
    index_within_level = 0;
    for (int i = history_array_size; i > 0; --i) {
      lua_rawgeti(L, history_array, i);
      int t = lua_gettop(L);
      for (lua_pushnil(L); lua_next(L, t) != 0; lua_pop(L, 1)) {
        const char* definition_name = lua_tostring(L, -2);
        if (is_special_history_key(definition_name)) continue;
        lua_getfield(L, cgt, definition_name);
        int definition_depth = lua_tointeger(L, -1);
        if (definition_depth == depth) {
          if (is_current_definition(L, definition_name, i, history_array, history_array_size)) {
            if (level == highlight_level && index_within_level == highlight_index_within_level) {
              draw_highlighted_definition_name(definition_name);
              strncpy(highlight, definition_name, CURRENT_DEFINITION_LEN);
            } else {
              draw_definition_name(definition_name);
            }
            ++index_within_level;
          }
          drew_anything = true;
        }
        lua_pop(L, 1);  // depth of value
      }
      lua_pop(L, 1);  // history element
    }
    y += 2;
    if (!drew_anything) break;
    level_size[level] = index_within_level;
    level++;
  }

  // unused functions
  mvaddstr(y, 0, "                ");
  /* no need to level++ because the final iteration above didn't draw anything */
  index_within_level = 0;
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
      if (is_function && lua_isnoneornil(L, -1)) {
        if (is_current_definition(L, definition_name, i, history_array, history_array_size)) {
          if (level == highlight_level && index_within_level == highlight_index_within_level) {
            draw_highlighted_definition_name(definition_name);
            strncpy(highlight, definition_name, CURRENT_DEFINITION_LEN);
          } else {
            draw_definition_name(definition_name);
          }
          ++index_within_level;
        }
      }
      lua_pop(L, 1);  // depth of value
    }
    lua_pop(L, 1);  // history element
  }
  level_size[level] = index_within_level;
  int max_level = level;

  lua_settop(L, 0);
  render_previous_error();

  char query[CURRENT_DEFINITION_LEN+1] = {0};
  int qlen = 0;
  while (1) {
    big_picture_menu();
    for (int x = 0; x < COLS; ++x)
      mvaddch(LINES-2, x, ' ');
//?     mvprintw(20, 60, "%d %d\n", highlight_level, highlight_index_within_level);
    mvprintw(LINES-2, 0, "Edit: %s", query);
    int c = getch();
    if (c == KEY_BACKSPACE || c == DELETE || c == CTRL_H) {
      if (qlen != 0) query[--qlen] = '\0';
    } else if (c == CTRL_X) {
      return;
    } else if (c == ENTER) {
      if (query[0] != '\0') {
        save_to_current_definition_and_editor_buffer(L, query);
        int back_to_big_picture = edit_current_definition(L);
        if (back_to_big_picture) goto restart;
        return;
      }
    } else if (c == CTRL_U) {
      qlen = 0;
      query[qlen] = '\0';
    } else if (c == CTRL_R) {
      recent_changes_view(L);
      goto restart;
    } else if (c == KEY_LEFT) {
      highlight_index_within_level--;
      if (highlight_index_within_level < 0) highlight_index_within_level = 0;
      goto restart;
    } else if (c == KEY_RIGHT) {
      highlight_index_within_level++;
      if (highlight_index_within_level >= level_size[highlight_level])
        highlight_index_within_level = level_size[highlight_level]-1;
      if (highlight_index_within_level < 0) highlight_index_within_level = 0;
      goto restart;
    } else if (c == KEY_UP) {
      highlight_level--;
      if (highlight_level < 0) highlight_level = 0;
      if (highlight_index_within_level >= level_size[highlight_level])
        highlight_index_within_level = level_size[highlight_level]-1;
      if (highlight_index_within_level < 0) highlight_index_within_level = 0;
      goto restart;
    } else if (c == KEY_DOWN) {
      highlight_level++;
      if (highlight_level > max_level) highlight_level = max_level;
      if (highlight_index_within_level >= level_size[highlight_level])
        highlight_index_within_level = level_size[highlight_level]-1;
      if (highlight_index_within_level < 0) highlight_index_within_level = 0;
      goto restart;
    } else if (c == CTRL_G) {
      save_to_current_definition_and_editor_buffer(L, highlight);
      int back_to_big_picture = edit_current_definition(L);
      if (back_to_big_picture) goto restart;
      return;
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
 *  - editor_state is applicable to the current image
 * Implicitly loads current editor state. */
int editor_view_in_progress(lua_State* L) {
  FILE* in = fopen("teliva_editor_state", "r");
  if (in == NULL) return 0;
  int oldtop = lua_gettop(L);
  teliva_load_definition(L, in);
  int t = lua_gettop(L);
  lua_getfield(L, t, "image");
  const char* image_name  = lua_tostring(L, -1);
  int result = (strcmp(image_name, Image_name) == 0);
  lua_pop(L, 1);  /* image value */
  lua_setglobal(L, "__teliva_editor_state");
  assert(lua_gettop(L) == oldtop);
  return result;
}

extern int load_editor_buffer_to_current_definition_in_image(lua_State* L);
extern int resumeEdit(lua_State* L);
extern int edit_from(lua_State* L, char* filename, int rowoff, int coloff, int cy, int cx);
int restore_editor_view(lua_State* L) {
  lua_getglobal(L, "__teliva_editor_state");
  int editor_state_index = lua_gettop(L);
  lua_getfield(L, editor_state_index, "definition");
  const char* definition = lua_tostring(L, -1);
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
  int oldtop = lua_gettop(L);
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
  if (lua_gettop(L) != oldtop) {
    endwin();
    printf("edit_from: memory leak %d -> %d\n", oldtop, lua_gettop(L));
    exit(1);
  }
  return back_to_big_picture;
}

char** Argv = NULL;
extern void cleanup_curses(void);
void developer_mode(lua_State* L) {
  /* clobber the app's ncurses colors; we'll restart the app when we rerun it. */
  assume_default_colors(COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_NORMAL, COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_SELECTABLE, COLOR_SELECTABLE_FOREGROUND, COLOR_SELECTABLE_BACKGROUND);
  init_pair(COLOR_PAIR_FADE, COLOR_FADE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MENU_ALTERNATE, COLOR_MENU_ALTERNATE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_COMMENT, COLOR_LUA_COMMENT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_KEYWORD, COLOR_LUA_KEYWORD, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_CONSTANT, COLOR_LUA_CONSTANT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MATCH, COLOR_MATCH_FOREGROUND, COLOR_MATCH_BACKGROUND);
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  nodelay(stdscr, 0);  /* always make getch() block in developer mode */
  curs_set(1);  /* always display cursor in developer mode */
  int switch_to_big_picture_view = 1;
  if (editor_view_in_progress(L))
    switch_to_big_picture_view = restore_editor_view(L);
  if (switch_to_big_picture_view)
    big_picture_view(L);
  cleanup_curses();
  execv(Argv[0], Argv);
  /* never returns */
}

char Current_definition[CURRENT_DEFINITION_LEN+1] = {0};

extern int mkstemp(char* template);
extern FILE* fdopen(int fd, const char* mode);
void save_editor_state(int rowoff, int coloff, int cy, int cx) {
  if (strlen(Current_definition) == 0) return;
  char outfilename[] = "teliva_editor_state_XXXXXX";
  int outfd = mkstemp(outfilename);
  if (outfd == -1) {
    endwin();
    perror("error in creating temporary file");
    abort();
  }
  FILE* out = fdopen(outfd, "w");
  assert(out != NULL);
  fprintf(out, "- image: %s\n", Image_name);
  fprintf(out, "  definition: %s\n", Current_definition);
  fprintf(out, "  rowoff: %d\n", rowoff);
  fprintf(out, "  coloff: %d\n", coloff);
  fprintf(out, "  cy: %d\n", cy);
  fprintf(out, "  cx: %d\n", cx);
  fclose(out);
  rename(outfilename, "teliva_editor_state");
}

/* when found, return 1 and leave string on top of stack
 * when not found, return 0
 * caller is responsible for cleaning up the stack. */
static int look_up_definition (lua_State* L, const char* name) {
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

void save_to_current_definition_and_editor_buffer(lua_State* L, const char* definition) {
  int oldtop = lua_gettop(L);
  strncpy(Current_definition, definition, CURRENT_DEFINITION_LEN);
  int status = look_up_definition(L, Current_definition);
  char outfilename[] = "teliva_editor_buffer_XXXXXX";
  int outfd = mkstemp(outfilename);
  if (outfd == -1) {
    endwin();
    perror("save_to_current_definition_and_editor_buffer: error in creating temporary file");
    abort();
  }
  FILE* out = fdopen(outfd, "w");
  assert(out != NULL);
  if (status)
    fprintf(out, "%s", lua_tostring(L, -1));
  fclose(out);
  rename(outfilename, "teliva_editor_buffer");
  lua_settop(L, oldtop);
}

/* I don't understand the best way to read all of a text file.
 * I'm currently using fread, but its error handling is really designed for
 * binary data containing fixed-size records. */
static void read_editor_buffer(char* out, int capacity) {
  FILE* in = fopen("teliva_editor_buffer", "r");
  fread(out, capacity, 1, in);  /* TODO: handle overly large file */
  fclose(in);
}

static void update_definition(lua_State* L, const char* name, char* new_contents) {
  int oldtop = lua_gettop(L);
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
  char* time_string = ctime(&t);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  lua_settop(L, oldtop);
}

extern void save_tlv(lua_State* L, char* filename);
extern int docall(lua_State* L, int narg, int clear);
extern int load_editor_buffer_to_current_definition_in_image(lua_State* L) {
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents, 8190);
  update_definition(L, Current_definition, new_contents);
  save_tlv(L, Image_name);
  /* reload binding */
  return luaL_loadbuffer(L, new_contents, strlen(new_contents), Current_definition)
      || docall(L, 0, 1);
}

/* return true if user chose to back into the big picture view */
/* But only if there are no errors. Otherwise things can get confusing. */
extern int edit(lua_State* L, char* filename);
static int edit_current_definition(lua_State* L) {
  int back_to_big_picture = edit(L, "teliva_editor_buffer");
  // error handling
  int oldtop = lua_gettop(L);
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
  if (lua_gettop(L) != oldtop) {
    endwin();
    printf("edit_current_definition: memory leak %d -> %d\n", oldtop, lua_gettop(L));
    exit(1);
  }
  return back_to_big_picture;
}

static void recent_changes_menu(int cursor, int history_array_size) {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
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
static int render_wrapped_lua_text(int y, int xmin, int xmax, const char* text) {
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

static void render_recent_changes(lua_State* L, int start_index) {
  clear();
  attrset(A_BOLD);
  mvaddstr(1, 0, "Recent changes");
  attrset(A_NORMAL);
  int oldtop = lua_gettop(L);
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  int history_array_size = luaL_getn(L, history_array);
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
      const char* definition_name = lua_tostring(L, -2);
      if (is_special_history_key(definition_name)) continue;
      addstr(definition_name);
      /* save timestamp of binding if available */
      lua_getfield(L, t, "__teliva_timestamp");
      if (!lua_isnil(L, -1)) {
        char buffer[128] = {0};
        strncpy(buffer, lua_tostring(L, -1), 120);
        if (buffer[strlen(buffer)-1] == '\n')
          buffer[strlen(buffer)-1] = '\0';
        attron(COLOR_PAIR(COLOR_PAIR_FADE));
        printw("  %s", buffer);
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
      const char* definition_contents = lua_tostring(L, -1);
      y = render_wrapped_lua_text(y, 0, COLS, definition_contents);
      y++;
      if (y >= LINES-1) break;  /* leave cruft on the stack */
    }
    lua_settop(L, t);  /* clean up cruft on the stack */
    lua_pop(L, 1);  // history element
    y++;
    if (y >= LINES-1) break;
  }
  lua_pop(L, 1);  // history array
  if (lua_gettop(L) != oldtop) {
    endwin();
    printf("render_recent_changes: memory leak %d -> %d\n", oldtop, lua_gettop(L));
    exit(1);
  }
  recent_changes_menu(start_index, history_array_size);
  refresh();
}

static void add_undo_event(lua_State* L, int cursor) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  /* create a new table containing the undo event */
  lua_createtable(L, /*number of fields per mutation*/2, 0);
  lua_pushinteger(L, cursor);
  lua_setfield(L, -2, "__teliva_undo");
  /* include timestamp at which event was created */
  time_t t;
  time(&t);
  char* time_string = ctime(&t);
  lua_pushstring(L, time_string);
  lua_setfield(L, -2, "__teliva_timestamp");
  /* append the new table to the history of mutations */
  int history_array_size = luaL_getn(L, history_array);
  ++history_array_size;
  lua_rawseti(L, history_array, history_array_size);
  /* clean up */
  lua_pop(L, 1);
}

static void save_note_to_editor_buffer(lua_State* L, int cursor) {
  lua_getglobal(L, "teliva_program");
  lua_rawgeti(L, -1, cursor);
  lua_getfield(L, -1, "__teliva_note");
  const char* contents = lua_tostring(L, -1);
  char outfilename[] = "teliva_editor_buffer_XXXXXX";
  int outfd = mkstemp(outfilename);
  if (outfd == -1) {
    endwin();
    perror("save_note_to_editor_buffer: error in creating temporary file");
    abort();
  }
  FILE* out = fdopen(outfd, "w");
  assert(out != NULL);
  if (contents != NULL)
    fprintf(out, "%s", contents);
  fclose(out);
  rename(outfilename, "teliva_editor_buffer");
  lua_pop(L, 3);  /* contents, table at cursor, teliva_program */
}

static void load_note_from_editor_buffer(lua_State* L, int cursor) {
  lua_getglobal(L, "teliva_program");
  char new_contents[8192] = {0};
  read_editor_buffer(new_contents, 8190);
  lua_rawgeti(L, -1, cursor);
  lua_pushstring(L, new_contents);
  lua_setfield(L, -2, "__teliva_note");
  lua_pop(L, 2);  /* table at cursor, teliva_program */
}

static void recent_changes_view(lua_State* L) {
  lua_getglobal(L, "teliva_program");
  int history_array = lua_gettop(L);
  assert(history_array == 1);
  int history_array_size = luaL_getn(L, history_array);
  int cursor = history_array_size;
  lua_pop(L, 1);
  int quit = 0;
  while (!quit) {
    /* refresh state after each operation so we pick up modifications */
    render_recent_changes(L, cursor);
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
  }
}

static int binding_exists (lua_State *L, const char *name) {
  int result = 0;
  lua_getglobal(L, name);
  result = !lua_isnil(L, -1);
  lua_pop(L, 1);
  return result;
}

extern int dostring(lua_State* L, const char* s, const char* name);
static int load_definitions(lua_State* L) {
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

static int run_tests(lua_State* L) {
  clear();
  lua_pushinteger(L, 0);
  lua_setglobal(L, "teliva_num_test_failures");
  lua_pushnil(L);
  lua_setglobal(L, "teliva_first_failure");
  lua_pushvalue(L, LUA_GLOBALSINDEX);
  int table = lua_gettop(L);
  for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
    const char* key = lua_tostring(L, -2);
    if (strncmp("test_", key, strlen("test_")) != 0) continue;
    if (!lua_isfunction(L, -1)) continue;
    int status = lua_pcall(L, 0, 0, 0);
    if (status) {
      printw("E%d: %s", status, lua_tostring(L, -1));
      /* increment teliva_num_test_failures */
      lua_getglobal(L, "teliva_num_test_failures");
      int num_failures = lua_tointeger(L, -1);
      lua_pop(L, 1);
      lua_pushinteger(L, num_failures+1);
      lua_setglobal(L, "teliva_num_test_failures");
      /* if unset, set teliva_first_failure */
      lua_getglobal(L, "teliva_first_failure");
      int first_failure_clear = lua_isnil(L, -1);
      lua_pop(L, 1);
      if (first_failure_clear)
        lua_setglobal(L, "teliva_first_failure");
    }
    lua_pushnil(L);  /* just to undo loop update */
  }
  lua_pop(L, 1);
  lua_getglobal(L, "teliva_num_test_failures");
  int num_failures = lua_tointeger(L, -1);
  lua_pop(L, 1);
  if (num_failures == 0) return 0;
  if (num_failures == 1)
    addstr("1 failure");
  else
    printw("%d failures", num_failures);
  getch();
  /* take first failure back to developer mode */
  lua_getglobal(L, "teliva_first_failure");
  assert(!lua_isnil(L, -1));
  return 1;
}

static void clear_call_graph(lua_State* L) {
  int oldtop = lua_gettop(L);
  luaL_newmetatable(L, "__teliva_call_graph_depth");
  int cgt = lua_gettop(L);
  lua_pushnil(L);
  while (lua_next(L, cgt) != 0) {
    lua_pop(L, 1);  /* old value */
    lua_pushvalue(L, -1);  /* duplicate key */
    lua_pushnil(L);  /* new value */
    lua_settable(L, cgt);
    /* one copy of key left for lua_next */
  }
  lua_pop(L, 1);
  assert(lua_gettop(L) == oldtop);
}

int file_operations_allowed = false;
int net_operations_allowed = false;

static void permissions_menu() {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
  menu_column = 2;
  draw_menu_item("^x", "go back");
  draw_menu_item("^f", "toggle file permissions");
  draw_menu_item("^n", "toggle network permissions");
  attrset(A_NORMAL);
}

static void render_permissions_screen(lua_State* L) {
  clear();
  attrset(A_BOLD);
  mvaddstr(1, 20, "😈 When can apps perform...? 😈");
//?   mvaddstr(1, 30, "😈 ⛧  When can apps perform...? ⛧ 😈");  // most fonts don't have pentagrams
  attrset(A_NORMAL);
  int file_colors = file_operations_allowed ? COLOR_PAIR_WARN : COLOR_PAIR_SAFE;
  int net_colors = net_operations_allowed ? COLOR_PAIR_WARN : COLOR_PAIR_SAFE;
  if (file_operations_allowed && net_operations_allowed) {
    file_colors = net_colors = COLOR_PAIR_RISK;
  }

  attron(COLOR_PAIR(file_colors));
  mvaddstr(3, 5, "File operations");
  attron(A_REVERSE);
  switch (file_colors) {
    case COLOR_PAIR_SAFE:
      mvaddstr(3, 30, " never                         ");
      break;
    case COLOR_PAIR_WARN:
      mvaddstr(3, 30, " always                        ");
      break;
    case COLOR_PAIR_RISK:
      mvaddstr(3, 30, "                               ");
      break;
    default:
      abort();
  }
  attroff(A_REVERSE);
  attroff(COLOR_PAIR(file_colors));

  attron(COLOR_PAIR(net_colors));
  mvaddstr(5, 5, "Network operations");
  attron(A_REVERSE);
  switch (net_colors) {
    case COLOR_PAIR_SAFE:
      mvaddstr(5, 30, " never                         ");
      break;
    case COLOR_PAIR_WARN:
      mvaddstr(5, 30, " always                        ");
      break;
    case COLOR_PAIR_RISK:
      mvaddstr(5, 30, "                               ");
      break;
    default:
      abort();
  }
  attroff(A_REVERSE);
  attroff(COLOR_PAIR(net_colors));

  if (file_operations_allowed && net_operations_allowed) {
    attron(COLOR_PAIR(COLOR_PAIR_RISK));
    mvaddstr(8, 5, "⚠️  Teliva can't protect you if this app does something sketchy. Consider choosing stronger conditions. ⚠️");
//?     mvaddstr(8, 5, "🦮 ⚖ 🙈 Teliva can't tell how much it's protecting you. Consider simplifying the conditions.");
    attroff(COLOR_PAIR(COLOR_PAIR_RISK));
  }
  permissions_menu();
  refresh();
}

static void permissions_view(lua_State* L) {
  while (true) {
    render_permissions_screen(L);
    int c = getch();
    switch (c) {
      case CTRL_X:
        return;
      case CTRL_F:
        file_operations_allowed = !file_operations_allowed;
        break;
      case CTRL_N:
        net_operations_allowed = !net_operations_allowed;
        break;
    }
  }
}

static const char* user_configuration_filename() {
  const char* home = getenv("HOME");
  if (home == NULL) {
    endwin();
    fprintf(stderr, "$HOME is not set; unclear where to save permissions.\n");
    abort();
  }
  static char config_filename[1024] = {0};
  memset(config_filename, '\0', 1024);
  const char* config_home = getenv("XDG_CONFIG_HOME");
  if (config_home == NULL)
    snprintf(config_filename, 1024, "%s/.teliva", home);
  else
    snprintf(config_filename, 1024, "%s/.teliva", config_home);
  return config_filename;
}

static void save_permissions_to_user_configuration(lua_State* L) {
  const char* rcfilename = user_configuration_filename();
  FILE* in = fopen(rcfilename, "r");  /* can be NULL when rcfile doesn't exist */
  char outfilename[] = "telivarc_XXXXXX";
  int outfd = mkstemp(outfilename);
  if (outfd == -1) {
    endwin();
    perror("error in creating temporary file");
    abort();
  }
  FILE* out = fdopen(outfd, "w");
  assert(out != NULL);
  /* read entries from rcfilename and write them to outfilename. If image name
   * matches the current Image_name, ignore. */
  int oldtop = lua_gettop(L);
  while (in && !feof(in)) {
    teliva_load_definition(L, in);
    if (lua_isnil(L, -1)) break;
    lua_getfield(L, -1, "image_name");
    const char* image_name = lua_tostring(L, -1);
    if (strcmp(image_name, Image_name) != 0) {
      fprintf(out, "- image_name: %s\n", image_name);
      lua_getfield(L, -2, "file_operations_allowed");
      fprintf(out, "  file_operations_allowed: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);  /* file_operations_allowed */
      lua_getfield(L, -2, "net_operations_allowed");
      fprintf(out, "  net_operations_allowed: %s\n", lua_tostring(L, -1));
      lua_pop(L, 1);  /* net_operations_allowed */
    }
    lua_pop(L, 1);  /* image_name */
  }
  lua_settop(L, oldtop);
  fprintf(out, "- image_name: %s\n", Image_name);
  fprintf(out, "  file_operations_allowed: %d\n", file_operations_allowed);
  fprintf(out, "  net_operations_allowed: %d\n", net_operations_allowed);
  fclose(out);
  if (in) fclose(in);
  rename(outfilename, rcfilename);
}

static void load_permissions_from_user_configuration(lua_State* L) {
  const char* rcfilename = user_configuration_filename();
  FILE* in = fopen(rcfilename, "r");
  if (in == NULL) return;
  /* read entries from rcfilename and look for a match with the current
   * Image_name. */
  int oldtop = lua_gettop(L);
  while (!feof(in)) {
    teliva_load_definition(L, in);
    if (lua_isnil(L, -1)) break;
    lua_getfield(L, -1, "image_name");
    const char* image_name = lua_tostring(L, -1);
    if (strcmp(image_name, Image_name) == 0) {
      lua_getfield(L, -2, "file_operations_allowed");
      file_operations_allowed = lua_tointeger(L, -1);
      lua_pop(L, 1);  /* file_operations_allowed */
      lua_getfield(L, -2, "net_operations_allowed");
      net_operations_allowed = lua_tointeger(L, -1);
      lua_pop(L, 1);  /* net_operations_allowed */
    }
    lua_pop(L, 1);  /* image_name */
  }
  lua_settop(L, oldtop);
  fclose(in);
}

void permissions_mode(lua_State* L) {
  assume_default_colors(COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_NORMAL, COLOR_FOREGROUND, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_SELECTABLE, COLOR_SELECTABLE_FOREGROUND, COLOR_SELECTABLE_BACKGROUND);
  init_pair(COLOR_PAIR_FADE, COLOR_FADE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MENU_ALTERNATE, COLOR_MENU_ALTERNATE, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_COMMENT, COLOR_LUA_COMMENT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_KEYWORD, COLOR_LUA_KEYWORD, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_LUA_CONSTANT, COLOR_LUA_CONSTANT, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_MATCH, COLOR_MATCH_FOREGROUND, COLOR_MATCH_BACKGROUND);
  init_pair(COLOR_PAIR_ERROR, COLOR_ERROR_FOREGROUND, COLOR_ERROR_BACKGROUND);
  /* permissions colors slightly different than in the menu */
  init_pair(COLOR_PAIR_SAFE, COLOR_SAFE_NORMAL, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_WARN, COLOR_WARN_NORMAL, COLOR_BACKGROUND);
  init_pair(COLOR_PAIR_RISK, COLOR_RISK_NORMAL, COLOR_BACKGROUND);
  nodelay(stdscr, 0);  /* always make getch() block in developer mode */
  curs_set(1);  /* always display cursor in developer mode */
  permissions_view(L);
  save_permissions_to_user_configuration(L);
  cleanup_curses();
  execv(Argv[0], Argv);
  /* never returns */
}

char* Image_name = NULL;
extern void set_args (lua_State *L, char **argv, int n);
extern void load_tlv(lua_State* L, char* filename);
int handle_image(lua_State* L, char** argv, int n) {
  int status;
  set_args(L, argv, n);
  /* parse and load file contents (teliva_program array) */
  Image_name = argv[n];
  load_tlv(L, Image_name);
//?   save_tlv(L, Image_name);  // manual test; should always return identical result, modulo key order
//?   exit(1);
  status = load_definitions(L);
  if (status != 0) return 0;
  status = run_tests(L);
  if (status != 0) return report_in_developer_mode(L, status);
  /* clear callgraph stats from running tests */
  clear_call_graph(L);
  /* initialize permissions */
  load_permissions_from_user_configuration(L);
  /* call main() */
  lua_getglobal(L, "main");
  status = docall(L, 0, 1);
  if (status != 0) return report_in_developer_mode(L, status);
  return 0;
}
