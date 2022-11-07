/* This file contains the move-generator for CHESS.
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

#define pseudolegal(fromval,toval) ((toval == 0) ? NORMAL : ((fromval <= 6 && fromval >= 1 && toval <= 6 && toval >= 1 ? DEFENSE : ((fromval <= 6 && fromval >= 1 && toval < 0) || (fromval < 0 && toval <= 6 && toval >= 1) ? CAPTURE : OFF))))

int attackers[10],defenders[10],natt,ndef;

extern int nmoves,gentype;
extern struct dirtypes pieces[MAXPC+6];
extern int pcval[MAXPC+1];
extern char *pctype[(MAXPC*2)+1];

#define addmove(bd,_from,_to,moves)\
{\
    moves[nmoves].from = _from;\
    moves[nmoves].to = _to;\
    moves[nmoves].movpiece = bd[_from].piece;\
    moves[nmoves].flags = 0;\
    moves[nmoves].capcount = 0;\
    if (bd[_to].piece != 0) {\
	moves[nmoves].flags |= CAPFLAG;\
	moves[nmoves].cappiece = bd[_to].piece;\
	moves[nmoves].capcount = bd[_to].moved;  /* Save move count */\
	permflag = moves[nmoves].flags;\
    }\
    switch(bd[_from].piece) {\
	case WK : if (_from == 95) switch(_to) {\
			case 97 : moves[nmoves].flags |= KCASFLAG;\
				  break;\
			case 93 : moves[nmoves].flags |= QCASFLAG;\
				  break;\
		  }\
		  break;\
	case BK : if (_from == 25) switch(_to) {\
			case 27 : moves[nmoves].flags |= KCASFLAG;\
				  break;\
			case 23 : moves[nmoves].flags |= QCASFLAG;\
				  break;\
		  }\
		  break;\
	case WP : if (_to < 29) {\
		for (ii = WQ; ii >= WN; ii--) {\
			moves[nmoves].from = _from;\
			moves[nmoves].to = _to;\
			moves[nmoves].movpiece = WP;\
			moves[nmoves].propiece = ii;\
			moves[nmoves].cappiece = bd[_to].piece;\
			moves[nmoves].flags = permflag;\
			moves[nmoves].flags |= PROMFLAG;\
			nmoves = nmoves + 1;\
		}\
		nmoves = nmoves - 1;\
		break;\
	}\
	case BP : if (_to > 90) {\
		for (ii = BQ; ii <= BN; ii++) {\
			moves[nmoves].from = _from;\
			moves[nmoves].to = _to;\
			moves[nmoves].movpiece = BP;\
			moves[nmoves].propiece = ii;\
			moves[nmoves].cappiece = bd[_to].piece;\
			moves[nmoves].flags = permflag;\
			moves[nmoves].flags |= PROMFLAG;\
			nmoves = nmoves + 1;\
		}\
		nmoves = nmoves - 1;\
		break;\
	}\
    }\
    nmoves = nmoves + 1;\
}

/*
 * Generate creates a move list given the current board configuration.
 * The move list will contain all pseudo-legal moves for the side on
 * move.
 */
int generate(bd,moves)
register struct bdtype *bd;
register struct mvlist *moves;
{
  register ii, i,j,emptsq,incr,movetype,type;
  register rank2lo,rank2hi,pwncap1,pwncap2,pwnmov1,pwnmov2,pc,oppcol;
  register kingloc,krloc,qrloc,ksdest,qsdest,kingtyp,rooktyp;
  char permflag;

  nmoves = 0;
  if (bd[TOMOVE].moved == WHITE) {  
    rank2lo = 80; rank2hi = 89; pwncap1 = -9; pwncap2 = -11;
    pwnmov1 = -10; pwnmov2 = -20; pc = WP; oppcol = BLACK;
    kingloc = 95; krloc = 98; qrloc = 91; ksdest = 97; qsdest = 93;
    kingtyp = WK; rooktyp = WR;
  } else {
    rank2lo = 30; rank2hi = 39; pwncap1 = 9; pwncap2 = 11;
    pwnmov1 = 10; pwnmov2 = 20; pc = BP; oppcol = WHITE;
    kingloc = 25; krloc = 28; qrloc = 21; ksdest = 27; qsdest = 23;
    kingtyp = BK; rooktyp = BR;
  }    
  if (bd[kingloc].piece == kingtyp && bd[krloc].piece == rooktyp &&
      bd[kingloc].moved == 0 && bd[krloc].moved == 0 &&
      bd[kingloc+1].piece == EMP && bd[krloc-1].piece == EMP &&
      !sqattacked(bd,kingloc,oppcol) && !sqattacked(bd,kingloc+1,oppcol) &&
      !sqattacked(bd,krloc-1,oppcol))
    {
      addmove(bd,kingloc,ksdest,moves);
    }
  if (bd[kingloc].piece == kingtyp && bd[qrloc].piece == rooktyp &&
      bd[kingloc].moved == 0 && bd[qrloc].moved == 0 &&
      bd[kingloc-1].piece == EMP && bd[qrloc+1].piece == EMP &&
      bd[qrloc+2].piece == EMP && !sqattacked(bd,kingloc,oppcol) &&
      !sqattacked(bd,kingloc-1,oppcol) && !sqattacked(bd,qrloc+1,oppcol) &&
      !sqattacked(bd,qrloc+2,oppcol) && !sqattacked(bd,qrloc+3,oppcol))
    {
      addmove(bd,kingloc,qsdest,moves);
    }
  for (i = 21; i <= 98; i++)
    if (bd[i].piece == pc) {
     if (COLOR(bd[i+pwncap1].piece) == oppcol)
	addmove(bd,i,i+pwncap1,moves);
     if (COLOR(bd[i+pwncap2].piece) == oppcol)
	addmove(bd,i,i+pwncap2,moves);
     emptsq = (bd[i+pwnmov1].piece == EMP);
     if (emptsq) {
	if (i > rank2lo && i < rank2hi && bd[i+pwnmov2].piece == EMP)
	  addmove(bd,i,i+pwnmov2,moves);
	addmove(bd,i,i+pwnmov1,moves);
      }
    }
  else if (COLOR(bd[i].piece) == bd[TOMOVE].moved)
    {
      type = abs(bd[i].piece);
      for (j = 0; j < pieces[type].ndirs; j++)
	{
	  incr = pieces[type].dirs[j];
	  for (;;) {
	    movetype = pseudolegal(bd[i].piece,bd[i+incr].piece);
	    if (movetype == NORMAL || movetype == CAPTURE)
	      addmove(bd,i,i+incr,moves);
	    if (!pieces[type].slide || movetype != NORMAL)
	      break;
	    incr += pieces[type].dirs[j];
	  }
	}
    }
    return(nmoves);
}

/*
 * sqattacked returned true if the given square on the given board
 * is attacked by any piece of the given color.
 */
sqattacked(bd,sq,attackcol)
register struct bdtype *bd;
{
    int i, j, incr, pc;
/*
    printf("Is square %d attacked by color %d\n",sq,attackcol);
*/
    if (attackcol == WHITE)
      {
	if (bd[sq+9].piece == WP || bd[sq+11].piece == WP)
	    return(TRUE);
      }
    else if (attackcol == BLACK)
      {
	if (bd[sq-9].piece == BP || bd[sq-11].piece == BP)
	  return(TRUE);
      }
    for (i = WN; i <= WK; i++)
      {
	pc = (attackcol == WHITE) ? i : -i;
	for (j = 0; j < pieces[i].ndirs; j++)
	  {
	    incr = pieces[i].dirs[j];
	    for (;;)
	      {
		if (bd[sq+incr].piece == pc)
		    return(TRUE);
		if (!pieces[i].slide || bd[sq+incr].piece != EMP)
		  break;
		incr += pieces[i].dirs[j];
	      }
	}
      }
    return(FALSE);
}

/*
 * showstatic prints a list of the static exchanges that will
 * occur on a square during a swapoff of all pieces that attack
 * a piece on that square.
 */
showstatic(bd,sq)
struct bdtype bd[120];
{
  printf("staticexchange at ");
  lin_to_alg(sq,stdout);
  putchar('\n');
  fflush(stdout);
  printf(" = %d\n",staticexchange(bd,sq));
}

#define addatt(val) {attackers[natt++] = val;}
#define adddef(val) {defenders[ndef++] = val;}

/*
 * showattacks prints a list of the pieces which attack the given
 * square on the given board.
 */
showattacks(bd,sq)
struct bdtype bd[120];
{
    int i, j, incr, pc, gotone,viccol;
    natt = ndef = 0;
    viccol = COLOR(bd[sq].piece);
    for (i = BP+1; i <= WK; i++)
      {
	for (j = 0; j < pieces[i].ndirs; j++)
	  {
	    incr = pieces[i].dirs[j];
	    for (;;)
	      {
		if (((i == BP+1) && (bd[sq+incr].piece == BP)) ||
		  ((abs(bd[sq+incr].piece) == i) && (i != BP+1) &&
		   bd[sq+incr].piece != BP))
		  {
		    if (COLOR(bd[sq+incr].piece) == viccol)
		      adddef(pcval[abs(bd[sq+incr].piece)])
		    else
		      addatt(pcval[abs(bd[sq+incr].piece)]);
#ifdef EXCHDEBUG
		    printf("%s at ",(bd[sq+incr].piece < 0) ?
 			pctype[abs(bd[sq+incr].piece)-1] :
 			pctype[bd[sq+incr].piece+6]);
		    lin_to_alg(sq+incr,stdout);
		    putchar('\n');
#endif
		  }
		if (!pieces[i].slide || bd[sq+incr].piece != EMP)
		  break;
		incr += pieces[i].dirs[j];
	      }
	  }
      }
}

/*
 * staticexchange actually does a full static exchange on the
 * given square, returning a residue representing how much
 * material is won or lost.
 */
staticexchange(bd,sq)
struct bdtype bd[120];
{
    int attindex,defindex,prevresidue,residue,i;
    showattacks(bd,sq);
#ifdef EXCHDEBUG
    printf("natt = %d, ndef = %d\n",natt,ndef);
    for (i = 0; i < natt; i++)
      printf("attackers[%d]=%d\n",i,attackers[i]);
    for (i = 0; i < ndef; i++)
      printf("defenders[%d]=%d\n",i,defenders[i]);
#endif
    if (natt == 0) return(0);
    else if (ndef == 0) return(pcval[abs(bd[sq].piece)]);
    attindex = -1;
    defindex = -2;
    residue = 0;
    for (i = 1;; i++)
      {
	if (++defindex < ndef)
	  {
	    prevresidue = residue;
	    if (i == 1)
	      residue += pcval[abs(bd[sq].piece)];
	    else
	      residue += defenders[defindex];
	    if (residue < 0)
	      {
#ifdef EXCHDEBUG
		printf("End Defender = %d, prevresidue = %d, residue = %d\n",
		       i,prevresidue,residue);
#endif
		return(MIN(MAX(prevresidue,0),pcval[abs(bd[sq].piece)]));
	      }
#ifdef EXCHDEBUG
    	    printf("Defender = %d, prevresidue = %d, residue = %d\n",
	       i,prevresidue,residue);
#endif
	  }
	else break;
	if (++attindex < natt)
	  {
	    prevresidue = residue;
	    residue -= attackers[attindex];
	    if (residue > pcval[abs(bd[sq].piece)])
	      {
#ifdef EXCHDEBUG
		printf("End Attacker = %d, prevresidue = %d, residue = %d\n",
		       i,prevresidue,residue);
#endif
		return(MIN(MAX(prevresidue,0),pcval[abs(bd[sq].piece)]));
	      }
#ifdef EXCHDEBUG
    	    printf("Attacker = %d, prevresidue = %d, residue = %d\n",
	       i,prevresidue,residue);
#endif
	  }
	else break;
      }
#ifdef EXCHDEBUG
    printf("End Iter, prevresidue = %d, residue = %d\n",
       prevresidue,residue);
#endif
    return(MIN(MAX(prevresidue,0),pcval[abs(bd[sq].piece)]));
}

/*
 * modmat looks at the material value for both sides on the
 * given board, and returns a value which, if added to that
 * material value, represents the actual material value after
 * static exchanges are taken into account.
 */
modmat(bd)
struct bdtype bd[120];
{
    int i,movemat,movemat2,curmat2,movedmat,curmat;
    movemat = movemat2 = curmat2 = movedmat = curmat = 0;
    for (i = 21; i <= 98; i++)
      if (bd[i].piece != EMP && bd[i].piece != OFF)
	{
#ifdef EXCHDEBUG
	  printf("sq = ");
	  lin_to_alg(i,stdout);
	  putchar('\n');
#endif
	  if (OPPCOLOR(COLOR(bd[i].piece)) == bd[TOMOVE].moved)
	    {
	      showattacks(bd,i);
	      curmat = staticexchange(bd,i);
#ifdef EXCHDEBUG
	      printf("curmat = %d\n",curmat);
#endif
	      if (curmat > movedmat)
		movedmat = curmat;
	    }
	  else if (COLOR(bd[i].piece) == bd[TOMOVE].moved)
	    {
	      showattacks(bd,i);
	      curmat2 = staticexchange(bd,i);
#ifdef EXCHDEBUG
	      printf("curmat2 = %d\n",curmat2);
#endif
	      if (curmat2 > movemat)
	      {
		movemat2 = movemat;
		movemat = curmat2;
	      }
	      else if (curmat2 > movemat2)
		movemat2 = curmat2;
	    }
	}
#ifdef EXCHDEBUG
    printf("modmat: movedmat = %d, movemat = %d, movemat2 = %d\n",
	   movedmat,movemat,movemat2);
    printf("modmat: movedmat - (.75 * movemat2) = %f\n",
	   (float)movedmat - (.75 * (float)movemat2));
#endif
    return(movedmat - (.75 * movemat2));
 }
