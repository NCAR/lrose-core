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
// TrackData.cc: TrackData handling - abstract base class
//
// Provides track data to client.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#include "TrackData.hh"
#include <toolsa/str.h>
using namespace std;

// Constructor

TrackData::TrackData (const string &prog_name, const Params &params,
		      DsInputPath &input) :
  _progName(prog_name),
  _params(params),
  _input(input)

{

  // initialize
  
  OK = TRUE;
  areaHist = (double *) NULL;
  MEM_zero(_coord);
  nProjRuns = 0;
  projRuns = NULL;

  return;

}

// Destructor

TrackData::~TrackData()

{

  if (areaHist) {
    ufree(areaHist);
  }
  if (projRuns) {
    ufree(projRuns);
  }

}

//////////////////////////////////////////
// _printTrack()
//
// Print track.
//

void TrackData::_printTrack(FILE *out)

{

  fprintf(out, "\n");
  fprintf(out, "TRACK: start_time: %s\n", utimstr(startTime));
  fprintf(out, "       end_time: %s\n", utimstr(endTime));
  fprintf(out, "       durationInScans: %d\n", durationInScans);
  fprintf(out, "       durationInSecs: %d\n", durationInSecs);
  fprintf(out, "\n");

}

//////////////////////////////////////////
// _printEntry()
//
// Print track entry.
//

void TrackData::_printEntry(FILE *out)

{

  fprintf(out, "\n");
  fprintf(out, "ENTRY: time: %s\n", utimstr(entryTime));
  fprintf(out, "       entryHistoryInScans: %d\n", entryHistoryInScans);
  fprintf(out, "       nDbzIntervals: %d\n", entryHistoryInScans);
  
  fprintf(out, "       centroidX: %g\n", centroidX);
  fprintf(out, "       centroidY: %g\n", centroidY);
  fprintf(out, "       tops: %g\n", tops);
  fprintf(out, "       volume: %g\n", volume);
  fprintf(out, "       area: %g\n", area);
  fprintf(out, "       dbzMax: %g\n", dbzMax);
  fprintf(out, "       dxDt: %g\n", dxDt);
  fprintf(out, "       dyDt: %g\n", dyDt);
  fprintf(out, "       majorRadius: %g\n", majorRadius);
  fprintf(out, "       minorRadius: %g\n", minorRadius);
  fprintf(out, "       ellipseOrientation: %g\n", ellipseOrientation);
  fprintf(out, "       lowDbzThreshold: %g\n", lowDbzThreshold);
  fprintf(out, "       dbzHistInterval: %g\n", dbzHistInterval);
  
  for (int i = 0; i < nDbzIntervals; i++) {
    double dbz = lowDbzThreshold + (i + 0.5) * dbzHistInterval;
    fprintf(out, "       hist: %d, %g, %g\n",
 	    i, dbz, areaHist[i]);
  }
  fprintf(out, "\n");

}

