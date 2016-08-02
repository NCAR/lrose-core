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
 * find_overlap.c - find all intervals in a control block that overlap a
 * given interval
 */

#include <rdi/clump.h>

/*
 * Inputs:
 * x - the xth interval from the intervals in row_cb1
 * start   - the interval in row_cb2 to start looking for overlap
 * row_cb1 - the interval row control block containing x
 * row_cb2 - the other interval row control block
 *
 * Outputs:
 * overlap_begin - the first interval in dest_row_cb that overlaps x
 * overlap_end	- the last interval in dest_row_cb that overlaps x
 *
 * NOTE: The variable overlap_begin is initialized to be greater than
 * overlap_end.  It will be reset if there is an interval in row_cb2 that
 * overlaps x.
 *
 * Returns:
 * 1 if x overlaps in interval in dest_row_cb
 * 0 otherwise
 */
int find_overlap(x, start, row_hdr1, row_hdr2, overlap_begin, overlap_end)
     int x;
     int start;
     Row_hdr *row_hdr1;
     Row_hdr *row_hdr2;
     int *overlap_begin;
     int *overlap_end;
{
  int j;

  /*
   * Initialize overlap_begin to an impossible value and set overlap_end
   * to the last possible value.
   */
  *overlap_begin = row_hdr2->size+1;
  *overlap_end = row_hdr2->size;

  /* find overlap_begin */
  for (j=start; j<row_hdr2->size; j++)
    {
      switch (overlap(&(row_hdr1->intervals[x]), &(row_hdr2->intervals[j])))
	{
	case -1:
	  /* interval x is before interval j */
	  *overlap_end = j;
	  return(0);

	case 0:
	  /* interval x and interval j overlap */
	  *overlap_begin = j;
	  j++;
	  while (j < row_hdr2->size && overlap(&(row_hdr1->intervals[x]), &(row_hdr2->intervals[j])) == 0)
	    j++;
	  *overlap_end = j-1;
	  return(1);

	case 1:
	  /* interval j is of interval x */
	  continue;
	}
    }
  return(0);
}
