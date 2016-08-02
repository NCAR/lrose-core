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

/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/*
 * Module: clump_volume.c
 *
 * Author: Gerry Wiener
 *
 * Date:   6/17/96
 *
 * Description:
 *     
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <euclid/clump.h>
#include <euclid/boundary.h>

int OEG_clump_volume_float
(
 float *grid,			/* I - input grid */
 int grid_nx,			/* I - number of columns in input grid */
 int grid_ny,			/* I - number of rows in input grid */
 int grid_nz,			/* I - number of planes in input grid */
 double threshold,		/* I - threshold to clump at */
 int min_overlap,		/* I - minimum interval overlap */
 OClump_info *clump_info	/* O - set of output clumps */
)
{
  int clear;
  OClump_order *clump_order = NULL;
  int isize;
  Interval **interval_order = NULL;
  Interval *intervals = NULL;
  int num_clumps;
  int num_intervals;
  Row_hdr *row_hdr = NULL;

  /* clump the data in the grid */

  /* find the intervals in array */

  /* allocate row_hdr */
  row_hdr = (Row_hdr *)EG_malloc(grid_ny*grid_nz*sizeof(Row_hdr));
  if (row_hdr == NULL)
    goto error_return;

  isize = 0;

  /* find the intervals above threshold in the data */
  num_intervals = EG_find_intervals_float(grid_ny * grid_nz, grid_nx, grid, &intervals, &isize, row_hdr, threshold);

  if (num_intervals < 0)
    goto error_return;
    
  interval_order = EG_malloc((num_intervals+1) * sizeof(Interval *));
  /*  printf("num_intervals %d\n", num_intervals); */
  if (interval_order == NULL)
    goto error_return;

  clump_order = EG_calloc((num_intervals+1), sizeof(OClump_order));
  if (clump_order == NULL)
    goto error_return;

  /* clump the information in the grid */
  clear = 1;		/* clear the interval ids  */

  num_clumps = OEG_rclump_3d(row_hdr, grid_ny, grid_nz, clear, min_overlap, interval_order, clump_order);


  /* set clump_info structure */
  clump_info->volume = NULL;
  clump_info->num_planes = grid_nz;
  clump_info->num_rows = grid_ny * grid_nz;
  clump_info->num_cols = grid_nx;
  clump_info->threshold = threshold;
  clump_info->isize = isize;
  clump_info->num_ints = num_intervals;
  clump_info->num_clumps = num_clumps;
  clump_info->intervals = intervals;
  clump_info->row_hdr = row_hdr;
  clump_info->interval_order = interval_order;
  clump_info->clump_order = clump_order;

  OEG_find_ci_3d_bbox(clump_info);

  return(0);

 error_return:
  if (interval_order != NULL)
    {
      free(interval_order);
      interval_order = NULL;
    }
  if (clump_order != NULL)
    {
      free(clump_order);
      clump_order = NULL;
    }
  if (intervals != NULL)
    {
      free(intervals);
      intervals = NULL;
    }
  if (row_hdr != NULL)
    {
      free(row_hdr);
      row_hdr = NULL;
    }
  return(-1);
}

