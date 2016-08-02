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
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Jan 2000
//
//////////////////////////////////////////////////////////

#include "SymprodRender.hh"
#include "SymprodRenderObj.hh"
#include <dataport/bigend.h>
using namespace std;

//////////////////////////////////////////
// default constructor

SymprodRender::SymprodRender(Display *display)
  : Symprod(),
    _display(display)

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
  _objs.clear();
  _objTypes.erase(_objTypes.begin(), _objTypes.end());
}

//////////////////////////////////
// de-serialize - load from buffer
//
// returns 0 on success, -1 on failure

int SymprodRender::deserialize(void *in_buf, int buf_len /* = -1*/ )

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
      obj = new SymprodRenderText(_display, in_buf, offset);
      break;
    case Symprod::OBJ_POLYLINE:
      obj = new SymprodRenderPolyline(_display, in_buf, offset);
      break;
    case Symprod::OBJ_STROKED_ICON:
      obj = new SymprodRenderStrokedIcon(_display, in_buf, offset);
      break;
    case Symprod::OBJ_ICONLINE:
      obj = new SymprodRenderIconline(_display, in_buf, offset);
      break;
    case Symprod::OBJ_NAMED_ICON:
      obj = new SymprodRenderNamedIcon(_display, in_buf, offset);
      break;
    case Symprod::OBJ_BITMAP_ICON:
      obj = new SymprodRenderBitmapIcon(_display, in_buf, offset);
      break;
    case Symprod::OBJ_ARC:
      obj = new SymprodRenderArc(_display, in_buf, offset);
      break;
    case Symprod::OBJ_RECTANGLE:
      obj = new SymprodRenderRectangle(_display, in_buf, offset);
      break;
    case Symprod::OBJ_CHUNK:
      obj = new SymprodRenderChunk(_display, in_buf, offset);
      break;
    }
    
    _objs.push_back(obj);
    _objTypes.push_back(objType);

  } // i

  return 0;

}

//////////////////////////////////
// draw the product

void SymprodRender::draw(RenderContext *context)
  
{
  
  for (size_t i = 0; i < _objs.size(); i++) {
    _objs[i]->draw(context);
  }

}


  
