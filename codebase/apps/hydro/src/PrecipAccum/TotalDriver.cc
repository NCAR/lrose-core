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
  
  // initialize accumulation data object
  
  _accum->init();
  _accum->setTargetPeriod(_args.endTime - _args.startTime);
  
  // read the files a day at a time
  
  int startDay = _args.startTime / SECS_IN_DAY;
  int endDay = _args.endTime / SECS_IN_DAY;
  
  for (int iday = startDay; iday <= endDay; iday++) {

    time_t periodStart;
    if (iday == startDay) {
      periodStart = _args.startTime + 1;
    } else {
      periodStart = iday * SECS_IN_DAY + 1;
    }
    
    time_t periodEnd;
    if (iday == endDay) {
      periodEnd = _args.endTime;
    } else {
      periodEnd = (iday + 1) * SECS_IN_DAY - 1;
    }

    // get the file list
    
    DsInputPath inputFiles(_progName, _params.debug,
			   _params.input_rdata_dir,
			   periodStart, periodEnd);
  
    // accumulate for all of the files

    char *inputPath;
    while ((inputPath = inputFiles.next()) != NULL) {
      
      PMU_auto_register("TotalDriver::Run - reading in file");
      
      // process this file
      
      _accum->processFile(inputPath);
      
    } // while ((inputPath ...
    
  } // iday
  
  if (_accum->dataFound()) {

    PMU_auto_register("TotalDriver::Run - data found");

    if (_accum->computeAndWrite(_args.startTime,
                                _args.endTime,
                                _args.endTime)) {
      cerr << "ERROR - TotalDriver::Run" << endl;
      return -1;
    }

  }

  return 0;

}

      
