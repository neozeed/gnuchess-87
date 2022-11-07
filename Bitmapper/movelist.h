/* This file contains move-list structures for CHESS.
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

/*
 * Movelist.h -- structure of a move list, game list
 */

#define M_KCASTLE    1		/* A King-side castle */
#define M_QCASTLE    2		/* A Queen-side castle */
#define M_PROMOTE    4		/* A Promotion */
#define M_ENPASS     8		/* An Enpassant */

struct mvlist {
    int from;			/* Origination square */
    int to;			/* Destination square */
    int movpiece;		/* Moving piece */
    int cappiece;		/* Captured piece */
    int capcount;		/* Count of moves by captured piece */
    int propiece;		/* Promoted to this */
    int score;			/* Rating of move */
    char flags;			/* Associated flags above */
};

struct gmlist {
    int depth;			/* Depth searched to */
    long nodes;			/* Number of nodes searched */
    int score;			/* Score of position */
    float cpu;			/* Cpu used */
    float rate;			/* Nodes searched per second */
    struct mvlist move;		/* Associated move list */
};


