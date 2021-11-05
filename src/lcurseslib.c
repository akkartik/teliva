#include <ncurses.h>

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


static int optint (lua_State *L, int narg, lua_Integer def) {
  if (lua_isnoneornil(L, narg))
    return cast(int, def);
  return cast(int, checkinteger(L, narg, "int or nil"));
}


static int Waddstr (lua_State *L) {
  WINDOW *w = checkwin(L, 1);
  const char *str = luaL_checkstring(L, 2);
  int n = optint(L, 3, -1);
  lua_pushboolean(L, waddnstr(w, str, n));
  return 1;
}


static const luaL_Reg curses_window_methods[] =
{
  {"addstr", Waddstr},
  {NULL, NULL}
};


LUALIB_API int luaopen_curses (lua_State *L) {
  luaL_newmetatable(L, "curses:window");
  luaL_register(L, NULL, curses_window_methods);
  luaL_register(L, "curses", curseslib);
  /* save main window on registry */
  curses_newwin(L, stdscr);
  lua_pushstring(L, "curses:stdscr");
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  return 1;
}

