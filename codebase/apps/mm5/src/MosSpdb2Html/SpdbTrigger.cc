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

#include "SpdbTrigger.hh"
#include "ConstrueFilename.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>         
#include <toolsa/umisc.h>

#include <Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
using namespace std;

//////////////////////////////////////////////////////////
//
// Constructor. Just make copies of input args.
//
SpdbTrigger::SpdbTrigger(  const Params *params){

  _params = (Params *) params;
  
  //
  // set up the initial trigger times
  // to trigger on regualr intervals.
  //
  time_t now = time(NULL);
  time_t startHour = (now / 3600) * 3600;
  int secsSinceHour = now - startHour;
  int nTrig = secsSinceHour / _params->TriggerInterval;
  _prevTime = startHour + nTrig * _params->TriggerInterval;
  _nextTime = _prevTime + _params->TriggerInterval;       

}

//////////////////////////////////////////////////////////
//
// Main method - fills vector of entries, passed by ref.
// returns 0 if OK.
//
int SpdbTrigger::getNextTimes(vector<SpdbTrigger::entry_t> &UnprocessedEntries ){

  //
  // Trigger on regular intervals.
  //
  time_t triggerTime = SpdbTrigger::_triggerOnInterval();

  //
  // Calculate the temporal window.
  //
  time_t start = triggerTime - _params->LookBack;
  time_t end   = triggerTime + _params->LookAhead;

  if (_params->Debug){
    cerr << "Triggering at " << utimstr(triggerTime) << endl;
    cerr << "Window runs from " << utimstr(start);
    cerr << " to " << utimstr(end) << endl;
  }

  //
  // Get all the data from the input in this interval.
  //
  DsSpdb InputSpdb;
  if (InputSpdb.getInterval(_params->TriggerUrl, start, end)) {
      cerr << "Failed to get spdb data from " << _params->TriggerUrl;
      cerr << " between " << utimstr(start) << " and " << utimstr(end) << endl;
      cerr << InputSpdb.getErrorStr() << endl;
      return -1;
  }                                                            

  //
  // For each chunk from the input, if there is not
  // a corresponding entry in the ouput (same time,
  // datatype, and datatype2) then push the entry
  // back into the vector for returning.
  //
  const vector<Spdb::chunk_t> &chunks = InputSpdb.getChunks();

  if (_params->Debug){
    cerr << InputSpdb.getNChunks() << " input chunks found." << endl;
  }

  int AlreadyThere=0;
  int Added = 0;
  for (int ichunk = 0; ichunk < InputSpdb.getNChunks(); ichunk++){

    //
    // Stat the file and see if it exists.
    //
    struct stat fileStat;
    string filePath;
    ConstrueFilename( chunks[ichunk].valid_time,
		      chunks[ichunk].data_type2, // Type 2 is the lead time.
		      chunks[ichunk].data_type,
		      _params->ArchiveDir,
		      true,
		      filePath);
    if (_params->Debug){
      cerr << "File : " << filePath;
    }

    if (stat(filePath.c_str(), &fileStat) != 0 || 
	(!(S_ISREG(fileStat.st_mode)))) {
      //
      // This means that the file was not found.
      //
      if (_params->Debug){
	cerr << " not found - will be generated." << endl;;
      }
  
      SpdbTrigger::entry_t E;
      E.dataTime = chunks[ichunk].valid_time;
      E.dataType = chunks[ichunk].data_type;
      E.dataType2= chunks[ichunk].data_type2;
      
      UnprocessedEntries.push_back( E );
      Added++;
    } else {
      AlreadyThere++;
      if (_params->Debug){
	cerr << " found - will not be regenerated." << endl;;
      }
    }
  }       
  
  if (_params->Debug){
    cerr << AlreadyThere << " files already in output." << endl;
    cerr << Added << " files to be added." << endl;
  }
  
  return 0;

}


//////////////////////////////////////////////////////////
//
// Destructor
//
SpdbTrigger::~SpdbTrigger(){

}

//////////////////////////////////////////////////////////
//
// Interval timer.
//
time_t SpdbTrigger::_triggerOnInterval(){

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
    PMU_auto_register(_params->PMU_String);
    sleep(1);
    now = time(NULL);
  }
                             
  //
  // Increment until _nextTime is off in the future.
  //

  do{
    _prevTime = _nextTime;
    _nextTime += _params->TriggerInterval;
    now = time(NULL);
  } while (_nextTime <= now);
  
  return (_prevTime);
                    
}





