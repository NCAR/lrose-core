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

#include <iostream>
#include "Params.hh"
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <dirent.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "processIrisVol.hh"

using namespace std;

int main(int argc, char *argv[]){

  //
  // Read in the TDRP params.
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("iris2dsr", P.instance,
                PROCMAP_REGISTER_INTERVAL);     


  PMU_auto_register( "Init");

  //
  // Is this a cry for help?
  //
  for (int i=1; i < argc; i++){
    if (0==strncmp("-h", argv[i], 2)){
      cerr << "Use -f to specify header file names, ie." << endl;
      cerr << " -f TIA070724053502." << endl;
      exit(0);
    }
  }

  //
  // Init our main processing object.
  // This opens the output FMQ, among other things.
  //
  processIrisVol procIris( &P );

  int istart = -1;
  int iend = -1;

  for (int i=1; i < argc; i++){
    if (0==strncmp("-f", argv[i], 2)){
      if (i == argc-1){
	cerr << "-f needs a list of files" << endl;
	exit(-1);
      }
      
      //
      // Find the start and end indicies of the filenames in the arguments.
      //
      istart = i+1;
      iend = istart;
      int go = 1;
      do {
	if (iend == argc - 1) break;
	char *p = argv[iend+1];
	if (p[0] != '-'){
	  iend++;
	} else {
	  go=0;
	}
      } while (go);
    
      P.mode = Params::ARCHIVE;
    }
  }

  if (P.mode == Params::ARCHIVE){
    for (int k=istart; k <=iend; k++){
      if (P.debug){
	cerr << "Processing " << argv[k] << endl;
      }
      
      if (procIris.initVol( argv[k] )){
	cerr << "Failed to init on file " << argv[k] << endl;
	continue;
      } else {
	int loop = 1;
	int icount = 0;
	do {
	  if (procIris.processTilt(argv[k])){
	    loop = 0;
	  } else {
	    icount++;
	    if (P.debug){
	      cerr << "  Processed tilt " << icount;
	      cerr << " of " << argv[k] << endl << endl;
	    }
	  }
	} while(loop);
      }
      PMU_auto_register(argv[k]);
    }
  } else {
    //
    // REALTIME mode.
    //
    time_t lastFileTime = 0L;
    while (1) {

      //
      // Scan the directory looking for files aged
      // less than 2*strobeDelay that end in a dot
      //
      DIR *dirp;
      dirp = opendir(P.inputDir);
      if (dirp == NULL){
	cerr << "Failed to open " << P.inputDir << endl;
	perror(P.inputDir);
	return -1;
      }

      struct dirent *dp;
      for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)){
	//
	// PMU checkin.
	//
	PMU_auto_register("Reading dir ...");
	//
	// Skip . and ..
	//
	if (strcmp(dp->d_name, ".") == 0 ||
	    strcmp(dp->d_name, "..") == 0){
	  continue;
	}
	//
	// See if the file name ends in a dot.
	//
	if ('.' != dp->d_name[strlen(dp->d_name)-1]){
	  continue;
	}
	
	//
	// See if the file is too old.
	// Need to put the whole path together
	// and stat it to check.
	//
	char fullPath[1024];
	sprintf(fullPath,"%s/%s", P.inputDir, dp->d_name);
	
	struct stat buf;
	if (stat(fullPath, &buf)) continue;

	int ok2Process = 0;
	if (lastFileTime == 0L){
	  long age = time(NULL) - buf.st_ctime;
	  if (age < 2*P.strobeDelay){
	    ok2Process = 1;
	    lastFileTime = buf.st_ctime;
	  }
	} else {
	  if (buf.st_ctime > lastFileTime){
	    ok2Process = 1;
	    lastFileTime = buf.st_ctime;
	  }
	}
	
	if (ok2Process){
	  if (procIris.initVol( dp->d_name )){
	    cerr << "Failed to init on file " << dp->d_name << endl;
	    continue;
	  } else {
	    int loop = 1;
	    int icount = 0;
	    do {
	      if (procIris.processTilt(dp->d_name)){
		loop = 0;
	      } else {
		icount++;
		if (P.debug){
		  cerr << "  Processed tilt " << icount;
		  cerr << " of " << dp->d_name << endl << endl;
		}
	      }
	      PMU_auto_register(dp->d_name);
	    } while(loop);
	  }
	}
      }

      closedir(dirp);

      //
      // Delay before next check.
      //
      for (int j=0; j < P.strobeDelay; j++){
	sleep(1);
	PMU_auto_register("Waiting for data, realtime mode...");
      }
    } // eternal loop in realtime mode.
  }

  return 0;

}
