# luasocket src/makefile
#
# Definitions in this section can be overriden on the command line or in the
# environment.
#
# These are equivalent:
#
#    export PLAT=linux DEBUG=DEBUG
#    make
#
# and
#
#   make PLAT=linux DEBUG=DEBUG

# PLAT: bsd linux macosx win32 win64 mingw
# platform to build for
PLAT?=linux

# LUAV: 5.1 5.2
# lua version to build against
LUAV?=5.1

# MYCFLAGS: to be set by user if needed
MYCFLAGS?=

# MYLDFLAGS: to be set by user if needed
MYLDFLAGS?=

# DEBUG: NODEBUG DEBUG
# debug mode causes luasocket to collect and return timing information useful
# for testing and debugging luasocket itself
DEBUG?=NODEBUG

# DESTDIR: (no default)
# used by package managers to install into a temporary destination
DESTDIR?=

#------
# Definitions below can be overridden on the make command line, but
# shouldn't have to be.


print:
	@echo PLAT=$(PLAT)
	@echo DEBUG=$(DEBUG)
	@echo CFLAGS=$(CFLAGS)
	@echo LDFLAGS=$(LDFLAGS)

#------
# Supported platforms
#
PLATS= macosx linux win32 win64 mingw solaris

#------
# Compiler and linker settings
# for Mac OS X
SO_macosx=so
O_macosx=o
CC_macosx=gcc
DEF_macosx= -DLUASOCKET_$(DEBUG) -DUNIX_HAS_SUN_LEN
CFLAGS_macosx=-g -O2 -Wall -fno-common $(DEF)
LDFLAGS_macosx= -bundle -undefined dynamic_lookup -o 
LD_macosx=gcc
SOCKET_macosx=usocket.o

#------
# Compiler and linker settings
# for Linux
SO_linux=so
O_linux=o
CC_linux=gcc
DEF_linux=-DLUASOCKET_$(DEBUG)
CFLAGS_linux= -g -O2 -Wall -Wpedantic -Wshadow -Wextra $(DEF)
LDFLAGS_linux= -o
LD_linux=ld
SOCKET_linux=usocket.o

#------
# Compiler and linker settings
# for BSD
SO_bsd=so
O_bsd=o
CC_bsd=gcc
DEF_bsd=-DLUASOCKET_$(DEBUG) -DUNIX_HAS_SUN_LEN
CFLAGS_bsd=-g -O2 -Wall -Wshadow -Wextra -Wimplicit $(DEF)
LDFLAGS_bsd=-O -shared -fpic -o
LD_bsd=gcc
SOCKET_bsd=usocket.o

#------
# Compiler and linker settings
# for Solaris
SO_solaris=so
O_solaris=o
CC_solaris=gcc
DEF_solaris=-DLUASOCKET_$(DEBUG)
CFLAGS_solaris=-g -O2 -Wall -Wshadow -Wextra -Wimplicit $(DEF)
LDFLAGS_solaris=-lnsl -lsocket -lresolv -O -shared -fpic -o 
LD_solaris=gcc
SOCKET_solaris=usocket.o

#------
# Compiler and linker settings
# for MingW
SO_mingw=dll
O_mingw=o
CC_mingw=gcc
DEF_mingw= -DLUASOCKET_$(DEBUG) -DWINVER=0x0501
CFLAGS_mingw= -O2 -Wall -fno-common $(DEF)
LDFLAGS_mingw= -shared -Wl,-s -lws2_32 -o 
LD_mingw=gcc
SOCKET_mingw=wsocket.o


#------
# Compiler and linker settings
# for Win32
SO_win32=dll
O_win32=obj
CC_win32=cl
DEF_win32= //D "WIN32" //D "NDEBUG" //D "_WINDOWS" //D "_USRDLL" \
     //D "_CRT_SECURE_NO_WARNINGS" \
     //D "_WINDLL"  \
     //D "LUASOCKET_$(DEBUG)"
CFLAGS_win32=$(DEF) //O2 //Ot //MD //W3 //nologo
LDFLAGS_win32= //nologo //link //NOLOGO //DLL //INCREMENTAL:NO \
    //MANIFEST //MANIFESTFILE:"intermediate.manifest" \
    /MANIFESTUAC:"level='asInvoker' uiAccess='false'" \
    //SUBSYSTEM:WINDOWS //OPT:REF //OPT:ICF //DYNAMICBASE:NO \
    //MACHINE:X86 \
    ws2_32.lib //OUT:

LD_win32=cl
SOCKET_win32=wsocket.obj

#------
# Compiler and linker settings
# for Win64
SO_win64=dll
O_win64=obj
CC_win64=cl
DEF_win64= //D "WIN32" //D "NDEBUG" //D "_WINDOWS" //D "_USRDLL" \
     //D "_CRT_SECURE_NO_WARNINGS" \
     //D "_WINDLL"  \
     //D "LUASOCKET_$(DEBUG)"
CFLAGS_win64=$(DEF) //O2 //Ot //MD //W3 //nologo
LDFLAGS_win64= //nologo //link //NOLOGO //DLL //INCREMENTAL:NO \
    //MANIFEST //MANIFESTFILE:"intermediate.manifest" \
    /MANIFESTUAC:"level='asInvoker' uiAccess='false'" \
    //SUBSYSTEM:WINDOWS //OPT:REF //OPT:ICF //DYNAMICBASE:NO \
    ws2_32.lib //OUT:

LD_win64=cl
SOCKET_win64=wsocket.obj

.SUFFIXES: .obj

.c.obj:
	$(CC) $(CFLAGS) //Fo"$@" //c $<

#------
# Output file names
#
SO=$(SO_$(PLAT))
O=$(O_$(PLAT))
A=a
SOCKET_A=socket.$(A)
MIME_A=mime.$(A)
UNIX_A=unix.$(A)
SERIAL_A=serial.$(A)
SOCKET=$(SOCKET_$(PLAT))

#------
# Settings selected for platform
#
CC=$(CC_$(PLAT))
DEF=$(DEF_$(PLAT))
AR= ar rc
RANLIB= ranlib
CFLAGS=$(CFLAGS_$(PLAT)) $(MYCFLAGS)
LDFLAGS=$(LDFLAGS_$(PLAT)) $(MYLDFLAGS)
LD=$(LD_$(PLAT))

#------
# Modules belonging to socket-core
#
SOCKET_OBJS= \
	luasocket.$(O) \
	timeout.$(O) \
	buffer.$(O) \
	io.$(O) \
	auxiliar.$(O) \
	compat.$(O) \
	options.$(O) \
	inet.$(O) \
	$(SOCKET) \
	except.$(O) \
	select.$(O) \
	tcp.$(O) \
	udp.$(O)

#------
# Modules belonging mime-core
#
MIME_OBJS= \
	mime.$(O) \
	compat.$(O)

#------
# Modules belonging unix (local domain sockets)
#
UNIX_OBJS=\
	buffer.$(O) \
	auxiliar.$(O) \
	options.$(O) \
	timeout.$(O) \
	io.$(O) \
	usocket.$(O) \
	unixstream.$(O) \
	unixdgram.$(O) \
	compat.$(O) \
	unix.$(O)

#------
# Modules belonging to serial (device streams)
#
SERIAL_OBJS=\
	buffer.$(O) \
	compat.$(O) \
	auxiliar.$(O) \
	options.$(O) \
	timeout.$(O) \
	io.$(O) \
	usocket.$(O) \
	serial.$(O)

#------
# Files to install
#
TO_SOCKET_LDIR= \
	http.lua \
	url.lua \
	tp.lua \
	ftp.lua \
	headers.lua \
	smtp.lua

TO_TOP_LDIR= \
	ltn12.lua \
	socket.lua \
	mime.lua

#------
# Targets
#
default: $(PLAT)


bsd:
	$(MAKE) all-unix PLAT=bsd

macosx:
	$(MAKE) all-unix PLAT=macosx

win32:
	$(MAKE) all PLAT=win32

win64:
	$(MAKE) all PLAT=win64

linux:
	$(MAKE) all-unix PLAT=linux

mingw:
	$(MAKE) all PLAT=mingw

solaris:
	$(MAKE) all-unix PLAT=solaris

none:
	@echo "Please run"
	@echo "   make PLATFORM"
	@echo "where PLATFORM is one of these:"
	@echo "   $(PLATS)"

all: $(SOCKET_A) $(MIME_A)

$(SOCKET_A): $(SOCKET_OBJS)
	$(AR) $@ $(SOCKET_OBJS)
	$(RANLIB) $@

$(MIME_A): $(MIME_OBJS)
	$(AR) $@ $(MIME_OBJS)
	$(RANLIB) $@

all-unix: all $(UNIX_SO) $(SERIAL_SO)

$(UNIX_A): $(UNIX_OBJS)
	$(AR) $@ $(UNIX_OBJS)
	$(RANLIB) $@

$(SERIAL_A): $(SERIAL_OBJS)
	$(AR) $@ $(SERIAL_OBJS)
	$(RANLIB) $@

clean:
	rm -f $(SOCKET_A) $(SOCKET_OBJS) $(SERIAL_OBJS)
	rm -f $(MIME_A) $(UNIX_A) $(SERIAL_A) $(MIME_OBJS) $(UNIX_OBJS)

.PHONY: all $(PLATS) default clean echo none

#------
# List of dependencies
#
compat.$(O): compat.c compat.h
auxiliar.$(O): auxiliar.c auxiliar.h
buffer.$(O): buffer.c buffer.h io.h timeout.h
except.$(O): except.c except.h
inet.$(O): inet.c inet.h socket.h io.h timeout.h usocket.h
io.$(O): io.c io.h timeout.h
luasocket.$(O): luasocket.c luasocket.h auxiliar.h except.h \
	timeout.h buffer.h io.h inet.h socket.h usocket.h tcp.h \
	udp.h select.h
mime.$(O): mime.c mime.h
options.$(O): options.c auxiliar.h options.h socket.h io.h \
	timeout.h usocket.h inet.h
select.$(O): select.c socket.h io.h timeout.h usocket.h select.h
serial.$(O): serial.c auxiliar.h socket.h io.h timeout.h usocket.h \
  options.h unix.h buffer.h
tcp.$(O): tcp.c auxiliar.h socket.h io.h timeout.h usocket.h \
	inet.h options.h tcp.h buffer.h
timeout.$(O): timeout.c auxiliar.h timeout.h
udp.$(O): udp.c auxiliar.h socket.h io.h timeout.h usocket.h \
	inet.h options.h udp.h
unix.$(O): unix.c auxiliar.h socket.h io.h timeout.h usocket.h \
	options.h unix.h buffer.h
usocket.$(O): usocket.c socket.h io.h timeout.h usocket.h
wsocket.$(O): wsocket.c socket.h io.h timeout.h usocket.h
