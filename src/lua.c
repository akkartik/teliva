/*
** $Id: lua.c,v 1.160.1.2 2007/12/28 15:32:23 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/


#include <locale.h>
#ifdef __NetBSD__
#include <curses.h>
#else
#include <ncurses.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lua_c

#include "lua.h"
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


int docall (lua_State *L, int narg, int clear) {
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
void set_args (lua_State *L, char **argv, int n) {
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


int dostring (lua_State *L, const char *s, const char *name) {
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
  luaL_openlibs(L);
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
  status = dorequire(L, "src/jsonf.lua", "jsonf");
  if (status != 0) return 0;
  status = dorequire(L, "src/task.lua", "task");
  if (status != 0) return 0;
  status = dorequire(L, "src/file.lua", "file");
  if (status != 0) return 0;
  lua_gc(L, LUA_GCRESTART, 0);
  s->status = handle_luainit(L);
  if (s->status != 0) return 0;
  s->status = handle_image(L, argv, 1);
  if (s->status != 0) return 0;
  return 0;
}


extern void cleanup_curses(void);
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
  render_trusted_teliva_data(L);
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

