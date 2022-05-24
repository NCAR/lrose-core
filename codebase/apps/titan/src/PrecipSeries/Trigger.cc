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
// File-based trigger mechanism 
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include "Trigger.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <iostream>
using namespace std;

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Abstract base class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// constructor
//

Trigger::Trigger(string &prog_name, Params &params) :
  _progName(prog_name),
  _params(params)

{
}

/////////////
// destructor
//

Trigger::~Trigger()

{
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Archive mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

ArchiveTrigger::ArchiveTrigger(string &prog_name, Params &params,
			       time_t start_time, time_t end_time)
  : Trigger(prog_name, params)

{
  
  _inputPath = new DsInputPath((char *) _progName.c_str(),
			       _params.debug >= Params::DEBUG_VERBOSE,
			       _params.input_rdata_dir,
			       start_time, end_time);
  
}

/////////////
// destructor

ArchiveTrigger::~ArchiveTrigger()

{
  delete (_inputPath);
}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t ArchiveTrigger::next()

{

  char *path = _inputPath->next();

  if (path == NULL) {
    return (-1);
  }

  // find the slash delimiter for the data subdir

  char *slash = strstr(path, PATH_DELIM);
  if (slash == NULL) {
    cerr << "ERROR - " << _progName << ":ArchiveTrigger::next" << endl;
    cerr << "  Invalid path: " << path << endl;
    return (-1);
  }
  char *last_slash;
  while (slash != NULL) {
    last_slash = slash;
    slash = strstr(slash + 1, PATH_DELIM);
  }

  // copy the date and time strings

  char dateStr[16];
  char timeStr[16];
  strncpy(dateStr, last_slash - 8, 8);
  strncpy(timeStr, last_slash + 1, 6);

  // decode date and time

  date_time_t path_time;

  if (sscanf(dateStr, "%4d%2d%2d",
	     &path_time.year, &path_time.month, &path_time.day) != 3) {
    cerr << "ERROR - " << _progName << ":ArchiveTrigger::next" << endl;
    cerr << "  Invalid path: " << path << endl;
    cerr << "  Cannot decode date." << endl;
    return (-1);
  }
  
  if (sscanf(timeStr, "%2d%2d%2d",
	     &path_time.hour, &path_time.min, &path_time.sec) != 3) {
    cerr << "ERROR - " << _progName << ":ArchiveTrigger::next" << endl;
    cerr << "  Invalid path: " << path << endl;
    cerr << "  Cannot decode time." << endl;
    return (-1);
  }

  if (!uvalid_datetime(&path_time)) {
    cerr << "ERROR - " << _progName << ":ArchiveTrigger::next" << endl;
    cerr << "  Invalid path: " << path << endl;
    cerr << "  Invalid date/time from path name." << endl;
    return (-1);
  }

  uconvert_to_utime(&path_time);

  return (path_time.unix_time);

}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// Realtime mode file-based derived class
//
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

//////////////
// Constructor

RealtimeTrigger::RealtimeTrigger(string &prog_name, Params &params)
  : Trigger(prog_name, params)

{

  // init latest data handle
  LDATA_init_handle(&_lData, (char *) _progName.c_str(),
		    _params.debug >= Params::DEBUG_VERBOSE);
  
}

/////////////
// destructor

RealtimeTrigger::~RealtimeTrigger()

{

  // free up latest data info handle
  LDATA_free_handle(&_lData);

}

////////////////////////////////////////
// get next time - returns -1 on failure

time_t RealtimeTrigger::next()

{

  // wait for change in latest info,
  // sleep 1 second between tries.

  LDATA_info_read_blocking(&_lData, _params.input_rdata_dir,
			   360, 1000,
			   PMU_auto_register);
  
  return (_lData.ltime.unix_time);

}

