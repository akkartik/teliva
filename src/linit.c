/*
** $Id: linit.c,v 1.14.1.1 2007/12/27 13:02:25 roberto Exp $
** Initialization of libraries for lua.c
** See Copyright Notice in lua.h
*/


#define linit_c
#define LUA_LIB

#include "lua.h"

#include "lualib.h"
#include "lauxlib.h"


static const luaL_Reg lualibs[] = {
  {"", luaopen_base},
  {LUA_TABLIBNAME, luaopen_table},
  {LUA_IOLIBNAME, luaopen_io},
  {LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_LFSLIBNAME, luaopen_lfs},
  {LUA_CURSESLIBNAME, luaopen_curses},
  {LUA_SOCKETCORELIBNAME, luaopen_socket_core},
  {LUA_MIMECORELIBNAME, luaopen_mime_core},
  {LUA_SSLLIBNAME, luaopen_ssl_core},
  {LUA_SSLCONTEXTLIBNAME, luaopen_ssl_context},
  {LUA_SSLX509LIBNAME, luaopen_ssl_x509},
  {LUA_SSLCONFIGLIBNAME, luaopen_ssl_config},
  {NULL, NULL}
};


LUALIB_API void luaL_openlibs (lua_State *L) {
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}

