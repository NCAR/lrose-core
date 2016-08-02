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
 * contours.cc
 *
 * module for contouring
 *
 * Mike Dixon, RAP, NCAR, Boulder, CO, USA
 *
 * June 2001
 *
 *************************************************************************/

#include <X11/Xlib.h>
#include <cstdio>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
#include <toolsa/TaArray.hh>
#include <Mdv/MdvxField.hh>
#include <rapplot/gplot.h>

#include "Rview.hh"
using namespace std;

static void draw_contour_level(int dev,
			       const gframe_t *frame,
			       GC gc,
			       psgc_t *psgc,
			       int index,
			       const MdvxContourLevel &level,
			       char *label_loc,
			       int max_n_labels_x,
			       int max_n_labels_y);

static void draw_contour_labels(int dev,
				const gframe_t *frame,
				psgc_t *psgc,
				int contourIndex,
				int n_points,
				GPoint *gpoints,
				double level,
				char *label_loc,
				int max_n_labels_x,
				int max_n_labels_y);

static void _GFillPolygon(int dev, 
			  const gframe_t *frame, 
			  GC xgc, 
			  const psgc_t *psgc, 
			  const MdvxContourTriangle &triangle,
			  int mode);

////////////////////////////////////////////////
// draw_contours()
//
// draws contours to frame

#define MAX_N_LABELS_X 15
#define MAX_N_LABELS_Y 45

void draw_contours(int dev,
		   const gframe_t *frame,
		   GC pos_gc,
		   GC zero_gc,
		   GC neg_gc,
		   psgc_t *pos_psgc,
		   psgc_t *zero_psgc,
		   psgc_t *neg_psgc,
		   MdvxContour &cont)

{

  // create array for keeping track of label positions

  int max_n_labels_x = MAX_N_LABELS_X;
  int max_n_labels_y = MAX_N_LABELS_Y;

  char label_loc[MAX_N_LABELS_Y * MAX_N_LABELS_X];
  memset(label_loc, 0, max_n_labels_y * max_n_labels_x * sizeof(char));

  const vector<MdvxContourLevel> levels = cont.getLevels();
  
  for (size_t ii = 0; ii < levels.size(); ii++) {

    const MdvxContourLevel level = levels[ii];

    if (-0.0001 < level.val && level.val < 0.0001) {
      
      draw_contour_level(dev, frame, zero_gc, zero_psgc, ii, level,
			 label_loc, max_n_labels_x, max_n_labels_y);
      
    } else if (level.val < 0) {
      
      draw_contour_level(dev, frame, neg_gc, neg_psgc, ii, level,
			 label_loc, max_n_labels_x, max_n_labels_y);
      
    } else {
      
      draw_contour_level(dev, frame, pos_gc, pos_psgc, ii, level,
			 label_loc, max_n_labels_x, max_n_labels_y);
      
    }
    
  } // ii

}

//////////////////////////////
// draw given contour level

static void draw_contour_level(int dev,
			       const gframe_t *frame,
			       GC gc,
			       psgc_t *psgc,
			       int index,
			       const MdvxContourLevel &level,
			       char *label_loc,
			       int max_n_labels_x,
			       int max_n_labels_y)

{

  psgc->file = frame->psgc->file;

  if (level.lines.size() > 0) {

    // contiguous lines

    for (size_t il = 0; il < level.lines.size(); il++) {
      
      const MdvxContourLine &line = level.lines[il];
      
      if (line.pts.size() > 0) {
	
	// load up gpoint array

	TaArray<GPoint> gpoints_;
	GPoint *gpoints = gpoints_.alloc(line.pts.size());
	
	for (size_t ii = 0; ii < line.pts.size(); ii++) {
	  gpoints[ii].x = line.pts[ii].x;
	  gpoints[ii].y = line.pts[ii].y;
	} // i
	
	// draw the lines
	
	GDrawLines(dev, frame, gc,psgc, gpoints,
		   line.pts.size(), CoordModeOrigin);
	
	// put in the labels
	
	draw_contour_labels(dev, frame, psgc, index,
			    (int) line.pts.size(), gpoints, level.val,
			    label_loc, max_n_labels_x, max_n_labels_y);
	
      } // if (line.pts.size() > 0)
      
    } // il

  } else {

    // line segments

    for (size_t is = 0; is < level.segments.size(); is++) {
      
      const MdvxContourSegment &seg = level.segments[is];
      
      GDrawLine(dev, frame, gc, psgc,
		seg.start.x,
		seg.start.y,
		seg.end.x,
		seg.end.y);

    } // is

  } // if (level.lines.size() > 0) 
  
}

/*********************************
 * draw and label contour segments
 */

static void draw_contour_labels(int dev,
				const gframe_t *frame,
				psgc_t *psgc,
				int contourIndex,
				int n_points,
				GPoint *gpoints,
				double level,
				char *label_loc,
				int max_n_labels_x,
				int max_n_labels_y)

{

  static int n_ideal_contourlabels;
  static double ps_contourlabel_fontsize;
  
  static bool first_call = true;
  if (first_call) {
    n_ideal_contourlabels = uGetParamLong(Glob->prog_name,
					  "n_ideal_contourlabels",
					  5);
    ps_contourlabel_fontsize = uGetParamDouble(Glob->prog_name,
					       "ps_contourlabel_fontsize",
					       PS_CONTOURLABEL_FONTSIZE);
    first_call = false;
  }
  
  double mean_dim = ((frame->w_xmax - frame->w_xmin) +
		     (frame->w_ymax - frame->w_ymin)) * 0.5;
  double label_spacing = mean_dim / n_ideal_contourlabels;

  if (dev == PSDEV) {
    PsGsave(frame->psgc->file);
    PsSetFont(frame->psgc->file, frame->psgc->fontname,
	      ps_contourlabel_fontsize);
  }
	  
  double dist_to_next = (label_spacing * (0.25 + (contourIndex % 10) * 0.1));
  char label_str[64];
  sprintf(label_str, "%g", level);
  psgc->fontsize = ps_contourlabel_fontsize;

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
	  (((labelx - frame->w_xmin) / (frame->w_xmax - frame->w_xmin)) *
	   max_n_labels_x);
	if (ipos_x < 0) {
	  ipos_x = 0;
	}
	if (ipos_x > max_n_labels_x - 1) {
	  ipos_x = max_n_labels_x - 1;
	}
	
	int ipos_y = (int)
	  (((labely - frame->w_ymin) / (frame->w_ymax - frame->w_ymin)) *
	   max_n_labels_y);
	if (ipos_y < 0) {
	  ipos_y = 0;
	}
	if (ipos_y > max_n_labels_y - 1) {
	  ipos_y = max_n_labels_y - 1;
	}
	int ipos = ipos_y * max_n_labels_x + ipos_x;
	
	if (label_loc[ipos] == 0) {
	  GDrawImageString(dev, frame, Glob->contourlabel_gc,
			   Glob->x_contourlabel_font, psgc, 
			   XJ_CENTER, XJ_CENTER, labelx, labely, label_str);
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
    GDrawImageString(dev, frame, Glob->contourlabel_gc,
		     Glob->x_contourlabel_font, psgc, 
		     XJ_CENTER, XJ_CENTER,
		     gpoints[ii].x, gpoints[ii].y,
		     label_str);
  }

  if (dev == PSDEV) {
    PsGrestore(frame->psgc->file);
  }

  return;

}


////////////////////////////////////////////////
// draw_contour_triangles
//
// draws contour triangles to frame

void draw_contour_triangles(int dev,
			    const gframe_t *frame,
			    g_color_scale_t *colors,
			    MdvxContour &cont)
  
{

  const vector<MdvxContourLevel> levels = cont.getLevels();
  
  for (size_t ilevel = 0; ilevel < levels.size(); ilevel++) {
    
    const MdvxContourLevel level = levels[ilevel];

    GC gc;
    psgc_t *psgc;

    if (level.extra) {
      gc = Glob->pixmap_gc;
      psgc = frame->psgc;
    } else {
      gc = colors->level[ilevel].gc;
      psgc = colors->level[ilevel].psgc;
    }
 
    for (size_t ii = 0; ii < level.triangles.size(); ii++) {
	
      _GFillPolygon(dev, frame, gc, psgc,
		    level.triangles[ii], CoordModeOrigin);
      
    } // ii

  } // ilevel

}

/**************************************************************************
 * GFillPolygon(): fills polygon
 *
 *************************************************************************/

static void _GFillPolygon(int dev, 
			  const gframe_t *frame, 
			  GC xgc, 
			  const psgc_t *psgc, 
			  const MdvxContourTriangle &triangle,
			  int mode)

{

  int i;
  double xmin, ymin, xmax, ymax;
  PsPoint *pspoint, *prev_pspoint;
  XPoint *xpoint;
  int npoints = 3;

  /*
   * return early if the line cannot fall within the frame
   */

  xmin = LARGE_DOUBLE;
  ymin = LARGE_DOUBLE;
  xmax = -LARGE_DOUBLE;
  ymax = -LARGE_DOUBLE;

  const MdvxContourPt *vtex = triangle.vertex;

  for (i = 0; i < npoints; i++, vtex++) {
    xmin = MIN(xmin, vtex->x);
    ymin = MIN(ymin, vtex->y);
    xmax = MAX(xmax, vtex->x);
    ymax = MAX(ymax, vtex->y);
  } /* i */

  if (xmax < frame->w_xmin ||
      ymax < frame->w_ymin ||
      xmin > frame->w_xmax ||
      ymin > frame->w_ymax)
    return;

  /*
   * draw the polygons
   */

  switch (dev) {

  case XDEV: {
    
    /*
     * create array of XPoints, and load them up
     */

    TaArray<XPoint> xpoints_;
    XPoint *xpoints = xpoints_.alloc(npoints);

    xpoint = xpoints;
    vtex = triangle.vertex;
    
    for (i = 0; i < npoints; i++, xpoint++, vtex++) {

      xpoint->x = (int) ((vtex->x - frame->w_xmin) * frame->x->xscale + 0.5);
      xpoint->y = (int) ((frame->w_ymax - vtex->y) * frame->x->yscale + 0.5);
      
    }

    XFillPolygon(frame->x->display, frame->x->drawable, xgc,
		 xpoints, npoints, Complex, mode);

  }

  break;

  case PSDEV: {

    PsGsave(frame->psgc->file);

    /*
     * set the gralevel
     */
    
    fprintf(frame->psgc->file, "%g setgray\n", psgc->graylevel);

    TaArray<PsPoint> pspoints_;
    PsPoint *pspoints = pspoints_.alloc(npoints);

    pspoint = pspoints;
    vtex = triangle.vertex;

    for (i = 0; i < npoints; i++, pspoint++, vtex++) {

      pspoint->x = (int) ((vtex->x - frame->w_xmin) * frame->ps->xscale
			  + frame->ps->xmin + 0.5);
      pspoint->y = (int) ((vtex->y - frame->w_ymin) * frame->ps->yscale
			  + frame->ps->ymin + 0.5);

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

    PsGrestore(frame->psgc->file);

  }

  break;

  }

}

