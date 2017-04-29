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
// (1) a MsgHdr_t which contains n_parts, the number of message parts 
// (2) n_parts * PartHdr_t, the header for each part
// (3) n_parts * contents - which are padded to align on
//     8-byte boundaries 
//
// The message length is:
//   sizeof(MsgHdr_t) +
//   n_parts * sizeof(PartHdr_t) +
//   sum of padded length for each part.
//
// The PartHdr_t headers contain the length and offset of each part.
//
//////////////////////////////////////////////////////////////////////////

#include <Radx/ByteOrder.hh>      
#include <Radx/RadxMsg.hh>
#include <cstring>
#include <iostream>
using namespace std;

//////////////
// constructor

RadxMsg::RadxMsg(int msgType /* = 0 */,
                 int subType /* = 0 */) :
        _msgType(msgType), _subType(subType)
        
{
  clearParts();
  _debug = false;
  _swap = false;
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

/////////////////////////////
// copy

RadxMsg &RadxMsg::_copy(const RadxMsg &rhs)

{

  if (&rhs == this) {
    return *this;
  }
  
  _debug = rhs._debug;
  _swap = rhs._swap;
  
  _msgType = rhs._msgType;
  _subType = rhs._subType;
  _nParts = rhs._nParts;
  
  // copy in the parts
  
  for (ssize_t ii = 0; ii < _nParts; ii++) {
    Part *part = new Part(*rhs._parts[ii]);
    _parts.push_back(part);
  }

  _assembledMsg = rhs._assembledMsg;

  return *this;

}

//////////////////////////
// decode a message header
//
// This is used if you just want to peek at the header before
// deciding how to handle the message.
// 
// Returns 0 on success, -1 on error

int RadxMsg::decodeHeader(const void *inMsg, size_t msgLen)

{

  // check validity of operation
  
  if (!inMsg) {
    cerr << "ERROR - RadxMsg::decodeHeader" << endl;
    cerr << "  null message" << endl;
    return -1;
  }
  
  if (msgLen < sizeof(MsgHdr_t)) {
    cerr << "ERROR - RadxMsg::decodeHeader" << endl;
    cerr << "  Message too short, len: " << msgLen << endl;
    cerr << "  requiredLen: " << sizeof(MsgHdr_t) << endl;
    return -1;
  }
  
  // copy in header
  
  MsgHdr_t hdr;
  memcpy(&hdr, inMsg, sizeof(MsgHdr_t));

  // check for swapping
  _swap = false;
  if (hdr.cookie != _cookie) {
    Radx::si64 cookie = hdr.cookie;
    swapMsgHdr(hdr);
    if (hdr.cookie != _cookie) {
      // bad data
      cerr << "ERROR - RadxMsg::decodeHeader" << endl;
      cerr << "  Bad cookie: " << cookie << endl;
      return -1;
    }
    _swap = true;
  }
  
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
//   to hold at least a MsgHdr_t. Otherwise, assumes
//   that the message is big enough and blindly copies
//   memory.
//
// Returns 0 on success, -1 on error

int RadxMsg::disassemble(const void *inMsg, size_t msgLen)

{

  int status = decodeHeader(inMsg, msgLen);
  if (status < 0) {
    return -1;
  }
  
  _allocParts(_nParts);
  for (ssize_t ii = 0; ii < _nParts; ii++) {
    if (_parts[ii]->loadFromMsg(ii, inMsg, msgLen, _swap)) {
      printHeader(cerr, "  ");
      return -1;
    }
  }
  return 0;
}

////////////////////////////////////////////////
// does a part exist?
// returns the number of parts of the given type

int RadxMsg::partExists(int partType) const

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

RadxMsg::Part *RadxMsg::getPart(size_t index) const
  
{
  if ((int) index > _nParts - 1) {
    return NULL;
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

RadxMsg::Part *RadxMsg::getPartByType(int partType,
                                      int index /* = 0*/ ) const
  
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

void RadxMsg::addPart(int partType, size_t len, const void *data)

{
  _allocParts(_nParts + 1);
  _parts[_nParts]->loadFromMem(partType, len, data);
  _nParts++;
}

/////////////////////////////////////
// assemble the parts into a message
//
// Returns pointer to the assembled message.

void *RadxMsg::assemble()

{
  
  // reset the message buffer

  _assembledMsg.reset();
  
  // load up header

  MsgHdr_t header;
  memset(&header, 0, sizeof(MsgHdr_t));
  header.cookie = _cookie;
  header.msgType = _msgType;
  header.subType = _subType;
  header.nParts = _nParts;
  _assembledMsg.add(&header, sizeof(MsgHdr_t));

  // memcpy(_assembledMsg, &header, sizeof(MsgHdr_t));

  // compute data offset for each part
  // and load the part headers into the message
  
  ssize_t offset = sizeof(MsgHdr_t);
  offset += _nParts * sizeof(PartHdr_t);

  for (ssize_t ii = 0; ii < _nParts; ii++) {
    Part *part = _parts[ii];
    part->setOffset(offset);
    offset += part->getPaddedLength();
    PartHdr_t phdr;
    part->loadPartHdr(phdr);
    _assembledMsg.add(&phdr, sizeof(PartHdr_t));
  }

  // add the data for each part to the message

  for (ssize_t ii = 0; ii < _nParts; ii++) {
    Part *part = _parts[ii];
    _assembledMsg.add(part->getBuf(), part->getPaddedLength());
  }

  return _assembledMsg.getPtr();

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

////////////////////////////////////////////////
// allocate the buffer for the assembled message

void RadxMsg::_allocParts(size_t nParts)
  
{
  while (nParts > _parts.size()) {
    Part *newPart = new Part;
    _parts.push_back(newPart);
  }
}

///////////////////////////////////////////////
// byte swapping routines
///////////////////////////////////////////////

// swap message header

void RadxMsg::swapMsgHdr(MsgHdr_t &hdr)
{
  ByteOrder::swap64(&hdr, sizeof(MsgHdr_t));
}

// swap part header

void RadxMsg::swapPartHdr(PartHdr_t &hdr)
{
  ByteOrder::swap64(&hdr, sizeof(PartHdr_t));
}

///////////////////////////
// Part default constructor

RadxMsg::Part::Part()

{
  _partType = -1;
  _length = 0;
  _paddedLength = 0;
  _offset = 0;
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
// Part constructor given data.
// Buffer contains padded length.

RadxMsg::Part::Part(int partType, size_t length, const void *data) :
        _partType(partType),
        _length(length),
        _offset(0)
{
  _paddedLength = ((_length / 8) + 1) * 8;
  _rbuf.add(data, length);
  int nExtra = _paddedLength - _length;
  if (nExtra > 0) {
    char *extra = new char[nExtra];
    memset(extra, 0, nExtra);
    _rbuf.add(extra, nExtra);
    delete[] extra;
  }
}
    
//////////////
// destructor

RadxMsg::Part::~Part()

{

}

/////////////////////////////
// Assignment
//

RadxMsg::Part &RadxMsg::Part::operator=(const Part &rhs)
  
{
  return _copy(rhs);
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
  
  _rbuf = rhs._rbuf;

  return *this;

}

////////////////////////////////////////////////////////////
// load part header from the paryt object

void RadxMsg::Part::loadPartHdr(PartHdr_t &phdr)
{
  memset(&phdr, 0, sizeof(phdr));
  phdr.partType = _partType;
  phdr.offset = _offset;
  phdr.length = _length;
  phdr.length = _length;
  phdr.paddedLength = _paddedLength;
}
  
////////////////////////////////////////////////////////////
// load a part from an incoming message
//
// Returns 0 on success, -1 on error
// Error occurs if end of part is beyond end of message.

int RadxMsg::Part::loadFromMsg(size_t partNum,
                               const void *inMsg,
                               size_t msgLen,
                               bool swap)
  
{
  
  size_t offset = sizeof(MsgHdr_t) + partNum * sizeof(PartHdr_t);
  size_t requiredLen = offset + sizeof(PartHdr_t);

  if (msgLen < requiredLen) {
    cerr << "ERROR - RadxMsg::Part::loadFromMsg" << endl;
    cerr << "  partNum: " << partNum << endl;
    cerr << "  Message too short, len: " << msgLen << endl;
    cerr << "  requiredLen: " << requiredLen << endl;
    return -1;
  }

  Radx::ui08 *inBuf = (Radx::ui08 *) inMsg;
  PartHdr_t phdr;
  memcpy(&phdr, inBuf + offset, sizeof(PartHdr_t));
  if (swap) {
    swapPartHdr(phdr);
  }

  _partType = phdr.partType;
  _length = phdr.length;
  _paddedLength = ((_length / 8) + 1) * 8;
  _offset = phdr.offset;
  

  _rbuf.load(inBuf + _offset, _length);

  return 0;

}

////////////////////////////////////////////////////////////
// load a part from a memory buffer which is assumed to
// be in host byte order

void RadxMsg::Part::loadFromMem(int partType,
                                size_t len,
                                const void *inMem)

{

  _partType = partType;
  _length = len;
  _paddedLength = ((_length / 8) + 1) * 8;
  
  _rbuf.load(inMem, _length);
  
}

///////////////
// print header

void RadxMsg::Part::printHeader(ostream &out, const char *spacer,
                                int num /* = -1*/ ) const
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
                                int num /* = -1*/ ) const
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

