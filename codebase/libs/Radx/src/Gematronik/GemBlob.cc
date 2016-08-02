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
///////////////////////////////////////////////////////////////
// GemBlob.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2014
//
///////////////////////////////////////////////////////////////

#include "GemBlob.hh"
#include <Radx/Radx.hh>
#include <Radx/ByteOrder.hh>
#include <cstring>
#include <cstdlib>
#include <zlib.h>

using namespace std;


// Constructor

GemBlob::GemBlob(int id,
                 bool debug) :
        _id(id),
        _debug(debug)
  
{
  _data = NULL;
}

// destructor

GemBlob::~GemBlob()

{
  clearData();
}

////////////////////////////////////////
// load data
//
// returns 0 on success, -1 on failure

int GemBlob::loadData(int size,
                      const string &compression,
                      const char *data)
  
{

  if (_debug) {
    cerr << "Loading data for BLOB, id: " << _id << endl;
  }

  // init

  clearData();

  // if not compressed, copy and return now
  
  if (compression != "qt") {
    if (_debug) {
      cerr << "  Not compressed, size: " << size << endl;
    }
    _size = size;
    _data = new char[_size];
    memcpy(_data, data, _size);
    return 0;
  }

  // QT compression has a 4-byte BE integer at the start of the buffer
  // to indicate the uncompressed size
  // apart from that it is standard Zlib compression

  Radx::ui32 expectedLen;
  memcpy(&expectedLen, data, sizeof(Radx::ui32));
  if (!ByteOrder::hostIsBigEndian()) {
    ByteOrder::swap32(&expectedLen, sizeof(Radx::ui32));
  }

  if (_debug) {
    cerr << "  Compressed size: " << size << endl;
    cerr << "  Uncompressed size: " << expectedLen << endl;
  }

  _size = expectedLen;
  _data = new char[_size];

  // uncompress

  z_stream stream;
  
  stream.next_in = (Bytef*) (data + sizeof(Radx::ui32));
  stream.avail_in = (uInt) (size - sizeof(Radx::ui32));
  
  stream.next_out = (Bytef*) _data;
  stream.avail_out = (uInt) expectedLen;
  
  stream.zalloc = (alloc_func)0;
  stream.zfree = (free_func)0;
  
  int err = inflateInit(&stream);
  if (err != Z_OK) return -1;
  
  err = inflate(&stream, Z_FINISH);
  if (err != Z_STREAM_END) {
    inflateEnd(&stream);
    return -1;
  }
  
  uLongf uncompLen = stream.total_out;
  if (inflateEnd(&stream)) {
    return -1;
  }
  if (uncompLen != expectedLen) {
    cerr << "ERROR - GemBlob::loadData" << endl;
    cerr << "  Bad uncompression" << endl;
    cerr << "  Expected nbytes: " << expectedLen << endl;
    cerr << "  Got nBytes: " << uncompLen << endl;
    return -1;
  }
  
  return 0;

}

//////////////
// clear data

void GemBlob::clearData()

{
  if (_data != NULL) {
    delete[] _data;
    _data = NULL;
  }
}

