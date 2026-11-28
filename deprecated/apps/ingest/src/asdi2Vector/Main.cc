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
// This is the main module for the asdi2Vector application. The input
// data are read from a stream - ie. by connection to a socket - or
// from archived data files.
//
#include <stdio.h>
#include <toolsa/pmu.h>

#include <didss/DsInputPath.hh>

#include "Params.hh"
#include "Args.hh"
#include "asdi2Vector.hh"

static void tidy_and_exit (int sig);
 
int main(int argc, char *argv[])
{

  //
  // Intercept interrupt signals. Call the tidy_and_exit function
  // if one is recieved.
  //
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  //
  // Get the command line arguments.
  //
  Args *_args = new Args(argc, argv, "asdi2Vector");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: asdi2Vector\n");
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }                 

  //
  // Get the TDRP parameters.
  //
  Params *TDRP = new  Params(); 

  if (TDRP->loadFromArgs(argc, argv, _args->override.list, &_args->paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  //
  // Check in to procmap.
  //
  if (TDRP->debug){
    fprintf(stderr,"Checking in with instance :%s:\n",TDRP->instance);
  }
  PMU_auto_init("asdi2Vector", TDRP->instance, PROCMAP_REGISTER_INTERVAL);
  
  //
  // If using streams, just connect to that stream.
  //
  if (TDRP->mode == Params::REALTIME_STREAM) { 
      asdi2Vector M(TDRP);
      M.ProcFile(NULL, TDRP); // Should go on forever.
      exit(-1);               // Just in case it doesn't.
  }
  //
  // Otherwise, we are reading a file, in realtime or archive mode.
  // Set up DsInputPath.
  //
  DsInputPath *_input=NULL;

  if (TDRP->mode == Params::ARCHIVE) {
    //
    // Archive mode - take the list of files from
    // the command line.
    //
    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    fprintf(stdout,"%d files :\n",_args->nFiles);
    for (int i=0;i<_args->nFiles;i++){
      fprintf(stdout,"[ %d ] %s\n",i+1,_args->filePaths[i]);
    }

    _input = new DsInputPath("asdi2Vector",
			     TDRP->debug,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  } 


  if (TDRP->mode == Params::REALTIME_FILE) { // REALTIME mode with files.
    //
    // Realtime mode - watch a directory for new incoming files.
    //
   _input = new DsInputPath("asdi2Vector",
                             TDRP->debug,
                             TDRP->InDir,
                             TDRP->max_realtime_valid_age,
                             PMU_auto_register,
			     false, true);

  }


  //
  // Use the _input object to process the files.
  //
  PMU_auto_register("Waiting for data");

  asdi2Vector M(TDRP);

  char *FilePath;
  _input->reset();

  while ((FilePath = _input->next()) != NULL) {

    M.ProcFile(FilePath,TDRP);
    PMU_auto_register("Waiting for data");

  } // while loop.


}

/////////////////////////////////////////////
//
// Small routine to exit if an interrupt is recieved.
//
static void tidy_and_exit (int sig){
  // PMU_auto_unregister(); // This seems to cause problems on cntrl-C
  exit(sig);
}








