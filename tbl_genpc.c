/* This file contains the table-driven piece move generator for CHESS.
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

extern struct dirtypes pieces[MAXPC+6];
int movtab[MAXPC+1][MAXATTACKS][120];
int mvtabcnt;

genpctab(bd,sq,pc)
struct bdtype bd[120];
{
    register i, incr;
    mvtabcnt = 0;
    for (i = 0; i < pieces[pc].ndirs; i++)
      {
	incr = pieces[pc].dirs[i];
	if (bd[sq+incr].piece == EMP)
	  {
	    addtotab(pc,sq,sq+incr);
	    if (pieces[pc].slide)
	      {
		incr += pieces[pc].dirs[i];
		while (bd[sq+incr].piece != OFF)
		  {
		    addtotab(pc,sq,sq+incr);
		    incr += pieces[pc].dirs[i];
		  }
	      }
	    addtotab(pc,sq,0);
	  }
      }
    addtotab(pc,sq,-1);
  }

fillpctab(bd)
struct bdtype bd[120];
{
  register i,j;
  for (j = WN; j <= WK; j++)
    for (i = 21; i < 99; i++)
      if ((i % 10 != 0) && (i % 10 != 9))
      {
	clear_bd(bd);
	bd[i].piece = j;
	bd[i].moved = 0;
	genpctab(bd,i,j);
      }
}
