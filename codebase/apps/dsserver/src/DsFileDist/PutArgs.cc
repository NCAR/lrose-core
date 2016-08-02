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
// PutArgs.cc
//
// Args needed for puts
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2000
//
/////////////////////////////////////////////////////////////

#include "PutArgs.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/file_io.h>
#include <ctime>
#include <sys/stat.h>
using namespace std;

PutArgs::PutArgs(const string &dest_url_str,
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
                 const string &error_fmq_path) :
  destUrl(dest_url_str),
  dirPath(dir_path_str),
  ldataInfo(ldata_info),
  fileName(file_name),
  fileTime(file_time),
  overwriteAge(overwrite_age),
  maxAgeAtCopyTime(max_age_at_copy_time),
  forceCopy(force_copy),
  removeAfterCopy(remove_after_copy),
  compressionType(compression_type),
  expectedSpeed(expected_speed),
  maxTries(max_tries),
  openTimeoutMsecs(open_timeout_msecs),
  Debug(debug),
  errorFmqPath(error_fmq_path)

{

  isOK = true;
  timeAdded = time(NULL);
  destHostName = destUrl.getHost();

  // compute the file path
  
  filePath = ldataInfo.getDataPath();

  // set file size
  
  struct stat fileStat;
  if (ta_stat(filePath.c_str(), &fileStat)) {
    isOK = false;
    if (debug) {
      cerr << "ERROR - PutArgs::PutArgs()" << endl;
      cerr << "  Cannot stat file: " << filePath << endl;
    }
    return;
  }
  nbytesFile = fileStat.st_size;
  nbytesInserted = 0;

  // thread details

  inProgress = false;
  done = false;
  putPid = 0;
  childRunning = false;
  start_time = 0;
  
}

//////////////  
// Destructor

PutArgs::~PutArgs()

{

}
  

