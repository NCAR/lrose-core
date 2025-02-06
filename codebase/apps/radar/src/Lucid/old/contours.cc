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
/**********************************************************************
 * CONTOURS.CC
 *
 * Contour rendering
 */

#include "cidd.h"
#include <Mdv/MdvxContour.hh>
#include <vector>

#define IDEAL_NUM_VERT_CONT_LEVELS 10.0
#define MAX_N_LABELS_X 15
#define MAX_N_LABELS_Y 45

#ifdef NOTYET
static void draw_lines_for_level(const GframeObj &frame,
                                 int contourIndex,
                                 const MdvxContourLevel &level,
                                 contour_info_t *crec );

static void draw_labels_for_level(const GframeObj &frame,
				  int contourIndex,
				  const MdvxContourLevel &level,
				  contour_info_t *crec,
				  QFont *font,
				  char *label_loc,
				  int max_n_labels_x,
				  int max_n_labels_y);
  
static void draw_labels_for_line(const GframeObj *gframe,
                                 int contourIndex,
                                 int n_points,
                                 GPoint *gpoints,
                                 double level,
                                 QBrush brush,
                                 QFont *font,
                                 char *label_loc,
                                 int max_n_labels_x,
                                 int max_n_labels_y);

static void fill_triangle(const GframeObj *frame, QBrush xgc, 
			  const MdvxContourTriangle &triangle, int mode);
#endif

/////////////////////////////////////////////////////////////////////////

void RenderLineContours(QPaintDevice *pdev, contour_info_t *crec,
			bool is_vert /* = false */)

{

  // get the data field

  MetRecord *mr = gd.mrec[crec->field];
  MdvxField field;
  if (is_vert) {
    if(mr->v_mdvx->getField(0) == NULL) return;
    field = *mr->v_mdvx->getField(0);
  } else {
    if(mr->h_mdvx->getField(0) == NULL) return;
    field = *mr->h_mdvx->getField(0);
  }
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  fl32 *data = (fl32 *) field.getVol();
  int npts = fhdr.nx * fhdr.ny;
  fl32 missing = fhdr.missing_data_value;
  fl32 bad = fhdr.bad_data_value;

  // map bad to missing

  if (bad != missing) {
    fl32 *fptr = data;
    for (int ii = 0; ii < npts; ii++, fptr++) {
      if (*fptr == bad) {
	*fptr = missing;
      }
    }
  }

  // map missing to min value

  if (gd.layers.map_missing_to_min_value) {
    fl32 min_val = fhdr.min_value;
    fl32 *fptr = data;
    for (int ii = 0; ii < npts; ii++, fptr++) {
      if (*fptr == missing) {
	*fptr = min_val;
      }
    }
  }

  MdvxContour cont;

  if (crec->interval != 0) {

    // set the contours vals from min, max and interval
    
    double interval = crec->interval;
    if (is_vert) {
      double dyn_range = (fhdr.max_value - fhdr.min_value);
      if (dyn_range > 0) {
	interval =
	  compute_cont_interval(dyn_range / IDEAL_NUM_VERT_CONT_LEVELS);
      }
    }
    
    cont.setVals(crec->min, crec->max, interval);

  } else {

    // set the contour values from the color scale

    bool increasing = true;

    Valcolormap_t *vcm;
    if (is_vert) {
      vcm = &mr->h_vcm;
    } else {
      vcm = &mr->v_vcm;
    }

    if (vcm->nentries > 1) {
      if (vcm->vc[vcm->nentries - 1]->min < vcm->vc[0]->min) {
	increasing = false;
      }
    }
    
    if (increasing) {
      for (int ilevel = 0; ilevel < vcm->nentries; ilevel++) {
	cont.addVal(vcm->vc[ilevel]->min);
      }
      cont.addVal(vcm->vc[vcm->nentries - 1]->max);
    } else {
      for (int ilevel = vcm->nentries - 1; ilevel >= 0; ilevel--) {
	cont.addVal(vcm->vc[ilevel]->min);
      }
      cont.addVal(vcm->vc[0]->max);
    }

  }
    
  if (crec->labels_on) {
    if (cont.computeLines(field)) {
      cerr << "ERROR - RenderLineContours" << endl;
      return;
    }
  } else {
    if (cont.computeSegments(field)) {
      cerr << "ERROR - RenderLineContours" << endl;
      return;
    }
  }

#ifdef NOTYET

  // Pick a font size based on the detail thresholds and domain

  // int font_index = gd.uparams->getLong("contour_font_num", 6);
  int font_index = 0;
  if(font_index <0) font_index = 0;
  if(font_index >= _params.fonts_n) font_index = _params.fonts_n -1;
  QFont *font = gd.fontst[font_index];
  
  // set up the domain for rendering

  win_param_t *win;
  double min_x, max_x;
  double min_y, max_y;
  if (is_vert) {
    win = &gd.v_win;
    min_x = 0;
    max_x = gd.h_win.route.total_length;
    min_y = win->min_ht;
    max_y = win->max_ht;
  } else {
    win = &gd.h_win;
    min_x = win->cmin_x;
    max_x = win->cmax_x;
    min_y = win->cmin_y;
    max_y = win->cmax_y;
  }
  
  double x_dproj_per_pixel = (max_x - min_x) / (double)win->img_dim.width;
  double y_dproj_per_pixel = (max_y - min_y) / (double)win->img_dim.height;

  double left_km = (double)win->margin.left * x_dproj_per_pixel;
  double right_km = (double)win->margin.right * x_dproj_per_pixel;
  double top_km = (double)win->margin.top * y_dproj_per_pixel;
  double bot_km = (double)win->margin.bot * y_dproj_per_pixel;  
  
  GframeObj *gframe = RenderContext::gCreateFrame(min_x - left_km,
                                                  min_y - bot_km,
                                                  max_x + right_km,
                                                  max_y + top_km);

  GSetXRef(gframe, gd.dpy, xid,
	   win->can_dim.width, win->can_dim.height,
	   font);

  // create array for keeping track of label positions
  int max_n_labels_x = MAX_N_LABELS_X;
  int max_n_labels_y = MAX_N_LABELS_Y;
  vector<char> label_loc;
  label_loc.resize(max_n_labels_y * max_n_labels_x);
  memset(label_loc.data(), 0, max_n_labels_y * max_n_labels_x * sizeof(char));

  // render lines for each level

  const vector<MdvxContourLevel> levels = cont.getLevels();
  for (size_t ii = 0; ii < levels.size(); ii++) {
    draw_lines_for_level(*gframe, ii, levels[ii], crec);
  } // ii

  // render labels for each level

  for (size_t ii = 0; ii < levels.size(); ii++) {
    draw_labels_for_level(*gframe, ii, levels[ii], crec,
			  font, label_loc.data(), max_n_labels_x, max_n_labels_y);
  } // ii
  
  // Add the list of times to the time plot

  if(!is_vert && mr->time_list.num_entries > 0) {
    if(gd.time_plot) gd.time_plot->add_grid_tlist(mr->button_name,
				 mr->time_list.tim,
				 mr->time_list.num_entries,
				 mr->h_date.unix_time);
  }

  delete gframe;

#endif

}

#ifdef NOTYET
  
//////////////////////////////
// draw given contour level

static void draw_lines_for_level(const GframeObj &frame,
				 int contourIndex,
				 const MdvxContourLevel &level,
				 contour_info_t *crec)

{

  // set gc
  
  QBrush brush;
  double val = level.val;

  // check for special value

  bool val_is_special = false;
  if (fabs(val - gd.layers.special_contour_value) < 1.0e-6) {
      val_is_special = true;
  }

  if (crec->interval != 0) { 
      gc =  crec->color->gc;
  } else {
      gc = Val2GC(crec->vcm,val);
  }

  if(gc == NULL) return; // No color for this level

  if(val_is_special) {
    XSetLineAttributes(gd.dpy, gc, gd.layers.contour_line_width*2,
		       LineSolid, CapButt, JoinMiter);
  } else {
    XSetLineAttributes(gd.dpy, gc, gd.layers.contour_line_width,
		       LineSolid, CapButt, JoinMiter);
  }

  if (level.lines.size() > 0) {
    
    // contiguous lines
    
    for (size_t il = 0; il < level.lines.size(); il++) {
      
      const MdvxContourLine &line = level.lines[il];
      
      if (line.pts.size() > 0) {
	
	// load up gpoint array

        vector<GPoint> gpoints;
        gpoints.resize(line.pts.size());
	
	for (size_t ii = 0; ii < line.pts.size(); ii++) {
	  gpoints[ii].x = line.pts[ii].x;
	  gpoints[ii].y = line.pts[ii].y;
	} // i
	
	// draw the lines
	
	GDrawLines(XDEV, &frame, gc, NULL,
		   gpoints.data(), line.pts.size(), CoordModeOrigin);
	
      } // if (line.pts.size() > 0)
      
    } // il

  } else {

    // line segments

    for (size_t is = 0; is < level.segments.size(); is++) {
      
      const MdvxContourSegment &seg = level.segments[is];
      
      GDrawLine(XDEV, &frame, gc, NULL,
		seg.start.x,
		seg.start.y,
		seg.end.x,
		seg.end.y);

    } // is

  } // if (level.lines.size() > 0)

}

#endif

///////////////////////////////////////
// draw labels for given contour level

#ifdef NOTYET
  
static void draw_labels_for_level(const GframeObj &frame,
				  int contourIndex,
				  const MdvxContourLevel &level,
				  contour_info_t *crec,
				  QFont *font,
				  char *label_loc,
				  int max_n_labels_x,
				  int max_n_labels_y)
  
{

  if (level.lines.size() == 0) {
    return;
  }
    
  // set gc
  
  QBrush brush;
  double val = level.val;

  // check for special value

  // bool val_is_special = false;
  // if (fabs(val - gd.layers.special_contour_value) < 1.0e-6) {
  //     val_is_special = true;
  // }

  if (crec->interval != 0) { 
      gc =  crec->color->gc;
  } else {
      gc = Val2GC(crec->vcm,val);
  }

  if(gc == NULL) return; // No color for this level

  XSetFont(gd.dpy, gc, font->fid);
  
  // contiguous lines
  
  for (size_t il = 0; il < level.lines.size(); il++) {
    
    const MdvxContourLine &line = level.lines[il];
    
    if (line.pts.size() > 0) {
      
      // load up gpoint array
      
      vector<GPoint> gpoints;
      gpoints.resize(line.pts.size());
      
      for (size_t ii = 0; ii < line.pts.size(); ii++) {
	gpoints[ii].x = line.pts[ii].x;
	gpoints[ii].y = line.pts[ii].y;
      } // i
      
      // put in the labels
      
      draw_labels_for_line(&frame, contourIndex, (int) line.pts.size(),
			   gpoints.data(), level.val, gc, font,
			   label_loc, max_n_labels_x, max_n_labels_y);
      
    } // if (line.pts.size() > 0)
    
  } // il

}

#endif
  
#ifdef NOTYET
  
/*********************************
 * draw labels for given line
 */

static void draw_labels_for_line(const GframeObj *gframe,
				 int contourIndex,
				 int n_points,
				 GPoint *gpoints,
				 double level,
				 QBrush brush,
				 QFont *font,
				 char *label_loc,
				 int max_n_labels_x,
				 int max_n_labels_y)

{

  // int n_ideal_labels = _params.n_ideal_contour_labels;
  int n_ideal_labels = 6;
  double mean_dim = ((gframe->w_xmax - gframe->w_xmin) +
		     (gframe->w_ymax - gframe->w_ymin)) * 0.5;
  double label_spacing = mean_dim / n_ideal_labels;
  
  double dist_to_next =
    (label_spacing * (0.25 + (contourIndex % 10) * 0.1));
  
  char label_str[64];
  snprintf(label_str, 64, "%g", level);

  bool plotted = false;
  for (int ii = 0; ii < n_points - 1; ii++) {

    double x1 = gpoints[ii].x;
    double x2 = gpoints[ii+1].x;
    double y1 = gpoints[ii].y;
    double y2 = gpoints[ii+1].y;

    double dx = x2 - x1;
    double dy = y2 - y1;

    double len = sqrt(dx * dx + dy * dy);
    
    if (len > dist_to_next) {
      
      double dist_since_last = label_spacing - dist_to_next;
      int nThisSeg = (int) ((len + dist_since_last) / label_spacing);
      double d_left = dist_to_next - len;

      for (int ii = 1; ii <= nThisSeg; ii++) {

	double dist = ii * label_spacing - dist_since_last;
	d_left = len - dist;
	double fraction = dist / len;
	double xincr = dx * fraction;
	double yincr = dy * fraction;
	double labelx = x1 + xincr;
	double labely = y1 + yincr;
	
	int ipos_x = (int)
	  (((labelx - gframe->w_xmin) / (gframe->w_xmax - gframe->w_xmin)) *
	   max_n_labels_x);
	if (ipos_x < 0) {
	  ipos_x = 0;
	}
	if (ipos_x > max_n_labels_x - 1) {
	  ipos_x = max_n_labels_x - 1;
	}
	
	int ipos_y = (int)
	  (((labely - gframe->w_ymin) / (gframe->w_ymax - gframe->w_ymin)) *
	   max_n_labels_y);
	if (ipos_y < 0) {
	  ipos_y = 0;
	}
	if (ipos_y > max_n_labels_y - 1) {
	  ipos_y = max_n_labels_y - 1;
	}
	int ipos = ipos_y * max_n_labels_x + ipos_x;
	
	if (label_loc[ipos] == 0) {
	  GDrawImageString(XDEV, gframe, gc, font, NULL,
			   XJ_CENTER, XJ_CENTER,
			   labelx, labely,
			   label_str);
	  label_loc[ipos] = 1;
	}
	
	plotted = true;

      } // ii
      
      dist_to_next = label_spacing - d_left;
      
    } else {
      
      dist_to_next -= len;

    }

  } // ii

  // if not plotted, label the mid point

  if (!plotted) {
    int ii = n_points / 2;
    GDrawImageString(XDEV, gframe, gc, font, NULL,
		     XJ_CENTER, XJ_CENTER,
		     gpoints[ii].x, gpoints[ii].y,
		     label_str);
  }

}

#endif

//////////////////////////
// render filled polygons

void RenderFilledPolygons(QPaintDevice *pdev,
			  MetRecord *mr, 
			  bool is_vert /* = false */)

{

#ifdef NOTYET
  
  // get the data field

  MdvxField field;
  if (is_vert) {
    if(mr->v_mdvx->getField(0) == NULL) return;
    field = *mr->v_mdvx->getField(0);
  } else {
    if(mr->h_mdvx->getField(0) == NULL) return;
    field = *mr->h_mdvx->getField(0);
  }
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  fl32 *data = (fl32 *) field.getVol();
  int npts = fhdr.nx * fhdr.ny;
  fl32 missing = fhdr.missing_data_value;
  fl32 bad = fhdr.bad_data_value;

  // map bad to missing

  if (bad != missing) {
    fl32 *fptr = data;
    for (int ii = 0; ii < npts; ii++, fptr++) {
      if (*fptr == bad) {
	*fptr = missing;
      }
    }
  }

  // map missing to min value

  if (gd.layers.map_missing_to_min_value) {
    fl32 min_val = fhdr.min_value;
    fl32 *fptr = data;
    for (int ii = 0; ii < npts; ii++, fptr++) {
      if (*fptr == missing) {
	*fptr = min_val;
      }
    }
  }

  // compute the contour polygons

  Valcolormap_t *vcm;
  if (is_vert) {
    vcm = &mr->h_vcm;
  } else {
    vcm = &mr->v_vcm;
  }
  
  MdvxContour cont;

  // check for contour ordering

  bool increasing = true;
  if (vcm->nentries > 1) {
    if (vcm->vc[vcm->nentries - 1]->min < vcm->vc[0]->min) {
      increasing = false;
    }
  }

  // set up contour levels

  if (increasing) {
    for (int ilevel = 0; ilevel < vcm->nentries; ilevel++) {
      cont.addVal(vcm->vc[ilevel]->min);
    }
    cont.addVal(vcm->vc[vcm->nentries - 1]->max);
  } else {
    for (int ilevel = vcm->nentries - 1; ilevel >= 0; ilevel--) {
      cont.addVal(vcm->vc[ilevel]->min);
    }
    cont.addVal(vcm->vc[0]->max);
  }
  
  if (cont.computeTriangles(field)) {
    cerr << "ERROR - RenderFilledPolygons" << endl;
    return;
  }
  
  // set up the domain for rendering

  win_param_t *win;
  double min_x, max_x;
  double min_y, max_y;
  if (is_vert) {
    win = &gd.v_win;
    min_x = 0;
    max_x = gd.h_win.route.total_length;
    min_y = win->min_ht;
    max_y = win->max_ht;
  } else {
    win = &gd.h_win;
    min_x = win->cmin_x;
    max_x = win->cmax_x;
    min_y = win->cmin_y;
    max_y = win->cmax_y;
  }
  
  double x_dproj_per_pixel = (max_x - min_x) / (double)win->img_dim.width;
  double y_dproj_per_pixel = (max_y - min_y) / (double)win->img_dim.height;

  double left_km = (double)win->margin.left * x_dproj_per_pixel;
  double right_km = (double)win->margin.right * x_dproj_per_pixel;
  double top_km = (double)win->margin.top * y_dproj_per_pixel;
  double bot_km = (double)win->margin.bot * y_dproj_per_pixel;  
  
  GframeObj *gframe = GCreateFrame(min_x - left_km,
				  min_y - bot_km,
				  max_x + right_km,
				  max_y + top_km);
  
  GSetXRef(gframe, gd.dpy, xid,
	   win->can_dim.width, win->can_dim.height, NULL);

  //  Fill in background color
  if(!is_vert && gd.layers.missing_data_color->gc != NULL) {
      double    x_dproj,y_dproj;
      double    x_dproj2,y_dproj2;

      // Map grid point 0,0 to the display projection
      grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);

      // Map grid point nx,ny,0 to the display projection
      grid_to_disp_proj(mr,mr->h_fhdr.nx-1,mr->h_fhdr.ny-1,&x_dproj2,&y_dproj2);
      
      GFillRectangle(XDEV, gframe, gd.layers.missing_data_color->gc, NULL,
		     x_dproj,y_dproj,(x_dproj2 - x_dproj), (y_dproj2 - y_dproj));
  }

  if(!is_vert && gd.layers.bad_data_color->gc != NULL) {
      double    x_dproj,y_dproj;
      double    x_dproj2,y_dproj2;

      // Map grid point 0,0 to the display projection
      grid_to_disp_proj(mr,0,0,&x_dproj,&y_dproj);

      // Map grid point nx,ny,0 to the display projection
      grid_to_disp_proj(mr,mr->h_fhdr.nx-1,mr->h_fhdr.ny-1,&x_dproj2,&y_dproj2);
      
      GFillRectangle(XDEV, gframe, gd.layers.bad_data_color->gc, NULL,
		     x_dproj,y_dproj,(x_dproj2 - x_dproj), (y_dproj2 - y_dproj));
  }

  // render for each level

  const vector<MdvxContourLevel> levels = cont.getLevels();

  for (size_t ilevel = 0; ilevel < levels.size(); ilevel++) {

    const MdvxContourLevel level = levels[ilevel];

    QBrush brush;
    if ((int) ilevel < vcm->nentries) {
      if (level.extra) {
	gc = gd.legends.background_color->gc;
      } else {
	if (increasing) {
	  gc = vcm->vc[ilevel]->gc;
	} else {
	  gc = vcm->vc[vcm->nentries-ilevel-1]->gc;
	}
      }
    } else {
      gc = gd.legends.background_color->gc;
    }

    for (size_t ii = 0; ii < level.triangles.size(); ii++) {
      fill_triangle(gframe, gc, level.triangles[ii], CoordModeOrigin);
    } // ii
    
  } // ilevel

  // clean up

  GFreeFrame(gframe);

#endif

}

#ifdef NOTYET
  
//////////////////////////////////////////////////////////////////////////////
// GFillTriangle()

static void fill_triangle(const GframeObj *frame, QBrush xgc, 
			  const MdvxContourTriangle &triangle,
			  int mode)
  
{

  // clip
  
  double xmin = LARGE_DOUBLE;
  double ymin = LARGE_DOUBLE;
  double xmax = -LARGE_DOUBLE;
  double ymax = -LARGE_DOUBLE;

  const MdvxContourPt *vtex = triangle.vertex;

  for (int i = 0; i < 3; i++, vtex++) {
    xmin = MIN(xmin, vtex->x);
    ymin = MIN(ymin, vtex->y);
    xmax = MAX(xmax, vtex->x);
    ymax = MAX(ymax, vtex->y);
  }

  if (xmax < frame->w_xmin || ymax < frame->w_ymin ||
      xmin > frame->w_xmax || ymin > frame->w_ymax) {
    return;
  }

  // create array of XPoints, and load them up
  
  XPoint xpoints[3];
  
  XPoint *xpoint = xpoints;
  vtex = triangle.vertex;
  
  for (int i = 0; i < 3; i++, xpoint++, vtex++) {
    xpoint->x = (int) ((vtex->x - frame->w_xmin) * frame->x->xscale + 0.5);
    xpoint->y = (int) ((frame->w_ymax - vtex->y) * frame->x->yscale + 0.5);
  }

  // draw polygon

  XFillPolygon(frame->x->display, frame->x->drawable, xgc,
	       xpoints, 3, Complex, mode);

}

#endif

