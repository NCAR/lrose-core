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
// DsMsgPart.cc
//
// Terri Betancourt
// RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
// 
// May 1998
// 
////////////////////////////////////////////////////////////////////////////////

#include <didss/ds_message.h>
#include <didss/DsMsgPart.hh>
#include <dataport/bigend.h>
#include <cstring>
#include <cassert>
#include <iostream>
using namespace std;

///////////////
// constructor
// You must choose the memory model in the constructor.
//
// if bufIsLocal, the buffer is allocated in the object
// and the memory contents are copied in. If !bufIsLocal,
// _buf just points to the incoming memory.

DsMsgPart::DsMsgPart(int buf_is_local)

{
  _type   = -1;
  _length = 0;
  _paddedLength = 0;
  _buf = NULL;
  _nBufAlloc = 0;
  _bufIsLocal = buf_is_local;
}

/////////////////////////////
// Copy constructor
//

DsMsgPart::DsMsgPart(const DsMsgPart &rhs)
  
{
  if (this != &rhs) {
    _copy(rhs);
  }
}

//////////////
// destructor

DsMsgPart::~DsMsgPart()

{
  if (_bufIsLocal && _buf) {
    delete[] _buf;
  }
}

/////////////////////////////
// Assignment
//

DsMsgPart &DsMsgPart::operator=(const DsMsgPart &rhs)
  
{
  return _copy(rhs);
}

////////////////////////////////////////////////////////////
// load a part from an incoming message which is assumed to
// be in BE byte order
//
// If msg_len is provided, the part is checked to make
// sure it does not run over the end of the message.
//
// Returns 0 on success, -1 on error
// Error occurs if end of part is beyond end of message.

int DsMsgPart::loadFromMsg(const ssize_t part_num,
			   const void *in_msg,
			   const ssize_t msg_len /* = -1*/ )

{

  ui08 *inBuf = (ui08 *) in_msg;

  DsMsgPart_t msgPart;

  memcpy(&msgPart,
	 inBuf + sizeof(DsMsgHdr_t) + part_num * sizeof(DsMsgPart_t),
	 sizeof(DsMsgPart_t));

  BE_to_DsMsgPart(&msgPart);

  _type = msgPart.dataType;
  _length = msgPart.len;
  _paddedLength = ((_length / 8) + 1) * 8;
  _offset = msgPart.offset;
  
  if (msg_len > 0 && (_offset + _length) > msg_len) {
    cerr << "ERROR - DsMsgPart::loadFromMsg.\n";
    cerr << "  End of part " << part_num << " is beyond end of message.\n";
    cerr << "  End of part offset: " << _offset + _length << endl;
    cerr << "  End of message offset: " << msg_len << endl;
    return (-1);
  }

  if (_bufIsLocal) {
    _allocBuf();
    memcpy(_buf, inBuf + _offset, _length);
  } else {
    _buf = inBuf + _offset;
  }

  return (0);

}

////////////////////////////////////////////////////////////
// load a part from a memory buffer which is assumed to
// be in host byte order

void DsMsgPart::loadFromMem(const int data_type,
			    const ssize_t len,
			    const void *in_mem)

{

  _type = data_type;
  _length = len;
  _paddedLength = ((_length / 8) + 1) * 8;
  
  if (_bufIsLocal) {
    _allocBuf();
    memcpy(_buf, in_mem, _length);
  } else {
    _buf = (ui08 *) in_mem;
  }

}

///////////////
// print header

void DsMsgPart::printHeader(ostream &out, const char *spacer,
			    ssize_t num /* = -1*/ ) const
{
  if (num >= 0) {
    out << spacer << "---- part: " << num << " ----" << endl;
  }
  out << spacer << "  type:   " << getType() << endl;
  out << spacer << "  length: " << getLength() << endl;
  out << spacer << "  padded: " << getPaddedLength() << endl;
  out << spacer << "  offset: " << getOffset() << endl;
}

void DsMsgPart::printHeader(ostream &out, const char *spacer,
			    const string &label,
			    ssize_t num /* = -1*/ ) const
{
  if (num >= 0) {
    out << spacer << "---- part: " << num << " ----" << endl;
  }
  out << spacer << "  label:   " << label << endl;
  out << spacer << "  type:   " << getType() << endl;
  out << spacer << "  length: " << getLength() << endl;
  out << spacer << "  padded: " << getPaddedLength() << endl;
  out << spacer << "  offset: " << getOffset() << endl;
}

////////////////////////////
// allocate the local buffer

void DsMsgPart::_allocBuf()

{
  assert(_bufIsLocal);
  if ((_length > _nBufAlloc) || (_length < _nBufAlloc/2)) {
    if (_buf) {
      delete[] _buf;
    }
    _buf = new ui08[_length];
    _nBufAlloc = _length;
  }
}

/////////////////////////////
// copy
//

DsMsgPart &DsMsgPart::_copy(const DsMsgPart &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  _type = rhs._type;
  _length = rhs._length;
  _paddedLength = rhs._paddedLength;
  _offset = rhs._offset;
  
  // the copy never has local buffer
  // so allocate buffer and copy data in

  _buf = NULL;
  _nBufAlloc = 0;
  _bufIsLocal = true;
  _allocBuf();
  memcpy(_buf, rhs._buf, _length);

  return *this;

}
