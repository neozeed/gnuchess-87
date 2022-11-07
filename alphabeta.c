/* This file contains the alphabeta search for CHESS.
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
#include "gnuchess.h"

#define in_check(bd) FALSE

#ifdef TREEPOS
#define fast_eval ((bd[TOMOVE].moved == WHITE) ? bd[WMAT].moved - bd[BMAT].moved + bd[WPOS].moved - bd[BPOS].moved : bd[BMAT].moved - bd[WMAT].moved + bd[BPOS].moved - bd[WPOS].moved)
#else
#define fast_eval ((bd[TOMOVE].moved == WHITE) ? bd[WMAT].moved - bd[BMAT].moved : bd[BMAT].moved - bd[WMAT].moved)
#endif TREEPOS

int iv = 0;
extern int stacktot,bestfrom,bestto,maxdepth,compare(),sorttype,histtot,bestmove;
extern int snodestot,fnodestot,termnodes,treenodes;
extern struct mvlist stack[MAXSTACK];
extern struct mvlist game[MAXGAME];
extern struct timeval timcpu;
struct pvstr pv[20];

/*
 * quies conducts a quiescence search on the given board for
 * depth more ply with the supplied alpha and beta cutoff values.
 * (cf. Thompson, Advances in Computer Chess 3, page 52.)
 */
#ifdef QUIES
int quies(bd,alpha,beta,depth)
struct bdtype bd[120];
int alpha,beta,depth;
{
    int i,width,value;
    struct mvlist moves[MAXMOVES];
    termnodes++;
    value = fast_eval;
    if (depth >= MAXQUIES)
	return(value);
    if (value >= alpha)
      {
	if (value >= beta)
	  return(beta);
	alpha = value;
      }
    width = generate(bd,moves);
    for (i = 0; i < width; i++)
      if (moves[i].flags & CAPFLAG)
	{
#ifdef SEARCHTREE
	  printf("\tMake: ");
	  if (moves[i].flags & NULLMV)
	    printf("null\n");
	  else {
   	      lin_to_alg(moves[i].from,stdout);
   	      lin_to_alg(moves[i].to,stdout);
	      putchar('\n');
	  }
#endif SEARCHTREE
          makemove(moves[i],bd);
          value = -quies(bd,-beta,-alpha,depth+1);
#ifdef SEARCHTREE
	  printf("\tUnmake: ");
	  if (moves[i].flags & NULLMV)
	    printf("null\n");
	  else {
  	      lin_to_alg(moves[i].from,stdout);
	      lin_to_alg(moves[i].to,stdout);
   	      putchar('\n');
	  }
#endif SEARCHTREE
          unmakemove(moves[i],bd);
	  if (value > alpha)		/* An improvement */
	    {
	      if (value >= beta)
		return(beta);
	      alpha = value;
	    }
	}
    return(alpha);
}
#endif QUIES

/*
 * search conducts an alphabeta search on the given board for
 * depth more ply with the supplied alpha and beta cutoff values.
 * (cf. Thompson, Advances in Computer Chess 3, page 52.)
 */
int search(bd,alpha,beta,depth)
struct bdtype bd[120];
int alpha,beta,depth;
{
    int i,width,value;
    struct mvlist moves[MAXMOVES];
    treenodes++;
#ifdef QUIES
    depth--;
    if (depth == (maxdepth-1))
#else
    if (depth <= 0)
    {
	termnodes++;
	treenodes--;
	return(fast_eval);
    }
    if (depth == maxdepth)
#endif QUIES
      width = pps(bd,moves);
    else
      width = generate(bd,moves);
    for (i = 0; i < width; i++)
    {
      makemove(moves[i],bd);
#ifdef SEARCHTREE
      printf("\tMake: ");
      lin_to_alg(moves[i].from,stdout);
      lin_to_alg(moves[i].to,stdout);
      putchar('\n');
#endif SEARCHTREE
#ifdef QUIES
      if (depth > 0)
	value = -search(bd,-beta,-alpha,depth);
      else
	value = -quies(bd,-beta,-alpha,maxdepth);
#else
	value = -search(bd,-beta,-alpha,depth-1);
#endif QUIES
#ifdef SEARCHTREE
      printf("\tUnmake: ");
      lin_to_alg(moves[i].from,stdout);
      lin_to_alg(moves[i].to,stdout);
      putchar('\n');
#endif SEARCHTREE
      unmakemove(moves[i],bd);
      if (value > alpha)		/* An improvement */
	{
#ifdef QUIES
	  if (depth == (maxdepth-1))
#else
	  if (depth == maxdepth)
#endif QUIES
	    {
#ifdef SEARCHTREE
	      printf("Newbest: ");
	      lin_to_alg(moves[i].from,stdout);
	      lin_to_alg(moves[i].to,stdout);
	      putchar('\n');
#endif SEARCHTREE
	      bestfrom = moves[i].from;
	      bestto = moves[i].to;
	      moves[i].score = value;
	    }
	  if (value >= beta)		/* A cutoff */
	    return(beta);
	  alpha = value;
	}
  }
  return(alpha);
}

/*
 * increm is an attempt at creating iterative deepening, which
 * is not generally useful with the overall design of this
 * program due to a total lack of positional knowledge at
 * terminal positions (by design).
 */
increm(bd,depth)
struct bdtype bd[120];
{
    int value,from,to,width,alpha,beta,index;
    float timused;
    struct mvlist moves[MAXMOVES];
    value = 0;
    width = pps(bd,moves);
    for (maxdepth = 1; maxdepth <= depth; maxdepth++)
      {
	printf("Ply %d: ",maxdepth); fflush(stdout);
	alpha = value - 100;
	beta = value + 100;
	fnodestot = snodestot = 0;
	timeon();
/*
	value = alphabeta(bd,moves,width,MININT,MAXINT,maxdepth);
	if (value >= beta)
	  value = alphabeta(bd,moves,width,value,MAXINT,maxdepth);
	else if (value <= alpha)
	  value = alphabeta(bd,moves,width,MININT,value,maxdepth);
*/
	timeoff();
	moves[bestmove].score += value;
	from = moves[bestmove].from;
	to = moves[bestmove].to;
	lin_to_alg(from,stdout);
	lin_to_alg(to,stdout);
        timused = (float)timcpu.tv_sec+((float)timcpu.tv_usec/1000000.);
	printf(". Slow = %d, Fast = %d, Score = %d, Time = %.2f, Rate = %.2f\n",
	       snodestot,fnodestot,value,timused,fnodestot/timused);
	sorttype = SCORESORT;
	qsort(moves,width,sizeof(struct mvlist),compare);
      }
    index = searchlist(from,to,moves,width);
    makemove(moves[index],bd);
}

/*
 * Aspiration implments aspiration search. This is a modifiction
 * of ordinary alpha-beta search such that the window is made
 * smaller, resulting in faster searches. When the returned value
 * is outside the search, a "failed search" is said to result
 * thus resulting in the need to search with a wider window.
 * (cf. "Parallel alpha-beta search on Arachne", Fishburn, J. &
 * Finkel, R., Tech. Report 394, Computer Science Department,
 * University of Wisconsin, Madison, Wis., July 1980 ; 
 * Parallel Search of Strongly Ordered Game Trees, Marsland, T.A. &
 * Campbell, M., Department of Computing Science, University of
 * Alberta, Edmonton, Canada T6G 2H1)
 */
aspiration(bd,alpha,beta,depth)
struct bdtype bd[120];
int alpha,beta,depth;
{
    int a,b,v;
    a = -100;			/* Create a window */
    b = 100;			/* Two pawn's wide */
    a = iv + a;			/* Around current expected score */
    b = iv + b;
    v = search(bd,a,b,depth);
    if (v >= b)
      v = search(bd,b,MAXINT,depth);
    else if (v <= a)
      v = search(bd,MININT,a,depth);
    iv = v;			/* Reset iv so next search will use it */
    return(v);
}

