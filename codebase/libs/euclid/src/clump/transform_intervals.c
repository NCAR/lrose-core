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
 *	transform_intervals
 *
 * PURPOSE
 *	Transform a set of intervals by a translation of (xoff, yoff)
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Jun 17, 1994: Created.
 */

#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 *	transform_interval_2d
 *
 * IN:
 *	in_row_hdr - input row header
 *      xoff - positive translation of x
 *      yoff - positive translation of y
 *
 * OUT:
 *	out_row_hdr - output row header
 *
 * RETURNS:
 *       
 *
 * NOTES:
 *	
 */


void EG_transform_interval_2d(Row_hdr *in_row_hdr, int ydim, int xoff, int yoff, Row_hdr *out_row_hdr , Interval *intervals)
{
  int count;
  int i;
  int j;

  count = 0;
  for (i=0; i<ydim; i++)
    {
      out_row_hdr[i+yoff].size = in_row_hdr[i].size;
      out_row_hdr[i+yoff].intervals = &intervals[count];
      for (j=0; j<in_row_hdr[i].size; j++)
	{
	  intervals[count+j].id = in_row_hdr[i].intervals[j].id;
	  intervals[count+j].row_in_vol = in_row_hdr[i].intervals[j].row_in_vol + yoff;
	  intervals[count+j].begin = in_row_hdr[i].intervals[j].begin + xoff;
	  intervals[count+j].end = in_row_hdr[i].intervals[j].end + xoff;
	}
      count += in_row_hdr[i].size;
    }
}

#ifdef NOTNOW
  int id;			/* clump id */
  short overlaps[4][2]; /* overlaps[i][0] - index of begin overlap, overlaps[i][1] - index of end overlap */
  short plane;			/* plane index of interval */
  short row;		    /* row index of interval in volume - not row index in plane if there are multiple planes */
  short begin;		    /* interval begins in this column */
  short end;		     /* interval ends in this column */
#endif

