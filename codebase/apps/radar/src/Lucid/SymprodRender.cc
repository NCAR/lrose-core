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
#include <dataport/bigend.h>

//////////////////////////////////////////
// default constructor

SymprodRender::SymprodRender(Params &params,
                             Product &p) :
        Symprod(),
        _params(params),
        _gd(GlobalData::Instance()),
        prod(p)

{
}

//////////////
// destructor
  
SymprodRender::~SymprodRender()
{
  _freeObjs();
}

/////////////////////
// clear all products
  
void SymprodRender::clear()
{
  _freeObjs();
  Symprod::clear();

}
  
///////////////
// free objects

void SymprodRender::_freeObjs()

{
  for (size_t i = 0; i < _objs.size(); i++) {
    delete _objs[i];
  }
  _objs.erase(_objs.begin(), _objs.end());
  _objTypes.erase(_objTypes.begin(), _objTypes.end());
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
  
  si32 *beOffsets = (si32 *)
    ((char *) in_buf + sizeof(prod_hdr_props_t));
  
  for (int i = 0; i < _prodProps.num_objs; i++) {
    
    int offset = BE_to_si32(beOffsets[i]);
    obj_hdr_t *hdr = (obj_hdr_t *) ((char *) in_buf + offset);
    int objType = BE_to_si32(hdr->object_type);
    int objLen = sizeof(obj_hdr_t) + BE_to_si32(hdr->num_bytes);
    _objDataType = BE_to_si32(hdr->object_id);

    if (buf_len >= 0 && buf_len < (offset + objLen)) {
      _errStr += "ERROR - SymprodRender::deserialize\n";
      _errStr += "  Bad input buffer - too short.\n";
      _addIntErr("  Buf len: ", buf_len);
      clear();
      return -1;
    }
    
    SymprodRenderObj *obj;
    switch (objType) {
    case Symprod::OBJ_TEXT:
      obj = new SymprodRenderText(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_POLYLINE:
      obj = new SymprodRenderPolyline(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_ICONLINE:
      obj = new SymprodRenderIconline(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_STROKED_ICON:
      obj = new SymprodRenderStrokedIcon(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_NAMED_ICON:
      obj = new SymprodRenderNamedIcon(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_BITMAP_ICON:
      obj = new SymprodRenderBitmapIcon(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_ARC:
      obj = new SymprodRenderArc(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_RECTANGLE:
      obj = new SymprodRenderRectangle(_params, in_buf, offset, prod, this);
      break;
    case Symprod::OBJ_CHUNK:
      obj = new SymprodRenderChunk(_params, in_buf, offset, prod, this);
      break;
    }
    
    _objs.push_back(obj);
    _objTypes.push_back(objType);

  } // i

  return 0;

}


//////////////////////////////////
// draw the product

void SymprodRender::draw(RenderContext &context)
  
{
  char msg1[128];
  char msg2[256];

  strftime(msg1,128,"%b %d %H:%M:%S", gmtime((time_t *) &(_prodProps.start_time)));
  snprintf(msg2,256,"Product: %s - %d objects, %s",
	      _prodProps.label,
	      _prodProps.num_objs,
	      msg1);

  // add_message_to_status_win(msg2,0);
  
  for (size_t i = 0; i < _objs.size(); i++) {
    // if(!_gd.io_info.busy_status) _objs[i]->draw(context);
    _objs[i]->draw(context);
  }

}

//////////////////////////////////
// Pick the closest Object out of the set of objects

double SymprodRender::pick_closest_obj(double lat, double lon)
  
{
  double min_dist = 999999.9;
  double dist;

  for (size_t i = 0; i < _objs.size(); i++) {
     dist =  _objs[i]->dist(lat,lon);
	 if(dist < min_dist) {
		 closest_obj = _objs[i];
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
