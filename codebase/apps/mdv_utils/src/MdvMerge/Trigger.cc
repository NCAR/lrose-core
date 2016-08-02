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
/////////////////////////////////////////////////////////////
// Trigger.cc
//
// Trigger mechanism 
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#include "Trigger.hh"
#include "Params.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <iostream>
#include <cstdlib>  
#include <unistd.h> 


// For wait3() call.      
#define _USE_BSD
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>




#include <toolsa/umisc.h>
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

}

/////////////
// destructor
//

Trigger::~Trigger()

{

  STRfree(_progName);

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Archive mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTimeTrigger::ArchiveTimeTrigger(char *prog_name, Params *params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _startTime = start_time;
  _endTime = end_time;
  _prevTime = _startTime - _params->time_trigger_interval;
  
}

/////////////
// destructor

ArchiveTimeTrigger::~ArchiveTimeTrigger()

{
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveTimeTrigger::next()

{

  time_t thisTime = _prevTime + _params->time_trigger_interval;
  if (thisTime > _endTime) {
    return (-1);
  } else {
    _prevTime = thisTime;
    return (thisTime);
  }

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Archive mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveFileTrigger::ArchiveFileTrigger(char *prog_name, Params *params,
				       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{

  _inputPath = new DsMdvxInput;
  _inputPath->setArchive(_params->_input_urls[0],
			 start_time, end_time);
 
}

/////////////
// destructor

ArchiveFileTrigger::~ArchiveFileTrigger()

{
  delete _inputPath;
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveFileTrigger::next()

{

  time_t data_time;
  while(!_inputPath->endOfData()) {
    if(_inputPath->getTimeNext(data_time)) {
      cerr << "ERROR - " << _progName << ":ArchiveFileTrigger::next"  << endl;
      cerr << _inputPath->getErrStr() << endl;
      return (-1);
    }

    return (data_time);

  }

  return (-1);
  
}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Realtime mode time-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTimeTrigger::RealtimeTimeTrigger(char *prog_name, Params *params)
  : Trigger(prog_name, params)
    
{

  // set up the initial trigger times

  time_t now = time(NULL);
  time_t startHour = (now / 3600) * 3600;
  int secsSinceHour = now - startHour;
  int nTrig = secsSinceHour / _params->time_trigger_interval;
  _prevTime = startHour + nTrig * _params->time_trigger_interval;
  _nextTime = _prevTime + _params->time_trigger_interval;

}

/////////////
// destructor

RealtimeTimeTrigger::~RealtimeTimeTrigger()

{
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeTimeTrigger::next()

{

  // check if _nextTime crosses hour - if so, set it to the
  // next hour

  long prevHour = _prevTime / 3600;
  long nextHour = _nextTime / 3600;

  if (nextHour != prevHour) {
    _nextTime = nextHour * 3600;
  }

  // wait for next time

  time_t now = time(NULL);
  while (now < _nextTime) {
    PMU_auto_register("In RealtimeTimeTrigger::next");
    sleep(1);
    now = time(NULL);
  }

  _prevTime = _nextTime;
  _nextTime += _params->time_trigger_interval;
  
  return (_prevTime);

}

///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
//
// Realtime mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeFileTrigger::RealtimeFileTrigger(char *prog_name, Params *params)
  : Trigger(prog_name, params)

{
  //
  // Do sanity check on the input parameters.
  //
  for (int i=0; i < _params->urls_that_must_have_updated_n; i++){
    int UrlNum =  _params->_urls_that_must_have_updated[i];
    if (
	(UrlNum > _params->input_urls_n-1 ) ||
	(UrlNum < 0)
	){
      cerr << "ERROR - param file specifies invalid URL number ";
      cerr << "in urls_that_must_have_updated array - edit params and re-run.";
      cerr << endl;
      exit(-1);
    }
  }


  //
  // Set up the vector of DsMdvxTimes objects.
  //
  mdvxInputs.clear();

  for (int i=0; i < _params->input_urls_n; i++){
    DsMdvxTimes *mdvxInput = new DsMdvxTimes();
    //
    // Set to non-blocking reads to get times.
    //
    mdvxInput->setRealtime( _params->_input_urls[i], 
			    _params->max_valid_age,
			    PMU_auto_register,
			    -1);
    mdvxInputs.push_back( mdvxInput );
  }
  
  //
  // Set up lastTriggerTime.
  //
  lastTriggerTime = 0;

  //
  // Allocate space for array of time for each URL.
  // Initialize to 0 with calloc.
  //
  lastUrlTime = (time_t *) calloc(_params->input_urls_n,
				  sizeof(time_t));
  //
  // Ditto for the array of current times.
  //
  currentUrlTime = (time_t *) calloc(_params->input_urls_n,
				  sizeof(time_t));


}

/////////////
// destructor

RealtimeFileTrigger::~RealtimeFileTrigger()

{
  //
  // Free up the arrays of URL times.
  //
  free( lastUrlTime );  free( currentUrlTime );
  //
  // Free the vector of DsMdvxTimes objects.
  //
  for (int i=0; i < _params->input_urls_n; i++){
    delete mdvxInputs[i];
  }
  mdvxInputs.clear();

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeFileTrigger::next()

{
  time_t when = 0;
  //
  // Initialize the time arrays to what is actually out there.
  //
  initTimeArrays();
  //
  // We remain in this do loop until the return time we have
  // in the variable 'when' exceeds the last trigger time by at
  // least _params->min_time_between_triggers.
  //
  do {
      PMU_auto_register("Waiting for new data.");
      sleep(1);
    //
    // First, check that all the URLs that must update
    // before we can go on have done so.
    //
    bool allUpdated;
    do {
      allUpdated = true;
      for (int i=0; i < _params->urls_that_must_have_updated_n; i++){
	int UrlNum =  _params->_urls_that_must_have_updated[i];
	if (!(hasUpdated(UrlNum))) {
	  allUpdated = false;
	  break;
	}
      }
      PMU_auto_register("Waiting for all URLs to update.");
      sleep(1);
    } while(!(allUpdated));
    //
    // Then, check that at least the minimum number of URLs have
    // updated.
    //    
    int numUpdated;
    do {
      numUpdated = 0;
      for (int i=0; i < _params->input_urls_n; i++){
	if (hasUpdated(i)) 
	  numUpdated++;
      }
      PMU_auto_register("Waiting for enough URLs to update.");
      sleep(1);
    } while(numUpdated < _params->min_number_updated_urls);
    //
    // OK - if we are here, then the URLs that must update before we
    // go further have updated, and we have the requisite number of URLs
    // that have updated. Find the most recent time from all
    // the URLs, and return the most recent.
    //
    updateCurrent();
    for (int i=0; i < _params->input_urls_n; i++){
      if (currentUrlTime[i] > when){
	when = currentUrlTime[i];
      }
    }
    //
    // End of outermost do loop (below) just makes sure we have
    // met or exceeded the specified min_time_between_triggers.
    //
  } while ((when == 0) ||
	   (when < lastTriggerTime + _params->min_time_between_triggers));

  //
  // Store the last trigger time before we return it.
  //
  lastTriggerTime = when;
  //
  // Update the array of times for each URL.
  //
  updateLatestToCurrent();
  //
  // Return the time.
  //
  return (when);

}

/////////////////////////////////////////////////
//
// Small method to see if a URL has updated.
//
bool RealtimeFileTrigger::hasUpdated(int UrlNum){
  //
  // Update all current times to what is currently out there
  //
  updateCurrent();
  //
  // See if this particular one has updated.
  //  
  if (currentUrlTime[UrlNum] > lastUrlTime[UrlNum])
    return true;

  return false;

}

/////////////////////////////////////////////////
//
// Small method to initialize the time arrays.
//
void RealtimeFileTrigger::initTimeArrays(){

  //
  // Set both the current time and the latest time
  // for the URLs to what is currently out there.
  //
  for (int i=0; i < _params->input_urls_n; i++){
    time_t dataTime=0;
    if (0 == mdvxInputs[i]->getNext( dataTime )){
      currentUrlTime[i] = dataTime;
      lastUrlTime[i] = dataTime;
    } else {
      currentUrlTime[i] = 0L;
      lastUrlTime[i] = 0L;
    }
  }
  return;
}

/////////////////////////////////////////////////
//
// Small method to update the latest time array
// to the current time array. Updates the current time array first.
//
void RealtimeFileTrigger::updateLatestToCurrent(){

  updateCurrent();
  for (int i=0; i < _params->input_urls_n; i++){
    if (currentUrlTime[i] > lastUrlTime[i])
      lastUrlTime[i] = currentUrlTime[i];
  }
  return;
}

/////////////////////////////////////////////////
//
// Small method to update the current time array
// by looking at the actual URLs.
//
void RealtimeFileTrigger::updateCurrent(){

  for (int UrlNum=0; UrlNum < _params->input_urls_n; UrlNum++){
    time_t dataTime=0;
    if (0 ==mdvxInputs[UrlNum]->getNext( dataTime )){
      if (dataTime > currentUrlTime[UrlNum]){
	//
	// If we have a current data time, and it is greater
	// than the time we used to have, update the current array.
	//
	currentUrlTime[UrlNum] = dataTime;
      }
    }
  }

  return;

}

