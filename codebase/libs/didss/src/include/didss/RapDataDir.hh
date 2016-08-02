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
//
// Class for managing RAP_DATA_DIR
//
// This header file not only defines the class,
// but also declares a static instance of the class
// such that to get the RAP_DATA_DIR location:
//    
//    #include <didss/RapDataDir.hh>
//    char* dataPath = RapDataDir.location();
//
//  Terri L. Betancourt RAP, NCAR, Boulder, CO, 80307, USA
//  April 1999
//
// $Id: RapDataDir.hh,v 1.19 2016/03/03 18:03:31 dixon Exp $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _RAP_DATA_DIR_INC_
#define _RAP_DATA_DIR_INC_

#include <cstdlib>
#include <toolsa/Path.hh>
#include <didss/DsURL.hh>
using namespace std;

class RAPDataDir {

public:

   RAPDataDir();

   const char*  location() const { return dataDir; }
   bool   isEnvSet() const { return fromEnv; }
   const char*  getEnvVarName() const { return "RAP_DATA_DIR"; }
   const char*  tmpLocation() const { return tmpDir; }

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

  const char*  dataDir;
  const char*  tmpDir;
  bool         fromEnv;

};

//
// Make one static copy available for everyone to get to
//
static RAPDataDir RapDataDir;

#endif
