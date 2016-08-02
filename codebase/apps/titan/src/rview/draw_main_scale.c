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
/************************************************************************
 * draw_main_scale: draw scale
 *
 * rview routine
 * 
 * Mike Dixon, RAP, NCAR, June 1990
 *************************************************************************/

#include <math.h>
#include <string.h>
#include "rview.h"

#define TEXT_MARGIN 0.1
#define LEGEND_X 0.1
#define LEGEND_Y1 0.05
#define LEGEND_Y2 0.94
#define UNITS_X 0.5
#define UNITS_Y 0.97

#define MAX_LINE_LEGENDS 1
#define MAX_STORM_LEGENDS 10
#define MAX_VECTOR_LEGENDS 10

#define N_STORM_SEGMENTS 180
#define ARROW_HEAD_ANGLE 20.0
#define ARROW_HEAD_LENGTH_FRACTION 0.01

void draw_main_scale(int dev,
		     g_color_scale_t *colors,
		     int symbol_legend_flag)

{

  int i;
  int plot_symbol_legends;
  int n_symbol_legends;
  int n_line_legends;
  int n_storm_legends, n_vector_legends;

  si32 nlevels;

  double legendwidth, legendheight;
  double legend_x, legend_y;
  double textx, texty;
  double lower, upper, next_upper;
  double unitscale;
  double w_max_legend_element_ht;
  double w_legend_element_width;
  double arrow_head_length;

  gframe_t *frame;

  char legend_str[80];

  char line_legend_str[MAX_LINE_LEGENDS][80];
  GC line_legend_gc[MAX_LINE_LEGENDS];

  char storm_legend_str[MAX_STORM_LEGENDS][80];
  GC storm_legend_gc[MAX_STORM_LEGENDS];
  psgc_t *storm_legend_psgc[MAX_STORM_LEGENDS];

  char vector_legend_str[MAX_VECTOR_LEGENDS][80];
  GC vector_legend_gc[MAX_VECTOR_LEGENDS];
  psgc_t *vector_legend_psgc[MAX_VECTOR_LEGENDS];

  if (Glob->debug) {
    fprintf(stderr, "** draw_main_scale **\n");
  }

  frame = Glob->main_scale_frame;

  if (colors) {
    nlevels = colors->nlevels;
  } else {
    nlevels = 0;
  }

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    safe_XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  /*
   * get resources - dimensions of legend blocks
   */

  switch (dev) {

  case XDEV:

    w_max_legend_element_ht =
      (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
			   "x_max_legend_element_ht",
			   X_MAX_LEGEND_ELEMENT_HT) / frame->x->yscale;

    w_legend_element_width =
      (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
			   "x_legend_element_width",
			   X_LEGEND_ELEMENT_WIDTH) / frame->x->xscale;

    break;

  case PSDEV:

    unitscale = xGetResDouble(Glob->rdisplay, Glob->prog_name,
			     "ps_unitscale", PS_UNITSCALE);

    w_max_legend_element_ht =
      ((xGetResDouble(Glob->rdisplay, Glob->prog_name,
		      "ps_max_legend_element_ht",
		      PS_MAX_LEGEND_ELEMENT_HT)) *
       (unitscale / frame->ps->yscale));

    w_legend_element_width =
      ((xGetResDouble(Glob->rdisplay, Glob->prog_name,
		      "ps_legend_element_width",
		      PS_LEGEND_ELEMENT_WIDTH)) *
       (unitscale / frame->ps->xscale));

    break;

  }

  /*
   * draw in units string
   */

  if (Glob->plot_image)
    GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		 XJ_CENTER, YJ_CENTER, UNITS_X, UNITS_Y,
		 Glob->cappi_info->field_units);
  
  /*
   * decide how many symbol legends are to be plotted
   */

  if (symbol_legend_flag == SCALE_PLOT_LEGENDS &&
      (Glob->plot_tracks || Glob->vsection.active || (n_aircraft() > 0)))
    plot_symbol_legends = TRUE;
  else
    plot_symbol_legends = FALSE;

  n_symbol_legends = 0;
  n_line_legends = 0;
  n_storm_legends = 0;
  n_vector_legends = 0;

  if (plot_symbol_legends) {

    if (Glob->vsection.active && dev == XDEV) {

      strcpy(line_legend_str[n_line_legends], "vsection");
      line_legend_gc[n_line_legends] = Glob->vsection_pos_gc;
      n_line_legends++;
      n_symbol_legends++;

    }

    if (Glob->plot_tracks) {

      if (Glob->track_graphic == ELLIPSES ||
	  Glob->track_graphic == POLYGONS) {
	
	if (Glob->plot_past) {
	  strcpy(storm_legend_str[n_storm_legends], "past");
	  storm_legend_gc[n_storm_legends] = Glob->past_storm_gc[0];
	  storm_legend_psgc[n_storm_legends] = &Glob->past_storm_psgc[0];
	  n_storm_legends++;
	  n_symbol_legends++;
	}

	strcpy(storm_legend_str[n_storm_legends], "current");
	storm_legend_gc[n_storm_legends] = Glob->current_storm_gc;
	storm_legend_psgc[n_storm_legends] = &Glob->current_storm_psgc;
	n_storm_legends++;
	n_symbol_legends++;

	if (Glob->plot_future) {
	  strcpy(storm_legend_str[n_storm_legends], "future");
	  storm_legend_gc[n_storm_legends] = Glob->future_storm_gc[0];
	  storm_legend_psgc[n_storm_legends] = &Glob->future_storm_psgc[0];
	  n_storm_legends++;
	  n_symbol_legends++;
	}

	if (Glob->plot_forecast) {
	  strcpy(storm_legend_str[n_storm_legends], "forecast");
	  storm_legend_gc[n_storm_legends] = Glob->forecast_storm_gc[0];
	  storm_legend_psgc[n_storm_legends] = &Glob->forecast_storm_psgc[0];
	  n_storm_legends++;
	  n_symbol_legends++;
	}

      } /* if (Glob->track_graphic == ELLIPSES ... ) */

      if (Glob->plot_past) {
	strcpy(vector_legend_str[n_vector_legends], "past");
	vector_legend_gc[n_vector_legends] = Glob->past_vector_gc;
	vector_legend_psgc[n_vector_legends] = &Glob->past_vector_psgc;
	n_vector_legends++;
	n_symbol_legends++;
      }

      if (Glob->plot_future) {
	strcpy(vector_legend_str[n_vector_legends], "future");
	vector_legend_gc[n_vector_legends] = Glob->future_vector_gc;
	vector_legend_psgc[n_vector_legends] = &Glob->future_vector_psgc;
	n_vector_legends++;
	n_symbol_legends++;
      }
      
      if (Glob->plot_forecast) {
	strcpy(vector_legend_str[n_vector_legends], "forecast");
	vector_legend_gc[n_vector_legends] = Glob->forecast_vector_gc;
	vector_legend_psgc[n_vector_legends] = &Glob->forecast_vector_psgc;
	n_vector_legends++;
	n_symbol_legends++;
      }

    } /* if (Glob->plot_tracks) */

    for (i = 0; i < n_aircraft(); i++) {
      strcpy(vector_legend_str[n_vector_legends], get_aircraft_ident(i));
      vector_legend_gc[n_vector_legends] = get_aircraft_gc(i);
      vector_legend_psgc[n_vector_legends] = get_aircraft_psgc(i);
      n_vector_legends++;
      n_symbol_legends++;
    } /* i */

    if (get_plot_flares()) {

      strcpy(vector_legend_str[n_vector_legends], "eburn");
      XSetLineAttributes(Glob->rdisplay, get_end_burn_gc(), 1,
			 LineSolid, CapButt, JoinMiter);
      vector_legend_gc[n_vector_legends] = get_end_burn_gc();
      vector_legend_psgc[n_vector_legends] = get_aircraft_psgc(0);
      n_vector_legends++;
      n_symbol_legends++;
      
      strcpy(vector_legend_str[n_vector_legends], "bip");
      XSetLineAttributes(Glob->rdisplay, get_bip_gc(), 1,
			 LineSolid, CapButt, JoinMiter);
      vector_legend_gc[n_vector_legends] = get_bip_gc();
      vector_legend_psgc[n_vector_legends] = get_aircraft_psgc(0);
      n_vector_legends++;
      n_symbol_legends++;
      
      strcpy(vector_legend_str[n_vector_legends], "eb+bip");
      XSetLineAttributes(Glob->rdisplay, get_end_burn_and_bip_gc(), 1,
			 LineSolid, CapButt, JoinMiter);
      vector_legend_gc[n_vector_legends] = get_end_burn_and_bip_gc();
      vector_legend_psgc[n_vector_legends] = get_aircraft_psgc(0);
      n_vector_legends++;
      n_symbol_legends++;
      
    }

  } /* if (plot_symbol_legends) */
    
  /*
   * compute dimensions
   */

  legendheight = 
    (((double) LEGEND_Y2 - (double) LEGEND_Y1) /
     ((double) nlevels + (double) n_symbol_legends));
    
  if (legendheight > w_max_legend_element_ht)
    legendheight = w_max_legend_element_ht;

  legendwidth = w_legend_element_width;

  legend_x = (double) LEGEND_X;
  legend_y = (double) LEGEND_Y2 - legendheight;
  textx = legend_x + legendwidth + (double) TEXT_MARGIN;
  texty = legend_y + legendheight;

  /*
   * get arrow head length in world coordinates
   */

  arrow_head_length = (sqrt(pow(frame->w_xmax - frame->w_xmin, 2.0) +
			    pow(frame->w_ymax - frame->w_ymin, 2.0)) *
		       ARROW_HEAD_LENGTH_FRACTION);
  
  /*
   * draw in rectangles and their labels
   */
  
  if (Glob->plot_image) {
    
    for (i = nlevels - 1; i >= 0; i--) {

      switch (dev) {
	
      case XDEV:
	GFillRectangle(dev, frame,
		       colors->level[i].gc,
		       frame->psgc,
		       legend_x, legend_y, legendwidth, legendheight);
	break;
	
      case PSDEV:
	GFillRectangle(dev, frame,
		       frame->x->gc,
		       colors->level[i].psgc,
		       legend_x, legend_y, legendwidth, legendheight);
	break;
	
      }
      
      if (i == nlevels - 1) {
	
	upper = colors->level[i].end;
	sprintf(legend_str, "%g", upper);
	GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		     XJ_LEFT, YJ_CENTER, textx, texty, legend_str);
	texty -= legendheight;
	
      }
      
      if(i != 0) {
	
	lower = colors->level[i].start;
	next_upper = colors->level[i-1].end;
	
	if (lower != next_upper)
	  sprintf(legend_str, "%g/%g", lower, next_upper);
	else
	  sprintf(legend_str, "%g", lower);
	
      } else {
	
	lower = colors->level[i].start;
	sprintf(legend_str, "%g", lower);
	
      }
      
      GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		   XJ_LEFT, YJ_CENTER, textx, texty, legend_str);
      
      legend_y -= legendheight;
      texty -= legendheight;
      
    } /* i */
    
  } /* if (Glob->plot_image) */
  
  /*
   * plot in symbol legends if required
   */

  if (plot_symbol_legends) {

    if (dev == PSDEV)
      PsGsave(frame->psgc->file);

    for (i = 0; i < n_line_legends; i++) {

      GDrawLine(dev, frame,
		 line_legend_gc[i],
		 frame->psgc,
		 legend_x, texty,
		 legend_x + legendwidth, texty);

      GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		   XJ_LEFT, YJ_CENTER, textx, texty, line_legend_str[i]);

      texty -= legendheight;
      
    } /* i */
    
    for (i = 0; i < n_vector_legends; i++) {

      if (dev == PSDEV) {
	PsSetLineWidth(frame->psgc->file,
		       vector_legend_psgc[i]->line_width);
	PsSetLineDash(frame->psgc->file,
		      vector_legend_psgc[i]->dash_length,
		      vector_legend_psgc[i]->space_length);
      }

      GDrawArrow(dev, frame,
		 vector_legend_gc[i],
		 frame->psgc,
		 legend_x, texty,
		 legend_x + legendwidth, texty,
		 ARROW_HEAD_ANGLE, arrow_head_length);

      GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		   XJ_LEFT, YJ_CENTER, textx, texty, vector_legend_str[i]);

      texty -= legendheight;
      
    } /* i */
    
    for (i = 0; i < n_storm_legends; i++) {

      if (dev == PSDEV) {
	PsSetLineWidth(frame->psgc->file,
		       storm_legend_psgc[i]->line_width);
	PsSetLineDash(frame->psgc->file,
		      storm_legend_psgc[i]->dash_length,
		      storm_legend_psgc[i]->space_length);
      }

      GDrawArc(dev, frame,
	       storm_legend_gc[i],
	       frame->psgc,
	       legend_x + legendwidth / 2.0, texty,
	       legendwidth/2.0,
	       legendheight / 3.0,
	       0.0, 360.0, 0.0, N_STORM_SEGMENTS);

      GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		   XJ_LEFT, YJ_CENTER, textx, texty, storm_legend_str[i]);

      texty -= legendheight;

    } /* i */

    if (dev == PSDEV)
      PsGrestore(frame->psgc->file);
    
  } /* if (plot_symbol_legends) */
  
}
