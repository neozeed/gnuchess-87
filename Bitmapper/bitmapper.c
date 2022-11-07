/* This file contains the main entry-point for CHESS.
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
#include "bitmacs.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "movelist.h"

extern struct timeval timcpu;
extern struct rusage rubuf;
extern int ngamemvs;
extern struct gmlist game[MAXGAME];
int gfrsq,gtosq,gmover,gpc;

MAP pieces[NCOLORS][NPIECES][WPERM];
MAP side[NCOLORS][WPERM];
MAP empty[WPERM];
MAP edge[WPERM];
MAP sanctuary[WPERM];
MAP sanct1st[NCOLORS][WPERM];
MAP rows[MAXROWS][WPERM];
MAP cols[MAXCOLS][WPERM];
MAP diagleft[MAXDIAG][WPERM];
MAP diagright[MAXDIAG][WPERM];
MAP knightmvmaps[(BPERW * WPERM)][WPERM];
MAP kingmvmaps[(BPERW * WPERM)][WPERM];

MAP attackfr[NCOLORS][BPERW*WPERM][WPERM];
MAP attackto[NCOLORS][BPERW*WPERM][WPERM];
MAP pmovefr[NCOLORS][BPERW*WPERM][WPERM];
MAP pmoveto[NCOLORS][BPERW*WPERM][WPERM];

int okprint,tomove,sqorigin,sqdest;

extern unsigned int sqbits[];
extern char *sqchar[];
extern char *strpiece[];

main (argc, argv)
int     argc;
char   *argv[];
{
    int     niterations;
    register i;
    char cmd[20];
    float timused;
    printf("Welcome.");
    init();
    printf("\n");
    gen();
    for (;;)
    {
        evalpos(FALSE,tomove);
        printf("%s_",tomove == WHITE ? "white":"black");
	scanf ("%s",cmd);
	if (strcmp(cmd,"bd") == 0)
	  prbd();
	else if (strcmp(cmd,"game") == 0)
	  list_game();
	else if (strcmp(cmd,"wgame") == 0)
	  write_game();
	else if (strcmp(cmd,"eval") == 0)
	  evalpos(TRUE,tomove);
	else if (strcmp(cmd,"test") == 0)
	  {
	    printf("Test incremental update move generation.\n");
	    printf("Which move to make?\n");
	    scanf("%s",cmd);
	    makemvstr(cmd);	/* First, get global values for iteration */
	    genupdate(OPP(tomove),FALSE);
	    unmakemvpc(OPP(tomove),
		       game[ngamemvs].move.from,
		       game[ngamemvs].move.to,
		       game[ngamemvs].move.movpiece);
	    genupdate(tomove,TRUE);
	    printf("Iteration count? ");
	    scanf("%d",&niterations);
	    timeon();
	    test_igen(niterations);
	    timeoff();
	    timused = (float)timcpu.tv_sec+((float)timcpu.tv_usec/1000000.);
	    printf("%.2f gens/sec\n",niterations/timused);
	  }
	else if (strcmp(cmd,"maps") == 0)
	    print_maps();
	else if (strcmp(cmd,"amaps") == 0)
	  {
	    printf("--------- White attack maps ----------\n");
	    pramaps(WHITE);
	    printf("--------- Black attack maps ----------\n");
	    pramaps(BLACK);
	  }
	else if (strcmp(cmd,"reset") == 0)
	  {
	    init();
	    gen();
	  }
	else if (strcmp(cmd,"legals") == 0)
	    legals();
	else if (strcmp(cmd,"quit") == 0)
	    exit(0);
	else if (strcmp(cmd,"undo") == 0)
	  {
	    if (ngamemvs >= 0)
	      {
		unmakemvpc(OPP(tomove),
			   game[ngamemvs].move.from,
			   game[ngamemvs].move.to,
			   game[ngamemvs].move.movpiece);
		genupdate(tomove,TRUE);
	      }
	    else printf("Sorry, no more moves to retract.\n");
	  }
	else if (((strlen(cmd) == 4) && isalpha(cmd[0]) && isdigit(cmd[1]) &&
		  isalpha(cmd[2]) && isdigit(cmd[3])) || 
		  strcmp(cmd,"o-o") == 0 || strcmp(cmd,"o-o-o") == 0)
	  {
	    makemvstr(cmd);
	    genupdate(OPP(tomove),FALSE);
	  }
	else if (strcmp(cmd,"?") == 0 || strcmp(cmd,"help") == 0)
	{
	    printf("? or help   - show this help list\n");
	    printf("amaps       - show attack bitmaps\n");
	    printf("bd          - print the board\n");
	    printf("eval        - show evaluation of current board\n");
	    printf("game        - print the game so far with statistics\n");
	    printf("legals      - show movelists for current position\n");
	    printf("maps        - show initialization bitmaps\n");
	    printf("quit        - quit and exit from program\n");
	    printf("test        - run a timing test of move generation\n");
	    printf("undo        - retract last move\n");
	    printf("wgame       - as 'game' but to the file ch<number>\n");
	}
    }
}

test_igen (niterations)
{
  register i;
  for (i = 0; i < niterations; i++)
    {
      makemvpc(tomove,gfrsq,gtosq,gpc);
      genupdate(OPP(tomove),FALSE);
      unmakemvpc(OPP(tomove),
		 game[ngamemvs].move.from,
		 game[ngamemvs].move.to,
		 game[ngamemvs].move.movpiece);
      genupdate(tomove,TRUE);
    }
}
