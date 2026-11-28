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
 *  @file Main.cc
 * 
 *  entry point for execution of MdvBlender
 *
 *  @author P. Prestopnik, G. McCabe
 * 
 *  @date July, 2014
 *
 *  @version $Id: Main.cc,v 1.6 2016/07/12 20:11:53 minnawin Exp $
 */


// C++ include files
#include <csignal>
#include <cstdlib>
#include <new>

// for segv handling
#include <unistd.h>
#include <cstdio>
#include <ctime>
#include <string>

// System/RAP include files
#include "toolsa/port.h"          

// Local include files
#include "MdvBlender.hh"

using namespace std;

//static makes these accesible only in this file
static std::string cmdline = "";
static MdvBlender *mainObj = 0;

void store_arguments(int argc, char **argv){
	
	for (int ix = 0; ix < argc; ix++){
		cmdline += argv[ix];
		cmdline += " ";
	}
}



// based on blog at http://www.alexonlinux.com/how-to-handle-sigsegv-but-also-generate-core-dump
// NOTE:  that comments on the blog indicate the core file generated on red hat or on multi-threaded programs
//        might contain unhelpful information.
void segv_handler(int signum)
{
	time_t rawtime;
  struct tm * timeinfo;
  char timebuffer[80];
  char cwdbuffer[PATH_MAX+1];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(timebuffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
  getcwd(cwdbuffer,PATH_MAX+1);

  fprintf(stderr, "FATAL ERROR (SEGFAULT): Process %d got signal %d @ local time = %s\n", getpid(), signum, timebuffer);
  fprintf(stderr, "FATAL ERROR (SEGFAULT): Look for a core file in %s\n",cwdbuffer);
  fprintf(stderr, "FATAL ERROR (SEGFAULT): Process command line: %s\n",cmdline.c_str());
  signal(signum, SIG_DFL);
  kill(getpid(), signum);
}

void tidy_and_exit(int signal)
{
   // Remove the top-level application class
   delete mainObj;
   printf("Exiting %d\n", signal);
   exit(signal);
}


void no_memory() {
  cerr << "Caught bad_alloc exception (Failed to allocate memory!)\n";
  tidy_and_exit(1);
}
/////////////////////////////////////////////////////////////////////////
//
// Function Name: 	main
// 
// Description:		main routine for CipAlgo
// 
// Returns:		-1 for failure and iret id program runs
// 

int main(int argc, char **argv)
{
	store_arguments(argc,argv);
  // create main object
  mainObj = MdvBlender::instance(argc, argv);
  if(!mainObj->isOK()) {
    tidy_and_exit(-1);
  } 
  // set signal handling
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  PORTsignal(SIGPIPE, (PORTsigfunc)SIG_IGN);
  PORTsignal(SIGSEGV, segv_handler);
  set_new_handler(no_memory);

  // segv test  
  //int *p = 0;
  //*p = 5;


  // run it
  int iret = mainObj->run();

  // clean up and exit
  tidy_and_exit(iret);
}




































