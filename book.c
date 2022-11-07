/* This file contains book code for CHESS.
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
#include <ctype.h>

char *fgets();
char *tempfile = "/tmp/chXXXXXX";

typedef struct
{
	char	*dptr;
	int	dsize;
} datum;

datum	fetch();
datum firstkey();
datum nextkey();

int bookopen = FALSE;
FILE *bf;
char iline[MAXSTR];
char posstr[100],bkmove[10],bigpos[100];
extern int row,col;
extern int histtot;
extern long hashval;
struct bdtype bbd[120];
extern struct gmlist history[MAXGAME];
extern struct bdtype startbd[120];
struct mvlist bookmove;
int bmoves;
datum pos,key,posp;

/*
 * Open_book:
 *     Determines if there is a book to use for compilation.
 */
open_book(infile)
char *infile;
{
    if ((bf = fopen(infile,"r")) == NULL)     
      {
	      printf("Sorry, book %s not available.\n",infile);
	      return(FALSE);
      }
    return(TRUE);
}

/*
 * Make_book: 
 *     Actually compiles the book and produces the database.
 */
make_book()
{
    register i,tomove,movei,ngames,npos;
    char lastl,chashval;
    tomove = WHITE;	
    movei = 0;
    lastl = NULL;
    ngames = 0;
    npos = 0;
    while (fgets(iline,MAXSTR,bf) != NULL)
      for (i = 0; iline[i] != NULL; i++)
	if (iline[0] == '#')
	  continue;
        else if (iline[0] == '\n')
	{
	  copybd(startbd,bbd);
	  init_book();
	  ngames++;
	}
        else if (isdigit(iline[i]) && isdigit(iline[i+1]))
	  continue;
        else if (isspace(iline[i]) || (isdigit(iline[i]) && !isalnum(lastl)))
	  {
	      if (movei != 0)
	      {
		bookmove.flags = 0;
	        bookmove.from = ((10-(bkmove[1]-'0'))*10)+bkmove[0]-'a'+1;
	        bookmove.to = ((10-(bkmove[3]-'0'))*10)+bkmove[2]-'a'+1;
	        bookmove.movpiece = bbd[bookmove.from].piece;
	        bookmove.cappiece = bbd[bookmove.to].piece;
	        if (bookmove.movpiece == WK || bookmove.movpiece == BK)
	        {
		  if ((bookmove.from == 95 && bookmove.to == 97) ||
		      (bookmove.from == 25 && bookmove.to == 27))
		        bookmove.flags |= KCASFLAG;
		  else if ((bookmove.from == 95 && bookmove.to == 93) ||
		      (bookmove.from == 25 && bookmove.to == 23))
		        bookmove.flags |= QCASFLAG;
	        }
/*  add these only if we want position stored in book
		oboard(bbd,posstr,100);
*/
		strcpy(bigpos,bkmove);
/*
		strcat(bigpos,bbd[TOMOVE].moved == WHITE ? "+" : "-");
		printf("storing %s\n",bigpos);
*/
/*  add these only if we want position stored in book
		strcat(bigpos,posstr);
*/
		pos.dptr = bigpos;
		pos.dsize = strlen(bigpos);
		key.dptr = (char *)&hashval;
		key.dsize = 4;
		posp = fetch(key);
		if (posp.dptr == NULL)
		{		
			npos++;
			store(key,pos);
			key.dptr = (char *)&hashval;
			key.dsize = 2;
			posp = fetch(key);
/*
			printf("Stored position before %s\n",bkmove);
*/
		}
		else
		        {
/*
		    printf("Already stored position before %s\n",bkmove);
			   pboard(bbd,FALSE);
*/
		 }
		makemove(bookmove,bbd);		
	      }
	      bkmove[movei++] = NULL;
	      movei = 0;
	      lastl = iline[i];
          }
         else if (isalnum(iline[i]))
	   {
		 bkmove[movei++] = iline[i];
		 lastl = iline[i];
	   }		 
    fclose(bf);
    printf("\n%d games added, %d positions added, %d total positions in book\n",
	   ngames+1,npos+1,totpos());
}


/*
 * Init_book:
 *     Initializes board and the randomized hashing.
 */
init_book()
{
	clear_bd(bbd);
	copybd(startbd,bbd);
	init_rands();
	init_hash(bbd);
}

/*
 * Book:
 *     Control routine for compiling a new book.
 */
book()
{
    if (open_book(BOOKSRC))
    {
	printf("Compiling book, please wait...");
	fflush(stdout);
	init_book();
	dbminit(BOOKOBJ);
	make_book();
/*
	dbmclose();    
*/
    }
}

/*
 * Write_book:
 *      This allows a person to add the game played
 *      thus far to the opening book.
 */
write_book()
{
#define MAXICOM 5		/* Maximum allowed comment lines */
    register i,icom = 0;
    int pf[2];			/* Pipe to communicate with mail */
    int sent;			/* Was it successfully mailed? */
    char com[MAXSTR];		/* Room for his comments */
    char mailcmd[MAXSTR];	/* Mail command line */
    char resp[MAXSTR];		/* Put person's response here */
    FILE *tempfp;		/* Temp file stream */
    if (histtot == 0)		/* Is there a game to enter? */
    {
	printf("Sorry, there is no game that has been played.\n");
	printf("Did you want to enter a file containing games? ");
	fflush(stdout);
	getchar();		/* Don't ask why this is necessary */
				/* I don't know myself */
	if (getchar() == 'y')
	  {
	      get_book();
	      return;
	  }
	else {  
	    printf("When you decide what you want to do, come back to me\n");
	    printf("and I will be more friendly.\n");
	    return;
        }
    }
    if ((bf = fopen(BOOKSRC,"a")) == NULL) /* Try to open book */
    {
	printf("Sorry, something is wrong. I cannot append or create %s\n",
		BOOKSRC);
	printf("Please insure that this file exists and has appropriate\n");
	printf("permissions for you to write it.\n");
	return;
    }
    mktemp(tempfile);
    if ((tempfp = fopen(tempfile,"w")) == NULL)	/* Create temp file */
    {
	printf("Sorry, a temp file %s could not be created.\n",
	    tempfile);
	return;
    }	      
    printf("------------------------------------------------------------\n");
    printf("Please type a few lines about the game just played. Example:\n");
    printf("Fischer vs. Karpov, Collected Games of Fischer, page 130.\n");
    printf("Terminate your input with end-of-file\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
/*
 * Now we try some cleverness. Mail this person's new entry to the
 * maintainers of the book. The idea here is to try to speedup
 * the growth of the database.
 */
    sprintf(mailcmd,"/bin/mail %s < %s\n",
	GNUPEOPLE, tempfile);
 loop:
    while (icom++ < MAXICOM && gets(com) != NULL) /* Collect input */
    {
        fputs("\n# ",bf);
        fputs(com,bf);
	fputs("\n# ",tempfp);
	fputs(com,tempfp);
    }
    fputs("\n",bf);
    fputs("\n",tempfp);
    for (i = 1; i <= histtot; i++) /* Write out game */
    {
	if (i % 5 == 0)	/* Carriage return every 5 moves */
	{
	    fputs("\n",bf);
	    fputs("\n",tempfp);
	}
	fprintf(bf,"%d ",i);
	fprintf(tempfp,"%d ",i);
	lin_to_alg(history[i].wmove.from,bf);
	lin_to_alg(history[i].wmove.to,bf);
	lin_to_alg(history[i].wmove.from,tempfp);
	lin_to_alg(history[i].wmove.to,tempfp);
	fputs(" ",bf);
	fputs(" ",tempfp);
	if (history[i].bmove.from != NULL) 
	  {
		  lin_to_alg(history[i].bmove.from,bf);
		  lin_to_alg(history[i].bmove.to,bf);
		  lin_to_alg(history[i].bmove.from,tempfp);
		  lin_to_alg(history[i].bmove.to,tempfp);
		  fputs(" ",tempfp);
		  fputs(" ",bf);
	  }
    }
    fclose(tempfp);
    sent = system(mailcmd);
    if (sent != 127 && sent != 32512)
	sent = TRUE;
    else sent = FALSE;
    fputs("\n",bf);
    fputs("\n",tempfp);
    fclose(bf);
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("Your game has been entered %s.\n",
	   sent ? "and sent to the maintainers" :
	          "but could not be sent to the maintainers");
    printf("Thank-you for your contribution.\n");
    printf("Now please make sure the Free Software Foundation, Inc. and\n");
    printf("the author have received your newly updated book, so that\n");
    printf("others can benefit from your contribution.\n");    
    printf("-----------------------------------------------------------\n");
}

/*
 * Open a book of games and add these to the human-readable book.
 * The book of games is in Thompson format, but we make the
 * assumption that variations have been made into their own game.
 * Someday we will allow variations external to a game listing.
 */
get_book()
{
    FILE *fp;
    int npos,ngames;
    char fname[MAXSTR];
    printf("Filename of collected games: ");
    fflush(stdout);
    getchar();			/* Don't ask me why this is necessary */
    gets(fname);		/* I don't know myself */
    if ((fp = fopen(fname,"r")) == NULL)
    {
	printf("Sorry, file %s not available.\n",fname);
	return;
    }
    ngames = -1;
    npos = -1;
    printf("Compiling book, please wait...");
    fflush(stdout);
    get_all_games(fp,&npos,&ngames);
    printf("\n%d games added, %d positions added, %d total positions in book\n",
	   ngames+1,npos+1,totpos());
}

/*
 * Count and return number of positions in book
 */
totpos()
{
    int nposb;
    nposb = 0;
    for (key = firstkey(); key.dptr != NULL; key = nextkey(key))
      nposb++;
    return(nposb);
}

/*
 * Get all games from book, assuming non-abbreviated algebraic notation.
 */
get_all_games(fp,npos,ngames)
FILE *fp;
int *npos;
int *ngames;
{
    register i;
    int stri,index,seenspace;
    char curgam[MAXSTR];
    char str[MAXSTR];
    char iline[MAXSTR];
    struct mvlist moves[MAXMOVES];
    stri = 0;
    while (fgets(iline,MAXSTR,fp) != NULL)
    {
	if (iline[0] == '#')
	  continue;
        else if (iline[strlen(iline)-2] == ':')
	{
	    strcpy(curgam,iline);
	    printf("%s",curgam);
	    *ngames = *ngames + 1;
	    init_book();
	    seenspace = TRUE;
	    continue;
	}
	if (iline[0] == '\n' || strcmp(iline,"\t++$X\n") == 0) continue;
	for (i = 0; i < strlen(iline); i++)
	{
	    if (seenspace && (isdigit(iline[i]) || iline[i] == '.'
		|| iline[i] == ' '))
	      continue;  /* <move> <digit(s)><period> */
	    else if (iline[i] == ',' || iline[i] == ';' ||
		     iline[i] == '\n' || iline[i] == ' ')
	      /* move completed */
	    {
		if (iline[i] != ' ' && iline[i] != '\n')
		  seenspace = FALSE;
		else
		  seenspace = TRUE;
		str[stri] = NULL;
		stri = 0;
	        bmoves = generate(bbd,moves);
		index = match_redalg(bbd,moves,bmoves,str);
		if (index >= 0)
		{
		 	lin_to_str(moves[index].from,moves[index].to,bkmove);
/*  add these only if we want position stored in book
		oboard(bbd,posstr,100);
*/
		strcpy(bigpos,bkmove);
/*
		strcat(bigpos,bbd[TOMOVE].moved == WHITE ? "+" : "-");
		printf("storing %s\n",bigpos);
*/
/*  add these only if we want position stored in book
		strcat(bigpos,posstr);
*/
			pos.dptr = bigpos;
			pos.dsize = strlen(bigpos);
			key.dptr = (char *)&hashval;
			key.dsize = 4;
			posp = fetch(key);
			if (posp.dptr == NULL)
			{		
				*npos = *npos + 1;
				store(key,pos);
				key.dptr = (char *)&hashval;
				key.dsize = 2;
				posp = fetch(key);
/*
                                printf("Stored position before %s\n",bkmove);
*/
                	}
		        else {
/*
   		          printf("Already stored position before %s\n",bkmove);
  			  pboard(bbd,FALSE);
*/
		        }
		        makemove(moves[index],bbd);
	        }
		else {
	            printf("Bad move %s in line %s",str,iline);
		    pboard(bbd,FALSE);
/*
    		    listmoves(moves,bmoves);
*/
		    break;
		}
	    }
	    else   /* Normal letter, add to move so far */
	      {
		str[stri++] = iline[i];
		seenspace = FALSE;
	      }
	}
    }
}

/*
 * Given a reduced algebraic move such as Ng3 or d5 (as str)
 * this routine returns an index into the given move list
 * that corresponds to the actual internal move. If no
 * match is found, -1 is returned.
 */
match_redalg(bd,moves,nmoves,str)
struct bdtype bd[120];
struct mvlist moves[MAXMOVES];
int nmoves;
char str[];
{
    register i;
    int piece;		/* Piece moving */
    int dest;		/* Destination square */
    int cap;		/* Capture piece */
    int scolumn;	/* Ambiguous moves are resolved with this info */
    int srow;		/* Same as above */
    if (isalpha(str[0]) && isdigit(str[1]) && (str[2] == NULL) ||
					      (str[2] == '+'))
    {				/* take care of d4, d4+, d4++ */
        piece = (bd[TOMOVE].moved == WHITE) ? WP : BP;   /* A pawn is moving */
	dest = ((10 - (str[1]-'0'))*10) + str[0]-'a'+1;
	cap = 0;
	srow = scolumn = -1; /* say we know nothing about row/column */
    }    
    else if (isalpha(str[0]) && isalpha(str[1]) && isdigit(str[2]) &&
	    (str[3] == NULL || str[3] == '+'))
    {				/* take care of Nc3, Nc3+, Nc3++ */
	if (str[0] == 'N') piece = (bd[TOMOVE].moved == WHITE) ? WN : BN;
	else if (str[0] == 'B') piece = (bd[TOMOVE].moved == WHITE) ? WB : BB;
	else if (str[0] == 'R') piece = (bd[TOMOVE].moved == WHITE) ? WR : BR;
	else if (str[0] == 'Q') piece = (bd[TOMOVE].moved == WHITE) ? WQ : BQ;
	else if (str[0] == 'K') piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
	dest = ((10 - (str[2]-'0'))*10) + str[1]-'a'+1;
	cap = 0;
	srow = scolumn = -1; /* say we know nothing about srow/column */
    }
    else if (isalpha(str[0]) && str[1] == ':' && isalpha(str[2]) &&
	    (isdigit(str[3]) && (str[4] == NULL || str[4] == '+')))
    {				/* take care of d:e4, N:g3, N:g5+, N:g5++ */
	if (islower(str[0])) piece = (bd[TOMOVE].moved == WHITE) ? WP : BP;
	else if (str[0] == 'N') piece = (bd[TOMOVE].moved == WHITE) ? WN : BN;
	else if (str[0] == 'B') piece = (bd[TOMOVE].moved == WHITE) ? WB : BB;
	else if (str[0] == 'R') piece = (bd[TOMOVE].moved == WHITE) ? WR : BR;
	else if (str[0] == 'Q') piece = (bd[TOMOVE].moved == WHITE) ? WQ : BQ;
	else if (str[0] == 'K') piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
	dest = ((10 - (str[3]-'0'))*10) + str[2]-'a'+1;
	srow = scolumn = -1; /* say we know nothing about row/column */
	if (islower(str[0]))   /* if a pawn move, set scolumn */
	    scolumn = str[0] - 'a' + 1;    /* piece must be in this column */
	cap = bd[dest].piece;
    }
    else if ((isalpha(str[0]) && isalpha(str[1]) && isalpha(str[2]) &&
	isdigit(str[3]) && (str[4] == NULL || str[4] == '+')) ||
	     ((isalpha(str[0]) && isalpha(str[1]) && str[2] == ':' &&
	       isalpha(str[3]) && isdigit(str[4]) && (str[5] == NULL
					     || str[5] == '+'))))
    { /* Nbd7 Rh:f8 */
	if (str[0] == 'N') piece = (bd[TOMOVE].moved == WHITE) ? WN : BN;
	else if (str[0] == 'B') piece = (bd[TOMOVE].moved == WHITE) ? WB : BB;
	else if (str[0] == 'R') piece = (bd[TOMOVE].moved == WHITE) ? WR : BR;
	else if (str[0] == 'Q') piece = (bd[TOMOVE].moved == WHITE) ? WQ : BQ;
	else if (str[0] == 'K') piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
        if (str[2] == ':')
	  dest = ((10 - (str[4]-'0'))*10) + str[3]-'a'+1;
	else
	  dest = ((10 - (str[3]-'0'))*10) + str[2]-'a'+1;
	srow = -1; /* know nothing about row */
	scolumn = str[1] - 'a' + 1;    /* piece must be in this column */
	cap = (str[2] == ':');
    }
    else if ((isalpha(str[0]) && isdigit(str[1]) && isalpha(str[2]) &&
	isdigit(str[3]) && (str[4] == NULL || str[4] == '+')) ||
	     ((isalpha(str[0]) && isdigit(str[1]) && str[2] == ':' &&
	      isalpha(str[3]) && isdigit(str[4])) && (str[5] == NULL ||
					  str[5] == '+')))
    { /* N1d7 N1:d7 */
	if (str[0] == 'N') piece = (bd[TOMOVE].moved == WHITE) ? WN : BN;
	else if (str[0] == 'B') piece = (bd[TOMOVE].moved == WHITE) ? WB : BB;
	else if (str[0] == 'R') piece = (bd[TOMOVE].moved == WHITE) ? WR : BR;
	else if (str[0] == 'Q') piece = (bd[TOMOVE].moved == WHITE) ? WQ : BQ;
	else if (str[0] == 'K') piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
	if (str[2] == ':')
	  dest = ((10 - (str[4]-'0'))*10) + str[3]-'a'+1;
	else
	  dest = ((10 - (str[3]-'0'))*10) + str[2]-'a'+1;
	srow = ((10 - (str[1]-'0'))*10);  /* piece must be in this row */
	scolumn = -1; /* know nothing about column */
	cap = (str[2] == ':');
    }
    else if (strcmp(str,"O-O") == 0)
    {
	piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
	dest = (bd[TOMOVE].moved == WHITE) ? 97 : 27;
	srow = scolumn = -1;
/*
	scolumn = 7;
	if (bd[TOMOVE].moved == WHITE) srow = 9; else srow = 2;
*/
	cap = 0;
    }
    else if (strcmp(str,"O-O-O") == 0)
    {
	piece = (bd[TOMOVE].moved == WHITE) ? WK : BK;
	dest = (bd[TOMOVE].moved == WHITE) ? 93 : 23;
	srow = scolumn = -1;
/*
	scolumn = 3;
	if (bd[TOMOVE].moved == WHITE) srow = 9; else srow = 2;
*/
	cap = 0;
    }
    else return(-1);
    for (i = 0; i < nmoves; i++)
    {  /* match move according to destination, piece, and capture */
/*
	printf("to = %d, dest = %d, movpiece = %d, piece = %d, cappiece = %d, cap = %d\n",
             moves[i].to,dest,moves[i].movpiece,
	     piece,bd[moves[i].to].piece,cap);
*/
	if (moves[i].to == dest && moves[i].movpiece == piece &&
	    ((bd[moves[i].to].piece != 0 && cap) ||
	     (bd[moves[i].to].piece == 0 && !cap)))
/*
	   && (((moves[i].flags & CAPFLAG) && cap) ||
	     (!(moves[i].flags & CAPFLAG) && !cap)))
*/
	  {
/*
    printf("row = %d, column = %d, from = %d, to = %d\n",
	   srow,scolumn,moves[i].from,moves[i].to);
-1 7
*/
          if (srow > -1 || scolumn > -1)
	    {
/*
		    printf("srow = %d, scolumn = %d div from = %d, mod from = %d, from = %d\n",srow,scolumn,moves[i].from / 10, moves[i].from % 10,moves[i].from);
*/
		    if (srow > -1)
		      if (((moves[i].from / 10) * 10) == srow)
			return(i);
		    if (scolumn > -1)
		      if ((moves[i].from % 10) == scolumn)
			return(i);
	    }
          else
	    return(i);
/*
    printf("row = %d, column = %d\n",srow,scolumn);
		return(i);
*/
          }
    }
/*
    printf("row = %d, column = %d\n",srow,scolumn);
*/
    return(-1);
}
