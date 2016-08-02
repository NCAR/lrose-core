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
// DsMdvxInput.hh
//
// Class to provide input control for Mdv data.
//
// There are three modes of operation:
//
// Realtime mode:
//   The constructor passes in a URL to watch, as well as
//   as max valid age and a heartbeat function to be called while
//   waiting for new data to arrive.
//   When requested to read, the object will watch for new data.
//   When new data arrives it is read. If the heartbeat_func is not NULL,
//   it is called while polling continues.
//
// Archive mode:
//   The constructor passes in a URL and a start and end time.
//   A list of available times is compiled and stored.
//   When requested to read, the next data time is used.
//   The endOfData flag is set when the data is exhausted.
//
// Filelist mode:
//   The constructor passes in a vector of input paths.
//   When requested to read, the next path is used.
//   The endOfData flag is set when the data is exhausted.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2000
//
/////////////////////////////////////////////////////////////

#ifndef DsMdvxInput_HH
#define DsMdvxInput_HH

#include <string>
#include <vector>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh>
using namespace std;

class DsMdvxInput {
  
public:

  typedef void (*heartbeat_t)(const char *label);

  //////////////
  // Constructor
  //

  DsMdvxInput ();

  /////////////
  // destructor
  
  virtual ~DsMdvxInput();

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
  
  /////////////////////////////
  // Set archive fcst mode. 
  //
  // Load up archive fcst time list.
  // The list will contain all available data times
  // between the start and end times for the given URL.
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.

  int setArchiveFcst(const string &url,
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
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.
  
  int setRealtime(const string &url,
		  const int max_valid_age,
		  const heartbeat_t heartbeat_func = NULL,
		  const int delay_msecs = 5000);

  //////////////////////////////////////////////////////////////////////
  // Set filelist mode.
  //
  //  input_file_list: list of files through which to iterate for reads.
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.
  
  int setFilelist(const vector<string> &input_file_list);

  //////////////////////////////////////////////////////////////////////
  // set the time margin for reading MDV data
  // this defaults to 0 for most purposes, but sometimes the data
  // is not well behaved so we need to widen the search margin

  void setSearchMarginSecs(int val) { _searchMarginSecs = val; }
  
  //////////////////////////////////////////////////////////////////////
  // Get the data time of the current data set
  //

  time_t getDataTime(){ return _dataTime; }
  time_t getForecastTime(){ return _forecastTime; }

  //////////////////////////////////////////////////////////////////////
  // Get the data time from a filepath
  //
  // NOTE: this is a static utility method which can be invoked
  //       without instantiating a DsMdvxInput class,
  //       thus cannot be used with getErrStr()
  //
  //  path: file path to parse for determining data time
  //  dataTime: returned utime parsed from the data set path
  //
  // Returns 0 on success, -1 on error.

  static int getDataTime( const string& path, time_t& dataTime );

  ///////////////////////////////////////////////////////////
  // Read the next data set, using the DsMdvx object provided.
  //
  // In ARCHIVE mode, reads data for the next time in the list.
  // In FILELIST mode, reads data from the next path in the list.
  // In REALTIME mode, blocks until a new data time is available.
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.
  
  int readAllHeadersNext(DsMdvx &mdvx);
  int readVolumeNext(DsMdvx &mdvx);
  int readVsectionNext(DsMdvx &mdvx);
  

  ///////////////////////////////////////////////////////////
  // Read the next data set, using the DsMdvx object provided,
  // for the case of REALTIME mode, using the max_valid_age value
  // that was provided in the setRealtime method. The readVolumeNext
  // method does not use that value, it requires an exact time match
  //
  // Returns 0 on success, -1 on error.
  // Use getErrStr() for error message.
  //
  // If the mode is not REALTIME, returns -1
  //
  int readVolumeNextWithMaxValidAge(DsMdvx &mdvx);

  ///////////////////////////////////////////////////////////
  // Get the next time.
  //
  // Passes the request down to the DsMdvxTimes object.
  //
  // Only available in ARCHIVE and REALTIME modes.
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
  
  int getTimeNext(time_t &next_time);
  
  /////////////////////////
  // reset to start of list
  // 
  // ARCHIVE and FILELIST modes only.
  
  void reset();

  //////////////////////////////////////////////
  // check whether we are at the end of the data
  
  bool endOfData() const;
  
  // Get the Error String. This has contents when an error is returned.

  string getErrStr() const { return _errStr; }

protected:

  typedef enum {
    NO_MODE,
    ARCHIVE_MODE,
    ARCHIVE_FCST_MODE,
    REALTIME_MODE,
    FILELIST_MODE
  } mode_t;
  
  string _errStr;
  mode_t _mode;
  string _url;
  vector<string> _fileList;
  size_t _fileListPtr;
  DsMdvxTimes _times;
  time_t _dataTime;
  time_t _forecastTime;
  int _searchMarginSecs;
  
  void _clearErrStr();
  void _addIntErr(const char *err_str, const int iarg);
  void _addStrErr(const char *err_str, const string &sarg);

private:

  int _setMdvxInArchiveFcstMode(DsMdvx &mdvx);

};

#endif
