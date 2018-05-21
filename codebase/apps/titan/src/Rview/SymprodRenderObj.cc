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
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "SymprodRenderObj.hh"
#include <Spdb/Product_defines.hh>
#include <toolsa/str.h>
#include <toolsa/TaArray.hh>
using namespace std;

#define TEXT_SCALE_THRESHOLD 0.4

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

SymprodRenderObj::SymprodRenderObj(Display *display)
  : _display(display)
  
{

  _gc = XCreateGC(_display, DefaultRootWindow(_display), 0, 0);

  _background_pixel = 0;
  _background_pixel_init = false;
  
  MEM_zero(_last_foreground_color);
  MEM_zero(_last_background_color);

}

SymprodRenderObj::~SymprodRenderObj()
{
  XFreeGC(_display, _gc);
}


//////////////////////////////////////////////////////////////////////////////
// Text objects

SymprodRenderText::SymprodRenderText(Display *display,
				     const void *prod_buffer,
				     const int obj_offset)
  : SymprodRenderObj(display),
    SymprodText(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderText::draw(RenderContext *context)
{

  _context = context;

  if (_context->iconScale < TEXT_SCALE_THRESHOLD) {
    return;
  }

  // double iconScale = _context->iconScale;
  // if (_hdr.detail_level == -1) {
  //   iconScale = 1.0;
  // }

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

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
  
  // Draw the text where indicated

#ifdef DEBUG
  fprintf(stderr, "Drawing label \"%s\" at lat = %f, lon = %f\n",
	  _props.text, _props.origin.lat, _props.origin.lon);
#endif
  
  double x_world;
  double y_world;
  
  _latLon2World(_props.origin.lat, _props.origin.lon,
		x_world, y_world);
  
  GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			 _context->frame.x->font, &_context->psgc,
			 x_just, y_just, x_world, y_world,
			 _props.offset.x, _props.offset.y,
			 (char *) _text.c_str());
  
}

//////////////////////////////////////////////////////////////////////////////
// Polyline objects

SymprodRenderPolyline::
SymprodRenderPolyline(Display *display,
		      const void *prod_buffer,
		      const int obj_offset)
  : SymprodRenderObj(display),
    SymprodPolyline(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderPolyline::draw(RenderContext *context)

{

  _context = context;

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
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
  
  if (_context->dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(_display, _gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
    if (_props.close_flag &&
	_props.fill >= Symprod::FILL_STIPPLE10 &&
	_props.fill <= Symprod::FILL_STIPPLE90)  {
      setStipple((Symprod::fill_t) _props.fill);
    }
  }
  
  // initialize the world conversion
  
  double x_start, y_start;
  double x_end, y_end;

  _latLon2World(_points[istart].lat, _points[istart].lon,
		x_start, y_start);
  _latLon2WorldConstrained(_points[iend].lat, _points[iend].lon,
			   x_start, x_end, y_end);

  // loop through the points, rendering each time we get a penup

  MemBuf gptsBuf;
  
  for (int ii = 0; ii < _props.num_points; ii++)  {
    
    bool penup = false;
    
    if (_points[ii].lat != Symprod::WPT_PENUP &&
	_points[ii].lon != Symprod::WPT_PENUP) {
      GPoint gpt;
      _latLon2WorldConstrained(_points[ii].lat, _points[ii].lon,
			       x_start, gpt.x, gpt.y);
      gptsBuf.add(&gpt, sizeof(gpt));
      penup = false;
    } else {
      penup = true;
    }
    
    // render and reset as appropriate
    
    if (penup || ii == _props.num_points - 1) {
      
      if (_context->dev == XDEV) {
	XSetFillStyle(_display, _gc, FillSolid);
      }
      
      GPoint *gpts = (GPoint *) gptsBuf.getPtr();
      int npts = gptsBuf.getLen() /  sizeof(GPoint);
      GDrawLines(_context->dev, &_context->frame,
		 _gc, &_context->psgc, gpts, npts, CoordModeOrigin);
      
      if (_props.close_flag &&
	  _props.fill != Symprod::FILL_NONE) {
	if (_context->dev == XDEV) {
	  XSetFillStyle(_display, _gc, FillStippled);
	}
	GFillPolygon(_context->dev, &_context->frame,
		     _gc, &_context->psgc, gpts, npts, CoordModeOrigin);
      }

      gptsBuf.free();
      
    } // if (penup ...

  } // ii

  // close as required
  
  if (_props.close_flag) {
    GDrawLine(_context->dev, &_context->frame,
	      _gc, &_context->psgc, x_end, y_end, x_start, y_start);
  }

}

//////////////////////////////////////////////////////////////////////////////
// Iconline objects

SymprodRenderIconline::
SymprodRenderIconline(Display *display,
		      const void *prod_buffer,
		      const int obj_offset)
  : SymprodRenderObj(display),
    SymprodIconline(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderIconline::draw(RenderContext *context)

{

  if (_props.num_points < 2) {
    return;
  }

  _context = context;

  double iconScale = _context->iconScale;
  if (_hdr.detail_level == -1) {
    iconScale = 1.0;
  }

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }
  
  // Get the centroid in x and y
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y);
  
  // create a wpt_t array of points, computing the world coords by
  // taking the origin and scaling the offsets accordingly

  double penup = -9999.0;
  TaArray<GPoint> _gpts;
  GPoint *gpts = _gpts.alloc(_props.num_points);
  
  for (int i = 0; i < _props.num_points; i++) {
    if (_points[i].x == Symprod::PPT_PENUP &&
	_points[i].y == Symprod::PPT_PENUP) {
      gpts[i].x = penup;
      gpts[i].y = penup;
    } else {
      if (_context->dev == XDEV) {
	if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
	  gpts[i].x = origin_x + _points[i].x / _context->frame.x->xscale;
	  gpts[i].y = origin_y + _points[i].y / _context->frame.x->yscale;
	} else {
	  gpts[i].x = origin_x + _points[i].x *
	    (iconScale / _context->frame.x->xscale);
	  gpts[i].y = origin_y + _points[i].y *
	    (iconScale / _context->frame.x->yscale);
	}
      } else {
	if (_hdr.detail_level & Symprod::DETAIL_LEVEL_DO_NOT_SCALE) {
	  gpts[i].x = origin_x + _points[i].x / _context->frame.ps->xscale;
	  gpts[i].y = origin_y + _points[i].y / _context->frame.ps->yscale;
	} else {
	  gpts[i].x = origin_x + _points[i].x *
	    (iconScale / _context->frame.ps->xscale);
	  gpts[i].y = origin_y + _points[i].y *
	    (iconScale / _context->frame.ps->yscale);
	}
      }
    }
    
    // gpts[i].x +=  _context->offset_x / _context->frame.x->xscale;
    // gpts[i].y -=  _context->offset_y / _context->frame.x->yscale;

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
  
  if (_context->dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(_display, _gc,
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
	if (_context->dev == XDEV) {
	  XSetFillStyle(_display, _gc, FillSolid);
	}
      
	GDrawLines(_context->dev, &_context->frame,
		   _gc, &_context->psgc,
		   gpts + start, count,
		   CoordModeOrigin);
      
	if (_props.close_flag &&
	    _props.fill != Symprod::FILL_NONE) {
	  if (_context->dev == XDEV &&
	      _props.fill >= Symprod::FILL_STIPPLE10 &&
	      _props.fill <= Symprod::FILL_STIPPLE90)  {
	    setStipple((Symprod::fill_t) _props.fill);
	    XSetFillStyle(_display, _gc, FillStippled);
	  }
	  GFillPolygon(_context->dev, &_context->frame,
		       _gc, &_context->psgc,
		       gpts + start, count,
		       CoordModeOrigin);
	  if (_context->dev == XDEV &&
	      _props.fill >= Symprod::FILL_STIPPLE10 &&
	      _props.fill <= Symprod::FILL_STIPPLE90)  {
	    XSetFillStyle(_display, _gc, FillSolid);
	  }
	}
      } // if (count >= 2) 

      start = ii + 1;
      count = 0;

    } // if (penup ...

  } // ii

  // close as required
  
  if (_props.close_flag) {
    GDrawLine(_context->dev, &_context->frame,
	      _gc, &_context->psgc,
	      gpts[iend].x, gpts[iend].y,
	      gpts[istart].x, gpts[istart].y);
  }
  
}

//////////////////////////////////////////////////////////////////////////////
// StrokedIcon objects

SymprodRenderStrokedIcon::
SymprodRenderStrokedIcon(Display *display,
			 const void *prod_buffer,
			 const int obj_offset)
  : SymprodRenderObj(display),
    SymprodStrokedIcon(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderStrokedIcon::draw(RenderContext *context)

{

  if (_props.num_icons < 1) {
    return;
  }

  _context = context;

  double iconScale = _context->iconScale;
  if (_hdr.detail_level == -1) {
    iconScale = 1.0;
  }

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }
  
  // initialize the world conversion
  
  double x_start, y_start;
  _latLon2World(_iconOrigins[0].lat, _iconOrigins[0].lon,
		x_start, y_start);

  // Draw the icon at each of the given positions.
  
  for (int ii = 0; ii < _props.num_icons; ii++) {
    
#ifdef DEBUG    
    fprintf(stderr,
	    "Creating stroked icon at lat = %f, lon = %f\n",
	    _iconOrigins[ii].lat, _iconOrigins[ii].lon);
#endif
    
    // Determine the world coordinates for the icon.
    
    double origin_x, origin_y;
    _latLon2WorldConstrained(_iconOrigins[ii].lat, _iconOrigins[ii].lon,
			     x_start, origin_x, origin_y);
    
    // Draw each stroke in the icon.  The strokes are drawn individually
    // from icon_pt-1 to icon_pt.

    for (int ipt = 1; ipt < _props.num_icon_pts; ipt++) {

      if (_iconPts[ipt-1].x == Symprod::PPT_PENUP ||
	  _iconPts[ipt-1].y == Symprod::PPT_PENUP ||
	  _iconPts[ipt].x == Symprod::PPT_PENUP ||
	  _iconPts[ipt].y == Symprod::PPT_PENUP)
	continue;
      
      double world_x1 =
	GXWorldx(&_context->frame,
		 GXWindowx(&_context->frame, origin_x) +
		 (int) (iconScale * _iconPts[ipt - 1].x + 0.5));
      double world_y1 =
	GXWorldy(&_context->frame,
		 GXWindowy(&_context->frame, origin_y) +
		 (int) (iconScale * _iconPts[ipt - 1].y + 0.5));
      double world_x2 =
	GXWorldx(&_context->frame,
		 GXWindowx(&_context->frame, origin_x) +
		 (int) (iconScale * _iconPts[ipt].x + 0.5));
      double world_y2 =
	GXWorldy(&_context->frame,
		 GXWindowy(&_context->frame, origin_y) +
		 (int) (iconScale * _iconPts[ipt].y + 0.5));
      
      GDrawLine(_context->dev, &_context->frame, _gc, &_context->psgc,
		world_x1, world_y1, world_x2, world_y2);
      
    } // ipt
    
  } // ii

}

//////////////////////////////////////////////////////////////////////////////
// NamedIcon objects

SymprodRenderNamedIcon::
SymprodRenderNamedIcon(Display *display,
		       const void *prod_buffer,
		       const int obj_offset)
  : SymprodRenderObj(display),
    SymprodNamedIcon(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderNamedIcon::draw(RenderContext *context)

{

  _context = context;

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

  cerr << "ERROR - SymprodRenderNamedIcon::draw()" << endl;
  cerr << "  Not yet implemented." << endl;

}

//////////////////////////////////////////////////////////////////////////////
// BitmapIcon objects

SymprodRenderBitmapIcon::
SymprodRenderBitmapIcon(Display *display,
			const void *prod_buffer,
			const int obj_offset)
  : SymprodRenderObj(display),
    SymprodBitmapIcon(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderBitmapIcon::draw(RenderContext *context)

{

  _context = context;

  double iconScale = _context->iconScale;
  if (_hdr.detail_level == -1) {
    iconScale = 1.0;
  }

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

  // create array of active points

  MemBuf offsetBuf;
  int size = _props.bitmap_x_dim * _props.bitmap_y_dim;
  
  for (int ii = 0; ii < size; ii++) {
    if (_bitmap[ii])  {
      XPoint offset;
      offset.x = -(_props.bitmap_x_dim / 2 - ii % _props.bitmap_x_dim);
      offset.x = (int) (offset.x * iconScale + 0.5);
      offset.y = _props.bitmap_y_dim / 2 - ii / _props.bitmap_x_dim;
      offset.y = (int) (offset.y * iconScale + 0.5);
      offsetBuf.add(&offset, sizeof(offset));
    }
  }
  
  // initialize the world conversion
  
  double x_start, y_start;
  _latLon2World(_iconOrigins[0].lat, _iconOrigins[0].lon,
		x_start, y_start);

  // Draw the icon at each of the given centroids

  for (int ii = 0; ii < _props.num_icons; ii++) {

    // Determine the world coordinates for the icon.
    
    double origin_x, origin_y;
    _latLon2WorldConstrained(_iconOrigins[ii].lat, _iconOrigins[ii].lon,
			     x_start, origin_x, origin_y);
    
    // Now draw the icon.

    int nOffsets = offsetBuf.getLen() / sizeof(XPoint);
    GDrawPoints(_context->dev, &_context->frame,
		_gc, &_context->psgc,	origin_x, origin_y,
		(XPoint *) offsetBuf.getPtr(), nOffsets);
    
  } // ii
  
}

//////////////////////////////////////////////////////////////////////////////
// Arc objects

SymprodRenderArc::
SymprodRenderArc(Display *display,
		 const void *prod_buffer,
		 const int obj_offset)
  : SymprodRenderObj(display),
    SymprodArc(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderArc::draw(RenderContext *context)

{

  _context = context;

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

  // set the line attributes.
  
  if (_context->dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(_display, _gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
    if (_props.fill >= Symprod::FILL_STIPPLE10 &&
	_props.fill <= Symprod::FILL_STIPPLE90)  {
      setStipple((Symprod::fill_t) _props.fill);
    }
  }

  // Get the centroid
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y);
    
  // draw the arc
  
  if (_props.fill == Symprod::FILL_SOLID) {
    GFillArc(_context->dev, &_context->frame, _gc, &_context->psgc,
	     origin_x, origin_y,
	     _props.radius_x, _props.radius_y,
	     _props.angle1, _props.angle2,
	     _props.axis_rotation, _props.nsegments);
  } else {
    GDrawArc(_context->dev, &_context->frame, _gc, &_context->psgc,
	     origin_x, origin_y,
	     _props.radius_x, _props.radius_y,
	     _props.angle1, _props.angle2,
	     _props.axis_rotation, _props.nsegments);
  }
  
}

//////////////////////////////////////////////////////////////////////////////
// Rectangle objects

SymprodRenderRectangle::
SymprodRenderRectangle(Display *display,
		       const void *prod_buffer,
		       const int obj_offset)
  : SymprodRenderObj(display),
    SymprodRectangle(prod_buffer, obj_offset)
  
{
  
}

void SymprodRenderRectangle::draw(RenderContext *context)

{

  _context = context;

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

  // set the line attributes.
  
  if (_context->dev == XDEV) {
    Symprod::capstyle_t capstyle = (Symprod::capstyle_t) _props.capstyle;
    Symprod::joinstyle_t joinstyle = (Symprod::joinstyle_t) _props.joinstyle;
    Symprod::linetype_t linetype = (Symprod::linetype_t) _props.linetype;
    XSetLineAttributes(_display, _gc,
		       _props.linewidth,
		       _convertLinetype2X(linetype),
		       _convertCapstyle2X(capstyle),
		       _convertJoinstyle2X(joinstyle));
    if (_props.fill >= Symprod::FILL_STIPPLE10 &&
	_props.fill <= Symprod::FILL_STIPPLE90)  {
      setStipple((Symprod::fill_t) _props.fill);
    }
  }

  // Get the centroid
  
  double origin_x, origin_y;
  _latLon2World(_props.origin.lat, _props.origin.lon,
		origin_x, origin_y);
    
  // draw the rectangle
  
  if (_props.fill == Symprod::FILL_SOLID) {
    GFillRectangle(_context->dev, &_context->frame, _gc, &_context->psgc,
		   origin_x, origin_y,
		   _props.width, _props.height);
  } else {
    GDrawRectangle(_context->dev, &_context->frame, _gc, &_context->psgc,
		   origin_x, origin_y,
		   _props.width, _props.height);
  }
  
}

//////////////////////////////////////////////////////////////////////////////
// Chunk objects

SymprodRenderChunk::
SymprodRenderChunk(Display *display,
		   const void *prod_buffer,
		   const int obj_offset)
  : SymprodRenderObj(display),
    SymprodChunk(prod_buffer, obj_offset)
  
{
  _barbShaftLen = 0;
}

void SymprodRenderChunk::draw(RenderContext *context)

{

  _context = context;

  // set the colors

  if (_context->dev == XDEV) {
    _setXColors(_hdr.color, _hdr.background_color);
  }

  // Render the chunk based on its type.

  switch (_props.chunk_type)  {
  
  case SPDB_STATION_REPORT_ID :
    _drawStationSingle();
    break;
    
  case SPDB_STATION_REPORT_ARRAY_ID :
    _drawStationsFromArray();
    break;
    
  default :
    cerr << "WARNING:  SymprodRenderChunk::draw." << endl;
    cerr << "  Don't know how to render chunk type: "
	 << _props.chunk_type << endl;
    cerr << "  Skipping ...." << endl;
    break;
    
  } /* endswitch - _props.chunk_type */

}

////////////////////////////////////////////////////////
// loop through stations in a station array, render them

void SymprodRenderChunk::_drawStationsFromArray()
  
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
    _drawStation(station);
  }

}

////////////////////////////////////////////////////////
// draw station from single-entry buffer

void SymprodRenderChunk::_drawStationSingle()
  
{
  
  // make a copy of the buffer
  
  MemBuf buf;
  buf.add(_data, _props.nbytes_chunk);

  // swap it

  station_report_from_be((station_report_t *) buf.getPtr());

  // draw it

  _drawStation((station_report_t *) buf.getPtr());

}

////////////////////////////////////////////////////////
// draw station

#define TEXT_LEN 512

void SymprodRenderChunk::_drawStation(station_report_t *station)
  
{
  
  double iconScale = _context->iconScale;
  if (_hdr.detail_level == -1) {
    iconScale = 1.0;
  }

  // get the barb shaft len
  
  if(_barbShaftLen == 0) {
    // _barbShaftLen = XRSgetLong(gd.cidd_db, "cidd.barb_shaft_len", 33);
    _barbShaftLen = 33;
  }
  
  // Find world/projection coordinates
  
  double station_x ,station_y;
  _latLon2World(station->lat, station->lon, station_x, station_y);

  // terminate the text string

  char text[TEXT_LEN];
  text[0] = '\0';
  
  // Plot TEMPERATURE as text - Upper Left

  int len, direct, ascent, descent;
  XCharStruct overall;
  
  if (station->temp != STATION_NAN) {

    sprintf(text, "%.0f", station->temp);
    len = strlen(text);
    XTextExtents(_context->frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    
    int xoffset = -(overall.width) - _blankPixelSpacing; 
    int yoffset = -overall.ascent  - overall.descent - _blankPixelSpacing;
    
    if (iconScale >= TEXT_SCALE_THRESHOLD) {
      GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			     _context->frame.x->font, &_context->psgc,
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
    if(wtype & WT_SG) strcpy(text,"SG"); /* Light Snow */
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
    if(wtype & WT_GS) strcpy(text,"GS"); /* Small Hail */
    if(wtype & WT_GR) strcpy(text,"GR"); /* Hail */
    if(wtype & WT_FZRA) strcpy(text,"FZRA"); /* Freezing Rain */
    if(wtype & WT_PFZRA) strcpy(text,"+FZRA"); /* Freezing Rain */
    if(wtype & WT_FC) strcpy(text,"FC"); /* Funnel Cloud */
    
  }
     
  len = strlen(text);
  if( len > 1) {
    XTextExtents(_context->frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    int xoffset =  -(overall.width) - _blankPixelSpacing; 
    int yoffset =  - overall.descent;
    if (iconScale >= TEXT_SCALE_THRESHOLD) {
      GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			     _context->frame.x->font, &_context->psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }
  
  // Plot DEWPOINT as text - Lower Left
  
  text[0] = '\0';
  if(station->dew_point != STATION_NAN) {
    sprintf(text, "%.0f", station->dew_point);
    len = strlen(text);
    XTextExtents(_context->frame.x->font ,text, len,
		 &direct, &ascent, &descent, &overall);
  
    int xoffset =  -(overall.width) - _blankPixelSpacing; 
    int yoffset =  overall.ascent  +  _blankPixelSpacing;
    
    if (iconScale >= TEXT_SCALE_THRESHOLD) {
      GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			     _context->frame.x->font, &_context->psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }

  // use zoom level to decide whether to draw label
  // scale > 250 is zoomed in for lat/lon
  // scale > 2.5 is zoomed in for flat

  double scale = (_context->frame.x->xscale + _context->frame.x->yscale) / 2.0;
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
    XTextExtents(_context->frame.x->font ,text, len,
		 &direct, &ascent, &descent, &overall);
  
    int xoffset =  _blankPixelSpacing; 
    int yoffset =  -overall.ascent  - overall.descent - _blankPixelSpacing;
    
    if (iconScale >= TEXT_SCALE_THRESHOLD) {
      GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			     _context->frame.x->font, &_context->psgc,
			     XJ_LEFT, YJ_BELOW,
			     station_x, station_y,
			     xoffset, -yoffset,
			     text);
    }
  }
  
  // Plot  GUST as text - Lower Right

  if (station->windgust != STATION_NAN && station->windgust > 2.0) {
    
    sprintf(text,"G%.0f",station->windgust * NMH_PER_MS);
    len = strlen(text);
    XTextExtents(_context->frame.x->font, text, len,
		 &direct, &ascent, &descent, &overall);
    
    int xoffset =  _blankPixelSpacing; 
    int yoffset =  _blankPixelSpacing;
    
    if (iconScale >= TEXT_SCALE_THRESHOLD) {
      GDrawImageStringOffset(_context->dev, &_context->frame, _gc,
			     _context->frame.x->font, &_context->psgc,
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

    XSetLineAttributes(_display, _gc, line_width,
		       LineSolid, CapButt, JoinBevel);
    
    double u_wind = -sin(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(u_wind < 0.1 && u_wind > -0.1) u_wind = 0.0;

    double v_wind = -cos(station->winddir * RAD_PER_DEG) *
      station->windspd * NMH_PER_MS;
    if(v_wind < 0.1 && v_wind > -0.1) v_wind = 0.0;
 
    GDrawWindBarb(_context->dev, &_context->frame, _gc, &_context->psgc,
		  station_x, station_y,
		  u_wind, v_wind, shaft_len);

    XSetLineAttributes(_display, _gc, 1, LineSolid, CapButt, JoinBevel);

  }

}

////////////////////////////////////////////////////////////////////////
// Set the colors

void SymprodRenderObj::_setXColors(char *foreground_color,
				   char *background_color)

{

  // If the Color has changed, Set the GC
  
  if (!STRequal_exact(foreground_color, _last_foreground_color)) {
    
    // Set the Foreground color in the GC for rendering.
    
    XColor xcolor;
    
    if (XParseColor(_display, _context->cmap,
		    foreground_color, &xcolor) == 0) {
      cerr << "WARNING - SymprodRenderObj::_setColors" << endl;
      cerr << "Cannot match foreground color: '"
	   << foreground_color << "'" << endl;
    } else {
      XAllocColor(_display, _context->cmap, &xcolor);
      XSetForeground(_display, _gc, xcolor.pixel);
      STRncopy(_last_foreground_color, foreground_color, SYMPROD_COLOR_LEN);
    }
    
  }
  
  if (background_color[0] == '\0') {
    
    if (_background_pixel_init)
      XSetBackground(_display, _gc, _background_pixel);
    
  } else if (!STRequal_exact(background_color,
			     _last_background_color)) {
    
    // Set the Background color in the GC for rendering.
    
    XColor xcolor;
    
    if (XParseColor(_display, _context->cmap,
		    background_color, &xcolor) == 0) {
      cerr << "WARNING - SymprodRenderObj::_setColors" << endl;
      cerr << "Cannot match background color: '"
	   << background_color << "'" << endl;
    } else {
      XAllocColor(_display, _context->cmap, &xcolor);
      XSetBackground(_display, _gc , xcolor.pixel);
      STRncopy(_last_background_color, background_color,
	       SYMPROD_COLOR_LEN);
    }
    
  }
  
}

////////////////////////////////////////////////////////////////////////
// Set the backgroud color

void SymprodRenderObj::_setBackgroundColor(char *color_name)

{

  XColor xcolor;

  if (XParseColor(_display, _context->cmap, color_name, &xcolor) == 0) {
    cerr << "ERROR - SymprodRenderObj::_setBackgroundColor" << endl;
    cerr << "Cannot match color: '" << color_name << "'" << endl;
    return;
  }
  
  XAllocColor(_display, _context->cmap, &xcolor);
  _background_pixel = xcolor.pixel;
  _background_pixel_init = true;

}

//////////////////////////////////////////////////////////////////////
// setStipple()
//
// Set the stipple pattern for a gc

void SymprodRenderObj::setStipple(Symprod::fill_t filltype)

{

  Pixmap pixmap;

  switch (filltype) {

  case Symprod::FILL_STIPPLE10 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple10_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE20 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple20_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE30 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple30_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE40 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple40_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE50 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple50_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE60 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple60_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE70 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple70_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE80 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple80_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;
    
  case Symprod::FILL_STIPPLE90 :
    pixmap =
      XCreateBitmapFromData(_display, RootWindow(_display, 0),
			    (char *) _stipple90_bitmap_bits,
			    _stipple_bitmap_width, _stipple_bitmap_height);
    break;

  default:
    return;
    
  }
  
  if (pixmap == None) {
    return;
  }

  XSetStipple(_display, _gc, pixmap);
  XFreePixmap(_display, pixmap);

}

//////////////////////////////////////////////////////////////////////
// Convert the lat/lon values in a SYMPROD object to the correct
// world coordinates for the display.

void SymprodRenderObj::_latLon2World(double lat,
				     double lon,
				     double &world_x,
				     double &world_y)
{
  
  _context->proj.latlon2xy(lat, lon, world_x, world_y);
  
  if (_context->proj.getProjType() == Mdvx::PROJ_LATLON) {
    // Normalize the longitudes to fall within the window frame
    if (world_x >= -720.0 && world_x < 720.0) {
      while (world_x < _context->frame.w_xmin) {
	world_x += 360.0;
      }
      while (world_x >= _context->frame.w_xmin + 360.0) {
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
						double &world_y)
{
  
  _context->proj.latlon2xy(lat, lon, world_x, world_y);
  
  if (_context->proj.getProjType() == Mdvx::PROJ_LATLON) {
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
}


//////////////////////////////////////////////////////////////////////
// Convert the SYMPROD joinstyle to the value used by X.

int SymprodRenderObj::_convertJoinstyle2X(Symprod::joinstyle_t joinstyle)
{
  switch (joinstyle) {
  case Symprod::JOINSTYLE_BEVEL :
    return(JoinBevel);
  case Symprod::JOINSTYLE_MITER :
    return(JoinMiter);
  case Symprod::JOINSTYLE_ROUND :
    return(JoinRound);
  }
  return(JoinBevel);
}


//////////////////////////////////////////////////////////////////////
// Convert the SYMPROD linetype to the value used by X.

int SymprodRenderObj::_convertLinetype2X(Symprod::linetype_t linetype)
{
  switch (linetype) {
  case Symprod::LINETYPE_SOLID :
    return(LineSolid);
  case Symprod::LINETYPE_DASH :
    return(LineOnOffDash);
  case Symprod::LINETYPE_DOT_DASH :
    return(LineDoubleDash);
  }
  return(LineSolid);
}

