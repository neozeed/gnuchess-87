/* This file contains the table-driven pawn move generator for CHESS.
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

extern mvtabcnt,movtab[MAXPC+1][MAXATTACKS][120];
extern struct dirtypes pieces[MAXPC+6];

addtotab(pc,frsq,tosq)
{
    movtab[pc][mvtabcnt++][frsq] = tosq;
}    

genpwntab(bd,sq,pc)
struct bdtype bd[120];
{
  int rank2low,rank2hi,nxtsq,nxtnxtsq,pcactual,capsq1,capsq2;
  mvtabcnt = 0;
  rank2low = (pc == WP) ? 80 : 30;
  rank2hi = (pc == WP) ? 89 : 39;
  nxtsq = (pc == WP) ? -10 : 10;
  nxtnxtsq = (pc == WP) ? -20 : 20;
  capsq1 = (pc == WP) ? -9 : 9;
  capsq2 = (pc == WP) ? -11 : 11;
  pcactual = (pc == WP) ? WP : BP + 1;
  if (sq > rank2low && sq < rank2hi)
    {
      addtotab(pcactual,sq,sq+nxtsq);
      addtotab(pcactual,sq,sq+nxtnxtsq);
      addtotab(pcactual,sq,0);
    }
  else {
      addtotab(pcactual,sq,sq+nxtsq);
      addtotab(pcactual,sq,0);
    }
  if (bd[sq+capsq1].piece == EMP)
      addtotab(pcactual,sq,sq+capsq1);
  addtotab(pcactual,sq,0);
  if (bd[sq+capsq2].piece == EMP)
      addtotab(pcactual,sq,sq+capsq2);
  addtotab(pcactual,sq,-1);
}

fillpwntab(bd)
struct bdtype bd[120];
{
  register i, j;
  for (j = BP; j <= WP; j += 2)
    for (i = 31; i < 89; i++)
      if ((i % 10 != 0) && (i % 10 != 9))
      {
	clear_bd(bd);
	bd[i].piece = j;
	bd[i].moved = 0;
	genpwntab(bd,i,j);
      }
}
