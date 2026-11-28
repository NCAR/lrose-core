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
// $Id: Main.cc,v 1.3 2016/03/04 02:22:12 dixon Exp $
//
// main for MdvOutlier
//
// Yan Chen, RAL, NCAR
//
// Jan. 2008
//
///////////////////////////////////////////////////////////////
//
// MdvOutlier performs gross error checking for MDV files.
// It searches in key fields for abnormalies that exceed some 
// threshold deviation from the mean. If a threshold is exceeded, 
// write out the offending data to a log file.
//
// The standard deviation and mean data are the results of MdvStats.
// It currently only supports statistics data from MdvStats using
// "ALL" data collection type.
//
////////////////////////////////////////////////////////////////

#include <new>
#include <signal.h>
#include <wait.h>
#include <toolsa/port.h>
#include "MdvOutlier.hh"

using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void cleanChildren(int sig);
static void out_of_store();
static MdvOutlier *_prog;
static int _argc;
static char **_argv;

// main

int main(int argc, char **argv)

{

  _argc = argc;
  _argv = argv;

  // create program object

  _prog = new MdvOutlier(argc, argv);
  if (!_prog->isOK) {
    return(-1);
  }

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
  PORTsignal(SIGCHLD, cleanChildren);

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

  delete(_prog);
  exit(sig);

}

/////////////////////
// Handle the exit of child process

static void cleanChildren(int sig) {

  switch (sig) {

    case SIGCHLD:
      // Reap any outstanding children to keep from having a bunch
      // of "defunct" processes.
      while (waitpid((pid_t)-1, (int *)NULL, (int)(WNOHANG | WUNTRACED)) > 0) {
        _prog->childProcessesReduced();
      }
      break;

    default:
      break;
  }
 
  return;
}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
//

static void out_of_store()

{

  fprintf(stderr, "FATAL ERROR - program MdvOutlier\n");
  fprintf(stderr, "  Operator new failed - out of store\n");
  exit(-1);

}

