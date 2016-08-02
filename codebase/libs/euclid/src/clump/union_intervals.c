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
 * 	union_int.c
 *
 * PURPOSE
 * 	Find the union of two arrays of intervals.
 *
 * NOTES
 *
 * HISTORY
 *     wiener - Jan 12, 1993: Created.
 */

#include <stdio.h>
#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	union_row - given an array of pointers to intervals all of
 * which belong to one row, form the union of the intervals
 *
 * INPUTS:
 * 	interval_order - array of pointers to intervals
 * 	size - size of interval_order array
 *
 * OUTPUTS:
 * 	out - output intervals after taking union (the dimension of
 * the output array should be at least as big as the input array and must
 * be allocated by the user)
 *
 * RETURNS:
 *       size of output array or -1 on error
 *
 * NOTES:
 * 	the input array is assumed to be sorted in ascending order
 */

int EG_union_row(Interval **interval_order, int size, Interval *out)
{
  int ct;
  int i;
  int ov_res;
  
  ct = 0;
  out[ct] = *interval_order[0];
/*  EG_print_interval(&out[ct]); */
  for (i=1; i<size; i++)
    {
      /* if intervals overlap, merge them */
      ov_res = EG_overlap(&out[ct], interval_order[i], 1);
/*      printf("i %d\n", i);
      EG_print_interval(&out[ct]);
      EG_print_interval(interval_order[i]); */
      switch (ov_res)
	{
	case -1:
	  /* out[ct] is to the left of interval_order[i] */
/*	  printf("-1 happened\n");
	  EG_print_interval(&out[ct]);
	  EG_print_interval(interval_order[i]); */
	  ct++;
	  out[ct] = *interval_order[i];
	  break;
	  
	case 0:
/*	  printf("0 happened\n"); */
	  /*
	   * out[ct] overlaps interval_order[i] - if the intervals are
	   * different, form their union
	   */
	  if (out[ct].end < interval_order[i]->end)
	    {
	      out[ct].end = interval_order[i]->end;
	    }
	  break;
	  
	case 1:
	  /* out[ct] is to the right of interval_order[i] */
/*	  ct++;
	  out[ct] = *interval_order[i]; */

	  EG_print_interval(&out[ct]);
	  EG_print_interval(interval_order[i]); 
	  fprintf(stderr, "union_row(): overlay error\n");
	  return(-1); 
	  break;
	}
    }
  ct++;
  return(ct);
}

/*
 * DESCRIPTION:    
 *	union_int_2d -  Find the union of two arrays of intervals.
 *
 * INPUTS:
 *	interval_order1 - array of intervals sorted according to row and column
 *	interval_order2 - array of intervals sorted according to row and column
 *
 * OUTPUTS:
 *	interval_order3 - array of intervals sorted according to row and column
 *
 * RETURNS:
 *      size of interval_order3
 *
 * METHOD:
 * 	Given two arrays of pointers to intervals with respective
 * sizes.  Record the union of the two arrays in a third array and return
 * the sum of sizes.  If two intervals from the two arrays overlap and
 * are in the same row, merge them.  The intervals in the input arrays
 * must be sorted according to increasing row and increasing column.
 *	
 */
int EG_union_int_2d(Interval **interval_order1, int size1, Interval **interval_order2, int size2, Interval **interval_order3)
{
#ifdef NOTNOW /* needs revision */
  int i;
  int j;
  int k;
  int ov_res;
  
  for (i=0, j=0, k=0; i<size1, j<size2; k++)
    {
      if (interval_order1[i]->row < interval_order2[j]->row)
	{
	  *interval_order3[k] = *interval_order1[i];
	  i++;
	}
      else if (interval_order1[i]->row == interval_order2[j]->row)
	{
	  /* if intervals overlap, merge them */
	  ov_res = overlap(interval_order1[i], interval_order2[j]);
	  switch (ov_res)
	    {
	    case -1:
	      /* interval_order1[i] is to the left of interval_order2[j] */
	      *interval_order3[k] = *interval_order1[i];
	      i++;
	      break;
	      
	    case 0:
	      /* interval_order1[i] overlaps interval_order2[j] */
	      interval_order3[k]->begin = MIN(interval_order1[i]->begin, interval_order2[j]->begin);
	      interval_order3[k]->end = MAX(interval_order1[i]->end, interval_order2[j]->end);
	      interval_order3[k]->row = interval_order1[i]->row;
	      i++;
	      j++;
	      break;
	      
	    case 1:
	      /* interval_order1[i] is to the right of interval_order2[j] */
	      *interval_order3[k] = *interval_order2[j];
	      j++;
	      break;
	    }
	}
      else
	{
	  interval_order3[k] = interval_order2[j];
	  j++;
	}
    }
      
  /* clean up for remaining elements */
  if (i < size1)
    {
      for (; i<size1; i++)
	{
	  interval_order3[k] = interval_order1[i];
	  k++;
	}
    }
  else if (j < size2)
    {
      for (; j<size2; j++)
	{
	  interval_order3[k] = interval_order2[j];
	  k++;
	}
    }

  return(k);
#endif
  return (0);
}





