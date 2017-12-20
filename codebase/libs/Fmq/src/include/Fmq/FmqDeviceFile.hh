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
//
// file-based device for Fmq
//
// Mike Dixon, RAL, NCAR, Boulder, Colorado
//
// Jan 2009
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef _FMQ_DEVICE_FILE_HH_INCLUDED_
#define _FMQ_DEVICE_FILE_HH_INCLUDED_

#include <Fmq/FmqDevice.hh>

using namespace std;

// class definition

class FmqDeviceFile : public FmqDevice

{

public:                  

  // constructor
  
  FmqDeviceFile(const string &fmqPath, 
		size_t numSlots, 
		size_t bufSize,
		TA_heartbeat_t heartbeatFunc);

  // destructor - closes queue
  
  virtual ~FmqDeviceFile();
  
  //////////////////////////////////////////////////////////
  // methods

  // open

  virtual int do_open(const char *mode);

  // close / clear

  virtual void do_close();

  // seek

  virtual off_t do_seek(ident_t id, off_t offset);
  off_t _seek(const string &path, int id, off_t offset);

  // locking for writes

  virtual int lock();
  virtual int unlock();

  // read
  
  virtual int do_read(ident_t id, void *mess, size_t len);
  int _read(const string &path, int fd, void *mess, size_t len);
  virtual int update_last_id_read(int lastIdRead);
  
  // write

  virtual int do_write(ident_t id, const void *mess, size_t len);
  int _write(const string &path, int fd, const void *mess, size_t len);

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

  // paths to fmq, and files
  
  string _stat_path;      /* status path */
  string _buf_path;       /* buffer path */
  string _path[N_IDENT];

  // file handles

  FILE *_stat_file;      /* streams file pointer - for fopen/fclose */
  FILE *_buf_file;       /* streams file pointer - for fopen/fclose */
  FILE *_file[N_IDENT];

  int _stat_fd;
  int _buf_fd;
  int _fd[N_IDENT];

};

#endif

