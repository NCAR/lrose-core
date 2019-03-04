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
// $Id: MdvBlend.cc,v 1.7 2019/03/04 00:22:24 dixon Exp $
//
// MdvBlend
//
// Yan Chen, RAL, NCAR
//
// Dec. 2007
//
///////////////////////////////////////////////////////////////

#include <string>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <Mdv/DsMdvxTimes.hh>
#include "Blender.hh"
#include "MdvBlend.hh"

using namespace std;

////////////////////////////////////////////
// Constructor

MdvBlend::MdvBlend(int argc, char **argv)

{

  isOK = true;
  
  // set programe name

  _progName = "MdvBlend";
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

  // parse the start and end time

  if (
    sscanf(
      _params.start_date_time, "%d %d %d %d %d %d",
      &startTime.year, &startTime.month, &startTime.day,
      &startTime.hour, &startTime.min, &startTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&startTime);
  }

  if (
    sscanf(
      _params.end_date_time, "%d %d %d %d %d %d",
      &endTime.year, &endTime.month, &endTime.day,
      &endTime.hour, &endTime.min, &endTime.sec
    ) != 6
  ) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Cannot parse start_date_time: "
         << "\"" << _params.start_date_time << "\"" << endl;
    cerr << "Problem with TDRP parameters." << endl;
    isOK = false;
  } else {
    uconvert_to_utime(&endTime);
  }

  // check if start time is later than end time

  if (startTime.unix_time > endTime.unix_time) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Start time is later than end time." << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (startTime.unix_time == 0 || endTime.unix_time == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode, must specify start and end dates."
	   << endl << endl;
      _args.usage(cerr);
      isOK = false;
    }
  }

  // check blending weight

  if (_params.max_blending_weight < 0) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Max blending weight cannot be negative." << endl;
    isOK = false;
  }

  // check blending zone

  if (
    _params.blending_zone.southern_limit >
    _params.blending_zone.northern_limit
  ) {
    cerr << "Warning: " << _progName << endl;
    cerr << "  southern_limit is greater than northern_limit." << endl;
    cerr << "  They will be switched." << endl;
    double tmp = _params.blending_zone.southern_limit;
    _params.blending_zone.southern_limit =
       _params.blending_zone.northern_limit;
    _params.blending_zone.northern_limit = tmp;
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

MdvBlend::~MdvBlend()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvBlend::Run ()
{

  PMU_auto_register("MdvBlend::Run");

  int ret = 0;

  if (_params.mode == Params::REALTIME) {

    ret = _processRealtime();

  } else if (_params.mode == Params::ARCHIVE) {

    ret = _processArchive(startTime.unix_time, endTime.unix_time);

  } else if (_params.mode == Params::FILELIST) {

    ret = _processFilelist();

  }

  return (ret);
}

//////////////////////////////////////////////////
// process in REALTIME mode

int MdvBlend::_processRealtime() {

  int ret = 0;

  DsLdataInfo outLdata(_params.output_url_dir, _params.debug);
  DsLdataInfo shLdata(_params.input_sh_url, _params.debug);
  DsLdataInfo nhLdata(_params.input_nh_url, _params.debug);

  int readOut = outLdata.read();
  int readSh = shLdata.read();
  int readNh = nhLdata.read();

  time_t outLatestTime = outLdata.getLatestTime();
  time_t shLatestTime = shLdata.getLatestTime();
  time_t nhLatestTime = nhLdata.getLatestTime();

  time_t inLatestTime =
    shLatestTime < nhLatestTime ? shLatestTime : nhLatestTime;

  if (_params.debug) {
    cerr << "Pre-processing..." << endl;
    cerr << "Output latest time: " << ctime(&outLatestTime) << endl;
    cerr << "Input latest time:  " << ctime(&inLatestTime) << endl;
  }

  if (outLatestTime < inLatestTime) {

    // process all the data between outLatestTime and inLatestTime.

    ret = _processArchiveInChild(outLatestTime + 3600, inLatestTime);

    if (_params.debug) {
      cerr << "pre-processing result: " << ret << endl;
    }

  } else if (outLatestTime > inLatestTime) {

    // re-process inLatestTime

    ret = _processArchiveInChild(inLatestTime, inLatestTime);

    if (_params.debug) {
      cerr << "pre-processing result: " << ret << endl;
    }

  }

  // pre-processing is done. Monitor two input directories for latest data.

  if (_params.debug) {
    cerr << "Monitoring..." << endl;
  }

  time_t timeLastPrint = 0;

  shLdata.setReadFmqFromStart(true);
  nhLdata.setReadFmqFromStart(true);

  while (true) {

    // register with procmap

    PMU_auto_register("Looking for new data ...");

    bool shHasNew = shLdata.read(_params.max_realtime_valid_age) == 0;
    bool nhHasNew = nhLdata.read(_params.max_realtime_valid_age) == 0;

    if (shHasNew || nhHasNew) {

      if (shHasNew) {
        inLatestTime = shLdata.getLatestTime();

        if (_params.debug) {
          cerr << "Found data: " << ctime(&inLatestTime) << endl;
          cerr << "  Url: " << _params.input_sh_url << endl;
	}

	// check if data available from NH

        if (arrivedNHSet.find(inLatestTime) == arrivedNHSet.end()) {

	  // don't have a pair, save it
          arrivedSHSet.insert(inLatestTime);

	} else {

	  // have a pair, process it
          ret = _processRealtimeInChild(inLatestTime);

          arrivedNHSet.erase(inLatestTime);
	}

      }

      if (nhHasNew) {
        inLatestTime = nhLdata.getLatestTime();

        if (_params.debug) {
          cerr << "Found data: " << ctime(&inLatestTime) << endl;
          cerr << "  Url: " << _params.input_nh_url << endl;
	}

	// check if data available from SH

        if (arrivedSHSet.find(inLatestTime) == arrivedSHSet.end()) {

	  // don't have a pair, save it
          arrivedNHSet.insert(inLatestTime);

	} else {

	  // have a pair, process it
          ret = _processRealtimeInChild(inLatestTime);

          arrivedSHSet.erase(inLatestTime);
	}
      }

      timeLastPrint = 0;

    } else {

      if (_params.debug) {
        time_t now = time(NULL);
        if ((now - timeLastPrint > 120) || (now % 60 == 0)) {
          cerr << "MdvBlend: waiting for new data at " << ctime(&now)
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

  } // while

  return ret;
}

//////////////////////////////////////////////////
// process realtime in child process

int MdvBlend::_processRealtimeInChild(time_t process_time) {

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

    DsMdvx shMdvx;
    DsMdvx nhMdvx;

    shMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_sh_url,
      0,            // search margin
      process_time, // search time
      0             // forecast lead time
    );
    nhMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_nh_url,
      0,
      process_time,
      0
    );

    Blender blender(_progName, _params);

    if (blender.blendFiles(shMdvx, nhMdvx)) {
      cerr << "ERROR: MdvBlend::_processRealtimeInChild()" << endl;
      cerr << " Blender::blendFiles() failed." << endl;
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
// processing in child process

int MdvBlend::_processArchiveInChild(
  time_t start_time,
  time_t end_time
) {

  pid_t childPid = fork();

  if (childPid == 0) {
    if (_params.debug) {
      cerr << "Child processing: " << getpid() << endl;
    }

    int ret = _processArchive(start_time, end_time);

    if (_params.debug) {
      cerr << "Child process " << getpid() << " exit: " << ret << endl;
    }

    exit(ret);
  }

  // parent, return
  _numChildren++;

  return(0);
}

//////////////////////////////////////////////////
// process in ARCHIVE mode

int MdvBlend::_processArchive(
  time_t start_time,
  time_t end_time
) {

  if (start_time > end_time) {
    cerr << "ERROR: MdvBlend::_processArchive()" << endl;
    cerr << "  Start time is later than end time." << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Start time: " << ctime(&start_time) << endl;
    cerr << "End time: " << ctime(&end_time) << endl;
  }

  DsMdvxTimes mdv_time_list_sh;
  int iret = mdv_time_list_sh.setArchive(
    _params.input_sh_url,
    start_time,
    end_time
  );
  if (iret) {
    cerr << "ERROR: MdvBlend::_processArchive()" << endl;
    cerr << "url: " << _params.input_sh_url << endl;
    cerr << mdv_time_list_sh.getErrStr() << endl;
    return -1;
  }

  DsMdvxTimes mdv_time_list_nh;
  iret = mdv_time_list_nh.setArchive(
    _params.input_nh_url,
    start_time,
    end_time
  );
  if (iret) {
    cerr << "ERROR: MdvBlend::_processArchive()" << endl;
    cerr << "url: " << _params.input_nh_url << endl;
    cerr << mdv_time_list_nh.getErrStr() << endl;
    return -1;
  }

  const vector<time_t> sh_timelist = mdv_time_list_sh.getArchiveList();
  const vector<time_t> nh_timelist = mdv_time_list_nh.getArchiveList();

  int n_sh_files = sh_timelist.size();
  int n_nh_files = nh_timelist.size();

  if (_params.debug == Params::DEBUG_VERBOSE) {
    cerr << "Number of files found from SH: " << n_sh_files << endl;
    cerr << "Number of files found from NH: " << n_nh_files << endl;
  }

  if (n_sh_files == 0) {
    cerr << "ERROR: MdvBlend::_processArchive()" << endl;
    cerr << "  Could not find files, url: "
         << _params.input_sh_url << endl;
    return -1;
  }
  if (n_nh_files == 0) {
    cerr << "ERROR: MdvBlend::_processArchive()" << endl;
    cerr << "  Could not find files, url: "
         << _params.input_nh_url << endl;
    return -1;
  }

  int n_files = n_sh_files + n_nh_files;

  Blender blender(_progName, _params);

  int sh_index = 0;
  int nh_index = 0;
  for (int file_index = 0; file_index < n_files; file_index++) {
    if (sh_index >= n_sh_files)
      break;
    if (nh_index >= n_nh_files)
      break;

    time_t sh_time = sh_timelist.at(sh_index);
    time_t nh_time = nh_timelist.at(nh_index);
    if (sh_time > nh_time) {
      cerr << "File from SH is missing, time: " << ctime(&nh_time);
      cerr << "Skipped to the next time." << endl << endl;

      nh_index++;
      continue;
    }

    if (sh_time < nh_time) {
      cerr << "File from NH is missing, time: " << ctime(&sh_time);
      cerr << "Skipped to the next time." << endl << endl;

      sh_index++;
      continue;
    }

    // sh_time and nh_time are the same

    DsMdvx shMdvx;
    DsMdvx nhMdvx;

    shMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_sh_url,
      0,       // search margin
      sh_time, // search time
      0        // forecast lead time
    );
    nhMdvx.setReadTime(
      Mdvx::READ_CLOSEST,
      _params.input_nh_url,
      0,
      nh_time,
      0
    );

    if (blender.blendFiles(shMdvx, nhMdvx)) {
      cerr << "ERROR: MdvBlend::_processArchive()" << endl;
      cerr << " Blender::blendFiles() failed." << endl;
      cerr << " time: " << utimstr(sh_time) << endl << endl;
    }

    sh_index++;
    nh_index++;
  }

  return 0;

}

//////////////////////////////////////////////////
// process in FILELIST mode

int MdvBlend::_processFilelist() {

  Blender blender(_progName, _params);

  for (int i = 0; i < _params.input_files_n; i++) {
    DsMdvx shMdvx;
    DsMdvx nhMdvx;

    string sh_path(_params.input_sh_url);
    sh_path += string(_params._input_files[i].sh_file);
    shMdvx.setReadPath(sh_path);

    string nh_path(_params.input_nh_url);
    nh_path += string(_params._input_files[i].nh_file);
    nhMdvx.setReadPath(nh_path);

    if (blender.blendFiles(shMdvx, nhMdvx)) {
      cerr << "ERROR: MdvBlend::_processFilelist()" << endl;
      cerr << " Blender::blendFiles() failed." << endl;
      cerr << " path: " << sh_path << endl;
      cerr << "       " << nh_path << endl;
      continue;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////
// bookkeeping for the number of child processes.
//

void MdvBlend::childProcessesReduced(void) {

  _numChildren--;
}
