/* This file contains map-printing routines for CHESS.
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

#include "algdefs.h"
#include "crucialdefs.h"
#include "fixeddefs.h"
#include "externmap.h"
#include "bitmacs.h"

extern char *sqchar[];
char strcpiece[NPIECES] = {
    'P', 'N', 'B', 'R', 'Q', 'K'};
extern unsigned int sqbits[];

prbd()
{
    register i,j,k,colsq,colonsq;
    colsq = 0;
    for (i = 56; i >= 0; i = i - 8)
    {
	for (j = 0; j < 8; j++)
	{
	    if (MEMBER(i+j,empty))
		printf("%s ",(colsq == 0) ? "--" : "**");
	    else {
	        if (MEMBER(i+j,side[WHITE]))
                {
		    colonsq = WHITE;
		    putchar('W');
	        }
	        else if (MEMBER(i+j,side[BLACK]))
                {
		    colonsq = BLACK;
		    putchar('B');
	        }
	        for (k = 0; k < NPIECES; k++)
		    if (MEMBER(i+j,pieces[colonsq][k]))
		        printf("%c ",strcpiece[k]);
	    }
	    colsq ^= 1;
        }
	colsq ^= 1;
	putchar('\n');
    }
}

prsetbits(map)
MAP *map;
{
    register i,j;
    for (i = 0; i < WPERM; i++)
      for (j = 0; j < BPERW; j++)
	{
	  if (sqbits[j] & map[i])
	    printf("%s ",sqchar[j+(BPERW*i)]);
	}
}

print_maps()
{
    int i;
    printf("White: ");
    prsetbits(side[WHITE]);
    printf("\nBlack: ");
    prsetbits(side[BLACK]);
    printf("\nEmpty: ");
    prsetbits(empty);
    printf("\nEdge: ");
    prsetbits(edge);
    printf("\nWhite.Pawn: ");
    prsetbits(pieces[WHITE][PAWN]);
    printf("\nWhite.Knight: ");
    prsetbits(pieces[WHITE][KNIGHT]);
    printf("\nWhite.Bishop: ");
    prsetbits(pieces[WHITE][BISHOP]);
    printf("\nWhite.Rook: ");
    prsetbits(pieces[WHITE][ROOK]);
    printf("\nWhite.Queen: ");
    prsetbits(pieces[WHITE][QUEEN]);
    printf("\nWhite.King: ");
    prsetbits(pieces[WHITE][KING]);
    printf("\nBlack.Pawn: ");
    prsetbits(pieces[BLACK][PAWN]);
    printf("\nBlack.Knight: ");
    prsetbits(pieces[BLACK][KNIGHT]);
    printf("\nBlack.Bishop: ");
    prsetbits(pieces[BLACK][BISHOP]);
    printf("\nBlack.Rook: ");
    prsetbits(pieces[BLACK][ROOK]);
    printf("\nBlack.Queen: ");
    prsetbits(pieces[BLACK][QUEEN]);
    printf("\nBlack.King: ");
    prsetbits(pieces[BLACK][KING]);
    for (i = 0; i < MAXROWS; i++)
      {
	printf("\nRow %d: ",i+1);
	prsetbits(rows[i]);
      }
    for (i = 0; i < MAXCOLS; i++)
      {
	printf("\nCol %d: ",i+1);
	prsetbits(cols[i]);
      }
    for (i = A1; i <= H8; i++)
    {
	printf("\nKnight mv map, sq %s: ",sqchar[i]);
	prsetbits(knightmvmaps[i]);
    }
    for (i = A1; i <= H8; i++)
    {
	printf("\nKing mv map, sq %s: ",sqchar[i]);
        prsetbits(kingmvmaps[i]);
    }
    printf("\n");
}

pramaps(color)
{
     register i;
     for (i = A1; i <= H8; i++)
	 if (NOT_ZERO(attackfr[color][i]))
	   {
	     printf("attackfr[%s][%s]= ",color == WHITE ? "white" : "black",
		    sqchar[i]);
	     prsetbits(attackfr[color][i]);
	     putchar('\n');
	   }
     for (i = A1; i <= H8; i++)
	 if (NOT_ZERO(attackto[color][i]))
	   {
	    printf("attackto[%s][%s]= ",color == WHITE ? "white" : "black",
		 sqchar[i]);
	    prsetbits(attackto[color][i]);
	    putchar('\n');
	   }
     for (i = A1; i <= H8; i++)
	 if (NOT_ZERO(pmovefr[color][i]))
	   {
            printf("pmovefr[%s][%s]= ",color == WHITE ? "white" : "black",
		    sqchar[i]);
	    prsetbits(pmovefr[color][i]);
	    putchar('\n');
	   }
     for (i = A1; i <= H8; i++)
	 if (NOT_ZERO(pmoveto[color][i]))
	   {
            printf("pmoveto[%s][%s]= ",color == WHITE ? "white" : "black",
		    sqchar[i]);
	    prsetbits(pmoveto[color][i]);
	    putchar('\n');
	   }
}
