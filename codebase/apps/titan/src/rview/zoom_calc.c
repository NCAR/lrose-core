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
 * zoom_calc.c: calcluates the zoom parameters
 *
 * RAP, NCAR, Boulder CO
 *
 * October 1990   
 *
 * Mike Dixon
 *
 *********************************************************************/

#include "rview.h"

void zoom_calc(int dev,
	       gframe_t *frame)

{

  ui32 image_width, image_height;
  double aspect;
  double mid_x, mid_y;
  double range_x, range_y;
  double plot_min_x, plot_max_x, plot_min_y, plot_max_y;
  double xaxismargin, yaxismargin, topmargin;
  double unitscale;

  zoom_t *zoom;

  if (Glob->debug) {
    fprintf(stderr, "** zoom_calc **\n");
  }

  zoom = Glob->zoom + Glob->zoom_level;
  
  /*
   * get margins from the parameters resources
   */

  if (dev == XDEV) {

    xaxismargin = (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
				       "x_xaxismargin", X_XAXISMARGIN);
    yaxismargin = (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
				       "x_yaxismargin", X_YAXISMARGIN);

    topmargin = (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
				     "x_topmargin", X_TOPMARGIN);

    image_width =
      (double) frame->x->width - yaxismargin;

    image_height =
      (double) frame->x->height - xaxismargin - topmargin;

  } else {

    unitscale = xGetResDouble(Glob->rdisplay, Glob->prog_name,
			      "ps_unitscale",
			      PS_UNITSCALE);

    xaxismargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_xaxismargin", PS_XAXISMARGIN) * unitscale;

    yaxismargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_yaxismargin", PS_YAXISMARGIN) * unitscale;

    topmargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_topmargin", PS_TOPMARGIN) * unitscale;

    image_width =
      (double) frame->ps->width - yaxismargin;

    image_height =
      (double) frame->ps->height - xaxismargin - topmargin;

  }

  if (Glob->zoom_level > 0) {

    /*
     * compute mid point of zoom area
     */

    mid_x = (zoom->min_x + zoom->max_x) / 2.0;

    mid_y = (zoom->min_y + zoom->max_y) / 2.0;

    /*
     * adjust aspect ratio so that it is the same as the
     * for the full space
     */

    range_x = (zoom->max_x - zoom->min_x);

    range_y = (zoom->max_y - zoom->min_y);

    aspect = range_y / range_x;

    if (aspect >= Glob->full_aspect) {

      range_x = range_y / Glob->full_aspect;

      zoom->min_x = mid_x - range_x / 2.0;
      zoom->max_x = mid_x + range_x / 2.0;

      if (zoom->min_x <	Glob->zoom[0].min_x) {

	zoom->min_x = Glob->zoom[0].min_x;
	zoom->max_x =
	  zoom->min_x + range_x;

      } else if (zoom->max_x > Glob->zoom[0].max_x) {

	zoom->max_x = Glob->zoom[0].max_x;
	zoom->min_x =
	  zoom->max_x - range_x;

      }

    } else {

      range_y = range_x * Glob->full_aspect;

      zoom->min_y = mid_y - range_y / 2.0;
      zoom->max_y = mid_y + range_y / 2.0;

      if (zoom->min_y <	Glob->zoom[0].min_y) {

	zoom->min_y = Glob->zoom[0].min_y;
	zoom->max_y =
	  zoom->min_y + range_y;

      } else if (zoom->max_y > Glob->zoom[0].max_y) {

	zoom->max_y = Glob->zoom[0].max_y;
	zoom->min_y =
	  zoom->max_y - range_y;

      }

    } /* if (aspect >= Glob->full_aspect) */

    zoom->range_x = range_x;
    zoom->range_y = range_y;

  } /* if (Glob->zoom_level > 0) */

  /*
   * compute plot limits including margins
   */

  plot_min_x = zoom->min_x;
  plot_max_x = zoom->max_x;
  range_x = plot_max_x - plot_min_x;
  plot_max_x += range_x * ((double) yaxismargin / (double) image_width);
  
  plot_min_y = zoom->min_y;
  plot_max_y = zoom->max_y;
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
