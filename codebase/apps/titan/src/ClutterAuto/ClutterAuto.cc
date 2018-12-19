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
// ClutterAuto.cc
//
// ClutterAuto object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2003
//
///////////////////////////////////////////////////////////////
//
// ClutterAuto computes clutter auto-correlation statistics
//
////////////////////////////////////////////////////////////////

#include "ClutterAuto.hh"
#include "StatsData.hh"
#include "OutputFile.hh"

using namespace std;

// Constructor

ClutterAuto::ClutterAuto(int argc, char **argv)

{

  isOK = TRUE;
  _trigger = NULL;
  
  // set programe name
  
  _progName = "ClutterAuto";
  ucopyright((char *) _progName.c_str());
  
  // get command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
    return;
  }

  // check params
  
  // check start and end in ARCHIVE mode

  if ((_params.mode == Params::ARCHIVE) &&
      (_args.startTime == 0 || _args.endTime == 0)) {
    cerr << "ERROR - ClutterAuto" << endl;
    cerr << "  In ARCHIVE mode start and end times must be specified." << endl;
    isOK = false;
    return;
  }

  // create the trigger

  if (_params.mode == Params::REALTIME) {

    _trigger = new RealtimeTrigger(_progName, _params);

  } else {
    
    _trigger = new ArchiveTrigger(_progName, _params,
				  _args.startTime,  _args.endTime);
    
  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

ClutterAuto::~ClutterAuto()

{

  if (_trigger) {
    delete(_trigger);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutterAuto::Run ()
{

  PMU_auto_register("Start of Run");
  
  // loop through times
  
  while ((_triggerTime = _trigger->next()) >= 0) {
    
    PMU_auto_register("Run - got trigger");
    
    if (_params.debug) {
      cerr << "----> Trigger time: " << utimstr(_triggerTime) << endl;
    }
    
    // compute the stats
    
    if (_computeStats()) {
      cerr << "ERROR - ClutterAuto::Run" << endl;
      cerr << "  Computation failed" << endl;
      cerr << "  Trigger time: " << utimstr(_triggerTime) << endl;
    }
    
  } // while
  
  return 0;
  
}

/////////////////////////////////////////////////////
// Compute stats

int ClutterAuto::_computeStats()
{
  
  PMU_auto_register("_computeStats");
  
  // get list of input files in the running time period
  
  time_t periodStart = _triggerTime - _params.lookback_duration + 1;
  time_t periodEnd = _triggerTime;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Computing stats for period %s to %s\n",
	    utimstr(periodStart), utimstr(periodEnd));
  }
  
  DsMdvx mdvx;
  mdvx.setTimeListModeValid(_params.input_url, periodStart, periodEnd);
  if (mdvx.compileTimeList()) {
    cerr << "ERROR - ClutterAuto::_computeStats()" << endl;
    cerr << "  Cannot compile time list" << endl;
    cerr << "  " << mdvx.getErrStr() << endl;
    return -1;
  }

  // set up stats data object
  
  StatsData stats(_progName, _params);

  // loop through the times

  for (int ii = 0; ii < mdvx.getNTimesInList(); ii++) {

    PMU_auto_register("reading in file");

    // process this file
    
    time_t fileTime = mdvx.getTimeFromList(ii);
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "---> getting data for time: " << utimstr(fileTime) << endl;
    }
    if (stats.processTime(fileTime)) {
      cerr << "ERROR - ClutterAuto::_computeStats" << endl;
      cerr << "  Cannot process file at time: " << utimstr(fileTime) << endl;
    }
    
  } // ii

  if (_params.debug) {
    cerr << "Computing stats" << endl;
  }
  
  if (stats.compute()) {
    cerr << "ERROR - ClutterAuto::_computeStats" << endl;
    cerr << "  Could not compute stats" << endl;
    return -1;
  }
  
  PMU_auto_register("stats computed");
  
  // write out
  
  OutputFile out(_progName, _params);
  
  if (out.write(periodStart,
		periodEnd,
		periodEnd,
		stats.getMhdr(), 
		stats.getFhdr(), 
		stats.getVhdr(), 
		stats.getFrac(),
		stats.getMean(),
		stats.getSdev(),
		stats.getCorr())) {
    cerr << "ERROR - ClutterAuto::_computeStats" << endl;
    cerr << "  Could not write out data" << endl;
    return -1;
  }
  
  return 0;

}


