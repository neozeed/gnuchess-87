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


/* RCS Info: $Revision: 1.2 $ on $Date: 86/11/26 12:10:38 $
 *           $Source: /users/faustus/xchess/RCS/popup.c,v $
 * Copyright (c) 1986 Wayne A. Christopher, U. C. Berkeley CAD Group 
 *	 faustus@cad.berkeley.edu, ucbvax!faustus
 * Permission is granted to modify and re-distribute this code in any manner
 * as long as this notice is preserved.  All standard disclaimers apply.
 *
 * A simple pop-up menu system.
 */

#include "xchess.h"

/* Open a small window with some text in it and two buttons -- yes and no.
 * Use black and white pixel, and the medium font.
 */

bool
pop_question(win, text)
	windata *win;
	char *text;
{
	char *s, *t;
	int nlines = 1, ncols = 0, i = 0, j;
	int x, y;
	Window w;
	bool ch;
	XEvent ev;
	Pixmap btile, ttile;

	for (s = text; *s; s++) {
		if ((*s == '\n') && s[1])
			nlines++;
		if ((*s == '\n') || !s[1]) {
			if (i > ncols)
				ncols = i;
			i = 0;
		} else
			i++;
	}

	if (ncols < 12)
		ncols = 12;
	nlines += 4;
	ncols += 4;

	x = (BASE_WIDTH - ncols * win->medium->width) / 2;
	y = (BASE_HEIGHT - nlines * win->medium->height) / 2;

	XSync(0);
	XSetDisplay(win->display);
	btile = XMakeTile(win->border.pixel);
	ttile = XMakeTile(win->textback.pixel);
	w = XCreateWindow(win->basewin, x, y, ncols *
			win->medium->width, nlines * win->medium->height,
			BORDER_WIDTH, btile, ttile);
	XMapWindow(w);

	for (i = 0, s = text; i < nlines - 4; i++) {
		for (t = s, j = 0; *t && (*t != '\n'); t++, j++)
			;
		XText(w, (ncols - j) / 2 * win->medium->width, (i + 1) *
				win->medium->height, s, j, win->medium->id,
				win->textcolor.pixel, win->textback.pixel);
		s = t + 1;
	}
	XText(w, (ncols - 8) * win->medium->width / 4, (nlines - 2) *
			win->medium->height, "YES", 3, win->medium->id,
			win->textcolor.pixel, win->textback.pixel);
	XText(w, (ncols - 4) * win->medium->width * 3 / 4, (nlines - 2) *
			win->medium->height, "NO", 2, win->medium->id,
			win->textcolor.pixel, win->textback.pixel);
	
	XSelectInput(w, ButtonPressed);
	XWindowEvent(w, ButtonPressed, &ev);
	x = ((XKeyOrButtonEvent *) &ev)->x;
	y = ((XKeyOrButtonEvent *) &ev)->y;

	if (x > ncols * win->medium->width / 2)
		ch = false;
	else
		ch = true;

	XFreePixmap(btile);
	XFreePixmap(ttile);
	XDestroyWindow(w);
	return (ch);
}

