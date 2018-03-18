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
// DsSpdbThreaded.hh
//
// DsSpdbThreaded object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////
//
// The DsSpdbThreaded adds threading for get() methods
// to the DsSpdb class.
//
///////////////////////////////////////////////////////////////

#ifndef DsSpdbThreaded_HH
#define DsSpdbThreaded_HH

#include <Spdb/DsSpdb.hh>
#include <pthread.h>
using namespace std;

class DsSpdbMsg;

///////////////////////////////////////////////////////////////
// class definition

class DsSpdbThreaded : public DsSpdb

{

public:
  
  // constructor

  DsSpdbThreaded();

  // destructor
  
  virtual ~DsSpdbThreaded();

  // Overload get methods. See DsSpdb.hh for more details

  // In this threaded implemetation, these calls spawn a thread to
  // get the data from the server or local file. The call returns an
  // error if the thread cannot be created, and the error string is
  // loaded up accordingly.
  //
  // If the call returns successfully, the thread has been created to
  // perform the read and write. You need to monitor the progress,
  // completion and error condition of the read or write, using calls
  // from the main thread.
  //
  // The relevant calls are:
  //   
  //   getThreadDone(): returns true when read/write is done
  //   getThreadRetVal(): get error return from read/write
  //   getNbytesExpected(): get number of bytes expected to be read
  //   getNbytesDone(): get number of read bytes received so far
  //   getPercentComplete(): get percentage of read completed so far
  //
  // If the get call is taking too long, you can cancel it by
  // making a call to cancelThread().
  // If the cancel call interrupts the get in progress,
  // the threadRetVal will be -1. If the get was
  // successful in spite of the cancellation, the retval will be 0.

  ///////////////////////////////////////////////////////////////////
  // getExact() - overrides Spdb
  //
  // Get data at exactly the given time.
  //

  virtual int getExact(const string &url_str,
		       const time_t request_time,
		       const int data_type = 0,
		       const int data_type2 = 0,
		       const bool get_refs_only = false,
		       const bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getClosest() - overrides Spdb
  //
  // Get data closest to the given time, within the time margin.
  //

  virtual int getClosest(const string &url_str,
			 const time_t request_time,
			 const int time_margin,
			 const int data_type = 0,
			 const int data_type2 = 0,
			 const bool get_refs_only = false,
			 const bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getInterval() - overrides Spdb
  //
  // Get data in the time interval.
  //
  
  virtual int getInterval(const string &url_str,
			  const time_t start_time,
			  const time_t end_time,
			  const int data_type = 0,
			  const int data_type2 = 0,
			  const bool get_refs_only = false,
			  const bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getValid() - overrides Spdb
  //
  // Get data valid at the given time.
  //

  virtual int getValid(const string &url_str,
		       const time_t request_time,
		       const int data_type = 0,
		       const int data_type2 = 0,
		       const bool get_refs_only = false,
		       const bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getLatest() - overrides Spdb
  //
  // Get latest data, within the given time margin. This is the
  // same as getInterval(latestTime - margin, latestTime).
  //

  virtual int getLatest(const string &url_str,
			const int time_margin = 0,
			const int data_type = 0,
			const int data_type2 = 0,
			const bool get_refs_only = false,
			const bool respect_zero_types = false);

  ///////////////////////////////////////////////////////////////////
  // getFirstBefore() - overrides Spdb
  //
  // Get first data at or before the requested time,
  // within the given time margin.
  //

  virtual int getFirstBefore(const string &url_str,
			     const time_t request_time,
			     const int time_margin,
			     const int data_type = 0,
			     const int data_type2 = 0,
			     const bool get_refs_only = false,
			     const bool respect_zero_types = false);
  
  ///////////////////////////////////////////////////////////////////
  // getFirstAfter() - overrides Spdb
  //
  // Get first data at or after the requested time,
  // within the given time margin.
  //

  virtual int getFirstAfter(const string &url_str,
			    const time_t request_time,
			    const int time_margin,
			    const int data_type = 0,
			    const int data_type2 = 0,
			    const bool get_refs_only = false,
			    const bool respect_zero_types = false);

  ////////////////////////////////////////////////////////////
  // get the first, last and last_valid_time in the data base
  // Use getFirstTime(), getLastTime() and getLastValidTime()
  // to access the times after making this call.
  //
  // Note that if you use the other version of getTimes() you
  // will get single-threaded behavior.

  virtual int getTimes(const string &url_str);

  //////////////////////////////////////////////////////////
  // compile time list - overrides DsSpdb
  //
  // Compile a list of available data times at the specified
  // url between the start and end times.
  //
  // The optional minimum_interval arg specifies the minimum
  // interval in secs between times in the time list. This
  // allows you to cull the list to suit your needs.
  // minimum_interval default to 1 sec, which will return a 
  // full list with no duplicates. If you set to to 0, you
  // will get duplicates if there is more than one entry at
  // the same time. If you set it to > 1, you will cull the
  // list to the required sparseness.
  //
  // Returns 0 on success, -1 on failure
  // getErrStr() retrieves the error string.
  //
  // After a successful call to compileTimeList(), access the
  // time list via the following functions:
  //   getTimeList(): vector of time_t
  //   getNTimesInList(): n entries in list
  //   getTimeFromList(int i): time_t from list
  
  virtual int compileTimeList(const string &url_str,
			      time_t start_time,
			      time_t end_time,
			      size_t minimum_interval = 1);
  
  // access to the threading details

  bool getThreadDone();
  int getThreadRetVal() { return (_threadRetVal); }
  int getNbytesExpected() { return (_nbytesExpected); }
  ssize_t getNbytesDone() { return (_nbytesDone); }
  double getPercentComplete();
  void cancelThread();

  // For debugging it can be useful to turn the threading off.
  // This should not normally be used, since it defeats the purpose
  // of the class.
  
  void setThreadingOff();

protected:

  //  threading

  bool _threadsOn; // turned off for for debugging only
  bool _threadDone;
  int _threadRetVal;
  ssize_t _nbytesExpected;
  ssize_t _nbytesDone;
  pthread_mutex_t _mutex;
  pthread_t _thread;

  // args for threaded functions

  string _urlStr;
  time_t _startTime;
  time_t _endTime;
  time_t _requestTime;
  time_t _minimumInterval;
  int _timeMargin;
  int _dataType;
  int _dataType2;
  bool _getRefsOnly;
  bool _respectZeroTypes;

  // overload communicate

  int _communicateGet(DsSpdbMsg &msg,
                      void *buf,
                      int buflen,
                      DsURL &url);

  int _prepareThread();
  void _tidyThread(int ret_val);

  void _getExactThreadRun();
  void _getClosestThreadRun();
  void _getIntervalThreadRun();
  void _getValidThreadRun();
  void _getLatestThreadRun();
  void _getFirstBeforeThreadRun();
  void _getFirstAfterThreadRun();
  void _getTimesThreadRun();
  void _compileTimeListThreadRun();

  // Static functions for thread creation.

  static void *_getExactThreadEntry(void *args);
  static void *_getClosestThreadEntry(void *args);
  static void *_getIntervalThreadEntry(void *args);
  static void *_getValidThreadEntry(void *args);
  static void *_getLatestThreadEntry(void *args);
  static void *_getFirstBeforeThreadEntry(void *args);
  static void *_getFirstAfterThreadEntry(void *args);
  static void *_getTimesThreadEntry(void *args);
  static void *_compileTimeListThreadEntry(void *args);

  // block signal in thread

  static void _blockSignal(int isig) ;
  
  // cleanup after thread is cancelled

  static void _cleanupThread(void *args);

private:

};

#endif


