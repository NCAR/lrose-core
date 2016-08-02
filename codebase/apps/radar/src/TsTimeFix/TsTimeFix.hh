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
////////////////////////////////////////////////////////////////////
// TsTimeFix.hh
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2014
//
/////////////////////////////////////////////////////////////////////
//
// TsTimeFix reads raw time-series data, adjusts the time of selected
// components, and writes the results out to a specified directory.
//
/////////////////////////////////////////////////////////////////////

#ifndef TsTimeFix_H
#define TsTimeFix_H

#include <string>
#include <vector>

#include "Args.hh"
#include "Params.hh"
#include <Fmq/DsFmq.hh>
#include <radar/IwrfTsReader.hh>
#include <radar/iwrf_data.h>
#include <Radx/RadxTime.hh>

using namespace std;

////////////////////////
// This class

class TsTimeFix {
  
public:

  // constructor

  TsTimeFix(int argc, char **argv);

  // destructor
  
  ~TsTimeFix();

  // run 

  int Run();

  // data members

  bool isOK;

protected:
  
private:

  string _progName;
  Args _args;
  char *_paramsPath;
  Params _params;

  // reading data in

  IwrfTsReader *_masterReader;
  IwrfTsReader *_georefReader;
  
  // Pulse count and sequence number

  int _inputPulseCount;
  int _outputPulseCount;
  si64 _prevPulseSeqNum;
  si64 _prevGeorefSeqNum;

  // georeference

  iwrf_platform_georef_t _pulseGeoref;
  iwrf_platform_georef_t _prevGeoref;
  iwrf_platform_georef_t _latestGeoref;
  RadxTime _prevGeorefTime;
  RadxTime _latestGeorefTime;
  RadxTime _usedGeorefTime;
  bool _georefsFound;
  
  // output file

  FILE *_out;
  time_t _outputTime;
  string _outputDir;
  string _outputName;
  string _relPath;

  // functions

  int _processPulse(IwrfTsPulse *pulse);
  void _adjustTime(IwrfTsPulse *pulse, bool &georefFound);
  int _getMatchingGeoref(const IwrfTsPulse *pulse);
  int _openNewFile(const IwrfTsPulse *pulse);
  int _closeFile();

};

#endif
