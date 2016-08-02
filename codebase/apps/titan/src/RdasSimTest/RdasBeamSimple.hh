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
// RdasBeamSimple.hh
//
// Simple RDAS beam
//
// Mike Dixon
// Feb 2003
//
//////////////////////////////////////////////////////////

#ifndef RdasBeamSimple_H
#define RdasBeamSimple_H

#include <dataport/port_types.h>
using namespace std;

// simple beam definition

#define RDAS_SIMPLE_NGATES 1000

typedef struct {
  si16 cookie1;
  si16 cookie2;
  si16 year;
  si16 month;
  si16 day;
  si16 hour;
  si16 min;
  si16 sec;
  si16 flags1;
  si16 flags2;
  si16 az;
  si16 el;
  si16 spare[12];
  si16 counts[RDAS_SIMPLE_NGATES];
} rdas_simple_beam_t;

// Counts will be 14 bit values
// Word 1 Flags will be the following:
// bit 0 = System warmed up and ready ( 1 = ready) ( 0 = not ready  or
// indicates system fault ).
// bit 1 = Waveguide Pressure (1 = good ) ( 0 = low ).
// bit 2 = Magnetron power is on. ( 1 = ON ) ( 0 = OFF or Fault ).
// bit 3  to bit 15 all = 0 for now.
// Word 2 is not used yet and will all be = 0.

class RdasBeamSimple {
  
public:

  RdasBeamSimple();
  ~RdasBeamSimple();

  void setTime(time_t time);
  void setPowerStatus(bool status);
  void setWaveguideStatus(bool status);
  void setMagnetronStatus(bool status);
  void setEl(double el);
  void setAz(double az);
  void setCounts(si16 *counts, int ncounts);

  void assemble();
  int disassemble(void *buf, int nbytes);

  int getBufNbytes() { return sizeof(_buf); }
  void *getBufPtr() { return (void *) &_buf; }

  time_t getTime() { return _time; }
  bool getPowerStatus() { return _powerStatus; }
  bool getWaveguideStatus() { return _waveguideStatus; }
  bool getMagnetronStatus() { return _magnetronStatus; }
  double getEl() { return _el; }
  double getAz() { return _az; }
  si16 *getCounts() { return _counts; }
  int getNCounts() { return RDAS_SIMPLE_NGATES; }

protected:
private:

  time_t _time;
  bool _powerStatus;
  bool _waveguideStatus;
  bool _magnetronStatus;
  double _el;
  double _az;
  si16 _counts[RDAS_SIMPLE_NGATES];

  rdas_simple_beam_t _buf;

};

#endif
