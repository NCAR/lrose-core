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
 * draw_vsection.c
 *
 * vertical section drawing routines
 * 
 * Mike Dixon, RAP, NCAR, Boulder, CO, 80523, USA
 *
 * May 1992
 *
 *************************************************************************/

#include "rview.h"
#include <toolsa/servmap.h>

static ui08 *interpolate_vsection(ui08 *vert_data,
				    si32 *image_nl,
				    si32 *image_nz,
				    cd_grid_info_t *grid_info,
				    cd_reply_t *reply,
				    int dz_constant);

static ui08 *read_vsection_plane(date_time_t **time_ptr,
				   cd_grid_info_t **info_ptr,
				   cd_reply_t **reply_ptr,
				   int *dz_constant_ptr,
				   double **plane_heights_ptr);

static void draw_varying_dz_section(int dev,
				    gframe_t *frame,
				    g_color_scale_t *colors,
				    si32 image_nl,
				    si32 image_nz,
				    double delta_l,
				    double *plane_heights,
				    ui08 *image_data,
				    si32 bad_data_val);

/************************************************************************
 * draw_vsection_title()
 */

void draw_vsection_title(int dev)

{

  char tstring[BUFSIZ];
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_vsection_title **\n");
  }

  if (!Glob->vsection.active)
    return;

  frame = Glob->vsection_title_frame;

  /*
   * clear window
   */

  if (dev == XDEV) {
    XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
			 Glob->background);
    safe_XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  if (strcmp(Glob->vsection_info->source_name, "")) {

    sprintf(tstring,"VERT SECTION - %s %s",
	    Glob->vsection_info->source_name,
	    Glob->vsection_info->field_name);

  } else {

    sprintf(tstring,"VERT SECTION");

  }

  GDrawString(dev, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, tstring);

}

/************************************************************************
 * draw_vsection_button()
 */

void draw_vsection_button(si32 n,
			  ui32 background)

{

  static si32 first_call = TRUE;
  static char **button_label;
  char *label_str;
  int i;
  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** draw_vsection_button **\n");
  }

  frame = Glob->vsection_button_frame[n];

  /*
   * if first call, load up the label strings
   */

  if (first_call) {

    button_label = (char **) umalloc (N_VSECTION_BUTTONS * sizeof(char *));

    label_str = (char *) umalloc((ui32) (strlen(VSECTION_BUTTON_LABELS) + 1));
    strcpy(label_str, VSECTION_BUTTON_LABELS);

    for (i = 0; i < N_VSECTION_BUTTONS; i++) {

      if (i == 0)
	button_label[i] = strtok(label_str, " ");
      else
	button_label[i] = strtok((char *) NULL, " ");

    }

    first_call = FALSE;

  }

  /*
   * clear window and set background
   */

  XSetWindowBackground(Glob->rdisplay, frame->x->drawable,
		       background);
  safe_XClearWindow(Glob->rdisplay, frame->x->drawable);

  /*
   * write the label to the window
   */

  GDrawString(XDEV, frame, frame->x->gc, frame->x->font,
	      frame->psgc, XJ_CENTER, YJ_CENTER,
	      (double) 0.5, (double) 0.5, button_label[n]);

}

/************************************************************************
 * draw_vsection_plot()
 *
 * In X, goes to a pixmap. In PS, goes to an output file.
 */

#define HDR_MAX 80

void draw_vsection_plot(int dev,
			g_color_scale_t *colors)

{

  ui08 *vert_data, *image_data;
  char label[20];
  char hstring[HDR_MAX];

  int dz_constant;

  si32 i;
  si32 image_nl, image_nz;
  si32 ilabel;
  si32 xaxismargin, yaxismargin, topmargin;
  long approx_nticks, nticksx, nticksy;

  double tickmin_x, tickmin_y;
  double delta_tickx, delta_ticky;
  double min_x, max_x, min_y, max_y;
  double image_min_x, image_min_y, image_max_x, image_max_y;
  double delta_l;
  double gridx, gridy, textx, texty;
  double x_ticklength, ps_ticklength;
  double xticklength, yticklength, xtickmargin, ytickmargin;
  double xaxisendmargin, yaxisendmargin;
  double tick_clearance;
  double text_margin_x, text_margin_y;
  double unitscale;
  double *plane_heights;

  gframe_t *frame;
  vsection_t *vsection;

  date_time_t *vsection_time;

  cd_grid_info_t *grid_info;
  cd_reply_t *reply;

  Drawable window;
  GRectangle clip_rectangle[1];

  if (Glob->debug) {
    fprintf(stderr, "** draw_vsection_plot **\n");
  }

  /*
   * set local pointers
   */

  if (dev == XDEV)
    frame = Glob->vsection_plot_frame;
  else
    frame = Glob->vsection_ps_plot_frame;

  vsection = &Glob->vsection;

  /*
   * If device is an X window,
   * save the frame drawable in 'window', and set it temporarily 
   * to 'pixmap'. Also initialize the pixmap.
   */

  if (dev == XDEV) {
    window = frame->x->drawable;
    frame->x->drawable = vsection->pixmap;

    XFillRectangle(Glob->rdisplay, frame->x->drawable,
		   Glob->pixmap_gc, 0, 0,
		   frame->x->width, frame->x->height);
  }
  
  /*
   * get the vertical section image data - returns NULL on failure
   */

  if ((vert_data = read_vsection_plane(&vsection_time,
				       &grid_info,
				       &reply,
				       &dz_constant,
				       &plane_heights)) == NULL) {
    /*
     * data access failed
     *
     * set the plotframe drawable back to the window
     */
    
    if (dev ==  XDEV)
      frame->x->drawable = window;
    
    return;
    
  }
  
  /*
   * calculate the vsection geometry
   */

  vsection_calc(dev, frame, grid_info, reply, plane_heights);

  /*
   * adjust the color scale to suit the scale and bias of the data
   */

  if (dev == XDEV)
    GAdjustGCScale(colors, reply->scale, reply->bias);
  else
    GAdjustPsgcScale(colors, reply->scale, reply->bias);

  /*
   * get resources
   */

  switch (dev) {

  case XDEV:

    xaxismargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "x_xaxismargin", X_XAXISMARGIN);

    yaxismargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "x_yaxismargin", X_YAXISMARGIN);

    topmargin = xGetResLong(Glob->rdisplay, Glob->prog_name,
			    "x_topmargin", X_TOPMARGIN);
 
   xaxisendmargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				  "x_xaxisendmargin",
				  X_XAXISENDMARGIN) / frame->x->xscale;
 
   yaxisendmargin = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				  "x_yaxisendmargin",
				  X_YAXISENDMARGIN) / frame->x->yscale;

 
   tick_clearance = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				  "x_tick_clearance",
				  X_TICK_CLEARANCE);

    x_ticklength = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				"x_ticklength", X_TICKLENGTH);
    xticklength = x_ticklength / frame->x->yscale;
    yticklength = x_ticklength / frame->x->xscale;
    xtickmargin = tick_clearance / frame->x->yscale;
    ytickmargin = tick_clearance / frame->x->xscale;

    text_margin_x =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    text_margin_y =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->yscale;
    
    break;

  case PSDEV:

    unitscale = xGetResDouble(Glob->rdisplay, Glob->prog_name,
			     "ps_unitscale", PS_UNITSCALE);

    xaxismargin =
      (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
			   "ps_xaxismargin", PS_XAXISMARGIN) * unitscale);

    yaxismargin =
      (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
			   "ps_yaxismargin", PS_YAXISMARGIN) * unitscale);

    topmargin =
      (si32) (xGetResDouble(Glob->rdisplay, Glob->prog_name,
			   "ps_topmargin", PS_TOPMARGIN) * unitscale);

    xaxisendmargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_xaxisendmargin",
		   PS_XAXISENDMARGIN) * unitscale / frame->ps->xscale;
    yaxisendmargin =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_yaxisendmargin",
		   PS_YAXISENDMARGIN) * unitscale / frame->ps->yscale;

    tick_clearance =
      xGetResDouble(Glob->rdisplay, Glob->prog_name, "ps_tick_clearance",
		   PS_TICK_CLEARANCE) * unitscale;

    ps_ticklength = xGetResDouble(Glob->rdisplay, Glob->prog_name,
				 "ps_ticklength", PS_TICKLENGTH) * unitscale;

    xticklength = ps_ticklength / frame->ps->yscale;
    yticklength = ps_ticklength / frame->ps->xscale;
    xtickmargin = tick_clearance / frame->ps->yscale;
    ytickmargin = tick_clearance / frame->ps->xscale;

    text_margin_x =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_text_margin", PS_TEXT_MARGIN)
	* unitscale / frame->ps->xscale;
    text_margin_y =
      xGetResDouble(Glob->rdisplay, Glob->prog_name,
		    "ps_text_margin", PS_TEXT_MARGIN)
	* unitscale / frame->ps->yscale;
    
    break;

  }

  approx_nticks = xGetResLong(Glob->rdisplay, Glob->prog_name,
			      "approx_nticks", APPROX_NTICKS);

  /*
   * set plot and image limits
   */

  min_x = 0.0;
  max_x = vsection->length;
  min_y = vsection->min_z;
  max_y = vsection->max_z;

  delta_l = vsection->length / (double) (reply->nx - 1);
  image_min_x = min_x - delta_l / 2.0;
  image_max_x = max_x + delta_l / 2.0;
  image_min_y = min_y;
  image_max_y = max_y;

  /*
   * compute linear tick parameters
   */

  GLinearTicks(min_x, max_x, approx_nticks,
	       &nticksx, &tickmin_x, &delta_tickx);

  GLinearTicks(min_y, max_y, approx_nticks,
	       &nticksy, &tickmin_y, &delta_ticky);

  /*
   * interpolate the vertical section
   */

  image_data = interpolate_vsection(vert_data, &image_nl, &image_nz,
				    grid_info, reply, dz_constant);

  /*
   * set the clipping rectangle
   */

  clip_rectangle->x = min_x;
  clip_rectangle->y = min_y;
  clip_rectangle->width = (max_x - min_x);
  clip_rectangle->height = (max_y - min_y);

  if (dev == PSDEV)
    PsGsave(frame->psgc->file);

  GSetClipRectangles(dev, frame, Glob->ring_gc, frame->psgc,
		     clip_rectangle, (int) 1);

  /*
   * plot image
   */

  if (Glob->plot_image && image_data != NULL) {

    if (dz_constant) {

      GDrawImage(dev, frame, colors,
		 image_min_x, image_min_y, 
		 image_max_x - image_min_x,
		 image_max_y - image_min_y,
		 image_nl, image_nz,
		 IMAGE_START_BOTLEFT,
		 image_data,
		 reply->bad_data_val);

    } else {

      draw_varying_dz_section(dev, frame, colors,
			      image_nl, image_nz,
			      delta_l,
			      plane_heights,
			      image_data,
			      reply->bad_data_val);
      
    }

  }

  /*
   * plot in contours if desired
   */

  if (Glob->plot_vsection_contours && image_data != NULL &&
      dz_constant) {

    GDrawContours(dev, frame,
		  Glob->pos_contour_gc,
		  Glob->zero_contour_gc,
		  Glob->neg_contour_gc,
		  &Glob->pos_contour_psgc,
		  &Glob->zero_contour_psgc,
		  &Glob->neg_contour_psgc,
		  image_min_x, image_min_y, 
		  image_max_x - image_min_x,
		  image_max_y - image_min_y,
		  image_nl, image_nz,
		  IMAGE_START_BOTLEFT,
		  image_data,
		  reply->scale, reply->bias,
		  &Glob->fcontrol[Glob->field].contours,
		  reply->bad_data_val);

  }

  /*
   * if X, clear the rectangles for the margins
   */

  if (topmargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   min_x,
		   max_y,
		   frame->w_xmax - frame->w_xmin,
		   frame->w_ymax - max_y);

  if (xaxismargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   frame->w_xmin,
		   frame->w_ymin,
		   frame->w_xmax - frame->w_xmin,
		   min_y - frame->w_ymin);

  if (yaxismargin != 0)
    GFillRectangle(dev, frame,
		   Glob->pixmap_gc,
		   frame->psgc,
		   max_x,
		   frame->w_ymin,
		   frame->w_xmax - max_x,
		   frame->w_ymax - frame->w_ymin);

  /*
   * restore graphics state from clipping operation
   */

    if (dev == PSDEV)
      PsGrestore(frame->psgc->file);

  /*
   * draw in line between grid and data
   */


  if (yaxismargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      max_x, min_y, max_x, max_y);

  if (xaxismargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      min_x, min_y, max_x, min_y);

  if (topmargin != 0)
    GDrawLine(dev, frame, frame->x->gc, frame->psgc,
	      min_x, max_y, max_x, max_y);

  /*
   * if printer, set tick line width and ticklabel fontsize
   */

  if (dev == PSDEV) {
    PsSetLineWidth(frame->psgc->file,
		   xGetResDouble(Glob->rdisplay, Glob->prog_name,
				 "ps_tick_width", PS_TICK_WIDTH));
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_ticklabel_fontsize);
  }

  /*
   * grid and scale marks
   */

  texty = (frame->w_ymin + xticklength + xtickmargin);
  gridx = tickmin_x;

  for (i = 0; i < nticksx; i++) {

    if (gridx > min_x + xaxisendmargin && gridx < max_x - xaxisendmargin) {

      ilabel = (si32) floor((double)(gridx + 0.5));

      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		gridx, frame->w_ymin,
		gridx, frame->w_ymin + xticklength);
      sprintf(label, "%d", ilabel);
      GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font,
		  frame->psgc,
		  XJ_CENTER, YJ_ABOVE, gridx, texty, label);
    }

    gridx += delta_tickx;

  }

  textx = frame->w_xmax - yticklength - ytickmargin;
  gridy = tickmin_y;

  for (i = 0; i < nticksy; i++) {

    if (gridy > min_y + yaxisendmargin && gridy < max_y - yaxisendmargin) {

      ilabel = (si32) floor((double)(gridy + 0.5));

      GDrawLine(dev, frame, Glob->tick_gc, frame->psgc, 
		frame->w_xmax, gridy, 
		frame->w_xmax - yticklength, gridy);
      sprintf(label, "%d", ilabel);
      GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font, frame->psgc,
		  XJ_RIGHT, YJ_CENTER, textx, gridy, label);

    }
    
    gridy += delta_ticky;

  }

  /*
   * write in header
   */

  if (dev == PSDEV) {
    
    if (!strcmp(grid_info->units_label_x, grid_info->units_label_y)) {
      sprintf(hstring,
	      "%.4d/%.2d/%.2d   %.2d:%.2d:%.2d   (%g,%g) to (%g,%g) (%s)",
	      vsection_time->year, vsection_time->month,
	      vsection_time->day, vsection_time->hour,
	      vsection_time->min, vsection_time->sec,
	      vsection->start_x, vsection->start_y,
	      vsection->end_x, vsection->end_y,
	      grid_info->units_label_x);
    } else {
      sprintf(hstring,
	      "%.4d/%.2d/%.2d   %.2d:%.2d:%.2d   (%g,%g) to (%g,%g) (%s,%s)",
	      vsection_time->year, vsection_time->month,
	      vsection_time->day, vsection_time->hour,
	      vsection_time->min, vsection_time->sec,
	      vsection->start_x, vsection->start_y,
	      vsection->end_x, vsection->end_y,
	      grid_info->units_label_x, grid_info->units_label_y);
    }
    
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_text_fontsize);
    
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_BELOW,
		frame->w_xmin + text_margin_x,
		frame->w_ymax - text_margin_y,
		hstring);
    
  }
  
  if (!strcmp(grid_info->units_label_x, grid_info->units_label_y))
    strcpy(hstring, grid_info->units_label_x);
  else
    sprintf(hstring, "%s/%s",
	    grid_info->units_label_x, grid_info->units_label_y);

  GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
	      frame->psgc, XJ_LEFT, YJ_ABOVE,
	      frame->w_xmin + text_margin_x,
	      frame->w_ymin + text_margin_y,
	      hstring);

  GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
	      frame->psgc, XJ_RIGHT, YJ_BELOW,
	      frame->w_xmax - text_margin_x,
	      frame->w_ymax - text_margin_y,
	      grid_info->units_label_z);


  /*
   * if image not available, write message
   */

  if (image_data == NULL) {

    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_RIGHT, YJ_BELOW,
		max_x - text_margin_x,
		max_y - text_margin_y,
		"Image not available");
    
  }

  /*
   * plot in cappi ht as a tick mark if X
   */

  if (dev == XDEV)
    GDrawLine(dev, frame, Glob->tick_gc, frame->psgc,
	      max_x,
	      Glob->z_requested,
	      max_x + yticklength,
	      Glob->z_requested);

  /*
   * set the plotframe drawable back to the window
   */

  if (dev ==  XDEV){

    frame->x->drawable = window;

  }

}

/**********************************************************************
 * draw_varying_dz_section
 *
 * draw section with dz's which vary
 */

static void draw_varying_dz_section(int dev,
				    gframe_t *frame,
				    g_color_scale_t *colors,
				    si32 image_nl,
				    si32 image_nz,
				    double delta_l,
				    double *plane_heights,
				    ui08 *image_data,
				    si32 bad_data_val)

{

  si32 iz, il;
  si32 image_val;

  double base, top, height;
  double x;

  if (Glob->debug) {
    fprintf(stderr, "** draw_varying_dz_section **\n");
  }

  for (iz = 0; iz < image_nz; iz++) {

    base = plane_heights[iz * N_PLANE_HEIGHT_VALUES + PLANE_BASE_INDEX];
    top = plane_heights[iz * N_PLANE_HEIGHT_VALUES + PLANE_TOP_INDEX];
    height = top - base;

    x = -delta_l / 2.0;

    for (il = 0; il < image_nl; il++) {

      image_val = *image_data;

      if (image_val != bad_data_val) {
	
	/*
	 * draw the rectangle
	 */

	switch (dev) {
	
	case XDEV:

	  if (colors->gc[image_val] != NULL)
	    GFillRectangle(dev, frame,
			   colors->gc[image_val],
			   frame->psgc,
			   x, base, delta_l, height);
	  break;
	  
	case PSDEV:
	  if (colors->psgc[image_val] != NULL)
	    GFillRectangle(dev, frame,
			   frame->x->gc,
			   colors->psgc[image_val],
			   x, base, delta_l, height);
	  break;
	  
	}
      
      } /* if (image_val != bad_data_val) */

      image_data++;
      x += delta_l;

    } /* il */

  } /* iz */

}


/************************************************************************
 * interpolate_vsection()
 *
 * Interpolates the vertical section image if required.
 *
 * If interpolation not required, copies the vert array as is.
 *
 * The memory management is done in this routine. The returned array
 * should not be freed by the calling routine. 
 */

static ui08 *interpolate_vsection(ui08 *vert_data,
				  si32 *image_nl,
				  si32 *image_nz,
				  cd_grid_info_t *grid_info,
				  cd_reply_t *reply,
				  int dz_constant)

{
  
  static int array_allocated = FALSE;
  static si32 n_points_allocated;
  static ui08 *interp_data;
  
  int data_found, start_iz_found;
  
  si32 il, nl, iz, jz, ires;
  si32 nz, nres;
  si32 n_points;
  si32 start_iz = 0, end_iz;
  si32 start_run = 0, end_run;
  si32 start_val = 0, end_val;
  si32 missing_val;
  si32 data_val, prev_val;
  
  double incr, delta_val;
  
  /*
   * if no image data, return NULL
   */
  
  if (vert_data == NULL)
    return ((ui08 *) NULL);
  
  if (!Glob->vsection_interp || !dz_constant) {
    
    /*
     * if no interp required, or dz varies,
     * copy the vert data to the interp array unchanged
     */
  
    if (!array_allocated) {
      
      interp_data = (ui08 *) umalloc
	((ui32) (reply->n_points * sizeof(ui08)));
      
      n_points_allocated = reply->n_points;
      
      array_allocated = TRUE;
      
    } else if (reply->n_points > n_points_allocated) {
      
      interp_data = (ui08 *) urealloc
	((char *) interp_data,
	 (ui32) (reply->n_points * sizeof(ui08)));
      
      n_points_allocated = reply->n_points;
      
    }
    
    memcpy ((void *) interp_data,
            (void *)  vert_data,
            (size_t) (reply->n_points * sizeof(ui08)));
    
    *image_nl = reply->nx;
    *image_nz = reply->ny;
    
    return (interp_data);
    
  } else {
    
    /*
     * interpolation required
     */
    
    /*
     * set missing data value
     */
    
    missing_val = reply->bad_data_val;
    
    /*
     * compute the resolution factor
     */
    
    nres = (si32) (grid_info->dz / Glob->vsection_interp_res + 1.0);
    nl = reply->nx;
    nz = reply->ny * nres;
    n_points = nl * nz;
    
    /*
     * allocate space for the interp data, set image pointer
     */
    
    if (!array_allocated) {
      
      interp_data = (ui08 *) umalloc
	((ui32) (n_points * sizeof(ui08)));
      
      n_points_allocated = n_points;
      
      array_allocated = TRUE;
      
    } else if (n_points > n_points_allocated) {
      
      interp_data = (ui08 *) urealloc
	((char *) interp_data,
	 (ui32) (n_points * sizeof(ui08)));
      
      n_points_allocated = n_points;
      
    }
    
    /*
     * copy image data to interp array, duplicating the data in the
     * vertical by nres times
     */
    
    for (il = 0; il < nl; il++) {
      
      for (iz = 0; iz < reply->ny; iz++) {
	
	data_val = *(vert_data + (iz * nl + il));
	
	for (ires = 0; ires < nres; ires++) {
	  
	  interp_data[((iz * nres) + ires) * nl + il] = data_val;
	  
	} /* ires */
	
      } /* iz */
      
    } /* il */
    
    /*
     * perform a linear interpolation on the data. The interpolation
     * takes place between the mid_points of runs of data os the
     * same value. This is almost the same as interpolation the
     * data between original radar points
     */
    
    for (il = 0; il < nl; il++) {
      
      /*
       * initialize flags
       */
      
      data_found = FALSE;
      start_iz_found = FALSE;
      prev_val = missing_val;
      
      for (iz = 0; iz < nz; iz ++) {
	
	data_val = interp_data[iz * nl + il];
	
	if (data_found != FALSE || data_val != missing_val) {
	  
	  if (data_val != prev_val || iz == nz - 1) {
	    
	    if (prev_val != missing_val) {
	      
	      end_run = iz - 1;
	      
	      if (start_iz_found) {
		
		end_iz = (start_run + end_run) / 2;
		end_val = prev_val;

		if (end_iz - start_iz > 1) {
  
		  incr = ((double) (end_val - start_val) /
			  (double) (end_iz - start_iz));
  
		  delta_val = 0;
		  
		  for (jz = start_iz + 1; jz < end_iz; jz++) {
    
		    delta_val += incr;
    
		    interp_data[jz * nl + il] = 
		      ((ui08) start_val +
		       (ui08) (floor (delta_val + 0.5)));
    
		  } /* jz */

		} /* if (end_iz - start_iz > 1) */

		start_iz = end_iz + 1;
		
	      } else {
		
		start_iz = (start_run + end_run) / 2;
		start_iz_found = TRUE;
		
	      } /* if (start_iz_found) */
	      
	      start_val = prev_val;
	      
	    } /* if (prev_val != missing_val) */
	    
	    start_run = iz;
	    
	  } /* if (data_val != prev_val || iz == nz - 1) */
	  
	  if (data_val != missing_val) {
	    
	    data_found = TRUE;
	    
	  } else {
	    
	    data_found = FALSE;
	    start_iz_found = FALSE;
	    
	  } /* if (data_val != missing_val) */
	  
	} /* if (data_found != FALSE || data_val != missing_val) */
	
	prev_val = data_val;
	
      } /* iz */
      
    } /* il */
    
  } /* if (!Glob->vsection_interp) */
  
  /*
   * set array size and return
   */
  
  *image_nl = nl;
  *image_nz = nz;

  return (interp_data);
  
}
/************************************************************************
 * expose_vsection_pixmap()
 */

void expose_vsection_pixmap(void)

{

  gframe_t *frame;

  if (Glob->debug) {
    fprintf(stderr, "** expose_vsection_pixmap **\n");
  }

  frame = Glob->vsection_plot_frame;

  /*
   * copy the pixmap to the window
   */

  XCopyArea(frame->x->display, Glob->vsection.pixmap,
	    frame->x->drawable,
	    Glob->copyarea_gc, 0, 0,
	    frame->x->width, frame->x->height, 0, 0);

  safe_XFlush(Glob->rdisplay);

}

/*********************************************************************
 * read_vsection_plane()
 *
 * Reads a vsection plane from the dobson_server.
 *
 * Returns a pointer to the data plane, NULL if failure.
 *
 */

static ui08 *read_vsection_plane(date_time_t **time_ptr,
				 cd_grid_info_t **info_ptr,
				 cd_reply_t **reply_ptr,
				 int *dz_constant_ptr,
				 double **plane_heights_ptr)

{
  
  static int dz_constant = FALSE;
  static date_time_t vsection_time;
  static cd_reply_t reply;
  static cd_grid_info_t grid_info;

  field_control_t *fcontrol;
  vsection_t *vsection;
  cd_command_t *command;
  cdata_index_t *cdata_index;

  if (Glob->debug) {
    fprintf(stderr, "** read_vsection_plane **\n");
  }

  /*
   * return NULL if nfields is not positive
   */

  if (Glob->nfields < 1) {
    return ((ui08 *) NULL);
  }

  /*
   * initialize
   */

  MEM_zero(grid_info);
  MEM_zero(reply);
  MEM_zero(vsection_time);
  
  *info_ptr = &grid_info;
  *reply_ptr = &reply;
  *time_ptr = &vsection_time;
  *dz_constant_ptr = FALSE;
  *plane_heights_ptr = NULL;

  /*
   * set local pointers
   */

  fcontrol = Glob->fcontrol + Glob->field;
  cdata_index = &fcontrol->cdata_index;
  vsection = &Glob->vsection;
  command = &cdata_index->command;

  Glob->vsection_info = &grid_info;

  /*
   * load up the command struct
   */

  memset (command, 0, sizeof(cd_command_t));

  if (Glob->mode == ARCHIVE) {
    
    command->primary_com = GET_INFO | GET_DATA | GET_PLANE_HEIGHTS;
    command->time_min = Glob->time - fcontrol->time_window;
    command->time_max = Glob->time + fcontrol->time_window;
    command->time_cent = Glob->time;
    cdata_index->want_realtime = FALSE;
    
  } else {
    
    command->primary_com =
      GET_MOST_RECENT | GET_INFO | GET_DATA | GET_PLANE_HEIGHTS;
    cdata_index->want_realtime = TRUE;
    
  }
  
  command->second_com = GET_V_PLANE;
  
  command->lat_origin = Glob->grid.proj_origin_lat;
  command->lon_origin = Glob->grid.proj_origin_lon;
  command->ht_origin = 0;
  
  command->min_x = vsection->req_start_x;
  command->min_y = vsection->req_start_y;
  command->max_x = vsection->req_end_x;
  command->max_y = vsection->req_end_y;
  command->min_z = -10.0;
  command->max_z = 100.0;
  command->data_field = fcontrol->field;
  
  /*
   * get the data
   */
  
  if (cdata_read(cdata_index)) {
    return (NULL);
  }

  /*
   * load up return values
   */

  grid_info = cdata_index->grid_info;
  reply = cdata_index->reply;
  vsection_time.unix_time = reply.time_cent;
  uconvert_from_utime(&vsection_time);

  if (grid_info.dz == 0.0) {
    dz_constant = FALSE;
    grid_info.dz = 1.0;
  } else {
    dz_constant = TRUE;
  }

  *dz_constant_ptr = dz_constant;
  *plane_heights_ptr = cdata_index->plane_heights;

  return (cdata_index->data);
  
}
