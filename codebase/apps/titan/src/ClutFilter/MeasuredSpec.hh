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
// MeasuredSpec.hh
//
// Measured spectrum
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2004
//
/////////////////////////////////////////////////////////////

#ifndef MEASURED_SPEC_HH
#define MEASURED_SPEC_HH

#include <cstdio>
#include <ctime>
#include "Complex.hh"
using namespace std;

class MeasuredSpec {
  
public:

  MeasuredSpec();
  ~MeasuredSpec();

  // set state by reading input file
  // returns 0 on success, -1 on failure

  int read(FILE *inFile);
  
  // getting the values

  int getIndex() const { return _index; }
  time_t getTime() const { return _time; }
  double getEl() const { return _el; }
  double getAz() const { return _az; }
  int getGateNum() const { return _gateNum; }
  double getSnr() const { return _snr; }
  double getVel() const { return _vel; }
  double getWidth() const { return _width; }
  int getNSamples() const { return _nSamples; }
  const Complex_t *getIQ() const { return _iq; }
  
protected:
private:

  int _index;
  time_t _time;
  double _el;
  double _az;
  int _gateNum;
  double _snr;
  double _vel;
  double _width;
  int _nSamples;
  Complex_t *_iq;
  
};

#endif
