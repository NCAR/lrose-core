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
/*
 * NAME
 *	erode_clump
 *
 * PURPOSE
 * 	erode points from a clump
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Jul 12, 1994: Created.
 */

#include <euclid/clump.h>

#define MIN_THRESHOLD 1

static char *prog;

/*
 * DESCRIPTION:    
 * 	erode_clump_2d
 *
 * INPUTS:
 *      intervals - array of intervals
 *      num_ints - size of intervals array
 *      xdim - number of columns in underlying array
 *      ydim - number of rows in underlying array
 *      threshold - erode all points within threshold distance of the
 *        boundary if they are not adjacent to points further in from the
 *        boundary
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * OUTPUTS:
 * 	new_clump_array - array of new clumps (This array will also be
 * dimensioned similar to the 2D Example above.
 *      num_clumps - number of output clumps
 *
 * RETURNS:
 *         0 successful
 *       -1 on error
 *
 * NOTES:
 * 	
 */
int EG_erode_clump_2d(Row_hdr *row_hdr, int num_intervals, int xdim, int ydim, int threshold, unsigned char *new_clump_array, int *num_clumps)
{
  Interval **interval_order;
  Clump_order *clump_order;
  Interval *new_intervals;
  unsigned char *edm_array;
  int int_array_size;
  int num_new_ints;
  struct mem_ptr ptr_array[32];
  int ptr_ct;
  int size;
  Row_hdr *new_row_hdr;
  int value;
  int yoff;

  ptr_ct = 0;
  yoff = 1;
  new_intervals = NULL;
  int_array_size = 0;

  /* allocate space for arrays */

  /* allocate space for edm_array */
  size = xdim * ydim;
  edm_array = (unsigned char *)EG_malloc(size);
  ptr_array[ptr_ct].p = edm_array;
  ptr_array[ptr_ct].size = size;
  ptr_ct++;
  if (edm_array == NULL)
    {
      fprintf(stderr, "%s: can't malloc edm_array, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }

  /* apply euclidean distance map */
  printf("calculating euclidean distance map\n");
  EG_edm_2d(row_hdr, edm_array, xdim, ydim, yoff);
/*  EG_print_carray_2d(edm_array, ydim, xdim); */


  /* erode set of points of a particular value */
  for (value=threshold; value>0; value--)
    EG_erode_level_2d(row_hdr, edm_array, xdim, ydim, value);

  /* allocate space for a new row header */
  new_row_hdr = EG_malloc(ydim*sizeof(Row_hdr));
  ptr_array[ptr_ct].p = new_row_hdr;
  ptr_array[ptr_ct].size = ydim*sizeof(Row_hdr);
  ptr_ct++;
  if (new_row_hdr == NULL)
    {
      fprintf(stderr, "%s: can't malloc row_hdr, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* find the intervals over a given threshold distance */
  printf("finding new intervals\n");
  num_new_ints = EG_find_intervals(ydim, xdim, edm_array, &new_intervals, &int_array_size, new_row_hdr, MIN_THRESHOLD);
  ptr_array[ptr_ct].p = new_intervals;
  ptr_array[ptr_ct].size = int_array_size * sizeof(Interval);
  ptr_ct++;

  /* set clump ids in new_intervals to NULL_ID */
  EG_reset_clump_id(new_intervals, int_array_size);
  printf("the number of new intervals = %d\n", num_new_ints);
  /*  print_intervals(new_intervals, num_new_ints); */
  
  /* allocate space for clump_order */
  clump_order = (Clump_order *)EG_malloc((num_intervals+1)*sizeof(Clump_order));
  ptr_array[ptr_ct].p = clump_order;
  ptr_array[ptr_ct].size = (num_intervals+1)*sizeof(Clump_order);
  ptr_ct++;
  if (clump_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc clump_order, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* allocate space for interval_order */
  interval_order = (Interval **)EG_malloc((num_intervals)*sizeof(Interval *));
  ptr_array[ptr_ct].p = interval_order;
  ptr_array[ptr_ct].size = (num_intervals)*sizeof(Interval *);
  ptr_ct++;
  if (interval_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc interval_order, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* reclump the output array to erode the original clump */
  printf("clumping eroded intervals\n");
  *num_clumps = EG_rclump_2d(new_row_hdr,  ydim, 0, 1, interval_order, clump_order);
  
  printf("number of clumps in eroded array =  %d\n", *num_clumps);
  
  /* assign clump values */
  EG_set_intervals_clump(new_clump_array, new_row_hdr, ydim, xdim);
  printf("eroded array is as follows:\n");
/*  EG_print_carray_2d(new_clump_array, ydim, xdim); */

  EG_free_mem(ptr_array, ptr_ct);
  return(0);
}

/*
 * DESCRIPTION:    
 * 	erode_level_2d - Remove all points of a given value that do
 * not have a neighboring point having a value one greater.
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *      int value - test value - points with this value
 *      are candidates.
 *
 * IN_OUT:
 * 	edm_array - this is modified by the erosion process.
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	
 */

void EG_erode_level_2d(Row_hdr *row_hdr, unsigned char *edm_array, int xdim, int ydim, int value)
{
  int begin;
  int end;
  int i;
  Interval *iptr;
  int j;
  int k;
  int row_offset;

  row_offset = xdim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  Check to see that the point's value is equal to the given value.
   * 2.  If so, check to see whether the point has a neighbor of value
   * one greater.  If so, keep the point.  If not erase the point.
   * 3.  Loop
   */
  for (i=0; i<ydim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      
	      if (edm_array[k] == value)
		{
		  if (edm_array[k-1] == value + 1)
		    continue;
		  if (edm_array[k+1] == value + 1)
		    continue;
		  if (edm_array[k-row_offset-1] == value + 1)
		    continue;
		  if (edm_array[k-row_offset] == value + 1)
		    continue;
		  if (edm_array[k-row_offset+1] == value + 1)
		    continue;
		  if (edm_array[k+row_offset-1] == value + 1)
		    continue;
		  if (edm_array[k+row_offset] == value + 1)
		    continue;
		  if (edm_array[k+row_offset+1] == value + 1)
		    continue; 

		  edm_array[k] = 0;
		}
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	erode_lesser_2d - Remove all points of a given value for which
 * all neigbors have lesser values.
 * Similar to erode_level_2d, except the test is >= instead of ==v+1
 *
 * For other details, see erode_below_2d 
 */

void EG_erode_lesser_2d(Row_hdr *row_hdr,
			unsigned char *val_array,
			int xdim, int ydim, int value)
{

  int begin, end;
  int i, j, k;
  Interval *iptr;
  int row_offset = xdim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  Check to see that the point's value is equal to the given value.
   * 2.  If so, check to see whether the point has a neighbor of value
   * one greater.  If so, keep the point.  If not erase the point.
   * 3.  Loop
   */

  for (i=0; i<ydim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      
	      if (val_array[k] == value)
		{

		  if (val_array[k-1] >= value) continue;
		  if (val_array[k+1] >= value) continue;
		  if (val_array[k+row_offset] >= value) continue;
		  if (val_array[k-row_offset] >= value) continue;

		  if (val_array[k-row_offset-1] >= value) continue;
		  if (val_array[k-row_offset+1] >= value) continue;
		  if (val_array[k+row_offset-1] >= value) continue;
		  if (val_array[k+row_offset+1] >= value) continue; 

		  val_array[k] = 0;

		}
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	erode_lesser_or_equal_2d - Remove all points of a given value
 *      for which all neigbors have lesser or equal values.
 * Similar to erode_level_2d, except the test is > instead of ==v+1
 *
 * For other details, see erode_below_2d 
 */

void EG_erode_lesser_or_equal_2d(Row_hdr *row_hdr,
				 unsigned char *val_array,
				 int xdim, int ydim, int value)
{

  int begin, end;
  int i, j, k;
  Interval *iptr;
  int row_offset = xdim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  Check to see that the point's value is equal to the given value.
   * 2.  If so, check to see whether the point has a neighbor of value
   * one greater.  If so, keep the point.  If not erase the point.
   * 3.  Loop
   */

  for (i=0; i<ydim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      
	      if (val_array[k] == value)
		{

		  if (val_array[k-1] > value) continue;
		  if (val_array[k+1] > value) continue;
		  if (val_array[k+row_offset] > value) continue;
		  if (val_array[k-row_offset] > value) continue;
		  if (val_array[k-row_offset-1] > value) continue;
		  if (val_array[k-row_offset+1] > value) continue;
		  if (val_array[k+row_offset-1] > value) continue;
		  if (val_array[k+row_offset+1] > value) continue; 

		  val_array[k] = 0;

		}
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	erode_level_2d - Remove all points of a given value that do
 * not have a neighboring point having a value one greater.
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *      int value - test value - points with this value
 *      are candidates.
 *
 * IN_OUT:
 * 	edm_array - this is modified by the erosion process.
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 	
 */

void EG_erode_below_score_2d(Row_hdr *row_hdr, unsigned char *edm_array, int xdim, int ydim, int value, int min_score)

{
  int begin;
  int end;
  int i;
  Interval *iptr;
  int j;
  int k;
  int row_offset;
  int score, diff;

  row_offset = xdim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  Check to see that the point's value is equal to the given value.
   * 2.  If so, check to see whether the point has a neighbor of value
   * one greater.  If so, keep the point.  If not erase the point.
   * 3.  Loop
   */
  for (i=0; i<ydim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      
	      if (edm_array[k] == value)
		{
		  score = 0;

		  diff = edm_array[k-1] - value + 1;
		  if (diff > 0) score += diff * 2;

		  diff = edm_array[k+1] - value + 1;
		  if (diff > 0) score += diff * 2;

		  diff = edm_array[k-row_offset] - value + 1;
		  if (diff > 0) score += diff * 2;

		  diff = edm_array[k+row_offset] - value + 1;
		  if (diff > 0) score += diff * 2;

		  diff = edm_array[k-row_offset-1] - value + 1;
		  if (diff > 0) score += diff;

		  diff = edm_array[k-row_offset+1] - value + 1;
		  if (diff > 0) score += diff;

		  diff = edm_array[k+row_offset-1] - value + 1;
		  if (diff > 0) score += diff;

		  diff = edm_array[k+row_offset+1] - value + 1;
		  if (diff > 0) score += diff;

		  if (score < min_score) {
		    edm_array[k] = 0;
		  }

		}
	    }
	}
    }
}

/*
 * DESCRIPTION:    
 * 	erode_bridges_2d - remove bridges in grid of width 1
 *      Bridges are single-width connections with a value of
 *      1, and 0's on opposite sides.
 *
 * EXAMPLES:
 *
 *   0000000000             000000000000
 *   0111111110             011110111110
 *   0111111110   vertical  011110111110
 *   0000100000 <- bridge   011111111110
 *   0111111110             011110111110
 *   0111111110             011110111110
 *   0000000000             000000000000
 *                               ^
 *                           Horizontal
 *                             Bridge
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array back
 * to an array without the extra rows and columns of zeroes.
 *
 * INPUTS:
 * 	Row_hdr *row_hdr - array of intervals
 * 	int xdim - x dimension of input data set
 * 	int ydim - y dimension of input data set
 *      are candidates.
 *
 * IN_OUT:
 * 	val_array - this is modified by the erosion process.
 *
 * RETURNS:
 * 	Void.
 *
 * NOTES:
 * 
 * Mike Dixon, May 1995 	
 */

void EG_erode_bridges_2d(Row_hdr *row_hdr,
			 unsigned char *val_array,
			 int xdim, int ydim)
{

  int begin, end;
  int i, j, k;
  Interval *iptr;
  int row_offset = xdim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  If points left and right are both 0, clear point.
   * 2.  If points above and below are both 0, cleat point.
   * 3.  Loop
   *
   */
  for (i=0; i<ydim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      if (val_array[k] == 1)
		{
		  if (val_array[k-1] == 0 && val_array[k+1] == 0) {
		    val_array[k] = 0;
		  } else if (val_array[k+row_offset] == 0 &&
			     val_array[k-row_offset] == 0) {
		    val_array[k] = 0;
		  }
		}
	    }
	}
    }
  return;
}

/*
 * DESCRIPTION:    
 * 	erode_clump_3d
 *
 * INPUTS:
 * 	intervals - array of intervals
 *      num_ints - size of intervals array
 *      xdim - number of columns in underlying array
 *      ydim - number of rows in underlying array
 *      threshold - erode all points within threshold distance of the
 *        boundary if they are not adjacent to points further in from the
 *        boundary
 *
 * The underlying data is assumed to be offset using the intervals array
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 *
 * By staging the data in this manner, the euclidean distance map
 * algorithm is significantly simplified.  A conversion routine,
 * translate_array, is provided to convert the output array parray back
 * to an array without the extra rows and columns of zeroes.
 *
 * OUTPUTS:
 * 	new_clump_array - array of new clumps (This array will also be
 * dimensioned similar to the 2D Example above.
 *      num_clumps - number of output clumps
 *
 * RETURNS:
 *         0 successful 
 *       -1 on error
 *
 * NOTES:
 * 	
 */
int EG_erode_clump_3d(Row_hdr *row_hdr, int num_intervals, int xdim, int ydim, int zdim, int threshold, unsigned char *new_clump_array, int *num_clumps)
{
  Interval **interval_order;
  Clump_order *clump_order;
  Interval *new_intervals;
  unsigned char *edm_array;
  int int_array_size;
  int num_new_ints;
  struct mem_ptr ptr_array[32];
  int ptr_ct;
  int size;
  Row_hdr *new_row_hdr;
  int value;
  int yoff;

  ptr_ct = 0;
  yoff = 1;
  new_intervals = NULL;
  int_array_size = 0;

  /* allocate space for arrays */

  /* allocate space for edm_array */
  size = xdim * ydim * zdim;
  edm_array = (unsigned char *)EG_malloc(size);
  ptr_array[ptr_ct].p = edm_array;
  ptr_array[ptr_ct].size = size;
  ptr_ct++;
  if (edm_array == NULL)
    {
      fprintf(stderr, "%s: can't malloc edm_array, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }

  /* apply euclidean distance map */
  printf("calculating euclidean distance map\n");
  EG_edm_3d(row_hdr, edm_array, xdim, ydim, zdim, yoff);
/*   EG_print_carray_3d(edm_array, xdim, ydim, zdim); */

  /* erode set of points of a particular value and below */
  for (value=threshold; value>0; value--)
    EG_erode_level_3d(row_hdr, edm_array, xdim, ydim, zdim, value);

  /* allocate space for a new row header */
  new_row_hdr = EG_malloc(ydim*zdim*sizeof(Row_hdr));
  ptr_array[ptr_ct].p = new_row_hdr;
  ptr_array[ptr_ct].size = ydim*zdim*sizeof(Row_hdr);
  ptr_ct++;
  if (new_row_hdr == NULL)
    {
      fprintf(stderr, "%s: can't malloc row_hdr, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* find the intervals over a given threshold distance */
  printf("finding new intervals\n");
  num_new_ints = EG_find_intervals(ydim*zdim, xdim, edm_array, &new_intervals, &int_array_size, new_row_hdr, MIN_THRESHOLD);
  ptr_array[ptr_ct].p = new_intervals;
  ptr_array[ptr_ct].size = int_array_size * sizeof(Interval);
  ptr_ct++;

  /* set clump ids in new_intervals to NULL_ID */
  EG_reset_clump_id(new_intervals, int_array_size);
  printf("the number of new intervals = %d\n", num_new_ints);
  /*  print_intervals(new_intervals, num_new_ints); */
  
  /* allocate space for clump_order */
  clump_order = (Clump_order *)EG_malloc((num_intervals+1)*sizeof(Clump_order));
  ptr_array[ptr_ct].p = clump_order;
  ptr_array[ptr_ct].size = (num_intervals+1)*sizeof(Clump_order);
  ptr_ct++;
  if (clump_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc clump_order, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* allocate space for interval_order */
  interval_order = (Interval **)EG_malloc((num_intervals)*sizeof(Interval *));
  ptr_array[ptr_ct].p = interval_order;
  ptr_array[ptr_ct].size = (num_intervals)*sizeof(Interval *);
  ptr_ct++;
  if (interval_order == NULL)
    {
      fprintf(stderr, "%s: can't malloc interval_order, file %s, line %d\n", prog, __FILE__, __LINE__);
      EG_free_mem(ptr_array, ptr_ct);
      return(-1);
    }
  
  /* reclump the output array to erode the original clump */
  printf("clumping eroded intervals\n");
  *num_clumps = EG_rclump_3d(new_row_hdr, ydim, zdim, 0, 1, interval_order, clump_order);
  
  printf("number of clumps in eroded array =  %d\n", *num_clumps);
  
  /* assign clump values */
  EG_set_intervals_clump(new_clump_array, new_row_hdr, ydim*zdim, xdim);
  printf("eroded array is as follows:\n");
/*   (void)print_carray_3d(new_clump_array, xdim, ydim, zdim); */

  EG_free_mem(ptr_array, ptr_ct);
  return(0);
}

/*
 * Remove all points of a given value that do not have a neighboring
 * point having a value one greater.
 */
/*
 * DESCRIPTION:    
 *	
 *
 * IN:
 *	
 *
 * IN-OUT:
 *	
 *
 * OUT:
 *	
 *
 * RETURNS:
 *       
 *
 * NOTES:
 *	
 */

void EG_erode_level_3d(Row_hdr *row_hdr, unsigned char *edm_array, int xdim, int ydim, int zdim, int value)
{
  int begin;
  int end;
  int i;
  Interval *iptr;
  int j;
  int k;
  int plane;
  int row_offset;

  row_offset = xdim;
  plane = xdim * ydim;

  /*
   * for each point in the given set of clumps do the following: 
   *
   * 1.  Check to see that the point's value is equal to the given value.
   * 2.  If so, check to see whether the point has a neighbor of value
   * one greater.  If so, keep the point.  If not erase the point.
   * 3.  Loop
   */
  for (i=0; i<ydim*zdim; i++)
    {
      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];

	  begin = iptr->row_in_vol * xdim + iptr->begin;
	  end = iptr->row_in_vol * xdim + iptr->end;
	  for (k=begin; k<=end; k++)
	    {
	      
	      if (edm_array[k] == value)
		{
		  /*
		   * check the neighboring values on the same plane
		   * including the corners
		   */
		  if (edm_array[k-1] == value + 1)
		    continue;
		  if (edm_array[k+1] == value + 1)
		    continue;
		  if (edm_array[k-row_offset-1] == value + 1)
		    continue;
		  if (edm_array[k-row_offset] == value + 1)
		    continue;
		  if (edm_array[k-row_offset+1] == value + 1)
		    continue;
		  if (edm_array[k+row_offset-1] == value + 1)
		    continue;
		  if (edm_array[k+row_offset] == value + 1)
		    continue;
		  if (edm_array[k+row_offset+1] == value + 1)
		    continue; 

		  /*
		   * check the neighboring values on the plane below
		   * omitting the corners
		   */
		  if (edm_array[k-plane] == value + 1)
		    continue;
		  if (edm_array[k-plane-1] == value + 1)
		    continue;
		  if (edm_array[k-plane+1] == value + 1)
		    continue;
		  if (edm_array[k-plane-row_offset] == value + 1)
		    continue;
		  if (edm_array[k-plane+row_offset] == value + 1)
		    continue;

		  /*
		   * check the neighboring values on the plane above
		   * omitting the corners
		   */
		  if (edm_array[k+plane] == value + 1)
		    continue;
		  if (edm_array[k+plane-1] == value + 1)
		    continue;
		  if (edm_array[k+plane+1] == value + 1)
		    continue;
		  if (edm_array[k+plane-row_offset] == value + 1)
		    continue;
		  if (edm_array[k+plane+row_offset] == value + 1)
		    continue;

		  edm_array[k] = 0;
		}
	    }
	}
    }
}

