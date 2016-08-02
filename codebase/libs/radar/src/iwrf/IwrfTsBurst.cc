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
// IwrfTsBurst.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
////////////////////////////////////////////////////////////////

#include <iostream>
#include <cerrno>
#include <toolsa/mem.h>
#include <toolsa/sincos.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/IwrfTsBurst.hh>
using namespace std;

// Constructor

IwrfTsBurst::IwrfTsBurst(IwrfDebug_t debug) :
        _debug(debug)
  
{

  iwrf_burst_header_init(_hdr);

  _iq = NULL;
  _packed = NULL;

  _packedEncoding = IWRF_IQ_ENCODING_LAST;
  _packedScale = 1.0;
  _packedOffset = 0.0;

}

/////////////////////////////
// Copy constructor

IwrfTsBurst::IwrfTsBurst(const IwrfTsBurst &rhs)

{
  if (this != &rhs) {
    _copy(rhs);
  }
}

/////////////////////////////
// destructor

IwrfTsBurst::~IwrfTsBurst()

{
}

/////////////////////////////
// Assignment
//

IwrfTsBurst &IwrfTsBurst::operator=(const IwrfTsBurst &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////////////////////
// clear all

void IwrfTsBurst::clear()

{
  
  _clearIq();
  _clearPacked();
  iwrf_burst_header_init(_hdr);

}

////////////////////////////////////////////////////////////
// set time

void IwrfTsBurst::setTime(time_t secs, int nano_secs)
{
  iwrf_set_packet_time(_hdr.packet, secs, nano_secs);
}

void IwrfTsBurst::setTimeToNow()
{
  struct timeval time;
  gettimeofday(&time, NULL);
  setTime(time.tv_sec, time.tv_usec * 1000);
}

////////////////////////////////////////////////////////////
// set header directly

void IwrfTsBurst::setHeader(const iwrf_burst_header_t &hdr)
{
  _hdr = hdr;
  _hdr.packet.id = IWRF_BURST_HEADER_ID;
  _hdr.packet.len_bytes = sizeof(iwrf_burst_header_t);
  _hdr.packet.version_num = 1;
}

////////////////////////////////////////////////////////////
// set sequence number

void IwrfTsBurst::setPktSeqNum(si64 pkt_seq_num)
{
  _hdr.packet.seq_num = pkt_seq_num;
}

///////////////////////////////////////////////////////////
// set from buffer, swapping as required
// optionally convert iq data to floats
// Returns 0 on success, -1 on failure

int IwrfTsBurst::setFromBuffer(const void *buf, int len,
			       bool convertToFloat)
  
{

  // check validity of packet
  
  int packet_id;
  if (iwrf_get_packet_id(buf, len, packet_id)) {
    cerr << "ERROR - IwrfTsBurst::setFromBuffer" << endl;
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

  if (packet_id != IWRF_BURST_HEADER_ID) {
    cerr << "ERROR - IwrfTsBurst::setFromBuffer" << endl;
    fprintf(stderr, "  Incorrect packet id: 0x%x\n", packet_id);
    cerr << "                  len: " << len << endl;
    cerr << "                 type: "
         << iwrf_packet_id_to_str(packet_id) << endl;
    delete[] copy;
    return -1;
  }

  memcpy(&_hdr, copy, sizeof(iwrf_burst_header_t));
  
  _ftime = (double) _hdr.packet.time_secs_utc +
    _hdr.packet.time_nano_secs / 1.0e9;
  
  // load up IQ data
  
  int requiredLen =
    (int) sizeof(iwrf_burst_header_t) + _hdr.n_samples * 2 * sizeof(si16);
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    requiredLen =
      (int) sizeof(iwrf_burst_header_t) + _hdr.n_samples * 2 * sizeof(fl32);
  }
  if (len < requiredLen) {
    cerr << "ERROR - IwrfTsBurst::setFromBuffer" << endl;
    cerr << "  Buffer passed in too short, len: " << len << endl;
    cerr << "  Must be at least len: " << requiredLen << endl;
    cerr << "sizeof(iwrf_burst_header_t): " << sizeof(iwrf_burst_header_t) << endl; 
    iwrf_burst_header_print(stderr, _hdr);
    delete[] copy;
    return -1;
  }
  
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    
    _clearPacked();
    fl32 *iq = (fl32 *) (copy + sizeof(iwrf_burst_header_t));
    _iq = (fl32 *) _iqBuf.load(iq, _hdr.n_samples * 2 * sizeof(fl32));

  } else {
    
    _clearIq();
    si16 *packed = (si16 *) (copy + sizeof(iwrf_burst_header_t));
    _packed = (si16 *) _packedBuf.load(packed, _hdr.n_samples * 2 * sizeof(si16));

  }

  _packedEncoding = (iwrf_iq_encoding_t) _hdr.iq_encoding;
  _packedScale = _hdr.scale;
  _packedOffset = _hdr.offset;
  
  if (convertToFloat) {
    convertToFL32();
  }

  delete[] copy;
  return 0;

}

///////////////////////////////////////////////////////////
// set IQ data as floats

void IwrfTsBurst::setIqFloats(int nSamples,
			      const fl32 *iq)
  
{

  _hdr.n_samples = nSamples;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
  
  _iq = (fl32 *) _iqBuf.load(iq, _hdr.n_samples * 2 * sizeof(fl32));
  
  _clearPacked();
  _packedEncoding = IWRF_IQ_ENCODING_FL32;
  _packedScale = _hdr.scale;
  _packedOffset = _hdr.offset;

}

///////////////////////////////////////////////////////////
// set IQ data as packed si16
  
void IwrfTsBurst::setIqPacked(int nSamples,
			      iwrf_iq_encoding_t encoding,
			      const si16 *packed,
			      double scale = 1.0,
			      double offset = 0.0)
  
{
  
  _hdr.n_samples = nSamples;
  _hdr.iq_encoding = encoding;
  _hdr.scale = scale;
  _hdr.offset = offset;
  
  _packed = (si16 *) _packedBuf.load(packed, _hdr.n_samples * 2 * sizeof(si16));
  
  _clearIq();
  _packedEncoding = encoding;
  _packedScale = scale;
  _packedOffset = offset;

}

///////////////////////////////////////////////////////////
// convert packed data to float32

void IwrfTsBurst::convertToFL32()
  
{

  if (_packedEncoding == IWRF_IQ_ENCODING_FL32) {
    return;
  }

  _iq = (fl32 *) _iqBuf.prepare(_hdr.n_samples * 2 * sizeof(fl32));
  
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    
    // compute from signed scaled values
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0; ii < _hdr.n_samples * 2; ii++) {
      _iq[ii] = packed[ii] * _packedScale + _packedOffset;
    }
    
  } else if (_hdr.iq_encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {

    // compute from power and phase
    
    si16 *packed = (si16 *) _packed;
    double phaseMult = 180.0 / 32767.0;
    for (int ii = 0; ii < _hdr.n_samples * 2; ii += 2) {
      double powerDbm = packed[ii] * _packedScale + _packedOffset;
      double power = pow(10.0, powerDbm / 10.0);
      double phaseDeg = packed[ii+1] * phaseMult;
      double sinPhase, cosPhase;
      ta_sincos(phaseDeg * DEG_TO_RAD, &sinPhase, &cosPhase);
      double mag = sqrt(power);
      _iq[ii] = mag * cosPhase;
      _iq[ii+1] = mag * sinPhase;

    }
    
  }
  
  _hdr.scale = 1.0;
  _hdr.offset = 0.0;
  _hdr.iq_encoding = IWRF_IQ_ENCODING_FL32;
  
}

///////////////////////////////////////////////////////////
// convert to specified packing

void IwrfTsBurst::convertToPacked(iwrf_iq_encoding_t encoding,
				  double scale,
				  double offset)
  
{

  if (encoding == _packedEncoding) {
    // nothing to do
    _hdr.scale = _packedScale;
    _hdr.offset = _packedOffset;
    _hdr.iq_encoding = _packedEncoding;
    return;
  }

  if (encoding == IWRF_IQ_ENCODING_FL32 && _iq != NULL) {
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

  if (_iq == NULL) {
    // make sure we haev float 32 data available
    convertToFL32();
  }

  // prepare packed buffer

  _packed = (si16 *) _packedBuf.prepare(_hdr.n_samples * 2 * sizeof(si16));

  // fill packed array
  
  if (encoding == IWRF_IQ_ENCODING_SCALED_SI16) {
    
    // compute to scaled signed int16
    
    si16 *packed = (si16 *) _packed;
    for (int ii = 0; ii < _hdr.n_samples * 2; ii++) {
      int packedVal = (int) floor((_iq[ii] - offset) / scale + 0.5);
      if (packedVal < -32768) {
	packedVal = -32768;
      } else if (packedVal > 32767) {
	packedVal = 32767;
      }
      packed[ii] = packedVal;
    }
    
  } else if (encoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {

    // compute from power and phase
    
    si16 *packed = (si16 *) _packed;
    double phaseMult = 180.0 / 32767.0;
    for (int ii = 0; ii < _hdr.n_samples * 2; ii += 2) {

      double ival = _iq[ii];
      double qval = _iq[ii+1];

      double power = ival * ival + qval * qval;
      double powerDbm = 10.0 * log10(power);
      int packedPower = (int) floor((powerDbm - offset) / scale + 0.5);
      if (packedPower < -32767) {
	packedPower = -32767;
      } else if (packedPower > 32767) {
	packedPower = 32767;
      }
      packed[ii] = (si16) packedPower;
      
      double phaseDeg = 0.0;
      if (ival != 0 || qval != 0) {
	phaseDeg = atan2(qval, ival) * RAD_TO_DEG;
      }
      int packedPhase = (int) (phaseDeg / phaseMult + 0.5);
      if (packedPhase < -32767) {
	packedPhase = -32767;
      } else if (packedPhase > 32767) {
	packedPhase = 32767;
      }
      packed[ii+1] = (si16) packedPhase;

    }
    
  }

  // save values

  _packedScale = scale;
  _packedOffset = offset;
  _packedEncoding = encoding;

  _hdr.scale = _packedScale;
  _hdr.offset = _packedOffset;
  _hdr.iq_encoding = _packedEncoding;

}

///////////////////////////////////////////////////////////
// assemble into a single buffer

void IwrfTsBurst::assemble(MemBuf &buf) const
{
  iwrf_burst_header_t hdr = _hdr;
  int nbytesIq = 0;
  if (hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    nbytesIq = hdr.n_samples * 2 * sizeof(fl32);
  } else {
    nbytesIq = hdr.n_samples * 2 * sizeof(si16);
  }
  hdr.packet.len_bytes = sizeof(iwrf_burst_header_t) + nbytesIq;
  buf.add(&hdr, sizeof(hdr));
  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    buf.add(_iq, nbytesIq);
  } else {
    buf.add(_packed, nbytesIq);
  }
}

//////////////////////////////
// get packed data
// returns NULL on error

const void *IwrfTsBurst::getPackedData() const

{

  if (_hdr.iq_encoding == IWRF_IQ_ENCODING_FL32) {
    return _iq;
  } else {
    return _packed;
  }

}

////////////////////////////////////////////////////////////
// print headers

void IwrfTsBurst::printHeader(FILE *out) const
{
  iwrf_burst_header_print(out, _hdr);
}

////////////////////////////////////////////////////////////
// print data

void IwrfTsBurst::printData(FILE *out) const
{
  printData(out, 0, _hdr.n_samples - 1);
}

void IwrfTsBurst::printData(FILE *out, int startSample, int endSample) const
{
  
  // copy this object for printing
  
  IwrfTsBurst burst(*this);
  burst.convertToFL32();

  if (startSample < 0) {
    startSample = 0;
  }
  if (endSample > _hdr.n_samples - 1) {
    endSample = _hdr.n_samples - 1;
  }

  fprintf(out, "================ Burst IQ data channel %d ===================\n",
	  _hdr.channel_id);
  
  fprintf(out, " %5s %15s %15s %15s %15s",
	  "sample", "i(volts)", "q(volts)", "power(dBm)", "phase(deg)");
  if (_packed != NULL) {
    if (_packedEncoding == IWRF_IQ_ENCODING_SCALED_SI16) {
      fprintf(out, " %14s %14s", "i(scaled)", "q(scaled)");
    } else if (_packedEncoding == IWRF_IQ_ENCODING_DBM_PHASE_SI16) {
      fprintf(out, " %14s %14s", "power(scaled)", "phase(scaled)");
    }
  }
  fprintf(out, "\n");

  const fl32 *iq = burst._iq;
  const si16 *packed = _packed;
  for (int isample = startSample, ii = startSample * 2;
       isample <= endSample; isample++, ii += 2) {
    
    double ival = iq[ii];
    double qval = iq[ii + 1];

    double power = ival * ival + qval * qval;
    double powerDbm = 10.0 * log10(power);
    double phaseRad = 0.0;
    if (ival != 0 || qval != 0) {
      phaseRad = atan2(qval, ival);
    }
    double phaseDeg = phaseRad * RAD_TO_DEG;

    fprintf(out, " %5d %15.8e %15.8e %15.3f %15.3f",
	    isample, ival, qval, powerDbm, phaseDeg);

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

  } // isample

  fprintf(out, "=================================================================\n");

  
}

///////////////////////////////////////////////////
// write to file in IWRF format
//
// returns 0 on success, -1 on failure

int IwrfTsBurst::writeToFile(FILE *out) const

{

  if (out == NULL) {
    return -1;
  }
  
  MemBuf buf;
  assemble(buf);
  if (fwrite(buf.getPtr(), buf.getLen(), 1, out) != 1) {
    int errNum = errno;
    cerr << "ERROR - IwrfTsBurst::write2File" << endl;
    cerr << "  Cannot write burst" << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }

  return 0;

}
  
/////////////////////////////
// copy

IwrfTsBurst &IwrfTsBurst::_copy(const IwrfTsBurst &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy members

  _hdr = rhs._hdr;
  _debug = rhs._debug;
  _ftime = rhs._ftime;

  _iqBuf = rhs._iqBuf;
  _iq = (fl32 *) _iqBuf.getPtr();

  _packedBuf = rhs._packedBuf;
  _packed = (si16 *) _packedBuf.getPtr();

  _packedEncoding = rhs._packedEncoding;
  _packedScale = rhs._packedScale;
  _packedOffset = rhs._packedOffset;

  return *this;

}

////////////////////
// clean up memory

void IwrfTsBurst::_clearIq()
{
  _iqBuf.free();
  _iq = NULL;
}

void IwrfTsBurst::_clearPacked()
{
  _packedBuf.free();
  _packed = NULL;
}

