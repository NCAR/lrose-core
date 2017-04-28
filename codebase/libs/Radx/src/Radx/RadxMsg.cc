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
//////////////////////////////////////////////////////////////////////////
// RadxMsg.cc
//
// Class for serializing and deserializing Radx objects
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
//////////////////////////////////////////////////////////////////////////
//
// A message consists of:
//
// (1) a Hdr_t which contains n_parts, the number of message parts 
// (2) n_parts * Part_t, the header for each part
// (3) n_parts * contents - which are padded to align on
//     8-byte boundaries 
//
// The message length is:
//   sizeof(Hdr_t) +
//   n_parts * sizeof(Part_t) +
//   sum of padded length for each part.
//
// The Part_t headers contain the length and offset of each part.
//
//////////////////////////////////////////////////////////////////////////

#include <Radx/ByteOrder.hh>      
#include <Radx/RadxMsg.hh>
#include <cstring>
#include <iostream>
using namespace std;

//////////////
// constructor

RadxMsg::RadxMsg(const string &name) :
        _name(name)
{
  _assembledMsg = NULL;
  _lengthAssembled = 0;
  _nAssembledAlloc = 0;
  _debug = false;
  clearAll();
}

/////////////////////////////
// Copy constructor
//

RadxMsg::RadxMsg(const RadxMsg &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////
// destructor

RadxMsg::~RadxMsg()
{

  // free assembled message

  if (_assembledMsg) {
    delete[] _assembledMsg;
  }

  // free parts

  for (size_t ii = 0; ii < _parts.size(); ii++) {
    delete _parts[ii];
  }
  _parts.clear();
  
}

/////////////////////////////
// Assignment
//

RadxMsg &RadxMsg::operator=(const RadxMsg &rhs)

{
  return _copy(rhs);
}

//////////////////////////
// decode a message header
//
// This is used if you just want to peek at the header before
// deciding how to handle the message.
// 
// If msgLen is set, checks that the msg is big enough
//   to hold at least a Hdr_t. Otherwise, assumes
//   that the message is big enough and blindly copies
//   memory.
//
// Returns: -1 If msgLen is set and is smaller than a Hdr_t.
//           0 Otherwise.
// Virtual.
int RadxMsg::decodeHeader(const void *inMsg, const ssize_t msgLen /* = -1*/ )

{

  if (!inMsg || (msgLen != -1 && msgLen < (int) sizeof(Hdr_t))) {
    return -1;
  }
  
  Hdr_t hdr;
  memcpy(&hdr, inMsg, sizeof(Hdr_t));
  BE_to_Hdr(&hdr);

  _msgType = hdr.msgType;
  _subType = hdr.subType;
  _nParts = hdr.nParts;

  return 0;
}

////////////////////////////////////////////////////
// disassemble a message into parts, store in
// RadxMsg object.
//
// If msgLen is provided, the parts are checked to make
// sure they do not run over the end of the message.
// 
// If msgLen is set, checks that the msg is big enough
//   to hold at least a Hdr_t. Otherwise, assumes
//   that the message is big enough and blindly copies
//   memory.
//
// Returns: -1 If msgLen is set and is smaller than a Hdr_t.
//               or if one of the message parts cannot be decoded.
//           0 Otherwise.

int RadxMsg::disassemble(const void *inMsg, const ssize_t msgLen /* = -1*/ )

{
  int status = decodeHeader(inMsg, msgLen);
  if (status < 0) {
    return -1;
  }
  
  _allocParts(_nParts);
  for (ssize_t ii = 0; ii < _nParts; ii++) {
    if (_parts[ii]->loadFromMsg(ii, inMsg, msgLen)) {
      printHeader(cerr, "  ");
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////
// does a part exist?
// returns the number of parts of the given type

int RadxMsg::partExists(const int partType) const

{
  int count = 0;
  for (ssize_t ii = 0; ii < _nParts; ii++) {
    if (_parts[ii]->getPartType() == partType) {
      count++;
    }
  }
  return count;
}

////////////////////////////////////////////
// Get a part from the parts array, given
// the index into the array.
//
// Returns pointer to part, NULL on failure.

RadxMsg::Part *RadxMsg::getPart(const ssize_t index) const
  
{
  if (index > _nParts - 1) {
    return (NULL);
  }
  return _parts[index];
}

////////////////////////////////////////////////////////////
// get a part by type.
//
// If more than 1 part of this type exists, use index to
// select the required one.
//
// Returns pointer to the requested part, NULL on failure.

RadxMsg::Part *RadxMsg::getPartByType(const int partType,
                                      const ssize_t index /* = 0*/ ) const
  
{
  ssize_t count = 0;
  for (ssize_t ii = 0; ii < _nParts; ii++) {
    if (_parts[ii]->getPartType() == partType) {
      if (count == index) {
	return _parts[ii];
      }
      count++;
    }
  }
  return NULL;
}
  
////////////////////////////////////
// set the message headerattributes
//
// These overwrite the existing attributes.

void RadxMsg::setHdrAttr(const int msgType,
                         const int subType /* = -1*/)

{
  _msgType = msgType;
  _subType = subType;
}

/////////////////////////////
// clear before adding parts.
//
// This initializes the number of parts to 0.
//
// It does NOT clear the header attributes set using the
// set() routines.

void RadxMsg::clearParts()

{
  _nParts = 0;
}

///////////////////////////////////////////////////////////////
// clear everything -- header and parts.
//
// A convenience routine for clients who want to call setType()
// instead of setHdrAttr() before assembling a message.

void RadxMsg::clearAll()
{
  _msgType = -1;
  _subType = -1;
  _nParts = 0;
}

////////////////////////////
// Add a part to the object.
//
// The part is added at the end of the part list.
//
// The buffer must be in BE byte order.

void RadxMsg::addPart(const int type, const ssize_t len, const void *data)

{
  _allocParts(_nParts + 1);
  _parts[_nParts]->loadFromMem(type, len, data);
  _nParts++;
}

void RadxMsg::addPart(const string &name, ssize_t length, const void *data)

{
  Part *part = new Part(name, length, data);
  _parts.push_back(part);
  _nParts = _parts.size();
}

/////////////////////////////////////
// assemble the parts into a message
//
// Returns pointer to the assembled message.

Radx::ui08 *RadxMsg::assemble()

{
  
  // compute total message length

  _lengthAssembled = 0;
  _lengthAssembled += sizeof(Hdr_t);
  _lengthAssembled += _nParts * sizeof(Part_t);

  for (ssize_t i = 0; i < _nParts; i++) {
    _lengthAssembled += _parts[i]->getPaddedLength();
  }

  // allocate memory

  _allocAssembledMsg();

  // load up header

  Hdr_t header;
  memset(&header, 0, sizeof(Hdr_t));
  header.msgType = _msgType;
  header.subType = _subType;
  header.nParts = _nParts;
  BE_from_Hdr(&header);
  memcpy(_assembledMsg, &header, sizeof(Hdr_t));

  // load up parts

  ssize_t partHdrOffset = sizeof(Hdr_t);
  ssize_t partDataOffset = partHdrOffset + _nParts * sizeof(Part_t);

  for (ssize_t i = 0; i < _nParts; i++) {
    Part_t msgPart;
    memset(&msgPart, 0, sizeof(Part_t));
    msgPart.partType = _parts[i]->getPartType();
    msgPart.len = _parts[i]->getLength();
    _parts[i]->setOffset(partDataOffset);
    msgPart.offset = partDataOffset;
    BE_from_Part(&msgPart);
    memcpy(_assembledMsg + partHdrOffset, &msgPart, sizeof(Part_t));
    memcpy(_assembledMsg + partDataOffset,
	   _parts[i]->getBuf(), _parts[i]->getLength());
    partHdrOffset += sizeof(Part_t);
    partDataOffset += _parts[i]->getPaddedLength();
  }

  return (_assembledMsg);

}

//////////////////////////////////////////
// print out main header and parts headers
//

void RadxMsg::print(ostream &out, const char *spacer) const
{
  printHeader(out, spacer);
  printPartHeaders(out, spacer);
}

////////////////////////////////
// print out the message header
//

void RadxMsg::printHeader(ostream &out, const char *spacer) const
{

  out << spacer << "Message type: " << _msgType << endl;
  out << spacer << "        subType: " << _subType << endl;
  out << spacer << "        nParts: " << _nParts << endl;

}

/////////////////////
// print part headers

void RadxMsg::printPartHeaders(ostream &out, const char *spacer) const
{
  for (ssize_t i = 0; i < getNParts(); i++) {
    Part *part = getPart(i);
    part->printHeader(out, spacer, i);
  }
}

///////////////////////////////////////////////////////////////
// print part headers, using strings to label IDs as appropriate
// Labels are passed in as a map.

void RadxMsg::printPartHeaders(ostream &out, const char *spacer,
                               const PartHeaderLabelMap &labels) const
{
  for (ssize_t i = 0; i < getNParts(); i++) {
    Part *part = getPart(i);
    int id = part->getPartType();
    PartHeaderLabelMap::const_iterator pos = labels.find(id);
    if (pos != labels.end()) {
      part->printHeader(out, spacer, pos->second, i);
    } else {
      part->printHeader(out, spacer, i);
    }
  }
}

////////////////////////////////
// print out the message header
// Backward-compatibility

void RadxMsg::printHeader(ostream *out, const char *spacer) const
{
  printHeader(*out, spacer);
}

////////////////////////////////////////////////
// allocate the buffer for the assembled message

void RadxMsg::_allocParts(const ssize_t nParts)
  
{
  while (nParts > (int) _parts.size()) {
    Part *newPart = new Part;
    _parts.push_back(newPart);
  }
}

////////////////////////////////////////////////
// allocate the buffer for the assembled message

void RadxMsg::_allocAssembledMsg()

{
  if (_lengthAssembled > _nAssembledAlloc) {
    if (_assembledMsg) {
      delete[] _assembledMsg;
    }
    _assembledMsg = new Radx::ui08[_lengthAssembled];
    _nAssembledAlloc = _lengthAssembled;
  }
  memset(_assembledMsg, 0, _lengthAssembled);
}

/////////////////////////////
// copy
//

RadxMsg &RadxMsg::_copy(const RadxMsg &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _name = rhs._name;
  _msgType = rhs._msgType;
  _subType = rhs._subType;
  _debug = rhs._debug;

  // copy in the parts
  
  for (ssize_t ii = 0; ii < rhs._nParts; ii++) {
    Part *part = new Part(*rhs._parts[ii]);
    _parts.push_back(part);
  }
  _nParts = rhs._nParts;

  // assemble if needed
  
  _lengthAssembled = 0;
  _assembledMsg = NULL;
  if (rhs._assembledMsg != NULL) {
    assemble();
  }
  
  return *this;

}

///////////////////////////
// Part default constructor

RadxMsg::Part::Part()

{
  _partType   = -1;
  _length = 0;
  _paddedLength = 0;
  _buf = NULL;
  _nBufAlloc = 0;
}

/////////////////////////////
// Part copy constructor
//

RadxMsg::Part::Part(const Part &rhs)
  
{
  if (this != &rhs) {
    _copy(rhs);
  }
}

///////////////////////////////////
// Part constructor with message

RadxMsg::Part::Part(const string &name, size_t length, const void *data) :
        _name(name),
        _length(length)
{
  _paddedLength = ((_length / 8) + 1) * 8;
  _rbuf.add(data, length);
  int nExtra = _paddedLength - _length;
  char *extra = new char[nExtra];
  memset(extra, 0, nExtra);
  _rbuf.add(extra, nExtra);
}
    
//////////////
// destructor

RadxMsg::Part::~Part()

{
  if (_buf) {
    delete[] _buf;
  }
}

/////////////////////////////
// Assignment
//

RadxMsg::Part &RadxMsg::Part::operator=(const Part &rhs)
  
{
  return _copy(rhs);
}

////////////////////////////////////////////////////////////
// load a part from an incoming message which is assumed to
// be in BE byte order
//
// If msgLen is provided, the part is checked to make
// sure it does not run over the end of the message.
//
// Returns 0 on success, -1 on error
// Error occurs if end of part is beyond end of message.

int RadxMsg::Part::loadFromMsg(const ssize_t partNum,
                               const void *inMsg,
                               const ssize_t msgLen /* = -1*/ )

{

  Radx::ui08 *inBuf = (Radx::ui08 *) inMsg;

  Part_t msgPart;

  memcpy(&msgPart,
	 inBuf + sizeof(Hdr_t) + partNum * sizeof(Part_t),
	 sizeof(Part_t));

  BE_to_Part(&msgPart);

  _partType = msgPart.partType;
  _length = msgPart.len;
  _paddedLength = ((_length / 8) + 1) * 8;
  _offset = msgPart.offset;
  
  if (msgLen > 0 && (_offset + _length) > msgLen) {
    cerr << "ERROR - RadxMsg::Part::loadFromMsg.\n";
    cerr << "  End of part " << partNum << " is beyond end of message.\n";
    cerr << "  End of part offset: " << _offset + _length << endl;
    cerr << "  End of message offset: " << msgLen << endl;
    return -1;
  }

  _allocBuf();
  memcpy(_buf, inBuf + _offset, _length);

  return 0;

}

////////////////////////////////////////////////////////////
// load a part from a memory buffer which is assumed to
// be in host byte order

void RadxMsg::Part::loadFromMem(const int partType,
                                const ssize_t len,
                                const void *inMem)

{

  _partType = partType;
  _length = len;
  _paddedLength = ((_length / 8) + 1) * 8;
  
  _allocBuf();
  memcpy(_buf, inMem, _length);

}

///////////////
// print header

void RadxMsg::Part::printHeader(ostream &out, const char *spacer,
                                ssize_t num /* = -1*/ ) const
{
  if (num >= 0) {
    out << spacer << "---- part: " << num << " ----" << endl;
  }
  out << spacer << "  partType:   " << getPartType() << endl;
  out << spacer << "  length: " << getLength() << endl;
  out << spacer << "  padded: " << getPaddedLength() << endl;
  out << spacer << "  offset: " << getOffset() << endl;
}

void RadxMsg::Part::printHeader(ostream &out, const char *spacer,
                                const string &label,
                                ssize_t num /* = -1*/ ) const
{
  if (num >= 0) {
    out << spacer << "---- part: " << num << " ----" << endl;
  }
  out << spacer << "  label:   " << label << endl;
  out << spacer << "  partType:   " << getPartType() << endl;
  out << spacer << "  length: " << getLength() << endl;
  out << spacer << "  padded: " << getPaddedLength() << endl;
  out << spacer << "  offset: " << getOffset() << endl;
}

////////////////////////////
// allocate the local buffer

void RadxMsg::Part::_allocBuf()

{
  if ((_length > _nBufAlloc) || (_length < _nBufAlloc/2)) {
    if (_buf) {
      delete[] _buf;
    }
    _buf = new Radx::ui08[_length];
    _nBufAlloc = _length;
  }
}

/////////////////////////////
// copy

RadxMsg::Part &RadxMsg::Part::_copy(const Part &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _partType = rhs._partType;
  _length = rhs._length;
  _paddedLength = rhs._paddedLength;
  _offset = rhs._offset;
  
  // the copy never has local buffer
  // so allocate buffer and copy data in

  _buf = NULL;
  _nBufAlloc = 0;
  _allocBuf();
  memcpy(_buf, rhs._buf, _length);

  return *this;

}

/*
 * BE swapping routines
 */

/*******************
 * BE_to_Hdr()
 *
 * Convert BE to Hdr_t
 */

void RadxMsg::BE_to_Hdr(Hdr_t *hdr)
     
{
  if (!ByteOrder::hostIsBigEndian()) {
    ByteOrder::swap32(hdr, sizeof(Hdr_t));
  }
}

/*******************
 * BE_from_Hdr()
 *
 * Convert Hdr_t to BE
 */

void RadxMsg::BE_from_Hdr(Hdr_t *hdr)

{
  if (!ByteOrder::hostIsBigEndian()) {
    ByteOrder::swap32(hdr, sizeof(Hdr_t));
  }
}

/*******************
 * BE_to_Part()
 *
 * Convert BE to Part_t
 */

void RadxMsg::BE_to_Part(Part_t *part)

{
  if (!ByteOrder::hostIsBigEndian()) {
    ByteOrder::swap32(part, sizeof(Part_t));
  }
}

/***************************
 * BE_from_Part()
 *
 * Convert Part_t to BE
 */

void RadxMsg::BE_from_Part(Part_t *part)

{
  if (!ByteOrder::hostIsBigEndian()) {
    ByteOrder::swap32(part, sizeof(Part_t));
  }
}

