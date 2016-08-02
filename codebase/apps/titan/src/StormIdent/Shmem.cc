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
/////////////////////////////////////////////////////////////
// Shmem.cc
//
// Shared memory object for communication with storm track program
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "StormIdent.hh"
#include "Shmem.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
using namespace std;

// permissions for the shared memory and semaphores

#define S_PERMISSIONS 0666

//////////////
// constructor
//

Shmem::Shmem(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  _shmemAttached = FALSE;
  _semsAttached = FALSE;

}

/////////////
// destructor
//

Shmem::~Shmem()

{

  STRfree(_progName);

}

///////////////////////////////////////
// create()
//
// Create semaphores and shared memory
//
// Returns 0 on success, -1 on failure.
//

int Shmem::create()

{

  // create the semaphore set
  
  if(_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "%s: Trying to create semaphores\n", _progName);
  }
  
  if ((_semId = usem_create(_params->shmem_key,
			    N_STORM_IDENT_SEMS,
			    S_PERMISSIONS)) < 0) {
    fprintf(stderr, "ERROR - %s:Shmem::create.\n", _progName);
    fprintf(stderr,
	    "Cannot create semaphore set, key = %x\n",
	    (unsigned) _params->shmem_key);
    return(-1);
  }
  
  if(_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "%s: created semaphores\n", _progName);
  }
  
  _semsAttached = TRUE;
  
  // initialize semaphores
  
  if (usem_clear(_semId, STORM_TRACK_ACTIVE_SEM)) {
    fprintf(stderr, "ERROR - %s:Shmem::create.\n", _progName);
    fprintf(stderr, "Clearing STORM_TRACK_ACTIVE_SEM\n");
    return(-1);
  }
  
  if (usem_clear(_semId, STORM_IDENT_QUIT_SEM)) {
    fprintf(stderr, "ERROR - %s:Shmem::create.\n", _progName);
    fprintf(stderr, "Clearing STORM_IDENT_QUIT_SEM\n");
    return(-1);
  }

  // create shared memory

  if(_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "%s: Trying to create shared memory\n", _progName);
  }
  
  if ((_shmem = (storm_tracking_shmem_t *)
       ushm_create(_params->shmem_key + 1,
		   sizeof(storm_tracking_shmem_t),
		   S_PERMISSIONS)) == NULL) {
    fprintf(stderr, "ERROR - %s:Shmem::create.\n", _progName);
    fprintf(stderr,
	    "Cannot create shared memory header, key = %x\n",
	    (unsigned) (_params->shmem_key + 1));
    return(-1);
  }
  
  strcpy(_shmem->storm_data_dir,
	 _params->storm_data_dir);
  
  _shmemAttached = TRUE;

  if(_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "%s: got shmem\n", _progName);
  }

  return (0);

}

////////////////////////////
// init()
//
// initialize the shared mem

// void Shmem::init(char *header_file_path)

// {
//   strcpy(_shmem->storm_header_file_path, header_file_path);
// }

///////////////////////////////////////////////a
// peformTracking()
//
// Signal storm tracking program to do tracking
//
// Returns 0 on success, -1 on failure.
//

int Shmem::performTracking(char *header_file_path,
			   int tracking_mode)
  
{

  // set the header file path for storm tracking

  strcpy(_shmem->storm_header_file_path, header_file_path);

  // set tracking mode

  _shmem->tracking_mode = tracking_mode;
  
  // set the storm_track active semaphore to indicate to
  // storm_track that it may go ahead
  
  if (usem_set(_semId, STORM_TRACK_ACTIVE_SEM) != 0) {
    fprintf(stderr, "ERROR - %s:Shmem::performTracking\n", _progName);
    fprintf(stderr, "Setting STORM_TRACK_ACTIVE_SEM sempahore\n");
    return(-1);
  }
  
  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "STORM_TRACK_ACTIVE_SEM set\n");
  }
  
  // wait for STORM_TRACK_ACTIVE_SEM to clear
  // This is done on a non-blocking loop so that
  // PMU_register may be called

  while (usem_check(_semId, STORM_TRACK_ACTIVE_SEM)) {
    PMU_auto_register("Waiting for storm tracking to complete");
    if (_params->debug >= Params::DEBUG_VERBOSE) {
      fprintf(stderr, "In performTracking - waiting for "
	      "STORM_TRACK_ACTIVE_SEM to clear\n");
    }
    sleep(1);
  }
  
  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Shmem::performTracking - tracking complete\n");
  }
  
  return (0);
  
}

////////////////////////////////////////////////////////
// signalExit()
//
// Signal to storm tracking program that StormIdent is exiting.
// Wait to storm tracking to exit.
//

void Shmem::signalExit(int sig)

{

  if (_semsAttached) {

    // set the flag for removing the track file as appropriate

    if (_shmemAttached) {
      if (_params->remove_old_files_on_restart) {
	_shmem->remove_track_file = TRUE;
      } else {
	_shmem->remove_track_file = FALSE;
      }
      if (sig == EXIT_AND_RESTART) {
	_shmem->auto_restart = TRUE;
      } else {
	_shmem->auto_restart = FALSE;
      }
    }
    
    // set the quit semaphore, then wait for it to be 
    // cleared by storm_track
    
    if (usem_set(_semId, STORM_IDENT_QUIT_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:Shmem::signalExit.\n", _progName);
      fprintf(stderr, "Setting STORM_IDENT_QUIT_SEM sempahore\n");
    }

    // set the storm_track active semaphore to indicate to
    // storm_track that it may go ahead and quit
    
    if (usem_set(_semId, STORM_TRACK_ACTIVE_SEM) != 0) {
      fprintf(stderr, "ERROR - %s:Shmem::signalExit\n", _progName);
      fprintf(stderr, "Setting STORM_TRACK_ACTIVE_SEM sempahore\n");
    }
    
    // wait for storm_track to clear semaphore

    while (usem_check(_semId, STORM_TRACK_ACTIVE_SEM)) {
      PMU_auto_register("Waiting for storm tracking to die.");
      if (_params->debug >= Params::DEBUG_VERBOSE) {
	fprintf(stderr, "Testing STORM_TRACK_ACTIVE_SEM sempahore.\n");
	fprintf(stderr, "In Shmem::signalExit - waiting for "
		"STORM_TRACK_ACTIVE_SEM to die.\n");
      }
      sleep(1);
    }
    
    // remove the semaphore
    
    if ((sig != EXIT_AND_RESTART) &&
	usem_remove(_params->shmem_key) != 0) {
      if (_params->debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:Shmem::signalExit\n", _progName);
	fprintf(stderr, "Cannot remove sempahore set, key = %x.\n",
		(unsigned) _params->shmem_key);
      }
    }
    
  } // if (_semsAttached)
  
  if (sig != EXIT_AND_RESTART && _shmemAttached) {

    // remove shared memory
    
    if (ushm_remove(_params->shmem_key + 1) != 0) {
      if (_params->debug >= Params::DEBUG_NORM) {
	fprintf(stderr, "WARNING - %s:Shmem::signalExit\n", _progName);
	fprintf(stderr,
		"Cannot remove shared memory, key = %x.\n",
		(unsigned) (_params->shmem_key + 1));
      }
    }

  } // if (sig != EXIT_AND_RESTART ...

}

