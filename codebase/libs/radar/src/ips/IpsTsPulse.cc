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
///////////////////////////////////////////////////////////////
// IpsTsPulse.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Aug 2019
//
////////////////////////////////////////////////////////////////
//
// Support for Independent Pulse Sampling.
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <toolsa/mem.h>
#include <toolsa/sincos.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/IpsTsPulse.hh>
using namespace std;

const double IpsTsPulse::PHASE_MULT = 180.0 / 32767.0;

// Constructor

IpsTsPulse::IpsTsPulse(IpsTsInfo &info,
                       IpsTsDebug_t debug) :
        _info(info),
        _debug(debug)
  
{

  ips_ts_pulse_header_init(_hdr);

  _georefActive = false;
  _georefCorrActive = false;
  _invertHvFlag = false;

  _ftime = 0.0;
  _prf = 0.0;
  _phaseDiff[0] = 0.0;
  _phaseDiff[1] = 0.0;

  _iqData = NULL;

  for (int ii = 0; ii < IPS_TS_MAX_CHAN; ii++) {
    _chanIq[ii]= NULL;
  }

  _packedEncoding = ips_ts_iq_encoding_t::LAST;
  _packedScale = 1.0;
  _packedOffset = 0.0;
  _packed = NULL;

  // initialize mutexes

  _nClients = 0;
  pthread_mutex_init(&_nClientsMutex, NULL);

}

/////////////////////////////
// Copy constructor

IpsTsPulse::IpsTsPulse(const IpsTsPulse &rhs) :
	_info(rhs._info),
        _nClients(0)

{
  if (this != &rhs) {
    _copy(rhs);
    pthread_mutex_init(&_nClientsMutex, NULL);
  }
}

/////////////////////////////
// destructor

IpsTsPulse::~IpsTsPulse()

{
  _clearIq();
  _clearPacked();
  pthread_mutex_destroy(&_nClientsMutex);
}

/////////////////////////////
// Assignment
//

IpsTsPulse &IpsTsPulse::operator=(const IpsTsPulse &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////////////////////
// clear all

void IpsTsPulse::clear()

{
  
  _clearIq();
  _clearPacked();

  ips_ts_pulse_header_init(_hdr);

  _georefActive = false;
  _georefCorrActive = false;

}

////////////////////////////////////////////////////////////
// set time

void IpsTsPulse::setTime(time_t secs, int nano_secs) {
  ips_ts_set_packet_time(_hdr.packet, secs, nano_secs);
}

void IpsTsPulse::setTimeToNow() {
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set headers directly

void IpsTsPulse::setHeader(const ips_ts_pulse_header_t &hdr) {
  _hdr = hdr;
  _hdr.packet.id = IPS_TS_PULSE_HEADER_ID;
  _hdr.packet.len_bytes = sizeof(ips_ts_pulse_header_t);
  _hdr.packet.version_num = 1;
}

////////////////////////////////////////////////////////////
// set radar ID

void IpsTsPulse::setRadarId(int id) {

  _hdr.packet.radar_id = id;

}

////////////////////////////////////////////////////////////
// set sequence number for each header

void IpsTsPulse::setPktSeqNum(si64 pkt_seq_num) {
  _hdr.packet.seq_num = pkt_seq_num;
}

///////////////////////////////////////////////////////////
// set from pulse buffer, swapping as required
// optionally convert iq data to floats
// Returns 0 on success, -1 on failure

int IpsTsPulse::setFromBuffer(const void *buf, int len,
                              bool convertToFloat)
  
{

  // check validity of packet
  
  int packet_id;
  if (ips_ts_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - IpsTsPulse::setFromBuffer" << endl;
    fprintf(stderr, "  Bad packet, id: 0x%x\n", packet_id);
    cerr << "             len: " << len << endl;
    cerr << "            type: " << ips_ts_packet_id_to_str(packet_id) << endl;
    return -1;
  }

  // swap packet as required, using a copy to preserve const

  char *copy = new char[len];
  memcpy(copy, buf, len);
  ips_ts_packet_swap(copy, len);

  if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
    ips_ts_packet_print(stderr, copy, len);
  }

  if (packet_id != IPS_TS_PULSE_HEADER_ID) {
    cerr << "ERROR - IpsTsPulse::setFromBuffer" << endl;
    fprintf(stderr, "  Incorrect packet id: 0x%x\n", packet_id);
    cerr << "                  len: " << len << endl;
    cerr << "                 type: " << ips_ts_packet_id_to_str(packet_id) << endl;
    delete[] copy;
    return -1;
  }

  memcpy(&_hdr, copy, sizeof(ips_ts_pulse_header_t));

  // derive

  if (_hdr.elevation > 180) {
    _hdr.elevation -= 360.0;
  }

  int nGatesTotal = _hdr.n_gates;
  int nDataMin = _hdr.n_channels * (nGatesTotal) * 2;
  if ((int) _hdr.n_data < nDataMin) {
    if (_debug >= IpsTsDebug_t::VERBOSE) {
      cerr << "WARNING: IpsTsPulse::setFromTsBuffer: ndata set to low" << endl;
      cerr << "  nData in pulse header: " << _hdr.n_data << endl;
      cerr << "  Setting to min required: " << nDataMin << endl;
    }
    _hdr.n_data = nDataMin;
  }
  
  _ftime = (double) _hdr.packet.time_secs_utc +
    _hdr.packet.time_nano_secs / 1.0e9;

  _prf = 1.0 / _hdr.prt;
  
  // load up IQ data
  ips_ts_iq_encoding_t iqEncoding =
    static_cast<ips_ts_iq_encoding_t>(_hdr.iq_encoding);
  
  int requiredLen = 
    (int) sizeof(ips_ts_pulse_header_t) + _hdr.n_data * sizeof(si16);

  if (iqEncoding == ips_ts_iq_encoding_t::FL32 ||
      iqEncoding == ips_ts_iq_encoding_t::SCALED_SI32) {
    requiredLen =
      (int) sizeof(ips_ts_pulse_header_t) + _hdr.n_data * sizeof(fl32);
  }
  if (len < requiredLen) {
    cerr << "ERROR - IpsTsPulse::setFromTsPulse" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << requiredLen << endl;
    cerr << "sizeof(ips_ts_pulse_header_t): "
         << sizeof(ips_ts_pulse_header_t) << endl; 
    ips_ts_pulse_header_print(stderr, _hdr);
    delete[] copy;
    return -1;
  }
  
  if (iqEncoding == ips_ts_iq_encoding_t::FL32) {
    
    _clearPacked();
    fl32 *iq = (fl32 *) (copy + sizeof(ips_ts_pulse_header_t));
    _iqData = (fl32 *) _iqBuf.load(iq, _hdr.n_data * sizeof(fl32));

  } else if (iqEncoding == ips_ts_iq_encoding_t::SCALED_SI32) {
    
    _clearPacked();
    si32 *siq = (si32 *) (copy + sizeof(ips_ts_pulse_header_t));
    _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
    fl32 *iq = _iqData;
    double scale = _hdr.scale;
    double offset = _hdr.offset;
    for (int ii = 0; ii < _hdr.n_data; ii++, siq++, iq++) {
      *iq = *siq * scale + offset;
    }

  } else {

    _clearIq();
    si16 *packed = (si16 *) (copy + sizeof(ips_ts_pulse_header_t));
    _packed = (si16 *) _packedBuf.load(packed, _hdr.n_data * sizeof(si16));

  }

  _packedEncoding = iqEncoding;
  _packedScale = _hdr.scale;
  _packedOffset = _hdr.offset;
  
  if (convertToFloat) {
    convertToFL32();
  }

  _setDataPointers();

  _checkRangeMembers();

  delete[] copy;
  return 0;

}

///////////////////////////////////////////////////////////
// set IQ data as floats

void IpsTsPulse::setIqFloats(int nGates,
                             int nChannels,
                             const fl32 *iq)
  
{

  _hdr.n_gates = nGates;
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = static_cast<si32>(ips_ts_iq_encoding_t::FL32);
  
  _iqData = (fl32 *) _iqBuf.load(iq, _hdr.n_data * sizeof(fl32));

  _clearPacked();
  _packedEncoding = ips_ts_iq_encoding_t::FL32;
  _packedScale = _hdr.scale;
  _packedOffset = _hdr.offset;

  _setDataPointers();

}

///////////////////////////////////////////////////////////
// set IQ data as floats, converting from ScaledSi32

void IpsTsPulse::setIqFromScaledSi32(int nGates,
                                     int nChannels,
                                     const si32 *siq)
  
{

  _hdr.n_gates = nGates;
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = static_cast<si32>(ips_ts_iq_encoding_t::FL32);
  
  _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
  fl32 *iq = _iqData;
  double scale = _hdr.scale;
  double offset = _hdr.offset;
  for (int ii = 0; ii < _hdr.n_data; ii++, siq++, iq++) {
    *iq = *siq * scale + offset;
  }
  
  _clearPacked();
  _packedEncoding = ips_ts_iq_encoding_t::FL32;
  _packedScale = 1.0;
  _packedOffset = 0.0;

  _setDataPointers();
  
}

///////////////////////////////////////////////////////////
// set IQ data as packed si16
  
void IpsTsPulse::setIqPacked(int nGates, int nChannels, 
                             ips_ts_iq_encoding_t encoding,
                             const si16 *packed,
                             double scale = 1.0,
                             double offset = 0.0)
  
{
  
  _hdr.n_gates = nGates;
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = static_cast<si32>(encoding);
  _hdr.scale = scale;
  _hdr.offset = offset;
  
  _packed = (si16 *) _packedBuf.load(packed, _hdr.n_data * sizeof(si16));
  
  _clearIq();
  _packedEncoding = encoding;
  _packedScale = scale;
  _packedOffset = offset;

  _setDataPointers();

}

///////////////////////////////////////////////////////////
// Swap the PRT values in the pulse header.
// STAGGERED PRT mode only.
// There are two prt values in the pulse header, prt and prt next.
// prt is the time SINCE the PREVIOUS pulse
// prt_next is the time TO to the NEXT next pulse
// If the incoming data uses a different convention, this call
// can be used to swap the values
  
void IpsTsPulse::swapPrtValues()

{
  double prt = _hdr.prt_next;
  double prt_next = _hdr.prt;
  _hdr.prt_next = prt_next;
  _hdr.prt = prt;
}

///////////////////////////////////////////////////////////
// convert packed data to float32

void IpsTsPulse::convertToFL32()
  
{

  if (_packedEncoding == ips_ts_iq_encoding_t::FL32) {
    _setDataPointers();
    _fixZeroPower();
    return;
  }

  _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
  
  ips_ts_iq_encoding_t iqEncoding = static_cast<ips_ts_iq_encoding_t>(_hdr.iq_encoding);
  if (iqEncoding == ips_ts_iq_encoding_t::SCALED_SI16) {
    
    // compute from signed scaled values
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0; ii < _hdr.n_data; ii++) {
      _iqData[ii] = packed[ii] * _packedScale + _packedOffset;
      if (fabs(_iqData[ii]) == 0.0) {
        _iqData[ii] = 1.0e-20;
      }
    }
    
  } else if (iqEncoding == ips_ts_iq_encoding_t::DBM_PHASE_SI16) {

    // compute from power and phase
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0; ii < _hdr.n_data; ii += 2) {
      double powerDbm = packed[ii] * _packedScale + _packedOffset;
      double power = pow(10.0, powerDbm / 10.0);
      double phaseDeg = packed[ii+1] * PHASE_MULT;
      double sinPhase, cosPhase;
      ta_sincos(phaseDeg * DEG_TO_RAD, &sinPhase, &cosPhase);
      double mag = sqrt(power);
      if (mag == 0.0) {
        mag = 1.0e-20;
      }
      _iqData[ii] = mag * cosPhase;
      _iqData[ii+1] = mag * sinPhase;

    }
    
  }
  
  _hdr.scale = 1.0;
  _hdr.offset = 0.0;
  _hdr.iq_encoding = static_cast<si32>(ips_ts_iq_encoding_t::FL32);
  
  _setDataPointers();

}

///////////////////////////////////////////////////////////
// get IQ at a gate

void IpsTsPulse::getIq0(int gateNum,
                        fl32 &ival, fl32 &qval) const
{
  return getIq(0, gateNum, ival, qval);
}

void IpsTsPulse::getIq1(int gateNum,
                        fl32 &ival, fl32 &qval) const
{
  return getIq(1, gateNum, ival, qval);
}

void IpsTsPulse::getIq2(int gateNum,
                        fl32 &ival, fl32 &qval) const
{
  return getIq(2, gateNum, ival, qval);
}

void IpsTsPulse::getIq3(int gateNum,
                        fl32 &ival, fl32 &qval) const
{
  return getIq(3, gateNum, ival, qval);
}

///////////////////////////////////////////////////////////
// get IQ at a gate and channel


void IpsTsPulse::getIq(int chanNum,
                       int gateNum,
                       fl32 &ival, fl32 &qval) const
  
{

  // initialize to missing

  ival = IPS_TS_MISSING_FLOAT;
  qval = IPS_TS_MISSING_FLOAT;

  // check for valid geometry

  if (chanNum >= getNChannels()) {
    return;
  }

  if (gateNum < 0 || gateNum >= getNGates()) {
    return;
  }

  // compute data offset into iq array

  int offset = (chanNum * _hdr.n_gates + gateNum) * 2;

  // floats? return as is

  if (_packedEncoding == ips_ts_iq_encoding_t::FL32) {

    ival = _iqData[offset];
    qval = _iqData[offset + 1];

  } else if (_packedEncoding == ips_ts_iq_encoding_t::SCALED_SI16) {
    
    // unscale
    
    si16 ipacked = _packed[offset];
    si16 qpacked = _packed[offset + 1];
    ival = ipacked * _packedScale + _packedOffset;
    qval = qpacked * _packedScale + _packedOffset;
    
  } else if (_packedEncoding == ips_ts_iq_encoding_t::DBM_PHASE_SI16) {

    // compute from power and phase
    
    double powerDbm = _packed[offset] * _packedScale + _packedOffset;
    double power = pow(10.0, powerDbm / 10.0);
    double phaseDeg = _packed[offset + 1] * PHASE_MULT;
    double sinPhase, cosPhase;
    ta_sincos(phaseDeg * DEG_TO_RAD, &sinPhase, &cosPhase);
    double mag = sqrt(power);
    ival = mag * cosPhase;
    qval = mag * sinPhase;

  }

}

///////////////////////////////////////////////////////////
// convert to specified packing

void IpsTsPulse::convertToPacked(ips_ts_iq_encoding_t encoding)
  
{
  
  if (encoding == _packedEncoding) {
    // nothing to do
    _hdr.scale = _packedScale;
    _hdr.offset = _packedOffset;
    _hdr.iq_encoding = static_cast<si32>(_packedEncoding);
    return;
  }
  
  if (encoding == ips_ts_iq_encoding_t::FL32 && _iqData != NULL) {
    // use float data as packed data
    _clearPacked();
    _hdr.scale = 1.0;
    _hdr.offset = 0.0;
    _hdr.iq_encoding = static_cast<si32>(ips_ts_iq_encoding_t::FL32);
    _packedScale = 1.0;
    _packedOffset = 0.0;
    _packedEncoding = ips_ts_iq_encoding_t::FL32;
    return;
  }

  if (_iqData == NULL) {
    // make sure we have float 32 data available
    convertToFL32();
  }

  // prepare packed buffer
  
  _packed = (si16 *) _packedBuf.prepare(_hdr.n_data * sizeof(si16));
  
  // fill packed array
  
  if (encoding == ips_ts_iq_encoding_t::SCALED_SI16) {
    
    // compute max absolute val
    
    double maxAbsVal = -1.0e99;
    fl32 *iq = _iqData;
    for (int ii = 0; ii < _hdr.n_data; ii++, iq++) {
      double absVal = fabs(*iq);
      if (absVal > maxAbsVal) maxAbsVal = absVal;
    }

    // compute scale and offset

    double scale = maxAbsVal / 32767.0;
    double offset = 0;
    
    // convert to scaled signed int16
    
    si16 *packed = (si16 *) _packed;
    iq = _iqData;
    for (int ii = 0; ii < _hdr.n_data; ii++, iq++, packed++) {
      int packedVal = (int) floor(*iq / scale + 0.5);
      if (packedVal < -32767) {
        packedVal = -32767;
      } else if (packedVal > 32767) {
        packedVal = 32767;
      }
      *packed = (si16) packedVal;
    }
    
    // save scale and offset

    _packedScale = scale;
    _packedOffset = offset;
    
  } else if (encoding == ips_ts_iq_encoding_t::DBM_PHASE_SI16) {

    // compute power and phase, save in arrays
    // also compute min and max power in db
    
    double minPowerDb = 9999.0;
    double maxPowerDb = -9999.0;

    double *powerDbmArray = new double[_hdr.n_data / 2];
    double *phaseDegArray = new double[_hdr.n_data / 2];
    
    for (int ii = 0, jj = 0; ii < _hdr.n_data / 2; ii++, jj += 2) {
      
      double ival = _iqData[jj];
      double qval = _iqData[jj+1];
      
      double power = ival * ival + qval * qval;
      double powerDbm = 10.0 * log10(power);
      if (powerDbm < minPowerDb) minPowerDb = powerDbm;
      if (powerDbm > maxPowerDb) maxPowerDb = powerDbm;
      powerDbmArray[ii] = powerDbm;
      
      double phaseDeg = 0.0;
      if (ival != 0 || qval != 0) {
        phaseDeg = atan2(qval, ival) * RAD_TO_DEG;
      }
      phaseDegArray[ii] = phaseDeg;
      
    } // ii
    
    // compute scale and offset
    
    double powerRange = maxPowerDb - minPowerDb;
    double powerScale = powerRange / 65535.0;
    double powerOffset = (maxPowerDb + minPowerDb) / 2.0; // mid point
    
    // compute from power and phase
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0, jj = 0; ii < _hdr.n_data / 2; ii++, jj += 2) {

      int packedPower =
        (int) floor((powerDbmArray[ii] - powerOffset) / powerScale + 0.5);
      if (packedPower < -32767) {
	packedPower = -32767;
      } else if (packedPower > 32767) {
	packedPower = 32767;
      }
      packed[jj] = (si16) packedPower;
      
      int packedPhase = (int) (phaseDegArray[ii] / PHASE_MULT + 0.5);
      if (packedPhase < -32767) {
	packedPhase = -32767;
      } else if (packedPhase > 32767) {
	packedPhase = 32767;
      }
      packed[jj+1] = (si16) packedPhase;
      
    }
    
    _packedScale = powerScale;
    _packedOffset = powerOffset;

    delete[] powerDbmArray;
    delete[] phaseDegArray;

  }

  // save values

  _packedEncoding = encoding;

  _hdr.scale = _packedScale;
  _hdr.offset = _packedOffset;
  _hdr.iq_encoding = static_cast<si32>(_packedEncoding);

}

///////////////////////////////////////////////////////////
// convert to scaled si16 packing

void IpsTsPulse::convertToScaledSi16(double scale,
                                     double offset)
  
{
  
  if (_packedEncoding != ips_ts_iq_encoding_t::FL32 || _iqData == NULL) {
    // make sure we have float 32 data available
    convertToFL32();
  }
  
  // prepare packed buffer
  
  _packed = (si16 *) _packedBuf.prepare(_hdr.n_data * sizeof(si16));
  
  // convert to scaled signed int16
  
  si16 *packed = (si16 *) _packed;
  fl32 *iq = _iqData;
  for (int ii = 0; ii < _hdr.n_data; ii++, iq++, packed++) {
    int packedVal = (int) floor(*iq / scale + 0.5);
    if (packedVal < -32767) {
      packedVal = -32767;
    } else if (packedVal > 32767) {
      packedVal = 32767;
    }
    *packed = (si16) packedVal;
  }
  
  // save scale and offset
  
  _packedScale = scale;
  _packedOffset = offset;

  // save values

  _packedEncoding = ips_ts_iq_encoding_t::SCALED_SI16;
  _hdr.scale = _packedScale;
  _hdr.offset = _packedOffset;
  _hdr.iq_encoding = static_cast<si32>(_packedEncoding);

}

///////////////////////////////////////////////////////////
// set the scale and offset values for scaled si16 packing
// does not change the data, only the metadata

void IpsTsPulse::setScaleAndOffsetForSi16(double scale,
                                          double offset)
  
{
  
  if (_packedEncoding != ips_ts_iq_encoding_t::SCALED_SI16) {
    // do nothing, not si16 encoding
    return;
  }
  
  _packedScale = scale;
  _packedOffset = offset;

  _hdr.scale = _packedScale;
  _hdr.offset = _packedOffset;

}

///////////////////////////////////////////////////////////////////
// swaps I and Q, because they are stored in the incorrect order
// in the data arrays

void IpsTsPulse::swapIQ()
{

  int nIQ = _hdr.n_data / 2;

  if (_iqData != NULL) {
    fl32 *_i_p = _iqData;
    fl32 *_q_p = _iqData + 1;
    for (int i = 0; i < nIQ; i++) {
      fl32 tmp = *_i_p;
      *_i_p = *_q_p;
      *_q_p = tmp;
      _i_p += 2;
      _q_p += 2;
    }
  }

  if (_packed != NULL) {
    si16 *_i_p = _packed;
    si16 *_q_p = _packed + 1;
    for (int i = 0; i < nIQ; i++) {
      si16 tmp = *_i_p;
      *_i_p = *_q_p;
      *_q_p = tmp;
      _i_p += 2;
      _q_p += 2;
    }
  }

}

///////////////////////////////////////////////////////////////////
// invert Q values in the data arrays

void IpsTsPulse::invertQ()
{

  int nIQ = _hdr.n_data / 2;

  if (_iqData != NULL) {
    fl32 *_q_p = _iqData + 1;
    for (int i = 0; i < nIQ; i++) {
      *_q_p *= -1.0;
      _q_p += 2;
    }
  }

  if (_packed != NULL) {
    si16 *_q_p = _packed + 1;
    for (int i = 0; i < nIQ; i++) {
      *_q_p *= -1;
      _q_p += 2;
    }
  }

}
 
////////////////////////////////////////////////////////////
// copy the pulse width from the ts_proc in the info

void IpsTsPulse::copyPulseWidthFromTsProc() {
  
  if (_info.isTsProcessingActive()) {
    _hdr.pulse_width_us = _info.getProcPulseWidthUs();
  }

}

///////////////////////////////////////////////////////////
// assemble into a single buffer

void IpsTsPulse::assemble(MemBuf &buf) const
{
  ips_ts_pulse_header_t hdr = _hdr;
  int nbytesIq = 0;
  ips_ts_iq_encoding_t iqEncoding = static_cast<ips_ts_iq_encoding_t>(hdr.iq_encoding);
  if (iqEncoding == ips_ts_iq_encoding_t::FL32) {
    nbytesIq = hdr.n_data * sizeof(fl32);
  } else {
    nbytesIq = hdr.n_data * sizeof(si16);
  }
  hdr.packet.len_bytes = sizeof(ips_ts_pulse_header_t) + nbytesIq;
  buf.add(&hdr, sizeof(hdr));
  if (iqEncoding == ips_ts_iq_encoding_t::FL32) {
    buf.add(_iqData, nbytesIq);
  } else {
    buf.add(_packed, nbytesIq);
  }
}

/////////////////////////////////////////////////////////////////
// Check to see if horizontally polarized

bool IpsTsPulse::isHoriz() const

{

  if (!_invertHvFlag) {
    if (_hdr.hv_flag) {
      return true;
    } else {
      return false;
    }
  } else {
    if (_hdr.hv_flag) {
      return false;
    } else {
      return true;
    }
  }

}

//////////////////////////////
// get packed data
// returns NULL on error

const void *IpsTsPulse::getPackedData() const

{

  if (_hdr.iq_encoding == static_cast<si32>(ips_ts_iq_encoding_t::FL32)) {
    return _iqData;
  } else {
    return _packed;
  }

}

/////////////////////////////
// get elevation and azimuth

double IpsTsPulse::getElevation() const {
  if (std::isnan(_hdr.elevation)) {
    return _hdr.elevation;
  }
  // if (_hdr.elevation <= 180) {
  //   return _hdr.elevation;
  // } else {
  //   return (_hdr.elevation - 360.0);
  // }
  return _hdr.elevation;
}

double IpsTsPulse::getAzimuth() const {
  if (std::isnan(_hdr.azimuth)) {
    return _hdr.azimuth;
  }
  // if (_hdr.azimuth >= 0) {
  //   return _hdr.azimuth;
  // } else {
  //   return _hdr.azimuth + 360.0;
  // }
  return _hdr.azimuth;
}

double IpsTsPulse::getFixedAngle() const {
  if (std::isnan(_hdr.fixed_angle)) {
    return _hdr.fixed_angle;
  }
  if (_hdr.fixed_angle <= 180) {
    return _hdr.fixed_angle;
  } else {
    return (_hdr.fixed_angle - 360.0);
  }
}

/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

int IpsTsPulse::computePhaseDiffs
  (const deque<IpsTsPulse *> &pulseQueue, int maxTrips) const
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - IpsTsPulse::computePhaseDiffs()" << endl;
    cerr << "  You must add this pulse before calling this function" << endl;
    return -1;
  }

  _phaseDiffs.clear();
  
  // phase diffs for maxTrips previous beams
  
  int qSize = (int) pulseQueue.size();
  
  for (int ii = 0; ii < maxTrips; ii++) { // ii is (trip - 1)
    if (ii == 0) {
      _phaseDiffs.push_back(0.0);
    } else {
      if (ii <= qSize) {
	double sum = _phaseDiffs[ii-1] + pulseQueue[ii-1]->getPhaseDiff0();
	while (sum > 360.0) {
	  sum -= 360.0;
	}
	_phaseDiffs.push_back(sum);
      } else {
	// actual phase diff not available
	// use previous trip's value
	_phaseDiffs.push_back(_phaseDiffs[ii-1]);
      }
    }
  }

  return 0;

}

int IpsTsPulse::computePhaseDiffs
  (int qSize, const IpsTsPulse **pulseQueue, int maxTrips) const
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - IpsTsPulse::computePhaseDiffs()" << endl;
    cerr << "  You must add this pulse before calling this function" << endl;
    return -1;
  }

  _phaseDiffs.clear();
  
  // phase diffs for maxTrips previous beams
  
  for (int ii = 0; ii < maxTrips; ii++) { // ii is (trip - 1)
    if (ii == 0) {
      _phaseDiffs.push_back(0.0);
    } else {
      if (ii <= qSize) {
	double sum = _phaseDiffs[ii-1] + pulseQueue[ii-1]->getPhaseDiff0();
	while (sum > 360.0) {
	  sum -= 360.0;
	}
	_phaseDiffs.push_back(sum);
      } else {
	// actual phase diff not available
	// use previous trip's value
	_phaseDiffs.push_back(_phaseDiffs[ii-1]);
      }
    }
  }

  return 0;

}

////////////////////////////////////////////////////////////
// print headers

void IpsTsPulse::printHeader(FILE *out) const
{
  if (_georefActive) {
    // fprintf(out, "====>>> this georef applies to following pulse <<<====\n");
    // ips_ts_platform_georef_print(out, _georef);
    ips_ts_pulse_header_print(out, _hdr, &_georef);
  } else {
    ips_ts_pulse_header_print(out, _hdr);
  }
}

////////////////////////////////////////////////////////////
// print data

void IpsTsPulse::printData(FILE *out) const
{
  printData(out, 0, _hdr.n_gates - 1);
}

void IpsTsPulse::printData(FILE *out, int startGate, int endGate) const
{
  
  // copy this object for printing
  
  IpsTsPulse pulse(*this);
  pulse.convertToFL32();
  pulse.printHeader(out);
  for (int ichan = 0; ichan < _hdr.n_channels; ichan++) {
    pulse._printFl32Data(out, ichan, startGate, endGate);
  }
  
}

///////////////////////////////////////////////////
// write to file in IPS format
//
// returns 0 on success, -1 on failure

int IpsTsPulse::writeToFile(FILE *out) const

{

  if (out == NULL) {
    return -1;
  }
  
  MemBuf buf;
  assemble(buf);
  if (fwrite(buf.getPtr(), buf.getLen(), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - IpsTsPulse::write2File" << endl;
    cerr << "  Cannot write pulse" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}
  
/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class uses the notion of clients to decide when it should be deleted.
// If removeClient() returns 0, the object should be deleted.
// These functions are protected by a mutex for multi-threaded ops

int IpsTsPulse::addClient() const
  
{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  _nClients++;
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

int IpsTsPulse::removeClient() const

{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  _nClients--;
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

void IpsTsPulse::deleteIfUnused(IpsTsPulse *pulse)
  
{
  if (pulse->removeClient() == 0) {
    delete pulse;
  }
}

int IpsTsPulse::getNClients() const

{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

/////////////////////////////
// copy

IpsTsPulse &IpsTsPulse::_copy(const IpsTsPulse &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _hdr = rhs._hdr;

  _georef = rhs._georef;
  _georefActive = rhs._georefActive;

  _georefCorr = rhs._georefCorr;
  _georefCorrActive = rhs._georefCorrActive;

  _invertHvFlag = rhs._invertHvFlag;
  _debug = rhs._debug;
  _ftime = rhs._ftime;
  _prf = rhs._prf;
  _phaseDiff[0] = rhs._phaseDiff[0];
  _phaseDiff[1] = rhs._phaseDiff[1];
  _phaseDiffs = rhs._phaseDiffs;

  _iqBuf = rhs._iqBuf;
  _packedBuf = rhs._packedBuf;

  _packedEncoding = rhs._packedEncoding;
  _packedScale = rhs._packedScale;
  _packedOffset = rhs._packedOffset;

  _setDataPointers();

  return *this;

}

////////////////////////////////////////////////////////////
// compute az and el angles by applying the georef data

void IpsTsPulse::computeElAzFromGeoref()

{

  if (!_georefActive) {
    // no georef
    return;
  }

  double rollCorr = 0.0;
  double pitchCorr = 0.0;
  double headingCorr = 0.0;
  double driftCorr = 0.0;
  double tiltCorr = 0.0;
  double rotationCorr = 0.0;

  if (_georefCorrActive) {
    rollCorr = _georefCorr.roll_corr_deg;
    pitchCorr = _georefCorr.pitch_corr_deg;
    headingCorr = _georefCorr.heading_corr_deg;
    driftCorr = _georefCorr.drift_corr_deg;
    tiltCorr = _georefCorr.tilt_corr_deg;
    rotationCorr = _georefCorr.rot_angle_corr_deg;
  }

  double R = (_georef.roll_deg + rollCorr) * DEG_TO_RAD;
  double P = (_georef.pitch_deg + pitchCorr) * DEG_TO_RAD;
  double H = (_georef.heading_deg + headingCorr) * DEG_TO_RAD;
  double D = (_georef.drift_angle_deg + driftCorr) * DEG_TO_RAD;
  double T = H + D;
  
  double sinP = sin(P);
  double cosP = cos(P);
  double sinD = sin(D);
  double cosD = cos(D);
  
  double theta_a = 
    (_georef.rotation_angle_deg + rotationCorr) * DEG_TO_RAD;
  double tau_a =
    (_georef.tilt_deg + tiltCorr) * DEG_TO_RAD;
  double sin_tau_a = sin(tau_a);
  double cos_tau_a = cos(tau_a);
  double sin_theta_rc = sin(theta_a + R); /* roll corrected rotation angle */
  double cos_theta_rc = cos(theta_a + R); /* roll corrected rotation angle */
  
  double xsubt = (cos_theta_rc * sinD * cos_tau_a * sinP
                  + cosD * sin_theta_rc * cos_tau_a
                  -sinD * cosP * sin_tau_a);
  
  double ysubt = (-cos_theta_rc * cosD * cos_tau_a * sinP
                  + sinD * sin_theta_rc * cos_tau_a
                  + cosP * cosD * sin_tau_a);
  
  double zsubt = (cosP * cos_tau_a * cos_theta_rc
                  + sinP * sin_tau_a);
  
  // _georef.rotation_angle_deg = atan2(xsubt, zsubt) * RAD_TO_DEG;
  // _georef.tilt_deg = asin(ysubt) * RAD_TO_DEG;
  double lambda_t = atan2(xsubt, ysubt);
  double azimuthRad = fmod(lambda_t + T, M_PI * 2.0);
  double elevationRad = asin(zsubt);
  
  setElevation(elevationRad * RAD_TO_DEG);
  setAzimuth(azimuthRad * RAD_TO_DEG);

  if (_debug >= IpsTsDebug_t::EXTRAVERBOSE) {
    cerr << "========== IpsTsPulse::computeElAzFromGeoref() ========" << endl;
    cerr << "  roll: " << _georef.roll_deg << endl;
    cerr << "  pitch: " << _georef.pitch_deg << endl;
    cerr << "  heading: " << _georef.heading_deg << endl;
    cerr << "  drift_angle: " << _georef.drift_angle_deg << endl;
    cerr << "  rotation_angle: " << _georef.rotation_angle_deg << endl;
    cerr << "  tilt: " << _georef.tilt_deg << endl;
    cerr << "  el: " << getElevation() << endl;
    cerr << "  az: " << getAzimuth() << endl;
    cerr << "========================================================" << endl;
  }
  

}

////////////////////////////////////////////////////////////
// set start range and gate spacing in header, if needed

void IpsTsPulse::_checkRangeMembers() {

  if (_info.isTsProcessingActive()) {

    if (_hdr.start_range_m == 0) {
      _hdr.start_range_m = _info.getProcStartRangeM();
    }
    
    if (_hdr.gate_spacing_m == 0) {
      _hdr.gate_spacing_m = _info.getProcGateSpacingM();
    }

  }

}

///////////////////////////////////////////////////////////
// set pointers into the data
// for some data sets the burst pulse is
// in the first 2 gates

void IpsTsPulse::_setDataPointers()
  
{

  int nGatesPerChan = _hdr.n_gates;
  int nIqPerChan = nGatesPerChan * 2;

  _iqData = (fl32 *) _iqBuf.getPtr();
  _packed = (si16 *) _packedBuf.getPtr();

  _chanIq[0] = _iqData;
  
  for (int ichan = 1; ichan < _hdr.n_channels; ichan++) {
    int chanOffset = ichan * nIqPerChan;
    _chanIq[ichan] = _chanIq[0] + chanOffset;
  }

}

///////////////////////////////////////////////////////////
// Check for 0 power.
// If 0 found, set to small non-zero values.

void IpsTsPulse::_fixZeroPower()
  
{

  if (_packedEncoding != ips_ts_iq_encoding_t::FL32) {
    convertToFL32();
  }
  
  for (int ii = 0; ii < _hdr.n_data; ii++) {
    if (fabs(_iqData[ii]) == 0.0) {
      _iqData[ii] = 1.0e-20;
    }
  }

}

////////////////////
// clean up memory

void IpsTsPulse::_clearIq()
{
  _iqBuf.free();
  _iqData = NULL;
}

void IpsTsPulse::_clearPacked()
{
  _packedBuf.free();
  _packed = NULL;
}

////////////////////////////////////////////////////////////
// print data

void IpsTsPulse::_doPrintData(FILE *out, int channel, int startGate, int endGate)
{

  // if starting at beginning, include burst gates
  if (startGate <= 0) {
    startGate = 0;
  }
  if (endGate > _hdr.n_gates - 1) {
    endGate = _hdr.n_gates - 1;
  }

  fprintf(out, "====== ========== IQ data channel %d =========================\n",
	  channel);

  fprintf(out, " %5s %15s %15s %15s %15s",
	  "gate", "i(volts)", "q(volts)", "power(dBm)", "phase(deg)");
  if (_packed != NULL) {
    if (_packedEncoding == ips_ts_iq_encoding_t::SCALED_SI16) {
      fprintf(out, " %14s %14s", "i(scaled)", "q(scaled)");
    } else if (_packedEncoding == ips_ts_iq_encoding_t::DBM_PHASE_SI16) {
      fprintf(out, " %14s %14s", "power(scaled)", "phase(scaled)");
    }
  }
  fprintf(out, "\n");

  const fl32 *iq = _iqData;
  const si16 *packed = _packed;
  for (int igate = startGate, ii = startGate * 2; igate <= endGate; igate++, ii += 2) {
    
    double ival = iq[ii];
    double qval = iq[ii + 1];

    double power = ival * ival + qval * qval;
    double powerDbm = 10.0 * log10(power);
    double phaseRad = 0.0;
    if (ival != 0 || qval != 0) {
      phaseRad = atan2(qval, ival);
    }
    double phaseDeg = phaseRad * RAD_TO_DEG;

    if (igate < 0) {
      // burst gate
      fprintf(out, " %4dB %15.8e %15.8e %15.3f %15.3f",
              igate * -1, ival, qval, powerDbm, phaseDeg);
    } else {
      fprintf(out, " %5d %15.8e %15.8e %15.3f %15.3f",
              igate, ival, qval, powerDbm, phaseDeg);
    }

    if (_packed != NULL) {
      si16 ipacked = packed[ii];
      si16 qpacked = packed[ii + 1];
      if (_packedEncoding == ips_ts_iq_encoding_t::SCALED_SI16) {
	fprintf(out, " %14d %14d", ipacked, qpacked);
      } else if (_packedEncoding == ips_ts_iq_encoding_t::DBM_PHASE_SI16) {
	fprintf(out, " %14d %14d", ipacked, qpacked);
      }
    }

    fprintf(out, "\n");

  } // igate

  fprintf(out, "=================================================================\n");

}

////////////////////////////////////////////////////////////
// print float data

void IpsTsPulse::_printFl32Data(FILE *out, int channel, int startGate, int endGate)
{

  // if starting at beginning, include burst gates
  if (startGate <= 0) {
    startGate = 0;
  }
  if (endGate > _hdr.n_gates - 1) {
    endGate = _hdr.n_gates - 1;
  }
  
  fprintf(out, "====== ========== IQ data channel %d =========================\n",
	  channel);
  
  fprintf(out, " %5s %15s %15s %15s %15s\n",
	  "gate", "i(volts)", "q(volts)", "power(dBm)", "phase(deg)");
  
  const fl32 *iq = _chanIq[channel];
  for (int igate = startGate, ii = startGate * 2; igate <= endGate; igate++, ii += 2) {
    
    double ival = iq[ii];
    double qval = iq[ii + 1];

    double power = ival * ival + qval * qval;
    double powerDbm = 10.0 * log10(power);
    double phaseRad = 0.0;
    if (ival != 0 || qval != 0) {
      phaseRad = atan2(qval, ival);
    }
    double phaseDeg = phaseRad * RAD_TO_DEG;

    if (igate < 0) {
      // burst gate
      fprintf(out, " %4dB %15.8e %15.8e %15.3f %15.3f\n",
              igate * -1, ival, qval, powerDbm, phaseDeg);
    } else {
      fprintf(out, " %5d %15.8e %15.8e %15.3f %15.3f\n",
              igate, ival, qval, powerDbm, phaseDeg);
    }

  } // igate

  fprintf(out, "=================================================================\n");

}

