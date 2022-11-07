/* This file contains additional move routines for CHESS.
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

extern int histtot,nmoves;
extern int pcval[MAXPC+1];
extern long hashval;
extern long rands[(2*MAXPC)+1][120];
extern struct mvlist game[MAXGAME];
extern struct gmlist history[MAXGAME];
extern int sqcontrol[120];
extern int cent[MAXPC+1];

struct mvlist save;

/*
 * casrook castles or uncastles a rook from the given square to
 * the given square depending on whether undo is true or false.
 */
casrook(bd,fromsq,tosq,undo)
struct bdtype bd[120];
int fromsq,tosq,undo;
{
#ifdef TREEPOS
    if (bd[fromsq].piece > 0)
      {
      if (!undo)
	bd[WPOS].moved += ((sqcontrol[tosq]*cent[abs(bd[tosq].piece)]) -
			 sqcontrol[fromsq]*cent[abs(bd[fromsq].piece)]);
      else 
	bd[WPOS].moved += ((sqcontrol[fromsq]*cent[abs(bd[fromsq].piece)]) -
			 sqcontrol[tosq]*cent[abs(bd[tosq].piece)]);
      }
    else  {
      if (!undo)
	bd[BPOS].moved += ((sqcontrol[tosq]*cent[abs(bd[tosq].piece)]) -
			 sqcontrol[fromsq]*cent[abs(bd[fromsq].piece)]);
      else
	bd[BPOS].moved += ((sqcontrol[fromsq]*cent[abs(bd[fromsq].piece)]) -
			 sqcontrol[tosq]*cent[abs(bd[tosq].piece)]);
    }
#endif TREEPOS
    bd[tosq].piece = bd[fromsq].piece;
    bd[tosq].moved = bd[fromsq].moved + ((undo == TRUE) ? -1 : 1);
    bd[fromsq].piece = 0;
    bd[fromsq].moved = 0;
}

/*
 * addmove adds the given move characterized by the from/to pair
 * to a move list given the current board on which the move is
 * pseudo-legal.
 */
addmove(bd,from,to,moves)
struct bdtype bd[120];
int from, to;
struct mvlist moves[MAXMOVES];
{
    register i;
    char permflag;
    moves[nmoves].from = from;
    moves[nmoves].to = to;
    moves[nmoves].movpiece = bd[from].piece;
    moves[nmoves].flags = 0;
    moves[nmoves].capcount = 0;
    if (bd[to].piece != 0) {
	moves[nmoves].flags |= CAPFLAG;
	moves[nmoves].cappiece = bd[to].piece;
	moves[nmoves].capcount = bd[to].moved;  /* Save move count */
	permflag = moves[nmoves].flags;
    }
    switch(bd[from].piece) {
	case WK : if (from == 95) switch(to) {
			case 97 : moves[nmoves].flags |= KCASFLAG;
				  break;
			case 93 : moves[nmoves].flags |= QCASFLAG;
				  break;
		  }
		  break;
	case BK : if (from == 25) switch(to) {
			case 27 : moves[nmoves].flags |= KCASFLAG;
				  break;
			case 23 : moves[nmoves].flags |= QCASFLAG;
				  break;
		  }
		  break;
	case WP : if (to < 29) {
		for (i = WQ; i >= WN; i--) {
			moves[nmoves].from = from;
			moves[nmoves].to = to;
			moves[nmoves].movpiece = WP;
			moves[nmoves].propiece = i;
			moves[nmoves].cappiece = bd[to].piece;
			moves[nmoves].flags = permflag;
			moves[nmoves].flags |= PROMFLAG;
			nmoves = nmoves + 1;
		}
		nmoves = nmoves - 1;
		break;
	}
	case BP : if (to > 90) {
		for (i = BQ; i <= BN; i++) {
			moves[nmoves].from = from;
			moves[nmoves].to = to;
			moves[nmoves].movpiece = BP;
			moves[nmoves].propiece = i;
			moves[nmoves].cappiece = bd[to].piece;
			moves[nmoves].flags = permflag;
			moves[nmoves].flags |= PROMFLAG;
			nmoves = nmoves + 1;
		}
		nmoves = nmoves - 1;
		break;
	}
    }
    nmoves = nmoves + 1;
}

/*
 * recordmove records a given move with associated statistics in
 * the game history array.
 */
recordmove(move,depth,nodes,score,cpu,rate,coltomove)
struct mvlist move;
float cpu,rate;
int coltomove,depth,nodes,score;
{
    if (coltomove)
      {
	      histtot++;
	      history[histtot].wmove.from = move.from;
	      history[histtot].wmove.to = move.to;
	      history[histtot].wmove.movpiece = move.movpiece;
	      history[histtot].wmove.cappiece = move.cappiece;
	      history[histtot].wmove.capcount = move.capcount;
	      history[histtot].wmove.propiece = move.propiece;
	      history[histtot].wmove.flags = move.flags;
	      if (depth > 0)
		{
			history[histtot].depth = depth;
			history[histtot].nodes = nodes;
			history[histtot].score = score;
			history[histtot].cpu = cpu;
			history[histtot].rate = rate;
		}
       }
    else {
	      history[histtot].bmove.from = move.from;
	      history[histtot].bmove.to = move.to;
	      history[histtot].bmove.movpiece = move.movpiece;
	      history[histtot].bmove.cappiece = move.cappiece;
	      history[histtot].bmove.capcount = move.capcount;
	      history[histtot].bmove.propiece = move.propiece;
	      history[histtot].bmove.flags = move.flags;
	      if (depth > 0)
		{
			history[histtot].depth = depth;
			history[histtot].nodes = nodes;
			history[histtot].score = score;
			history[histtot].cpu = cpu;
			history[histtot].rate = rate;
		}
      }
}

/*
 * savemove saves the move indexed by index into a moves list in
 * a canonical place.
 */
savemove(moves,index)
struct mvlist moves[MAXMOVES];
int index;
{
    save.from = moves[index].from;
    save.to = moves[index].to;
    save.movpiece = moves[index].movpiece;
    save.cappiece = moves[index].cappiece;
    save.propiece = moves[index].propiece;
    save.flags = moves[index].flags;
}

/*
 * makemove makes a given move on the given board
 */
makemove(moves,bd)
struct mvlist moves;
struct bdtype bd[120];
{
    int fromsq,tosq,movpiece,cappiece,propiece;
    char flags;
    fromsq = moves.from;
    tosq = moves.to;
    bd[tosq].moved = bd[fromsq].moved + 1;  /* increment move count */
    bd[fromsq].moved = 0;
    movpiece = moves.movpiece;
    cappiece = moves.cappiece;
    propiece = moves.propiece;
    flags = moves.flags;
    hashval ^= rands[movpiece+6][fromsq];
    hashval ^= rands[movpiece+6][tosq];
    bd[fromsq].piece = 0;
    bd[tosq].piece = movpiece;
    bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
    if (movpiece == WK)
      bd[WKING].moved = tosq;
    else if (movpiece == BK)
      bd[BKING].moved = tosq;
#ifdef TREEPOS
    if (movpiece > 0)
	    bd[WPOS].moved += ((sqcontrol[tosq]*cent[abs(movpiece)]) -
			       sqcontrol[fromsq]*cent[abs(movpiece)]);
    else
	    bd[BPOS].moved += ((sqcontrol[tosq]*cent[abs(movpiece)]) -
			       sqcontrol[fromsq]*cent[abs(movpiece)]);
#endif TREEPOS
    if (flags & CAPFLAG)	/* update material balance */
  	if (movpiece > 0)
	  {
	    bd[BMAT].moved -= pcval[abs(cappiece)];
#ifdef TREEPOS
	    bd[BPOS].moved -= (sqcontrol[tosq]*cent[abs(cappiece)]);
#endif TREEPOS
	  }
	else {
	    bd[WMAT].moved -= pcval[cappiece];
#ifdef TREEPOS
	    bd[WPOS].moved -= (sqcontrol[tosq]*cent[abs(cappiece)]);
#endif TREEPOS
	  }
    if (flags & PROMFLAG)
    {
	bd[tosq].piece = propiece;
	if (movpiece > 0)
	  {
	    bd[WMAT].moved += (pcval[propiece] - pcval[0]);
#ifdef TREEPOS
	    bd[WPOS].moved += ((sqcontrol[tosq]*cent[abs(propiece)]) -
			       sqcontrol[fromsq]*cent[abs(movpiece)]);
#endif TREEPOS
	  }
	else {
	    bd[BMAT].moved += (pcval[abs(propiece)] - pcval[0]);
#ifdef TREEPOS
	    bd[BPOS].moved += ((sqcontrol[tosq]*cent[abs(propiece)]) -
			       sqcontrol[fromsq]*cent[abs(movpiece)]);
#endif TREEPOS
	  }
    }
    if (flags & (KCASFLAG | QCASFLAG))
	if (movpiece > 0) {
	    if (flags & KCASFLAG)
		casrook(bd,98,96,FALSE);
	    else if (flags & QCASFLAG)
		casrook(bd,91,94,FALSE);
	}
	else {
	    if (flags & KCASFLAG)
		casrook(bd,28,26,FALSE);
	    else if (flags & QCASFLAG)
		casrook(bd,21,24,FALSE);
	}
}		

/*
 * unmakemove unmakes a given move on the given board
 */
unmakemove(moves,bd)
struct mvlist moves;
struct bdtype bd[120];
{
    int fromsq, tosq, movpiece, cappiece, propiece;
    char flags;
    fromsq = moves.from;
    tosq = moves.to;
    movpiece = moves.movpiece;
    cappiece = moves.cappiece;
    propiece = moves.propiece;
    flags = moves.flags;
    hashval ^= rands[movpiece+6][tosq];
    hashval ^= rands[movpiece+6][fromsq];
    bd[tosq].piece = 0;
    bd[fromsq].piece = movpiece;
    bd[fromsq].moved = bd[tosq].moved - 1;
    bd[tosq].moved = 0;
#ifdef TREEPOS
    if (movpiece > 0)
	    bd[WPOS].moved += ((sqcontrol[fromsq]*cent[abs(movpiece)]) -
			       sqcontrol[tosq]*cent[abs(movpiece)]);
    else
	    bd[BPOS].moved += ((sqcontrol[fromsq]*cent[abs(movpiece)]) -
			       sqcontrol[tosq]*cent[abs(movpiece)]);
#endif TREEPOS
    if (flags & CAPFLAG) {
        bd[tosq].moved = moves.capcount;
	if (movpiece > 0)
	  {
	    bd[BMAT].moved += pcval[abs(cappiece)];
#ifdef TREEPOS
	    bd[BPOS].moved += (sqcontrol[tosq]*cent[abs(cappiece)]);
#endif TREEPOS
	  }
	else {
	    bd[WMAT].moved += pcval[cappiece];
#ifdef TREEPOS
	    bd[WPOS].moved += (sqcontrol[tosq]*cent[abs(cappiece)]);
#endif TREEPOS
	  }
	bd[tosq].piece = cappiece;
    }
    if (flags & PROMFLAG)
    {
	if (movpiece > 0)
	  {
	    bd[WMAT].moved -= (pcval[propiece] - pcval[0]);
#ifdef TREEPOS
	    bd[WPOS].moved += ((sqcontrol[fromsq]*cent[abs(movpiece)]) -
			       sqcontrol[tosq]*cent[abs(propiece)]);
#endif TREEPOS
	  }
	else {
	    bd[BMAT].moved -= (pcval[abs(propiece)] - pcval[0]);
#ifdef TREEPOS
	    bd[BPOS].moved += ((sqcontrol[fromsq]*cent[abs(movpiece)]) -
			       sqcontrol[tosq]*cent[abs(propiece)]);
#endif TREEPOS
	  }
    }
    if (flags & (KCASFLAG|QCASFLAG))
	if (movpiece > 0) {
	    if (flags & KCASFLAG)
		casrook(bd,96,98,TRUE);
	    else if (flags & QCASFLAG)
		casrook(bd,94,91,TRUE);
	}
 	else {
	    if (flags & KCASFLAG)
		casrook(bd,26,28,TRUE);
	    else if (flags & QCASFLAG)
		casrook(bd,24,21,TRUE);
	}
    bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
}

/*
 * movem accepts a string which may represent a move and then
 * tries to either make that move on the given board or unmake depending
 * on undo. It returns true if was able to do something constructive,
 * otherwise false.
 */
int movem(moves,cmd,undo,bd)
struct mvlist moves[MAXMOVES];
char *cmd;
int undo;
struct bdtype bd[120];
{
    char frsq[2],tosq[2];
    int nfrsq,ntosq,index,whitomove;
    if (undo == FALSE) {
        generate(bd,moves);
        strncpy(frsq,cmd,2);
        strncpy(tosq,&cmd[2],2);
        nfrsq = alg_to_lin(frsq);
        ntosq = alg_to_lin(tosq);
        index = searchlist(nfrsq,ntosq,moves,nmoves);
        if (index == NOMATCH)
	{
    	    printf("Illegal move\n");
	    return(FALSE);
	}
        else {
		if (bd[TOMOVE].moved == WHITE)
		  whitomove = TRUE;
		else 
		  whitomove = FALSE;
	        recordmove(moves[index],0,0,0,0.0,0.0,whitomove);
		makemove(moves[index],bd);
		return(TRUE);
        }
    }
    else {
	if (histtot > 0) {
	    if (bd[TOMOVE].moved == BLACK)
	      {
		      unmakemove(history[histtot].wmove,bd);
		      histtot--;
	      }
	    else 
	      {
		      unmakemove(history[histtot].bmove,bd);
		      history[histtot].bmove.from = NULL;
	      }
	    
	}
	return(TRUE);
    }
}
