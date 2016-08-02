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
/* clump_intervals.c - color components using intervals */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <rdi/clump.h>

extern int malloc_amount;

/*
 * clump_2d - assign clump_ids to the clumps in a 2-d data set
 *
 * Algorithm:
 *
 * Organize the intervals for later processing
 *
 * Looping through the elements of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id,
 * use it as a seed in a region filling algorithm, seed_2d.  Seed_2d will
 * set the id for all elements that
 *
 * overlap (Two intervals overlap if they share an identical
 * subinterval.)
 *
 */ 
int clump_2d(intervals, num_intervals, num_rows)
     Interval intervals[];
     int num_intervals;
     int num_rows;
{
  short clump_index;	  /* used for sorting intervals with same clump_ids */
  int curr_row;
  int i;
  int j;
  int num_ints;
  Row_hdr *row_hdr;
  int value;

  /* initialize clump_index */
  clump_index = 0;

  /* initialize the stack */
  if (init_stack() == -1)
    return(-1);

  if (num_rows == 0)
     return(-1);

  if (num_intervals == 0)
     return(0); 

  /* malloc an array of row_hdr */
  row_hdr = malloc_new(num_rows*sizeof(Row_hdr)); 

  if (row_hdr == NULL)
    return(-1);

  /* initialize row header sizes to -1 */
  for (i=0; i<num_rows; i++)
    row_hdr[i].size = -1;

  /* initialize row_hdr */
  curr_row = intervals[0].row;
  row_hdr[curr_row].intervals = &intervals[0];
  for (i=0; i<num_intervals; i++)
    {
      if (curr_row != intervals[i].row)
	{
	  row_hdr[intervals[i].row].intervals = &intervals[i];
	  row_hdr[curr_row].size = row_hdr[intervals[i].row].intervals - 
                                   row_hdr[curr_row].intervals;
	  curr_row = intervals[i].row;
	}
    }
  row_hdr[curr_row].size = 1 + (&intervals[num_intervals-1] - 
                           row_hdr[curr_row].intervals);

  /* find the overlaps */
  overlap_plane(num_rows, row_hdr);

  value = NULL_ID + 1;
  for (i=0; i<num_rows; i++)
    for (j=0; j<row_hdr[i].size; j++)
      {
	if (row_hdr[i].intervals[j].id == NULL_ID)
	  {
	    num_ints = seed_2d(i, j, num_rows, row_hdr, value, 
                               intervals, &clump_index);
	    value++;
	  }
      }
 
  /* free row_hdr */
  free_new(row_hdr, num_rows*sizeof(Row_hdr));

  /* free the stack */
  free_stack();

  return(value-1);
}

/*
 */
int seed_2d(i, j, rows, row_hdr, value, intervals, clump_index)
     int i;
     int j;
     int rows;
     Row_hdr row_hdr[];
     int value;
     Interval intervals[];
     short *clump_index;
{
  int k;
  int y;
  int x;
  int overlap_begin;
  int overlap_end;
  int count = 0;


/*  for (k=0; k<rows; k++)
    printf("row_hdr[%d].intervals[0] = %d\n", k, row_hdr[k].intervals[0]); */

/*  color_point_2d(i, j, row_hdr, value); */
  row_hdr[i].intervals[j].id = value;
  intervals[(*clump_index)++].index = &row_hdr[i].intervals[j] - intervals;
  if (push_2d(i, j) == 0)
    return(-1);

  while (1)
    {
      if (pop_2d(&y, &x))
	{
	  count++;
	  
	  if (y < rows - 1)
	    {
	      overlap_begin = row_hdr[y].intervals[x].overlaps[SOUTH_INTERVAL][OV_BEGIN];
	      overlap_end = row_hdr[y].intervals[x].overlaps[SOUTH_INTERVAL][OV_END];
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (row_hdr[y+1].intervals[k].id == NULL_ID)
/*		    if (color_point_2d(y+1, k, row_hdr, value) == 0) */
		    {
		      row_hdr[y+1].intervals[k].id = value;
		      intervals[(*clump_index)++].index = &row_hdr[y+1].intervals[k] - intervals;
		      if (push_2d(y+1, k) == 0)
			return(-1);
		    }
		}
	    }
	  if (y >= 1)
	    {
	      overlap_begin = row_hdr[y].intervals[x].overlaps[NORTH_INTERVAL][OV_BEGIN];
	      overlap_end = row_hdr[y].intervals[x].overlaps[NORTH_INTERVAL][OV_END];
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (row_hdr[y-1].intervals[k].id == NULL_ID)
/*		    if (color_point_2d(y-1, k, row_hdr, value) == 0) */
		    {
		      row_hdr[y-1].intervals[k].id = value;
		      intervals[(*clump_index)++].index = &row_hdr[y-1].intervals[k] - intervals;
		      if (push_2d(y-1, k) == 0)
			return(-1);
		    }
		}

	    }
	}
      else
	break;
    }
  return(count);
}

/*
 * Color_point_2d() is used for coloring points in contiguous clumps.
 * Currently color_point_2d() does the following:
 *
 * 1.  Colors the point.
 * 2.  Pushes the point onto a stack so its neighbors can be colored
 * later.
 *
 * color_point_2d() could also be used to find the centroid of a region.
 */
int color_point_2d(y, x, row_hdr, value)
     int y;
     int x;
     Row_hdr row_hdr[];
     int value;
{
  row_hdr[y].intervals[x].id = value;
  return(push_2d(y, x));
}


