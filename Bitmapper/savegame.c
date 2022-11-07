/* This file contains the history-mechanism for CHESS.
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
#include "bitmacs.h"
#include "movelist.h"

FILE *hisf;
char *histmp = "gameXXXXXX";

extern int ngamemvs;
extern struct gmlist game[MAXGAME];
extern char *sqchar[];

zero_game()
{
    int i;
    for (i = 0; i < MAXGAME; i++)
      game[i].depth = -1;
}

write_game()
{
    int i;
    if (ngamemvs == -1)
      {
	printf("Sorry, game not played yet.\n");
	return;
      }
    unlink(histmp);
    mktemp(histmp);
    unlink(histmp);
    hisf = fopen(histmp,"w");
    if (hisf == NULL)
    {
        fprintf(stderr, "cannot open game temp file %s\n",histmp);
        return;
    }
    fprintf(hisf,"  White Black Depth  Nodes  Score    Cpu     Rate\n");
    for (i = 0; i <= ngamemvs; i++)
      {
	if (i % 2 == 0)
	  fprintf(hisf,"%d. ",(i/2)+1);
	else
	  fprintf(hisf," ");
	fprintf(hisf,"%s%s",sqchar[game[i].move.from],
	       sqchar[game[i].move.to]);
	if (i % 2 == 0)
	  fprintf(hisf," ");
	if (i % 2 != 0)
	  fprintf(hisf,"\t %2d %6d %6d %6.2f      %.0f\n",
		  game[i].depth,
		  game[i].nodes,
		  game[i].score,
		  game[i].cpu,
		  game[i].rate);
      }
    fclose(hisf);
}

list_game()
{
    register i;
    char lastchar;
    if (ngamemvs == -1)
      {
	printf("Sorry, game not played yet.\n");
	return;
      }
    printf("  White Black Depth  Nodes  Score    Cpu     Rate\n");
    for (i = 0; i <= ngamemvs; i++)
      {
	if (i % 2 == 0)
	  printf("%d. ",(i/2)+1);
	else
	  putchar(' ');
	printf("%s%s",sqchar[game[i].move.from],
	       sqchar[game[i].move.to]);
	if (i % 2 == 0)
	  {
	    putchar(' ');
	    lastchar = ' ';
	  }
	if (i % 2 != 0)
	  {
	    printf("\t %2d %6d %6d %6.2f      %.0f\n",
		   game[i].depth,
		   game[i].nodes,
		   game[i].score,
		   game[i].cpu,
		   game[i].rate);
	    lastchar = '\n';
	  }
      }
    if (lastchar != '\n') putchar('\n');
}

