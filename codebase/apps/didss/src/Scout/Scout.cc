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
//
// main for the Scout
//
// Niles Oien, from
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
///////////////////////////////////////////////////////////////

#include <unistd.h> // for call to sleep between runs.
#include <signal.h>
#include <new>
#include <cstdlib>
#include <string>

#include <toolsa/pmu.h> // For registering ourselves.
#include <toolsa/procmap.h>
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/file_io.h>
#include <toolsa/Path.hh>
#include <didss/RapDataDir.hh>

#include "ScoutOut.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;

static void out_of_store();
static void tidy_and_exit (int sig);

int main(int argc, char **argv)
{

  const string progName = "Scout";

  // Get CLI arguments.
  Args args;
  if (args.parse(progName, argc, argv)) {
    cerr << "ERROR - " << progName << endl;
    cerr << "  Check command line args." << endl;
    return -1;
  }
  
  // Get TDRP arguments.

  char *paramsPath = NULL;
  Params P;
  if (P.loadFromArgs(argc, argv, args.override.list, &paramsPath)) {
    cerr << "ERROR - " << progName << endl;
    cerr << "  Problem with TDRP parameters." << endl;
    return -1;
  }

  // set top dir and lock file name

  string TopDir = RapDataDir.location();
  if (P.OverrideTopDir) {
    TopDir = P.TopDir;
  }
  string LockFileName = TopDir;
  LockFileName += PATH_DELIM;
  LockFileName += "_ScoutLock";

  // Set up out_of_store routine.
  set_new_handler(out_of_store);

  // Trap signals.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);

  // Lock?
  FILE *lfp = NULL;
  if( P.Lock )
  {
    lfp = ta_create_lock_file(LockFileName.c_str());
    if (lfp == NULL)
    {
      cerr << "Lock file exists: " << LockFileName << endl;
      return -1;
    }
  }

  // PMU init
  PMU_auto_init(progName.c_str(), P.Instance, PROCMAP_REGISTER_INTERVAL);

  // run it, repeatedly if needs be.

  do {

    PMU_auto_register("Processing");

    ScoutOut Q(P, TopDir, 0);
    Q.Recurse();

    if (P.Debug) {
      cerr << "Done with pass, zzzzz ....." << endl;
      cerr << "Sleeping for " << P.BetweenPassDelay << " secs" << endl;
    }

    if (!(P.OnceOnly)){
      for(int k=0; k<P.BetweenPassDelay; k++){
	PMU_auto_register("Sleeping between runs");
	sleep(1); // Wait between runs.
      }
    }
    
  } while(!(P.OnceOnly));

  // Unlock?
  if( P.Lock )
  {
    ta_remove_lock_file(LockFileName.c_str(), lfp);
  }
  
  cout << progName << " exiting" << endl;

  tidy_and_exit(0);
  
}

//////////////////////////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{
  PMU_auto_unregister();
  exit(sig);
}

//////////////////////////////////////
// Deal with failed allocation.

static void out_of_store()
{

  fprintf(stderr,"Scout : problem with memory allocation.\n");
  exit(-1);

}











