/* This file contains bitmap definitions for CHESS.
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
 * Algdefs.h -- algebraic definitions for the 64 squares.
 *    bitmaps are ordered left-to-right, bottom-to-top
 *    as viewing the board with square a1 bottom left.
 *   rank:      1st   2nd    3rd          8th
 *   bits:     [0-7] [8-15] [16-23] ... [56-63]
 *   For example, square 16 is a3.
 */

#define a1 1<<0
#define b1 1<<1
#define c1 1<<2
#define d1 1<<3
#define e1 1<<4
#define f1 1<<5
#define g1 1<<6
#define h1 1<<7
#define a2 a1<<8
#define b2 b1<<8
#define c2 c1<<8
#define d2 d1<<8
#define e2 e1<<8
#define f2 f1<<8
#define g2 g1<<8
#define h2 h1<<8
#define a3 a2<<8
#define b3 b2<<8
#define c3 c2<<8
#define d3 d2<<8
#define e3 e2<<8
#define f3 f2<<8
#define g3 g2<<8
#define h3 h2<<8
#define a4 a3<<8
#define b4 b3<<8
#define c4 c3<<8
#define d4 d3<<8
#define e4 e3<<8
#define f4 f3<<8
#define g4 g3<<8
#define h4 h3<<8

/* could change these so a5 = h1 */
#define a5 a1
#define b5 b1
#define c5 c1
#define d5 d1
#define e5 e1
#define f5 f1
#define g5 g1
#define h5 h1
#define a6 a2
#define b6 b2
#define c6 c2
#define d6 d2
#define e6 e2
#define f6 f2
#define g6 g2
#define h6 h2
#define a7 a3
#define b7 b3
#define c7 c3
#define d7 d3
#define e7 e3
#define f7 f3
#define g7 g3
#define h7 h3
#define a8 a4
#define b8 b4
#define c8 c4
#define d8 d4
#define e8 e4
#define f8 f4
#define g8 g4
#define h8 h4

#define A1 0
#define B1 1
#define C1 2
#define D1 3
#define E1 4
#define F1 5
#define G1 6
#define H1 7
#define A2 8
#define B2 9
#define C2 10
#define D2 11
#define E2 12
#define F2 13
#define G2 14
#define H2 15
#define A3 16
#define B3 17
#define C3 18
#define D3 19
#define E3 20
#define F3 21
#define G3 22
#define H3 23
#define A4 24
#define B4 25
#define C4 26
#define D4 27
#define E4 28
#define F4 29
#define G4 30
#define H4 31
#define A5 32
#define B5 33
#define C5 34
#define D5 35
#define E5 36
#define F5 37
#define G5 38
#define H5 39
#define A6 40
#define B6 41
#define C6 42
#define D6 43
#define E6 44
#define F6 45
#define G6 46
#define H6 47
#define A7 48
#define B7 49
#define C7 50
#define D7 51
#define E7 52
#define F7 53
#define G7 54
#define H7 55
#define A8 56
#define B8 57
#define C8 58
#define D8 59
#define E8 60
#define F8 61
#define G8 62
#define H8 63

