CMOD=ssl.so
LMOD=ssl.lua

OBJS= \
 options.o \
 x509.o    \
 context.o \
 ssl.o     \
 config.o  \
 ec.o

WARN=-Wall -pedantic

BSD_CFLAGS=-O2 -fPIC $(WARN) $(INCDIR) $(DEFS)
BSD_LDFLAGS=-O -fPIC -shared $(LIBDIR)

LNX_CFLAGS=-O2 $(WARN) $(INCDIR) $(DEFS)
LNX_LDFLAGS=-O

MAC_ENV=env MACOSX_DEPLOYMENT_TARGET='$(MACVER)'
MAC_CFLAGS=-O2 -fno-common $(WARN) $(INCDIR) $(DEFS)
MAC_LDFLAGS=-bundle -undefined dynamic_lookup $(LIBDIR)

INSTALL  = install
CC      ?= cc
CCLD      ?= $(MYENV) $(CC)
CFLAGS  += $(MYCFLAGS)
LDFLAGS += $(MYLDFLAGS)
AR= ar rc
RANLIB= ranlib

.PHONY: all clean install none linux bsd macosx luasocket

all: luasocket

install: $(CMOD) $(LMOD)
	$(INSTALL) -d $(DESTDIR)$(LUAPATH)/ssl $(DESTDIR)$(LUACPATH)
	$(INSTALL) $(CMOD) $(DESTDIR)$(LUACPATH)
	$(INSTALL) -m644 $(LMOD) $(DESTDIR)$(LUAPATH)
	$(INSTALL) -m644 https.lua $(DESTDIR)$(LUAPATH)/ssl

linux: $(OBJS)
	$(AR) ssl.a $(OBJS)
	$(RANLIB) ssl.a

bsd:
	@$(MAKE) $(CMOD) MYCFLAGS="$(BSD_CFLAGS)" MYLDFLAGS="$(BSD_LDFLAGS)" EXTRA="$(EXTRA)"

macosx:
	@$(MAKE) $(CMOD) MYCFLAGS="$(MAC_CFLAGS)" MYLDFLAGS="$(MAC_LDFLAGS)" MYENV="$(MAC_ENV)" EXTRA="$(EXTRA)"

luasocket:
	@cd luasocket && $(MAKE)

$(CMOD): $(EXTRA) $(OBJS)
	$(CCLD) $(LDFLAGS) -o $@ $(OBJS) luasocket/libluasocket.a -lssl -lcrypto

clean:
	cd luasocket && $(MAKE) clean
	rm -f $(OBJS) $(CMOD)

options.o: options.h options.c
ec.o: ec.c ec.h
x509.o: x509.c x509.h compat.h
context.o: context.c context.h ec.h compat.h options.h
ssl.o: ssl.c ssl.h context.h x509.h compat.h
config.o: config.c ec.h options.h compat.h
