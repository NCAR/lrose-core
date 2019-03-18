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
///////////////////////////////////////////////////////////////
// TsReader.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2019
//
///////////////////////////////////////////////////////////////
//
// Reads in pulses, creates beams.
//
////////////////////////////////////////////////////////////////

#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/ReadDir.hh>
#include <toolsa/Path.hh>
#include "TsReader.hh"
using namespace std;

/////////////////////////
// TimePath constructor

TsReader::TimePath::TimePath(time_t valid_time,
                             time_t start_time,
                             time_t end_time,
                             const string &name,
                             const string &path) :
        validTime(valid_time), 
        startTime(start_time), 
        endTime(end_time), 
        fileName(name),
        filePath(path)
  
{

  // scan name for fixed angle

  fixedAngle = 0.0;
  int iang;
  int year, month, day, hour, min, sec;
  
  if (sscanf(fileName.c_str(),
             "%4d%2d%2d_%2d%2d%2d_%d",
             &year, &month, &day, 
             &hour, &min, &sec, 
             &iang) == 7) {
    fixedAngle = iang / 10.0;
  }

}

////////////////////////////////////////////////////
// TsReader Constructor

TsReader::TsReader(const string &prog_name,
                   const Params &params,
                   const Args &args) :
        _progName(prog_name),
        _params(params),
        _args(args)
  
{

  OK = true;

  _pulseReader = NULL;
  _scanType = SCAN_TYPE_UNKNOWN;
  
  _pulseSeqNum = 0;
  _nPulsesRead = 0;
  
  _nSamples = _params.n_samples;
  
  _isAlternating = false;
  _isStaggeredPrt = false;
  
  _time = 0;
  _az = 0.0;
  _el = 0.0;

  _nGates = 0;

  _prt = 0.0;
  _prtShort = 0.0;
  _prtLong = 0.0;

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _iwrfDebug = IWRF_DEBUG_VERBOSE;
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _iwrfDebug = IWRF_DEBUG_NORM;
  } else {
    _iwrfDebug = IWRF_DEBUG_OFF;
  }

  _isRhi = false;
  _scanModeStr = "PPI";
  _scanType = SCAN_TYPE_PPI;
  _indexedRes = _params.indexed_resolution_ppi;

  switch (_params.input_mode) {
    case Params::REALTIME_FMQ_MODE:
    default: {
      _pulseReader = new IwrfTsReaderFmq(_params.input_fmq_url,
                                         _iwrfDebug,
                                         _params.seek_to_start_of_fmq);
      break;
    }
    case Params::REALTIME_TCP_MODE: {
      _pulseReader = new IwrfTsReaderTcp(_params.input_tcp_address,
                                         _params.input_tcp_port,
                                         _iwrfDebug);
      break;
    }
    case Params::ARCHIVE_TIME_MODE: {
      _timeSpanSecs = _params.ascope_time_span_secs;
      _archiveStartTime.set(_params.archive_start_time);
      _archiveEndTime = _archiveStartTime + _timeSpanSecs;
      DsInputPath input(_progName,
                        _params.debug >= Params::DEBUG_VERBOSE,
                        _params.archive_data_dir,
                        _archiveStartTime.utime(),
                        _archiveEndTime.utime());
      vector<string> paths = input.getPathList();
      if (paths.size() < 1) {
        cerr << "ERROR: " << _progName << " - ARCHIVE mode" << endl;
        cerr << "  No paths found, dir: " << _params.archive_data_dir << endl;
        cerr << "  Start time: " << DateTime::strm(_archiveStartTime.utime()) << endl;
        cerr << "  End time: " << DateTime::strm(_archiveEndTime.utime()) << endl;
        OK = false;
      }
      _pulseReader = new IwrfTsReaderFile(paths, _iwrfDebug);
      break;
    }
    case Params::FILE_LIST_MODE: {
      _pulseReader = new IwrfTsReaderFile(_args.inputFileList,
                                          _iwrfDebug);
      break;
    }
    case Params::FOLLOW_MOMENTS_MODE: {
      // leave pulse reader NULL for now
      break;
    }

  } // switch

}

//////////////////////////////////////////////////////////////////
// destructor

TsReader::~TsReader()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  _clearPulseQueue();

}

/////////////////////////////////////////////////////////
// get the next beam in realtime or archive sequence
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getNextBeam()
  
{
  
  _clearPulseQueue();
  
  // read in pulses, load up pulse queue
  
  while (true) {
    
    PMU_auto_register("getNextBeam");
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      cerr << "ERROR - TsReader::getNextBeam()" << endl;
      cerr << "  end of data" << endl;
      _clearPulseQueue();
      return NULL;
    }
    _nPulsesRead++;

    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
    }
    
    if (!_params.prt_is_for_previous_interval) {
      pulse->swapPrtValues();
    }
    
    // check scan mode and type
    
    int scanMode = pulse->getScanMode();
    if (scanMode == IWRF_SCAN_MODE_RHI) {
      if (!_isRhi) {
        // change in scan mode, clear pulse queue
        _clearPulseQueue();
      }
      _isRhi = true;
      _scanModeStr = "RHI";
      _scanType = SCAN_TYPE_RHI;
      _indexedRes = _params.indexed_resolution_rhi;
    } else {
      if (_isRhi) {
        // change in scan mode, clear pulse queue
        _clearPulseQueue();
      }
      _isRhi = false;
      _scanModeStr = "PPI";
      _scanType = SCAN_TYPE_PPI;
      _indexedRes = _params.indexed_resolution_ppi;
    }

    // check for missing pulses
    if (_pulseQueue.size() == 0) {
      _prevPulseSeqNum = pulse->get_pulse_seq_num();
    } else {
      if (pulse->get_pulse_seq_num() != _prevPulseSeqNum + 1) {
        cerr << "ERROR - TsReader::getNextBeam()" << endl;
        cerr << "  missing pulses, clearing queue" << endl;
        _clearPulseQueue();
      }
      _prevPulseSeqNum = pulse->get_pulse_seq_num();
    }
    
    // Create a new pulse object and save a pointer to it in the
    // _pulseQueue.  _pulseQueue is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    // add pulse to queue, managing memory appropriately
    
    _addPulseToQueue(pulse);

    if ((int) _pulseQueue.size() == (_nSamples + 4)) {
      // got the pulses we need
      break;
    }
    
  } // while

  // set prt and nGates, assuming single PRT for now
  
  _prt = _pulseQueue[0]->getPrt();
  _nGates = _pulseQueue[0]->getNGates();
  
  // check if we have alternating h/v pulses
  
  _checkIsAlternating();

  // check if we have staggered PRT pulses - does not apply
  // to alternating mode

  if (_isAlternating) {
    _isStaggeredPrt = false;
  } else {
    _checkIsStaggeredPrt();
  }

  // in non-staggered mode check we have constant nGates
  
  if (!_isStaggeredPrt) {
    int nGates0 = _pulseQueue[0]->getNGates();
    for (int i = 1; i < (int) _pulseQueue.size(); i += 2) {
      if (_pulseQueue[i]->getNGates() != nGates0) {
        cerr << "ERROR - TsReader::getNextBeam()" << endl;
        cerr << "  nGates varies in non-staggered mode" << endl;
        return NULL;
      }
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    if (_isAlternating) {
      cerr << "==>> Fast alternating mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    } else if (_isStaggeredPrt) {
      cerr << "==>> Staggered PRT mode" << endl;
      cerr << "     prt short: " << _prtShort << endl;
      cerr << "     prt long: " << _prtLong << endl;
      cerr << "     ngates short PRT: " << _nGatesPrtShort << endl;
      cerr << "     ngates long PRT: " << _nGatesPrtLong << endl;
    } else {
      cerr << "==>> Single PRT mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    }
  }

  _filePath = _pulseReader->getPathInUse();

  size_t midIndex = _pulseQueue.size() / 2;
  _az = _conditionAz(_pulseQueue[midIndex]->getAz());
  _el = _conditionEl(_pulseQueue[midIndex]->getEl());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "got beam, _nPulsesRead: " << _nPulsesRead << endl;
  }

  return _makeBeam(midIndex);

}

/////////////////////////////////////////////////////////
// get the previous beam in realtime or archive sequence
// we need to reset the queue and position to read the 
// previous beam
//
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getPreviousBeam()
  
{
  
  // first position for previous beam

  if (positionForPreviousBeam()) {
    cerr << "ERROR - TsReader::getPreviousBeam()" << endl;
    return NULL;
  }

  // then read the next beam

  return getNextBeam();

}

////////////////////////////////////////////////////////////////////
// position to get the previous beam in realtime or archive sequence
// we need to reset the queue and position to read the previous beam
// returns 0 on success, -1 on error

int TsReader::positionForPreviousBeam()
  
{

  // save the location to which we want to return

  int64_t seekLoc = _nPulsesRead - (_nSamples + 4) * 2;
  if (seekLoc < 0) {
    seekLoc = 0;
  }

  // reset the pulse queue

  _pulseReader->reset();

  // read pulses to move to seek location

  _nPulsesRead = 0;
  while (_nPulsesRead < seekLoc) {
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    if (pulse == NULL) {
      cerr << "ERROR - TsReader::positionForPreviousBeam()" << endl;
      cerr << "  Cannot reposition to seekLoc: " << seekLoc << endl;
      _clearPulseQueue();
      return -1;
    }
    _nPulsesRead++;
    delete pulse;
  }

  return 0;

}

////////////////////////////////////////////////////////////////////
// position at end of queue

void TsReader::seekToEndOfQueue()
  
{
  _pulseReader->seekToEnd();
}

/////////////////////////////////////////////////////////
// get the closest beam to the location specified
// and within the specified time
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getClosestBeam(time_t startTime, time_t endTime,
                               double az, double el, bool isRhi)
  
{

  if (_params.debug) {
    cerr << "TsReader - finding closest beam" << endl;
    cerr << "  Data start time: " << DateTime::strm(startTime) << endl;
    cerr << "  Data end time: " << DateTime::strm(endTime) << endl;
    cerr << "  Az, el: " << az << ", " << el << endl;
    cerr << "  isRhi: " << isRhi << endl;
  }

  // find the best file for the requested time and location

  if (_findBestFile(startTime, endTime, az, el, isRhi)) {
    cerr << "ERROR - TsReader::getClosestBeam()" << endl;
    cerr << "  no suitable data found" << endl;
    _clearPulseQueue();
    return NULL;
  }
  
  // clear the queue, delete the pulse reader

  if (_pulseReader) {
    delete _pulseReader;
  }
  _clearPulseQueue();

  // create new pulse reader for this file

  vector<string> fileList;
  fileList.push_back(_filePath);
  _pulseReader = new IwrfTsReaderFile(fileList, _iwrfDebug);

  // read all pulses in File
  
  if (_readFile()) {
    cerr << "ERROR - TsReader::getClosestBeam()" << endl;
    cerr << "  cannot read file: " << _filePath << endl;
    _clearPulseQueue();
    return NULL;
  }

  // check we have enough pulses

  if ((int) _pulseQueue.size() < (_nSamples + 4)) {
    // too few pulses
    cerr << "ERROR - TsReader::getClosestBeam()" << endl;
    cerr << "  too few pulses, file: " << _filePath << endl;
    _clearPulseQueue();
    return NULL;
  }

  // for indexing, modify the angles according to scan mode

  if (_indexedBeams) {
    if (_isRhi) {
      _el = (int) floor(((el / _indexedRes) + 0.5) * _indexedRes);
    } else {
      _az = (int) floor(((az / _indexedRes) + 0.5) * _indexedRes);
    }
  } else {
    _az = az;
    _el = el;
  }

  _az = _conditionAz(_az);
  _el = _conditionEl(_el);

  if (_isRhi) {
    return _getBeamRhi();
  } else {
    return _getBeamPpi();
  }

}

//////////////////////////////////////////////////
// get a beam in PPI
// returns Beam object pointer on success, NULL on failure

Beam *TsReader::_getBeamPpi()
  
{

  for (size_t ii = 0; ii < _pulseQueue.size() - 1; ii++) {
    if (_checkIsBeamPpi(ii)) {
      Beam *beam = _makeBeam(ii);
      return beam;
    }
  }

  return NULL;

}

//////////////////////////////////////////////////
// get a beam in RHI
// returns Beam object pointer on success, NULL on failure

Beam *TsReader::_getBeamRhi()
  
{
  
  for (size_t ii = 0; ii < _pulseQueue.size() - 1; ii++) {
    if (_checkIsBeamRhi(ii)) {
      Beam *beam = _makeBeam(ii);
      return beam;
    }
  }

  return NULL;

}

/////////////////////////////////////////////////
// check for a beam at the stated index

bool TsReader::_checkIsBeamPpi(size_t midIndex)
  
{
  
  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _conditionAz(_pulseQueue[midIndex]->getAz(), _az);
  double midAz2 = _conditionAz(_pulseQueue[midIndex+1]->getAz(), midAz1);
  
  cerr << "_az, midAz1, midAz2: " << _az << ", " << midAz1 << ", " << midAz2 << endl;

  // Check if the azimuths at the center of the data straddle
  // the target azimuth
  
  if (midAz1 <= _az && midAz2 >= _az) {

    // az1 is below and az2 above - clockwise rotation
    return true;
    
  } else if (midAz1 >= _az && midAz2 <= _az) {
    
    // az1 is above and az2 below - counterclockwise rotation
    return true;
    
  } else if (_az == 0.0) {
    
    if (midAz1 > 360.0 - _indexedRes && midAz2 < _indexedRes) {
      
      // az1 is below 0 and az2 above 0 - clockwise rotation
      return true;
	
    } else if (midAz2 > 360.0 - _indexedRes && midAz1 < _indexedRes) {
      
      // az1 is above 0 and az2 below 0 - counterclockwise rotation
      return true;
      
    }

  }
  
  return false;

}

/////////////////////////////////////////////////
// check for a beam at the stated index

bool TsReader::_checkIsBeamRhi(size_t midIndex)
  
{
  
  // compute elevations which need to straddle the center of the beam
  
  double midEl1 = _conditionEl(_pulseQueue[midIndex]->getEl());
  double midEl2 = _conditionEl(_pulseQueue[midIndex+1]->getEl());
  
  // Check if the elevations at the center of the data straddle
  // the target elevation
  
  if (midEl1 <= _el && midEl2 >= _el) {

    // el1 is below and el2 above - el increasing
    return true;
      
  } else if (midEl1 >= _el && midEl2 <= _el) {
      
    // el1 is above and el2 below - el decreasing
    return true;

  }

  return false;

}

//////////////////////////////////////////////////
// make a beam at the specified index
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::_makeBeam(size_t midIndex)
  
{

  // set index limits
  
  int startIndex = midIndex - _nSamples / 2;
  if (startIndex < 0) {
    startIndex = 0;
  }

  int endIndex = startIndex + _nSamples - 1;
  int qSize = (int) _pulseQueue.size();
  if (endIndex > qSize - 4) {
    endIndex = qSize - 4;
    startIndex = endIndex - _nSamples + 1;
  }

  // adjust to make sure we start on correct pulse
  
  if (_isAlternating) {
    bool startsOnHoriz = _pulseQueue[startIndex]->isHoriz();
    if (!startsOnHoriz) {
      startIndex++;
      endIndex++;
    }
  } else if (_isStaggeredPrt) {
    double prt0 = _pulseQueue[startIndex]->getPrt();
    double prt1 = _pulseQueue[startIndex+1]->getPrt();
    if (prt0 > prt1) {
      startIndex++;
      endIndex++;
    }
  }

  // load the pulse pointers into a deque for
  // just this beam
  
  deque<const IwrfTsPulse *> beamPulses;
  for (int ii = startIndex; ii <= endIndex; ii++) {
    beamPulses.push_back(_pulseQueue[ii]);
  }

  // create new beam
  
  Beam *beam = new Beam(_progName, _params);
  beam->init(_scanType == SCAN_TYPE_RHI,
             _nSamples,
             _nGates,
             _nGatesPrtLong,
             _indexedBeams,
             _indexedRes,
             _isAlternating,
             _isStaggeredPrt,
             _prt,
             _prtLong,
             _pulseReader->getOpsInfo(),
             beamPulses);
  
  return beam;
  
}

/////////////////////////////////////////////////
// add the pulse to the pulse queue
    
void TsReader::_addPulseToQueue(const IwrfTsPulse *pulse)
  
{

  // push pulse onto back of queue
  
  pulse->addClient();
  _pulseQueue.push_back(pulse);

  // print missing pulses in verbose mode
  
  if ((int) pulse->getSeqNum() != (int) (_pulseSeqNum + 1)) {
    if (_params.debug >= Params::DEBUG_VERBOSE && _pulseSeqNum != 0) {
      cerr << "******** Missing seq num: Expected=" << _pulseSeqNum+1
	   << " Rx'd=" <<  pulse->getSeqNum() << " ********" << endl;
    }
  }

  _pulseSeqNum = pulse->getSeqNum();

}

//////////////////////////////////////////////////////////////////
// clear data

void TsReader::_clearPulseQueue()

{

  for (size_t ii = 0; ii < _pulseQueue.size(); ii++) {
    // only delete the pulse object if it is no longer being used
    if (_pulseQueue[ii]->removeClient() == 0) {
      delete _pulseQueue[ii];
    }
  } // ii
  _pulseQueue.clear();

}

//////////////////////////////////////////////////////
// find the files for the requested time and location

int TsReader::_findBestFile(time_t startTime, time_t endTime,
                            double az, double el, bool isRhi)
  
{

  double fixedAngle = el;
  if (isRhi) {
    fixedAngle = az;
  }

  TimePathSet dayDirs;
  _getDayDirs(_params.archive_data_dir, dayDirs);
  
  // check for suitable days
  
  TimePathSet filePaths;
  TimePathSet::iterator ii;
  for (ii = dayDirs.begin(); ii != dayDirs.end(); ii++) {
    
    if (startTime > ii->endTime || endTime < ii->startTime) {
      // no overlap in time
      continue;
    }
    
    const string &dayDir = ii->filePath;
    
    ReadDir rdir;
    if (rdir.open(dayDir.c_str())) {
      return -1;
    }
      
    // Loop thru directory looking for the data file names

    struct dirent *dp;
    for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
      
      // exclude dir entries beginning with '.'
      
      if (dp->d_name[0] == '.') {
	continue;
      }

      // ignore transitional files

      if (strstr(dp->d_name, "_trans.iwrf_ts") != NULL) {
        continue;
      }
      
      // is this in yyyymmdd_hhmmss format?
      
      int year, month, day;
      int hour, min, sec;
      if (sscanf(dp->d_name, "%4d%2d%2d_%2d%2d%2d",
                 &year, &month, &day, &hour, &min, &sec) == 6) {
	if (hour >= 0 && hour <= 23 && min >= 0 && min <= 59 &&
	    sec >= 0 && sec <= 59) {
          DateTime dtime(year, month, day, hour, min, sec);
          time_t validTime = dtime.utime();
          string pathStr = dayDir;
          pathStr += PATH_DELIM;
          pathStr += dp->d_name;
          TimePath tpath(validTime, validTime, validTime, 
                         dp->d_name, pathStr);
          filePaths.insert(filePaths.end(), tpath);
        }
      }
    } // dp
    
    rdir.close();
    
  } // ii
  
  if (filePaths.size() < 1) {
    if (_params.debug) {
      cerr << "No data found" << endl;
    }
    return -1;
  }

  // find file with valid time within start and end times
  // give 1 seconds leeway
  
  TimePathSet::iterator jj;
  bool fileFound = false;
  string bestPath = filePaths.begin()->filePath;
  time_t bestTime = filePaths.begin()->validTime;
  double minAngleDiff = 1.0e99;
  for (jj = filePaths.begin(); jj != filePaths.end(); jj++) {
    string name = jj->fileName;
    time_t fileTime = jj->validTime;
    if (fileTime >= startTime && fileTime <= endTime) {
      double angleDiff = fabs(fixedAngle - jj->fixedAngle);
      if (angleDiff < minAngleDiff) {
        bestPath = jj->filePath;
        bestTime = jj->validTime;
        minAngleDiff = angleDiff;
        fileFound = true;
      }
    }
  }

  if (!fileFound) {
    if (_params.debug) {
      cerr << "No data found" << endl;
    }
    return -1;
  }
  _filePath = bestPath;
  
  if (_params.debug) {
    cerr << "Found file, time: "
         << bestPath << ", "
         << DateTime::strm(bestTime) << endl;
  }

  return 0;

}
  
/////////////////////////////////////
// get day dirs for top dir

void TsReader::_getDayDirs(const string &topDir,
                           TimePathSet &dayDirs)
  
{
  
  // set up dirs in time order
  
  ReadDir rdir;
  if (rdir.open(topDir.c_str())) {
    return;
  }
  
  // Loop thru directory looking for subdir names which represent dates
  
  struct dirent *dp;
  for (dp = rdir.read(); dp != NULL; dp = rdir.read()) {
    
    // exclude dir entries and files beginning with '.'
    
    if (dp->d_name[0] == '.') {
      continue;
    }

    // is this a yyyy directory - using extended paths?
    // if so, call this routine recursively

    if (strlen(dp->d_name) == 4) {
      int yyyy;
      if (sscanf(dp->d_name, "%4d", &yyyy) == 1) {
        // year dir, call this recursively
        string yyyyDir = topDir;
        yyyyDir += PATH_DELIM;
        yyyyDir += dp->d_name;
        _getDayDirs(yyyyDir, dayDirs);
      }
      continue;
    }

    // exclude dir entries too short for yyyymmdd
    
    if (strlen(dp->d_name) < 8 || dp->d_name[0] == '.') {
      continue;
    }

    // check that subdir name is in the correct format
    
    int year, month, day;
    if (sscanf(dp->d_name, "%4d%2d%2d", &year, &month, &day) != 3) {
      continue;
    }
    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
      continue;
    }
    
    DateTime midday(year, month, day, 12, 0, 0);
    DateTime start(year, month, day, 0, 0, 0);
    DateTime end(year, month, day, 23, 59, 59);
    Path dayDirPath(topDir, dp->d_name);
    string pathStr(dayDirPath.getPath());
    TimePath tpath(midday.utime(), start.utime(), end.utime(), 
                   dp->d_name, pathStr);
    dayDirs.insert(dayDirs.end(), tpath);

  } // for (dp ...
  
  rdir.close();
    
}

//////////////////////////////////////////////////
// read all pulses in File

int TsReader::_readFile()
  
{

  _clearPulseQueue();

  // read in pulse, load up pulse queue

  while (true) {

    PMU_auto_register("readFile");
    
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    
    if (pulse == NULL) {
      if (_params.debug) {
        cerr << "TsReader::readFilePulses: found n pulses: "
             << _pulseQueue.size() << ", scan mode: "
             << _scanModeStr << endl;
      }
      break;
    }
    
    if (_params.invert_hv_flag) {
      pulse->setInvertHvFlag(true);
    }
    
    if (!_params.prt_is_for_previous_interval) {
      pulse->swapPrtValues();
    }
    
    // check scan mode and type
    
    int scanMode = pulse->getScanMode();
    if (scanMode == IWRF_SCAN_MODE_RHI) {
      _isRhi = true;
      _scanModeStr = "RHI";
      _scanType = SCAN_TYPE_RHI;
      _indexedRes = _params.indexed_resolution_rhi;
    } else {
      _isRhi = false;
      _scanModeStr = "PPI";
      _scanType = SCAN_TYPE_PPI;
      _indexedRes = _params.indexed_resolution_ppi;
    }
    
    // Create a new pulse object and save a pointer to it in the
    // _pulseQueue.  _pulseQueue is a FIFO, with elements
    // added at the end and dropped off the beginning. So if we have a
    // full buffer delete the first element before shifting the
    // elements to the left.
    
    // add pulse to queue, managing memory appropriately

    _addPulseToQueue(pulse);
    
  } // while

  // set prt and nGates, assuming single PRT for now
  
  _prt = _pulseQueue[0]->getPrt();
  _nGates = _pulseQueue[0]->getNGates();
  
  // check if we have alternating h/v pulses
  
  _checkIsAlternating();

  // check if we have staggered PRT pulses - does not apply
  // to alternating mode

  if (_isAlternating) {
    _isStaggeredPrt = false;
  } else {
    _checkIsStaggeredPrt();
  }

  // in non-staggered mode check we have constant nGates
  
  if (!_isStaggeredPrt) {
    int nGates0 = _pulseQueue[0]->getNGates();
    for (int i = 1; i < (int) _pulseQueue.size(); i += 2) {
      if (_pulseQueue[i]->getNGates() != nGates0) {
        cerr << "ERROR - TsReader::readFile()" << endl;
        cerr << "  nGates varies in non-staggered mode" << endl;
        return -1;
      }
    }
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (_isAlternating) {
      cerr << "==>> Fast alternating mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    } else if (_isStaggeredPrt) {
      cerr << "==>> Staggered PRT mode" << endl;
      cerr << "     prt short: " << _prtShort << endl;
      cerr << "     prt long: " << _prtLong << endl;
      cerr << "     ngates short PRT: " << _nGatesPrtShort << endl;
      cerr << "     ngates long PRT: " << _nGatesPrtLong << endl;
    } else {
      cerr << "==>> Single PRT mode" << endl;
      cerr << "     prt: " << _prt << endl;
      cerr << "     ngates: " << _nGates << endl;
    }
  }

  return 0;

}

///////////////////////////////////////////      
// check if we have alternating h/v pulses
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

void TsReader::_checkIsAlternating()
  
{
  
  _isAlternating = false;

  bool prevHoriz = _pulseQueue[0]->isHoriz();
  for (int i = 1; i < (int) _pulseQueue.size(); i++) {
    bool thisHoriz = _pulseQueue[i]->isHoriz();
    if (thisHoriz == prevHoriz) {
      return;
    }
    prevHoriz = thisHoriz;
  }
  _isAlternating = true;
  
}

///////////////////////////////////////////      
// check if we have staggered PRT mode
//
// Also check that we start on a short prt
// If so, the queue is ready to make a beam

void TsReader::_checkIsStaggeredPrt()

{

  _isStaggeredPrt = false;
  
  double prt0 = _pulseQueue[0]->getPrt(); // most recent pulse
  double prt1 = _pulseQueue[1]->getPrt(); // next most recent pulse

  int nGates0 = _pulseQueue[0]->getNGates();
  int nGates1 = _pulseQueue[1]->getNGates();

  if (fabs(prt0 - prt1) < 0.00001) {
    return;
  }

  for (int ii = 2; ii < _nSamples + 1; ii += 2) {
    if (fabs(_pulseQueue[ii]->getPrt() - prt0) > 0.00001) {
      return;
    }
    if (_pulseQueue[ii]->getNGates() != nGates0) {
      return;
    }
  }

  for (int ii = 1; ii < _nSamples + 1; ii += 2) {
    if (fabs(_pulseQueue[ii]->getPrt() - prt1) > 0.00001) {
      return;
    }
    if (_pulseQueue[ii]->getNGates() != nGates1) {
      return;
    }
  }

  _isStaggeredPrt = true;

  // We want to start on short PRT, which means we must
  // end on long PRT, since we always have an even number
  // of pulses.
  // However, the prt in the headers refers to the
  // prt from the PREVIOUS pulse to THIS pulse, so the
  // short prt pulse is actually identified by the longer of
  // the prts, and has the smaller number of gates

  if (prt0 < prt1) {
    _prtShort = prt0;
    _prtLong = prt1;
    _nGatesPrtShort = nGates1;
    _nGatesPrtLong = nGates0;
  } else {
    _prtShort = prt1;
    _prtLong = prt0;
    _nGatesPrtShort = nGates0;
    _nGatesPrtLong = nGates1;
  }

  _prt = _prtShort;
  _nGates = _nGatesPrtShort;

}

////////////////////////////////
// condition azimuth angle

double TsReader::_conditionAz(double az)
  
{
  if (az > 360.0) {
    return (az - 360);
  } else if (az < 0) {
    return (az + 360.0);
  }
  return az;
}
  
double TsReader::_conditionAz(double az, double refAz)
  
{
  double diff = az - refAz;
  if (diff > 180.0) {
    return (az - 360);
  } else if (diff < -180) {
    return (az + 360.0);
  }
  return az;
}
  
////////////////////////////////
// condition elevation angle

double TsReader::_conditionEl(double el)
  
{
  if (el > 180.0) {
    return (el - 360);
  } else if (el < -180) {
    return (el + 360.0);
  }
  return el;
}
  
