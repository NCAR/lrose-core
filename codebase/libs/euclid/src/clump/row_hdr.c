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
 *	row_hdr
 *
 * PURPOSE
 *	Generate row_hdr for interval array
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Feb 9, 1993: Created.
 */
#include <stdio.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	make_row_hdr - generate a row header structure for an array of
 * intervals (similar to gen_row_hdr but does not allocate row header array
 *
 * IN:
 * 	intervals - array of intervals
 * 	num_intervals - size of intervals array
 * 	ydim - total number of rows covered by the intervals array (may
 * 	cover multiple planes)
 *
 * OUT:
 * 	row_hdr - contains output row_hdr array
 *
 * RETURNS:
 *      size of row_hdr array or -1 on failure
 *
 * NOTES:
 *      The intervals are assumed to be sorted according to row, column order.
 */
int
EG_make_row_hdr(Interval *intervals, int num_intervals, int ydim, Row_hdr *row_hdr)
{
  int curr_row;
  int i;
  int new_row;

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
  
  return(curr_row);
}

/*
 * DESCRIPTION:    
 *	gen_row_hdr - generate a row header structure for an array of intervals
 *
 * INPUTS:
 *	intervals - array of intervals
 *	num_intervals - size of intervals array
 *	ydim - total number of rows covered by the intervals array (may
 *	cover multiple planes)
 *
 * OUTPUTS:
 *	prow_hdr - contains output row_hdr array
 *
 * RETURNS:
 *      size of row_hdr array or -1 on failure
 *
 * NOTES:
 *      This function allocates space for a row_hdr array.  To free this space
 *      call EG_free(row_hdr, ydim*sizeof(Row_hdr)) or
 *      free(row_hdr, ydim*sizeof(Row_hdr)) if you wish to use the ordinary
 *     allocation and free system calls.
 *
 */
int
EG_gen_row_hdr(Interval *intervals, int num_intervals, int ydim, Row_hdr **prow_hdr, int *prow_hdr_size)
{
  int curr_row;
  int i;
  int min_size;
  int new_row;
  Row_hdr *row_hdr;
  int row_hdr_size;

  /* organize the intervals using a row_hdr array */

  row_hdr = *prow_hdr;
  row_hdr_size = *prow_hdr_size;

  /* reallocate output array if necessary */
  min_size = ydim;
  if (row_hdr_size < min_size)
    {
      row_hdr = (Row_hdr *)EG_realloc(row_hdr, min_size * sizeof(Row_hdr));
      if (row_hdr == NULL)
	return(-1);
      
      row_hdr_size = min_size;
    }

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
  
  *prow_hdr = row_hdr;
  *prow_hdr_size = row_hdr_size;
  return(curr_row);
}

int
EG_gen_srow_hdr(Sinterval *intervals, int num_intervals, int ydim, Srow_hdr **prow_hdr)
{
  int curr_row;
  int i;
  int new_row;
  Srow_hdr *row_hdr;

  /* organize the intervals using a row_hdr array */

  /* malloc an array of row_hdr */
  row_hdr = (Srow_hdr *)EG_malloc(ydim*sizeof(Row_hdr)); 

  if (row_hdr == NULL)
    return(-1);

  /* initialize row header sizes to -1 for all rows */
  for (i=0; i<ydim; i++)
    row_hdr[i].size = -1;

  /* initialize row_hdr */
  curr_row = intervals[0].row;
  row_hdr[curr_row].intervals = &intervals[0];
  for (i=0; i<num_intervals; i++)
    {
      if (curr_row != intervals[i].row)
	{
	  new_row = intervals[i].row;
	  row_hdr[new_row].intervals = &intervals[i];
	  row_hdr[curr_row].size = row_hdr[new_row].intervals - row_hdr[curr_row].intervals;
	  curr_row = new_row;
	}
    }
  row_hdr[curr_row].size = 1 + (&intervals[num_intervals-1] - row_hdr[curr_row].intervals);

  *prow_hdr = row_hdr;
  return(curr_row);
}

#ifdef NOTNOW
/*
 * DESCRIPTION:    
 * 	translate_row_hdr - translate a row_hdr
 *
 * INPUTS:
 *
 * OUTPUTS:
 * 	
 *
 * RETURNS:
 *       
 *
 * METHOD:
 * 	
 */
void EG_translate_row_hdr(Row_hdr *row_hdr, Row_hdr **new_row_hdr, int ydim, int zdim, int new_xdim, int new_ydim, int xoff, int yoff, int zoff)
{
  int i;
  int j;
  Interval *iptr;
  int loc;
  int new_row;
  int nrows;

  new_xdim = xdim + 2*xoff;
  new_ydim = ydim + 2*yoff;
  new_zdim = zdim + 2*zoff;

  /* allocate the new row header */
  nrows = new_ydim * new_zdim;

  size = 0;
  for (i=0; i<ydim*zdim; i++)
    {
      /* determine the size of the interval array to allocate */
      size += row_hdr[i].size;
    }

  /* allocate the interval array */
  
  offset = 0;
  for (i=0; i<ydim*zdim; i++)
    {
      /* translate the row */
      new_row = i % ydim + (i/ydim + zoff) * new_ydim + yoff;

      if (row_hdr[i].size > 0)
	{

	  /* attach the pointer */
	  new_row_hdr[new_row].size = row_hdr[i].size;
	  new_row_hdr[new_row].intervals = &new_intervals[offset];
	  offset += row_hdr[i].size;

	  /* set the interval array */
	  for (j=0; j<row_hdr[i].size; j++)
	    {
	      EG_new_row_hdr[new_row].intervals[j] = row_hdr[i].intervals[j];
	      
	}
      else
	EG_new_row_hdr[new_row].size = 0;
	}
    }
  for (i=0; i<nrows; i++)
    {

      for (j=0; j<row_hdr[i].size; j++)
	{
	  iptr = &row_hdr[i].intervals[j];
	  loc = new_row*new_xdim + iptr->begin + xoff;

/*	  printf("row %d, begin %d, end %d\n", iptr->row, iptr->begin, iptr->end);
	  printf("loc is %d\n", loc); */
	  memset (&buffer[loc], (int)value, iptr->end - iptr->begin + 1);
	}
    }
}
#endif




