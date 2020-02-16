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
// Mdv/MdvReadChunk.hh
//
// Class for representing chunks in the Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
////////////////////////////////////////////////////////////////////
//
// A chunk is represented by:
//
//   1. chunk header
//   2. chunk data
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


#ifndef MdvxChunk_hh
#define MdvxChunk_hh

#include <Mdv/Mdvx.hh>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaFile.hh>
using namespace std;

class MdvxChunk {

  friend class Mdvx;

public:

  // default constructor
  
  MdvxChunk();

  // copy constructor
  
  MdvxChunk(const MdvxChunk &other);
  
  // Constuct from chunk header and data parts.
  //
  // If the data pointer is not NULL, space is allocated for it and the
  // data is copied in.
  //
  // If the data pointer is NULL, space is allocated for data based on
  // the size in the chunk header.
  
  MdvxChunk(const Mdvx::chunk_header_t &c_hdr,
	    const void *data = NULL);

  // destructor

  virtual ~MdvxChunk();

  // assignment

  void operator=(const MdvxChunk &other);

  // access to header - const reference
  // Note: use the set functions to set items in the header
  
  const Mdvx::chunk_header_t &getHeader() const { return (_chdr); }

  int getId() const { return (_chdr.chunk_id); }
  int64_t getSize() const { return (_chdr.size); }
  string getInfo() const { return (_chdr.info); }

  // pointer to data

  const void *getData() const { return (_dataBuf.getPtr()); }

  // clear the chunk

  void clear();

  // setting chunk data
  //
  // If chunk id is recognized by Mdvx, the chunk will be byte-swapped by
  // the library, and should not be swapped prior to calling this routine.
  // If the chunk id is not recognized by Mdvx, not automatic swapping
  // will occur.
  
  void setData(const void *chunkData, int64_t size);

  // set ID and info

  void setId(int id);
  void setInfo(const char* info);

  // printing

  void printHeader(ostream &out) const;

  // Get the Error String. This has contents when an error is returned.

  string getErrStr() const { return _errStr; }

  // clear the error string

  void clearErrStr() const { _errStr = ""; }

protected:

  Mdvx::chunk_header_t _chdr;
  MemBuf _dataBuf;
  mutable string _errStr;
  
  int _read_data(TaFile &infile);

  int _write_data(TaFile &outfile,
		  int64_t this_offset,
		  int64_t &next_offset) const;

private:

};

#endif


