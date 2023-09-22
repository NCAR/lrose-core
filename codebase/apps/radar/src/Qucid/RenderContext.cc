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
//////////////////////////////////////////////////////////
// RenderContext.cc
//
// Container object holding context for product rendering.
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "RenderContext.hh"

//////////////////////////////////////////
// DEFAULT CONSTRUCTOR

RenderContext::RenderContext(Display *dpy, Drawable xid, GC gc,
                             Colormap cmap, const MdvxProj &in_proj)
                             : clip_limits_changed(false),
			       vert_limits_changed(false),
			       proj(in_proj), display(dpy),
			       cmap(cmap),
			       gc(gc)
{
  
  iconScale = 1.0;

  show_hidden = false;
  draw_pick_boxes = false;

  dev = XDEV;  // X windows 

  *last_foreground_color = '\0';
  *last_background_color = '\0';

  frame.ps = NULL; 
  frame.psgc = NULL;

  min_lat = -90.0;
  max_lat = 90.0;
  min_lon = -180.0;
 max_lon = 180.0;
  min_alt = 0.0;
  max_alt = 100000.0;

  offset_x = 0;
  offset_y = 0;

  xref.display = dpy;
  xref.drawable = xid;
}

//////////////
// DESTRUCTOR
  
RenderContext::~RenderContext()
{
}

////////////////////////////////////////////////////////////////////////
// SET_CLIP_LIMITS
//

void RenderContext::set_clip_limits(double minlat, double minlon,
				    double maxlat, double maxlon)
{
    clip_limits_changed = true;
  
    min_lat = minlat;
    min_lon = minlon;

    max_lat = maxlat;
    max_lon = maxlon;
}

////////////////////////////////////////////////////////////////////////
// SET_VERT_LIMITS
//

void RenderContext::set_vert_limits(double minalt, double maxalt)
{
  vert_limits_changed = true;
  
  min_alt = minalt;
  max_alt = maxalt;
}

////////////////////////////////////////////////////////////////////////
// SET_TIMES
//

void RenderContext::set_times(time_t ep_start, time_t ep_end, 
                         time_t fr_start, time_t fr_end,
			 time_t dat_time)
{
    epoch_start = ep_start;
    epoch_end = ep_end;
    frame_start = fr_start;
    frame_end = fr_end;
    data_time = dat_time;
}


////////////////////////////////////////////////////////////////////////
// RESET_FGBG: Reset last Foreground and Background colors.
//

void RenderContext::reset_fgbg()
{
  *last_foreground_color = '\0';
  *last_background_color = '\0';
}

////////////////////////////////////////////////////////////////////////
// SET_ICONSCALE  Set a Icon scaling factor.
//     Icons are 1.0 at when the distance acroos the screen is equal to
//     km_across_screen 
//

void RenderContext::set_iconscale(double km_across_screen)
{
  if (scaleConstant <= 0) {
    iconScale = 1.0;
  } else {
    iconScale = log10(scaleConstant / km_across_screen) + 1.0;
    if (iconScale < 0.05) iconScale = 0.0;
  }
}

////////////////////////////////////////////////////////////////////////
// SET_DOMAIN 
//

void RenderContext::set_domain( double xmin, double xmax, double ymin, double ymax,
                           int width,int height, XFontStruct *fontst)
{
    frame.w_xmin = xmin;
    frame.w_xmax = xmax;
    frame.w_ymin = ymin;
    frame.w_ymax = ymax;

    frame.x = &xref;

    xref.width = width;
    xref.height = height;

    xref.xscale = (double)xref.width / (frame.w_xmax - frame.w_xmin);
    xref.yscale = (double)xref.height / (frame.w_ymax - frame.w_ymin); 


    xref.gc = gc; 

    xref.font = fontst;
}

