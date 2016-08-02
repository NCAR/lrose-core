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
/*********************************************************************
 * vsection_calc.c: calcluates the vsection frame geometry
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

void vsection_calc(int dev,
		   gframe_t *frame,
		   cd_grid_info_t *grid_info,
		   cd_reply_t *reply,
		   double *plane_heights)

{

  si32 top_index, base_index;

  double image_width, image_height;
  double range_x, range_y;
  double delta_x, delta_y;
  double plot_min_x, plot_max_x;
  double plot_min_y, plot_max_y;
  double xaxismargin, yaxismargin, topmargin;

  vsection_t *vsection;

  if (Glob->debug) {
    fprintf(stderr, "** vsection_calc **\n");
  }

  /*
   * set local pointers
   */

  vsection = &Glob->vsection;

  /*
   * get margins from the parameters resources
   */

  if (dev == XDEV) {

    xaxismargin = (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
				       "x_xaxismargin", X_XAXISMARGIN);
    yaxismargin = (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
				       "x_yaxismargin", X_YAXISMARGIN);

    topmargin = 0.0;

    image_width =
      (double) frame->x->width - yaxismargin;

    image_height =
      (double) frame->x->height - xaxismargin - topmargin;

  } else {

    xaxismargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				"ps_xaxismargin", PS_XAXISMARGIN);
    yaxismargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				"ps_yaxismargin", PS_YAXISMARGIN);

    topmargin = 0.0;

    image_width =
      (double) frame->ps->width - yaxismargin;

    image_height =
      (double) frame->ps->height - xaxismargin - topmargin;

  }

  vsection->start_x =
    grid_info->min_x + ((double) reply->x1 + 0.5) * grid_info->dx;

  vsection->end_x =
    grid_info->min_x + ((double) reply->x2 + 0.5) * grid_info->dx;

  vsection->start_y =
    grid_info->min_y + ((double) reply->y1 + 0.5) * grid_info->dy;

  vsection->end_y =
    grid_info->min_y + ((double) reply->y2 + 0.5) * grid_info->dy;

  delta_x =
    vsection->start_x - vsection->end_x;

  delta_y =
    vsection->start_y - vsection->end_y;

  vsection->length =
    (double) sqrt (delta_x * delta_x + delta_y * delta_y);
  
  base_index = reply->z1 * N_PLANE_HEIGHT_VALUES + PLANE_BASE_INDEX;
  top_index = reply->z2 * N_PLANE_HEIGHT_VALUES + PLANE_TOP_INDEX;
  vsection->min_z = plane_heights[base_index];
  vsection->max_z = plane_heights[top_index];

  plot_min_x = 0.0;
  plot_max_x = vsection->length;
  range_x = plot_max_x - plot_min_x;
  plot_max_x += range_x * ((double) yaxismargin / (double) image_width);

  plot_min_y = vsection->min_z;
  plot_max_y = vsection->max_z;
  range_y = plot_max_y - plot_min_y;
  plot_min_y -= range_y * ((double) xaxismargin / (double) image_height);
  plot_max_y += range_y * ((double) topmargin / (double) image_height);

  /*
   * set up plot frame world
   */

  if (dev == XDEV) {

    GMoveFrameWorld(frame,
		    plot_min_x, plot_min_y,
		    plot_max_x, plot_max_y);
  
    GXSetGeomFrame(frame,
		   frame->x->width,
		   frame->x->height);

  } else {

    GMoveFrameWorld(frame,
		    plot_min_x, plot_min_y,
		    plot_max_x, plot_max_y);
  
    GPsSetGeomFrame(frame,
		    frame->ps->xmin,
		    frame->ps->ymin,
		    frame->ps->width,
		    frame->ps->height);

  }

}
