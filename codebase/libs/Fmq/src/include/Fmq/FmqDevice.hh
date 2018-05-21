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
////////////////////////////////////////////////////////////////////////////////
// FmqDevice.cc
//
// Abstract base class for file-based and shmem-based devices for queue
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
//
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _FMQ_DEVICE_HH_INCLUDED_
#define _FMQ_DEVICE_HH_INCLUDED_

#include <string>
#include <toolsa/heartbeat.h>
using namespace std;

// class definition

class FmqDevice

{

public:                  

  typedef enum {
    STAT_IDENT,
    BUF_IDENT,
    N_IDENT
  } ident_t;

  // constructor
  
  FmqDevice(const string &fmqPath,
	    size_t numSlots, 
	    size_t bufSize,
	    TA_heartbeat_t heartbeatFunc);

  // destructor - closes queue
  
  virtual ~FmqDevice();
  
  //////////////////////////////////////////////////////////
  // methods

  // open

  virtual int do_open(const char *mode) = 0;

  // close / clear

  virtual void do_close() = 0;

  // seek

  virtual off_t do_seek(ident_t id, off_t offset) = 0;

  // locking for writes

  virtual int lock() = 0;
  virtual int unlock() = 0;

  // read
  
  virtual int do_read(ident_t id, void *mess, size_t len) = 0;
  virtual int update_last_id_read(int lastIdRead) = 0;
  
  // write

  virtual int do_write(ident_t id, const void *mess, size_t len) = 0;

  // checking for existence
  // Returns 0 on success, -1 on failure
  
  virtual int check_exists() = 0;

  // Check that the device buffer is the valid size.
  // Returns 0 on success, -1 on failure

  virtual int check_size(ident_t id, size_t expectedSize) = 0;
  
  // Get size of device buffer

  virtual int get_size(ident_t id) = 0;
  
  ///////////////////////////////////////////////////////////////////
  // error string is set during open/read/write operations
  // get error string is an error is returned

  void clearErrStr() { _errStr = ""; }
  const string &getErrStr() { return (_errStr); }

protected:

  // path
  
  const string _fmqPath;

  // error string

  mutable string _errStr;

  // heartbeat function for procmap registration

  TA_heartbeat_t _heartbeatFunc;
  
private:

};

#endif

