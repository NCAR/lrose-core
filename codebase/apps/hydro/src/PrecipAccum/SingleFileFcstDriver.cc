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
// SingleFileFcstDriver.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////
//
// Single-file accumulation derived class
//
//////////////////////////////////////////////////////////////


#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
#include <dsdata/DsLdataTrigger.hh>

#include "SingleFileFcstDriver.hh"
#include "Trigger.hh"
#include "AccumData.hh"

using namespace std;

//////////////
// Constructor

SingleFileFcstDriver::SingleFileFcstDriver(const string &prog_name,
				   const Args &args, const Params &params)
  : MethodDriver(prog_name, args, params)
{

}

//////////////
// destructor

SingleFileFcstDriver::~SingleFileFcstDriver()

{

}

//////////////////////////////////////////////////
// Run

int SingleFileFcstDriver::Run ()
{

  DsLdataTrigger *trigger = new DsLdataTrigger();
  if (trigger->init(_params.input_rdata_dir, 600,
		    PMU_auto_register) != 0)
  {
    cerr << trigger->getErrStr() << endl;
    return (-1);
  }
      
  _dataTrigger = trigger;

  // register with procmap
  
  PMU_auto_register("MethodDriver::Run");

  while (!_dataTrigger->endOfData())
  {
      TriggerInfo triggerInfo;
      _dataTrigger->next(triggerInfo);
      if (_doAccum(triggerInfo.getIssueTime(),
		   triggerInfo.getForecastTime() - triggerInfo.getIssueTime(), 
		   triggerInfo.getFilePath()))
      {
        cerr << "SingleFileFcstDriver::Run" <<"  Errors in processing time: "
             <<  triggerInfo.getIssueTime()
             << " input file: " << triggerInfo.getFilePath() << endl;
	      
	return (-1);
	
      }
    } // while

  return (0);
  
}

/////////////////////////////////////////////////////
// SingleFileFcstDriver::_doAccum()
//
// Compute running accumulations, writing out each time.
//

int SingleFileFcstDriver::_doAccum(time_t inputTime, int leadTime, const string filePath)
{

  cerr << "SingleFileFcstDriver::_doAccum\n";
  cerr << "inputTime = " << DateTime(inputTime) << endl;
  cerr << "leadTime = " << leadTime << endl;
  cerr << endl;
  
  PMU_auto_register("SingleFileFcstDriver::_doAccum");

  // get just a single file
  
  time_t periodStart = inputTime;
  time_t periodEnd = inputTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "SingleFileFcstDriver: period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }
  
  // initialize accumulation data object
  
  _accum->init();
  _accum->setTargetPeriod((double) _params.running_duration);

  // accumulate

  if (_accum->processFile(filePath, inputTime, leadTime)) {
    cerr << "SingleFileFcstDriver::_doAccum" << endl;
    cerr << "  Cannot process file: " << filePath << endl;
    cerr << "  " << DateTime::str() << endl;
    }

  if (_accum->dataFound()) {
    
    PMU_auto_register("SingleFileFcstDriver::_doAccum - data found");

    if (_params.debug) {
      cerr << "SingelFileFcstDriver:_doAccum\n";
      cerr << "accumedataCentroidTime() = " 
           << DateTime(_accum->dataCentroidTime() ) << endl;
      cerr << endl;
    }
    
    if (_accum->computeAndWrite(_accum->dataStartTime(),
                                _accum->dataEndTime(),
                                _accum->dataCentroidTime(),
                                leadTime)) {
      cerr << "ERROR - SingleFileFcstDriver::Run" << endl;
      return -1;
    }
    
  }

  return 0;

}

      
