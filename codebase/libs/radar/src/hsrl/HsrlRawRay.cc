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
/////////////////////////////////////////////////////////////
// HsrlRawRay
//
// Holds raw data from a single HSRL ray or beam
//
// Mike Dixon, Brad Schoenrock, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2017
//
///////////////////////////////////////////////////////////////

#include <radar/HsrlRawRay.hh>
#include <cstring>
using namespace std;

///////////////////////////////////////////////////////////////
// default constructor

HsrlRawRay::HsrlRawRay()

{

  // initialize

  _packetBuf = NULL;
  _bufLen = 0;
  
}
  
///////////////////////////////////////////////////////////////
// destructor
  
HsrlRawRay::~HsrlRawRay()

{

  // clear vectors

  _combinedHi.clear();
  _combinedLo.clear();
  _molecular.clear();
  _cross.clear();

  // free up buffer

  if (_packetBuf != NULL) {
    delete[] _packetBuf;
  }

}

///////////////////////////////////////////////////////////////
// set the fields

void HsrlRawRay::setFields(int nGates,
                           const float32 *combinedHi,
                           const float32 *combinedLo,
                           const float32 *molecular,
                           const float32 *cross)

{

  _nGates = nGates;

  _combinedHi.resize(_nGates);
  memcpy(&_combinedHi[0], combinedHi, _nGates * sizeof(float32));

  _combinedLo.resize(_nGates);
  memcpy(&_combinedLo[0], combinedLo, _nGates * sizeof(float32));

  _molecular.resize(_nGates);
  memcpy(&_molecular[0], molecular, _nGates * sizeof(float32));

  _cross.resize(_nGates);
  memcpy(&_cross[0], cross, _nGates * sizeof(float32));

}


///////////////////////////////////////////////////////////////
// serialize into buffer for transmission

char *HsrlRawRay::serialize()

{
  
  // compute buf sizes
  
  int fieldLen = _nGates * sizeof(float32);
  int bufLen = sizeof(tcp_hdr_t) + _nFields * fieldLen;

  // check buffer space
  
  if (fieldLen != _fieldLen || bufLen != _bufLen) {
    if (_packetBuf != NULL) {
      delete[] _packetBuf;
    }
    _packetBuf = NULL;
  }

  // set lengths

  _fieldLen = fieldLen;
  _bufLen = bufLen;

  // allocate buffer space
  
  if (_packetBuf == NULL) {
    _packetBuf = new char[_bufLen];
  }

  // set the header

  tcp_hdr_t hdr;
  memset(&hdr, 0, sizeof(hdr));

  hdr.id = cookie;
  hdr.len_bytes = _bufLen;
  _seqNum++;
  hdr.seq_num = _seqNum;
  hdr.version_num = 1;

  hdr.time_secs_utc = _timeSecs;
  hdr.time_nano_secs = (int64_t) (_subSecs * 1.0e9 + 0.5);
  
  hdr.total_energy = _totalEnergy;
  hdr.pol_angle = _polAngle;
  hdr.n_gates = _nGates;

  // copy the header into the buffer

  int offset = 0;
  memcpy(_packetBuf + offset, &hdr, sizeof(hdr));

  // copy the fields into the buffer
  
  offset += sizeof(hdr);
  memcpy(_packetBuf + offset, &_combinedHi[0], fieldLen);
  
  offset += fieldLen;
  memcpy(_packetBuf + offset, &_combinedLo[0], fieldLen);
  
  offset += fieldLen;
  memcpy(_packetBuf + offset, &_molecular[0], fieldLen);
  
  offset += fieldLen;
  memcpy(_packetBuf + offset, &_cross[0], fieldLen);
  
  // return buffer
  
  return _packetBuf;

}


