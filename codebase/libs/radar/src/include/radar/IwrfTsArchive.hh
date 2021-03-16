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
#include <deque>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>
using namespace std;

////////////////////////
// Base class

class IwrfTsArchive {
  
public:

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
    IwrfTsBurst *getBurst() { return _burst; }

    double getScanRate() const { return _scanRate; }
    
    iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }
    iwrf_xmit_phase_mode_t getXmitPhaseMode() const { return _xmitPhaseMode; }
    iwrf_prf_mode_t getPrfMode() const { return _prfMode; }
    iwrf_pol_mode_t getPolMode() const { return _polMode; }
    
  private:

    IwrfTsPulse *_pulse;
    IwrfTsBurst *_burst;

    double _scanRate;

    iwrf_xmit_rcv_mode_t _xmitRcvMode;
    iwrf_xmit_phase_mode_t _xmitPhaseMode;
    iwrf_prf_mode_t _prfMode;
    iwrf_pol_mode_t _polMode;
    
  }; // class PulseEntry

  // constructor
  
  IwrfTsArchive(IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsArchive();

  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }

  // set top level directory for data files
  // actual files will be 1 level down in yyyymmdd day dirs
  
  void setTopDir(const string &topDir) { _topDir = topDir; }

  // reset the queue to the beginning

  virtual void resetToStart();

  // reset the queue at the end
  
  virtual void resetToEnd();
  
  // get methods

  const string getPathInUse() const { return _pathInUse; }
  const string getPrevPathInUse() const { return _prevPathInUse; }
  
  double getLatitudeDeg() const { return _latitudeDeg; }
  double getLongitudeDeg() const { return _longitudeDeg; }
  double getAltitudeM() const { return _altitudeM; }

  iwrf_radar_platform_t getRadarPlatform() const { return _platformType; }

  double getBeamWidthDegH() const { return _beamwidthDegH; }
  double getBeamWidthDegV() const { return _beamwidthDegV; }
  double getWavelengthCm() const { return _wavelengthCm; }
  
  double getNominalAntGainDbH() const { return _nominalAntGainDbH; }
  double getNominalAntGainDbV() const { return _nominalAntGainDbV; }
  
  string getRadarName() const { return _radarName; }
  string getSiteName() const { return _siteName; }
  
  const IwrfCalib &getCalib() const { return _calib; }

  const DateTime &getStartTime() const { return _startTime; }
  const DateTime &getEndTime() const { return _endTime; }

  // get a beam of pulses, given the time, el and az
  // returns empty vector on failure
  // data in the beam must be used before any other operations
  // are performed on the archive
  
  vector<PulseEntry *> getBeam(DateTime &searchTime,
                               double searchElev, double searchAz);
  
protected:
  
  ////////////////////////////////////////////////
  // protected members of IwrfTsArchive
  
  IwrfDebug_t _debug;

  // top level directory for the data files
  // the actual files will be in day dirs below this level

  string _topDir;

  // start and end times for the pulses in the queue

  DateTime _startTime;
  DateTime _endTime;

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

  // pulses

  deque<PulseEntry *> _pulseEntries;

private:
  
};

#endif

