/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
#ifdef __cplusplus                                                        
 extern "C" {                                                         
#endif
/* clump.h - structures related to clumps */

#ifndef CLUMP_H
#define CLUMP_H

#define CLUMP_INCR 1          /* increment for realloc of clump_array */
#define MAX_STACK 16384		/* initial max stack size */
#define NORTH_INTERVAL 0     /* used for overlapping intervals */
#define NULL_ID 0		        /* clump ids should be initialized to this value */
#define OV_BEGIN 0		      /* index for overlap array */
#define OV_END 1		        /* index for overlap array */
#define OVERLAP_DIM 2		 /* outer dimension of overlap array*/
#define SOUTH_INTERVAL 1     /* used for overlapping intervals */
#define STACK_INCR 4096		 /* increment for realloc of stack */

typedef struct interval
{
  short id;			/* clump id */
  short index;			/* used for miscellaneous indexing */
  short overlaps[OVERLAP_DIM][2]; /* overlaps[i][0] - index of begin overlap, overlaps[i][1] - index of end overlap */
  short row;		    /* row index of interval */
  short begin;		    /* interval begins in this column */
  short end;		     /* interval ends in this column */
} Interval;

typedef struct row_hdr /* row header */
{
  int size;			/* size of the intervals array below */
  Interval *intervals;	  /* array of intervals (pointers allocated later) */
} Row_hdr;

void *realloc_new();

void free_new();

void *malloc_new();

/* routines in clump_intervals.c */

int seed_2d(int i, int j, int rows, Row_hdr row_hdr[], int value,
	    Interval intervals[], short *clump_index);

int clump_2d(Interval intervals[], int num_intervals, int num_rows);

/* routines in find_overlap.c */

int find_overlap(int x, int start, Row_hdr *row_hdr1, Row_hdr *row_hdr2,
		 int *overlap_begin, int *overlap_end);

/* routines in overlap.c */

int overlap(Interval *interval1_ptr, Interval *interval2_ptr);

/* routines in overlap_plane.c */

void overlap_plane(int ydim, Row_hdr *row_hdr);

/* routines in overlap_rows.c */

void overlap_rows(Row_hdr *row_hdr1, Row_hdr *row_hdr2, int direct);

/* routines in stack_interval.c */

int init_stack(void);
void free_stack(void);
int push_2d(int x, int y);
int pop_2d(int *x, int *y);

#endif
#ifdef __cplusplus             
}
#endif
