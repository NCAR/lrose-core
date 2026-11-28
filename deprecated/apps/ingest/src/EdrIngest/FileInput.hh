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
//  FileInput is an EdrInput sub-class for reading from LDM output files
//
//  Sue Dettling RAP, NCAR, Boulder, CO, 80307, USA
//
//  November 2004
//
////////////////////////////////////////////////////////////////////////////////

#ifndef _FILE_INPUT_INC_
#define _FILE_INPUT_INC_

#include <string>
#include <toolsa/pmu.h>
#include <cstdio>
#include <didss/DsInputPath.hh>
#include <toolsa/DateTime.hh>
#include <vector>

#include "Params.hh"
#include "Args.hh"
#include "EdrReport.hh"
#include "EdrInput.hh"

using namespace std;

class FileInput : public EdrInput
{
public:
  FileInput();
  ~FileInput();
  
  //
  // Return 0 upon success, -1 upon failure
  //
  int init( Params& params , const vector<string> &fileList, string &prog );

  //
  // Read entire message into buffer (for ASCII data) or just
  // the filename (for BUFR) data.
  // 
  EdrReport::status_t readFile( ui08* &buffer, DateTime &msgTime);
  
  
private:


  //
  // Read message contents into buffer for parsing.
  // Used for ASCII UAL message parsing.
  // 
  EdrReport::status_t readEdrMsg( ui08* &buffer, DateTime &msgTime);

  //
  // Get filename next edr message.
  // Used for BUFR UAL message parsing
  //
  EdrReport::status_t getEdrFile(ui08* &filename,  DateTime &msgTime);

  //
  // Get filetime from file path assuming the path is
  // */yyyymmdd/hhMMss.*
  //
  int getFileTime( char *filename);

  string progName;
  Params params;

  //
  // File handling
  //
  DsInputPath       *fileTrigger;
  FILE              *edrFile;
  bool               fileIsOpen;
  int                msgsReadPerFile;
  char               currFileName[256];              
  int                year, month, day, hour, min, sec;
};

#endif











