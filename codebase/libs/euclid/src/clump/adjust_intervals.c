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
 *	adjust_intervals
 *
 * PURPOSE
 *	Adjust arrays of intervals according to offsets
 *
 * NOTES
 *	
 *
 * HISTORY
 *     wiener - Jun 23, 1994: Created.
 */

#include <euclid/clump.h>

/*
 * DESCRIPTION:    
 * 	adjust_intervals - adjust an array of intervals by offsets yoff, xoff
 *
 * INPUTS:
 * 	in_row_hdr - input row header of intervals
 *      in_ydim - dimension of in_row_hdr
 *      yoff - offset in y direction or number of rows
 *      xoff - offset in x direction or number of columns
 *
 * The underlying data is assumed to be offset using the intervals 
 * in the following manner:
 *
 * 2D Example:
 *
 * o o o o o <- yoff = 1
 * o x x x o
 * o x x x o
 * o x x x o
 * o o o o o
 * ^          
 * xoff = 1
 *
 * Note that the offsets occur both before and after the array elements
 * corresponding to the intervals!
 *
 * OUTPUTS:
 * 	out_row_hdr - output row header of intervals
 *
 * RETURNS:
 *       number of intervals in out_row_hdr which equals in_ydim + yoff
 *
 * NOTES:
 * 	
 */
int EG_adjust_intervals(Row_hdr *in_row_hdr, int in_ydim, int yoff, int xoff, Row_hdr *out_row_hdr)
{
  int i;
  Interval *iptr;
  int j;
  int out_ydim;

  out_ydim = in_ydim + 2*yoff;

  for (i=0; i<yoff; i++)
    {
      out_row_hdr[i].size = 0;
    }

  for (i=yoff; i<out_ydim-yoff; i++)
    {
      out_row_hdr[i].size = in_row_hdr[i-yoff].size;

      if (out_row_hdr[i].size > 0)
	{
	  out_row_hdr[i].intervals = in_row_hdr[i-yoff].intervals;
/*	  printf("i is %d\n", i);
	  EG_print_intervals(&(in_row_hdr[i-yoff].intervals[0]));
	  EG_print_intervals(&(out_row_hdr[i].intervals[0]));*/
	  iptr = out_row_hdr[i].intervals;
	  for (j=0; j<out_row_hdr[i].size; j++)
	    {
	      iptr[j].row_in_vol += yoff;
	      iptr[j].begin += xoff;
	      iptr[j].end += xoff;
	    }
	}
    }

  for (i=out_ydim-yoff; i<out_ydim; i++)
    {
      out_row_hdr[i].size = 0;
    }

  return(out_ydim);
}
