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
 * Module: intervals_to_array.c
 *
 * Author: Gerry Wiener
 *
 * Date:   1/30/98
 *
 * Description:  Given an matrix and an array of intervals for the
 *      matrix, copy the elements of the matrix into an output array.
 *      Return the size of the output array.
 */

#include <euclid/clump.h>

int EG_intervals_to_arrayf
(
 float *in_matrix,	/* I - input matrix */
 Interval **intervals,	/* I - input array of interval pointers */
 int size,		/* I - size of input array */
 int ncols,		/* I - number of columns in entire matrix (not just in plane) */
 float *out_array	/* O - output array */
)
{
  int ct;
  int i;
  Interval *iptr;
  int j;
  int len;
  
  ct = 0;
  for (i=0; i<size; i++)
    {
      iptr = intervals[i];
      len = iptr->end - iptr->begin + 1;
      for (j=0; j<len; j++)
	out_array[ct+j] = in_matrix[iptr->row_in_vol * ncols + iptr->begin + j];
      ct += len;
    }

  return(ct);
}
