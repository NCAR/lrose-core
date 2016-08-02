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
 * 	scan_interval
 *
 * PURPOSE
 * 	Convert scan format to interval format.
 *
 * NOTES
 *
 *
 * HISTORY
 * 	wiener - Apr 29, 1992: Created.
 */

#include <euclid/clump.h>
#include <euclid/scan.h>

/*
 * DESCRIPTION:    
 *	Convert an array of interval data from scan to interval format
 *
 * INPUTS:
 *	array - array of intervals
 *	dim   - array dimension
 *	scan  - scan information
 *
 * OUTPUTS:
 *	row_hdr - array of interval control blocks
 *	pinterval - pointer to array of intervals used by row_hdr
 *
 * RETURNS:
 *       
 *
 * METHOD:
 *	
 */
int EG_scan_interval(int *array, int array_dim, Scan_struct *scan, Row_hdr **prow_hdr, int *prow_hdr_size, Interval **pinterval, int *pinterval_size)
{
  int i;
  Row_hdr *row_hdr;
  int row_hdr_size;
  Interval *iptr = NULL;
  Interval *int_ptr;
  int iy;
  int j;
  int min_size;
  int n_intervals;
  int offset;
  int tot_intervals;
  
  
  printf("in scan_interval\n");
  /* organize the intervals using a row_hdr array */
  int_ptr = *pinterval;
  row_hdr = *prow_hdr;
  row_hdr_size = *prow_hdr_size;
  
  /* reallocate row_hdr array if necessary */
  min_size = scan->ny;
  printf("min_size is %d, row_hdr_size is %d\n", min_size, row_hdr_size);
  if (row_hdr_size < min_size)
    {
      row_hdr = (Row_hdr *)EG_realloc(row_hdr, min_size * sizeof(Row_hdr));
      if (row_hdr == NULL)
	return(-1);
      
      row_hdr_size = min_size;
    }
  
  /* initialize row header sizes */
  for (i=0; i<scan->ny; i++)
    row_hdr[i].size = 0;

  i = 0;
  tot_intervals = 0;
  while (i < array_dim)
    {
      /* read iy and n_intervals */
      iy = array[i];
      n_intervals = array[i+1];
      row_hdr[iy].size = n_intervals;
      i += 2;
      
      tot_intervals += n_intervals;
      
      /* reallocate the interval array if necessary */
      if (*pinterval_size < tot_intervals)
	{
	  int_ptr = (Interval *)EG_realloc(int_ptr, tot_intervals * sizeof(Interval));
	  if (int_ptr == NULL)
	    return(-1);
	  else
	    *pinterval_size = tot_intervals;
	}

      iptr = &int_ptr[tot_intervals - n_intervals];
      
      /* set the interval array */
      for (j=0; j<n_intervals; j++)
	{
	  iptr[j].begin = array[i++];
	  iptr[j].end = array[i++];
	  iptr[j].row_in_vol = iy;
	}
    }

  /* set the pointers to row_hdr (cannot be done in the previous loop due to reallocation) */
  offset = 0;
  for (i=0; i<scan->ny; i++)
    {
      row_hdr[i].intervals = &int_ptr[offset];
      offset += row_hdr[i].size;
    }


  *prow_hdr = row_hdr;
  *prow_hdr_size = row_hdr_size;
  *pinterval = int_ptr;
  
  return(tot_intervals);
} /* scan_interval */


