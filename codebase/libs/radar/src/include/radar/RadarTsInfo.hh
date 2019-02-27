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
// RadarTsInfo.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2007
//
///////////////////////////////////////////////////////////////
//
// Stores current radar ops info
//
////////////////////////////////////////////////////////////////
//
// Class has been deprecated.
// Use IwrfTsInfo instead.
//
///////////////////////////////////////////////////////////////

#ifndef _RadarTsInfo_hh
#define _RadarTsInfo_hh

#include <string>
#include <dataport/port_types.h>
#include <rapformats/ds_radar_ts.h>
using namespace std;

typedef enum {
  RTS_DEBUG_OFF = 0,
  RTS_DEBUG_NORM,
  RTS_DEBUG_VERBOSE
} RadarTsDebug_t;

////////////////////////
// This class

class RadarTsInfo {
  
public:

  // constructor
  
  RadarTsInfo(RadarTsDebug_t debug = RTS_DEBUG_OFF);

  // destructor
  
  ~RadarTsInfo();

  // debugging

  void setDebug(RadarTsDebug_t debug) { _debug = debug; }

  // set from a TS buffer
  
  int setFromTsBuffer(const void *buf, int len);

  // load value from TS ops info struct
  
  void load(const ts_ops_info_t &info);
    
  // set RVP8-specific fields
  
  void setRvp8Info(const ts_ops_info_t &info,
		   const ts_pulse_hdr_t &pulse);
  
  // find search string in data
  // Returns 0 on succes, -1 on failure (EOF)
  
  static int findNextStr(FILE *in, const char *searchStr);
  
  // read in info from RVP8 file
  // If this is run, the derivedFromRvp8 flag will be set
  
  int readFromRvp8File(FILE *in);
  bool isDerivedFromRvp8() { return _derivedFromRvp8; }
  
  // clear values

  void clear();

  // override radar name and/or site name
  
  void overrideRadarName(const string &radarName);
  void overrideSiteName(const string &siteName);
  
  // override radar location
  
  void overrideRadarLocation(double altitudeMeters,
                             double latitudeDeg,
                             double longitudeDeg);
  // override gate geometry
  
  void overrideGateGeometry(double startRangeMeters,
                            double gateSpacingMeters);
  // override wavelength

  void overrideWavelength(double wavelengthCm);
  
  // printing
  
  void print(ostream &out) const;

  // write in tsarchive format
  
  int write2TsarchiveFile(FILE *out);

  // get methods

  const ts_ops_info_t &getInfo() const { return _info; }

  double getAltitudeM() const { return _info.altitudeM; }
  double getLatitudeDeg() const { return _info.latitudeDeg; }
  double getLongitudeDeg() const { return _info.longitudeDeg; }

  double getStartRangeM() const { return _info.startRangeM; }
  double getGateSpacingM() const { return _info.gateSpacingM; }
  double getStartRangeKm() const { return _info.startRangeM / 1000.0; }
  double getGateSpacingKm() const { return _info.gateSpacingM / 1000.0; }

  ts_scan_mode_t getScanMode() const {
    return ( ts_scan_mode_t) _info.scanMode;
  }
  ts_xmit_rcv_mode_t getXmitRcvMode() const { 
    return (ts_xmit_rcv_mode_t) _info.xmitRcvMode; 
  }
  ts_prf_mode_t getPrfMode() const {
    return (ts_prf_mode_t) _info.prfMode;
  }
  ts_xmit_phase_mode_t getXmitPhaseMode() const {
    return (ts_xmit_phase_mode_t) _info.xmitPhaseMode;
  }

  double getWavelengthCm() const { return _info.wavelengthCm; }
  double getBeamWidthDegH() const { return _info.beamWidthDegH; }
  double getBeamWidthDegV() const { return _info.beamWidthDegV; }

  const ds_radar_calib_t &getCalib() const { return _info.calib; }

  const string getSiteName() const { return _info.siteName; }
  const string getRadarName() const { return _info.radarName; }
  
  double getRvp8ClockMhz() const { return _info.rvp8.fSyClkMHz; }
  double getRvp8SaturationDbm() const { return _info.rvp8.fSaturationDBM; }
  double getRvp8PulseWidthUs() const { return _info.rvp8.fPWidthUSec; }

  double getRvp8DbzCalib() const { return _info.rvp8.fDBzCalib; }
  double getRvp8NoiseDbmChan0() const { return _info.rvp8.fNoiseDBm[0]; }
  double getRvp8NoiseDbmChan1() const { return _info.rvp8.fNoiseDBm[1]; }

protected:
  
private:

  RadarTsDebug_t _debug;

  // most data is in ops info struct

  ts_ops_info_t _info;

  // flag to indicate this info was derived from
  // an RVP8 file

  bool _derivedFromRvp8;

  // functions
  
  int _readRvp8Info(FILE *in);
  void _deriveRangeFromRvp8Info();

};

#endif

