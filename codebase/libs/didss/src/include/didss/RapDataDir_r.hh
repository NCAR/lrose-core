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
//
// Class for managing RAP_DATA_DIR - no statics
//

#ifndef _RAP_DATA_DIR_R_INC_
#define _RAP_DATA_DIR_R_INC_

#include <cstdlib>
#include <toolsa/Path.hh>
#include <didss/DsURL.hh>
using namespace std;

class RAPDataDir_r {

public:

   RAPDataDir_r();

   const char*  location() const      { return dataDir; }
   bool   isEnvSet() const      { return fromEnv; }
   const char*  getEnvVarName() const { return RAP_DATA_DIR_ENV_VAR; }

   // fill out the path, given the URL
   // If the file starts with . or /, it is fully qualified and is used
   // unchanged.
   // Otherwise, RAPDataFile and the file are concatenated to form the path.

   void fillPath(const DsURL &url, string  &path);

   // fill out the path, given the file string
   // If the file starts with . or /, it is fully qualified and is used
   // unchanged.
   // Otherwise, RAPDataFile and the file are concatenated to form the path.

   void fillPath(const string &file, string &path);

   // Strip RapDataDir from the start of the given path, to leave the
   // file portion. This is the inverse of fillPath().
   // If the path does not start with RapDataDir it is copied unchanged.
   // If the path starts with RapDataDir, this is removed from the start
   // of the string, inclusive of the trailing delimiter.

   void stripPath(const string &path, string &file);

private:

   char*        dataDir;
   bool         fromEnv;
   const char*  RAP_DATA_DIR_ENV_VAR;
   const char*  DATA_DIR_ENV_VAR;

};

#endif
