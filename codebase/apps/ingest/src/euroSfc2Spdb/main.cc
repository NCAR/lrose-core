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
// This is the top level module for the euroSfc2Spdb application.
// This application reads input ASCII files of surface station
// reports in the SAMS format, and saves them out in the SPDB format.
//
// This module uses the DsInputPath class to watch a directory
// for new input files in Realtime mode, or takes a list of
// input files from the command line in archive mode.
//
// In either case the input file name is passed on to the euroSfc2Spdb
// class for conversion to SPDB.
//
#include <cstdio>
#include <toolsa/pmu.h>

#include <didss/DsInputPath.hh>

#include "Params.hh"
#include "Args.hh"
#include "euroSfc2Spdb.hh"

using namespace std;

static void tidy_and_exit (int sig);
 
int main(int argc, char *argv[])
{

  //
  // Intercept interrupt signals so that the tidy_and_exit()
  // routine is called.
  //
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  //
  // Get the arguments from the command line.
  //
  Args *_args = new Args(argc, argv, "euroSfc2Spdb");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: euroSfc2Spdb\n");
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }                 

  //
  // Get the TDRP parameters.
  //
  Params *TDRP = new  Params(); 
  //
  if (TDRP->loadFromArgs(argc, argv, _args->override.list, &_args->paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  //
  // Check in to the process mapping system.
  //
  PMU_auto_init("euroSfc2Spdb", TDRP->instance, PROCMAP_REGISTER_INTERVAL);
  
  //
  // Set up an instance of the DsInputPath class. This is
  // really at the heart of this module.
  //
  DsInputPath *_input;
  //
  // Setup of this class depends on what mode we are in. In archive
  // mode, take the list of files to process from the command line
  // (from the Args class). In realtime mode, watch an input directory
  // for new files to appear.
  //
  if (TDRP->mode == Params::ARCHIVE) {
    //
    // Archive mode - set up to process a list of files specified on
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

    _input = new DsInputPath("euroSfc2Spdb",
			     TDRP->debug,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  } else {
    //
    // realtime mode - set up to watch an input directory for new files
    // to arrive.
    //
   _input = new DsInputPath("euroSfc2Spdb",
                             TDRP->debug,
                             TDRP->inputPath,
                             TDRP->max_realtime_valid_age,
                             PMU_auto_register,
			     false, true);
     
  }

  //
  // Use the DsInputPath object.
  //
  PMU_auto_register("Waiting for data");

  //
  // Make a new euroSfc2Spdb object.
  //
  euroSfc2Spdb *S = new euroSfc2Spdb();  

  char *FilePath;
  _input->reset();

  //
  // Go into a loop, processing new files as they become available.
  //
  //
  while ((FilePath = _input->next()) != NULL) {

    S->ReadSamFile(FilePath, TDRP);
    PMU_auto_register("Waiting for data");

    sleep(2); // Just to stop hogging.

  } // while loop.

  delete S;
  tidy_and_exit(0);

  return 0;

}

/////////////////////////////////////////////
//
// Small routine to exit the program if an interrupt signal happens.
//
static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}








