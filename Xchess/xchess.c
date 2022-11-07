/* This file contains code for X-CHESS.
   Copyright (C) 1986 Free Software Foundation, Inc.

This file is part of X-CHESS.

X-CHESS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.  No author or distributor
accepts responsibility to anyone for the consequences of using it
or for whether it serves any particular purpose or works at all,
unless he says so in writing.  Refer to the X-CHESS General Public
License for full details.

Everyone is granted permission to copy, modify and redistribute
X-CHESS, but only under the conditions described in the
X-CHESS General Public License.   A copy of this license is
supposed to have been given to you along with X-CHESS so you
can know your rights and responsibilities.  It should be in a
file named COPYING.  Among other things, the copyright notice
and this notice must be preserved on all copies.  */


/* RCS Info: $Revision: 1.5 $ on $Date: 86/11/26 12:11:32 $
 *           $Source: /users/faustus/xchess/RCS/xchess.c,v $
 * Copyright (c) 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 */

#define USAGE	"xchess [ -d ] [ -f recordfile ] [ -r savedfile ] [ -i ]\n\
\t[ -t moves/timeunit ] [ -c ] [ -p program ]  [ -b ] [ -bnw ] [ -s ]\n\
\t[ -n ] [ -h host ] [ -v ] [ -R ] [ whitedisplay ] [ blackdisplay ]"

#include "xchess.h"

bool debug = false;
bool oneboard = false;
bool bnwflag = false;
bool progflag = false;
bool blackflag = false;
bool quickflag = false;

char *progname = DEF_PROGRAM;
char *proghost = NULL;
char *piecenames[] = { "pawn", "rook", "knight", "bishop", "queen", "king" } ;
char *colornames[] = { "white", "black", "none" } ;
char *movetypenames[] = { "move", "qcastle", "kcastle", "capture" } ;
char *dispname1 = NULL, *dispname2 = NULL;

char *black_piece_color = BLACK_PIECE_COLOR;
char *white_piece_color = WHITE_PIECE_COLOR;
char *black_square_color = BLACK_SQUARE_COLOR;
char *white_square_color = WHITE_SQUARE_COLOR;
char *border_color = BORDER_COLOR;
char *text_color = TEXT_COLOR;
char *text_back = TEXT_BACK;
char *error_text = ERROR_TEXT;
char *player_text = PLAYER_TEXT;
char *cursor_color = CURSOR_COLOR;

int num_flashes = NUM_FLASHES;
int flash_size = FLASH_SIZE;

void
main(ac, av)
	char **av;
{
	char *program = av[0];
	char *recfile = NULL, *s;
	bool randflag = false;
	move *m;

	/* Process args. */
	av++; ac--;
	while (**av == '-') {
		if (eq(*av, "-d")) {
			debug = true;
		} else if (eq(*av, "-f")) {
			av++; ac--;
			if (*av)
				record_file = *av;
			else
				goto usage;
		} else if (eq(*av, "-r")) {
			av++; ac--;
			if (*av)
				recfile = *av;
			else
				goto usage;
		} else if (eq(*av, "-i")) {
			record_english = false;
		} else if (eq(*av, "-R")) {
			randflag = true;
		} else if (eq(*av, "-v")) {
			win_flashmove = true;
		} else if (eq(*av, "-q")) {
			quickflag = true;
		} else if (eq(*av, "-t")) {
			av++; ac--;
			if (*av) {
				movesperunit = atoi(*av);
				if (s = index(*av, '/'))
					timeunit = atoi(s + 1);
				else
					timeunit = 60;
			} else
				goto usage;
		} else if (eq(*av, "-p")) {
			av++; ac--;
			if (*av)
				progname = *av;
			else
				goto usage;
		} else if (eq(*av, "-h")) {
			av++; ac--;
			if (*av)
				proghost = *av;
			else
				goto usage;
		} else if (eq(*av, "-b")) {
			blackflag = true;
		} else if (eq(*av, "-c")) {
			progflag = true;
		} else if (eq(*av, "-bnw")) {
			bnwflag = true;
		} else if (eq(*av, "-s")) {
			saveflag = true;
		} else if (eq(*av, "-n")) {
			noisyflag = true;
		} else
			goto usage;
		av++; ac--;
	}
	if (ac > 0)
		dispname1 = av[0];
	if (ac > 1)
		dispname2 = av[1];
	if (ac > 2)
		goto usage;

	if (!dispname2)
		oneboard = true;
	
	srandom(getpid());

	if (!oneboard && randflag && (random() % 2)) {
		s = dispname1;
		dispname1 = dispname2;
		dispname2 = s;
	}
		
	if (!dispname1)
		dispname1 = getenv("DISPLAY");
	
	/* Now get boolean defaults... */
	if ((s = XGetDefault(program, "noisy")) && eq(s, "on"))
		noisyflag = true;
	if ((s = XGetDefault(program, "savemoves")) && eq(s, "on"))
		saveflag = true;
	if ((s = XGetDefault(program, "algebraic")) && eq(s, "on"))
		record_english = false;
	if ((s = XGetDefault(program, "blackandwhite")) && eq(s, "on"))
		bnwflag = true;
	if ((s = XGetDefault(program, "quickrestore")) && eq(s, "on"))
		quickflag = true;
	if ((s = XGetDefault(program, "flash")) && eq(s, "on"))
		win_flashmove = true;
	
	/* ... numeric variables ... */
	if (s = XGetDefault(program, "numflashes"))
		num_flashes = atoi(s);
	if (s = XGetDefault(program, "flashsize"))
		flash_size = atoi(s);
	
	/* ... and strings. */
	if (s = XGetDefault(program, "progname"))
		progname = s;
	if (s = XGetDefault(program, "proghost"))
		proghost = s;
	if (s = XGetDefault(program, "recordfile"))
		recfile = s;
	if (s = XGetDefault(program, "blackpiece"))
		black_piece_color = s;
	if (s = XGetDefault(program, "whitepiece"))
		white_piece_color = s;
	if (s = XGetDefault(program, "blacksquare"))
		black_square_color = s;
	if (s = XGetDefault(program, "whitesquare"))
		white_square_color = s;
	if (s = XGetDefault(program, "bordercolor"))
		border_color = s;
	if (s = XGetDefault(program, "textcolor"))
		text_color = s;
	if (s = XGetDefault(program, "textback"))
		text_back = s;
	if (s = XGetDefault(program, "errortext"))
		error_text = s;
	if (s = XGetDefault(program, "playertext"))
		player_text = s;
	if (s = XGetDefault(program, "cursorcolor"))
		cursor_color = s;

	/* Set up the board. */
	board_setup();

	/* Create the windows. */
	win_setup(dispname1, dispname2);

	board_drawall();

	/* Start the program if necessary. */
	if (progflag)
		if (!program_init(progname))
			exit(1);
	
	XSync(0);
	if (recfile)
		load_game(recfile);

	/* Go into a loop of prompting players alternately for moves, checking
	 * them, and updating things.
	 */
	for (;;) {
		win_process(false);
		clock_update();
		if (progflag && ((!blackflag && (nexttomove == BLACK)) ||
				(blackflag && (nexttomove == WHITE)))) {
			m = program_get();
			if (m)
				prog_move(m);
		}
	}

usage:	fprintf(stderr, "Usage: %s\n", USAGE);
	exit(1);
}

