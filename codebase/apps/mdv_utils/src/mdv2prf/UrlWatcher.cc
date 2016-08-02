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
#include <Mdv/DsMdvxTimes.hh>
#include <iostream>
#include <strings.h>

#include "Params.hh"
#include "UrlWatcher.hh"
#include "Process.hh"
using namespace std;


//////////////////////////////////////////////////
// Main method - run.
//
int UrlWatcher::init( int argc, char **argv ){

  //
  // Parse command line args. Pretty minimal for this program.
  //
  if (ParseArgs(argc,argv)) return -1;


  // Initialize the time arrays to what is actually out there.

  _initTimeArrays();


  return 0;

}


int UrlWatcher::run(){


  if ((_endTime != 0L) && (_startTime != 0L)){
    //
    // Archive mode
    //
    DsMdvxTimes InMdv;
    if (InMdv.setArchive(P._input_urls[0].url,
                         _startTime,
                         _endTime)){
      cerr << "Failed to set URL " << P._input_urls[0].url << endl;
      return -1;
    }

    do{
      
      time_t Time;
    
      InMdv.getNext( Time );
    
      if (Time == (time_t)NULL){
	return 0; // Reached end of the list in archive mode.
      }

      if (Time != (time_t)NULL){
	Process S;
	S.Derive(&P, Time);
      }
      
    } while (1);

  } // End of if we are in archive mode

  do{

    time_t Time = next();
    Process S;
    S.Derive(&P, Time);
    
  } while (1);

  return 0;

}

///////////////////////////////////////////////
// Parse command line args.
//
int UrlWatcher::ParseArgs(int argc,char *argv[]){

  for (int i=1; i<argc; i++){
 
    if ( (!strcmp(argv[i], "-h")) ||
         (!strcmp(argv[i], "--")) ||
         (!strcmp(argv[i], "-?")) ) {                
      cout << "USAGE : mdv2prf [-print_params to get parameters]" << endl;
      cout << "[-h or -- or -? to get this help message]" << endl;
      cout << "[-interval YYYYMMDDhhmmss YYYYMMDDhhmmss -params pfile to run in archive mode - first URL is used as trigger]" << endl;
      return 0;

    }
    
    if (!strcmp(argv[i], "-interval")){
      i++;
      if (i == argc) {
	cerr << "Must specify start time after -interval, format is YYYYMMDDhhmmss" << endl;
	exit(-1);
      }
      
      date_time_t T;
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for time is YYYYMMDDhhmmss" << endl;
	return -1;
      }
      uconvert_to_utime( &T );
      _startTime = T.unix_time;
    

      i++;
      if (i == argc) {
	cerr << "Must specify end time after -interval, format is YYYYMMDDhhmmss" << endl;
	exit(-1);
      }
      
      if (6!=sscanf(argv[i],"%4d%2d%2d%2d%2d%2d",
		    &T.year, &T.month, &T.day,
		    &T.hour, &T.min, &T.sec)){
	cerr << "Format for time is YYYYMMDDhhmmss" << endl;
	return -1;
      }
      uconvert_to_utime( &T );
      _endTime = T.unix_time;


    }

  }

  return 0; // All systems go.
  
}

     
/////////////////// Triggering routines ////////////////////////////

//////////////
// Constructor

UrlWatcher::UrlWatcher(int argc,char *argv[]) {

  // Initialize

  _startTime = 0L;
  _endTime = 0L;

  //
  // Get TDRP args and check in to PMU.
  //
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    exit(-1);
  }                       
  
  PMU_auto_init("mdv2prf", P.Instance,
                PROCMAP_REGISTER_INTERVAL);

  // Set up the vectors

  for (int ii = 0; ii < P.input_urls_n; ii++) {

    if (P.Debug)
      cerr << "Initializing with " << P._input_urls[ii].url << endl;
    
    DsMdvxTimes *mdvxTimes = new DsMdvxTimes();
    
    // Set to non-blocking reads to get times.
    
    mdvxTimes->setRealtime(P._input_urls[ii].url, 
			   P.max_realtime_valid_age,
			   PMU_auto_register,
			   -1);

    _mdvxTimes.push_back(mdvxTimes);

    _lastUrlTimes.push_back(0);
    _currentUrlTimes.push_back(0);
    
  }
  
  // initialize lastTriggerTime.
  
  _lastTriggerTime = 0;

  return;

}

/////////////
// destructor

UrlWatcher::~UrlWatcher(){
  
  for (size_t i = 0; i < _mdvxTimes.size(); i++){
    delete _mdvxTimes[i];
  }

  return;

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t UrlWatcher::next(){
  
  time_t when = 0;
  
  // We remain in this do loop until the return time we have
  // in the variable 'when' exceeds the last trigger time by at
  // least P.min_time_between_triggers.

  while ((when == 0) ||
	 (when < _lastTriggerTime + P.min_time_between_file_triggers)) {

    PMU_auto_register("Waiting for new data.");
    umsleep(1000);

    
    if (P.Debug){
      cerr << "Waiting for at least ";
      cerr << P.min_number_updated_urls << " url(s) to update......" << endl;
    }


    // Check that at least the minimum number of URLs have
    // updated.

    int numUpdated = 0;
    while(numUpdated < P.min_number_updated_urls) {
      numUpdated = 0;
      for (int ii = 0; ii < P.input_urls_n; ii++){
	if (_hasUpdated(ii)) {
	  numUpdated++;
	}
      }
      PMU_auto_register("Waiting for enough URLs to update.");
      umsleep(1000);
    } // while

    if (P.Debug){
      cerr << ".........Minimum number of URLs have updated" << endl;
    }


    // OK - if we are here, then the URLs that must update before we
    // go further have updated, and we have the requisite number of URLs
    // that have updated. Find the most recent time from all
    // the URLs, and return the most recent.

    _updateCurrent();
    for (int ii = 0; ii < P.input_urls_n; ii++) {
      if (_currentUrlTimes[ii] > when){
	when = _currentUrlTimes[ii];
      }
    }

    // End of outermost do loop (below) just makes sure we have
    // met or exceeded the specified min_time_between_triggers.
    
  } // while ((when == 0) ...

  // Store the last trigger time before we return it.

  _lastTriggerTime = when;

  // Update the array of times for each URL.

  _updateLatestToCurrent();

  // Return the time.

  if (P.Debug){
    cerr << "Trigger returning time " << utimstr( when ) << endl;
  }
  
  return when;

}

/////////////////////////////////////////////////
// Has URL updated?

bool UrlWatcher::_hasUpdated(int UrlNum)

{

  // Update all current times to what is currently out there

  //  if (P.Debug){
  //    cerr << "Checking to see if ";
  //    cerr << P._input_urls[UrlNum].url;
  //    cerr << " has updated..." << endl;
  //  }

  _updateCurrent();

  // See if this particular one has updated.

  if (_currentUrlTimes[UrlNum] > _lastUrlTimes[UrlNum]) {
    //    if (P.Debug){
    //      cerr << "...it has, time now " << utimstr(_currentUrlTimes[UrlNum]) << endl;
    //    }
    return true;
  }

  //  if (P.Debug){
  //    cerr << "...it has not, time still " << utimstr(_currentUrlTimes[UrlNum]) << endl;
  //  }

  return false;

}

/////////////////////////////////////////////////
//
// Initialize the time arrays.

void UrlWatcher::_initTimeArrays()

{

  // Set both the current time and the latest time
  // for the URLs to what is currently out there.

  for (int ii = 0; ii < P.input_urls_n; ii++) {

    time_t dataTime = 0;

    if (P.Debug)
      cerr << "Considering URL " << P._input_urls[ii].url << endl;

    if (NULL == _mdvxTimes[ii]){
      cerr << "NULL" << endl;
      exit(-1);
    }

    if (0 == _mdvxTimes[ii]->getNext(dataTime)) {

      if (P.Debug)
	cerr << "  Time returned was " << utimstr(dataTime) << endl;

      if (P.futureLimit.applyFutureLimit){
	time_t now = time(NULL);
	if (dataTime > now + P.futureLimit.futureLimitTime){
	  _currentUrlTimes[ii] = 0;
	  _lastUrlTimes[ii] = 0;
	  continue;
	}
      }
      _currentUrlTimes[ii] = dataTime;

      if (P.current_at_startup)
	_lastUrlTimes[ii] = dataTime;
      else
	_lastUrlTimes[ii] = 0L;

    } else {
      _currentUrlTimes[ii] = 0;
      _lastUrlTimes[ii] = 0;
    }

  } // ii

  return;

}

/////////////////////////////////////////////////
//
// Update the latest time array to the current
// time array. Updates the current time array first.

void UrlWatcher::_updateLatestToCurrent()

{

  _updateCurrent();

  for (int ii = 0; ii < P.input_urls_n; ii++){
    if (_currentUrlTimes[ii] > _lastUrlTimes[ii]) {
      _lastUrlTimes[ii] = _currentUrlTimes[ii];
    }
  }

  return;

}

/////////////////////////////////////////////////
//
// Update the current time array
// by looking at the actual URLs.

void UrlWatcher::_updateCurrent()

{

  for (int ii = 0; ii < P.input_urls_n; ii++){

    time_t dataTime = 0;

    if (0 == _mdvxTimes[ii]->getNext(dataTime)) {

      if (P.futureLimit.applyFutureLimit){
	time_t now = time(NULL);
	if (dataTime > now + P.futureLimit.futureLimitTime){
	  continue;
	}
      }

      if (dataTime > _currentUrlTimes[ii]){

	// If we have a current data time, and it is greater
	// than the time we used to have, update the current array.

	_currentUrlTimes[ii] = dataTime;

      }

    }

  } // ii

  return;

}







