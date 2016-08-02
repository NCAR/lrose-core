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
///////////////////////////////////////////////////////////////
// TimeProps.cc
//
// TimeProps object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// February 1998
//
///////////////////////////////////////////////////////////////

#include "TimeProps.h"
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <rapmath/math_macros.h>
using namespace std;

// Constructor

TimeProps::TimeProps(int argc, char **argv)

{

  OK = TRUE;
  Done = FALSE;

  // set programe name

  _progName = STRdup("TimeProps");
  ucopyright(_progName);

  // get command line args

  _args = new Args(argc, argv, _progName);
  if (!_args->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with command line args\n");
    OK = FALSE;
    return;
  }
  if (_args->Done) {
    Done = TRUE;
    return;
  }

  // get TDRP params

  _params = new Params(_args->paramsFilePath,
		       &_args->override,
		       _progName,
		       _args->checkParams,
		       _args->printParams,
		       _args->printShort);
  
  if (!_params->OK) {
    fprintf(stderr, "ERROR: %s\n", _progName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    OK = FALSE;
    return;
  }
  if (_params->Done) {
    Done = TRUE;
    return;
  }

  // Set up dataSet object

  _dataSet = new DataSet(_progName, _params);
  if (!_dataSet->OK) {
    OK = FALSE;
  }

  if (!OK) {
    return;
  }

  PMU_auto_init(_progName, _params->p.instance, PROCMAP_REGISTER_INTERVAL);
  
}

// destructor

TimeProps::~TimeProps()

{

  // unregister process

  PMU_auto_unregister();

  // call destructors

  delete _dataSet;
  delete _params;
  delete _args;

}

//////////////////////////////////////////////////
// Run

int TimeProps::Run ()
{

  if (_params->p.mode == DELTA_TIME_MODE) {
    _outputDtimes(stdout);
  }
  
  if (_params->p.mode == ACTIVITY_MODE) {
    _outputActivity(stdout);
  }

  return (0);

}

//////////////////////////////////////////////////
// _outputDtimes ()
//
// Output the dtimes to file
//

void TimeProps::_outputDtimes(FILE *out)

{

  fprintf(out, "#labels: utime(hr),time_of_day(hr),dtime(hr)\n");

  int start_pos = 0;
  int end_pos;
  double tod_offset = _params->p.activity_time_of_day_offet * 3600.0;

  // loop through active periods

  while (_dataSet->GetActivePeriod(start_pos, &end_pos) == 0) {
    
    if (end_pos - start_pos > _params->p.activity_nstorms_min) {
    
      double time, dur;
      double prev_time;
      
      double count = 0.0;
      _dataSet->Get(start_pos, &prev_time, &dur);
      
      for (int i = start_pos + 1; i <= end_pos; i++) {
	
	_dataSet->Get(i, &time, &dur);
	count++;
	
	if (time != prev_time) {
	  
	  // spread dtime amongst the number of points
	  
	  double dtime = (time - prev_time) / count;
	  
	  // output the required number of values
	  
	  for (int j = 0; j < count; j++) {
	    if (!_params->p.limit_dtime_data ||
		(dtime >= _params->p.dtime_min &&
		 dtime <= _params->p.dtime_max)) {
	      double this_time = prev_time + j * dtime;
	      double offset_time = this_time + tod_offset;
	      double time_of_day = fmod(offset_time, 86400.0);
	      fprintf(out, "%.4f %g %g\n",
		      this_time / 3600.0,
		      time_of_day / 3600.0,
		      dtime / 3600.0);
	      
	    }
	  } // j
	  
	  count = 0.0;
	  prev_time = time;
	  
	} // if (time != prev_time)

      } // i

    } // if (end_pos - start_pos ...

    start_pos = end_pos + 1;

  } // while

}

//////////////////////////////////////////////////
// _outputActivity ()
//
// Output the activity times to file
//

void TimeProps::_outputActivity(FILE *out)

{

  fprintf(out, "%s\n",
	  "#labels: "
	  "activity_start_time(hr),"
	  "start_time_of_day(hr),"
	  "activity_end_time(hr),"
	  "end_time_of_day(hr),"
	  "activity_duration(hr),"
	  "gap_since_previous(hr),"
	  "nstorms");

  int start_pos = 0;
  int end_pos;

  double time, dur;
  double prev_activity_end_time;
  double tod_offset = _params->p.activity_time_of_day_offet * 3600.0;

  // initialize initial gap to 1 year

  _dataSet->Get(0, &time, &dur);
  prev_activity_end_time = time - 31536000;

  while (_dataSet->GetActivePeriod(start_pos, &end_pos) == 0) {
  
    if (end_pos - start_pos > _params->p.activity_nstorms_min) {

      _dataSet->Get(start_pos, &time, &dur);
    
      double activity_start_time = time;
      double activity_end_time = time + dur;

      for (int i = start_pos + 1; i <= end_pos; i++) {
	_dataSet->Get(i, &time, &dur);
	activity_end_time = MAX(activity_end_time, (time + dur));
      } // i

      double start_tod = fmod(activity_start_time + tod_offset, 86400.0);
      double end_tod = fmod(activity_end_time + tod_offset, 86400.0);
      
      double activity_duration = activity_end_time - activity_start_time;
      double gap = activity_start_time - prev_activity_end_time;
      
      fprintf(out, "%.4f %g %.4f %g %g %g %d\n",
	      activity_start_time / 3600.0,
	      start_tod / 3600.0,
	      activity_end_time / 3600.0,
	      end_tod / 3600.0,
	      activity_duration / 3600.0,
	      gap / 3600.0,
	      end_pos - start_pos + 1);
      
      prev_activity_end_time = activity_end_time;

    } // if (end_pos - start_pos ...

    start_pos = end_pos + 1;

  } // while

  return;

}


