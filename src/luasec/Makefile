CMOD=ssl.a
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

linux_CFLAGS=-g -O2 $(WARN) $(INCDIR) $(DEFS)

macosx_ENV=env MACOSX_DEPLOYMENT_TARGET='$(MACVER)'
macosx_CFLAGS=-O2 -fno-common $(WARN) $(INCDIR) $(DEFS)
macosx_LDFLAGS=-bundle -undefined dynamic_lookup $(LIBDIR)

INSTALL  = install
CC      ?= gcc
CCLD      ?= $(MYENV) $(CC)
CFLAGS = $(linux_CFLAGS)
CFLAGS  += $(MYCFLAGS)
LDFLAGS += $(MYLDFLAGS)
AR= ar rc
RANLIB= ranlib

.PHONY: all clean install none linux bsd macosx

all:

install: $(CMOD) $(LMOD)
	$(INSTALL) -d $(DESTDIR)$(LUAPATH)/ssl $(DESTDIR)$(LUACPATH)
	$(INSTALL) $(CMOD) $(DESTDIR)$(LUACPATH)
	$(INSTALL) -m644 $(LMOD) $(DESTDIR)$(LUAPATH)
	$(INSTALL) -m644 https.lua $(DESTDIR)$(LUAPATH)/ssl

linux: $(OBJS)
	$(AR) $(CMOD) $(OBJS)
	$(RANLIB) $(CMOD)

bsd:
	@$(MAKE) $(CMOD) MYCFLAGS="$(BSD_CFLAGS)" MYLDFLAGS="$(BSD_LDFLAGS)" EXTRA="$(EXTRA)"

macosx:
	@$(MAKE) $(CMOD) MYCFLAGS="$(macosx_CFLAGS)" MYLDFLAGS="$(macosx_LDFLAGS)" MYENV="$(macosx_ENV)" EXTRA="$(EXTRA)"

clean:
	rm -f $(OBJS) $(CMOD)

options.o: options.h options.c
ec.o: ec.c ec.h
x509.o: x509.c x509.h compat.h
context.o: context.c context.h ec.h compat.h options.h
ssl.o: ssl.c ssl.h context.h x509.h compat.h
config.o: config.c ec.h options.h compat.h
