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
// SpectraScope.cc
//
// SpectraScope object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2000
//
///////////////////////////////////////////////////////////////
//
// Displays METAR data in strip chart form
//
///////////////////////////////////////////////////////////////

#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include "SpectraScope.hh"
using namespace std;

// Constructor

SpectraScope::SpectraScope(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;
  isOK = true;
  _coordShmem = NULL;

  // set programe name

  _progName = "SpectraScope";
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  // set up data input

  if (_params.input_mode == Params::INPUT_SPDB) {
    
    // create CIDD coord shmem 

    _coordShmem = (coord_export_t *)
      ushm_create(_params.cidd_shmem_key, sizeof(coord_export_t), 0666);
    if (_coordShmem == NULL) {
      cerr << "Could not attach shared memory for CIDD communications" << endl;
      cerr << "  shmem key: " << _params.cidd_shmem_key << endl;
      isOK = false;
    }

  } else {

    // initialize FMQ for reading

    DsFmq::openPosition openPos = DsFmq::END;
    if (_params.seek_to_start_of_fmq) {
      openPos = DsFmq::START;
    }

    while (_inputFmq.initReadBlocking(_params.spectra_fmq_url,
				      "SpectraScope",
				      _params.debug >= Params::DEBUG_VERBOSE,
				      openPos,
				      _params.fmq_read_delay_msecs)) {
      PMU_auto_register("Waiting for FMQ to be created");
      if (_params.debug) {
	cerr << "Waiting for FMQ to be created: " << _params.spectra_fmq_url << endl;
      }
      umsleep(5000);
    }

  }

  return;

}

// destructor

SpectraScope::~SpectraScope()

{

  // Detach from and remove shared memory

  if(_coordShmem != NULL) {
    ushm_detach(_coordShmem);
    if (ushm_nattach(_params.cidd_shmem_key) <= 0) {
      ushm_remove(_params.cidd_shmem_key);
    }
  }

  // free up strip chart memory

  strip_chart_free();
  
  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int SpectraScope::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // call the main entry point for strip_chart

  int iret = 0;
  if (_params.staggered_mode) {
    iret = stag_chart_main(_argc, _argv, &_params, _coordShmem, _inputFmq);
  } else {
    iret = strip_chart_main(_argc, _argv, &_params, _coordShmem, _inputFmq);
  }

  return iret;
  
}

