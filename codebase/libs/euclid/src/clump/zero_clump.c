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
 * Module: zero_clump.c
 *
 * Author: Gerry Wiener
 *
 * Date:   6/26/96
 *
 * Description:
 *     Zero out data in grid that belong to small clumps.
 */

#include <stdio.h>
#include <euclid/clump.h>
#include <euclid/boundary.h>

void OEG_set_interval_float
(
 float *grid,			/* I/O - grid */
 int grid_nx,			/* I - number of columns in grid */
 Interval *iptr,		/* I - intervals to set */
 float val			/* I - value to set */
)
{
  int i;
  int offset;

  offset = iptr->row_in_vol * grid_nx;
  for (i=offset + iptr->begin; i<=offset + iptr->end; i++)
    {
      grid[i] = val;
    }
}

/* zero out clumps that are
   a.  too small
   b.  not strong enough in magnitude
   c.  horizontal lines
   d.  vertical lines
   */
int OEG_zero_clump_float
(
 float *grid,			/* I/O - input/output grid */
 OClump_info *ci,		/* I - clump information structure */
 int min_size,			/* I - minimum clump size */
 double min_mag			/* I - minimum clump magnitude */
)
{
  int ct = 0;
  int i;
  int j;

  for (i=1; i<ci->num_clumps+1; i++)
    {
      if (ci->clump_order[i].pts < min_size || ci->clump_order[i].mag < min_mag || ci->clump_order[i].ymin == ci->clump_order[i].ymax || ci->clump_order[i].xmin == ci->clump_order[i].xmax)
	{
	  ct++;
	  for (j=0; j<ci->clump_order[i].size; j++)
	      OEG_set_interval_float(grid, ci->num_cols, ci->clump_order[i].ptr[j], 0);
	}
    }

  return(ct);
}

