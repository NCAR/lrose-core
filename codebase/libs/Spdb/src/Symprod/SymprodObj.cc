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
// SymprodObj.cc
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

#include <Spdb/SymprodObj.hh>
#include <dataport/bigend.h>
#include <cstring>
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// Abstract base class

SymprodObj::SymprodObj(const void *prod_buffer, int obj_offset)

{
  char *ptr = (char *) prod_buffer + obj_offset;
  memcpy(&_hdr, ptr, sizeof(Symprod::obj_hdr_t));
  Symprod::objHdrFromBE(&_hdr);
}

SymprodObj::SymprodObj(const Symprod::obj_hdr_t &hdr)

{
  _hdr = hdr;
}

void SymprodObj::loadBuf(MemBuf &buf)

{
  Symprod::obj_hdr_t hdr = _hdr;
  Symprod::objHdrToBE(&hdr);
  buf.add(&hdr, sizeof(hdr));
}

//////////////////////////////////////////////////////////////////////////////
// Text objects

SymprodText::SymprodText(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::text_props_t));
  Symprod::textFromBE(&_props);
  ptr += sizeof(Symprod::text_props_t);
  _text = ptr;
}

SymprodText::SymprodText(const Symprod::obj_hdr_t &hdr,
			 const Symprod::text_props_t &props,
			 const string &text)
  : SymprodObj(hdr)

{
  _props = props;
  _text = text;
}

void SymprodText::print(ostream &out)

{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printTextProps(out, _props);
  out << "   text = <" << _text << ">" << endl;
  out << endl;
}

void SymprodText::loadBuf(MemBuf &buf)

{

  SymprodObj::loadBuf(buf);

  Symprod::text_props_t props = _props;
  Symprod::textToBE(&props);
  buf.add(&props, sizeof(props));

  buf.add(_text.c_str(), _text.size() + 1);

}

//////////////////////////////////////////////////////////////////////////////
// Polyline objects

SymprodPolyline::SymprodPolyline(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::polyline_props_t));
  Symprod::polylineFromBE(&_props);
  ptr += sizeof(Symprod::polyline_props_t);
  int nbytesPoly = _props.num_points * sizeof(Symprod::wpt_t);
  _pointsBuf.free();
  _points = (Symprod::wpt_t *) _pointsBuf.reserve(nbytesPoly);
  memcpy(_points, ptr, nbytesPoly);
  BE_to_array_32(_points, nbytesPoly);
}

SymprodPolyline::SymprodPolyline(const Symprod::obj_hdr_t &hdr,
				 const Symprod::polyline_props_t &props,
				 const Symprod::wpt_t *points)
  : SymprodObj(hdr)
{
  _props = props;
  int nbytesPoly = _props.num_points * sizeof(Symprod::wpt_t);
  _pointsBuf.free();
  _points = (Symprod::wpt_t *) _pointsBuf.reserve(nbytesPoly);
  memcpy(_points, points, nbytesPoly);
}

void SymprodPolyline::print(ostream &out)

{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printPolylineProps(out, _props);
  out << endl;
  Symprod::printPolylinePtsArray(out, _props.num_points, _points);
}

void SymprodPolyline::loadBuf(MemBuf &buf)

{

  SymprodObj::loadBuf(buf);

  Symprod::polyline_props_t props = _props;
  Symprod::polylineToBE(&props);
  buf.add(&props, sizeof(props));

  for (int i = 0; i < _props.num_points; i++) {
    Symprod::wpt_t wpt = _points[i];
    Symprod::wptToBE(&wpt);
    buf.add(&wpt, sizeof(wpt));
  }

}

//////////////////////////////////////////////////////////////////////////////
// Iconline objects

SymprodIconline::SymprodIconline(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::iconline_props_t));
  Symprod::iconlineFromBE(&_props);
  ptr += sizeof(Symprod::iconline_props_t);
  int nbytesIcon = _props.num_points * sizeof(Symprod::ppt_t);
  _pointsBuf.free();
  _points = (Symprod::ppt_t *) _pointsBuf.reserve(nbytesIcon);
  memcpy(_points, ptr, nbytesIcon);
  BE_to_array_32(_points, nbytesIcon);
}

SymprodIconline::SymprodIconline(const Symprod::obj_hdr_t &hdr,
				 const Symprod::iconline_props_t &props,
				 const Symprod::ppt_t *points)
  : SymprodObj(hdr)
{
  _props = props;
  int nbytesIcon = _props.num_points * sizeof(Symprod::ppt_t);
  _pointsBuf.free();
  _points = (Symprod::ppt_t *) _pointsBuf.reserve(nbytesIcon);
  memcpy(_points, points, nbytesIcon);
}

void SymprodIconline::print(ostream &out)

{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printIconlineProps(out, _props);
  out << endl;
  Symprod::printIconlinePtsArray(out, _props.num_points, _points);
}

void SymprodIconline::loadBuf(MemBuf &buf)

{

  SymprodObj::loadBuf(buf);

  Symprod::iconline_props_t props = _props;
  Symprod::iconlineToBE(&props);
  buf.add(&props, sizeof(props));

  for (int i = 0; i < _props.num_points; i++) {
    Symprod::ppt_t ppt = _points[i];
    Symprod::pptToBE(&ppt);
    buf.add(&ppt, sizeof(ppt));
  }

}

//////////////////////////////////////////////////////////////////////////////
// StrokedIcon objects

SymprodStrokedIcon::SymprodStrokedIcon(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{

  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::stroked_icon_props_t));
  Symprod::strokedIconFromBE(&_props);
  ptr += sizeof(Symprod::stroked_icon_props_t);

  int nbytesPts = _props.num_icon_pts * sizeof(Symprod::ppt_t);
  _iconPtsBuf.free();
  _iconPts = (Symprod::ppt_t *) _iconPtsBuf.reserve(nbytesPts);
  memcpy(_iconPts, ptr, nbytesPts);
  BE_to_array_32(_iconPts, nbytesPts);
  ptr += nbytesPts;

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, ptr, nbytesOrigins);
  BE_to_array_32(_iconOrigins, nbytesOrigins);

}

SymprodStrokedIcon::
SymprodStrokedIcon(const Symprod::obj_hdr_t &hdr,
		   const Symprod::stroked_icon_props_t &props,
		   const Symprod::ppt_t *icon_pts,
		   const Symprod::wpt_t *icon_origins)
  : SymprodObj(hdr)

{
  _props = props;

  int nbytesPts = _props.num_icon_pts * sizeof(Symprod::ppt_t);
  _iconPtsBuf.free();
  _iconPts = (Symprod::ppt_t *) _iconPtsBuf.reserve(nbytesPts);
  memcpy(_iconPts, icon_pts, nbytesPts);

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, icon_origins, nbytesOrigins);
}


void SymprodStrokedIcon::print(ostream &out)
  
{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printStrokedIconProps(out, _props);
  out << endl;
  Symprod::printIconPoints(out, _props.num_icon_pts, _iconPts);
  out << endl;
  Symprod::printIconOrigins(out, _props.num_icons, _iconOrigins);
}

void SymprodStrokedIcon::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::stroked_icon_props_t props = _props;
  Symprod::strokedIconToBE(&props);
  buf.add(&props, sizeof(props));

  for (int i = 0; i < _props.num_icon_pts; i++) {
    Symprod::ppt_t ppt = _iconPts[i];
    Symprod::pptToBE(&ppt);
    buf.add(&ppt, sizeof(ppt));
  }

  for (int i = 0; i < _props.num_icons; i++) {
    Symprod::wpt_t wpt = _iconOrigins[i];
    Symprod::wptToBE(&wpt);
    buf.add(&wpt, sizeof(wpt));
  }

}

//////////////////////////////////////////////////////////////////////////////
// NamedIcon objects

SymprodNamedIcon::SymprodNamedIcon(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::named_icon_props_t));
  Symprod::namedIconFromBE(&_props);
  ptr += sizeof(Symprod::named_icon_props_t);

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, ptr, nbytesOrigins);
  BE_to_array_32(_iconOrigins, nbytesOrigins);

}

SymprodNamedIcon::
SymprodNamedIcon(const Symprod::obj_hdr_t &hdr,
		 const Symprod::named_icon_props_t &props,
		 const Symprod::wpt_t *icon_origins)
  : SymprodObj(hdr)
  
{
  _props = props;

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, icon_origins, nbytesOrigins);
}


void SymprodNamedIcon::print(ostream &out)
  
{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printNamedIconProps(out, _props);
  out << endl;
  Symprod::printIconOrigins(out, _props.num_icons, _iconOrigins);
}

void SymprodNamedIcon::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::named_icon_props_t props = _props;
  Symprod::namedIconToBE(&props);
  buf.add(&props, sizeof(props));

  for (int i = 0; i < _props.num_icons; i++) {
    Symprod::wpt_t wpt = _iconOrigins[i];
    Symprod::wptToBE(&wpt);
    buf.add(&wpt, sizeof(wpt));
  }

}

//////////////////////////////////////////////////////////////////////////////
// BitmapIcon objects

SymprodBitmapIcon::SymprodBitmapIcon(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{

  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::bitmap_icon_props_t));
  Symprod::bitmapIconFromBE(&_props);
  ptr += sizeof(Symprod::bitmap_icon_props_t);

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, ptr, nbytesOrigins);
  BE_to_array_32(_iconOrigins, nbytesOrigins);
  ptr += nbytesOrigins;

  int nbytesBitmap = _props.bitmap_x_dim * _props.bitmap_y_dim * sizeof(ui08);
  _bitmapBuf.free();
  _bitmap = (ui08 *) _bitmapBuf.reserve(nbytesBitmap);
  memcpy(_bitmap, ptr, nbytesBitmap);

}

SymprodBitmapIcon::SymprodBitmapIcon(const Symprod::obj_hdr_t &hdr,
				     const Symprod::bitmap_icon_props_t &props,
				     const Symprod::wpt_t *icon_origins,
				     const ui08 *bitmap)
  : SymprodObj(hdr)

{

  _props = props;

  int nbytesOrigins = _props.num_icons * sizeof(Symprod::wpt_t);
  _iconOriginsBuf.free();
  _iconOrigins = (Symprod::wpt_t *) _iconOriginsBuf.reserve(nbytesOrigins);
  memcpy(_iconOrigins, icon_origins, nbytesOrigins);

  int nbytesBitmap = _props.bitmap_x_dim * _props.bitmap_y_dim * sizeof(ui08);
  _bitmapBuf.free();
  _bitmap = (ui08 *) _bitmapBuf.reserve(nbytesBitmap);
  memcpy(_bitmap, bitmap, nbytesBitmap);

}
  
void SymprodBitmapIcon::print(ostream &out)
  
{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printBitmapIconProps(out, _props);
  out << endl;
  Symprod::printIconOrigins(out, _props.num_icons, _iconOrigins);
  out << endl;
  Symprod::printBitmap(out, _props.bitmap_x_dim, _props.bitmap_y_dim, _bitmap);
}

void SymprodBitmapIcon::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::bitmap_icon_props_t props = _props;
  Symprod::bitmapIconToBE(&props);
  buf.add(&props, sizeof(props));

  for (int i = 0; i < _props.num_icons; i++) {
    Symprod::wpt_t wpt = _iconOrigins[i];
    Symprod::wptToBE(&wpt);
    buf.add(&wpt, sizeof(wpt));
  }

  buf.add(_bitmap,
	  _props.bitmap_x_dim * _props.bitmap_y_dim * sizeof(ui08));

}

//////////////////////////////////////////////////////////////////////////////
// Arc objects

SymprodArc::SymprodArc(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::arc_props_t));
  Symprod::arcFromBE(&_props);
}

SymprodArc::SymprodArc(const Symprod::obj_hdr_t &hdr,
		       const Symprod::arc_props_t &props)
  : SymprodObj(hdr)
{
  _props = props;
}


void SymprodArc::print(ostream &out)
  
{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printArcProps(out,  _props);
}

void SymprodArc::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::arc_props_t props = _props;
  Symprod::arcToBE(&props);
  buf.add(&props, sizeof(props));

}

//////////////////////////////////////////////////////////////////////////////
// Rectangle objects

SymprodRectangle::SymprodRectangle(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)
  
{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::rectangle_props_t));
  Symprod::rectangleFromBE(&_props);
}

SymprodRectangle::SymprodRectangle(const Symprod::obj_hdr_t &hdr,
				   const Symprod::rectangle_props_t &props)
  : SymprodObj(hdr)
{
  _props = props;
}


void SymprodRectangle::print(ostream &out)
  
{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printRectangleProps(out,  _props);
}

void SymprodRectangle::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::rectangle_props_t props = _props;
  Symprod::rectangleToBE(&props);
  buf.add(&props, sizeof(props));

}

//////////////////////////////////////////////////////////////////////////////
// Chunk objects

SymprodChunk::SymprodChunk(const void *prod_buffer, int obj_offset)
  : SymprodObj(prod_buffer, obj_offset)

{
  char *ptr = (char *) prod_buffer + obj_offset + sizeof(Symprod::obj_hdr_t);
  memcpy(&_props, ptr, sizeof(Symprod::chunk_props_t));
  Symprod::chunkFromBE(&_props);
  ptr += sizeof(Symprod::chunk_props_t);
  
  int nbytesChunk = _props.nbytes_chunk;
  _dataBuf.free();
  _data = (ui08 *) _dataBuf.reserve(nbytesChunk);
  memcpy(_data, ptr, nbytesChunk);

}

SymprodChunk::SymprodChunk(const Symprod::obj_hdr_t &hdr,
			   const Symprod::chunk_props_t &props,
			   const void *data)
  : SymprodObj(hdr)

{
  _props = props;
  
  int nbytesChunk = _props.nbytes_chunk;
  _dataBuf.free();
  _data = (ui08 *) _dataBuf.reserve(nbytesChunk);
  memcpy(_data, data, nbytesChunk);
}

void SymprodChunk::print(ostream &out)

{
  Symprod::printObjHdr(out, _hdr);
  Symprod::printChunkProps(out,  _props);
}

void SymprodChunk::loadBuf(MemBuf &buf)
  
{
  
  SymprodObj::loadBuf(buf);

  Symprod::chunk_props_t props = _props;
  Symprod::chunkToBE(&props);
  buf.add(&props, sizeof(props));

  buf.add(_data, _props.nbytes_chunk);

}


