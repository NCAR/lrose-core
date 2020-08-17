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
// FmqDeviceShmem class
//
// Shared-memory-based message queue
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cstdio>
#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <Fmq/FmqDeviceShmem.hh>
#include <Fmq/Fmq.hh>
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/fcntl.h>
#include <semaphore.h>
using namespace std;

FmqDeviceShmem::FmqDeviceShmem(const string &fmqPath,
			       size_t numSlots, 
			       size_t bufSize,
			       TA_heartbeat_t heartbeatFunc) :
	FmqDevice(fmqPath, numSlots, bufSize, heartbeatFunc)
  
{
  
  _lock_path = fmqPath + ".lock";
  _lock_file = NULL;
  
  _statKey = 0;
  _bufKey = 0;

  _statPtr = NULL;
  _bufPtr = NULL;

  _offset[STAT_IDENT] = 0;
  _offset[BUF_IDENT] = 0;

  // initialize size of stat and buf segments
  // these will be updated later if queue already exists
  
  _nbytes[STAT_IDENT] =
    sizeof(Fmq::q_stat_t) + numSlots * sizeof(Fmq::q_slot_t);
  _nbytes[BUF_IDENT] = bufSize;

}

FmqDeviceShmem::~FmqDeviceShmem()
{
  FmqDeviceShmem::do_close();
}

////////////////////////////////////////
// get the shmem keys
// returns 0 on success, -1 on failure

int FmqDeviceShmem::_getShmemKeys()

{

  if (_statKey != 0) {
    // already done
    return 0;
  }

  key_t baseKey;
  if (Fmq::getShmemKey(_fmqPath, baseKey)) {
    _errStr += "ERROR - FmqDeviceShmem::_getShmemKeys\n";
    TaStr::AddStr(_errStr, "Cannot get shmem key val, path: ", _fmqPath);
    return -1;
  }

  _statKey = baseKey;
  _bufKey = baseKey + 1;

  _key[STAT_IDENT] = _statKey;
  _key[BUF_IDENT] = _bufKey;

  return 0;

}

////////////////////////////////////////////////////////////
//  Open the status and buffer segments
//
//  Parameters:
//    mode - uses fopen() style modes.
//           Valid modes:
//             "r"  - read only
//             "r+" - read/write
//             "w+" - write/read, creates if non-existent
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceShmem::do_open(const char *mode)

{

  if (_getShmemKeys()) {
    return -1;
  }

  // seek to start

  _offset[STAT_IDENT] = 0;
  _offset[BUF_IDENT] = 0;

  // close segments if already open

  FmqDeviceShmem::do_close();

  // create mode

  if (!strcmp(mode, "w+")) {

    if (_open_create()) {
      return -1;
    }
    
  } else if (!strcmp(mode, "r+") || !strcmp(mode, "r")) {

    if (_open_rdwr()) {
      return -1;
    }

  }

  // in write mode, create the directory if needed
  
  if (!strcmp(mode, "w+") || !strcmp(mode, "r+")) {

    Path lockPath(_lock_path);
    if (ta_makedir_recurse(lockPath.getDirectory().c_str())) {
      int errNum = errno;
      _errStr += "ERROR - FmqDeviceShmem::open\n";
      TaStr::AddStr(_errStr, "Cannot create directory: ", lockPath.getDirectory());
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

    // Open the lock file
    
    if ((_lock_file = fopen(_lock_path.c_str(), "w+")) == NULL) {
      int errNum = errno;
      _errStr += "ERROR - FmqDeviceShmem::open\n";
      TaStr::AddStr(_errStr, "Cannot create lock file: ", _lock_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

  }
  
  return 0;

}

////////////////////////////////////////////////////////////
// OPEN and create
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceShmem::_open_create()

{

  // remove shmem segments if they already exist but are too small
  
  if (_ushmCheck(_statKey, 0)) {
    if (!_ushmCheck(_statKey, _nbytes[STAT_IDENT])) {
      _ushmRemove(_statKey);
    }
  }
  
  if (_ushmCheck(_bufKey, 0)) {
    if (!_ushmCheck(_bufKey, _nbytes[BUF_IDENT])) {
      _ushmRemove(_bufKey);
    }
  }
  
  // create shmem segments
  
  if ((_statPtr = (char *) _ushmCreate(_statKey, _nbytes[STAT_IDENT], 0666)) == NULL) {
    _errStr += "ERROR - FmqDeviceShmem::_open_create\n";
    TaStr::AddInt(_errStr, "Cannot create shmem for stat, key: ", _statKey);
    TaStr::AddInt(_errStr, "size: ", _nbytes[STAT_IDENT]);
    return -1;
  }
  _ptr[STAT_IDENT] = _statPtr;
  
  if ((_bufPtr = (char *) _ushmCreate(_bufKey, _nbytes[BUF_IDENT], 0666)) == NULL) {
    _errStr += "ERROR - FmqDeviceShmem::_open_create\n";
    TaStr::AddInt(_errStr, "Cannot create shmem for buf, key: ", _bufKey);
    TaStr::AddInt(_errStr, "size: ", _nbytes[BUF_IDENT]);
    return -1;
  }
  _ptr[BUF_IDENT] = _bufPtr;

  return 0;

}

////////////////////////////////////////////////////////////
// OPEN read-write
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceShmem::_open_rdwr()

{

  // first read the status struct, to get the sizes

  if (_set_sizes_from_existing_queue()) {
    return -1;
  }

  // read-write mode
  
  if ((_statPtr = (char *) _ushmGet(_statKey, _nbytes[STAT_IDENT])) == NULL) {
    return -1;
  }
  _ptr[STAT_IDENT] = _statPtr;
  
  if ((_bufPtr = (char *) _ushmGet(_bufKey, _nbytes[BUF_IDENT])) == NULL) {
    return -1;
  }
  _ptr[BUF_IDENT] = _bufPtr;
  
  return 0;

}

////////////////////////////////////////////////////////////
// Set sizes from status struct in existing queue
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceShmem::_set_sizes_from_existing_queue()

{

  if (_getShmemKeys()) {
    return -1;
  }

  // read the status struct, to get the sizes
  
  Fmq::q_stat_t stat;
  void *statPtr = NULL;
  if ((statPtr = _ushmGet(_statKey, sizeof(Fmq::q_stat_t))) == NULL) {
    return -1;
  }
  memcpy(&stat, statPtr, sizeof(Fmq::q_stat_t));
  Fmq::be_to_stat(&stat);
  
  // set the sizes
  
  _nbytes[STAT_IDENT] =
    sizeof(Fmq::q_stat_t) + stat.nslots * sizeof(Fmq::q_slot_t);
  _nbytes[BUF_IDENT] = stat.buf_size;
  
  // release the segment

  _ushmDetach(statPtr);

  return 0;

}

/////////////////////////////////////////////////////////////////
// CLOSE

void FmqDeviceShmem::do_close()

{

  // detach stat segment
  
  if (_statPtr != NULL) {
    _ushmDetach(_statPtr);
    _statPtr = NULL;
  }

  //  detach buf segment
  
  if (_bufPtr != NULL) {
    _ushmDetach(_bufPtr);
    _bufPtr = NULL;
  }

  // close lock file
  
  if (_lock_file != NULL) {
    fclose(_lock_file);
    _lock_file = NULL;
  }

}

/////////////////////////////////////////////////////////////////
// SEEK
//
// Seeks to specified offset, from start.
//
//  Return value:
//    0 on success, -1 on error.

off_t FmqDeviceShmem::do_seek(ident_t id, off_t offset)
  
{
  _offset[id] = offset;
  return offset;
}

/////////////////////////////////////////////////////////////////
// LOCKING
//
//  Locks the stat file for writing - blocks until file
//  available for writing and reading.
//
//  Call heartbeat function while waiting, if non-NULL.
//
//  Return value:
//    0 on success, -1 on error.
///

int FmqDeviceShmem::lock()
     
{

  if (_lock_file == NULL) {
    return 0;
  }

  if (ta_lock_file_heartbeat(_lock_path.c_str(),
			     _lock_file, "w",
			     _heartbeatFunc)) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::lock\n";
    TaStr::AddStr(_errStr, "Cannot get write lock, file: ", _lock_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////
//  FmqDeviceShmem::_unlock()
//
//  Unlocks the stat file
//
//  Return value:
//    0 on success, -1 on error.

int FmqDeviceShmem::unlock()

{

  if (_lock_file == NULL) {
    return 0;
  }

  if (ta_unlock_file(_lock_path.c_str(),
		     _lock_file)) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////////////////////
// READ
//
// Returns nbytes actually read, -1 on error

int FmqDeviceShmem::do_read(ident_t id, void *mess, size_t len)

{

  off_t offset = _offset[id];
  char *start = _ptr[id] + offset;
  size_t nbytesAvail = _nbytes[id] - offset;

  if (nbytesAvail < len) {
    _errStr += "ERROR - FmqDeviceShmem::do_read\n";
    TaStr::AddInt(_errStr, "Read error, nbytes requested: ", len);
    TaStr::AddInt(_errStr, "nbytes available: ", nbytesAvail);
    TaStr::AddInt(_errStr, "current offset: ", offset);
    return -1;
  }
  
  memcpy(mess, start, len);
  _offset[id] += len;
  return len;

}

////////////////////////////////////////////////////////////
// WRITE TO FILE
//
// returns number of bytes written, -1 on error

int FmqDeviceShmem::do_write(ident_t id, const void *mess, size_t len)

{

  off_t offset = _offset[id];
  char *start = _ptr[id] + offset;
  size_t nbytesAvail = _nbytes[id] - offset;

  if (nbytesAvail < len) {
    _errStr += "ERROR - FmqDeviceShmem::do_write\n";
    TaStr::AddInt(_errStr, "Write error, nbytes requested: ", len);
    TaStr::AddInt(_errStr, "nbytes available: ", nbytesAvail);
    TaStr::AddInt(_errStr, "current offset: ", offset);
    return -1;
  }
  
  memcpy(start, mess, len);
  _offset[id] += len;
  return len;

}

////////////////////////////////////////////////////////////
//  Checks that the shmem segments exist.
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceShmem::check_exists()

{

  if (_set_sizes_from_existing_queue()) {
    return -1;
  }

  if (!_ushmCheck(_statKey, _nbytes[STAT_IDENT])) {
    return -1;
  }
  
  if (!_ushmCheck(_bufKey, _nbytes[BUF_IDENT])) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
//  FmqDeviceShmem::_check_device_sizes()
//
//  Checks that the FMQ files are valid sizes.
//  Also reads in the slots array.
//
//  Assumes the files are already open.
//
//  Return value:
//    true on 0, -1 on failure

int FmqDeviceShmem::check_size(ident_t id, size_t expectedSize)

{

  if (_set_sizes_from_existing_queue()) {
    return -1;
  }

  if (!_ushmCheck(_key[id], _nbytes[id])) {
    return -1;
  }

  return 0;
  
}

////////////////////////////////////////////////////
//  Get device buffer size

int FmqDeviceShmem::get_size(ident_t id)

{
  return _nbytes[id];
}

////////////////////////////////////////////////////////////
//  Update the last_id_read in the status struct if the
//  write mode is blocking.
//
//  Return value:
//    0 on success, -1 on failure.
///

int FmqDeviceShmem::update_last_id_read(int lastIdRead)

{
  
  Fmq::q_stat_t stat;
  off_t offset = (char *) &stat.last_id_read - (char *) &stat.magic_cookie;

  si32 last_id_read = lastIdRead;
  BE_from_array_32(&last_id_read, sizeof(si32));

  memcpy(_statPtr + offset, &last_id_read, sizeof(si32));

  return 0;

}

////////////////////////////////////////////////////////////
// Get the segment name

const char *FmqDeviceShmem::_getSegName(ident_t id)

{
  if (id == STAT_IDENT) {
    return "Status segment";
  } else {
    return "Buffer segment";
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// SEMAPHORES and SHARED MEMORY
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// creates and attaches shared memory
// returns memory location on success, NULL on error

void *FmqDeviceShmem::_ushmCreate(key_t key, size_t size, int permissions)
{

  // create the shared memory 

  int shmid;
  if ((shmid = shmget(key, size, permissions | IPC_CREAT)) < 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::_ushmCreate\n";
    _errStr += "Getting shared memory with 'shmget'\n";
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
   return NULL;
  }
  
  // attach the shared memory regions

  errno = 0;
  void *memory_region = (void *) shmat(shmid, (char *) 0, 0);

  if (errno != 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::_ushmCreate\n";
    _errStr += "Attaching shared memory with 'shmat'\n";
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return NULL;
  }
  
  return memory_region;

}

/////////////////////////////////////////////////////////////////////////////
// gets and attaches shared memory
// returns memory location on success, NULL on error

void *FmqDeviceShmem::_ushmGet(key_t key, size_t size)
{
  
  // get the shared memory 

  int shmid;
  if ((shmid = shmget(key, size, 06666)) < 0) {
    return NULL;
  }

  // attach the shared memory region

  errno = 0;
  void *memory_region = shmat(shmid, 0, 0);
  if (errno != 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::_ushmGet\n";
    _errStr += "Attaching shared memory with 'shmat'\n";
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return NULL;
  }

  return memory_region;

}

/////////////////////////////////////////////////////////////////////////////
// Check for existence of shmem segment of sufficient size
// Return TRUE if the segment currently exists, and is big enough
//        FALSE otherwise

bool FmqDeviceShmem::_ushmCheck(key_t key, size_t size)
{

  int iret = shmget(key, size, 0666);

  if (iret >= 0) {
    return true;
  } else {
    return false;
  }

}

/////////////////////////////////////////////////////////////////////////////
// get the number of processes currently attached to a shared memory segment
//
// Returns the number of processes currently attached to the indicated
// shared memory segment, or -1 on error.

int FmqDeviceShmem::_ushmNattach(key_t key)
{

  
  // find the shared memory 

  int shmid;
  struct shmid_ds shm_info;
  if ((shmid = shmget(key, 0, 0666)) < 0) {
    return -1;
  }

  // Get the shared memory information
  
  if (shmctl(shmid, IPC_STAT, &shm_info) != 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::_ushmNattach\n";
    _errStr += "Cannot get shared memory info with 'shmclt'\n";
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return shm_info.shm_nattch;

}

/////////////////////////////////////////////////////////////////////////////
// detach from the shared memory segment
//
// returns 0 on success, -1 on failure

int FmqDeviceShmem::_ushmDetach(void *shm_ptr)
{
  if (shmdt(shm_ptr) == 0) {
    return 0;
  } else {
    return -1;
  }
}

/////////////////////////////////////////////////////////////////////////////
// remove shared memory segment

int FmqDeviceShmem::_ushmRemove(key_t key)
{

  int shmid;

  // find the shared memory 

  if ((shmid = shmget(key, 0, 0666)) < 0) {
    return -1;
  }
  
  // remove the shared memory
  
  if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) != 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceShmem::_ushmRemove\n";
    _errStr += "Removing shared memory info with 'shmclt'\n";
    TaStr::AddInt(_errStr, "  key: ", key);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return 0;

}

