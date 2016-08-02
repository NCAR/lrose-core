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
/* clump_intervals.c - identify components using intervals */

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <euclid/clump.h>
#include <euclid/alloc.h>

/*
 * DESCRIPTION:    
 *
 * EG_iclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 * num_intervals - the number of intervals in "intervals"
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

int EG_iclump_2d(Interval *intervals, int num_intervals,
		int ydim, int clear, int min_overlap,
		Interval **interval_order, Clump_order *clump_order)

{
  int interval_index;	  
  int curr_row;
  int i;
  int j;
  int new_row;
  int old_index;
  Row_hdr *row_hdr;
  int value;

  /* initialize interval_index */
  interval_index = 0;

  /* initialize the stacks */
  if (EG_init_stack_2d() == -1)
    return(-1);

  /* clear interval id's if required */
  if (clear) {
    EG_reset_clump_id(intervals, num_intervals);
  }
  
  /* organize the intervals using a row_hdr array */

  /* malloc an array of row_hdr */
  row_hdr = EG_malloc(ydim*sizeof(Row_hdr)); 

  if (row_hdr == NULL)
    return(-1);

  /* initialize row header sizes to -1 for all rows */
  for (i=0; i<ydim; i++)
    row_hdr[i].size = -1;

  /* initialize row_hdr */
  curr_row = intervals[0].row_in_vol;
  row_hdr[curr_row].intervals = &intervals[0];
  for (i=0; i<num_intervals; i++)
    {
      if (curr_row != intervals[i].row_in_vol)
	{
	  new_row = intervals[i].row_in_vol;
	  row_hdr[new_row].intervals = &intervals[i];
	  row_hdr[curr_row].size = row_hdr[new_row].intervals - row_hdr[curr_row].intervals;
	  curr_row = new_row;
	}
    }
  row_hdr[curr_row].size = 1 + (&intervals[num_intervals-1] - row_hdr[curr_row].intervals);

  /* find the overlaps */
  EG_overlap_plane(ydim, row_hdr, min_overlap);

  value = NULL_ID + 1;
  for (i=0; i<ydim; i++)
    for (j=0; j<row_hdr[i].size; j++)
      {
	if (row_hdr[i].intervals[j].id == NULL_ID)
	  {
	    old_index = interval_index;
	    clump_order[value].ptr = &interval_order[interval_index];
	    clump_order[value].pts = EG_seed_2d(i, j, ydim, row_hdr, value,  &interval_index, interval_order);
	    clump_order[value].size = interval_index - old_index;
	    value++;
	  }
      }
  
  /* free row_hdr */
  (void)EG_free(row_hdr);

  /* free the stacks */
  EG_free_stack_2d();

  return(value-1);
}

/*
 * DESCRIPTION:    
 *
 * EG_rclump_2d - assign clump_ids to clump of data points in a 2d data set.
 * The 2d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 2d data set.  The function
 * rclump_2d differs from iclump_2d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_2d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * ydim - the number of rows in the plane or equivalently, the dimension
 * of row_hdr 
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 2-d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_2d.  Seed_2d will set the id for all intervals
 * that overlap in adjacent rows.
 */

int EG_rclump_2d(Row_hdr *row_hdr, int ydim, int clear, int min_overlap,
	      Interval **interval_order, Clump_order *clump_order)

{
  int interval_index;
  int i;
  int j;
  int old_index;
  int value;

  /* initialize interval_index */
  interval_index = 0;

  /* initialize the stack */
  if (EG_init_stack_2d() == -1)
    return(-1); 

  /* clear interval id's */
  if (clear) {
    for (i=0; i<ydim; i++) {
      for (j=0; j<row_hdr[i].size; j++) {
	row_hdr[i].intervals[j].id = NULL_ID;
      }
    }
  }

  /* find the overlaps */
  EG_overlap_plane(ydim, row_hdr, min_overlap);

  value = NULL_ID + 1;
  for (i=0; i<ydim; i++)
    for (j=0; j<row_hdr[i].size; j++)
      {
	if (row_hdr[i].intervals[j].id == NULL_ID)
	  {
	    old_index = interval_index;
	    clump_order[value].ptr = &interval_order[interval_index];
	    clump_order[value].pts = EG_seed_2d(i, j, ydim, row_hdr, value,  &interval_index, interval_order);
	    clump_order[value].size = interval_index - old_index;
	    value++;
	  }
      }
  
  /* free the stack */
  EG_free_stack_2d();

  return(value-1);
}

/*
 * DESCRIPTION:    
 * 	EG_seed_2d - find all intervals that are 1-connected to a particular
 * interval (see paper by Wiener, Goodrich, Dixon)
 *
 * INPUTS:
 * 	i - row index of initial seed
 * 	j - column index of initial seed
 * 	ydim - number of rows in 2d array
 * 	row_hdr - array of structures organizing intervals in a given row 
 * 	value - clump id for this seed
 * 	interval_index - index used for sorting intervals according to clump
 * 	interval_order - array ordering intervals according to clump
 *
 * OUTPUTS:
 * 	row_hdr - various subfields in the intervals fields are set
 * 	  including clump_id
 * 	interval_index - increased for each interval added to a clump
 * 	interval_order - set for each interval added to a clump
 *
 * RETURNS:
 *       The number of intervals in the clump or -1 on failure.
 *
 * METHOD:
 * 	Starting at a seed interval, classify all intervals in row_hdr
 * that are 1-connected to it using a seed-fill approach.
 */
int EG_seed_2d(int i, int j, int ydim, Row_hdr row_hdr[], int value, int *interval_index, Interval **interval_order)
{
  int k;
  int y;
  int x;
  int overlap_begin;
  int overlap_end;
  int count = 0;
  Row_hdr *rh_base;
  Row_hdr *rh_ptr;

/*  for (k=0; k<ydim; k++)
    printf("row_hdr[%d].intervals[0] = %d\n", k, row_hdr[k].intervals[0]); */

/*  color_point_2d(i, j, row_hdr, value); */
  row_hdr[i].intervals[j].id = value;
  if (EG_push_2d(i, j) == 0)
    return(-1);
  interval_order[(*interval_index)++] = &row_hdr[i].intervals[j];
/*  printf("interval_index is %d\n", *interval_index); */

  while (1)
    {
      if (EG_pop_2d(&y, &x))
	{
	  rh_base = &row_hdr[y];
	  count += rh_base->intervals[x].end - rh_base->intervals[x].begin + 1;
	  
	  if (y < ydim - 1)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[SOUTH_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[SOUTH_INTERVAL][OV_END];
	      rh_ptr = &row_hdr[y+1];
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
/*		    if (EG_color_point_2d(y+1, k, row_hdr, value) == 0) */
		    {
		      rh_ptr->intervals[k].id = value;

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
/*		      printf("interval_index is %d\n", *interval_index); */
		      if (EG_push_2d(y+1, k) == 0)
			return(-1);
		    }
		}
	    }
	  if (y >= 1)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[NORTH_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[NORTH_INTERVAL][OV_END];
	      rh_ptr = &row_hdr[y-1];
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
/*		    if (EG_color_point_2d(y-1, k, row_hdr, value) == 0) */
		    {
		      rh_ptr->intervals[k].id = value;

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
/*		      printf("interval_index is %d\n", *interval_index); */
		      if (EG_push_2d(y-1, k) == 0)
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
 * DESCRIPTION:    
 *
 * EG_iclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.
 *
 * INPUTS:
 *
 * intervals - an array of intervals (see clump.h for definition)
 *
 * num_intervals - the number of intervals in "intervals"
 *
 * zdim - the number of planes in the volume
 *
 * ydim - the number of rows in each plane of the volume
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval in "intervals" will be set
 * with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */

int EG_iclump_3d(Interval *intervals, int num_intervals,
	       int ydim, int zdim, int clear, int min_overlap,
	       Interval **interval_order, Clump_order *clump_order)

{
  int interval_index;	  
  int curr_row;
  int i;
  int j;
  int k;
  int new_row;
  int old_index;
  Row_hdr *rh_ptr;
  Row_hdr *row_hdr;
  int value;

  /* initialize interval_index */
  interval_index = 0;

  /* initialize the stack */
  if (EG_init_stack_3d() == -1)
    return(-1);

  /* clear interval id's if required */

  if (clear) {
    EG_reset_clump_id(intervals, num_intervals);
  }

  /* organize the intervals using a row_hdr array */

  /* malloc an array of row_hdr */
  row_hdr = EG_malloc(zdim*ydim*sizeof(Row_hdr)); 

  if (row_hdr == NULL)
    return(-1);

  /* initialize row header sizes to -1 for all planes and rows */
    for (i=0; i<zdim*ydim; i++)
      row_hdr[i].size = -1;

  /* initialize row_hdr */
  curr_row = intervals[0].row_in_vol;
  row_hdr[curr_row].intervals = &intervals[0];
  for (i=0; i<num_intervals; i++)
    {
      if (curr_row != intervals[i].row_in_vol)
	{
	  new_row = intervals[i].row_in_vol;
	  row_hdr[new_row].intervals = &intervals[i];
	  row_hdr[curr_row].size = row_hdr[new_row].intervals - row_hdr[curr_row].intervals;
	  curr_row = new_row;
	}
    }

  /* set the last row's interval array size specially */
  row_hdr[curr_row].size = 1 + (&intervals[num_intervals-1] - row_hdr[curr_row].intervals);

  /* find the overlaps */
  EG_overlap_volume(zdim, ydim, row_hdr, min_overlap);

  value = NULL_ID + 1;

  for (i=0; i<zdim; i++)
    for (j=0; j<ydim; j++)
      {
	rh_ptr = &(row_hdr[i*ydim+j]);
	for (k=0; k<rh_ptr->size; k++)
	  {
	    if (rh_ptr->intervals[k].id == NULL_ID)
	      {
		old_index = interval_index;
		clump_order[value].ptr = &interval_order[interval_index];
		clump_order[value].pts = EG_seed_3d(i, j, k, zdim, ydim, row_hdr, value, &interval_index, interval_order);
		clump_order[value].size = interval_index - old_index;
		value++;
	      }
	  }
      }

  /* free row_hdr */
  (void)EG_free(row_hdr);

  /* free the stack */
  EG_free_stack_3d();

  return(value-1);
}

/*
 * DESCRIPTION:    
 *
 * EG_rclump_3d - assign clump_ids to clump of data points in a 3d data set.
 * The 3d data set is determined by an array of intervals.  An interval
 * is a run of nonzero data in a row of the 3d data set.  The function
 * rclump_3d differs from iclump_3d since the user must set up a row
 * header structure which provides organization to the set of intervals.
 * The function iclump_3d does this automatically for the user.
 *
 * INPUTS:
 *
 * row_hdr - an array of structures.  Each structure contains an array of
 * intervals along with the size of the array.  The array of intervals
 * corresponds to a row of intervals.
 *
 * zdim - the number of planes in the volume
 * ydim - the number of rows in each plane
 *
 * clear - this indicates that the interval id's should be cleared prior
 * starting. This should usually be set TRUE (1). It should only be
 * FALSE (0) if the user wishes to clump intervals which have been added
 * since this routine was last called.
 *
 * min_overlap - 
 *   If min_overlap is +n, then the intervals must overlap by n
 *   grid positions.
 *   If min_overlap is 0, the intervals 'overlap' if their corners 
 *   touch diagonally.
 *   If min_overlap is -n, the intervals are considered to
 *   overlap if there is a gap of n grid positions or less between them.
 *
 * OUTPUTS:
 *
 * intervals - the id field for each interval pointed to by row_hdr will
 * be set with an appropriate clump identification number.
 *
 * interval_order - this array is organized according to clump and
 * contains pointers to all intervals as they were found in the clumps.
 * For example the first clump may consist of intervals with indices 1,
 * 5, 3 and the second clump may consist of intervals with indices 2, 4,
 * 6.  Thus interval_order would contain pointers to intervals 1, 5, 3,
 * 2, 4, 6.
 * interval_order must be allocated by the calling routine,
 * to size ((num_intervals+1) * sizeof(Interval *)).
 *
 * clump_order - this array contains the size of each clump as well as an
 * integer pointer to the beginning of each clump in interval_order
 * clump_order must be allocated by the calling routine,
 * to size ((num_intervals + 1) * sizeof(Clump_order)).
 *
 * If one wishes to list the intervals in the clump with a particular id
 * say n, then size = clump_order[n].size will be the size of the clump
 * and ptr = clump_order[n].ptr will point to the first interval in this
 * clump in interval_order i.e., clump_order[n].ptr thru
 * clump_order[n].ptr + size-1 will point to the intervals in clump n.
 *
 * NOTE THAT CLUMP IDS BEG_IN AT 1 NOT 0.
 *
 * RETURNS:
 *
 * The number of clumps.
 *
 * METHOD:
 *
 * Looping through the intervals of a 3d array of intervals, do the
 * following:
 *
 * If an interval has a null clump_id, use it as a seed in a region
 * filling algorithm, seed_3d.  Seed_3d will set the id for all intervals
 * that overlap in adjacent rows.
 */
int EG_rclump_3d(Row_hdr *row_hdr, int ydim, int zdim,
		int clear, int min_overlap,
		Interval **interval_order, Clump_order *clump_order)
{

  int interval_index;	  
  int i;
  int j;
  int k;
  Row_hdr *rh_ptr;
  int old_index;
  int offset;
  int value;

  /* initialize interval_index */
  interval_index = 0;

  /* initialize the stack */
  if (EG_init_stack_3d() == -1)
    return(-1);

  /* clear the interval id's */

  if (clear) {
    for (i=0; i<zdim; i++)
      {
	offset = i*ydim;
	for (j=0; j<ydim; j++)
	  {
	    rh_ptr = &(row_hdr[offset+j]);
	    for (k=0; k<rh_ptr->size; k++)
	      {
		rh_ptr->intervals[k].id = NULL_ID;
	      }
	  }
      }
  }

  /* find the overlaps */
  EG_overlap_volume(zdim, ydim, row_hdr, min_overlap);

  value = NULL_ID + 1;

  for (i=0; i<zdim; i++)
    {
      offset = i*ydim;
      for (j=0; j<ydim; j++)
	{
	  rh_ptr = &(row_hdr[offset+j]);
	  for (k=0; k<rh_ptr->size; k++)
	    {
	      if (rh_ptr->intervals[k].id == NULL_ID)
		{
		  old_index = interval_index;
		  clump_order[value].ptr = &interval_order[interval_index];
		  clump_order[value].pts = EG_seed_3d(i, j, k, zdim, ydim, row_hdr, value, &interval_index, interval_order);
		  clump_order[value].size = interval_index - old_index;
		  value++;
		}
	    }
	}
    }

  /* free the stack */
  EG_free_stack_3d(); 

  return(value-1);
}

/*
 * DESCRIPTION:    
 * 	EG_seed_3d - find all intervals that are 1-connected to a particular
 * interval (see paper by Wiener, Goodrich, Dixon)
 *
 * INPUTS:
 * 	i - plane index of initial seed
 * 	j - row index of initial seed
 * 	k - column index of initial seed
 *      zdim - number of planes in volume
 *	ydim - number of rows in each plane
 * 	row_hdr - array of structures organizing intervals in a given row 
 * 	value - clump id for this seed
 * 	interval_index - index used for sorting intervals according to clump
 * 	interval_order - array ordering intervals according to clump
 *
 * OUTPUTS:
 * 	row_hdr - various subfields in the intervals fields are set
 * 	  including clump_id
 * 	interval_index - increased for each interval added to a clump
 * 	interval_order - set for each interval added to a clump
 *
 * RETURNS:
 *       The number of intervals in the clump or -1 on failure.
 *
 * METHOD:
 * 	Starting at a seed interval, classify all intervals in row_hdr
 * that are 1-connected to it using a seed-fill approach.
 */
int EG_seed_3d(int i, int j, int k, int zdim, int ydim, Row_hdr *row_hdr, int value, int *interval_index, Interval **interval_order)
{
  int z;
  int y;
  int x;
  int overlap_begin;
  int overlap_end;
  int count;
  Row_hdr *rh_base;
  Row_hdr *rh_ptr;

  count = 0;
/*  color_point_3d(i, j, k, row_hdr, value); */
  rh_base = &row_hdr[i*ydim+j];
  rh_base->intervals[k].id = value;
  EG_push_3d(i, j, k);
  interval_order[(*interval_index)++] = &rh_base->intervals[k];

  while (1)
    {
      if (EG_pop_3d(&z, &y, &x))
	{
	  rh_base = &row_hdr[z * ydim + y];
	  count += rh_base->intervals[x].end - rh_base->intervals[x].begin + 1;
	  if (y+1 < ydim)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[SOUTH_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[SOUTH_INTERVAL][OV_END];
	      rh_ptr = rh_base + 1;
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
		    {
/*		      EG_color_point_3d(z, y+1, k, row_hdr, value); */
		      rh_ptr->intervals[k].id = value;
		      EG_push_3d(z, y+1, k);

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
		    }
		}
	    }
	  if (y-1 >= 0)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[NORTH_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[NORTH_INTERVAL][OV_END];
	      rh_ptr = rh_base-1;
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
		    {
/*		      EG_color_point_3d(z, y-1, k, row_hdr, value); */
		      rh_ptr->intervals[k].id = value;
		      EG_push_3d(z, y-1, k);

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
		    }
		}

	    }
	  if (z+1 < zdim)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[UP_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[UP_INTERVAL][OV_END];
	      rh_ptr = rh_base + ydim;
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
		    {
/*		      EG_color_point_3d(z+1, y, k, row_hdr, value); */
		      rh_ptr->intervals[k].id = value;
		      EG_push_3d(z+1, y, k);

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
		    }
		}
	    }
	  if (z-1 >= 0)
	    {
	      overlap_begin = rh_base->intervals[x].overlaps[DOWN_INTERVAL][OV_BEG_IN];
	      overlap_end = rh_base->intervals[x].overlaps[DOWN_INTERVAL][OV_END];
	      rh_ptr = rh_base - ydim;
	      for (k=overlap_begin; k<=overlap_end; k++)
		{
		  if (rh_ptr->intervals[k].id == NULL_ID)
		    {
/*		      EG_color_point_3d(z-1, y, k, row_hdr, value); */
		      rh_ptr->intervals[k].id = value;
		      EG_push_3d(z-1, y, k);

		      /*
		       * store the offset of the interval which was just
		       * clumped for easy retrieval of clump intervals by
		       * calling functions
		       */
		      interval_order[(*interval_index)++] = &rh_ptr->intervals[k];
		    }
		}

	    }
	}
      else
	break;
    }
  return(count);
}

