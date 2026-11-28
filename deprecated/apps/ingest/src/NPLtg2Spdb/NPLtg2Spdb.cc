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
// Noaa Port Lightning to Spdb Translator
// This version watches an output dir for new files
// And processes them directly into Spdb
//
// Original version  by Niles Oien.
// Modified to read encrypted files directly.

#include <toolsa/pmu.h>
#include <iostream>
#include <strings.h>
#include <didss/DsInputPath.hh>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Params.hh"
#include "NPLtg2Spdb.hh"

//
// Constructor
//
NPLtg2Spdb::NPLtg2Spdb(){
  nFiles = 0;
  return;
}
//////////////////////////////////////////////  
//
// Destructor
//
NPLtg2Spdb::~NPLtg2Spdb(){
  return;
}

//////////////////////////////////////////////////
// Main method - run.
//
int NPLtg2Spdb::init( int argc, char **argv ){

  //
  // Parse command line args.
  //
  if (ParseArgs(argc,argv)) return -1;
  //
  // Get TDRP args and check in to PMU.
  //
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("NPLtg2Spdb", P.Instance, PROCMAP_REGISTER_INTERVAL);     

  if (nFiles > 0) P.Mode = Params::ARCHIVE;

  return 0;

}

//////////////////////////////////////////////////////
//
//
//
int NPLtg2Spdb::run(){
  //
  // Set up for input.
  // Depends on what mode we're in.
  //
  if (P.Mode == Params::ARCHIVE){

    for (int k=0; k < nFiles; k++){
      cerr << "Processing " << filePaths[k] << endl;
      ltg_decrypt(filePaths[k], &P);
    }


  } else { // in realtime mode.

   if (P.Debug) cerr << "Watching " << P.InputDir << " for files newer than " << P.MaxRealtimeFileAge <<
				 "secs" << endl;

    DsInputPath input("NPLtg2Spdb",
		      P.Debug,
		      P.InputDir,
		      P.MaxRealtimeFileAge,
		      PMU_auto_register,
		      P.ldataInfoAvailable,
		      P.latestFileOnly);

    PMU_auto_register("Waiting for data");
 
    char *FilePath;
    input.reset();
 
    //
    // In fact, we never leave this while loop.
    //
    while ((FilePath = input.next()) != NULL) {

      //
      // Test if this is a file that contains old data,
      // but for some reason the data are repeatedly
      // transmitted, causing the file to grow continually.
      //
      char *p = FilePath + strlen(FilePath) - strlen("20060802_1901.ltg");
      date_time_t T; T.sec = 0;
      if (5==sscanf(p, "%4d%2d%2d_%2d%2d", &T.year, &T.month, &T.day,
		    &T.hour, &T.min)){

	uconvert_to_utime( &T );

	struct stat buf;
	if (0==stat(FilePath, &buf)){

	  long timeDiff = buf.st_mtime - T.unix_time;
	  if (timeDiff < 0) timeDiff = -timeDiff; // Unlikely.
	  if (timeDiff > P.MaxRealtimeFileTimeDiff){
	    if (P.Debug){
	      cerr << "Skipping \"old data\" file " << FilePath << endl;
	    }
	    continue;
	  }
	} else {
	  cerr << "Failed to stat file " << FilePath << endl;
	}
      } else {
	cerr << "WARNING : Unable to read time from file " << FilePath << endl;
	cerr << "Input file time check not performed. Filename may" << endl;
	cerr << "not match 20060802_1902.ltg convention?" << endl;
	cerr << "filename sans path is " << p;
      }
 
      for (int k=0; k <  P.ProcessingDelay; k++){
	sleep(1);
	PMU_auto_register("Sleeping before processing");
      }

      // Read the file and decode directly into Spdb
      cerr << "Processing " << FilePath << endl;
      ltg_decrypt(FilePath, &P);

    } // while loop.             

  } // End of if we are in realtime.

  //
  //
  return 0;

}


///////////////////////////////////////////////
// Parse command line args.
//
int NPLtg2Spdb::ParseArgs(int argc, char *argv[]){

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : NPLtg2Spdb [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-f filelist to run in archive mode]" << endl;
      return -1;

    }
    
    if (!strcmp(argv[i], "-f")){
      i++;
      if (i == argc) {
	cerr << "Must specify at least one input file after -f" << endl;
	exit(-1);
      }

      P.Mode = Params::ARCHIVE;

      if(i < argc) {
	int j;

	int go=1; j = i;
	do{
	  if (argv[j][0] == '-') {
	    go = 0;
	  } else {
	    cerr << argv[j] << endl;
	  }
	  j++;
	  if (j > argc-1) go = 0;
	} while (go);
	
	//
	// compute number of files
	//

	nFiles = j - i;

	cerr << nFiles << " files specified on command line." << endl;

	// set file name array

	filePaths = argv + i;

	
      }
      
    } // if
    
  }
  return 0;
}


     






