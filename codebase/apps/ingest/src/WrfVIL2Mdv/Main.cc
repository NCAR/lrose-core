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
/*********************************************************************
 * Main.cc: WrfVIL2Mdv main routine
 *
 * RAP, NCAR, Boulder CO
 *
 * Feb 2008
 *
 * Dan Megenhardt
 *
 *********************************************************************/

#include <cstdio>
#include <toolsa/pmu.h>

#include <didss/DsInputPath.hh>

#include "Params.hh"
#include "Args.hh"
#include "WrfVIL2Mdv.hh"

using namespace std;

static void tidy_and_exit (int sig);
 
int main(int argc, char *argv[])
{

  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  

  // Get the CLI arguments.

  Args *_args = new Args(argc, argv, (char *) "WrfVIL2Mdv");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: WrfVIL2Mdv\n");
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

  PMU_auto_init("WrfVIL2Mdv", P->instance, PROCMAP_REGISTER_INTERVAL);
  
  // Set up DsInputPath.

  DsInputPath *_input;

  if (P->mode == Params::ARCHIVE) {

    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    fprintf(stdout,"%d files :\n",_args->nFiles);
    for (int i=0;i<_args->nFiles;i++){
      fprintf(stdout,"[ %d ] %s\n",i+1,_args->filePaths[i]);
    }

    _input = new DsInputPath("WrfVIL2Mdv",
			     P->debug,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  } else { // REALTIME mode.

   _input = new DsInputPath("WrfVIL2Mdv",
                             P->debug,
                             P->InDir,
                             P->max_realtime_valid_age,
                             PMU_auto_register,
			     P->use_ldata_info,
			     false);
   //
   // Set the file extension to search for, if specified.
   //
   if (strlen(P->fileExtension) > 0){
     _input->setSearchExt(P->fileExtension);
   }

  }


  // And use it.
  PMU_auto_register("Waiting for data");

  char *FilePath;
  _input->reset();

  WrfVIL2Mdv S( P );

  while ((FilePath = _input->next()) != NULL) {

    S.WrfVIL2MdvFile(FilePath);
    PMU_auto_register("Waiting for data");

  } // while loop.

  delete _input;
  delete _args;
  delete P;
  
}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}








