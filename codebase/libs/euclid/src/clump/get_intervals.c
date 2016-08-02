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
/* get_intervals.c - get intervals in a row or interval */

#include <stdio.h>
#include <sys/types.h>
#include <euclid/clump.h>

/*
 * Determine the set of intervals between begin and end in the array row
 * which are above threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */
int
EG_get_intervals(unsigned char row[],
		 int begin,
		 int end,
		 Interval interval_array[],
		 int threshold)
{
  int count;
  int j;

/*  printf("get_intervals: begin = %d, end = %d\n", begin, end);  */

  /* get the set of intervals in this row */
  count = 0;
  j = begin;
  while (1)
    {
      /*
       * advance to the first array element greater than or equal to
       * threshold
       */
      while (j <= end && row[j] < threshold)
	j++;
      
      /* determine whether we have a true beginning of the interval */
      if (j <= end)
	{
	  interval_array[count].begin = j;
/*	  printf("begin is %d, elt is %d\n", j, row[j]); */
	}
      else
	break;
      
      /* determine the end of the interval */
      while (j <= end && row[j] >= threshold)
	j++;
      
      interval_array[count].end = j-1;

      count++;
    }
  return(count);
}     

/*
 * Determine the set of intervals between begin and end in the array row
 * which are below threshold and store the results in interval_array.
 *
 * Inputs:
 *  	row 		- array of bytes to examine
 * 	begin		- beginning of data to examine in row
 * 	end		- end of data to examine in row
 *  	threshold	- the given threshold
 *
 * Outputs:
 * 	the intervals found are stored in interval_array
 *
 * Returns:
 *  	the number of intervals found
 */
int
EG_get_intervals_below(unsigned char row[],
		      int begin,
		      int end,
		      Interval interval_array[],
		      int threshold)
{
  int count;
  int j;

/*  printf("get_intervals: begin = %d, end = %d\n", begin, end);  */

  /* get the set of intervals in this row */
  count = 0;
  j = begin;
  while (1)
    {
      /*
       * advance to the first array element less than threshold
       */
      while (j <= end && row[j] >= threshold)
	j++;
      
      /* determine whether we have a true beginning of the interval */
      if (j <= end)
	{
	  interval_array[count].begin = j;
/*	  printf("begin is %d, elt is %d\n", j, row[j]); */
	}
      else
	break;
      
      /* determine the end of the interval */
      while (j <= end && row[j] < threshold)
	j++;
      
      interval_array[count].end = j-1;

      count++;
    }
  return(count);
}     



