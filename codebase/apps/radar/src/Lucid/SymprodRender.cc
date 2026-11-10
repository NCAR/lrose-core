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
// SymprodRender.cc
//
// Symbolic product class
//
// Jan 2000
//////////////////////////////////////////////////////////

#include "SymprodRender.hh"
#include "SymprodRenderObj.hh"
#include "SymprodRenderContext.hh"
#include <dataport/bigend.h>
#include <limits>
#include <memory>

//////////////////////////////////////////
// default constructor

SymprodRender::SymprodRender(Product &p) :
        Symprod(),
        _objDataType(0),
        prod(p),
        closest_obj(nullptr)

{
}

//////////////
// destructor
  
SymprodRender::~SymprodRender()
  = default;

/////////////////////
// clear all products
  
void SymprodRender::clear()
{
  closest_obj = nullptr;
  _objs.clear();
  _objTypes.clear();
  Symprod::clear();

}

//////////////////////////////////
// de-serialize - load from buffer
//
// returns 0 on success, -1 on failure

int SymprodRender::deserialize(void *in_buf, int buf_len /* = -1 */)

{

  clear(); 
  _clearErrStr();
  
  if (buf_len >= 0 && buf_len < (int) sizeof(prod_hdr_props_t)) {
    _errStr += "ERROR - SymprodRender::deserialize\n";
    _errStr += "  Bad input buffer - too short.\n";
    _addIntErr("  Buf len: ", buf_len);
    return -1;
  }
  
  memcpy(&_prodProps, in_buf, sizeof(prod_hdr_props_t));
  prodHdrFromBE(&_prodProps);

  if (_prodProps.num_objs < 0 ||
      static_cast<size_t>(_prodProps.num_objs) >
        (std::numeric_limits<size_t>::max() - sizeof(prod_hdr_props_t)) /
          sizeof(si32)) {
    _errStr += "ERROR - SymprodRender::deserialize\n";
    _errStr += "  Invalid object count.\n";
    return -1;
  }
  const size_t offsetTableEnd = sizeof(prod_hdr_props_t) +
    static_cast<size_t>(_prodProps.num_objs) * sizeof(si32);
  if (buf_len >= 0 && offsetTableEnd > static_cast<size_t>(buf_len)) {
    _errStr += "ERROR - SymprodRender::deserialize\n";
    _errStr += "  Object offset table exceeds input buffer.\n";
    return -1;
  }
  
  si32 *beOffsets = (si32 *)
    ((char *) in_buf + sizeof(prod_hdr_props_t));
  
  for (int i = 0; i < _prodProps.num_objs; i++) {
    
    const int offset = BE_to_si32(beOffsets[i]);
    if (offset < static_cast<int>(offsetTableEnd) ||
        (buf_len >= 0 &&
         static_cast<size_t>(offset) + sizeof(obj_hdr_t) >
           static_cast<size_t>(buf_len))) {
      _errStr += "ERROR - SymprodRender::deserialize\n";
      _errStr += "  Invalid object offset.\n";
      _addIntErr("  Object offset: ", offset);
      clear();
      return -1;
    }
    obj_hdr_t *hdr = (obj_hdr_t *) ((char *) in_buf + offset);
    int objType = BE_to_si32(hdr->object_type);
    const int payloadLen = BE_to_si32(hdr->num_bytes);
    if (payloadLen < 0 ||
        payloadLen > std::numeric_limits<int>::max() -
                       static_cast<int>(sizeof(obj_hdr_t))) {
      _errStr += "ERROR - SymprodRender::deserialize\n";
      _errStr += "  Invalid object length.\n";
      clear();
      return -1;
    }
    const int objLen = sizeof(obj_hdr_t) + payloadLen;
    _objDataType = BE_to_si32(hdr->object_id);

    if (buf_len >= 0 && buf_len < (offset + objLen)) {
      _errStr += "ERROR - SymprodRender::deserialize\n";
      _errStr += "  Bad input buffer - too short.\n";
      _addIntErr("  Buf len: ", buf_len);
      clear();
      return -1;
    }
    
    unique_ptr<SymprodRenderObj> obj;
    switch (objType) {
    case Symprod::OBJ_TEXT:
      obj = make_unique<SymprodRenderText>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_POLYLINE:
      obj = make_unique<SymprodRenderPolyline>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_ICONLINE:
      obj = make_unique<SymprodRenderIconline>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_STROKED_ICON:
      obj = make_unique<SymprodRenderStrokedIcon>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_NAMED_ICON:
      obj = make_unique<SymprodRenderNamedIcon>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_BITMAP_ICON:
      obj = make_unique<SymprodRenderBitmapIcon>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_ARC:
      obj = make_unique<SymprodRenderArc>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_RECTANGLE:
      obj = make_unique<SymprodRenderRectangle>(in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_CHUNK:
      obj = make_unique<SymprodRenderChunk>(in_buf, offset, prod, this);
      break;
    default:
      _errStr += "ERROR - SymprodRender::deserialize\n";
      _errStr += "  Unsupported symbolic product object type.\n";
      _addIntErr("  Object type: ", objType);
      clear();
      return -1;
    }
    
    _objs.push_back(std::move(obj));
    _objTypes.push_back(objType);

  } // i

  return 0;

}


void SymprodRender::drawQt(QPainter &painter,
                           const SymprodRenderContext &context) const
{
  for (size_t ii = 0; ii < _objs.size(); ++ii) {
    _objs[ii]->drawQt(painter, context);
  }
}

//////////////////////////////////
// Pick the closest Object out of the set of objects

double SymprodRender::pick_closest_obj(double lat, double lon)
  
{
  closest_obj = nullptr;
  double min_dist = 999999.9;
  double dist;

  for (size_t i = 0; i < _objs.size(); i++) {
     dist =  _objs[i]->dist(lat,lon);
	 if(dist < min_dist) {
		 closest_obj = _objs[i].get();
		 min_dist = dist;
	 }
  }

  return min_dist;
}

//////////////////////////////////
// Fill in points and labels for export.

void SymprodRender::fill_export_fields(vector<world_pt_t> &wpt, string &label)
{
  for (size_t i = 0; i < _objs.size(); i++) {
      _objs[i]->fill_export_fields(wpt,label);
  }
}
