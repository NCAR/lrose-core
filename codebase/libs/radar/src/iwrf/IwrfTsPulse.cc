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
// IwrfTsPulse.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <cmath>
#include <toolsa/mem.h>
#include <toolsa/sincos.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/IwrfTsPulse.hh>
using namespace std;

pthread_mutex_t IwrfTsPulse::_sigmetLutMutex = PTHREAD_MUTEX_INITIALIZER;
bool IwrfTsPulse::_sigmetFloatLutReady = false;
bool IwrfTsPulse::_sigmetLegacyUnpacking = false;
bool IwrfTsPulse::_sigmetLegacyUnpackingActive = false;
fl32 IwrfTsPulse::_sigmetFloatLut[65536] = { 0 };
const double IwrfTsPulse::PHASE_MULT = 180.0 / 32767.0;

// Constructor

IwrfTsPulse::IwrfTsPulse(IwrfTsInfo &info,
			 IwrfDebug_t debug) :
        _info(info),
        _debug(debug)
  
{

  iwrf_pulse_header_init(_hdr);
  iwrf_rvp8_pulse_header_init(_rvp8_hdr);
  _rvp8_header_active = false;

  _georefActive = false;
  _georefCorrActive = false;
  _invertHvFlag = false;

  _ftime = 0.0;
  _prf = 0.0;
  _phaseDiff[0] = 0.0;
  _phaseDiff[1] = 0.0;

  _iqData = NULL;

  for (int ii = 0; ii < IWRF_MAX_CHAN; ii++) {
    _chanIq[ii]= NULL;
    _burstIq[ii] = NULL;
  }

  _packedEncoding = IWRF_IQ_ENCODING_LAST;
  _packedScale = 1.0;
  _packedOffset = 0.0;
  _packed = NULL;

  // initialize mutexes

  _nClients = 0;
  pthread_mutex_init(&_nClientsMutex, NULL);

}

/////////////////////////////
// Copy constructor

IwrfTsPulse::IwrfTsPulse(const IwrfTsPulse &rhs) :
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

IwrfTsPulse::~IwrfTsPulse()

{
  _clearIq();
  _clearPacked();
  pthread_mutex_destroy(&_nClientsMutex);
}

/////////////////////////////
// Assignment
//

IwrfTsPulse &IwrfTsPulse::operator=(const IwrfTsPulse &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////////////////////
// clear all

void IwrfTsPulse::clear()

{
  
  _clearIq();
  _clearPacked();

  iwrf_pulse_header_init(_hdr);
  iwrf_rvp8_pulse_header_init(_rvp8_hdr);
  _rvp8_header_active = false;

  _georefActive = false;
  _georefCorrActive = false;

}

////////////////////////////////////////////////////////////
// set time

void IwrfTsPulse::setTime(time_t secs, int nano_secs) {
  iwrf_set_packet_time(_hdr.packet, secs, nano_secs);
  iwrf_set_packet_time(_rvp8_hdr.packet, secs, nano_secs);
}

void IwrfTsPulse::setTimeToNow() {
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set headers directly

void IwrfTsPulse::setHeader(const iwrf_pulse_header_t &hdr) {
  _hdr = hdr;
  _hdr.packet.id = IWRF_PULSE_HEADER_ID;
  _hdr.packet.len_bytes = sizeof(iwrf_pulse_header_t);
  _hdr.packet.version_num = 1;
}

void IwrfTsPulse::setRvp8Header(const iwrf_rvp8_pulse_header_t &hdr) {
  _rvp8_hdr = hdr;
  _rvp8_hdr.packet.id = IWRF_RVP8_PULSE_HEADER_ID;
  _rvp8_hdr.packet.len_bytes = sizeof(iwrf_rvp8_pulse_header_t);
  _rvp8_hdr.packet.version_num = 1;
}

////////////////////////////////////////////////////////////
// set radar ID

void IwrfTsPulse::setRadarId(int id) {

  _hdr.packet.radar_id = id;
  _rvp8_hdr.packet.radar_id = id;

}

////////////////////////////////////////////////////////////
// activate RVP8 header

void IwrfTsPulse::activateRvp8Header() {
  _rvp8_header_active = false;
}

////////////////////////////////////////////////////////////
// set sequence number for each header

void IwrfTsPulse::setPktSeqNum(si64 pkt_seq_num) {
  _hdr.packet.seq_num = pkt_seq_num;
}

void IwrfTsPulse::setRvp8PktSeqNum(si64 pkt_seq_num) {
  _rvp8_hdr.packet.seq_num = pkt_seq_num;
}

///////////////////////////////////////////////////////////
// set from pulse buffer, swapping as required
// optionally convert iq data to floats
// Returns 0 on success, -1 on failure

int IwrfTsPulse::setFromBuffer(const void *buf, int len,
			       bool convertToFloat)
  
{

  // check validity of packet
  
  int packet_id;
  if (iwrf_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - IwrfTsPulse::setFromBuffer" << endl;
    fprintf(stderr, "  Bad packet, id: 0x%x\n", packet_id);
    cerr << "             len: " << len << endl;
    cerr << "            type: " << iwrf_packet_id_to_str(packet_id) << endl;
   return -1;
  }

  // swap packet as required, using a copy to preserve const

  char *copy = new char[len];
  memcpy(copy, buf, len);
  iwrf_packet_swap(copy, len);

  if (_debug >= IWRF_DEBUG_EXTRA) {
    iwrf_packet_print(stderr, copy, len);
  }

  if (packet_id != IWRF_PULSE_HEADER_ID &&
      packet_id != IWRF_RVP8_PULSE_HEADER_ID) {
    cerr << "ERROR - IwrfTsPulse::setFromBuffer" << endl;
    fprintf(stderr, "  Incorrect packet id: 0x%x\n", packet_id);
    cerr << "                  len: " << len << endl;
    cerr << "                 type: " << iwrf_packet_id_to_str(packet_id) << endl;
    delete[] copy;
    return -1;
  }

  if (packet_id == IWRF_RVP8_PULSE_HEADER_ID) {
    memcpy(&_rvp8_hdr, copy, sizeof(iwrf_rvp8_pulse_header_t));
    delete[] copy;
    return 0;
  }

  memcpy(&_hdr, copy, sizeof(iwrf_pulse_header_t));

  // derive

  if (_hdr.elevation > 180) {
    _hdr.elevation -= 360.0;
  }
  _checkScanModeForVert();

  int nGatesTotal = _hdr.n_gates + _hdr.n_gates_burst;
  int nDataMin = _hdr.n_channels * (nGatesTotal) * 2;
  if ((int) _hdr.n_data < nDataMin) {
    if (_debug >= IWRF_DEBUG_VERBOSE) {
      cerr << "WARNING: IwrfTsPulse::setFromTsBuffer: ndata set to low" << endl;
      cerr << "  nData in pulse header: " << _hdr.n_data << endl;
      cerr << "  Setting to min required: " << nDataMin << endl;
    }
    _hdr.n_data = nDataMin;
  }
  
  _ftime = (double) _hdr.packet.time_secs_utc +
    _hdr.packet.time_nano_secs / 1.0e9;

  _prf = 1.0 / _hdr.prt;
  
  _phaseDiff[0] = (_rvp8_hdr.i_burst_arg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (_rvp8_hdr.i_burst_arg[1] / 65536.0) * 360.0;
  
  // special 9999 code is stored in uiqOnce[0] to indicate
  // nGatesBurst is stored in uiqOnce[1]
  
  _rvp8_hdr.uiq_once[0] = 9999;
  _rvp8_hdr.uiq_once[1] = _hdr.n_gates_burst;
  
  // load up IQ data
  
  int requiredLen = 
    (int) sizeof(iwrf_pulse_header_t) + _hdr.n_data * sizeof(si16);
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32 ||
      _hdr.iq_encoding == IWRF_IQ_ENCODING_SCALED_SI32) {
    requiredLen =
      (int) sizeof(iwrf_pulse_header_t) + _hdr.n_data * sizeof(fl32);
  }
  if (len < requiredLen) {
    cerr << "ERROR - IwrfTsPulse::setFromTsPulse" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << requiredLen << endl;
    cerr << "sizeof(iwrf_pulse_header_t): "
         << sizeof(iwrf_pulse_header_t) << endl; 
    iwrf_pulse_header_print(stderr, _hdr);
    delete[] copy;
    return -1;
  }
  
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    
    _clearPacked();
    fl32 *iq = (fl32 *) (copy + sizeof(iwrf_pulse_header_t));
    _iqData = (fl32 *) _iqBuf.load(iq, _hdr.n_data * sizeof(fl32));

  } else if (_hdr.iq_encoding == IWRF_IQ_ENCODING_SCALED_SI32) {
    
    _clearPacked();
    si32 *siq = (si32 *) (copy + sizeof(iwrf_pulse_header_t));
    _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
    fl32 *iq = _iqData;
    double scale = _hdr.scale;
    double offset = _hdr.offset;
    for (int ii = 0; ii < _hdr.n_data; ii++, siq++, iq++) {
      *iq = *siq * scale + offset;
    }

  } else {

    _clearIq();
    si16 *packed = (si16 *) (copy + sizeof(iwrf_pulse_header_t));
    _packed = (si16 *) _packedBuf.load(packed, _hdr.n_data * sizeof(si16));

  }

  _packedEncoding = (iwrf_iq_encoding_t) _hdr.iq_encoding;
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

void IwrfTsPulse::setIqFloats(int nGates,
			      int nChannels,
			      const fl32 *iq)
  
{

  _hdr.n_gates = nGates;
  _hdr.n_gates_burst = 0;
  MEM_zero(_hdr.iq_offset);
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
  
  _iqData = (fl32 *) _iqBuf.load(iq, _hdr.n_data * sizeof(fl32));

  _clearPacked();
  _packedEncoding = IWRF_IQ_ENCODING_FL32;
  _packedScale = _hdr.scale;
  _packedOffset = _hdr.offset;

  _setDataPointers();
  
}

///////////////////////////////////////////////////////////
// set IQ data as floats, converting from ScaledSi32

void IwrfTsPulse::setIqFromScaledSi32(int nGates,
                                      int nChannels,
                                      const si32 *siq)
  
{

  _hdr.n_gates = nGates;
  _hdr.n_gates_burst = 0;
  MEM_zero(_hdr.iq_offset);
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
  
  _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
  fl32 *iq = _iqData;
  double scale = _hdr.scale;
  double offset = _hdr.offset;
  for (int ii = 0; ii < _hdr.n_data; ii++, siq++, iq++) {
    *iq = *siq * scale + offset;
  }
  
  _clearPacked();
  _packedEncoding = IWRF_IQ_ENCODING_FL32;
  _packedScale = 1.0;
  _packedOffset = 0.0;

  _setDataPointers();
  
}

///////////////////////////////////////////////////////////
// set IQ data as packed si16
  
void IwrfTsPulse::setIqPacked(int nGates, int nChannels, 
			      iwrf_iq_encoding_t encoding,
			      const si16 *packed,
			      double scale = 1.0,
			      double offset = 0.0)
  
{
  
  _hdr.n_gates = nGates;
  _hdr.n_gates_burst = 0;
  MEM_zero(_hdr.iq_offset);
  _hdr.n_channels = nChannels;
  _hdr.n_data = nChannels * nGates * 2;
  _hdr.iq_encoding = encoding;
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
  
void IwrfTsPulse::swapPrtValues()

{
  double prt = _hdr.prt_next;
  double prt_next = _hdr.prt;
  _hdr.prt_next = prt_next;
  _hdr.prt = prt;
}

///////////////////////////////////////////////////////////
// convert packed data to float32

void IwrfTsPulse::convertToFL32()
  
{

  if (_packedEncoding == IWRF_IQ_ENCODING_FL32) {
    _setDataPointers();
    _fixZeroPower();
    return;
  }

  _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
  
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    
    // compute from signed scaled values
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0; ii < _hdr.n_data; ii++) {
      _iqData[ii] = packed[ii] * _packedScale + _packedOffset;
      if (fabs(_iqData[ii]) == 0.0) {
        _iqData[ii] = 1.0e-20;
      }
    }
    
  } else if (_hdr.iq_encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {

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

#ifdef DEBUG_PRINT	
      if (ii == 0) {
	cerr << "========================================" << endl;
	cerr << "IWRF_IQ_ENCODING_DBM_PHASE_SI16 to float" << endl;
	cerr << "packedPwr: " << packed[ii] << endl;
	cerr << "packedPhase: " << packed[ii+1] << endl;
	cerr << "powerDbm: " << powerDbm << endl;
	cerr << "power: " << power << endl;
	cerr << "phaseDeg: " << phaseDeg << endl;
	cerr << "II: " << _iqData[ii] << endl;
	cerr << "QQ: " << _iqData[ii+1] << endl;
	cerr << "========================================" << endl;
      }
#endif

    }
    
  } else if (_hdr.iq_encoding == IWRF_IQ_ENCODING_SIGMET_FL16) {
    
    _loadIqFromSigmetFL16();

  }
  
  _hdr.scale = 1.0;
  _hdr.offset = 0.0;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
  
  _setDataPointers();

}

///////////////////////////////////////////////////////////
// get IQ at a gate

void IwrfTsPulse::getIq0(int gateNum,
                         fl32 &ival, fl32 &qval) const
{
  return getIq(0, gateNum, ival, qval);
}

void IwrfTsPulse::getIq1(int gateNum,
                         fl32 &ival, fl32 &qval) const
{
  return getIq(1, gateNum, ival, qval);
}

void IwrfTsPulse::getIq2(int gateNum,
                         fl32 &ival, fl32 &qval) const
{
  return getIq(2, gateNum, ival, qval);
}

void IwrfTsPulse::getIq3(int gateNum,
                         fl32 &ival, fl32 &qval) const
{
  return getIq(3, gateNum, ival, qval);
}

///////////////////////////////////////////////////////////
// get IQ at a gate and channel


void IwrfTsPulse::getIq(int chanNum,
                        int gateNum,
                        fl32 &ival, fl32 &qval) const
  
{

  // initialize to missing

  ival = IWRF_MISSING_FLOAT;
  qval = IWRF_MISSING_FLOAT;

  // check for valid geometry

  if (chanNum >= getNChannels()) {
    return;
  }

  if (gateNum < 0 || gateNum >= getNGates()) {
    return;
  }

  // compute data offset into iq array

  int offset = (chanNum * (_hdr.n_gates + _hdr.n_gates_burst) +
                _hdr.n_gates_burst + gateNum) * 2;

  // floats? return as is

  if (_packedEncoding == IWRF_IQ_ENCODING_FL32) {

    ival = _iqData[offset];
    qval = _iqData[offset + 1];

  } else if (_packedEncoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    
    // unscale
    
    si16 ipacked = _packed[offset];
    si16 qpacked = _packed[offset + 1];
    ival = ipacked * _packedScale + _packedOffset;
    qval = qpacked * _packedScale + _packedOffset;
    
  } else if (_packedEncoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {

    // compute from power and phase
    
    double powerDbm = _packed[offset] * _packedScale + _packedOffset;
    double power = pow(10.0, powerDbm / 10.0);
    double phaseDeg = _packed[offset + 1] * PHASE_MULT;
    double sinPhase, cosPhase;
    ta_sincos(phaseDeg * DEG_TO_RAD, &sinPhase, &cosPhase);
    double mag = sqrt(power);
    ival = mag * cosPhase;
    qval = mag * sinPhase;

  } else if (_packedEncoding == IWRF_IQ_ENCODING_SIGMET_FL16) {

    // compute lookup table - first time only

    _computeSigmetFloatLut();
    
    // unpack the shorts into floats
    
    ival = _sigmetFloatLut[(ui16)_packed[offset]] *
      _info.getRvp8SaturationMult();
    qval = _sigmetFloatLut[(ui16)_packed[offset+1]] *
      _info.getRvp8SaturationMult();
    
  }

}

///////////////////////////////////////////////////////////
// convert to specified packing

void IwrfTsPulse::convertToPacked(iwrf_iq_encoding_t encoding)
  
{
  
  if (encoding == _packedEncoding) {
    // nothing to do
    _hdr.scale = _packedScale;
    _hdr.offset = _packedOffset;
    _hdr.iq_encoding = _packedEncoding;
    return;
  }
  
  if (encoding == IWRF_IQ_ENCODING_FL32 && _iqData != NULL) {
    // use float data as packed data
    _clearPacked();
    _hdr.scale = 1.0;
    _hdr.offset = 0.0;
    _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
    _packedScale = 1.0;
    _packedOffset = 0.0;
    _packedEncoding = IWRF_IQ_ENCODING_FL32;
    return;
  }

  if (_iqData == NULL) {
    // make sure we have float 32 data available
    convertToFL32();
  }

  // prepare packed buffer
  
  _packed = (si16 *) _packedBuf.prepare(_hdr.n_data * sizeof(si16));
  
  // fill packed array
  
  if (encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    
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
    
  } else if (encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {

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

  } else if (encoding == IWRF_IQ_ENCODING_SIGMET_FL16) {

    // adjust for saturation value
    // sigmet stores their data to give a power 6 dB lower than measured

    fl32 *siqArray = new fl32[_hdr.n_data];
    fl32 *siq = siqArray;
    fl32 *iq = _iqData;
    for (int ii = 0; ii < _hdr.n_data; ii++, siq++, iq++) {
      *siq = *iq / _info.getRvp8SaturationMult();
    }
    vecPackIQFromFloatIQ((ui16 *) _packed, siqArray, _hdr.n_data);
    delete[] siqArray;
    _packedScale = 1.0;
    _packedOffset = 0.0;

  }

  // save values

  _packedEncoding = encoding;

  _hdr.scale = _packedScale;
  _hdr.offset = _packedOffset;
  _hdr.iq_encoding = _packedEncoding;

}

///////////////////////////////////////////////////////////////////
// swaps I and Q, because they are stored in the incorrect order
// in the data arrays

void IwrfTsPulse::swapIQ()
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

void IwrfTsPulse::invertQ()
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
 
///////////////////////////////////////////////////////////
// cohere to the burst phase
//
// subtract the burst phase from the phase of each IQ pair

void IwrfTsPulse::cohereIqToBurstPhase()
  
{

  // convert to floats

  convertToFL32();

  // loop through the channels

  for (int ichan = 0; ichan < _hdr.n_channels; ichan++) {

    // get pointers to IQ and burst

    fl32 *iq = _chanIq[ichan];
    fl32 burstArgRad = _hdr.burst_arg[ichan] * DEG_TO_RAD;

    if (_hdr.n_gates_burst > 0) {
      // use burst in IQ data
      double iburst = iq[-2];
      double qburst = iq[-1];
      burstArgRad = atan2(qburst, iburst);
    }

    // create a normalized complex number with same phase as burst
    // and a magnitude of 1.0
    
    double qBurstNorm, iBurstNorm;
    ta_sincos(burstArgRad, &qBurstNorm, &iBurstNorm);
    
    fl32 *II = _iqData;
    fl32 *QQ = _iqData + 1;
    for (int igate = -_hdr.n_gates_burst;
         igate < _hdr.n_gates; igate++, II += 2, QQ += 2) {
      
      double ival = *II;
      double qval = *QQ;
      
      double ivalCohered = ival * iBurstNorm + qval * qBurstNorm;
      double qvalCohered = qval * iBurstNorm - ival * qBurstNorm;
      
      *II = ivalCohered * 1.0;
      *QQ = qvalCohered * 1.0;
      
    } // igate
    
  } // ichan

  _hdr.phase_cohered = 1;

}

////////////////////////////////////////////////////////////
// copy the pulse width from the ts_proc in the info

void IwrfTsPulse::copyPulseWidthFromTsProc() {
  
  if (_info.isTsProcessingActive()) {
    _hdr.pulse_width_us = _info.get_proc_pulse_width_us();
  }

}

///////////////////////////////////////////////////////////
// assemble into a single buffer

void IwrfTsPulse::assemble(MemBuf &buf) const
{
  iwrf_pulse_header_t hdr = _hdr;
  int nbytesIq = 0;
  if (hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    nbytesIq = hdr.n_data * sizeof(fl32);
  } else {
    nbytesIq = hdr.n_data * sizeof(si16);
  }
  hdr.packet.len_bytes = sizeof(iwrf_pulse_header_t) + nbytesIq;
  buf.add(&hdr, sizeof(hdr));
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    buf.add(_iqData, nbytesIq);
  } else {
    buf.add(_packed, nbytesIq);
  }
}

///////////////////////////////////////////////////////////
// set RVP8-specific fields

void IwrfTsPulse::setRvp8Hdr(const iwrf_pulse_header_t &hdr)
  
{
  
  if (_rvp8_hdr.i_version != 0) {
    return;
  }
  
  _rvp8_hdr.i_polar_bits = hdr.hv_flag;
  _rvp8_hdr.i_viq_per_bin = hdr.n_channels;
  
  _rvp8_hdr.i_az = (ui16) (hdr.azimuth * 65535.0 / 360.0 + 0.5);
  _rvp8_hdr.i_el = (ui16) (hdr.elevation * 65535.0 / 360.0 + 0.5);
  _rvp8_hdr.i_num_vecs = hdr.n_gates;
  _rvp8_hdr.i_max_vecs = hdr.n_gates;

}
  
///////////////////////////////////////////////////////////
// read pulse from rvp8 time series file
//
// Returns 0 on success, -1 on failure

int IwrfTsPulse::readFromRvp8File(FILE *in)

{

  // read in header

  if (_readRvp8Header(in)) {
    return -1;
  }

  _deriveFromRvp8Header();

  if (_readRvp8Data(in)) {
    return -1;
  }

  return 0;

}
  
/////////////////////////////////////////////////////////////////
// Check to see if horizontally polarized

bool IwrfTsPulse::isHoriz() const

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

const void *IwrfTsPulse::getPackedData() const

{

  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    return _iqData;
  } else {
    return _packed;
  }

}

/////////////////////////////
// get elevation and azimuth

double IwrfTsPulse::getEl() const {
  if (isnan(_hdr.elevation)) {
    return _hdr.elevation;
  }
  if (_hdr.elevation <= 180) {
    return _hdr.elevation;
  } else {
    return (_hdr.elevation - 360.0);
  }
}

double IwrfTsPulse::getAz() const {
  if (isnan(_hdr.azimuth)) {
    return _hdr.azimuth;
  }
  if (_hdr.azimuth >= 0) {
    return _hdr.azimuth;
  } else {
    return _hdr.azimuth + 360.0;
  }
}

double IwrfTsPulse::getFixedAngle() const {
  if (_hdr.scan_mode == IWRF_SCAN_MODE_RHI ||
      _hdr.scan_mode == IWRF_SCAN_MODE_EL_SUR_360) {
    return getFixedAz();
  } else {
    return getFixedEl();
  }
}

double IwrfTsPulse::getFixedEl() const {
  if (isnan(_hdr.fixed_el)) {
    return _hdr.fixed_el;
  }
  if (_hdr.fixed_el <= 180) {
    return _hdr.fixed_el;
  } else {
    return (_hdr.fixed_el - 360.0);
  }
}

double IwrfTsPulse::getFixedAz() const {
  if (isnan(_hdr.fixed_az)) {
    return _hdr.fixed_az;
  }
  if (_hdr.fixed_az >= 0) {
    return _hdr.fixed_az;
  } else {
    return _hdr.fixed_az + 360.0;
  }
}

/////////////////////////////////////////////////////////////////
// Compute phase differences between this pulse and previous ones
// to be able to cohere to multiple trips
//
// Before this method is called, this pulse should be added to
// the queue.

int IwrfTsPulse::computePhaseDiffs
  (const deque<IwrfTsPulse *> &pulseQueue, int maxTrips) const
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - IwrfTsPulse::computePhaseDiffs()" << endl;
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

int IwrfTsPulse::computePhaseDiffs
  (int qSize, const IwrfTsPulse **pulseQueue, int maxTrips) const
  
{
  
  if (pulseQueue[0] != this) {
    cerr << "ERROR - IwrfTsPulse::computePhaseDiffs()" << endl;
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

void IwrfTsPulse::printHeader(FILE *out) const
{
  if (_georefActive) {
    // fprintf(out, "====>>> this georef applies to following pulse <<<====\n");
    // iwrf_platform_georef_print(out, _georef);
    iwrf_pulse_header_print(out, _hdr, &_georef);
  } else {
    iwrf_pulse_header_print(out, _hdr);
  }
  if (_rvp8_hdr.i_version != 0) {
    iwrf_rvp8_pulse_header_print(out, _rvp8_hdr);
  }
}

////////////////////////////////////////////////////////////
// print data

void IwrfTsPulse::printData(FILE *out) const
{
  printData(out, 0, _hdr.n_gates);
}

void IwrfTsPulse::printData(FILE *out, int startGate, int endGate) const
{
  
  // copy this object for printing
  
  IwrfTsPulse pulse(*this);
  pulse.convertToFL32();
  pulse.printHeader(out);
  for (int ichan = 0; ichan < _hdr.n_channels; ichan++) {
    pulse._doPrintData(out, ichan, startGate, endGate);
  }
  
}

///////////////////////////////////////////////////
// write to file in IWRF format
//
// returns 0 on success, -1 on failure

int IwrfTsPulse::writeToFile(FILE *out) const

{

  if (out == NULL) {
    return -1;
  }
  
  if (_rvp8_header_active) {
    if (fwrite(&_rvp8_hdr, sizeof(_rvp8_hdr), 1, out) != 1) {
      int errNum = errno;
      cerr << "ERROR - IwrfTsPulse::write2File" << endl;
      cerr << "  Cannot write rvp8_pulse_header packet" << endl;
      cerr << "  " << strerror(errNum) << endl;
      return -1;
    }
  }

  MemBuf buf;
  assemble(buf);
  if (fwrite(buf.getPtr(), buf.getLen(), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - IwrfTsPulse::write2File" << endl;
    cerr << "  Cannot write pulse" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}
  
///////////////////////////////////////////////
// write to tsarchive file

int IwrfTsPulse::writeToTsarchiveFile(FILE *out) const

{
  
  if (out == NULL) {
    return -1;
  }

  fprintf(out, "rvptsPulseHdr start\n");

  fprintf(out, "iVersion=%d\n", _rvp8_hdr.i_version);
  fprintf(out, "iFlags=%d\n", _rvp8_hdr.i_flags);
  fprintf(out, "iMSecUTC=%d\n", _hdr.packet.time_nano_secs);
  fprintf(out, "iTimeUTC=%ld\n", (long) _hdr.packet.time_secs_utc);
  fprintf(out, "iBtimeAPI=%d\n", _rvp8_hdr.i_btime_api);
  fprintf(out, "iSysTime=%d\n", _rvp8_hdr.i_sys_time);
  fprintf(out, "iPrevPRT=%d\n", _rvp8_hdr.i_prev_prt);
  fprintf(out, "iNextPRT=%d\n", _rvp8_hdr.i_next_prt);
  fprintf(out, "iSeqNum=%ld\n", (long) _hdr.pulse_seq_num);
  fprintf(out, "iAqMode=%d\n", _rvp8_hdr.i_aq_mode);
  fprintf(out, "iPolarBits=%d\n", _rvp8_hdr.i_polar_bits);
  fprintf(out, "iTxPhase=%d\n", _rvp8_hdr.i_tx_phase);
  fprintf(out, "iAz=%d\n", _rvp8_hdr.i_az);
  fprintf(out, "iEl=%d\n", _rvp8_hdr.i_el);
  fprintf(out, "iNumVecs=%d\n", _rvp8_hdr.i_num_vecs);
  fprintf(out, "iMaxVecs=%d\n", _rvp8_hdr.i_max_vecs);
  fprintf(out, "iVIQPerBin=%d\n", _rvp8_hdr.i_viq_per_bin);
  fprintf(out, "iTgBank=%d\n", _rvp8_hdr.i_tg_bank);
  fprintf(out, "iTgWave=%d\n", _rvp8_hdr.i_tg_wave);

  fprintf(out, "uiqPerm.iLong=%d %d\n",
          (unsigned) _rvp8_hdr.uiq_perm[0], (unsigned) _rvp8_hdr.uiq_perm[1]);

  fprintf(out, "uiqOnce.iLong=%d %d\n",
          (unsigned) _rvp8_hdr.uiq_once[0], (unsigned) _rvp8_hdr.uiq_once[1]);

  fprintf(out, "RX[0].fBurstMag=%g\n", _rvp8_hdr.f_burst_mag[0]);
  fprintf(out, "RX[0].iBurstArg=%d\n", _rvp8_hdr.i_burst_arg[0]);
  fprintf(out, "RX[1].fBurstMag=%g\n", _rvp8_hdr.f_burst_mag[1]);
  fprintf(out, "RX[1].iBurstArg=%d\n", _rvp8_hdr.i_burst_arg[1]);

  fprintf(out, "rvptsPulseHdr end\n");

  return 0;

}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class uses the notion of clients to decide when it should be deleted.
// If removeClient() returns 0, the object should be deleted.
// These functions are protected by a mutex for multi-threaded ops

int IwrfTsPulse::addClient() const
  
{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  _nClients++;
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

int IwrfTsPulse::removeClient() const

{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  _nClients--;
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

void IwrfTsPulse::deleteIfUnused(IwrfTsPulse *pulse)
  
{
  if (pulse->removeClient() == 0) {
    delete pulse;
  }
}

int IwrfTsPulse::getNClients() const

{
  int nClients;
  pthread_mutex_lock(&_nClientsMutex);
  nClients = _nClients;
  pthread_mutex_unlock(&_nClientsMutex);
  return nClients;
}

/////////////////////////////
// copy

IwrfTsPulse &IwrfTsPulse::_copy(const IwrfTsPulse &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _hdr = rhs._hdr;
  _rvp8_hdr = rhs._rvp8_hdr;
  _rvp8_header_active = rhs._rvp8_header_active;

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

void IwrfTsPulse::computeElAzFromGeoref()

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
  
  set_elevation(elevationRad * RAD_TO_DEG);
  set_azimuth(azimuthRad * RAD_TO_DEG);

  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "========== IwrfTsPulse::computeElAzFromGeoref() ========" << endl;
    cerr << "  roll: " << _georef.roll_deg << endl;
    cerr << "  pitch: " << _georef.pitch_deg << endl;
    cerr << "  heading: " << _georef.heading_deg << endl;
    cerr << "  drift_angle: " << _georef.drift_angle_deg << endl;
    cerr << "  rotation_angle: " << _georef.rotation_angle_deg << endl;
    cerr << "  tilt: " << _georef.tilt_deg << endl;
    cerr << "  el: " << get_elevation() << endl;
    cerr << "  az: " << get_azimuth() << endl;
    cerr << "========================================================" << endl;
  }
  

}

////////////////////////////////////////////////////////////
// set start range and gate spacing in header, if needed

void IwrfTsPulse::_checkRangeMembers() {

  if (_info.isTsProcessingActive()) {

    if (_hdr.start_range_m == 0) {
      _hdr.start_range_m = _info.get_proc_start_range_m();
    }
    
    if (_hdr.gate_spacing_m == 0) {
      _hdr.gate_spacing_m = _info.get_proc_gate_spacing_m();
    }

  }

}

///////////////////////////////////////////////////////////////
// check the scan mode for vert pointing
//
// If surveillance and angle is high, set to vert point

void IwrfTsPulse::_checkScanModeForVert()
  
{

  if (_hdr.scan_mode == IWRF_SCAN_MODE_AZ_SUR_360 &&
      _hdr.elevation > 85 && _hdr.elevation < 95) {
    _hdr.scan_mode = IWRF_SCAN_MODE_VERTICAL_POINTING;
  }
  
}
  
///////////////////////////////////////////////////////////////
// read in pulse header
//
// Returns 0 on success, -1 on failure

int IwrfTsPulse::_readRvp8Header(FILE *in)

{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "--->> Reading RVP8 Pulse Header <<-----" << endl;
  }

  char line[BUFSIZ];

  // initialize to missing values

  iwrf_pulse_header_init(_hdr);

  bool versionFound = false;
  bool iFlagsFound = false;
  bool timeNanoSecsFound = false;
  bool timeSecsUTCFound = false;
  bool iBtimeAPIFound = false;
  bool iSysTimeFound = false;
  bool iPrevPRTFound = false;
  bool iNextPRTFound = false;
  bool pulseSeqNumFound = false;
  bool iAqModeFound = false;
  bool iPolarBitsFound = false;
  bool iTxPhaseFound = false;
  bool iAzFound = false;
  bool iElFound = false;
  bool iNumVecsFound = false;
  bool iMaxVecsFound = false;
  bool iVIQPerBinFound = false;
  bool iTgBankFound = false;
  bool iTgWaveFound = false;
  bool uiqPermFound = false;
  bool uiqOnceFound = false;
  bool fBurstMag0Found = false;
  bool iBurstArg0Found = false;
  bool fBurstMag1Found = false;
  bool iBurstArg1Found = false;

  // read to the pulse header
  
  if (IwrfTsInfo::findNextRvp8Str(in, "PulseHdr start")) {
    return -1;
  }
    
  while (!feof(in)) {

    if (fgets(line, BUFSIZ, in) == NULL) {
      return -1;
    }
    
    if (_debug >= IWRF_DEBUG_EXTRA) {
      char shortLine[80];
      memcpy(shortLine, line, 79);
      shortLine[79] = '\0';
      cerr << shortLine;
    }
    
    if (strstr(line, "PulseHdr end")) {
      return 0;
    }

    // look for integers

    int ival, jval;

    if (!versionFound && sscanf(line, "iVersion=%d", &ival) == 1) {
      _rvp8_hdr.i_version = ival;
      versionFound = true;
      continue;
    }

    if (!iFlagsFound && sscanf(line, "iFlags=%d", &ival) == 1) {
      _rvp8_hdr.i_flags = ival;
      iFlagsFound = true;
      continue;
    }

    if (!timeNanoSecsFound && sscanf(line, "iMSecUTC=%d", &ival) == 1) {
      _rvp8_hdr.packet.time_nano_secs = ival * 1000000;
      timeNanoSecsFound = true;
      continue;
    }

    if (!timeSecsUTCFound && sscanf(line, "iTimeUTC=%d", &ival) == 1) {
      _rvp8_hdr.packet.time_secs_utc = ival;
      timeSecsUTCFound = true;
      continue;
    }

    if (!iBtimeAPIFound && sscanf(line, "iBtimeAPI=%d", &ival) == 1) {
      _rvp8_hdr.i_btime_api = ival;
      iBtimeAPIFound = true;
      continue;
    }

    if (!iSysTimeFound && sscanf(line, "iSysTime=%d", &ival) == 1) {
      _rvp8_hdr.i_sys_time = ival;
      iSysTimeFound = true;
      continue;
    }

    if (!iPrevPRTFound && sscanf(line, "iPrevPRT=%d", &ival) == 1) {
      _rvp8_hdr.i_prev_prt = ival;
      iPrevPRTFound = true;
      continue;
    }

    if (!iNextPRTFound && sscanf(line, "iNextPRT=%d", &ival) == 1) {
      _rvp8_hdr.i_next_prt = ival;
      iNextPRTFound = true;
      continue;
    }

    if (!pulseSeqNumFound && sscanf(line, "iSeqNum=%d", &ival) == 1) {
      _rvp8_hdr.i_seq_num = ival;
      pulseSeqNumFound = true;
      continue;
    }

    if (!iAqModeFound && sscanf(line, "iAqMode=%d", &ival) == 1) {
      _rvp8_hdr.i_aq_mode = ival;
      iAqModeFound = true;
      continue;
    }

    if (!iPolarBitsFound && sscanf(line, "iPolarBits=%d", &ival) == 1) {
      _rvp8_hdr.i_polar_bits = ival;
      iPolarBitsFound = true;
      continue;
    }

    if (!iTxPhaseFound && sscanf(line, "iTxPhase=%d", &ival) == 1) {
      _rvp8_hdr.i_tx_phase = ival;
      iTxPhaseFound = true;
      continue;
    }

    if (!iAzFound && sscanf(line, "iAz=%d", &ival) == 1) {
      _rvp8_hdr.i_az = ival;
      iAzFound = true;
      continue;
    }

    if (!iElFound && sscanf(line, "iEl=%d", &ival) == 1) {
      _rvp8_hdr.i_el = ival;
      iElFound = true;
      continue;
    }

    if (!iNumVecsFound && sscanf(line, "iNumVecs=%d", &ival) == 1) {
      _rvp8_hdr.i_num_vecs = ival;
      iNumVecsFound = true;
      continue;
    }

    if (!iMaxVecsFound && sscanf(line, "iMaxVecs=%d", &ival) == 1) {
      _rvp8_hdr.i_max_vecs = ival;
      iMaxVecsFound = true;
      continue;
    }

    if (!iVIQPerBinFound && sscanf(line, "iVIQPerBin=%d", &ival) == 1) {
      _rvp8_hdr.i_viq_per_bin = ival;
      iVIQPerBinFound = true;
      continue;
    }

    if (!iTgBankFound && sscanf(line, "iTgBank=%d", &ival) == 1) {
      _rvp8_hdr.i_tg_bank = ival;
      iTgBankFound = true;
      continue;
    }

    if (!iTgWaveFound && sscanf(line, "iTgWave=%d", &ival) == 1) {
      _rvp8_hdr.i_tg_wave = ival;
      iTgWaveFound = true;
      continue;
    }

    if (!uiqPermFound && sscanf(line, "uiqPerm.iLong=%d %d", &ival, &jval) == 2) {
      _rvp8_hdr.uiq_perm[0] = ival;
      _rvp8_hdr.uiq_perm[1] = jval;
      uiqPermFound = true;
      continue;
    }
    
    if (!uiqOnceFound && sscanf(line, "uiqOnce.iLong=%d %d", &ival, &jval) == 2) {
      _rvp8_hdr.uiq_once[0] = ival;
      _rvp8_hdr.uiq_once[1] = jval;
      uiqOnceFound = true;
      continue;
    }
    
    // look for floats
    
    double dval;

    if (!fBurstMag0Found && sscanf(line, "RX[0].fBurstMag=%lg", &dval) == 1) {
      _rvp8_hdr.f_burst_mag[0] = dval;
      fBurstMag0Found = true;
      continue;
    }

    if (!iBurstArg0Found && sscanf(line, "RX[0].iBurstArg=%d", &ival) == 1) {
      _rvp8_hdr.i_burst_arg[0] = ival;
      iBurstArg0Found = true;
      continue;
    }

    if (!fBurstMag1Found && sscanf(line, "RX[1].fBurstMag=%lg", &dval) == 1) {
      _rvp8_hdr.f_burst_mag[1] = dval;
      fBurstMag1Found = true;
      continue;
    }

    if (!iBurstArg1Found && sscanf(line, "RX[1].iBurstArg=%d", &ival) == 1) {
      _rvp8_hdr.i_burst_arg[1] = ival;
      iBurstArg1Found = true;
      continue;
    }

  } // while
  
  return 0;

}

///////////////////////////////////////////////////////////////
// derive quantities from rvp8 pulse header

void IwrfTsPulse::_deriveFromRvp8Header()

{

  // time

  _hdr.packet.time_nano_secs = _rvp8_hdr.packet.time_nano_secs;
  _hdr.packet.time_secs_utc = _rvp8_hdr.packet.time_secs_utc;
  _hdr.pulse_seq_num = _rvp8_hdr.i_seq_num;

  _ftime =
    (double) _hdr.packet.time_secs_utc + _hdr.packet.time_nano_secs / 1.0e9;

  // if special 9999 code is stored in uiqOnce[0], then the
  // nGatesBurst is stored in uiqOnce[1]
  // otherwise use the default
  
  if (_rvp8_hdr.uiq_once[0] == 9999) {
    _hdr.n_gates_burst = _rvp8_hdr.uiq_once[1];
  } else {
    _hdr.n_gates_burst = IWRF_RVP8_NGATES_BURST;
  }
  
  // first gates hold the burst
  _hdr.n_gates = _rvp8_hdr.i_num_vecs - _hdr.n_gates_burst;
  _hdr.n_channels = _rvp8_hdr.i_viq_per_bin;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_SIGMET_FL16;
  _hdr.n_data = _rvp8_hdr.i_num_vecs * _rvp8_hdr.i_viq_per_bin * 2;
  _hdr.azimuth = (_rvp8_hdr.i_az / 65535.0) * 360.0;
  _hdr.elevation = (_rvp8_hdr.i_el / 65535.0) * 360.0;
  if (_hdr.elevation > 180) {
    _hdr.elevation -= 360.0;
  }
  _checkScanModeForVert();

  double fSyClkMhz = _info.getRvp8Info().f_sy_clk_mhz;
  if (_rvp8_hdr.i_prev_prt == 0) {
    _hdr.prt = 0.001;
  } else {
    _hdr.prt = ((double) _rvp8_hdr.i_prev_prt / fSyClkMhz) / 1.0e6;
  }
  if (_rvp8_hdr.i_next_prt == 0) {
    _hdr.prt_next = 0.001;
  } else {
    _hdr.prt_next = ((double) _rvp8_hdr.i_next_prt / fSyClkMhz) / 1.0e6;
  }

  _prf = 1.0 / _hdr.prt;

  _hdr.pulse_width_us = _info.getRvp8Info().f_pwidth_usec;
  _hdr.sweep_num = -1;
  _hdr.volume_num = -1;
  _hdr.hv_flag = _rvp8_hdr.i_polar_bits;
  _hdr.phase_cohered = 1;

  _hdr.scale = _info.getRvp8SaturationMult();

  _phaseDiff[0] = (_rvp8_hdr.i_burst_arg[0] / 65536.0) * 360.0;
  _phaseDiff[1] = (_rvp8_hdr.i_burst_arg[1] / 65536.0) * 360.0;

  _hdr.burst_mag[0] = _rvp8_hdr.f_burst_mag[0];
  _hdr.burst_mag[1] = _rvp8_hdr.f_burst_mag[1];
  
  _hdr.burst_arg[0] = (_rvp8_hdr.i_burst_arg[0] / 65536.0) * 360.0;
  _hdr.burst_arg[1] = (_rvp8_hdr.i_burst_arg[1] / 65536.0) * 360.0;

}

///////////////////////////////////////////////////////////////
// read in pulse data
//
// Should follow _readRvp8Header and _derivedFromPulseHeader()
// immediately
//
// Returns 0 on success, -1 on failure

int IwrfTsPulse::_readRvp8Data(FILE *in)
  
{
  
  if (_debug >= IWRF_DEBUG_EXTRA) {
    cerr << "--->> Reading RVP8 Pulse Data <<-----" << endl;
  }

  // read in packed data

  _hdr.n_data = _rvp8_hdr.i_num_vecs * _rvp8_hdr.i_viq_per_bin * 2;
  _packed = (si16 *) _packedBuf.prepare(_hdr.n_data * sizeof(si16));
  int nRead = (int) fread(_packed, sizeof(si16), _hdr.n_data, in);
  if (nRead != _hdr.n_data) {
    cerr << "ERROR - Pulse::_readRvp8Data" << endl;
    cerr << "  Cannot fread on pulse data" << endl;
    cerr << "  Expecting nData: " << _hdr.n_data << endl;
    cerr << "  Got nRead: " << nRead << endl;
    if (feof(in)) {
      cerr << "  At end of file" << endl;
    }
    return -1;
  }
  
  // load the IQ float data

  _iqData = (fl32 *) _iqBuf.prepare(_hdr.n_data * sizeof(fl32));
  _loadIqFromSigmetFL16();

  // set pointers to the data

  _setDataPointers();
  
  return 0;

}

///////////////////////////////////////////////////////////
// load IQ data from 16-bit sigmet float array

void IwrfTsPulse::_loadIqFromSigmetFL16()
  
{

  // create the LUT if needed

  _computeSigmetFloatLut();

  // unpack the shorts into floats
  // adjust the IQ values for the saturation characteristics
  // apply the square root of the multiplier, since power is
  // I squared plus Q squared

  for (int ii = 0; ii < (int) _hdr.n_data; ii++) {
    _iqData[ii] =
      _sigmetFloatLut[(ui16)_packed[ii]] * _info.getRvp8SaturationMult();
  }

}

///////////////////////////////////////////////////////////
// set pointers into the data
// for some data sets (e.g. RVP8) the burst pulse is
// in the first 2 gates

void IwrfTsPulse::_setDataPointers()
  
{

  int nGatesPerChan = _hdr.n_gates + _hdr.n_gates_burst;
  int nIqPerChan = nGatesPerChan * 2;
  int burstIqOffset = _hdr.n_gates_burst * 2;

  _iqData = (fl32 *) _iqBuf.getPtr();
  _packed = (si16 *) _packedBuf.getPtr();

  _burstIq[0] = _iqData;
  _chanIq[0] = _burstIq[0] + burstIqOffset;
  _hdr.iq_offset[0] = burstIqOffset;
  
  for (int ichan = 1; ichan < _hdr.n_channels; ichan++) {
    int chanOffset = ichan * nIqPerChan;
    _burstIq[ichan] = _burstIq[0] + chanOffset;
    _chanIq[ichan] = _chanIq[0] + chanOffset;
    _hdr.iq_offset[ichan] = _hdr.iq_offset[0] + chanOffset;
  }

}

///////////////////////////////////////////////////////////
// Check for 0 power.
// If 0 found, set to small non-zero values.

void IwrfTsPulse::_fixZeroPower()
  
{

  if (_packedEncoding != IWRF_IQ_ENCODING_FL32) {
    convertToFL32();
  }
  
  for (int ii = 0; ii < _hdr.n_data; ii++) {
    if (fabs(_iqData[ii]) == 0.0) {
      _iqData[ii] = 1.0e-20;
    }
  }

}

///////////////////////////////////////////////////////////////
// compute lookup table for converting packed shorts to floats
// using the Sigmet 16-bit float definition

void IwrfTsPulse::_computeSigmetFloatLut() const

{

  pthread_mutex_lock(&_sigmetLutMutex);

  if (!_sigmetFloatLutReady ||
      _sigmetLegacyUnpacking != _sigmetLegacyUnpackingActive) {

    ui16 packed[65536];
    for (int ii = 0; ii < 65536; ii++) {
      packed[ii] = ii;
    }
    if (_sigmetLegacyUnpacking) {
      vecFloatIQFromPackIQLegacy(_sigmetFloatLut, packed, 65536);
    } else {
      vecFloatIQFromPackIQ(_sigmetFloatLut, packed, 65536);
    }

    _sigmetFloatLutReady = true;
    _sigmetLegacyUnpackingActive = _sigmetLegacyUnpacking;

  }

  pthread_mutex_unlock(&_sigmetLutMutex);

}

///////////////////////////////////////////////////////////////
// set RVP8 legacy unpacking
// uses 11-bit mantissa instead of the later 12-bit mantissa
  
void IwrfTsPulse::setSigmetLegacyUnpacking(bool state) {
  
  pthread_mutex_lock(&_sigmetLutMutex);
  _sigmetLegacyUnpacking = state;
  pthread_mutex_unlock(&_sigmetLutMutex);

}

////////////////////
// clean up memory

void IwrfTsPulse::_clearIq()
{
  _iqBuf.free();
  _iqData = NULL;
}

void IwrfTsPulse::_clearPacked()
{
  _packedBuf.free();
  _packed = NULL;
}

/* ======================================================================
 * Convert a normalized floating "I" or "Q" value from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 *
 * From Sigmet lib.
 */

void IwrfTsPulse::vecFloatIQFromPackIQ
  ( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
    si32 iCount_a)
{

  si32 iCount ; volatile const ui16 *iCodes = iCodes_a ;
  volatile fl32 *fIQVals = fIQVals_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode = *iCodes++ ; fl32 fVal = 0.0 ;
    
    if( iCode & 0xF000 ) {
      si32 iMan =  iCode        & 0x7FF ;
      si32 iExp = (iCode >> 12) & 0x00F ;
      
      if( iCode & 0x0800 ) iMan |= 0xFFFFF000 ;
      else                 iMan |= 0x00000800 ;
      
      fVal = ((fl32)iMan) * ((fl32)(ui32)(1 << iExp)) / 3.355443E7 ;
    }
    else {
      fVal = ( (fl32)(((si32)iCode) << 20) ) / 1.759218E13 ;
    }
    *fIQVals++ = fVal ;
  }
}

// legacy version - 11-bit mantissa

void IwrfTsPulse::vecFloatIQFromPackIQLegacy
  ( volatile fl32 fIQVals_a[], volatile const ui16 iCodes_a[],
    si32 iCount_a)
{

  si32 iCount ; volatile const ui16 *iCodes = iCodes_a ;
  volatile fl32 *fIQVals = fIQVals_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode = *iCodes++ ; fl32 fVal = 0.0 ;
    
    if( iCode ) {
      si32 iMan =  iCode        & 0x3FF ;
      si32 iExp = (iCode >> 11) & 0x01F ;
      
      if( iCode & 0x0400 ) iMan |= 0xFFFFF800 ;
      else                 iMan |= 0x00000400 ;
      
      fVal = ((fl32)iMan) * ((fl32)(ui32)(1 << iExp)) / 1.099511627E12 ;
    }
    *fIQVals++ = fVal ;
  }
}

/* Convert a normalized floating "I" or "Q" value to/from the signal
 * processor's 16-bit packed format.  The floating values are in the
 * range -4.0 to +4.0, i.e., they are normalized so that full scale CW
 * input gives a magnitude of 1.0, while still leaving a factor of
 * four of additional amplitude headroom (12dB headroom power) to
 * account for FIR filter transients.
 */

/* ------------------------------
 * Convert an array of FLT4's to packed floating.
 * assumes HIGH SNR packing and no swapping
 */

#define NINT(fvalue) ((si32)floor(0.5+(double)(fvalue)))

void IwrfTsPulse::vecPackIQFromFloatIQ
  ( volatile ui16 iCodes_a[], volatile const fl32 fIQVals_a[],
    si32 iCount_a )
{

  si32 iCount ; volatile const fl32 *fIQVals = fIQVals_a ;
  volatile ui16 *iCodes = iCodes_a ;

  /* High SNR packed format with 12-bit mantissa
   */
  for( iCount=0 ; iCount < iCount_a ; iCount++ ) {
    ui16 iCode ; fl32 fIQVal = *fIQVals++ ;
    
    if     (  fIQVal >=  4.0 ) iCode = 0xF7FF ;
    else if(  fIQVal <= -4.0 ) iCode = 0xF800 ;
    else if( (fIQVal >  -1.221299E-4) &&
             (fIQVal <   1.220703E-4) ) {
      si32 iUnderFlow = NINT( 1.677721E7 * fIQVal ) ;
      iCode = 0xFFF & MAX( -2048, MIN( 2047, iUnderFlow ) ) ;
    } else {
      int iSign, iExp, iMan ;
#ifdef IEEE754_FLOAT_BIAS
      /* Much faster (x4.5) version on LINUX uses hardware definition
       * of fl32.  Native mantissa has 23 bits, we want 12, rounded.
       * Courtesy of Nathan Parker, MIT/LL, Aug 2002.
       */
      union ieee754_float u ; int uMan ;
      u.f = fIQVal ; uMan = u.ieee.mantissa ; iSign = u.ieee.negative ;
      iMan = (uMan >> 12) | (1<<11) ; if( iSign ) iMan = ~iMan ;
      iMan += ((uMan >> 11) & 1 ) ^ iSign ;
      
      iExp = (int)u.ieee.exponent - (IEEE754_FLOAT_BIAS - 1) + 13 ;
#else
      iMan = NINT( 4096.0 * frexp( fIQVal, &iExp ) ) ;
      iExp += 13 ; iSign = (fIQVal < 0.0) ;
#endif
      
      if( iMan ==  4096 ) iExp++ ; /* Fix up mantissa overflows */
      if( iMan == -2048 ) iExp-- ; /*   by adjusting the exponent. */
      
      iCode = (iExp << 12) | (iSign << 11) | (0x7FF & iMan) ;
    }
    *iCodes++ = iCode ;
  }

}

////////////////////////////////////////////////////////////
// print data

void IwrfTsPulse::_doPrintData(FILE *out, int channel, int startGate, int endGate)
{

  // if starting at beginning, include burst gates
  if (startGate <= 0) {
    startGate = -_hdr.n_gates_burst;
  }
  if (endGate > _hdr.n_gates - 1) {
    endGate = _hdr.n_gates - 1;
  }

  fprintf(out, "====== ========== IQ data channel %d =========================\n",
	  channel);

  fprintf(out, " %5s %15s %15s %15s %15s",
	  "gate", "i(volts)", "q(volts)", "power(dBm)", "phase(deg)");
  if (_packed != NULL) {
    if (_packedEncoding == IWRF_IQ_ENCODING_SCALED_SI16) {
      fprintf(out, " %14s %14s", "i(scaled)", "q(scaled)");
    } else if (_packedEncoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {
      fprintf(out, " %14s %14s", "power(scaled)", "phase(scaled)");
    }
  }
  fprintf(out, "\n");

  const fl32 *iq = _iqData + _hdr.iq_offset[channel];
  const si16 *packed = _packed + _hdr.iq_offset[channel];
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
      if (_packedEncoding == IWRF_IQ_ENCODING_SCALED_SI16) {
	fprintf(out, " %14d %14d", ipacked, qpacked);
      } else if (_packedEncoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {
	fprintf(out, " %14d %14d", ipacked, qpacked);
      }
    }

    fprintf(out, "\n");

  } // igate

  fprintf(out, "=================================================================\n");

}

