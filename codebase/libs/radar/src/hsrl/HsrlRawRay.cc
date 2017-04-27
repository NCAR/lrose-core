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
#include <Radx/RadxTime.hh>
#include <cstring>
using namespace std;

///////////////////////////////////////////////////////////////
// default constructor

HsrlRawRay::HsrlRawRay() :
  _timeSecs(0),
  _subSecs(0),
  _telescopeLocked(false),
  _telescopeDirn(0),  // pointing down
  _totalEnergy(0),
  _polAngle(0.0),
  _nGates(0),
  _fieldLen(0),
  _bufLen(0),
  _packetBuf(NULL)

{
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
// Serialize object into buffer for transmission.
// After calling, call getBufPtr() and getBufLen()
// to get the details of the buffer.

void HsrlRawRay::serialize()

{
  
  // compute buf sizes
  
  int fieldLen = _nGates * sizeof(float32);
  int bufLen = sizeof(_tcp_hdr_t) + _NFIELDS * fieldLen;

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

  _tcp_hdr_t hdr;
  memset(&hdr, 0, sizeof(hdr));

  hdr.id = COOKIE;
  hdr.len_bytes = _bufLen;
  hdr.version_num = _HEADER_VERSION;

  hdr.time_secs_utc = _timeSecs;
  hdr.time_nano_secs = (int64_t) (_subSecs * 1.0e9 + 0.5);
  
  hdr.telescope_locked = _telescopeLocked;
  hdr.telescope_dirn = _telescopeDirn;

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
  
}

///////////////////////////////////////////////////////////////
// deserialize from buffer into the object
// returns 0 on success, -1 on error

int HsrlRawRay::deserialize(const void *buffer, int bufLen)

{

  // save to local buffer

  if (bufLen != _bufLen) {
    if (_packetBuf != NULL) {
      delete[] _packetBuf;
    }
    _packetBuf = NULL;
  }

  // set lengths

  _bufLen = bufLen;

  // allocate buffer space
  
  if (_packetBuf == NULL) {
    _packetBuf = new char[_bufLen];
  }
  
  // copy it in

  memcpy(_packetBuf, buffer, _bufLen);

  // check we have enough len for header

  _tcp_hdr_t hdr;
  if (bufLen < (int) sizeof(hdr)) {
    cerr << "ERROR - HsrlRawRay::deserialize()" << endl;
    cerr << "  buffer too short" << endl;
    cerr << "  bufLen: " << bufLen << endl;
    cerr << "  min valid len: " << sizeof(hdr) << endl;
    return -1;
  }

  // copy in header

  memcpy(&hdr, buffer, sizeof(hdr));

  // check if we need to swap
  
  int64_t id = hdr.id;
  if (id != COOKIE) {
    _SwapHdr(&hdr);
    if (hdr.id != COOKIE) {
      cerr << "ERROR - HsrlRawRay::deserialize()" << endl;
      cerr << "  unrecognized id: " << id << endl;
      return -1;
    }
  }

  // set members

  _timeSecs = hdr.time_secs_utc;
  _subSecs = (double) hdr.time_nano_secs / 1.0e9;
  _telescopeLocked = hdr.telescope_locked;
  _telescopeDirn = hdr.telescope_dirn;

  _totalEnergy = hdr.total_energy;
  _polAngle = hdr.pol_angle;

  // re-check the buffer length

  int nGates = hdr.n_gates;
  _fieldLen = nGates * sizeof(float32);
  int bufSizeNeeded = sizeof(hdr) + _NFIELDS * _fieldLen;
  if (bufSizeNeeded > bufLen) {
    cerr << "ERROR - HsrlRawRay::deserialize()" << endl;
    cerr << "  buffer too short" << endl;
    cerr << "  bufLen: " << bufLen << endl;
    cerr << "  min valid len: " << bufSizeNeeded << endl;
    return -1;
  }

  // populate the fields

  int offset = sizeof(hdr);
  const float32 *combinedHi = (float32 *) ((char *) buffer + offset);

  offset += _fieldLen;
  const float32 *combinedLo = (float32 *) ((char *) buffer + offset);

  offset += _fieldLen;
  const float32 *molecular = (float32 *) ((char *) buffer + offset);

  offset += _fieldLen;
  const float32 *cross = (float32 *) ((char *) buffer + offset);

  setFields(nGates, combinedHi, combinedLo, molecular, cross);

  return 0;

}

//////////////////////////////////////////////////////////////////////////
// Swap the TCP header

void HsrlRawRay::_SwapHdr(_tcp_hdr_t *hdr)
{

  // swap the 64-bit words

  _Swap64(hdr, 80);

  // swap the 32-bit words

  _Swap32((char *) hdr + 80, 48);

}
  
//////////////////////////////////////////////////////////////////////////
//  Perform in-place 64-bit word byte swap, to produce
//  BE representation from machine representation, or vice-versa.
//  Array must be aligned.

void HsrlRawRay::_Swap64(void *array, size_t nbytes)
  
{

  int ndoubles = nbytes / 8;
  char *ptr = (char*) array;
  for (int i = 0; i < ndoubles; i++) {
    
    // Copy the 8 bytes to 2 ui32's - Reversing 1st & 2nd
    // PTR                 L1      L2
    // 1 2 3 4 5 6 7 8 ->  5 6 7 8 1 2 3 4

    int32_t l1,l2;
    memcpy((void*)&l2,(void*)ptr,4);
    memcpy((void*)&l1,(void*)(ptr+4),4);

    // Reverse the 4 bytes of each ui32
    // 5 6 7 8  -> 8 7 6 5
    l1 = (((l1 & 0xff000000) >> 24) |
	  ((l1 & 0x00ff0000) >> 8) |
	  ((l1 & 0x0000ff00) << 8) |
	  ((l1 & 0x000000ff) << 24));

    // 1 2 3 4 -> 4 3 2 1
    l2 = (((l2 & 0xff000000) >> 24) |
	  ((l2 & 0x00ff0000) >> 8) |
	  ((l2 & 0x0000ff00) << 8) |
	  ((l2 & 0x000000ff) << 24));


    // Copy the reversed value back into place
    memcpy(ptr, &l1, 4);
    memcpy(ptr + 4, &l2, 4);

    ptr+=8;  // Move to the next 8 byte value

  } // i

}

//////////////////////////////////////////////////////////////////////////
//  Performs an in-place 32-bit word byte swap, if necessary, to produce
//  BE representation from machine representation, or vice-versa.
//  Array must be aligned.
 
void HsrlRawRay::_Swap32(void *array, size_t nbytes)
  
{

  int nlongs = nbytes / sizeof(int32_t);
  int32_t *this_long = (int32_t *) array;
  
  for (int i = 0; i < nlongs; i++) {

    int32_t l = *this_long;
    
    *this_long = (((l & 0xff000000) >> 24) |
		  ((l & 0x00ff0000) >> 8) |
		  ((l & 0x0000ff00) << 8) |
		  ((l & 0x000000ff) << 24));
    
    this_long++;

  }

}

//////////////////////////////////////////////////////////////////////////
//  Print metadata
 
void HsrlRawRay::printMetaData(ostream &out)
  
{

  out << "========= HsrlRawRay =============" << endl;

  RadxTime dataTime(_timeSecs, _subSecs);
  out << "  time: " << dataTime.asString(3) << endl;
  out << "  telescopeLocked: " << (_telescopeLocked?"Y":"N") << endl;
  if (_telescopeDirn == 1) {
    out << "  telescope pointing up" << endl;
  } else {
    out << "  telescope pointing down" << endl;
  }
  out << "  totalEnergy: " << _totalEnergy << endl;
  out << "  polAngle: " << _polAngle << endl;
  out << "  nGates: " << _nGates << endl;
  out << "  size of combinedHi: " << _combinedHi.size() << endl;
  out << "  size of combinedLo: " << _combinedLo.size() << endl;
  out << "  size of molecular: " << _molecular.size() << endl;
  out << "  size of cross: " << _cross.size() << endl;
  out << "  bufLen: " << _bufLen << endl;

  out << "==================================" << endl;

}

//////////////////////////////////////////////////////////////////////////
//  Print tcp message header in the current buffer
 
void HsrlRawRay::printTcpHdr(ostream &out)
  
{
  
  out << "========= HsrlRawRay TCP header =============" << endl;

  const _tcp_hdr_t *hdr = (_tcp_hdr_t *) _packetBuf;

  out << "  id: " << hdr->id << endl;
  out << "  len_bytes: " << hdr->len_bytes << endl;
  out << "  version_num: " << hdr->version_num << endl;
  out << "  time_secs_utc: " << hdr->time_secs_utc << endl;
  out << "  time_nano_secs: " << hdr->time_nano_secs << endl;
  out << "  total_energy: " << hdr->total_energy << endl;
  out << "  telescope_locked: " << hdr->telescope_locked << endl;
  out << "  telescope_dirn: " << hdr->telescope_dirn << endl;
  out << "  pol_angle: " << hdr->pol_angle << endl;
  out << "  n_gates: " << hdr->n_gates << endl;

  out << "=============================================" << endl;

}

