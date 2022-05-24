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
// TrackGridStats.cc
//
// TrackGridStats object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "TrackGridStats.hh"
#include "TrackData.hh"
#include "TitanTrackData.hh"
#include "ModelTrackData.hh"
#include "StatsGrid.hh"
#include <toolsa/str.h>
#include <didss/DsInputPath.hh>
using namespace std;

// Constructor

TrackGridStats::TrackGridStats(int argc, char **argv)

{

  isOK = TRUE;
  _trackData = NULL;
  _input = NULL;

  // set programe name
  
  _progName = "TrackGridStats";
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
  
  if (_params.smoothing_kernel_size % 2 == 0) {
    cerr << "ERROR - TrackGridStats" << endl;
    cerr << "  Parameter smoothing_kernel_size must be odd." << endl;
    cerr << "  It is set to: " << _params.smoothing_kernel_size << endl;
    isOK = false;
    return;
  }

  if (_params.track_data_type == Params::MODEL_TRACKS) {
    if (_args.inputFileList.size() < 1) {
      cerr << "ERROR - TrackGridStats" << endl;
      cerr << "  MODEL_TRACKS mode." << endl;
      cerr << "  You must have at least 1 input file." << endl;
      isOK = false;
      return;
    }
  } else {
    if (_args.inputFileList.size() < 1 &&
	_args.startTime == 0 && _args.endTime == 0) {
      cerr << "ERROR - TrackGridStats" << endl;
      cerr << "  TITAN_TRACKS mode." << endl;
      cerr << "  You must specify start and end times." << endl;
      isOK = false;
      return;
    }
  }
    
 // file input object

  if (_params.track_data_type == Params::MODEL_TRACKS) {
    _input = new DsInputPath(_progName,
			     _params.debug >= Params::DEBUG_VERBOSE,
			     _args.inputFileList);
  } else {
    if (_args.inputFileList.size() > 0) {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _args.inputFileList);
    } else {
      _input = new DsInputPath(_progName,
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_dir,
			       _args.startTime, _args.endTime);
    }
    _input->setSearchExt(_params.input_file_ext);
  }
  
  // trackData object

  if (_params.track_data_type == Params::TITAN_TRACKS) {
    
    _trackData = new TitanTrackData(_progName, _params, *_input);

    if (!_trackData->OK) {
      fprintf(stderr, "ERROR - %s:TrackGridStats::TrackGridStats\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot create TitanTrackData object\n");
      isOK = FALSE;
    }

  } else if (_params.track_data_type == Params::MODEL_TRACKS) {
   
    _trackData = new ModelTrackData(_progName, _params, *_input);
    
    if (!_trackData->OK) {
      fprintf(stderr, "ERROR - %s:TrackGridStats::TrackGridStats\n",
	      _progName.c_str());
      fprintf(stderr, "Cannot create ModelTrackData object\n");
      isOK = FALSE;
    }

  }

  // initialize process registration
  
  PMU_auto_init(_progName.c_str(), _params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

TrackGridStats::~TrackGridStats()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  if (_input != NULL) {
    delete _input;
  }
  if (_trackData != NULL) {
    delete _trackData;
  }

}

//////////////////////////////////////////////////
// Run

int TrackGridStats::Run ()
{

  // create StatsGrid object

  StatsGrid statsGrid(_progName, _params, _args, *_trackData,
		      _args.startTime, _args.endTime);
  
  // compute the stats

  statsGrid.compute();

  // Write out

  statsGrid.writeOutputFile();

  return (0);

}

