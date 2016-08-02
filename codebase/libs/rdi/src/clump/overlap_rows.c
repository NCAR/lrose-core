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
 * overlap_rows.c - find the overlap of intervals in two different rows
 */

#include <rdi/clump.h>

/*
 * ALGORITHM:
 * 	
 * 1.  Get the next interval, next_interal, in row_cb1.
 * 2.  Find the set of intervals it overlaps in row_cb2.
 * 3.  Record the (begin, end) indices of the overlapping intervals from
 *     row_cb2 in the appropriate direction subfield of next_interval
 * 4.  Go back to 1.
 *
 * INPUTS:
 *
 * row_cb1 - the first interval row control block
 * row_cb2 - the second interval row control block
 * direct  - the direction to update.  This is a subfield of the Interval
 * structure.  0 - north, 1 - south, 2 - up, 3 - down
 *
 * OUTPUTS:
 *
 * the intervals array in the interval row control block row_cb1 are
 * adjusted to reflect the overlap of intervals of row_cb1 and row_cb2
 *
 * RETURNS:
 *
 * no return
 */
void overlap_rows(row_hdr1, row_hdr2, direct)
     Row_hdr *row_hdr1;
     Row_hdr *row_hdr2;
     int direct;
{
  int i;
  int overlap_begin;
  int overlap_end;
  
  int start_index;

  start_index = 0;
  for (i=0; i<row_hdr1->size; i++)
    {
      find_overlap(i, start_index, row_hdr1, row_hdr2, &overlap_begin, &overlap_end);
      row_hdr1->intervals[i].overlaps[direct][0] = overlap_begin;
      row_hdr1->intervals[i].overlaps[direct][1] = overlap_end;
      start_index = overlap_end;
    }
}
