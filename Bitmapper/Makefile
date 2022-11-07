# This file contains the Makefile for CHESS.
#   Copyright (C) 1986 Free Software Foundation, Inc.
#
# This file is part of CHESS.
#
# CHESS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.  No author or distributor
# accepts responsibility to anyone for the consequences of using it
# or for whether it serves any particular purpose or works at all,
# unless he says so in writing.  Refer to the CHESS General Public
# License for full details.
#
# Everyone is granted permission to copy, modify and redistribute
# CHESS, but only under the conditions described in the
# CHESS General Public License.   A copy of this license is
# supposed to have been given to you along with CHESS so you
# can know your rights and responsibilities.  It should be in a
# file named COPYING.  Among other things, the copyright notice
# and this notice must be preserved on all copies.  */
#
PROGRAM		=	bitmapper

CFLAGS		=	-O -ffpa

LIBS		=

OBJECTS 	=	$(PROGRAM).o\
			bitmacs.o\
			eval.o\
			genmov.o\
			initmap.o\
			legals.o\
			move.o\
			prmap.o\
			savegame.o\
			util.o

SOURCES		=	$(PROGRAM).c\
			bitmacs.c\
			eval.c\
			initmap.c\
			genmov.c\
			legals.c\
			move.c\
			prmap.c\
			savegame.c\
			util.c

HEADERS		= 	algdefs.h\
			bitmacs.h\
			externmap.h\
			crucialdefs.h\
			fixeddefs.h\
			movelist.h

$(PROGRAM):	$(OBJECTS)
	$(CC) -o $@ $(CFLAGS) $(OBJECTS) $(LIBS)

enter:
	co $(HEADERS) $(SOURCES)
	make

leave:
	ci $(HEADERS) $(SOURCES)
	make clean

clean:
	rm -f core *~ *.o $(PROGRAM) picture.out mon.out #* *BAK *.lint *.prof

count:
	wc $(HEADERS) $(SOURCES)

lint:
	lint -h $(HEADERS) $(SOURCES) > $(PROGRAM).lint

print:
	cat $(HEADERS) $(SOURCES) | lpr
