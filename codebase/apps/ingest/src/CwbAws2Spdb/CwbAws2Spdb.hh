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
// CwbAws2Spdb.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2008
//
///////////////////////////////////////////////////////////////
//
// CwbAws2Spdb reads automated weather station surface
// observations, converts them to station_report_t format
// and writes them to an SPDB data base
//
///////////////////////////////////////////////////////////////////////

#ifndef CwbAws2Spdb_H
#define CwbAws2Spdb_H

#include <string>
#include <map>
#include <Spdb/DsSpdb.hh>
#include <rapformats/metar.h>
#include <rapformats/station_reports.h>
#include <rapformats/WxObs.hh>
#include "Args.hh"
#include "Params.hh"
using namespace std;

////////////////////////
// This class

class CwbAws2Spdb {
  
public:

  // constructor

  CwbAws2Spdb (int argc, char **argv);

  // destructor
  
  ~CwbAws2Spdb();

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

  int _processAwsFile(const char *file_path);
  int _processMdfFile(const char *file_path);
  int _processOneMinAwsFile(const char *file_path);
  int _processPrecipFile(const char *file_path);
  int _processPrecipMdfFile(const char *file_path);
  
};

#endif

