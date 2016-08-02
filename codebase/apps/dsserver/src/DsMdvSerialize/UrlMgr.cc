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

#include <toolsa/pmu.h>
#include <Mdv/DsMdvxInput.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <vector>

#include "UrlMgr.hh"

using namespace std;

////////////////////////////////////////////////
//
// Constructor
//
UrlMgr::UrlMgr( int argc, char **argv ){

  // Is this a cry for help?

  for (int i=1; i < argc; i++){
    if (!(strcmp("-h", argv[i]))){
      cerr << "Use -print_params for documentation - Niles." << endl;
      exit(0);
    }
  }

  // Start off pessimistic.

  OK = false;

  // Load up the TDRP parameters.

  if (_params.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return;
  }

  // Check in with PMU.

  PMU_auto_init("DsMdvSerialize", _params.Instance,
                PROCMAP_REGISTER_INTERVAL);

  // Set up the FMQ.

  if (_Queue.initCreate( _params.outFMQ,
                         "DsMdvSerialize",
                         true)){
    cerr << "Could not initialize FMQ!" << endl;
    exit(-1);
  }

  OK = true;
  return;

}

////////////////////////////////////////////////
//
// Main method - run.
//
int UrlMgr::Run( ){

  //
  // Set up a vector of DsMdvxTimes objects in non-blocking mode
  // to get the times.
  //

  vector<time_t> lastUrlTimesVec;
  vector<time_t> currentUrlTimesVec;
  vector<DsMdvxTimes *> mdvxTimesVec;

  for (int ii = 0; ii < _params.input_urls_n; ii++) {
    
    DsMdvxTimes *mdvxTimes = new DsMdvxTimes();
    
    // Set to non-blocking reads to get times.
    
    mdvxTimes->setRealtime(_params._input_urls[ii].url, 
			   _params.max_realtime_valid_age,
			   PMU_auto_register,
			   -1);

    mdvxTimesVec.push_back(mdvxTimes);

    lastUrlTimesVec.push_back(0L);
    currentUrlTimesVec.push_back(0L);
    
  }

  //
  // Watch them to see if they update. Infinite loop.
  //

  while( 1 ){
    PMU_auto_register("Watching URLs");

    //
    // Get current times from all the URLs.
    //
    for (int ii = 0; ii < _params.input_urls_n; ii++) {
      time_t latestTime;
      int retVal = mdvxTimesVec[ii]->getNext( latestTime );
      if ((retVal == 0) && (mdvxTimesVec[ii]->getNextSuccess())){
	currentUrlTimesVec[ii] = latestTime;
      }
    }

    //
    // See if any have updated, deal with the ones that have.
    //
    for (int ii = 0; ii < _params.input_urls_n; ii++) {
      //
      if (currentUrlTimesVec[ii] > lastUrlTimesVec[ii]){
	//
	// It has updated ...
	//
	lastUrlTimesVec[ii] = currentUrlTimesVec[ii];

	_dealWithUpdate(currentUrlTimesVec[ii],
			ii );

      }
    }
    sleep(1);
  }
  //
  // Should never get here ...
  //
  for (size_t i = 0; i < mdvxTimesVec.size(); i++){
    delete mdvxTimesVec[i];
  }


  return 0;

}


void UrlMgr::_dealWithUpdate(time_t dataTime, int urlNum){

  cerr << "I have update from ";
  cerr << _params._input_urls[urlNum].url << " (";
  cerr << _params._input_urls[urlNum].uniqueName << ")" << endl;
  cerr << "    Data time " << utimstr(dataTime);
  time_t now = time(NULL);
  cerr << " update detected at " << utimstr(now) << endl;

  char outDir[MAX_PATH_LEN];
  sprintf(outDir, "%s/%s", _params.outDir,  
	  _params._input_urls[urlNum].uniqueName );

  if (ta_makedir_recurse( outDir )){
    return;
  }

  date_time_t T;
  T.unix_time = dataTime;
  uconvert_from_utime( &T );

  char outName[MAX_PATH_LEN];
  sprintf(outName, "%s/%d%02d%02d_%02d%02d%02d.mdvSerial", outDir,  
	  T.year, T.month, T.day,
	  T.hour, T.min, T.sec);
  
  //
  // Read in, and then write out to that path.
  //
  DsMdvx M;

  M.setReadTime(Mdvx::READ_FIRST_BEFORE, 
		_params._input_urls[urlNum].url,
		0, dataTime);

  if (M.readVolume()){
    return;
  }

  if (M.writeToPath( outName )){
    return;
  }

  //
  // OK - the file has been written, write it out to
  // our FMQ.
  //

  if (_Queue.writeMsg( 12, 0, outName, strlen(outName)+1 )){
    cerr << "FMQ write failed!" << endl;
    return;
  }


  return;

}



////////////////////////////////////////////////
//
// Destructor
//
UrlMgr::~UrlMgr(){


  return;
}

