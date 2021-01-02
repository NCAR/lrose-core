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
// DoVerify.hh
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2005
//
///////////////////////////////////////////////////////////////
//
// Verify against TITAN tracks themselves
//
////////////////////////////////////////////////////////////////

#ifndef DoVerify_HH
#define DoVerify_HH

#include <string>
#include <vector>
#include <toolsa/toolsa_macros.h>
#include "Params.hh"
#include "vt_structs.hh"
using namespace std;

#define KM_PER_DEG_AT_EQUATOR 111.12

#define CONSTRAIN_VAL(x, low, high) if ((x) < (low)) (x) = (low); \
                                else if ((x) > (high)) (x) = (high)

#define SMALL_ANGLE 0.000001
#define PI_BY_TWO 1.570796327
#define ALMOST_PI_BY_TWO (PI_BY_TWO - SMALL_ANGLE)
#ifndef PI
#define PI 3.141592654
#endif
#define ALMOST_PI (PI - SMALL_ANGLE)

class DoVerify {
  
public:

  // constructor

  DoVerify (const string &prog_name,
            const Params &params);

  // destructor
  
  ~DoVerify();

  // run 

  int Run();

  // data members

  bool isOK;

  // process a track file

  int processFile(const char *trackFilePath);

  // compute total stats

  void computeTotalStats() { compute_stats(&_totalStats); }

  // get methods

  const vt_count_t *getTotalCount() const { return &_totalCount; }
  const vt_stats_t *getTotalStats() const { return &_totalStats; }

  int getNx() const { return _nx; }
  int getNy() const { return _ny; }
  double getMinx() const { return _minx; }
  double getMiny() const { return _miny; }
  double getDx() const { return _dx; }
  double getDy() const { return _dy; }

protected:
  
private:

  const string &_progName;
  const Params &_params;
  
  storm_file_handle_t _sHandle;
  track_file_handle_t _tHandle;
  
  int _gridType;
  vector<date_time_t> _scanTimes; 

  vt_count_t _totalCount;
  vt_stats_t _totalStats;

  int _nx, _ny;
  double _minx, _miny;
  double _dx, _dy;
  
  int _nbytesGrid;
  ui08 **_forecastGrid;
  ui08 **_truthGrid;
  ui08 *_visited;
  simple_track_params_t _lastParams;

  int _openFiles(const char *trackFilePath);

  int _loadScanTimes();

  int _performVerification();

  int _findLastDescendant(int level);

  void _computeLookup(titan_grid_t *grid,
                      long *x_lookup,
                      long *y_lookup);

  bool _loadProps(const track_file_entry_t *entry,
                  const storm_file_global_props_t *gprops,
                  double lead_time,
                  track_file_forecast_props_t *f_props);

  void load_forecast_grid(track_file_entry_t *entry,
                          storm_file_global_props_t *gprops,
                          double lead_time,
                          ui08 **grid);

  void load_truth_grid(vt_storm_t *storm, ui08 **grid);

  void load_ellipse_forecast_grid(storm_file_params_t *sparams,
                                  track_file_params_t *tparams,
                                  storm_file_scan_header_t *scan,
                                  storm_file_global_props_t *gprops,
                                  track_file_entry_t *entry,
                                  double lead_time,
                                  ui08 **grid);

  void load_ellipse_truth_grid(storm_file_params_t *sparams,
                               storm_file_scan_header_t *scan,
                               storm_file_global_props_t *gprops,
                               ui08 **grid);

  void load_ellipse_grid(double ellipse_x,
                         double ellipse_y,
                         double major_radius,
                         double minor_radius,
                         double axis_rotation,
                         ui08 **grid);
  
  void load_polygon_forecast_grid(storm_file_params_t *sparams,
                                  track_file_params_t *tparams,
                                  storm_file_scan_header_t *scan,
                                  storm_file_global_props_t *gprops,
                                  track_file_entry_t *entry,
                                  double lead_time,
                                  ui08 **grid);
  
  void load_polygon_truth_grid(storm_file_params_t *sparams,
                               storm_file_scan_header_t *scan,
                               storm_file_global_props_t *gprops,
                               ui08 **grid);
  
  void load_runs_truth_grid(vt_storm_t *storm,
                            ui08 **grid);
  
  int point_in_polygon(double centroid_x,
                       double centroid_y,
                       double *radials,
                       double start_az,
                       double delta_az,
                       long n_sides,
                       double grid_dx,
                       double grid_dy,
                       double search_x,
                       double search_y);

  void compute_contingency_data(ui08 **forecast_grid,
                                ui08 **truth_grid,
                                long nbytes_grid,
                                vt_count_t *count);
  
  void compute_best_fit(ui08 **forecast_grid,
                        ui08 **truth_grid,
                        vt_count_t *count);
  
  void compute_as_is(ui08 **forecast_grid,
                     ui08 **truth_grid,
                     long nbytes_grid,
                     vt_count_t *count);
  
  void increment_count(vt_count_t *general,
                       vt_count_t *specific);

  void debug_print(vt_simple_track_t *stracks,
                   long ncurrent,
                   const vector<vt_entry_index_t> &current,
                   ui08 **forecast_grid,
                   ui08 **runs_truth_grid,
                   vt_count_t *count);

  void compute_errors(long nverify,
                      track_file_forecast_props_t *props_current,
                      track_file_forecast_props_t *props_forecast,
                      track_file_forecast_props_t *props_verify,
                      vt_stats_t *complex_stats,
                      vt_stats_t *file_stats,
                      vt_stats_t *total_stats);
  
  void compute_spd_and_dirn(track_file_forecast_props_t *current,
                            track_file_forecast_props_t *future,
                            double x_scale_factor,
                            double y_scale_factor);
    
  double compute_dist_error(track_file_forecast_props_t *forecast,
                            track_file_forecast_props_t *verify,
                            double x_scale_factor,
                            double y_scale_factor);
  
  void load_errors(fl32 *forecast_ptr,
                   track_file_forecast_props_t *PropsForecast,
                   track_file_forecast_props_t *PropsVerify,
                   vt_stats_t *ComplexStats,
                   vt_stats_t *FileStats,
                   vt_stats_t *TotalStats,
                   double scale_factor,
                   int vol_weighted,
                   int dirn_prop);
		      
  void set_contingency(vt_count_t *count,
                       track_file_contingency_data_t *cont);
   
  void compute_stats(vt_stats_t *stats);

  void load_stats(vt_stats_t *stats, fl32 *prop_p, double n);

  void load_file_stats(track_file_params_t *tparams,
                       vt_stats_t *stats,
                       si32 *n_samples,
                       track_file_forecast_props_t *bias,
                       track_file_forecast_props_t *rmse);
  
  int _performMdvVerification();

  int _readMdvTruthFile(const storm_file_scan_header_t *scan);

  void _saveVerificationParameters();

};

#endif
