/* This file contains the command-parser for CHESS.
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
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/file.h>
#else
#include <time.h>
#include <winsock.h>
#include <fcntl.h>
#define F_OK 0
#define random rand
#define srandom srand
#endif

#include "gnuchess.h"

#define VERSION "GNU Chess 11.26.86\n\
  Copyright (C) 1986  Stuart Cracraft/Free Software Foundation, Inc.\n"
#define FSIZE 120
#define MAXTESTBD 10
#define fast_eval ((bd[TOMOVE].moved == WHITE) ? bd[WMAT].moved - bd[BMAT].moved : bd[BMAT].moved - bd[WMAT].moved)

FILE *hisf = NULL;
char *fgets();
/*
 * First 10 positions from Bratko/Kopec, "A Test for Comparison of
 * Human  and Computer Performance in Chess", Advances in Computer Chess 3,
 * Pergamon Press, 1982
 * These positions are used by the test-moves and test-search commands
 * for testing purposes.
 */
char *testbds[MAXTESTBD] = {
    "1k1r4pp1b1R23q2pp4p32B54Q3PPP2B22K5-",
    "3r1k24npp11ppr3pp6PP2PPPP11NR55K22R5+",
    "2q1rr1k3bbnnpp2p1pp12pPp3PpP1P1P11P2BNNP2BQ1PRK7R-",
    "rnbqkb1rp3pppp1p62ppP33N42P5PPP1QPPPR1B1KB1R+",
    "r1b2rk12q1b1ppp2ppn21p63QP31BN1B3PPP3PPR4RK1+",
    "2r3k1pppR1pp14p34P1P15P21P4K1P1P58+",
    "1nk1r1r1pp2n1pp4p3q2pPp1Nb1pP1P2B1P2R22P1B1PPR2Q2K1+",
    "4b3p3kp26p13pP2p2pP1P24K1P1P3N2P8+",
    "2kr1bnrpbpq42n1pp23p3p3P1P1B2N2N1QPPP3PP2KR1B1R+",
    "3rr1k1pp3pp11qn2np183p4PP1R1P22P1NQPPR1B3K1-"};

typedef struct
{
	char	*dptr;
	int	dsize;
} datum;

datum	fetch();

int gentype = MAILBOX, neither = FALSE;
#ifdef PARALLEL
int parallel = TRUE;
#endif PARALLEL
extern int iv;
extern int compare(),sorttype,myprocid;
extern long hashval;
extern struct timeval timcpu;
extern struct rusage rubuf;
struct mvlist scoredmvs[MAXMOVES];
char forsythe[80],bkmove[10];
int gotbook;
//long random();
datum key,posp;
int moveinbook[MAXMOVES],mbi;

int pcval[MAXPC+1] = {0, 100, 325, 350, 500, 900, 5000};

struct bdtype bd[120] = {
    OFF,9050,OFF,9050,OFF,WHITE,OFF,0,OFF,0,OFF,95,OFF,25,OFF,0,OFF,0,OFF,0,
    OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,
    OFF,0,BR, 0,BN, 0,BB, 0,BQ, 0,BK, 0,BB, 0,BN, 0,BR, 0,OFF,0,
    OFF,0,BP, 0,BP, 0,BP, 0,BP, 0,BP, 0,BP, 0,BP, 0,BP, 0,OFF,0,
    OFF,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,OFF,0,
    OFF,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,OFF,0,
    OFF,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,OFF,0,
    OFF,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,EMP,0,OFF,0,
    OFF,0,WP, 0,WP, 0,WP, 0,WP, 0,WP ,0,WP, 0,WP, 0,WP, 0,OFF,0,
    OFF,0,WR, 0,WN, 0,WB, 0,WQ, 0,WK, 0,WB, 0,WN, 0,WR, 0,OFF,0,
    OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,
    OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0,OFF,0};

struct bdtype startbd[120];

struct dirtypes pieces[MAXPC+6] = {
    2, FALSE, -9, -11, 0, 0, 0, 0, 0, 0,	/* black pawns */
    2, FALSE, 9, 11, 0, 0, 0, 0, 0, 0,          /* white pawns */
    8, FALSE, -19, -21, -8, -12, 12, 8, 21, 19, /* knights */
    4, TRUE,  -11, -9, 9, 11, 0,  0,  0,  0,	/* bishops */
    4, TRUE,  -10, -1, 1, 10, 0, 0, 0, 0,	/* rooks */
    8, TRUE, -11, -9, 9, 11, -10, -1, 1, 10,	/* queens */
    8, FALSE, -11, -9, 9, 11, -10, -1, 1, 10};	/* kings */

struct mvlist moves[MAXMOVES];
struct gmlist history[MAXGAME];
struct mvlist stack[MAXSTACK];

int nmoves = 0,
    nmovessco = 0,
    compat = 1,			/* 1 => turn on display-handling */
    cnt = 1,			/* 0 => otherwise don't */
    histtot,     		/* Total history of game */
    stacktot = -1,
    fnodestot = 0,
    snodestot = 0,
    treenodes = 0,
    termnodes = 0,
    bestmove = 0,
    bestscore = 0,
    bestfrom = 0,
    bestto = 0,
    maxdepth = 0;
char *histmp = "GAMES/chXXXXXX";

//long random();

/*
 * Zero_history wipes out all history.
 */
zero_history()
{
    int i;
    for (i = 0; i < MAXGAME; i++)
      history[i].depth = -1;
}

/*
 * read_history attempts to read a history file.
 */
read_history()
{
    int index, ply, scor, rate;
    long node;
    float cpu;
    char wmove[10],bmove[10],sqr[5];
    FILE *fp;
    char filename[MAXSTR];
    index = 0;
    printf("Name of history file? ");
    if (scanf("%s",filename) != EOF)
    {
	if ((fp = fopen(filename,"r")) == NULL)
	{
	    printf("Sorry, cannot open %s\n",filename);
	    return;
	}
	if (fscanf(fp,"  White Black Depth  Nodes  Score    Cpu     Rate") ==
	    EOF)
	{
	    printf("Bad history file %s. No game restored.\n",filename);
	    return;
	}
	histtot = 0;
	while (fscanf(fp,"%d. %s %s %d %d %d %f %d",
	    &index,wmove,bmove,&ply,&node,&scor,&cpu,&rate) != EOF)
	{
   	    index--;
	    history[index].depth = ply;
	    history[index].nodes = node;
	    history[index].score = scor;
	    history[index].cpu = cpu;
	    history[index].rate = rate;
	    history[index].wmove.flags = 0;
	    history[index].bmove.flags = 0;
	    sqr[0] = wmove[0];
	    sqr[1] = wmove[1];
	    sqr[2] = NULL;
	    history[index].wmove.from = alg_to_lin(sqr);
	    sqr[0] = wmove[2];
	    sqr[1] = wmove[3];
	    sqr[2] = NULL;
	    history[index].wmove.to = alg_to_lin(sqr);
	    history[index].wmove.movpiece = 
	      bd[history[index].wmove.from].piece;
	    history[index].wmove.cappiece = 
	      bd[history[index].wmove.to].piece;
	    if (history[index].wmove.movpiece == WK)
	      if (history[index].wmove.to == 97 &&
		  history[index].wmove.from == 95)
		history[index].wmove.flags |= KCASFLAG;
	      else if (history[index].wmove.to == 93 && 
		history[index].wmove.from == 95)
		history[index].wmove.flags |= QCASFLAG;
	    makemove(history[index].wmove,bd);
	    sqr[0] = bmove[0];
	    sqr[1] = bmove[1];
	    sqr[2] = NULL;
	    history[index].bmove.from = alg_to_lin(sqr);
	    sqr[0] = bmove[2];
	    sqr[1] = bmove[3];
	    sqr[2] = NULL;
	    history[index].bmove.to = alg_to_lin(sqr);
	    history[index].bmove.movpiece = 
	      bd[history[index].bmove.from].piece;
	    history[index].bmove.cappiece = 
	      bd[history[index].bmove.to].piece;
	    if (history[index].bmove.movpiece == BK)
	      if (history[index].bmove.to == 27 && history[index].bmove.from ==
		  25)
		history[index].bmove.flags |= KCASFLAG;
	      else if (history[index].bmove.to == 23 && 
		       history[index].bmove.from == 25)
		history[index].bmove.flags |= QCASFLAG;
	    makemove(history[index].bmove,bd);
	    histtot++;
	}
	if (index > 0)
	   printf("Game of %d moves retrieved\n",index+1);
	else
	   printf("Sorry, that is an empty game file\n");
	fclose(fp);
    }	      
    else
        printf("No file entered.\n");
}

/*
 * Write out a history file.
 */
write_history()
{
    int i,dstat,nzero;
    long nstat;
    float cstat;	
    nzero = dstat = nstat = 0;
    cstat = 0.0;
    unlink(histmp);
    mktemp(histmp);
    unlink(histmp);
    hisf = fopen(histmp,"w");
    if (hisf == NULL)
    {
        fprintf(stderr, "cannot open history temp file %s\n",histmp);
        return;
    }
    fprintf(hisf,"  White Black Depth  Nodes  Score    Cpu     Rate\n");
    for (i = 1; i <= histtot; i++)
        if (history[i].depth >= 0)
	  {
	    fprintf(hisf,"%d. ",i);
	    lin_to_alg(history[i].wmove.from,hisf);
	    lin_to_alg(history[i].wmove.to,hisf);
	    fprintf(hisf," ");
	    if (history[i].bmove.from != NULL)
	    {
		lin_to_alg(history[i].bmove.from,hisf);
		lin_to_alg(history[i].bmove.to,hisf);
	    }
	    if (history[i].nodes > 0)
	      {
		      dstat += history[i].depth;
		      nstat += history[i].nodes;
		      cstat += history[i].cpu;
	      }
	    else nzero++;
	    if (history[i].depth > 0)
		fprintf(hisf,"\t %2d %6d %6d %6.2f      %.0f\n",
		       history[i].depth,
		       history[i].nodes,
		       history[i].score,
		       (history[i].cpu < 0) ? 0.0 : history[i].cpu,
		       (history[i].nodes > 0) ? history[i].rate : 0);
	  }
        else break;
    if (histtot - nzero > 0)
      fprintf(hisf,"Averages: %5.2f ply, %.0f nodes, %5.2f cpu, %.0f nodes/sec\n",
	   (float)dstat / (float)((histtot) - nzero),
	   (float)nstat / (float)((histtot) - nzero),
	   (float)cstat / (float)((histtot) - nzero),
	   (float)(nstat / (float)((histtot) - nzero)) /
	   (float) (cstat / (float)((histtot) - nzero)));
    fclose(hisf);
}

/*
 * Print a history of the current game on the terminal.
 */
list_history()
{
    int i,dstat,nzero;
    long nstat;
    float cstat;	
    nzero = dstat = nstat = 0;
    cstat = 0.0;
    printf("  White Black Depth  Nodes  Score    Cpu     Rate\n");
    for (i = 1; i <= histtot; i++)
      {
        if (history[i].depth >= 0)
	  {
	    printf("%d. ",i);
            putchar(' ');
	    lin_to_alg(history[i].wmove.from,stdout);
	    lin_to_alg(history[i].wmove.to,stdout);
	    putchar(' ');
	    if (history[i].bmove.from != NULL)
	    {
	        lin_to_alg(history[i].bmove.from,stdout);
	        lin_to_alg(history[i].bmove.to,stdout);
	    }	
	    if (history[i].nodes > 0)
	      {
		      dstat += history[i].depth;
		      nstat += history[i].nodes;
		      cstat += history[i].cpu;
	      }
	    else nzero++;
    	    printf("\t %2d %6d %6d %6.2f      %.0f\n",
		       history[i].depth,
		       history[i].nodes,
		       history[i].score,
		       (history[i].cpu < 0) ? 0.0 : history[i].cpu,
		       (history[i].nodes > 0) ? history[i].rate : 0);
	  }
        else break;
    }
    if (histtot - nzero > 0)
      {
/*
      printf("histtot = %d, nzero = %d\n",
	     histtot, nzero);
*/
      printf("Averages: %5.2f ply, %.0f nodes, %5.2f cpu, %.0f nodes/sec\n",
	   (float)dstat / (float)((histtot) - nzero),
	   (float)nstat / (float)((histtot) - nzero),
	   (float)cstat / (float)((histtot) - nzero),
	   (float)(nstat / (float)((histtot) - nzero)) /
	   (float) (cstat / (float)((histtot) - nzero)));
       }
}

testmoves()
{
    int i,j;
    float tottim = 0.0,timused;
    printf("1000 move generations): ");
    fflush(stdout);
    timeon();
    for (i = 0; i < 1000; i++)
	generate(bd,moves);
    timeoff();
    timused = (float)timcpu.tv_sec+((float)timcpu.tv_usec/1000000.);
    printf("%.2f gens/sec\n",1000/timused);
    for (i = 0; i < MAXTESTBD; i++)
      {
	termnodes = treenodes = 0;
	clear_bd(bd);
	iboard(bd,testbds[i]);
	timeon();
	printf("Bd %d. ",i); fflush(stdout);
	for (j = 0; j < 500; j++)
	  generate(bd,moves);
	bd[TOMOVE].moved == OPPCOLOR(bd[TOMOVE].moved);
	for (j = 0; j < 500; j++)
	  generate(bd,moves);
	timeoff();
	bd[TOMOVE].moved == OPPCOLOR(bd[TOMOVE].moved);
	timused = (float)timcpu.tv_sec+
	  ((float)timcpu.tv_usec/1000000.);
	tottim  += 1000/timused;
	printf("generate rate = %.2f gens/sec\n",1000/timused);
	fflush(stdout);
      }
    printf("Average for all positions: %.2f\n",tottim/MAXTESTBD);
    fflush(stdout);
}

testsearch()
{
    int i,j,width;
    float timused;
    for (i = 0; i < MAXTESTBD; i++)
      {
	termnodes = treenodes = 0;
	clear_bd(bd);
	iboard(bd,testbds[i]);
#ifdef TRANSP
        init_hash();
	clear_hash();
#endif TRANSP
	timeon();
	bestscore = search(bd,MININT,MAXINT,maxdepth);
	timeoff();
	width = pps(bd,moves);
	sorttype = SCORESORT;
	qsort(moves,width,sizeof (struct mvlist), compare);
	printf("%d. My move: ",i+1,maxdepth);
	lin_to_alg(bestfrom,stdout);
	lin_to_alg(bestto,stdout);
	timused = (float)timcpu.tv_sec+
	  ((float)timcpu.tv_usec/1000000.);
	printf(" Nodes = %d/%d Score = %d Time = %.2f Rate = %.2f\n",
	       treenodes,termnodes,bestscore,timused,termnodes/timused);
	bestmove = searchlist(bestfrom,bestto,moves,width);
        makemove(moves[bestmove],bd);
      }
}  

help()
{
    printf("Legal commands are:\n");
    printf("A move such as: e2e4, g8f6, e1g1, etc.\n");
    printf("attack\t\t- determine if square is attacked by color\n");
    printf("bd\t\t- print regular board\n");
    printf("book\t\t- compile book into binary format\n");
    printf("cbd\t\t- print move count board\n");
    printf("depth\t\t- set depth of search\n");
    printf("enter\t\t- enter book played thus far into book\n");
    printf("first\t\t- computer should play white\n");
    printf("fill\t\t- fill board with new position\n");
    printf("history\t\t- print history of game\n");
    printf("historyf\t- print history of game to temp file\n");
    printf("legals\t\t- show legal moves for side to move\n");
    printf("neither\t\t- play neither side\n");
    printf("quit\t\t- exit program\n");
    printf("read\t\t- read a history file\n");
    printf("reset\t\t- set board to starting position\n");
    printf("score\t\t- statically evaluate current position\n");
    printf("self\t\t- play self\n");
    printf("static\t\t- show static exchange on a square\n");
    printf("switch\t\t- force computer to move\n");
    printf("test-moves\t- test move generator\n");
    printf("test-search\t- test search\n");
    printf("undo\t\t- retract last move\n");
}

main(argc,argv)
int argc;
char *argv[];
{
    register i,j,movei;
    int x;
    char bookfl1[80],bookfl2[80];
    char obd[MAXSTR],stdptr[MAXSTR];
    long mover;
    float timused;
    int index,score,depth,width,prevscore = 0,self = FALSE,seval,feval,
        toteval,totmoves,tempint,nsame,fd1,fd2;
    char cmd[20],frsq[3],tosq[3],tempstr[10];;

    init_rands();		/* Initialize random numbers */
    init_hash(bd);		/* And hash table */
    strcpy(bookfl1,BOOKOBJ);	/* Figure out names of opening book database */
    strcat(bookfl1,".dir");
    strcpy(bookfl2,BOOKOBJ);
    strcat(bookfl2,".pag");
    gotbook = FALSE;
    if (access(bookfl1,F_OK) >= 0 && access(bookfl2,F_OK) >= 0)	/* Exist? */
      {
          dbminit(BOOKOBJ);		/* Yes, open it */
          gotbook = TRUE;
      }
    else 
      {				/* Non-existant, create them */
	      close(creat(bookfl1,0600));
	      close(creat(bookfl2,0600));
      }    
#ifdef TRANSP
    clear_hash();
#endif
    zero_history();
    histtot = 0;
    nmoves = 0;
#ifdef TABLEGEN
    copybd(bd,startbd);
    fillpctab(startbd);
    fillpwntab(startbd);
    listtabs();			/* List move-gen tables */
#endif
    maxdepth = DEPTH;
#ifdef PARALLEL
    if (argc > 1)  /* Is this a parallel-processor process? */
    { /* must be invoked via "gnuchess -p <input-board> depth index len host" */
	myprocid = atoi(argv[2]);
        printf("processor %d starting...\n",myprocid);
	fflush(stdout);
	do_parallel();
	exit();
/*
        width = pps(bd,moves);
	if (width > 1)
        {
   	    maxdepth = atoi(argv[3]);
	    clear_bd(bd);
	    iboard(bd,argv[2]);
   	    single_parallel(bd,atoi(argv[3]),atoi(argv[4]),atoi(argv[5]),
			    argv[6]);
	    exit();
        }
*/
    }	      
    else {
#endif PARALLEL
        while (argc > 1 && argv[1][0] == '-') {
	    if (isdigit(argv[1][1]))
			maxdepth = argv[1][1] - '0';
	    else
		compat = 0;
	argv++;
	argc--;
      }
#ifdef PARALLEL
    }
#endif PARALLEL
    if (compat) {
	    printf("Chess\n");
#ifndef _WIN32
	    setlinebuf(stdout);
#endif
    }
    else
      printf(VERSION);
    depth = maxdepth;
    copybd(bd,startbd);
    for (;;)
    {
	if (self == FALSE)
	{
	    if (!compat)
	      putchar('_');
	    if (scanf("%s",cmd) == EOF)
	      strcpy(cmd,"quit");
	}
	else cmd[0] = NULL;
        if (strcmp(cmd,"bd") == 0)
	  pboard(bd,FALSE);
	else if (strcmp(cmd,"cbd") == 0)
	  {
	    printf("Command has been disabled.\n");
	    pboard(bd,TRUE);
	  }
	else if (strcmp(cmd,"book") == 0)
	  {
	      unlink(bookfl1);	/* Get rid of stray book */
	      unlink(bookfl2);
	      fd1 = open(bookfl1,O_CREAT|O_BINARY,0666); /* Create empty new book */
	      fd2 = open(bookfl2,O_CREAT|O_BINARY,0666);
	      close(fd1);
	      close(fd2);
	      book();		/* Do the actual creation of book */
	      init_rands();	/* Create random numbers */
	      init_hash(bd);	/* Initialize hash values */
	      dbminit(BOOKOBJ);	/* Initialize and open our new book */
	  }
	else if (strcmp(cmd,"enter") == 0) /* Enter current game in book */
	  {
	      write_book();
	  }
	else if (strcmp(cmd,"test-moves") == 0)
	    testmoves();
	else if (strcmp(cmd,"?") == 0)
	    help();
	else if (strcmp(cmd,"test-search") == 0)
	  testsearch();
	else if (strcmp(cmd,"legals") == 0)
	  {
	    printf("side? ");
	    scanf("%s",tempstr);
	    tempint = bd[TOMOVE].moved;
	    bd[TOMOVE].moved = ((strcmp(tempstr,"white") == 0) ? WHITE : 
				BLACK);
	    width = pps(bd,moves);
/*
	    sorttype = SCORESORT;
	    qsort(moves,width,sizeof (struct mvlist), compare);
*/
	    listmoves(moves,width);
	    bd[TOMOVE].moved = tempint;
	  }
	else if (strcmp(cmd,"read") == 0) {
	    read_history();
 	}
  	else if (strcmp(cmd,"reset") == 0) {
   	    iv = 0;		/* Initial value for aspiration */
	    for (i = 0; i < 120; i++)
	    {
		bd[i].piece = startbd[i].piece;
		bd[i].moved = startbd[i].moved;
	    }
#ifdef TRANSP
	    init_hash();
	    clear_hash();
#endif
	    histtot = 0;
	    nmoves = 0;
	}
        else if (strcmp(cmd,"static") == 0)
	  {
	    printf("sq? "); fflush(stdout);
	    scanf("%d",&tempint);
	    if (tempint != 0)
	      {
	        showstatic(bd,tempint);
		printf("total static for board = %d\n",modmat(bd));
	      }
	    else
	        printf("%d\n",modmat(bd));
	  }
	else if (strcmp(cmd,"attack") == 0)
	  {
	    printf("sq attacked by color? "); fflush(stdout);
	    scanf("%d %s",&tempint,tempstr);
	    if (sqattacked(bd,tempint,(strcmp(tempstr,"white") == 0) ?
			   WHITE : BLACK))
	      printf("Yes. Square %d is attacked by %s\n",tempint,tempstr);
	    else
	      printf("No. Square %d is not attacked by %s\n",tempint,tempstr);
	    showattacks(bd,tempint);
	  }
	else if (strcmp(cmd,"depth") == 0)
	  {
	    printf("depth (non-zero positive integer)? "); fflush(stdout);
	    scanf("%d",&tempint);
	    depth = maxdepth = tempint;
	  }
	else if (strcmp(cmd,"fill") == 0)
	  {
	    clear_bd(bd);
	    printf("Please input board in Forsythe notation.\n");
	    printf("Terminate with return.\n");
	    printf("Initial setup is rnbqkbnrpppppppp8888PPPPPPPPRNBQKBNR+\n");
	    fflush(stdout);
	    scanf("%s",forsythe);
	    iboard(bd,forsythe);
#ifdef TRANSP
	    init_hash();
	    clear_hash();
#endif
	    printf("WMAT = %d, BMAT = %d\n",
		   bd[WMAT].moved,bd[BMAT].moved);
	  }
	else if (strcmp(cmd,"history") == 0)
	    list_history();
	else if (strcmp(cmd,"historyf") == 0)
	    write_history();
	else if (strcmp(cmd,"list-hash") == 0)
#ifdef TRANSP
	    list_hash();
#else
	    printf("Hash table not installed presently.\n");
#endif
	else if (strcmp(cmd,"quit") == 0)
	{
/*
	    dbmclose();
*/
#ifdef PARALLEL
	    if (parallel)
		      quit_parallel();
#endif PARALLEL
	    break;
	}
        else if (strcmp(cmd,"increm") == 0)
	  {
	    printf("Command has been disabled.\n");
/*	    increm(bd,maxdepth);    */
	  }
#ifdef PARALLEL
	else if (strcmp(cmd,"parallel") == 0)
	  {
	      if (!parallel)
		{
			printf("enabling parallelism...\n");
			list_parallel();
			make_data();
			start_parallel();
			parallel = TRUE;
			myprocid = 2;
		}
	      
	      else 
		{
			printf("disabling parallelism...\n");
			parallel = FALSE;
		}
	}
#endif PARALLEL
	else if (strcmp(cmd,"score") == 0) {
	    feval = fast_eval;
	    printf("Evaluation is from point of view of player to move.\n");
	    printf("material = %d",feval);
	  }
	else if (strcmp(cmd,"undo") == 0)
		movem(moves,cmd,TRUE,bd);
	else if (strcmp(cmd,"remove") == 0)
	  {
		  movem(moves,cmd,TRUE,bd);
		  movem(moves,cmd,TRUE,bd);
	  }		  
	else if (strcmp(cmd,"self") == 0)
		self = TRUE;
	else if (strcmp(cmd,"neither") == 0)
	  neither = TRUE;
	else if (((strlen(cmd) == 4) && isalpha(cmd[0]) && isdigit(cmd[1]) &&
				       isalpha(cmd[2]) && isdigit(cmd[3]))
		|| (strcmp(cmd,"switch") == 0) || ((strcmp(cmd,"first") == 0)
		    && histtot == 0) || self)
	{
	        termnodes = treenodes = 0;
		if (strcmp(cmd,"switch") == 0 || strcmp(cmd,"first") == 0)
		  {
		      neither = FALSE;
	          }
		if (self || (strcmp(cmd,"switch")==0) || ((strcmp(cmd,"first") == 0) && histtot == 0) || movem(moves,cmd,FALSE,bd))
		{
		    write_history();
		    nmovessco = 0;
		    if (!neither) {
		      if (compat && strcmp(cmd,"first") != 0)
			{
		            if (bd[TOMOVE].moved == BLACK)
				printf("%d. %s\n", histtot, cmd);
	 	            else {
			        printf("%d. ... %s\n", histtot,cmd);
			    }
			}
#ifdef TRANSP
		      clear_hash();
#endif
		      width = pps(bd,moves);
/*
		      sorttype = SCORESORT;
		      qsort(moves,width,sizeof (struct mvlist), compare);
*/
/* uncomment this only if we want to see output of
   pps before each search
		      listmoves(moves,width);
*/
		      if (width > 1) /* Regular move */
		  	{
		            timeon();
/* Randomize across opening moves, otherwise pick first variation */
#ifdef RANDOM
			    mbi = 0;
			    if (gotbook)
			    {
			     for (movei = 0; movei < width; movei++)
			     {
			      makemove(moves[movei],bd);
			      key.dptr = (char *)&hashval;
			      key.dsize = 4;
			      posp = fetch(key);
/* match all moves that have the same position as well as 
 * the correct side on move
 */
/*
			      strcpy(stdptr,posp.dptr);
			      printf("stdptr = %s\n",stdptr);
			      fflush(stdout);
			      if (rindex(posp.dptr,"+"))
				printf("White move\n");
			      else printf("Black move\n");
			      fflush(stdout);
*/
			      if (posp.dptr != NULL)
/*
				  && ((stdptr[strlen(stdptr)-1] == '+') && bd[TOMOVE].moved == WHITE) ||
				   ((stdptr[strlen(stdptr)-1] == '-') && bd[TOMOVE].moved == BLACK))
*/
			      {
/*
				 printf("Move %d(%s)[%ld] in book: ",movei,
					posp.dptr,hashval);
				 lin_to_alg(moves[movei].from,stdout);
				 lin_to_alg(moves[movei].to,stdout);
				 putchar('\n');
*/
				 moveinbook[mbi++] = movei;
			       }
			       unmakemove(moves[movei],bd);
			    }
			    moveinbook[mbi] = -1;
/*
			    printf("A total of %d book moves available!\n",
				   mbi);
*/
			    if (mbi > 0)
			      {
/*
				printf("Book moves are available\n");
				for (mbi = 0; moveinbook[mbi] != -1; mbi++)
				{				
				    lin_to_alg(moves[moveinbook[mbi]].from,stdout);
				    lin_to_alg(moves[moveinbook[mbi]].to,stdout);
				    putchar(' ');
				}
*/
			        srandom(getpid()+(int)time(0));
				mover = random()%mbi;
				bestmove = moveinbook[mover];
				bestfrom = moves[bestmove].from;
				bestto = moves[bestmove].to;
/*
				putchar('\n');
*/
			      }
#ifdef PARALLEL
			    else if (!parallel)
#else
			    else
#endif PARALLEL
				bestscore = aspiration(bd,MININT,MAXINT,depth);
/*
				bestscore = search(bd,MININT,MAXINT,depth);
*/
#ifdef PARALLEL
			    else
				bestscore = multiple_parallel(bd,depth);
#endif PARALLEL
#else
			    key.dptr = (char *)&hashval;
			    key.dsize = 4;
			    posp = fetch(key);
			    if (posp.dptr == NULL)  /* Not in book */
/*
				bestscore = search(bd,MININT,MAXINT,depth);
*/
				bestscore = aspiration(bd,MININT,MAXINT,depth);
			    else if (posp.dptr != NULL) /* in book */
			    {
/*
				printf("Position in book!\n");
*/
				bestmove = searchlist(((10-(posp.dptr[1]-
					'0'))*10)+posp.dptr[0]-'a'+1,
			         ((10-(posp.dptr[3]-'0'))*10)+posp.dptr[2]-
					'a'+1,
				 moves,width);
			    }
#endif RANDOM
			   }
		            else {
#ifdef PARALLEL
				if (!parallel)
#endif PARALLEL
				bestscore = aspiration(bd,MININT,MAXINT,depth);
/*
				    bestscore = search(bd,MININT,MAXINT,depth);
*/
#ifdef PARALLEL
				else
				    bestscore = multiple_parallel(bd,depth);
#endif PARALLEL
			    }
			    timeoff();
			}
		      else if (width == 1)  /* Only one legal move */
			{
			    bestmove = 0;
			    bestfrom = moves[bestmove].from;
			    bestto = moves[bestmove].to;
		        }
		      else if (width == 0) /* Checkmate/Stalemate */
			{
			    if (sqattacked(bd,locate_king(bd),
				OPPCOLOR(bd[TOMOVE].moved)))
			      printf("%s wins\n",bd[TOMOVE].moved == WHITE ?
				   "Black" : "White");
			    else {
					printf("Stale mate\n");
					return;
					}
				return;
   			    continue;
			}
		      width = pps(bd,moves);
/*
		      sorttype = SCORESORT;
		      qsort(moves,width,sizeof (struct mvlist), compare);
*/
		      if (!compat)
 		      	    printf("My move: ");
		      else if (bd[TOMOVE].moved == BLACK)
 		      	    printf("%d. ... ", histtot);
		      else if (bd[TOMOVE].moved == WHITE)
			printf("%d. ",histtot);
		      bestmove = searchlist(bestfrom,bestto,moves,width);
		      lin_to_alg(moves[bestmove].from,stdout);
		      lin_to_alg(moves[bestmove].to,stdout);
		      putchar('\n');
/*
		      printf("Principal variation: ");
		      listpv();
*/
		      timused = (float)timcpu.tv_sec+
			((float)timcpu.tv_usec/1000000.);
		      if (!compat)
		        printf("Depth = %d, Nodes = %d/%d, Score = %d, Time = %.2f, Rate = %.2f\n",
			depth,treenodes,termnodes,bestscore,timused,termnodes/timused);
		      recordmove(moves[bestmove],depth,termnodes,bestscore,
				 timused,termnodes/timused,
				 bd[TOMOVE].moved == WHITE);
		      makemove(moves[bestmove],bd);
		      write_history();

	  pboard(bd,FALSE);

	             }
		}
	}
#ifdef XWINDOW
      else
        fprintf(stderr, "Illegal command\n");
#endif XWINDOW
    }
}
