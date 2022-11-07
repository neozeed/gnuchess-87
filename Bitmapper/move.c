/* This file contains move-routines for CHESS.
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

extern tomove,sqorigin,sqdest;
extern char *strpiece[];
extern *sqchar[];
extern unsigned int sqbits[];
extern gfrsq,gtosq,gmover,gpc;

int ngamemvs;
struct gmlist game[MAXGAME];

makemvstr(cmd)
char *cmd;
{
    register frow,fcol,trow,tcol,pc,frsq,tosq,mover;
    frow = cmd[1] - '0';
    trow = cmd[3] -'0';
    fcol = cmd[0] - 'a';
    tcol = cmd[2] - 'a';
    frsq = fcol+((frow-1)*8);
    tosq = tcol+((trow-1)*8);
    for (pc = 0; pc < NPIECES; pc++)
      {
      if ((tomove == WHITE) && (MEMBER(frsq,pieces[WHITE][pc])))
	{
	  mover = WHITE;
          makemvpc(mover,frsq,tosq,pc);
	  break;
	}
      else  if ((tomove == BLACK) && MEMBER(frsq,pieces[BLACK][pc]))
	{
	  mover = BLACK;
          makemvpc(mover,frsq,tosq,pc);
	  break;
	}
    }
    gfrsq = frsq;
    gtosq = tosq;
    gmover = mover;
    gpc = pc;
}

makemvpc(color,frsq,tosq,pc)
{
    register i,takepc;
    sqorigin = frsq;
    sqdest = tosq;
    takepc = -1;
    BIT_ZERO(attackfr[color][frsq]);
    BIT_ZERO(pmovefr[color][frsq]);
    BIT_ZERO(pmovefr[color][tosq]);
    BIT_ZERO(pmoveto[color][tosq]);
    BIT_PUNSET(frsq,pc,color,pieces);
    BIT_PSET(tosq,pc,color,pieces);
    BIT_S(tosq,side[color]);
    BIT_U(frsq,side[color]);
    BIT_S(frsq,empty);
    BIT_U(tosq,empty);
    if (MEMBER(tosq,side[OPP(color)]))
	for (i = 0; i < NPIECES; i++)
	  if (MEMBER(tosq,pieces[OPP(color)][i]))
	    {
	      takepc = i;
	      break;
	    }
    if (takepc != -1)
      {
	BIT_PUNSET(tosq,takepc,OPP(color),pieces);
	BIT_U(tosq,side[OPP(color)]);
	BIT_ZERO(attackfr[OPP(color)][tosq]);
      }
    ++ngamemvs;			/* Update game list */
    game[ngamemvs].move.from = frsq;
    game[ngamemvs].move.to = tosq;
    game[ngamemvs].move.movpiece = pc;
    game[ngamemvs].move.cappiece = takepc;
    game[ngamemvs].depth = 0;
    tomove = OPP(tomove);
}

unmakemvpc(color,frsq,tosq,pc)
{
    register i,takepc;
    sqorigin = tosq;
    sqdest = frsq;
    takepc = -1;
    BIT_ZERO(attackfr[color][frsq]);
    BIT_ZERO(pmovefr[color][frsq]);
    BIT_ZERO(pmovefr[color][tosq]);
    BIT_ZERO(pmoveto[color][frsq]);
    BIT_PUNSET(tosq,pc,color,pieces);
    BIT_PSET(frsq,pc,color,pieces);
    BIT_S(frsq,side[color]);
    BIT_U(tosq,side[color]);
    BIT_S(tosq,empty);
    BIT_U(frsq,empty);
    if (game[ngamemvs].move.cappiece >= PAWN &&
	game[ngamemvs].move.cappiece <= KING)
      {
	BIT_S(tosq,side[OPP(color)]);
	BIT_U(tosq,empty);
	BIT_PSET(tosq,game[ngamemvs].move.cappiece,OPP(color),pieces);
      }
    game[ngamemvs].depth = -1;
    --ngamemvs;			/* Update game list */
    tomove = OPP(tomove);
}

