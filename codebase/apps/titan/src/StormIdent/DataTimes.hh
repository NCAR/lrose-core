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
// DataTimes.hh
//
// File time limits class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#ifndef DataTimes_HH
#define DataTimes_HH

#include "Params.hh"
#include <ctime>
#include <toolsa/ldata_info.h>
using namespace std;

////////////////////////////////
// DataTimes

class DataTimes {
  
public:

  // REALTIME mode constructor

  DataTimes(char *prog_name, Params *params);

  // ARCHIVE mode constructor

  DataTimes(char *prog_name, Params *params,
	    time_t start_time, time_t end_time);

  // destructor
  
  virtual ~DataTimes();

  // get latest input data time
  time_t getLatest();

  // get the restart time
  time_t getRestartTime();

  // get the storm file date
  time_t getStormFileDate();

  // Get the start and end times for program startup
  void getStartupTimeLimits(time_t *start_time_p, time_t *end_time_p);

protected:
  
private:

  char *_progName;
  Params *_params;

  bool _realtimeMode;
  time_t _startTime;
  time_t _restartTime;
  time_t _endTime;
  time_t _stormFileDate;
  LDATA_handle_t _ldata;

  void _setRestartTime();
  void _setStormFileDate(time_t date);
  void _setTimeLimitsRealtime();
  int _setStartTimeFromStormFile();
  void _searchBackwardsForStartTime(time_t end_time);
  
};

#endif


