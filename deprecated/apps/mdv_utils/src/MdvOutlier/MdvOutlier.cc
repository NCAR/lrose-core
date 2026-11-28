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
// $Id: MdvOutlier.cc,v 1.4 2019/03/04 00:22:24 dixon Exp $
//
// MdvOutlier
//
// Yan Chen, RAL, NCAR
//
// Jan. 2008
//
///////////////////////////////////////////////////////////////

#include <string>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include "CheckOutlier.hh"
#include "MdvOutlier.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvOutlier::MdvOutlier(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvOutlier";
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
  }

  if (!isOK) {
    return;
  }

  _numChildren = 0;
  _maxChildren = 5;

  // init process mapper registration
  
  PMU_auto_init(const_cast<char*>(_progName.c_str()), _params.instance, 
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

//////////////////////////
// destructor

MdvOutlier::~MdvOutlier()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvOutlier::Run ()
{

  PMU_auto_register("MdvOutlier::Run");

  int ret = 0;

  if (_params.mode == Params::REALTIME) {

    ret = _processRealtime();

  } else if (_params.mode == Params::ARCHIVE) {

    ret = _processArchive();

  }

  return (ret);
}

//////////////////////////////////////////////////
// process in REALTIME mode

int MdvOutlier::_processRealtime() {

  int ret = 0;

  DsLdataInfo stdLdata(_params.std_dev_url_dir, _params.debug);
  DsLdataInfo meanLdata(_params.mean_url_dir, _params.debug);

  if (_params.debug) {
    cerr << "Monitoring..." << endl;
  }

  time_t timeLastPrint = 0;
  time_t inLatestTime = 0;

  stdLdata.setReadFmqFromStart(true);
  meanLdata.setReadFmqFromStart(true);

  while (true) {

    // register with procmap

    PMU_auto_register("Looking for new data ...");

    bool stdHasNew = stdLdata.read(_params.max_realtime_valid_age) == 0;
    bool meanHasNew = meanLdata.read(_params.max_realtime_valid_age) == 0;

    if (stdHasNew || meanHasNew) {

      if (stdHasNew) {
        inLatestTime = stdLdata.getLatestTime();

        if (_params.debug) {
          cerr << "Found data: " << ctime(&inLatestTime) << endl;
          cerr << "  Url: " << _params.std_dev_url_dir << endl;
	}

        if (_arrivedSet.find(inLatestTime) == _arrivedSet.end()) {

	  // don't have a pair, save it
          _arrivedSet.insert(inLatestTime);

	} else {

	  // have a pair, process it
          ret = _processRealtimeInChild(inLatestTime);

          _arrivedSet.erase(inLatestTime);
	}

      }

      if (meanHasNew) {
        inLatestTime = meanLdata.getLatestTime();

        if (_params.debug) {
          cerr << "Found data: " << ctime(&inLatestTime) << endl;
          cerr << "  Url: " << _params.mean_url_dir << endl;
	}

        if (_arrivedSet.find(inLatestTime) == _arrivedSet.end()) {

	  // don't have a pair, save it
          _arrivedSet.insert(inLatestTime);

	} else {

	  // have a pair, process it
          ret = _processRealtimeInChild(inLatestTime);

          _arrivedSet.erase(inLatestTime);
	}
      }

      timeLastPrint = 0;

    } else {

      if (_params.debug) {
        time_t now = time(NULL);
        if ((now - timeLastPrint > 120) || (now % 60 == 0)) {
          cerr << "MdvOutlier: waiting for new data at " << ctime(&now)
               << " Sleeping..." << endl;
          timeLastPrint = now;
        }
      }

      // while we are waiting, check if we can process the waiting list.

      int waitSize = timesWaitingToProcess.size();
      if (waitSize > 0 && _numChildren < _maxChildren) {
        time_t process_time = timesWaitingToProcess.front();
        _processRealtimeInChild(process_time);
        timesWaitingToProcess.pop_front();
      }

      // sleep milliseconds
      umsleep(_params.sleep_secs * 1000);
    }

  }

  return ret;
}

//////////////////////////////////////////////////
// process realtime in child process

int MdvOutlier::_processRealtimeInChild(time_t process_time) {

  // make sure we don't have too many child processes running
  if (_numChildren >= _maxChildren) {
    timesWaitingToProcess.push_back(process_time);
    return 0;
  }

  pid_t childPid = fork();

  if (childPid == 0) {

    if (_params.debug) {
      cerr << "Child processing: " << getpid()
           << " for data: " << ctime(&process_time) <<endl;
    }

    CheckOutlier checkOutlier(_progName, _params);

    checkOutlier.stddevMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.std_dev_url_dir,
      0,            // search margin
      process_time, // search time
      0             // forecast lead time
    );
    checkOutlier.meanMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.mean_url_dir,
      0,
      process_time,
      0
    );

    if (checkOutlier.check()) {
      cerr << "ERROR: MdvOutlier::_processRealtimeInChild()" << endl;
      cerr << " CheckOutlier::check() failed." << endl;
      cerr << " time: " << ctime(&process_time) << endl;
    }

    if (_params.debug) {
      cerr << "Child process " << getpid() << " exit..." << endl;
    }

    exit(0);
  }

  // parent, return
  _numChildren++;

  return(0);
}

//////////////////////////////////////////////////
// process in ARCHIVE mode

int MdvOutlier::_processArchive() {

  CheckOutlier checkOutlier(_progName, _params);

  checkOutlier.stddevMdvx.setReadPath(_params.std_dev_path);
  checkOutlier.meanMdvx.setReadPath(_params.mean_path);

  int iret = checkOutlier.check();

  return (iret);
}

//////////////////////////////////////////////////////////
// bookkeeping for the number of child processes.
//

void MdvOutlier::childProcessesReduced(void) {

  _numChildren--;
}

