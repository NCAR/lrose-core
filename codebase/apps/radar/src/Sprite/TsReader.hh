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
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////

#ifndef TsReader_hh
#define TsReader_hh

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <toolsa/DateTime.hh>
#include <radar/IwrfTsInfo.hh>
#include <radar/IwrfTsPulse.hh>
#include <radar/IwrfTsGet.hh>
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
            const Args &args);
  
  // destructor
  
  ~TsReader();

  // constructor OK?

  bool OK;

  // set the number of samples

  void setNSamples(int val) { _nSamples = val; }

  //////////////////////////////////////////////////////
  // read the next beam
  
  // get the next beam in realtime or archive sequence
  // returns Beam object pointer on success, NULL on failure
  // caller must free beam
  
  Beam *getNextBeam();
  
  // get the previous beam in realtime or archive sequence
  // returns Beam object pointer on success, NULL on failure
  // caller must free beam
  
  Beam *getPreviousBeam();
  
  // get a beam from the getter, based on the time and
  // location from the display
  
  Beam *getBeamFollowDisplay(const DateTime &searchTime,
                             double searchEl, 
                             double searchAz);
  
  // position at end of queue
  
  void seekToEndOfQueue();

  // get the closest beam to the location specified
  // and within the specified time
  // returns Beam object pointer on success, NULL on failure
  // caller must free beam
  
  // Beam *getClosestBeam(time_t startTime, time_t endTime,
  //                      double az, double el, bool isRhi);

  //////////////////////////////////////////////////////
  // reading data in follow mode

  // get the path to best file

  string getFilePath() const { return _filePath; }
  
  // get scan mode

  string getScanModeStr() const { return _scanModeStr; }
  
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

  // Pulse readers
  
  IwrfTsGet *_pulseGetter;
  IwrfTsReader *_pulseReader;
  
  string _filePath;
  DateTime _archiveStartTime;
  DateTime _archiveEndTime;
  DateTime _beamTime;
  DateTime _prevBeamTime;
  double _beamIntervalSecs;
  int _timeSpanSecs;

  double _searchEl, _searchAz;

  // pulse queues
  
  deque<const IwrfTsPulse *> _getterQueue;
  deque<const IwrfTsPulse *> _readerQueue;
  int64_t _nPulsesRead;
  int64_t _prevPulseSeqNum;
  
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

  Beam *_getBeamViaGetter(const DateTime &searchTime,
                          double searchEl, double searchAz);
  Beam *_getBeamViaReader();
  int _positionReaderForPreviousBeam();
  
  void _addPulseToGetterQueue(const IwrfTsPulse *pulse);
  void _clearPulseGetterQueue();

  void _addPulseToReaderQueue(const IwrfTsPulse *pulse);
  void _clearPulseReaderQueue();

  void _checkIsAlternating();
  void _checkIsStaggeredPrt();
  double _conditionAz(double az);
  double _conditionAz(double az, double refAz);
  double _conditionEl(double el);
  

};

#endif

