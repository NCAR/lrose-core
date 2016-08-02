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
// DsSpdbInput.hh
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
/////////////////////////////////////////////////////////////

#ifndef DsSpdbInput_HH
#define DsSpdbInput_HH

#include <string>
#include <vector>
#include <iostream>
#include <Spdb/DsSpdb.hh>
#include <didss/DsURL.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

class DsSpdbInput {
  
public:

  typedef void (*heartbeat_t)(const char *label);

  //////////////
  // Constructor
  //

  DsSpdbInput ();

  /////////////
  // destructor
  
  virtual ~DsSpdbInput();

  /////////////////////////////
  // Set archive mode.
  //
  // Load up archive time list.
  // The list will contain all available data times
  // between the start and end times for the given URL.
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.
  
  int setArchive(const string &url,
		 const time_t start_time,
		 const time_t end_time);
  
  //////////////////////////////
  // Set realtime mode.
  //
  //  url: URL for which the data times are required.
  //
  //  max_valid_age: the max valid age for data (secs)
  //     The object will not return data which has not arrived
  //     within this period. (NOTE: NOT IMPLEMENTED YET)
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
  //     Check getNextSuccess() to see if data is available.
  
  int setRealtime(const string &url,
		  const int max_valid_age,
		  const heartbeat_t heartbeat_func = NULL,
		  const int delay_msecs = 5000);

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

  int getNext(time_t &next_time);

  bool getNextSuccess() { return(_nextSuccess); }

  /////////////////////////////////
  // get latest available data time
  //
  // Realtime mode only
  //
  // Returns 0 on success, -1 on failure.
  
  int getLatest(time_t &latest_time);
  
  //////////////////////////////////////
  // get last available time in data set
  //
  // Returns 0 on success, -1 on failure.
  
  int getLast(time_t &last_time);

  //////////////////////////////////////////////////////
  // get new data time if new data has arrived since the
  // time passed in.
  //
  // Realtime mode only
  //
  // Returns 0 on success, -1 on failure.

  int getNew(time_t previous_time, time_t &new_time);
  
  /////////////////////////
  // reset to start of list
  // 
  // Archive mode only.

  void reset();

  /////////////////////////
  // get vector of times
  // 
  // Archive mode only.

  const vector<time_t> &getArchiveList() const { return _archiveList; }

  //////////////////////////////////////////////
  // check whether we are at the end of the data

  bool endOfData() const;

  // Get the Error String. This has contents when an error is returned.

  string getErrStr() const { return _errStr; }

  // print the object

  void print(ostream &out) const;

protected:

  typedef enum {
    NO_MODE,
    ARCHIVE_MODE,
    REALTIME_MODE
  } mode_t;
  
  string _errStr;
  string _urlStr;
  DsURL _dsUrl;
  mode_t _mode;
  DmapAccess _dmapAccess;

  time_t _startTime;
  time_t _endTime;
  vector<time_t> _archiveList;
  size_t _archivePtr;
  
  time_t _prevTime;
  int _maxRealtimeAge;
  int _delayMsecs;

  bool _nextSuccess;

  heartbeat_t _heartbeatFunc;

  DsLdataInfo _ldata;
  DsSpdb _spdb;

  void _clearErrStr() { _errStr = ""; }
  int _getNextArchive(time_t &next_time);
  int _getNextRealtime(time_t &next_time);
  void _addIntErr(const char *err_str, const int iarg);
  void _addStrErr(const char *err_str, const string &sarg);

private:

};

#endif
