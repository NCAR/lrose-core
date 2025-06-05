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
////////////////////////////////////////////////////////////////////
// <titan/TitanNcFile.hh>
//
// TITAN C++ NetCDF file io
//
// Mike Dixon, EOL, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// May 2025
//
////////////////////////////////////////////////////////////////////

#ifndef TitanNcFile_HH
#define TitanNcFile_HH

#include <titan/storm.h>
#include <titan/track.h>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>
#include <string>

using namespace std;

class TitanNcFile
{

private:
  
  ////////////////////////////////////////////////////////
  // inner classes for netcdf variables

  // top level

  class TopLevelVars
  {
  public:
    NcxxVar file_time;
    NcxxVar start_time;
    NcxxVar end_time;
    NcxxVar n_scans;
  };

  // scan vars

  class ScanVars
  {
  public:
    // scan details
    NcxxVar scan_min_z;
    NcxxVar scan_delta_z;
    NcxxVar scan_num;
    NcxxVar scan_nstorms;
    NcxxVar scan_time;
    NcxxVar scan_gprops_offset;
    NcxxVar scan_first_offset;
    NcxxVar scan_last_offset;
    NcxxVar scan_ht_of_freezing;
    // grid details
    NcxxVar grid_nx;
    NcxxVar grid_ny;
    NcxxVar grid_nz;
    NcxxVar grid_minx;
    NcxxVar grid_miny;
    NcxxVar grid_minz;
    NcxxVar grid_dx;
    NcxxVar grid_dy;
    NcxxVar grid_dz;
    NcxxVar grid_dz_constant;
    NcxxVar grid_sensor_x;
    NcxxVar grid_sensor_y;
    NcxxVar grid_sensor_z;
    NcxxVar grid_sensor_lat;
    NcxxVar grid_sensor_lon;
    NcxxVar grid_unitsx;
    NcxxVar grid_unitsy;
    NcxxVar grid_unitsz;
    // projection details
    NcxxVar proj_type;
    NcxxVar proj_origin_lat;
    NcxxVar proj_origin_lon;
    NcxxVar proj_lat1;
    NcxxVar proj_lat2;
    NcxxVar proj_tangent_lat;
    NcxxVar proj_tangent_lon;
    NcxxVar proj_pole_type;
    NcxxVar proj_central_scale;
  };

  // storm identification parameter vars

  class StormParamsVars
  {
  public:
    NcxxVar low_dbz_threshold;
    NcxxVar high_dbz_threshold;
    NcxxVar dbz_hist_interval;
    NcxxVar hail_dbz_threshold;
    NcxxVar base_threshold;
    NcxxVar top_threshold;
    NcxxVar min_storm_size;
    NcxxVar max_storm_size;
    NcxxVar morphology_erosion_threshold;
    NcxxVar morphology_refl_divisor;
    NcxxVar min_radar_tops;
    NcxxVar tops_edge_margin;
    NcxxVar z_p_coeff;
    NcxxVar z_p_exponent;
    NcxxVar z_m_coeff;
    NcxxVar z_m_exponent;
    NcxxVar sectrip_vert_aspect;
    NcxxVar sectrip_horiz_aspect;
    NcxxVar sectrip_orientation_error;
    NcxxVar poly_start_az;
    NcxxVar poly_delta_az;
    NcxxVar check_morphology;
    NcxxVar check_tops;
    NcxxVar vel_available;
    NcxxVar n_poly_sides;
    NcxxVar ltg_count_time;
    NcxxVar ltg_count_margin_km;
    NcxxVar hail_z_m_coeff;
    NcxxVar hail_z_m_exponent;
    NcxxVar hail_mass_dbz_threshold;
    NcxxVar gprops_union_type;
    NcxxVar tops_dbz_threshold;
    NcxxVar precip_computation_mode;
    NcxxVar precip_plane_ht;
    NcxxVar low_convectivity_threshold;
    NcxxVar high_convectivity_threshold;
  };

  // storm global properties vars
  
  class StormGpropsVars
  {
  public:
    NcxxVar vol_centroid_x;
    NcxxVar vol_centroid_y;
    NcxxVar vol_centroid_z;
    NcxxVar refl_centroid_x;
    NcxxVar refl_centroid_y;
    NcxxVar refl_centroid_z;
    NcxxVar top;
    NcxxVar base;
    NcxxVar volume;
    NcxxVar area_mean;
    NcxxVar precip_flux;
    NcxxVar mass;
    NcxxVar tilt_angle;
    NcxxVar tilt_dirn;
    NcxxVar dbz_max;
    NcxxVar dbz_mean;
    NcxxVar dbz_max_gradient;
    NcxxVar dbz_mean_gradient;
    NcxxVar ht_of_dbz_max;
    NcxxVar rad_vel_mean;
    NcxxVar rad_vel_sd;
    NcxxVar vorticity;
    NcxxVar precip_area;
    NcxxVar precip_area_centroid_x;
    NcxxVar precip_area_centroid_y;
    NcxxVar precip_area_orientation;
    NcxxVar precip_area_minor_radius;
    NcxxVar precip_area_major_radius;
    NcxxVar proj_area;
    NcxxVar proj_area_centroid_x;
    NcxxVar proj_area_centroid_y;
    NcxxVar proj_area_orientation;
    NcxxVar proj_area_minor_radius;
    NcxxVar proj_area_major_radius;
    NcxxVar proj_area_polygon;
    NcxxVar storm_num;
    NcxxVar n_layers;
    NcxxVar base_layer;
    NcxxVar n_dbz_intervals;
    NcxxVar n_runs;
    NcxxVar n_proj_runs;
    NcxxVar top_missing;
    NcxxVar range_limited;
    NcxxVar second_trip;
    NcxxVar hail_present;
    NcxxVar anom_prop;
    NcxxVar bounding_min_ix;
    NcxxVar bounding_min_iy;
    NcxxVar bounding_max_ix;
    NcxxVar bounding_max_iy;
    NcxxVar layer_props_offset;
    NcxxVar dbz_hist_offset;
    NcxxVar runs_offset;
    NcxxVar proj_runs_offset;
    NcxxVar vil_from_maxz;
    NcxxVar ltg_count;
    NcxxVar convectivity_median;
    NcxxVar hail_FOKRcategory;
    NcxxVar hail_waldvogelProbability;
    NcxxVar hail_hailMassAloft;
    NcxxVar hail_vihm;
    NcxxVar hail_poh;
    NcxxVar hail_shi;
    NcxxVar hail_posh;
    NcxxVar hail_mehs;
  };

  // storm layer properties vars
  
  class StormLpropsVars
  {
  public:
    NcxxVar vol_centroid_x;
    NcxxVar vol_centroid_y;
    NcxxVar refl_centroid_x;
    NcxxVar refl_centroid_y;
    NcxxVar area;
    NcxxVar dbz_max;
    NcxxVar dbz_mean;
    NcxxVar mass;
    NcxxVar rad_vel_mean;
    NcxxVar rad_vel_sd;
    NcxxVar vorticity;
    NcxxVar convectivity_median;
  };

  // storm dbz histograms vars
  
  class StormHistVars
  {
  public:
    NcxxVar percent_volume;
    NcxxVar percent_area;
  };
  
  // storm runs vars
  
  class StormRunsVars
  {
  public:
    NcxxVar ix;
    NcxxVar iy;
    NcxxVar iz;
    NcxxVar n;
  };
  
  // tracking state vars

  class TrackingStateVars
  {
  public:
    NcxxVar tracking_valid;
    NcxxVar tracking_modify_code;
    NcxxVar n_simple_tracks;
    NcxxVar n_complex_tracks;
    NcxxVar tracking_n_samples_for_forecast_stats;
    NcxxVar tracking_last_scan_num;
    NcxxVar max_simple_track_num;
    NcxxVar max_complex_track_num;
    NcxxVar tracking_max_parents;
    NcxxVar tracking_max_children;
    NcxxVar tracking_max_nweights_forecast;
  };

  // tracking parameter vars

  class TrackingParamsVars
  {
  public:
    NcxxVar tracking_forecast_weights;
    NcxxVar tracking_weight_distance;
    NcxxVar tracking_weight_delta_cube_root_volume;
    NcxxVar tracking_merge_split_search_ratio;
    NcxxVar tracking_max_speed;
    NcxxVar tracking_max_speed_for_valid_forecast;
    NcxxVar tracking_parabolic_growth_period;
    NcxxVar tracking_smoothing_radius;
    NcxxVar tracking_min_fraction_overlap;
    NcxxVar tracking_min_sum_fraction_overlap;
    NcxxVar tracking_scale_forecasts_by_history;
    NcxxVar tracking_use_runs_for_overlaps;
    NcxxVar tracking_grid_type;
    NcxxVar tracking_nweights_forecast;
    NcxxVar tracking_forecast_type;
    NcxxVar tracking_max_delta_time;
    NcxxVar tracking_min_history_for_valid_forecast;
    NcxxVar tracking_spatial_smoothing;
  };
  
  // verification vars

  class ContingencyVars
  {
  public:
    NcxxVar n_success;
    NcxxVar n_failure;
    NcxxVar n_false_alarm;
  };

  // simple track vars

  class SimpleTrackVars
  {
  public:
    NcxxVar simple_track_num;
    NcxxVar last_descendant_simple_track_num;
    NcxxVar simple_start_scan;
    NcxxVar simple_end_scan;
    NcxxVar simple_last_descendant_end_scan;
    NcxxVar simple_scan_origin;
    NcxxVar simple_start_time;
    NcxxVar simple_end_time;
    NcxxVar simple_last_descendant_end_time;
    NcxxVar simple_time_origin;
    NcxxVar simple_history_in_scans;
    NcxxVar simple_history_in_secs;
    NcxxVar simple_duration_in_scans;
    NcxxVar simple_duration_in_secs;
    NcxxVar simple_nparents;
    NcxxVar simple_nchildren;
    NcxxVar simple_parent;
    NcxxVar simple_child;
    NcxxVar complex_track_num_for_simple;
    NcxxVar simple_first_entry_offset;
  };
  
  // complex track vars

  class ComplexTrackVars
  {
  public:
    NcxxVar volume_at_start_of_sampling;
    NcxxVar volume_at_end_of_sampling;
    NcxxVar complex_track_num;
    NcxxVar complex_start_scan;
    NcxxVar complex_end_scan;
    NcxxVar complex_duration_in_scans;
    NcxxVar complex_duration_in_secs;
    NcxxVar complex_start_time;
    NcxxVar complex_end_time;
    NcxxVar complex_n_simple_tracks;
    NcxxVar complex_n_top_missing;
    NcxxVar complex_n_range_limited;
    NcxxVar complex_start_missing;
    NcxxVar complex_end_missing;
    NcxxVar complex_n_samples_for_forecast_stats;
  };
  
  // track forecast properties
  
  class ForecastPropsVars
  {
  public:
    NcxxVar forecast_proj_area_centroid_x;
    NcxxVar forecast_proj_area_centroid_y;
    NcxxVar forecast_vol_centroid_z;
    NcxxVar forecast_refl_centroid_z;
    NcxxVar forecast_top;
    NcxxVar forecast_dbz_max;
    NcxxVar forecast_volume;
    NcxxVar forecast_precip_flux;
    NcxxVar forecast_mass;
    NcxxVar forecast_proj_area;
    NcxxVar forecast_smoothed_proj_area_centroid_x;
    NcxxVar forecast_smoothed_proj_area_centroid_y;
    NcxxVar forecast_smoothed_speed;
    NcxxVar forecast_smoothed_direction;
  };

  // track verification
  
  class TrackVerifyVars
  {
  public:
    NcxxVar verification_performed;
    NcxxVar verify_forecast_lead_time;
    NcxxVar verify_end_time;
    NcxxVar verify_forecast_lead_time_margin;
    NcxxVar verify_forecast_min_history;
    NcxxVar verify_before_forecast_time;
    NcxxVar verify_after_track_dies;
  };

public:
  
  // constructor
  
  TitanNcFile();
  
  // destructor
  
  virtual ~TitanNcFile();

  // storm data access

  const storm_file_header_t &storm_header() const { return _storm_header; }
  const storm_file_params_t &storm_params() const { return _storm_header.params; }
  const storm_file_scan_header_t &scan() const { return _scan; }
  const storm_file_global_props_t *gprops() const { return _gprops; }
  const storm_file_layer_props_t *lprops() const { return _lprops; }
  const storm_file_dbz_hist_t *hist() const { return _hist; }
  const storm_file_run_t *runs() const { return _runs; }
  const storm_file_run_t *proj_runs() const { return _proj_runs; }
  const int *scan_offsets() const { return _scan_offsets; }
  int storm_num() const { return _storm_num; }
  
  const string &storm_header_file_path() { return _storm_header_file_path; }
  const string &storm_header_file_label() { return _storm_header_file_label; }
  const string &storm_data_file_path() { return _storm_data_file_path; }
  const string &storm_data_file_label() { return _storm_data_file_label; }

  // track data access

  const track_file_header_t &track_header() const { return _track_header; }
  const track_file_params_t &track_params() const { return _track_header.params; }
  const simple_track_params_t &simple_params() const;
  const complex_track_params_t &complex_params() const;
  const track_file_entry_t &entry() const { return _entry; }
  const track_file_entry_t *scan_entries() const { return _scan_entries; }
  const track_file_scan_index_t *scan_index() const { return _scan_index; }
  const track_utime_t *track_utime() const { return _track_utime; }
  int n_scan_entries() { return _n_scan_entries; }
  
  const string &track_header_file_path() { return _track_header_file_path; }
  const string &track_header_file_label() { return _track_header_file_label; }
  const string &track_data_file_path() { return _track_data_file_path; }
  const string &track_data_file_label() { return _track_data_file_label; }

  const si32 *complex_track_nums() { return _complex_track_nums; }
  const si32 *complex_track_offsets() { return _complex_track_offsets; }
  const si32 *simple_track_offsets() { return _simple_track_offsets; }
  const si32 *nsimples_per_complex() { return _nsimples_per_complex; }
  const si32 *simples_per_complex_offsets() { return _simples_per_complex_offsets; }
  si32 **simples_per_complex() { return _simples_per_complex; }

  // public functions

  // memory allocation and freeing - storms

  void AllocLayers(int n_layers);
  void FreeLayers();
  void AllocHist(int n_dbz_intervals);
  void FreeHist();
  void AllocRuns(int n_runs);
  void FreeRuns();
  void AllocProjRuns(int n_proj_runs);
  void FreeProjRuns();
  void AllocGprops(int nstorms);
  void FreeGprops();
  void AllocScanOffsets(int n_scans_needed);
  void FreeScanOffsets();
  void FreeStormsAll();
    
  // memory allocation and freeing - tracks

  void AllocSimpleArrays(int n_simple_needed);
  void FreeSimpleArrays();
  void AllocComplexArrays(int n_complex_needed);
  void FreeComplexArrays();
  void AllocSimplesPerComplex(int n_simple_needed);
  void FreeSimplesPerComplex();
  void AllocScanEntries(int n_entries_needed);
  void FreeScanEntries();
  void AllocScanIndex(int n_scans_needed);
  void FreeScanIndex();
  void AllocUtime();
  void FreeUtime();
  void FreeTracksAll();

  /////////////////////////////////////////////////////
  // NetCDF FileIO
  
  int openNcFile(const string &path,
                 NcxxFile::FileMode mode);
  
  void closeNcFile();

  /////////////////////////////////////////////////////
  // set up groups

  void _setUpGroups();
  
  // get group relative to a parent group
  
  NcxxGroup _getGroup(const std::string& name,
                      NcxxGroup &parent);
  
  /////////////////////////////////////////////////////
  // set up dimensions

  void _setUpDims();
  
  // get dim relative to a parent group
  
  NcxxDim _getDim(const std::string& name,
                  NcxxGroup &parent);
  
  NcxxDim _getDim(const std::string& name,
                  size_t dimSize,
                  NcxxGroup &parent);
  
  /////////////////////////////////////////////////////
  // set up variables

  void _setUpVars();
  
  // get scalar variable
  
  NcxxVar _getVar(const std::string& name,
                  const NcxxType& ncType,
                  NcxxGroup &group);
  
  // get array variable
  
  NcxxVar _getVar(const std::string& name,
                  const NcxxType& ncType,
                  const NcxxDim& dim,
                  NcxxGroup &group);
  
  /////////////////////////////////////////////////////
  // Storms
  
  // Open the storm header and data files
  
  int OpenStormFiles(const char *mode,
                     const char *header_file_path,
                     const char *data_file_ext = NULL);
  
  // Close the storm header and data files

  void CloseStormFiles();
  
  // Flush the storm header and data files
  
  void FlushStormFiles();
  
  // Put an advisory lock on the header file
  // Mode is "w" - write lock, or "r" - read lock.
  // returns 0 on success, -1 on failure
  
  int LockStormHeaderFile(const char *mode);
  
  // Remove advisory lock from the header file
  // returns 0 on success, -1 on failure

  int UnlockStormHeaderFile();
  
  // read the storm file header

  int ReadStormHeader(bool clear_error_str = true);
     
  // read in the storm projected area runs
  // Space for the array is allocated.
  // returns 0 on success, -1 on failure

  int ReadProjRuns(int storm_num);
     
  // Read in the scan info and global props for a particular scan
  // in a storm properties file.
  // If storm num is set, only the gprops for that storm is swapped
  // returns 0 on success, -1 on failure

  int ReadScan(int scan_num, int storm_num = -1);
  
  // read in the seconday storm property data (lprops, hist, runs)
  // for a given storm in a scan.
  // Space for the arrays of structures is allocated as required.
  // returns 0 on success, -1 on failure

  int ReadProps(int storm_num);
     
  // seek to the end of the storm data in data file
  // returns 0 on success, -1 on failure

  int SeekStormEndData();

  // seek to the start of the storm data in data file
  // returns 0 on success, -1 on failure

  int SeekStormStartData();
  
  // write the storm_file_header_t structure to a storm file.
  // returns 0 on success, -1 on failure
  
  int WriteStormHeader(const storm_file_header_t &storm_file_header);
     
  // write the storm layer property and histogram data for a storm,
  // at the end of the file.
  // returns 0 on success, -1 on failure

  int WriteProps(int storm_num);

  // write scan header and global properties for a particular scan
  // in a storm properties file.
  // Performs the writes from the end of the file.
  // returns 0 on success, -1 on failure
  
  int WriteScan(int scan_num);
     
  // Convert the ellipse data (orientation, major_radius and minor_radius)
  // for a a gprops struct to local (km) values.
  // This applies to structs which were derived from lat-lon grids, for
  // which some of the fields are in deg instead of km.
  // It is a no-op for other projections.
  //
  // See Note 3 in storms.h

  void GpropsEllipses2Km(const storm_file_scan_header_t &scan,
			 storm_file_global_props_t &gprops);
  
     
  // Convert the (x,y) km locations in a gprops struct to lat-lon.
  // This applies to structs which were computed for non-latlon 
  // grids. It is a no-op for lat-lon grids.
  //
  // See Note 3 in storms.h
  
  void GpropsXY2LatLon(const storm_file_scan_header_t &scan,
		       storm_file_global_props_t &gprops);
  
  // Truncate header file
  // Returns 0 on success, -1 on failure.

  int TruncateStormHeaderFile(int length);

  // Truncate data file
  // Returns 0 on success, -1 on failure.

  int TruncateStormDataFile(int length);

  /////////////////////////////////////////////////////
  // Tracks
  
  // Open the track header and data files
  // Returns 0 on success, -1 on error

  int OpenTrackFiles(const char *mode,
                     const char *header_file_path,
                     const char *data_file_ext = NULL);
  
  // Close the storm header and data files

  void CloseTrackFiles();
     
  // Flush the storm header and data files

  void FlushTrackFiles();
  
  // Put an advisory lock on the header file.
  // Mode is "w" - write lock, or "r" - read lock.
  // returns 0 on success, -1 on failure

  int LockTrackHeaderFile(const char *mode);

  // Remove advisory lock from the header file
  // returns 0 on success, -1 on failure

  int UnlockTrackHeaderFile();
  
  // read in the track_file_header_t structure from a track file.
  // Read in associated arrays (complex_track_nums, complex_track_offsets,
  //   simple_track_offsets, scan_index, nsimples_per_complex,
  //   simples_per_complex_offsets)
  // returns 0 on success, -1 on failure

  int ReadTrackHeader(bool clear_error_str = true);
     
  // Read in the track_file_header_t and scan_index array.
  // returns 0 on success, -1 on failure

  int ReadScanIndex(bool clear_error_str = true);
     
  // reads in the parameters for a complex track
  // For normal reads, read_simples_per_complex should be set true. This
  // is only set FALSE in Titan, which creates the track files.
  // returns 0 on success, -1 on failure
  
  int ReadComplexParams(int track_num, bool read_simples_per_complex,
			bool clear_error_str = true);
     
  // read in the parameters for a simple track
  // returns 0 on success, -1 on failure

  int ReadSimpleParams(int track_num,
		       bool clear_error_str = true);
     
  // read in an entry for a track
  // If first_entry is set to TRUE, then the first entry is read in. If not
  // the next entry is read in.
  // returns 0 on success, -1 on failure
  
  int ReadEntry();
  
  // read in the array of simple tracks for each complex track
  // returns 0 on success, -1 on failure
  
  int ReadSimplesPerComplex();
  
  // read in entries for a scan
  // returns 0 on success, -1 on failure

  int ReadScanEntries(int scan_num);
     
  // read in track_utime_t array
  // Returns 0 on success or -1 on error

  int ReadUtime();
  
  // Reinitialize headers and arrays

  void Reinit();

  // Set a complex params slot in the file available for
  // reuse, by setting the offset to its negative value.
  // returns 0 on success, -1 on failure

  int ReuseComplexSlot(int track_num);
     
  // prepare a simple track for reading by reading in the simple track
  // params and setting the first_entry flag
  // returns 0 on success, -1 on failure

  int RewindSimple(int track_num);
     
  // rewrite an entry for a track in the track data file
  // The entry is written at the file offset of the original entry
  // returns 0 on success, -1 on failure
  
  int RewriteEntry();
     
  // seek to the end of the track file data

  int SeekTrackEndData();

  // seek to the start of data in track data file

  int SeekTrackStartData();
     
  // write the track_file_header_t structure to a track data file
  // returns 0 on success, -1 on failure

  int WriteTrackHeader();

  // write simple track params at the end of the file
  // returns 0 on success, -1 on failure
  
  int WriteSimpleParams(int track_num);
     
  // write complex track params
  // returns 0 on success, -1 on failure
  
  int WriteComplexParams(int track_num);
     
  // write an entry for a track in the track data file
  // The entry is written at the end of the file
  // returns offset of last entry written on success, -1 on failure
  
  long WriteEntry(int prev_in_track_offset,
		  int prev_in_scan_offset);
     
  ///////////////////////////////////////////////////////////////////
  // error string
  
  const string &getErrStr() { return (_errStr); }

  /// add integer value to error string, with label
  
  void _addErrInt(string label, int iarg,
                  bool cr = true);
  
  /// add double value to error string, with label
  
  void _addErrDbl(string label, double darg,
                  string format = "%g", bool cr = true);

  /// add string value to error string, with label
  
  void _addErrStr(string label, string strarg = "",
                  bool cr = true);

protected:

  // format version

  string _convention;
  string _version;
  
  // netcdf file
  
  NcxxFile _ncFile;
  string _tmpPath;

  ////////////////////////////////////////////////////////
  // dimensions
  
  NcxxDim _n_scans;
  NcxxDim _n_storms;
  NcxxDim _n_simple;
  NcxxDim _n_complex;
  NcxxDim _n_entries;
  NcxxDim _n_poly;

  ////////////////////////////////////////////////////////
  // groups
  
  // top level groups
  
  NcxxGroup _scansGroup;
  NcxxGroup _stormsGroup;
  NcxxGroup _tracksGroup;
  
  // storms group
  
  NcxxGroup _gpropsGroup;
  NcxxGroup _lpropsGroup;
  NcxxGroup _histGroup;
  NcxxGroup _runsGroup;
  NcxxGroup _projRunsGroup;

  // tracks group
  
  NcxxGroup _simpleGroup;
  NcxxGroup _complexGroup;
  NcxxGroup _entriesGroup;

  //////////////////////////////////
  // netcdf variables - grouped
  
  TopLevelVars _topLevelVars;
  ScanVars _scanVars;

  // storm identification
  
  StormParamsVars _sparamsVars;
  StormGpropsVars _gpropsVars;
  StormLpropsVars _lpropsVars;
  StormHistVars _histVars;
  StormRunsVars _runsVars;
  StormRunsVars _projRunsVars;

  // tracking
  
  TrackingStateVars _tstateVars;
  TrackingParamsVars _tparamsVars;
  SimpleTrackVars _simpleVars;
  ComplexTrackVars _complexVars;
  
  // verification - global

  ContingencyVars _ellipseVerifyGlobalVars;
  ContingencyVars _polygonVerifyGlobalVars;

  ForecastPropsVars _forecastPropsGlobalVars;
  ForecastPropsVars _forecastBiasGlobalVars;
  ForecastPropsVars _forecastRmseGlobalVars;

  TrackVerifyVars _trackVerifyVars;
  
  // verification - per complex track

  ContingencyVars _ellipseVerifyComplexVars;
  ContingencyVars _polygonVerifyComplexVars;
  
  // storm file details
  
  string _storm_header_file_path;
  string _storm_header_file_label;
  string _storm_data_file_path;
  string _storm_data_file_label;

  FILE *_storm_header_file;
  FILE *_storm_data_file;

  // storm data

  storm_file_header_t _storm_header;
  storm_file_scan_header_t _scan;
  storm_file_global_props_t *_gprops;
  storm_file_layer_props_t *_lprops;
  storm_file_dbz_hist_t *_hist;
  storm_file_run_t *_runs;
  storm_file_run_t *_proj_runs;
  si32 *_scan_offsets;
  int _storm_num;

  // storm memory allocation

  int _max_scans;
  int _max_storms;
  int _max_layers;
  int _max_dbz_intervals;
  int _max_runs;
  int _max_proj_runs;

  // track file
  
  string _track_header_file_path;
  string _track_header_file_label;
  string _track_data_file_path;
  string _track_data_file_label;
  
  FILE *_track_header_file;
  FILE *_track_data_file;

  bool _first_entry;  // set to TRUE if first entry of a track
  
  // track data

  track_file_header_t _track_header;
  simple_track_params_t _simple_params;
  complex_track_params_t _complex_params;
  track_file_entry_t _entry;

  track_file_scan_index_t *_scan_index;
  track_file_entry_t *_scan_entries;
  track_utime_t *_track_utime;
  
  si32 *_complex_track_nums;
  si32 *_complex_track_offsets;
  si32 *_simple_track_offsets;
  si32 *_nsimples_per_complex;
  si32 *_simples_per_complex_offsets;
  si32 **_simples_per_complex;
  int _n_scan_entries;
  
  // track memory allocation control

  int _n_simple_allocated;
  int _n_complex_allocated;
  int _n_simples_per_complex_allocated;
  int _n_scan_entries_allocated;
  int _n_scan_index_allocated;
  int _n_utime_allocated;
  int _lowest_avail_complex_slot;

  // errors
  
  string _errStr;
  
  // functions
  
  void _clearErrStr();
  
  void _convert_ellipse_2km(const titan_grid_t &tgrid,
			    double centroid_x,
			    double centroid_y,
			    fl32 &orientation,
			    fl32 &minor_radius,
			    fl32 &major_radius);

  int _truncateStormFiles(FILE *&fd, const string &path, int length);

public:

  // friends for Titan program which writes the storm and track files
  
  friend class StormFile;
  friend class Area;
  friend class Props;
  friend class Identify;
  friend class StormIdent;

private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  TitanNcFile(const TitanNcFile & orig);
  TitanNcFile & operator = (const TitanNcFile & other);

public:

  // strings for netcdf
  
  // groups

  static constexpr const char* SCANS = "scans";
  static constexpr const char* STORMS = "storms";
  static constexpr const char* TRACKS = "tracks";
  static constexpr const char* GPROPS = "gprops";
  static constexpr const char* LPROPS = "lprops";
  static constexpr const char* HIST = "hist";
  static constexpr const char* RUNS = "runs";
  static constexpr const char* PROJ_RUNS = "proj_runs";
  static constexpr const char* SIMPLE = "simple";
  static constexpr const char* COMPLEX = "complex";
  static constexpr const char* ENTRIES = "entries";

  // top-level dimensions attributes

  static constexpr const char* FILE_TIME = "file_time";
  static constexpr const char* START_TIME = "start_time";
  static constexpr const char* END_TIME = "end_time";
  static constexpr const char* N_SCANS = "n_scans";
  static constexpr const char* N_STORMS = "n_storms";
  static constexpr const char* N_SIMPLE = "n_simple";
  static constexpr const char* N_COMPLEX = "n_complex";
  static constexpr const char* N_ENTRIES = "n_entries";
  static constexpr const char* N_POLY = "n_poly";
  
  // storm identification parameters

  static constexpr const char* LOW_DBZ_THRESHOLD = "low_dbz_threshold";
  static constexpr const char* HIGH_DBZ_THRESHOLD = "high_dbz_threshold";
  static constexpr const char* DBZ_HIST_INTERVAL = "dbz_hist_interval";
  static constexpr const char* HAIL_DBZ_THRESHOLD = "hail_dbz_threshold";
  static constexpr const char* BASE_THRESHOLD = "base_threshold";
  static constexpr const char* TOP_THRESHOLD = "top_threshold";
  static constexpr const char* MIN_STORM_SIZE = "min_storm_size";
  static constexpr const char* MAX_STORM_SIZE = "max_storm_size";
  static constexpr const char* MORPHOLOGY_EROSION_THRESHOLD = "morphology_erosion_threshold";
  static constexpr const char* MORPHOLOGY_REFL_DIVISOR = "morphology_refl_divisor";
  static constexpr const char* MIN_RADAR_TOPS = "min_radar_tops";
  static constexpr const char* TOPS_EDGE_MARGIN = "tops_edge_margin";
  static constexpr const char* Z_P_COEFF = "z_p_coeff";
  static constexpr const char* Z_P_EXPONENT = "z_p_exponent";
  static constexpr const char* Z_M_COEFF = "z_m_coeff";
  static constexpr const char* Z_M_EXPONENT = "z_m_exponent";
  static constexpr const char* SECTRIP_VERT_ASPECT = "sectrip_vert_aspect";
  static constexpr const char* SECTRIP_HORIZ_ASPECT = "sectrip_horiz_aspect";
  static constexpr const char* SECTRIP_ORIENTATION_ERROR = "sectrip_orientation_error";
  static constexpr const char* POLY_START_AZ = "poly_start_az";
  static constexpr const char* POLY_DELTA_AZ = "poly_delta_az";
  static constexpr const char* CHECK_MORPHOLOGY = "check_morphology";
  static constexpr const char* CHECK_TOPS = "check_tops";
  static constexpr const char* VEL_AVAILABLE = "vel_available";
  static constexpr const char* N_POLY_SIDES_ = "n_poly_sides";
  static constexpr const char* LTG_COUNT_TIME = "ltg_count_time";
  static constexpr const char* LTG_COUNT_MARGIN_KM = "ltg_count_margin_km";
  static constexpr const char* HAIL_Z_M_COEFF = "hail_z_m_coeff";
  static constexpr const char* HAIL_Z_M_EXPONENT = "hail_z_m_exponent";
  static constexpr const char* HAIL_MASS_DBZ_THRESHOLD = "hail_mass_dbz_threshold";
  static constexpr const char* GPROPS_UNION_TYPE = "gprops_union_type";
  static constexpr const char* TOPS_DBZ_THRESHOLD = "tops_dbz_threshold";
  static constexpr const char* PRECIP_COMPUTATION_MODE = "precip_computation_mode";
  static constexpr const char* PRECIP_PLANE_HT = "precip_plane_ht";
  static constexpr const char* LOW_CONVECTIVITY_THRESHOLD = "low_convectivity_threshold";
  static constexpr const char* HIGH_CONVECTIVITY_THRESHOLD = "high_convectivity_threshold";

  // scan details

  static constexpr const char* SCAN_MIN_Z = "scan_min_z";
  static constexpr const char* SCAN_DELTA_Z = "scan_delta_z";
  static constexpr const char* SCAN_NUM = "scan_num";
  static constexpr const char* SCAN_NSTORMS = "scan_nstorms";
  static constexpr const char* SCAN_TIME = "scan_time";
  static constexpr const char* SCAN_GPROPS_OFFSET = "scan_gprops_offset";
  static constexpr const char* SCAN_FIRST_OFFSET = "scan_first_offset";
  static constexpr const char* SCAN_LAST_OFFSET = "scan_last_offset";
  static constexpr const char* SCAN_HT_OF_FREEZING = "scan_ht_of_freezing";
  
  // grid and projection details
  
  static constexpr const char* GRID_NX = "grid_nx";
  static constexpr const char* GRID_NY = "grid_ny";
  static constexpr const char* GRID_NZ = "grid_nz";
  static constexpr const char* GRID_MINX = "grid_minx";
  static constexpr const char* GRID_MINY = "grid_miny";
  static constexpr const char* GRID_MINZ = "grid_minz";
  static constexpr const char* GRID_DX = "grid_dx";
  static constexpr const char* GRID_DY = "grid_dy";
  static constexpr const char* GRID_DZ = "grid_dz";
  static constexpr const char* GRID_DZ_CONSTANT = "grid_dz_constant";
  static constexpr const char* GRID_SENSOR_X = "grid_sensor_x";
  static constexpr const char* GRID_SENSOR_Y = "grid_sensor_y";
  static constexpr const char* GRID_SENSOR_Z = "grid_sensor_z";
  static constexpr const char* GRID_SENSOR_LAT = "grid_sensor_lat";
  static constexpr const char* GRID_SENSOR_LON = "grid_sensor_lon";
  static constexpr const char* GRID_UNITSX = "grid_unitsx";
  static constexpr const char* GRID_UNITSY = "grid_unitsy";
  static constexpr const char* GRID_UNITSZ = "grid_unitsz";

  static constexpr const char* PROJ_TYPE = "proj_type";
  static constexpr const char* PROJ_ORIGIN_LAT = "proj_origin_lat";
  static constexpr const char* PROJ_ORIGIN_LON = "proj_origin_lon";
  static constexpr const char* PROJ_LAT1 = "proj_lat1";
  static constexpr const char* PROJ_LAT2 = "proj_lat2";
  static constexpr const char* PROJ_TANGENT_LAT = "proj_tangent_lat";
  static constexpr const char* PROJ_TANGENT_LON = "proj_tangent_lon";
  static constexpr const char* PROJ_POLE_TYPE = "proj_pole_type";
  static constexpr const char* PROJ_CENTRAL_SCALE = "proj_central_scale";

  // storm global properties
  
  static constexpr const char* VOL_CENTROID_X = "vol_centroid_x";
  static constexpr const char* VOL_CENTROID_Y = "vol_centroid_y";
  static constexpr const char* VOL_CENTROID_Z = "vol_centroid_z";
  static constexpr const char* REFL_CENTROID_X = "refl_centroid_x";
  static constexpr const char* REFL_CENTROID_Y = "refl_centroid_y";
  static constexpr const char* REFL_CENTROID_Z = "refl_centroid_z";
  static constexpr const char* TOP = "top";
  static constexpr const char* BASE = "base";
  static constexpr const char* VOLUME = "volume";
  static constexpr const char* AREA_MEAN = "area_mean";
  static constexpr const char* PRECIP_FLUX = "precip_flux";
  static constexpr const char* MASS = "mass";
  static constexpr const char* TILT_ANGLE = "tilt_angle";
  static constexpr const char* TILT_DIRN = "tilt_dirn";
  static constexpr const char* DBZ_MAX = "dbz_max";
  static constexpr const char* DBZ_MEAN = "dbz_mean";
  static constexpr const char* DBZ_MAX_GRADIENT = "dbz_max_gradient";
  static constexpr const char* DBZ_MEAN_GRADIENT = "dbz_mean_gradient";
  static constexpr const char* HT_OF_DBZ_MAX = "ht_of_dbz_max";
  static constexpr const char* RAD_VEL_MEAN = "rad_vel_mean";
  static constexpr const char* RAD_VEL_SD = "rad_vel_sd";
  static constexpr const char* VORTICITY = "vorticity";
  static constexpr const char* PRECIP_AREA = "precip_area";
  static constexpr const char* PRECIP_AREA_CENTROID_X = "precip_area_centroid_x";
  static constexpr const char* PRECIP_AREA_CENTROID_Y = "precip_area_centroid_y";
  static constexpr const char* PRECIP_AREA_ORIENTATION = "precip_area_orientation";
  static constexpr const char* PRECIP_AREA_MINOR_RADIUS = "precip_area_minor_radius";
  static constexpr const char* PRECIP_AREA_MAJOR_RADIUS = "precip_area_major_radius";
  static constexpr const char* PROJ_AREA = "proj_area";
  static constexpr const char* PROJ_AREA_CENTROID_X = "proj_area_centroid_x";
  static constexpr const char* PROJ_AREA_CENTROID_Y = "proj_area_centroid_y";
  static constexpr const char* PROJ_AREA_ORIENTATION = "proj_area_orientation";
  static constexpr const char* PROJ_AREA_MINOR_RADIUS = "proj_area_minor_radius";
  static constexpr const char* PROJ_AREA_MAJOR_RADIUS = "proj_area_major_radius";
  static constexpr const char* PROJ_AREA_POLYGON = "proj_area_polygon";
  static constexpr const char* STORM_NUM = "storm_num";
  static constexpr const char* N_LAYERS = "n_layers";
  static constexpr const char* BASE_LAYER = "base_layer";
  static constexpr const char* N_DBZ_INTERVALS = "n_dbz_intervals";
  static constexpr const char* N_RUNS = "n_runs";
  static constexpr const char* N_PROJ_RUNS = "n_proj_runs";
  static constexpr const char* TOP_MISSING = "top_missing";
  static constexpr const char* RANGE_LIMITED = "range_limited";
  static constexpr const char* SECOND_TRIP = "second_trip";
  static constexpr const char* HAIL_PRESENT = "hail_present";
  static constexpr const char* ANOM_PROP = "anom_prop";
  static constexpr const char* BOUNDING_MIN_IX = "bounding_min_ix";
  static constexpr const char* BOUNDING_MIN_IY = "bounding_min_iy";
  static constexpr const char* BOUNDING_MAX_IX = "bounding_max_ix";
  static constexpr const char* BOUNDING_MAX_IY = "bounding_max_iy";
  static constexpr const char* LAYER_PROPS_OFFSET = "layer_props_offset";
  static constexpr const char* DBZ_HIST_OFFSET = "dbz_hist_offset";
  static constexpr const char* RUNS_OFFSET = "runs_offset";
  static constexpr const char* PROJ_RUNS_OFFSET = "proj_runs_offset";
  static constexpr const char* VIL_FROM_MAXZ = "vil_from_maxz";
  static constexpr const char* LTG_COUNT = "ltg_count";
  static constexpr const char* CONVECTIVITY_MEDIAN = "convectivity_median";
  static constexpr const char* HAIL_FOKRCATEGORY = "hail_FOKRcategory";
  static constexpr const char* HAIL_WALDVOGELPROBABILITY = "hail_waldvogelProbability";
  static constexpr const char* HAIL_HAILMASSALOFT = "hail_hailMassAloft";
  static constexpr const char* HAIL_VIHM = "hail_vihm";
  static constexpr const char* HAIL_POH = "hail_poh";
  static constexpr const char* HAIL_SHI = "hail_shi";
  static constexpr const char* HAIL_POSH = "hail_posh";
  static constexpr const char* HAIL_MEHS = "hail_mehs";

  // storm layer properties
  
  static constexpr const char* LAYER_VOL_CENTROID_X = "layer_vol_centroid_x";
  static constexpr const char* LAYER_VOL_CENTROID_Y = "layer_vol_centroid_y";
  static constexpr const char* LAYER_REFL_CENTROID_X = "layer_refl_centroid_x";
  static constexpr const char* LAYER_REFL_CENTROID_Y = "layer_refl_centroid_y";
  static constexpr const char* LAYER_AREA = "layer_area";
  static constexpr const char* LAYER_DBZ_MAX = "layer_dbz_max";
  static constexpr const char* LAYER_DBZ_MEAN = "layer_dbz_mean";
  static constexpr const char* LAYER_MASS = "layer_mass";
  static constexpr const char* LAYER_RAD_VEL_MEAN = "layer_rad_vel_mean";
  static constexpr const char* LAYER_RAD_VEL_SD = "layer_rad_vel_sd";
  static constexpr const char* LAYER_VORTICITY = "layer_vorticity";
  static constexpr const char* LAYER_CONVECTIVITY_MEDIAN = "layer_convectivity_median";

  // storm dbz histogram

  static constexpr const char* HIST_PERCENT_VOLUME = "hist_percent_volume";
  static constexpr const char* HIST_PERCENT_AREA = "hist_percent_area";

  // storm runs
  
  static constexpr const char* RUN_IX = "run_ix";
  static constexpr const char* RUN_IY = "run_iy";
  static constexpr const char* RUN_IZ = "run_iz";
  static constexpr const char* RUN_N = "run_n";

  // tracking parameters

  static constexpr const char* TRACKING_VALID = "tracking_valid";
  static constexpr const char* TRACKING_MODIFY_CODE = "tracking_modify_code";
  
  static constexpr const char* TRACKING_FORECAST_WEIGHTS = "tracking_forecast_weights";  
  static constexpr const char* TRACKING_WEIGHT_DISTANCE = "tracking_weight_distance";
  static constexpr const char* TRACKING_WEIGHT_DELTA_CUBE_ROOT_VOLUME = "tracking_weight_delta_cube_root_volume";
  static constexpr const char* TRACKING_MERGE_SPLIT_SEARCH_RATIO = "tracking_merge_split_search_ratio";
  static constexpr const char* TRACKING_MAX_SPEED = "tracking_max_speed";
  static constexpr const char* TRACKING_MAX_SPEED_FOR_VALID_FORECAST = "tracking_max_speed_for_valid_forecast";
  static constexpr const char* TRACKING_PARABOLIC_GROWTH_PERIOD = "tracking_parabolic_growth_period";
  static constexpr const char* TRACKING_SMOOTHING_RADIUS = "tracking_smoothing_radius";
  static constexpr const char* TRACKING_MIN_FRACTION_OVERLAP = "tracking_min_fraction_overlap";
  static constexpr const char* TRACKING_MIN_SUM_FRACTION_OVERLAP = "tracking_min_sum_fraction_overlap";
  static constexpr const char* TRACKING_SCALE_FORECASTS_BY_HISTORY = "tracking_scale_forecasts_by_history";
  static constexpr const char* TRACKING_USE_RUNS_FOR_OVERLAPS = "tracking_use_runs_for_overlaps";
  static constexpr const char* TRACKING_GRID_TYPE = "tracking_grid_type";
  static constexpr const char* TRACKING_NWEIGHTS_FORECAST = "tracking_nweights_forecast";
  static constexpr const char* TRACKING_FORECAST_TYPE = "tracking_forecast_type";
  static constexpr const char* TRACKING_MAX_DELTA_TIME = "tracking_max_delta_time";
  static constexpr const char* TRACKING_MIN_HISTORY_FOR_VALID_FORECAST = "tracking_min_history_for_valid_forecast";
  static constexpr const char* TRACKING_SPATIAL_SMOOTHING = "tracking_spatial_smoothing";

  static constexpr const char* N_SIMPLE_TRACKS = "n_simple_tracks";
  static constexpr const char* N_COMPLEX_TRACKS = "n_complex_tracks";

  static constexpr const char* TRACKING_N_SAMPLES_FOR_FORECAST_STATS = "tracking_n_samples_for_forecast_stats";
  static constexpr const char* TRACKING_LAST_SCAN_NUM = "tracking_last_scan_num";
  static constexpr const char* MAX_SIMPLE_TRACK_NUM = "max_simple_track_num";
  static constexpr const char* MAX_COMPLEX_TRACK_NUM = "max_complex_track_num";
  static constexpr const char* TRACKING_MAX_PARENTS = "tracking_max_parents";
  static constexpr const char* TRACKING_MAX_CHILDREN = "tracking_max_children";
  static constexpr const char* TRACKING_MAX_NWEIGHTS_FORECAST = "tracking_max_nweights_forecast";

  // verification contingency stats
  
  static constexpr const char* ELLIPSE_VERIFY_N_SUCCESS = "ellipse_verify_n_success";
  static constexpr const char* ELLIPSE_VERIFY_N_FAILURE = "ellipse_verify_n_failure";
  static constexpr const char* ELLIPSE_VERIFY_N_FALSE_ALARM = "ellipse_verify_n_false_alarm";
  static constexpr const char* POLYGON_VERIFY_N_SUCCESS = "polygon_verify_n_success";
  static constexpr const char* POLYGON_VERIFY_N_FAILURE = "polygon_verify_n_failure";
  static constexpr const char* POLYGON_VERIFY_N_FALSE_ALARM = "polygon_verify_n_false_alarm";

  // track forecast properties
  
  static constexpr const char* FORECAST_PROJ_AREA_CENTROID_X = "forecast_proj_area_centroid_x";
  static constexpr const char* FORECAST_PROJ_AREA_CENTROID_Y = "forecast_proj_area_centroid_y";
  static constexpr const char* FORECAST_VOL_CENTROID_Z = "forecast_vol_centroid_z";
  static constexpr const char* FORECAST_REFL_CENTROID_Z = "forecast_refl_centroid_z";
  static constexpr const char* FORECAST_TOP = "forecast_top";
  static constexpr const char* FORECAST_DBZ_MAX = "forecast_dbz_max";
  static constexpr const char* FORECAST_VOLUME = "forecast_volume";
  static constexpr const char* FORECAST_PRECIP_FLUX = "forecast_precip_flux";
  static constexpr const char* FORECAST_MASS = "forecast_mass";
  static constexpr const char* FORECAST_PROJ_AREA = "forecast_proj_area";
  static constexpr const char* FORECAST_SMOOTHED_PROJ_AREA_CENTROID_X = "forecast_smoothed_proj_area_centroid_x";
  static constexpr const char* FORECAST_SMOOTHED_PROJ_AREA_CENTROID_Y = "forecast_smoothed_proj_area_centroid_y";
  static constexpr const char* FORECAST_SMOOTHED_SPEED = "forecast_smoothed_speed";
  static constexpr const char* FORECAST_SMOOTHED_DIRECTION = "forecast_smoothed_direction";

  // simple tracks

  static constexpr const char* SIMPLE_TRACK_NUM = "simple_track_num";
  static constexpr const char* LAST_DESCENDANT_SIMPLE_TRACK_NUM = "last_descendant_simple_track_num";
  static constexpr const char* SIMPLE_START_SCAN = "simple_start_scan";
  static constexpr const char* SIMPLE_END_SCAN = "simple_end_scan";
  static constexpr const char* SIMPLE_LAST_DESCENDANT_END_SCAN = "simple_last_descendant_end_scan";
  static constexpr const char* SIMPLE_SCAN_ORIGIN = "simple_scan_origin";
  static constexpr const char* SIMPLE_START_TIME = "simple_start_time";
  static constexpr const char* SIMPLE_END_TIME = "simple_end_time";
  static constexpr const char* SIMPLE_LAST_DESCENDANT_END_TIME = "simple_last_descendant_end_time";
  static constexpr const char* SIMPLE_TIME_ORIGIN = "simple_time_origin";
  static constexpr const char* SIMPLE_HISTORY_IN_SCANS = "simple_history_in_scans";
  static constexpr const char* SIMPLE_HISTORY_IN_SECS = "simple_history_in_secs";
  static constexpr const char* SIMPLE_DURATION_IN_SCANS = "simple_duration_in_scans";
  static constexpr const char* SIMPLE_DURATION_IN_SECS = "simple_duration_in_secs";
  static constexpr const char* SIMPLE_NPARENTS = "simple_nparents";
  static constexpr const char* SIMPLE_NCHILDREN = "simple_nchildren";
  static constexpr const char* SIMPLE_PARENT = "simple_parent";
  static constexpr const char* SIMPLE_CHILD = "simple_child";
  static constexpr const char* COMPLEX_TRACK_NUM_FOR_SIMPLE = "complex_track_num_for_simple";
  static constexpr const char* SIMPLE_FIRST_ENTRY_OFFSET = "simple_first_entry_offset";
  
  // complex tracks

  static constexpr const char* VOLUME_AT_START_OF_SAMPLING = "volume_at_start_of_sampling";
  static constexpr const char* VOLUME_AT_END_OF_SAMPLING = "volume_at_end_of_sampling";
  static constexpr const char* COMPLEX_TRACK_NUM = "complex_track_num";
  static constexpr const char* COMPLEX_START_SCAN = "complex_start_scan";
  static constexpr const char* COMPLEX_END_SCAN = "complex_end_scan";
  static constexpr const char* COMPLEX_DURATION_IN_SCANS = "complex_duration_in_scans";
  static constexpr const char* COMPLEX_DURATION_IN_SECS = "complex_duration_in_secs";
  static constexpr const char* COMPLEX_START_TIME = "complex_start_time";
  static constexpr const char* COMPLEX_END_TIME = "complex_end_time";
  static constexpr const char* COMPLEX_N_SIMPLE_TRACKS = "complex_n_simple_tracks";
  static constexpr const char* COMPLEX_N_TOP_MISSING = "complex_n_top_missing";
  static constexpr const char* COMPLEX_N_RANGE_LIMITED = "complex_n_range_limited";
  static constexpr const char* COMPLEX_START_MISSING = "complex_start_missing";
  static constexpr const char* COMPLEX_END_MISSING = "complex_end_missing";
  static constexpr const char* COMPLEX_N_SAMPLES_FOR_FORECAST_STATS = "complex_n_samples_for_forecast_stats";

  // track entry

  static constexpr const char* ENTRY_TIME = "entry_time";
  static constexpr const char* ENTRY_TIME_ORIGIN = "entry_time_origin";
  static constexpr const char* ENTRY_SCAN_ORIGIN = "entry_scan_origin";
  static constexpr const char* ENTRY_SCAN_NUM = "entry_scan_num";
  static constexpr const char* ENTRY_STORM_NUM = "entry_storm_num";
  static constexpr const char* ENTRY_SIMPLE_TRACK_NUM = "entry_simple_track_num";
  static constexpr const char* ENTRY_COMPLEX_TRACK_NUM = "entry_complex_track_num";
  static constexpr const char* ENTRY_HISTORY_IN_SCANS = "entry_history_in_scans";
  static constexpr const char* ENTRY_HISTORY_IN_SECS = "entry_history_in_secs";
  static constexpr const char* ENTRY_DURATION_IN_SCANS = "entry_duration_in_scans";
  static constexpr const char* ENTRY_DURATION_IN_SECS = "entry_duration_in_secs";
  static constexpr const char* ENTRY_FORECAST_VALID = "entry_forecast_valid";
  static constexpr const char* ENTRY_PREV_ENTRY_OFFSET = "entry_prev_entry_offset";
  static constexpr const char* ENTRY_THIS_ENTRY_OFFSET = "entry_this_entry_offset";
  static constexpr const char* ENTRY_NEXT_ENTRY_OFFSET = "entry_next_entry_offset";
  static constexpr const char* ENTRY_NEXT_SCAN_ENTRY_OFFSET = "entry_next_scan_entry_offset";

  // entry dval_dt
  
  static constexpr const char* ENTRY_DVAL_DT_PROJ_AREA_CENTROID_X = "entry_dval_dt_proj_area_centroid_x";
  static constexpr const char* ENTRY_DVAL_DT_PROJ_AREA_CENTROID_Y = "entry_dval_dt_proj_area_centroid_y";
  static constexpr const char* ENTRY_DVAL_DT_VOL_CENTROID_Z = "entry_dval_dt_vol_centroid_z";
  static constexpr const char* ENTRY_DVAL_DT_REFL_CENTROID_Z = "entry_dval_dt_refl_centroid_z";
  static constexpr const char* ENTRY_DVAL_DT_TOP = "entry_dval_dt_top";
  static constexpr const char* ENTRY_DVAL_DT_DBZ_MAX = "entry_dval_dt_dbz_max";
  static constexpr const char* ENTRY_DVAL_DT_VOLUME = "entry_dval_dt_volume";
  static constexpr const char* ENTRY_DVAL_DT_PRECIP_FLUX = "entry_dval_dt_precip_flux";
  static constexpr const char* ENTRY_DVAL_DT_MASS = "entry_dval_dt_mass";
  static constexpr const char* ENTRY_DVAL_DT_PROJ_AREA = "entry_dval_dt_proj_area";
  static constexpr const char* ENTRY_DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_X = "entry_dval_dt_smoothed_proj_area_centroid_x";
  static constexpr const char* ENTRY_DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_Y = "entry_dval_dt_smoothed_proj_area_centroid_y";
  static constexpr const char* ENTRY_DVAL_DT_SMOOTHED_SPEED = "entry_dval_dt_smoothed_speed";
  static constexpr const char* ENTRY_DVAL_DT_SMOOTHED_DIRECTION = "entry_dval_dt_smoothed_direction";

  // forecast bias
  
  static constexpr const char* FORECAST_BIAS_PROJ_AREA_CENTROID_X = "forecast_bias_proj_area_centroid_x";
  static constexpr const char* FORECAST_BIAS_PROJ_AREA_CENTROID_Y = "forecast_bias_proj_area_centroid_y";
  static constexpr const char* FORECAST_BIAS_VOL_CENTROID_Z = "forecast_bias_vol_centroid_z";
  static constexpr const char* FORECAST_BIAS_REFL_CENTROID_Z = "forecast_bias_refl_centroid_z";
  static constexpr const char* FORECAST_BIAS_TOP = "forecast_bias_top";
  static constexpr const char* FORECAST_BIAS_DBZ_MAX = "forecast_bias_dbz_max";
  static constexpr const char* FORECAST_BIAS_VOLUME = "forecast_bias_volume";
  static constexpr const char* FORECAST_BIAS_PRECIP_FLUX = "forecast_bias_precip_flux";
  static constexpr const char* FORECAST_BIAS_MASS = "forecast_bias_mass";
  static constexpr const char* FORECAST_BIAS_PROJ_AREA = "forecast_bias_proj_area";
  static constexpr const char* FORECAST_BIAS_SMOOTHED_PROJ_AREA_CENTROID_X = "forecast_bias_smoothed_proj_area_centroid_x";
  static constexpr const char* FORECAST_BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y = "forecast_bias_smoothed_proj_area_centroid_y";
  static constexpr const char* FORECAST_BIAS_SMOOTHED_SPEED = "forecast_bias_smoothed_speed";
  static constexpr const char* FORECAST_BIAS_SMOOTHED_DIRECTION = "forecast_bias_smoothed_direction";

  // forecast rmse
  
  static constexpr const char* FORECAST_RMSE_PROJ_AREA_CENTROID_X = "forecast_rmse_proj_area_centroid_x";
  static constexpr const char* FORECAST_RMSE_PROJ_AREA_CENTROID_Y = "forecast_rmse_proj_area_centroid_y";
  static constexpr const char* FORECAST_RMSE_VOL_CENTROID_Z = "forecast_rmse_vol_centroid_z";
  static constexpr const char* FORECAST_RMSE_REFL_CENTROID_Z = "forecast_rmse_refl_centroid_z";
  static constexpr const char* FORECAST_RMSE_TOP = "forecast_rmse_top";
  static constexpr const char* FORECAST_RMSE_DBZ_MAX = "forecast_rmse_dbz_max";
  static constexpr const char* FORECAST_RMSE_VOLUME = "forecast_rmse_volume";
  static constexpr const char* FORECAST_RMSE_PRECIP_FLUX = "forecast_rmse_precip_flux";
  static constexpr const char* FORECAST_RMSE_MASS = "forecast_rmse_mass";
  static constexpr const char* FORECAST_RMSE_PROJ_AREA = "forecast_rmse_proj_area";
  static constexpr const char* FORECAST_RMSE_SMOOTHED_PROJ_AREA_CENTROID_X = "forecast_rmse_smoothed_proj_area_centroid_x";
  static constexpr const char* FORECAST_RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y = "forecast_rmse_smoothed_proj_area_centroid_y";
  static constexpr const char* FORECAST_RMSE_SMOOTHED_SPEED = "forecast_rmse_smoothed_speed";
  static constexpr const char* FORECAST_RMSE_SMOOTHED_DIRECTION = "forecast_rmse_smoothed_direction";

  // track verification
  
  static constexpr const char* VERIFICATION_PERFORMED = "verification_performed";
  static constexpr const char* VERIFY_FORECAST_LEAD_TIME = "verify_forecast_lead_time";
  static constexpr const char* VERIFY_END_TIME = "verify_end_time";
  static constexpr const char* VERIFY_FORECAST_LEAD_TIME_MARGIN = "verify_forecast_lead_time_margin";
  static constexpr const char* VERIFY_FORECAST_MIN_HISTORY = "verify_forecast_min_history";
  static constexpr const char* VERIFY_BEFORE_FORECAST_TIME = "verify_before_forecast_time";
  static constexpr const char* VERIFY_AFTER_TRACK_DIES = "verify_after_track_dies";

  // verification contingency stats
  
  static constexpr const char* COMPLEX_ELLIPSE_VERIFY_N_SUCCESS = "complex_ellipse_verify_n_success";
  static constexpr const char* COMPLEX_ELLIPSE_VERIFY_N_FAILURE = "complex_ellipse_verify_n_failure";
  static constexpr const char* COMPLEX_ELLIPSE_VERIFY_N_FALSE_ALARM = "complex_ellipse_verify_n_false_alarm";
  static constexpr const char* COMPLEX_POLYGON_VERIFY_N_SUCCESS = "complex_polygon_verify_n_success";
  static constexpr const char* COMPLEX_POLYGON_VERIFY_N_FAILURE = "complex_polygon_verify_n_failure";
  static constexpr const char* COMPLEX_POLYGON_VERIFY_N_FALSE_ALARM = "complex_polygon_verify_n_false_alarm";

  // forecast bias
  
  static constexpr const char* COMPLEX_FORECAST_BIAS_PROJ_AREA_CENTROID_X = "complex_forecast_bias_proj_area_centroid_x";
  static constexpr const char* COMPLEX_FORECAST_BIAS_PROJ_AREA_CENTROID_Y = "complex_forecast_bias_proj_area_centroid_y";
  static constexpr const char* COMPLEX_FORECAST_BIAS_VOL_CENTROID_Z = "complex_forecast_bias_vol_centroid_z";
  static constexpr const char* COMPLEX_FORECAST_BIAS_REFL_CENTROID_Z = "complex_forecast_bias_refl_centroid_z";
  static constexpr const char* COMPLEX_FORECAST_BIAS_TOP = "complex_forecast_bias_top";
  static constexpr const char* COMPLEX_FORECAST_BIAS_DBZ_MAX = "complex_forecast_bias_dbz_max";
  static constexpr const char* COMPLEX_FORECAST_BIAS_VOLUME = "complex_forecast_bias_volume";
  static constexpr const char* COMPLEX_FORECAST_BIAS_PRECIP_FLUX = "complex_forecast_bias_precip_flux";
  static constexpr const char* COMPLEX_FORECAST_BIAS_MASS = "complex_forecast_bias_mass";
  static constexpr const char* COMPLEX_FORECAST_BIAS_PROJ_AREA = "complex_forecast_bias_proj_area";
  static constexpr const char* COMPLEX_FORECAST_BIAS_SMOOTHED_PROJ_AREA_CENTROID_X = "complex_forecast_bias_smoothed_proj_area_centroid_x";
  static constexpr const char* COMPLEX_FORECAST_BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y = "complex_forecast_bias_smoothed_proj_area_centroid_y";
  static constexpr const char* COMPLEX_FORECAST_BIAS_SMOOTHED_SPEED = "complex_forecast_bias_smoothed_speed";
  static constexpr const char* COMPLEX_FORECAST_BIAS_SMOOTHED_DIRECTION = "complex_forecast_bias_smoothed_direction";

  // forecast rmse
  
  static constexpr const char* COMPLEX_FORECAST_RMSE_PROJ_AREA_CENTROID_X = "complex_forecast_rmse_proj_area_centroid_x";
  static constexpr const char* COMPLEX_FORECAST_RMSE_PROJ_AREA_CENTROID_Y = "complex_forecast_rmse_proj_area_centroid_y";
  static constexpr const char* COMPLEX_FORECAST_RMSE_VOL_CENTROID_Z = "complex_forecast_rmse_vol_centroid_z";
  static constexpr const char* COMPLEX_FORECAST_RMSE_REFL_CENTROID_Z = "complex_forecast_rmse_refl_centroid_z";
  static constexpr const char* COMPLEX_FORECAST_RMSE_TOP = "complex_forecast_rmse_top";
  static constexpr const char* COMPLEX_FORECAST_RMSE_DBZ_MAX = "complex_forecast_rmse_dbz_max";
  static constexpr const char* COMPLEX_FORECAST_RMSE_VOLUME = "complex_forecast_rmse_volume";
  static constexpr const char* COMPLEX_FORECAST_RMSE_PRECIP_FLUX = "complex_forecast_rmse_precip_flux";
  static constexpr const char* COMPLEX_FORECAST_RMSE_MASS = "complex_forecast_rmse_mass";
  static constexpr const char* COMPLEX_FORECAST_RMSE_PROJ_AREA = "complex_forecast_rmse_proj_area";
  static constexpr const char* COMPLEX_FORECAST_RMSE_SMOOTHED_PROJ_AREA_CENTROID_X = "complex_forecast_rmse_smoothed_proj_area_centroid_x";
  static constexpr const char* COMPLEX_FORECAST_RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y = "complex_forecast_rmse_smoothed_proj_area_centroid_y";
  static constexpr const char* COMPLEX_FORECAST_RMSE_SMOOTHED_SPEED = "complex_forecast_rmse_smoothed_speed";
  static constexpr const char* COMPLEX_FORECAST_RMSE_SMOOTHED_DIRECTION = "complex_forecast_rmse_smoothed_direction";
  
};

#endif
