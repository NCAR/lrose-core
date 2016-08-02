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
// RdasBeam.cc
//
// Simple RDAS beam
//
// Mike Dixon
// Sept 2003
//
//////////////////////////////////////////////////////////

#include "RdasBeam.hh"
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <rapmath/stats.h>

using namespace std;

// constructor

RdasBeam::RdasBeam()

{

  _time = 0;
  _mainPower = false;
  _systemReady = false;
  _servoPower = false;
  _radiate = false;
  _statusFlags = 0;
  _errorFlags = 0;
  _el = 0.0;
  _az = 0.0;
  _nGates = 0;
  _nFields = 1;
  MEM_zero(_counts);

}

// destructor

RdasBeam::~RdasBeam()

{

}

// set methods

void RdasBeam::setTime(time_t time) {
  _time = time;
}

void RdasBeam::setMainPower(bool status) {
  _mainPower = status;
}

void RdasBeam::setSystemReady(bool status) {
  _systemReady = status;
}

void RdasBeam::setServoPower(bool status) {
  _servoPower = status;
}

void RdasBeam::setRadiate(bool status) {
  _radiate = status;
}

void RdasBeam::setStatusFlags(int flags) {
  _statusFlags = flags;
}

void RdasBeam::setErrorFlags(int flags) {
  _errorFlags = flags;
}

void RdasBeam::setEl(double el) {
  _el = el;
}

void RdasBeam::setAz(double az) {
  _az = az;
}

void RdasBeam::setGateSpacing(double gate_spacing) {
  _gateSpacing = gate_spacing;
}

void RdasBeam::setStartRange(double start_range) {
  _startRange = start_range;
}

void RdasBeam::setPulseWidth(double pulse_width) {
  _pulseWidth = pulse_width;
}

void RdasBeam::setPrf(double prf) {
  _prf = prf;
}

void RdasBeam::setBeamCount(int count) {
  _beamCount = count;
}

void RdasBeam::setTiltCount(int count) {
  _tiltCount = count;
}

void RdasBeam::setEndOfTiltFlag(int flag) {
  _endOfTiltFlag = flag;
}

void RdasBeam::setEndOfVolFlag(int flag) {
  _endOfVolFlag = flag;
}

void RdasBeam::setStatusString(const string &str) {
  _statusString = str;
}

void RdasBeam::setCounts(si16 *counts, int ngates) {
  if (ngates > _maxCounts) {
    ngates = _maxCounts;
  }
  _nGates = ngates;
  MEM_zero(_counts);
  memcpy(_counts, counts, ngates * sizeof(si16));
}

void RdasBeam::assemble(bool bigEndian /* = true */) {

  // load up the values

  rdas_beam_hdr_t hdr;
  MEM_zero(hdr);
  
  hdr.cookie = 0x5A5A5A5A;
  hdr.version = 1;
  hdr.struct_len = sizeof(rdas_beam_hdr_t);
  hdr.count_data_included = 1;

  DateTime dtime(_time);
  hdr.year = (si16) dtime.getYear();
  hdr.month = (si16) dtime.getMonth();
  hdr.day = (si16) dtime.getDay();
  hdr.hour = (si16) dtime.getHour();
  hdr.min = (si16) dtime.getMin();
  hdr.sec = (si16) dtime.getSec();
  hdr.msec = 0;
  hdr.ngates = _nGates;
  hdr.nfields = _nFields;
  hdr.nsamples = 32;
  hdr.beam_count = _beamCount;
  hdr.tilt_count = _tiltCount;
  hdr.end_of_tilt_flag = _endOfTiltFlag;
  hdr.end_of_vol_flag = _endOfVolFlag;
  
  int powerFlags = 0;
  if (_mainPower) {
    powerFlags |= 1;
  }
  if (_systemReady) {
    powerFlags |= 2;
  }
  if (_servoPower) {
    powerFlags |= 4;
  }
  if (_radiate) {
    powerFlags |= 8;
  }
  hdr.power_flags = (si32) powerFlags;
  hdr.status_flags = (si32) _statusFlags;
  hdr.error_flags = (si32) _errorFlags;
  hdr.field_codes[0] = 99;
  
  hdr.el = _el;
  hdr.el_target = _el;
  hdr.az = _az;

  hdr.lat_deg = -28.0;
  hdr.lat_frac_deg = 0.5;
  hdr.lon_deg = 28.0;
  hdr.lon_frac_deg = 0.25;
  hdr.gate_spacing = _gateSpacing;
  hdr.start_range = _startRange;
  hdr.pulse_width = _pulseWidth;
  hdr.prf = _prf;
  for (int i = 0; i < RDAS_BEAM_NSTATUS; i++) {
    double xx = 1.0 + (STATS_uniform_gen() - 0.5) * 0.1;
    hdr.analog_status[i] = (fl32) ((i + 1.0) * xx);
  }

  if (_statusString.size() > 0) {
    STRncopy(hdr.status_string, _statusString.c_str(), 64);
  }
  
  // swap hdr
  
  if (bigEndian) {
    BE_from_array_32(&hdr, RDAS_BEAM_N32 * sizeof(si32));
  }
  
  // counts
  
  int nBytes = _nGates * _nFields * sizeof(short);
  TaArray<short> counts_;
  short *counts = counts_.alloc(_nGates * _nFields);
  memcpy(counts, _counts, nBytes);
  if (bigEndian) {
    BE_from_array_16(counts, nBytes);
  }

  // add to buffer

  _buf.reset();
  _buf.add(&hdr, sizeof(hdr));
  _buf.add(counts, _nGates * _nFields * sizeof(short));

}
