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
// TrackData.h: TrackData handling - abstract base class
//
// Provides track data to client.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#ifndef TrackData_hh
#define TrackData_hh

#include <toolsa/umisc.h>
#include "Params.hh"
#include <string>
#include <didss/DsInputPath.hh>
#include <rapformats/titan_grid.h>
#include <titan/storm.h>
#include <Mdv/Mdvx.hh>
using namespace std;

class TrackData {
  
public:

  // constructor
  
  TrackData (const string &prog_name, const Params &params,
	     DsInputPath &input);
  
  // Destructor
  
  virtual ~TrackData();
  
  // load up data for next track

  virtual int loadNextTrack(int *no_more_tracks_p) = 0;

  // load up data for next entry in track

  virtual int loadNextEntry(int *no_more_entries_p) = 0;

  // flag to indicate construction was successful

  int OK;

  // grid parameters

  const Mdvx::coord_t &getGrid() const { return _coord; }

  // track data

  time_t startTime;
  time_t endTime;
  int durationInScans;
  int durationInSecs;
  double scanIntervalSecs;

  // entry data

  time_t entryTime;
  int entryHistoryInScans;
  int nDbzIntervals;

  double centroidX;
  double centroidY;
  double tops;
  double volume;
  double area;
  double dbzMax;
  double dxDt;
  double dyDt;
  double majorRadius;
  double minorRadius;
  double ellipseOrientation;
  double lowDbzThreshold;
  double dbzHistInterval;

  double precipArea;
  double precipFlux;
  double precipDepthMm;

  double *areaHist;

  int nProjRuns;
  storm_file_run_t *projRuns;

protected:
  
  const string &_progName;
  const Params &_params;
  DsInputPath &_input;

  // coordinate grids

  titan_grid_t _titanGrid;
  bool _titanGridInitialized;
  Mdvx::coord_t _coord;

  void _printTrack(FILE *out);
  void _printEntry(FILE *out);

private:

};

#endif
