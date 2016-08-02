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
// ClutterCompute.cc
//
// ClutterCompute object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// ClutterCompute computes the median values from a series of MDV files.
//
///////////////////////////////////////////////////////////////

#include <vector>
#include <list>

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/MemBuf.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRemapLut.hh>
#include <Mdv/DsMdvxInput.hh>
#include "ClutterCompute.hh"
#include "ComputeMgr.hh"
using namespace std;

// Constructor

ClutterCompute::ClutterCompute(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "ClutterCompute";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  In ARCHIVE mode, must specify start and end dates."
	   << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
    }
  }

  if (_params.field_names_n < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Must specify at least one field." << endl << endl;
    isOK = false;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  return;

}

// destructor

ClutterCompute::~ClutterCompute()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int ClutterCompute::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::REALTIME) {

    if (_params.debug) {
      cerr << "Running in REALTIME mode" << endl;
    }

    // create object to watch for new files

    DsMdvxInput watch;
    if (watch.setRealtime(_params.input_dir, 600,
			  PMU_auto_register)) {
      cerr << "ERROR - ClutterCompute::Run" << endl;
      cerr << "  Cannot setup input object to watch dir: "
	   << _params.input_dir << endl;
      return -1;
    }
    
    // get the next data time
    
    time_t latestTime;
    while (watch.getTimeNext(latestTime) == 0) {
      
      // create DsMdvxInput object to get data from this time
      // back by the specified period
      
      DsMdvxInput input;
      if (input.setArchive(_params.input_dir,
			   latestTime - _params.realtime_past_period,
			   latestTime)) {
	cerr << "ERROR - ClutterCompute::Run" << endl;
	cerr << "  Cannot setup input object for dir: "
	     << _params.input_dir << endl;
	return -1;
      }
      
      // process this input
      
      iret = _process(input);
      
    } // while
    
  } else if (_params.mode == Params::ARCHIVE) {

    if (_params.debug) {
      cerr << "Running in ARCHIVE mode" << endl;
      cerr << "  Start time: " << DateTime::strn(_args.startTime) << endl;
      cerr << "  End   time: " << DateTime::strn(_args.endTime) << endl;
    }

    DsMdvxInput input;
    if (input.setArchive(_params.input_dir,
			 _args.startTime,
			 _args.endTime)) {
      cerr << "ERROR - ClutterCompute::Run" << endl;
      cerr << "  Cannot setup input object for dir: "
	   << _params.input_dir << endl;
      return -1;
    }
    
    // process this input
    
    iret = _process(input);

  } else if (_params.mode == Params::FILELIST) {

    if (_params.debug) {
      cerr << "Running in FILELIST mode" << endl;
    }

    DsMdvxInput input;
    if (input.setFilelist(_args.inputFileList)) {
      cerr << "ERROR - ClutterCompute::Run" << endl;
      cerr << "  Cannot setup input object for dir: "
	   << _params.input_dir << endl;
      return -1;
    }
    
    // process this input
    
    iret = _process(input);

  }

  return iret;

}

//////////////////////////////////////////////////
// process the input files

int ClutterCompute::_process(DsMdvxInput &input)

{

  // register with procmap
  
  PMU_auto_register("_process");

  // create the compute manager object

  ComputeMgr computeMgr(_params);

  // scan the files

  if (computeMgr.scanFiles(input)) {
    cerr << "ERROR - ClutterCompute::_process" << endl;
    return -1;
  }

  // allocate the arrays

  if (computeMgr.allocArrays()) {
    cerr << "ERROR - ClutterCompute::_process" << endl;
    return -1;
  }
  
  // compute the medians

  if (computeMgr.computeMedian(input)) {
    cerr << "ERROR - ClutterCompute::_process" << endl;
    return -1;
  }

  // write the output file

  if (computeMgr.writeOutput(input)) {
    cerr << "ERROR - ClutterCompute::_process" << endl;
    return -1;
  }

  return 0;

}

