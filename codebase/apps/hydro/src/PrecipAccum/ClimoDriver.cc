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
// ClimoDriver.cc
//
// Climatology accumulation Driver object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////

#include "ClimoDriver.hh"
#include "AccumData.hh"

#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <didss/DataFileNames.hh>
using namespace std;

//////////////
// Constructor

ClimoDriver::ClimoDriver(const string &prog_name,
			 const Args &args, const Params &params)
        : MethodDriver(prog_name, args, params)
{
  
}

//////////////
// destructor

ClimoDriver::~ClimoDriver()

{
}

//////////////////////////////////////////////////
// Run - override Run method

int ClimoDriver::Run ()
{

  PMU_auto_register("ClimoDriver::Run");

  // initialize accum object

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

      PMU_auto_register("ClimoDriver::Run - checking file");
        

      time_t fileTime;
      bool dateOnly;
      if (DataFileNames::getDataTime(inputPath, fileTime, dateOnly)) {
        cerr << "ERROR - PrecipAccum::ClimoDriver::Run" << endl;
        cerr << "  Cannot get date/time of file: " << inputPath << endl;
        cerr << "  File will not be processed" << endl;
        continue;
      }

      // is this within our specified climo time period

      DateTime dtime(fileTime);
      DateTime stime(dtime.getYear(),
                     _params.climo_start_month, _params.climo_start_day,
                     0, 0, 0);
      DateTime etime(dtime.getYear(),
                     _params.climo_end_month, _params.climo_end_day,
                     0, 0, 0);

      if (stime.utime() <= dtime.utime() &&
          dtime.utime() <= etime.utime()) {
        
        PMU_auto_register("ClimoDriver::Run - reading in file");

        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "Climo - selecting file: " << inputPath << endl;
        }
        
        // process this file
        
        _accum->processFile(inputPath);

      }
      
    } // while ((inputPath ...
    
  } // iday
  
  if (_accum->dataFound()) {

    if (_accum->computeAndWrite(_args.startTime,
                                _args.endTime,
                                _args.endTime)) {
      cerr << "ERROR - ClimoDriver::Run" << endl;
      return -1;
    }

  }

  return 0;

}

      
