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
 * 	sort_intervals
 *
 * PURPOSE
 * 	Given an array of pointers to intervals, sort into row column
 * order.  The sort is done in place.
 *
 * NOTES
 * 	Use qsort.
 *
 * HISTORY
 *     wiener - Jan 19, 1993: Created.
 */
#include <stdlib.h>
#include <euclid/clump.h>

/* compare intervals in the same row */
int EG_compare_ints_1d(const void *p, const void *q)
{
  Interval **r;
  Interval **s;

  r = (Interval **)p;
  s = (Interval **)q;
  return((*r)->begin - (*s)->begin);
/* stricter return((*r)->begin - (*s)->begin + (*r)->end - (*s)->end); */
}

/* compare intervals in potentially different rows but the same plane */
int EG_compare_ints_2d(const void *p, const void *q)
{
  Interval *r;
  Interval *s;

  r = (Interval *)p;
  s = (Interval *)q;
  if (r->row_in_vol == s->row_in_vol)
    return(r->begin - s->begin);
/* stricter    return(r->begin - s->begin + r->end - s->end); */
  else
    return(r->row_in_vol - s->row_in_vol);
}

/* sort intervals contained in one row into ascending column order */
void EG_sort_ints_1d(Interval **interval_order, int size)
{
  qsort(interval_order, size, sizeof(Interval *), EG_compare_ints_1d);
}

/*
 * sort a 2d array of pointers to intervals into first row then column
 * ascending order
 */
void EG_sort_ints_2d(Interval **interval_order, int size)
{
  qsort(interval_order, size, sizeof(Interval *), EG_compare_ints_2d);
}

/*
 * DESCRIPTION:    
 * 	sort_ints1_2d - like sort_ints, this function sorts an array of
 * pointers to intervals in row-column order, i.e., intervals with
 * smaller row indices precede intervals with larger row indices and
 * intervals having the same row indices are then sorted into ascending
 * column indices.  This function differs from sort_ints_2d in that it links
 * intervals into rows first, then sorts each row whereas sort_ints_2d
 * compares two intervals by comparing both rows and columns.  Hence
 * sort_ints_2d has order n(log(n)) whereas sort_ints1_2d should have order
 * k(log(n/k))+cn where k is the number of rows inhabited by intervals
 * and hence is less than n.
 *
 * INPUTS:
 * 	interval_order - input array of pointers to intervals to be sorted
 *	num_ints - size of interval_order
 *	num_rows - total number of rows in plane containing intervals
 *
 * OUTPUTS:
 * 	out_interval - output array of intervals (not pointers to intervals)
 *
 * RETURNS:
 *       num_ints if successful, -1 on failure
 *
 * NOTES:
 * 	
 */
int EG_sort_ints1_2d(Interval **interval_order, int num_ints, int num_rows, Interval *out_interval)
{
  int i;
  int j;
  Interval **int_array;
  Interval_link_hdr *link_array;
  Interval_link *links;
  int offset;

  /* allocate space for link arrays */
  link_array = EG_malloc(num_rows * sizeof(Interval_link_hdr));
  if (link_array == NULL)
    {
      return(-1);
    }

  links = EG_malloc(num_ints * sizeof(Interval_link));
  if (links == NULL)
    {
      /* free up unnecessary allocated space */
      EG_free(link_array);
      return(-1);
    }

  /* link the intervals in interval_order_array to appropriate rows */
  EG_link_pintervals(interval_order, num_ints, num_rows, link_array, links);
  
  /* allocate an array of pointers to intervals to dump the links */
  int_array= EG_malloc(num_ints * sizeof(Interval *));
  if (int_array == NULL)
    {
      /* free up unnecessary allocated space */
      EG_free(link_array);
      EG_free(links);
      return(-1);
    }

  /*
   * for each row, dump the links, sort the resulting intervals into
   * ascending order and store the intervals in the output array
   */
  offset = 0;
  for (i=0; i<num_rows; i++)
    {
      if (link_array[i].size == 0)
	continue;

      /* dump links for one row - this is why 1 is used in the call below */
      EG_dump_links(&link_array[i], 1, int_array);

      /* sort each row into interval ascending order */
      EG_sort_ints_1d(int_array, link_array[i].size);

      /* copy intervals to output array */
      for (j=0; j<link_array[i].size; j++)
	{
	  out_interval[offset] = *int_array[j];
	  offset++;
	}
    }

  (void)EG_free(link_array);
  (void)EG_free(links);
  (void)EG_free(int_array);

  /* we should have processed num_ints intervals so check it */
  if (offset != num_ints)
    return(-1);
  else
    return(offset);
}

/* compare intervals in potentially different rows and different planes */
int EG_compare_ints_3d(const void *p, const void *q)
{
  Interval *r;
  Interval *s;

  r = (Interval *)p;
  s = (Interval *)q;
  if (r->plane == s->plane)
    {
      if (r->row_in_vol == s->row_in_vol)
	return(r->begin - s->begin);
      else
	return(r->row_in_vol - s->row_in_vol);
    }
  else
    return(r->plane - s->plane);
}

/*
 * sort a 3d array of pointers to intervals into first plane, then row
 * then column ascending order
 */
void EG_sort_ints_3d(Interval **interval_order, int size)
{
  qsort(interval_order, size, sizeof(Interval *), EG_compare_ints_3d);
}

