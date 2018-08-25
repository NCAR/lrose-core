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
// CronusPointSelect.cc
//
// CronusPointSelect object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 1999
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/ushmem.h>
#include <toolsa/pjg.h>
#include <toolsa/DateTime.hh>
#include <toolsa/MsgLog.hh>
#include <rapmath/math_macros.h>

#include "CronusPointSelect.hh"

#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
using namespace std;



// Constructor

CronusPointSelect::CronusPointSelect(int argc, char **argv)
  
{
  
  isOK = true;
  
  // set programe name

  _progName = "CronusPointSelect";
  ucopyright((char *) _progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = FALSE;
  }

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // Attach to Cidd's shared memory segment
  
  if ((_coordShmem =
       (coord_export_t *) ushm_create(_params.coord_shmem_key,
				      sizeof(coord_export_t),
				      0666)) == NULL) {
    cerr << "ERROR: " << _progName << endl;
    cerr <<
      "  Cannot create/attach Cidd's coord export shmem segment." << endl;
    isOK = false;
  }
  
  //
  // Set up printing objects.
  //

  for (int ip=0; ip < _params.sources_n; ip++){
    doPrint *Pr = new doPrint(&_params,ip);
    Printers.push_back( Pr );
  }

  return;

}

// destructor

CronusPointSelect::~CronusPointSelect()
  
{

  // detach from shared memory
  
  ushm_detach(_coordShmem);
  
  // unregister process
  
  PMU_auto_unregister();

  // Delete the objects we created.

  for (int ip=0; ip < _params.sources_n; ip++){
    delete Printers[ip];
  }

}

//////////////////////////////////////////////////
// Run

int CronusPointSelect::Run ()
{

  // register with procmap
  
  PMU_auto_register("Run");

  // Make sure the display is ready before using the shared memory
  
  while (!_coordShmem->shmem_ready && !_params.no_wait_for_shmem) {
    PMU_auto_register("Waiting for shmem_ready flag");
    umsleep(_params.sleep_msecs);
  }
  
  // Store the latest click number at startup
  
  int last_no = _coordShmem->pointer_seq_num - 1;
  time_t last_display_time = 0;
  
  // Now, operate forever
  
  double sum;
  while (true) {
    
    // Register with the process mapper
    
    PMU_auto_register("Checking for user input");
    
    // Check for new clicks
    
    time_t curr_time = time(NULL);
    
    if (_coordShmem->pointer_seq_num != last_no ||
	(_params.auto_click_interval > 0 &&
	 abs(curr_time - last_display_time) > _params.auto_click_interval)) {
      
      if (_coordShmem->pointer_seq_num != last_no) {
	
	if (_params.mouse_button == 0 ||
	    _coordShmem->button == _params.mouse_button) {
	  
	  if (_params.debug) {
	    fprintf(stderr,
		    "Click - lat = %g, lon = %g, mouse button = %d\n",
		    _coordShmem->pointer_lat,
		    _coordShmem->pointer_lon,
		    (int) _coordShmem->button);
	  }
	  
	  time_t data_time;

	  if(_params.use_cidd_time) {
	    data_time = _coordShmem->time_max;
	  } else {
	    data_time = curr_time;
	  }
	 

	  for (int ip=0; ip < _params.sources_n; ip++){
	    Printers[ip]->printData(data_time,
				    _coordShmem->pointer_lat,
				    _coordShmem->pointer_lon,
				    &sum); 
	  }
 
	  last_display_time = curr_time;
	  
	} else {

	  if (_params.debug) {
	    fprintf(stderr, "   Not processing clicks for mouse button %ld\n",
		    _coordShmem->button);
	  }

	}
      
	last_no = _coordShmem->pointer_seq_num;
	
      } else {

	time_t data_time;

	if(_params.use_cidd_time) {
	  //
	  // Use the time at the end of the current frame.
	  //
	  data_time = (time_t) _coordShmem->time_max;
	} else {
	  data_time = curr_time;
	}


	for (int ip=0; ip < _params.sources_n; ip++){
	  Printers[ip]->printData(data_time,
				  _params.startup_location.lat,
				  _params.startup_location.lon,
				  &sum);
	}
	
	last_display_time = curr_time;
	
      }
      
    } // if (_coordShmem->pointer_seq_num != last_no ... 
    
    umsleep(_params.sleep_msecs);
    
  } // while(forever)
  
  return (0);

}

