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
// TodPeriod.cc
//
// Keeps track of which Time-Of-Day period we are in.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1998
//
///////////////////////////////////////////////////////////////

#include "TodPeriod.hh"
#include <toolsa/str.h>
#include <toolsa/pmu.h>
using namespace std;

//////////////
// constructor
//

TodPeriod::TodPeriod(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{

  OK = TRUE;
  _periodNum = -1;

  // alloc the times array for the periods
  
  _nPeriods = _params.restart_time_of_day_n;
  _periods = (period_times_t *)
    umalloc (_nPeriods * sizeof(period_times_t));

  // set the start and end times for each period

  for (int i = 0; i < _nPeriods; i++) {
    _periods[i].start =
      _params._restart_time_of_day[i].hour * 3600 +
      _params._restart_time_of_day[i].min * 60 +
      _params._restart_time_of_day[i].sec;
  }
  for (int i = 0; i < _nPeriods - 1; i++) {
    _periods[i].end = _periods[i+1].start;
  }
  _periods[_nPeriods-1].end = _periods[0].start;

  // check that periods are sequential
  
  for (int i = 0; i < _nPeriods - 1; i++) {
    if (_periods[i].start > _periods[i+1].start) {
      fprintf(stderr, "ERROR - %s:TodPeriod::TodPeriod\n",
	      _progName.c_str());
      fprintf(stderr, "Restart times must be in increasing order.\n");
      fprintf(stderr, "Check 'restart_time_of_day' in params file.\n");
      OK = FALSE;
      ufree(_periods);
      _periods = NULL;
      return;
    }
  }

  if (_params.debug >= Params::DEBUG_NORM) {
    for (int i = 0; i < _nPeriods; i++) {
      fprintf(stderr, "==> Period %d: %.2d:%.2d:%.2d to %.2d:%.2d:%.2d\n",
	      i,
	      (int) (_periods[i].start / 3600),
	      (int) ((_periods[i].start % 3600) / 60),
	      (int) (_periods[i].start % 60),
	      (int) (_periods[i].end / 3600),
	      (int) ((_periods[i].end % 3600) / 60),
	      (int) (_periods[i].end % 60));
    }
  }

}

/////////////
// destructor
//

TodPeriod::~TodPeriod()

{

  if (_periods) {
    ufree(_periods);
  }

}

////////
// set()
//
// Set the period times given the trigger time.
// Returns TRUE if period number has changed, FALSE otherwise.

int TodPeriod::set(time_t trigger_time)

{
  
  time_t secs_in_day = trigger_time % 86400;
  time_t jday = trigger_time / 86400;

  // compute period number

  int period_num = -1;
  for (int i = 0; i < _nPeriods-1; i++) {
    if (secs_in_day > _periods[i].start &&
	secs_in_day <= _periods[i].end) {
      period_num = i;
      break;
    }
  }

  if (period_num < 0) {

    // not yet set, must be last period which
    // always spans the end of the day

    period_num = _nPeriods - 1;

  }

  if (period_num != _periodNum || trigger_time > periodEnd) {

    // period num has changed

    _periodNum = period_num;
    periodStart = jday * 86400 + _periods[_periodNum].start;
    if (_periodNum < _nPeriods - 1) {
      // period ends in same day
      periodEnd = jday * 86400 + _periods[_periodNum].end;
    } else {
      // period ends next day
      periodEnd = (jday+1) * 86400 + _periods[_periodNum].end;
    }

    // check that we are on the right day - we may have to move
    // back a day because the period spans the beginning of the
    // day

    if (periodStart > trigger_time) {
      periodStart -= 86400;
      periodEnd -= 86400;
    } else if (periodEnd < trigger_time) {
      periodStart += 86400;
      periodEnd += 86400;
    }

    return (TRUE);

  } else {

    // period num has not changed

    return (FALSE);
    
  }

}

