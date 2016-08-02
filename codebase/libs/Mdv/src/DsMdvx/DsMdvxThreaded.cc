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
// DsMdvxThreaded.cc
//
// DsMdvxThreaded object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 1999
//
///////////////////////////////////////////////////////////////
//
// The DsMdvxThreaded adds threading to the DsMdvx class.
//
///////////////////////////////////////////////////////////////

#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/DsMdvxThreaded.hh>
#include <Mdv/DsMdvxMsg.hh>
#include <dsserver/DsLocator.hh>
#include <dsserver/DsThreadedClient.hh>
#include <didss/RapDataDir.hh>
#include <cerrno>
#include <csignal>
using namespace std;

DsMdvxThreaded::DsMdvxThreaded() : DsMdvx()
{
  pthread_mutex_init(&_mutex, NULL);
  _threadsOn = true;
}

DsMdvxThreaded::~DsMdvxThreaded()
{
}

//////////////////////////////////////////////////////////////////
// set threading off for debugging
// This should not normally be used, since it defeats the puspose
// of the class.

void DsMdvxThreaded::setThreadingOff()
{
  _threadsOn = false;
}

//////////////////////////////////
// override readAllHeaders method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::readAllHeaders()
{

  clearErrStr();

  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::readAllHeaders()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
    
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _readAllHeadersThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::readAllHeaders\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::readAllHeaders();
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override readVolume method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::readVolume()
{

  clearErrStr();

  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::readVolume()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }

  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _readVolumeThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::readVolume\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::readVolume();
    _tidyThread(iret);
  }
  
  return (0);

}

//////////////////////////////////
// override readVsection method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::readVsection()
{

  clearErrStr();

  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::readVsection()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _readVsectionThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::readVsection\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::readVsection();
    _tidyThread(iret);
  }
 
  return 0;

}

//////////////////////////////////
// override writeToDir method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::writeToDir(const string &output_url)
{
  
  clearErrStr();
  
  _outputUrl = output_url;

  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::writeToDir()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _writeToDirThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::writeToDir\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::writeToDir(output_url);
    _tidyThread(iret);
  }
  
  return 0;
  
}

//////////////////////////////////
// override writeToPath method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::writeToPath(const string &output_url)
{

  clearErrStr();
  _outputUrl = output_url;
  
  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::writeToPath()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _writeToPathThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::writeToPath\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::writeToPath(output_url);
    _tidyThread(iret);
  }
  
  return 0;

}

//////////////////////////////////
// override compileTimeList method
//
// See DsMdvxThreaded.hh on how to use this function.

int DsMdvxThreaded::compileTimeList()
{

  clearErrStr();
  
  if (_prepareThread()) {
    cerr << "ERROR - DsMdvxThreaded::compileTimeList()" << endl;
    cerr << "  " << DateTime::str() << endl;
    return -1;
  }
  
  if (_threadsOn) {
    if (pthread_create(&_thread, NULL,
		       _compileTimeListThreadEntry, this)) {
      _errStr += "ERROR - DsMdvxThreaded::compileTimeList\n";
      _errStr += "  Cannot create thread.\n";
      pthread_mutex_unlock(&_mutex);
      return -1;
    }
  } else {
    int iret = DsMdvx::compileTimeList();
    _tidyThread(iret);
  }
  
  return 0;

}

//////////////////////////////////////////
// prepare for thread

int DsMdvxThreaded::_prepareThread()

{
  if (_threadsOn) {
    if (pthread_mutex_trylock(&_mutex) == EBUSY) {
      cerr << "ERROR - DsMdvxThreaded::_prepareThread" << endl;
      cerr << "  Cannot lock mutex - already in use." << endl;
      cerr << "  " << DateTime::str() << endl;
      return -1;
    }
  }
  _threadDone = false;
  _threadRetVal = -1;
  _nbytesReadExpected = 0;
  _nbytesReadDone = 0;
  _nbytesWriteExpected = 0;
  _nbytesWriteDone = 0;
  return 0;
}

//////////////////////////////////////////
// tidy up after thread is done

void DsMdvxThreaded::_tidyThread(int ret_val)

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

void DsMdvxThreaded::_cleanupThread(void *args)
  
{
  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_tidyThread(-1);
}

///////////////////////////////////
// get percentage of read complete

double DsMdvxThreaded::getPercentReadComplete()

{

  if (_threadDone) {
    return 100.0;
  }

  if (_nbytesReadExpected == 0 || _nbytesReadDone == 0) {
    return 0.0;
  }

  return (((double) _nbytesReadDone * 100.0) /
	  (double) _nbytesReadExpected);

}

///////////////////////////////////
// get percentage of write complete

double DsMdvxThreaded::getPercentWriteComplete()

{

  if (_threadDone) {
    return 100.0;
  }

  if (_nbytesWriteExpected == 0 || _nbytesWriteDone == 0) {
    return 0.0;
  }

  return (((double) _nbytesWriteDone * 100.0) /
	  (double) _nbytesWriteExpected);

}

////////////////////////////////////
// check if thread is done
// Join thread if it is done.

bool DsMdvxThreaded::getThreadDone()

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

void DsMdvxThreaded::cancelThread()

{
  _errStr += "INFO - DsMdvxThreaded - thread cancelled\n";
  if (_threadsOn) {
    pthread_cancel(_thread);
    pthread_join(_thread, NULL);
  }
}

/////////////////////////////////////////////
// Communicate with server
//  Uses http tunnel and proxy if required.
//
// Open the server's  socket. 
// Write the
// request message, receive and disassemble the reply.
// Add and strip Http headers if required.
//
// Returns 0 on success, -1 on error.

int DsMdvxThreaded::_communicate(DsURL &url,
				 DsMdvxMsg &msg,
				 const void *msgBuf,
				 const ssize_t msgLen)
  
{

  // enable thread cancellation
  
  int last_state, last_type;
  if (_threadsOn) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
  }

  // communicate with server

  DsThreadedClient client;
  client.setDebug(_debug);
  client.setErrStr("ERROR - DsMdvxThreaded::_communicate\n");
  
  if (client.communicateAutoFwd(url, DsMdvxMsg::MDVP_REQUEST_MESSAGE,
				msgBuf, msgLen,
				_nbytesWriteExpected, _nbytesWriteDone,
				_nbytesReadExpected, _nbytesReadDone)) {
    _errStr += client.getErrStr();
    if (_threadsOn) {
      pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
    }
    return -1;
  }
  
  // disable thread cancellation
  
  if (_threadsOn) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);
  }

  // disassemble the reply
  
  if (_debug) {
    cerr << "----> DsMdvxThreaded::_communicate() dissasembling reply" << endl;
  }
  
  if (msg.disassemble(client.getReplyBuf(), client.getReplyLen(), *this)) {
    _errStr += "ERROR - COMM - DsMdvx::_communicate msg.disassemble\n";
    _errStr += "Invalid reply - cannot disassemble\n";
    _errStr += msg.getErrStr();
    _errStr += "\n";
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////
// function for readAllHeaders thread entry point

void *DsMdvxThreaded::_readAllHeadersThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_readAllHeadersThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running readAllHeaders in thread

void DsMdvxThreaded::_readAllHeadersThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::readAllHeaders();
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for readVolume thread entry point

void *DsMdvxThreaded::_readVolumeThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_readVolumeThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running readVolume in thread

void DsMdvxThreaded::_readVolumeThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::readVolume();
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for readVsection thread entry point

void *DsMdvxThreaded::_readVsectionThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_readVsectionThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running readVsection in thread

void DsMdvxThreaded::_readVsectionThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::readVsection();
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for writeToDir thread entry point

void *DsMdvxThreaded::_writeToDirThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_writeToDirThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running writeToDir in thread

void DsMdvxThreaded::_writeToDirThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::writeToDir(_outputUrl);
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for writeToPath thread entry point

void *DsMdvxThreaded::_writeToPathThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_writeToPathThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running writeToPath in thread

void DsMdvxThreaded::_writeToPathThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::writeToPath(_outputUrl.c_str());
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// function for compileTimeList thread entry point

void *DsMdvxThreaded::_compileTimeListThreadEntry(void *args)
  
{

  // block SIGALRM

  _blockSignal(SIGALRM);

  // disable cancelling for the moment

  int last_state;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &last_state);

  // args points to the DsMdvxThreaded object which spawned the thread

  DsMdvxThreaded *mdvx = (DsMdvxThreaded *) args;
  mdvx->_compileTimeListThreadRun();
  return (NULL);
  
}

/////////////////////////////////////////////////
// function for running compileTimeList in thread

void DsMdvxThreaded::_compileTimeListThreadRun()
  
{
  pthread_cleanup_push(_cleanupThread, (void *) this);
  int iret = DsMdvx::compileTimeList();
  _tidyThread(iret);
  pthread_cleanup_pop(0);
}

/////////////////////////////////////////////////
// block signals in the threads

void DsMdvxThreaded::_blockSignal(int isig)

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

  


