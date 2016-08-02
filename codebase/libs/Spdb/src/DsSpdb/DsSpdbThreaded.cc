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
// DsSpdbThreaded.cc
//
// DsSpdbThreaded object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 2000
//
///////////////////////////////////////////////////////////////
//
// The DsSpdbThreaded adds threading to the DsSpdb class.
//
///////////////////////////////////////////////////////////////

#include <Spdb/DsSpdbThreaded.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaStr.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsThreadedClient.hh>
#include <didss/RapDataDir.hh>
#include <cerrno>
#include <csignal>
using namespace std;

DsSpdbThreaded::DsSpdbThreaded() : DsSpdb()
{
  pthread_mutex_init(&_mutex, NULL);
  _threadsOn = true;
}

DsSpdbThreaded::~DsSpdbThreaded()
{
}

//////////////////////////////////////////////////////////////////
// set threading off for debugging
// This should not normally be used, since it defeats the puspose
// of the class.

void DsSpdbThreaded::setThreadingOff()
{
  _threadsOn = false;
}

//////////////////////////////////
// override getExact() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getExact(const string &url_str,
			     const time_t request_time,
			     const int data_type /* = 0*/,
			     const int data_type2 /* = 0*/,
			     const bool get_refs_only /* = false*/,
			     const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _requestTime = request_time;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getExact\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getExact()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
    
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getExactThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getExact\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getExact(_urlStr, _requestTime,
				_dataType, _dataType2,
				_getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getClosest() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getClosest(const string &url_str,
			       const time_t request_time,
			       const int time_margin,
			       const int data_type /* = 0*/,
			       const int data_type2 /* = 0*/,
			       const bool get_refs_only /* = false*/,
			       const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _requestTime = request_time;
  _timeMargin = time_margin;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getClosest\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getClosest()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getClosestThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getClosest\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getClosest(_urlStr, _requestTime, _timeMargin,
				  _dataType, _dataType2, 
				  _getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getInterval() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getInterval(const string &url_str,
				const time_t start_time,
				const time_t end_time,
				const int data_type /* = 0*/,
				const int data_type2 /* = 0*/,
				const bool get_refs_only /* = false*/,
				const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _startTime = start_time;
  _endTime = end_time;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;
  
  _errStr = "ERROR - COMM - DsSpdbThreaded::getInterval\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getInterval()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getIntervalThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getInterval\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getInterval(_urlStr, _startTime, _endTime,
				   _dataType, _dataType2,
				   _getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getValid() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getValid(const string &url_str,
			     const time_t request_time,
			     const int data_type /* = 0*/,
			     const int data_type2 /* = 0*/,
			     const bool get_refs_only /* = false*/,
			     const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _requestTime = request_time;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getValid\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getValid()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
    
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getValidThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getValid\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getValid(_urlStr, _requestTime,
				_dataType, _dataType2,
				_getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getLatest() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getLatest(const string &url_str,
			      const int time_margin,
			      const int data_type /* = 0*/,
			      const int data_type2 /* = 0*/,
			      const bool get_refs_only /* = false*/,
			      const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _timeMargin = time_margin;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getLatest\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getLatest()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getLatestThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getLatest\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getLatest(_urlStr, _timeMargin,
				 _dataType, _dataType2,
				 _getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getFirstBefore() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getFirstBefore(const string &url_str,
				   const time_t request_time,
				   const int time_margin,
				   const int data_type /* = 0*/,
				   const int data_type2 /* = 0*/,
				   const bool get_refs_only /* = false*/,
				   const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _requestTime = request_time;
  _timeMargin = time_margin;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getFirstBefore\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getFirstBefore()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getFirstBeforeThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getFirstBefore\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getFirstBefore(_urlStr, _requestTime, _timeMargin,
				      _dataType, _dataType2,
				      _getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getFirstAfter() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getFirstAfter(const string &url_str,
				  const time_t request_time,
				  const int time_margin,
				  const int data_type /* = 0*/,
				  const int data_type2 /* = 0*/,
				  const bool get_refs_only /* = false*/,
				  const bool respect_zero_types /* = false*/ )
  
{

  _urlStr = url_str;
  _requestTime = request_time;
  _timeMargin = time_margin;
  _dataType = data_type;
  _dataType2 = data_type2;
  _getRefsOnly = get_refs_only;
  _respectZeroTypes = respect_zero_types;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getFirstAfter\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getFirstAfter()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getFirstAfterThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getFirstAfter\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getFirstAfter(_urlStr, _requestTime, _timeMargin,
				     _dataType, _dataType2,
				     _getRefsOnly, _respectZeroTypes);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override getTimes() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::getTimes(const string &url_str)
  
{

  _urlStr = url_str;

  _errStr = "ERROR - COMM - DsSpdbThreaded::getTimes\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::getTimes()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _getTimesThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::getTimes\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::getTimes(_urlStr);
    _tidyThread(iret);
  }
 
  return 0;

}

///////////////////////////////////////////////////
// override compileTimeList() method
//
// See DsSpdbThreaded.hh on how to use this function.

int DsSpdbThreaded::compileTimeList(const string &url_str,
				    time_t start_time,
				    time_t end_time,
				    size_t minimum_interval /* = 1*/ )
  
{

  _urlStr = url_str;
  _startTime = start_time;
  _endTime = end_time;
  _minimumInterval = minimum_interval;
  
  _errStr = "ERROR - COMM - DsSpdbThreaded::compileTimeList\n";
  
  if (_prepareThread()) {
    cerr << "ERROR - DsSpdbThreaded::compileTimeList()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _compileTimeListThreadEntry, this)) {
      _errStr += "ERROR - DsSpdbThreaded::compileTimeList\n";
      _errStr += "  Cannot create thread.\n";
      TaStr::AddStr(_errStr, "  URL: ", url_str);
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsSpdb::compileTimeList(_urlStr,
				       _startTime, _endTime,
				       _minimumInterval);
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////////////
// prepare for thread

int DsSpdbThreaded::_prepareThread()

{
  if (_threadsOn) {
    if (pthread_mutex_trylock(&_mutex) == EBUSY) {
      cerr << "ERROR - COMM - DsSpdbThreaded::_prepareThread" << endl;
      cerr << "  Cannot lock mutex - already in use." << endl;
      cerr << "  " << DateTime::str() << endl;
      return -1;
    }
  }
  _threadDone = false;
  _threadRetVal = -1;
  _nbytesExpected = 0;
  _nbytesDone = 0;
  return 0;
}

//////////////////////////////////////////
// tidy up after thread is done

void DsSpdbThreaded::_tidyThread(int ret_val)

{
  _threadRetVal = ret_val;
  _threadDone = true;
  _sock.close();
  _sock.freeData();
  if (_threadsOn) {
    pthread_mutex_unlock(&_mutex);
  }
}

///////////////////////////////////////////////
// cleanup after thread is cancelled

void DsSpdbThreaded::_cleanupThread(void *args)
  
{
  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_tidyThread(-1);
}

///////////////////////////////////
// get percentage of read complete

double DsSpdbThreaded::getPercentComplete()

{

  if (_threadDone) {
    return 100.0;
  }

  if (_nbytesExpected == 0 || _nbytesDone == 0) {
    return 0.0;
  }
  
  return (((double) _nbytesDone * 100.0) /
	  (double) _nbytesExpected);
  
}

////////////////////////////////////
// check if thread is done
// Join thread if it is done.

bool DsSpdbThreaded::getThreadDone()

{
  if (_threadDone) {
    if (_threadsOn) {
      pthread_join(_thread, NULL);
    }
  }
  return (_threadDone);
}

///////////////////////////
// cancel thread

void DsSpdbThreaded::cancelThread()

{
  _errStr += "INFO - DsSpdbThreaded - thread cancelled\n";
  if (_threadsOn) {
    pthread_cancel(_thread);
    pthread_join(_thread, NULL);
  }
}

/////////////////////////////////////////////
// communicate with server
//
// This involves opening the socket, writing the
// request message, receiving the reply and 
// disassembling the reply.
//
// Returns 0 on success, -1 on error.

int DsSpdbThreaded::_communicateGet(DsSpdbMsg &msg,
				    void *buf,
				    int buflen,
				    DsURL &url)
  
{

  // enable thread cancellation

  int last_state, last_type;
  if (_threadsOn) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
  }

  // contact server

  DsThreadedClient client;
  if (_debug) {
    client.setDebug(true);
  }
  client.setErrStr("ERROR - DsSpdb::_communicate\n");
  
  if (client.communicateAutoFwd(url, DsSpdbMsg::DS_MESSAGE_TYPE_SPDB,
				buf, buflen,
				_nbytesExpected, _nbytesDone,
				_nbytesExpected, _nbytesDone)) {
    _errStr += client.getErrStr();
    TaStr::AddStr(_errStr, "  URL: ", url.getURLStr());
    if (_threadsOn) {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
      return -1;
    }
  }
  
  // disable thread cancellation

  if (_threadsOn) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
  }

  // disassemble the reply
  
  if (msg.disassemble(client.getReplyBuf(), client.getReplyLen())) {
    _errStr += "  Invalid reply - cannot disassemble.\n";
    TaStr::AddStr(_errStr, "  URL: ", url.getURLStr());
    return-1;
  }
  
  return 0;

}

/////////////////////////////////////////////////
// function for getExact thread entry point

void *DsSpdbThreaded::_getExactThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getExactThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getExact in thread

void DsSpdbThreaded::_getExactThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getExact(_urlStr, _requestTime,
			      _dataType, _dataType2,
			      _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getClosest thread entry point

void *DsSpdbThreaded::_getClosestThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getClosestThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getClosest in thread

void DsSpdbThreaded::_getClosestThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getClosest(_urlStr, _requestTime, _timeMargin,
				_dataType, _dataType2,
				_getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getInterval thread entry point

void *DsSpdbThreaded::_getIntervalThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getIntervalThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getInterval in thread

void DsSpdbThreaded::_getIntervalThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getInterval(_urlStr, _startTime, _endTime,
				 _dataType, _dataType2,
				 _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getValid thread entry point

void *DsSpdbThreaded::_getValidThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getValidThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getValid in thread

void DsSpdbThreaded::_getValidThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getValid(_urlStr, _requestTime,
			      _dataType, _dataType2,
			      _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getLatest thread entry point

void *DsSpdbThreaded::_getLatestThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getLatestThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getLatest in thread

void DsSpdbThreaded::_getLatestThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getLatest(_urlStr, _timeMargin,
			       _dataType, _dataType2,
			       _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getFirstBefore thread entry point

void *DsSpdbThreaded::_getFirstBeforeThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getFirstBeforeThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getFirstBefore in thread

void DsSpdbThreaded::_getFirstBeforeThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getFirstBefore(_urlStr, _requestTime, _timeMargin,
				    _dataType, _dataType2,
				    _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getFirstAfter thread entry point

void *DsSpdbThreaded::_getFirstAfterThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getFirstAfterThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getFirstAfter in thread

void DsSpdbThreaded::_getFirstAfterThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getFirstAfter(_urlStr, _requestTime, _timeMargin,
				   _dataType, _dataType2,
				   _getRefsOnly, _respectZeroTypes);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for getTimes thread entry point

void *DsSpdbThreaded::_getTimesThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsSpdbThreaded object which spawned the thread

  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_getTimesThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running getTimes in thread

void DsSpdbThreaded::_getTimesThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::getTimes(_urlStr);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for compileTimeList thread entry point

void *DsSpdbThreaded::_compileTimeListThreadEntry(void *args)
  
{
  
  // block SIGALRM
  
  _blockSignal(SIGALRM);
  
  // disable cancelling for the moment
  
  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
  
  // args points to the DsSpdbThreaded object which spawned the thread
  
  DsSpdbThreaded *spdb = (DsSpdbThreaded *) args;
  spdb->_compileTimeListThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running compileTimeList in thread

void DsSpdbThreaded::_compileTimeListThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsSpdb::compileTimeList(_urlStr, _startTime, _endTime,
				     _minimumInterval);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// block signals in the threads

void DsSpdbThreaded::_blockSignal(int isig)

{

  // get current signal mask
  sigset_t oldset;
  pthread_sigmask(0, NULL, &oldset);

  // block it if it is in the set
  if (!sigismember(&oldset, isig)) {
    sigset_t newset;
    sigemptyset(&newset);
    sigaddset(&newset, isig);
    pthread_sigmask(SIG_BLOCK, &newset, &oldset);
  }

}

