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
// Class for managing RAP_DATA_DIR - reentrant
//

#include <toolsa/os_config.h>
#include <toolsa/Path.hh>
#include <didss/RapDataDir_r.hh>
using namespace std;

RAPDataDir_r::RAPDataDir_r() :
  RAP_DATA_DIR_ENV_VAR("RAP_DATA_DIR"), DATA_DIR_ENV_VAR("DATA_DIR")

{

   fromEnv = false;
   dataDir = (char*)Path::CWD;

   char *envVar = getenv( RAP_DATA_DIR_ENV_VAR );
   if ( envVar != NULL ) {
      dataDir = envVar;
      fromEnv = true;
   } else {
     envVar = getenv( DATA_DIR_ENV_VAR );
     if ( envVar != NULL ) {
       dataDir = envVar;
       fromEnv = true;
     }
   }

}

// Fill out the path, given a URL.
//
// If the url file starts with . or /, it is fully qualified and is used
// unchanged.
//
// Otherwise, RAPDataDir and the file are concatenated to form the path.

void RAPDataDir_r::fillPath(const DsURL &url, string &path)
{
  const string &file = url.getFile();
  fillPath(file, path);
}

// Fill out the path, given a file.
//
// If the file starts with . or /, it is fully qualified and is used
// unchanged.
//
// Otherwise, RAPDataFile and the file are concatenated to form the path.

void RAPDataDir_r::fillPath(const string &file, string &path)
{
  if (file[0] == '.') {
    // file starts with '.' so it is fully qualified
    path = file;
  } else if (file.substr(0, strlen(PATH_DELIM)) == PATH_DELIM) {
    // file starts with PATH_DELIM so it is fully qualified
    path = file;
  } else {
    path = dataDir;
    // add in trailing delimiter if missing
    string delimStr = PATH_DELIM;
    if (path.substr(path.size() - delimStr.size()) != delimStr) {
      path += delimStr;
    }
    path += file;
  }
}

// Strip RapDataDir from the start of the given path, to leave the
// file portion. This is the inverse of fillPath().
//
// If the path does not start with RapDataDir it is copied unchanged.
// If the path starts with RapDataDir, this is removed from the start
// of the string, inclusive of the trailing delimiter.

void RAPDataDir_r::stripPath(const string &path, string &file)
{

  // set search string to RAP_DATA_DIR

  string dataDirStr = dataDir;

  // if data dir is empty, return the string unchanged.

  if (dataDirStr.size() == 0) {
    file = path;
    return;
  }

  // add in trailing delimiter if missing

  string delimStr = PATH_DELIM;
  if (dataDirStr.substr(dataDirStr.size() - delimStr.size()) != delimStr) {
    dataDirStr += delimStr;
  }

  // if dataDirStr is at start of path, strip it off.
  // otherwise set file to path unchanged.

  if (dataDirStr == path.substr(0, dataDirStr.size())) {
    file = path.substr(dataDirStr.size());
  } else {
    file = path;
  }

  return;

}

