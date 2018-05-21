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
// FmqDeviceFile.cc
// FmqDeviceFile class
//
// File-based message queue
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#include <cerrno>                                 
#include <unistd.h>
#include <dataport/bigend.h>
#include <toolsa/file_io.h>
#include <toolsa/uusleep.h>
#include <toolsa/Path.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/uusleep.h>
#include <Fmq/FmqDeviceFile.hh>
#include <Fmq/Fmq.hh>
using namespace std;

FmqDeviceFile::FmqDeviceFile(const string &fmqPath, 
			     size_t numSlots, 
			     size_t bufSize,
			     TA_heartbeat_t heartbeatFunc) :
	FmqDevice(fmqPath, numSlots, bufSize, heartbeatFunc)
  
{

  // file paths
  
  _stat_path = fmqPath + ".stat";
  _buf_path = fmqPath + ".buf";
  _path[STAT_IDENT] = _stat_path;
  _path[BUF_IDENT] = _buf_path;
  
  // file handle
  
  _stat_file = NULL;
  _buf_file = NULL;

  _stat_fd = 0;
  _buf_fd = 0;

}

FmqDeviceFile::~FmqDeviceFile()
{
  FmqDeviceFile::do_close();
}

////////////////////////////////////////////////////////////
// FILE OPEN
//
//  Opens the FMQ status and buffer files
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

int FmqDeviceFile::do_open(const char *mode)

{

  clearErrStr();

  // close files if already open

  do_close();

  // in "w+" mode, create the directory if needed

  if (!strcmp(mode, "w+")) {
    Path statPath(_stat_path);
    if (ta_makedir_recurse(statPath.getDirectory().c_str())) {
      int errNum = errno;
      _errStr += "ERROR - FmqDeviceFile::open\n";
      TaStr::AddStr(_errStr, "Cannot create directory: ", statPath.getDirectory());
      _errStr += strerror(errNum);
      return -1;
    }
  }

  // Open the stat file
  
  if ((_stat_file = fopen(_stat_path.c_str(), mode)) == NULL) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::open\n";
    TaStr::AddStr(_errStr, "Cannot open stat file: ", _stat_path);
    TaStr::AddStr(_errStr, "mode: ", mode);
    _errStr += strerror(errNum);
    return -1;
  }
  
  // get file descriptor for low-level read/writes

  _stat_fd = fileno(_stat_file);
  
  // Open the buf file
  
  if ((_buf_file = fopen(_buf_path.c_str(), mode)) == NULL) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::open\n";
    TaStr::AddStr(_errStr, "Cannot open buf file: ", _buf_path);
    TaStr::AddStr(_errStr, "mode: ", mode);
    _errStr += strerror(errNum);
    return -1;
  }

  // get file descriptor for low-level read/writes

  _buf_fd = fileno(_buf_file);

  _file[STAT_IDENT] = _stat_file;
  _file[BUF_IDENT] = _buf_file;

  _fd[STAT_IDENT] = _stat_fd;
  _fd[BUF_IDENT] = _buf_fd;

  return 0;

}

/////////////////////////////////////////////////////////////////
// FILE CLOSE
//
//  Close the FMQ status and buffer files.
//  Free up memory associated with the files. 
//
//  Return value:
//    0 on success, -1 on failure
///

void FmqDeviceFile::do_close()

{

  clearErrStr();

  // close stat file
  
  if (_stat_file != NULL) {
    fclose(_stat_file);
    _stat_file = NULL;
  }

  //  close buf file
  
  if (_buf_file != NULL) {
    fclose(_buf_file);
    _buf_file = NULL;
  }

}

/////////////////////////////////////////////////////////////////
// FILE SEEK
//
// Seeks to specified offset, from start.
//
//  Return value:
//    0 on success, -1 on error.

off_t FmqDeviceFile::do_seek(ident_t id, off_t offset)

{
  return _seek(_path[id], _fd[id], offset);
}

off_t FmqDeviceFile::_seek(const string &path, int fd, off_t offset)
{
  clearErrStr();
  if (lseek(fd, offset, SEEK_SET) < 0) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::seek\n";
    TaStr::AddInt(_errStr, "Cannot seek to offset: ", offset);
    TaStr::AddStr(_errStr, "file: ", path);
    _errStr += strerror(errNum);
    return -1;
  }
  return offset;
}

/////////////////////////////////////////////////////////////////
// FILE LOCKING
//
//  Locks the stat file for writing - blocks until file
//  available for writing and reading.
//
//  Call heartbeat function while waiting, if non-NULL.
//
//  Return value:
//    0 on success, -1 on error.
///

int FmqDeviceFile::lock()
     
{
  
  clearErrStr();

  if (ta_lock_file_heartbeat(_stat_path.c_str(),
			     _stat_file, "w",
			     _heartbeatFunc)) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::lock\n";
    TaStr::AddStr(_errStr, "Cannot get write lock, file: ", _stat_path);
    _errStr += strerror(errNum);
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////
//  FmqDeviceFile::_unlock()
//
//  Unlocks the stat file
//
//  Return value:
//    0 on success, -1 on error.

int FmqDeviceFile::unlock()

{
  
  clearErrStr();

  if (ta_unlock_file(_stat_path.c_str(), _stat_file)) {
    return -1;
  } else {
    return 0;
  }

}

/////////////////////////////////////////////////////////////////
// READ FROM FILE - unbuffered with retries
//
// Returns nbytes actually read, -1 on error

int FmqDeviceFile::do_read(ident_t id, void *mess, size_t len)
{
  return _read(_path[id], _fd[id], mess, len);
}

int FmqDeviceFile::_read(const string &path, int fd, void *mess, size_t len)

{
  
  clearErrStr();

  long bytes_read;
  long target_len = len;
  char *ptr = (char *) mess;
  int retries = 100;
  int total = 0;
  int err_count = 0;
  
  while(target_len) {
    errno = 0;
    bytes_read = read(fd, ptr, target_len);
    if(bytes_read <= 0) {
      if (errno != EINTR) { // system call was not interrupted 
	err_count++;
      }
      if(err_count >= retries) {
	int errNum = errno;
	_errStr += "ERROR - FmqDeviceFile::_read\n";
	TaStr::AddStr(_errStr, "Read error, file: ", path);
	TaStr::AddInt(_errStr, "  nbytes requested: ", len);
	TaStr::AddInt(_errStr, "  nbytes read: ", total);
	_errStr += strerror(errNum);
	return total;
      }
      // Block for 1 millisecond
      uusleep(1000);
    } else {
      err_count = 0;
    }
    if (bytes_read > 0) {
      target_len -= bytes_read;
      ptr += bytes_read;
      total += bytes_read;
    }
  }
  
  return total;
}

////////////////////////////////////////////////////////////
// WRITE TO FILE - unbuffered with retries
//
// returns number of bytes written, -1 on error

int FmqDeviceFile::do_write(ident_t id, const void *mess, size_t len)
{
  return _write(_path[id], _fd[id], mess, len);
}

int FmqDeviceFile::_write(const string &path, int fd, const void *mess, size_t len)

{

  clearErrStr();

  int bytes_written;
  int target_len = len;
  char *ptr = (char *) mess;
  int retries = 100;
  int total = 0;
  int err_count = 0;
  
  while(target_len > 0) {
    
    errno = 0;
    bytes_written = write(fd,ptr,target_len);

    if(bytes_written <= 0) {
      
      if (errno != EINTR) { // system call was not interrupted
	err_count++;
      }
      
      if(err_count >= retries) {
	int errNum = errno;
	_errStr += "ERROR - FmqDeviceFile::_write\n";
	TaStr::AddStr(_errStr, "Write error, file: ", path);
	TaStr::AddInt(_errStr, "  nbytes requested: ", len);
	TaStr::AddInt(_errStr, "  nbytes written: ", total);
	_errStr += strerror(errNum);
	return total;
      }

      // Block for 1 millisecond

      uusleep(1000);
      
    } else {

      err_count = 0;
      
    }
    
    if (bytes_written > 0) {
      target_len -= bytes_written;
      ptr += bytes_written;
      total += bytes_written;
    }

  }
  
  return (total);

}

////////////////////////////////////////////////////////////
//  Checks that the queue device exists.
//
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceFile::check_exists()

{

  struct stat file_stat;

  // check status file

  if (stat(_stat_path.c_str(), &file_stat)) {
    return -1;
  }

  // check buf file

  if (stat(_buf_path.c_str(), &file_stat)) {
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////
//  Checks that the device buffer is the valid size.
//  Return value:
//    0 on success, -1 on failure

int FmqDeviceFile::check_size(ident_t id, size_t expectedSize)

{

  clearErrStr();
  
  struct stat file_stat;

  if (ta_stat(_path[id].c_str(), &file_stat)) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::check_size\n";
    TaStr::AddStr(_errStr, "Cannot stat file: ", _path[id]);
    _errStr += strerror(errNum);
    return -1;
  }
  
  if ((int) file_stat.st_size != (int) expectedSize) {
    _errStr += "ERROR - FmqDeviceFile::check_size\n";
    TaStr::AddStr(_errStr, "File is incorrect size: ", _path[id]);
    TaStr::AddInt(_errStr, "Expected size: ", expectedSize);
    TaStr::AddInt(_errStr, "Actual   size: ", file_stat.st_size);
    return -1;
  }
  
  return 0;

}

////////////////////////////////////////////////////
//  Get the device buffer size.
//  Return value:
//    size of buffer

int FmqDeviceFile::get_size(ident_t id)

{

  clearErrStr();
  
  struct stat file_stat;

  if (ta_stat(_path[id].c_str(), &file_stat)) {
    int errNum = errno;
    _errStr += "ERROR - FmqDeviceFile::check_size\n";
    TaStr::AddStr(_errStr, "Cannot stat file: ", _path[id]);
    _errStr += strerror(errNum);
    return -1;
  }

  return (int) file_stat.st_size;

}

////////////////////////////////////////////////////////////
//  update_last_id_read()
//
//  Write the last_id_read in the status struct, if the
//  write mode is blocking.
//
//  Return value:
//    0 on success, -1 on failure.

int FmqDeviceFile::update_last_id_read(int lastIdRead)

{
  
  // need to reopen the file, since it may have been opened read-only


  FILE *stat_file;
  if ((stat_file = fopen(_stat_path.c_str(), "r+")) == NULL) {
    return -1;
  }
  
  Fmq::q_stat_t stat;
  off_t offset = (char *) &stat.last_id_read - (char *) &stat.magic_cookie;

  int stat_id = fileno(stat_file);
  
  if (_seek(_stat_path, stat_id, offset) != offset) {
    fclose(stat_file);
    return -1;
  }

  si32 last_id_read = lastIdRead;
  BE_from_array_32(&last_id_read, sizeof(si32));
  
  int iret = 0;
  if (_write(_stat_path, stat_id, &last_id_read, sizeof(si32)) != sizeof(si32)) {
    iret = -1;
  }

  fclose(stat_file);
  return iret;

}

