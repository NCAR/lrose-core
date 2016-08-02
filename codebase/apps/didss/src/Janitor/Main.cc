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
// main for Janitor
//
// Niles Oien, from
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 1998
//
///////////////////////////////////////////////////////////////
//
// Uses the C++ mode of TDRP
//
////////////////////////////////////////////////////////////////

/**
 * @file Main.cc
 *
 * Main routine for Janitor.
 *
 * @author Niles Oien
 * @see something
 */

#include <new>
#include <signal.h>
#include <unistd.h> // for call to sleep between runs.

#include <toolsa/file_io.h> // For locking.
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>

#include "Janitor.hh"
using namespace std;

// file scope

static void tidy_and_exit (int sig);
static void out_of_store();

static Janitor *Prog = NULL;

// main

int main(int argc, char **argv)

{

  // create program object

  Janitor *Prog;

  // Set up out_of_store routine.
  set_new_handler(out_of_store);

  // Trap signals.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGKILL, tidy_and_exit);

  Prog = new Janitor(argc, argv);
  
  if (!Prog->OK) {
    return(-1);
  }

  int iret= Prog->Run();

  // exit

  tidy_and_exit(iret);
  return (iret);
  
}
//////////////////////////////////////
// tidy up on exit

static void tidy_and_exit (int sig)

{
  PMU_auto_unregister();
  if (Prog) {
    delete(Prog);
  }
  exit(sig);
}

//////////////////////////////////////
// Deal with failed allocation.

static void out_of_store()
{

  fprintf(stderr,"Janitor : problem with memory allocation.\n");
  exit(-1);

}
