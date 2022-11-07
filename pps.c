/* This file contains the positional pre-sort for CHESS.
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
 * Please note: This is where all the important chess knowledge lives.
 * If you add code here, your code does not have to be efficient.
 * Because the knowledge is applied to (at most) 50 or so positions,
 * this happens in a fraction of a second. You can add as much
 * knowledge as inefficiently (or efficiently!) as you like.
 * The search will not be slowed down by the addition of new chess knowledge.
 */

#include <stdio.h>
#include "gnuchess.h"

#define PK2K4       30		/* Bonus for pawn to K4 from K2*/
#define PK3K4        2		/* Bonus for pawn to K4 from K3 */
#define PQ2Q4       20		/* Bonus for pawn to Q4 from Q2 */
#define PQ3Q4        2		/* Bonus for pawn to Q4 from Q3 */
#define KCAS       100		/* Bonus for king-side castle */
#define QCAS        75		/* Bonus for queen-side castle */
#define NR3        -15		/* Penalty for knight moves to rook-3  */
#define BLOKPWN    -50		/* Penalty for blocking center pawn w/ piece */
#define LIBBISH     30		/* Bonus for liberating undeveloped bishops */
#define PCMOVK       2		/* Bonus for moving piece on king side */
#define CAPCENT      5		/* Bonus for pawn captures toward center */
#define CAPACENT    -5		/* Penalty for pawn caps away from center */
#define CAPISOP    -10		/* Penalty for pawn cap giving isolated pawn */
#define WNGPADV    -20		/* Penalty for advancing pawns on the wing */
#define CAPCENTUSUP 50		/* Bonus for taking an unsupported K/Q pawn */
#define CAPCENTSUP -15		/* Penalty for capturing supported K/Q pawn */
#define KMOVE      -40          /* Penalty for moving a king before castling */
#define PDOUB      -15		/* Penalty for pawn move which creates a
				   doubled pawn */
#define PFLANKE      5          /* Bonus for pawn entering phalanx of pawns */
#define PFLANKL     -7          /* Penalty for pawn leaving phalanx */
/* add these for opening before 10th move */
#define N2NDBD       5		/* Knight develop to 2nd rank after
				   corresponding bishop developed */
#define NBISH3      15		/* Knight develop bishop 3 */
#define EQUEENM	   -40    	/* Early queen move penalty */
        			/* Decreased by 10 for each minor piece
				   that has ever developed */
/* and these */
#define ROOKOPEN    40		/* Rook on pure open file (no pawns) */
#define ROOKSOPEN   25		/* Rook on semi-open file (attacking 
				   unsupported enemy pawn) */
#define ROOKSEVEN  100		/* Rook to seventh rank */

int     sqcontrol[120] = {	/* Value of squares on board */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 2, 3, 3, 2, 1, 0, 0,
    0, 1, 3, 4, 5, 5, 4, 3, 1, 0,
    0, 2, 4, 6, 7, 7, 6, 4, 2, 0,
    0, 3, 5, 7, 8, 8, 7, 5, 3, 0,
    0, 3, 5, 7, 8, 8, 7, 5, 3, 0,
    0, 2, 4, 6, 7, 7, 6, 4, 2, 0,
    0, 1, 3, 4, 5, 5, 4, 3, 1, 0,
    0, 0, 1, 2, 3, 3, 2, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * Multiplier of sqcontrol based on piece-type, e.g. bonus for moving to square */
int cent[MAXPC+1] = {0, 1, 4, 3, 2, 1, -1}; 
extern int pcval[MAXPC+1];
extern histtot,sorttype,propiece,compare();

/* Return castling bonuses if appropriate */
ppscastle(movpiece,flags)
char flags;
{
    if (abs(movpiece) == WK)
      {
	if (flags&KCASFLAG) return(KCAS);
        else if (flags&QCASFLAG) return(QCAS);
      }
}

/* Modify centrality bonus if in middle-game */
ppsmiddle(from,to)
{
    cent[1] = 3;
    cent[2] = 4;
    cent[3] = 3;
    cent[4] = 2;
    cent[5] = 1;
    cent[6] = 1;
    return(0);
}

/* Modify centrality bonus if in end-game */
ppsending(from,to)
{
    cent[1] = 0;
    cent[2] = 4;
    cent[3] = 3;
    cent[4] = 1;
    cent[5] = 1;
    cent[6] = 4;
    return(0);
}

/*
 * Opening heuristics
 */
ppsopening(bd,from,to,movpiece,flags)
struct bdtype bd[120];
char flags;     
{
        int nundminors;
	int posval = 0;
	switch (movpiece) {
	    case WN:
	    case BN:
		if ((movpiece == WN && from == 92 && bd[93].piece == WB) ||
		    (movpiece == WN && from == 97 && bd[96].piece == WB) ||
		    (movpiece == BN && from == 22 && bd[23].piece == BB) ||
		    (movpiece == BN && from == 27 && bd[26].piece == BB))
		    posval += N2NDBD;
		break;
	    case WB:
	    case BB:
		if (((from == 92 && to == 73) || (from == 97 && to == 76)) ||
		    ((from == 22 && to == 43) || (from == 27 && to == 46)))
		    posval += NBISH3;
	        break;
	    case WQ:
	    case BQ:
		if (histtot <= 10)
		{
		    if (movpiece == WQ)
			nundminors = (bd[92].piece != WN +
				      bd[93].piece != WB +
				      bd[96].piece != WB +
				      bd[97].piece != WN);
		    else
			nundminors = (bd[22].piece != BN +
				      bd[23].piece != BB +
				      bd[26].piece != BB +
				      bd[27].piece != BN);
		    /* Decrease queen penalty by 10 for each developed
		       minor piece */
		    posval += (EQUEENM + (nundminors * 10));
		}
	}
    return(posval);
}	

ppsheuristics(bd,from,to,movpiece,flags)
struct bdtype bd[120];
char flags;
{
    int i,doub;
    int posval = 0;
    posval -= (bd[from].moved*5);	/* Penalize early multiple moves */
    switch(movpiece) {
       case WP: 
       case BP:
	        posval += (((to % 10 > 1 && bd[to-1].piece == movpiece) ||
			   (to % 10 < 8 && bd[to+1].piece == movpiece)) ?
			   PFLANKE : 0) +
			  (((from % 10 > 1 && bd[from-1].piece == movpiece)
			    || (from % 10 < 8 && bd[from+1].piece == movpiece))
			   ? PFLANKL : 0);
	    	posval +=
	                  (((movpiece == WP) && (from % 10 <= 4) &&
			    ((from - to) % 9 == 0)) ||
			   (movpiece == BP && (from % 10 <= 4) &&
			    ((from - to) % 11 == 0)) ||
			   (movpiece == WP && (from % 10 >= 5) &&
			    ((from - to) % 11 == 0)) ||
			   (movpiece == BP && (from % 10 >= 5) &&
			    ((from - to) % 9 == 0)) ? CAPCENT : 0);
		posval += 
	                  (((movpiece == WP) && (from % 10 <= 4) &&
			    ((from - to) % 11 == 0)) ||
			   (movpiece == BP && (from % 10 <= 4) &&
			    ((from - to) % 9 == 0)) ||
			   (movpiece == WP && (from % 10 >= 5) &&
			    ((from - to) % 9 == 0)) ||
			   (movpiece == BP && (from % 10 >= 5) &&
			    ((from - to) % 11 == 0)) ? CAPACENT : 0);
		posval +=   (((movpiece == BP && ((from == 34 && 
			      bd[23].piece == BB) || ((from == 35 &&
			      bd[26].piece == BB)))) ||
			   ((movpiece == WP && ((from == 84 && 
			      bd[93].piece == WB) || ((from == 85 &&
					       bd[96].piece == WB))))))
			    ? LIBBISH : 0);
		posval +=
	                  (((from == 85 && to == 65) ||
			    (from == 35 && to == 55)) ? PK2K4 : 0) +
			  (((from == 75 && to == 65) ||
			    (from == 45 && to == 55)) ? PK3K4 : 0);
		posval +=
			  (((from == 84 && to == 64) ||
			    (from == 34 && to == 54)) ? PQ2Q4 : 0) +
			  (((from == 74 && to == 64) ||
			    (from == 44 && to == 54)) ? PQ3Q4 : 0) +
			  (((from % 10 == 1 || from % 10 == 8) &&
			    (to % 10 == 1 || to % 10 == 8)) ? WNGPADV : 0);
		doub = 0;
		for (i = from; i < 99; i+=10)
		    if (bd[i].piece == movpiece)
			doub++;
		for (i = from; i > 20; i-=10)
		    if (bd[i].piece == movpiece)
			doub++;
		posval += (doub != 0 ? PDOUB : 0);
		break;
      case WN:
      case BN: posval += ((movpiece == WN && (to == 71 || to == 78))
			       ? NR3 : 0) +
			     ((movpiece == BN && (to == 31 || to == 38))
			       ? NR3 : 0) +
				 ((from % 10 >= 5) ? PCMOVK : 0);
		posval += (((movpiece == WN) && ((to == 74 && 
						bd[84].piece == WP) ||
			   (to == 75 && bd[85].piece == WP))) ||
			 (((movpiece == BN) && ((to == 44 && 
						bd[34].piece == BP) ||
  			  (to == 45 && bd[35].piece == BP)))) ? BLOKPWN : 0);
                break;
      case WQ:
      case BQ:	posval += (((movpiece == WQ) && ((to == 74 && 
						bd[84].piece == WP) ||
			   (to == 75 && bd[85].piece == WP))) ||
			 (((movpiece == BQ) && ((to == 44 && 
						bd[34].piece == BP) ||
  			  (to == 45 && bd[35].piece == BP)))) ? BLOKPWN : 0);
		break;		
      case WK:
      case BK:
		posval += ((bd[(movpiece == WK) ? 93 : 23].moved == 0) &&
			    (to != 97 && to != 93 && to != 27 &&
			     to != 23) ? KMOVE : 0);
   		break;
      case WB:
      case BB: posval += ((from % 10 == 5 || from % 10 == 6 ||
			      from % 10 == 7 || from % 10 == 8)
			       ? PCMOVK : 0) +
			 (((movpiece == WB) && ((to == 74 && 
						bd[84].piece == WP) ||
			   (to == 75 && bd[85].piece == WP))) ||
			 (((movpiece == BB) && ((to == 44 && 
						bd[34].piece == BP) ||
  			  (to == 45 && bd[35].piece == BP)))) ? BLOKPWN : 0);

                break;
      }
      return(posval);
}			    

pps(bd,moves)
struct bdtype bd[120];
struct mvlist moves[MAXMOVES];
{
    register i,from,to,posval,width,twidth,ourkloc,hiskloc,incheck,
	     real_width,spec;
    struct mvlist tempmoves[MAXMOVES];
    /* Scan board trying to find our king */
    ourkloc = locate_king(bd);
    /* Set variable incheck according to if king of side-to-move */
    /* is in check */
    incheck = sqattacked(bd,ourkloc,OPPCOLOR(bd[TOMOVE].moved));
    /* Generate all moves, pseudo-legal or otherwise */
    width = generate(bd,moves);
    real_width = width;
    for (i = 0; i < width; i++)
      {
        posval = 0;
	twidth = 0;
	/*
         * The following is necessary to prune out all moves which
	 * leave a checked king in check.
	 */
	makemove(moves[i],bd);
	bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
	ourkloc = locate_king(bd);
	bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
	incheck = sqattacked(bd,ourkloc,bd[TOMOVE].moved);
	unmakemove(moves[i],bd);
	if (incheck)
	  {
/*
                lin_to_alg(moves[i].from,stdout);
	        lin_to_alg(moves[i].to,stdout);
	 	printf(" leaves computer in check... discarding\n");
*/
		moves[i].score = MAXINT;
		real_width--;
		continue;
	  }
	from = moves[i].from;
	to = moves[i].to;
#ifdef SPPSDEBUG
        lin_to_alg(from,stdout);
	lin_to_alg(to,stdout);
	printf(" = ");
#endif
/*  temporarily disable this, not sure how good it is.
	if (histtot <= 10)
	  posval += ppsopening(bd,from,to,moves[i].movpiece,moves[i].flags);
*/
	/* Give castling credit and heuristic credit */
	posval += ppscastle(moves[i].movpiece,moves[i].flags) +
	          ppsheuristics(bd,from,to,moves[i].movpiece,moves[i].flags);
	/* Modify mobility credits according to phase of game */
	if (bd[WMAT].moved <= 6920 || bd[BMAT].moved <= 6920)
	  posval += ppsending(from,to);
	else
	  posval += ppsmiddle(from,to);
	/* Now credit extra mobility */
	makemove(moves[i],bd);
	twidth = 0;
	bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
	twidth = generate(bd,tempmoves);
    	posval += twidth;
	unmakemove(moves[i],bd);
	bd[TOMOVE].moved = OPPCOLOR(bd[TOMOVE].moved);
#ifdef SPPSDEBUG
	printf("ppsphase(%d) + cent(%d) + cap(%d) + prom(%d)=",
          posval,(sqcontrol[to]-sqcontrol[from])*cent[abs(moves[i].movpiece)],
	       (moves[i].flags&CAPFLAG) ? pcval[abs(moves[i].cappiece)] : 0,
	       (moves[i].flags&PROMFLAG) ? 
	       pcval[abs(moves[i].propiece)] : 0);
#endif
	/* Credit square control */
	posval += (sqcontrol[to]-sqcontrol[from])*
	  cent[abs(moves[i].movpiece)];
	/* Force captures to top of list */
	if (moves[i].flags&CAPFLAG)
	  posval += pcval[abs(moves[i].cappiece)];
	/* Force promotions to top of list */
	if (moves[i].flags&PROMFLAG)
	  posval += pcval[abs(moves[i].propiece)];
#ifdef SPPSDEBUG
	printf("%d\n",posval);
#endif
        moves[i].score = -posval;
      }
    /* Sort the whole list using the scores, best move to worst move */
    sorttype = SCORESORT;
    qsort(moves,width,sizeof(struct mvlist),compare);
    return(real_width);
}
