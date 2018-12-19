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
// MdvMedian.cc
//
// MdvMedian object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2002
//
///////////////////////////////////////////////////////////////
//
// MdvMedian computes the median values from a series of MDV files.
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
#include "MdvMedian.hh"
#include "ComputeMgr.hh"
using namespace std;

// Constructor

MdvMedian::MdvMedian(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = "MdvMedian";
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

MdvMedian::~MdvMedian()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvMedian::Run ()
{

  int iret = 0;

  // register with procmap
  
  PMU_auto_register("Run");

  if (_params.mode == Params::REALTIME) {

    // create object to watch for new files

    DsMdvxInput watch;
    if (watch.setRealtime(_params.input_url, 600,
			  PMU_auto_register)) {
      cerr << "ERROR - MdvMedian::Run" << endl;
      cerr << "  Cannot setup input object to watch url: "
	   << _params.input_url << endl;
      return -1;
    }
    
    // get the next data time
    
    time_t latestTime;
    while (watch.getTimeNext(latestTime) == 0) {
      
      // create DsMdvxInput object to get data from this time
      // back by the specified period
      
      DsMdvxInput input;
      if (input.setArchive(_params.input_url,
			   latestTime - _params.realtime_past_period,
			   latestTime)) {
	cerr << "ERROR - MdvMedian::Run" << endl;
	cerr << "  Cannot setup input object for url: "
	     << _params.input_url << endl;
	return -1;
      }

      // process this input
      
      iret = _process(input);
      
    } // while

  } else if (_params.mode == Params::ARCHIVE) {

    DsMdvxInput input;
    if (input.setArchive(_params.input_url,
			 _args.startTime,
			 _args.endTime)) {
      cerr << "ERROR - MdvMedian::Run" << endl;
      cerr << "  Cannot setup input object for url: "
	   << _params.input_url << endl;
      return -1;
    }
    
    // process this input
    
    iret = _process(input);

  } else if (_params.mode == Params::FILELIST) {

    DsMdvxInput input;
    if (input.setFilelist(_args.inputFileList)) {
      cerr << "ERROR - MdvMedian::Run" << endl;
      cerr << "  Cannot setup input object for url: "
	   << _params.input_url << endl;
      return -1;
    }
    
    // process this input
    
    iret = _process(input);

  }

  return iret;

}

//////////////////////////////////////////////////
// process the input files

int MdvMedian::_process(DsMdvxInput &input)

{

  // register with procmap
  
  PMU_auto_register("_process");

  // create the compute manager object

  ComputeMgr computeMgr(_params);

  // scan the files

  if (computeMgr.scanFiles(input)) {
    cerr << "ERROR - MdvMedian::_process" << endl;
    return -1;
  }

  // allocate the arrays

  if (computeMgr.allocArrays()) {
    cerr << "ERROR - MdvMedian::_process" << endl;
    return -1;
  }
  
  // compute the medians

  if (computeMgr.computeMedian(input)) {
    cerr << "ERROR - MdvMedian::_process" << endl;
    return -1;
  }

  // write the output file

  if (computeMgr.writeOutput(input)) {
    cerr << "ERROR - MdvMedian::_process" << endl;
    return -1;
  }

  return 0;

}

