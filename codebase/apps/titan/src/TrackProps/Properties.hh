/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/////////////////////////////////////////////////////////////
// Properties.h
//
// Track properties object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// January 1998
//
//////////////////////////////////////////////////////////

#ifndef Properties_h
#define Properties_h

#include "TrackProps.hh"
#include "Params.hh"
#include <toolsa/membuf.h>

// struct for time series properties

typedef struct {

  // times

  time_t scan_time;     // unix time
  double scan_duration;    // hrs
  double time_since_start; // hrs

  int n_parts;

  // vertical measures

  double ht_of_dbz_max;
  double refl_centroid_z;
  double top;
  double base;

  // max reflectivity

  double dbz_max;

  // size

  double volume;
  double volume_above_alt; // volume above given height threshold

  double mass;
  double mass_above_alt;   // mass above given height threshold

  double area;

  // precip flux

  double precip_flux;

  // location

  double x;
  double y;

  // movement

  double u;
  double v;

  // reflectivity fit

  double area_dbz_fit;

  // ellipse shape

  double ellipse_ratio;
  double ellipse_orient;

} time_series_props_t;

class Properties {
  
public:

  // constructor
       
  Properties (const char *prog_name,
	      const Params &params,
	      storm_file_handle_t *s_handle,
	      track_file_handle_t *t_handle);
  
  // compute properties
  
  int compute(int complex_track_num);

  // print properties
  
  void print(FILE *out);

  // destructor
  
  ~Properties();
  
  int OK;

protected:
  
private:

  char *_progName;
  int _debug;
  const Params &_params;

  MEMbuf *_tSeriesBuf;
  time_series_props_t *_tSeriesProps;

  int _complexTrackNum;

  int _nScans, _startScan, _endScan;
  time_t _startTime, _endTime;
  double _duration;

  double _startX, _startY;

  double _dbzThresh;

  double _ati;
  double _precipMass;

  // gaussian fit to area data
  double _gaussD;     // duration measure
  double _gaussAmean; // mean area

  time_series_props_t _mean;
  time_series_props_t _min;
  time_series_props_t _max;
  time_series_props_t _sdev;

  double _speed;
  double _dirn;
  double _serialCorrU;
  double _serialCorrV;

  storm_file_handle_t *_s_handle;
  track_file_handle_t *_t_handle;

  int _load_tseries_props();

  void _compute_z_props(storm_file_global_props_t *gprops,
			double *volume_above_alt_p,
			double *mass_above_alt_p,
			double *base_p,
			double *top_p);
  
  void _compute_tseries_stats(double *tseries_val,
			      double *mean_p,
			      double *min_p,
			      double *max_p,
			      double *sdev_p);

  void _compute_tseries_integrals();

  void _compute_area_fit();
  
  void _compute_serial_corr(double *tseries_val,
			    double *scorr_p);
  
  void _compute_elliptical_dbz_fit(storm_file_params_t *sparams,
				   storm_file_global_props_t *gprops,
				   storm_file_dbz_hist_t *hist,
				   double *dbz_fit_p);
  
};

#endif
