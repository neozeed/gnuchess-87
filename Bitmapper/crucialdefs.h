/* This file contains user-settable definitions for CHESS.
   Copyright (C) 1986 Free Software Foundation, Inc.

This file is part of CHESS.

CHESS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the CHESS General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
CHESS, but only under the conditions described in the
CHESS General Public License.   A copy of this license is
supposed to have been given to you along with CHESS so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */

/*
 * Crucialdefs.h -- crucial definitions that are settable
 */

#define BPERW   32		/* bits per machine (unsigned) word */
#define WPERM   (64 / BPERW)	/* words per bitmap to make 64 bits */
#define MAXMOVES 100		/* maximum size of a move list  */
#define LOBD   0		/* array index of low board in bitmap  */
#define HIBD    1		/* array index of high board in bitmap */
#define MAP unsigned int	/* A fraction of a bitmap looks like this */
#define MAXLEGALS 150		/* Maximum legal move list size */
#define MAXGAME 200		/* Maximum length of game */
