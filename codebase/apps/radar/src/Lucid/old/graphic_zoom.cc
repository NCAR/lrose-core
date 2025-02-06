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
/*************************************************************************
 * GRAPHIC_ZOOM.C: 
 *
 * For the Cartesian Interactive Data Display (CIDD) 
 * Frank HageJuly 1991 NCAR, Research Applications Program
 */

#define GRAPHIC_ZOOM 1

#include "cidd.h"

class GeomState {
public:
  double minx, miny;
  double maxx, maxy;
  GeomState(double minx, double miny, double maxx, double maxy) :
          minx(minx),
          miny(miny),
          maxx(maxx),
          maxy(maxy)
  {
  }
};

static deque<GeomState> _zoom_stack;
static void _do_zoom();

extern int b_lastx,b_lasty; /* Boundry end point */
extern int b_startx,b_starty; /* Boundry start point */
extern int p_lastx,p_lasty; /* Pan end point */
extern int p_startx,p_starty; /* Pan start point */
extern int r_lastx,r_lasty; /* ROUTE end point */
extern int r_startx,r_starty; /* ROUTE start point */

/*************************************************************************
 * DO_VERT_ZOOM : Zoom Cross (Vertical) Section 
 */

void do_vert_zoom()
{
  int index;
  double km_x1,km_x2,ht1,ht2;
 
  pixel_to_disp_proj_v(&gd.v_win.margin,b_startx,b_starty,&km_x1,&ht1);
  pixel_to_disp_proj_v(&gd.v_win.margin,b_lastx,b_lasty,&km_x2,&ht2);

  index = 0;

  static double min_zoom_threshold = 0.0;
 
  if(min_zoom_threshold == 0.0) {
    min_zoom_threshold = _params.min_zoom_threshold;
  }
  /* Only do additional zooming if request is at least min_zoom_threshold on one side */
  if((fabs(km_x2 - km_x1) > min_zoom_threshold) ||
     (fabs(ht2 - ht1) > min_zoom_threshold)) {
    if(km_x1 < km_x2) { /* put coords in ascending order */
      gd.v_win.zmin_x[index] = km_x1;
      gd.v_win.zmax_x[index] = km_x2;
    } else {
      gd.v_win.zmin_x[index] = km_x2;
      gd.v_win.zmax_x[index] = km_x1;
    }
    if(ht2 < ht1) { 
      gd.v_win.zmin_y[index] = ht2;
      gd.v_win.zmax_y[index] = ht1;
    } else {
      gd.v_win.zmin_y[index] = ht1;
      gd.v_win.zmax_y[index] = ht2;
    }
  }

  /* Set current area to our indicated zoom area */
  gd.v_win.cmin_x = gd.v_win.zmin_x[index];
  gd.v_win.cmax_x = gd.v_win.zmax_x[index];
  gd.v_win.cmin_y = gd.v_win.zmin_y[index];
  gd.v_win.cmax_y = gd.v_win.zmax_y[index];

  set_redraw_flags(0,1);
}

/*************************************************************************
 * DO_ZOOM : Zoom area
 */

void do_zoom()
{

  // save current state on the zoom stack

  save_current_zoom(gd.h_win.cmin_x, gd.h_win.cmin_y, 
                     gd.h_win.cmax_x, gd.h_win.cmax_y);

  // do the zoom
  _do_zoom();

}

static void _do_zoom()

{

  int index;
  double km_x1,km_x2,km_y1,km_y2;
  double dx,dy;
  static double min_zoom_threshold = 0.0;
 
  if(min_zoom_threshold == 0.0) {
    min_zoom_threshold = _params.min_zoom_threshold;
    if(gd.display_projection == Mdvx::PROJ_LATLON) {
      min_zoom_threshold /= KM_PER_DEG_AT_EQ;
    }
  }
  pixel_to_disp_proj(&gd.h_win.margin,b_startx,b_starty,&km_x1,&km_y1);
  pixel_to_disp_proj(&gd.h_win.margin,b_lastx,b_lasty,&km_x2,&km_y2);

  index = gd.h_win.num_zoom_levels -1;

  /* Only do additional zooming if request is at least min_zoom_threshold on one side */
  if((fabs(km_x2 - km_x1) > min_zoom_threshold) ||
     (fabs(km_y2 - km_y1) > min_zoom_threshold)) {
    if(km_x1 < km_x2) /* put coords in ascending order */
    {
      gd.h_win.zmin_x[index] = km_x1;
      gd.h_win.zmax_x[index] = km_x2;
    }
    else
    {
      gd.h_win.zmin_x[index] = km_x2;
      gd.h_win.zmax_x[index] = km_x1;
    }
    if(km_y1 < km_y2)
    { 
      gd.h_win.zmin_y[index] = km_y1;
      gd.h_win.zmax_y[index] = km_y2;
    }
    else
    {
      gd.h_win.zmin_y[index] = km_y2;
      gd.h_win.zmax_y[index] = km_y1;
    }
  }

  /* Force zoomed display area to be consistant with the window */
  dx = gd.h_win.zmax_x[index] - gd.h_win.zmin_x[index];
  dy = gd.h_win.zmax_y[index] - gd.h_win.zmin_y[index];

  switch(gd.display_projection) {
    case Mdvx::PROJ_LATLON:
      // Use the current average latitude to set the aspect correction
      gd.aspect_correction = cos(((gd.h_win.zmax_y[index] + gd.h_win.zmin_y[index])/2.0) * DEG_TO_RAD);

      /* forshorten the Y coords to make things look better */
      dy /= gd.aspect_correction;
      break;
  }

  /* Force the domain into the aspect ratio */
  // dy *= _params.aspect_ratio;

  /* use largest dimension */
  if(dx > dy)
  {
    gd.h_win.zmax_y[index] += (dx - dy) /2.0;
    gd.h_win.zmin_y[index] -= (dx - dy) /2.0;
  }
  else
  {
    gd.h_win.zmax_x[index] += (dy - dx) /2.0;
    gd.h_win.zmin_x[index] -= (dy - dx) /2.0;
  }

  /* make sure coords are within the limits of the display */
  if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
  {
    gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
    gd.h_win.zmax_x[index] = gd.h_win.max_x;
  }
  if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
  {
    gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
    gd.h_win.zmax_y[index] = gd.h_win.max_y;
  }
  if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
  {
    gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
    gd.h_win.zmin_x[index] = gd.h_win.min_x;
  }
  if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
  {
    gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
    gd.h_win.zmin_y[index] = gd.h_win.min_y;
  }

  /* Set current area to our indicated zoom area */
  gd.h_win.cmin_x = gd.h_win.zmin_x[index];
  gd.h_win.cmax_x = gd.h_win.zmax_x[index];
  gd.h_win.cmin_y = gd.h_win.zmin_y[index];
  gd.h_win.cmax_y = gd.h_win.zmax_y[index];

  double min_lat,max_lat,min_lon,max_lon;
  get_bounding_box(min_lat,max_lat,min_lon,max_lon);
	 
  gd.r_context->set_clip_limits(min_lat, min_lon, max_lat, max_lon);
  gd.prod_mgr->reset_product_valid_flags_zoom();

  gd.h_win.zoom_level = index;
  // xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);

  set_redraw_flags(1,0);
  if(!_params.always_get_full_domain) {
    reset_time_list_valid_flags();
    reset_data_valid_flags(1,0);
    reset_terrain_valid_flags(1,0);
  }
}

/*************************************************************************
 * DO_ZOOM_PAN : Pan the zoomed area
 */

void do_zoom_pan()
{
  int index;
  double km_x1,km_x2,km_y1,km_y2;
  double dx,dy;
 
  pixel_to_disp_proj(&gd.h_win.margin,p_startx,p_starty,&km_x1,&km_y1);
  pixel_to_disp_proj(&gd.h_win.margin,p_lastx,p_lasty,&km_x2,&km_y2);
 
  dx = km_x1 - km_x2;
  dy = km_y1 - km_y2;

  if(gd.h_win.zoom_level < gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS) {
    index = gd.h_win.num_zoom_levels - NUM_CUSTOM_ZOOMS;
  } else {
    index = gd.h_win.zoom_level;
    if(index >= gd.h_win.num_zoom_levels)
      index = gd.h_win.num_zoom_levels -1;
  }

  gd.h_win.zmin_x[index] = gd.h_win.cmin_x + dx;
  gd.h_win.zmin_y[index] = gd.h_win.cmin_y + dy;
  gd.h_win.zmax_x[index] = gd.h_win.cmax_x + dx;
  gd.h_win.zmax_y[index] = gd.h_win.cmax_y + dy;

  /* make sure coords are within the limits of the display */
  if(gd.h_win.zmax_x[index] > gd.h_win.max_x)
  {
    gd.h_win.zmin_x[index] -= (gd.h_win.zmax_x[index] - gd.h_win.max_x);
    gd.h_win.zmax_x[index] = gd.h_win.max_x;
  }
  if(gd.h_win.zmax_y[index] > gd.h_win.max_y)
  {
    gd.h_win.zmin_y[index] -= (gd.h_win.zmax_y[index] - gd.h_win.max_y);
    gd.h_win.zmax_y[index] = gd.h_win.max_y;
  }
  if(gd.h_win.zmin_x[index] < gd.h_win.min_x)
  {
    gd.h_win.zmax_x[index] += (gd.h_win.min_x - gd.h_win.zmin_x[index]);
    gd.h_win.zmin_x[index] = gd.h_win.min_x;
  }
  if(gd.h_win.zmin_y[index] < gd.h_win.min_y)
  {
    gd.h_win.zmax_y[index] += (gd.h_win.min_y - gd.h_win.zmin_y[index]);
    gd.h_win.zmin_y[index] = gd.h_win.min_y;
  }

  /* Set current area to our indicated zoom area */
  gd.h_win.cmin_x = gd.h_win.zmin_x[index];
  gd.h_win.cmax_x = gd.h_win.zmax_x[index];
  gd.h_win.cmin_y = gd.h_win.zmin_y[index];
  gd.h_win.cmax_y = gd.h_win.zmax_y[index];

  // Use the current average latitude to set the aspect correction
  gd.aspect_correction = cos(((gd.h_win.zmax_y[index] + gd.h_win.zmin_y[index])/2.0) * DEG_TO_RAD);
 
  double min_lat,max_lat,min_lon,max_lon;
  get_bounding_box(min_lat,max_lat,min_lon,max_lon);

  gd.r_context->set_clip_limits(min_lat, min_lon, max_lat, max_lon);
  gd.prod_mgr->reset_product_valid_flags_zoom();

  gd.h_win.zoom_level = index;
  // xv_set(gd.zoom_pu->domain_st, PANEL_VALUE, index, NULL);

  set_redraw_flags(1,0);
  if(!_params.always_get_full_domain) {
    reset_time_list_valid_flags();
    reset_data_valid_flags(1,0);
    reset_terrain_valid_flags(1,0);
  }

}


/*************************************************************************
 * REDRAW_VERT_ZOOM_BOX: Draws the Zoom define box 
 *
 */

void redraw_vert_zoom_box()
{
#ifdef NOTYET
  int dx,dy;/* pixel distances */
  int sx,sy;

  dx = b_lastx - b_startx;
  dy = b_lasty - b_starty;

  if(dx < 0) { sx = b_startx + dx; dx = -dx; } else { sx = b_startx; } 
  if(dy < 0) { sy = b_starty + dy; dy = -dy; } else { sy = b_starty; } 

  XDrawRectangle(gd.dpy,gd.vcan_xid,gd.ol_gc,sx,sy,dx,dy);
#endif
  
}
 


/*************************************************************************
 * REDRAW_ZOOM_BOX: Draws the Zoom define box 
 *
 */

void redraw_zoom_box()
{
#ifdef NOTYET
  int dx,dy;/* pixel distances */
  int sx,sy;

  dx = b_lastx - b_startx;
  dy = b_lasty - b_starty;

  if(dx < 0) { sx = b_startx + dx; dx = -dx; } else { sx = b_startx; } 
  if(dy < 0) { sy = b_starty + dy; dy = -dy; } else { sy = b_starty; } 

  XDrawRectangle(gd.dpy,gd.hcan_xid,gd.ol_gc,sx,sy,dx,dy);
#endif
}
 
/*************************************************************************
 * REDRAW_PAN_LINE: Redraw the Pan Define line 
 *
 */

void redraw_pan_line()
{

  //XDrawLine(gd.dpy,gd.hcan_xid,gd.clear_ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* BIT PLANE MODE */
#ifdef NOTYET
  XDrawLine(gd.dpy,gd.hcan_xid,gd.ol_gc,p_startx,p_starty,p_lastx,p_lasty); /* XOR MODE */
#endif
}

/*************************************************************************
 * save the current zoom
 */

void save_current_zoom(double zoom_min_x, double zoom_min_y,
                       double zoom_max_x, double zoom_max_y)

{

  // save current state on the zoom stack
  GeomState geom(zoom_min_x, zoom_min_y, 
                 zoom_max_x, zoom_max_y);
  _zoom_stack.push_back(geom);
  
  // keep the stack to size less than 100
  if (_zoom_stack.size() > 100) {
    _zoom_stack.pop_front();
  }

}

/*************************************************************************
 * Go back to previous zoom
 */
void zoom_back()
{
  if (_zoom_stack.size() > 0) {
    // GeomState geom = _zoom_stack[_zoom_stack.size()-1];
    _zoom_stack.pop_back();
    // set_domain_zoom(geom.minx, geom.miny, geom.maxx, geom.maxy);
  }
}

/*************************************************************************
 * Clear the zoom stack
 */
void clear_zoom_stack()
{
  _zoom_stack.clear();
}

