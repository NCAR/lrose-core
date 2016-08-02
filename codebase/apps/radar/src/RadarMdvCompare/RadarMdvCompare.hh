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
// RadarMdvCompare object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2007
//
///////////////////////////////////////////////////////////////

#ifndef RadarMdvCompare_HH
#define RadarMdvCompare_HH

#include <cstdio>
#include <string>
#include <Mdv/DsMdvx.hh>

#include "Args.hh"
#include "Params.hh"
#include "Trigger.hh"

using namespace std;

/////////////////////////
// Forward declarations

class RadarMdvCompare {
  
public:

  // constructor

  RadarMdvCompare (int argc, char **argv);

  // destructor
  
  ~RadarMdvCompare();

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
  Trigger *_trigger;
  
  DsMdvx _primary;
  DsMdvx _secondary;

  time_t _primaryTime;
  time_t _secondaryTime;

  FILE *_statsFile;
  FILE *_tableFile;
  FILE *_summaryFile;

  string _statsPath;
  string _tablePath;
  string _summaryPath;
  
  // Methods
  
  void _createTrigger();
  int _readPrimary(time_t requestTime);
  int _readSecondary(time_t requestTime);
  int _compareFiles();
  void _computeDiffStats(vector<double> &diffs);
  int _openOutputFiles();
  int _writeResultsToSpdb(double count, double mean,
                          double sdev, double median);

};

#endif
