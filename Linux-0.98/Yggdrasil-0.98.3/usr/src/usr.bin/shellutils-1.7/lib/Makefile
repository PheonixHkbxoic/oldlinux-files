# Generated automatically from Makefile.in by configure.
# Makefile for library files used by GNU shell utilities.
# Do not use this makefile directly, but only from `../Makefile'.
# Copyright (C) 1991 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

SHELL = /bin/sh

srcdir = .
VPATH = .

SOURCES = basename.c error.c stripslash.c xmalloc.c xstrdup.c xwrite.c \
putenv.c getopt.c getopt1.c getugroups.c regex.c getdate.y \
posixtm.y strftime.c gethostname.c getusershell.c mktime.c stime.c \
strcspn.c strstr.c strtol.c strtoul.c strtod.c alloca.c

OBJECTS = basename.o error.o stripslash.o xmalloc.o xstrdup.o xwrite.o \
putenv.o getopt.o getopt1.o getugroups.o regex.o getdate.o \
posixtm.o strftime.o  getusershell.o 

DISTFILES = Makefile.in getopt.h regex.h $(SOURCES)

all: libshu.a
.PHONY: all

.c.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(DEFS) -I$(srcdir) $<

install: all
.PHONY: install

TAGS: $(SOURCES)
	etags $(SOURCES)

clean:
	rm -f *.a *.o
.PHONY: clean

mostlyclean: clean
.PHONY: mostlyclean

distclean: clean
	rm -f Makefile *.tab.c getdate.c *posixtm.c
.PHONY: distclean

realclean: distclean
	rm -f TAGS
.PHONY: realclean

dist:
	ln $(DISTFILES) ../`cat ../.fname`/lib
.PHONY: dist

libshu.a: $(OBJECTS)
	rm -f $@
	$(AR) cr $@ $(OBJECTS)
	-$(RANLIB) $@

getdate.c: getdate.y
	@echo expect 9 shift/reduce conflicts
	-bison -o getdate.c $(srcdir)/getdate.y || yacc $(srcdir)/getdate.y
	test ! -f y.tab.c || mv y.tab.c getdate.c

# Make the rename atomic, in case sed is interrupted and later rerun.
posixtm.c: posixtm.y
	-bison -o posixtm.tab.c $(srcdir)/posixtm.y || yacc $(srcdir)/posixtm.y
	test ! -f y.tab.c || mv y.tab.c posixtm.tab.c
	sed -e 's/yy/zz/g' posixtm.tab.c > tposixtm.c
	mv tposixtm.c posixtm.c
	rm -f posixtm.tab.c

getopt1.o: getopt.h
regex.o: regex.h

# Prevent GNU make v3 from overflowing arg limit on SysV.
.NOEXPORT:
