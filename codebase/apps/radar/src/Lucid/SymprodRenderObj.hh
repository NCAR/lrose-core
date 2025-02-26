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
// class.
//
// Jan 2000
////////////////////////////////////////////////////////////////

#ifndef SymprodRenderObj_HH
#define SymprodRenderObj_HH

#include <Spdb/SymprodObj.hh>
#include <rapformats/station_reports.h>
#include "Constants.hh"
#include "GlobalData.hh"

class SymprodRender;
class RenderContext;
class Product;

//////////////////////
// Abstract base class

class SymprodRenderObj

{

public:

  SymprodRenderObj(SymprodRender *c);
  
  virtual ~SymprodRenderObj();
  
  virtual void draw(RenderContext &context);  

  virtual double dist(double lat, double lon); 

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

  SymprodRender *container;   // The container that holds this object.

  int objtype;

protected:
  
  GlobalData &gd;
  
  unsigned long _background_pixel;
  bool _background_pixel_init;
  
  QPixmap _stipple;
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

  void _setXColors(char *foreground_color, char *background_color,
		   RenderContext &context);
  
  void _setBackgroundColor(char *color_name, RenderContext &context);
  
  void setStipple(Symprod::fill_t filltype, RenderContext &context);
  
  void _latLon2World(double lat,
		     double lon,
		     double &world_x,
		     double &world_y,
		     RenderContext &context);

  void _latLon2WorldConstrained(double lat,
				double lon,
				double ref_x,
				double &world_x,
				double &world_y,
				RenderContext &context);

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
  
  SymprodRenderText(const void *prod_buffer,
		    const int obj_offset,
		    Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderText() {};
  
  virtual void draw(RenderContext &context);

  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// Polyline objects

class SymprodRenderPolyline
  : public SymprodRenderObj, public SymprodPolyline

{

public:

  SymprodRenderPolyline(const void *prod_buffer,
			const int obj_offset,
			Product &prod, SymprodRender *c);

  virtual ~SymprodRenderPolyline() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);
protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// Iconline objects

class SymprodRenderIconline
  : public SymprodRenderObj, public SymprodIconline

{

public:

  SymprodRenderIconline(const void *prod_buffer,
			const int obj_offset,
			Product &prod, SymprodRender *c);

  virtual ~SymprodRenderIconline() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);
  
protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// StrokedIcon objects

class SymprodRenderStrokedIcon
  : public SymprodRenderObj, public SymprodStrokedIcon

{

public:

  SymprodRenderStrokedIcon( const void *prod_buffer,
			   const int obj_offset,
			   Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderStrokedIcon() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// NamedIcon objects

class SymprodRenderNamedIcon
  : public SymprodRenderObj, public SymprodNamedIcon

{

public:

  SymprodRenderNamedIcon(const void *prod_buffer,
			 const int obj_offset,
			 Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderNamedIcon() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// BitmapIcon objects

class SymprodRenderBitmapIcon
  : public SymprodRenderObj, public SymprodBitmapIcon

{

public:

  SymprodRenderBitmapIcon( const void *prod_buffer,
			  const int obj_offset,
			  Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderBitmapIcon() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// Arc objects

class SymprodRenderArc
  : public SymprodRenderObj, public SymprodArc

{

public:

  SymprodRenderArc(const void *prod_buffer,
		   const int obj_offset,
		   Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderArc() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// Rectangle objects

class SymprodRenderRectangle
  : public SymprodRenderObj, public SymprodRectangle

{

public:

  SymprodRenderRectangle( const void *prod_buffer,
			 const int obj_offset,
			 Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderRectangle() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

};

/////////////////////////////////////////////////////////////////////
// Chunk objects

class SymprodRenderChunk
  : public SymprodRenderObj, public SymprodChunk

{

public:

  SymprodRenderChunk(const void *prod_buffer,
		     const int obj_offset,
		     Product &prod, SymprodRender *c);
  
  virtual ~SymprodRenderChunk() {};

  virtual void draw(RenderContext &context);
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:
  void _drawStation(station_report_t *station, RenderContext &context);

  void _drawStationSingle(RenderContext &context);

  void _drawStationsFromArray(RenderContext &context);

  int _barbShaftLen;

private:
  Product &prod;

};

#endif
