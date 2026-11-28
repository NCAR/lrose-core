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
//

//
#include <cstdio>
//
// Include stuff from the RAL libraries.
//
#include <toolsa/pmu.h>
#include <didss/DsInputPath.hh>
//
// Local include files - most notably Params.hh, which
// is generated from the paramdef file using tdrp_gen.
//
#include "Params.hh"
#include "readScipuffDump.hh"
//
// Use the standard namespace. This is necessary for more
// recent versions of the compiler.
//
using namespace std;
//
// The 'tidy_and_exit' routine is called by the program if the
// system gets an interrupt signal.
//
static void tidy_and_exit (int sig);
//
// Main program starts.
// 
int main(int argc, char *argv[])
{
  //
  // Trap signals with reference to the 'tidy_and_exit' routine.
  //
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  //
  // Get the Table Driven Runtime Parameters loaded
  // into the 'tdrp' structure.
  //
  Params *tdrp = new  Params(); 
  
  if (tdrp->loadFromArgs(argc, argv, NULL, NULL)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 
  //
  // Instantiate an instance of the object that will
  // actually do most of the work.
  //
  readScipuffDump D( tdrp );
  //
  //
  D.readScipuffDumpFile( );

  //
  // Should only get here if we come to the end of the input
  // file list.
  //
  return 0;
}

/////////////////////////////////////////////
//
// Small routine called in the event of an interrupt signal.
//
static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}








