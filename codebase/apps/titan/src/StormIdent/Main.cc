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
// main for StormIdent
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 1998
//
///////////////////////////////////////////////////////////////
//
// StormIdent identifies storms in MDV files
//
////////////////////////////////////////////////////////////////

#include "StormIdent.hh"
#include "Shmem.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/DateTime.hh>
#include <signal.h>
#include <new>
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static StormIdent *_prog;
static int _argc;
static char **_argv;

// main

int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;

  // create program object

  _prog = new StormIdent(argc, argv);
  if (!_prog->OK) {
    return(-1);
  }

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = _prog->Run();

  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

///////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{

  // check malloc

  umalloc_verify();
  umalloc_map();

  // signal storm tracking

  if (_prog->shMem) {
    _prog->shMem->signalExit(sig);
  }

  // If auto_restart, register so that procmap will have the latest time.
  // If not, unregister process.

  if (sig == EXIT_AND_RESTART) {
    PMU_force_register("In tidy_and_exit for auto_restart");
  } else {
    PMU_auto_unregister();
  }
  
  // if a restart is required, load up string with the original command
  // line and call system to restart this program

  if (sig == EXIT_AND_RESTART) {

    char call_str[BUFSIZ];
    memset ((void *) call_str,
            (int) 0, (size_t) BUFSIZ);

    for (int iarg = 0; iarg < _argc; iarg++) {

      if (!strcmp(_argv[iarg], "-start")) {
	DateTime stime(_prog->getNewStartTime());
	char startStr[128];
	sprintf(startStr, "-start \"%d %d %d %d %d %d\" ",
		stime.getYear(), stime.getMonth(), stime.getDay(), 
		stime.getHour(), stime.getMin(), stime.getSec());
	ustr_concat(call_str, startStr, BUFSIZ);
	iarg++;
      } else if (!strcmp(_argv[iarg], "-end")) {
	char endStr[128];
	sprintf(endStr, "-end \"%s\" ",	_argv[iarg+1]);
	ustr_concat(call_str, endStr, BUFSIZ);
	iarg++;
      } else {
	ustr_concat(call_str, _argv[iarg], BUFSIZ);
	ustr_concat(call_str, " ", BUFSIZ);
      }

    }

    strcat(call_str, " & ");

    if (_prog->getDebug()) {
      cerr << "Restarting ...." << endl;
      cerr << call_str << endl;
    }

    system(call_str);

  } /* if (sig == EXIT_AND_RESTART) */

  delete(_prog);

  /*
   * exit with code sig
   */

  exit(sig);

}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  fprintf(stderr, "FATAL ERROR - program StormIdent\n");
  fprintf(stderr, "  Operator new failed - out of store\n");
  exit(-1);

}



