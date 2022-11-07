/* This file contains bitmap-manipulation routines for CHESS.
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
#include "crucialdefs.h"
#include "fixeddefs.h"
#include "externmap.h"
#include "bitmacs.h"

int stacki[64];
int stackip;
extern unsigned int sqbits[];

/*
 * count_bits -- takes a bitmap as an argument
 *               returns a count of the number of bits that are set
 */
count_bits(map)
unsigned int map[2];
{
    register i,j,count;
    count = 0;
    if (NOT_ZERO(map))
      for (i = 0; i < WPERM; i++)
	for (j = 0; j < BPERW; j++)
	  if (sqbits[j] & map[i])
	    count++;
    return(count);
}
/* 
 * next_bit -- takes a bitmap as an argument
 *             modifies the integer stack so that all the bits
 *             turned on in that bitmap are stored on the stack.
 */
next_bit(map)
unsigned int map[2];
{
    register i,j;
    stackip = -1;
    if (map[0] != 0 || map[1] != 0)
      {
	for (i = 0; i < WPERM; i++)    
	  for (j = 0; j < BPERW; j++)
	    if (sqbits[j] & map[i])
	      {
		stacki[++stackip] = j+(BPERW*i);
/*
		printf("stacki element %d = %d\n", 
		       stackip,stacki[stackip]);
*/
	      }
      }
    return(stackip);
}

translmap(color,map,amap)
unsigned int map[NCOLORS][BPERW*WPERM][2];
unsigned int amap[NCOLORS][BPERW*WPERM][2];
{
    register i,j;
    for (i = A1; i <= H8; i++)
      BIT_ZERO(amap[color][i]);
    for (j = A1; j <= H8; j++)
      if (NOT_ZERO(map[color][j]))
	for (i = A1; i <= H8; i++)
	  if (MEMBER(i,map[color][j]))
	    BIT_MSET(j,i,color,amap);
}
