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


/* RCS Info: $Revision: 1.5 $ on $Date: 86/11/26 12:11:15 $
 *           $Source: /users/faustus/xchess/RCS/window.c,v $
 * Copyright (c) 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Deal with the two (or one) windows.
 */

#include "xchess.h"
#include <sys/time.h>

#include "pawn.bitmap"
#include "rook.bitmap"
#include "knight.bitmap"
#include "bishop.bitmap"
#include "queen.bitmap"
#include "king.bitmap"

#include "pawn_outline.bitmap"
#include "rook_outline.bitmap"
#include "knight_outline.bitmap"
#include "bishop_outline.bitmap"
#include "queen_outline.bitmap"
#include "king_outline.bitmap"

#include "pawn_mask.bitmap"
#include "rook_mask.bitmap"
#include "knight_mask.bitmap"
#include "bishop_mask.bitmap"
#include "queen_mask.bitmap"
#include "king_mask.bitmap"

#include "shade.bitmap"

#include "xchess.cur"
#include "xchess_mask.cur"

#include "xchess.icon"

windata *win1, *win2;
bool win_flashmove = false;

extern bool setup();
extern void service(), drawgrid(), icon_refresh();

bool
win_setup(disp1, disp2)
	char *disp1, *disp2;
{
	win1 = alloc(windata);
	if (!oneboard)
		win2 = alloc(windata);

	if (!setup(disp1, win1) || (!oneboard && !setup(disp2, win2)))
		return (false);

	if (blackflag) {
		win1->color = BLACK;
		win1->flipped = true;
	} else
		win1->color = WHITE;
	win_drawboard(win1);

	if (!oneboard) {
		win2->color = BLACK;
		win2->flipped = true;
		win_drawboard(win2);
	}
	
	return(true);
}

/* Draw the chess board... */

void
win_drawboard(win)
	windata *win;
{
	int i, j;

	XSetDisplay(win->display);

	drawgrid(win);

	/* Now toss on the squares... */
	for (i = 0; i < SIZE; i++)
		for (j = 0; j < SIZE; j++)
			win_erasepiece(j, i, win->color);

	return;
}

/* Draw one piece. */

void
win_drawpiece(p, y, x, wnum)
	piece *p;
	int y, x;
	color wnum;
{
	short *bits, *maskbits, *outline;
	windata *win;
	Bitmap mask;
	char buf[BSIZE];

	if (oneboard || (wnum == win1->color))
		win = win1;
	else
		win = win2;

	XSetDisplay(win->display);

	if (win->flipped) {
		y = SIZE - y - 1;
		x = SIZE - x - 1;
	}

	/*
	if (debug)
		fprintf(stderr, "draw a %s at (%d, %d) on board %d\n",
				piecenames[(int) p->type], y, x, wnum);
	 */

if ((x < 0) || (x > 7) || (y < 0) || (y > 7)) exit(1);

	switch (p->type) {
	    case PAWN:
		bits = pawn_bits;
		maskbits = pawn_mask_bits;
		outline = pawn_outline_bits;
		break;

	    case ROOK:
		bits = rook_bits;
		maskbits = rook_mask_bits;
		outline = rook_outline_bits;
		break;

	    case KNIGHT:
		bits = knight_bits;
		maskbits = knight_mask_bits;
		outline = knight_outline_bits;
		break;

	    case BISHOP:
		bits = bishop_bits;
		maskbits = bishop_mask_bits;
		outline = bishop_outline_bits;
		break;

	    case QUEEN:
		bits = queen_bits;
		maskbits = queen_mask_bits;
		outline = queen_outline_bits;
		break;

	    case KING:
		bits = king_bits;
		maskbits = king_mask_bits;
		outline = king_outline_bits;
		break;

	    default:
		fprintf(stderr,
			"Internal Error: win_drawpiece: bad piece type %d\n",
			p->type);
	}

	/* There are two things we can do... If this is a black and white
	 * display, we have to shade the square and use an outline if the piece
	 * is white.  We also have to use a mask...  Since we don't want
	 * to use up too many bitmaps, create the mask bitmap, put the bits,
	 * and then destroy it.
	 */
	if (win->bnw && (p->color == WHITE))
		bits = outline;
	if (win->bnw && !iswhite(win, x, y)) {
		XBitmapBitsPut(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
			y * (SQUARE_HEIGHT + BORDER_WIDTH), SQUARE_WIDTH,
			SQUARE_HEIGHT, shade_bits, BlackPixel, WhitePixel,
			0, GXcopy, AllPlanes);
		mask = XStoreBitmap(SQUARE_WIDTH, SQUARE_HEIGHT, maskbits);
		XBitmapBitsPut(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
			y * (SQUARE_HEIGHT + BORDER_WIDTH), SQUARE_WIDTH,
			SQUARE_HEIGHT, bits, BlackPixel, WhitePixel,
			mask, GXcopy, AllPlanes);
		XFreeBitmap(mask);
	} else if (win->bnw){
		XBitmapBitsPut(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
			y * (SQUARE_HEIGHT + BORDER_WIDTH), SQUARE_WIDTH,
			SQUARE_HEIGHT, bits, BlackPixel, WhitePixel,
			0, GXcopy, AllPlanes);
	} else {
		XBitmapBitsPut(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
			y * (SQUARE_HEIGHT + BORDER_WIDTH), SQUARE_WIDTH,
			SQUARE_HEIGHT, bits, ((p->color == WHITE) ?
			win->whitepiece.pixel : win->blackpiece.pixel),
			(iswhite(win, x, y) ? win->whitesquare.pixel :
			win->blacksquare.pixel), 0, GXcopy, AllPlanes);
	}

	if (!record_english) {
		if (!x) {
			sprintf(buf, " %d", SIZE - y);
			XText(win->boardwin, 1, (y + 1) * (SQUARE_HEIGHT + 
					BORDER_WIDTH) - BORDER_WIDTH - 
					win->small->height - 1, buf, 2,
					win->small->id, win->textcolor.pixel, 
					((iswhite(win, x, y) || win->bnw) ?
					win->whitesquare.pixel : 
					win->blacksquare.pixel));
		}
		if (y == SIZE - 1) {
			sprintf(buf, "%c", 'A' + x);
			XText(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH)
					+ 1, SIZE * (SQUARE_HEIGHT + 
					BORDER_WIDTH) - BORDER_WIDTH - 
					win->small->height - 1, buf, 1,
					win->small->id, win->textcolor.pixel,
					((iswhite(win, x, y) || win->bnw) ? 
					win->whitesquare.pixel : 
					win->blacksquare.pixel));
		}
	}
	return;
}

void
win_erasepiece(y, x, wnum)
	int y, x;
	color wnum;
{
	windata *win;
	char buf[BSIZE];

	if (oneboard || (wnum == win1->color))
		win = win1;
	else
		win = win2;
		
	XSetDisplay(win->display);

	if (win->flipped) {
		y = SIZE - y - 1;
		x = SIZE - x - 1;
	}

	/*
	if (debug)
		fprintf(stderr, "erase square (%d, %d) on board %d\n", y, x,
				wnum);
	 */

if ((x < 0) || (x > 7) || (y < 0) || (y > 7)) exit(1);

	if (win->bnw && !iswhite(win, x, y)) {
		XBitmapBitsPut(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
			y * (SQUARE_HEIGHT + BORDER_WIDTH), SQUARE_WIDTH,
			SQUARE_HEIGHT, shade_bits, BlackPixel, WhitePixel,
			0, GXcopy, AllPlanes);
	} else {
		XPixSet(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH),
				y * (SQUARE_HEIGHT + BORDER_WIDTH),
				SQUARE_WIDTH, SQUARE_HEIGHT, iswhite(win, x, y)
				? win->whitesquare.pixel :
				win->blacksquare.pixel);
	}

	if (!record_english) {
		if (!x) {
			sprintf(buf, " %d", SIZE - y);
			XText(win->boardwin, 1, (y + 1) * (SQUARE_HEIGHT + 
					BORDER_WIDTH) - BORDER_WIDTH - 
					win->small->height - 1, buf, 2,
					win->small->id, win->textcolor.pixel, 
					((iswhite(win, x, y) || win->bnw) ? 
					win->whitesquare.pixel : 
					win->blacksquare.pixel));
		}
		if (y == SIZE - 1) {
			sprintf(buf, "%c", 'A' + x);
			XText(win->boardwin, x * (SQUARE_WIDTH + BORDER_WIDTH)
					+ 1, SIZE * (SQUARE_HEIGHT + 
					BORDER_WIDTH) - BORDER_WIDTH - 
					win->small->height - 1, buf, 1,
					win->small->id, win->textcolor.pixel,
					((iswhite(win, x, y) || win->bnw) ? 
					win->whitesquare.pixel : 
					win->blacksquare.pixel));
		}
	}

	return;
}

void
win_flash(m, wnum)
	move *m;
	color wnum;
{
	windata *win;
	int sx, sy, ex, ey, i;

	if (oneboard || (wnum == win1->color))
		win = win1;
	else
		win = win2;
		
	XSetDisplay(win->display);

	if (win->flipped) {
		sx = SIZE - m->fromx - 1;
		sy = SIZE - m->fromy - 1;
		ex = SIZE - m->tox - 1;
		ey = SIZE - m->toy - 1;
	} else {
		sx = m->fromx;
		sy = m->fromy;
		ex = m->tox;
		ey = m->toy;
	}
	sx = sx * (SQUARE_WIDTH + BORDER_WIDTH) + SQUARE_WIDTH / 2;
	sy = sy * (SQUARE_HEIGHT + BORDER_WIDTH) + SQUARE_HEIGHT / 2;
	ex = ex * (SQUARE_WIDTH + BORDER_WIDTH) + SQUARE_WIDTH / 2;
	ey = ey * (SQUARE_HEIGHT + BORDER_WIDTH) + SQUARE_HEIGHT / 2;

	for (i = 0; i < num_flashes * 2; i++)
		XLine(win->boardwin, sx, sy, ex, ey, flash_size, flash_size,
				BlackPixel, GXinvert, AllPlanes);
	return;
}

/* Handle input from the players. */

void
win_process(quick)
	bool quick;
{
	int i, rfd = 0, wfd = 0, xfd = 0;
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = (quick ? 0 : 500000);

	XSetDisplay(win1->display);
	if (XPending())
		service(win1);
	if (!oneboard) {
		XSetDisplay(win2->display);
		if (XPending())
			service(win2);
	}

	if (oneboard)
		rfd = 1 << win1->display->fd;
	else
		rfd = (1 << win1->display->fd) | (1 << win2->display->fd);
	if (!(i = select(32, &rfd, &wfd, &xfd, &timeout)))
		return;
	if (i == -1) {
		perror("select");
		exit(1);
	}
	if (rfd & (1 << win1->display->fd))
		service(win1);
	if (!oneboard && (rfd & (1 << win2->display->fd)))
		service(win2);

	return;
}

static void
service(win)
	windata *win;
{
	XEvent ev;

	XSetDisplay(win->display);

	while(XPending()) {
		XNextEvent(&ev);
		if (TxtFilter(&ev))
			continue;

		if (ev.window == win->boardwin) {
			switch (ev.type) {
			    case ButtonPressed:
				button_pressed(&ev, win);
				break;

			    case ButtonReleased:
				button_released(&ev, win);
				break;

			    case ExposeRegion:
			    case ExposeWindow:
				/* Redraw... */
				win_redraw(win, &ev);
				break;

			    case 0:
				break;
			    default:
				fprintf(stderr, "Bad event type %d\n", ev.type);
				exit(1);
			}
		} else if (ev.window == win->wclockwin) {
			switch (ev.type) {
			    case ExposeRegion:
			    case ExposeWindow:
				clock_draw(win, WHITE);
				break;

			    case 0:
				break;
			    default:
				fprintf(stderr, "Bad event type %d\n", ev.type);
				exit(1);
			}
		} else if (ev.window == win->bclockwin) {
			switch (ev.type) {
			    case ExposeRegion:
			    case ExposeWindow:
				clock_draw(win, BLACK);
				break;

			    case 0:
				break;
			    default:
				fprintf(stderr, "Bad event type %d\n", ev.type);
				exit(1);
			}
		} else if (ev.window == win->jailwin) {
			switch (ev.type) {
			    case ExposeRegion:
			    case ExposeWindow:
				jail_draw(win);
				break;

			    case 0:
				break;
			    default:
				fprintf(stderr, "Bad event type %d\n", ev.type);
				exit(1);
			}
		} else if (ev.window == win->buttonwin) {
			switch (ev.type) {
			    case ButtonPressed:
				button_service(win, &ev);
				break;

			    case ExposeRegion:
			    case ExposeWindow:
				button_draw(win, &ev);
				break;

			    case 0:
				break;
			    default:
				fprintf(stderr, "Bad event type %d\n", ev.type);
				exit(1);
			}
		} else if (ev.window == win->icon) {
			icon_refresh(win);
		} else if (ev.window == win->basewin) {
			message_send(win, &ev);
		} else {
			fprintf(stderr, "Internal Error: service: bad win\n");
			fprintf(stderr, "window = %d, event = %d\n", ev.window,
					ev.type);
		}
	}
	return;
}

void
win_redraw(win, event)
	windata *win;
	XEvent *event;
{
	XExposeEvent *ev = (XExposeEvent *) event;
	int x1, y1, x2, y2, i, j;

	drawgrid(win);
	if (ev) {
		x1 = ev->x / (SQUARE_WIDTH + BORDER_WIDTH);
		y1 = ev->y / (SQUARE_HEIGHT + BORDER_WIDTH);
		x2 = (ev->x + ev->width) / (SQUARE_WIDTH + BORDER_WIDTH);
		y2 = (ev->y + ev->height) / (SQUARE_HEIGHT + BORDER_WIDTH);
	} else {
		x1 = 0;
		y1 = 0;
		x2 = SIZE - 1;
		y2 = SIZE - 1;
	}

	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 < 0) x2 = 0;
	if (y2 < 0) y2 = 0;
	if (x1 > SIZE - 1) x1 = SIZE - 1;
	if (y1 > SIZE - 1) y1 = SIZE - 1;
	if (x2 > SIZE - 1) x2 = SIZE - 1;
	if (y2 > SIZE - 1) y2 = SIZE - 1;

	if (win->flipped) {
		y1 = SIZE - y2 - 1;
		y2 = SIZE - y1 - 1;
		x1 = SIZE - x2 - 1;
		x2 = SIZE - x1 - 1;
	}

	for (i = x1; i <= x2; i++) 
		for (j = y1; j <= y2; j++) {
			if (chessboard->square[j][i].color == NONE)
				win_erasepiece(j, i, WHITE);
			else
				win_drawpiece(&chessboard->square[j][i], j, i,
						WHITE);
			if (!oneboard) {
				if (chessboard->square[j][i].color == NONE)
					win_erasepiece(j, i, BLACK);
				else
					win_drawpiece(&chessboard->square[j][i],
							j, i, BLACK);
			}
		}
	
	return;
}

static bool
setup(dispname, win)
	char *dispname;
	windata *win;
{
	Pixmap btile, ttile;
	char buf[BSIZE];
	Bitmap bm;
	Cursor cur;

	if (!(win->display = XOpenDisplay(dispname)))
		return (false);
	
	if ((DisplayPlanes() == 1) || bnwflag)
		win->bnw = true;
	
	/* Allocate colors... */
	if (win->bnw) {
		win->blackpiece.pixel = BlackPixel;
		win->whitepiece.pixel = WhitePixel;
		win->blacksquare.pixel = BlackPixel;
		win->whitesquare.pixel = WhitePixel;
		win->border.pixel = BlackPixel;
		win->textcolor.pixel = BlackPixel;
		win->textback.pixel = WhitePixel;
		win->playertext.pixel = BlackPixel;
		win->errortext.pixel = BlackPixel;
		win->cursorcolor.pixel = BlackPixel;
	} else {
		if (!XParseColor(black_piece_color, &win->blackpiece) ||
		    !XParseColor(white_piece_color, &win->whitepiece) ||
		    !XParseColor(black_square_color, &win->blacksquare) ||
		    !XParseColor(white_square_color, &win->whitesquare) ||
		    !XParseColor(border_color, &win->border) ||
		    !XParseColor(text_color, &win->textcolor) ||
		    !XParseColor(text_back, &win->textback) ||
		    !XParseColor(error_text, &win->errortext) ||
		    !XParseColor(player_text, &win->playertext) ||
		    !XParseColor(cursor_color, &win->cursorcolor) ||
		    !XGetHardwareColor(&win->blackpiece) ||
		    !XGetHardwareColor(&win->whitepiece) ||
		    !XGetHardwareColor(&win->blacksquare) ||
		    !XGetHardwareColor(&win->whitesquare) ||
		    !XGetHardwareColor(&win->border) ||
		    !XGetHardwareColor(&win->textcolor) ||
		    !XGetHardwareColor(&win->textback) ||
		    !XGetHardwareColor(&win->errortext) ||
		    !XGetHardwareColor(&win->playertext) ||
		    !XGetHardwareColor(&win->cursorcolor))
			fprintf(stderr, "Can't get color...\n");
	}

	/* Get fonts... */
	win->small = XOpenFont(SMALL_FONT);
	win->medium = XOpenFont(MEDIUM_FONT);
	win->large = XOpenFont(LARGE_FONT);
	
	/* Create the windows... */

	btile = XMakeTile(win->border.pixel);
	ttile = XMakeTile(win->textback.pixel);
	win->basewin = XCreateWindow(RootWindow, BASE_XPOS, BASE_YPOS,
			BASE_WIDTH, BASE_HEIGHT, 0, WhitePixmap, WhitePixmap);
	win->boardwin = XCreateWindow(win->basewin, BOARD_XPOS, BOARD_YPOS,
			BOARD_WIDTH, BOARD_HEIGHT, BORDER_WIDTH, btile,
			WhitePixmap);
	win->recwin = XCreateWindow(win->basewin, RECORD_XPOS, RECORD_YPOS,
			RECORD_WIDTH, RECORD_HEIGHT, BORDER_WIDTH,
			btile, ttile);
	win->jailwin = XCreateWindow(win->basewin, JAIL_XPOS, JAIL_YPOS,
			JAIL_WIDTH, JAIL_HEIGHT, BORDER_WIDTH,
			btile, ttile);
	win->wclockwin = XCreateWindow(win->basewin, WCLOCK_XPOS, WCLOCK_YPOS,
			CLOCK_WIDTH, CLOCK_HEIGHT, BORDER_WIDTH, btile,
			ttile);
	win->bclockwin = XCreateWindow(win->basewin, BCLOCK_XPOS, BCLOCK_YPOS,
			CLOCK_WIDTH, CLOCK_HEIGHT, BORDER_WIDTH, btile,
			ttile);
	win->messagewin = XCreateWindow(win->basewin, MESS_XPOS, MESS_YPOS,
			MESS_WIDTH, MESS_HEIGHT, BORDER_WIDTH, btile, ttile);
	win->buttonwin = XCreateWindow(win->basewin, BUTTON_XPOS, BUTTON_YPOS,
			BUTTON_WIDTH, BUTTON_HEIGHT, BORDER_WIDTH, btile,
			ttile);
	
	/* Let's define an icon... */
	bm = XStoreBitmap(icon_width, icon_height, icon_bits);
	win->iconpixmap = XMakePixmap(bm, win->blacksquare.pixel,
			win->whitesquare.pixel);
	win->icon = XCreateWindow(RootWindow, BASE_XPOS, BASE_YPOS, icon_width,
			icon_height, 2, btile, WhitePixmap);
	XSetIconWindow(win->basewin, win->icon);
	XSelectInput(win->icon, ExposeRegion);

	cur = XCreateCursor(xchess_width, xchess_height, xchess_bits,
			xchess_mask_bits, xchess_x_hot, xchess_y_hot,
			win->cursorcolor.pixel, WhitePixel, GXcopy);
	XDefineCursor(win->basewin, cur);

	XMapWindow(win->basewin);
	XMapSubwindows(win->basewin);

	XSelectInput(win->basewin, KeyPressed);
	XSelectInput(win->boardwin, ButtonPressed | ButtonReleased |
			ExposeRegion | ExposeWindow);
	XSelectInput(win->recwin, ButtonReleased | ExposeRegion |
			ExposeWindow | ExposeCopy);
	XSelectInput(win->jailwin, ExposeRegion | ExposeWindow);
	XSelectInput(win->wclockwin, ExposeRegion | ExposeWindow);
	XSelectInput(win->bclockwin, ExposeRegion | ExposeWindow);
	XSelectInput(win->messagewin, ButtonReleased | ExposeRegion |
			ExposeWindow | ExposeCopy);
	XSelectInput(win->buttonwin, ButtonPressed | ExposeRegion |
			ExposeWindow);
	
	message_init(win);
	record_init(win);
	button_draw(win);
	jail_init(win);
	clock_init(win, WHITE);
	clock_init(win, BLACK);
	if (timeunit) {
		if (timeunit > 1800)
			sprintf(buf, "%d moves every %.2lg hours.\n",
				movesperunit, ((double) timeunit) / 3600);
		else if (timeunit > 30)
			sprintf(buf, "%d moves every %.2lg minutes.\n",
				movesperunit, ((double) timeunit) / 60);
		else
			sprintf(buf, "%d moves every %d seconds.\n",
				movesperunit, timeunit);
		message_add(win, buf, false);
	}

	XFreePixmap(btile);
	XFreePixmap(ttile);
	
	return (true);
}

static void
drawgrid(win)
	windata *win;
{
	int i;

	XSetDisplay(win->display);
	/* Draw the lines... horizontal, */
	for (i = 1; i < SIZE; i++)
		XLine(win->boardwin, 0, i * (SQUARE_WIDTH + BORDER_WIDTH) -
				(BORDER_WIDTH + 1) / 2 - 1, SIZE *
				(SQUARE_WIDTH + BORDER_WIDTH),
				i * (SQUARE_WIDTH +
				BORDER_WIDTH) - (BORDER_WIDTH + 1) / 2 - 1,
				BORDER_WIDTH, BORDER_WIDTH, win->border.pixel,
				GXcopy, AllPlanes);

	/* and vertical... */
	for (i = 1; i < SIZE; i++)
		XLine(win->boardwin, i * (SQUARE_WIDTH + BORDER_WIDTH) -
				(BORDER_WIDTH + 1) / 2 - 1, 0,
				i * (SQUARE_WIDTH +
				BORDER_WIDTH) - (BORDER_WIDTH + 1) / 2 - 1,
				SIZE * (SQUARE_WIDTH + BORDER_WIDTH),
				BORDER_WIDTH, BORDER_WIDTH, win->border.pixel,
				GXcopy, AllPlanes);
	return;
}

void
win_restart()
{
	win1->flipped = false;
	win_redraw(win1, (XEvent *) NULL);
	if (!oneboard) {
		win2->flipped = true;
		win_redraw(win2, (XEvent *) NULL);
	}
	return;
}

static void
icon_refresh(win)
	windata *win;
{
	XPixmapPut(win->icon, 0, 0, 0, 0, icon_width, icon_height,
			win->iconpixmap, GXcopy, AllPlanes);
	return;
}

