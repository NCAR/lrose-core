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
// DsRadarPower.hh
//
// C++ class for dealing with radar power information
//
// Mike Dixon, RAP, NCAR
// POBox 3000, Boulder, CO, USA
//
// Sept 2007
//////////////////////////////////////////////////////////////

#ifndef _DsRadarPower_hh
#define _DsRadarPower_hh

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <toolsa/MemBuf.hh>
#include <dataport/port_types.h>

using namespace std;

#define ID_POWER_INFO 1999

// UDP packet for power information
// the following fields are sent in network byte order,
// most significant byte first, aka BigEndian

typedef struct
{
  ui32 id;
  ui32 length;
  ui32 utime_secs; // secs_since_1970;
  ui32 utime_nano_secs;  // since even second, if available
  fl32 xmit_power_h; // watts - power in the H waveguide
  fl32 xmit_power_v; // watts - power in the V waveguide
  ui32 spare0;
  ui32 spare1;
  ui32 spare2;
  ui32 spare3;
} power_info_packet_t;

class DsRadarPower {

public:

  // constructor

  DsRadarPower();

  // destructor

  ~DsRadarPower();

  //////////////////////// set methods /////////////////////////

  // clear all data members

  void clear();

  //////////////////////// set methods /////////////////////////
  
  void setTime(time_t utime, int nano_secs) {
    _utimeSecs = utime;
    _utimeNanoSecs = nano_secs;
  }
  
  void setXmitPowerH(double power) { _xmitPowerH = power; }
  void setXmitPowerV(double power) { _xmitPowerV = power; }

  //////////////////////// get methods /////////////////////////

  int getVersionNum() const { return _versionNum; }

  time_t getUtimeSecs() const { return _utimeSecs; }
  int getUtimeNanosecs() const { return _utimeNanoSecs; }

  double getXmitPowerH() const { return _xmitPowerH; }
  double getXmitPowerV() const { return _xmitPowerV; }

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
  
  // definition of struct for storing power

  static const int MISSING_VAL = -9999;
  
protected:

  int _versionNum;
  
  time_t _utimeSecs;   // secs_since_1970;
  int _utimeNanoSecs;  // since even second, if available

  double _xmitPowerH;
  double _xmitPowerV;

  // buffer for assemble / disassemble

  MemBuf _memBuf;

private:

};


#endif
