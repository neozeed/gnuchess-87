/* This file contains utilities for CHESS.
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

struct timeval timcpu;
struct rusage rubuf;

timeon()
{
    getrusage(RUSAGE_SELF,&rubuf);
    timcpu.tv_sec = rubuf.ru_utime.tv_sec;
    timcpu.tv_usec = rubuf.ru_stime.tv_usec;
}

timeoff()
{
    getrusage(RUSAGE_SELF,&rubuf);
    timcpu.tv_sec = rubuf.ru_utime.tv_sec - timcpu.tv_sec;
    timcpu.tv_usec = rubuf.ru_utime.tv_usec - timcpu.tv_usec;
}

setrowcol(sq,specrow,speccol)
int *specrow,*speccol;
{
    *specrow = sq / 8 + 1;
    *speccol = sq % 8 + 1;
}
