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
// MultBufPart.cc
//
// From DsMsgPart.cc
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307-3000
//
// March 2000
//
////////////////////////////////////////////////////////////

#include <cstring>
#include <dataport/bigend.h>
#include <toolsa/TaStr.hh>
#include <rapformats/MultBufPart.hh>
using namespace std;

///////////////
// constructor

MultBufPart::MultBufPart()

{
  _type   = -1;
  _length = 0;
  _offset = 0;
}

//////////////
// destructor

MultBufPart::~MultBufPart()

{

}

////////////////////////////////////////////////////////////
// load a part from an incoming buffer which is assumed to
// be in BE byte order
//
// Returns 0 on success, -1 on error
// Error occurs if end of part is beyond end of buffer.
// Error retrieved with getErrStr().

int MultBufPart::loadFromBuf(const int part_num,
			     const void *in_buf,
			     const int buf_len)

{

  _errStr = "ERROR - MultBufPart::loadFromBuf.\n";
  
  // copy in the part header
  
  ui08 *inBuf = (ui08 *) in_buf;
  MultBuf::part_hdr_t partHdr;
  memcpy(&partHdr,
	 (inBuf + sizeof(MultBuf::header_t) +
	  part_num * sizeof(MultBuf::part_hdr_t)),
	 sizeof(MultBuf::part_hdr_t));
  MultBuf::_BE_to_part_hdr(partHdr);
  
  _type = partHdr.type;
  _length = partHdr.len;
  _offset = partHdr.offset;
  
  if (buf_len > 0 && (_offset + _length) > buf_len) {
    TaStr::AddInt(_errStr, "  End of part ", part_num);
    _errStr += " is beyond end of buffer.\n";
    TaStr::AddInt(_errStr, "  End of part offset: ", _offset + _length, true);
    TaStr::AddInt(_errStr, "  End of buffer offset: ", buf_len, true);
    return -1;
  }

  _buf.free();
  _buf.add((inBuf + _offset), _length);

  return 0;

}

////////////////////////////////////////////////////////////
// load a part from a memory buffer which is assumed to
// be in host byte order

void MultBufPart::loadFromMem(const int part_type,
			      const int len,
			      const void *in_mem)

{

  _type = part_type;
  _length = len;
  
  _buf.free();
  _buf.add(in_mem, _length);

}

///////////////
// print header

void MultBufPart::printHeader(ostream &out,
			      const int num /* = -1*/,
			      const string &spacer /* = ""*/ ) const
{
  if (num >= 0) {
    out << spacer << "---- part: " << num << " ----" << endl;
  }
  out << spacer << "  type:   " << _type << endl;
  out << spacer << "  length: " << _length << endl;
  out << spacer << "  offset: " << _offset << endl;
}



