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
 * RENDER_CLICK_MARKS : Rendering marks showing click points
 *
 * Mike Dixon Dec 2009  NCAR - RAL
 */

#define RENDER_CLICK_MARKS

#include "cidd.h"

static void _draw_mark(Drawable xid, double xx, double yy,
                       const Color_gc_t *color, int size);

void render_click_marks()
{

  // do we need to plot clicks?
  
  bool plotClick = false;
  bool plotClient = false;

  if (_params.mark_latest_click_location) {
    if (gd.coord_expt->pointer_x != 0 ||
        gd.coord_expt->pointer_y != 0) {
      plotClick = true;
    }
  }
  
  if (gd.mark_latest_client_location) {
    if (gd.coord_expt->pointer_x != 0 ||
        gd.coord_expt->pointer_y != 0) {
      plotClient = true;
    }
  }

  if (!plotClick && !plotClient) {
    return;
  }

  // copy the backing store over to the current visibile canvas
  // so we can clear any previous marks and then add the marks
  // to the canvas

  int xx1 = gd.h_win.can_dim.x_pos;
  int yy1 = gd.h_win.can_dim.y_pos;
  int xwidth = gd.h_win.can_dim.width;
  int yheight = gd.h_win.can_dim.height;
  XCopyArea(gd.dpy,gd.h_win.can_xid[gd.h_win.cur_cache_im],
            gd.h_win.vis_xid,
            gd.def_gc,
            xx1, yy1,
            xwidth, yheight,
            xx1, yy1);

  // mark latest click location in CIDD

  if (plotClick) {
    double xx = 0, yy = 0;
    double lon = gd.coord_expt->pointer_lon;
    double lat = gd.coord_expt->pointer_lat;
    gd.proj.latlon2xy(lat, lon, xx, yy);
    int size = _params.latest_click_mark_size;
    _draw_mark(gd.h_win.vis_xid, xx, yy, gd.legends.latest_click_mark_color, size);
  }

  // mark latest client location from client

  if (plotClient) {
    double xx = 0, yy = 0;
    double lon = gd.coord_expt->client_lon;
    double lat = gd.coord_expt->client_lat;
    gd.proj.latlon2xy(lat, lon, xx, yy);
    int size = _params.latest_click_mark_size;
    _draw_mark(gd.h_win.vis_xid, xx, yy, gd.legends.latest_client_mark_color, size);
  }
  
}

/////////////////////////////////////////////////////////////
// Draw a cross at the mark location

static void _draw_mark(Drawable xid, double xx, double yy,
                       const Color_gc_t *color, int size)

{

  int ix, iy;
  disp_proj_to_pixel(&(gd.h_win.margin), xx, yy, &ix, &iy);

  int halfSize = size / 2;
  if (halfSize < 2) {
    halfSize = 2;
  }

  XPoint bpt[7];

  bpt[0].x = ix;
  bpt[0].y = iy;
  
  bpt[1].x = ix + halfSize;
  bpt[1].y = iy;

  bpt[2].x = ix - halfSize;
  bpt[2].y = iy;

  bpt[3].x = ix;
  bpt[3].y = iy;
  
  bpt[4].x = ix;
  bpt[4].y = iy + halfSize;

  bpt[5].x = ix;
  bpt[5].y = iy - halfSize;

  bpt[6].x = ix;
  bpt[6].y = iy;
  
  XDrawLines(gd.dpy, xid, color->gc, bpt, 7, CoordModeOrigin);

}
