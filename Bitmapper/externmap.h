/* This file contains the external bitmap definitions for CHESS.
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
 * extern.h -- External variable definitions
 */

extern unsigned int pieces[NCOLORS][NPIECES][WPERM];
extern unsigned int side[NCOLORS][WPERM];
extern unsigned int empty[WPERM];
extern unsigned int sanctuary[WPERM];
extern unsigned int edge[WPERM];
extern unsigned int rows[MAXROWS][WPERM];
extern unsigned int cols[MAXCOLS][WPERM];
extern unsigned int diagleft[MAXDIAG][WPERM];
extern unsigned int diagright[MAXDIAG][WPERM];
extern unsigned int knightmvmaps[(BPERW*WPERM)][WPERM];
extern unsigned int kingmvmaps[(BPERW*WPERM)][WPERM];

/*
 * Attack maps: fr is indexed by square doing the attacking.
 * E.g. d1 would contain bitmap of squares attacked by d1
 *      in initial position. 
 * to is indexed by the square beinng attacked.
 * E.g. d1 would contain bitmap of squares attacking d1
 *      in initial position.
 * Pawns are handled differently. Pawn attacks and captures
 * are contained in the regular attack tables. Pawn advances
 * are contained in the additional tables wp and bp;
 */
extern unsigned int attackfr[NCOLORS][BPERW*WPERM][WPERM];
extern unsigned int attackto[NCOLORS][BPERW*WPERM][WPERM];
extern unsigned int pmovefr[NCOLORS][BPERW*WPERM][WPERM];
extern unsigned int pmoveto[NCOLORS][BPERW*WPERM][WPERM];
