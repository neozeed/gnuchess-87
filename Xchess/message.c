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


/* RCS Info: $Revision: 1.4 $ on $Date: 86/11/26 12:10:22 $
 *           $Source: /users/faustus/xchess/RCS/message.c,v $
 * Copyright (c) 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
 *	Permission is granted to do anything with this code except sell it
 *	or remove this message.
 *
 * Do stuff with the message window.  Font 0 is the normal font, font 1
 * is large, and font 3 is normal red.
 */

#include "xchess.h"

#define MESSAGE_HEADER	"1  XChess Messages0\n\n"

void
message_init(win)
	windata *win;
{
	TxtGrab(win->messagewin, "xchess", win->medium, 
			win->textback.pixel, win->textcolor.pixel,
				win->cursorcolor.pixel);
	TxtAddFont(win->messagewin, 1, win->large, win->textcolor.pixel);
	TxtAddFont(win->messagewin, 2, win->medium, win->errortext.pixel);
	TxtAddFont(win->messagewin, 3, win->medium, win->playertext.pixel);

	TxtWriteStr(win->messagewin, MESSAGE_HEADER);
	return;
}

void
message_add(win, string, err)
	windata *win;
	char *string;
	bool err;
{
	XSetDisplay(win->display);
	if (err) {
		TxtWriteStr(win->messagewin, "2");
		TxtWriteStr(win->messagewin, string);
		TxtWriteStr(win->messagewin, "0");
		XFeep(0);
	} else
		TxtWriteStr(win->messagewin, string);
	return;
}

void
message_send(win, event)
	windata *win;
	XEvent *event;
{
	XKeyOrButtonEvent *ev = (XKeyOrButtonEvent *) event;
	windata *ow = (win == win1) ? win2 : win1;
	char buf[BSIZE], *s;
	int i;

	s = XLookupMapping(ev, &i);
	strncpy(buf, s, i);
	buf[i] = '\0';
	for (s = buf; *s; s++)
		if (*s == '\r')
			*s = '\n';
		else if (*s == '\177')
			*s = '';

	XSetDisplay(win->display);
	TxtWriteStr(win->messagewin, "3");
	TxtWriteStr(win->messagewin, buf);
	TxtWriteStr(win->messagewin, "0");
	if (ow) {
		XSetDisplay(ow->display);
		TxtWriteStr(ow->messagewin, "3");
		TxtWriteStr(ow->messagewin, buf);
		TxtWriteStr(ow->messagewin, "0");
	}
	return;
}

