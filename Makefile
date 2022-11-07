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
# and this notice must be preserved on all copies.
#
#
# Flags for compiling GNU Chess
#    QUIES     - use quiescence search (MAXQUIES must be set)
#    MAXQUIES  - maximum depth to which quiescence-search reaches
#    DEPTH     - depth to which full-width search reaches
#    HASH      - include code to do hashing in tree (currently disabled)
#    PARALLEL  - include code for parallelism
#    RANDOM    - randomize opening moves, otherwise first move in book
#    XWINDOW   - allow use of X-window front-end
#    SUNWINDOW - allow use of SUN-window front-end
# Default setting is full-width search to 3 ply, quiescence to
# 6 ply. This setting works well on low-end VAX's, SUN's, etc.
# For faster machines or if parallelism is being used, DEPTH and
# MAXQUIES can be incremented slightly.  (Although the parallelism
# isn't thoroughly debugged and long move times may result
# due to waiting for the last processor to finish. This will shortly be
# fixed.) In general, for every 500% increase in processor power,
# you can increase these values by 1. For example, when we
# go from a one-SUN configuration to 7 SUN's, we increase DEPTH
# to 4 and MAXQUIES to 7.
CFLAGS = -DRANDOM -DQUIES -DMAXQUIES=6 -DDEPTH=3 -DHASH -O

# Source and object files in the distribution
SRCS = alphabeta.c book.c gen.c hash.c main.c moves.c parallel.c pps.c util.c dbm.c
OBJS = alphabeta.o book.o gen.o hash.o main.o moves.o parallel.o pps.o util.o dbm.o

# General non-source files, including header files
NSRCS = CHANGES COPYING MAN-PAGE README SAMPLE-GAME TODO Makefile gnuchess.h\
  Bitmapper/Makefile bookin bookin.bdg bookin.tal convert.el

# Source but not part of the standard compilation, usually being
# experimental additions that are either verified to be less than
# optimal or have some theoretical interest.
SRCS2 = tbl_gen.c tbl_genpc.c tbl_genpwn.c Bitmapper/*.c Bitmapper/*.h Xchess/*

# The following files are what make up the general distribution
DIST = $(NSRCS) $(SRCS) $(SRCS2)

gnuchess: $(OBJS)
	$(CC) -o gnuchess $(CFLAGS) $(OBJS)

alphabeta.o: alphabeta.c gnuchess.h
	$(CC) -c $(CFLAGS) alphabeta.c 

book.o: book.c gnuchess.h
	$(CC) -c $(CFLAGS) book.c

gen.o:	gen.c gnuchess.h
	$(CC) -c $(CFLAGS) gen.c 
	
hash.o: hash.c gnuchess.h
	$(CC) -c $(CFLAGS) hash.c 

main.o: main.c gnuchess.h
	$(CC) -c $(CFLAGS) main.c 

moves.o:moves.c gnuchess.h
	$(CC) -c $(CFLAGS) moves.c 

parallel.o: parallel.c gnuchess.h
	$(CC) -c $(CFLAGS) parallel.c

pps.o:  pps.c gnuchess.h
	$(CC) -c $(CFLAGS) pps.c 

util.o: util.c gnuchess.h
	$(CC) -c $(CFLAGS) util.c 

dist:
	tar cvf - $(DIST) > /tmp/gnuchess.tar

tape:
	tar cv $(DIST)

backup:
	rm -fr Backup
	mkdir Backup
	cp $(SRCS) Backup
