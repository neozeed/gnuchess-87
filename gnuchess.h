/*
This file contains the header definitions for CHESS.
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

/*#define BOOKSRC "/usr/redwood/cracraft/cc/Chess/bookin"	/* Source book */
/*#define BOOKOBJ "/usr/redwood/cracraft/cc/Chess/book"  	/* DBM book(s) */

#define BOOKSRC "bookin"	/* Source book */
#define BOOKOBJ "book"  	/* DBM book(s) */

/*
 * The following entry is used whenever the program tries to be clever
 * and send data to the GNU Chess maintainers. Entries within the string
 * should be separated by a space.
 */
#define GNUPEOPLE "cracraft@math.ucla.edu"

#define FALSE 0
#define TRUE 1
#define NOMATCH -1

#define CAPSORT 0
#define SCORESORT 1

#define MATE 30000
#define DRAW 30001

#define TABLE 0
#define MAILBOX 1

#define WHITE 0
#define BLACK 1

#define OPENING 10
#define MAXPC 6
#define MAXDIRS 8
#define MAXMOVES 100
#define MAXGAME 200
#define MAXSTACK 10000
#define MAXATTACKS 40
#define HASHTBLSZ 65536
#define MAXSTR 128		/* Maximum length of our strings */

#define WP 1
#define WN 2
#define WB 3
#define WR 4
#define WQ 5
#define WK 6
#define BP -WP
#define BN -WN
#define BB -WB
#define BR -WR
#define BQ -WQ
#define BK -WK

#define WMAT 0
#define BMAT 1
#define TOMOVE 2
#define WPOS 3
#define BPOS 4
#define WKING 5
#define BKING 6

#define MAXINT 32767
#define MININT -32767

#define OFF	 99
#define EMP	  0
#define NORMAL	  1
#define CAPTURE   2
#define DEFENSE	  3
#define CAPFLAG  1
#define PROMFLAG 2
#define KCASFLAG 4
#define QCASFLAG 8
#define VALID 	16
#define LBOUND 	32
#define UBOUND 	64
#define NULLMV 128

#define OPPCOLOR(col) ((col == WHITE) ? BLACK : WHITE)
#define COLOR(pc) ((pc < 0) ? BLACK : ((pc > 0 && pc < 99) ? WHITE : OFF))
#define MAX(A,B) (((A) > (B)) ? (A) : (B))
#define MIN(A,B) (((A) < (B)) ? (A) : (B))
#define abs(A) (((A) >= 0) ? (A) : (-1 * (A)))

struct pvstr {
    int from;
    int to;
};

struct bdtype {
    int piece;
    int moved;
};

struct dirtypes {
    int ndirs;
    int slide;
    int dirs[MAXDIRS];
};

struct mvlist {
    int from;
    int to;
    int movpiece;
    int cappiece;
    int capcount;
    int propiece;
    int score;
    char flags;
};

struct gmlist {
    int depth;
    long nodes;
    int score;
    float cpu;
    float rate;
    struct mvlist wmove;
    struct mvlist bmove;
};

struct hashentry {
    int depth;
    long match;
    struct mvlist move;
};
