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
// IwrfTsArchive
//
// Retrieves time series data from a file archive.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsArchive_hh
#define IwrfTsArchive_hh

#include <string>
#include <toolsa/pmu.h>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>
using namespace std;

////////////////////////
// Base class

class IwrfTsArchive {
  
public:

  // constructor
  
  IwrfTsArchive(IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsArchive();

  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }
  
  // reset the queue to the beginning

  virtual void resetToStart();

  // reset the queue at the end
  
  virtual void resetToEnd();
  
  // get the current file path in use

  virtual const string getPathInUse() const { return _pathInUse; }
  
  // get the previous file path in use
  
  virtual const string getPrevPathInUse() const { return _prevPathInUse; }
  
protected:
  
  /////////////////////////////////////////////////////////
  // inner class for pulse and info entries

  class PulseEntry
  {  
    
  public:   
    
    // constructor
    
    PulseEntry(IwrfTsPulse *pulse);
    
    // destructor
    
    virtual ~PulseEntry();
    
    // get methods
    
    IwrfTsPulse *getPulse() { return _pulse; }
    
  private:

    IwrfTsPulse *_pulse;
    IwrfTsBurst *_burst;

    double _scanRate;

    iwrf_xmit_rcv_mode_t _xmitRcvMode;
    iwrf_xmit_phase_mode_t _xmitPhaseMode;
    iwrf_prf_mode_t _prfMode;
    iwrf_pol_mode_t _polMode;
  
    
  }; // class PulseEntry

  ////////////////////////////////////////////////
  // protected members of IwrfTsArchive
  
  IwrfDebug_t _debug;

  // paths in the read
  
  string _pathInUse;
  string _prevPathInUse;

  // radar info
  
  double _latitudeDeg;
  double _longitudeDeg;
  double _altitudeM;
  iwrf_radar_platform_t _platformType;

  double _beamwidthDegH;
  double _beamwidthDegV;
  double _wavelengthCm;
  
  double _nominalAntGainDbH;
  double _nominalAntGainDbV;

  string _radarName;
  string _siteName;

  // calibration

  IwrfCalib _calib;

private:
  
};

#endif

