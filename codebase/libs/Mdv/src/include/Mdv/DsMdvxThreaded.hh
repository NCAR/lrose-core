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
// DsMdvxThreaded.hh
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

#ifndef DsMdvxThreaded_HH
#define DsMdvxThreaded_HH

#include <Mdv/DsMdvx.hh>
#include <pthread.h>
using namespace std;

class DsMdvxMsg;

///////////////////////////////////////////////////////////////
// class definition

class DsMdvxThreaded : public DsMdvx

{

public:
  
  // constructor

  DsMdvxThreaded();

  // destructor
  
  virtual ~DsMdvxThreaded();

  // Overload read, write and time_list methods.
  //
  // See Mdvx_read.hh, Mdvx_write.hh and Mdvx_timelist.hh for details
  // on how these are set up and their basic use.
  
  virtual int readAllHeaders();
  virtual int readVolume();
  virtual int readVsection();
  virtual int writeToDir(const string &output_url);
  virtual int writeToPath(const string &output_url);
  virtual int compileTimeList();

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
  //   getNbytesReadExpected(): get number of bytes expected to be read
  //   getNbytesReadDone(): get number of read bytes received so far
  //   getPercentReadComplete(): get percentage of read completed so far
  //   getNbytesWriteExpected(): get number of bytes expected to be written
  //   getNbytesWriteDone(): get number of write bytes sent so far
  //   getPercentWriteComplete(): get percentage of write completed so far
  //
  // If the read/write call is taking too long, you can cancel it by
  // making a call to cancelThread().
  // If the cancel call interrupts the read or write in progress,
  // the threadRetVal will be -1. If the read or write was
  // successful in spite of the cancellation, the retval will be 0.

  bool getThreadDone();
  int getThreadRetVal() { return (_threadRetVal); }
  ssize_t getNbytesReadExpected() { return (_nbytesReadExpected); }
  ssize_t getNbytesReadDone() { return (_nbytesReadDone); }
  ssize_t getNbytesWriteExpected() { return (_nbytesWriteExpected); }
  ssize_t getNbytesWriteDone() { return (_nbytesWriteDone); }
  double getPercentReadComplete();
  double getPercentWriteComplete();
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
  ssize_t _nbytesReadExpected;
  ssize_t _nbytesReadDone;
  ssize_t _nbytesWriteExpected;
  ssize_t _nbytesWriteDone;
  pthread_mutex_t _mutex;
  pthread_t _thread;

  int _communicate(DsURL &url, DsMdvxMsg &msg,
                   const void *msgBuf, const ssize_t msgLen);

  int _prepareThread();
  void _tidyThread(int ret_val);

  void _readAllHeadersThreadRun();
  void _readVolumeThreadRun();
  void _readVsectionThreadRun();
  void _writeToDirThreadRun();
  void _writeToPathThreadRun();
  void _compileTimeListThreadRun();

  // Static functions for thread creation.

  static void *_readAllHeadersThreadEntry(void *args);
  static void *_readVolumeThreadEntry(void *args);
  static void *_readVsectionThreadEntry(void *args);
  static void *_writeToDirThreadEntry(void *args);
  static void *_writeToPathThreadEntry(void *args);
  static void *_compileTimeListThreadEntry(void *args);

  // block signal in thread

  static void _blockSignal(int isig) ;
  
  // cleanup after thread is cancelled

  static void _cleanupThread(void *args);

private:

};

#endif


