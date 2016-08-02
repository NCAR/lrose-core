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
// Spdb/SymprodObj.hh
//
// Symprod objects
//
// These classes provide handling routines for the various
// types of symprod objects used for rendering.
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
////////////////////////////////////////////////////////////////

#ifndef SymprodObj_HH
#define SymprodObj_HH

#include <Spdb/Symprod.hh>
#include <toolsa/MemBuf.hh>
#include <iostream>
#include <string>
using namespace std;

/////////////////////////////////////////
// Abstract base class

class SymprodObj

{

public:

  SymprodObj(const void *prod_buffer, int obj_offset);

  SymprodObj(const Symprod::obj_hdr_t &hdr);

  virtual ~SymprodObj() {}; 

  virtual void print(ostream &out) = 0;  // forces object to be abstract

  virtual void loadBuf(MemBuf &buf);

  int getBaseType() { return (_hdr.object_type); }

  virtual int getType() = 0;

protected:

  Symprod::obj_hdr_t _hdr;

private:

};

/////////////////////////////////////////
// Text objects

class SymprodText : public SymprodObj

{

public:

  SymprodText(const void *prod_buffer, int obj_offset);

  SymprodText(const Symprod::obj_hdr_t &hdr,
	      const Symprod::text_props_t &props,
	      const string &text);

  virtual ~SymprodText() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::text_props_t _props;
  string _text;

private:

};

/////////////////////////////////////////
// Polyline objects

class SymprodPolyline : public SymprodObj

{

public:

  SymprodPolyline(const void *prod_buffer, int obj_offset);

  SymprodPolyline(const Symprod::obj_hdr_t &hdr,
		  const Symprod::polyline_props_t &props,
		  const Symprod::wpt_t *points);

  virtual ~SymprodPolyline() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::polyline_props_t _props;
  Symprod::wpt_t *_points;
  MemBuf _pointsBuf;

private:

};

/////////////////////////////////////////
// Iconline objects

class SymprodIconline : public SymprodObj

{

public:

  SymprodIconline(const void *prod_buffer, int obj_offset);

  SymprodIconline(const Symprod::obj_hdr_t &hdr,
		  const Symprod::iconline_props_t &props,
		  const Symprod::ppt_t *points);

  virtual ~SymprodIconline() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::iconline_props_t _props;
  Symprod::ppt_t *_points;
  MemBuf _pointsBuf;

private:

};

/////////////////////////////////////////
// StrokedIcon objects

class SymprodStrokedIcon : public SymprodObj

{

public:

  SymprodStrokedIcon(const void *prod_buffer, int obj_offset);

  SymprodStrokedIcon(const Symprod::obj_hdr_t &hdr,
		     const Symprod::stroked_icon_props_t &props,
		     const Symprod::ppt_t *icon_pts,
		     const Symprod::wpt_t *icon_origins);

  virtual ~SymprodStrokedIcon() {};

  virtual void print(ostream &out);
  
  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::stroked_icon_props_t _props;
  Symprod::ppt_t *_iconPts;
  Symprod::wpt_t *_iconOrigins;
  MemBuf _iconPtsBuf;
  MemBuf _iconOriginsBuf;

private:

};

/////////////////////////////////////////
// NamedIcon objects

class SymprodNamedIcon : public SymprodObj

{

public:

  SymprodNamedIcon(const void *prod_buffer, int obj_offset);

  SymprodNamedIcon(const Symprod::obj_hdr_t &hdr,
		   const Symprod::named_icon_props_t &props,
		   const Symprod::wpt_t *icon_origins);

  virtual ~SymprodNamedIcon() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::named_icon_props_t _props;
  Symprod::wpt_t *_iconOrigins;
  MemBuf _iconOriginsBuf;
  

private:

};

/////////////////////////////////////////
// BitmapIcon objects

class SymprodBitmapIcon : public SymprodObj

{

public:

  SymprodBitmapIcon(const void *prod_buffer, int obj_offset);

  SymprodBitmapIcon(const Symprod::obj_hdr_t &hdr,
		    const Symprod::bitmap_icon_props_t &props,
		    const Symprod::wpt_t *icon_origins,
		    const ui08 *bitmap);
  
  virtual ~SymprodBitmapIcon() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::bitmap_icon_props_t _props;
  Symprod::wpt_t *_iconOrigins;
  ui08 *_bitmap;
  MemBuf _iconOriginsBuf;
  MemBuf _bitmapBuf;

private:

};

/////////////////////////////////////////
// Arc objects

class SymprodArc : public SymprodObj

{

public:

  SymprodArc(const void *prod_buffer, int obj_offset);

  SymprodArc(const Symprod::obj_hdr_t &hdr,
	     const Symprod::arc_props_t &props);

  virtual ~SymprodArc() {};

  virtual void print(ostream &out);
  
  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::arc_props_t _props;

private:

};

/////////////////////////////////////////
// Rectangle objects

class SymprodRectangle : public SymprodObj

{

public:

  SymprodRectangle(const void *prod_buffer, int obj_offset);

  SymprodRectangle(const Symprod::obj_hdr_t &hdr,
		   const Symprod::rectangle_props_t &props);

  virtual ~SymprodRectangle() {};

  virtual void print(ostream &out);
  
  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::rectangle_props_t _props;

private:

};

/////////////////////////////////////////
// Chunk objects

class SymprodChunk : public SymprodObj

{

public:

  SymprodChunk(const void *prod_buffer, int obj_offset);

  SymprodChunk(const Symprod::obj_hdr_t &hdr,
	       const Symprod::chunk_props_t &props,
	       const void *data);

  virtual ~SymprodChunk() {};

  virtual void print(ostream &out);

  virtual int getType() { return (_hdr.object_type); }

  virtual void loadBuf(MemBuf &buf);

protected:

  Symprod::chunk_props_t _props;
  void *_data;
  MemBuf _dataBuf;
 
  
private:

};

#endif
