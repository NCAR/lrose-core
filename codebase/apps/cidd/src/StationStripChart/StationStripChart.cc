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
// StationStripChart.cc
//
// StationStripChart object
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
#include "StationStripChart.hh"
using namespace std;

// Constructor

StationStripChart::StationStripChart(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;
  isOK = true;
  _coordShmem = NULL;

  // set programe name

  _progName = "StationStripChart";
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

  // create coord shmem as appropriate

  if(_params.cidd_shmem_key) {
    _coordShmem = (coord_export_t *)
      ushm_create(_params.cidd_shmem_key, sizeof(coord_export_t), 0666);
    if (_coordShmem == NULL) {
      cerr << "Could not attach shared memory for CIDD communications" << endl;
      cerr << "  shmem key: " << _params.cidd_shmem_key << endl;
      isOK = false;
    }
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);
  
  return;

}

// destructor

StationStripChart::~StationStripChart()

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

int StationStripChart::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // call the main entry point for strip_chart

  int iret = strip_chart_main(_argc, _argv, &_params, _coordShmem);

  return iret;
  
}

