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
// RdasBeamSimple.cc
//
// Simple RDAS beam
//
// Mike Dixon
// Feb 2003
//
//////////////////////////////////////////////////////////

#include "RdasBeamSimple.hh"
#include <dataport/bigend.h>
#include <toolsa/mem.h>
#include <toolsa/DateTime.hh>

using namespace std;

// constructor

RdasBeamSimple::RdasBeamSimple()

{

  _time = 0;
  _mainPower = false;
  _magnetronPower = false;
  _servoPower = false;
  _radiate = false;
  _el = 0.0;
  _az = 0.0;
  MEM_zero(_counts);

}

// destructor

RdasBeamSimple::~RdasBeamSimple()

{

}

// set methods

void RdasBeamSimple::setTime(time_t time) {
  _time = time;
}

void RdasBeamSimple::setMainPower(bool status) {
  _mainPower = status;
}

void RdasBeamSimple::setMagnetronPower(bool status) {
  _magnetronPower = status;
}

void RdasBeamSimple::setServoPower(bool status) {
  _servoPower = status;
}

void RdasBeamSimple::setRadiate(bool status) {
  _radiate = status;
}

void RdasBeamSimple::setEl(double el) {
  _el = el;
}

void RdasBeamSimple::setAz(double az) {
  _az = az;
}

void RdasBeamSimple::setCounts(si16 *counts, int ncounts) {

  if (ncounts > RDAS_SIMPLE_NGATES) {
    ncounts = RDAS_SIMPLE_NGATES;
  }
  MEM_zero(_counts);
  memcpy(_counts, counts, ncounts * sizeof(si16));

}

void RdasBeamSimple::assemble(bool bigEndian /* = true */) {

  // load up the values

  MEM_zero(_buf);

  DateTime dtime(_time);
  _buf.year = (si16) dtime.getYear();
  _buf.month = (si16) dtime.getMonth();
  _buf.day = (si16) dtime.getDay();
  _buf.hour = (si16) dtime.getHour();
  _buf.min = (si16) dtime.getMin();
  _buf.sec = (si16) dtime.getSec();

  int flags1 = 0;
  if (_mainPower) {
    flags1 |= 1;
  }
  if (_magnetronPower) {
    flags1 |= 2;
  }
  if (_servoPower) {
    flags1 |= 4;
  }
  if (_radiate) {
    flags1 |= 8;
  }
  _buf.flags1 = (si16) flags1;

  _buf.el = (si16) (_el * 10.0 + 0.5);
  _buf.az = (si16) (_az * 10.0 + 0.5);

  memcpy(_buf.counts, _counts, sizeof(_counts));

  // swap them

  if (bigEndian) {
    BE_from_array_16(&_buf, sizeof(_buf));
  }

  // set the cookie

  _buf.cookie1 = 0x5A5A;
  _buf.cookie2 = 0x5A5A;

}

int RdasBeamSimple::disassemble(void *buf, int nbytes) {

  if (nbytes != sizeof(_buf)) {
    cerr << "ERROR - RdasBeamSimple::disassemble" << endl;
    cerr << "  Wrong input buffer size" << endl;
    cerr << "  Buf size: " << nbytes << endl;
    cerr << "  Expecting: " << sizeof(_buf) << endl;
    return -1;
  }

  // copy in and swap
  
  memcpy(&_buf, buf, nbytes);
  if (_buf.cookie1 != 0x5A5A || _buf.cookie2 != 0x5A5A) {
    cerr << "ERROR - RdasBeamSimple::disassemble" << endl;
    cerr << "  Bad magic cookie" << endl;
    return -1;
  }
  
  // swap

  BE_to_array_16(&_buf, sizeof(_buf));

  // load up the values

  DateTime dtime(_buf.year, _buf.month, _buf.day,
		 _buf.hour, _buf.min, _buf.sec);
  _time = dtime.utime();

  if (_buf.flags1 & 1) {
    _mainPower = true;
  } else {
    _mainPower = false;
  }

  if (_buf.flags1 & 2) {
    _magnetronPower = true;
  } else {
    _magnetronPower = false;
  }

  if (_buf.flags1 & 4) {
    _servoPower = true;
  } else {
    _servoPower = false;
  }

  if (_buf.flags1 & 8) {
    _radiate = true;
  } else {
    _radiate = false;
  }

  _el = (double) _buf.el / 10.0;
  _az = (double) _buf.az / 10.0;

  memcpy(_counts, _buf.counts, sizeof(_counts));

  return 0;

}


