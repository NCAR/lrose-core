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
// PutArgs.hh
//
// Args needed for puts
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
/////////////////////////////////////////////////////////////

#ifndef PutArgs_HH
#define PutArgs_HH

#include <string>
#include <toolsa/compress.h>
#include <didss/DsURL.hh>
#include <didss/LdataInfo.hh>
#include <didss/DsMessage.hh>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include "Params.hh"
using namespace std;

///////////////////////
// abstract base class

class PutArgs {
  
public:

  // constructor
  
  PutArgs(const string &dest_url_str,
	  const string &dir_path_str,
	  const LdataInfo &ldata_info,
	  const string &file_name,
	  time_t file_time,
	  int overwrite_age,
	  int max_age_at_copy_time,
	  bool force_copy,
	  bool remove_after_copy,
	  ta_compression_method_t compression_type,
	  int expected_speed,
	  int max_tries,
          int open_timeout_msecs,
	  Params::debug_t debug,
          const string &error_fmq_path);
  
  // Destructor

  virtual ~PutArgs();
  
  // public data
  
  time_t timeAdded;
  DsURL destUrl;
  string dirPath;
  LdataInfo ldataInfo;
  string fileName;
  time_t fileTime;
  int overwriteAge;
  int maxAgeAtCopyTime;
  bool forceCopy;
  bool removeAfterCopy;
  ta_compression_method_t compressionType;
  int expectedSpeed;
  int maxTries;
  int openTimeoutMsecs;
  Params::debug_t Debug;

  bool isOK;
  string destHostName;

  // file details

  string filePath;
  int nbytesFile; // size of this file
  int nbytesInserted; // nbytes inserted ahead while waiting to be sent

  // thread details

  bool inProgress;
  bool done;
  pid_t putPid;
  bool childRunning;
  time_t start_time;

  // error FMQ path

  string errorFmqPath;

protected:
  
private:

};

#endif
