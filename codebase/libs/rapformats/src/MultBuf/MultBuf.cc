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
// MultBuf.cc
//
//
// A buffer with multiple parts - see also MultBufPart.hh
//
// A MultBuf represents a buffer which has multiple parts.
// The objects may be assembled (serialized) or
// disassembled (deserialized) to facilitate buffer storage.
//
// A MultBuf has 3 sections, a header struct which contains the
// number of parts, an array of part structs which indicate the
// part types, lengths and offsets, and the data parts themselves.
//
// MultBuf format:
//
//   header_t
//   nParts * part_t
//   nParts * data
//
// Much of this code copied from DsMessage.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// March 2000
//
////////////////////////////////////////////////////////////


#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/TaStr.hh>
#include <rapformats/MultBuf.hh>
#include <rapformats/MultBufPart.hh>
using namespace std;

//////////////
// constructor
//

MultBuf::MultBuf()
{
  _debug = false;
  _version = currentVersion;
  clearAll();
}

/////////////////////////////
// Copy constructor
//

MultBuf::MultBuf(const MultBuf &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////
// destructor

MultBuf::~MultBuf()
{
  clearParts();
}

/////////////////////////////
// Assignment
//

MultBuf &MultBuf::operator=(const MultBuf &rhs)

{
  return _copy(rhs);
}

////////////////////////////////////////////////////////////////
// peek at header
//
// This is used if you just want to peek at the header id and
// version before deciding how to handle the buffer.
// 
// Returns: 0 on success, -1 on error
// Error string retrieved with getErrStr().

int MultBuf::peekAtHeader(const void *in_buf, const int buf_len,
			  int *id /* = NULL*/,
			  int *version /* = NULL*/ )
  
{
  
  _errStr = "ERROR - MultBuf::peekAtHeader.\n";
  
  if (buf_len < (int) sizeof(header_t)) {
    _errStr += "  Buffer too short for header.\n";
    TaStr::AddInt(_errStr, "  Buffer len: ",  buf_len, true);
    TaStr::AddInt(_errStr, "  Header size: ", sizeof(header_t), true);
    return -1;
  }
  
  header_t hdr;
  memcpy(&hdr, in_buf, sizeof(header_t));
  _BE_to_header(hdr);
  
  if (id) {
    *id = hdr.id;
  }
  if (version) {
    *version = hdr.version;
  }

  return 0;
}

////////////////////////////////////////////////////
// disassemble a buffer into parts, store in
// MultBuf object.
//
// Returns: 0 on success, -1 on error
  
int MultBuf::disassemble(const void *in_buf, const int buf_len)

{

  _errStr = "ERROR - MultBuf::disassemble.\n";
  
  if (buf_len < (int) sizeof(header_t)) {
    _errStr += "  Buffer too short for header.\n";
    TaStr::AddInt(_errStr, "  Buffer len: ", buf_len, true);
    TaStr::AddInt(_errStr, "  Header size: ", sizeof(header_t), true);
    return -1;
  }
  
  header_t hdr;
  memcpy(&hdr, in_buf, sizeof(header_t));
  _BE_to_header(hdr);
  
  _id = hdr.id;
  _version = hdr.version;
  
  clearParts();
  
  for (int i = 0; i < hdr.n_parts; i++) {
    MultBufPart *part = new MultBufPart;
    if (part->loadFromBuf(i, in_buf, buf_len)) {
      _errStr += part->getErrStr();
      return -1;
    }
    _parts.push_back(part);
  }
  return 0;
}

////////////////////////////////////////////////
// does a part exist?
// returns the number of parts of the given type

int MultBuf::partExists(const int part_type) const

{
  int count = 0;
  for (size_t i = 0; i < _parts.size(); i++) {
    if (_parts[i]->getType() == part_type) {
      count++;
    }
  }
  return (count);
}

////////////////////////////////////////////
// Get a part from the parts array, given
// the index into the array.
//
// Returns pointer to part, NULL on failure.

MultBufPart *MultBuf::getPart(const int index) const
  
{
  if (index > (int) _parts.size() - 1) {
    return (NULL);
  }
  return (_parts[index]);
}

////////////////////////////////////////////////////////////
// get a part by type.
//
// If more than 1 part of this type exists, use index to
// select the required one.
//
// Returns pointer to the requested part, NULL on failure.

MultBufPart *MultBuf::getPartByType(const int part_type,
				    const int index /* = 0*/ ) const
  
{
  int count = 0;
  for (size_t i = 0; i < _parts.size(); i++) {
    if (_parts[i]->getType() == part_type) {
      if (count == index) {
	return(_parts[i]);
      }
      count++;
    }
  }
  return (NULL);
}
  
/////////////////////////////
// clear before adding parts.
//
// This initializes the number of parts to 0.
//
// It does NOT clear the header attributes set using the
// set() routines.

void MultBuf::clearParts()

{
  // free parts
  for(size_t i = 0; i < _parts.size(); i++) {
    delete _parts[i];
  }
  _parts.erase(_parts.begin(), _parts.end());
}

///////////////////////////////////////////////////////////////
// clear everything -- header and parts.
//
// A convenience routine for clients who want to call setType()
// instead of setHdrAttr() before assembling a buffer.

void MultBuf::clearAll()
{
  _id = -1;
  clearParts();
}

////////////////////////////////////////////////////////////
// Add a part to the object.
//
// The part is added at the end of the part list.
//
// The buffer must be in BE byte order.

void MultBuf::addPart(const int type, const int len, const void *data)

{
  
  MultBufPart *part = new MultBufPart;
  part->loadFromMem(type, len, data);
  _parts.push_back(part);
}

/////////////////////////////////////
// assemble the parts into a buffer
//
// Returns pointer to the assembled buffer.

void *MultBuf::assemble()

{
  
  _assembledBuf.free();

  // load up header
  
  header_t header;
  MEM_zero(header);
  header.id = _id;
  header.version = _version;
  header.n_parts = (int) _parts.size();
  _BE_from_header(header);
  _assembledBuf.add(&header, sizeof(header));

  // compute the part offsets

  int partDataOffset = sizeof(header_t) + _parts.size() * sizeof(part_hdr_t);
  for (size_t i = 0; i < _parts.size(); i++) {
    _parts[i]->setOffset(partDataOffset);
    partDataOffset += _parts[i]->getLength();
  }

  // load up part headers
  
  for (size_t i = 0; i < _parts.size(); i++) {
    part_hdr_t partHdr;
    MEM_zero(partHdr);
    partHdr.type = _parts[i]->getType();
    partHdr.len = _parts[i]->getLength();
    partHdr.offset = _parts[i]->getOffset();
    _BE_from_part_hdr(partHdr);
    _assembledBuf.add(&partHdr, sizeof(part_hdr_t));
  }

  // load up part data
  
  for (size_t i = 0; i < _parts.size(); i++) {
    _assembledBuf.add(_parts[i]->getBuf(), _parts[i]->getLength());
  }

  return (_assembledBuf.getPtr());
  
}

//////////////////////////////////////////
// print out main header and parts headers
//

void MultBuf::print(ostream &out, const string &spacer) const
{
  printHeader(out, spacer);
  printPartHeaders(out, spacer);
}

////////////////////////////////
// print out the buffer header
//

void MultBuf::printHeader(ostream &out, const string &spacer) const
{
  
  out << spacer << "Buffer id: " << _id<< endl;
  out << spacer << "  version: " << _version << endl;
  out << spacer << "  n_parts: " << _parts.size() << endl;

}

/////////////////////
// print part headers

void MultBuf::printPartHeaders(ostream &out, const string &spacer) const
{
  for (size_t i = 0; i < _parts.size(); i++) {
    getPart(i)->printHeader(out, i, spacer);
  }
}

/////////////////////////////////////////////////
// byte order swapping routines


void MultBuf::_BE_to_header(header_t &hdr)
     
{
  BE_to_array_32(&hdr, sizeof(header_t));
}

void MultBuf::_BE_from_header(header_t &hdr)

{
  BE_from_array_32(&hdr, sizeof(header_t));
}

void MultBuf::_BE_to_part_hdr(part_hdr_t &part)

{
  BE_to_array_32(&part, sizeof(part_hdr_t));
}

void MultBuf::_BE_from_part_hdr(part_hdr_t &part)

{
  BE_from_array_32(&part, sizeof(part_hdr_t));
}


/////////////////////////////////////////////////
// copy
  
MultBuf &MultBuf::_copy(const MultBuf &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _id = rhs._id;
  _version = rhs._version;
  _assembledBuf = rhs._assembledBuf;
  _debug = rhs._debug;
  _errStr = "";

  for (size_t ii = 0; ii < rhs._parts.size(); ii++) {
    delete _parts[ii];
  }
  _parts.clear();
  
  for (size_t ii = 0; ii < rhs._parts.size(); ii++) {
    MultBufPart *part = new MultBufPart(*rhs._parts[ii]);
    _parts.push_back(part);
  }

  // return ref to self
  
  return *this;

}


