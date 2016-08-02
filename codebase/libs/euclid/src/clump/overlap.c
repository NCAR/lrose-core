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
/* overlap.c - when do two intervals overlap? */

#include <sys/types.h>
#include <euclid/clump.h>

/*
 * overlap() returns info on whether two intervals overlap.
 *
 * If min_overlap is +n, then the intervals must overlap by n
 * grid positions.
 * If min_overlap is 0, the intervals 'overlap' if their corners 
 * touch diagonally.
 * If min_overlap is -n, the intervals are considered to
 * overlap if there is a gap of n grid positions or less between them.
 *
 * If interval1 is before interval2 including min overlap
 * 	return -1
 *
 * If interval1 and interval2 overlap including min overlap
 * 	return  0
 *
 * If interval1 is after interval2 including min overlap
 * 	return  1
 */

int EG_overlap(Interval *interval1_ptr, Interval *interval2_ptr, int min_overlap)
{

  int delta = 1 - min_overlap;

  if (interval1_ptr->begin < interval2_ptr->begin)
    {
      if (interval1_ptr->end < interval2_ptr->begin - delta)
	return(-1);
      else
	return(0);
    }
  else
    {
      if (interval2_ptr->end < interval1_ptr->begin - delta)
	return(1);
      else
	return(0);
    }

}


