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

#include <QBrush>
#include <QPaintDevice>
#include <QPixmap>

#include <qtplot/ColorMap.hh>
#include <Spdb/Symprod.hh>
#include <Mdv/MdvxProj.hh>

#include "Constants.hh"

class ColorMap;
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

/*
 * text justification
 */

typedef enum {
  XJ_LEFT = 1,
  XJ_CENTER = 2,
  XJ_RIGHT = 3,
  YJ_ABOVE = 4,
  YJ_CENTER = 5,
  YJ_BELOW = 6
} test_just_t;

///////////////////////////////////////////////////////////////
// class definitions

class XrefObj {

public:
  
  XrefObj() {
    pdev = NULL;
    pixmap = NULL;
    width = height = 0;
    xscale = yscale = 1.0;
    QBrush brush;
    font = NULL;
  }
  
  QPaintDevice *pdev;
  QPixmap *pixmap;
  size_t width, height;
  double xscale, yscale;
  QBrush brush;
  QFont *font;

};

class GframeObj {

public:
  
  GframeObj() {
    w_xmin = w_ymin = -1000;
    w_xmax = w_ymax = 1000;
    x = new XrefObj;
  }
  
  ~GframeObj() {
    delete x;
  }

public:
  
  double w_xmin, w_ymin, w_xmax, w_ymax;
  XrefObj *x;
  
};

/*
 * structure for point in world coords
 */

typedef struct {
  double x, y;
} GPoint;

/*
 * structure for rectangle in world coords
 */

typedef struct {
  double x, y, width, height;
} GRectangle;

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
  
  RenderContext(QPaintDevice *pdev, QBrush brush,
		ColorMap cmap, const MdvxProj &in_proj);
  

  void set_domain( double xmin, double xmax, double ymin, double ymax,
                   int width,int height, QFont *font);
  
  void set_times(time_t ep_start, time_t ep_end,
                 time_t fr_start, time_t fr_end,
		 time_t dat_time);

  void set_clip_limits(double min_lat, double min_lon,
		       double max_lat, double max_lon);

  void set_vert_limits(double min_alt, double max_alt);


  void set_drawable(QPaintDevice *dev) { pdev = dev;  }

  void set_draw_pick_boxes(bool st) { draw_pick_boxes = st; }

  void set_brush(QBrush b) { brush = b; xref.brush = b; }
  
  void set_offsets(int xoff, int yoff) { offset_x = xoff; offset_y = yoff; }

  void set_showing_hidden(bool a) { showing_hidden = a; }

  void set_hidden(bool show) { show_hidden = show; }

  void set_xid(QPaintDevice *x) {  xref.pdev = x; }
  
  void set_iconscale(double km_across_screen); 
  
  void set_scale_constant(double constant) { scaleConstant = constant; }

  // destructor

  virtual ~RenderContext();

  char last_foreground_color[SYMPROD_COLOR_LEN];
  char last_background_color[SYMPROD_COLOR_LEN];

  void reset_fgbg(void); // Clear lst colors.

  int offset_x, offset_y;  

  static GframeObj *gCreateFrame(double w_xmin, 
                                 double w_ymin, 
                                 double w_xmax, 
                                 double w_ymax);

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

  // Display *display;  // X11 related Members
  ColorMap cmap;
  QPaintDevice *pdev;
  QBrush brush;

  // Gplot structs
  GframeObj frame;
  XrefObj   xref;
  int dev;

  double iconScale;
  double scaleConstant;
};

#endif


