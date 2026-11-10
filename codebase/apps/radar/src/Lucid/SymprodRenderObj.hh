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
#include <QImage>
#include <QPainter>
#include "Constants.hh"
#include "GlobalData.hh"
#include "SymprodRenderContext.hh"

class SymprodRender;
class Product;

//////////////////////
// Abstract base class

class SymprodRenderObj

{

public:

  SymprodRenderObj(SymprodRender *c);
  
  virtual ~SymprodRenderObj();
  
  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;

  virtual double dist(double lat, double lon); 

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

  SymprodRender *container;   // The container that holds this object.

  int objtype;

protected:
  
  Params &_params;
  GlobalData &_gd;
  
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
  
  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;

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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;

  // Map definitions are owned by GlobalData and outlive downloaded products.
  // The lookup is cached because symbolic products are repainted frequently.
  mutable const Geo_feat_icondef_t *_iconDef;
  mutable bool _iconLookupComplete;

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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

protected:

private:
  Product &prod;
  mutable QImage _bitmapImage;

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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
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

  virtual void drawQt(QPainter &painter,
                      const SymprodRenderContext &context) const;
  
  virtual double dist(double lat, double lon);

  virtual void fill_export_fields( vector<world_pt_t> &wpt, string &label);

private:
  void _drawStationQt(const station_report_t &station,
                      QPainter &painter,
                      const SymprodRenderContext &context) const;

  Product &prod;

};

#endif
