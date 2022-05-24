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

#include <cstdio>
#include <toolsa/pmu.h>
#include <toolsa/port.h>
#include <signal.h>

#include "Args.hh"
#include "Params.hh"
#include "DsServerMgr.hh"

using namespace std;

// file scope

static void tidy_and_exit (int sig);
static int exit_count = 0;
static DsServerMgr *_mgr = NULL;

int main(int argc, char **argv)
{
    // Load the arguments.
    Args * args = new Args(argc, argv);

    if (!args->isOkay()) {
        args->usage(stderr);
        return 1;
    }

    if (args->isShowUsage()) {
        args->usage(stderr);
        return 1;
    }

    // Load the params.
    Params * params = new Params();

    bool failure = params->loadFromArgs(argc, argv,
                                         args->getOverride()->list,
                                         NULL);
    if (failure) {
        cerr << "Error: Could not read params file: "
             << args->getParamsFilePath() << endl;
        return 1;
    }

    _mgr = new DsServerMgr(args->getAppName(),
			   params->instance,
			   params->port,
			   params->maxQuiescentSecs,
			   params->maxClients,
			   params->debug >= Params::DEBUG_NORM,
			   params->debug >= Params::DEBUG_VERBOSE,
			   params->runSecure,
                           params->mdvReadOnly,
                           params->spdbReadOnly);
    
    // Check if the server was constructed properly.
    if (!_mgr->isOkay()) {
      cerr << "Error: Could not construct DsServerMgr object: "
	   << _mgr->getErrString() << endl;
      return 1;
    }
    
    if (params->noThreads) {
      _mgr->setNoThreadDebug(true);
    }
    
    // set signal handling
    
    PORTsignal(SIGINT, tidy_and_exit);
    PORTsignal(SIGHUP, tidy_and_exit);
    PORTsignal(SIGTERM, tidy_and_exit);
    PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

    // enter loop to wait for clients

    _mgr->waitForClients(1000);  // Wait one second before timeout.
    
    cerr << "Main is done. Got return from waitForClients()." << endl;

    tidy_and_exit(0);
    
}

///////////////////
// tidy up on exit

void tidy_and_exit (int sig)

{

  PMU_auto_unregister();
  exit_count++;
  if (exit_count == 1) {
    delete(_mgr);
    exit(sig);
  }

}

