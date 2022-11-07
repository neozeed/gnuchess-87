/* This file contains the table-driven move generator for CHESS.
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
#include "gnuchess.h"

extern char *pctype[(MAXPC*2)+1];
extern struct dirtypes pieces[MAXPC+6];
extern int movtab[MAXPC+1][MAXATTACKS][120];
extern int mvtabcnt,nmoves;

listtabs()
{
    register pc,sq,attacks;
    for (pc = WN; pc <= WK; pc++)
      {
	printf("%s on ",pctype[pc+6]);
	for (sq = 21; sq < 99; sq++)
	  if ((sq % 10 != 0) && (sq % 10 != 9))
	  {
	    lin_to_alg(sq,stdout); printf(" ");
	    for (attacks = 0; (attacks < MAXATTACKS) && 
		 (movtab[pc][attacks][sq] != -1); attacks++)
	      {
		if (movtab[pc][attacks][sq] != 0)
		  {
		    lin_to_alg(movtab[pc][attacks][sq],stdout);
		    putchar(' ');
		  }
	      }
	    putchar('\n');
	    if (sq != 98) printf("     ");
	  }
      }
    for (pc = BP+1; pc <= WP; pc++)
      {
	if (pc == BP+1) printf("BP on "); else printf("WP on ");
	for (sq = 31; sq < 89; sq++)
	  if ((sq % 10 != 0) && (sq % 10 != 9))
	  {
	    lin_to_alg(sq,stdout); printf(" ");
	    for (attacks = 0; (attacks < MAXATTACKS) && 
		 (movtab[pc][attacks][sq] != -1); attacks++)
	      {
		if (movtab[pc][attacks][sq] != 0)
		  {
		    lin_to_alg(movtab[pc][attacks][sq],stdout);
		    putchar(' ');
		  }
	      }
	    putchar('\n');
	    if (sq != 98) printf("     ");
	  }
      }
  }

genman(bd,moves,pc,sq)
struct bdtype bd[120];
struct mvlist moves[MAXMOVES];
{
    register i,tosq;
    tosq = movtab[pc][0][sq];
    for (i = 0; tosq != -1; i++)
    {
	if (tosq != 0)
	{
	    if (bd[tosq].piece == EMP &&
			!((pc == WP || pc == BP+1) && (i >= 3)))
		addmove(bd,sq,tosq,moves);
	    else if (bd[tosq].piece == OFF ||
		(COLOR(bd[sq].piece) == COLOR(bd[tosq].piece)))
		{
		        tosq = movtab[pc][i][sq];
		        for (; tosq != -1; i++)
			    if (tosq == 0) break;
			    else tosq = movtab[pc][i+1][sq];
		}
	    else
		if (COLOR(bd[sq].piece) == OPPCOLOR(bd[tosq].piece))
		{
		    addmove(bd,sq,tosq,moves);
		    tosq = movtab[pc][i][sq];
		    for (; tosq != -1; i++)
			if (tosq == 0) break;
		        else tosq = movtab[pc][i+1][sq];
		}
	}
    tosq = movtab[pc][i+1][sq];
    }
}

generate(bd,moves)
struct bdtype bd[120];
struct mvlist moves[MAXMOVES];
{
    register i,j;
    nmoves = 0;
    if (bd[TOMOVE].moved == WHITE)
    {
	for (i = 21; i < 99; i++)
	    if (bd[i].piece >= WP && bd[i].piece <= WK)
		genman(bd,moves,bd[i].piece,i);
    }
    else
	for (i = 21; i < 99; i++)
	    if (bd[i].piece >= BK && bd[i].piece <= BP)
	    {
		if (bd[i].piece == BP)
		    genman(bd,moves,bd[i].piece+1,i);
		else
		    genman(bd,moves,abs(bd[i].piece),i);
	    }
    return(nmoves);
}
showstatic()
{
}
modmat()
{
}
sqattacked()
{
}
showattacks()
{
}
