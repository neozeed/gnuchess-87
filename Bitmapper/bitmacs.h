/* This file contains the bitmap-macros for CHESS.
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
 * Bitmacs.h -- macros for simple bitmap manipulations.
 */
#define MEMBER(sq,map) (sqbits[sq] & map[(sq < 32 ? LOBD : HIBD)])
#define BIT_COPY(map1,map2) {map2[0] = map1[0]; map2[1] = map1[1];}
#define NOT_ZERO(map) (map[0] != 0 || map[1] != 0)
#define IS_ZERO(map) (map[0] == 0 && map[1] == 0)
#define BIT_ZERO(map) {map[0] = 0; map[1] = 0;}
#define BIT_U(sq,map)\
    {map[sq < 32 ? LOBD : HIBD] &= ~sqbits[sq];}
#define BIT_S(sq,map)\
    {map[sq < 32 ? LOBD : HIBD] |= sqbits[sq];}
#define BIT_SET(sq,map)\
    {map[sq][sq < 32 ? LOBD : HIBD] |= sqbits[sq];}
#define BIT_MSET(sq,index,color,map)\
    {map[color][index][sq < 32 ? LOBD : HIBD] |= sqbits[sq];}
#define BIT_PSET(sq,pc,color,map)\
    {map[color][pc][sq < 32 ? LOBD : HIBD] |= sqbits[sq];}
#define BIT_UNSET(sq,map)\
    {map[sq][sq < 32 ? LOBD : HIBD] &= ~sqbits[sq];}
#define BIT_MUNSET(sq,index,color,map)\
    {map[color][index][index < 32 ? LOBD : HIBD] &= ~sqbits[sq];}
#define BIT_PUNSET(sq,pc,color,map)\
    {map[color][pc][sq < 32 ? LOBD : HIBD] &= ~sqbits[sq];}
#define OPP(color) ((color == WHITE) ? BLACK : WHITE)
#define BIT_LEFT(map,nbits,dest)\
    {dest[1] = (nbits < BPERW) ? (map[1] << nbits) \
       | (map[0] >> (BPERW - nbits)) : map[0] << nbits;\
       dest[0] = (nbits < BPERW) ? map[0] << nbits : 0;}
#define BIT_RIGHT(map,nbits,dest)\
    {dest[0] = (nbits < BPERW) ? (map[0] >> nbits) \
         | (map[1] << (BPERW - nbits)) : map[1] >> nbits;\
	 dest[1] =  (nbits < BPERW) ? map[1] >> nbits : 0;}
#define BIT_AND(map1,map2)\
    {map2[0] = map1[0] & map2[0]; map2[1] = map1[1] & map2[1];}
#define BIT_OR(map1,map2)\
    {map2[0] = map1[0] | map2[0]; map2[1] = map1[1] | map2[1];}
#define BIT_XOR(map1,map2)\
    {map2[0] = map1[0] ^ map2[0]; map2[1] = map1[1] ^ map2[1];}
#define BIT_NOT(map1,dest)\
     {dest[0] = ~map1[0]; dest[1] = ~map1[1];}
#define IFMAPZERO(map) if (map[0] == 0 && map[1] == 0)
#define IFMAPNOTZERO(map) if (map[0] != 0 || map[1] != 0)

