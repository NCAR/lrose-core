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

//////////////////////////////////////////////////////////////
// mdv/MdvReadChunk.hh
//
// Class for reading/writing/representing MDV chunks
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 1999
//
////////////////////////////////////////////////////////////////////
//
// For a given chunk, you may read the following:
//
//   1. chunk header
//   2. chunk data
//
// You cannot use the read routines in the class directly. You need to
// call them via the routines in the MdvRead object.
// The read routines are private to this class.
//
// However, the public member functions of the class give you access
// to the headers and data once they have been read.
//
// Header representation.
//
//   getHeader() returns a reference to the chunk header.
//
// Data representation.
//
//   getData() returns a pointer to the chunk data.
//
////////////////////////////////////////////////////////////////////////////


#ifndef MdvReadChunk_hh
#define MdvReadChunk_hh

#include <toolsa/MemBuf.hh>
#include <Mdv/mdv/mdv_file.h>
#include <cstdio>
#include <string>
using namespace std;

class MdvRead;

class MdvReadChunk {

  friend class MdvRead;

public:

  // default constructor
  // If this is used, you must call init() before using the object
  
  MdvReadChunk();

  // primary constructor
  
  MdvReadChunk(const MdvRead *mdv, const int chunk_num);
  
  // copy constructor
  
  MdvReadChunk(const MdvReadChunk &other);
  
  // destructor

  virtual ~MdvReadChunk();

  // init - this must be called before the object is used.
  // It is called automatically by the primary constructor.

  void init(const MdvRead *mdv, const int chunk_num);

  // assignment

  void operator=(const MdvReadChunk &other);

  // access to members
  
  MDV_chunk_header_t &getHeader() { return (_chunkHeader); }
  void  *getData() { return (_data); }

protected:

  bool _initDone;
  const MdvRead *_mdv;
  int _chunkNum;
  MDV_chunk_header_t _chunkHeader;
  void *_data;
  MemBuf _dataBuf;
  bool _headerRead;
  
  //////////////////////////////////////
  // read chunk header and data
  // Returns 0 on success, -1 on failure

  int _read();

  //////////////////////////////////////
  // read chunk header
  // Returns 0 on success,  -1 on failure
  
  int _readHeader();
  
  //////////////////////////////////////
  // read chunk data
  // Returns 0 on success,  -1 on failure

  int _readData();
  
private:

};

#endif


