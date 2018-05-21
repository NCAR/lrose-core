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
// FmqDeviceShmem.cc
//
// Shared memory device for Fmq
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
//
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _FMQ_DEVICE_SHMEM_HH_INCLUDED_
#define _FMQ_DEVICE_SHMEM_HH_INCLUDED_

#include <sys/types.h>
#include <Fmq/FmqDevice.hh>
using namespace std;

// class definition

class FmqDeviceShmem : public FmqDevice

{

public:                  

  // constructor
  
  FmqDeviceShmem(const string &fmqPath, 
		 size_t numSlots, 
		 size_t bufSize,
		 TA_heartbeat_t heartbeatFunc);

  // destructor - closes queue
  
  virtual ~FmqDeviceShmem();
  
  //////////////////////////////////////////////////////////
  // methods

  // open

  virtual int do_open(const char *mode);

  // close / clear

  virtual void do_close();

  // seek

  virtual off_t do_seek(ident_t id, off_t offset);

  // locking for writes

  virtual int lock();
  virtual int unlock();

  // read
  
  virtual int do_read(ident_t id, void *mess, size_t len);
  virtual int update_last_id_read(int lastIdRead);
  
  // write

  virtual int do_write(ident_t id, const void *mess, size_t len);

  // checking for existence
  // Returns 0 on success, -1 on failure
  
  virtual int check_exists();

  // Check that the device is the valid size.
  // Returns 0 on success, -1 on failure
  
  virtual int check_size(ident_t id, size_t expectedSize);
  
  // Get size of device buffer

  virtual int get_size(ident_t id);
  
protected:

private:

  // size of stat and buf

  size_t _nbytes[N_IDENT];

  // shared memory and semaphore keys

  key_t _statKey;
  key_t _bufKey;
  key_t _key[N_IDENT];
  
  char *_statPtr; // pointer to status segment
  char *_bufPtr;  // pointer to buffer segment
  char *_ptr[N_IDENT];

  // off_t _statOffset; // current offset in status segment
  // off_t _bufOffset; // current offset in buf segment
  off_t _offset[N_IDENT];

  // lock file for synchronization
  
  string _lock_path;
  FILE *_lock_file;
  
  const char *_getSegName(ident_t id);
  
  int _open_create();
  int _open_rdwr();
  int _set_sizes_from_existing_queue();

  int _getShmemKeys();
  
  void *_ushmCreate(key_t key, size_t size, int permissions);
  void *_ushmGet(key_t key, size_t size);
  bool _ushmCheck(key_t key, size_t size);
  int _ushmNattach(key_t key);
  int _ushmDetach(void *shm_ptr);
  int _ushmRemove(key_t key);

};

#endif

