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
// MdvReadChunk.cc
//
// Class for handling access to Mdv chunks
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/mdv/MdvReadChunk.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/mdv/MdvReadChunk.hh>
#include <toolsa/mem.h>
#include <Mdv/mdv/mdv_read.h>
#include <Mdv/mdv/MdvRead.hh>
#include <cassert>
using namespace std;

////////////////////////////////////////////////////////////////////////
// Default constructor
//
// If you use this constructor you must call init() before using object
//

MdvReadChunk::MdvReadChunk()
  

{

  _initDone = false;
  _headerRead = false;

}

/////////////////////////////
// Primary constructor
//

MdvReadChunk::MdvReadChunk(const MdvRead *mdv, const int chunk_num)

{

  init(mdv, chunk_num);

}

/////////////////////////////
// Copy constructor
//

MdvReadChunk::MdvReadChunk(const MdvReadChunk &other)

{
  *this = other;
}

/////////////////////////////
// Destructor

MdvReadChunk::~MdvReadChunk()

{


}

////////////////////////////////////////////////////////
// init - this must be called before the object is used.
// It is called automatically by the primary constructor.
//

void MdvReadChunk::init(const MdvRead *mdv, const int chunk_num)

{

  _mdv = mdv;
  MEM_zero(_chunkHeader);
  _chunkNum = chunk_num;
  _headerRead = false;
  _data = NULL;
  _initDone = true;

}

/////////////////////////////
// Assignment
//

void MdvReadChunk::operator=(const MdvReadChunk &other)

{

  if (this == &other) {
    return;
  }

  init(other._mdv, other._chunkNum);
  _chunkHeader = other._chunkHeader;
  _headerRead = other._headerRead;
  if (_headerRead) {
    _dataBuf = other._dataBuf;
    _data = _dataBuf.getBufPtr();
  } else {
    MEM_zero(_chunkHeader);
    _dataBuf.free();
    _data = NULL;
  }
  _initDone = other._initDone;

}

//////////////////////////////////////
// read chunk header and data
//
// Returns 0 on success, -1 on failure

int MdvReadChunk::_read()

{

  if (_readHeader()) {
    return (-1);
  }
  if (_readData()) {
    return (-1);
  }
  return (0);
}
  
//////////////////////////////////////
// read chunk header
//
// Returns 0 on success,  -1 on failure

int MdvReadChunk::_readHeader()
  
{

  assert (_initDone);

  // don't read twice

  if (_headerRead) {
    return (0);
  }

  // check open file

  if (!_mdv->_fp) {
    cerr << "ERROR - MdvReadChunk::readHeader" << endl;
    cerr << "  File not open" << endl;
    return (-1);
  }

  // Read the chunk header.
  
  if (MDV_load_chunk_header(_mdv->_fp, &_chunkHeader,
			    (MDV_master_header_t *) &_mdv->_masterHeader,
			    _chunkNum) != MDV_SUCCESS) {
    cerr << "ERROR - MdvReadChunk::readHeader" << endl;
    cerr << "  Cannot load chunk header, chunk_num: " << _chunkNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  _headerRead = true;

  return (0);

}

//////////////////////////////////////
// read chunk data
//
// Returns 0 on success,  -1 on failure

int MdvReadChunk::_readData()
  
{
  
  if (_readHeader()) {
    return (-1);
  }

  // Read the chunk data
  
  void *data;
  if ((data = MDV_get_chunk_data(_mdv->_fp, &_chunkHeader)) == NULL) {
    cerr << "ERROR - MdvReadChunk::readData" << endl;
    cerr << "  Cannot get chunk data, chunk_num: " << _chunkNum << endl;
    cerr << "  File path '" << _mdv->_filePath << "'" << endl;
    return (-1);
  }

  _dataBuf.free();
  _dataBuf.add(data, _chunkHeader.size);
  _data = _dataBuf.getBufPtr();

  return (0);

}


