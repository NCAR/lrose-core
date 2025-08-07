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
// SymprodRenderObj.cc
//
// Symprod rendering objects
//
// These classes provide handling routines for the various
// types of symprod objects used for rendering.
//
// Jan 2000
//////////////////////////////////////////////////////////

#include "SymprodRenderObj.hh"
#include "RenderContext.hh"
#include "Product.hh"
#include <Spdb/Symprod.hh>
#include <Spdb/Product_defines.hh>
#include <vector>
#include <toolsa/str.h>
#include <toolsa/pjg.h>     // Map projection geometry

void set_pick_box_xy(int x, int y, QPoint *bpt);
//
// initialize static variables

const int SymprodRenderObj::_stipple_bitmap_width = 10;
const int SymprodRenderObj::_stipple_bitmap_height = 10;
const int SymprodRenderObj::_blankPixelSpacing = 10;

const unsigned char SymprodRenderObj::_stipple10_bitmap_bits[] =
  {  0x82, 0x00, 0x04, 0x10, 0x40,
     0x00, 0x01, 0x10, 0x02, 0x04,
     0x20, 0x00, 0x00 
  };

const unsigned char SymprodRenderObj::_stipple20_bitmap_bits[] =
  {  0x92, 0x14, 0x04, 0x90, 0x41,
     0x40, 0x81, 0x12, 0x02, 0x04,
     0x20, 0x42, 0x00 
  };

const unsigned char SymprodRenderObj::_stipple30_bitmap_bits[] =
  {  0x92, 0x15, 0x14, 0x98, 0x41,
     0x42, 0x85, 0x52, 0x82, 0x44,
     0x24, 0x42, 0x20 
  };

const unsigned char SymprodRenderObj::_stipple40_bitmap_bits[] =
  {  0x92, 0x95, 0x54, 0x9a, 0x49,
     0x4a, 0xa5, 0x52, 0xa2, 0x45,
     0x24, 0x62, 0xa0 
  };

const unsigned char SymprodRenderObj::_stipple50_bitmap_bits[] =
  {  0xba, 0x95, 0x54, 0xda, 0xc9,
     0xca, 0xa5, 0x56, 0xaa, 0x4d,
     0x64, 0x66, 0xa0 
  };

const unsigned char SymprodRenderObj::_stipple60_bitmap_bits[] =
  {  0xba, 0xd5, 0x5d, 0xda, 0xcd,
     0xda, 0xf5, 0x56, 0xaa, 0x5f,
     0x64, 0x6e, 0xa0 
  };

const unsigned char SymprodRenderObj::_stipple70_bitmap_bits[] =
  {  0xbb, 0xf5, 0x5f, 0xde, 0xbb,
     0xba, 0xf5, 0x77, 0xab, 0x5f,
     0x65, 0x6e, 0xb0 
  };

const unsigned char SymprodRenderObj::_stipple80_bitmap_bits[] =
  {  0xfb, 0xf7, 0x7f, 0xde, 0xdd,
     0xdb, 0xff, 0x77, 0xeb, 0x5f,
     0x6d, 0x7f, 0xb0 
  };

const unsigned char SymprodRenderObj::_stipple90_bitmap_bits[] =
  {  0xff, 0xf7, 0x7f, 0xff, 0xff,
     0xfb, 0xff, 0x7f, 0xeb, 0x7f,
     0x6f, 0xff, 0xb0 
  };



//////////////////////////////////////////////////////////////////////////////
// Abstract base class

SymprodRenderObj::SymprodRenderObj(Params &params,
                                   SymprodRender *c) :
        _params(params),
        _gd(GlobalData::Instance())
{

  container = c;
  _background_pixel = 0;
  _background_pixel_init = false;

}

SymprodRenderObj::~SymprodRenderObj()
{
}

void SymprodRenderObj::draw(RenderContext &context)
{
  fprintf(stderr,"SymprodRenderObj:: Should not Get Here - Draw\n");
}

double SymprodRenderObj::dist(double lat, double lon)
{
  fprintf(stderr,"SymprodRenderObj::Should not Get Here - Dist\n");
  return 999999.0;
}

void SymprodRenderObj::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{ 
  fprintf(stderr,"SymprodRenderObj::Should not Get Here - Fill\n");
}

//////////////////////////////////////////////////////////////////////////////
// Text objects

SymprodRenderText::SymprodRenderText(Params &params,
                                     const void *prod_buffer,
				     const int obj_offset,
				     Product &prod, SymprodRender *c)
        : SymprodRenderObj(params, c),
          SymprodText(prod_buffer, obj_offset),
          prod(prod)
  
{
  objtype = Symprod::OBJ_TEXT;
  
}

void SymprodRenderText::draw(RenderContext &context)
{
  // Normally, do not render hidden objects
  if(_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN) {
    if(context.show_hidden ) { 
      if(context.showing_hidden) {
        return;  // Bail out - one already showing.
      } else {
        context.showing_hidden = true;
      }
    } else {
      return; // Bail out - Not showing productrs.
    } 
  }

  // Don't show text if the image scale is too large.
  if (!(_hdr.detail_level & Symprod::DETAIL_LEVEL_IGNORE_TRESHOLDS) &&
      context.iconScale < prod._prodInfo.text_off_threshold) {
    return;
  }

  // double iconScale = context.iconScale;
  // if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE ) {
  //   iconScale = 1.0;
  // }

  // set the colors

  // if (context.dev == XDEV) {
  _setXColors(_hdr.color, _hdr.background_color,context);
  // }

#ifdef NOTYET
  int x_just = XJ_LEFT;
  int y_just = YJ_BELOW;
  
  // Convert the justification values.

  switch(_props.horiz_alignment) {
    case Symprod::HORIZ_ALIGN_LEFT :
      x_just = XJ_LEFT;
      break;
    case Symprod::HORIZ_ALIGN_CENTER :
      x_just = XJ_CENTER;
      break;
    case Symprod::HORIZ_ALIGN_RIGHT :
      x_just = XJ_RIGHT;
      break;
  }
  
  switch(_props.vert_alignment) {
    case Symprod::VERT_ALIGN_TOP :
      y_just = YJ_BELOW;
      break;
    case Symprod::VERT_ALIGN_CENTER :
      y_just = YJ_CENTER;
      break;
    case Symprod::VERT_ALIGN_BOTTOM :
      y_just = YJ_ABOVE;
      break;
  }
#endif
  
  // Draw the text where indicated

#ifdef DEBUG
  fprintf(stderr, "Drawing label \"%s\" at lat = %f, lon = %f\n",
	  _props.text, _props.origin.lat, _props.origin.lon);
#endif
  
  double x_world;
  double y_world;
  
  _latLon2World(_props.origin.lat, _props.origin.lon,
		x_world, y_world,context);

  x_world += context.offset_x / context.frame.x->xscale;
  y_world -= context.offset_y / context.frame.x->yscale;

#ifdef NOTYET
  int offset_x = _props.offset.x;
  int offset_y = _props.offset.y;

  QFont *propsFont = XLoadQueryFont(context.frame.x->display,
                                    _props.fontname);
  QFont *font;
  if (propsFont == NULL) {
    font = context.frame.x->font;
  } else {
    font = propsFont;
    XSetFont(context.frame.x->display, context.gc, font->fid);
  }

  // Allocate a buffer for text manipulation.
  char *buf_ptr;
  if((buf_ptr = (char *) calloc(_text.size() +1,1)) == NULL) {
    perror("SymprodRenderText::draw Calloc failure");
    return;
  }
  strncpy(buf_ptr,_text.c_str(),_text.size());
  char *ptr = buf_ptr;

  char *tptr; 
  char *end_ptr = ptr + _text.size();
  int  y_line_offset = 0;

  int direct, ascent, descent;
  XCharStruct overall;
  XTextExtents(context.frame.x->font, ptr, _text.size(),
               &direct, &ascent, &descent, &overall);
    
  // Skip down 1.2 font height units
  int  y_line_delta =  lrint((overall.ascent + overall.descent) * 1.2);
  
  // Deal with Multi Line text products.
  while (ptr < end_ptr) {
    if((tptr = strchr(ptr,'\012')) != NULL) *tptr = '\000'; // Null terminate.

    if (strlen(_hdr.background_color) > 0) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
                             font, &context.psgc,
                             x_just, y_just, x_world, y_world,
                             offset_x, offset_y - y_line_offset,
                             ptr);

    } else {
      GDrawStringOffset(context.dev, &context.frame, context.gc,
                        font, &context.psgc,
                        x_just, y_just, x_world, y_world,
                        offset_x, offset_y - y_line_offset,
                        ptr);
      
    }
    // Move pointer over the previous line.
    if(tptr != NULL) { ptr = tptr +1; } else { ptr = end_ptr;}
    
    y_line_offset += y_line_delta; // move down on the screen.
  }

  if(buf_ptr) free(buf_ptr);

  if (propsFont != NULL) {
    XFreeFont(context.frame.x->display, propsFont);
  }
#endif
  
}

double SymprodRenderText::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.

  if (( _gd.r_context == NULL ) || ( _gd.r_context->frame.x == NULL )){
    // Just not ready yet - still in the rendering process. Niles Oien June 2010.
    return d;
  }

  double theta = 0.0;

  double lat2,lon2,dx,dy;

  dx = _props.offset.x / _gd.r_context->frame.x->xscale;
  dy = _props.offset.y / _gd.r_context->frame.x->yscale;

  // Shift the point closer to the actual text location.
  PJGLatLonPlusDxDy(_props.origin.lat,_props.origin.lon,dx,dy,&lat2,&lon2);

  // Get the distance from the shifted point 
  PJGLatLon2RTheta(lat2,lon2,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderText::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  label =   _text;
}

//////////////////////////////////////////////////////////////////////////////
// Polyline objects

SymprodRenderPolyline::
  SymprodRenderPolyline(Params &params,
                        const void *prod_buffer,
                        const int obj_offset,
                        Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodPolyline(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_POLYLINE;
}

void SymprodRenderPolyline::draw(RenderContext &context)

{

#ifdef NOTYET
  // set the colors

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  // find start and end points
  
  int istart = -1;
  int iend = -1;
  
  for (int i = 0; i < _props.num_points; i++)  {
    if (_points[i].lat != Symprod::WPT_PENUP &&
	_points[i].lon != Symprod::WPT_PENUP) {
      istart = i;
      break;
    }
  }
  for (int i = _props.num_points - 1; i >= 0; i--)  {
    if (_points[i].lat != Symprod::WPT_PENUP &&
	_points[i].lon != Symprod::WPT_PENUP) {
      iend = i;
      break;
    }
  }

  if (istart == iend) {
    // need at least 2 non-penup points for a line
    return;
  }

  // set the line attributes.
  
  if (context.dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(context.display, context.gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
  }

  // copy the points

  vector<Symprod::wpt_t> points;
  points.resize(_props.num_points);
  memcpy(points.data(), _points, _props.num_points * sizeof(Symprod::wpt_t));

  // adjust the lon range of the latlon points as applicable

  if (context.proj.getProjType() == Mdvx::PROJ_LATLON) {
    
    // count up how many points would plot in the window unchanged,
    // and how many would plot in the window if we adjust their
    // lon by either -360 or + 360

    int nSubtract = 0, nNoChange = 0, nAdd = 0;
    for (int ii = 0; ii < _props.num_points; ii++) {
      if (points[ii].lon != Symprod::WPT_PENUP) {
	if (points[ii].lon >= context.frame.w_xmin &&
	    points[ii].lon <= context.frame.w_xmax) {
	  nNoChange++;
	} else if (points[ii].lon - 360.0 >= context.frame.w_xmin &&
		   points[ii].lon - 360.0 <= context.frame.w_xmax) {
	  nSubtract++;
	} else if (points[ii].lon + 360.0 >= context.frame.w_xmin &&
		   points[ii].lon + 360.0 <= context.frame.w_xmax) {
	  nAdd++;
	}
      }
    } // ii

    if (nSubtract > nNoChange && nSubtract > nAdd) {
      for (int ii = 0; ii < _props.num_points; ii++) {
	points[ii].lon -= 360.0;
      }
    } else if (nAdd > nNoChange && nAdd > nSubtract) {
      for (int ii = 0; ii < _props.num_points; ii++) {
	points[ii].lon += 360.0;
      }
    }

  } // if (context.proj.getProjType() == Mdvx::PROJ_LATLON)

  // loop through the points, rendering each time we get a penup
  
  MemBuf gptsBuf;
  
  for (int ii = 0; ii < _props.num_points; ii++)  {
    
    bool penup = false;
    
    if (points[ii].lat != Symprod::WPT_PENUP &&
	points[ii].lon != Symprod::WPT_PENUP) {
      if (ii != _props.num_points - 1 &&
	  points[ii + 1].lon != Symprod::WPT_PENUP &&
	  fabs(points[ii + 1].lon - points[ii].lon) > 180.0) {
	// folding across lon boundaries
	penup = true;
      } else {
	// accept this point
	GPoint gpt;
	context.proj.latlon2xy(points[ii].lat, points[ii].lon, gpt.x, gpt.y);


	gpt.x += context.offset_x / context.frame.x->xscale;
	gpt.y -= context.offset_y / context.frame.x->yscale;

	gptsBuf.add(&gpt, sizeof(gpt));
	penup = false;
      }
    } else {
      penup = true;
    }
    
    // render and reset as appropriate
    
    if (penup || ii == _props.num_points - 1) {
      
      if (context.dev == XDEV) {
	XSetFillStyle(context.display, context.gc, FillSolid);
      }
      
      GPoint *gpts = (GPoint *) gptsBuf.getPtr();

      int npts = gptsBuf.getLen() /  sizeof(GPoint);
      GDrawLines(context.dev, &context.frame,
		 context.gc, &context.psgc, gpts, npts, CoordModeOrigin);

      if (_props.close_flag && _props.fill != Symprod::FILL_NONE) {
	if (context.dev == XDEV &&
	    _props.fill >= Symprod::FILL_STIPPLE10 &&
	    _props.fill <= Symprod::FILL_STIPPLE90)  {
	  setStipple((Symprod::fill_t) _props.fill, context);
	  XSetFillStyle(context.display, context.gc, FillStippled);
	}
	GFillPolygon(context.dev, &context.frame,
		     context.gc, &context.psgc, gpts, npts, CoordModeOrigin);
	if (context.dev == XDEV &&
	    _props.fill >= Symprod::FILL_STIPPLE10 &&
	    _props.fill <= Symprod::FILL_STIPPLE90)  {
	  XSetFillStyle(context.display, context.gc, FillSolid);
	}
      }

      gptsBuf.free();
      
    } // if (penup ...

  } // ii

  // close as required
  
  if (_props.close_flag) {
    GPoint gpt_start, gpt_end;

    context.proj.latlon2xy(points[0].lat, points[0].lon,
			   gpt_start.x, gpt_start.y);

    gpt_start.x += context.offset_x / context.frame.x->xscale;
    gpt_start.y -= context.offset_y / context.frame.x->yscale;

    context.proj.latlon2xy(points[_props.num_points - 1].lat,
			   points[_props.num_points - 1].lon,
			   gpt_end.x, gpt_end.y);

    gpt_end.x += context.offset_x / context.frame.x->xscale;
    gpt_end.y -= context.offset_y / context.frame.x->yscale;
	
    GDrawLine(context.dev, &context.frame,
	      context.gc, &context.psgc,
	      gpt_end.x, gpt_end.y, gpt_start.x, gpt_start.y);
  }

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Use a point close to the center of the polyline
    int index =  _props.num_points / 2;

    // Make sure the point is not a pen up.
    while (_points[index].lon == Symprod::WPT_PENUP && index > 0) {
      index--;
    }
    // Determine the screen coordinates
    context.proj.latlon2xy(points[index].lat,points[index].lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif
}

double SymprodRenderPolyline::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  // Use a point close to the center of the polyline
  int index =  _props.num_points / 2;

  // Make sure the point is not a pen up.
  while (_points[index].lon == Symprod::WPT_PENUP && index > 0) {
    index--;
  }

  PJGLatLon2RTheta(_points[index].lat,_points[index].lon,lat,lon,&d,&theta);


  return d;
}

void SymprodRenderPolyline::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int i = 0; i < _props.num_points; i++)  {
    w.lon = _points[i].lon;
    w.lat = _points[i].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// Iconline objects

SymprodRenderIconline::
  SymprodRenderIconline(Params &params,
                        const void *prod_buffer,
                        const int obj_offset,
                        Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodIconline(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_ICONLINE;
}

void SymprodRenderIconline::draw(RenderContext &context)

{

#ifdef NOTYET
  
  double iconScale = context.iconScale;
  // Get the centroid in x and y
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y, context);

  // create a wpt_t array of points, computing the world coords by
  // taking the origin and scaling the offsets accordingly

  double penup = -9999.0;

  vector<GPoint> gpts;
  gpts.resize(_props.num_points);

  for (int i = 0; i < _props.num_points; i++) {
    if (_points[i].x == Symprod::PPT_PENUP &&
	_points[i].y == Symprod::PPT_PENUP) {
      gpts[i].x = penup;
      gpts[i].y = penup;
    } else {
      if (context.dev == XDEV) {
        if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
          gpts[i].x = origin_x + _points[i].x / context.frame.x->xscale;
          gpts[i].y = origin_y + _points[i].y / context.frame.x->yscale;
        } else {
          gpts[i].x = origin_x + _points[i].x * (iconScale / context.frame.x->xscale);
          gpts[i].y = origin_y + _points[i].y * (iconScale / context.frame.x->yscale);
        }
      } else {
        if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
          gpts[i].x = origin_x + _points[i].x / context.frame.ps->xscale;
          gpts[i].y = origin_y + _points[i].y / context.frame.ps->yscale;
        } else {
          gpts[i].x = origin_x + _points[i].x * (iconScale / context.frame.ps->xscale);
          gpts[i].y = origin_y + _points[i].y * (iconScale / context.frame.ps->yscale);
        }
      }
    }

    gpts[i].x +=  context.offset_x / context.frame.x->xscale;
    gpts[i].y -=  context.offset_y / context.frame.x->yscale;

  }

  // set the colors

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }
  
  // find start and end points
  
  int istart = -1;
  int iend = -1;
  
  for (int i = 0; i < _props.num_points; i++)  {
    if (gpts[i].x != penup &&
	gpts[i].y != penup) {
      istart = i;
      break;
    }
  }
  for (int i = _props.num_points - 1; i >= 0; i--)  {
    if (gpts[i].x != penup &&
	gpts[i].y != penup) {
      iend = i;
      break;
    }
  }

  if (istart == iend) {
    // need at least 2 non-penup points for a line
    return;
  }

  // set the line attributes.
  
  if (context.dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(context.display, context.gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
  }
  
  // loop through the points, rendering each time we get a penup

  int start = 0;
  int count = 0;
  
  for (int ii = 0; ii < _props.num_points; ii++)  {
    
    if (gpts[ii].x != penup && gpts[ii].y != penup) {
      penup = false;
      count++;
    } else {
      penup = true;
    }
    
    // render and reset as appropriate
    
    if (penup || ii == _props.num_points - 1) {

      if (count >= 2) {
	if (context.dev == XDEV) {
	  XSetFillStyle(context.display, context.gc, FillSolid);
	}
      
	GDrawLines(context.dev, &context.frame,
		   context.gc, &context.psgc,
		   gpts.data() + start, count,
		   CoordModeOrigin);
      
	if (_props.close_flag &&
	    _props.fill != Symprod::FILL_NONE) {
	  if (context.dev == XDEV &&
	      _props.fill >= Symprod::FILL_STIPPLE10 &&
	      _props.fill <= Symprod::FILL_STIPPLE90)  {
	    setStipple((Symprod::fill_t) _props.fill, context);
	    XSetFillStyle(context.display, context.gc, FillStippled);
	  }
	  GFillPolygon(context.dev, &context.frame,
		       context.gc, &context.psgc,
		       gpts.data() + start, count,
		       CoordModeOrigin);
	  if (context.dev == XDEV &&
	      _props.fill >= Symprod::FILL_STIPPLE10 &&
	      _props.fill <= Symprod::FILL_STIPPLE90)  {
	    XSetFillStyle(context.display, context.gc, FillSolid);
	  }
	}
      } // if (count >= 2) 

      start = ii + 1;
      count = 0;

    } // if (penup ...

  } // ii

  // close as required
  
  if (_props.close_flag) {
    GDrawLine(context.dev, &context.frame,
	      context.gc, &context.psgc,
	      gpts[iend].x, gpts[iend].y,
	      gpts[istart].x, gpts[istart].y);
  }

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Determine the screen coordinates
    context.proj.latlon2xy(_props.origin.lat, _props.origin.lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif

}

double SymprodRenderIconline::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}


void SymprodRenderIconline::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// StrokedIcon objects

SymprodRenderStrokedIcon::
  SymprodRenderStrokedIcon(Params &params,
                           const void *prod_buffer,
                           const int obj_offset,
                           Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodStrokedIcon(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_STROKED_ICON;
}

#define ICON_BUF_SIZE 32

void SymprodRenderStrokedIcon::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }
	  
  double iconScale = context.iconScale;
  if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
    iconScale = 1.0;
  }

  double x_start, y_start;
  double origin_x, origin_y;
  int    pix_x, pix_y;
  int    npoints;

  static XPoint    *bpt;
  static size_t buf_size = 0;

  if (_props.num_icons < 1) {
    return;
  }

  // set the colors & line width

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
    XSetLineAttributes(context.display, context.gc,
		       _props.linewidth,
		       LineSolid,CapButt,JoinRound);
  }

  // First time through - grab a small buffer
  if( buf_size == 0) {
    if((bpt = (XPoint *) calloc(ICON_BUF_SIZE,sizeof(XPoint))) == NULL) {
      perror("Malloc Error in SymprodRenderStrokedIcon::draw"); 
      exit(-1);
    }
    buf_size = ICON_BUF_SIZE;
  }
  
  // initialize the world conversion
  _latLon2World(_iconOrigins[0].lat, _iconOrigins[0].lon,
		x_start, y_start,context);

  // Draw the icon at each of the given positions.
  for (int ii = 0; ii < _props.num_icons; ii++) {
      
    // worse case check - one big line
    if( _props.num_icon_pts  >= (int) buf_size )   {

      // double the size until big enough
      while(_props.num_icon_pts >= (int) buf_size) buf_size *= 2;

      // Reallocate the bufer
      if((bpt = (XPoint *) realloc(bpt,buf_size * sizeof(XPoint))) == NULL) { 
        perror("Realloc Error in SymprodRenderStrokedIcon::draw");
        exit(-1);
      }
    }

#ifdef DEBUG    
    fprintf(stderr,
	    "Creating stroked icon at lat = %f, lon = %f\n",
	    _iconOrigins[ii].lat, _iconOrigins[ii].lon);
#endif
    
    // Determine the world coordinates for the icon.
    _latLon2WorldConstrained(_iconOrigins[ii].lat, _iconOrigins[ii].lon,
			     x_start, origin_x, origin_y,context);
    

    // Determine the screen coordinates
    pix_x = GXWindowx(&context.frame, origin_x);
    pix_y = GXWindowy(&context.frame, origin_y);

    // Draw each line segment in the icon. 
    npoints = 0;
    for (int ipt = 0; ipt < _props.num_icon_pts; ipt++) {
      if (_iconPts[ipt].x == Symprod::PPT_PENUP || _iconPts[ipt].y == Symprod::PPT_PENUP ) {
        XDrawLines(context.display,context.xid,context.gc,bpt,npoints,CoordModeOrigin);
        npoints = 0;
      } else { // Add point to the buffer 

        if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
          bpt[npoints].x =  _iconPts[ipt].x + pix_x;
          bpt[npoints].y = pix_y - _iconPts[ipt].y;
        } else {
          bpt[npoints].x = (short) floor(iconScale * _iconPts[ipt].x + 0.5) + pix_x;
          bpt[npoints].y = pix_y - (short) floor(iconScale * _iconPts[ipt].y + 0.5);
        }
	npoints++;
      }
    }// ipt
    // Render icon without penup at the end
    if(npoints) XDrawLines(context.display,context.xid,context.gc,bpt,npoints,CoordModeOrigin);
    
  } // ii

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Determine the screen coordinates
    context.proj.latlon2xy(_iconOrigins[0].lat, _iconOrigins[0].lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif

}

double SymprodRenderStrokedIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  if( _iconOrigins != NULL) PJGLatLon2RTheta(_iconOrigins[0].lat, _iconOrigins[0].lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderStrokedIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int ipt = 0; ipt < _props.num_icon_pts; ipt++) {
    w.lon = _iconOrigins[ipt].lon;
    w.lat = _iconOrigins[ipt].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}

//////////////////////////////////////////////////////////////////////////////
// NamedIcon objects

SymprodRenderNamedIcon::
  SymprodRenderNamedIcon(Params &params,
                         const void *prod_buffer,
                         const int obj_offset,
                         Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodNamedIcon(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_NAMED_ICON;
  
}

void SymprodRenderNamedIcon::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }

  // set the colors
  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  cerr << "ERROR - SymprodRenderNamedIcon::draw()" << endl;
  cerr << "  Not yet implemented." << endl;

#endif

}

double SymprodRenderNamedIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.

  // Not Implemented
  //PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderNamedIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  // Do nothing. - Not implemented.
}

//////////////////////////////////////////////////////////////////////////////
// BitmapIcon objects

SymprodRenderBitmapIcon::
  SymprodRenderBitmapIcon(Params &params,
                          const void *prod_buffer,
                          const int obj_offset,
                          Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodBitmapIcon(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_BITMAP_ICON;
  
}

void SymprodRenderBitmapIcon::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }
	  
  double iconScale = context.iconScale;
  if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
    iconScale = 1.0;
  }

  // set the colors
  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  // create array of active points

  MemBuf offsetBuf;
  int size = _props.bitmap_x_dim * _props.bitmap_y_dim;
  
  for (int ii = 0; ii < size; ii++) {
    if (_bitmap[ii])  {
      XPoint offset;
      offset.x = -(_props.bitmap_x_dim / 2 - ii % _props.bitmap_x_dim);
      offset.x = (int) floor(offset.x * iconScale + 0.5);
      offset.y = _props.bitmap_y_dim / 2 - ii / _props.bitmap_x_dim;
      offset.y = (int) floor(offset.y * iconScale + 0.5);
      offsetBuf.add(&offset, sizeof(offset));
    }
  }
  
  // initialize the world conversion
  
  double x_start, y_start;
  _latLon2World(_iconOrigins[0].lat, _iconOrigins[0].lon,
		x_start, y_start,context);

  // Draw the icon at each of the given centroids

  for (int ii = 0; ii < _props.num_icons; ii++) {

    // Determine the world coordinates for the icon.
    
    double origin_x, origin_y;
    _latLon2WorldConstrained(_iconOrigins[ii].lat, _iconOrigins[ii].lon,
			     x_start, origin_x, origin_y,context);
    
    // Now draw the icon.

    int nOffsets = offsetBuf.getLen() / sizeof(XPoint);
    GDrawPoints(context.dev, &context.frame,
		context.gc, &context.psgc,	origin_x, origin_y,
		(XPoint *) offsetBuf.getPtr(), nOffsets);
    
  } // ii

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Determine the screen coordinates
    context.proj.latlon2xy(_iconOrigins[0].lat, _iconOrigins[0].lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif
  
}


double SymprodRenderBitmapIcon::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  // Measure from the first Icon.
  PJGLatLon2RTheta(_iconOrigins[0].lat, _iconOrigins[0].lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderBitmapIcon::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  for (int ipt = 0; ipt < _props.num_icons; ipt++) {
    w.lon = _iconOrigins[ipt].lon;
    w.lat = _iconOrigins[ipt].lat;
    wpt.push_back(w);
  }
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}


//////////////////////////////////////////////////////////////////////////////
// Arc objects

SymprodRenderArc::
  SymprodRenderArc(Params &params,
                   const void *prod_buffer,
                   const int obj_offset,
                   Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodArc(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_ARC;
}

void SymprodRenderArc::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }

  double iconScale = context.iconScale;
  if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
    iconScale = 1.0;
  }

  // set the colors

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  // set the line attributes.
  
  if (context.dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(context.display, context.gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
    if (_props.fill >= Symprod::FILL_STIPPLE10 &&
	_props.fill <= Symprod::FILL_STIPPLE90)  {
      setStipple((Symprod::fill_t) _props.fill, context);
    }
  }

  // Get the centroid in x and y
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y,context);

  double radius_x = _props.radius_x;
  double radius_y = _props.radius_y;

  if (_props.radii_in_pixels) {

    // if the radii are in pixels, convert to km before calling the
    // rendering routines
    
    if (context.dev == XDEV) {
      radius_x =
	_props.radius_x * (iconScale / context.frame.x->xscale);
      radius_y =
	_props.radius_y * (iconScale / context.frame.x->yscale);
    } else {
      radius_x =
	_props.radius_x * (iconScale / context.frame.ps->xscale);
      radius_y =
	_props.radius_y * (iconScale / context.frame.ps->yscale);
    }

  } else if (context.proj.getProjType() == Mdvx::PROJ_LATLON) {

    // for lat/lon projection, convert radii to degrees

    double lat = origin_y;
    double cosLat = cos(lat * DEG_TO_RAD);
    
    radius_y /= KM_PER_DEG_AT_EQ;
    radius_x /= (KM_PER_DEG_AT_EQ * cosLat);

  }
  
  // draw the arc
  
  if (_props.fill == Symprod::FILL_SOLID) {
    GFillArc(context.dev, &context.frame, context.gc, &context.psgc,
	     origin_x, origin_y,
	     radius_x, radius_y,
	     _props.angle1, _props.angle2,
	     _props.axis_rotation, _props.nsegments);
  } else {
    GDrawArc(context.dev, &context.frame, context.gc, &context.psgc,
	     origin_x, origin_y,
	     radius_x, radius_y,
	     _props.angle1, _props.angle2,
	     _props.axis_rotation, _props.nsegments);
  }
  
  if (context.dev == XDEV &&
      _props.fill >= Symprod::FILL_STIPPLE10 &&
      _props.fill <= Symprod::FILL_STIPPLE90)  {
    XSetFillStyle(context.display, context.gc, FillSolid);
  }

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Determine the screen coordinates
    context.proj.latlon2xy(_props.origin.lat, _props.origin.lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif

}


double SymprodRenderArc::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderArc::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}


//////////////////////////////////////////////////////////////////////////////
// Rectangle objects

SymprodRenderRectangle::
  SymprodRenderRectangle(Params &params,
                         const void *prod_buffer,
                         const int obj_offset,
                         Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodRectangle(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_RECTANGLE;
}

void SymprodRenderRectangle::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }

  // set the colors

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  // set the line attributes.
  
  if (context.dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(context.display, context.gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
    if (_props.fill >= Symprod::FILL_STIPPLE10 &&
	_props.fill <= Symprod::FILL_STIPPLE90)  {
      setStipple((Symprod::fill_t) _props.fill, context);
    }
  }

  // Get the centroid
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y,context);
    
  // draw the rectangle
  
  if (_props.fill == Symprod::FILL_SOLID) {
    GFillRectangle(context.dev, &context.frame, context.gc, &context.psgc,
		   origin_x, origin_y,
		   _props.width, _props.height);
  } else {
    GDrawRectangle(context.dev, &context.frame, context.gc, &context.psgc,
		   origin_x, origin_y,
		   _props.width, _props.height);
  }
  
  if (context.dev == XDEV &&
      _props.fill >= Symprod::FILL_STIPPLE10 &&
      _props.fill <= Symprod::FILL_STIPPLE90)  {
    XSetFillStyle(context.display, context.gc, FillSolid);
  }

  if(context.draw_pick_boxes) {
    int pix_x, pix_y;
    double km_x,km_y;
    XPoint    bpt[8];

    // Determine the screen coordinates
    context.proj.latlon2xy(_props.origin.lat, _props.origin.lon,km_x,km_y);
    disp_proj_to_pixel(&(_gd.h_win.margin),km_x,km_y,&pix_x,&pix_y);

    set_pick_box_xy(pix_x + context.offset_x,pix_y + context.offset_y, bpt);
	
    XDrawLines(context.display,context.xid,context.gc,bpt,8,CoordModeOrigin);
  }

#endif

}

double SymprodRenderRectangle::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.
  double theta = 0.0;

  PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}

void SymprodRenderRectangle::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  world_pt_t w;

  w.lon = _props.origin.lon;
  w.lat = _props.origin.lat;
  wpt.push_back(w);
  w.lat = -9999.9;
  w.lon = -9999.9;
  wpt.push_back(w);
}



//////////////////////////////////////////////////////////////////////////////
// Chunk objects

SymprodRenderChunk::
  SymprodRenderChunk(Params &params,
                     const void *prod_buffer,
                     const int obj_offset,
                     Product &prod, SymprodRender *c)
          : SymprodRenderObj(params, c),
            SymprodChunk(prod_buffer, obj_offset),
            prod(prod)
  
{
  objtype = Symprod::OBJ_CHUNK;
  _barbShaftLen = 0;
}

void SymprodRenderChunk::draw(RenderContext &context)

{

#ifdef NOTYET
  
  // Normally, do not render hidden objects
  if(!context.show_hidden &&
     (_hdr.detail_level & Symprod::DETAIL_LEVEL_USUALLY_HIDDEN)) {
    return;
  }

  // set the colors

  if (context.dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color,context);
  }

  // Render the chunk based on its type.

  switch (_props.chunk_type)  {
  
    case SPDB_STATION_REPORT_ID :
      _drawStationSingle(context);
      break;
    
    case SPDB_STATION_REPORT_ARRAY_ID :
      _drawStationsFromArray(context);
      break;
    
    default :
      cerr << "WARNING:  SymprodRenderChunk::draw." << endl;
      cerr << "  Don't know how to render chunk type: "
           << _props.chunk_type << endl;
      cerr << "  Skipping ...." << endl;
      break;
    
  } /* endswitch - _props.chunk_type */

#endif

}

double SymprodRenderChunk::dist(double lat, double lon)
{
  double d = 999999.99; // A huge distance by default.

  // Not Implemented.
  //PJGLatLon2RTheta(_props.origin.lat,_props.origin.lon,lat,lon,&d,&theta);

  return d;
}


void SymprodRenderChunk::fill_export_fields( vector<world_pt_t> &wpt, string &label)
{
  // Do nothing. - Not implemented.
}

////////////////////////////////////////////////////////
// loop through stations in a station array, render them

void SymprodRenderChunk::_drawStationsFromArray(RenderContext &context)
  
{
  
  // make a copy of the buffer
  
  MemBuf buf;
  buf.add(_data, _props.nbytes_chunk);
  
  // compute pointers into the buffer
  
  station_report_array_header_t *hdr_ptr =
    (station_report_array_header_t *) buf.getPtr();
  station_report_t *stations = (station_report_t *)
    ((char *) buf.getPtr() + sizeof(station_report_array_header_t));
  
  // Convert this chunk to native format
  
  station_report_array_from_be(hdr_ptr, stations);

  // render the individual stations

  int n_stations = hdr_ptr->num_reports;
  station_report_t *station = stations;
  for(int i = 0; i < n_stations; i++, station++) {
    _drawStation(station,context);
  }

}

////////////////////////////////////////////////////////
// draw station from single-entry buffer

void SymprodRenderChunk::_drawStationSingle(RenderContext &context)
  
{
  
  // make a copy of the buffer
  
  MemBuf buf;
  buf.add(_data, _props.nbytes_chunk);

  // swap it

  station_report_from_be((station_report_t *) buf.getPtr());

  // draw it

  _drawStation(((station_report_t *) buf.getPtr()),context);

}

////////////////////////////////////////////////////////
// draw station

#define TEXT_LEN 512

void SymprodRenderChunk::_drawStation(station_report_t *station, RenderContext &context)
  
{
  

#ifdef NOTYET
  
  double iconScale = context.iconScale;
  if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
    iconScale = 1.0;
  }

  // get the barb shaft len
  
  if(_barbShaftLen == 0) {
    _barbShaftLen = _params.barb_shaft_len;
  }
  
  // Find world/projection coordinates
  
  double station_x ,station_y;
  _latLon2World(station->lat, station->lon, station_x, station_y,context);

  // terminate the text string

  char text[TEXT_LEN];
  text[0] = '\0';
  
  // Plot TEMPERATURE as text - Upper Left

  int len, direct, ascent, descent;
  XCharStruct overall;
  
  if (station->temp != STATION_NAN) {

    snprintf(text, TEXT_LEN, "%.0f", station->temp);
    len = strlen(text);
    XTextExtents(context.frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    
    int xoffset = -(overall.width) - _blankPixelSpacing; 
    int yoffset = -overall.ascent  - overall.descent - _blankPixelSpacing;
    
    if (iconScale >= prod._prodInfo.text_off_threshold) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
			     context.frame.x->font, &context.psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }

  }

  // Plot Weather Type indicators as text - Middle Left

  text[0] = '\0';
  if (station->msg_id == METAR_REPORT) {
    
    char *space_pos;
     
    // copy in the metar weather, and truncate the text after the
    // first token, i.e. at the first space
     
    STRncopy(text, station->shared.metar.weather_str, TEXT_LEN);
    space_pos = strchr(text, ' ');
    if (space_pos != NULL) {
      *space_pos = '\0';
    }
     
  } else {
    
    ui32 wtype = station->weather_type;
    
    // place these in order of priority - Last = highest
    // Priorities set for Aircraft Indutstry
    
    if(wtype & WT_FG) strcpy(text,"FG"); /*  Fog */
    if(wtype & WT_HZ) strcpy(text,"HZ"); /* Haze */
    if(wtype & WT_UP) strcpy(text,"UP"); /* Undet Precip */
    if(wtype & WT_DZ) strcpy(text,"DZ"); /* Drizzle */
    if(wtype & WT_DS) strcpy(text,"DS"); /* Dust Storm */
    if(wtype & WT_MRA) strcpy(text,"-RA"); /* Light Rain */
    if(wtype & WT_RA) strcpy(text,"RA"); /* Rain */
    if(wtype & WT_TS) strcpy(text,"TS"); /* Thunder Storms */
    if(wtype & WT_MSN) strcpy(text,"-SN"); /* Light Snow */
    if(wtype & WT_PFZDZ) strcpy(text,"+FZDZ"); /* Heavy Freezing Drizzle */
    if(wtype & WT_BLSN) strcpy(text,"BLSN"); /* Blowing Snow */
    if(wtype & WT_FZFG) strcpy(text,"FZFG"); /* Freezing Fog */
    if(wtype & WT_SQ) strcpy(text,"SQ"); /* Squall */
    if(wtype & WT_MFZRA) strcpy(text,"-FZRA"); /* Freezing Rain */
    if(wtype & WT_SN) strcpy(text,"SN"); /* Snow */
    if(wtype & WT_PRA) strcpy(text,"+RA"); /* Heavy Rain */
    if(wtype & WT_PTS) strcpy(text,"+TS"); /* Heavy Thunderstorms */
    if(wtype & WT_FZDZ) strcpy(text,"FZDZ"); /* Freezing Drizzle */
    if(wtype & WT_PSN) strcpy(text,"+SN"); /*  Heavy Snow */
    if(wtype & WT_PE) strcpy(text,"PE"); /* Ice Pellets */
    if(wtype & WT_MFZDZ) strcpy(text,"-FZDZ"); /* Light Freezing Drizzle */
    if(wtype & WT_GR) strcpy(text,"GR"); /* Hail */
    if(wtype & WT_FZRA) strcpy(text,"FZRA"); /* Freezing Rain */
    if(wtype & WT_PFZRA) strcpy(text,"+FZRA"); /* Heavy Freezing Rain */
    if(wtype & WT_FC) strcpy(text,"FC"); /* Funnel Cloud */
    
  }
     
  len = strlen(text);
  if( len > 1) {
    XTextExtents(context.frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    int xoffset =  -(overall.width) - _blankPixelSpacing; 
    int yoffset =  - overall.descent;
    if (iconScale >= prod._prodInfo.text_off_threshold) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
			     context.frame.x->font, &context.psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }
  
  // Plot DEWPOINT as text - Lower Left
  
  text[0] = '\0';
  if(station->dew_point != STATION_NAN) {
    snprintf(text, TEXT_LEN, "%.0f", station->dew_point);
    len = strlen(text);
    XTextExtents(context.frame.x->font ,text, len,
		 &direct, &ascent, &descent, &overall);
  
    int xoffset =  -(overall.width) - _blankPixelSpacing; 
    int yoffset =  overall.ascent  +  _blankPixelSpacing;
    
    if (iconScale >= prod._prodInfo.text_off_threshold) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
			     context.frame.x->font, &context.psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }

  // use zoom level to decide whether to draw label
  // scale > 250 is zoomed in for lat/lon
  // scale > 2.5 is zoomed in for flat

  double scale = (context.frame.x->xscale + context.frame.x->yscale) / 2.0;
  bool draw_label;
  if (scale > 250.0) {
    draw_label = true;
  } else if (scale > 2.5 && scale < 40.0) {
    draw_label = true;
  } else {
    draw_label = false;
  }

  // Plot Site ID as text - Upper Right

  if(draw_label && strlen(station->station_label) > 1) {

    // Only display the last part of the label if it starts with 'K'
    // Don't ask

    if(station->station_label[0] != 'K') {
      STRcopy(text, station->station_label, TEXT_LEN);
    } else {
      STRcopy(text, station->station_label+1, TEXT_LEN);
    }
    len = strlen(text);
    XTextExtents(context.frame.x->font ,text, len,
		 &direct, &ascent, &descent, &overall);
  
    int xoffset =  _blankPixelSpacing; 
    int yoffset =  -overall.ascent  - overall.descent - _blankPixelSpacing;
    
    if (iconScale >= prod._prodInfo.text_off_threshold) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
			     context.frame.x->font, &context.psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }
  
  // Plot  GUST as text - Lower Right

  if (station->windgust != STATION_NAN && station->windgust > 2.0) {
    
    snprintf(text,TEXT_LEN,"G%.0f",station->windgust * NMH_PER_MS);
    len = strlen(text);
    XTextExtents(context.frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    
    int xoffset =  _blankPixelSpacing; 
    int yoffset =  _blankPixelSpacing;
    
    if (iconScale >= prod._prodInfo.text_off_threshold) {
      GDrawImageStringOffset(context.dev, &context.frame, context.gc,
			     context.frame.x->font, &context.psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
    
  }

  // Determine the U & V componets of the wind & PLOT A BARB

  if (station->winddir != STATION_NAN && station->windspd != STATION_NAN) {

    int shaft_len = (int) (_barbShaftLen * iconScale + 0.5);
    int line_width = 1;

    //     if(_gd.h_win.km_across_screen < 200.0) {
    //       shaft_len = (int)  _barbShaftLen * 1.5;
    //       line_width = 2;
    //     }
    //     if(_gd.h_win.km_across_screen < 50.0) {
    //       line_width = 3;
    //       shaft_len = (int) _barbShaftLen * 2.5;
    //     }

    XSetLineAttributes(context.display, context.gc, line_width,
		       LineSolid, CapButt, JoinBevel);
    
    double u_wind = -sin(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(u_wind < 0.1 && u_wind > -0.1) u_wind = 0.0;

    double v_wind = -cos(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(v_wind < 0.1 && v_wind > -0.1) v_wind = 0.0;
 
    GDrawWindBarb(context.dev, &context.frame, context.gc, &context.psgc,
		  station_x, station_y,
		  u_wind, v_wind, shaft_len);

    XSetLineAttributes(context.display, context.gc, 1, LineSolid, CapButt, JoinBevel);

  }

#endif

}

////////////////////////////////////////////////////////////////////////
// Set the colors

void SymprodRenderObj::_setXColors(char *foreground_color,
				   char *background_color,
				   RenderContext &context)

{

#ifdef NOTYET
  
  // If the Color has changed, Set the GC
  
  if (!STRequal_exact(foreground_color, context.last_foreground_color)) {
    
    // Set the Foreground color in the GC for rendering.
    
    XColor xcolor;
    
    if (XParseColor(context.display, context.cmap,
		    foreground_color, &xcolor) == 0) {
      cerr << "WARNING - SymprodRenderObj::_setColors" << endl;
      cerr << "Cannot match foreground color: '"
	   << foreground_color << "'" << endl;
    } else {
      XAllocColor(context.display, context.cmap, &xcolor);
      XSetForeground(context.display, context.gc, xcolor.pixel);
      STRncopy(context.last_foreground_color, foreground_color, SYMPROD_COLOR_LEN);
    }
  }
  
  if (background_color[0] == '\0') {
    
    if (_background_pixel_init)
      XSetBackground(context.display, context.gc, _background_pixel);
    
  } else if (!STRequal_exact(background_color,
			     context.last_background_color)) {
    
    // Set the Background color in the GC for rendering.
    
    XColor xcolor;
    
    if (XParseColor(context.display, context.cmap,
		    background_color, &xcolor) == 0) {
      cerr << "WARNING - SymprodRenderObj::_setColors" << endl;
      cerr << "Cannot match background color: '"
	   << background_color << "'" << endl;
    } else {
      XAllocColor(context.display, context.cmap, &xcolor);
      XSetBackground(context.display, context.gc , xcolor.pixel);
      STRncopy(context.last_background_color, background_color,
	       SYMPROD_COLOR_LEN);
    }
  }

#endif
  
}

////////////////////////////////////////////////////////////////////////
// Set the backgroud color

void SymprodRenderObj::_setBackgroundColor(char *color_name,
                                           RenderContext &context)

{

#ifdef NOTYET
  
  XColor xcolor;

  if (XParseColor(context.display, context.cmap, color_name, &xcolor) == 0) {
    cerr << "ERROR - SymprodRenderObj::_setBackgroundColor" << endl;
    cerr << "Cannot match color: '" << color_name << "'" << endl;
    return;
  }
   
  XAllocColor(context.display, context.cmap, &xcolor);
  _background_pixel = xcolor.pixel;
  _background_pixel_init = true;

#endif

}

//////////////////////////////////////////////////////////////////////
// setStipple()  
//
// Set the stipple pattern for a gc

void SymprodRenderObj::setStipple(Symprod::fill_t filltype, 
                                  RenderContext &context)

{

#ifdef NOTYET
  
  Pixmap pixmap;

  switch (filltype) {

    case Symprod::FILL_NONE :
    case Symprod::FILL_SOLID :
      XSetFillStyle(context.display, context.gc, FillSolid);
      return;
      break;
    
    case Symprod::FILL_STIPPLE10 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple10_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE20 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple20_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE30 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple30_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE40 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple40_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE50 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple50_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE60 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple60_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE70 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple70_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE80 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple80_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;
    
    case Symprod::FILL_STIPPLE90 :
      pixmap =
        XCreateBitmapFromData(context.display, RootWindow(context.display, 0),
                              (char *) _stipple90_bitmap_bits,
                              _stipple_bitmap_width, _stipple_bitmap_height);
      break;

    default:
      return;
    
  }
  
  if (pixmap == None) {
    return;
  }

  XSetStipple(context.display, context.gc, pixmap);
  XFreePixmap(context.display, pixmap);

#endif

}

//////////////////////////////////////////////////////////////////////
// Convert the lat/lon values in a SYMPROD object to the correct
// world coordinates for the display.

void SymprodRenderObj::_latLon2World(double lat,
				     double lon,
				     double &world_x,
				     double &world_y,
				     RenderContext &context)
{
  
  context.proj.latlon2xy(lat, lon, world_x, world_y);
  
  if (context.proj.getProjType() == Mdvx::PROJ_LATLON) {
    // Normalize the longitudes to fall within the window frame
    if (world_x >= -720.0 && world_x < 720.0) {
      while (world_x < context.frame.w_xmin) {
	world_x += 360.0;
      }
      while (world_x >= context.frame.w_xmin + 360.0) {
	world_x -= 360.0;
      }
    }
  }
  
}

//////////////////////////////////////////////////////////////////////
// Convert the lat/lon values in a SYMPROD object to the correct
// world coordinates for the display.  The lon value is normalized,
// but is constrained to fall on the given side of the given point.
//
// If constrain_to_east is true, the resulting world point will be
// east of the given constraint point.  Otherwise, it will be west
// of the given constraint point.

void SymprodRenderObj::_latLon2WorldConstrained(double lat,
						double lon,
						double ref_x,
						double &world_x,
						double &world_y,
						RenderContext &context)
{
  
  context.proj.latlon2xy(lat, lon, world_x, world_y);
  
  if (context.proj.getProjType() == Mdvx::PROJ_LATLON) {
    // Normalize the longitudes to fall within the window frame
    if (world_x >= -720.0 && world_x < 720.0) {
      while (world_x < ref_x - 180.0) {
	world_x += 360.0;
      }
      while (world_x >= ref_x + 180.0) {
	world_x -= 360.0;
      }
    }
  }
  
}

//////////////////////////////////////////////////////////////////////
// Convert the SYMPROD capstyle to the value used by X.

int SymprodRenderObj::_convertCapstyle2X(Symprod::capstyle_t capstyle)
  
{
#ifdef NOTYET
  switch (capstyle) {
    case Symprod::CAPSTYLE_BUTT :
      return(CapButt);
    case Symprod::CAPSTYLE_NOT_LAST :
      return(CapNotLast);
    case Symprod::CAPSTYLE_PROJECTING :
      return(CapProjecting);
    case Symprod::CAPSTYLE_ROUND :
      return(CapRound);
  }
  return(CapButt);
#endif
  return 0;
}


//////////////////////////////////////////////////////////////////////
// Convert the SYMPROD joinstyle to the value used by X.

int SymprodRenderObj::_convertJoinstyle2X(Symprod::joinstyle_t joinstyle)
{
#ifdef NOTYET
  switch (joinstyle) {
    case Symprod::JOINSTYLE_BEVEL :
      return(JoinBevel);
    case Symprod::JOINSTYLE_MITER :
      return(JoinMiter);
    case Symprod::JOINSTYLE_ROUND :
      return(JoinRound);
  }
  return(JoinBevel);
#endif
  return 0;
}


//////////////////////////////////////////////////////////////////////
// Convert the SYMPROD linetype to the value used by X.

int SymprodRenderObj::_convertLinetype2X(Symprod::linetype_t linetype)
{
#ifdef NOTYET
  switch (linetype) {
    case Symprod::LINETYPE_SOLID :
      return(LineSolid);
    case Symprod::LINETYPE_DASH :
      return(LineOnOffDash);
    case Symprod::LINETYPE_DOT_DASH :
      return(LineDoubleDash);
  }
  return(LineSolid);
#endif
  return 0;
}
//////////////////////////////////////////////////////////////////////
// Set up an 8 point pick box around the input coords.

#define PICK_BOX_SIZE 3

void set_pick_box_xy(int x, int y, QPoint *bpt)
{
#ifdef NOTYET
  bpt[0].x = x - PICK_BOX_SIZE;
  bpt[0].y = y - PICK_BOX_SIZE;

  bpt[1].x = x - PICK_BOX_SIZE;
  bpt[1].y = y + PICK_BOX_SIZE;

  bpt[2].x = x + PICK_BOX_SIZE;
  bpt[2].y = y + PICK_BOX_SIZE;
    
  bpt[3].x = x + PICK_BOX_SIZE;
  bpt[3].y = y - PICK_BOX_SIZE;

  bpt[4].x = x - PICK_BOX_SIZE;
  bpt[4].y = y - PICK_BOX_SIZE;

  bpt[5].x = x + PICK_BOX_SIZE;
  bpt[5].y = y + PICK_BOX_SIZE;

  bpt[6].x = x + PICK_BOX_SIZE;
  bpt[6].y = y - PICK_BOX_SIZE;

  bpt[7].x = x - PICK_BOX_SIZE;
  bpt[7].y = y + PICK_BOX_SIZE;
#endif
}
  
