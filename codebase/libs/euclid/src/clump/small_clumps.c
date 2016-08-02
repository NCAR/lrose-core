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
/* small_clumps.c - reset array values for small clumps */

#include <stdio.h>
#include <sys/types.h>
#include <euclid/clump.h>

int
EG_small_clumps(unsigned char *array, int num_rows, int num_cols, int num_planes, int clump_threshold, int small_clump_size, int value)
{
  int i;
  int isize;
  Interval *intervals = NULL;
  int num_clumps;
  int num_ints;
  Row_hdr *row_hdr;
  Interval **interval_order = NULL;
  Clump_order *clump_order = NULL;
  int reject_clumps = 0;

  if (num_rows <= 0 || num_cols <= 0)
    {
      return(-1);
    }
    
  /* malloc row_hdr */
  row_hdr = (Row_hdr *)EG_malloc(num_rows*num_planes*sizeof(Row_hdr));
  if (row_hdr == NULL)
    {
      return(-1);
    }
  
  isize = 0;

  /* find the intervals in array */
  num_ints = EG_find_intervals(num_rows*num_planes, num_cols, array, &intervals, &isize, row_hdr, clump_threshold);
  
  printf("num_ints %d\n", num_ints);

  /* allocate space for interval_order and clump_order arrays */
  if ((interval_order = EG_malloc(num_ints * sizeof(Interval *))) == NULL)
	return(-1);
  if ((clump_order = EG_malloc((num_ints+1)*sizeof(Clump_order))) == NULL)
	return(-1);

  /* reset arrays */
  EG_reset_arrays(intervals, interval_order, clump_order, num_ints);

  /* print the row_hdr */
/*  EG_print_row_hdr(row_hdr, num_rows);  */

  /* find clumps */
  num_clumps = EG_rclump_3d(row_hdr, num_rows, num_planes, 0, 1, interval_order, clump_order);

  printf("num_clumps %d\n", num_clumps);
/*  num_clumps = EG_iclump_3d(intervals, num_intervals, num_rows, num_planes, 0, 1, interval_order, clump_order); */


  /* reset the values of the small clumps */
  for (i=1; i<=num_clumps; i++)
    {
/*      printf("i %d, pts %d, size %d, sm %d\n", i, clump_order[i].pts, clump_order[i].size, small_clump_size); */
      if (clump_order[i].pts  < small_clump_size)
	{
	  EG_set_intervals(array, num_cols, clump_order[i].ptr, clump_order[i].size, value);
	  reject_clumps++; 
	}
    }

/*  for (i=0; i<num_rows*num_cols*num_planes; i++)
    {
      printf("array[%d] = %d\n", i, array[i]);
    }
*/
  /* free up allocated space */
  EG_free(interval_order);
  EG_free(clump_order);
  EG_free(row_hdr);
  EG_free(intervals);

  return(num_clumps-reject_clumps);
}
  

