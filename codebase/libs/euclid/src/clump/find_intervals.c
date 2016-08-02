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
 * find_intervals.c - find intervals greater than threshold in an
 * array corresponding to a plane or volume of data
 */

#include <stdio.h>
#include <sys/types.h>
#include <euclid/clump.h>
#include <euclid/alloc.h>

#define MIN_INTV_ALLOC 4096

typedef enum intv_mode_2d {ABOVE_THRESH, BELOW_THRESH, WITHIN_AUX} intv_mode_2d_t;

/*
 * static find_intervals_2d() - generic 2d search routine.
 *
 * Has 3 modes:
 *  ABOVE_THRESH - finds intervals above threshold
 *  BELOW_THRESH - finds intervals below threshold
 *  WITHIN_AUX - finds intervals above threshold within bounds set by
 *               previous search
 *
 * Allocates array space to save these intervals and copies the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane
 *     	  or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	aux_row_hdr - row_hdr from previous call to EG_find_intervals(),
 *        set to NULL except for WITHIN_AUX mode
 * 	threshold - the given threshold
 *      mode
 *
 * Outputs:
 * 	intervals_p - pointer to intervals array
 * 	n_intv_alloc_p - number of allocated intervals in array
 *      row_hdr records the interval arrays and their sizes for each row
 *
 * Returns:
 * 	the number of intervals found
 */

static int find_intervals_2d(int nrows, int ncols, unsigned char array[],
			     Interval **intervals_p, int *n_intv_alloc_p,
			     Row_hdr *row_hdr, Row_hdr *aux_row_hdr,
			     int threshold, int mode)

{

  int n_intv_needed;
  int n_intv_alloc;
  int count = 0;
  int nalloc;
  int i, j, k;
  int nintvls;
  Interval *intvl, *intervals;
  Row_hdr *rhdr;
  Row_hdr *aux_row_ptr;
  
  nintvls = 0;
  intervals = *intervals_p;
  n_intv_alloc = *n_intv_alloc_p;

  /*
   * make sure that the array of intervals is large enough for storing
   * intervals
   */
  
  nalloc = MAX(ncols, MIN_INTV_ALLOC);
  n_intv_needed = nalloc;
  
  if (n_intv_alloc < n_intv_needed)
    {
      n_intv_alloc = n_intv_needed;
      if ((intervals = EG_realloc((void *)intervals,
				  n_intv_alloc*sizeof(Interval)))== NULL)
	{
	  return(-1);
	}
    }
  
  /* for each row, determine its intervals */

  rhdr = row_hdr;
  for (i=0; i<nrows; i++, rhdr++)
    {
      /* adjust the size of array intervals if necessary */
      n_intv_needed = nintvls + ncols;
      if (n_intv_alloc < n_intv_needed)
	{
	  n_intv_alloc += nalloc;
	  if ((intervals = EG_realloc((void *)intervals,
				      n_intv_alloc*sizeof(Interval)))== NULL)
	    {
	      return(-1);
	    }
	}
      
      /* determine the intervals in row i and store them in intervals */

      switch (mode) {
      case ABOVE_THRESH:
	count = EG_get_intervals(array+(ncols*i), 0, ncols-1,
				 intervals + nintvls, threshold);
	break;

      case BELOW_THRESH:
	count = EG_get_intervals_below(array+(ncols*i), 0, ncols-1,
				       intervals + nintvls, threshold);
	break;
	
      case WITHIN_AUX:
	aux_row_ptr = &aux_row_hdr[i];
	count = 0;
	for (k=0; k<aux_row_ptr->size; k++)
	  {
	    /*
	     * Points above threshold must be in the intervals determined by
	     * aux_cb_ptr.  Hence, look for new intervals in this array of
	     * intervals.
	     */
	    
	    /* get the set of intervals in this row */
	    intvl = &(aux_row_ptr->intervals[k]);
	    count += EG_get_intervals(array+(ncols*i),
				      intvl->begin, intvl->end,
				      intervals + nintvls + count, threshold); 
	  }
	break;

      } /* switch (mode) */
      
      /*
       * If intervals greater than threshold were found, store
       * information in row_hdr
       */
      if ((rhdr->size = count) > 0)
	{
	  intvl = intervals + nintvls;
	  for (j=0; j<count; j++, intvl++)
	    {
	      intvl->row_in_vol = i;
	      intvl->row_in_plane = i;
	      intvl->plane = 0;
	      intvl->len = intvl->end - intvl->begin + 1;
	      intvl->id = 0;
	    }

	  nintvls += count;
	}

    } /* i */

  /*
   * set row_hdr start_interval pointers - this is deferred to this point
   * in case a realloc causes a change in pointer values
   */

  nintvls = 0;
  rhdr = row_hdr;
  for (i = 0; i < nrows; i++, rhdr++)
    {
      if (rhdr->size > 0)
	rhdr->intervals = intervals + nintvls;
      else
	rhdr->intervals = NULL;

      nintvls += rhdr->size;
    }
  
  *n_intv_alloc_p = n_intv_alloc;
  *intervals_p = intervals;
  return(nintvls);

}

/*
 * int EG_find_intervals()
 *
 * For each row in a plane or volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane
 *     	  or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

int EG_find_intervals(int nrows, int ncols, unsigned char array[],
		      Interval **intervals_p, int *n_intv_alloc_p,
		      Row_hdr *row_hdr, int threshold)

{

  return(find_intervals_2d(nrows, ncols, array,
			   intervals_p, n_intv_alloc_p,
			   row_hdr, (Row_hdr *) NULL,
			   threshold, ABOVE_THRESH));

}

/*
 * int EG_find_intervals_below()
 *
 * For each row in a plane or volume, determine the intervals that are <
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane
 *     	  or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

int EG_find_intervals_below_(int nrows, int ncols, unsigned char array[],
			     Interval **intervals_p, int *n_intv_alloc_p,
			     Row_hdr *row_hdr, int threshold)

{

  return(find_intervals_2d(nrows, ncols, array,
			   intervals_p, n_intv_alloc_p,
			   row_hdr, (Row_hdr *) NULL,
			   threshold, BELOW_THRESH));

}

/*
 * int EG_find_intervals_aux()
 *
 * This function is equivalent to find_intervals() except that it use an
 * auxiliary interval array determined by aux_row_hdr to assist in
 * determining the intervals above threshold.  For each row in plane,
 * determine the intervals that are above threshold.  Use an auxiliary
 * interval array for speedup.  Allocate array space to save these
 * intervals and copy the interval information to the allocated array.
 *
 * Inputs:
 * 	array - array of unsigned chars 
 * 	nrows - total number of rows (in plane or volume)
 * 	ncols - column dimension 
 * 	intervals_p - pointer to intervals array realloc'd if necessary
 * 	n_intv_alloc_p - initial size of intervals array
 * 	row_hdr - storage for the intervals above threshold for
 *        each row (dim nrows)
 * 	aux_row_hdr - auxiliary row header
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - intervals found are stored here 
 * 	row_hdr - the interval arrays and their sizes for each row
 *
 * Returns:
 * 	the number of intervals found
 */

int EG_find_intervals_aux(int nrows, int ncols, unsigned char array[],
			  Row_hdr *aux_row_hdr,
			  Interval **intervals_p, int *n_intv_alloc_p,
			  Row_hdr *row_hdr, int threshold)

{

  return(find_intervals_2d(nrows, ncols, array,
			   intervals_p, n_intv_alloc_p,
			   row_hdr, aux_row_hdr,
			   threshold, WITHIN_AUX));

}

/*
 * int EG_find_intervals_3d()
 * 
 * For each row in a volume, determine the intervals that are >=
 * threshold.  Allocate array space to save these intervals and copy the
 * interval information to the allocated array.
 *
 * Inputs:
 * 	nrows - row dimension (number of total rows in plane or volume)
 * 	ncols - column dimension 
 * 	array - array of unsigned chars 
 *      intervals_p - pointer for interval array (pointer must point to
 *        the beginning of a block of allocated space or be NULL)
 * 	n_intv_alloc_p - size of intervals array if not NULL
 * 	row_hdr - the intervals above threshold for each row (dim nrows)
 *        should be preallocated to be of nrows size
 * 	threshold - the given threshold
 *
 * Outputs:
 * 	intervals - pointer to intervals array
 * 	n_intv_alloc_p - size of intervals array
 * 	the interval arrays and their sizes for each row are recorded in
 * 	row_hdr
 *
 * Returns:
 * 	the number of intervals found
 */

int EG_find_intervals_3d(int nplanes_in_vol, int nrows_in_vol, int nrows_in_plane,
			 int ncols, unsigned char array[], Interval **intervals_p,
			 int *n_intv_alloc_p, Row_hdr *row_hdr, int threshold)
{

  int n_intv_needed;
  int n_intv_alloc;
  int count;
  int nalloc;
  int i, j;
  int iplane, irow_in_plane;
  int nintvls;

  Interval *intvl, *intervals;
  Row_hdr *rhdr;

  nintvls = 0;
  intervals = *intervals_p;
  n_intv_alloc = *n_intv_alloc_p;

  /*
   * make sure that the array of intervals is large enough for storing
   * intervals
   */

  nalloc = MAX(ncols, MIN_INTV_ALLOC);
  n_intv_needed = nalloc;
  
  if (n_intv_alloc < n_intv_needed)
    {
      n_intv_alloc = n_intv_needed;
      if ((intervals = EG_realloc((void *)intervals,
				  n_intv_alloc*sizeof(Interval)))== NULL)
	{
	  return(-1);
	}
    }
  
  /* for each row, determine its intervals */

  rhdr = row_hdr;
  for (iplane = 0; iplane < nplanes_in_vol; iplane++)
    {
      for (irow_in_plane = 0; irow_in_plane < nrows_in_plane; irow_in_plane++, rhdr++)
	{

	  i = iplane * nrows_in_plane + irow_in_plane;

	  /* adjust the size of array intervals if necessary */
	  n_intv_needed = nintvls + ncols;
	  if (n_intv_alloc < n_intv_needed)
	    {
	      n_intv_alloc += nalloc;
	      if ((intervals = EG_realloc((void *)intervals,
					  n_intv_alloc*sizeof(Interval)))== NULL)
		{
		  return(-1);
		}
	    }
	  
	  /* determine the intervals in row i and store them in intervals */
	  count = EG_get_intervals(array+(ncols*i), 0, ncols-1,
				   intervals+nintvls, threshold);

	  /*
	   * If intervals greater than threshold were found, store
	   * information in row_hdr
	   */
	  if ((rhdr->size = count) > 0)
	    {
	      /*
	       * set the row index for each
	       * interval in the intervals array
	       */
	      intvl = intervals + nintvls;
	      for (j=0; j<count; j++, intvl++)
		{
		  intvl->row_in_vol = i;
		  intvl->row_in_plane = irow_in_plane;
		  intvl->plane = iplane;
		  intvl->len = intvl->end - intvl->begin + 1;
		}
	      
	      nintvls += count;
	    }
	  
	} /* irow_in_plane */
    } /* iplane */

  /*
   * set row_hdr start_interval pointers - this is deferred to this point
   * in case a realloc causes a change in pointer values
   */

  nintvls = 0;
  rhdr = row_hdr;
  for (i = 0; i < nplanes_in_vol * nrows_in_plane; i++, rhdr++)
    {
      if (rhdr->size > 0)
	rhdr->intervals = intervals + nintvls;
      else
	rhdr->intervals = NULL;

      nintvls += rhdr->size;
    }
  
  *n_intv_alloc_p = n_intv_alloc;
  *intervals_p = intervals;
  return(nintvls);

}

/*******************************************
 * EG_free_intervals()
 *
 * Free intervals allocated in this module.
 *
 */

void EG_free_intervals(Interval **intervals_p, int *n_intv_alloc_p)

{

  if (*intervals_p) {
    EG_free(*intervals_p);
  }
  *intervals_p = NULL;
  *n_intv_alloc_p = 0;

}
