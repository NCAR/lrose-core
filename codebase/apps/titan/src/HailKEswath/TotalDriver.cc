/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 ** Copyright UCAR (c) 1992 - 1997
 ** University Corporation for Atmospheric Research(UCAR)
 ** National Center for Atmospheric Research(NCAR)
 ** Research Applications Program(RAP)
 ** P.O.Box 3000, Boulder, Colorado, 80307-3000, USA
 ** All rights reserved. Licenced use only.
 ** Do not copy or distribute without authorization
 ** 1997/9/26 14:18:54
 *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
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
#include "MaxData.hh"
#include "OutputFile.hh"

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
  
  // set up accumulation data object

  MaxData maxData(_progName, _params);
  maxData.setTargetPeriod(_args.endTime - _args.startTime);
  
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
			   _params.input_hail_data_dir,
			   periodStart, periodEnd);
  
    // accumulate for all of the files

    char *inputPath;
    while ((inputPath = inputFiles.next()) != NULL) {
      
      PMU_auto_register("TotalDriver::Run - reading in file");
      
      // process this file
      
      maxData.processFile(inputPath);
      
    } // while ((inputPath ...
    
  } // iday
  
  if (maxData.dataFound()) {

    PMU_auto_register("TotalDriver::Run - data found");

    // write out

    OutputFile out(_progName, _params);

    if (out.write(_args.startTime,
		  _args.endTime,
		  _args.endTime,
		  maxData.grid(), 
		  maxData.maxHailKeFlux(),
		  maxData.maxHailMassFlux())) {
      cerr << "ERROR - TotalDriver::Run" << endl;
      return -1;
    }

  }

  return 0;

}

      
