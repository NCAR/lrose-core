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
// StartTime.cc
//
// StartTime object - for generating storm start time.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "StartTime.hh"
#include <toolsa/mem.h>
#include <toolsa/str.h>
#include <rapmath/stats.h>
using namespace std;

// Constructor

StartTime::StartTime(const char *prog_name, const Params &params) :
        _params(params)

{

  _progName = STRdup(prog_name);
  OK = TRUE;
  _eventStartTodCdf = (GenFromCdf *) NULL;

  // start generating at time 0

  _eventStart = 0.0;
  _eventEnd = 0.0;
  _prevStormTime = 0.0;

  // create CDF object for event start TOD

  int nstartHist = _params.event_start_tod_hist_n;
  Params::event_start_tod_hist_t *startHist = _params._event_start_tod_hist;
  
  double *startVals = (double *) umalloc(nstartHist * sizeof(double));
  double *startProb = (double *) umalloc(nstartHist * sizeof(double));
  
  for (int i = 0; i < nstartHist; i++) {
    startVals[i] = startHist[i].time;
    startProb[i] = startHist[i].prob;
  }

  _eventStartTodCdf =
    new GenFromCdf(nstartHist, startProb, startVals,
		   (char *) "event_start_tod_hist");
  if (!_eventStartTodCdf) {
    OK = FALSE;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _eventStartTodCdf->print(stderr);
  }

  ufree(startVals);
  ufree(startProb);

}

// destructor

StartTime::~StartTime()

{

  STRfree(_progName);

  if (_eventStartTodCdf != NULL) {
    delete (_eventStartTodCdf);
  }

}

//////////////////////////////////////////////////
// Generate()
//
// Generate next start point.

double StartTime::Generate()

{

  // generate storm start gap from lognormal - must be less
  // than event_gap_min

  double storm_start_gap = _params.event_gap_min * 2.0;

  while (storm_start_gap >= _params.event_gap_min ||
	 storm_start_gap <= _params.storm_start_gap_min) {
    double ln_storm_start_gap =
      STATS_normal_gen(_params.ln_storm_start_gap_norm.mean,
		       _params.ln_storm_start_gap_norm.sdev);
    storm_start_gap = exp(ln_storm_start_gap);
  }

  // compute storm start time
  
  double storm_start_time = _prevStormTime + storm_start_gap;

  // check whether the storm time fits within the current period
  // of event - if not, generate new event

  if (storm_start_time > _eventEnd) {
    _genEvent();
    storm_start_time = _eventStart;
  }
  
  _prevStormTime = storm_start_time;

  return (storm_start_time);
  
}

//////////////////////////////////////////////////
// _genEvent()
//
// Generate next event

void StartTime::_genEvent()

{

  // compute the event start time. This is reptitive.
  // First generate the event gap, and from that compute
  // the first guess of the start time. Then generate the
  // start time-of-day, and hence the final start time.
  // check that this exceeds the latest event end time
  // by a minimum amount. If not, repeat.

  double event_start = _eventEnd;

  while (event_start <=
	 _eventEnd + _params.event_gap_min) {
    
    // generate the event gap from lognormal - must exceed
    // event_gap_min
    
    double event_gap = -1.0;
    
    while (event_gap < _params.event_gap_min) {
      event_gap =
	STATS_gamma_gen(_params.event_gap_gamma.shape,
			_params.event_gap_gamma.scale);
    }

    // compute the first guess event start time
    
    double first_guess_start = _eventEnd + event_gap;

    // compute start day
    
    double start_day;
    modf(first_guess_start / 24.0, &start_day);
    
    // generate start time-of-day
    
    double event_start_tod = _eventStartTodCdf->Generate();
    
    // compute event start
    
    event_start = start_day * 24.0 + event_start_tod;

  } // while

  // generate event duration
  
  double event_dur =
    STATS_gamma_gen(_params.event_dur_gamma.shape,
		    _params.event_dur_gamma.scale);

  // set variables

  _eventStart = event_start;
  _eventEnd = event_start + event_dur;
  _eventDur = event_dur;

  return;

}
