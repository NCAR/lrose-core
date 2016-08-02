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
// StormTrack.hh
//
// StormTrack object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef StormTrack_H
#define StormTrack_H

#include "Worker.hh"
#include "TrStorm.hh"
#include <toolsa/MemBuf.hh>
#include <euclid/point.h>
using namespace std;

////////////////////////
// This class

class StormTrack : public Worker {
  
public:

  // constructor

  StormTrack (const string &prog_name, const Params &params,
	      const string &storm_header_path);

  // destructor
  
  ~StormTrack();

  // Re-track a storm file

  int ReTrack ();

  // Prepare a new track file

  int PrepareNewFile ();

  // Prepare existing track file for appending

  int PrepareForAppend ();

  // Track the last scan in storm file

  int TrackLastScan ();

  // was error fatal?

  bool fatalError() { return _fatalError; }

protected:
  
private:

  bool _fatalError;
  string _stormHeaderPath;
  string _stateFilePath;
  time_t _stateTag;
  TitanStormFile _sfile;
  TitanTrackFile _tfile;
  vector<TrStorm*> _storms1;
  vector<TrStorm*> _storms2;
  date_time_t _time1;
  date_time_t _time2;
  vector<track_utime_t> _trackUtime;
  bool _filePrepared;

  int _scan_num;
  date_time_t _scan_time;
  int _n_storms;
  long _entry_offset;
  long _prev_scan_entry_offset;
  bool _write_in_progress;

  // clearing

  void _clearStorms1();
  void _clearStorms2();

  // load bounding box

  void _loadBounds(double d_hours);

  // load up storm props

  int _loadStormProps(vector<TrStorm*> &storms);

  // initialize for tracking

  void _initForTracking();
  int _prepareTrackFile();

  // do tracking

  int _track(int scan_num);

  // transfer storms2 to storms1

  void _transferStorms();

  // match routines

  void _matchStorms(double d_hours);

  bool _matchFeasible(TrStorm &storm1, TrStorm &storm2,
		       double d_hours, int grid_type);

  void _resolveMatches();

  void _checkMatches();
  
  void _deleteSmallestOverlap(int this_storm_num,
				vector<TrStorm*> &these_storms,
				vector<TrStorm*> &other_storms);

  void _deleteMatch(TrStorm &storm, int storm_num);

  void _printMatches();

  void _printStatus(TrStorm &storm);


  // update functions

  int _updateTracks(date_time_t *dtime,
		     int scan_num,
		     double d_hours);

  int _handleCombined(date_time_t *dtime,
		       int scan_num,
		       double d_hours,
		       TrStorm &storm2);

  void _getCorrectionForCombination(TrStorm &storm2,
				       Point_d *pos_corr,
				       double d_hours);
  
  void _getCorrectionForMerger(TrStorm &storm2,
				  Point_d *pos_corr);

  void _getCorrectionForSplit(TrStorm &storm2,
				 Point_d *pos_corr);


  // save the current state to be used in the case of a restart
  // Returns 0 on success, -1 on failure

  int _saveCurrentState();

  // read the previous state, set up _storms1 and _trackUtime vectors
  // Returns 0 on success, -1 on failure

  int _readPrevState();

  // Remove the current state file
  
  void _removeCurrentStateFile();

  // storm and track file handling

  int _openFiles(const char *access_mode,
		 const char *storm_header_path);

  void _closeFiles();

  void _initTrackHeader(track_file_header_t &t_header);

  int _writeTrack(date_time_t *dtime,
		  vector<track_utime_t> &track_utime,
		  TrStorm &storm,
		  int storm_num,
		  int scan_num);

  int _setupScan(int scan_num);

  int _setHeaderInvalid();

  int _setHeaderValid(int scan_num, time_t &state_tag);
 




  
  

};

#endif

