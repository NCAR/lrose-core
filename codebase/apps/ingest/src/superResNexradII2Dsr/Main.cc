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
#include "Args.hh"
#include "superResNexradII2Dsr.hh"
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
  // Get the Command Line Interperter arguments using the
  // Args class.
  //
  Args *_args = new Args(argc, argv, (char *)"superResNexradII2Dsr");
  if (!_args->OK) {
    fprintf(stderr, "ERROR: superResNexradII2Dsr\n");
    fprintf(stderr, "Problem with command line args\n");
    exit(-1);
  }                 

  //
  // Get the Table Driven Runtime Parameters loaded
  // into the 'tdrp' structure.
  //
  Params *tdrp = new  Params(); 

  if (tdrp->loadFromArgs(argc, argv, _args->override.list, &_args->paramsFilePath)){
    fprintf(stderr,"Specify params file with -params option.\n");
    exit(-1);
  } 

  //
  // If -verbose or -debug specified, then set debug flag.
  //
  for (int k=0; k < argc; k++){
    if (0==strcmp(argv[k], "-debug")) {
      tdrp->debug = Params::DEBUG_NORM;
    }
    if (0==strcmp(argv[k], "-verbose")){
      tdrp->debug = Params::DEBUG_DATA;
    }
  }

  //
  // Check in to the process mapping system, using the instance string
  // we get from the tdrp parameters.
  //
  PMU_auto_init("superResNexradII2Dsr", tdrp->instance, PROCMAP_REGISTER_INTERVAL);
  
  //
  // Set up a DsInputPath object. At RAL, this is a reasonably standard
  // method of monitoring a directory to see if new file arrives.
  //
  DsInputPath *_input=NULL;

  //
  // The setup of this object depends on if we are running in archive
  // mode or realtime mode.
  //
  // In archive mode, the file names are specified on the command line
  // with the -f argument. This information is in the _args class.
  //
  if (tdrp->mode == Params::READ_ARCHIVE_FILES) {

    if (_args->nFiles <= 0){
      fprintf(stderr,"For ARCHIVE mode, specify file list a argument.\n");
      exit(-1);
    }

    if (tdrp->debug){
      fprintf(stderr,"%d files :\n",_args->nFiles);
      for (int i=0;i<_args->nFiles;i++){
	fprintf(stderr,"[ %d ] %s\n",i+1,_args->filePaths[i]);
      }
    }
    _input = new DsInputPath("superResNexradII2Dsr",
			     tdrp->debug >= Params::DEBUG_DATA,
			     _args->nFiles,
			     _args->filePaths);
                                                            
  }

  if (tdrp->mode == Params::READ_REALTIME_FILES) {
    //
    // In realtime mode, we monitor a directory for new files.
    //
    _input = new DsInputPath("superResNexradII2Dsr",
                             tdrp->debug >= Params::DEBUG_DATA,
                             tdrp->realtimeInput.realtimeDir,
                             tdrp->realtimeInput.realtimeMaxAgeSec,
                             PMU_auto_register,
			     true, true);

    _input->setSubString( tdrp->realtimeInput.realtimeFilenameSubString );

  }

  //
  // Reading from a socket
  //
  if (tdrp->mode == Params::READ_SOCKET){
    if (tdrp->timeAction == Params::TIME_FILENAME){
      cerr << "WARNING - cannot take time from filename in socket reading mode - using beam times." << endl;
      tdrp->timeAction = Params::TIME_BEAM;
    }

    superResNexradII2Dsr nxrd2( tdrp );
    nxrd2.procVols( NULL );
  }
  //
  //
  PMU_auto_register("Waiting for data");

  //
  // If we got here we're reading files.
  //

  char *FilePath;
  _input->reset();

  //
  // Instantiate an instance of the object that will
  // actually do most of the work.
  //
  superResNexradII2Dsr nxrd2( tdrp );
  //
  // Loop through, processing the input files.
  //
  while ((FilePath = _input->next()) != NULL) {

    nxrd2.procVols( FilePath );
    PMU_auto_register("Waiting for data");

    if (tdrp->realtimeInput.realtimeGzipInput){
      char com[1024];
      sprintf(com, "gzip -f %s", FilePath);
      system(com);
    }

  } // while loop.

  //
  // Should only get here if we come to the end of the input
  // file list in archive mode.
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








