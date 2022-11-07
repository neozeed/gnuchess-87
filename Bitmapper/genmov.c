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
#include "algdefs.h"
#include "fixeddefs.h"
#include "crucialdefs.h"
#include "externmap.h"
#include "movelist.h"
#include "bitmacs.h"

#define UPDATE TRUE
#define NOT_UPDATE FALSE

int rookdirs[4] = {-1, 1, 8, -8};
int bishdirs[4] = { 9, -9, 7, -7};

char *strpiece[NPIECES] = {
  "pawn", "knight", "bishop", "rook", "queen", "king"};
extern char *sqchar[];
extern unsigned int sqbits[];
extern okprint,sqorigin,sqdest,ngamemvs;
extern struct gmlist game[MAXGAME];

genpiece(update,color,sq,sqorigin,sqdest)
{
    register k;
    for (k = 0; k < NPIECES; k++)
	  if (MEMBER(sq,pieces[color][k]))
	    {
		    switch(k) {
			 case KNIGHT:
			      genknight(color,sq);
			   break;
			 case BISHOP:
			      genbishop(color,sq,BISHOP);
			   break;
			 case ROOK:
			      genrook(color,sq,ROOK);
			   break;
			 case QUEEN:
			      genbishop(color,sq,BISHOP);
			      genrook(color,sq,ROOK);
			   break;
			 case KING:
			      genking(color,sq);
			   break;
		         case PAWN:
			      if (update)
				genonepawn(color,sq);
			 default:
			      break;
			 }
		    break;
		  }
}

/*
 * Incremental move updating.
 *    Regenerate all moves for pieces that match the following
 *    criteria:
 *  1. Piece last moved
 *  2. Piece last captured
 *  3. Piece attacking origination square of piece last moved
 *  4. Piece attacking destination square of piece last moved
 *  5. Pawn behind origination square of piece last moved
 *  6. Pawn behind destination square of piece last moved
 */
genupdate(color,down)
{
    register sq,sqorigin,sqdest;
    sqorigin = game[(down == TRUE) ? ngamemvs+1 : ngamemvs].move.from;
    sqdest = game[(down == TRUE) ? ngamemvs+1 : ngamemvs].move.to;
    if (down)
      {
	BIT_ZERO(attackfr[color][sqdest]);
	BIT_ZERO(pmovefr[color][sqdest]);
      }
    else {
      BIT_ZERO(attackfr[color][sqorigin]);
      BIT_ZERO(pmovefr[color][sqorigin]);
      BIT_ZERO(attackfr[OPP(color)][sqdest]);
      BIT_ZERO(pmovefr[OPP(color)][sqdest]);
    }
    if (down)
      {
	genpiece(UPDATE,color,sqorigin); /* gen square to which we move */
      }
    else {
	genpiece(UPDATE,color,sqdest); /* gen square to which we move */
    }
    for (sq = A1; sq <= H8; sq++)
      {
	if (MEMBER(sq,attackto[color][sqorigin]))  /* gen sqs that attack */
	  {			                   /* origin */
	    BIT_ZERO(attackfr[color][sq]);
	    genpiece(UPDATE,color,sq);
	  }
	if (MEMBER(sq,attackto[OPP(color)][sqorigin])) /* gen sqs that */
	  {                                            /* attack origin */
	    BIT_ZERO(attackfr[color][sq]);
	    genpiece(UPDATE,OPP(color),sq);
	  }
	if (MEMBER(sq,attackto[color][sqdest]))   /* gen sqs that attack */
	  {                                       /* attack destination */
	    BIT_ZERO(attackfr[color][sq]);
	    genpiece(UPDATE,color,sq);
	  }
	if (MEMBER(sq,attackto[OPP(color)][sqdest])) /* gen sqs that attack */
	  {                                          /* destination */
	    BIT_ZERO(attackfr[color][sq]);
	    genpiece(UPDATE,OPP(color),sq);
	  }
      }
    /* Now regenerate all white pawns behind of and all black pawns
     * in front of piece.
     */
    if (sqorigin > 7 && MEMBER(sqorigin-8,pieces[WHITE][PAWN]))
      {
	genonepawn(WHITE,sqorigin-8);
      }
    if (sqdest > 7 && MEMBER(sqdest-8,pieces[WHITE][PAWN]))
      {
	genonepawn(WHITE,sqdest-8);
      }
    if (sqorigin < 56 && MEMBER(sqorigin+8,pieces[BLACK][PAWN]))
      {
	genonepawn(BLACK,sqorigin+8);
      }
    if (sqdest < 56 && MEMBER(sqdest+8,pieces[BLACK][PAWN]))
      {
	genonepawn(BLACK,sqdest+8);
      }
    translmap(color,attackfr,attackto);
    translmap(color,pmovefr,pmoveto);
}

genside(color)
{
    register i,k,sq;
    for (i = A1; i <= H8; i++)
      if (MEMBER(i,side[color]))
	{
	  BIT_ZERO(attackfr[color][i]);
	  genpiece(NOT_UPDATE,color,i);
	}
    genpawn(color);
    translmap(color,attackfr,attackto);
}

#define KSIDE 0
#define QSIDE 1

gencastle(color)
{
  register sq,sq2,region,legcas;
  MAP kregion[2][2];		/* Region of squares must be free */
  if (color == WHITE)		/* from enemy attack for castling */
    {
      kregion[KSIDE][LOBD] = e1|f1|g1;
      kregion[QSIDE][LOBD] = e1|d1|c1;
      kregion[KSIDE][HIBD] = 0;
      kregion[QSIDE][HIBD] = 0;
    }
  else {
      kregion[KSIDE][HIBD] = e8|f8|g8;
      kregion[QSIDE][HIBD] = e8|d8|c8;
      kregion[KSIDE][LOBD] = 0;
      kregion[QSIDE][LOBD] = 0;
  }
  for (region = KSIDE; region <= QSIDE; region++)
    {
      legcas = TRUE;		/* Assume legal to castle that side */
      for (sq = A1; sq <= H8; sq++)
	if (MEMBER(sq,kregion[region]))
	    if (NOT_ZERO(attackto[OPP(color)][sq]))
	    {
	      legcas = FALSE;
	      break;
	    }
      if (!legcas) printf("Castle illegal on %s %s-side\n",
			  color == WHITE ? "white" : "black",
			  region == KSIDE ? "king" : "queen");
    }
}

addlegals(color,sq,map,pc)
MAP *map;
{
    int nextsq, lastsq, i, gotone;
    switch(pc) {
    case ROOK: 
	gotone = FALSE;
	for (i = 0; i < 4; i++)
	{
	    nextsq = sq;
	    lastsq = nextsq;
	    nextsq = nextsq + rookdirs[i];
	    while (nextsq >= 0 && nextsq < 64)
	    {
	        if ((MEMBER(lastsq,cols[0]) && MEMBER(nextsq,cols[7])) ||
		    (MEMBER(lastsq,cols[7]) && MEMBER(nextsq,cols[0])))
		  break;
		  BIT_MSET(nextsq,sq,color,attackfr)
		if (!(MEMBER(nextsq,empty))) break;
		lastsq = nextsq;
		nextsq = nextsq + rookdirs[i];
	    }
	}
	break;
    case BISHOP:
	gotone = FALSE;
	for (i = 0; i < 4; i++)
	{
	    nextsq = sq;
	    lastsq = nextsq;
 	    nextsq = nextsq + bishdirs[i];
	    while (nextsq >= 0 && nextsq < 64)
	    {
	        if ((MEMBER(lastsq,cols[0]) && MEMBER(nextsq,cols[7])) ||
		    (MEMBER(lastsq,cols[7]) && MEMBER(nextsq,cols[0])))
		      break;
		  BIT_MSET(nextsq,sq,color,attackfr);
		if (!(MEMBER(nextsq,empty)))
		  break;
		lastsq = nextsq;
		nextsq = nextsq + bishdirs[i];
	    }
	}
	break;
    default:
	BIT_OR(map,attackfr[color][sq]);
        break;
    }
}

addplegals(color,map,mapfr,incr)
int color;
MAP *map;
unsigned int mapfr[NCOLORS][BPERW*WPERM][2];
int incr;
{
    register sq;
    for (sq = A1; sq <= H8; sq++)
	if (MEMBER(sq,map) && !(MEMBER(sq,cols[0]) &&
	    MEMBER((sq+incr),cols[7])) && !(MEMBER((sq+incr),cols[0]) &&
					    MEMBER(sq,cols[7])))
	    BIT_MSET(sq,(sq+incr),color,mapfr);
}

genonepawn(color,sq)
{
  register nxtsq,nxtnxtsq,leftsq,rightsq;
  register nxtsqi,nnsqi,leftsqi,rightsqi,prank2s,prank2e;
  MAP tempa[2],tempr[2];
  BIT_ZERO(pmovefr[color][sq]);
  BIT_ZERO(attackfr[color][sq]);
  BIT_ZERO(tempa);
  BIT_ZERO(tempr);
  if (color == WHITE)
    {
      nxtsq = sq + 8;
      nxtnxtsq = sq + 16;
      leftsq = sq + 7;
      rightsq = sq + 9;
      nxtsqi = -8;
      nnsqi = -16;
      leftsqi = -7;
      rightsqi = -9;
      prank2s = 8;
      prank2e = 15;
    }
  else {
      nxtsq = sq - 8;
      nxtnxtsq = sq - 16;
      leftsq = sq - 7;
      rightsq = sq - 9;
      nxtsqi = 8;
      nnsqi = 16;
      leftsqi = 7;
      rightsqi = 9;
      prank2s = 48;
      prank2e = 55;
    }
  if (MEMBER(nxtsq,empty))
    {
      BIT_S(nxtsq,tempr);
      addplegals(color,tempr,pmovefr,nxtsqi);
      if (sq >= prank2s && sq <= prank2e)
	if (MEMBER(nxtnxtsq,empty))
	  {
	    BIT_ZERO(tempr);
	    BIT_S(nxtnxtsq,tempr);
	    addplegals(color,tempr,pmovefr,nnsqi);
	  }
    }
  BIT_ZERO(tempa);
  BIT_S(leftsq,tempa);
  addplegals(color,tempa,attackfr,leftsqi);
  BIT_ZERO(tempa);
  BIT_S(rightsq,tempa);
  addplegals(color,tempa,attackfr,rightsqi);
}

genpawn(color)
{
    MAP temp[2];
    MAP temp2[2];
    register sq;
    for (sq = A1; sq <= H8; sq++)
      BIT_ZERO(pmovefr[color][sq]);
    switch (color) {
        case WHITE:
              BIT_LEFT(pieces[WHITE][PAWN],8,temp2);
	      BIT_AND(empty,temp2);
	      addplegals(color,temp2,pmovefr,-8);
	      BIT_AND(rows[2],temp2);
	      BIT_LEFT(temp2,8,temp2);
	      BIT_AND(empty,temp2);
	      addplegals(color,temp2,pmovefr,-16);
	      BIT_LEFT(pieces[WHITE][PAWN],7,temp2);
	      addplegals(color,temp2,attackfr,-7);
	      BIT_LEFT(pieces[WHITE][PAWN],9,temp2);
	      addplegals(color,temp2,attackfr,-9);
          break;
	case BLACK:
	      BIT_RIGHT(pieces[BLACK][PAWN],8,temp2);
	      BIT_AND(empty,temp2);
	      addplegals(color,temp2,pmovefr,8);
	      BIT_AND(rows[5],temp2);
	      BIT_RIGHT(temp2,8,temp2);
	      BIT_AND(empty,temp2);
	      addplegals(color,temp2,pmovefr,16);
	      BIT_RIGHT(pieces[BLACK][PAWN],7,temp2);
	      addplegals(color,temp2,attackfr,7);
	      BIT_RIGHT(pieces[BLACK][PAWN],9,temp2);
	      addplegals(color,temp2,attackfr,9);
	  break;
	default:
	  break;
	}
}


genking(color,sq)
{
    MAP temp[2];
    BIT_COPY(kingmvmaps[sq],temp);
    IFMAPNOTZERO(temp) addlegals(color,sq,temp,KING);
}

genknight(color,sq)
{
    MAP temp[2];
    BIT_COPY(knightmvmaps[sq],temp);
    IFMAPNOTZERO(temp) addlegals(color,sq,temp,KNIGHT);
}

genbishop(color,sq,pc)
{
    MAP temp[2];
    MAP leftdest[2], rightdest[2];
    int left,right,row,col;
    row = sq / 8 + 1;
    col = sq % 8 + 1;
    left =  col + row - 2;
    right = col + (7 - row);
    BIT_COPY(side[color],temp);
    BIT_AND(diagleft[left],temp);
    BIT_AND(diagright[right],temp);
    IFMAPNOTZERO(temp) addlegals(color,sq,temp,BISHOP);
}

genrook(color,sq,pc)
{
    MAP temp[2];
    MAP rowdest[2], coldest[2];
    int row, col;
    row = sq / 8 + 1;
    col = sq % 8 + 1;
    BIT_COPY(side[color],temp);
    BIT_AND(rows[row-1],temp);
    BIT_AND(cols[col-1],temp);
    IFMAPNOTZERO(temp) addlegals(color,sq,temp,ROOK);
}

gen()
{
    genside(WHITE);
    genside(BLACK);
    gencastle(WHITE);
    gencastle(BLACK);
}
