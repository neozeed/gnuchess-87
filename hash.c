/* This file contains the hashing algorithms for CHESS.
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

#ifdef _WIN32
#define random rand
#define srandom srand
#endif

long numhash = 0;
long rands[(MAXPC*2)+1][120],hashval;
#ifdef HASH
struct hashentry hashtbl[HASHTBLSZ];
#endif HASH
struct bdtype randsbd[120];
extern struct bdtype bd[120];

#ifdef HASH
/*
 * hstore stores a given move on the given board made at the given depth
 * into the in-memory transposition table.
 */
hstore(bd,depth,move)
struct bdtype bd[120];
int depth;
struct mvlist move;
{
    long index,match;
    index = hashval&0177777;
    match = hashval>>16;
#ifdef HASHDEBUG
    printf("Store: index = %ld, match = %ld [index].depth = %d\n",index,match,hashtbl[index].depth);
#endif HASHDEBUG
    if (numhash < HASHTBLSZ)
      {
	while (hashtbl[index].depth != -1)
	    index = (index + 1) % HASHTBLSZ;
#ifdef HASHDEBUG
	printf("Stored at %ld:\n",match);
	pboard(bd,FALSE);
	printf("match = %ld, depth = %ld, score = %ld\n",match,depth,move.score);
	printf("movpiece = %d, cappiece = %d, propiece = %d\n",move.movpiece,move.cappiece,move.propiece);
	printf("move = ");
	lin_to_alg(move.from,stdout);
	lin_to_alg(move.to,stdout);
	putchar('\n');
#endif HASHDEBUG
	hashtbl[index].match = match;
	hashtbl[index].depth = depth;
	hashtbl[index].move.score = move.score;
	hashtbl[index].move.flags = move.flags;
	hashtbl[index].move.from = move.from;
	hashtbl[index].move.to = move.to;
	hashtbl[index].move.movpiece = move.movpiece;
	hashtbl[index].move.cappiece = move.cappiece;
	hashtbl[index].move.propiece = move.propiece;
	numhash++;
      }
}

/*
 * retrieve extracts the given move on the given board from
 * the in-memory transposition table.
 */
retrieve(bd,tmove)
struct bdtype bd[120];
struct hashentry *tmove;
{
    long index,match,numchecked = 0;
    index = hashval&0177777;
    match = hashval>>16;
    if (hashtbl[index].depth == -1)
      {
	tmove->depth = -1;
	return;
      }
    while (hashtbl[index].depth != -1 && hashtbl[index].match != match &&
	   numchecked < HASHTBLSZ)
	index = (index + 1) % HASHTBLSZ;
    if (match == hashtbl[index].match)
      {
#ifdef HASHDEBUG
	printf("Retrieved. Score = %d, Depth = %d. Move = ",
		hashtbl[index].move.score,
		hashtbl[index].depth);
	lin_to_alg(hashtbl[index].move.from,stdout);
	lin_to_alg(hashtbl[index].move.to,stdout);
	putchar('\n');
	pboard(bd,FALSE);
#endif HASHDEBUG
	tmove->depth = hashtbl[index].depth;
	tmove->match = hashtbl[index].match;
	tmove->move.score = hashtbl[index].move.score;
	tmove->move.flags = hashtbl[index].move.flags;
	tmove->move.from = hashtbl[index].move.from;
	tmove->move.to = hashtbl[index].move.to;
	tmove->move.movpiece = hashtbl[index].move.movpiece;
	tmove->move.cappiece = hashtbl[index].move.cappiece;
	tmove->move.propiece = hashtbl[index].move.propiece;
      }
}

/*
 * list_hash lists in-memory transposition table in
 * a human readable format.
 */
list_hash()
{
    long i;
    printf("Hash table has %d entries and is %4.2f%% full:\n",numhash,
	(float)numhash/(float)HASHTBLSZ);
    for (i = 0; i < HASHTBLSZ; i++)
	if (hashtbl[i].depth != -1)
	{
	    printf("Index = %ld, Depth = %d, Move = ",i,hashtbl[i].depth);
	    lin_to_alg(hashtbl[i].move.from,stdout);
	    lin_to_alg(hashtbl[i].move.to,stdout);
	    printf(", Score = %d, Flag = %s\n",hashtbl[i].move.score,
		(hashtbl[i].move.flags&VALID) ? "VALID" : (
		(hashtbl[i].move.flags&LBOUND) ? "LBOUND" : (
		(hashtbl[i].move.flags&UBOUND) ? "UBOUND" : "NONE")));
	}
}

/*
 * clear_hash clears the in-memory transposition table
 */
clear_hash()
{
    long i;
    numhash = 0;
    for (i = 0; i < HASHTBLSZ; i++)
    {
      hashtbl[i].depth = -1;
      hashtbl[i].match = -1;
    }
}
#endif HASH

/*
 * init_rands initializes random numbers for each type of piece,
 * which are then used for creating hash values for the given board
 * as the XOR of all the corresponding chess pieces's corresponding
 * random numbers, each new move being simply the XOR of that piece's
 * random number into the overall hash value.
 * (cf. Gillogly, Performance Analysis of the Technology Chess Program,
 *      pp 12-13, Department of Computer Science, Carnegie-Mellon
 *      University (March 1978).
 *      Zobrist, "A new hashing method with application for game playing",
 *      U. Wis. Computer Sciences Department TR #88 (Apr 1970)).
 */
init_rands()
{
    int i,j;
    hashval = 0;
    srandom(1);
    srandom(getpid());
    for (j = WP; j <= WK; j++)
      for (i = 21; i < 99; i++)
	if ((i % 10 != 0) && (i % 10 != 9))
	  {
	    rands[-j+6][i] = random();
	    rands[j+6][i] = random();
	  }
}

/*
 * init_hash initalizes the hash value for the given position
 * using the random numbers associated with each piece.
 */
init_hash(bd)
struct bdtype bd[120];
{
    int i;
    hashval = 0;
    for (i = 21; i < 99; i++)
        if ((i % 10 != 0) && (i % 10 != 9) && (bd[i].piece != EMP))
	    hashval ^= rands[bd[i].piece+6][i];
}
