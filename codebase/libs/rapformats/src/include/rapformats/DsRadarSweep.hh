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
// DsRadarSweep.hh
//
// C++ class for dealing with radar sweep information
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// June 2006
//////////////////////////////////////////////////////////////

#ifndef _DsRadarSweep_hh
#define _DsRadarSweep_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>
using namespace std;

#define SWEEP_INFO_ID 999

typedef enum {
  ScanMode_CALBRATION = 0,
  ScanMode_PPI_SECTOR = 1,
  ScanMode_COPLANE = 2,
  ScanMode_RHI = 3,
  ScanMode_VERT_POINT = 4,
  ScanMode_TARGET = 5,
  ScanMode_MANUAL = 6,
  ScanMode_IDLE = 7,
  ScanMode_SURVEILLANCE = 8,
  ScanMode_AIR = 9,
  ScanMode_HORIZONTAL = 10,
  ScanMode_SUNSCAN = 11,
  ScanMode_POINTING = 12,
  ScanMode_FOLLOW_VEHICLE = 13,
  ScanMode_EL_SURV = 14,
  ScanMode_MANPPI = 15,
  ScanMode_MANRHI = 16,
  ScanMode_SUNSCAN_RHI = 17
} sweep_info_scan_mode_t;

// Sweep info UDP packet
// the following fields are sent in network byte order,
// most significant byte first, aka BigEndian

typedef struct 
{
  ui32 id;
  ui32 length;
  ui32 time_secs; // secs_since_1970;
  ui32 nano_secs;  // since even second, if available
  ui32 scan_type; // RHI, PPI, SURVEILLANCE, VERT, SUN etc.
  ui32 volume_num;
  ui32 sweep_num;
  ui32 end_of_sweep_flag; // to be set for one packet only
  ui32 end_of_vol_flag;  // to be set for one packet only
  fl32 cur_el; // measured elevation
  fl32 cur_az; // measured azimuth
  fl32 fixed_el; // target elevation angle for PPI, SUR
  fl32 fixed_az; // target azimuth for  RHI
  ui32 num_gates;
  ui32 num_samples;
  ui32 transition; // antenna is in transition
  ui32 spare0;
  ui32 spare1;
  ui32 spare2;
  ui32 spare3;
} sweep_info_packet_t;

class DsRadarSweep {

public:

  // constructor

  DsRadarSweep();

  // destructor

  ~DsRadarSweep();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  //////////////////////// set methods /////////////////////////
  
  void setName(const string &name) { _name = name; }
  void setScanMode(const string &modeStr);
  void setScanMode(int mode);
  
  void setStartTime(time_t utime, int nano_secs) {
    _startUTime = utime;
    _startNanoSecs = nano_secs;
  }
  
  void setEndTime(time_t utime, int nano_secs) {
    _endUTime = utime;
    _endNanoSecs = nano_secs;
  }
  
  void setStartEl(double el) { _startEl = el; }
  void setStartAz(double az) { _startAz = az; }

  void setEndEl(double el) { _endEl = el; }
  void setEndAz(double az) { _endAz = az; }

  void setFixedEl(double el) { _fixedEl = el; }
  void setFixedAz(double az) { _fixedAz = az; }

  void setClockWise(bool isClockWise) { _isClockWise = isClockWise; }

  void setPrf(double prf) { _prf = prf; }

  void setVolNum(int volNum) { _volNum = volNum; }
  void setTiltNum(int tiltNum) { _tiltNum = tiltNum; } // in a volume
  void setSweepNum(int sweepNum) { _sweepNum = sweepNum; } // grows forever

  void setNSamples(int nSamples) { _nSamples = nSamples; }
  void setNGates(int nGates) { _nGates = nGates; }

  //////////////////////// get methods /////////////////////////

  int getVersionNum() const { return _versionNum; }
  
  const string &getName() const { return _name; }
  const string &getScanModeStr() const { return _scanModeStr; }
  int getScanMode() const { return _scanMode; }

  time_t getStartUTime() const { return _startUTime; }
  int getStartNanoSecs() const { return _startNanoSecs; }

  double getStartTime() const {
    return ((double) _startUTime + _startNanoSecs / 1.0e9);
  }

  time_t getEndUTime() const { return _endUTime; }
  int getEndNanoSecs() const { return _endNanoSecs; }

  double getEndTime() const {
    return ((double) _endUTime + _endNanoSecs / 1.0e9);
  }

  double getStartEl() const { return _startEl; }
  double getStartAz() const { return _startAz; }

  double getEndEl() const { return _endEl; }
  double getEndAz() const { return _endAz; }

  double getFixedEl() const { return _fixedEl; }
  double getFixedAz() const { return _fixedAz; }

  bool getClockWise() const { return _isClockWise; }
  
  double getPrf() const { return _prf; }

  int getVolNum() const { return _volNum; }
  int getTiltNum() const { return _tiltNum; } // in a volume
  int getSweepNum() const { return _sweepNum; } // grows forever

  int getNSamples() const { return _nSamples; }
  int getNGates() const { return _nGates; }

  bool getAntennaTransition() const { return _antennaTransition; }
  
  //////////////////////////////////////////////
  // Convert scan mode to string and vice versa
  
  static string scanMode2Str(int scanMode);
  static int str2ScanMode(const string &modeStr);

  ///////////////////////////////////////////
  // assemble()
  // Load up the buffer from the object.
  // Handles byte swapping.
  
  void assemble();
  
  // get the assembled buffer info
  
  const void *getBufPtr() const { return _memBuf.getPtr(); }
  int getBufLen() const { return _memBuf.getLen(); }
  
  ///////////////////////////////////////////////////////////
  // disassemble()
  // Disassembles a buffer, sets the values in the object.
  // Handles byte swapping.
  // Returns 0 on success, -1 on failure
  
  int disassemble(const void *buf, int len);
  
  /////////////////////////
  // print
  
  void print(ostream &out, string spacer = "") const;
  
  // definition of struct for storing sweep

  static const int MISSING_VAL = -9999;

protected:

  int _versionNum;
  
  string _name;
  string _scanModeStr;
  int _scanMode;
  
  time_t _startUTime;
  int _startNanoSecs;

  time_t _endUTime;
  int _endNanoSecs;

  double _startEl;
  double _startAz;

  double _endEl;
  double _endAz;

  double _fixedEl; // PPI
  double _fixedAz; // RHI

  bool _isClockWise;

  double _prf;

  int _volNum;
  int _tiltNum; // in a volume
  int _sweepNum; // grows forever

  int _nSamples;
  int _nGates;

  bool _antennaTransition;
  
  // buffer for assemble / disassemble
  
  MemBuf _memBuf;

private:

};


#endif
