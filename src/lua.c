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
#include <unistd.h>

#define lua_c

#include "lua.h"

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
  if (pname) printw("%s: ", pname);
  printw("%s\n", msg);
  refresh();
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


void stackDump (lua_State *L) {
  int i;
  int top = lua_gettop(L);
  for (i = 1; i <= top; i++) {  /* repeat for each level */
    int t = lua_type(L, i);
    switch (t) {

      case LUA_TSTRING:  /* strings */
        printf("`%s'", lua_tostring(L, i));
        break;

      case LUA_TBOOLEAN:  /* booleans */
        printf(lua_toboolean(L, i) ? "true" : "false");
        break;

      case LUA_TNUMBER:  /* numbers */
        printf("%g", lua_tonumber(L, i));
        break;

      default:  /* other values */
        printf("%s", lua_typename(L, t));
        break;

    }
    printf("  ");  /* put a separator */
  }
  printf("\n");  /* end the listing */
}


char *Image_name = NULL;
static int handle_image (lua_State *L, char **argv, int n) {
  int status;
  int narg = getargs(L, argv, n);  /* collect arguments */
  lua_setglobal(L, "arg");
  /* parse and load file contents (teliva_program table) */
  Image_name = argv[n];
  status = luaL_loadfile(L, Image_name);
  lua_insert(L, -(narg+1));
  if (status != 0) {
    return status;
  }
  status = docall(L, narg, 0);
  lua_getglobal(L, "teliva_program");
  int table = lua_gettop(L);
  /* parse and load each binding in teliva_program */
  for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
    const char* key = lua_tostring(L, -2);
    const char* value = lua_tostring(L, -1);
    dostring(L, value, key);
  }
  /* call main() */
  lua_getglobal(L, "main");
  docall(L, 0, 1);
  return 0;
}


#define CURRENT_DEFINITION_LEN 256
char Current_definition[CURRENT_DEFINITION_LEN+1] = {0};
void save_to_current_definition_and_editor_buffer (lua_State *L, const char *definition) {
    strncpy(Current_definition, definition, CURRENT_DEFINITION_LEN);
    lua_getglobal(L, "teliva_program");
    lua_getfield(L, -1, Current_definition);
    const char *contents = lua_tostring(L, -1);
    lua_pop(L, 1);
    int outfd = open("teliva_editbuffer", O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if (contents != NULL)
      write(outfd, contents, strlen(contents));
    close(outfd);
    lua_settop(L, 0);
}


static void read_contents (lua_State *L, char *filename, char *out) {
    int infd = open(filename, O_RDONLY);
    read(infd, out, 8190);  /* TODO: handle overly large file */
    close(infd);
}


/* table to update is at top of stack */
static void update_definition (lua_State *L, const char *name, char *out) {
    lua_getglobal(L, "teliva_program");
    lua_pushstring(L, out);
    assert(strlen(name) > 0);
    lua_setfield(L, -2, name);
    lua_settop(L, 0);
}


static void save_image (lua_State *L) {
    lua_getglobal(L, "teliva_program");
    int table = lua_gettop(L);
    FILE* fp = fopen(Image_name, "w");
    fprintf(fp, "teliva_program = {\n");
    for (lua_pushnil(L); lua_next(L, table) != 0; lua_pop(L, 1)) {
      const char* key = lua_tostring(L, -2);
      const char* value = lua_tostring(L, -1);
      fprintf(fp, "  %s = [==[", key);
      fprintf(fp, "%s", value);
      fprintf(fp, "]==],\n");
    }
    fprintf(fp, "}\n");
    fclose(fp);
    lua_settop(L, 0);
}


/* death and rebirth */
char **Argv = NULL;
extern void edit(lua_State *L, char *filename, const char *message);
extern void editorClear(void);
extern int editorOpen(char *filename);
void editBuffer(lua_State* L, const char* message) {
  edit(L, "teliva_editbuffer", message);
}
void editorRefreshBuffer(void) {
  editorClear();
  editorOpen("teliva_editbuffer");
}


void load_editor_buffer_to_current_definition_in_image(lua_State *L) {
  char new_contents[8192] = {0};
  read_contents(L, "teliva_editbuffer", new_contents);
  update_definition(L, Current_definition, new_contents);
  save_image(L);
  /* reload binding if possible */
  dostring(L, new_contents, Current_definition);
}


void editImage (lua_State *L, const char *definition) {
  save_to_current_definition_and_editor_buffer(L, definition);
  editBuffer(L, /*status message*/ "");
  load_editor_buffer_to_current_definition_in_image(L);
}


extern void draw_menu_item (const char* key, const char* name);
static void browserMenu (void) {
  attrset(A_REVERSE);
  for (int x = 0; x < COLS; ++x)
    mvaddch(LINES-1, x, ' ');
  attrset(A_NORMAL);
  extern int menu_column;
  menu_column = 2;
  draw_menu_item("Esc", "go back");
  draw_menu_item("Enter", "submit");
  attrset(A_NORMAL);
}


#define BG(i) (COLOR_PAIR((i)+8))
#define FG(i) (COLOR_PAIR(i))
void browseDefinition (const char *definition_name) {
  attron(BG(7));
  addstr(definition_name);
  attrset(A_NORMAL);
  addstr("  ");
}

/* return true if submitted */
int browseImage (lua_State *L) {
  clear();
  luaL_newmetatable(L, "__teliva_call_graph_depth");
  int cgt = lua_gettop(L);
  // special-case: we don't instrument the call to main, but it's always 1
  lua_pushinteger(L, 1);
  lua_setfield(L, cgt, "main");
  // segment definitions by depth
  lua_getglobal(L, "teliva_program");
  int t = lua_gettop(L);

  int y = 2;
  mvaddstr(y, 0, "data:           ");
  // first: data (non-functions) that's not the Teliva menu or curses variables
  for (lua_pushnil(L); lua_next(L, t) != 0;) {
    const char *definition_name = lua_tostring(L, -2);
    lua_getglobal(L, definition_name);
    int is_userdata = lua_isuserdata(L, -1);
    int is_function = lua_isfunction(L, -1);
    lua_pop(L, 1);
    if (strcmp(definition_name, "menu") != 0  // required by all Teliva programs
        && !is_function  // functions are not data
        && !is_userdata  // including curses window objects
                         // (unlikely to have an interesting definition)
    ) {
      browseDefinition(definition_name);
    }
    lua_pop(L, 1);  // value
    // leave key on stack for next iteration
  }

  // second: menu and other userdata
  for (lua_pushnil(L); lua_next(L, t) != 0;) {
    const char* definition_name = lua_tostring(L, -2);
    lua_getglobal(L, definition_name);
    int is_userdata = lua_isuserdata(L, -1);
    lua_pop(L, 1);
    if (strcmp(definition_name, "menu") == 0
        || is_userdata  // including curses window objects
    ) {
      browseDefinition(definition_name);
    }
    lua_pop(L, 1);  // value
    // leave key on stack for next iteration
  }

  // functions by level
  y += 2;
  mvprintw(y, 0, "functions: ");
  y++;
  for (int level = 1; level < 5; ++level) {
    mvaddstr(y, 0, "                ");
    for (lua_pushnil(L); lua_next(L, t) != 0;) {
      const char* definition_name = lua_tostring(L, -2);
      lua_getfield(L, cgt, definition_name);
      int depth = lua_tointeger(L, -1);
      if (depth == level)
        browseDefinition(definition_name);
      lua_pop(L, 1);  // depth of value
      lua_pop(L, 1);  // value
      // leave key on stack for next iteration
    }
    y += 2;
  }

  // unused functions
  mvaddstr(y, 0, "                ");
  for (lua_pushnil(L); lua_next(L, t) != 0;) {
    const char* definition_name = lua_tostring(L, -2);
    lua_getglobal(L, definition_name);
    int is_function = lua_isfunction(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, cgt, definition_name);
    if (is_function && lua_isnoneornil(L, -1))
      browseDefinition(definition_name);
    lua_pop(L, 1);  // depth of value
    lua_pop(L, 1);  // value
    // leave key on stack for next iteration
  }

  lua_settop(L, 0);

  enum {
    ENTER = 10,
    CTRL_U = 21,
    ESC = 27,
  };
  char query[CURRENT_DEFINITION_LEN+1] = {0};
  int qlen = 0;
  while (1) {
    browserMenu();
    mvprintw(LINES-2, 0, "Edit: %s", query);
    int c = getch();
    if (c == KEY_BACKSPACE) {
      if (qlen != 0) query[--qlen] = '\0';
    } else if (c == ESC) {
      return 0;
    } else if (c == ENTER) {
      editImage(L, query);
      return 1;
    } else if (c == CTRL_U) {
      query[0] = '\0';
      qlen = 0;
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
void switch_to_editor (lua_State *L, const char *message) {
  /* clobber the app's ncurses colors; we'll restart the app when we rerun it. */
  for (int i = 0; i < 8; ++i)
    init_pair(i, i, -1);
  for (int i = 0; i < 8; ++i)
    init_pair(i+8, -1, i);
  if (browseImage(L)) {
    cleanup_curses();
  }
  execv(Argv[0], Argv);
  /* never returns */
}


const char *Previous_error = NULL;
static int show_error_in_editor (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    Previous_error = lua_tostring(L, -1);
    if (Previous_error == NULL) Previous_error = "(error object is not a string)";
    switch_to_editor(L, Previous_error);
  }
  return status;
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
        *pi = 1;  /* go through */
      case 'v':
        notail(argv[i]);
        *pv = 1;
        break;
      case 'e':
        *pe = 1;  /* go through */
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


struct Smain {
  int argc;
  char **argv;
  int status;
};


static int pmain (lua_State *L) {
  struct Smain *s = (struct Smain *)lua_touserdata(L, 1);
  char **argv = s->argv;
  int image;
  int has_i = 0, has_v = 0, has_e = 0;
  globalL = L;
  if (argv[0] && argv[0][0]) progname = argv[0];
  lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
  luaL_openlibs(L);  /* open libraries */
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

