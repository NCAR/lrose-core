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
// StatsGrid.h
//
// Compute stats, writes out MDV file
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
//////////////////////////////////////////////////////////

#ifndef StatsGrid_h
#define StatsGrid_h

#include "Args.hh"
#include "Params.hh"
#include "TrackData.hh"
#include <string>
#include <toolsa/umisc.h>
#include <Mdv/DsMdvx.hh>
using namespace std;

// struct for grid stats

typedef enum {
  N_EVENTS_POS,
  N_WEIGHTED_POS,
  N_COMPLEX_POS,
  PERCENT_ACTIVITY_POS,
  N_START_POS,
  N_MID_POS,
  PRECIP_POS,
  VOLUME_POS,
  DBZ_MAX_POS,
  TOPS_POS,
  SPEED_POS,
  U_POS,
  V_POS,
  DISTANCE_POS,
  DX_POS,
  DY_POS,
  AREA_POS,
  DURATION_POS,
  LN_AREA_POS,
  ELLIPSE_U,
  ELLIPSE_V,
  N_STATS_FIELDS
} stats_field_t;

typedef struct {

  double n_events;
  double n_weighted;
  double n_complex;
  double percent_activity;
  double n_start;
  double n_mid;
  double precip;
  double volume;
  double dbz_max;
  double tops;
  double speed;
  double u;
  double v;
  double distance;
  double dx;
  double dy;
  double area;
  double duration;
  double ln_area;
  double ellipse_u;
  double ellipse_v;

} grid_stats_t;

class StatsGrid {
  
public:

  // constructor
  
  StatsGrid (const string &prog_name,
	     const Params &params,
	     const Args &args,
	     TrackData &trackData,
	     time_t startTime,
	     time_t endTime);
  
  // Destructor
  
  virtual ~StatsGrid();

  // compute stats

  void compute();

  // write the output file

  int writeOutputFile();

protected:
  
private:

  const string &_progName;
  const Params &_params;
  const Args &_args;
  TrackData &_trackData;
  time_t _startTime;
  time_t _endTime;
  grid_stats_t **_stats;

  time_t _dataStart;
  time_t _dataEnd;

  double _nScansElapsed;

  void _loadTrackData();

  void _loadFromRuns(si32 &start_ix,
		     si32 &start_iy,
		     si32 &end_ix,
		     si32 &end_iy,
		     ui08 **storm_grid,
		     double **precip_grid);

  void _loadFromEllipse(si32 &start_ix,
			si32 &start_iy,
			si32 &end_ix,
			si32 &end_iy,
			ui08 **storm_grid,
			double **precip_grid);
  
  void _setRunsInGrid(si32 &start_ix,
		      si32 &start_iy,
		      si32 &end_ix,
		      si32 &end_iy,
		      ui08 **target_grid);
  
  void _setEllipseInGrid(double ellipse_x,
			 double ellipse_y,
			 double major_radius,
			 double minor_radius,
			 double axis_rotation,
			 si32 &start_ix,
			 si32 &start_iy,
			 si32 &end_ix,
			 si32 &end_iy,
			 ui08 **target_grid);

  void _setRunsPrecipFromFlux(ui08 **storm_grid,
			      double **precip_grid);
  
  void _setRunsPrecipFromHist(ui08 **storm_grid,
			      double **precip_grid);
  
  void _setEllipsePrecipFromFlux(si32 start_ix,
				 si32 start_iy,
				 si32 end_ix,
				 si32 end_iy,
				 ui08 **storm_grid,
				 double **precip_grid);

  void _setEllipsePrecipFromHist(double ellipse_x,
				 double ellipse_y,
				 double major_radius,
				 double minor_radius,
				 double axis_rotation,
				 double **precip_grid);

  void _initMdvx(DsMdvx &mdvx,
		 time_t start_time,
		 time_t centroid_time,
		 time_t end_time);

  void _setFieldName(Mdvx::field_header_t &fhdr,
		     const char *name,
		     const char *name_long,
		     const char *units);

  void _computeDensities();

};

#endif
