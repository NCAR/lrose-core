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
// MdvxChunk.cc
//
// Class for handling access to chunks for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// September 1999
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxChunk.hh> for details.
//
///////////////////////////////////////////////////////////

#include <Mdv/MdvxChunk.hh>
#include <toolsa/mem.h>
#include <toolsa/umisc.h>
#include <dataport/bigend.h>
using namespace std;

//////////////////////////////////////////////////////////
// Default constructor
//

MdvxChunk::MdvxChunk()
  

{
  MEM_zero(_chdr);
  _chdr.record_len1 = sizeof(Mdvx::chunk_header_t) - (2 * sizeof(si32));
  _chdr.struct_id = Mdvx::CHUNK_HEAD_MAGIC_COOKIE_64;
  _chdr.record_len2 = _chdr.record_len1;
}

////////////////////////////////////////////////////////////
// Copy constructor
//

MdvxChunk::MdvxChunk(const MdvxChunk &other)

{
  if (this == &other) {
    return;
  }
  *this = other;
}

///////////////////////////////////////////////////////////////
//
// Constuct from chunk header and data parts.
//
// If the data pointer is not NULL, space is allocated for it and the
// data is copied in.
//
// If the data pointer is NULL, space is allocated for data based on
// the size in the chunk header.

MdvxChunk::MdvxChunk(const Mdvx::chunk_header_t &c_hdr,
		     const void *data /* = NULL*/ )

{

  _chdr = c_hdr;
  
  if (data != NULL) {
    _dataBuf.add(data, c_hdr.size);
  } else {
    _dataBuf.prepare(c_hdr.size);
  }

}
  
/////////////////////////////
// Destructor

MdvxChunk::~MdvxChunk()

{
}

/////////////////////////////
// Assignment
//

void MdvxChunk::operator=(const MdvxChunk &other)

{
  if (this == &other) {
    return;
  }
  _chdr = other._chdr;
  _dataBuf = other._dataBuf;
}

//////////////////
// clear the chunk

void MdvxChunk::clear()
{
  MEM_zero(_chdr);
  _dataBuf.free();
  _errStr = "";
}

//////////////////////////
// setting chunk data
//
// If chunk id is recognized by Mdvx, the chunk will be byte-swapped by
// the library, and should not be swapped prior to calling this routine.
// If the chunk id is not recognized by Mdvx, automatic swapping
// will not occur.

void MdvxChunk::setData(const void *chunkData, int64_t size)
{
  _dataBuf.free();
  _dataBuf.add(chunkData, size);
  _chdr.size = size;
}

void MdvxChunk::setId(int id)
{
  _chdr.chunk_id = id;
}

void MdvxChunk::setInfo(const char* info)
{
  STRncopy( _chdr.info, info, MDV_CHUNK_INFO_LEN );
}

////////////
// printing

void MdvxChunk::printHeader(ostream &out) const {
  Mdvx::printChunkHeader(_chdr, out);
}
  
////////////////////////////////////////////////////////////////
// _read_data
//
// Uses the offset and size in the header, so make sure
// the header is correct before calling.
//
// Chunk data is not swapped on read - it is stored in BE format.
//
// Returns 0 on success, -1 on failure.

int MdvxChunk::_read_data(TaFile &infile)

{

  char errstr[512];

  clearErrStr();

  // Seek to the proper position in the input file.

  int64_t offset = _chdr.chunk_data_offset;
  if (infile.fseek(offset, SEEK_SET) != 0) {
    _errStr += "ERROR - MdvxChunk::_read_data.\n";
    sprintf(errstr, "  Seeking chunk data at offset %lld\n", (long long) offset);
    _errStr += errstr;
    _errStr += " Chunk info: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  // allocate buffer
  
  size_t size = _chdr.size;
  _dataBuf.prepare(_chdr.size);

  // read in
  
  if (infile.fread(_dataBuf.getPtr(), 1, size) != size) {
    _errStr += "ERROR - MdvxChunk::_read_data.\n";
    sprintf(errstr, "  Cannot read chunk. size %lld\n", (long long) size);
    _errStr += errstr;
    _errStr += " Chunk info: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  return 0;

}


////////////////////////////////////////////////////////////////
// _write_data
//
// The chunk data is swapped as appropriate, and then written to
// the file. If the chunk ID is not recognized, the data will
// not be swapped.
//
// Passed in is 'this_offset', the starting offset for the write.
//
// Side effects:
//  1: chunk_data_offset is set in the chunk header.
//  2: arg next_offset is set - it is the offset for the next write.
//
// Returns 0 on success, -1 on failure.

int MdvxChunk::_write_data(TaFile &outfile,
			   int64_t this_offset,
			   int64_t &next_offset) const

{

  char errstr[512];
  clearErrStr();
  
  size_t chunk_size = _chdr.size;
  ui32 be_size = BE_from_ui32(chunk_size);
  
  // Set the constant values in the header.

  _chdr.record_len1 = sizeof(Mdvx::chunk_header_t) - (2 * sizeof(si32));
  _chdr.struct_id = Mdvx::CHUNK_HEAD_MAGIC_COOKIE_64;
  _chdr.record_len2 = _chdr.record_len1;
  
  // set data offset in header, and next offset
  
  _chdr.chunk_data_offset = this_offset + sizeof(ui32);
  next_offset = this_offset + chunk_size + 2 * sizeof(ui32);

  // Seek to the start offset

  if (outfile.fseek(this_offset, SEEK_SET) != 0) {
    _errStr += "ERROR - MdvxChunk::_write_data.\n";
    sprintf(errstr, "  Seeking chunk data at this_offset %lld\n", (long long) this_offset);
    _errStr += errstr;
    _errStr += " Chunk info: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  // Write the leading FORTRAN record to disk.
  
  if (outfile.fwrite(&be_size, sizeof(be_size), 1) != 1) {
    _errStr += "ERROR - MdvxChunk::_write_data.\n";
    _errStr += "  Cannot write begin fortran len for chunk: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  // write the data - it is stored in BE format
  
  if (outfile.fwrite(_dataBuf.getPtr(), 1, chunk_size) != chunk_size) {
    _errStr += "ERROR - MdvxChunk::_write_data.\n";
    _errStr += "  Cannot write data for chunk: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  // Write the trailing FORTRAN record to disk.
  
  if (outfile.fwrite(&be_size, sizeof(be_size), 1) != 1) {
    _errStr += "ERROR - MdvxChunk::_write_data.\n";
    _errStr += "  Cannot write end fortran len for chunk: ";
    _errStr += _chdr.info;
    _errStr += "\n";
    return -1;
  }

  return 0;

}
