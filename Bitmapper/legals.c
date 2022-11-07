/* This file contains legal-move-list manipulation routines for CHESS.
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
#include "fixeddefs.h"
#include "crucialdefs.h"
#include "externmap.h"
#include "bitmacs.h"
#include "movelist.h"

int nmoves;
struct mvlist mvs[MAXMOVES];

extern unsigned int sqbits[];
extern char *sqchar[];
extern char *strpiece[];

list_legals(nmoves,mvs)
int nmoves;
struct mvlist mvs[MAXMOVES];
{
    char lastchar;
    register i,nprinted;
    for (i = 0; i < nmoves; i++)
      {
	printf("%s%s",sqchar[mvs[i].from],sqchar[mvs[i].to]);
	if (++nprinted % 10 == 0)
	  {
	    putchar('\n');
	    lastchar = '\n';
	  }
	else {
	  putchar(' ');
	  lastchar = ' ';
	}
      }
    if (lastchar != '\n') putchar('\n');
}

fill_legals(color,frsq,map,nmoves,mvs)
int color;
int *nmoves;
struct mvlist mvs[MAXMOVES];
MAP map[2];
{
    register sq;
    for (sq = A1; sq <= H8; sq++)
      {
	if (MEMBER(sq,map))
	  {
	    if (MEMBER(frsq,pieces[color][PAWN]) &&
		!MEMBER(sq,side[OPP(color)]) && (ABS(frsq-sq)%8 != 0))
	      continue;
	    mvs[*nmoves].from = frsq;
	    mvs[*nmoves].to = sq;
	    *nmoves = *nmoves + 1;
	  }
      }
}

create_legals(color,nmoves,mvs)
int color;
int *nmoves;
struct mvlist mvs[MAXMOVES];
{
  MAP temp[2];
  register sq;
  for (sq = A1; sq <= H8; sq++)
    if (NOT_ZERO(attackfr[color][sq]) || NOT_ZERO(pmovefr[color][sq]))
    {
      BIT_NOT(side[color],temp);
      BIT_AND(attackfr[color][sq],temp);
      if (NOT_ZERO(temp))
	fill_legals(color,sq,temp,nmoves,mvs);
      if (NOT_ZERO(pmovefr[color][sq]))
	fill_legals(color,sq,pmovefr[color][sq],nmoves,mvs);
    }
}

legals()
{
    MAP temp[2];
    register sq,color,printed,nprinted;
    for (color = 0; color <= 1; color++)
      {
	nmoves = 0;
	create_legals(color,&nmoves,mvs);
	printf("Total of %d moves for %s:\n",nmoves,
	       color == WHITE ? "white" : "black");
	list_legals(nmoves,mvs);
	printf("\t\t\t-----\n");
      }
}
