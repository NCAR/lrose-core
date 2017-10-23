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
// HsrlMon.hh
//
// HsrlMon object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// Oct 2015
// 
///////////////////////////////////////////////////////////////
//
// HsrlMon read UW HSRL raw data files in NetCDF format,
// extracts data for monitoring, and then writes out text 
// files summarizing the monitoring information.
// This is intended for transmission to the field catalog
//
////////////////////////////////////////////////////////////////

#ifndef HsrlMon_HH
#define HsrlMon_HH

#include "Args.hh"
#include "Params.hh"
#include <string>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/Radx.hh>
#include <physics/IcaoStdAtmos.hh>

using namespace std;

class HsrlMon {
  
public:

  // constructor
  
  HsrlMon (int argc, char **argv);

  // destructor
  
  ~HsrlMon();

  // run 

  int Run();

  // data members

  int OK;

protected:
private:

  string _progName;
  char *_paramsPath;
  Args _args;
  Params _params;
  vector<string> _readPaths;

  RadxTime _realtimeScheduledTime;

  int _runFilelist();
  int _runArchive();
  int _runRealtime();

  int _processFile(const string &filePath);

  int _processFileFromList(const string &filePath);
  int _performMonitoring(time_t startTime, time_t endTime);
  int _performMonitoring(const string &filePath,
                         time_t startTime,
                         time_t endTime);
  int _findFiles(time_t startTime,
                 time_t endTime,
                 vector<string> &filePaths);

  int _findFilesForDay(time_t startTime,
                       time_t endTime,
                       string dayDir,
                       vector<string> &filePaths);

};

#endif

