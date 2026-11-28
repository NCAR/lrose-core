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
// AsciiPosn2Spdb.hh
//
// AsciiPosn2Spdb object
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2018
//
///////////////////////////////////////////////////////////////
//
// AsciiPosn2Spdb reads ascii files containing location information
// for mobile assets, and and writes to SPDB as ac_posn data.
//
///////////////////////////////////////////////////////////////////////

#ifndef AsciiPosn2Spdb_H
#define AsciiPosn2Spdb_H

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>
#include <rapformats/ac_posn.h>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class AsciiPosn2Spdb {
  
public:

  // constructor

  AsciiPosn2Spdb (int argc, char **argv);

  // destructor
  
  ~AsciiPosn2Spdb();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;

  int _processFile(const char *file_path);
  int _decodeGpsLoggerLine(const char *line,
                           DsSpdb &spdb);
  int _doPut(DsSpdb &spdb);

};

#endif

