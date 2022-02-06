# makefile for building Lua
# see ../INSTALL for installation instructions
# see ../Makefile and luaconf.h for further customization

# == CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT =======================

# Your platform. See PLATS for possible values.
PLAT= none

CC= gcc
CFLAGS= -g -O2 -std=c99 -Wall -Wshadow $(MYCFLAGS) -D_DEFAULT_SOURCE
AR= ar rc
RANLIB= ranlib
RM= rm -f
LIBS= -lm $(MYLIBS) -lssl -lcrypto

MYCFLAGS=
MYLDFLAGS=
MYLIBS=

# == END OF USER SETTINGS. NO NEED TO CHANGE ANYTHING BELOW THIS LINE =========

PLATS= freebsd linux macosx netbsd openbsd

LUA_A=	liblua.a
CORE_O=	lapi.o lcode.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o lmem.o \
	lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o ltm.o  \
	lundump.o lvm.o lzio.o \
	realpath.o lfs.o kilo.o tlv.o teliva.o
LIB_O=	lauxlib.o lbaselib.o ldblib.o liolib.o lmathlib.o loslib.o \
	ltablib.o lstrlib.o loadlib.o linit.o

LUA_T=	teliva
LUA_O=	lua.o

ALL_O= $(CORE_O) $(LIB_O) $(LUA_O)
ALL_T= $(LUA_A) $(LUA_T)
ALL_A= $(LUA_A)

default: $(PLAT)

all:	$(ALL_T)

o:	$(ALL_O)

a:	$(ALL_A)

$(LUA_A): $(CORE_O) $(LIB_O)
	$(AR) $@ $(CORE_O) $(LIB_O)
	$(RANLIB) $@

$(LUA_T): $(LUA_O) $(LUA_A) lcurses/curses.o luasocket/socket.a luasocket/mime.a luasec/ssl.a
	$(CC) -o $@ $(MYLDFLAGS) $(LUA_O) $(LUA_A) lcurses/curses.o luasocket/socket.a luasocket/mime.a luasec/ssl.a $(LIBS)

clean:
	$(RM) lcurses/curses.o
	$(MAKE) -C luasocket clean
	$(MAKE) -C luasec clean
	$(RM) $(ALL_T) $(ALL_O)

depend:
	@$(CC) $(CFLAGS) -MM l*.c print.c

echo:
	@echo "PLAT = $(PLAT)"
	@echo "CC = $(CC)"
	@echo "CFLAGS = $(CFLAGS)"
	@echo "AR = $(AR)"
	@echo "RANLIB = $(RANLIB)"
	@echo "RM = $(RM)"
	@echo "MYCFLAGS = $(MYCFLAGS)"
	@echo "MYLDFLAGS = $(MYLDFLAGS)"
	@echo "MYLIBS = $(MYLIBS)"

# convenience targets for popular platforms

none:
	@echo "Please choose a platform:"
	@echo "   $(PLATS)"

freebsd:
	$(MAKE) -C luasocket bsd
	$(MAKE) -C luasec bsd
	$(MAKE) all MYLIBS="-Wl,-E -lncursesw"

linux:
	$(MAKE) -C lcurses CC="$(CC)" CFLAGS="$(CFLAGS)"
	$(MAKE) -C luasocket linux
	$(MAKE) -C luasec linux
	$(MAKE) all MYCFLAGS=-Wpedantic MYLIBS="-Wl,-E -lncursesw"

macosx:
	$(MAKE) -C lcurses CC="$(CC)" CFLAGS="$(CFLAGS)"
	$(MAKE) -C luasocket macosx
	$(MAKE) -C luasec macosx
	$(MAKE) all MYCFLAGS=-Wpedantic MYLIBS="-L/usr/local/opt/openssl@3/lib -lncurses"

openbsd:
	$(MAKE) -C lcurses CC="$(CC)" CFLAGS="$(CFLAGS)"
	$(MAKE) -C luasocket bsd
	$(MAKE) -C luasec bsd
	$(MAKE) all MYLIBS="-Wl,-E -lncurses"

netbsd:
	$(MAKE) -C lcurses CC="$(CC)" CFLAGS="$(CFLAGS)"
	$(MAKE) -C luasocket bsd
	$(MAKE) -C luasec bsd
	$(MAKE) all MYLIBS="-Wl,-E -lcurses"

# list targets that do not create files (but not all makes understand .PHONY)
.PHONY: all $(PLATS) default o a clean depend echo none

# DO NOT DELETE

lapi.o: lapi.c lua.h luaconf.h lapi.h lobject.h llimits.h ldebug.h \
  lstate.h ltm.h lzio.h lmem.h ldo.h lfunc.h lgc.h lstring.h ltable.h \
  lundump.h lvm.h
lauxlib.o: lauxlib.c lua.h luaconf.h lauxlib.h
lbaselib.o: lbaselib.c lua.h luaconf.h lauxlib.h lualib.h
lcode.o: lcode.c lua.h luaconf.h lcode.h llex.h lobject.h llimits.h \
  lzio.h lmem.h lopcodes.h lparser.h ldebug.h lstate.h ltm.h ldo.h lgc.h \
  ltable.h
ldblib.o: ldblib.c lua.h luaconf.h lauxlib.h lualib.h
ldebug.o: ldebug.c lua.h luaconf.h lapi.h lobject.h llimits.h lcode.h \
  llex.h lzio.h lmem.h lopcodes.h lparser.h ldebug.h lstate.h ltm.h ldo.h \
  lfunc.h lstring.h lgc.h ltable.h lvm.h
ldo.o: ldo.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h ltm.h \
  lzio.h lmem.h ldo.h lfunc.h lgc.h lopcodes.h lparser.h lstring.h \
  ltable.h lundump.h lvm.h
ldump.o: ldump.c lua.h luaconf.h lobject.h llimits.h lstate.h ltm.h \
  lzio.h lmem.h lundump.h
lfunc.o: lfunc.c lua.h luaconf.h lfunc.h lobject.h llimits.h lgc.h lmem.h \
  lstate.h ltm.h lzio.h
lgc.o: lgc.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h ltm.h \
  lzio.h lmem.h ldo.h lfunc.h lgc.h lstring.h ltable.h
linit.o: linit.c lua.h luaconf.h lualib.h lauxlib.h
liolib.o: liolib.c lua.h luaconf.h lauxlib.h lualib.h
llex.o: llex.c lua.h luaconf.h ldo.h lobject.h llimits.h lstate.h ltm.h \
  lzio.h lmem.h llex.h lparser.h lstring.h lgc.h ltable.h
lmathlib.o: lmathlib.c lua.h luaconf.h lauxlib.h lualib.h
lmem.o: lmem.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h \
  ltm.h lzio.h lmem.h ldo.h
loadlib.o: loadlib.c lua.h luaconf.h lauxlib.h lualib.h
lobject.o: lobject.c lua.h luaconf.h ldo.h lobject.h llimits.h lstate.h \
  ltm.h lzio.h lmem.h lstring.h lgc.h lvm.h
lopcodes.o: lopcodes.c lopcodes.h llimits.h lua.h luaconf.h
loslib.o: loslib.c lua.h luaconf.h lauxlib.h lualib.h
lparser.o: lparser.c lua.h luaconf.h lcode.h llex.h lobject.h llimits.h \
  lzio.h lmem.h lopcodes.h lparser.h ldebug.h lstate.h ltm.h ldo.h \
  lfunc.h lstring.h lgc.h ltable.h
lstate.o: lstate.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h \
  ltm.h lzio.h lmem.h ldo.h lfunc.h lgc.h llex.h lstring.h ltable.h
lstring.o: lstring.c lua.h luaconf.h lmem.h llimits.h lobject.h lstate.h \
  ltm.h lzio.h lstring.h lgc.h
lstrlib.o: lstrlib.c lua.h luaconf.h lauxlib.h lualib.h
ltable.o: ltable.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h \
  ltm.h lzio.h lmem.h ldo.h lgc.h ltable.h
ltablib.o: ltablib.c lua.h luaconf.h lauxlib.h lualib.h
ltm.o: ltm.c lua.h luaconf.h lobject.h llimits.h lstate.h ltm.h lzio.h \
  lmem.h lstring.h lgc.h ltable.h
lua.o: lua.c lua.h luaconf.h lauxlib.h lualib.h teliva.h
lundump.o: lundump.c lua.h luaconf.h ldebug.h lstate.h lobject.h \
  llimits.h ltm.h lzio.h lmem.h ldo.h lfunc.h lstring.h lgc.h lundump.h
lvm.o: lvm.c lua.h luaconf.h ldebug.h lstate.h lobject.h llimits.h ltm.h \
  lzio.h lmem.h ldo.h lfunc.h lgc.h lopcodes.h lstring.h ltable.h lvm.h
lzio.o: lzio.c lua.h luaconf.h llimits.h lmem.h lstate.h lobject.h ltm.h \
  lzio.h
print.o: print.c ldebug.h lstate.h lua.h luaconf.h lobject.h llimits.h \
  ltm.h lzio.h lmem.h lopcodes.h lundump.h
realpath.o: realpath.c
lfs.o: lfs.c lfs.h lua.h lauxlib.h lualib.h
kilo.o: kilo.c lua.h teliva.h
teliva.o: teliva.c lua.h lauxlib.h lualib.h teliva.h tlv.h

# (end of Makefile)
