// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
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

#include "Rview.hh"
#include <toolsa/servmap.h>
#include <toolsa/pjg.h>
#include <toolsa/str.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
using namespace std;

static string VsectFieldName;
static date_time_t VsectTime;
static double VsectDxKm;
static vector<Mdvx::vsect_waypt_t> VsectWayPts;
static vector<Mdvx::vsect_samplept_t> VsectSamplePts;

static double Start_x, Start_y;
static double End_x, End_y;
static double Min_z, Max_z;
static double Length;

static int read_vsection(MdvxField &floatField,
			 MdvxField &byteField);

static void draw_vsection_variable(int dev,
				   gframe_t *frame,
				   g_color_scale_t *colors,
				   const Mdvx::field_header_t &fhdr,
				   const Mdvx::vlevel_header_t &vhdr,
				   ui08 *image_data);

static void vsection_calc(int dev,
			  gframe_t *frame,
			  const Mdvx::field_header_t &fhdr,
			  const Mdvx::vlevel_header_t &vhdr);

/////////////////////////////////
// get reference to vsect objects

const vector<Mdvx::vsect_waypt_t> getVsectWayPts()
{
  return VsectWayPts;
}
const vector<Mdvx::vsect_samplept_t> getVsectSamplePts()
{
  return VsectSamplePts;
}

/************************************************************************
 * draw_vsection_title()
 */

void draw_vsection_title(int dev)

{

  char tstring[BUFSIZ];
  gframe_t *frame;

  if (Glob->verbose) {
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
    XClearWindow(Glob->rdisplay, frame->x->drawable);
  }

  if (VsectFieldName.size() > 0) {
    if (Glob->vsection_interp) {
      sprintf(tstring,"VERT SECTION - %s - interpolated",
              VsectFieldName.c_str());
    } else {
      sprintf(tstring,"VERT SECTION - %s", VsectFieldName.c_str());
    }
  } else {
    sprintf(tstring,"No vert section data available");
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

  if (Glob->verbose) {
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
  XClearWindow(Glob->rdisplay, frame->x->drawable);

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

static const int HDR_MAX = 2048;

void draw_vsection_plot(int dev,
			g_color_scale_t *colors)
  
{

  char label[128];
  char hstring[HDR_MAX];

  si32 i;
  si32 image_nl, image_nz;
  si32 ilabel;
  si32 xaxismargin = 0, yaxismargin = 0, topmargin = 0;
  long approx_nticks, nticksx, nticksy;

  double tickmin_x, tickmin_y;
  double delta_tickx, delta_ticky;
  double min_x, max_x, min_y, max_y;
  double image_min_x, image_min_y, image_max_x, image_max_y;
  double gridx, gridy, textx, texty;
  double x_ticklength, ps_ticklength;
  double xticklength = 0, yticklength = 0, xtickmargin = 0, ytickmargin = 0;
  double xaxisendmargin = 0, yaxisendmargin = 0;
  double tick_clearance;
  double text_margin_x = 0, text_margin_y = 0;
  double unitscale;

  gframe_t *frame;
  vsection_t *vsection;

  Drawable window = 0;
  GRectangle clip_rectangle[1];

  if (Glob->verbose) {
    fprintf(stderr, "** draw_vsection_plot **\n");
  }

  draw_vsection_title(dev);

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
   * get the vertical section data
   */

  MdvxField byteField;
  MdvxField floatField;

  if (read_vsection(floatField, byteField)) {

    // data access failed
    // set the plotframe drawable back to the window
    
    if (dev ==  XDEV) {
      frame->x->drawable = window;
    }
    
    return;
    
  }

  const Mdvx::field_header_t &fhdr = byteField.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = byteField.getVlevelHeader();
  ui08 *byte_vsection = (ui08*) byteField.getVol();

  /*
   * calculate the vsection geometry
   */

  vsection_calc(dev, frame, fhdr, vhdr);

  /*
   * adjust the color scale to suit the scale and bias of the data
   */

  if (dev == XDEV)
    GAdjustGCScale(colors, fhdr.scale, fhdr.bias);
  else
    GAdjustPsgcScale(colors, fhdr.scale, fhdr.bias);

  /*
   * get resources
   */

  switch (dev) {

  case XDEV:

    xaxismargin = uGetParamLong(Glob->prog_name,
			      "x_xaxismargin", X_XAXISMARGIN);

    yaxismargin = uGetParamLong(Glob->prog_name,
			      "x_yaxismargin", X_YAXISMARGIN);

    topmargin = uGetParamLong(Glob->prog_name,
			    "x_topmargin", X_TOPMARGIN);
 
   xaxisendmargin = uGetParamDouble(Glob->prog_name,
				  "x_xaxisendmargin",
				  X_XAXISENDMARGIN) / frame->x->xscale;
 
   yaxisendmargin = uGetParamDouble(Glob->prog_name,
				  "x_yaxisendmargin",
				  X_YAXISENDMARGIN) / frame->x->yscale;

 
   tick_clearance = uGetParamDouble(Glob->prog_name,
				  "x_tick_clearance",
				  X_TICK_CLEARANCE);

    x_ticklength = uGetParamDouble(Glob->prog_name,
				"x_ticklength", X_TICKLENGTH);
    xticklength = x_ticklength / frame->x->yscale;
    yticklength = x_ticklength / frame->x->xscale;
    xtickmargin = tick_clearance / frame->x->yscale;
    ytickmargin = tick_clearance / frame->x->xscale;

    text_margin_x =
      uGetParamDouble(Glob->prog_name, "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->xscale;
    text_margin_y =
      uGetParamDouble(Glob->prog_name, "x_text_margin",
		    X_TEXT_MARGIN) / frame->x->yscale;
    
    break;

  case PSDEV:

    unitscale = uGetParamDouble(Glob->prog_name,
			     "ps_unitscale", PS_UNITSCALE);

    xaxismargin =
      (si32) (uGetParamDouble(Glob->prog_name,
			   "ps_xaxismargin", PS_XAXISMARGIN) * unitscale);

    yaxismargin =
      (si32) (uGetParamDouble(Glob->prog_name,
			   "ps_yaxismargin", PS_YAXISMARGIN) * unitscale);

    topmargin =
      (si32) (uGetParamDouble(Glob->prog_name,
			   "ps_topmargin", PS_TOPMARGIN) * unitscale);

    xaxisendmargin =
      uGetParamDouble(Glob->prog_name, "ps_xaxisendmargin",
		   PS_XAXISENDMARGIN) * unitscale / frame->ps->xscale;
    yaxisendmargin =
      uGetParamDouble(Glob->prog_name, "ps_yaxisendmargin",
		   PS_YAXISENDMARGIN) * unitscale / frame->ps->yscale;

    tick_clearance =
      uGetParamDouble(Glob->prog_name, "ps_tick_clearance",
		   PS_TICK_CLEARANCE) * unitscale;

    ps_ticklength = uGetParamDouble(Glob->prog_name,
				 "ps_ticklength", PS_TICKLENGTH) * unitscale;

    xticklength = ps_ticklength / frame->ps->yscale;
    yticklength = ps_ticklength / frame->ps->xscale;
    xtickmargin = tick_clearance / frame->ps->yscale;
    ytickmargin = tick_clearance / frame->ps->xscale;

    text_margin_x =
      uGetParamDouble(Glob->prog_name,
		    "ps_text_margin", PS_TEXT_MARGIN)
	* unitscale / frame->ps->xscale;
    text_margin_y =
      uGetParamDouble(Glob->prog_name,
		    "ps_text_margin", PS_TEXT_MARGIN)
	* unitscale / frame->ps->yscale;
    
    break;

  }

  approx_nticks = uGetParamLong(Glob->prog_name,
			      "approx_nticks", APPROX_NTICKS);

  /*
   * set plot and image limits
   */

  min_x = 0.0;
  max_x = Length;
  min_y = Min_z;
  max_y = Max_z;
  
  image_min_x = min_x;
  image_max_x = max_x;
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

  // interpolate as required

  image_nl = fhdr.nx;
  image_nz = fhdr.nz;
  ui08 *image_data = byte_vsection;
    
  /*
   * plot image
   */
  
  if (fhdr.nz > 1) {
    
    if (Glob->plot_vsection_image == IMAGE_ON) {
      
      if (fhdr.dz_constant) {
	
	GDrawImage(dev, frame, colors,
		   image_min_x, image_min_y, 
		   image_max_x - image_min_x,
		   image_max_y - image_min_y,
		   image_nl, image_nz,
		   IMAGE_START_BOTLEFT,
		   image_data,
		   (ui08) fhdr.missing_data_value);
	
      } else {
	
	draw_vsection_variable(dev, frame, colors,
			       fhdr, vhdr, image_data);
      }

    } else if (Glob->plot_vsection_image == IMAGE_ON_CONTOURED) {

      MdvxContour cont;
      for (int ilevel = 0; ilevel < colors->nlevels; ilevel++) {
	cont.addVal(colors->level[ilevel].start);
      }
      cont.addVal(colors->level[colors->nlevels - 1].end);
      
      if (cont.computeTriangles(floatField) == 0) {
	draw_contour_triangles(dev, frame, colors, cont);
      }

    }
    
    if (Glob->plot_vsection_contours) {
      
      MdvxContour cont;

      if (Glob->fcontrol[Glob->field].contour_int > 0) {
	cont.setVals(Glob->fcontrol[Glob->field].contour_min,
		     Glob->fcontrol[Glob->field].contour_max,
		     Glob->fcontrol[Glob->field].contour_int);
      } else {
	for (int ilevel = 0; ilevel < colors->nlevels; ilevel++) {
	  cont.addVal(colors->level[ilevel].start);
	}
	cont.addVal(colors->level[colors->nlevels - 1].end);
      }
      
      int iret;
      if (Glob->plot_vsection_contours == CONTOURS_ON_WITH_LABELS) {
	iret = cont.computeLines(floatField);
      } else {
	iret = cont.computeSegments(floatField);
      }
      
      if (iret == 0) {
	draw_contours(dev, frame,
		      Glob->pos_contour_gc,
		      Glob->zero_contour_gc,
		      Glob->neg_contour_gc,
		      &Glob->pos_contour_psgc,
		      &Glob->zero_contour_psgc,
		      &Glob->neg_contour_psgc,
		      cont);
      }
	
    } // if (Glob->plot_vsection_contours) 
      
  } // if (Glob->plot_vsection_image) 
      
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
		   uGetParamDouble(Glob->prog_name,
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
      snprintf(label, 128, "%d", ilabel);
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
      snprintf(label, 128, "%d", ilabel);
      GDrawString(dev, frame, Glob->ticklabel_gc, frame->x->font, frame->psgc,
		  XJ_RIGHT, YJ_CENTER, textx, gridy, label);

    }
    
    gridy += delta_ticky;

  }

  /*
   * write in header
   */

  const char *units_x = Mdvx::projType2XUnits(fhdr.proj_type);
  const char *units_y = Mdvx::projType2YUnits(fhdr.proj_type);

  if (dev == PSDEV) {
    
    if (!strcmp(units_x, units_y)) {
      snprintf(hstring, HDR_MAX,
               "%.4d/%.2d/%.2d   %.2d:%.2d:%.2d   (%g,%g) to (%g,%g) (%s)",
               VsectTime.year, VsectTime.month,
               VsectTime.day, VsectTime.hour,
               VsectTime.min, VsectTime.sec,
               Start_x, Start_y,
               End_x, End_y,
               units_x);
    } else {
      snprintf(hstring, HDR_MAX,
               "%.4d/%.2d/%.2d   %.2d:%.2d:%.2d   (%g,%g) to (%g,%g) (%s,%s)",
               VsectTime.year, VsectTime.month,
               VsectTime.day, VsectTime.hour,
               VsectTime.min, VsectTime.sec,
               Start_x, Start_y,
               End_x, End_y,
               units_x, units_y);
    }
    
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      Glob->ps_text_fontsize);
    
    GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
		frame->psgc, XJ_LEFT, YJ_BELOW,
		frame->w_xmin + text_margin_x,
		frame->w_ymax - text_margin_y,
		hstring);
    
  }
  
  if (!strcmp(units_x, units_y))
    STRncopy(hstring, units_x, HDR_MAX);
  else
    snprintf(hstring, HDR_MAX, "%s/%s",
             units_x, units_y);
  
  GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
	      frame->psgc, XJ_LEFT, YJ_ABOVE,
	      frame->w_xmin + text_margin_x,
	      frame->w_ymin + text_margin_y,
	      hstring);

  GDrawString(dev, frame, Glob->text_gc, Glob->x_text_font,
	      frame->psgc, XJ_RIGHT, YJ_BELOW,
	      frame->w_xmax - text_margin_x,
	      frame->w_ymax - text_margin_y,
	      Mdvx::vertTypeZUnits(fhdr.vlevel_type));


  /*
   * if image not available, write message
   */

  if (byte_vsection == NULL) {
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
 * draw_vsection_variable
 *
 * draw section with variable vertical spacing
 */

static void draw_vsection_variable(int dev,
				   gframe_t *frame,
				   g_color_scale_t *colors,
				   const Mdvx::field_header_t &fhdr,
				   const Mdvx::vlevel_header_t &vhdr,
				   ui08 *image_data)
  
{

  if (Glob->verbose) {
    fprintf(stderr, "** draw_vsection_variable **\n");
  }

  ui08 *idata = image_data;
  ui08 missing_val = (ui08) fhdr.missing_data_value;
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    
    double base, top;
    
    if (iz == 0) {
      base = Min_z;
    } else {
      base = (vhdr.level[iz - 1] + vhdr.level[iz]) / 2.0;
    }

    if (iz == fhdr.nz - 1) {
      top = Max_z;
    } else {
      top = (vhdr.level[iz] + vhdr.level[iz+1]) / 2.0;
    }

    double height = top - base;
    double dxkm = VsectDxKm;
    double xx = 0.0;

    for (int ix = 0; ix < fhdr.nx; ix++, idata++, xx += dxkm) {

      ui08 image_val = *idata;

      if (image_val != missing_val) {
	
	switch (dev) {
	
	case XDEV:
	  
	  if (colors->gc[image_val] != NULL)
	    GFillRectangle(dev, frame,
			   colors->gc[image_val],
			   frame->psgc,
			   xx, base, dxkm, height);

	  break;
	  
	case PSDEV:
	  if (colors->psgc[image_val] != NULL)
	    GFillRectangle(dev, frame,
			   frame->x->gc,
			   colors->psgc[image_val],
			   xx, base, dxkm, height);
	  break;
	  
	}
      
      } /* if (image_val != missing_val) */

    } /* ix */

  } /* iz */

}


/************************************************************************
 * expose_vsection_pixmap()
 */

void expose_vsection_pixmap()

{

  gframe_t *frame;

  if (Glob->verbose) {
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

  XFlush(Glob->rdisplay);

}

/*********************************************************************
 * read_vsection()
 *
 * Reads a vsection plane using an Mdvx object.
 *
 * Side effect - fills VsectMdvx object
 *
 * Returns 0 on success, -1 on failure.
 */

static int read_vsection(MdvxField &floatField,
			 MdvxField &byteField)

{

  // initialize

  VsectFieldName = "";

  if (Glob->verbose) {
    fprintf(stderr, "** read_vsection **\n");
  }

  // return error if nfields is not positive

  if (Glob->nfields < 1) {
    return (-1);
  }

  // set field control pointer
  
  field_control_t &fcontrol = Glob->fcontrol[Glob->field];

  // set the mdvx object for the read

  DsMdvx mdvx;

  mdvx.clearRead();

  if (Glob->mode == ARCHIVE) {
    mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
                     fcontrol.url,
                     fcontrol.time_window,
                     Glob->time + Glob->field_data_time_offset_secs);
  } else {
    
    mdvx.setReadTime(Mdvx::READ_LAST,
                     fcontrol.url,
                     fcontrol.time_window);

  }
    
  if (fcontrol.field_num < 0) {
    mdvx.addReadField(fcontrol.field_name);
  } else {
    mdvx.addReadField(fcontrol.field_num);
  }
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_ASIS);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_ASIS);
  
  double start_lat, start_lon, end_lat, end_lon;
  vsection_t *vsection = &Glob->vsection;
  Glob->proj.xy2latlon(vsection->req_start_x, vsection->req_start_y,
		       start_lat, start_lon);
  Glob->proj.xy2latlon(vsection->req_end_x, vsection->req_end_y,
		       end_lat, end_lon);

  double r, theta;
  PJGLatLon2RTheta(start_lat, start_lon, end_lat, end_lon,
		   &r, &theta);
  mdvx.addReadWayPt(start_lat, start_lon);
  for (int i = 1; i < 5; i++) {
    double lat, lon;
    double len = r * i / 5.0; 
    PJGLatLonPlusRTheta(start_lat, start_lon,
			len, theta,
			&lat, &lon);
    mdvx.addReadWayPt(lat, lon);
  }
  mdvx.addReadWayPt(end_lat, end_lon);
  // mdvx.setReadVsectDisableInterp();

  if (!Glob->vsection_interp) {
    mdvx.setReadVsectDisableInterp();
  }

  if (Glob->debug) {
    mdvx.printReadRequest(cerr);
  }
  
  // do the read
  
  if (mdvx.readVsection()) {
    cerr << mdvx.getErrStr() << endl;
    return (-1);
  }

  // set args and statics

  floatField = *mdvx.getFieldByNum(0);
  floatField.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);

  byteField = *mdvx.getFieldByNum(0);
  byteField.convertType(Mdvx::ENCODING_INT8, Mdvx::COMPRESSION_NONE);

  if (Glob->field_name_from_params) {
    VsectFieldName = fcontrol.description;
  } else {
    VsectFieldName = floatField.getFieldName();
  }
  
  VsectTime.unix_time = mdvx.getMasterHeader().time_centroid;
  uconvert_from_utime(&VsectTime);
  
  VsectWayPts = mdvx.getVsectWayPts();
  VsectSamplePts = mdvx.getVsectSamplePts();
  VsectDxKm = mdvx.getVsectDxKm();

  return (0);
  
}


/*********************************************************************
 * vsection_calc.c
 *
 * calcluates the vsection frame geometry
 *
 *********************************************************************/

static void vsection_calc(int dev,
			  gframe_t *frame,
			  const Mdvx::field_header_t &fhdr,
			  const Mdvx::vlevel_header_t &vhdr)

{

  double image_width, image_height;
  double range_x, range_y;
  double plot_min_x, plot_max_x;
  double plot_min_y, plot_max_y;
  double xaxismargin, yaxismargin, topmargin;

  if (Glob->verbose) {
    fprintf(stderr, "** vsection_calc **\n");
  }

  /*
   * set local pointers
   */

  // vsection_t *vsection = NULL;
  // vsection = &Glob->vsection;

  /*
   * get margins from the parameters resources
   */

  if (dev == XDEV) {

    xaxismargin = (double) uGetParamLong(Glob->prog_name,
				       "x_xaxismargin", X_XAXISMARGIN);
    yaxismargin = (double) uGetParamLong(Glob->prog_name,
				       "x_yaxismargin", X_YAXISMARGIN);

    topmargin = 0.0;

    image_width =
      (double) frame->x->width - yaxismargin;

    image_height =
      (double) frame->x->height - xaxismargin - topmargin;

  } else {

    xaxismargin = uGetParamDouble(Glob->prog_name,
				"ps_xaxismargin", PS_XAXISMARGIN);
    yaxismargin = uGetParamDouble(Glob->prog_name,
				"ps_yaxismargin", PS_YAXISMARGIN);

    topmargin = 0.0;

    image_width =
      (double) frame->ps->width - yaxismargin;

    image_height =
      (double) frame->ps->height - xaxismargin - topmargin;

  }

  int nWayPts = VsectWayPts.size();
  const Mdvx::vsect_waypt_t &startPt = VsectWayPts[0];
  const Mdvx::vsect_waypt_t &endPt = VsectWayPts[nWayPts-1];

  Glob->proj.latlon2xy(startPt.lat, startPt.lon,
		       Start_x, Start_y);
  Glob->proj.latlon2xy(endPt.lat, endPt.lon,
		       End_x, End_y);

  Length = fhdr.nx  * fhdr.grid_dx;

  if (fhdr.nz == 1) {
    Min_z = vhdr.level[0] - fhdr.grid_dz * 0.5;
    Max_z = vhdr.level[0] + fhdr.grid_dz * 0.5;
  } else {
    Min_z = vhdr.level[0] -
      (vhdr.level[1] - vhdr.level[0]) / 2.0;
    Max_z = vhdr.level[fhdr.nz - 1] +
      (vhdr.level[fhdr.nz - 1] - vhdr.level[fhdr.nz - 2]) / 2.0;
  }

  plot_min_x = 0.0;
  plot_max_x = Length;
  range_x = plot_max_x - plot_min_x;
  plot_max_x += range_x * ((double) yaxismargin / (double) image_width);

  plot_min_y = Min_z;
  plot_max_y = Max_z;
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
