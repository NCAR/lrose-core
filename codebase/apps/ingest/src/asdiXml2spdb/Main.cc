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

#include <cstdio>
#include <unistd.h>

#include <didss/DsInputPath.hh>
#include <toolsa/pmu.h>

#include "Params.hh"
#include "Args.hh"
#include "asdiXml2spdb.hh"

static void tidy_and_exit (int sig);

asdiXml2spdb *M = NULL;

int main(int argc, char *argv[])
{
  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  // Get the CLI arguments.
  char* prog_name = new char[100];
  strcpy(prog_name, "asdiXml2spdb");

  Args *_args = new Args(argc, argv, prog_name);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: asdiXml2spdb\n");
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }                 

  // Get the TDRP parameters.

  Params *P = new  Params(); 

  if (P->loadFromArgs(argc, argv, _args->override.list, &_args->paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  // Check in.

  if (P->debug){
    fprintf(stderr,"Checking in with instance :%s:\n",P->instance);
  }

  PMU_auto_init("asdiXml2spdb", P->instance, PROCMAP_REGISTER_INTERVAL);
  
  //
  // If using streams, just connect to that stream.
  //
  if (P->mode == Params::REALTIME_STREAM) { 
      asdiXml2spdb M(P);
      while(TRUE) {
	M.Stream(); // Should go on forever.
	cerr << "Socket read problem ...  restarting stream." << endl;
	usleep( 100 * 1000 );
      }
  }
  //
  // Otherwise, we are reading a file, in realtime or archive mode.
  // Set up DsInputPath.
  //
  DsInputPath *_input=NULL;

  if (P->mode == Params::ARCHIVE) {

    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    fprintf(stdout,"%d files :\n",_args->nFiles);
    for (int i=0;i<_args->nFiles;i++){
      fprintf(stdout,"[ %d ] %s\n",i+1,_args->filePaths[i]);
    }

    _input = new DsInputPath("asdiXml2spdb",
			     P->debug,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  } else
    if (P->mode == Params::REALTIME_FILE) { // REALTIME mode with files.

      _input = new DsInputPath("asdiXml2spdb",
			       P->debug,
			       P->InDir,
			       P->max_realtime_valid_age,
			       PMU_auto_register,
			       false, false);
      
    } else {
      fprintf(stderr,"Mode not properly defined.\n");
      exit(-1);
    }

  // And use it.
  PMU_auto_register("Waiting for data");

  M = new asdiXml2spdb(P);

  char *FilePath;
  _input->reset();

  while ((FilePath = _input->next()) != NULL) {

    M->File(FilePath);
    PMU_auto_register("Waiting for data");

    if (P->mode == Params::REALTIME_FILE) {
      std::remove(FilePath);
    }

  } // while loop.

  M->flush();

  delete M;
  delete _args;
  delete P;
  delete _input;


}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  // PMU_auto_unregister(); // This seems to cause problems on cntrl-C
  if(M != NULL)  
    M->flush();
  exit(sig);
}








