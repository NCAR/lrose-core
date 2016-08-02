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
///////////////////////////////////////////////////////////////
// TotalDriver.cc
//
// Total accumulation Driver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2000
//
///////////////////////////////////////////////////////////////

#include "TotalDriver.hh"
#include "AccumData.hh"

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
using namespace std;

//////////////
// Constructor

TotalDriver::TotalDriver(const string &prog_name,
			 const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)
{

}

//////////////
// destructor

TotalDriver::~TotalDriver()

{
}

//////////////////////////////////////////////////
// Run - override Run method

int TotalDriver::Run ()
{

  PMU_auto_register("TotalDriver::Run");
  
  // get time limits
  
  time_t dataStart = _args.startTime;
  time_t dataEnd = _args.endTime;
  
  // load up path list and durations
  
  _loadPathList(dataStart, dataEnd);
  
  // set target period

  _accum->init();
  _accum->setTargetPeriod((double) dataEnd - (double) dataStart);
  
  // loop through the input files
  
  for (size_t ii = 0; ii < _filePaths.size(); ii++) {
    
    // process this file
    
    if (_accum->processFile(_filePaths[ii],
                            _fileTimes[ii],
                            _fileDurations[ii])) {
      cerr << "TotalDriver::_doAccum" << endl;
      cerr << "  Cannot process file: " << _filePaths[ii] << endl;
      cerr << "  File time: " << DateTime::strm(_fileTimes[ii]) << endl;
    }

  }

  // compute totals and write file

  if (_accum->write(_params.total_accum_output_url)) {
    cerr << "ERROR - TotalDriver::Run" << endl;
    return -1;
  }

  _accum->free();

  return 0;

}

      
