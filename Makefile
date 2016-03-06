CC=gcc
CFLAGS = -m64 
LFLAGS=
DEBUG=1
PREFIX=/usr/local
UNAME=$(shell uname -s)

# Target config
MAJOR=1
MINOR=0
RELEASE=1
OBJS=obj/scut.o
LINK_NAME=libscut.so
SONAME=$(LINK_NAME).$(MAJOR)
REAL_NAME=$(LINK_NAME).$(MAJOR).$(MINOR).$(RELEASE)
LIB=obj/$(REAL_NAME)

# Flags/defines for various OS
ifeq ($(UNAME), SunOS)
CC=c99
CFLAGS += -D_POSIX_C_SOURCE=200112L
endif

# Flags for various compilers
ifeq ($(CC), gcc)
CFLAGS += -W -Wall -pedantic -std=c99 -fpic
LFLAGS += -shared -Wl,-soname,$(SONAME)
else ifeq ($(CC), c99)
CFLAGS += -Kpic
LFLAGS += -G -Wl,-soname,$(SONAME)
endif

ifeq ($(DEBUG), 1)
CFLAGS += -g
endif

.PHONY: all test lib install uninstall clean distclean

all: lib

obj:
	mkdir obj

bin:
	mkdir bin

test: bin bin/test_scut
	./bin/test_scut

bin/test_scut: test_scut.c scut.c 
	$(CC) $(CFLAGS) -o $@ $^

bin/example: bin lib example.c
	cd obj && test -L $(SONAME) || ln -s $(REAL_NAME) $(SONAME)
	cd obj && test -L $(LINK_NAME) || ln -s $(SONAME) $(LINK_NAME)
	$(CC) $(CFLAGS) -o $@ example.c -L./obj -lscut

lib: obj $(LIB)

$(LIB): $(OBJS)
	$(CC) $(CFLAGS) $(LFLAGS) -lc -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: lib install_$(UNAME)
	ln -s $(PREFIX)/lib/$(REAL_NAME) $(PREFIX)/lib/$(SONAME)
	ln -s $(PREFIX)/lib/$(SONAME) $(PREFIX)/lib/$(LINK_NAME)

install_SunOS:
	install -m 755 -c $(PREFIX)/lib $(LIB)
	install -m 644 -c $(PREFIX)/include scut.h

install_FreeBSD:
	install -m 755 $(LIB) $(PREFIX)/lib
	install -m 644 scut.h $(PREFIX)/include

uninstall:
	rm $(PREFIX)/include/scut.h
	rm $(PREFIX)/lib/$(LINK_NAME)
	rm $(PREFIX)/lib/$(SONAME)
	rm $(PREFIX)/lib/$(REAL_NAME)

clean:
	rm -f $(OBJS) $(LIB) bin/test_scut libscut.so.1 libscut.so bin/example

distclean:
	rm -rf obj bin
