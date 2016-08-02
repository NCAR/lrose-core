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
//////////////////////////////////////////////////////////
// DsSpdbInput.cc
//
// Class to provide Spdb data set time information to a client.
//
// There are two modes of operation:
//
// Archive mode:
//   The constructor passes in a URL and a start and end time.
//   A list of available times is compiled and stored.
//   The times are served out when next() is called.
//   If next() returns an error, the list is exhausted.
//
// Realtime mode:
//   The constructor passes in a URL to watch, as well as
//   as max valid age and a heartbeat function to be called while
//   waiting for new data to arrive. When next() is called, the
//   routine will watch the input directory for a new file. When
//   a new file arrives the path is returned by next(). If the
//   heartbeat_func is not NULL, it gets called while polling continues.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2001
//
//////////////////////////////////////////////////////////

#include <Spdb/DsSpdbInput.hh>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <toolsa/uusleep.h>
#include <ctime>
using namespace std;

//////////////
// Constructor

DsSpdbInput::DsSpdbInput ()

{
  _mode = NO_MODE;
}

/////////////
// Destructor

DsSpdbInput::~DsSpdbInput()
{
  
}

/////////////////////////////
// Set archive mode.
//
// Load up archive time list.
// The list will contain all available data times
// between the start and end times for the given URL.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsSpdbInput::setArchive(const string &url,
			    const time_t start_time,
			    const time_t end_time)

{

  _clearErrStr();

  _urlStr = url;
  _mode = ARCHIVE_MODE;
  _startTime = start_time;
  _endTime = end_time;

  // get spdb refs without chunks

  if (_spdb.getInterval(_urlStr, start_time, end_time, 0, 0, true)) {
    _errStr += "ERROR - COMM - DsSpdbInput::setArchive\n";
    _addStrErr("  URL: ", url);
    _errStr += _spdb.getErrStr();
    return -1;
  }

  // compile the archive list

  _archiveList.clear();
  const vector<Spdb::chunk_t> &chunks = _spdb.getChunks();
  time_t prev = -1;
  if (chunks.size() > 0) {
    prev = chunks[0].valid_time - 1;
  }
  for (size_t ii = 0; ii < chunks.size(); ii++) {
    if (chunks[ii].valid_time != prev) {
      _archiveList.push_back(chunks[ii].valid_time);
      prev = chunks[ii].valid_time;
    }
  }

  reset();

  return 0;
  
}

//////////////////////////////
// Set realtime mode.
//
//  url: URL for which the data times are required.
//
//  max_valid_age: the max valid age for data (secs)
//     The object will not return data which has not arrived
//     within this period.
//
//  heartbeat_func: pointer to heartbeat_func.
//    If NULL this is ignored.
//    If non-NULL, this is called once per delay_msecs while
//    the routine is polling for new data.
//
//  delay_msecs
//     polling delay in millisecs.
//     The object will sleep for this time between polling attempts.
//     If delay_msecs is negative, getNext() will not block.
//     If delay_msecs is non-negative but less than 1000, it is set
//       to 1000 to keep the network from being saturated with
//       DataMapper messages.
//
// Returns 0 on success, -1 on error.
// Use getErrStr() for error message.

int DsSpdbInput::setRealtime(const string &url,
			     const int max_valid_age,
			     const heartbeat_t heartbeat_func /* = NULL*/,
			     const int delay_msecs /* = 5000*/ )
{

  _urlStr = url;
  _dsUrl = url;
  _mode = REALTIME_MODE;
  _maxRealtimeAge = max_valid_age;
  _delayMsecs = delay_msecs;
  _heartbeatFunc = heartbeat_func;
  _ldata.setDirFromUrl(_dsUrl);
  _prevTime = 0;
    
  // Make sure the value for _delayMsecs is at least a second
  // to try to keep the network from getting saturated.

  if (_delayMsecs >= 0 && _delayMsecs < 1000)
    _delayMsecs = 1000;
  
  // If possible, initialize the prev time from DsLdataInfo

  if (_ldata.readForced() == 0) {
    return 0;
  }

  // Otherwise initialize the prev time from the data base

  if (_spdb.getTimes(_urlStr) == 0) {
    _prevTime = _spdb.getLastValidTime();
  }

  return 0;

}

////////////////////////////////
// get next available data time
//
// In archive mode, gets the next time in the list,
// returns error when list is exhausted.
//
// In realtime mode, if delay_secs is non-negative
// blocks until a new data time is available.
// If delay_msecs is negative, does not block. Check
// getNextSuccess() to see if data is available.
//
// Returns 0 on success, -1 on failure.

int DsSpdbInput::getNext(time_t &next_time)
{
  
  _clearErrStr();
  _nextSuccess = false;
  next_time = 0;

  if (_mode == NO_MODE) {
    _errStr += "ERROR - COMM - DsSpdbInput::getNext\n";
    _errStr += "  Mode must be set to ARCHIVE or REALTIME.\n";
    return -1;
  }

  if (_mode == ARCHIVE_MODE) {
    return _getNextArchive(next_time);
  } else {
    return _getNextRealtime(next_time);
  }
}

////////////////////////////////////
// get time of latest data to arrive
//
// Realtime mode only
//
// Returns 0 on success, -1 on failure.

int DsSpdbInput::getLatest(time_t &latest_time)

{
  _clearErrStr();
  if (_mode != REALTIME_MODE) {
    _errStr += "ERROR - COMM - DsSpdbInput::getLatest\n";
    _errStr += "  Only valid in realtime mode.\n";
     return -1;
  }
  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("DsSpdbInput::getLatest");
  }

  // try DsLdataInfo
  
  if (_ldata.readForced() == 0) {

    latest_time = _ldata.getLatestValidTime();

  } else {

    if (getLast(latest_time)) {
      _clearErrStr();
      _errStr += "ERROR - COMM - DsSpdbInput::getLatest\n";
      _errStr += "  No latest data info available from server.\n";
      _addStrErr("  URL: ", _urlStr);
      return -1;
    }
    
  }

  return 0;

}

//////////////////////////////////////
// get last available time in data set
//
// Returns 0 on success, -1 on failure.

int DsSpdbInput::getLast(time_t &last_time)

{
  _clearErrStr();
  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("DsSpdbInput::getLast");
  }
  if (_spdb.getTimes(_urlStr) == 0) {
    last_time = _spdb.getLastValidTime();
    return 0;
  } else {
    _errStr += "ERROR - COMM - DsSpdbInput::getLast\n";
    _errStr += "  No last data info available from server.\n";
    _addStrErr("  URL: ", _urlStr);
  }
  return -1;
}

//////////////////////////////////////////////////////
// get new data time if new data has arrived since the
// time passed in.
//
// Realtime mode only
//
// Returns 0 on success, -1 on failure.

int DsSpdbInput::getNew(time_t previous_time, time_t &new_time)

{
  _clearErrStr();
  if (_mode != REALTIME_MODE) {
    _errStr += "ERROR - COMM - DsSpdbInput::getNew\n";
    _errStr += "  Only valid in realtime mode.\n";
     return -1;
  }
  if (_heartbeatFunc != NULL) {
    _heartbeatFunc("DsSpdbInput::getLatest");
  }
  if (_spdb.getTimes(_urlStr) == 0) {
    new_time = _spdb.getLastValidTime();
    return 0;
  } else {
    _errStr += "ERROR - COMM - DsSpdbInput::getLatest\n";
    _errStr += "  No new data available from server.\n";
    _addStrErr("  URL: ", _urlStr);
  }
  return -1;
}

/////////////////////////
// reset to start of list
// 
// Archive mode only.

void DsSpdbInput::reset()

{
  _archivePtr = 0;
}

//////////////////////////////////////////////
// check whether we are at the end of the data

bool DsSpdbInput::endOfData() const

{
  if (_mode == REALTIME_MODE) {
    return false;
  } else {
    if (_archivePtr >= _archiveList.size()) {
      return true;
    } else {
      return false;
    }
  }
}

///////////////////////////
// print the object

void DsSpdbInput::print(ostream &out) const

{

  out << "DsSpdbInput" << endl;
  out << "===========" << endl;

  if (_mode == NO_MODE) {
    out << "No mode set yet - object empty" << endl;
    return;
  }

  if (_mode == ARCHIVE_MODE) {
    cerr << "Mode: ARCHIVE" << endl;
  } else {
    cerr << "Mode: REALTIME" << endl;
  }

  cerr << "Url: " << _urlStr << endl;

  if (_mode == ARCHIVE_MODE) {

    out << "  Start time: " << DateTime::str(_startTime) << endl;
    out << "  End time: " << DateTime::str(_endTime) << endl;
    out << "  Current posn: " << _archivePtr << endl;
    for (size_t ii = 0; ii < _archiveList.size(); ii++) {
      out << "    Time " << ii << ": "
	  << DateTime::str(_archiveList[ii]) << endl;
    }
    
  } // ARCHIVE_MODE
  
  if (_mode == REALTIME_MODE) {

    out << "  maxRealtimeAge: " << _maxRealtimeAge << endl;
    out << "  delayMsecs: " << _delayMsecs << endl;
    out << "  prevTime: " << DateTime::str(_prevTime) << endl;
    
  } // REALTIME_MODE
  

}

//////////////////////////
// getNext() archive mode

int DsSpdbInput::_getNextArchive(time_t &next_time)

{

  if (_archivePtr >= _archiveList.size()) {
    _errStr += "ERROR - COMM - DsSpdbInput::getNext\n";
    _errStr += "  Archive mode - List exhausted.\n";
    _addStrErr("  URL: ", _urlStr);
    return -1;
  } else {
    next_time = _archiveList[_archivePtr];
    _archivePtr++;
  }

  return 0;

}

////////////////////////////////
// getNext() realtime mode

int DsSpdbInput::_getNextRealtime(time_t &next_time)
{
  while (true) {
    
    if (_heartbeatFunc != NULL) {
      _heartbeatFunc("DsSpdbInput::getNext");
    }
    
    // try DsLdataInfo
    
    if (_ldata.readForced(-1, false) == 0) {
      
      if (_ldata.read(_maxRealtimeAge) == 0) {
	_nextSuccess = true;
	next_time = _ldata.getLatestValidTime();
	return 0;
      }
      
    } else {
      
      // try DsSpdb instead
      
      if (_spdb.getTimes(_urlStr) == 0) {
	time_t latestTime = _spdb.getLastValidTime();
	if (latestTime > _prevTime) {
	  _prevTime = latestTime;
	  next_time = latestTime;
	  _nextSuccess = true;
	  return 0;
	}
      } // if (_spdb.getTimes(_urlStr) == 0)

    }
    
    // return immediately if delay negative
    
    if (_delayMsecs < 0) {
      return 0;
    }
    
    // sleep
    
    if (_delayMsecs <= 1000) {
      PMU_auto_register("DsSpdbInput::getNext zzzz");
      umsleep(_delayMsecs);
    } else {
      int msecsLeft = _delayMsecs;
      while (msecsLeft > 0) {
	PMU_auto_register("DsSpdbInput::getNext zzzz");
	umsleep(1000);
	msecsLeft -= 1000;
      }
    }
    
  } // while (true)

  return 0;

}

/////////////////////////////////////
// add error string with int argument

void DsSpdbInput::_addIntErr(const char *err_str, const int iarg)
{
  _errStr += err_str;
  char str[32];
  sprintf(str, "%d\n", iarg);
  _errStr += str;
}

////////////////////////////////////////
// add error string with string argument

void DsSpdbInput::_addStrErr(const char *err_str, const string &sarg)
{
  _errStr += err_str;
  _errStr += sarg;
  _errStr += "\n";
}

