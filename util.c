/* This file contains the random utility routines for CHESS.
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
#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/resource.h>
#include <ctype.h>
#include "gnuchess.h"

int sorttype;
struct rusage rubuf;
struct timeval timcpu;
extern FILE *hisf;
extern int nmoves,histtot;
#ifdef PARALLEL
extern int parallel;
#endif
extern long hashval;
extern int pcval[MAXPC+1];
extern struct pvstr pv[20];

char *pctype[(MAXPC*2)+1] =
	{ "BP", "BN", "BB", "BR", "BQ", "BK", "--", 
	  "WP", "WN", "WB", "WR", "WQ", "WK"};

/*
 * Clear_bd erases a board's contents.
 */
clear_bd(bd)
struct bdtype bd[120];
{
    int i;
    for (i = 21; i < 99; i++)
      if ((i % 10 != 0) && (i % 10 != 9))
	{
	  bd[i].piece = 0;
	  bd[i].moved = 0;
	}
    bd[WMAT].moved = 0;
    bd[BMAT].moved = 0;
    bd[TOMOVE].moved = WHITE;
}

/*
 * Copymvs copies a move list from one place to another.
 */
copymvs(oldmvs,newmvs,index)
struct mvlist oldmvs[MAXMOVES],newmvs[MAXMOVES];
int index;
{
	newmvs[index].from = oldmvs[index].from;
	newmvs[index].to = oldmvs[index].to;
	newmvs[index].propiece = oldmvs[index].propiece;
	newmvs[index].cappiece = oldmvs[index].cappiece;
	newmvs[index].movpiece = oldmvs[index].movpiece;
	newmvs[index].flags = oldmvs[index].flags;
	newmvs[index].score = oldmvs[index].score;
}

/*
 * Copybd copies a board from one place to another.
 */
copybd(oldbd,newbd)
struct bdtype oldbd[120],newbd[120];
{
    int i;
    for (i = 0; i < 120; i++)
    {
	newbd[i].piece = oldbd[i].piece;
	newbd[i].moved = oldbd[i].moved;
    }
}

/* 
 * lin_to_str is like lin_to_alg except that it prints to a string
 * variable.
 */
lin_to_str(sqfr,sqto,sp)
int sqfr,sqto;
char sp[];
{
   int row, col,row1,col1;
   row = 10 - sqfr / 10;
   col = sqfr - (sqfr / 10 * 10);
   row1 = 10 - sqto / 10;
   col1 = sqto - (sqto / 10 * 10);
   sprintf(sp,"%c%d%c%d\000",'a' + col - 1, row,'a' + col1 - 1, row1);
}     

/*
 * lin_to_alg converts a square to its algebraic equivalent.
 */
lin_to_alg(sq,fp)
int sq;
FILE *fp;
{
	int row, col;
	row = 10 - sq / 10;
	col = sq - (sq / 10 * 10);
	fprintf(fp,"%c%d",'a' + col - 1, row);
}

/*
 * alg_to_lin converts an algebraic square to its internal equivalent.
 * This is the reverse of lin_to_alg.
 */
int alg_to_lin(alg)
char *alg;
{
	char temp1,temp2;
	temp1 = *alg++;
	temp2 = *alg;
	return(((10-(temp2-'0'))*10)+temp1-'a'+1);
}

/*
 * listmoves takes a list of moves and prints them.
 */
listmoves(moves,nmoves)
struct mvlist moves[MAXMOVES];
int nmoves;
{
    int i;
    for (i = 0; i < nmoves; i++) {
      {
   	    lin_to_alg(moves[i].from,stdout);
	    if (moves[i].flags & CAPFLAG)
		putchar(':');	
	    lin_to_alg(moves[i].to,stdout);
	    if (moves[i].flags & PROMFLAG)
		printf("%s",
 			(moves[i].propiece < 0) ?
 			pctype[abs(moves[i].propiece)-1] :
 			pctype[moves[i].propiece+6]);
	}
   	printf(" = %d",-moves[i].score);
	if ((i + 1) % 7 == 0)
		putchar('\n');
	else
		putchar(' ');
    }
    putchar('\n');
}

#define error(str) {printf(str); return;}

/*
 * Convert a Forsythe-notation position into internal format.
 * Useful for reading in arbitrary positions.
 * A Forsythe notation for the opening position is:
 * rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR+
 * capital letters indicate white, lowercase indicate black.
 * a trailing plus means white to move, a trailing minus means black to move
 */
iboard(bd,bdstr)
struct bdtype bd[120];
char *bdstr;
{
    int i,j,wmat,bmat;
    char *p;
    histtot = OPENING + 1;
    p = bdstr;
    i = 21;
    bd[TOMOVE].moved = WHITE;
    bd[WMAT].moved = bd[BMAT].moved = 0;
    while ((isalpha(*p) || isdigit(*p)) && i <= 98)
      {
	if (isdigit(*p))
	  i += *p - '0' - 1;
	else switch(*p) {
	  case 'R': bd[i].piece = WR;
		    bd[WMAT].moved += pcval[WR];
		    break;
	  case 'r': bd[i].piece = BR;
		    bd[BMAT].moved += pcval[abs(BR)];
		    break;
	  case 'N': bd[i].piece = WN;
		    bd[WMAT].moved += pcval[WN];
		    break;
	  case 'n': bd[i].piece = BN;
		    bd[BMAT].moved += pcval[abs(BN)];
		    break;
	  case 'B': bd[i].piece = WB;
		    bd[WMAT].moved += pcval[WB];
		    break;
	  case 'b': bd[i].piece = BB;
		    bd[BMAT].moved += pcval[abs(BB)];
		    break;
	  case 'Q': bd[i].piece = WQ;
		    bd[WMAT].moved += pcval[WQ];
		    break;
	  case 'q': bd[i].piece = BQ;
		    bd[BMAT].moved += pcval[abs(BQ)];
		    break;
	  case 'K': bd[i].piece = WK;
		    bd[WMAT].moved += pcval[WK];
		    bd[WKING].moved = i;
		    break;
	  case 'k': bd[i].piece = BK;
		    bd[BMAT].moved += pcval[abs(BK)];
		    bd[BKING].moved = i;
		    break;
	  case 'P': bd[i].piece = WP;
		    bd[WMAT].moved += pcval[WP];
		    break;
	  case 'p': bd[i].piece = BP;
		    bd[BMAT].moved += pcval[abs(BP)];
		    break;
	  default:  error("Bad character in input board.\n");
		    break;
		  }
	i++;
	if (i % 10 == 9) i += 2;
	*p++;
      }
    if (*p == '+')
        bd[TOMOVE].moved = WHITE;
    else if (*p == '-')
        bd[TOMOVE].moved = BLACK;
}

/*
 * Converts the current position to Forsythe notation.
 * Useful for saving arbitrary positions. Also used by
 * opening book hash code.
 */
char pcfor[13] = {'k','q','r','b','n','p',NULL,'P','N','B','R','Q','K'};

oboard(bd,str,leng)
struct bdtype bd[120];
char str[];
int leng;
{
    register int i,nblank;
    char tchar[5];
    char *p;
    nblank = 0;
    for (i = 0; i < leng; i++) str[i] = NULL;
    for (i = 21; i < 99; i++)	/* For all squares on board */
      {
	if (bd[i].piece == OFF)
	  {
	      if (i - ((i / 10)*10) == 9) /* Report empty squares */
		{		          /* at end of each row */
		    if (nblank != 0)
		      {
			      sprintf(tchar,"%d",nblank);
			      strcat(str,tchar);
			      nblank = 0;
	              }
	        }
          }
	else if (bd[i].piece == EMP)	/* Empty contiguous squares are */
       					/* rolled into one number */
	    nblank++;
	else {		/* Report blanks if any, otherwise piece type */
	    if (nblank != 0)
	    {
		sprintf(tchar,"%d",nblank);
		strcat(str,tchar);
		nblank = 0;
	    }
	    sprintf(tchar,"%c",pcfor[bd[i].piece+6]);
	    strcat(str,tchar);
	}
      }
	/* Report who to move */
      sprintf(tchar,"%c",(bd[TOMOVE].moved == WHITE) ? '+' : '-');
      strcat(str,tchar);
}

/*
 * Print the board. If countbd is TRUE, print the board, but
 * instead of pieces, display the move count for each piece
 * on its corresponding square.
 */
pboard(bd,countbd)
struct bdtype bd[120];
{
	int i,sqwhite;
	sqwhite = 0;
	for (i = 21; i < 99; i++)
	{
	    if (bd[i].piece == 0)
		printf("%s ",(sqwhite % 2 == 0) ? "--" : "**");
	    else if (bd[i].piece != OFF) {
	      if (!countbd)
		printf("%s ",(bd[i].piece < 0) ? 
		       pctype[abs(bd[i].piece) - 1] :
		       pctype[bd[i].piece + 6]);
	      else
		printf("%2d ",bd[i].moved);
	    }
	    if ((i - (i / 10 * 10)) == 8)
	    {
		putchar('\n');
		sqwhite++;
	    }
            sqwhite++;	    
	}
/*
	printf("Material (%d, %d); Positional (%d, %d)\n",
	       bd[WMAT].moved,bd[BMAT].moved,bd[WPOS].moved,bd[BPOS].moved);
*/
	if (bd[TOMOVE].moved == WHITE) 
	  printf("White to move. ");
	else
	  printf("Black to move. ");
	putchar('\n');
}

/*
 * Searchlist, given a from square and a tosquare, along with a movelist
 * and a count of the number of moves on that movelist, returns an index
 * into the movelist representing the move that corresponds to the from/to
 * square combination.
 */
int searchlist(fromsq,tosq,moves,nmoves)
int fromsq,tosq;
struct mvlist moves[MAXMOVES];
int nmoves;
{
    int i;
    for (i = 0; i < nmoves; i++)
    {
	if ((fromsq == moves[i].from) && (tosq == moves[i].to))
		return(i);
    }
    return(NOMATCH);
}

/*
 * Compare is used by the system quick-sort routine in order
 * to compare two values, in this case a pair of moves, and
 * either for a capture sort or a score sort, used in the
 * tree search.
 */
compare(f1, f2)
	register struct mvlist *f1, *f2;
{
	if (sorttype == CAPSORT)
	{
		if (f1->flags&CAPFLAG > f2->flags&CAPFLAG)
			return(-1);
		if (f1->flags&CAPFLAG < f2->flags&CAPFLAG)
			return(1);
	}
	else if (sorttype == SCORESORT)
	{
		if (f1->score > f2->score)
			return(1);
		if (f1->score < f2->score)
			return(-1);
	}
	return(0);
}

long time();

/*
 * Timeon turns on a timer.
 */
timeon()
{
#ifdef PARALLEL
    if (parallel)
      {
	      timcpu.tv_sec = time(0);
              timcpu.tv_usec = 0;
      }
    else 
      {
#endif PARALLEL
	      getrusage(RUSAGE_SELF,&rubuf);
	      timcpu.tv_sec = rubuf.ru_utime.tv_sec;
	      timcpu.tv_usec = rubuf.ru_stime.tv_usec;
#ifdef PARALLEL
      }
#endif PARALLEL
}

/*
 * Timeoff turns off a timer.
 */
timeoff()
{
#ifdef PARALLEL
    if (parallel)
    {
	timcpu.tv_sec = time(0) - timcpu.tv_sec;
	timcpu.tv_usec = 0;
    }
    else
    {
#endif PARALLEL
	    getrusage(RUSAGE_SELF,&rubuf);
	    timcpu.tv_sec = rubuf.ru_utime.tv_sec - timcpu.tv_sec;
	    timcpu.tv_usec = rubuf.ru_utime.tv_usec - timcpu.tv_usec;
#ifdef PARALLEL
    }
#endif PARALLEL
}

/* 
 * Listpv lists a principal variation.
 */
listpv()
{
    int i;
    for (i = 0; i < 20; i++)
	if (pv[i].from != 0)
	{
	    lin_to_alg(pv[i].from,stdout);
	    lin_to_alg(pv[i].to,stdout);
	    putchar(' ');
	}
    putchar('\n');
}

/*
 * Locate_king, given a board, returns the square on which the
 * side-to-move's king resides.
 */
locate_king(bd)
struct bdtype bd[120];
{
    register i, ourkloc;
    for (i = 21; i < 99; i++)
      {
          if (abs(bd[i].piece) == WK && 
	      (COLOR(bd[i].piece) == bd[TOMOVE].moved))
	    {
	        ourkloc = i;    /* Store location of king for side-to-move */
		break;
    	    }
      }
    return(ourkloc);
}
