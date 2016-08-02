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
#include <toolsa/pmu.h>
#include <toolsa/TaArray.hh>
#include <cstdlib>

#include <didss/DsInputPath.hh>

#include "Params.hh"
#include "Args.hh"
#include "NidsVad2Spdb.hh"
using namespace std;

static void tidy_and_exit (int sig);
 
int main(int argc, char *argv[])
{

  // Trap.
  PORTsignal(SIGINT, tidy_and_exit);
  PORTsignal(SIGHUP, tidy_and_exit);
  PORTsignal(SIGTERM, tidy_and_exit);
  
  // Get the CLI arguments.

  Args *_args = new Args(argc, argv, "NidsVad2Spdb");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: NidsVad2Spdb\n");
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

  PMU_auto_init("NidsVad2Spdb", P->instance, PROCMAP_REGISTER_INTERVAL);
  
  int numLocationsToWatch;
  if (P->mode == Params::ARCHIVE) {
    numLocationsToWatch = 1;
  } else { // REALTIME mode.
    numLocationsToWatch = P->InDirs_n;
  }

  // Set up DsInputPath.

  TaArray<DsInputPath *> input_;
  DsInputPath **_input = input_.alloc(numLocationsToWatch);

  if (P->mode == Params::ARCHIVE) {

    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    fprintf(stdout,"%d files :\n",_args->nFiles);
    for (int i=0;i<_args->nFiles;i++){
      fprintf(stdout,"[ %d ] %s\n",i+1,_args->filePaths[i]);
    }
    //
    // Only one DsInputPath to watch in archive mode.
    //
    _input[0] = new DsInputPath("NidsVad2Spdb",
			    P->debug,
			    _args->nFiles,
			    _args->filePaths);
    
    char *FilePath;
    
    _input[0]->reset();

    while ((FilePath = _input[0]->next()) != NULL) {

      NidsVad2Spdb(FilePath,P);
      PMU_auto_register(FilePath);

    } // End of loop over command line specified files.

    tidy_and_exit(0); // End of archive mode.


  }

  if (P->mode == Params::REALTIME) {
    //
    //
    //
    TaArray<char *> latestFile_;
    char **latestFile = latestFile_.alloc(numLocationsToWatch);
    for (int i=0; i < numLocationsToWatch; i++){

      latestFile[i] = (char *)calloc(MAX_PATH_LEN, sizeof(char));

      _input[i] = new DsInputPath("NidsVad2Spdb",
				  P->debug,
				  P->_InDirs[i],
				  P->max_realtime_valid_age,
				  PMU_auto_register,
				  true, false);
      //
      // Set the file extension to search for, if specified.
      //
      if (strlen(P->fileExtension) > 0){
	_input[i]->setSearchExt(P->fileExtension);
      }
    }

    // And use the input paths.
    PMU_auto_register("Waiting for data");
    
    char *FilePath;
  
    for (int i=0; i < numLocationsToWatch; i++){
      _input[i]->reset();
    }

    //
    // Now run in REALTIME mode - infinite loop.
    //
  
    do{
      
      for (int i=0; i < numLocationsToWatch; i++){
	//
       	if (P->debug) {
	  fprintf(stderr,"Checking directory %d : %s\n",
		  i, P->_InDirs[i]);
	}
	//
	// Get the latest file for this dir. Sleep for
	// a second to avoid thrashing wildly.
	//
	sleep(1);
	FilePath = _input[i]->latest();
	//
	// The above does not work - FilePath is always NULL.
	// So - must fiddle about to get filename and see
	// if we have a new one.
	//
       
	LdataInfo L = _input[i]->getLdataInfo();
	string s = L.getRelDataPath();

	if (P->debug){
	  fprintf(stderr,"File from that dir is : %s\n", s.c_str());
	}
	//
	PMU_auto_register("Looking for input data..");
	//
	// See if this is a new filename.
	//
	if (strcmp(s.c_str(), latestFile[i])){
	  //
	  // It is, so process it.
	  //
	  sprintf(latestFile[i],"%s",s.c_str());
	  char fn[MAX_PATH_LEN];
	  sprintf(fn,"%s/%s",P->_InDirs[i], s.c_str());

	  if (P->debug){
	    fprintf(stderr,"New file to process : %s\n", fn);
	  }

	  //
	  //
	  NidsVad2Spdb(fn,P);
	  PMU_auto_register(fn);
	  
	} 
      } // End of loop over DsInputPath objects. 
    } while(1); // infinite while loop.
  } // End of If we are in realtime mode.

}

/////////////////////////////////////////////

static void tidy_and_exit (int sig){
  PMU_auto_unregister();
  exit(sig);
}








