/* This file contains static-evaluation routines for CHESS.
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

int pcval[NPIECES] = {100,325,350,500,900,0};
int sideval[NCOLORS];
extern char *strpiece[NPIECES];
extern char *sqchar[];
extern unsigned int sqbits[];
extern int okprint;

float evalpawn(color)
{
}

float evalbishop(color)
{
    unsigned int temp[2],temp2[2];
    register sq;
    float score;
    score = 0.0;
    for (sq = A1; sq <= H8; sq++)
      if (MEMBER(sq,pieces[color][BISHOP]))
	 {
	   BIT_COPY(attackfr[color][sq],temp);
	   BIT_NOT(pieces[color][PAWN],temp2);
	   BIT_AND(temp2,temp);
	   /* Square control */
	   /* 2.4 for each square attacked not including friendly pawns - 7 */
	   /* Development -11 if on back rank */
	   score += (2.4 * (count_bits(temp) - 7)) +
		    (MEMBER(sq,rows[color == WHITE ? 0 : 7]) ? -11.0 : 0.0);

	}
    return(score);
}

float evalrook(color,krow,kcol)
{
  register sq,sq2;
  float score;
  MAP temp[2],temp2[2];
  int prow,pcol;
  score = 0.0;
  for (sq = A1; sq <= H8; sq++)
    if (MEMBER(sq,pieces[color][ROOK]))
      {
	setrowcol(sq,&prow,&pcol);
	/* King tropism */
	/* -1.6 * minimum of vert/horiz dist from enemy king */
	/* Square control */
	/* 1.6 for each square attacked */
	score += (-1.6 * MIN(ABS(prow-krow),ABS(pcol-kcol))) +
	         (1.6 * count_bits(attackfr[color][sq]));
	/* 8 for each doubled rook */
	for (sq2 = A1; sq2 <= H8; sq2++)
	  if (MEMBER(sq2,attackfr[color][sq]))
	    if (MEMBER(sq2,pieces[color][ROOK]))
	      score += 8;
	/* 8 for each open file */
	BIT_COPY(cols[pcol-1],temp);
	BIT_COPY(pieces[color][PAWN],temp2);
	BIT_OR(pieces[OPP(color)][PAWN],temp2);
	BIT_AND(temp2,temp);
	if (!(NOT_ZERO(temp))) score += 8;
	/* 22 for being on 7th rank */
	score += (MEMBER(sq,rows[color == WHITE ? 7 : 0])) ? 22 : 0;
	/* 3 pt. bonus for rook being on semi-open file */
	/* (e.g. one with no friendly pawns, at least one enemy pawn */
	/* that is not defended by other enemy pawns, and if advanced */
	/* would be subject to attack by at least one of our pawns */
      }
  return(score);
}

float evalqueen(color,krow,kcol)
{
  register sq,sq2;
  float score;
  int prow,pcol;
  score = 0.0;
  for (sq = A1; sq <= H8; sq++)
    if (MEMBER(sq,pieces[color][QUEEN]))
      {
	setrowcol(sq,&prow,&pcol);
	/* King tropism */
	/* -0.8 * minimum of vert/horiz distances from enemy king */
	score += -0.8 * MIN(ABS(prow-krow),ABS(pcol-kcol));
	/* Square control */
	/* 0.8 for each square attacked not attacked by enemy */
	for (sq2 = A1; sq2 <= H8; sq2++)
	  if (MEMBER(sq2,attackfr[color][sq]))
	    if (!(NOT_ZERO(attackto[OPP(color)][sq2])))
		score += 0.8;
      }
  return(score);
}

float evalknight(color,krow,kcol)
{
    register sq;
    float score;
    int prow,pcol;
    score = 0.0;
    for (sq = A1; sq <= H8; sq++)
      if (MEMBER(sq,pieces[color][KNIGHT]))
	{
	  setrowcol(sq,&prow,&pcol);
	  /* Center tropism, king tropism, development */
	  score += (1.6 * (6. - (2. * (ABS(4.5 - prow) + ABS(4.5 - pcol))))) +
	           (1.2 * (5. - (ABS(krow-prow)+ABS(kcol-pcol)))) +
		     (MEMBER(sq,rows[color == WHITE ? 0 : 7]) ? -9.4 : 0.);
	}	  
    return((float)score);
}

eval(show,color,mat,pos)
int show,color;
float *mat,*pos;
{
    int npieces[NCOLORS][NPIECES];
    MAP temp[2];
    register i,j,sidewin,pa,mt,md,sq,kingloc,kingeloc,foundsafesq,nsqattacked;
    register nonpenemy,qpresent,importance,rowtouse,psidefiles,psamefile,
      nadj;
    int krow,kcol,ekrow,ekcol,prow,pcol;
    float matadv,ms,kingsafe,notcorner,cramp,noneighbor,noponfles,attacksq,
        pawneval,knighteval,bishopeval,rookeval,queeneval,toteval;

    okprint = FALSE;
    i = j = sidewin = pa = mt = md = sq = kingloc = 0;
    kingeloc = foundsafesq = nsqattacked = 0;
    nonpenemy = qpresent = importance = rowtouse = 0;
    psidefiles = psamefile = nadj = 0;
    krow = kcol = ekrow = ekcol = prow = pcol = 0;
    matadv = ms = kingsafe = notcorner = cramp = 0.0;
    noneighbor = noponfles = attacksq = 0.0;
    pawneval = knighteval = bishopeval = rookeval = 0.0;
    queeneval = toteval = 0.0;

    sideval[WHITE] = 0;
    sideval[BLACK] = 0;
    for (i = 0; i < NPIECES; i++)
      for (j = 0; j < NCOLORS; j++)
	{
	  npieces[j][i] = count_bits(pieces[j][i]);
	  sideval[j] += npieces[j][i]*pcval[i];
#ifdef PRINT_EVAL
	  printf("Number of %s %ss = %d\n",
		 j == WHITE ? "white" : "black",
		 strpiece[i],
		 npieces[j][i]);
#endif PRINT_EVAL
	}
#ifdef PRINT_EVAL
    for (j = 0; j < NCOLORS; j++)
      printf("Material for %s = %d\n",
	     j == WHITE ? "white" : "black",
	     sideval[j]);
#endif PRINT_EVAL
    sidewin = (sideval[WHITE] >= sideval[BLACK] ? 
	       WHITE : BLACK);
#ifdef PRINT_EVAL
    printf("Winning side is %s\n",sidewin == WHITE ? "white" : "black");
#endif PRINT_EVAL
    md = sideval[sidewin] - sideval[OPP(sidewin)];
    pa = npieces[sidewin][PAWN];
    mt = sideval[sidewin] + sideval[OPP(sidewin)];
    ms = MIN(2400,md) + md*pa*(8000-mt)/(6400*(pa+1));
    matadv = MIN(3100,ms);

/* Do king safety */
/* First calculate importance */
    
    nonpenemy = count_bits(side[OPP(color)]) - npieces[OPP(color)][PAWN];
    qpresent = npieces[OPP(color)][QUEEN] != 0 ? 2 : 0;
    importance = nonpenemy + qpresent - 2;
    if (importance <= 0)
      {
	printf("King safety ignored.\n");
	return;
      }

/* Now calculate sanctuary term */

    BIT_COPY(pieces[color][KING],temp);
    BIT_AND(sanctuary,temp);
    notcorner = NOT_ZERO(temp) ? 0.0 : 3.1;

/* Now find king location and store its coordinates */

    for (sq = A1; sq <= H8; sq++)
      if (MEMBER(sq,pieces[color][KING]))
	  kingloc = sq;
      else if (MEMBER(sq,pieces[OPP(color)][KING]))
	  kingeloc = sq;
    setrowcol(kingloc,&krow,&kcol);
    setrowcol(kingeloc,&ekrow,&ekcol);

/* Now figure out whether king is cramped */

    foundsafesq = FALSE;
    if (color == BLACK)
      {
	BIT_AND(rows[7],temp);
      }
    else
      {
	BIT_AND(rows[0],temp);
      }
    IFMAPNOTZERO(temp)
      {
	BIT_COPY(kingmvmaps[kingloc],temp);
        rowtouse = (color == BLACK) ? 6 : 1;
	BIT_AND(rows[rowtouse],temp);
	for (sq = A1; sq <= H8; sq++)
	  if (MEMBER(sq,temp))
	    if ((attackto[color][sq][LOBD] == 0 &&
		 attackto[color][sq][HIBD] == 0) ||
		!(MEMBER(sq,pieces[color][PAWN])))
	      {
		foundsafesq = TRUE;
		break;
	      }
	if (!foundsafesq)
	  cramp = 0.8;
      }      

/* Figure out number of adjacent pieces and pawns. */

    BIT_COPY(kingmvmaps[kingloc],temp);
    BIT_AND(side[color],temp);
    nadj = count_bits(temp);
    if (nadj == 1) {
      printf("Only one pawn/piece next to king.\n");
      noneighbor = 1.6;
    }
    else if (nadj == 0) {
      printf("No adjacent pawn/piece next to king.\n");
      noneighbor = 3.1;
    }

/* Figure out number of open files in front of king. */

    BIT_COPY(cols[kingloc % 8],temp);
    BIT_AND(pieces[color][PAWN],temp);
    psamefile = count_bits(temp);
    if (psamefile == 0) {
      noponfles = 4.1;
    }
    if (kingloc % 8 > 0) {
      BIT_COPY(cols[(kingloc % 8) - 1],temp);
      BIT_AND(pieces[color][PAWN],temp);
      psidefiles = count_bits(temp);
    }
    if (kingloc % 8 < 7) {
      BIT_COPY(cols[(kingloc % 8) + 1],temp);
      BIT_AND(pieces[color][PAWN],temp);
      psidefiles += count_bits(temp);
    }
    if (psidefiles < 1)
      noponfles += 3.6;

/* Now penalize attacked squares adjacent to the king */

/*
    printf("The following squares adjacent to the king are attacked: ");
*/
    BIT_COPY(kingmvmaps[kingloc],temp);
    for (sq = A1; sq <= H8; sq++)
      if (MEMBER(sq,temp))
	if (NOT_ZERO(attackto[OPP(color)][sq]))
	  {
/*
	    printf("%s ",sqchar[sq]);
*/
	    nsqattacked++;
	  }	    
/*
    if (nsqattacked == 0) printf("none");
    putchar('\n');
*/
    if (nsqattacked >= 2)
      attacksq = (nsqattacked - 1) * 1.2;

/* Should add king in endgame evaluation stuff here (pp 99 of Frey, 2nd
   and 3rd paragraphs from bottom */

/* Finally add up all terms and multiply by importance */

    kingsafe = -(importance*(notcorner+cramp+noneighbor+noponfles+attacksq));

/* Still need to add the following. Penalize king by
 *    7.8 * (ATPD - 6) where ATPD is the average of the
 *    taxicab distances of the king to each pawn. The
 *    taxicab distance is the sum of the rank and file distances
 *    to the pawn as if the king could move only horizontally or
 *    vertically. Only when there is a certain small amount
 *    of material (see Frey).
    npawn = 0;
    tottaxi = 0;    
    for (sq = A1; sq <= H8; sq++)
      if (MEMBER(sq,pieces[color][PAWN]) || 
	  MEMBER(sq,pieces[OPP(color)][PAWN]))
	{
	  setrowcol(sq,&prow,&pcol);
	  tottaxi = prow + pcol;
	  npawn++
	}
    score -= 7.8*((tottaxi/npawn) - 6);
 */

    pawneval = evalpawn(color);
    knighteval = evalknight(color,ekrow,ekcol);
    bishopeval = evalbishop(color);
    rookeval = evalrook(color,ekrow,ekcol);
    queeneval = evalqueen(color,ekrow,ekcol);
    matadv = (sidewin == color) ? matadv : -matadv;
    *mat = matadv;
    *pos = kingsafe+pawneval+knighteval+bishopeval+rookeval+queeneval;
    if (show)
      {
	printf("Total evaluation for %s = \n",
	       color == WHITE ? "white":"black");
	printf("Material(%.2f) + KingSafety(%.2f) + Pawns(%.2f) + Knights(%.2f) +\n\tBishops(%.2f) + Rooks(%.2f) + Queens(%.2f)\n",
	     matadv,kingsafe,pawneval,knighteval,bishopeval,rookeval,
	     queeneval);
      }
}

evalpos(show,tomove,mat,pos)
{
  float mat,pos;
  float evaldiff;
  eval(show,OPP(tomove),&mat,&pos);
  evaldiff = (-1)*pos;
  eval(show,tomove,&mat,&pos);
  evaldiff += mat + pos;
  if (evaldiff == 0.0)
    printf("Position is even.\n");
  else
    printf("%s is %s %5.2f pawns\n",
	   tomove == WHITE ? "white":"black",
	   evaldiff > 0.0 ? "ahead":"behind",
	   ABS(evaldiff/100.0));
}
