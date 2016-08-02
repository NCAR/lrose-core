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
// TsReader.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2008
//
///////////////////////////////////////////////////////////////

#ifndef TsReader_hh
#define TsReader_hh

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsReader.hh>
#include "Params.hh"
#include "Args.hh"
#include "Beam.hh"
using namespace std;

////////////////////////
// This class

class TsReader {
  
public:
  
  // inner class for storing paths and their times

  class TimePath {
  public:
    time_t validTime;
    time_t startTime;
    time_t endTime;
    string fileName;
    string filePath;
    double fixedAngle;
    TimePath(time_t valid_time,
             time_t start_time,
             time_t end_time,
             const string &name,
             const string &path);
  };
  
  class TimePathCompare {
  public:
    bool operator()(const TimePath &a, const TimePath &b) const {
      return a.validTime < b.validTime;
    }
  };

  typedef set<TimePath, TimePathCompare > TimePathSet;

  // scan mode for determining PPI vs RHI operations
  
  typedef enum {
    SCAN_TYPE_UNKNOWN,
    SCAN_TYPE_PPI,
    SCAN_TYPE_RHI
  } scan_type_t;
  
  // constructor
  
  TsReader (const string &prog_name,
            const Params &params,
            const Args &args,
            bool isRhi,
            bool indexedBeams,
            bool indexedRes);
  
  // destructor
  
  ~TsReader();

  // constructor OK?

  bool constructorOK;

  // find the best file to read

  int findBestFile(time_t startTime, time_t endTime,
                   double az, double el, bool isRhi);

  // get the path to best file

  string getFilePath() const { return _filePath; }
  
  // get scan mode

  string getScanModeStr() const { return _scanModeStr; }
  
  // read all pulses into queue
  
  int readAllPulses();
  
  // get a beam
  // returns Beam object pointer on success, NULL on failure
  // caller must free beam
  
  Beam *getBeam(double az, double el);
    
  // get ops info

  const IwrfTsInfo &getOpsInfo() const { return _pulseReader->getOpsInfo(); }
  bool isOpsInfoNew() const { return _pulseReader->isOpsInfoNew(); }
  scan_type_t getScanType() const { return _scanType; }

protected:
  
private:

  string _progName;
  const Params &_params;
  const Args &_args;
  bool _isRhi;
  string _scanModeStr;
  IwrfDebug_t _iwrfDebug;

  // Pulse reader
  
  IwrfTsReader *_pulseReader;
  string _filePath;

  // pulse queue
  
  deque<const IwrfTsPulse *> _pulseQueue;
  long _pulseSeqNum;
  
  // number of gates

  int _nGates;

  // number of samples
  
  int _nSamples;

  // indexing

  bool _indexedBeams;
  double _indexedRes;

  // beam time and location

  scan_type_t _scanType;
  time_t _time;
  double _az;
  double _el;
  double _prt;

  // pulse-to-pulse HV alternating mode

  bool _isAlternating;

  // staggered PRT

  bool _isStaggeredPrt;
  double _prtShort;
  double _prtLong;
  int _nGatesPrtShort;
  int _nGatesPrtLong;

  // private functions

  void _clear();
  void _clearPulseQueue();

  Beam *_getBeamPpi();
  Beam *_getBeamRhi();

  bool _checkIsBeamPpi(size_t midIndex);
  bool _checkIsBeamRhi(size_t midIndex);
  
  Beam *_makeBeam(size_t midIndex);
  
  bool _beamReady();
  bool _beamReadyPpi();
  bool _beamReadyRhi();
  void _checkIsAlternating();
  void _checkIsStaggeredPrt();
  double _conditionAz(double az);
  double _conditionAz(double az, double refAz);
  double _conditionEl(double el);
  void _addPulseToQueue(const IwrfTsPulse *pulse);
  void _getDayDirs(const string &topDir, TimePathSet &dayDirs);
  

};

#endif

