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
// IwrfTsGet.cc
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Retrieves time series data from a file archive.
//
// March 2021
//
///////////////////////////////////////////////////////////////
//
// IwrfTsGet retrieves time series data given time/el/az.
// It holds an array (deque) of pulseEntry objects, and
// creates from them an array of pulses for a beam.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <radar/IwrfTsGet.hh>
#include <didss/DataFileNames.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

IwrfTsGet::IwrfTsGet(IwrfDebug_t debug) :
        _debug(debug)
        
{
  _clearFileList();
  _clearPulseList();
  _timeMarginSecs = 15.0;
  _invertHvFlag = false;
  _prtIsForNextInterval = false;
  _reader = NULL;
  _isAlternating = false;
  _isStaggeredPrt = false;
  _prt = 0.0;
  _prtShort = 0.0;
  _prtLong = 0.0;
  _nGates = 0;
  _nGatesPrtShort = 0;
  _nGatesPrtLong = 0;
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfTsGet::~IwrfTsGet()

{

  if (_reader) {
    delete _reader;
  }
  _clearFileList();
  _clearPulseList();

}

//////////////////////////////////////////////////////////////////
// Get a beam of pulses, given the time, el and az.
// Fills the beamPulses vector.
// Returns 0 on success, -1 on failure.
// To search on time only, set az and el to -9999.
// The beamPulses vector points to pulses managed by this object.
// Data in the beam must be used before any further get operations
// are performed on this object.

int IwrfTsGet::retrieveBeam(const DateTime &searchTime,
                            double searchEl,
                            double searchAz,
                            int nSamples,
                            vector<IwrfTsPulse *> &beamPulses)

{

  // init
  
  beamPulses.clear();

  if (_debug >= IWRF_DEBUG_VERBOSE) {
    cerr << "==== IwrfTsGet::retrieveBeam() ====" << endl;
    cerr << "=====>> searchTime: " << searchTime.asString(3) << endl;
    cerr << "=====>> searchEl  : " << searchEl << endl;
    cerr << "=====>> searchAz  : " << searchAz << endl;
    cerr << "=====>> nSamples  : " << searchAz << endl;
    cerr << "=====>> cached pulse list start time: " << _pulsesStartTime.asString(3) << endl;
    cerr << "=====>> cached pulse list end   time: " << _pulsesEndTime.asString(3) << endl;
  }

  if (searchTime < _pulsesStartTime || searchTime > _pulsesEndTime) {
    // current data does not cover the time requested
    // load the pulse list
    _loadPulseList(searchTime);
  }
  
  // load up the pulses for the beam at this search time and location
  
  if (_loadBeamPulses(searchTime, searchEl, searchAz,
                      nSamples, beamPulses)) {
    // if this fails, reload the pulse list and try again
    _loadPulseList(searchTime);
    if (_loadBeamPulses(searchTime, searchEl, searchAz,
                        nSamples, beamPulses)) {
      return -1;
    }
  }

  return 0;

}

///////////////////////////////////////////////////////////////////
// Load up the file list, if not already done given the search time.
// We load up the list of files for the search day, plus
// one day before and one day after.

int IwrfTsGet::_loadFileList(const DateTime &searchTime)
  
{

  // check if we already have a suitable list
  
  if (_fileListMap.size() > 0) {
    if (searchTime - _filesStartTime > _timeMarginSecs &&
        _filesEndTime - searchTime > _timeMarginSecs) {
      // we already have files and the search time is
      // more than margin secs from each end
      return 0;
    }
  }

  // init

  _clearFileList();
  
  // we don't yet have the file list
  // load it up for this day, plus one day before and one day after

  DateTime prevDay = searchTime - 86400.0;
  DateTime nextDay = searchTime + 86400.0;

  for (DateTime day = prevDay; day <= nextDay; day += 86400) {
    
    // compute subdir path
    
    char subdirPath[1024];
    snprintf(subdirPath, 1024, "%s/%.4d%.2d%.2d",
             _topDir.c_str(),
             day.getYear(), day.getMonth(), day.getDay());
    
    // open dir
    
    DIR *dirp;
    if((dirp = opendir(subdirPath)) == NULL) {
      if (_debug >= IWRF_DEBUG_VERBOSE) {
        int errNum = errno;
        cerr << "WARNING - IwrfTsGet::_loadFileList()" << endl;
        cerr << "  Cannot open dir: " << subdirPath << endl;
        cerr << "  " << strerror(errNum) << endl;
      }
      continue;
    }

    // read entries
    
    struct dirent *dp;
    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
      
      string fileName(dp->d_name);

      // file path is named for start time of data in file
      
      time_t fileTime;
      bool dateOnly;
      if (DataFileNames::getDataTime(fileName, fileTime, dateOnly)) {
        continue;
      }
      if (dateOnly) {
        continue;
      }
      
      // got file from which we can decode the time from the name
      // compute path, add that to the map
      
      string filePath(subdirPath);
      filePath += "/";
      filePath += fileName;

      _fileListMap[fileTime] = filePath;

    } // dp
    
    closedir(dirp);
    
  } // day

  // check we got files
  
  if (_fileListMap.size() < 1) {
    if (_debug >= IWRF_DEBUG_VERBOSE) {
      cerr << "ERROR - IwrfTsGet::_loadFileList()" << endl;
      cerr << "  No files found, dir: " << _topDir << endl;
      cerr << "  Search time: " << searchTime.asString(3) << endl;
    }
    return -1;
  }

  // load up file list property vectors

  for (auto& xx: _fileListMap) {
    _fileListStartTimes.push_back(xx.first);
    _fileListPaths.push_back(xx.second);
  }
  if (_fileListStartTimes.size() == 1) {
    _fileListEndTimes.push_back(_fileListStartTimes[0]);
  } else {
    for (size_t ii = 0; ii < _fileListStartTimes.size(); ii++) {
      if (ii < _fileListStartTimes.size() - 1) {
        _fileListEndTimes.push_back(_fileListStartTimes[ii + 1]);
      } else {
        // for the last file we use the length of the previous file,
        // in seconds
        int prevFileLengthSecs =
          _fileListEndTimes[ii - 1] - _fileListStartTimes[ii - 1];
        _fileListEndTimes[ii] = _fileListStartTimes[ii] + prevFileLengthSecs;
      }
    }
  }
  
  if (_debug) {
    cerr << "====>> FileList <<====" << endl;
    cerr << "  nFiles: " << _fileListStartTimes.size() << endl;
    for (size_t ii = 0; ii < _fileListStartTimes.size(); ii++) {
      cerr << "  Index, start time, end time, path: "
           << ii << ", "
           << DateTime::strm(_fileListStartTimes[ii]) << ", "
           << DateTime::strm(_fileListEndTimes[ii]) << ", "
           << _fileListPaths[ii] << endl;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
// Clear the file list

void IwrfTsGet::_clearFileList()
{

  _fileListMap.clear();
  _fileListStartTimes.clear();
  _fileListEndTimes.clear();
  _fileListPaths.clear();

  _filesStartTime.set(DateTime::NEVER);
  _filesEndTime.set(DateTime::NEVER);
    
}
  
//////////////////////////////////////////////////////////////////
// Load up the pulse list given the search time.
// We load up pulses from the time series file that
// contains the search time, plus one before and one after.

int IwrfTsGet::_loadPulseList(const DateTime &searchTime)
{

  // clear
  
  _clearPulseList();
  // _pulseEntries.clear();

  // load the file list if needed
  
  if (_loadFileList(searchTime)) {
    return -1;
  }

  // From the file list, find the file that contains the search time.

  int searchIndex = -1;
  for (size_t ii = 0; ii < _fileListMap.size() - 1; ii++) {
    time_t searchSecs = searchTime.utime();
    if (_fileListStartTimes[ii] <= searchSecs &&
        _fileListEndTimes[ii] >= searchSecs) {
      searchIndex = ii;
      break;
    }
  }

  if (searchIndex < 0) {
    cerr << "ERROR - IwrfTsGet::_loadPulseList()" << endl;
    cerr << "  Cannot find file for time: " << searchTime.asString(3) << endl;
    return -1;
  }
  
  // load up pulses for 3 files:
  //   the file before the search time
  //   the file containing the search time
  //   the file after the search time
  
  vector<string> fileList;
  if (searchIndex > 0) {
    fileList.push_back(_fileListPaths[searchIndex - 1]);
  }
  fileList.push_back(_fileListPaths[searchIndex]);
  if (searchIndex < (int) _fileListMap.size() - 1) {
    fileList.push_back(_fileListPaths[searchIndex + 1]);
  }

  // create reader
  
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  if (_reader) {
    delete _reader;
  }
  _reader = new IwrfTsReaderFile(fileList, iwrfDebug);
  IwrfTsPulse *pulse = NULL;

  // read in pulses

  if (_debug) {
    cerr << "  ====>> reading in pulses ......." << endl;
  }

  bool infoSet = false;
  while ((pulse = _reader->getNextPulse(true)) != NULL) {
    if (_invertHvFlag) {
      pulse->setInvertHvFlag(true);
    }
    if (_prtIsForNextInterval) {
      pulse->swapPrtValues();
    }
    PulseEntry *entry = new PulseEntry(pulse);
    entry->setFilePath(_reader->getPathInUse());
    _pulseEntries.push_back(entry);
    if (!infoSet) {
      _info = _reader->getOpsInfo();
      infoSet = true;
    }
  } // while

  if (_pulseEntries.size() < 2) {
    cerr << "ERROR - IwrfTsGet::_loadPulseList()" << endl;
    cerr << "  n pulses found: " << _pulseEntries.size() << endl;
    return -1;
  }
  
  _pulsesStartTime = _pulseEntries[0]->getPulse()->getTime();
  _pulsesEndTime = _pulseEntries[_pulseEntries.size()-1]->getPulse()->getTime();

  // calibration

  _calib.set(_info.getCalibration());

  if (_debug) {
    cerr << "  loaded n pulses  : " << _pulseEntries.size() << endl;
    cerr << "  pulses start time: " << _pulsesStartTime.asString(3) << endl;
    cerr << "  pulses end time  : " << _pulsesEndTime.asString(3) << endl;
  }

  return 0;
  
}

//////////////////////////////////////////////////////////////////
// Clear the pulse list

void IwrfTsGet::_clearPulseList()
{

  for (size_t ii = 0; ii < _pulseEntries.size(); ii++) {
    PulseEntry *entry = _pulseEntries[ii];
    delete entry;
  }
  _pulseEntries.clear();

  _pulsesStartTime.set(DateTime::NEVER);
  _pulsesEndTime.set(DateTime::NEVER);

}
  
//////////////////////////////////////////////////////////////////
// Load the pulses for a beam
// To search on time only, set az and el to -9999.

int IwrfTsGet::_loadBeamPulses(const DateTime &searchTime,
                               double searchEl,
                               double searchAz,
                               int nSamples,
                               vector<IwrfTsPulse *> &beamPulses)
  
{

  // check we have enough pulses

  if ((int) _pulseEntries.size() < (nSamples + 4)) {
    // too few pulses
    if (_debug >= IWRF_DEBUG_VERBOSE) {
      cerr << "ERROR - IwrfTsGet::_loadBeamPulses()" << endl;
      cerr << "  too few pulses" << endl;
    }
    return -1;
  }

  // find closest pulse to search time

  double midTimeDiff = 1.0e99;
  size_t midTimeIndex = _pulseEntries.size() / 2;
  for (size_t ii = 0; ii < _pulseEntries.size(); ii++) {
    const DateTime &pulseTime = _pulseEntries[ii]->getTime();
    double timeDiff = fabs(searchTime - pulseTime);
    if (timeDiff < midTimeDiff) {
      midTimeDiff = timeDiff;
      midTimeIndex = ii;
    }
  } // ii

  // set index limits relative to midTimeIndex
  
  ssize_t index0 = _conditionPulseIndex(midTimeIndex - nSamples / 2);
  ssize_t index1 = _conditionPulseIndex(index0 + nSamples / 2);

  // determine whether this is a PPI or RHI
  
  double deltaAz = _conditionDeltaAz(_pulseEntries[index0]->getAz() -
                                     _pulseEntries[index1]->getAz());
  double deltaEl = fabs(_pulseEntries[index0]->getEl() -
                        _pulseEntries[index1]->getEl());
  bool isRhi = false;
  if (deltaEl > deltaAz) {
    isRhi = true;
  }
  
  if (_debug) {
    cerr << "===========>>>> isRhi: " << isRhi << endl;
  }

  // now find the initial search location

  double midDiff = 1.0e99;
  size_t midIndex = _pulseEntries.size() / 2;
  for (size_t ii = 0; ii < _pulseEntries.size(); ii++) {
    const PulseEntry *entry = _pulseEntries[ii];
    const DateTime &pulseTime = entry->getTime();
    double timeDiff = fabs(searchTime - pulseTime);
    double azDiff = _conditionDeltaAz(searchAz - entry->getAz());
    double elDiff = fabs(searchEl - entry->getEl());
    double diff = 0;
    if (isRhi) {
      diff = azDiff * 50.0 + elDiff + timeDiff;
    } else {
      diff = azDiff + elDiff * 50.0 + timeDiff;
    }
    if (diff < midDiff) {
      midDiff = diff;
      midIndex = ii;
    }
  } // ii
  
  index0 = _conditionPulseIndex(midIndex - nSamples);
  index1 = _conditionPulseIndex(index0 + nSamples);

  // compute the mean PRT
  
  const DateTime &time0 = _pulseEntries[index0]->getTime();
  const DateTime &time1 = _pulseEntries[index1]->getTime();
  double deltaTime = time1 - time0;
  double meanPrt = deltaTime / (double) (index1 - index0);
  
  // set search indices to look back and fwd by an extra 1 sec
  
  int nSamples1Sec = (int) (1.0 / meanPrt + 0.5);
  if (nSamples1Sec < nSamples) {
    nSamples1Sec = nSamples;
  }
  index0 = _conditionPulseIndex(index0 - nSamples1Sec);
  index1 = _conditionPulseIndex(index1 + nSamples1Sec);
  
  // find the mid index where the angles straddle the required value

  ssize_t centralIndex = midIndex;
  if (searchAz > -9990.0 && searchEl > -9999.0) {
    if (isRhi) {
      for (ssize_t ii = index0; ii <= index1; ii++) {
        if (_checkIsBeamRhi(ii, searchEl, 1.0)) {
          centralIndex = ii;
          break;
        }
      } // ii
    } else {
      for (ssize_t ii = index0; ii <= index1; ii++) {
        if (_checkIsBeamPpi(ii, searchAz, 1.0)) {
          centralIndex = ii;
          break;
        }
      } // ii
    }
  } // if (searchAz > -9990.0 && searchEl > -9999.0) {

  // reset index limits relative to centralIndex
  
  index0 = _conditionPulseIndex(centralIndex - nSamples / 2);
  index1 = _conditionPulseIndex(centralIndex + nSamples / 2);
  
  // update the info object based on properties of the
  // pulse at the central index

  PulseEntry *centralEntry = _pulseEntries[centralIndex];
  IwrfTsPulse *centralPulse = centralEntry->getPulse();
  DateTime pulseTime(centralPulse->getTime(), centralPulse->getNanoSecs() / 1.0e9);
  _pathInUse = centralEntry->getFilePath();
  _info.set_scan_rate(centralEntry->getScanRate());
  _info.set_xmit_info_xmit_rcv_mode(centralEntry->getXmitRcvMode());
  _info.set_xmit_info_xmit_phase_mode(centralEntry->getXmitPhaseMode());
  _info.set_xmit_info_prf_mode(centralEntry->getPrfMode());
  _info.set_proc_pol_mode(centralEntry->getPolMode());
  _prt = centralPulse->getPrt();
  _nGates = centralPulse->getNGates();

  // check for alternating or staggered mode
  
  _isAlternating = _checkIsAlternating(index0, index1);
  _isStaggeredPrt = _checkIsStaggeredPrt(index0, index1);
  if (_isAlternating) {
    _isStaggeredPrt = false;
  }

  // adjust to make sure we start on correct pulse
  
  if (_isAlternating) {
    bool startsOnHoriz = _pulseEntries[index0]->getPulse()->isHoriz();
    if (!startsOnHoriz) {
      index0++;
      index1++;
    }
  } else if (_isStaggeredPrt) {
    double prt0 = _pulseEntries[index0]->getPulse()->getPrt();
    double prt1 = _pulseEntries[index0+1]->getPulse()->getPrt();
    if (prt0 > prt1) {
      index0++;
      index1++;
    }
  }

  // load up beam pulses
  
  beamPulses.clear();
  for (ssize_t ii = index0; ii < index1; ii++) {
    // get the pulse
    IwrfTsPulse *pulse = _pulseEntries[ii]->getPulse();
    // update the info
    pulse->setOpsInfo(_info);
    // add to vector
    beamPulses.push_back(pulse);
  }

  if ((int) beamPulses.size() < nSamples) {
    if (_debug >= IWRF_DEBUG_VERBOSE) {
      cerr << "  Beam has too few pulses, beamPulses.size(): " 
           << nSamples << ", " << beamPulses.size() << endl;
      cerr << "  retyring ..." << endl;
    }
    return -1;
  }
  
  return 0;

}

/////////////////////////////////////////////////
// check for a ppi beam at the stated index

bool IwrfTsGet::_checkIsBeamPpi(ssize_t centralIndex,
                                double az, double indexedRes)
  
{
  
  // compute azimuths which need to straddle the center of the beam
  
  double midAz1 = _conditionAz(_pulseEntries[centralIndex]->getAz(), az);
  double midAz2 = _conditionAz(_pulseEntries[centralIndex+1]->getAz(), midAz1);
  
  // Check if the azimuths at the center of the data straddle
  // the target azimuth
  
  if (midAz1 <= az && midAz2 >= az) {
    // az1 is below and az2 above - clockwise rotation
    return true;
  } else if (midAz1 >= az && midAz2 <= az) {
    // az1 is above and az2 below - counterclockwise rotation
    return true;
  } else if (az == 0.0) {
    if (midAz1 > 360.0 - indexedRes && midAz2 < indexedRes) {
      // az1 is below 0 and az2 above 0 - clockwise rotation
      return true;
    } else if (midAz2 > 360.0 - indexedRes && midAz1 < indexedRes) {
      // az1 is above 0 and az2 below 0 - counterclockwise rotation
      return true;
    }
  }
  
  return false;

}

/////////////////////////////////////////////////
// check for an rhi beam at the stated index

bool IwrfTsGet::_checkIsBeamRhi(ssize_t centralIndex, double el, double indexedRes)
  
{
  
  // compute elevations which need to straddle the center of the beam
  
  double midEl1 = _conditionEl(_pulseEntries[centralIndex]->getEl());
  double midEl2 = _conditionEl(_pulseEntries[centralIndex+1]->getEl());
  
  // Check if the elevations at the center of the data straddle
  // the target elevation
  
  if (midEl1 <= el && midEl2 >= el) {
    // el1 is below and el2 above - el increasing
    return true;
  } else if (midEl1 >= el && midEl2 <= el) {
    // el1 is above and el2 below - el decreasing
    return true;
  }

  return false;

}

////////////////////////////////
// condition azimuth angle

double IwrfTsGet::_conditionAz(double az)
  
{
  if (az > 360.0) {
    return (az - 360);
  } else if (az < 0) {
    return (az + 360.0);
  }
  return az;
}
  
double IwrfTsGet::_conditionAz(double az, double refAz)
  
{
  double diff = az - refAz;
  if (diff > 180.0) {
    return (az - 360);
  } else if (diff < -180) {
    return (az + 360.0);
  }
  return az;
}
  
double IwrfTsGet::_conditionDeltaAz(double deltaAz)
  
{
  deltaAz = fabs(deltaAz);
  if (deltaAz > 180.0) {
    deltaAz = fabs(deltaAz - 360.0);
  }
  return deltaAz;
}
  
////////////////////////////////
// condition elevation angle

double IwrfTsGet::_conditionEl(double el)
  
{
  if (el > 180.0) {
    return (el - 360);
  } else if (el < -180) {
    return (el + 360.0);
  }
  return el;
}
  
////////////////////////////////
// condition a pulse index

ssize_t IwrfTsGet::_conditionPulseIndex(ssize_t index)

{
  if (index < 0) {
    return 0;
  }
  if (index > (ssize_t) _pulseEntries.size() - 1) {
    return _pulseEntries.size() - 1;
  }
  return index;
}

///////////////////////////////////////////      
// check if we have alternating h/v pulses
// 
// Also check that we start on a horizontal pulse
// If so, the queue is ready to make a beam

bool IwrfTsGet::_checkIsAlternating(ssize_t index0,
                                    ssize_t index1)
  
{
  
  _isAlternating = false;
  
  bool prevHoriz = _pulseEntries[index0]->getPulse()->isHoriz();
  for (ssize_t ii = index0 + 1; ii <= index1; ii++) {
    bool thisHoriz = _pulseEntries[ii]->getPulse()->isHoriz();
    if (thisHoriz == prevHoriz) {
      return false;
    }
    prevHoriz = thisHoriz;
  }

  return true;
  
}

///////////////////////////////////////////      
// check if we have staggered PRT mode
//
// Also check that we start on a short prt
// If so, the queue is ready to make a beam

bool IwrfTsGet::_checkIsStaggeredPrt(ssize_t index0,
                                     ssize_t index1)

{

  _isStaggeredPrt = false;
  
  double prt0 = _pulseEntries[index0]->getPulse()->getPrt();
  double prt1 = _pulseEntries[index0+1]->getPulse()->getPrt();
  
  int nGates0 = _pulseEntries[index0]->getPulse()->getNGates();
  int nGates1 = _pulseEntries[index0+1]->getPulse()->getNGates();

  // do the PRTs differ?
  
  if (fabs(prt0 - prt1) < 0.00001) {
    return false;
  }

  // are the prt and ngates constant for each prt?
  
  for (ssize_t ii = index0; ii < index1; ii += 2) {
    if (fabs(_pulseEntries[ii]->getPulse()->getPrt() - prt0) > 0.00001) {
      return false;
    }
    if (_pulseEntries[ii]->getPulse()->getNGates() != nGates0) {
      return false;
    }
  }
  
  for (int ii = index0 + 1; ii < index1; ii += 2) {
    if (fabs(_pulseEntries[ii]->getPulse()->getPrt() - prt1) > 0.00001) {
      return false;
    }
    if (_pulseEntries[ii]->getPulse()->getNGates() != nGates1) {
      return false;
    }
  }

  // is staggered
  
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

  return true;

}

///////////////////////////////////////////////////////////////
// PulseEntry inner class
///////////////////////////////////////////////////////////////

// Constructor

IwrfTsGet::PulseEntry::PulseEntry(IwrfTsPulse *pulse) :
        _pulse(pulse)

{

  _burst = NULL;

  _time.set(pulse->getTime(), pulse->getNanoSecs() / 1.0e9);

  _el = pulse->getEl();
  _az = pulse->getAz();
  
  const IwrfTsInfo &info = pulse->getTsInfo();
  setScanRate(info.get_scan_rate());
  setXmitRcvMode((iwrf_xmit_rcv_mode_t) info.get_proc_xmit_rcv_mode());
  setXmitPhaseMode((iwrf_xmit_phase_mode_t)info.get_proc_xmit_phase_mode());
  setPrfMode((iwrf_prf_mode_t) info.get_proc_prf_mode());
  setPolMode((iwrf_pol_mode_t) info.get_proc_pol_mode());

}  

// destructor

IwrfTsGet::PulseEntry::~PulseEntry() 
{
  delete _pulse;
  if (_burst != NULL) {
    delete _burst;
  }
}


