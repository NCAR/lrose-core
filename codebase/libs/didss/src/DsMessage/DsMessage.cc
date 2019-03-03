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

////////////////////////////////////////////////////////////////////////////////
//
// DsMessage.cc
//
// Terri Betancourt
// RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// May 1998
//
////////////////////////////////////////////////////////////////////////////////

#include <dataport/bigend.h>      
#include <didss/DsMessage.hh>
#include <didss/DsMsgPart.hh>
#include <cstring>
#include <iostream>
using namespace std;

//////////////
// constructor
//
// Memory model defaults to local copy

DsMessage::DsMessage(memModel_t mem_model /* = CopyMem*/ )
{
  _memModel = mem_model;
  _assembledMsg = NULL;
  _lengthAssembled = 0;
  _nAssembledAlloc = 0;
  _debug = false;
  clearAll();
}

/////////////////////////////
// Copy constructor
//

DsMessage::DsMessage(const DsMessage &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////
// destructor

DsMessage::~DsMessage()
{

  // free assembled message

  if (_assembledMsg) {
    delete[] _assembledMsg;
  }

  // free parts

  vector< DsMsgPart* >::iterator i;
  for(i = _parts.begin(); i != _parts.end(); i++) {
    delete *i;
  }
  _parts.erase(_parts.begin(), _parts.end());
  
}

/////////////////////////////
// Assignment
//

DsMessage &DsMessage::operator=(const DsMessage &rhs)

{
  return _copy(rhs);
}

//////////////////////////
// decode a message header
//
// This is used if you just want to peek at the header before
// deciding how to handle the message.
// 
// If msg_len is set, checks that the msg is big enough
//   to hold at least a DsMsgHdr_t. Otherwise, assumes
//   that the message is big enough and blindly copies
//   memory.
//
// Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
//           0 Otherwise.
// Virtual.
int DsMessage::decodeHeader(const void *in_msg, ssize_t msg_len /* = -1*/ )

{

  if (!in_msg || (msg_len != -1 && msg_len < (int) sizeof(DsMsgHdr_t))) {
    return -1;
  }
  
  DsMsgHdr_t hdr;
  memcpy(&hdr, in_msg, sizeof(DsMsgHdr_t));
  BE_to_DsMsgHdr(&hdr);

  _type = hdr.type;
  _subType = hdr.subType;
  _mode = hdr.mode;
  _flags = hdr.flags;
  _majorVersion = hdr.majorVersion;
  _minorVersion = hdr.minorVersion;
  _serialNum = hdr.serialNum;
  _category = hdr.category;
  _error = hdr.error;
  _nParts = hdr.nParts;

  return 0;
}

////////////////////////////////////////////////////
// disassemble a message into parts, store in
// DsMessage object.
//
// If msg_len is provided, the parts are checked to make
// sure they do not run over the end of the message.
// 
// If msg_len is set, checks that the msg is big enough
//   to hold at least a DsMsgHdr_t. Otherwise, assumes
//   that the message is big enough and blindly copies
//   memory.
//
// Returns: -1 If msg_len is set and is smaller than a DsMsgHdr_t.
//               or if one of the message parts cannot be decoded.
//           0 Otherwise.

int DsMessage::disassemble(const void *in_msg, const ssize_t msg_len /* = -1*/ )

{
  int status = decodeHeader(in_msg, msg_len);
  if (status < 0) {
    return -1;
  }
  
  _allocParts(_nParts);
  for (ssize_t i = 0; i < _nParts; i++) {
    if (_parts[i]->loadFromMsg(i, in_msg, msg_len)) {
      printHeader(cerr, "  ");
      return (-1);
    }
  }
  return (0);
}

////////////////////////////////////////////////
// does a part exist?
// returns the number of parts of the given type

int DsMessage::partExists(const int data_type) const

{
  int count = 0;
  for (ssize_t i = 0; i < _nParts; i++) {
    if (_parts[i]->getType() == data_type) {
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

DsMsgPart *DsMessage::getPart(const ssize_t index) const
  
{
  if (index > _nParts - 1) {
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

DsMsgPart *DsMessage::getPartByType(const int data_type,
				    const ssize_t index /* = 0*/ ) const
  
{
  ssize_t count = 0;
  for (ssize_t i = 0; i < _nParts; i++) {
    if (_parts[i]->getType() == data_type) {
      if (count == index) {
	return(_parts[i]);
      }
      count++;
    }
  }
  return (NULL);
}
  
////////////////////////////////////
// set the message headerattributes
//
// These overwrite the existing attributes.

void DsMessage::setHdrAttr(const int type,
			   const int sub_type /* = -1*/,
			   const int mode /* = -1*/,
			   const int flags /* = 0*/,
			   const int major_version /* = 1*/,
			   const int minor_version /* = 0*/,
			   const ssize_t serial_num /* = -1*/ )

{
  _type = type;
  _subType = sub_type;
  _mode = mode;
  _flags = flags;
  _majorVersion = major_version;
  _minorVersion = minor_version;
  _serialNum = serial_num;
  _category = 0;
  _error = 0;
}

/////////////////////////////
// clear before adding parts.
//
// This initializes the number of parts to 0.
//
// It does NOT clear the header attributes set using the
// set() routines.

void DsMessage::clearParts()

{
  _nParts = 0;
}

///////////////////////////////////////////////////////////////
// clear everything -- header and parts.
//
// A convenience routine for clients who want to call setType()
// instead of setHdrAttr() before assembling a message.

void DsMessage::clearAll()
{
  _type = -1;
  _subType = -1;
  _mode = -1;
  _flags = 0;
  _majorVersion = 1;
  _minorVersion = 0;
  _serialNum = -1;
  _category = 0;
  _error = 0;
  _nParts = 0;
}

////////////////////////////
// Add a part to the object.
//
// The part is added at the end of the part list.
//
// The buffer must be in BE byte order.

void DsMessage::addPart(const int type, const ssize_t len, const void *data)

{
  _allocParts(_nParts + 1);
  _parts[_nParts]->loadFromMem(type, len, data);
  _nParts++;
}

/////////////////////////////////////
// assemble the parts into a message
//
// Returns pointer to the assembled message.

ui08 *DsMessage::assemble()

{
  
  // compute total message length

  _lengthAssembled = 0;
  _lengthAssembled += sizeof(DsMsgHdr_t);
  _lengthAssembled += _nParts * sizeof(DsMsgPart_t);

  for (ssize_t i = 0; i < _nParts; i++) {
    _lengthAssembled += _parts[i]->getPaddedLength();
  }

  // allocate memory

  _allocAssembledMsg();

  // load up header

  DsMsgHdr_t header;
  memset(&header, 0, sizeof(DsMsgHdr_t));
  header.type = _type;
  header.subType = _subType;
  header.mode = _mode;
  header.flags = _flags;
  header.majorVersion = _majorVersion;
  header.minorVersion = _minorVersion;
  header.serialNum = _serialNum;
  header.category = _category;
  header.error = _error;
  header.nParts = _nParts;
  BE_from_DsMsgHdr(&header);
  memcpy(_assembledMsg, &header, sizeof(DsMsgHdr_t));

  // load up parts

  ssize_t partHdrOffset = sizeof(DsMsgHdr_t);
  ssize_t partDataOffset = partHdrOffset + _nParts * sizeof(DsMsgPart_t);

  for (ssize_t i = 0; i < _nParts; i++) {
    DsMsgPart_t msgPart;
    memset(&msgPart, 0, sizeof(DsMsgPart_t));
    msgPart.dataType = _parts[i]->getType();
    msgPart.len = _parts[i]->getLength();
    _parts[i]->setOffset(partDataOffset);
    msgPart.offset = partDataOffset;
    BE_from_DsMsgPart(&msgPart);
    memcpy(_assembledMsg + partHdrOffset, &msgPart, sizeof(DsMsgPart_t));
    memcpy(_assembledMsg + partDataOffset,
	   _parts[i]->getBuf(), _parts[i]->getLength());
    partHdrOffset += sizeof(DsMsgPart_t);
    partDataOffset += _parts[i]->getPaddedLength();
  }

  return (_assembledMsg);

}

//////////////////////////////////////////
// print out main header and parts headers
//

void DsMessage::print(ostream &out, const char *spacer) const
{
  printHeader(out, spacer);
  printPartHeaders(out, spacer);
}

////////////////////////////////
// print out the message header
//

void DsMessage::printHeader(ostream &out, const char *spacer) const
{

  out << spacer << "Message type: " << _type << endl;
  out << spacer << "        subType: " << _subType << endl;
  out << spacer << "        mode: " << _mode << endl;
  out << spacer << "        flags: " << _flags << endl;
  out << spacer << "        majorVersion: " << _majorVersion << endl;
  out << spacer << "        minorVersion: " << _minorVersion << endl;
  out << spacer << "        serialNum: " << _serialNum << endl;
  out << spacer << "        nParts: " << _nParts << endl;

}

/////////////////////
// print part headers

void DsMessage::printPartHeaders(ostream &out, const char *spacer) const
{
  for (ssize_t i = 0; i < getNParts(); i++) {
    DsMsgPart *part = getPart(i);
    part->printHeader(out, spacer, i);
  }
}

///////////////////////////////////////////////////////////////
// print part headers, using strings to label IDs as appropriate
// Labels are passed in as a map.

void DsMessage::printPartHeaders(ostream &out, const char *spacer,
				 const PartHeaderLabelMap &labels) const
{
  for (ssize_t i = 0; i < getNParts(); i++) {
    DsMsgPart *part = getPart(i);
    int id = part->getType();
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

void DsMessage::printHeader(ostream *out, const char *spacer) const
{
  printHeader(*out, spacer);
}

////////////////////////////////////////////////
// allocate the buffer for the assembled message

void DsMessage::_allocParts(const ssize_t n_parts)
  
{
  while (n_parts > (int) _parts.size()) {
    DsMsgPart *newPart = new DsMsgPart(_memModel == CopyMem);
    _parts.push_back(newPart);
  }
}

////////////////////////////////////////////////
// allocate the buffer for the assembled message

void DsMessage::_allocAssembledMsg()

{
  if (_lengthAssembled > _nAssembledAlloc) {
    if (_assembledMsg) {
      delete[] _assembledMsg;
    }
    _assembledMsg = new ui08[_lengthAssembled];
    _nAssembledAlloc = _lengthAssembled;
  } else {
    // case where the allocated value is bigger
    // than the needed value. In some cases
    // _assembledMsg can be NULL, so it needs
    // to be created here. (coverity bug fix)
    if (_assembledMsg == NULL) {
      _assembledMsg = new ui08[_nAssembledAlloc];
    }
  }
  
  memset(_assembledMsg, 0, _lengthAssembled);
}

/////////////////////////////
// copy
//

DsMessage &DsMessage::_copy(const DsMessage &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _type = rhs._type;
  _subType = rhs._subType;
  _mode = rhs._mode;
  _flags = rhs._flags;
  _majorVersion = rhs._majorVersion;
  _minorVersion = rhs._minorVersion;
  _serialNum = rhs._serialNum;
  _category = rhs._category;
  _error = rhs._error;
  _debug = rhs._debug;

  // copy always uses the local model

  _memModel = CopyMem;

  // copy in the parts
  
  for (ssize_t ii = 0; ii < rhs._nParts; ii++) {
    DsMsgPart *part = new DsMsgPart(*rhs._parts[ii]);
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

