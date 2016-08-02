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
 * gutil.c
 *
 * General graphics utility routines - uses xutil.c and psutil.c
 * routines.
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80307, USA
 *
 * December 1991
 *
 *********************************************************************/

#include <rapplot/gplot.h>
#include <rapplot/xudr.h>

#define SUCCESS 0
#define FAILURE 1

#define LARGE_DOUBLE 1.0e99

static void g_draw_triang_segments(int dev,
				   const gframe_t *frame,
				   GC pos_gc, 
				   GC zero_gc, 
				   GC neg_gc, 
				   const psgc_t *pos_psgc, 
				   const psgc_t *zero_psgc, 
				   const psgc_t *neg_psgc, 
				   double *value, 
				   double *x, 
				   double *y, 
				   g_contour_t *contours);

static void g_get_triang_order(double *value, 
			       int *i1, 
			       int *i2, 
			       int *i3);

static void g_interp_contours(double *value, 
			      int i1, 
			      int i2, 
			      double z, 
			      double *x, 
			      double *y, 
			      double *xp, 
			      double *yp);

static void g_draw_string(int dev, 
			  const gframe_t *frame, 
			  GC xgc, 
			  XFontStruct *font, 
			  const psgc_t *psgc, 
			  int xjust, 
			  int yjust, 
			  double wx, 
			  double wy, 
			  int x_offset,
			  int y_offset,
			  const char *text,
			  int image_flag);

static int parse_color_scale_line(char *line, 
				  double *start_val, 
				  double *end_val, 
				  char **colorname);

/*********************************************************************
 * GAdjustGCScale()
 *
 * Adjusts up the GC pointer array for a given color scale.
 * The GC pointer array is then used for locating a GC with the
 * color relevant to a byte data value in the field.
 *
 * Must use function GLoadGCScale first
 *
 *********************************************************************/

void GAdjustGCScale(g_color_scale_t *color_scale, 
		    double scale, 
		    double bias)

{
  
  int ilevel, ival;
  int start_level;
  double data_val;

  /* 
   * set scale and bias and number of data vals
   */

  color_scale->scale = scale;
  color_scale->bias = bias;

  /*
   * assign a GC pointer to each possible data value
   */

  start_level = 0;
  data_val = bias;

  for (ival = 0; ival < color_scale->nvalues; ival++) {

    color_scale->gc[ival] = NULL;

    for (ilevel = start_level; ilevel < color_scale->nlevels; ilevel++) {

      if ((data_val >= color_scale->level[ilevel].start) &&
	  (data_val <= color_scale->level[ilevel].end)) {

	color_scale->gc[ival] = color_scale->level[ilevel].gc;
	start_level = ilevel;

	break;
	  
      }

    } /* ilevel */

    data_val += scale;

  } /* ival */

}

/*********************************************************************
 * GAdjustPsgcScale()
 *
 * Adjusts up the Psgc pointer array for a given color scale.
 * The Psgc pointer array is then used for locating a Psgc with the
 * color relevant to a byte data value in the field.
 *
 * Must use function GLoadPsgcScale first
 *
 *********************************************************************/

void GAdjustPsgcScale(g_color_scale_t *color_scale, 
		      double scale, 
		      double bias)

{
  
  int ilevel, ival;
  int start_level;
  double data_val;

  /* 
   * set scale and bias and number of data vals
   */

  color_scale->scale = scale;
  color_scale->bias = bias;

  /*
   * assign a Psgc pointer to each possible data value
   */

  start_level = 0;
  data_val = bias;

  for (ival = 0; ival < color_scale->nvalues; ival++) {

    color_scale->psgc[ival] = NULL;

    for (ilevel = start_level; ilevel < color_scale->nlevels; ilevel++) {

      if ((data_val >= color_scale->level[ilevel].start) &&
	  (data_val <= color_scale->level[ilevel].end)) {

	color_scale->psgc[ival] = color_scale->level[ilevel].psgc;
	start_level = ilevel;

	break;
	  
      }

    } /* ilevel */

    data_val += scale;

  } /* ival */

}

/**************************************************************************
 * GCreateFrame(): creates a gframe_t structure, returns the pointer to the
 *                 structure
 *
 *************************************************************************/

gframe_t *GCreateFrame(double w_xmin, 
		       double w_ymin, 
		       double w_xmax, 
		       double w_ymax)

{

  gframe_t *gframe;

  gframe = (gframe_t *) ucalloc (1, sizeof(gframe_t));

  gframe->w_xmin = w_xmin;
  gframe->w_ymin = w_ymin;
  gframe->w_xmax = w_xmax;
  gframe->w_ymax = w_ymax;

  gframe->x = (xref_t *) ucalloc(1, sizeof(xref_t));
  gframe->ps = (psref_t *) ucalloc(1, sizeof(psref_t));
  gframe->psgc = (psgc_t *) ucalloc(1, sizeof(psgc_t));

  return gframe;

}

/**************************************************************************
 * GDrawArc()
 *
 * Draws and arc, counterclockwise from wangle1 to wangle2,
 * as part of an ellipse having radii wradiusx and wradiusy in each
 * axis dirn respectively. The center is at wx, wy. The whole ellipse
 * may be rotated anticlockwise by angle axis_rotation. All angles
 * in degrees. The X implementation uses the nsegments arg to
 * determine the number of line segments to be used to represent
 * the arc.
 *
 *************************************************************************/

void GDrawArc(int dev, 
	      const gframe_t *frame, 
	      GC xgc, 
	      const psgc_t *psgc, 
	      double wx, 
	      double wy, 
	      double wradiusx, 
	      double wradiusy, 
	      double wangle1, 
	      double wangle2, 
	      double axis_rotation, 
	      int nsegments)

{

  int x, y;
  int circular;
  int isegment;
  double ps_rx, ps_ry;
  double delta_theta;
  double start_theta, end_theta;
  double start_phi, end_phi;
  double start_x, start_y, end_x, end_y;
  double radius;
  double ratio_factor;
  double axis_ratio;
  double radius_max;
  double xmin, ymin, xmax, ymax;
  GPoint *points, *point;

  /*
   * return early if the arc cannot fall within the frame
   */

  radius_max = MAX(wradiusx, wradiusy);
  xmin = wx - radius_max;
  ymin = wy - radius_max;
  xmax = wx + radius_max;
  ymax = wy + radius_max;

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * plot the arc
   */

  switch (dev) {

  case XDEV:

    /*
     * decide if we are dealing with a circle or an ellipse
     */

    axis_ratio = fabs(wradiusx / wradiusy);

    if (0.999999 < axis_ratio && axis_ratio < 1.000001)
      circular = TRUE;
    else
      circular = FALSE;

    if (circular == TRUE)
      radius = wradiusx;
    else
      ratio_factor = pow(wradiusy / wradiusx, 2.0) - 1.0;

    /*
     * set up start angles
     */

    delta_theta = ((wangle2 - wangle1) * DEG_TO_RAD) / (double) nsegments;

    start_theta = wangle1 * DEG_TO_RAD;
    start_phi = start_theta + axis_rotation * DEG_TO_RAD;
    end_theta = start_theta + delta_theta;
    end_phi = start_phi + delta_theta;

    /*
     * set up start and end (x,y) coords
     */

    if (circular == FALSE)
      radius = wradiusy / sqrt(1.0 + pow(cos(start_theta), 2.0) * ratio_factor);

    start_x = wx + radius * cos(start_phi);
    start_y = wy + radius * sin(start_phi);

    if (circular == FALSE)
      radius = wradiusy / sqrt(1.0 + pow(cos(end_theta), 2.0) * ratio_factor);

    end_x = wx + radius * cos(end_phi);
    end_y = wy + radius * sin(end_phi);

    /*
     * load the segments
     */

    points = (GPoint *) ucalloc ((nsegments + 1), sizeof(GPoint));
    point = points;

    point->x = start_x;
    point->y = start_y;
    point++;
    
    for (isegment = 0; isegment < nsegments; isegment++) {
      
      point->x = end_x;
      point->y = end_y;
      point++;
    
      /*
       * update segment ends
       */

      start_x = end_x;
      start_y = end_y;

      end_theta += delta_theta;
      end_phi += delta_theta;

      if (circular == FALSE)
	radius = wradiusy /
	  sqrt(1.0 + pow(cos(end_theta), 2.0) * ratio_factor);

      end_x = wx + radius * cos(end_phi);
      end_y = wy + radius * sin(end_phi);

    }

    GDrawLines(dev, frame, xgc, psgc, points,
	       (int) (nsegments + 1), CoordModeOrigin);

    ufree ((char *) points);
    
    break;

  case PSDEV:

    x = (int) ((wx - frame->w_xmin) * frame->ps->xscale
	       + frame->ps->xmin + 0.5);
    y = (int) ((wy - frame->w_ymin) * frame->ps->yscale
	       + frame->ps->ymin + 0.5);
    ps_rx = wradiusx * frame->ps->xscale;
    ps_ry = wradiusy * frame->ps->yscale;

    fprintf(frame->psgc->file, " %d %d %g %g %g %g %g DrawArc\n",
	    x, y, ps_rx, ps_ry, wangle1, wangle2, axis_rotation);
    
    break;

  }

}

/**************************************************************************
 * GDrawArrow()
 *
 * draws arrow, given start end end points, head angle and head length
 *
 *************************************************************************/

void GDrawArrow(int dev, 
		const gframe_t *frame, 
		GC xgc, 
		const psgc_t *psgc, 
		double wx1, 
		double wy1, 
		double wx2, 
		double wy2, 
		double head_angle, 
		double head_length)

{

  double xscale, yscale;
  double x_dev_head_length, y_dev_head_length;
  double dev_head_length;
  double arrow_slope, head_slope;
  double head_x, head_y;

  /*
   * draw in body
   */

  GDrawLine(dev, frame, xgc, psgc, wx1, wy1, wx2, wy2);

  /*
   * because windows may have unequal x and y scales, we need to
   * determine head length in device coords, and translate back into
   * world coords after determining the x and y components
   */

  switch (dev) {
  
  case XDEV:
    xscale = fabs(frame->x->xscale);
    yscale = fabs(frame->x->yscale);
    break;

  case PSDEV:
    xscale = fabs(frame->ps->xscale);
    yscale = fabs(frame->ps->yscale);
    break;

  } /* switch */

  /*
   * get arrow slope
   */

  if (wx1 == wx2 && wy1 == wy2)
    arrow_slope = 0.0;
  else
    arrow_slope = atan2((wy1 - wy2) * yscale,
			(wx1 - wx2) * xscale);

  x_dev_head_length = head_length * xscale;
  y_dev_head_length = head_length * yscale;

  dev_head_length = sqrt(x_dev_head_length * x_dev_head_length +
			 y_dev_head_length * y_dev_head_length);
  
  /*
   * first head segment
   */

  head_slope = arrow_slope + head_angle * DEG_TO_RAD;
  head_x = wx2 + (dev_head_length * cos(head_slope)) / xscale;
  head_y = wy2 + (dev_head_length * sin(head_slope)) / yscale;

  GDrawLine(dev, frame, xgc, psgc, wx2, wy2, head_x, head_y);

  /*
   * second head segment
   */

  head_slope = arrow_slope - head_angle * DEG_TO_RAD;
  head_x = wx2 + (dev_head_length * cos(head_slope)) / xscale;
  head_y = wy2 + (dev_head_length * sin(head_slope)) / yscale;

  GDrawLine(dev, frame, xgc, psgc, wx2, wy2, head_x, head_y);

}

/**************************************************************************
 * GDrawContours()
 *
 * draws contours from image data
 *
 * This routine draws line segments representing parts of contours across
 * triangular shapes defined by the rectangular grid.
 *
 *************************************************************************/

void GDrawContours(int dev, 
		   const gframe_t *frame, 
		   GC pos_gc, 
		   GC zero_gc, 
		   GC neg_gc, 
		   psgc_t *pos_psgc, 
		   psgc_t *zero_psgc, 
		   psgc_t *neg_psgc, 
		   double wx, 
		   double wy, 
		   double wwidth, 
		   double wheight, 
		   long nsamples, 
		   long nlines, 
		   long startpoint, 
		   unsigned char *data_array, 
		   double scale, 
		   double bias, 
		   g_contour_t *contours, 
		   long missing_val)

{
  
  long iline, isample;
  long data_line_1, data_line_2;
  long data_n11, data_n12, data_n21, data_n22;

  double x11, x12, x21, x22;
  double y11, y12, y21, y22;
  double val11, val12, val21, val22;
  double value[3], x[3], y[3];
  double dx, dy;
  double minx, miny;

  /*
   * make sure the file is set in the psgcs
   */

  pos_psgc->file = frame->psgc->file;
  zero_psgc->file = frame->psgc->file;
  neg_psgc->file = frame->psgc->file;

  /*
   * element dimensions in each dirn
   */

  dx = (double) wwidth / (double) nsamples;
  dy = (double) wheight / (double) nlines;

  /*
   * min x and y of center of lower-left element
   */

  minx = (double) wx + dx / 2.0;
  miny = (double) wy + dy / 2.0;

  for (iline = 0; iline < nlines - 1; iline++) {

    /*
     * determine the data line, depending on the image data startpoint
     */
  
    if (startpoint == IMAGE_START_TOPLEFT) {
      data_line_1 = nlines - iline - 1;
      data_line_2 = data_line_1 - 1;
    } else if (startpoint == IMAGE_START_BOTLEFT) {
      data_line_1 = iline;
      data_line_2 = iline + 1;
    } else {
      fprintf(stderr, "ERROR - GDrawContours.\n");
      fprintf(stderr, "Image startpoint type %ld not recognized.\n",
	      startpoint);
      exit(-1);
    }

    for (isample = 0; isample < nsamples - 1; isample++) {

      /*
       * for each sample point, there are 2 triangles which must be dealt
       * with. First the corner point data indices are identified
       */

      data_n11 = data_line_1 * nsamples + isample;
      data_n12 = data_n11 + 1;
      data_n21 = data_line_2 * nsamples + isample;
      data_n22 = data_n21 + 1;

      /*
       * then the corner x and y vals are computed
       */

      x11 = minx + (double) isample * dx;
      x12 = x11 + dx;
      x21 = x11;
      x22 = x12;

      y11 = miny + (double) iline * dy;
      y12 = y11;
      y21 = y11 + dy;
      y22 = y21;

      /*
       * next the data vals are each corner are computed - if data is
       * missing, set to high negative value
       */

      if (data_array[data_n11] != missing_val)
	val11 = (double) data_array[data_n11] * scale + bias;
      else
	val11 = -99e99;

      if (data_array[data_n12] != missing_val)
	val12 = (double) data_array[data_n12] * scale + bias;
      else
	val12 = -99e99;

      if (data_array[data_n21] != missing_val)
	val21 = (double) data_array[data_n21] * scale + bias;
      else
	val21 = -99e99;

      if (data_array[data_n22] != missing_val)
	val22 = (double) data_array[data_n22] * scale + bias;
      else
	val22 = -99e99;

      /*
       * first traingle
       */

      value[0] = val11;
      value[1] = val12;
      value[2] = val22;

      x[0] = x11;
      x[1] = x12;
      x[2] = x22;

      y[0] = y11;
      y[1] = y12;
      y[2] = y22;

      g_draw_triang_segments(dev, frame,
			     pos_gc, zero_gc, neg_gc,
			     pos_psgc, zero_psgc, neg_psgc,
			     value, x, y,
			     contours);

      /*
       * second traingle
       */

      value[0] = val11;
      value[1] = val21;
      value[2] = val22;

      x[0] = x11;
      x[1] = x21;
      x[2] = x22;

      y[0] = y11;
      y[1] = y21;
      y[2] = y22;

      g_draw_triang_segments(dev, frame,
			     pos_gc, zero_gc, neg_gc,
			     pos_psgc, zero_psgc, neg_psgc,
			     value, x, y,
			     contours);

    } /* isample */

  } /* iline */

}


/****************************************************************************
 * g_draw_triang_segments
 *
 * Draws the line segments across a triangular region
 */

static void g_draw_triang_segments(int dev, 
				   const gframe_t *frame, 
				   GC pos_gc, 
				   GC zero_gc, 
				   GC neg_gc, 
				   const psgc_t *pos_psgc, 
				   const psgc_t *zero_psgc, 
				   const psgc_t *neg_psgc, 
				   double *value, 
				   double *x, 
				   double *y, 
				   g_contour_t *contours)

{

  int i1, i2, i3;
  int icont;
  double z;
  double xp1, yp1, xp2, yp2;

  /*
   * get the order of the value array
   */

  g_get_triang_order(value, &i1, &i2, &i3);

  /*
   * return now if the triangle corner values are outside the
   * desired range, or if any of the corner values are the missing
   * data value
   */

  
  if (value[i3] < contours->value[0] ||
      value[i1] > contours->value[contours->n - 1])
    return;

  if (value[i1] < -98e99 ||
      value[i2] < -98e99 ||
      value[i3] < -98e99)
    return;

  /*
   * determine starting contour index
   */

  for (icont = 0; icont < contours->n; icont++) {

    if (value[i3] < contours->value[icont])
      return;

    if (value[i1] <= contours->value[icont] &&
	value[i3] >= contours->value[icont]) {

      z = contours->value[icont];

      if (z <= value[i2])
	g_interp_contours(value, i1, i2, z, x, y, &xp1, &yp1);
      else
	g_interp_contours(value, i2, i3, z, x, y, &xp1, &yp1);

      g_interp_contours(value, i1, i3, z, x, y, &xp2, &yp2);

      switch (dev) {

      case XDEV:

	if (-0.0001 < z && z < 0.0001) {

	  GDrawLine(dev, frame, zero_gc, zero_psgc,
		    xp1, yp1, xp2, yp2);

	} else if (z < 0) {

	  GDrawLine(dev, frame, neg_gc, neg_psgc,
		    xp1, yp1, xp2, yp2);

	} else {
	  
	  GDrawLine(dev, frame, pos_gc, pos_psgc,
		    xp1, yp1, xp2, yp2);

	}

	break;

      case PSDEV:

	PsGsave(frame->psgc->file);

	if (-0.0001 < z && z < 0.0001) {

	  PsSetLineWidth(frame->psgc->file, zero_psgc->line_width);
	  PsSetLineDash(frame->psgc->file,
			zero_psgc->dash_length, zero_psgc->space_length);
	  GDrawLine(dev, frame, zero_gc, zero_psgc,
		    xp1, yp1, xp2, yp2);

	} else if (z < 0) {

	  PsSetLineWidth(frame->psgc->file, neg_psgc->line_width);
	  PsSetLineDash(frame->psgc->file,
			neg_psgc->dash_length, neg_psgc->space_length);
	  GDrawLine(dev, frame, neg_gc, neg_psgc,
		    xp1, yp1, xp2, yp2);

	} else {
	  
	  PsSetLineWidth(frame->psgc->file, pos_psgc->line_width);
	  PsSetLineDash(frame->psgc->file,
			pos_psgc->dash_length, pos_psgc->space_length);
	  GDrawLine(dev, frame, pos_gc, pos_psgc,
		    xp1, yp1, xp2, yp2);

	}

	PsGrestore(frame->psgc->file);

	break;

      } /* switch (dev) */

    } /* if */

  } /* icont */

}


/***************************************************************************
 * g_get_triang_order
 *
 * get the order of the data values at each vertex of the triangle
 */

static void g_get_triang_order(double *value, 
			       int *i1, 
			       int *i2, 
			       int *i3)

{

  double u1,u2,u3;

  u1 = value[0];
  u2 = value[1];
  u3 = value[2];

  if (u1 < u2)
    {
      if (u2 < u3)
        {
          *i1 = 0;
          *i2 = 1;
          *i3 = 2;
        }
      else
        {
          if (u3 < u1)
            {
              *i1 = 2;
              *i2 = 0;
              *i3 = 1;
            }
          else
            {
              *i1 = 0;
              *i2 = 2;
              *i3 = 1;
            }
        }
    }
  else
    {
      if (u2 < u3)
        {
          if (u1 < u3)
            {
              *i1 = 1;
              *i2 = 0;
              *i3 = 2;
            }
          else
            {
              *i1 = 1;
              *i2 = 2;
              *i3 = 0;
            }
        }
      else
        {
          *i1 = 2;
          *i2 = 1;
          *i3 = 0;
        }
    }
}


/*************************************************************************
 * g_interp_contours
 * 
 * interpolate to get the (x,y) pos of the contour as it
 * crosses a triangle boundary
 */

static void g_interp_contours(double *value, 
			      int i1, 
			      int i2, 
			      double z, 
			      double *x, 
			      double *y, 
			      double *xp, 
			      double *yp)

{

  if (value[i1] == value[i2]) {

    *xp = x[i1];
    *yp = y[i1];

  } else {

    if (x[i1] != x[i2]) {

      *xp = x[i1] + (x[i2] - x[i1]) /
	(value[i2]-value[i1])*(z-value[i1]);

      *yp = y[i1] + (y[i2] - y[i1]) /
	(x[i2]-x[i1])*(*xp - x[i1]);

    } else {

      *yp = y[i1] + (y[i2] - y[i1]) /
	(value[i2]-value[i1])*(z-value[i1]);

      *xp = x[i1] + (x[i2] - x[i1]) /
	(y[i2]-y[i1])*(*yp - y[i1]);

    }

  }

}

/**************************************************************************
 * GDrawImage(): draws image
 *
 *************************************************************************/

void GDrawImage(int dev, 
		const gframe_t *frame, 
		g_color_scale_t *colors, 
		double wx, 
		double wy, 
		double wwidth, 
		double wheight, 
		long nsamples, 
		long nlines, 
		long startpoint, 
		unsigned char *data_array, 
		long missing_val)

{

  unsigned char *d, *data;
  
  int samples_odd;
  int n_curr;

  long ctm[6];
  long i, j;
  long ddata;

  double curr_x;
  double dx, dy;
  double x, y, startx, starty;
  double el_width, el_height;

  GC new_gc, curr_gc;

  switch (dev) {

  case XDEV:

    el_width = wwidth / (double) nsamples;
    el_height = wheight / (double) nlines;
  
    if (startpoint == IMAGE_START_TOPLEFT) {
      dx = el_width;
      dy = -el_height;
      startx = wx;
      starty = wy + (nlines - 1) * el_height;
    } else if (startpoint == IMAGE_START_BOTLEFT) {
      dx = el_width;
      dy = el_height;
      startx = wx;
      starty = wy;
    } else {
      fprintf(stderr, "ERROR - GDrawImage.\n");
      fprintf(stderr, "Image startpoint type %ld not recognized.\n",
	      startpoint);
      exit(-1);
    }

    y = starty;
    d = data_array;

    for (i =  0; i < nlines; i++, y += dy) {
      
      x = startx;
      curr_gc = (GC) -1;
      n_curr = 0;

      for (j = 0; j  < nsamples; j++, d++, x += dx) {
	
	new_gc = colors->gc[*d];
	
	if (new_gc != NULL && *d != missing_val) {

	  /*
	   * look for runs of same color
	   */

	  if (new_gc == curr_gc) {

	    n_curr++;

	  } else {

	    if (n_curr > 0) {
	      GFillRectangle(dev, frame,
			     curr_gc,
			     frame->psgc,
			     curr_x, y,
			     el_width * (double) n_curr,
			     el_height);
	    } /* if (n_curr > 0) */

	    n_curr = 1;
	    curr_gc = new_gc;
	    curr_x = x;
	    
	  } /* if (new_gc == curr_gc) */

	} else {

	  /*
	   * fill in before null data if needed
	   */
	  
	  if (n_curr > 0) {
	    GFillRectangle(dev, frame,
			   curr_gc,
			   frame->psgc,
			   curr_x, y,
			   el_width * (double) n_curr,
			   el_height);
	    n_curr = 0;
	    curr_gc = (GC) -1;
	  } /* if (n_curr > 0) */
	  
	} /* if (new_gc != NULL ... */

      } /* j */

      /*
       * fill in at end of line if needed
       */
      
      if (n_curr > 0) {
	GFillRectangle(dev, frame,
		       curr_gc,
		       frame->psgc,
		       curr_x, y,
		       el_width * (double) n_curr,
		       el_height);
      } /* if (n_curr > 0) */
	  
    } /* i */

    break;

  case PSDEV:

    /*
     * set up transformation matrix
     */
    
    ctm[0] = nsamples;
    ctm[1] = 0;
    ctm[2] = 0;
    ctm[3] = 1;
    ctm[4] = 0;
    ctm[5] = 0;

    /*
     * decide if there is an odd number of samples
     */

    if (nsamples == (nsamples / 2) * 2)
      samples_odd = FALSE;
    else
      samples_odd = TRUE;
    
    dy = wheight / (double) nlines;
    el_height = dy;
  
    if (startpoint == IMAGE_START_TOPLEFT) {
      starty = wy + (nlines - 1) * dy;
      dy *= -1.0;
      data = data_array + (nlines - 1) * nsamples;
      ddata = -nsamples;
    } else if (startpoint == IMAGE_START_BOTLEFT) {
      starty = wy;
      data = data_array;
      ddata = nsamples;
    } else {
      fprintf(stderr, "ERROR - GDrawImage.\n");
      fprintf(stderr, "Image startpoint type %ld not recognized.\n",
	      startpoint);
      exit(-1);
    }

    y = starty;
    
    for (i =  0; i < nlines; i++, y += dy, data += ddata) {
      
      PsGsave(frame->psgc->file);
      GTranslate(PSDEV, frame, wx, y);
      GScale(PSDEV, frame, wwidth, el_height + 1.0);
      
      fprintf(frame->psgc->file, "%ld %d %d [%ld %ld %ld %ld %ld %ld]\n",
	      nsamples, 1, PS_GRAY_BITS,
	      ctm[0], ctm[1], ctm[2], ctm[3], ctm[4], ctm[5]);
      
      fprintf(frame->psgc->file, "{<");
      
      d = data;

      for (j = 0; j  < nsamples; j++, d++) {

	if (colors->psgc[*d] == NULL || *d == missing_val) {
	  fprintf(frame->psgc->file, "%s", PS_BACKGROUND_HEXSTRING);
	} else {
	  fprintf(frame->psgc->file, "%s",
		  colors->psgc[*d]->hexstring);
	}

      } /* j */

      /*
       * if odd number of samples, add an extra character to the stream
       * since postscript seems to need this
       */
      
      if (samples_odd) {
	fprintf(frame->psgc->file, "f");
      }

      fprintf(frame->psgc->file, ">} image\n");
    
      PsGrestore(frame->psgc->file);

    } /* i */
      
    break;

  } /* switch (dev) */

}

/**************************************************************************
 * GDrawImageProgDetail()
 * 
 * This draws image, using a progressive level of detail algorithm.
 * The level of detail in the image is decreased when zoomed out,
 * and increased when zoomed in.
 *
 *************************************************************************/

void GDrawImageProgDetail(int dev,
			  const gframe_t *frame, 
			  g_color_scale_t *colors, 
			  double wx, 
			  double wy, 
			  double wwidth, 
			  double wheight, 
			  long nsamples, 
			  long nlines, 
			  long startpoint, 
			  unsigned char *data_array, 
			  long missing_val,
			  int min_pix_per_grid)

{

  unsigned char *sparse_array;
  unsigned char *a, *d, *data;
  
  long mx, my;
  long sparse_nx, sparse_ny;
  long ddata;
  long start_ix, start_iy;
  long ix, iy;
  
  double dx, dy;
  double sparse_dx, sparse_dy;
  double sparse_width, sparse_height;
  double xscale, yscale;
  double xpix_per_grid, ypix_per_grid;
  
  switch (dev) {
  
  case XDEV:
    xscale = fabs(frame->x->xscale);
    yscale = fabs(frame->x->yscale);
    break;

  case PSDEV:
    xscale = fabs(frame->ps->xscale);
    yscale = fabs(frame->ps->yscale);
    break;

  } /* switch */

  dx = wwidth / (double) nsamples;
  dy = wheight / (double) nlines;

  xpix_per_grid = dx * xscale;
  ypix_per_grid = dy * yscale;

  mx = (long) (((double) min_pix_per_grid / xpix_per_grid) + 0.5);
  my = (long) (((double) min_pix_per_grid / ypix_per_grid) + 0.5);

  mx = MAX(1, mx);
  my = MAX(1, my);

  if (mx == 1 && my == 1) {

    GDrawImage(dev, frame, colors,
	       wx, wy, wwidth, wheight,
	       nsamples, nlines, startpoint,
	       data_array, missing_val);
    
  } else {

    sparse_dx = dx * mx;
    sparse_dy = dy * my;

    sparse_nx = nsamples / mx;
    sparse_ny = nlines / my;
    
    sparse_width = sparse_nx * sparse_dx;
    sparse_height = sparse_ny * sparse_dy;
    
    start_ix = (mx - 1) / 2;
    start_iy = (my - 1) / 2;

    if (startpoint == IMAGE_START_TOPLEFT) {
      data = data_array + (nlines - 1 + start_iy) * nsamples;
      ddata = -(nsamples * my);
    } else if (startpoint == IMAGE_START_BOTLEFT) {
      data = data_array + start_iy * nsamples;
      ddata = nsamples * my;
    } else {
      fprintf(stderr, "ERROR - GDrawImageProgDetail.\n");
      fprintf(stderr, "Image startpoint type %ld not recognized.\n",
	      startpoint);
      exit(-1);
    }

    sparse_array =
      (unsigned char *) umalloc (sparse_nx * sparse_ny * sizeof(unsigned char));
    a = sparse_array;

    for (iy = 0; iy < sparse_ny; iy++, data += ddata) {
      d = data + start_ix;
      for (ix = 0; ix < sparse_nx; ix++, d += mx, a++) {
 	*a = *d;
      } /* ix */
    } /* iy */

    GDrawImage(dev, frame, colors,
	       wx, wy, sparse_width, sparse_height,
	       sparse_nx, sparse_ny, IMAGE_START_BOTLEFT,
	       sparse_array, missing_val);

    ufree(sparse_array);
    
  }

  return;

}


/**************************************************************************
 * GDrawImageString(): draws string
 *
 *************************************************************************/

void GDrawImageString(int dev, 
		      const gframe_t *frame, 
		      GC xgc, 
		      XFontStruct *font, 
		      const psgc_t *psgc, 
		      int xjust, 
		      int yjust, 
		      double wx, 
		      double wy, 
		      const char *text)

{

  g_draw_string(dev, frame, xgc, font, psgc, xjust, yjust,
		wx, wy, 0, 0, text, TRUE);
  
}

/**************************************************************************
 * GDrawImageStringOffset(): draws string using the given pixel offsets
 *
 *************************************************************************/

void GDrawImageStringOffset(int dev, 
			    const gframe_t *frame, 
			    GC xgc, 
			    XFontStruct *font, 
			    const psgc_t *psgc, 
			    int xjust, 
			    int yjust, 
			    double wx, 
			    double wy, 
			    int x_offset,
			    int y_offset,
			    const char *text)
     
{

  g_draw_string(dev, frame, xgc, font, psgc, xjust, yjust,
		wx, wy, x_offset, y_offset, text, TRUE);
  
}

/**************************************************************************
 * g_draw_string(): draws string - Uses XDrawImageString() if image_flag
 *                  is TRUE, XDrawString() otherwise.  This is just an
 *                  internal routine used to reduce duplicate code.
 *
 *************************************************************************/

static void g_draw_string(int dev, 
			  const gframe_t *frame, 
			  GC xgc, 
			  XFontStruct *font, 
			  const psgc_t *psgc, 
			  int xjust, 
			  int yjust, 
			  double wx, 
			  double wy, 
			  int x_offset,
			  int y_offset,
			  const char *text,
			  int image_flag)

{

  int x, y;
  int strwidth, strheight;

  switch (dev) {

  case XDEV:

    x = (int) ((wx - frame->w_xmin) * frame->x->xscale + 0.5) + x_offset;
    y = (int) ((frame->w_ymax - wy) * frame->x->yscale + 0.5) - y_offset;

    strheight = font->ascent;
    strwidth = XTextWidth(font, text, strlen(text));

    switch (xjust) {

    case XJ_LEFT:
      break;

    case XJ_CENTER:
      x -= strwidth / 2;
      break;

    case XJ_RIGHT:
      x -= strwidth;

    }

    switch (yjust) {

    case YJ_ABOVE:
      break;

    case YJ_CENTER:
      y += strheight / 2;
      break;

    case YJ_BELOW:
      y += strheight;
      break;

    }
    
    if (image_flag)
      XDrawImageString(frame->x->display, frame->x->drawable, xgc, x, y,
		       text, strlen(text));
    else
      XDrawString(frame->x->display, frame->x->drawable, xgc, x, y,
		  text, strlen(text));

    break;

  case PSDEV:

    x = (int) ((wx - frame->w_xmin) * frame->ps->xscale
	       + frame->ps->xmin + 0.5) + x_offset;
    y = (int) ((wy - frame->w_ymin) * frame->ps->yscale
	       + frame->ps->ymin + 0.5) + y_offset;

    switch (yjust) {

    case YJ_ABOVE:

      switch (xjust) {

      case XJ_LEFT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrLeftAbove\n",
		x, y, text);
	break;

      case XJ_CENTER:
	fprintf(psgc->file, "%d %d (%s) DrawImstrCenterAbove\n",
		x, y, text);
	break;

      case XJ_RIGHT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrRightAbove\n",
		x, y, text);
	break;

      }

      break;

    case YJ_CENTER:

      switch (xjust) {

      case XJ_LEFT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrLeftCenter\n",
		x, y, text);
	break;

      case XJ_CENTER:
	fprintf(psgc->file, "%d %d (%s) DrawImstrCenterCenter\n",
		x, y, text);
	break;

      case XJ_RIGHT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrRightCenter\n",
		x, y, text);
	break;

      }

      break;

    case YJ_BELOW:

      switch (xjust) {

      case XJ_LEFT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrLeftBelow\n",
		x, y, text);
	break;

      case XJ_CENTER:
	fprintf(psgc->file, "%d %d (%s) DrawImstrCenterBelow\n",
		x, y, text);
	break;

      case XJ_RIGHT:
	fprintf(psgc->file, "%d %d (%s) DrawImstrRightBelow\n",
		x, y, text);
	break;

      }

      break;

    }

  }

}

/**************************************************************************
 * GDrawLine(): draws line
 *
 *************************************************************************/

void GDrawLine(int dev, 
	       const gframe_t *frame, 
	       GC xgc, 
	       const psgc_t *psgc, 
	       double wx1, 
	       double wy1, 
	       double wx2, 
	       double wy2)

{

  int xx1, yy1, xx2, yy2;
  double xmin, ymin, xmax, ymax;

  /*
   * return early if the line cannot fall within the frame
   */

  xmin = MIN(wx1, wx2);
  ymin = MIN(wy1, wy2);
  xmax = MAX(wx1, wx2);
  ymax = MAX(wy1, wy2);

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * plot the line
   */

  switch (dev) {

  case XDEV:

    xx1 = (int) ((wx1 - frame->w_xmin) * frame->x->xscale + 0.5);
    xx2 = (int) ((wx2 - frame->w_xmin) * frame->x->xscale + 0.5);
    yy1 = (int) ((frame->w_ymax - wy1) * frame->x->yscale + 0.5);
    yy2 = (int) ((frame->w_ymax - wy2) * frame->x->yscale + 0.5);

#ifdef NOTYET_REHAK
    if (xx1 >= 0 && xx2 >= 0 && yy1 >= 0 && yy2 >= 0)
#endif

      XDrawLine(frame->x->display, frame->x->drawable, xgc,
		xx1, yy1, xx2, yy2);

    break;

  case PSDEV:

    
    xx1 = (int) ((wx1 - frame->w_xmin) * frame->ps->xscale
		 + frame->ps->xmin + 0.5);
    xx2 = (int) ((wx2 - frame->w_xmin) * frame->ps->xscale
		 + frame->ps->xmin + 0.5);
    yy1 = (int) ((wy1 - frame->w_ymin) * frame->ps->yscale
		 + frame->ps->ymin + 0.5);
    yy2 = (int) ((wy2 - frame->w_ymin) * frame->ps->yscale
		 + frame->ps->ymin + 0.5);
    
    fprintf(psgc->file, " %d %d %d %d DrawLine\n", xx2, yy2, xx1, yy1);

    break;

  }

}

/**************************************************************************
 * GDrawLines(): draws lines
 *
 *************************************************************************/

/*ARGSUSED*/

void GDrawLines(int dev, 
		const gframe_t *frame, 
		GC xgc, 
		const psgc_t *psgc, 
		GPoint *gpoints, 
		int npoints, 
		int mode)

     /*
      * mode is either CoordModeOrigin or CoordModePrevious -
      * see X11R4 or later
      */
     
{

  int i;
  double xmin, ymin, xmax, ymax;
  GPoint *gpoint;
  PsPoint *pspoints, *pspoint, *prev_pspoint;
  XPoint *xpoints, *xpoint;

  /*
   * return early if the line cannot fall within the frame
   */

  xmin = LARGE_DOUBLE;
  ymin = LARGE_DOUBLE;
  xmax = -LARGE_DOUBLE;
  ymax = -LARGE_DOUBLE;

  gpoint = gpoints;

  for (i = 0; i < npoints; i++) {

    xmin = MIN(xmin, gpoint->x);
    ymin = MIN(ymin, gpoint->y);
    xmax = MAX(xmax, gpoint->x);
    ymax = MAX(ymax, gpoint->y);

    gpoint++;

  } /* i */

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * plot the lines
   */

  switch (dev) {

  case XDEV:

    /*
     * create array of XPoints, and load them up
     */

    xpoints = (XPoint *) ucalloc (npoints, sizeof(XPoint));

    xpoint = xpoints;
    gpoint = gpoints;

    for (i = 0; i < npoints; i++, xpoint++, gpoint++) {
      xpoint->x = (int) ((gpoint->x - frame->w_xmin) * frame->x->xscale + 0.5);
      xpoint->y = (int) ((frame->w_ymax - gpoint->y) * frame->x->yscale + 0.5);
    }

    XDrawLines(frame->x->display, frame->x->drawable, xgc,
	       xpoints, npoints, mode);
    
    ufree((char *) xpoints);

    break;

  case PSDEV:

    pspoints = (PsPoint *) ucalloc (npoints, sizeof(PsPoint));

    pspoint = pspoints;
    gpoint = gpoints;

    for (i = 0; i < npoints; i++) {

      pspoint->x = (int) ((gpoint->x - frame->w_xmin) * frame->ps->xscale
			  + frame->ps->xmin + 0.5);
      pspoint->y = (int) ((gpoint->y - frame->w_ymin) * frame->ps->yscale
			  + frame->ps->ymin + 0.5);

      pspoint++;
      gpoint++;

    }
      
    /* 
     * if mode is CoordModePrevious, the points are given relative to the 
     * previous, so compute relative to origin
     */

    if (mode == CoordModePrevious) {

      prev_pspoint = pspoints;
      pspoint = prev_pspoint + 1;

      for (i = 1; i < npoints; i++) {

	pspoint->x += prev_pspoint->x;
	pspoint->y += prev_pspoint->y;

	pspoint++;
	prev_pspoint++;
	
      } /* i */

    } /* if (mode == CoordModePrevious) */

    /*
     * write to ps file
     */

    pspoint = pspoints;

    fprintf(frame->psgc->file,
	    " newpath %d %d moveto\n", pspoint->x, pspoint->y);

    for (i = 1; i < npoints; i++) {

      pspoint++;
      fprintf(frame->psgc->file, " %d %d lineto\n", pspoint->x, pspoint->y);

    } /* i */

    fprintf(frame->psgc->file, " stroke\n");

    ufree((char *) pspoints);

    break;

  }

}

/**************************************************************************
 * GDrawPoints(): draws points
 *
 *************************************************************************/

/*ARGSUSED*/

void GDrawPoints(int dev, 
		 const gframe_t *frame, 
		 GC xgc, 
		 const psgc_t *psgc, 
		 double wx,
		 double wy,
		 XPoint *offsets,
		 int num_offsets)

{
  XPoint *xpoints;
  PsPoint *pspoints;
  
  int i;
  
  /*
   * plot the lines
   */

  switch (dev) {

  case XDEV:

    /*
     * Create the xpoints array to send to X.
     */

    xpoints = (XPoint *)ucalloc(num_offsets, sizeof(XPoint));
    
    for (i = 0; i < num_offsets; i++)
    {
      xpoints[i].x =
	(int)((wx - frame->w_xmin) * frame->x->xscale + 0.5) + offsets[i].x;
      xpoints[i].y =
	(int)((frame->w_ymax - wy) * frame->x->yscale + 0.5) - offsets[i].y;
    }
    
    XDrawPoints(frame->x->display, frame->x->drawable, xgc,
		xpoints, num_offsets, CoordModeOrigin);

    ufree((char *)xpoints);

    break;

  case PSDEV:

    pspoints = (PsPoint *) ucalloc(num_offsets, sizeof(PsPoint));

    for (i = 0; i < num_offsets; i++) {

      pspoints[i].x = (int)((wx - frame->w_xmin) * frame->ps->xscale
			    + frame->ps->xmin + 0.5) + offsets[i].x;
      pspoints[i].y = (int)((wy - frame->w_ymin) * frame->ps->yscale
			    + frame->ps->ymin + 0.5) + offsets[i].y;
    }
      
    /*
     * write to ps file.  I couldn't find in the PostScript
     * manual any instructions on how to draw a point, so I'm
     * just drawing VERY short lines.
     */

    fprintf(frame->psgc->file,
	    " newpath\n");

    for (i = 0; i < num_offsets; i++) {

      fprintf(frame->psgc->file,
	      " %d %d moveto\n", pspoints[i].x, pspoints[i].y);
      fprintf(frame->psgc->file,
	      " %d %d lineto\n", pspoints[i].x, pspoints[i].y);

    } /* i */

    fprintf(frame->psgc->file, " stroke\n");

    ufree((char *) pspoints);

    break;

  }

}

/**************************************************************************
 * GDrawPsFrameBorder()
 *
 * gutil graphics utility routine
 *
 *************************************************************************/

void GDrawPsFrameBorder(const gframe_t *frame, 
			double border_line_width)

{

  /*
   * set border line width
   */

  PsSetLineWidth(frame->psgc->file,
		 border_line_width);

  /*
   * draw borders
   */

  GDrawRectangle(PSDEV, frame,
		 frame->x->gc, frame->psgc,
		 frame->w_xmin, frame->w_ymin,
		 (frame->w_xmax - frame->w_xmin),
		 (frame->w_ymax - frame->w_ymin));

}

/**************************************************************************
 * GDrawRectangle(): draws rectangle
 *
 *************************************************************************/

/*ARGSUSED*/

void GDrawRectangle(int dev, 
		    const gframe_t *frame, 
		    GC xgc, 
		    const psgc_t *psgc, 
		    double wx, 
		    double wy, 
		    double wwidth, 
		    double wheight)

{

  int x, y;
  size_t width, height;

  switch (dev) {

  case XDEV:

    width = (size_t) (wwidth * frame->x->xscale + 0.5);
    height = (size_t) (wheight * frame->x->yscale + 0.5);
    x = (int) ((wx - frame->w_xmin) * frame->x->xscale + 0.5);
    y = (int) ((frame->w_ymax - wy) * frame->x->yscale + 0.5) - height;

    XDrawRectangle(frame->x->display, frame->x->drawable, xgc,
		   x, y, width, height);

    break;

  case PSDEV:

/*    width = (size_t) (wwidth * frame->ps->xscale + 0.5);
    height = (size_t) (wheight * frame->ps->yscale + 0.5);
    x = (int) ((wx - frame->w_xmin) * frame->ps->xscale
	       + frame->ps->xmin + 0.5);
    y = (int) ((wy - frame->w_ymin) * frame->ps->yscale
	       + frame->ps->ymin + 0.5);

    fprintf(frame->psgc->file, " %d %d %d %d DrawRect\n",
	    x, y, width, height); */
    
    fprintf(frame->psgc->file, " %g %g %g %g DrawRect\n",
	    (wx - frame->w_xmin) * frame->ps->xscale + frame->ps->xmin,
	    (wy - frame->w_ymin) * frame->ps->yscale + frame->ps->ymin,
	    wwidth * frame->ps->xscale,
	    wheight * frame->ps->yscale);

    break;

  }

}

/**************************************************************************
 * GDrawString(): draws string
 *
 *************************************************************************/

void GDrawString(int dev, 
		 const gframe_t *frame, 
		 GC xgc, 
		 XFontStruct *font, 
		 const psgc_t *psgc, 
		 int xjust, 
		 int yjust, 
		 double wx, 
		 double wy, 
		 const char *text)

{
  g_draw_string(dev, frame, xgc, font, psgc, xjust, yjust,
		wx, wy, 0, 0, text, FALSE);
}

/**************************************************************************
 * GDrawStringOffset(): draws string with given pixel offsets
 *
 *************************************************************************/

void GDrawStringOffset(int dev, 
		       const gframe_t *frame, 
		       GC xgc, 
		       XFontStruct *font, 
		       const psgc_t *psgc, 
		       int xjust, 
		       int yjust, 
		       double wx, 
		       double wy, 
		       int x_offset,
		       int y_offset,
		       const char *text)

{
  g_draw_string(dev, frame, xgc, font, psgc, xjust, yjust,
		wx, wy, x_offset, y_offset, text, FALSE);
}

/**************************************************************************
 * GDrawWindBarb(): draws a wind barb in the format used by WSDDM
 *
 *************************************************************************/

void GDrawWindBarb(int dev, 
		   const gframe_t *frame, 
		   GC xgc, 
		   const psgc_t *psgc, 
		   double world_x,
		   double world_y,
		   double u_wind,
		   double v_wind,
		   int barb_shaft_len)

{

  int pixel_x, pixel_y;

  /*
   * return early if the barb cannot fall within the frame
   */

  if (world_x < frame->w_xmin ||
      world_y < frame->w_ymin ||
      world_x > frame->w_xmax ||
      world_y > frame->w_ymax)
    return;

  /*
   * plot the barb
   */

  switch (dev) {

  case XDEV:

    pixel_x = (int) ((world_x - frame->w_xmin) * frame->x->xscale + 0.5);
    pixel_y = (int) ((frame->w_ymax - world_y) * frame->x->yscale + 0.5);

    XUDRwind_barb(frame->x->display, frame->x->drawable, xgc,
		  pixel_x, pixel_y, u_wind, v_wind, barb_shaft_len);
    
    break;

  case PSDEV:

    fprintf(stderr,
	    "WARNING: GDrawWindBarb() cannot draw barbs to PostScript devices\n");
    
    break;

  }

}

/**************************************************************************
 * GFillArc()
 *
 * Fill an arc, counterclockwise from wangle1 to wangle2,
 * as part of an ellipse having radii wradiusx and wradiusy in each
 * axis dirn respectively. The center is at wx, wy. The whole ellipse
 * may be rotated anticlockwise by angle axis_rotation. All angles
 * in degrees. The X implementation uses the nsegments arg to
 * determine the number of line segments to be used to represent
 * the arc.
 *
 *************************************************************************/

void GFillArc(int dev, 
	      const gframe_t *frame, 
	      GC xgc, 
	      const psgc_t *psgc, 
	      double wx, 
	      double wy, 
	      double wradiusx, 
	      double wradiusy, 
	      double wangle1, 
	      double wangle2, 
	      double axis_rotation, 
	      int nsegments)

{

  int circular;
  int isegment;
  double x, y;
  double ps_rx, ps_ry;
  double delta_theta;
  double start_theta, end_theta;
  double start_phi, end_phi;
  double start_x, start_y, end_x, end_y;
  double radius;
  double ratio_factor;
  double axis_ratio;
  double radius_max;
  double xmin, ymin, xmax, ymax;
  GPoint *points, *point;

  /*
   * return early if the arc cannot fall within the frame
   */

  radius_max = MAX(wradiusx, wradiusy);
  xmin = wx - radius_max;
  ymin = wy - radius_max;
  xmax = wx + radius_max;
  ymax = wy + radius_max;

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * plot the arc
   */

  switch (dev) {

  case XDEV:

    /*
     * decide if we are dealing with a circle or an ellipse
     */

    axis_ratio = fabs(wradiusx / wradiusy);

    if (0.999999 < axis_ratio && axis_ratio < 1.000001)
      circular = TRUE;
    else
      circular = FALSE;

    if (circular == TRUE)
      radius = wradiusx;
    else
      ratio_factor = pow(wradiusy / wradiusx, 2.0) - 1.0;

    /*
     * set up start angles
     */

    delta_theta = ((wangle2 - wangle1) * DEG_TO_RAD) / (double) nsegments;

    start_theta = wangle1 * DEG_TO_RAD;
    start_phi = start_theta + axis_rotation * DEG_TO_RAD;
    end_theta = start_theta + delta_theta;
    end_phi = start_phi + delta_theta;

    /*
     * set up start and end (x,y) coords
     */

    if (circular == FALSE)
      radius =
	wradiusy / sqrt(1.0 + pow(cos(start_theta), 2.0) * ratio_factor);

    start_x = wx + radius * cos(start_phi);
    start_y = wy + radius * sin(start_phi);

    if (circular == FALSE)
      radius = wradiusy / sqrt(1.0 + pow(cos(end_theta), 2.0) * ratio_factor);

    end_x = wx + radius * cos(end_phi);
    end_y = wy + radius * sin(end_phi);

    /*
     * load the segments
     */

    points = (GPoint *) ucalloc ((nsegments + 3), sizeof(GPoint));
    point = points;

    point->x = wx;
    point->y = wy;
    point++;

    point->x = start_x;
    point->y = start_y;
    point++;
    
    for (isegment = 0; isegment < nsegments; isegment++) {
      
      point->x = end_x;
      point->y = end_y;
      point++;
    
      /*
       * update segment ends
       */

      start_x = end_x;
      start_y = end_y;

      end_theta += delta_theta;
      end_phi += delta_theta;

      if (circular == FALSE)
	radius = wradiusy /
	  sqrt(1.0 + pow(cos(end_theta), 2.0) * ratio_factor);

      end_x = wx + radius * cos(end_phi);
      end_y = wy + radius * sin(end_phi);

    }

    point->x = wx;
    point->y = wy;

    GFillPolygon(dev, frame, xgc, psgc, points,
		 (int) (nsegments + 3), CoordModeOrigin);

    ufree ((char *) points);
    
    break;

  case PSDEV:

    x = (wx - frame->w_xmin) * frame->ps->xscale + frame->ps->xmin;
    y = (wy - frame->w_ymin) * frame->ps->yscale + frame->ps->ymin;
    ps_rx = wradiusx * frame->ps->xscale;
    ps_ry = wradiusy * frame->ps->yscale;

    fprintf(frame->psgc->file, " %g %g %g %g %g %g %g %g FillArc\n",
	    x, y, ps_rx, ps_ry, wangle1, wangle2, axis_rotation,
	    psgc->graylevel);
    
    break;

  }

}

/**************************************************************************
 * GFillPolygon(): fills polygon
 *
 *************************************************************************/

/*ARGSUSED*/

void GFillPolygon(int dev, 
		  const gframe_t *frame, 
		  GC xgc, 
		  const psgc_t *psgc, 
		  GPoint *gpoints, 
		  int npoints, 
		  int mode)

     /*
      * mode is either CoordModeOrigin or CoordModePrevious -
      * see X11R4 or later
      */
     
{

  int i;
  double xmin, ymin, xmax, ymax;
  GPoint *gpoint;
  PsPoint *pspoints, *pspoint, *prev_pspoint;
  XPoint *xpoints, *xpoint;

  /*
   * return early if the line cannot fall within the frame
   */

  xmin = LARGE_DOUBLE;
  ymin = LARGE_DOUBLE;
  xmax = -LARGE_DOUBLE;
  ymax = -LARGE_DOUBLE;

  gpoint = gpoints;

  for (i = 0; i < npoints; i++) {

    xmin = MIN(xmin, gpoint->x);
    ymin = MIN(ymin, gpoint->y);
    xmax = MAX(xmax, gpoint->x);
    ymax = MAX(ymax, gpoint->y);

    gpoint++;

  } /* i */

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * plot the lines
   */

  switch (dev) {

  case XDEV:

    /*
     * create array of XPoints, and load them up
     */

    xpoints = (XPoint *) ucalloc (npoints, sizeof(XPoint));

    xpoint = xpoints;
    gpoint = gpoints;

    for (i = 0; i < npoints; i++) {

      xpoint->x = (int) ((gpoint->x - frame->w_xmin) * frame->x->xscale + 0.5);
      xpoint->y = (int) ((frame->w_ymax - gpoint->y) * frame->x->yscale + 0.5);

      xpoint++;
      gpoint++;

    }

    XFillPolygon(frame->x->display, frame->x->drawable, xgc,
		 xpoints, npoints, Complex, mode);

    ufree((char *) xpoints);

    break;

  case PSDEV:

    PsGsave(frame->psgc->file);

    /*
     * set the gralevel
     */
    
    fprintf(frame->psgc->file, "%g setgray\n", psgc->graylevel);

    pspoints = (PsPoint *) ucalloc (npoints, sizeof(PsPoint));

    pspoint = pspoints;
    gpoint = gpoints;

    for (i = 0; i < npoints; i++) {

      pspoint->x = (int) ((gpoint->x - frame->w_xmin) * frame->ps->xscale
			  + frame->ps->xmin + 0.5);
      pspoint->y = (int) ((gpoint->y - frame->w_ymin) * frame->ps->yscale
			  + frame->ps->ymin + 0.5);

      pspoint++;
      gpoint++;

    }
      
    /* 
     * if mode is CoordModePrevious, the points are given relative to the 
     * previous, so compute relative to origin
     */

    if (mode == CoordModePrevious) {

      prev_pspoint = pspoints;
      pspoint = prev_pspoint + 1;

      for (i = 1; i < npoints; i++) {

	pspoint->x += prev_pspoint->x;
	pspoint->y += prev_pspoint->y;

	pspoint++;
	prev_pspoint++;
	
      } /* i */

    } /* if (mode == CoordModePrevious) */

    /*
     * write to ps file
     */

    pspoint = pspoints;

    fprintf(frame->psgc->file,
	    " newpath %d %d moveto\n", pspoint->x, pspoint->y);

    for (i = 1; i < npoints; i++) {

      pspoint++;
      fprintf(frame->psgc->file, " %d %d lineto\n", pspoint->x, pspoint->y);

    } /* i */

    fprintf(frame->psgc->file, " fill\n");

    ufree((char *) pspoints);

    break;

    PsGrestore(frame->psgc->file);

  }

}

/**************************************************************************
 * GFillRectangle(): fills rectangle
 *
 *************************************************************************/

void GFillRectangle(int dev, 
		    const gframe_t *frame, 
		    GC xgc, 
		    const psgc_t *psgc, 
		    double wx, 
		    double wy, 
		    double wwidth, 
		    double wheight)

{

  int x, y;
  size_t width, height;

  switch (dev) {

  case XDEV:

    width = (size_t) (wwidth * frame->x->xscale + 1.0);
    height = (size_t) (wheight * frame->x->yscale + 1.0);
    x = (int) ((wx - frame->w_xmin) * frame->x->xscale + 0.5);
    y = (int) ((frame->w_ymax - (wy + wheight)) * frame->x->yscale + 0.5);

    XFillRectangle(frame->x->display, frame->x->drawable, xgc,
		   x, y, width, height);

    break;

  case PSDEV:

    fprintf(frame->psgc->file, " %g %g %g %g %.3f FillRect\n",
	    (wx - frame->w_xmin) * frame->ps->xscale + frame->ps->xmin,
	    (wy - frame->w_ymin) * frame->ps->yscale + frame->ps->ymin,
	    wwidth * frame->ps->xscale,
	    wheight * frame->ps->yscale,
	    psgc->graylevel);

    break;

  }

}

/**************************************************************************
 * GFreeFrame(): frees a gframe_t structure and substructures
 *
 *************************************************************************/

void GFreeFrame(gframe_t *frame)

{

  if (frame != NULL) {

    if (frame->x != NULL)
      ufree((char *) frame->x);

    if (frame->ps != NULL)
      ufree((char *) frame->ps);

    if (frame->psgc != NULL)
      ufree((char *) frame->psgc);

    ufree((char *) frame);

  }

}

/*************************************************************************
 * GLinearTicks.c
 *
 * Sets linear tick interval and tick number for an axis,
 * given the mimumum and maximum values, and the
 * approximate number of ticks desired.
 *
 * Ticks are always at 1eX, 2eX or 5eX, where X varies
 * according to the data range
 *
 * Mike Dixon RAP, NCAR, Boulder, Colorado, November 1990
 *
 **************************************************************************/

void GLinearTicks(double min,
		  double max,
		  long approx_nticks,
		  long *nticks,
		  double *tickmin,
		  double *delta_tick)

{

  double approx_interval;
  double log_interval;
  double fract_part, rem;
  double int_part;
  double base;

  approx_interval = (max - min) / (double) (approx_nticks + 1);
  log_interval = log10(fabs(approx_interval));
  fract_part = modf(log_interval, &int_part);

  if (fract_part < 0) {
    fract_part += 1.0;
    int_part -= 1.0;
  }

  rem = pow(10.0, fract_part);

  if (rem > 7.5)
    base = 10.0;
  else if (rem > 3.5)
    base = 5.0;
  else if (rem > 1.5)
    base = 2.0;
  else 
    base = 1.0;

  *delta_tick = (base * pow (10.0, int_part));
  *tickmin = floor((min /* + *delta_tick */) / *delta_tick) * *delta_tick;

  *nticks = (long) ((max - *tickmin) / *delta_tick) + 1;

}

/*********************************************************************
 * GLoadGCScale()
 *
 * Loads up the GC pointer array for a given color scale.
 * The GC pointer array is then used for locating a GC with the
 * color relevant to a byte data value in the field.
 *
 * Returns 0 on success, 1 on failure
 *
 *********************************************************************/

int GLoadGCScale(g_color_scale_t *color_scale, 
		 Display *display, 
		 Colormap cmap, 
		 x_color_list_index_t *color_index, 
		 double scale, 
		 double bias)
{
  
  int ilevel, ival;
  int start_level;
  double data_val;

  /* 
   * set scale and bias and number of data vals
   */

  color_scale->scale = scale;
  color_scale->bias = bias;
  color_scale->nvalues = N_BYTE_DATA_VALS;

  /*
   * set x color GC for each level
   */

  for (ilevel = 0; ilevel < color_scale->nlevels; ilevel++) {

    color_scale->level[ilevel].gc =
      xGetColorGC(display, cmap, color_index,
		  color_scale->level[ilevel].colorname);


    if (color_scale->level[ilevel].gc == NULL) {
      fprintf(stderr, "ERROR - GLoadGCScale.\n");
      fprintf(stderr, "Getting GC for color '%s', level %d\n",
	      color_scale->level[ilevel].colorname, ilevel);
      return(FAILURE);
    }

  } /* ilevel */

  /*
   * assign a GC pointer to each possible data value
   */

  color_scale->gc = (GC *) ucalloc
    (color_scale->nvalues, sizeof(GC));

  start_level = 0;
  data_val = bias;

  for (ival = 0; ival < color_scale->nvalues; ival++) {

    color_scale->gc[ival] = NULL;

    for (ilevel = start_level; ilevel < color_scale->nlevels; ilevel++) {

      if ((data_val >= color_scale->level[ilevel].start) &&
	  (data_val <= color_scale->level[ilevel].end)) {

	color_scale->gc[ival] = color_scale->level[ilevel].gc;
	start_level = ilevel;

	break;
	  
      }

    } /* ilevel */

    data_val += scale;

  } /* ival */

  return(SUCCESS);

}


/*********************************************************************
 * GLoadPsgcScale()
 *
 * Loads up the psgc_t pointer array for a given color scale.
 * The psgc_t pointer array is then used for locating a psgc with the
 * color or gray scale relevant to a byte data value in the field.
 *
 * Returns 0 on success, 1 on failure
 *
 *********************************************************************/

int GLoadPsgcScale(g_color_scale_t *color_scale, 
		   Display *display, 
		   Colormap cmap, 
		   double scale, 
		   double bias)
{
  
  int ilevel, ival;
  int start_level;
  double data_val;
  double glevel;
  XColor x_color;

  /* 
   * set scale and bias and number of data vals
   */

  color_scale->scale = scale;
  color_scale->bias = bias;
  color_scale->nvalues = N_BYTE_DATA_VALS;

  /*
   * set psgc gray level for each data level
   */

  for (ilevel = 0; ilevel < color_scale->nlevels; ilevel++) {

    /*
     * get x color for the colorname
     */

    if (XParseColor(display, cmap,
		    color_scale->level[ilevel].colorname,
		    &x_color) == 0) {

      fprintf(stderr, "ERROR - GLoadPsgcScale\n");
      fprintf(stderr, "Cannot match color '%s' for level %d\n",
	      color_scale->level[ilevel].colorname, ilevel);
      
      return(FAILURE);
      
    }

    XAllocColor(display, cmap, &x_color);

    /*
     * compute graylevel
     */

    glevel = 1.0 -
      ((((double) x_color.red +
	 (double) x_color.green +
	 (double) x_color.blue)) /
       (3.0 * (double) X_COLOR_MAX));

    color_scale->level[ilevel].psgc =
      (psgc_t *) ucalloc (1, sizeof(psgc_t));

    color_scale->level[ilevel].psgc->graylevel = glevel;

    /*
     * set the graylevel hex string for postscript gray images
     */

    sprintf(color_scale->level[ilevel].psgc->hexstring, "%x",
	    (int) (glevel * PS_GRAY_MAX + 0.5));

    /*
     * free up the color
     */

    XFreeColors(display, cmap, &x_color.pixel, 1, 0L);

  } /* ilevel */

  /*
   * assign a psgc_t pointer to each possible data value
   */

  color_scale->psgc = (psgc_t **) ucalloc
    (color_scale->nvalues, sizeof(psgc_t *));

  start_level = 0;
  data_val = bias;

  for (ival = 0; ival < color_scale->nvalues; ival++) {

    color_scale->psgc[ival] = NULL;

    for (ilevel = start_level; ilevel < color_scale->nlevels; ilevel++) {

      if ((data_val >= color_scale->level[ilevel].start) &&
	  (data_val <= color_scale->level[ilevel].end)) {

	color_scale->psgc[ival] = color_scale->level[ilevel].psgc;
	start_level = ilevel;

	break;
	  
      }

    } /* ilevel */

    data_val += scale;

  } /* ival */

  return(SUCCESS);

}


/**************************************************************************
 * GMoveFrameWorld(): moves a frame's world coords
 *
 *************************************************************************/

void GMoveFrameWorld(gframe_t *frame, 
		     double w_xmin, 
		     double w_ymin, 
		     double w_xmax, 
		     double w_ymax)

{

  frame->w_xmin = w_xmin;
  frame->w_ymin = w_ymin;
  frame->w_xmax = w_xmax;
  frame->w_ymax = w_ymax;

}

/************************************************************************
 * GPageSetup: set up page origin and scale
 *
 *************************************************************************/

void GPageSetup(int dev, 
		const gframe_t *vert_page_frame, 
		const gframe_t *horiz_page_frame, 
		double print_width, 
		double ps_total_width, 
		double ps_total_height, 
		double page_width, 
		double page_length, 
		double width_margin, 
		double length_margin, 
		FILE *ps_file)

{

  int rotate_page, perhaps_rotate_page;
  double total_aspect_ratio, page_aspect_ratio;
  double print_height;
  double width_factor, height_factor;
  double final_scale, final_width, final_height;
  double origin_x, origin_y;
  double eff_page_length, eff_page_width;

  if(dev == XDEV)
    return;

  /*
   * set file pointer for psgc
   */

  vert_page_frame->psgc->file = ps_file;
  horiz_page_frame->psgc->file = ps_file;

  /*
   * compute effective page size, considering margins
   */

  eff_page_length = page_length - 2.0 * length_margin;
  eff_page_width = page_width - 2.0 * width_margin;

  /*
   * compute aspect ratios etc
   */

  total_aspect_ratio = ps_total_width / ps_total_height;
  page_aspect_ratio = page_width / page_length;
  print_height = print_width / total_aspect_ratio;

  /*
   * decide if page rotation should be a possibility
   */

  if (((total_aspect_ratio >= 1.0) && (page_aspect_ratio >= 1.0)) ||
      ((total_aspect_ratio <= 1.0) && (page_aspect_ratio <= 1.0)))
    perhaps_rotate_page = FALSE;
  else
    perhaps_rotate_page = TRUE;

  /*
   * decide if plot will fit on vertical page as is
   */

  if ((print_width <= eff_page_width) && (print_height <= eff_page_length)) {

    final_scale  = print_width / ps_total_width;
    rotate_page  = FALSE;

  } else {

    /*
     * if the aspect ratios indicated that the page should not be rotated,
     * scale the plot to fit the page
     */

    if (!perhaps_rotate_page) {

      rotate_page = FALSE;

      width_factor = print_width / eff_page_width;
      height_factor = print_height / eff_page_length;

      if (width_factor > height_factor)
	final_scale = eff_page_width / ps_total_width;
      else
	final_scale = eff_page_length / ps_total_height;

    } else {

      /*
       * rotate page
       */

      rotate_page = TRUE;

      /*
       * check if the plot will now fit without scaling
       */

      if ((print_width < eff_page_length) && (print_height < eff_page_width)) {

	final_scale = print_width / ps_total_width;

      } else {

	/*
	 * scale the plot to fit the rotated page
	 */

	width_factor = print_width / eff_page_length;
	height_factor = print_height / eff_page_width;

	if (width_factor > height_factor)
	  final_scale = eff_page_length / ps_total_width;
	else
	  final_scale  = eff_page_width / ps_total_height;

      } /* if ((print_width < eff_page_length ........ */

    } /* if (!perhaps_rotate_page ........ */

  } /* if ((print_width <= eff_page_width)..... */

  /*
   * set up page as required
   */

  if (rotate_page)  {

    GTranslate(PSDEV, vert_page_frame, page_width, 0.0);
    PsRotate(vert_page_frame->psgc->file, 90.0);

  }

  final_width = final_scale * ps_total_width;
  final_height = final_scale * ps_total_height;

  if (rotate_page) {

    origin_x = (page_length - final_width) / 2.0;
    origin_y = (page_width - final_height) / 2.0;

    GTranslate(PSDEV, horiz_page_frame, origin_x, origin_y);

  } else {


    origin_x = (page_width - final_width) / 2.0;
    origin_y = (page_length - final_height) / 2.0;

    GTranslate(PSDEV, vert_page_frame, origin_x, origin_y);

  }

  PsScale(vert_page_frame->psgc->file, final_scale, final_scale);

}

/**************************************************************************
 * GPsInitFrame(): initializes the postscript props of a frame
 *
 *************************************************************************/

void GPsInitFrame(psgc_t *psgc, 
		  FILE *file, 
		  const char *fontname, 
		  double fontsize, 
		  long line_width)

{

  psgc->file = file;
  psgc->fontname = (char *) umalloc((size_t) (strlen(fontname) + 1));
  strcpy(psgc->fontname, fontname);
  psgc->fontsize = fontsize;
  psgc->line_width = line_width;
  psgc->graylevel = 1.0;

}


/**************************************************************************
 * GPsSetGeomFrame(): sets frame geometry relative to postscript page
 *
 *************************************************************************/

void GPsSetGeomFrame(gframe_t *frame, 
		     int xmin, 
		     int ymin, 
		     size_t width, 
		     size_t height)

{

  frame->ps->xmin = xmin;
  frame->ps->ymin = ymin;
  frame->ps->width = width;
  frame->ps->height = height;

  if (frame->w_xmax != frame->w_xmin)
    frame->ps->xscale = (double) width / (frame->w_xmax - frame->w_xmin);
  else
    frame->ps->xscale = 1.0;
  
  if (frame->w_ymax != frame->w_ymin)
    frame->ps->yscale = (double) height / (frame->w_ymax - frame->w_ymin);
  else
    frame->ps->yscale = 1.0;

}

/*********************************************************************
 * GReadColorScale()
 *
 * Reads a color scale file and loads up the levels in a
 * g_color_scale_t struct. Allpcates the space for all
 * structs needed, including the g_color_scale_t struct.
 *
 * Returns 0 on success, 1 on failure
 *
 *********************************************************************/

int GReadColorScale(const char *file_name, 
		    g_color_scale_t **color_scale_ptr)

{

  char line[BUFSIZ];
  char *colorname;
  long ilevel  = 0, nlevels = 0;
  double start_val, end_val;
  g_color_scale_t *color_scale;
  FILE *color_scale_file;
  
  /*
   * open color_scale file
   */

  if ((color_scale_file = fopen(file_name, "r")) == NULL) {
    fprintf(stderr, "ERROR - GReadColorScale.\n");
    fprintf(stderr, "Cannot open color map file.\n");
    perror(file_name);
    exit(-1);
  }

  /*
   * allocate space for color_scale struct
   */

  color_scale = (g_color_scale_t *)
    ucalloc (1, sizeof(g_color_scale_t));
  *color_scale_ptr = color_scale;

  /*
   * read once through file to determine how many
   * lines there are with valid entries
   */

  while(fgets(line, BUFSIZ, color_scale_file) != NULL) {

    if (parse_color_scale_line(line, &start_val, &end_val,
			       &colorname) == SUCCESS) {
      nlevels++;
    }

  } /* while */

  /*
   * check that some levels were found
   */

  if (nlevels == 0) {
    fclose(color_scale_file);
    return(FAILURE);
  }
  
  /*
   * allocate space for levels
   */
  
  color_scale->nlevels = nlevels;

  color_scale->level = (g_color_scale_level_t *)
    ucalloc(nlevels, sizeof(g_color_scale_level_t));

  /*
   * rewind file
   */

  fseek(color_scale_file, 0L, 0);

  /*
   * read in the color data for the levels
   */

  while(fgets(line, BUFSIZ, color_scale_file) != NULL) {

    if (parse_color_scale_line(line, &start_val, &end_val,
			       &colorname) == SUCCESS) {

      color_scale->level[ilevel].start = start_val;
      color_scale->level[ilevel].end = end_val;

      color_scale->level[ilevel].colorname = 
	(char *) umalloc ((size_t) (strlen(colorname) + 1));

      strcpy(color_scale->level[ilevel].colorname, colorname);

      ilevel++;

    }

  } /* while */

  /*
   * close file
   */

  fclose(color_scale_file);

  return(SUCCESS);
  
}


/*****************************************************************************
 * parse_color_scale_line
 *
 * parses line in color scale file, and fills the start, end and
 * colorname fields.
 *
 * Returns 0 on success, 1 on failure
 */

static int parse_color_scale_line(char *line, 
				  double *start_val, 
				  double *end_val, 
				  char **colorname)

{

  char *start_pt, *end_pt;
  double start, end;

  *colorname = NULL;
  *start_val = 0.0;
  *end_val = 0.0;

  /*
   * get rgb values
   */

  line[strlen(line) - 1] = '\0';

  errno = 0;
  start_pt = line;
  start = strtod(start_pt, &end_pt);
  if (errno != 0 || start_pt == end_pt) {
    return (FAILURE);
  }

  errno = 0;
  start_pt = end_pt;
  end = strtod(start_pt, &end_pt);
  if (errno != 0 || start_pt == end_pt) {
    return (FAILURE);
  }

  start_pt = end_pt;
  while (*start_pt == ' ' || *start_pt == '\t')
    start_pt++;

  if (*start_pt == '\0') {
    return (FAILURE);
  }
  
  end_pt = start_pt + strlen(start_pt) - 1;

  while (*end_pt == ' ' || *end_pt == '\t') {
    *end_pt = '\0';
    end_pt--;
  }

  *colorname = start_pt;
  *start_val = start;
  *end_val = end;
  
  return(SUCCESS);

}

/**************************************************************************
 * GScale() : scale the page in worlds coords
 *
 *************************************************************************/

void GScale(int dev, 
	    const gframe_t *frame, 
	    double wwidth, 
	    double wheight)

{

  size_t width, height;
  
  switch (dev) {

  case XDEV:
    break;

  case PSDEV:

    width = (size_t) (wwidth * frame->ps->xscale + 0.5);
    height = (size_t) (wheight * frame->ps->yscale + 0.5);

    fprintf(frame->psgc->file, "%d %d scale\n", width, height);

    break;

  }

}


/**************************************************************************
 * GSetClipRectangles(): sets clipping rectangles in world coords
 *
 *************************************************************************/

void GSetClipRectangles(int dev, 
			const gframe_t *frame, 
			GC xgc, 
			const psgc_t *psgc, 
			GRectangle *grectangles, 
			int n)

{

  XRectangle *xrectangles;
  int x, y;
  size_t width, height;
  int clip_x_origin = 0, clip_y_origin = 0;
  int ordering = Unsorted;
  int i;

  switch (dev) {

  case XDEV:

    for  (i = 0; i < n; i++) {

      xrectangles = (XRectangle *) ucalloc(n, sizeof(XRectangle));

      xrectangles[i].width =
	(unsigned short) (grectangles[i].width * frame->x->xscale + 0.5);
      xrectangles[i].height =
	(unsigned short) (grectangles[i].height * frame->x->yscale + 0.5);
      xrectangles[i].x = 
	(short) ((grectangles[i].x - frame->w_xmin) * frame->x->xscale + 0.5);
      xrectangles[i].y = 
	(short) ((frame->w_ymax - (grectangles[i].y + grectangles[i].height)) *
		frame->x->yscale + 0.5);

    }

    XSetClipRectangles(frame->x->display, xgc, clip_x_origin, clip_y_origin,
		       xrectangles, n, ordering);

    ufree((char *) xrectangles);

    break;

  case PSDEV:

    for  (i = 0; i < n; i++) {

      width = (size_t) (grectangles[i].width * frame->ps->xscale + 0.5);
      height = (size_t) (grectangles[i].height * frame->ps->yscale + 0.5);
      x = (int) ((grectangles[i].x - frame->w_xmin) * frame->ps->xscale
		 + frame->ps->xmin + 0.5);
      y = (int) ((grectangles[i].y - frame->w_ymin) * frame->ps->yscale
		 + frame->ps->ymin + 0.5);

      fprintf(psgc->file, " %d %d %d %d SetClipRect\n",
	      x, y, width, height);
    
    }

    break;

  }

}

/**************************************************************************
 * GSetPsGC(): sets a psgc_t structure's values
 *
 *************************************************************************/

void GSetPsGC(psgc_t *psgc, 
	      FILE *file, 
	      const char *fontname, 
	      double fontsize, 
	      long line_width)

{

  psgc->file = file;
  psgc->fontname = (char *) umalloc(strlen(fontname) + 1);
  strcpy(psgc->fontname, fontname);
  psgc->fontsize = fontsize;
  psgc->line_width = line_width;
  psgc->graylevel = 1.0;

}

/**************************************************************************
 * GSetPsRef(): sets an psref_t structure's values
 *
 *************************************************************************/

void GSetPsRef(gframe_t *frame, 
	       int xmin, 
	       int ymin, 
	       size_t width, 
	       size_t height)

{

  frame->ps->xmin = xmin;
  frame->ps->ymin = ymin;
  frame->ps->width = width;
  frame->ps->height = height;
  frame->ps->xscale = (double) width / (frame->w_xmax - frame->w_xmin);
  frame->ps->yscale = (double) height / (frame->w_ymax - frame->w_ymin);

}

/**************************************************************************
 * GSetXRef(): sets an xref_t structure's values
 *
 *************************************************************************/

void GSetXRef(gframe_t *frame, 
	      Display *display, 
	      Drawable drawable, 
	      size_t width, 
	      size_t height, 
	      XFontStruct *font)

{

  frame->x->display = display;
  frame->x->drawable = drawable;
  frame->x->width = width;
  frame->x->height = height;
  frame->x->xscale = (double) width / (frame->w_xmax - frame->w_xmin);
  frame->x->yscale = (double) height / (frame->w_ymax - frame->w_ymin);
  frame->x->font = font;

}

/**************************************************************************
 * GTranslate() : translate the coords system
 *
 * Mainly intended for postscript
 *
 *************************************************************************/

void GTranslate(int dev, 
		const gframe_t *frame, 
		double wx, 
		double wy)

{

  int x, y;

  switch (dev) {

  case XDEV:
    break;

  case PSDEV:

    x = (int) ((wx - frame->w_xmin) * frame->ps->xscale
	       + frame->ps->xmin + 0.5);
    y = (int) ((wy - frame->w_ymin) * frame->ps->yscale
	       + frame->ps->ymin + 0.5);

    fprintf(frame->psgc->file, "%d %d translate\n", x, y);

    break;

  }
    
}


/**************************************************************************
 * GXInitFrame(): initializes the X properties of a frame
 *
 *************************************************************************/

void GXInitFrame(gframe_t *frame, 
		 Display *display, 
		 Drawable drawable, 
		 XFontStruct *font)

{

  frame->x->display = display;
  frame->x->drawable = drawable;
  frame->x->font = font;

}


/**************************************************************************
 * GXCreateFrame(): create a frame, with window and pixmap
 *
 *************************************************************************/

void GXCreateFrame(gframe_t *frame,
		   Display *display,
		   Window parent,
		   int x, int y,
		   int width, int height,
		   int border_width,
		   int foreground, int background,
		   int border_color,
		   XFontStruct *font)

{

  frame->x->display = display;

  frame->x->drawable = XCreateSimpleWindow(display, parent,
					   x, y, width, height,
					   border_width,
					   foreground, background);
  
  XSetWindowBorder(display, frame->x->drawable, border_color);
  
  frame->x->pixmap =
    XCreatePixmap(display, XDefaultRootWindow(frame->x->display),
		  width, height,
		  XDefaultDepth(display, XDefaultScreen(display)));
  
  frame->x->font = font;

  GXSetGeomFrame(frame, width, height);

}


/**************************************************************************
 * GXSetGeomFrame(): sets the frame geometry relative to an X window
 *
 *************************************************************************/

void GXSetGeomFrame(gframe_t *frame, 
		    size_t width, 
		    size_t height)

{

  frame->x->width = width;
  frame->x->height = height;
  
  if (frame->w_xmax != frame->w_xmin)
    frame->x->xscale = (double) width / (frame->w_xmax - frame->w_xmin);
  else
    frame->x->xscale = 1.0;
  
  if (frame->w_ymax != frame->w_ymin)
    frame->x->yscale = (double) height / (frame->w_ymax - frame->w_ymin);
  else
    frame->x->yscale = 1.0;
  
}


/**************************************************************************
 * GXResetFrame(): reset sizes and geometry
 *
 *************************************************************************/

void GXResetFrame(gframe_t *frame, 
		  int x, int y,
		  size_t width, 
		  size_t height)

{

  XMoveResizeWindow(frame->x->display,
		    frame->x->drawable,
		    x, y, width, height);
  
  XFreePixmap(frame->x->display, frame->x->pixmap);
  
  frame->x->pixmap =
    XCreatePixmap(frame->x->display, XDefaultRootWindow(frame->x->display),
		  width, height,
		  XDefaultDepth(frame->x->display,
				XDefaultScreen(frame->x->display)));

  GXSetGeomFrame(frame, width, height);
  
}

/**************************************************************************
 * GXWindowx(): gets X window x coord from frame world x coord
 *
 *************************************************************************/

int GXWindowx(const gframe_t *frame, 
	      double worldx)

{

  return ((int) ((worldx - frame->w_xmin) * frame->x->xscale + 0.5));

}

/**************************************************************************
 * GXWindowy(): gets X window y coord from frame world y coord
 *
 *************************************************************************/

int GXWindowy(const gframe_t *frame, 
	      double worldy)

{

  return (frame->x->height -
	  (int) ((worldy - frame->w_ymin) * frame->x->yscale + 0.5));

}

/**************************************************************************
 * GXWorldx(): gets frame world x coord from X window x coord
 *
 *************************************************************************/

double GXWorldx(const gframe_t *frame, 
		int windowx)

{

  return ((double) windowx / frame->x->xscale + frame->w_xmin);

}

/**************************************************************************
 * GXWorldy(): gets frame world y coord from X window y coord
 *
 *************************************************************************/

double GXWorldy(const gframe_t *frame, 
		int windowy)

{

  return (((double) frame->x->height - windowy) / frame->x->yscale
	  + frame->w_ymin);

}

/**************************************************************************
 * GPsWindowx(): gets X window x coord from frame world x coord
 *
 *************************************************************************/

int GPsWindowx(const gframe_t *frame, 
	       double worldx)
     
{
  
  return ((int) ((worldx - frame->w_xmin) * frame->ps->xscale + 0.5));
  
}

/**************************************************************************
 * GPsWindowy(): gets X window y coord from frame world y coord
 *
 *************************************************************************/

int GPsWindowy(const gframe_t *frame, 
	       double worldy)

{
  
  return (frame->x->height -
	  (int) ((worldy - frame->w_ymin) * frame->ps->yscale + 0.5));

}

/**************************************************************************
 * GPsWorldx(): gets frame world x coord from X window x coord
 *
 *************************************************************************/

double GPsWorldx(const gframe_t *frame, 
		 int windowx)
     
{
  
  return ((double) windowx / frame->ps->xscale + frame->w_xmin);

}

/**************************************************************************
 * GPsWorldy(): gets frame world y coord from X window y coord
 *
 *************************************************************************/

double GPsWorldy(const gframe_t *frame, 
		 int windowy)
     
{
  
  return (((double) frame->x->height - windowy) / frame->ps->yscale
	  + frame->w_ymin);

}

