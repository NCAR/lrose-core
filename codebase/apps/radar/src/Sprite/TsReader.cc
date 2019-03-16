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
#include <toolsa/DateTime.hh>
#include <toolsa/ReadDir.hh>
#include <toolsa/Path.hh>
#include "TsReader.hh"
using namespace std;

////////////////////////////////////////////////////
// Constructor

TsReader::TsReader(const string &prog_name,
                   const Params &params,
                   const Args &args) :
        _progName(prog_name),
        _params(params),
        _args(args)
  
{

  constructorOK = true;

  _pulseReader = NULL;
  _scanType = SCAN_TYPE_UNKNOWN;
  
  _pulseSeqNum = 0;
  
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

}

//////////////////////////////////////////////////////////////////
// destructor

TsReader::~TsReader()

{

  _clear();

}

//////////////////////////////////////////////////////////////////
// clear reader and data

void TsReader::_clear()

{

  if (_pulseReader) {
    delete _pulseReader;
  }

  _clearPulseQueue();

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

//////////////////////////////////////////////////
// read all pulses

int TsReader::readAllPulses()
  
{

  _clearPulseQueue();

  // read in pulse, load up pulse queue

  while (true) {

    PMU_auto_register("readAllPulses");
  
    IwrfTsPulse *pulse = _pulseReader->getNextPulse(true);
    
    if (pulse == NULL) {
      if (_params.debug) {
        cerr << "TsReader::readAllPulses: found n pulses: "
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
        cerr << "ERROR - TsReader::readAllPulses()" << endl;
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

//////////////////////////////////////////////////
// get a beam
// returns Beam object pointer on success, NULL on failure
// caller must free beam

Beam *TsReader::getBeam(double az, double el)
  
{

  if ((int) _pulseQueue.size() < (_nSamples + 4)) {
    // too few pulses
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
  
  Beam *beam = NULL;
  if (_scanType == SCAN_TYPE_PPI) {
    beam = new Beam(_progName, _params,
                    true, _az,
                    _isAlternating, _isStaggeredPrt,
                    _nGates, _nGatesPrtLong,
                    _prt, _prtLong,
                    _indexedBeams, _indexedRes,
                    _pulseReader->getOpsInfo(),
                    beamPulses);
  } else {
    beam = new Beam(_progName, _params,
                    false, _el,
                    _isAlternating, _isStaggeredPrt,
                    _nGates, _nGatesPrtLong,
                    _prt, _prtLong,
                    _indexedBeams, _indexedRes,
                    _pulseReader->getOpsInfo(),
                    beamPulses);
  }
  
  return beam;
  
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

//////////////////////////////////////////////////
// find the files for the requested time

int TsReader::findBestFile(time_t startTime, time_t endTime,
                           double az, double el, bool isRhi)
  
{

  if (_params.debug) {
    cerr << "TsReader - finding time series files" << endl;
    cerr << "  Data start time: " << DateTime::strm(startTime) << endl;
    cerr << "  Data end time: " << DateTime::strm(endTime) << endl;
    cerr << "  Az, el: " << az << ", " << el << endl;
    cerr << "  isRhi: " << isRhi << endl;
  }

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

  if (_params.debug) {
    cerr << "Found file, time: "
         << bestPath << ", "
         << DateTime::strm(bestTime) << endl;
  }

  _clear();

  _filePath = bestPath;
  vector<string> fileList;
  fileList.push_back(bestPath);
  _pulseReader = new IwrfTsReaderFile(fileList, _iwrfDebug);
  
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

