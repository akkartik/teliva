#include <ncurses.h>
#include <string.h>

#include "lua.h"
#include "llimits.h"

#include "lauxlib.h"
#include "lualib.h"


static int Pstdscr (lua_State *L) {
  lua_pushstring(L, "curses:stdscr");
  lua_rawget(L, LUA_REGISTRYINDEX);
  return 1;
}


static int Pcols (lua_State *L) {
  lua_pushinteger(L, COLS);
  return 1;
}


static const struct luaL_Reg curseslib [] = {
  {"cols", Pcols},
  {"stdscr", Pstdscr},
  {NULL, NULL}
};


static void curses_newwin (lua_State *L, WINDOW *nw) {
  if (nw) {
    WINDOW **w = lua_newuserdata(L, sizeof(WINDOW*));
    luaL_getmetatable(L, "curses:window");
    lua_setmetatable(L, -2);
    *w = nw;
  }
  else {
    lua_pushliteral(L, "failed to create window");
    lua_error(L);
  }
}


static WINDOW **lc_getwin (lua_State *L, int offset) {
  WINDOW **w = (WINDOW**)luaL_checkudata(L, offset, "curses:window");
  if (w == NULL)
    luaL_argerror(L, offset, "bad curses window");
  return w;
}


static WINDOW *checkwin (lua_State *L, int offset) {
  WINDOW **w = lc_getwin(L, offset);
  if (*w == NULL)
    luaL_argerror(L, offset, "attempt to use closed curses window");
  return *w;
}


static int
argtypeerror(lua_State *L, int narg, const char *expected)
{
  const char *got = luaL_typename(L, narg);
  return luaL_argerror(L, narg,
          lua_pushfstring(L, "%s expected, got %s", expected, got));
}


static lua_Integer checkinteger (lua_State *L, int narg, const char *expected) {
  lua_Integer d = lua_tointeger(L, narg);
  if (d == 0 && !lua_isnumber(L, narg))
    argtypeerror(L, narg, expected);
  return d;
}


static chtype checkch (lua_State *L, int narg) {
  if (lua_isnumber(L, narg))
    return cast(chtype, checkinteger(L, narg, "int"));
  if (lua_isstring(L, narg))
    return *lua_tostring(L, narg);

  return argtypeerror(L, narg, "int or char");
}


static int optint (lua_State *L, int narg, lua_Integer def) {
  if (lua_isnoneornil(L, narg))
    return cast(int, def);
  return cast(int, checkinteger(L, narg, "int or nil"));
}


static int W__tostring (lua_State *L) {
  WINDOW **w = lc_getwin(L, 1);
  char buff[34];
  if (*w == NULL)
    strcpy(buff, "closed");
  else
    sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "curses window (%s)", buff);
  return 1;
}


static int Waddstr (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  const char *str = luaL_checkstring(L, 2);
  int n = optint(L, 3, -1);
  lua_pushboolean(L, waddnstr(w, str, n));
  return 1;
}


static int Wattroff (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int attrs = checkinteger(L, 2, "int");
  lua_pushboolean(L, wattroff(w, attrs));
  return 1;
}


static int Wattron (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int attrs = checkinteger(L, 2, "int");
  lua_pushboolean(L, wattron(w, attrs));
  return 1;
}


static int Wclear (lua_State *L) {
  lua_pushboolean(L, wclear(checkwin(L, 1)));
  return 1;
}


static int Wgetyx (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int y, x;
  getyx(w, y, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, x);
  return 2;
}


static int Wgetmaxyx (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int y, x;
  getmaxyx(w, y, x);
  lua_pushinteger(L, y);
  lua_pushinteger(L, x);
  return 2;
}


static int Wmvaddch (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int y = checkinteger(L, 2, "int");
  int x = checkinteger(L, 3, "int");
  chtype ch = checkch(L, 4);
  mvwaddch(w, y, x, ch);
  return 1;
}


static int Wmvaddstr (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  int y = checkinteger(L, 2, "int");
  int x = checkinteger(L, 3, "int");
  const char *str = luaL_checkstring(L, 4);
  int n = optint(L, 5, -1);
  mvwaddnstr(w, y, x, str, n);
  return 1;
}


static const luaL_Reg curses_window_methods[] =
{
  {"__tostring", W__tostring},
  {"addstr", Waddstr},
  {"attroff", Wattroff},
  {"attron", Wattron},
  {"clear", Wclear},
  {"getmaxyx", Wgetmaxyx},
  {"getyx", Wgetyx},
  {"mvaddch", Wmvaddch},
  {"mvaddstr", Wmvaddstr},
  {NULL, NULL}
};


static void register_curses_constant(lua_State *L, const char* name, int val) {
  lua_pushstring(L, name);
  lua_pushinteger(L, val);
  lua_settable(L, -3);
}


static void register_curses_constants (lua_State *L) {
#define CC(s) register_curses_constant(L, #s, s)
  /* colors */
  CC(COLOR_BLACK);
  CC(COLOR_RED);
  CC(COLOR_GREEN);
  CC(COLOR_YELLOW);
  CC(COLOR_BLUE);
  CC(COLOR_MAGENTA);
  CC(COLOR_CYAN);
  CC(COLOR_WHITE);

  /* alternate character set */
  CC(ACS_BLOCK);
  CC(ACS_BOARD);

  CC(ACS_BTEE);
  CC(ACS_TTEE);
  CC(ACS_LTEE);
  CC(ACS_RTEE);
  CC(ACS_LLCORNER);
  CC(ACS_LRCORNER);
  CC(ACS_URCORNER);
  CC(ACS_ULCORNER);

  CC(ACS_LARROW);
  CC(ACS_RARROW);
  CC(ACS_UARROW);
  CC(ACS_DARROW);

  CC(ACS_HLINE);
  CC(ACS_VLINE);

  CC(ACS_BULLET);
  CC(ACS_CKBOARD);
  CC(ACS_LANTERN);
  CC(ACS_DEGREE);
  CC(ACS_DIAMOND);

  CC(ACS_PLMINUS);
  CC(ACS_PLUS);
  CC(ACS_S1);
  CC(ACS_S9);

  /* attributes */
  CC(A_NORMAL);
  CC(A_STANDOUT);
  CC(A_UNDERLINE);
  CC(A_REVERSE);
  CC(A_BLINK);
  CC(A_DIM);
  CC(A_BOLD);
  CC(A_PROTECT);
  CC(A_INVIS);
  CC(A_ALTCHARSET);
  CC(A_CHARTEXT);
  CC(A_ATTRIBUTES);
  CC(A_COLOR);
#undef CC
}


LUALIB_API int luaopen_curses (lua_State *L) {
  luaL_newmetatable(L, "curses:window");

  /* metatable.__index = metatable */
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  luaL_register(L, NULL, curses_window_methods);

  luaL_register(L, "curses", curseslib);

  /* save main window on registry */
  curses_newwin(L, stdscr);
  lua_pushstring(L, "curses:stdscr");
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);

  lua_pushvalue(L, -2);
  register_curses_constants(L);
  return 1;
}

