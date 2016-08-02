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
// RdasBeam.hh
//
// Simple RDAS beam
//
// Mike Dixon
// Feb 2003
//
//////////////////////////////////////////////////////////

#ifndef RdasBeam_H
#define RdasBeam_H

#include <dataport/port_types.h>
#include <toolsa/MemBuf.hh>
#include <string>
using namespace std;

#define RDAS_BEAM_NSTATUS 8
#define RDAS_BEAM_N32 80

// beam definition

typedef struct {
  si32 cookie;
  si32 version;
  si32 struct_len;
  si32 count_data_included;
  si32 radar_id;
  si32 year;
  si32 month;
  si32 day;
  si32 hour;
  si32 min;
  si32 sec;
  si32 msec;
  si32 ngates;
  si32 nfields;
  si32 nsamples;
  si32 polarization_code;
  si32 beam_count;
  si32 tilt_count;
  si32 end_of_tilt_flag;
  si32 end_of_vol_flag;
  si32 power_flags;
  si32 status_flags;
  si32 field_codes[12];
  si32 error_flags;
  si32 spare_ints[9];
  fl32 az;
  fl32 el;
  fl32 el_target;
  fl32 alt_km;
  fl32 lat_deg;
  fl32 lat_frac_deg;
  fl32 lon_deg;
  fl32 lon_frac_deg;
  fl32 gate_spacing;
  fl32 start_range;
  fl32 pulse_width;
  fl32 prf;
  fl32 analog_status[RDAS_BEAM_NSTATUS];
  fl32 spare_floats[16];
  char status_string[64];
} rdas_beam_hdr_t;

// Counts will be 14 bit values
// Word 1 Flags will be the following:
// bit 0 = System warmed up and ready ( 1 = ready) ( 0 = not ready  or
// indicates system fault ).
// bit 1 = Waveguide Pressure (1 = good ) ( 0 = low ).
// bit 2 = Magnetron power is on. ( 1 = ON ) ( 0 = OFF or Fault ).
// bit 3  to bit 15 all = 0 for now.
// Word 2 is not used yet and will all be = 0.

class RdasBeam {
  
public:

  RdasBeam();
  ~RdasBeam();

  void setTime(time_t time);
  void setMainPower(bool status);
  void setSystemReady(bool status);
  void setServoPower(bool status);
  void setRadiate(bool status);
  void setStatusFlags(int flags);
  void setErrorFlags(int flags);
  void setEl(double el);
  void setAz(double az);
  void setGateSpacing(double gate_spacing);
  void setStartRange(double start_range);
  void setPulseWidth(double width);
  void setPrf(double prf);

  void setBeamCount(int count);
  void setTiltCount(int count);
  void setEndOfTiltFlag(int flag);
  void setEndOfVolFlag(int flag);

  void setStatusString(const string &str);

  void setCounts(si16 *counts, int ngates);

  void assemble(bool bigEndian = true);
  int disassemble(void *buf, int nbytes);

  int getBufNbytes() { return _buf.getLen(); }
  void *getBufPtr() { return _buf.getPtr(); }
  
  time_t getTime() { return _time; }
  bool getMainPower() { return _mainPower; }
  bool getSystemReady() { return _systemReady; }
  bool getServoPower() { return _servoPower; }
  bool getRadiate() { return _radiate; }
  double getEl() { return _el; }
  double getAz() { return _az; }
  si16 *getCounts() { return _counts; }
  int getNCounts() { return _nGates * _nFields; }

protected:
private:

  time_t _time;

  bool _mainPower;
  bool _systemReady;
  bool _servoPower;
  bool _radiate;
  int _statusFlags;
  int _errorFlags;

  int _version;
  int _struct_len;
  int _nGates;
  int _nFields;
  int _beamCount;
  int _tiltCount;
  int _endOfTiltFlag;
  int _endOfVolFlag;

  double _el;
  double _az;
  double _gateSpacing;
  double _startRange;
  double _pulseWidth;
  double _prf;

  string _statusString;

  static const int _maxCounts = 2048;
  si16 _counts[_maxCounts];
  MemBuf _buf;

};

#endif
