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
// Properties.h
//
// Track properties object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1997
//
//////////////////////////////////////////////////////////

#ifndef Properties_h
#define Properties_h

#include "Cases.hh"

// struct for time series properties

typedef struct {

  // times

  time_t scan_time;   // unix time
  int delta_time;     // secs
  int scan_duration;  // secs

  int n_parts;

  // vertical measures

  double ht_of_dbz_max;
  double refl_centroid_z;
  double ht_max_minus_centroid_z;  // (ht_of_dbz_max - refl_centroid_z)
  double top;
  double base;

  // max reflectivity

  double dbz_max;

  // volume, mass and area

  double volume;
  double volume_above_alt; // volume above given height threshold

  double mass;
  double mass_above_alt;   // mass above given height threshold

  double area;

  // precip flux

  double precip_flux;

  // integral quantities

  double ati;          // area time integral (km2.hr)
  double precip_mass;  // (ktons)
  double vcdi;         // vertical centroid difference integral (km.hr)

} time_series_props_t;

class Properties {
  
public:

  // constructor
       
  Properties (char *prog_name,
	      int debug,
	      SeedCaseTracks::CaseTrack *this_case,
	      double altitude_threshold,
	      char *output_dir,
	      storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle,
	      rf_partial_track_t *ptrack);
  
  // compute properties
  
  int compute();

  // print properties
  
  void print();

  // destructor
  
  ~Properties();
  
  int isOK;

protected:
  
private:

  int _debug;

  char *_progName;
  char *_outputDir;
  char *_caseOutputPath;
  char *_tSeriesOutputPath;

  int _nScans, _startScan, _endScan;
  time_t _startTime, _endTime;

  int _durationBeforeDecision;
  int _durationAfterDecision;

  double _altitudeThreshold;

  time_series_props_t _mean;
  time_series_props_t _max;
  time_series_props_t _maxRoi;  // max rate of increase

  time_series_props_t *_tSeriesProps;

  FILE *_caseOutFile;
  FILE *_tSeriesOutFile;
  
  SeedCaseTracks::CaseTrack _case;

  storm_file_handle_t *_s_handle;
  track_file_handle_t *_t_handle;
  rf_partial_track_t *_pTrack;

  int _load_scan_limits();

  int _load_tseries_props();

  void _compute_z_props(storm_file_global_props_t *gprops,
			double *volume_above_alt_p,
			double *mass_above_alt_p,
			double *base_p,
			double *top_p);
  
  void _compute_tseries_stats(double *tseries_val,
			      time_t *scan_time,
			      double *mean_p,
			      double *max_p,
			      double *max_roi_p);

  void _compute_tseries_integrals();
  
};

#endif
