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
 * Module: bbox.c
 *
 * Author: Gerry Wiener
 *
 * Date:   1/8/97
 *
 * Description:
 *     Find bounding boxes for arrays of intervals.
 */
#include <limits.h>
#include <euclid/clump.h>

void EG_find_2d_bbox
(
 Interval **iarray,		/* I - array of intervals */
 int n_iarray,			/* I - size of iarray */
 box_2d_t *box_2d		/* O - bounding box containing intervals */
)
{
  int i;
  int xmin = INT_MAX;
  int ymin = INT_MAX;
  int xmax = INT_MIN;
  int ymax = INT_MIN;

  if (n_iarray == 0)
    return;

  ymin = iarray[0]->row_in_plane;
  ymax = iarray[0]->row_in_plane;
  xmin = iarray[0]->begin;
  xmax = iarray[0]->end;
  for (i=1; i<n_iarray; i++)
    {
      if (iarray[i]->row_in_plane < ymin)
	ymin = iarray[i]->row_in_plane;
      else if (iarray[i]->row_in_plane > ymax)
	ymax = iarray[i]->row_in_plane;

      if (iarray[i]->begin < xmin)
	xmin = iarray[i]->begin;
      else if (iarray[i]->end > xmax)
	xmax = iarray[i]->end;
    }
  
  box_2d->xmin = xmin;
  box_2d->xmax = xmax;
  box_2d->ymin = ymin;
  box_2d->ymax = ymax;
}

/* find bounding boxes for the clumps in a clump_info structure */
void OEG_find_ci_2d_bbox
(
 OClump_info *ci		/* I/O - clump information structure */
)
{
  box_2d_t box_2d;
  int i;

  for (i=1; i<ci->num_clumps+1; i++)
    {
      EG_find_2d_bbox(ci->clump_order[i].ptr, ci->clump_order[i].size, &box_2d);
      ci->clump_order[i].xmin = box_2d.xmin;
      ci->clump_order[i].xmax = box_2d.xmax;
      ci->clump_order[i].ymin = box_2d.ymin;
      ci->clump_order[i].ymax = box_2d.ymax;
      ci->clump_order[i].zmin = 0;
      ci->clump_order[i].zmax = 0;
    }
}

void EG_find_3d_bbox
(
 Interval **iarray,		/* I - array of intervals */
 int n_iarray,			/* I - size of iarray */
 int ny,			/* I - number of rows in plane */
 box_3d_t *box_3d		/* O - bounding box containing intervals */
)
{
  int i;
  int xmin = INT_MAX;
  int xmax = INT_MIN;
  int y;
  int ymin = INT_MAX;
  int ymax = INT_MIN;
  int z;
  int zmin = INT_MAX;
  int zmax = INT_MIN;

  if (n_iarray == 0)
    return;

  zmin = iarray[0]->row_in_plane / ny;
  zmax = iarray[0]->row_in_plane / ny;
  ymin = iarray[0]->row_in_plane % ny;
  ymax = iarray[0]->row_in_plane % ny;
  xmin = iarray[0]->begin;
  xmax = iarray[0]->end;
  for (i=1; i<n_iarray; i++)
    {
      y = iarray[i]->row_in_plane % ny;
      if (y < ymin)
	ymin = y;
      else if (y > ymax)
	ymax = y;

      z = iarray[i]->row_in_plane / ny;
      if (z < zmin)
	zmin = z;
      else if (z > zmax)
	zmax = z;

      if (iarray[i]->begin < xmin)
	xmin = iarray[i]->begin;
      else if (iarray[i]->end > xmax)
	xmax = iarray[i]->end;
    }
  
  box_3d->xmin = xmin;
  box_3d->xmax = xmax;
  box_3d->ymin = ymin;
  box_3d->ymax = ymax;
  box_3d->zmin = zmin;
  box_3d->zmax = zmax;
}

/* find bounding boxes for the clumps in a clump_info structure */
void OEG_find_ci_3d_bbox
(
 OClump_info *ci		/* I/O - clump information structure */
)
{
  box_3d_t box_3d;
  int i;
  int ny = ci->num_rows / ci->num_planes;

  for (i=1; i<ci->num_clumps+1; i++)
    {
      EG_find_3d_bbox(ci->clump_order[i].ptr, ci->clump_order[i].size, ny, &box_3d);
      ci->clump_order[i].xmin = box_3d.xmin;
      ci->clump_order[i].xmax = box_3d.xmax;
      ci->clump_order[i].ymin = box_3d.ymin;
      ci->clump_order[i].ymax = box_3d.ymax;
      ci->clump_order[i].zmin = box_3d.zmin;
      ci->clump_order[i].zmax = box_3d.zmax;
    }
}
