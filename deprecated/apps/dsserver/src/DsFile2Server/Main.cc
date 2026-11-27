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
// main for DsFile2Server
//
// Niles Oien, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2004
//
///////////////////////////////////////////////////////////////
//
// Reads DsMsg files from disk and send the messages to servers
//
////////////////////////////////////////////////////////////////

#include "DsFile2Server.hh"
#include <toolsa/str.h>
#include <toolsa/port.h>
#include <toolsa/pmu.h>
#include <signal.h>
#include <new>
#include <iostream>
using namespace std;

// Global.
DsFile2Server *Prog;

//
// file scope

static void tidy_and_exit (int sig);
static void out_of_store();
static int sigCount = 0;

// main

int main(int argc, char **argv)

{

  // Respond to the -h option

  for (int i=0; i < argc; i++){
    if (!(strcmp(argv[i], "-h"))){
      cerr << "Use -print_params for help." << endl;
      exit(0);
    }
  }


  // create program object
  Prog = new DsFile2Server(argc, argv);
  if (!Prog->isOK) {
    return(-1);
  }

  // set signal handling
  
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
  PORTsignal(SIGHUP, tidy_and_exit);

  // set new() memory failure handler function

  set_new_handler(out_of_store);

  // run it

  int iret = Prog->Run(argc, argv);

  // clean up

  tidy_and_exit(iret);
  return (iret);
  
}

// tidy up on exit

static void tidy_and_exit (int sig)

{

  PMU_auto_unregister();
  sigCount++;
  if (sigCount == 1) {
    delete(Prog);
    exit(sig);
  }
}

void heartbeat( const char *label)
{
	 Prog->Heartbeat(label);
}

////////////////////////////////////
// out_of_store()
//
// Handle out-of-memory conditions
static void out_of_store()

{

  cerr << "FATAL ERROR - program DsFile2Server" << endl;
  cerr << "  Operator new failed - out of store" << endl;
  exit(-1);

}
