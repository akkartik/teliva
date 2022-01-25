/*
 * POSIX library for Lua 5.1, 5.2 & 5.3.
 * (c) Gary V. Vaughan <gary@vaughan.pe>, 2013-2017
 * (c) Reuben Thomas <rrt@sc3d.org> 2010-2013
 * (c) Natanael Copa <natanael.copa@gmail.com> 2008-2010
 * Clean up and bug fixes by Leo Razoumov <slonik.az@gmail.com> 2006-10-11
 * Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br> 07 Apr 2006 23:17:49
 * Based on original by Claudio Terra for Lua 3.x.
 * With contributions by Roberto Ierusalimschy.
 * With documentation from Steve Donovan 2012
 */

#ifndef LCURSES__HELPERS_C
#define LCURSES__HELPERS_C 1

#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>		/* for _POSIX_VERSION */

#include <ncurses.h>
#include <term.h>

#include "../lua.h"
#include "../lualib.h"
#include "../lauxlib.h"

#if LUA_VERSION_NUM < 503
#  define lua_isinteger lua_isnumber
#  if LUA_VERSION_NUM == 501
#    include "compat-5.2.c"
#  endif
#endif

#if LUA_VERSION_NUM == 502 || LUA_VERSION_NUM == 503
#  define lua_objlen lua_rawlen
#  define lua_strlen lua_rawlen
#  define luaL_openlib(L,n,l,nup) luaL_setfuncs((L),(l),(nup))
#  define luaL_register(L,n,l) (luaL_newlib(L,l))
#endif

#ifndef STREQ
#  define STREQ(a, b)     (strcmp (a, b) == 0)
#endif

/* Mark unused parameters required only to match a function type
   specification. */
#ifdef __GNUC__
#  define LCURSES_UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define LCURSES_UNUSED(x) UNUSED_ ## x
#endif

/* LCURSES_STMT_BEG/END are used to create macros that expand to a
   single compound statement in a portable way. */
#if defined __GNUC__ && !defined __STRICT_ANSI__ && !defined __cplusplus
#  define LCURSES_STMT_BEG	(void)(
#  define LCURSES_STMT_END	)
#else
#  if (defined sun || defined __sun__)
#    define LCURSES_STMT_BEG	if (1)
#    define LCURSES_STMT_END	else (void)0
#  else
#    define LCURSES_STMT_BEG	do
#    define LCURSES_STMT_END	while (0)
#  endif
#endif


/* The extra indirection to these macros is required so that if the
   arguments are themselves macros, they will get expanded too.  */
#define LCURSES__SPLICE(_s, _t)	_s##_t
#define LCURSES_SPLICE(_s, _t)	LCURSES__SPLICE(_s, _t)

#define LCURSES__STR(_s)	#_s
#define LCURSES_STR(_s)		LCURSES__STR(_s)

/* The +1 is to step over the leading '_' that is required to prevent
   premature expansion of MENTRY arguments if we didn't add it.  */
#define LCURSES__STR_1(_s)	(#_s + 1)
#define LCURSES_STR_1(_s)	LCURSES__STR_1(_s)

#define LCURSES_CONST(_f)	LCURSES_STMT_BEG {			\
					lua_pushinteger(L, _f);		\
					lua_setfield(L, -2, #_f);	\
				} LCURSES_STMT_END

#define LCURSES_FUNC(_s)	{LCURSES_STR_1(_s), (_s)}

#define pushokresult(b)	pushboolresult((int) (b) == OK)

#ifndef errno
extern int errno;
#endif


/* ========================= *
 * Bad argument diagnostics. *
 * ========================= */


static int
argtypeerror(lua_State *L, int narg, const char *expected)
{
	const char *got = luaL_typename(L, narg);
	return luaL_argerror(L, narg,
		lua_pushfstring(L, "%s expected, got %s", expected, got));
}

static lua_Integer
checkinteger(lua_State *L, int narg, const char *expected)
{
	lua_Integer d = lua_tointeger(L, narg);
	if (d == 0 && !lua_isinteger(L, narg))
		argtypeerror(L, narg, expected);
	return d;
}

static int
checkint(lua_State *L, int narg)
{
	return (int)checkinteger(L, narg, "int");
}


static chtype
checkch(lua_State *L, int narg)
{
	if (lua_isnumber(L, narg))
		return (chtype)checkint(L, narg);
	if (lua_isstring(L, narg))
		return *lua_tostring(L, narg);

	return argtypeerror(L, narg, "int or char");
}


static chtype
optch(lua_State *L, int narg, chtype def)
{
	if (lua_isnoneornil(L, narg))
		return def;
	if (lua_isnumber(L, narg) || lua_isstring(L, narg))
		return checkch(L, narg);
	return argtypeerror(L, narg, "int or char or nil");
}


static int
optint(lua_State *L, int narg, lua_Integer def)
{
	if (lua_isnoneornil(L, narg))
		return (int) def;
	return (int)checkinteger(L, narg, "int or nil");
}

#define pushboolresult(b)	(lua_pushboolean(L, (b)), 1)

#define pushintresult(n)	(lua_pushinteger(L, (n)), 1)

#define pushstringresult(s)	(lua_pushstring(L, (s)), 1)



/* ================== *
 * Utility functions. *
 * ================== */

#define pushintegerfield(k,v) LCURSES_STMT_BEG {			\
	lua_pushinteger(L, (lua_Integer) v); lua_setfield(L, -2, k);	\
} LCURSES_STMT_END

#define pushnumberfield(k,v) LCURSES_STMT_BEG {				\
	lua_pushnumber(L, (lua_Number) v); lua_setfield(L, -2, k);	\
} LCURSES_STMT_END

#define pushstringfield(k,v) LCURSES_STMT_BEG {				\
	if (v) {							\
		lua_pushstring(L, (const char *) v);			\
		lua_setfield(L, -2, k);					\
	}								\
} LCURSES_STMT_END

#define pushliteralfield(k,v) LCURSES_STMT_BEG {			\
	if (v) {							\
		lua_pushliteral(L, v);					\
		lua_setfield(L, -2, k);					\
	}								\
} LCURSES_STMT_END

#define settypemetatable(t) LCURSES_STMT_BEG {				\
	if (luaL_newmetatable(L, t) == 1)				\
		pushliteralfield("_type", t);				\
	lua_setmetatable(L, -2);					\
} LCURSES_STMT_END

#define setintegerfield(_p, _n) pushintegerfield(LCURSES_STR(_n), _p->_n)
#define setnumberfield(_p, _n) pushnumberfield(LCURSES_STR(_n), _p->_n)
#define setstringfield(_p, _n) pushstringfield(LCURSES_STR(_n), _p->_n)

#endif /*LCURSES__HELPERS_C*/
