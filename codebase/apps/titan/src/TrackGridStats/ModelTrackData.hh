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
// ModelTrackData.h: ModelTrackData handling
//
// Serves out track and track_entry data.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#ifndef ModelTrackData_hh
#define ModelTrackData_hh

#include "TrackData.hh"
#include <toolsa/mem.h>
using namespace std;

class ModelTrackData : public TrackData {
  
public:

  // constructor
       
  ModelTrackData (const string &prog_name, const Params &params,
		  DsInputPath &input);
  
  // Destructor
  
  virtual ~ModelTrackData();
  
  // load up data for next track

  int loadNextTrack(int *no_more_tracks_p);

  // load up data for next entry in track

  int loadNextEntry(int *no_more_entries_p);

protected:
  
private:

  FILE *_modelFile;

  int _trackCount;
  int _nScans;
  double _startTime;
  double _durationHrs;
  double _durationSecs;
  double _Dm;
  double _AA;
  double _Amax;
  double _dbzThresh;
  double _dbzMean;
  double _dbzMax;
  double _dbzMin;
  double _startX;
  double _startY;
  double _u;
  double _v;
  double _speed;
  double _dirn;
  double _ellipseRatio;
  double _ellipseOrientation;

  int _iScan;

  int _openFile(char *model_file_path);

  int _readTrack();

  void _closeFile();

};

#endif
