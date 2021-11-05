#include <ncurses.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"


static int Pstdscr(lua_State *L) {
  lua_pushstring(L, "curses:stdscr");
  lua_rawget(L, LUA_REGISTRYINDEX);
  return 1;
}


static int Pcols(lua_State *L) {
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
    luaL_getmetatable(L, "meta.window");
    lua_setmetatable(L, -2);
    *w = nw;
  }
  else {
    lua_pushliteral(L, "failed to create window");
    lua_error(L);
  }
}


LUALIB_API int luaopen_curses (lua_State *L) {
  luaL_register(L, "curses", curseslib);
  /* save main window on registry */
  curses_newwin(L, stdscr);
  lua_pushstring(L, "curses:stdscr");
  lua_pushvalue(L, -2);
  lua_rawset(L, LUA_REGISTRYINDEX);
  return 1;
}

