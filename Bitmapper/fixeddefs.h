/* This file contains fixed definitions for CHESS.
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
 * Fixeddefs.h -- fixed definitions, not really changeable.
 */

#define FALSE 0
#define TRUE 1

#define WHITE   0
#define BLACK   1

#define NPIECES 6
#define NCOLORS 2

#define PAWN    0
#define KNIGHT  1
#define BISHOP  2
#define ROOK    3
#define QUEEN   4
#define KING    5

#define MAXROWS 8
#define MAXCOLS 8
#define MAXDIAG 15

#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))
#define ABS(A) ((A) < 0 ? -(A) : (A))
