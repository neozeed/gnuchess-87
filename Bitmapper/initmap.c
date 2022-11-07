/* This file contains the map-initialization routines for CHESS.
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

#include <stdio.h>
#include "algdefs.h"
#include "crucialdefs.h"
#include "fixeddefs.h"
#include "externmap.h"
#include "bitmacs.h"

extern FILE *hisf;
extern tomove,ngamemvs;

char *sqchar[BPERW*WPERM] = {
  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

MAP sqbits[BPERW*WPERM] = {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8};

init_diag()
{
    diagleft[0][LOBD] = a1;
    diagleft[0][HIBD] = 0;
    diagleft[1][LOBD] = b1|a2;
    diagleft[1][HIBD] = 0;
    diagleft[2][LOBD] = c1|b2|a3;
    diagleft[2][HIBD] = 0;
    diagleft[3][LOBD] = d1|c2|b3|a4;
    diagleft[3][HIBD] = 0;
    diagleft[4][LOBD] = e1|d2|c3|b4;
    diagleft[4][HIBD] = a5;
    diagleft[5][LOBD] = f1|e2|d3|c4;
    diagleft[5][HIBD] = b5|a6;
    diagleft[6][LOBD] = g1|f2|e3|d4;
    diagleft[6][HIBD] = c5|b6|a7;
    diagleft[7][LOBD] = h1|g2|f3|e4;
    diagleft[7][HIBD] = d5|c6|b7|a8;
    diagleft[8][LOBD] = h2|g3|f4;
    diagleft[8][HIBD] = e5|d6|c7|b8;
    diagleft[9][LOBD] = h3|g4;
    diagleft[9][HIBD] = f5|e6|d7|c8;
    diagleft[10][LOBD] = h4;
    diagleft[10][HIBD] = g5|f6|e7|d8;
    diagleft[11][LOBD] = 0;
    diagleft[11][HIBD] = h5|g6|f7|e8;
    diagleft[12][LOBD] = 0;
    diagleft[12][HIBD] = h6|g7|f8;
    diagleft[13][LOBD] = 0;
    diagleft[13][HIBD] = h7|g8;
    diagleft[14][LOBD] = 0;
    diagleft[14][HIBD] = h8;

    diagright[7][LOBD] = a1|b2|c3|d4;
    diagright[7][HIBD] = e5|f6|g7|h8;
    diagright[8][LOBD] = b1|c2|d3|e4;
    diagright[8][HIBD] = f5|g6|h7;
    diagright[9][LOBD] = c1|d2|e3|f4;
    diagright[9][HIBD] = g5|h6;
    diagright[10][LOBD] = d1|e2|f3|g4;
    diagright[10][HIBD] = h5;
    diagright[11][LOBD] = e1|f2|g3|h4;
    diagright[11][HIBD] = 0;
    diagright[12][LOBD] = f1|g2|h3;
    diagright[12][HIBD] = 0;
    diagright[13][LOBD] = g1|h2;
    diagright[13][HIBD] = 0;
    diagright[14][LOBD] = h1;
    diagright[14][HIBD] = 0;
    diagright[6][LOBD] = a2|b3|c4;
    diagright[6][HIBD] = d5|e6|f7|g8;
    diagright[5][LOBD] = a3|b4;
    diagright[5][HIBD] = c5|d6|e7|f8;
    diagright[4][LOBD] = a4;
    diagright[4][HIBD] = b5|c6|d7|e8;
    diagright[3][LOBD] = 0;
    diagright[3][HIBD] = a5|b6|c7|d8;
    diagright[2][LOBD] = 0;
    diagright[2][HIBD] = a6|b7|c8;
    diagright[1][LOBD] = 0;
    diagright[1][HIBD] = a7|b8;
    diagright[0][LOBD] = 0;
    diagright[0][HIBD] = a8;
}

init_rows()
{
    rows[0][0] = a1|b1|c1|d1|e1|f1|g1|h1;
    rows[0][1] = 0;
    rows[1][0] = a2|b2|c2|d2|e2|f2|g2|h2;
    rows[1][1] = 0;
    rows[2][0] = a3|b3|c3|d3|e3|f3|g3|h3;
    rows[2][1] = 0;
    rows[3][0] = a4|b4|c4|d4|e4|f4|g4|h4;
    rows[3][1] = 0;
    rows[4][0] = 0;
    rows[4][1] = a5|b5|c5|d5|e5|f5|g5|h5;
    rows[5][0] = 0;
    rows[5][1] = a6|b6|c6|d6|e6|f6|g6|h6;
    rows[6][0] = 0;
    rows[6][1] = a7|b7|c7|d7|e7|f7|g7|h7;
    rows[7][0] = 0;
    rows[7][1] = a8|b8|c8|d8|e8|f8|g8|h8;
}

init_cols()
{
    cols[0][0] = a1|a2|a3|a4;
    cols[0][1] = a5|a6|a7|a8;
    cols[1][0] = b1|b2|b3|b4;
    cols[1][1] = b5|b6|b7|b8;
    cols[2][0] = c1|c2|c3|c4;
    cols[2][1] = c5|c6|c7|c8;
    cols[3][0] = d1|d2|d3|d4;
    cols[3][1] = d5|d6|d7|d8;
    cols[4][0] = e1|e2|e3|e4;
    cols[4][1] = e5|e6|e7|e8;
    cols[5][0] = f1|f2|f3|f4;
    cols[5][1] = f5|f6|f7|f8;
    cols[6][0] = g1|g2|g3|g4;
    cols[6][1] = g5|g6|g7|g8;
    cols[7][0] = h1|h2|h3|h4;
    cols[7][1] = h5|h6|h7|h8;
}

init_maps()
{
    register sq,pc,col;
    for (pc = 0; pc < NPIECES; pc++)
      for (col = 0; col < NCOLORS; col++)
	BIT_ZERO(pieces[col][pc]);
    for (pc = 0; pc < NPIECES; pc++)
      for (col = 0; col < NCOLORS; col++)
	BIT_ZERO(pieces[col][pc]);
    for (sq = A1; sq <= H8; sq++)
      for (col = 0; col < NCOLORS; col++)
	{
	  BIT_ZERO(attackfr[col][sq]);
	  BIT_ZERO(attackto[col][sq]);
	  BIT_ZERO(pmoveto[col][sq]);
	  BIT_ZERO(pmovefr[col][sq]);
        }
    pieces[WHITE][KING][LOBD] = e1;
    pieces[WHITE][KING][HIBD] = 0;
    pieces[BLACK][KING][HIBD] = e8;
    pieces[BLACK][KING][LOBD] = 0;
    pieces[WHITE][QUEEN][LOBD] = d1;
    pieces[WHITE][QUEEN][HIBD] = 0;
    pieces[BLACK][QUEEN][HIBD] = d8;
    pieces[BLACK][QUEEN][LOBD] = 0;
    pieces[WHITE][ROOK][LOBD] = a1|h1;
    pieces[WHITE][ROOK][HIBD] = 0;
    pieces[BLACK][ROOK][HIBD] = a8|h8;
    pieces[BLACK][ROOK][LOBD] = 0;
    pieces[WHITE][BISHOP][LOBD] = c1|f1;
    pieces[WHITE][BISHOP][HIBD] = 0;
    pieces[BLACK][BISHOP][HIBD] = c8|f8;
    pieces[BLACK][BISHOP][LOBD] = 0;
    pieces[WHITE][KNIGHT][LOBD] = b1|g1;
    pieces[WHITE][KNIGHT][HIBD] = 0;
    pieces[BLACK][KNIGHT][HIBD] = b8|g8;
    pieces[BLACK][KNIGHT][LOBD] = 0;
    pieces[WHITE][PAWN][LOBD] = a2|b2|c2|d2|e2|f2|g2|h2;
    pieces[WHITE][PAWN][HIBD] = 0;
    pieces[BLACK][PAWN][HIBD] = a7|b7|c7|d7|e7|f7|g7|h7;
    pieces[BLACK][PAWN][LOBD] = 0;
    side[WHITE][LOBD] = a1|b1|c1|d1|e1|f1|g1|h1|a2|b2|c2|d2|e2|f2|g2|h2;
    side[WHITE][HIBD] = 0;
    side[BLACK][HIBD] = a7|b7|c7|d7|e7|f7|g7|h7|a8|b8|c8|d8|e8|f8|g8|h8;
    side[BLACK][LOBD] = 0;
    empty[LOBD] = a3|b3|c3|d3|e3|f3|g3|h3|a4|b4|c4|d4|e4|f4|g4|h4;
    empty[HIBD] = a5|b5|c5|d5|e5|f5|g5|h5|a6|b6|c6|d6|e6|f6|g6|h6;
    edge[LOBD] = a1|b1|c1|d1|e1|f1|g1|h1|a2|h2|a3|h3|a4|h4;
    edge[HIBD] = a5|h5|a6|h6|a7|h7|a8|b8|c8|d8|e8|f8|g8|h8;
    sanctuary[LOBD] = a1|a2|b1|b2|h1|h2|g1|g2;
    sanctuary[HIBD] = a7|a8|b7|b8|g7|g8|h7|h8;
}

#define Put_Kn_map(sq,low,hi) knightmvmaps[sq][LOBD] = low;\
                              knightmvmaps[sq][HIBD] = hi;

init_knight()
{
    Put_Kn_map(A1,b3|c2,0);
    Put_Kn_map(B1,a3|c3|d2,0);
    Put_Kn_map(C1,a2|b3|d3|e2,0);
    Put_Kn_map(D1,b2|c3|e3|f2,0);
    Put_Kn_map(E1,c2|d3|f3|g2,0);
    Put_Kn_map(F1,d2|e3|g3|h2,0);
    Put_Kn_map(G1,e2|f3|h3,0);
    Put_Kn_map(H1,f2|g3,0);
    Put_Kn_map(A2,b4|c3|c1,0);
    Put_Kn_map(B2,a4|c4|d3|d1,0);
    Put_Kn_map(C2,a1|a3|b4|d4|e3|e1,0);
    Put_Kn_map(D2,b1|b3|c4|e4|f3|f1,0);
    Put_Kn_map(E2,c1|c3|d4|f4|g3|g1,0);
    Put_Kn_map(F2,d1|d3|e4|g4|h3|h1,0);
    Put_Kn_map(G2,e1|e3|f4|h4,0);
    Put_Kn_map(H2,f1|f3|g4,0);
    Put_Kn_map(A3,c4|c2|b1,b5);
    Put_Kn_map(B3,d4|d2|a1|c1,a5|c5);
    Put_Kn_map(C3,a4|a2|b1|d1|e2|e4,b5|d5);
    Put_Kn_map(D3,b4|f4|b2|f2|c1|e1,c5|e5);
    Put_Kn_map(E3,c2|c4|g4|g2|d1|f1,d5|f5);
    Put_Kn_map(F3,d2|d4|h2|h4|e1|g1,e5|g5);
    Put_Kn_map(G3,e4|e2|f1|h1,f5|h5);
    Put_Kn_map(H3,f4|f2|g1,g5);
    Put_Kn_map(A4,b2|c3,b6|c5);
    Put_Kn_map(B4,a2|c2|d3,d5|c6|a6);
    Put_Kn_map(C4,a3|b2|d2|e3,a5|b6|d6|e5);
    Put_Kn_map(D4,b3|c2|e2|f3,b5|c6|e6|f5);
    Put_Kn_map(E4,c3|d2|f2|g3,c5|d6|f6|g5);
    Put_Kn_map(F4,d3|e2|g2|h3,d5|e6|g6|h5);
    Put_Kn_map(G4,e3|f2|h2,e5|f6|h6);
    Put_Kn_map(H4,f3|g2,g6|f5);
    Put_Kn_map(A5,b3|c4,b7|c6);
    Put_Kn_map(B5,a3|c3|d4,a7|c7|d6);
    Put_Kn_map(C5,a4|b3|d3|e4,a6|b7|d7|e6);
    Put_Kn_map(D5,b4|c3|e3|f4,b6|c7|e7|f6);
    Put_Kn_map(E5,c4|d3|f3|g4,c6|d7|f7|g6);
    Put_Kn_map(F5,d4|e3|g3|h4,d6|e7|g7|h6);
    Put_Kn_map(G5,e4|f3|h3,e6|f7|h7);
    Put_Kn_map(H5,f4|g3,f6|g7);
    Put_Kn_map(A6,b4,c5|c7|b8);
    Put_Kn_map(B6,a4|c4,d5|d7|a8|c8);
    Put_Kn_map(C6,b4|d4,a5|e5|a7|e7|b8|d8);
    Put_Kn_map(D6,c4|d4,b5|f5|b7|f7|c8|e8);
    Put_Kn_map(E6,d4|f4,c5|c7|g5|g7|d8|f8);
    Put_Kn_map(F6,e4|g4,d5|d7|h5|h7|e8|g8);
    Put_Kn_map(G6,f4|h4,e5|e7|f8|h8);
    Put_Kn_map(H6,g4,f5|f7|g8);
    Put_Kn_map(A7,0,c8|c6|b5);
    Put_Kn_map(B7,0,a5|c5|d6|d8);
    Put_Kn_map(C7,0,b5|d5|e6|e8|a6|a8);
    Put_Kn_map(D7,0,c5|e5|b6|f6|b8|f8);
    Put_Kn_map(E7,0,c6|c8|g6|g8|d5|f5);
    Put_Kn_map(F7,0,e5|g5|d6|d8|h6|h8);
    Put_Kn_map(G7,0,f5|h5|e6|e8);
    Put_Kn_map(H7,0,g5|f6|f8);
    Put_Kn_map(A8,0,b6|c7);
    Put_Kn_map(B8,0,a6|c6|d7);
    Put_Kn_map(C8,0,a7|b6|d6|e7);
    Put_Kn_map(D8,0,b7|c6|e6|f7);
    Put_Kn_map(E8,0,c7|d6|f6|g7);
    Put_Kn_map(F8,0,d7|e6|g6|h7);
    Put_Kn_map(G8,0,e7|f6|h6);
    Put_Kn_map(H8,0,f7|g6);
}

init()
{
    if (hisf != NULL)		/* Open game file? Close it. */
      fclose(hisf);
    hisf = NULL;		/* Start fresh game file. */
    tomove = WHITE;		/* White moves first */
    ngamemvs = -1;		/* Moves so far in game */
    zero_game();		/* Start fresh with game log */
    init_diag();		/* Initiate diagonals */
    init_rows();		/* And rows */
    init_cols();		/* And columns */
    init_knight();		/* And knight maps */
    init_king();		/* And King maps */
    init_maps();		/* Everything else */
}

#define Put_Ki_map(sq,low,hi) kingmvmaps[sq][LOBD] = low;\
                              kingmvmaps[sq][HIBD] = hi;

init_king()
{
    Put_Ki_map(A1,a2|b2|b1,0);
    Put_Ki_map(B1,a1|a2|b2|c2|c1,0);
    Put_Ki_map(C1,b1|b2|c2|d2|d1,0);
    Put_Ki_map(D1,c1|c2|d2|e2|e1,0);
    Put_Ki_map(E1,d1|d2|e2|f2|f1,0);
    Put_Ki_map(F1,e1|e2|f2|g2|g1,0);
    Put_Ki_map(G1,f1|f2|g2|h1|h2,0);
    Put_Ki_map(H1,g1|g2|h2,0);
    Put_Ki_map(A2,a3|b3|b2|b1|a1,0);
    Put_Ki_map(B2,a1|a2|a3|b1|b3|c1|c2|c3,0);
    Put_Ki_map(C2,b1|b2|b3|c1|c3|d1|d2|d3,0);
    Put_Ki_map(D2,c1|c2|c3|d1|d3|e1|e2|e3,0);
    Put_Ki_map(E2,d1|d2|d3|e1|e3|f1|f2|f3,0);
    Put_Ki_map(F2,e1|e2|e3|f1|f3|g1|g2|g3,0);
    Put_Ki_map(G2,f1|f2|f3|g1|g3|h1|h2|h3,0);
    Put_Ki_map(H2,g1|g2|g3|h1|h3,0);
    Put_Ki_map(A3,a4|b4|b3|b2|a2,0);
    Put_Ki_map(B3,a2|a3|a4|b4|b2|c2|c3|c4,0);
    Put_Ki_map(C3,b2|b3|b4|c2|c4|d2|d3|d4,0);
    Put_Ki_map(D3,c2|c3|c4|d2|d4|e2|e3|e4,0);
    Put_Ki_map(E3,d2|d3|d4|e2|e4|f2|f3|f4,0);
    Put_Ki_map(F3,e2|e3|e4|f2|f4|g2|g3|g4,0);
    Put_Ki_map(G3,f2|f3|f4|g2|g4|h2|h3|h4,0);
    Put_Ki_map(H3,g2|g3|g4|h2|h4,0);
    Put_Ki_map(A4,a3|b3|b4,a5|b5);
    Put_Ki_map(B4,a3|a4|b3|c3|c4,a5|b5|c5);
    Put_Ki_map(C4,b4|b3|c3|d3|d4,b5|c5|d5);
    Put_Ki_map(D4,c4|c3|d3|e3|e4,c5|d5|e5);
    Put_Ki_map(E4,d3|d4|e3|f3|f4,d5|e5|f5);
    Put_Ki_map(F4,e3|f4|f3|g3|g4,e5|f5|g5);
    Put_Ki_map(G4,f3|f4|g3|h3|h4,f5|g5|h5);
    Put_Ki_map(H4,h3|g3|g4,g5|h5)
    Put_Ki_map(A5,a4|b4,b5|b6|a6);
    Put_Ki_map(B5,a4|b4|c4,a5|c5|a6|b6|c6);
    Put_Ki_map(C5,b4|c4|d4,b5|d5|b6|c6|d6);
    Put_Ki_map(D5,c4|d4|e4,c5|e5|c6|d6|e6);
    Put_Ki_map(E5,d4|e4|f4,d5|f5|d6|e6|f6);
    Put_Ki_map(F5,e4|f4|g4,e5|g5|e6|f6|g6);
    Put_Ki_map(G5,f4|g4|h4,f5|h5|f6|g6|h6);
    Put_Ki_map(H5,g4|h4,g5|g6|h6);
    Put_Ki_map(A6,0,a5|b5|b6|a7|b7);
    Put_Ki_map(B6,0,a5|b5|c5|a6|c6|a7|b7|c7);
    Put_Ki_map(C6,0,b5|c5|d5|b6|d6|b7|c7|d7);
    Put_Ki_map(D6,0,c5|d5|e5|c6|e6|c7|d7|e7);
    Put_Ki_map(E6,0,d5|e5|f5|d6|f6|d7|e7|f7);
    Put_Ki_map(F6,0,e5|f5|g5|e6|g6|e7|f7|g7);
    Put_Ki_map(G6,0,f5|g5|h5|f6|h6|f7|g7|h7);
    Put_Ki_map(H6,0,g5|h5|g6|g7|h7);
    Put_Ki_map(A7,0,a6|b6|b7|a8|b8);
    Put_Ki_map(B7,0,a6|b6|c6|a7|c7|a8|b8|c8);
    Put_Ki_map(C7,0,b6|c6|d6|b7|d7|b8|c8|d8);
    Put_Ki_map(D7,0,c6|d6|e6|c7|d7|c8|d8|e8);
    Put_Ki_map(E7,0,d6|e6|f6|d7|f7|d8|e8|f8);
    Put_Ki_map(F7,0,e6|f6|g6|e7|g7|e8|f8|g8);
    Put_Ki_map(G7,0,f6|g6|h6|f7|h7|f8|g8|h8);
    Put_Ki_map(H7,0,g6|h6|g7|g8|h8);
    Put_Ki_map(A8,0,a7|b7|b8);
    Put_Ki_map(B8,0,a7|b7|c7|a8|c8);
    Put_Ki_map(C8,0,b8|b7|c7|d7|d8);
    Put_Ki_map(D8,0,c8|c7|d7|e7|e8);
    Put_Ki_map(E8,0,d7|e7|f7|d8|f8);
    Put_Ki_map(F8,0,e8|e7|f7|g7|g8);
    Put_Ki_map(G8,0,f7|g7|h7|f8|h8);
    Put_Ki_map(H8,0,g8|g7|h7);
}

