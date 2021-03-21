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
#include <radar/IwrfTsReader.hh>
#include <radar/IwrfTsInfo.hh>
#include <didss/DataFileNames.hh>
using namespace std;

////////////////////////////////////////////////////
// Base class

IwrfTsGet::IwrfTsGet(IwrfDebug_t debug) :
        _debug(debug)
        
{
  _timeMarginSecs = 10.0;
  _fileCount = 0;
}

//////////////////////////////////////////////////////////////////
// destructor

IwrfTsGet::~IwrfTsGet()

{

}

//////////////////////////////////////////////////////////////////
// reset queue to start

void IwrfTsGet::resetToStart()

{
}

//////////////////////////////////////////////////////////////////
// reset queue to end

void IwrfTsGet::resetToEnd()

{
}

//////////////////////////////////////////////////////////////////
// get a beam of pulses, given the time, el and az
// returns empty vector on failure
// data in the beam must be used before any other operations
// are performed on the archive

vector<IwrfTsGet::PulseEntry *>
  IwrfTsGet::getBeam(const DateTime &searchTime,
                     double searchElev,
                     double searchAz)
{

  // init

  
  // check if we have data already
  
  DateTime minTime = searchTime - _timeMarginSecs;
  DateTime maxTime = searchTime + _timeMarginSecs;
  if (minTime < _startTime || maxTime > _endTime) {
    // current data does not cover the time requested
    // retrieve data from relevant file
    _doRetrieve(searchTime);
  }
  
  vector<PulseEntry *> beamPulses;
  
  return beamPulses;
  
}

//////////////////////////////////////////////////////////////////
// retrieve more data

void IwrfTsGet::_doRetrieve(const DateTime &searchTime)
{

  // get path just after search time
  
  string firstPathAfter = _getFirstPathAfter(searchTime);
  if (firstPathAfter.size() == 0) {
    return;
  }

  // create reader

  vector<string> fileList;
  fileList.push_back(firstPathAfter);
  _fileCount++;
  IwrfDebug_t iwrfDebug = IWRF_DEBUG_OFF;
  if (_debug) {
    iwrfDebug = IWRF_DEBUG_NORM;
  }
  IwrfTsReaderFile reader(fileList, iwrfDebug);
  IwrfTsPulse *pulse = NULL;
  while ((pulse = reader.getNextPulse()) != NULL) {
    PulseEntry *entry = new PulseEntry(pulse);
    const IwrfTsInfo &info = reader.getOpsInfo();
    entry->setFileNum(_fileCount);
    entry->setScanRate(info.get_scan_rate());
    entry->setXmitRcvMode((iwrf_xmit_rcv_mode_t) info.get_proc_xmit_rcv_mode());
    entry->setXmitPhaseMode((iwrf_xmit_phase_mode_t)info.get_proc_xmit_phase_mode());
    entry->setPrfMode((iwrf_prf_mode_t) info.get_proc_prf_mode());
    entry->setPolMode((iwrf_pol_mode_t) info.get_proc_pol_mode());
    _pulseEntries.push_back(entry);
  }
  
}

///////////////////////////////////////////////////////////
// Find the first path after the search time

string IwrfTsGet::_getFirstPathAfter(const DateTime &searchTime)
  
{

  // compute subdir path

  char subdirPath[1024];
  snprintf(subdirPath, 1024, "%s/%.4d%.2d%.2d",
           _topDir.c_str(), searchTime.getYear(),
           searchTime.getMonth(), searchTime.getDay());
  
  // find all paths with this volume number
  
  DIR *dirp;
  if((dirp = opendir(subdirPath)) == NULL) {
    int errNum = errno;
    cerr << "WARNING - IwrfTsGet::_getFirstPathAfter()" << endl;
    cerr << "  Cannot open dir: " << subdirPath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return "";
  }
  
  struct dirent *dp;
  for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
    
    string fileName(dp->d_name);
    
    time_t fileTime;
    bool dateOnly;
    if (DataFileNames::getDataTime(fileName, fileTime, dateOnly)) {
      continue;
    }
    
    if (fileTime > searchTime.utime()) {
      string path(subdirPath);
      path += "/";
      path += fileName;
      closedir(dirp);
      return path;
    }
    
  } // dp

  closedir(dirp);
  cerr << "WARNING - IwrfTsGet::_getFirstPathAfter()" << endl;
  cerr << "  No file found, dir: " << subdirPath << endl;
  return "";

}

///////////////////////////////////////////////////////////////
// PulseEntry inner class
///////////////////////////////////////////////////////////////

// Constructor

IwrfTsGet::PulseEntry::PulseEntry(IwrfTsPulse *pulse) :
        _pulse(pulse)

{

  _burst = NULL;
  _fileNum = -1;
  
}  

// destructor

IwrfTsGet::PulseEntry::~PulseEntry() 
{
  delete _pulse;
  if (_burst != NULL) {
    delete _burst;
  }
}

// compute clump geom

// void IwrfTsGet::PulseEntry::computeGeom() 
// {

// }

