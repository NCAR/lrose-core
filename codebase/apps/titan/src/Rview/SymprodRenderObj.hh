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
// SymprodRenderObj.hh
//
// Symprod rendering objects
//
// These classes provide the rendering functions for the SymprodObj
// objects.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef SymprodRenderObj_HH
#define SymprodRenderObj_HH

#include <Spdb/SymprodObj.hh>
#include "RenderContext.hh"
#include <rapformats/station_reports.h>
using namespace std;

//////////////////////
// Abstract base class

class SymprodRenderObj

{

public:

  SymprodRenderObj(Display *display);

  virtual ~SymprodRenderObj();
  
  virtual void
  draw(RenderContext *context) = 0;  // forces object to be abstract

protected:
  
  Display *_display;
  RenderContext *_context;

  unsigned long _background_pixel;
  bool _background_pixel_init;
  
  char _last_foreground_color[SYMPROD_COLOR_LEN];
  char _last_background_color[SYMPROD_COLOR_LEN];

  GC _gc;
  Pixmap _stipple;
  bool _stippleCreated;

  static const int _blankPixelSpacing;
  static const int _stipple_bitmap_width;
  static const int _stipple_bitmap_height;
  static const unsigned char _stipple10_bitmap_bits[];
  static const unsigned char _stipple20_bitmap_bits[];
  static const unsigned char _stipple30_bitmap_bits[];
  static const unsigned char _stipple40_bitmap_bits[];
  static const unsigned char _stipple50_bitmap_bits[];
  static const unsigned char _stipple60_bitmap_bits[];
  static const unsigned char _stipple70_bitmap_bits[];
  static const unsigned char _stipple80_bitmap_bits[];
  static const unsigned char _stipple90_bitmap_bits[];

  void _setXColors(char *foreground_color, char *background_color);
  
  void _setBackgroundColor(char *color_name);
  
  void setStipple(Symprod::fill_t filltype);
  
  void _latLon2World(double lat,
		     double lon,
		     double &world_x,
		     double &world_y);

  void _latLon2WorldConstrained(double lat,
				double lon,
				double ref_x,
				double &world_x,
				double &world_y);

  static int _convertCapstyle2X(Symprod::capstyle_t capstyle);
  static int _convertJoinstyle2X(Symprod::joinstyle_t joinstyle);
  static int _convertLinetype2X(Symprod::linetype_t linetype);

private:
  
};

/////////////////////////////////////////////////////////////////////
// Text objects

class SymprodRenderText
  : public SymprodRenderObj, public SymprodText

{

public:
  
  SymprodRenderText(Display *display,
		    const void *prod_buffer,
		    const int obj_offset);
  
  virtual ~SymprodRenderText() {};
  
  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// Polyline objects

class SymprodRenderPolyline
  : public SymprodRenderObj, public SymprodPolyline

{

public:

  SymprodRenderPolyline(Display *display,
			const void *prod_buffer,
			const int obj_offset);

  virtual ~SymprodRenderPolyline() {};

  virtual void draw(RenderContext *context);
  
protected:

private:

};

/////////////////////////////////////////////////////////////////////
// Iconline objects

class SymprodRenderIconline
  : public SymprodRenderObj, public SymprodIconline

{

public:

  SymprodRenderIconline(Display *display,
			const void *prod_buffer,
			const int obj_offset);

  virtual ~SymprodRenderIconline() {};
  
  virtual void draw(RenderContext *context);
  
protected:

private:

};

/////////////////////////////////////////////////////////////////////
// StrokedIcon objects

class SymprodRenderStrokedIcon
  : public SymprodRenderObj, public SymprodStrokedIcon

{

public:

  SymprodRenderStrokedIcon(Display *display,
			   const void *prod_buffer,
			   const int obj_offset);
  
  virtual ~SymprodRenderStrokedIcon() {};

  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// NamedIcon objects

class SymprodRenderNamedIcon
  : public SymprodRenderObj, public SymprodNamedIcon

{

public:

  SymprodRenderNamedIcon(Display *display,
			 const void *prod_buffer,
			 const int obj_offset);
  
  virtual ~SymprodRenderNamedIcon() {};

  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// BitmapIcon objects

class SymprodRenderBitmapIcon
  : public SymprodRenderObj, public SymprodBitmapIcon

{

public:

  SymprodRenderBitmapIcon(Display *display,
			  const void *prod_buffer,
			  const int obj_offset);
  
  virtual ~SymprodRenderBitmapIcon() {};

  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// Arc objects

class SymprodRenderArc
  : public SymprodRenderObj, public SymprodArc

{

public:

  SymprodRenderArc(Display *display,
		   const void *prod_buffer,
		   const int obj_offset);
  
  virtual ~SymprodRenderArc() {};

  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// Rectangle objects

class SymprodRenderRectangle
  : public SymprodRenderObj, public SymprodRectangle

{

public:

  SymprodRenderRectangle(Display *display,
			 const void *prod_buffer,
			 const int obj_offset);
  
  virtual ~SymprodRenderRectangle() {};

  virtual void draw(RenderContext *context);

protected:

private:

};

/////////////////////////////////////////////////////////////////////
// Chunk objects

class SymprodRenderChunk
  : public SymprodRenderObj, public SymprodChunk

{

public:

  SymprodRenderChunk(Display *display,
		     const void *prod_buffer,
		     const int obj_offset);
  
  virtual ~SymprodRenderChunk() {};

  virtual void draw(RenderContext *context);

protected:
  
  void _drawStation(station_report_t *station);

  void _drawStationSingle();

  void _drawStationsFromArray();

  int _barbShaftLen;

private:

};

#endif
