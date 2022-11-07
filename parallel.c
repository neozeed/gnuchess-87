#ifdef PARALLEL
/* This file contains parallel-processing code for CHESS.
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
#include <sys/time.h>
#include <sys/file.h>
#include <sys/wait.h>
#include "gnuchess.h"

//#define DATAFILE "/usr/redwood/cracraft/parallel-data"
//#define LOCKFILE "/usr/redwood/cracraft/parallel-lock"

#define DATAFILE "parallel-data"
#define LOCKFILE "parallel-lock"


#define fast_eval ((bd[TOMOVE].moved == WHITE) ? bd[WMAT].moved - bd[BMAT].moved : bd[BMAT].moved - bd[WMAT].moved)

int bscoresofar,myprocid,nrfiles,nofiles;
extern int bestscore,bestfrom,bestto,maxdepth,compare(),sorttype,histtot,bestmove;
extern int snodestot,fnodestot,termnodes,treenodes;
extern struct mvlist game[MAXGAME];
extern struct bdtype bd[120];
FILE *fopen();
FILE *fp;

#define MAXPROC 7			/* Maximum processors we know of */

/* 
 * This is a structure containing all the processors we know about.
 * Information includes the network name, the processor type, whether
 * we use it for chess processing, and whether it shares the same disk
 * as the master control processor, which can be any of these.
 */
struct procstr {
    char *procname;		/* Processor name */
    char *type;			/* Processor type */
    int use;			/* Use it? (TRUE or FALSE) */
    int samedisk;		/* Share same disk? (TRUE OR FALSE) */
} procarray[MAXPROC] =
{
     "redwood", "SUN-3/180", TRUE, TRUE, 
     "sequoia", "SUN-3/50", TRUE, TRUE, 
     "toyon", "SUN-3/50", TRUE, TRUE,
     "redbud", "SUN-3/50", TRUE, TRUE,
     "ironwood", "SUN-3/50", TRUE, TRUE, 
     "buckeye", "SUN-3/50", TRUE, TRUE, 
     "madrone", "SUN-3/50", TRUE, TRUE
/*
     "juniper", "not-installed", FALSE, TRUE, 
     "mesquite", "not-installed", FALSE, TRUE,
     "sonia", "VAX-11/750", FALSE, FALSE, 
     "julia", "VAX-11/750", FALSE, FALSE, 
*/
};

/*
 * This is a structure containing the result of the parallel search
 * or work for the parallel search to do. If the former, 'from' and
 * 'to' will be filled in with the best move, nodes will contain
 * the number of nodes in the search, and score will be the value
 * of the search and worktodo will be false. If the latter, 'board'
 * will contain the position to make the work in and worktodo will be
 * true, and from/to will be null.
 */
struct procresults {
    char procname[20];		/* Processor name */
    char board[MAXSTR];		/* Board to search */
    int from;			/* Best from move */
    int to;			/* Best to move */
    int index;		      /* Index into move table */
    int len;			/* Number of moves to search */
    long nodes;			/* Nodes in search */
    int depth;			/* Depth to search */
    int score;			/* Score returned */
    int worktodo;		/* True if there is new work to do */
} result[MAXPROC];

#define NOMORE (strcmp(result[myprocid].board,"exit") == 0)
#define RMLOCK unlink(LOCKFILE)
#define RMDATA unlink(DATAFILE)
#define WORK (result[myprocid].worktodo && result[myprocid].from == 0)
#define MKLOCK close(creat(LOCKFILE,0600))
#define LOCKED (access(LOCKFILE,F_OK) >= 0)
#define GETDATA fopen(DATAFILE,"r")
#define GETWDATA fopen(DATAFILE,"a")

/*
 * psearch conducts an alphabeta search on the given board for
 * depth more ply with the supplied alpha and beta cutoff values,
 * starting at the mindex'th move on the top-level move-list
 * and doing a subset of size len of those top-level moves.
 * if mindex & len are 0, psearch does the entire search.
 */
int psearch(bd,alpha,beta,depth,mindex,len,hname)
struct bdtype bd[120];
int alpha,beta,depth;
int mindex,len;
char *hname;
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
    {
      width = pps(bd,moves);
/*
      printf("Host %s doing: ");
      for (i = mindex; i < mindex+len; i++)
	{
	      lin_to_alg(moves[i].from,stdout);
	      lin_to_alg(moves[i].to,stdout);
	      putchar(' ');
	}
      putchar('\n');
*/
/*
      sorttype = SCORESORT;
      qsort(moves,width,sizeof(struct mvlist),compare);
*/
    }
    else
      width = generate(bd,moves);
    if (mindex == 0 && len == 0)
      len = width;
    for (i = mindex; i < (mindex+len); i++)
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
	value = -psearch(bd,-beta,-alpha,depth,0,0,hname);
      else
	value = -quies(bd,-beta,-alpha,maxdepth);
#else
	value = -psearch(bd,-beta,-alpha,depth-1,0,0,hname);
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
/*
	      printf("Newbest(%s/%d): ",hname,depth);
	      lin_to_alg(moves[i].from,stdout);
	      lin_to_alg(moves[i].to,stdout);
	      putchar('\n');
*/
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
 * Do one leaf of a parallel-search.
 */
single_parallel(bd,depth,mindex,len,hname)
struct bdtype bd[120];
int depth,mindex,len;
char *hname;
{

/*
    printf("%s: doing single search of %d depth, mindex %d, len %d...\n",hname,depth,mindex,len);
    fflush(stdout);
*/
    if ((fp = fopen(hname,"a")) == NULL)
    {
	    printf("%s: cannot open data file\n",hname); fflush(stdout);
	    exit(1);
    }
    bestscore = psearch(bd,MININT,MAXINT,depth,mindex,len,hname);
    lin_to_alg(bestfrom,fp);
    lin_to_alg(bestto,fp);
    fprintf(fp," %d %d %s\n",bestscore,termnodes,hname);
/*
    printf("%s: done\n",hname);
*/
    fclose(fp);
}

/*
 * Do all leafs of a parallel-search.
 */
o_multiple_parallel(bd,depth)
struct bdtype bd[120];
{
    int i,nthproc,pid,width,len,mindex,totremaining;
    char cmdline[MAXSTR],obd[MAXSTR],dstr[10],istr[10],lstr[10],tstr[10];
    struct mvlist moves[MAXMOVES];
    oboard(bd,obd,MAXSTR);
    nthproc = 0;
    width = pps(bd,moves);
    totremaining = width;
    for (i = 0; i < MAXPROC; i++)
    {
	if (procarray[i].use)
	{
	    result[nthproc].worktodo = FALSE;
	    nthproc++;
	    len = (width/MAXPROC);
	    mindex = (nthproc-1) * len;
	    if (mindex + len > width)
		len = width - mindex;
/*
	    printf("Processor %s (%d,%d,%d)...\n",procarray[i].procname,
		depth,mindex,len);
	    fflush(stdout);
*/
	    sprintf(dstr,"%d",depth);
	    sprintf(istr,"%d",mindex);
	    sprintf(lstr,"%d",len);
	    sprintf(tstr,"%d",i);
	    pid = fork();
	    if (pid == 0)
            {
		execl("/usr/ucb/rsh","rsh", procarray[i].procname,
			   "in.pproc", "-p", obd, dstr, istr, lstr, 
			    procarray[i].procname,0);
		fprintf(stderr,"execl couldnt find program!\n");
		exit(1);
	    }
	}
    }
    wait_parallel();
}

int comp2();

/*
 * Wait for completion of parallel search
 */
wait_parallel()
{
    int found,i,nitems;
    char move[10];
    char fsq[4], tsq[4];
    char filename[30];
    long totnodes;
    union wait status;
    totnodes = 0;
    found = 0;
    for (i = 0; i < MAXPROC; i++)
	result[i].score = MININT;
    while (found < MAXPROC)
    {
	sleep(1);
	for (i = 0; i < MAXPROC; i++)
	{
	    if (procarray[i].use)
	    {
//		strcpy(filename,"/usr/redwood/cracraft/");
		strcpy(filename,"");
		strcat(filename,procarray[i].procname);
		if ((fp = fopen(filename,"r")) == NULL)
		{
		    continue;		/* Not here */
		}
		nitems = fscanf(fp,"%s%d%d%s",move,&result[i].score,
				&result[i].nodes,procarray[i].procname);
		if (nitems == 4)
		{
		    fsq[0] = move[0];     tsq[0] = move[2];
		    fsq[1] = move[1];     tsq[1] = move[3];
		    fsq[2] = NULL;        tsq[2] = NULL;
		    bestfrom = alg_to_lin(fsq);
		    bestto = alg_to_lin(tsq);
		    strcpy(result[i].procname,procarray[i].procname);
		    result[i].from = bestfrom;
		    result[i].to = bestto;
		    totnodes += result[i].nodes;
		    unlink(filename);
		    printf("%s %d %d ",
			   result[i].procname,
			   result[i].nodes,
			   result[i].score);
		    lin_to_alg(bestfrom,stdout);
		    lin_to_alg(bestto,stdout);
		    putchar('\n');
		    found++;
		}
		fclose(fp);
	    }
	}
    }
/*
    sorttype = SCORESORT;
    qsort(result,MAXPROC,sizeof(struct procresults),comp2);
*/
/*
    for (i = 0; i < MAXPROC; i++)
	if (result[i].score != MININT)
	{
	    printf("%s ",result[i].procname);
	    lin_to_alg(result[i].from,stdout);
	    lin_to_alg(result[i].to,stdout);
	    printf(" %d %d\n",result[i].score,result[i].nodes);
	}
*/
    bscoresofar = MININT;
    bestfrom = result[0].from;
    bestto = result[0].to;
    for (i = 0; i < MAXPROC; i++)
	if (result[i].score != MININT)
	  if (result[i].score > bscoresofar)
	    {
	          bestfrom = result[i].from;
		  bestto = result[i].to;
		  bscoresofar = result[i].score;
   	    }
    termnodes = totnodes;
    while (wait3(&status, WNOHANG, 0) > 0)
      ;
    return(bscoresofar);
}

/*
 * Comp2 is used for sorting the results.
 */
comp2(f1, f2)
register struct procresults *f1, *f2;
{
      if (sorttype == SCORESORT)
	{
		if (f1->score > f2->score)
			return(-1);
		if (f1->score < f2->score)
			return(1);
	}
	return(0);
}

/*
 * This starts the processors, putting them in an idle loop.
 */
start_parallel()
{
    int i,nthproc,pid,width,len,mindex,totremaining;
    struct mvlist moves[MAXMOVES];
    char tstr[10];
    nthproc = 0;
    for (i = 0; i < MAXPROC; i++)
    {
	if (procarray[i].use)
	{
	    nthproc++;
	    sprintf(tstr,"%d",i);
	    pid = fork();
	    if (pid == 0)
            {
		printf("doing rsh %s in.pproc -p %s\n",
		       procarray[i].procname,tstr);
		fflush(stdout);
		close(0);
		execl("/usr/ucb/rsh","rsh", procarray[i].procname,
		      "gnuchess", "-p", tstr, 0);
		fprintf(stderr,"execl couldnt find program!\n");
		exit(1);
	    }
	}
    }
}

/*
 * This lists the currently available processors.
 */
list_parallel()
{
    int i;
    for (i = 0; i < MAXPROC; i++)
	if (procarray[i].use)
	    printf("\t\t%s  - %s\n",
		procarray[i].procname,
		procarray[i].type);
}

read_data()
{
    FILE *fp;
    register i;
loopr:
    i = 0;
    if (access(LOCKFILE,F_OK) < 0)
    {
      MKLOCK;
      if ((fp = fopen(DATAFILE,"r")) != NULL)
	{
		while (i < MAXPROC)
		  {
			  fscanf(fp,"%s %s %d %d %d %d %ld %d %d %d\n",
				 result[i].procname,
				 result[i].board,
				 &result[i].from,
				 &result[i].to,
				 &result[i].index,
				 &result[i].len,
				 &result[i].nodes,
				 &result[i].depth,
				 &result[i].score,
				 &result[i].worktodo);
			  i++;
		  }
		fclose(fp);
		RMLOCK;
		return(TRUE);
	}
	RMLOCK;
     }
    sleep(1);
/*
    printf("Processor %s, waiting to read...\n",result[myprocid].procname);
*/
    goto loopr;
}

write_data()
{
    FILE *fp;
    register i;
loopw:
    i = 0;
    if (access(LOCKFILE,F_OK) < 0)
      {
	MKLOCK;
	RMDATA;
	if ((fp = fopen(DATAFILE,"a")) != NULL)
	{
		while (i < MAXPROC)
		  {
			  fprintf(fp,"%s %s %d %d %d %d %ld %d %d %d\n",
				  result[i].procname,
				  result[i].board[0] == NULL ? "empty" :
				      result[i].board,
				  result[i].from,
				  result[i].to,
				  result[i].index,
				  result[i].len,
				  result[i].nodes,
				  result[i].depth,
				  result[i].score,
				  result[i].worktodo);
			  i++;
		  }
		fclose(fp);
	  	RMLOCK;
		return(TRUE);
	}
	RMLOCK;
      }
    sleep(1);
    goto loopw;
}

zero_data()
{
    int i,j;
    for (i = 0; i < MAXPROC; i++)
    {
	strcpy(result[i].procname,procarray[i].procname);
	for (j = 0; j < MAXSTR; j++)
	    result[i].board[j] = NULL;
        result[i].from = 0;
        result[i].to = 0;
	result[i].nodes = 0;
	result[i].score = 0;
	result[i].index = 0;
	result[i].len = 0;
        result[i].worktodo = FALSE;
    }
}

/* 
 * Make_data creates a fresh data file
 */
make_data()
{
    RMLOCK;
    RMDATA;
    zero_data();
    write_data();
}

/*
 * work_parallel looks for work and does it
 */
work_parallel()
{
    if (!(LOCKED))
    {
	read_data();
	if (NOMORE)
	  exit(0);
	if (WORK)
	{
	    clear_bd(bd);
	    if (strcmp(result[myprocid].board,"empty") != 0)
	      iboard(bd,result[myprocid].board);
	    maxdepth = result[myprocid].depth;
	    printf("Processor %s starting search...\n",
		   result[myprocid].procname);
	    fflush(stdout);
	    bestscore = psearch(bd,MININT,MAXINT,result[myprocid].depth,
		 result[myprocid].index,result[myprocid].len,
				result[myprocid].procname);
	    while (LOCKED) sleep(1);
	    read_data();
	    result[myprocid].from = bestfrom;
	    result[myprocid].to = bestto;
	    result[myprocid].score = bestscore;
	    result[myprocid].nodes = termnodes;
	    termnodes = 0;
	    treenodes = 0;
	    write_data();
	}
    }
} 

/*
 * Tell leaf processors to exit upon next wakeup.
 */
quit_parallel()
{
	int i;
	if (access(DATAFILE,F_OK) >= 0)
	  {
		  read_data();
		  for (i = 0; i < MAXPROC; i++)
		    if (procarray[i].use)
		      strcpy(result[i].board,"exit");
		  write_data();
	  }
	
}

do_parallel()
{
     while (TRUE)
     {
         work_parallel();
	 sleep(1);
     }
}

multiple_parallel(bd,depth)
struct bdtype bd[120];
{
    int i,nthproc,pid,width,useid;
    char obd[MAXSTR];
    struct mvlist moves[MAXMOVES];
    width = pps(bd,moves);
    oboard(bd,obd,MAXSTR);
    read_data();
    for (i = 0; i < MAXPROC; i++)
    {
	    strcpy(result[i].board,obd);
	    result[i].len = (width/MAXPROC);
	    result[i].index = i * (width/MAXPROC);
	    if (result[i].index + result[i].len > width)
	      result[i].len = width - result[i].index;
	    result[i].depth = depth;
	    result[i].worktodo = TRUE;
	    result[i].from = result[i].to = 0;
    }
    write_data();
loop:
    sleep(1);
    read_data();
    for (i = 0; i < MAXPROC; i++)
      if (result[i].from == 0)
	{
/*
		printf("processor %s not finished\n",
		       result[i].procname);
*/
		goto loop;
	}
    bestfrom = result[0].from;
    bestto = result[0].to;
    bestscore = result[0].score;
    termnodes = result[0].nodes;
    for (i = 1; i < MAXPROC; i++)
      {
	      if (result[i].score > bestscore)
		{
			bestfrom = result[i].from;
			bestto = result[i].to;
			bestscore = result[i].score;
		}
	      termnodes += result[i].nodes;
      }
    return(bestscore);
}

#endif PARALLEL
