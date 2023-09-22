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
////////////////////////////////////////////////////////////////
// RenderContext.hh
//
// Object holding context for product rendering.
//
////////////////////////////////////////////////////////////////

#ifndef RenderContext_HH
#define RenderContext_HH

#include <rapplot/gplot.h>
#include <Spdb/Symprod.hh>
#include <Mdv/MdvxProj.hh>

class ProductMgr;
class Product;
class SymprodRender;
class SymprodRenderObj;
class SymprodRenderText;
class SymprodRenderPolyline;
class SymprodRenderIconline;
class SymprodRenderStrokedIcon;
class SymprodRenderNamedIcon;
class SymprodRenderBitmapIcon;
class SymprodRenderArc;
class SymprodRenderRectangle;
class SymprodRenderChunk;

///////////////////////////////////////////////////////////////
// class definition

class RenderContext

{

friend class ProductMgr;
friend class Product;
friend class SymprodRender;
friend class SymprodRenderObj;
friend class SymprodRenderText;
friend class SymprodRenderPolyline;
friend class SymprodRenderIconline;
friend class SymprodRenderStrokedIcon;
friend class SymprodRenderNamedIcon;
friend class SymprodRenderBitmapIcon;
friend class SymprodRenderArc;
friend class SymprodRenderRectangle;
friend class SymprodRenderChunk;

public:

  // default constructor
  
  RenderContext(Display *dpy, Drawable xid, GC gc,
		Colormap cmap, const MdvxProj &in_proj);
  

  void set_domain( double xmin, double xmax, double ymin, double ymax,
                    int width,int height, XFontStruct *fontst);
  
  void set_times(time_t ep_start, time_t ep_end,
                 time_t fr_start, time_t fr_end,
		 time_t dat_time);

  void set_clip_limits(double min_lat, double min_lon,
		       double max_lat, double max_lon);

  void set_vert_limits(double min_alt, double max_alt);


  void set_drawable(Drawable id) { xid = id;  }

  void set_draw_pick_boxes(bool st) { draw_pick_boxes = st; }

  void set_gc(GC g) { gc = g; xref.gc = g; }
  
  void set_offsets(int xoff, int yoff) { offset_x = xoff; offset_y = yoff; }

  void set_showing_hidden(bool a) { showing_hidden = a; }

  void set_hidden(bool show) { show_hidden = show; }

  void set_xid(Drawable x) {  xref.drawable = x; }

  void set_iconscale(double km_across_screen); 

  void set_scale_constant(double constant) { scaleConstant = constant; }

  // destructor
  virtual ~RenderContext();

  char last_foreground_color[SYMPROD_COLOR_LEN];
  char last_background_color[SYMPROD_COLOR_LEN];

  void reset_fgbg(void); // Clear lst colors.

  int offset_x, offset_y;  
protected:
  
  time_t epoch_start;
  time_t epoch_end;
  time_t frame_start;
  time_t frame_end;
  time_t data_time;

  // Clip Boundaries
  bool clip_limits_changed;
  double min_lat,max_lat;
  double min_lon,max_lon;

  bool vert_limits_changed;
  double min_alt,max_alt;

  bool draw_pick_boxes;

  bool show_hidden;
  
  bool showing_hidden;

private:

  const MdvxProj &proj;   // Display Projections

  Display *display;  // X11 related Members
  Drawable xid;
  Colormap cmap;
  GC gc;

  // Gplot structs
  gframe_t frame;
  xref_t   xref;
  psgc_t psgc;
  int dev;

  double iconScale;
  double scaleConstant;
};

#endif


