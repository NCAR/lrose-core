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
/**
 *
 * @file Main.cc
 *
 * Main program
 *  
 * @date 04/12/2018
 *
 */

#include <csignal>
#include <cstdlib>
#include <new>
#include <cstdio>
#include <unistd.h>
#include <ctime>
#include <string>

#include <toolsa/port.h>
#include <toolsa/umisc.h>

#include "GoesRGLM2Spdb.hh"

using namespace std;


// Prototypes for static functions

static void tidy_and_exit(int signum);

static void no_memory();

static void segv_handler(int signum);

// Global variables

GoesRGLM2Spdb *Prog = (GoesRGLM2Spdb *) NULL;


/*********************************************************************
 * main()
 */

int main(int argc, char **argv) {
  // Create program object.

  Prog = GoesRGLM2Spdb::Inst(argc, argv);
  if (!Prog->okay) {
    tidy_and_exit(-1);
  }


  if (!Prog->init()) {
    tidy_and_exit(-1);
  }

  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc) SIG_IGN);
  PORTsignal(SIGSEGV, segv_handler);
  set_new_handler(no_memory);

  // Run the program.

  Prog->run();

  // clean up

  tidy_and_exit(0);
}

////////////////////////////////////////////////////////////////////////////
// tidy_and_exit(int sig)
//
static void tidy_and_exit(int sig) {
  // Delete the program object.

  if (Prog != (GoesRGLM2Spdb *) NULL)
    delete Prog;

  cout << "Exiting signal: " << sig << endl;

  exit(sig);
}

////////////////////////////////////////////////////////////////////////////
// no_memory()
//
void no_memory() {
  cerr << "Caught bad_alloc exception (Failed to allocate memory!)\n";
  tidy_and_exit(1);
}

////////////////////////////////////////////////////////////////////////////
// segv_handler(int signum)
//
// based on blog at
//   http://www.alexonlinux.com/how-to-handle-sigsegv-but-also-generate-core-dump
// NOTE:  that comments on the blog indicate the core file generated on
//        red hat or on multi-threaded programs might contain unhelpful
//        information.
void segv_handler(int signum) {
  time_t rawtime;
  struct tm *timeinfo;
  char timebuffer[80];
  char cwdbuffer[PATH_MAX + 1];

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(timebuffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
  getcwd(cwdbuffer, PATH_MAX + 1);

  fprintf(stderr, "FATAL ERROR (SEGFAULT): Process %d got signal %d @ local time = %s\n", getpid(), signum, timebuffer);
  fprintf(stderr, "FATAL ERROR (SEGFAULT): Look for a core file in %s\n", cwdbuffer);
  signal(signum, SIG_DFL);
  kill(getpid(), signum);
}

