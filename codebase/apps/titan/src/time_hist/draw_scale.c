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
 * draw_scale: draw legend scale
 *
 * time_hist routine
 * 
 * Mike Dixon, RAP, NCAR, Boulder, Colorado USA
 *
 * April 1991
 *
 *************************************************************************/

#include "time_hist.h"

#define TEXT_MARGIN 0.075
#define LEGENDX 0.075
#define LEGENDY1 0.05
#define LEGENDY2 0.95

#define MAX_LINE_LEGENDS 4

void draw_scale(int dev,
		gframe_t *frame,
		g_color_scale_t *colors,
		int format,
		int plot_legend)

{

  int i;
  int n_line_legends;

  double legendwidth, elementheight;
  double legendx, legendy;
  double textx, texty;
  double lower, upper, next_upper;
  double max_scale_element_ht, w_max_scale_element_ht;
  double w_scale_element_width;

  char legend_str[80];
  char line_legend_str[MAX_LINE_LEGENDS][80];
  char *format_1, *format_2;

  GC line_legend_gc[MAX_LINE_LEGENDS];
  psgc_t *line_legend_psgc[MAX_LINE_LEGENDS];

  if (Glob->debug) {
    fprintf(stderr, "** draw_scale **\n");
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

    max_scale_element_ht =
      (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
			   "x_max_scale_element_ht",
			   X_MAX_SCALE_ELEMENT_HT);

    w_max_scale_element_ht =
      (double) max_scale_element_ht / frame->x->yscale;

    w_scale_element_width =
      (double) xGetResLong(Glob->rdisplay, Glob->prog_name,
			   "x_scale_element_width",
			   X_SCALE_ELEMENT_WIDTH) / frame->x->xscale;

    break;

  case PSDEV:

    max_scale_element_ht =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_max_scale_element_ht",
		    PS_MAX_SCALE_ELEMENT_HT);

    w_max_scale_element_ht =
      (double) max_scale_element_ht * Glob->ps_unitscale / frame->ps->yscale;

    w_scale_element_width =
      ((xGetResDouble(Glob->rdisplay, Glob->prog_name,
		      "ps_scale_element_width",
		      PS_SCALE_ELEMENT_WIDTH)) *
       (Glob->ps_unitscale / frame->ps->xscale));

    break;

  }

  /*
   * decide how many line legends are to be plotted
   */

  n_line_legends = 0;

  if (plot_legend == SCALE_PLOT_LEGENDS) {

    strcpy(line_legend_str[n_line_legends], "top/base");
    line_legend_gc[n_line_legends] = Glob->top_base_gc;
    line_legend_psgc[n_line_legends] = &Glob->top_base_psgc;
    n_line_legends++;
      
    if (Glob->timeht_centroids == SELECTED_LIMITED ||
	Glob->timeht_centroids == SELECTED_ALL) {

      strcpy(line_legend_str[n_line_legends], "Z centr");
      line_legend_gc[n_line_legends] = Glob->ht_refl_centroid_gc;
      line_legend_psgc[n_line_legends] = &Glob->ht_refl_centroid_psgc;
      n_line_legends++;
      
    }
      
    if (Glob->timeht_centroids == SELECTED_ALL) {

      strcpy(line_legend_str[n_line_legends], "centr");
      line_legend_gc[n_line_legends] = Glob->ht_centroid_gc;
      line_legend_psgc[n_line_legends] = &Glob->ht_centroid_psgc;
      n_line_legends++;

    }
    
    if (Glob->timeht_htmaxz != FALSE) {

      strcpy(line_legend_str[n_line_legends], "max Z");
      line_legend_gc[n_line_legends] = Glob->ht_maxdbz_gc;
      line_legend_psgc[n_line_legends] = &Glob->ht_maxdbz_psgc;
      n_line_legends++;
      
    }
    
  } /* if (plot_legend == SCALE_PLOT_LEGENDS) */

  /*
   * compute dimensions
   */

  elementheight = 
    ((double) LEGENDY2 - (double) LEGENDY1) /
      ((double) colors->nlevels + (double) n_line_legends);

  if (elementheight > w_max_scale_element_ht)
    elementheight = w_max_scale_element_ht;

  legendwidth = w_scale_element_width;

  legendx = (double) LEGENDX;
  legendy = (double) LEGENDY2 - elementheight;
  textx = legendx + legendwidth + TEXT_MARGIN;
  texty = legendy + elementheight;

  /*
   * draw in rectangles and their labels
   */

  if (Glob->timeht_mode != FALSE) {
    
    for (i = colors->nlevels - 1; i >= 0; i--) {
      
      switch (dev) {
	
      case XDEV:
	GFillRectangle(dev, frame,
		       colors->level[i].gc,
		       frame->psgc,
		       legendx, legendy, legendwidth, elementheight);
	break;
	
      case PSDEV:
	GFillRectangle(dev, frame,
		       frame->x->gc,
		       colors->level[i].psgc,
		       legendx, legendy, legendwidth, elementheight);
	break;
	
      }
      
      /*
       * set the label format
       */
      
      if (format == SCALE_LABEL_FORMAT_E) {
	
	format_1 = "%.0e";
	format_2 = "%.0e/%.0e";
	
      } else {
	
	format_1 = "%g";
	format_2 = "%g/%g";
	
      }      
      
      if (i == colors->nlevels - 1) {
	
	upper = colors->level[i].end;
	sprintf(legend_str, format_1, upper);
	GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		     XJ_LEFT, YJ_CENTER, textx, texty, legend_str);
	texty -= elementheight;
	
      }
      
      if(i != 0) {
	
	lower = colors->level[i].start;
	next_upper = colors->level[i-1].end;
	
	if (lower != next_upper)
	  sprintf(legend_str, format_2, lower, next_upper);
	else
	  sprintf(legend_str, format_1, lower);
	
      } else {
	
	lower = colors->level[i].start;
	sprintf(legend_str, format_1, lower);
	
      }
      
      GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		   XJ_LEFT, YJ_CENTER, textx, texty, legend_str);
      
      legendy -= elementheight;
      texty -= elementheight;
      
    } /* i */

  } /* if (Glob->timeht_mode != FALSE) */
    
  /*
   * draw in line legends
   */

  if (dev == PSDEV)
    PsGsave(frame->psgc->file);

  for (i = 0; i < n_line_legends; i++) {

    if (dev == PSDEV) {
      PsSetLineWidth(frame->psgc->file,
		     line_legend_psgc[i]->line_width);
      PsSetLineDash(frame->psgc->file,
		    line_legend_psgc[i]->dash_length,
		    line_legend_psgc[i]->space_length);
    }

    GDrawLine(dev, frame,
	      line_legend_gc[i],
	      frame->psgc,
	      legendx, texty,
	      legendx + legendwidth, texty);

    GDrawString (dev, frame, frame->x->gc, frame->x->font, frame->psgc,
		 XJ_LEFT, YJ_CENTER, textx, texty, line_legend_str[i]);

    texty -= elementheight;

  } /* i */

  if (dev == PSDEV)
    PsGrestore(frame->psgc->file);

  /*
   * draw frame border if postscript
   */

  if (dev == PSDEV) {
    GDrawPsFrameBorder(frame, Glob->ps_border_line_width);
  }

  safe_XFlush(Glob->rdisplay);

}
