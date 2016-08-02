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
// Symprod.cc
//
// Symbolic product class
//
// Mike Dixon, from Nancy Rehak
// RAP, NCAR, Boulder, CO, 80307, USA
//
// Dec 1999
//
//////////////////////////////////////////////////////////

#include <Spdb/Symprod.hh>
#include <Spdb/SymprodObj.hh>
#include <dataport/bigend.h>
#include <toolsa/udatetime.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
using namespace std;

const si32 Symprod::PPT_PENUP = 0x7FFFFFFF;
const fl32 Symprod::WPT_PENUP = -9999.9;

//////////////////////////////////////////
// default constructor

Symprod::Symprod(const time_t generate_time /* = 0*/,
		 const time_t received_time /* = 0*/,
		 const time_t start_time /* = 0*/,
		 const time_t expire_time /* = 0*/,
		 const int data_type /* = 0*/,
		 const int data_type2 /* = 0*/,
		 const char *label /* = NULL*/ )

{
  setTimes(generate_time, received_time, start_time, expire_time);
  setDataTypes(data_type, data_type2);
  setLabel(label);
  initBbox(_prodProps.bounding_box);
  _prodProps.num_objs = 0;
}

//////////////
// destructor
  
Symprod::~Symprod()
{
  _freeObjs();
}

/////////////////////
// clear all products
  
void Symprod::clear()
{
  _freeObjs();
  initBbox(_prodProps.bounding_box);
  _prodProps.num_objs = 0;
}
  
///////////////
// free objects

void Symprod::_freeObjs()

{
  for (size_t i = 0; i < _objs.size(); i++) {
    delete _objs[i];
  }
  _objs.erase(_objs.begin(), _objs.end());
  _objTypes.erase(_objTypes.begin(), _objTypes.end());
}

//////////////////////////////////////////
// set  times

void Symprod::setTimes(const time_t generate_time /* = 0*/,
		       const time_t received_time /* = 0*/,
		       const time_t start_time /* = 0*/,
		       const time_t expire_time /* = 0*/ )

{
  _prodProps.generate_time = generate_time;
  _prodProps.received_time = received_time;
  _prodProps.start_time = start_time;
  _prodProps.expire_time = expire_time;
}

//////////////////////////////////////////
// set data types

void Symprod::setDataTypes(const int data_type /* = 0*/,
			   const int data_type2 /* = 0*/ )

{
  _prodProps.data_type = data_type;
  _prodProps.data_type2 = data_type2;
}

void Symprod::setExpireTime(const time_t expire_time /* = 0*/ )
  
{
  _prodProps.expire_time = expire_time;
}

//////////////////////////////////////////
// set label

void Symprod::setLabel(const char *label /* = NULL*/ )
{
  MEM_zero(_prodProps.label);
  if (label != NULL) {
    STRncopy(_prodProps.label, label, SYMPROD_LABEL_LEN);
  }
}

//////////////////////////////////////////////////////////////////////
// Symprod::initBbox()
//
// Initialize bounding box
//

void Symprod::initBbox(bbox_t &bb)

{
  bb.min_lat = 360.0;
  bb.max_lat = -360.0;
  bb.min_lon = 720.0;
  bb.max_lon = -720.0;
}

///////////////////////////////////////////////////////////////////////
// updateBbox()
// Update a bounding box given a point

void Symprod::updateBbox(bbox_t &bb, double lat, double lon)
  
{

  if (lat < bb.min_lat) {
    bb.min_lat = lat;
  }

  if (lat > bb.max_lat) {
    bb.max_lat = lat;
  }

  if (lon < bb.min_lon) {
    bb.min_lon = lon;
  }

  if (lon > bb.max_lon) {
    bb.max_lon = lon;
  }

}

/////////////////////////////////////////////////////////////////////////
// updateBbox()
// Update one bounding box with another

void Symprod::updateBbox(bbox_t &bb, const bbox_t &template_bb)

{

  if (template_bb.min_lat < bb.min_lat) {
    bb.min_lat = template_bb.min_lat;
  }

  if (template_bb.max_lat > bb.max_lat) {
    bb.max_lat = template_bb.max_lat;
  }

  if (template_bb.min_lon < bb.min_lon) {
    bb.min_lon = template_bb.min_lon;
  }

  if (template_bb.max_lon > bb.max_lon) {
    bb.max_lon = template_bb.max_lon;
  }

}

//////////////////////////////////
// de-serialize - load from buffer
//
// returns 0 on success, -1 on failure

int Symprod::deserialize(void *in_buf, int buf_len /* = -1*/ )

{
  
  clear();
  _clearErrStr();
  
  if (buf_len >= 0 && buf_len < (int) sizeof(prod_hdr_props_t)) {
    _errStr += "ERROR - Symprod::deserialize\n";
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
      _errStr += "ERROR - Symprod::deserialize\n";
      _errStr += "  Bad input buffer - too short.\n";
      _addIntErr("  Buf len: ", buf_len);
      return -1;
    }
    
    SymprodObj *obj;
    switch (objType) {
    case Symprod::OBJ_TEXT:
      obj = new SymprodText(in_buf, offset);
      break;
    case Symprod::OBJ_POLYLINE:
      obj = new SymprodPolyline(in_buf, offset);
      break;
    case Symprod::OBJ_ICONLINE:
      obj = new SymprodIconline(in_buf, offset);
      break;
    case Symprod::OBJ_STROKED_ICON:
      obj = new SymprodStrokedIcon(in_buf, offset);
      break;
    case Symprod::OBJ_NAMED_ICON:
      obj = new SymprodNamedIcon(in_buf, offset);
      break;
    case Symprod::OBJ_BITMAP_ICON:
      obj = new SymprodBitmapIcon(in_buf, offset);
      break;
    case Symprod::OBJ_ARC:
      obj = new SymprodArc(in_buf, offset);
      break;
    case Symprod::OBJ_RECTANGLE:
      obj = new SymprodRectangle(in_buf, offset);
      break;
    case Symprod::OBJ_CHUNK:
      obj = new SymprodChunk(in_buf, offset);
      break;
    }
    
    _objs.push_back(obj);
    _objTypes.push_back(objType);

  } // i

  return 0;

}

///////////////////////////////////////////////////////////////////////
// Convert internal format to product buffer.
// Byte swapping is performed as necessary.

void Symprod::serialize(MemBuf &out_buf)

{

  // check number of objects

  if (_prodProps.num_objs != (int) _objs.size()) {
    cerr << "WARNING - incorrect internal count on num_objs." << endl;
    cerr << "  Vector size: " << _objs.size() << endl;
    cerr << "  Internal count: " << _prodProps.num_objs << endl;
    _prodProps.num_objs = _objs.size();
  }
  
  // add product property header

  out_buf.free();
  prod_hdr_props_t props = _prodProps;
  prodHdrToBE(&props);
  out_buf.add(&props, sizeof(props));

  // compute number of offsets
  
  int n_offsets = _prodProps.num_objs;
  bool n_is_odd;
  if ((_prodProps.num_objs / 2) == 1) {
    n_is_odd = true;
  } else {
    n_is_odd = false;
  }
  if (n_is_odd) {
    // this extra offset is added for 8-byte alignment 
    n_offsets++;
  }
  int hdrSize = sizeof(prod_hdr_props_t) + n_offsets * sizeof(si32);
  
  // accumulate object info in a separate buffer
  
  MemBuf objBuf;
  char padArray[8];
  MEM_zero(padArray);

  for (size_t i = 0; i < _objs.size(); i++) {

    // set offset

    si32 offset = hdrSize + objBuf.getLen();
    BE_to_array_32(&offset, sizeof(si32));
    out_buf.add(&offset, sizeof(offset));

    // load object into buffer
    
    _objs[i]->loadBuf(objBuf);

    // add bytes for 8-byte word alignment
    
    int npad = 8 - (objBuf.getLen() % 8);
    objBuf.add(padArray, npad);
    
  } // i
  
  // if odd number of offsets, add one for alignment
  
  if (n_is_odd) {
    si32 offset = 0;
    out_buf.add(&offset, sizeof(si32));
  }
  
  // concatenate the buffers
  
  out_buf.add(objBuf.getPtr(), objBuf.getLen());

}

/////////////////////////////////////
// add error string with int argument

void Symprod::_addIntErr(const char *err_str, const int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void Symprod::_addStrErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += sarg;
  _errStr += "\n";
}


