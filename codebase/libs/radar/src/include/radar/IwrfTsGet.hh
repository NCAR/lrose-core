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
// IwrfTsGet
//
// Retrieves time series data from a file archive.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2021
//
///////////////////////////////////////////////////////////////

#ifndef IwrfTsGet_hh
#define IwrfTsGet_hh

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsBurst.hh>
using namespace std;

////////////////////////
// Base class

class IwrfTsGet {
  
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
    
    // set methods
    
    void setPulse(IwrfTsPulse *val) { _pulse = val; }
    void setBurst(IwrfTsBurst *val) { _burst = val; }

    void setTime(const DateTime &val) { _time = val; }
    void setScanRate(double val) { _scanRate = val; }
    
    void setXmitRcvMode(iwrf_xmit_rcv_mode_t val) { _xmitRcvMode = val; }
    void setXmitPhaseMode(iwrf_xmit_phase_mode_t val) { _xmitPhaseMode = val; }
    void setPrfMode(iwrf_prf_mode_t val) { _prfMode = val; }
    void setPolMode(iwrf_pol_mode_t val) { _polMode = val; }

    void setFilePath(const string &val) { _filePath = val; }

    // get methods
    
    IwrfTsPulse *getPulse() { return _pulse; }
    IwrfTsBurst *getBurst() { return _burst; }

    const DateTime &getTime() const { return _time; }
    double getEl() const { return _el; }
    double getAz() const { return _az; }
    
    double getScanRate() const { return _scanRate; }
    
    iwrf_xmit_rcv_mode_t getXmitRcvMode() const { return _xmitRcvMode; }
    iwrf_xmit_phase_mode_t getXmitPhaseMode() const { return _xmitPhaseMode; }
    iwrf_prf_mode_t getPrfMode() const { return _prfMode; }
    iwrf_pol_mode_t getPolMode() const { return _polMode; }

    const string &getFilePath() const { return _filePath; }
    
  private:
    
    IwrfTsPulse *_pulse;
    IwrfTsBurst *_burst;

    DateTime _time;
    double _el, _az;

    double _scanRate;

    iwrf_xmit_rcv_mode_t _xmitRcvMode;
    iwrf_xmit_phase_mode_t _xmitPhaseMode;
    iwrf_prf_mode_t _prfMode;
    iwrf_pol_mode_t _polMode;

    string _filePath;
    
  }; // class PulseEntry

  // constructor
  
  IwrfTsGet(IwrfDebug_t debug = IWRF_DEBUG_OFF);
  
  // destructor
  
  virtual ~IwrfTsGet();

  // debugging

  void setDebug(IwrfDebug_t debug) { _debug = debug; }

  // set top level directory for data files
  // actual files will be 1 level down in yyyymmdd day dirs
  
  void setTopDir(const string &topDir) { _topDir = topDir; }

  // set the time margin, which is applied
  // to time range in secs for search.
  // We ensure that we have at
  // least this margin on either side of the
  // search time so that a beam can be suitably
  // constructed from the stored pulses
  
  void setTimeMarginSecs(double val) { _timeMarginSecs = val; }
  
  // Retrieve a beam of pulses, given the time, el and az.
  // Fills the beamPulses vector.
  // Returns 0 on success, -1 on failure.
  // The beamPulses vector points to pulses managed by this object.
  // Data in the beam must be used before any further get operations
  // are performed on this object.
  
  int retrieveBeam(const DateTime &searchTime,
                   double searchElev,
                   double searchAz,
                   int nSamples,
                   vector<IwrfTsGet::PulseEntry *> &beamPulses);
  
  // get methods, use after retrieve

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

  const DateTime &getPulsesStartTime() const { return _pulsesStartTime; }
  const DateTime &getPulsesEndTime() const { return _pulsesEndTime; }

  const DateTime &getFilesStartTime() const { return _filesStartTime; }
  const DateTime &getFilesEndTime() const { return _filesEndTime; }

  bool getIsAlternating() const { return _isAlternating; }
  bool getIsStaggeredPrt() const { return _isStaggeredPrt; }

protected:
  
  ////////////////////////////////////////////////
  // protected members of IwrfTsGet
  
  IwrfDebug_t _debug;

  // top level directory for the data files
  // the actual files will be in day dirs below this level

  string _topDir;

  // margin applied to time range in secs
  // for search. We ensure that we have at
  // least this margin on either side of the
  // search time so that a beam can be suitably
  // constructed from the stored pulses
  
  double _timeMarginSecs;
  
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

  // files

  map<time_t, string> _fileListMap;
  vector<time_t> _fileListStartTimes;
  vector<time_t> _fileListEndTimes;
  vector<string> _fileListPaths;
  DateTime _filesStartTime, _filesEndTime;

  // pulses

  vector<PulseEntry *> _pulseEntries;
  DateTime _pulsesStartTime;
  DateTime _pulsesEndTime;

  // pulse-to-pulse HV alternating mode

  bool _isAlternating;

  // staggered PRT

  bool _isStaggeredPrt;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;
  double _prt;
  int _nGates;
  
  // methods

  int _loadFileList(const DateTime &searchTime);
  void _clearFileList();

  int _loadPulseList(const DateTime &searchTime);
  void _clearPulseList();
  
  int _loadBeamPulses(const DateTime &searchTime,
                      double searchElev,
                      double searchAz,
                      int nSamples,
                      vector<IwrfTsGet::PulseEntry *> &beamPulses);

  bool _checkIsBeamPpi(ssize_t midIndex, double az, double indexedRes);
  bool _checkIsBeamRhi(ssize_t midIndex, double el, double indexedRes);

  double _conditionAz(double az);
  double _conditionAz(double az, double refAz);
  double _conditionEl(double el);
  double _conditionDeltaAz(double deltaAz);
  ssize_t _conditionPulseIndex(ssize_t index);

  void _checkIsAlternating(ssize_t index1, ssize_t index2);
  void _checkIsStaggeredPrt(ssize_t index1, ssize_t index2);

private:
  
};

#endif

