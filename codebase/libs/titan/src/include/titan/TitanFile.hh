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
// <titan/TitanFile.hh>
//
// TITAN C++ NetCDF file io
//
// Mike Dixon, EOL, NCAR
// POBox 3000, Boulder, CO, 80305-3000, USA
//
// May 2025
//
////////////////////////////////////////////////////////////////////

#ifndef TitanFile_HH
#define TitanFile_HH

#include <string>
#include <cstdint>

#include <titan/TitanData.hh>
#include <titan/TitanStormFile.hh>
#include <titan/TitanTrackFile.hh>
#include <titan/track.h>
#include <Mdv/MdvxProj.hh>
#include <Ncxx/Ncxx.hh>
#include <Ncxx/NcxxFile.hh>

using namespace std;

class TitanFile
{

private:
  
  ////////////////////////////////////////////////////////
  // inner classes to logically group netcdf variables

  // top level

  class TopLevelVars
  {
  public:
    NcxxVar file_time;
    NcxxVar start_time;
    NcxxVar end_time;
    NcxxVar n_scans;
    NcxxVar sum_storms;
    NcxxVar sum_layers;
    NcxxVar sum_hist;
    NcxxVar sum_runs;
    NcxxVar sum_proj_runs;
    NcxxVar max_simple_track_num;
    NcxxVar max_complex_track_num;
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
    // indices for first data for each scan
    NcxxVar scan_gprops_offset_0;
    NcxxVar scan_layer_offset_0;
    NcxxVar scan_hist_offset_0;
    NcxxVar scan_runs_offset_0;
    NcxxVar scan_proj_runs_offset_0;
  };

  class GridVars
  {
  public:
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
    NcxxVar proj_rotation;
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
    NcxxVar run_ix;
    NcxxVar run_iy;
    NcxxVar run_iz;
    NcxxVar run_len;
  };
  
  // tracking parameter vars

  class TrackingParamsVars
  {
  public:
    NcxxVar forecast_weights;
    NcxxVar weight_distance;
    NcxxVar weight_delta_cube_root_volume;
    NcxxVar merge_split_search_ratio;
    NcxxVar max_tracking_speed;
    NcxxVar max_speed_for_valid_forecast;
    NcxxVar parabolic_growth_period;
    NcxxVar smoothing_radius;
    NcxxVar min_fraction_overlap;
    NcxxVar min_sum_fraction_overlap;
    NcxxVar scale_forecasts_by_history;
    NcxxVar use_runs_for_overlaps;
    NcxxVar grid_type;
    NcxxVar nweights_forecast;
    NcxxVar forecast_type;
    NcxxVar max_delta_time;
    NcxxVar min_history_for_valid_forecast;
    NcxxVar spatial_smoothing;
  };
  
  // tracking state vars

  class TrackingStateVars
  {
  public:
    NcxxVar n_simple_tracks;
    NcxxVar n_complex_tracks;
    NcxxVar tracking_valid;
    NcxxVar tracking_modify_code;
    NcxxVar n_samples_for_forecast_stats;
    NcxxVar last_scan_num;
    NcxxVar max_simple_track_num;
    NcxxVar max_complex_track_num;
    NcxxVar max_parents;
    NcxxVar max_children;
    NcxxVar max_nweights_forecast;
  };

  // simple track vars

  class SimpleTrackVars
  {
  public:

    NcxxVar simple_track_num;
    NcxxVar complex_track_num;
    NcxxVar last_descendant_simple_track_num;
    NcxxVar start_scan;
    NcxxVar end_scan;
    NcxxVar last_descendant_end_scan;
    NcxxVar scan_origin;
    NcxxVar start_time;
    NcxxVar end_time;
    NcxxVar last_descendant_end_time;
    NcxxVar time_origin;
    NcxxVar history_in_scans;
    NcxxVar history_in_secs;
    NcxxVar duration_in_scans;
    NcxxVar duration_in_secs;
    NcxxVar nparents;
    NcxxVar nchildren;
    NcxxVar parent;
    NcxxVar child;
    NcxxVar first_entry_offset;

    NcxxVar n_simples_per_complex;
    NcxxVar simples_per_complex_1D;
    NcxxVar simples_per_complex_offsets;

  };
  
  // complex track vars

  class ComplexTrackVars
  {
  public:
    NcxxVar complex_track_num;
    NcxxVar volume_at_start_of_sampling;
    NcxxVar volume_at_end_of_sampling;
    NcxxVar start_scan;
    NcxxVar end_scan;
    NcxxVar duration_in_scans;
    NcxxVar duration_in_secs;
    NcxxVar start_time;
    NcxxVar end_time;
    NcxxVar n_simple_tracks;
    NcxxVar n_top_missing;
    NcxxVar n_range_limited;
    NcxxVar start_missing;
    NcxxVar end_missing;
    NcxxVar n_samples_for_forecast_stats;
  };
  
  // track entry vars
  
  class TrackEntryVars
  {
  public:
    NcxxVar time;
    NcxxVar time_origin;
    NcxxVar scan_origin;
    NcxxVar scan_num;
    NcxxVar storm_num;
    NcxxVar simple_track_num;
    NcxxVar complex_track_num;
    NcxxVar history_in_scans;
    NcxxVar history_in_secs;
    NcxxVar duration_in_scans;
    NcxxVar duration_in_secs;
    NcxxVar forecast_valid;
    NcxxVar prev_entry_offset;
    NcxxVar this_entry_offset;
    NcxxVar next_entry_offset;
    NcxxVar next_scan_entry_offset;
  };
  
  // track forecast properties
  
  class ForecastPropsVars
  {
  public:
    NcxxVar proj_area_centroid_x;
    NcxxVar proj_area_centroid_y;
    NcxxVar vol_centroid_z;
    NcxxVar refl_centroid_z;
    NcxxVar top;
    NcxxVar dbz_max;
    NcxxVar volume;
    NcxxVar precip_flux;
    NcxxVar mass;
    NcxxVar proj_area;
    NcxxVar smoothed_proj_area_centroid_x;
    NcxxVar smoothed_proj_area_centroid_y;
    NcxxVar smoothed_speed;
    NcxxVar smoothed_direction;
  };

  // track verification
  
  class TrackingVerifyVars
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

  // contingency table vars for verification of forecast shape
  
  class ShapeVerifyVars
  {
  public:
    NcxxVar ellipse_forecast_n_success;
    NcxxVar ellipse_forecast_n_failure;
    NcxxVar ellipse_forecast_n_false_alarm;
    NcxxVar polygon_forecast_n_success;
    NcxxVar polygon_forecast_n_failure;
    NcxxVar polygon_forecast_n_false_alarm;
  };

public:
  
  // constructor
  
  TitanFile();
  
  // destructor
  
  virtual ~TitanFile();

  // set global attributes for output netcdf file

  void setTitle(const string &val) { _title = val; }
  void setInstitution(const string &val) { _institution = val; }
  void setSource(const string &val) { _source = val; }
  void setComment(const string &val) { _comment = val; }

  // top level vars

  int nScans() const { return _nScans; }

  // storm data access

  const TitanData::StormHeader &storm_header() const { return _storm_header; }
  const TitanData::StormParams &storm_params() const { return _storm_header.params; }
  const TitanData::ScanHeader &scan() const { return _scan; }
  const vector<TitanData::StormGprops> &gprops() const { return _gprops; }
  const vector<TitanData::StormLprops> &lprops() const { return _lprops; }
  const vector<TitanData::StormDbzHist> &hist() const { return _hist; }
  const vector<TitanData::StormRun> &runs() const { return _runs; }
  const vector<TitanData::StormRun> &proj_runs() const { return _proj_runs; }
  
  // track data access

  const TitanData::TrackHeader &track_header() const { return _track_header; }
  const TitanData::TrackingParams &track_params() const { return _track_header.params; }
  const TitanData::SimpleTrackParams &simple_params() const;
  const TitanData::ComplexTrackParams &complex_params() const;
  const TitanData::TrackEntry &entry() const { return _entry; }
  const vector<TitanData::TrackEntry> &scan_entries() const { return _scan_entries; }
  const vector<TitanData::TrackScanIndex> &scan_index() const { return _scan_index; }
  const track_utime_t *track_utime() const { return _track_utime; }
  int n_scan_entries() { return _n_scan_entries; }
  
  const si32 *complex_track_nums() { return _complex_track_nums; }
  const si32 *n_simples_per_complex() { return _n_simples_per_complex; }
  const si32 *simples_per_complex_offsets() { return _simples_per_complex_offsets; }
  si32 *simples_per_complex_1D() { return _simples_per_complex_1D; }
  si32 **simples_per_complex_2D() { return _simples_per_complex_2D; }

  // public functions

  // memory allocation and freeing - storms

  void allocLayers(int n_layers);
  void freeLayers();
  void allocHist(int n_hist);
  void freeHist();
  void allocRuns(int n_runs);
  void freeRuns();
  void allocProjRuns(int n_proj_runs);
  void freeProjRuns();
  void allocGprops(int nstorms);
  void freeGprops();
  void freeStormsAll();
    
  // memory allocation and freeing - tracks

  void allocSimpleArrays(int n_simple_needed);
  void freeSimpleArrays();
  void allocComplexArrays(int n_complex_needed);
  void freeComplexArrays();
  void allocScanEntries(int n_entries_needed);
  void freeScanEntries();
  void allocScanIndex(int n_scans_needed);
  void freeScanIndex();
  void allocUtime();
  void freeUtime();
  void freeTracksAll();

  /////////////////////////////////////////////////////
  // NetCDF FileIO
  
  int openFile(const string &path,
               NcxxFile::FileMode mode);
  
  // Open file from dir and date
  
  int openFile(const string &dir,
               time_t date,
               NcxxFile::FileMode mode,
               bool isLegacyV5Format = false);
  
  void closeFile();

  // get path in use
  
  string getPathInUse() const { return _filePath; }
  string getStormFileHeaderPath() const;
  string getStormFileDataPath() const;
  string getTrackFileHeaderPath() const;
  string getTrackFileDataPath() const;
  
  /////////////////////////////////////////////////////
  // Storms
  
  // Open the storm header and data files
  
  int openStormFiles(const char *mode,
                     const char *header_file_path,
                     const char *data_file_ext = NULL);
  
  // Close the storm header and data files

  void closeStormFiles();
  
  // Flush the file forcing write
  
  void flushFile();
  
  // Put an advisory lock on the lock file
  // Mode is "w" - write lock, or "r" - read lock.
  // returns 0 on success, -1 on failure
  
  int lockFile(const char *mode);
  
  // Remove advisory lock from the lock file
  // returns 0 on success, -1 on failure

  int unlockFile();
  
  // read the storm file header

  int readStormHeader(bool clear_error_str = true);
     
  // read in the storm projected area runs
  // Space for the array is allocated.
  // returns 0 on success, -1 on failure

  int readProjRuns(int storm_num);
     
  // Read in the scan info and global props for a particular scan
  // in a storm properties file.
  // If storm num is set, only the gprops for that storm is swapped
  // returns 0 on success, -1 on failure

  int readStormScan(int scan_num, int storm_num = -1);
  
  // read in the auxiliary storm property data (lprops, hist, runs)
  // for a given storm in a scan.
  // Space for the arrays of structures is allocated as required.
  // returns 0 on success, -1 on failure

  int readStormAux(int storm_num);
     
  // seek to the end of the storm data in data file
  // returns 0 on success, -1 on failure

  int seekStormEndData();

  // seek to the start of the storm data in data file
  // returns 0 on success, -1 on failure

  int seekStormStartData();
  
  // write the TitanData::StormHeader structure to a storm file.
  //
  // NOTE: should be called after writeSecProps() and writeScan(),
  // so that appropriate n_scans can be determined.
  //
  // returns 0 on success, -1 on failure
  
  int writeStormHeader(const TitanData::StormHeader &storm_file_header);
  
  // write scan header and storm global properties
  // for a particular scan.
  //
  // NOTE: writeSecProps() must be called first, so that
  // the appropriate offsets can be set.
  //
  // returns 0 on success, -1 on failure
  
  int writeStormScan(const TitanData::StormHeader &storm_file_header,
                     const TitanData::ScanHeader &sheader,
                     const vector<TitanData::StormGprops> &gprops);
  
  // write the auxiliary storm properties:
  //   layers, dbz histograms, runs and proj_runs
  //
  // this must be called before writeScan(), so that the offsets
  // can be set appropriately
  //
  // NOTE: must be called, for all storms in a scan, before writeScan(),
  // so that the appropriate offsets can be set.
  //
  // returns 0 on success, -1 on failure

  int writeStormAux(int storm_num,
                    const TitanData::StormHeader &storm_file_header,
                    const TitanData::ScanHeader &sheader,
                    const vector<TitanData::StormGprops> &gprops,
                    const vector<TitanData::StormLprops> &lprops,
                    const vector<TitanData::StormDbzHist> &hist,
                    const vector<TitanData::StormRun> &runs,
                    const vector<TitanData::StormRun> &proj_runs);
  
  // truncate when rerunning
  // truncates storm data at the specified scan and
  // sets all tracking data to missing
  
  int truncateData(int lastGoodScanNum);

  /////////////////////////////////////////////////////
  // Tracks
  
  // read in the track_file_header_t structure from a track file.
  // Read in associated arrays (complex_track_nums,
  //   scan_index, n_simples_per_complex,
  //   simples_per_complex_offsets)
  // returns 0 on success, -1 on failure

  int readTrackHeader(bool clear_error_str = true);
     
  // Read in the track_file_header_t and scan_index array.
  // returns 0 on success, -1 on failure

  int readTrackScanIndex(bool clear_error_str = true);
     
  // reads in the parameters for a complex track
  // For normal reads, read_simples_per_complex should be set true. This
  // is only set FALSE in Titan, which creates the track files.
  // returns 0 on success, -1 on failure
  
  int readComplexTrackParams(int track_num, bool read_simples_per_complex,
                             bool clear_error_str = true);
  
  // read in the parameters for a simple track
  // returns 0 on success, -1 on failure

  int readSimpleTrackParams(int track_num,
                            bool clear_error_str = true);
     
  // read in an entry for a track
  // If first_entry is set to TRUE, then the first entry is read in. If not
  // the next entry is read in.
  // returns 0 on success, -1 on failure
  
  int readTrackEntry();
  
  // read in the array of simple tracks for each complex track
  // returns 0 on success, -1 on failure
  
  int readSimplesPerComplex();
  
  // load vector with simples per complex, in linear order
  // in memory these are stored in a si32** 2-d array
  
  void loadVecSimplesPerComplex(vector<si32> &simpsPerComplexLin,
                                vector<si32> &simpsPerComplexOffsets);
  
  // read in entries for a scan
  // returns 0 on success, -1 on failure

  int readScanEntries(int scan_num);
     
  // read in track_utime_t array
  // Returns 0 on success or -1 on error

  int readUtime();
  
  // Reinitialize headers and arrays

  void reinit();

  // Set a complex params slot in the file available for
  // reuse, by setting the offset to its negative value.
  // returns 0 on success, -1 on failure

  int reuseComplexSlot(int track_num);
     
  // Clear a complex params slot in the file available for
  // reuse, by setting the values to missing.

  int clearComplexSlot(int complex_track_num);
  
  // prepare a simple track for reading by reading in the simple track
  // params and setting the first_entry flag
  // returns 0 on success, -1 on failure

  int rewindSimpleTrack(int track_num);
     
  // rewrite an entry for a track in the track data file
  // The entry is written at the file offset of the original entry
  // returns 0 on success, -1 on failure
  
  int rewriteTrackEntry();
     
  // seek to the end of the track file data

  int seekTrackEndData();

  // seek to the start of data in track data file

  int seekTrackStartData();
     
  // write the track_file_header_t structure to a track data file
  // returns 0 on success, -1 on failure

  int writeTrackHeader(const TitanData::TrackHeader &track_file_header,
                       const si32 *complex_track_nums,
                       const si32 *n_simples_per_complex,
                       const si32 **simples_per_complex_2D);

  // write simple track params at the end of the file
  // returns 0 on success, -1 on failure
  
  int writeSimpleTrackParams(int track_num,
                             const TitanData::SimpleTrackParams &sparams);
     
  // write complex track params
  // complex_index is the index of this track in the complex_track_nums array
  // returns 0 on success, -1 on failure
  
  int writeComplexTrackParams(int complex_index,
                              const TitanData::ComplexTrackParams &cparams);
     
  // write an entry for a track in the track data file
  // on success returns the offset of the entry written
  // -1 on failure
  
  int writeTrackEntry(const TitanData::TrackEntry &entry);
  
  // write arrays designating which simple tracks are contained
  // in each complex track
  // returns 0 on success, -1 on failure
  
  int writeSimplesPerComplexArrays(int n_simple_tracks,
                                   const si32 *n_simples_per_complex,
                                   const si32 *simples_per_complex_offsets,
                                   const si32 *simples_per_complex_1D);
     
  // get the offset of storm or entry props, given the
  // scan_num and storm_num.
  //
  // First we read the scan first offset, and then add the
  // storm_num.
  
  int getScanEntryOffset(int scan_num, int storm_num);
  
  // get the next offset in the scan, for a given entry
  // returns -1 if this is the last entry in a scan
  
  int getNextScanEntryOffset(int scan_num,
                             int storm_num);
  
  // Convert the ellipse data (orientation, major_radius and minor_radius)
  // for a a gprops struct to local (km) values.
  // This applies to structs which were derived from lat-lon grids, for
  // which some of the fields are in deg instead of km.
  // It is a no-op for other projections.
  //
  // See Note 3 in storms.h

  void gpropsEllipses2Km(const TitanData::ScanHeader &scan,
			 storm_file_global_props_t &gprops);
  
     
  // Convert the (x,y) km locations in a gprops struct to lat-lon.
  // This applies to structs which were computed for non-latlon 
  // grids. It is a no-op for lat-lon grids.
  //
  // See Note 3 in storms.h
  
  void gpropsXY2LatLon(const TitanData::ScanHeader &scan,
		       storm_file_global_props_t &gprops);
  
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
  string _title;
  string _institution;
  string _source;
  string _comment;
 
  // netcdf file
  
  NcxxFile _ncFile;
  string _filePath;
  string _lockPath;
  int _lockId;
  NcxxFile::FileMode _fileMode;
  
  // legacy files
  
  TitanStormFile _sFile;
  TitanTrackFile _tFile;
  bool _isLegacyV5Format;
  
  ////////////////////////////////////////////////////////
  // dimensions
  
  NcxxDim _scansDim;
  NcxxDim _stormsDim;
  NcxxDim _simpleDim;
  NcxxDim _maxComplexDim;
  NcxxDim _entriesDim;
  NcxxDim _polyDim;
  NcxxDim _layersDim;
  NcxxDim _histDim;
  NcxxDim _runsDim;
  NcxxDim _projRunsDim;
  NcxxDim _maxFcastWtsDim;
  NcxxDim _maxParentsDim;
  NcxxDim _maxChildrenDim;
  
  ////////////////////////////////////////////////////////
  // groups
  
  // top level groups
  
  NcxxGroup _scansGroup;
  NcxxGroup _stormsGroup;
  NcxxGroup _tracksGroup;
  
  // storms group
  
  NcxxGroup _gpropsGroup;
  NcxxGroup _layersGroup;
  NcxxGroup _histGroup;
  NcxxGroup _runsGroup;
  NcxxGroup _projRunsGroup;

  // tracks group
  
  NcxxGroup _simpleGroup;
  NcxxGroup _complexGroup;
  NcxxGroup _entriesGroup;

  /////////////////////////////////////////////////////////////////
  // netcdf variables - grouped
  
  TopLevelVars _topLevelVars;
  ScanVars _scanVars;
  GridVars _scanGridVars;
  
  // storm identification
  
  StormParamsVars _sparamsVars;
  StormGpropsVars _gpropsVars;
  StormLpropsVars _lpropsVars;
  StormHistVars _histVars;
  StormRunsVars _runsVars;
  StormRunsVars _projRunsVars;

  // tracking
  
  TrackingParamsVars _tparamsVars;
  TrackingStateVars _tstateVars;
  TrackingVerifyVars _tverifyVars;
  GridVars _tverifyGridVars;
  SimpleTrackVars _simpleVars;
  NcxxVar _complexTrackNumsVar;
  ComplexTrackVars _complexVars;
  TrackEntryVars _entryVars;
  
  // forecast properties
  
  ForecastPropsVars _globalBiasVars;
  ForecastPropsVars _globalRmseVars;
  ForecastPropsVars _complexBiasVars;
  ForecastPropsVars _complexRmseVars;
  ForecastPropsVars _entryDvalDtVars;

  // tracking verification contingency tables
  
  ShapeVerifyVars _globalVerifyVars;
  ShapeVerifyVars _complexVerifyVars;

  //////////////////////////////////////////////////////////////

  // top level
  
  int _nScans;
  int _sumStorms;
  int _sumLayers;
  int _sumHist;
  int64_t _sumRuns;
  int64_t _sumProjRuns;

  // storm data

  TitanData::StormHeader _storm_header;
  TitanData::ScanHeader _scan;
  vector<TitanData::StormGprops> _gprops;
  vector<TitanData::StormLprops> _lprops;
  vector<TitanData::StormDbzHist> _hist;
  vector<TitanData::StormRun> _runs;
  vector<TitanData::StormRun> _proj_runs;

  // offsets for auxiliary storm properties

  vector<int64_t> _layerOffsets;
  vector<int64_t> _histOffsets;
  vector<int64_t> _runsOffsets;
  vector<int64_t> _projRunsOffsets;
  
  // storm memory allocation
  
  int _max_storms;
  int _max_layers;
  int _max_hist;
  int64_t _max_runs;
  int64_t _max_proj_runs;

  // track data

  bool _first_entry;  // set to TRUE if first entry of a track
  
  TitanData::TrackHeader _track_header;
  TitanData::SimpleTrackParams _simple_params;
  TitanData::ComplexTrackParams _complex_params;
  TitanData::TrackEntry _entry;
  
  vector<TitanData::TrackScanIndex> _scan_index;
  vector<TitanData::TrackEntry> _scan_entries;
  track_utime_t *_track_utime;
  
  si32 *_complex_track_nums;
  si32 *_n_simples_per_complex;
  si32 *_simples_per_complex_offsets;
  si32 *_simples_per_complex_1D;
  si32 **_simples_per_complex_2D;
  int _n_scan_entries;
  
  // track entry offsets
  
  long _prevEntryOffset;
  
  // track memory allocation control

  int _n_simple_allocated;
  int _n_complex_allocated;
  int _n_simples_per_complex_2D_allocated;
  // int _n_scan_entries_allocated;
  // int _n_scan_index_allocated;
  int _n_utime_allocated;
  int _lowest_avail_complex_slot;

  // units for horizontal grids

  string _horizGridUnits;
  string _horizGridUnitsPerHr;
  string _speedUnits;
  string _speedUnitsPerHr;
  
  // errors
  
  string _errStr;
  
  // functions

  int _openLegacyFiles(const string &path,
                       NcxxFile::FileMode mode);
  
  // set global attributes
  
  void _setGlobalAttributes();
  
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
  
  // get scalar variable
  
  NcxxVar _getVar(const std::string& name,
                  const NcxxType& ncType,
                  NcxxGroup &group,
                  std::string units = "");
  
  // get 1D array variable
  
  NcxxVar _getVar(const std::string& name,
                  const NcxxType& ncType,
                  const NcxxDim& dim,
                  NcxxGroup &group,
                  std::string units = "");
  
  // get 2D array variable
  
  NcxxVar _getVar(const std::string& name,
                  const NcxxType& ncType,
                  const NcxxDim& dim0,
                  const NcxxDim& dim1,
                  NcxxGroup &group,
                  std::string units = "");
  
  void _setFillValue(NcxxVar &var);
  
  /////////////////////////////////////////////////////
  // set up variables

  void _setUpVars();

  // update attributes for scan type
  
  void _updateScanAttributes(const TitanData::ScanHeader &scanHeader);

  // add projection flag attributes
  
  void _addProjectionFlagAttributes();

  /////////////////////////////////////////////////////
  // clear error string
  
  void _clearErrStr();

  // units conversion
  
  void _convertEllipse2Km(const Mdvx::coord_t &tgrid,
                          double centroid_x,
                          double centroid_y,
                          fl32 &orientation,
                          fl32 &minor_radius,
                          fl32 &major_radius);

  void _convertEllipse2Km(const titan_grid_t &tgrid,
                          double centroid_x,
                          double centroid_y,
                          fl32 &orientation,
                          fl32 &minor_radius,
                          fl32 &major_radius);

  // read entry at given offset

  int _readTrackEntry(TitanData::TrackEntry &entry, int entryOffset);

  // clear groups for truncation
  
  void _clearGroupVars(NcxxGroup &group, int startIndex);
  void _clear1DVar(NcxxVar &var, int startIndex);
  void _clear2DVar(NcxxVar &var, int startIndex);

public:

  // friends for Titan program which writes the storm and track files
  
  friend class StormFile;
  friend class Area;
  friend class Props;
  friend class Identify;
  friend class StormIdent;

private:
  
  // Private methods with no bodies. Copy and assignment not implemented.

  TitanFile(const TitanFile & orig);
  TitanFile & operator = (const TitanFile & other);

public:

  // strings for netcdf

  // attributes
  
  const std::string VERSION = "version";
  const std::string CONVENTION = "convention";
  const std::string TITLE = "title";
  const std::string INSTITUTION = "institution";
  const std::string SOURCE = "source";
  const std::string COMMENT = "comment";

  const std::string UNITS = "units";
  const std::string TIME0 = "seconds since 1970-01-01T00:00:00";
  const std::string SECONDS = "seconds";
  const std::string DEG = "deg";
  const std::string DBZ = "dBZ";
  const std::string DBZ_PER_KM = "dBZ/km";
  const std::string KM = "km";
  const std::string KM2 = "km2";
  const std::string KM3 = "km3";
  const std::string KTONS = "ktons";
  const std::string PER_SEC = "/s";
  const std::string M_PER_SEC = "m/s";
  const std::string M3_PER_SEC = "m3/s";
  const std::string KG_PER_M2 = "kg/m2";
  
  const std::string KM_PER_HR = "km/hr";
  const std::string DEG_PER_HR = "deg/hr";

  const std::string DBZ_PER_HR = "dBZ/hr";
  const std::string KM2_PER_HR = "km2/hr";
  const std::string KM3_PER_HR = "km3/hr";
  const std::string KTONS_PER_HR = "ktons/hr";
  const std::string M3_PER_SEC_PER_HR = "(m3/s)/hr";
  const std::string KM_PER_HR_PER_HR = "(km/hr)/hr";
  const std::string DEG_PER_HR_PER_HR = "(deg/hr)/hr";

  const std::string FLAG_VALUES = "flag_values";
  const std::string FLAG_MEANINGS = "flag_meanings";
  const std::string NOTE = "note";
  
  // groups

  const std::string SCANS = "scans";
  const std::string STORMS = "storms";
  const std::string TRACKS = "tracks";
  const std::string GPROPS = "gprops";
  const std::string LAYERS = "layers";
  const std::string HIST = "hist";
  const std::string RUNS = "runs";
  const std::string PROJ_RUNS = "proj_runs";
  const std::string SIMPLE = "simple";
  const std::string COMPLEX = "complex";
  const std::string ENTRIES = "entries";

  // top-level dimensions & attributes

  const std::string TIME = "time";
  const std::string FILE_TIME = "file_time";
  const std::string START_TIME = "start_time";
  const std::string END_TIME = "end_time";
  const std::string N_SCANS = "n_scans";
  const std::string N_STORMS = "n_storms";
  const std::string N_SIMPLE = "n_simple";
  const std::string MAX_SIMPLE_TRACK_NUM = "max_simple_track_num";
  const std::string N_COMPLEX = "n_complex";
  const std::string MAX_COMPLEX_TRACK_NUM = "max_complex_track_num";
  const std::string N_ENTRIES = "n_entries";
  const std::string N_POLY = "n_poly";
  const std::string N_LAYERS = "n_layers";
  const std::string N_RUNS = "n_runs";
  const std::string N_PROJ_RUNS = "n_proj_runs";
  const std::string N_HIST = "n_hist";
  const std::string MAX_FORECAST_WEIGHTS = "max_forecast_weights";

  const std::string SUM_STORMS = "sum_storms";
  const std::string SUM_LAYERS = "sum_layers";
  const std::string SUM_HIST = "sum_hist";
  const std::string SUM_RUNS = "sum_runs";
  const std::string SUM_PROJ_RUNS = "sum_proj_runs";

  // storm identification parameters

  const std::string LOW_DBZ_THRESHOLD = "low_dbz_threshold";
  const std::string HIGH_DBZ_THRESHOLD = "high_dbz_threshold";
  const std::string DBZ_HIST_INTERVAL = "dbz_hist_interval";
  const std::string HAIL_DBZ_THRESHOLD = "hail_dbz_threshold";
  const std::string BASE_THRESHOLD = "base_threshold";
  const std::string TOP_THRESHOLD = "top_threshold";
  const std::string MIN_STORM_SIZE = "min_storm_size";
  const std::string MAX_STORM_SIZE = "max_storm_size";
  const std::string MORPHOLOGY_EROSION_THRESHOLD = "morphology_erosion_threshold";
  const std::string MORPHOLOGY_REFL_DIVISOR = "morphology_refl_divisor";
  const std::string MIN_RADAR_TOPS = "min_radar_tops";
  const std::string TOPS_EDGE_MARGIN = "tops_edge_margin";
  const std::string Z_P_COEFF = "z_p_coeff";
  const std::string Z_P_EXPONENT = "z_p_exponent";
  const std::string Z_M_COEFF = "z_m_coeff";
  const std::string Z_M_EXPONENT = "z_m_exponent";
  const std::string SECTRIP_VERT_ASPECT = "sectrip_vert_aspect";
  const std::string SECTRIP_HORIZ_ASPECT = "sectrip_horiz_aspect";
  const std::string SECTRIP_ORIENTATION_ERROR = "sectrip_orientation_error";
  const std::string POLY_START_AZ = "poly_start_az";
  const std::string POLY_DELTA_AZ = "poly_delta_az";
  const std::string CHECK_MORPHOLOGY = "check_morphology";
  const std::string CHECK_TOPS = "check_tops";
  const std::string VEL_AVAILABLE = "vel_available";
  const std::string N_POLY_SIDES_ = "n_poly_sides";
  const std::string LTG_COUNT_TIME = "ltg_count_time";
  const std::string LTG_COUNT_MARGIN_KM = "ltg_count_margin_km";
  const std::string HAIL_Z_M_COEFF = "hail_z_m_coeff";
  const std::string HAIL_Z_M_EXPONENT = "hail_z_m_exponent";
  const std::string HAIL_MASS_DBZ_THRESHOLD = "hail_mass_dbz_threshold";
  const std::string GPROPS_UNION_TYPE = "gprops_union_type";
  const std::string TOPS_DBZ_THRESHOLD = "tops_dbz_threshold";
  const std::string PRECIP_COMPUTATION_MODE = "precip_computation_mode";
  const std::string PRECIP_PLANE_HT = "precip_plane_ht";
  const std::string LOW_CONVECTIVITY_THRESHOLD = "low_convectivity_threshold";
  const std::string HIGH_CONVECTIVITY_THRESHOLD = "high_convectivity_threshold";

  // scan details

  const std::string SCAN_MIN_Z = "scan_min_z";
  const std::string SCAN_DELTA_Z = "scan_delta_z";
  const std::string SCAN_NUM = "scan_num";
  const std::string SCAN_NSTORMS = "scan_nstorms";
  const std::string SCAN_TIME = "scan_time";
  const std::string SCAN_GPROPS_OFFSET = "scan_gprops_offset";
  const std::string SCAN_FIRST_OFFSET = "scan_first_offset";
  const std::string SCAN_LAST_OFFSET = "scan_last_offset";
  const std::string SCAN_HT_OF_FREEZING = "scan_ht_of_freezing";
  
  // initial index for data in each scan
  
  const std::string SCAN_GPROPS_OFFSET_0 = "scan_gprops_offset_0";
  const std::string SCAN_LAYER_OFFSET_0 = "scan_layer_offset_0";
  const std::string SCAN_HIST_OFFSET_0 = "scan_hist_offset_0";
  const std::string SCAN_RUNS_OFFSET_0 = "scan_runs_offset_0";
  const std::string SCAN_PROJ_RUNS_OFFSET_0 = "scan_proj_runs_offset_0";

  // grid and projection details
  
  const std::string GRID_NX = "grid_nx";
  const std::string GRID_NY = "grid_ny";
  const std::string GRID_NZ = "grid_nz";
  const std::string GRID_MINX = "grid_minx";
  const std::string GRID_MINY = "grid_miny";
  const std::string GRID_MINZ = "grid_minz";
  const std::string GRID_DX = "grid_dx";
  const std::string GRID_DY = "grid_dy";
  const std::string GRID_DZ = "grid_dz";
  const std::string GRID_DZ_CONSTANT = "grid_dz_constant";
  const std::string GRID_SENSOR_X = "grid_sensor_x";
  const std::string GRID_SENSOR_Y = "grid_sensor_y";
  const std::string GRID_SENSOR_Z = "grid_sensor_z";
  const std::string GRID_SENSOR_LAT = "grid_sensor_lat";
  const std::string GRID_SENSOR_LON = "grid_sensor_lon";
  const std::string GRID_UNITSX = "grid_unitsx";
  const std::string GRID_UNITSY = "grid_unitsy";
  const std::string GRID_UNITSZ = "grid_unitsz";

  const std::string PROJ_TYPE = "proj_type";
  const std::string PROJ_ORIGIN_LAT = "proj_origin_lat";
  const std::string PROJ_ORIGIN_LON = "proj_origin_lon";
  const std::string PROJ_ROTATION = "proj_rotation";
  const std::string PROJ_LAT1 = "proj_lat1";
  const std::string PROJ_LAT2 = "proj_lat2";
  const std::string PROJ_TANGENT_LAT = "proj_tangent_lat";
  const std::string PROJ_TANGENT_LON = "proj_tangent_lon";
  const std::string PROJ_POLE_TYPE = "proj_pole_type";
  const std::string PROJ_CENTRAL_SCALE = "proj_central_scale";

  // storm global properties
  
  const std::string VOL_CENTROID_X = "vol_centroid_x";
  const std::string VOL_CENTROID_Y = "vol_centroid_y";
  const std::string VOL_CENTROID_Z = "vol_centroid_z";
  const std::string REFL_CENTROID_X = "refl_centroid_x";
  const std::string REFL_CENTROID_Y = "refl_centroid_y";
  const std::string REFL_CENTROID_Z = "refl_centroid_z";
  const std::string TOP = "top";
  const std::string BASE = "base";
  const std::string VOLUME = "volume";
  const std::string AREA = "area";
  const std::string AREA_MEAN = "area_mean";
  const std::string PRECIP_FLUX = "precip_flux";
  const std::string MASS = "mass";
  const std::string TILT_ANGLE = "tilt_angle";
  const std::string TILT_DIRN = "tilt_dirn";
  const std::string DBZ_MAX = "dbz_max";
  const std::string DBZ_MEAN = "dbz_mean";
  const std::string DBZ_MAX_GRADIENT = "dbz_max_gradient";
  const std::string DBZ_MEAN_GRADIENT = "dbz_mean_gradient";
  const std::string HT_OF_DBZ_MAX = "ht_of_dbz_max";
  const std::string RAD_VEL_MEAN = "rad_vel_mean";
  const std::string RAD_VEL_SD = "rad_vel_sd";
  const std::string VORTICITY = "vorticity";
  const std::string PRECIP_AREA = "precip_area";
  const std::string PRECIP_AREA_CENTROID_X = "precip_area_centroid_x";
  const std::string PRECIP_AREA_CENTROID_Y = "precip_area_centroid_y";
  const std::string PRECIP_AREA_ORIENTATION = "precip_area_orientation";
  const std::string PRECIP_AREA_MINOR_RADIUS = "precip_area_minor_radius";
  const std::string PRECIP_AREA_MAJOR_RADIUS = "precip_area_major_radius";
  const std::string PROJ_AREA = "proj_area";
  const std::string PROJ_AREA_CENTROID_X = "proj_area_centroid_x";
  const std::string PROJ_AREA_CENTROID_Y = "proj_area_centroid_y";
  const std::string PROJ_AREA_ORIENTATION = "proj_area_orientation";
  const std::string PROJ_AREA_MINOR_RADIUS = "proj_area_minor_radius";
  const std::string PROJ_AREA_MAJOR_RADIUS = "proj_area_major_radius";
  const std::string PROJ_AREA_POLYGON = "proj_area_polygon";
  const std::string STORM_NUM = "storm_num";
  const std::string BASE_LAYER = "base_layer";
  const std::string N_DBZ_INTERVALS = "n_dbz_intervals";
  const std::string TOP_MISSING = "top_missing";
  const std::string RANGE_LIMITED = "range_limited";
  const std::string SECOND_TRIP = "second_trip";
  const std::string HAIL_PRESENT = "hail_present";
  const std::string ANOM_PROP = "anom_prop";
  const std::string BOUNDING_MIN_IX = "bounding_min_ix";
  const std::string BOUNDING_MIN_IY = "bounding_min_iy";
  const std::string BOUNDING_MAX_IX = "bounding_max_ix";
  const std::string BOUNDING_MAX_IY = "bounding_max_iy";
  const std::string LAYER_PROPS_OFFSET = "layer_props_offset";
  const std::string DBZ_HIST_OFFSET = "dbz_hist_offset";
  const std::string RUNS_OFFSET = "runs_offset";
  const std::string PROJ_RUNS_OFFSET = "proj_runs_offset";
  const std::string VIL_FROM_MAXZ = "vil_from_maxz";
  const std::string LTG_COUNT = "ltg_count";
  const std::string CONVECTIVITY_MEDIAN = "convectivity_median";
  const std::string HAIL_FOKRCATEGORY = "hail_FOKRcategory";
  const std::string HAIL_WALDVOGELPROBABILITY = "hail_waldvogelProbability";
  const std::string HAIL_HAILMASSALOFT = "hail_hailMassAloft";
  const std::string HAIL_VIHM = "hail_vihm";
  const std::string HAIL_POH = "hail_poh";
  const std::string HAIL_SHI = "hail_shi";
  const std::string HAIL_POSH = "hail_posh";
  const std::string HAIL_MEHS = "hail_mehs";

  // storm dbz histogram

  const std::string PERCENT_VOLUME = "percent_volume";
  const std::string PERCENT_AREA = "percent_area";

  // storm runs
  
  const std::string RUN_IX = "run_ix";
  const std::string RUN_IY = "run_iy";
  const std::string RUN_IZ = "run_iz";
  const std::string RUN_LEN = "run_len";

  // tracking

  const std::string TRACKING_VALID = "tracking_valid";
  const std::string TRACKING_MODIFY_CODE = "tracking_modify_code";
  
  const std::string FORECAST_WEIGHTS = "forecast_weights";  
  const std::string WEIGHT_DISTANCE = "weight_distance";
  const std::string WEIGHT_DELTA_CUBE_ROOT_VOLUME = "weight_delta_cube_root_volume";
  const std::string MERGE_SPLIT_SEARCH_RATIO = "merge_split_search_ratio";
  const std::string MAX_TRACKING_SPEED = "max_tracking_speed";
  const std::string MAX_SPEED_FOR_VALID_FORECAST = "max_speed_for_valid_forecast";
  const std::string PARABOLIC_GROWTH_PERIOD = "parabolic_growth_period";
  const std::string SMOOTHING_RADIUS = "smoothing_radius";
  const std::string MIN_FRACTION_OVERLAP = "min_fraction_overlap";
  const std::string MIN_SUM_FRACTION_OVERLAP = "min_sum_fraction_overlap";
  const std::string SCALE_FORECASTS_BY_HISTORY = "scale_forecasts_by_history";
  const std::string USE_RUNS_FOR_OVERLAPS = "use_runs_for_overlaps";
  const std::string GRID_TYPE = "grid_type";
  const std::string NWEIGHTS_FORECAST = "nweights_forecast";
  const std::string FORECAST_TYPE = "forecast_type";
  const std::string MAX_DELTA_TIME = "max_delta_time";
  const std::string MIN_HISTORY_FOR_VALID_FORECAST = "min_history_for_valid_forecast";
  const std::string SPATIAL_SMOOTHING = "spatial_smoothing";

  const std::string N_SAMPLES_FOR_FORECAST_STATS = "n_samples_for_forecast_stats";
  const std::string LAST_SCAN_NUM = "last_scan_num";
  const std::string MAX_PARENTS_ = "max_parents";
  const std::string MAX_CHILDREN_ = "max_children";
  const std::string MAX_NWEIGHTS_FORECAST_ = "max_nweights_forecast";

  // simple tracks

  const std::string N_SIMPLE_TRACKS = "n_simple_tracks";
  const std::string SIMPLE_TRACK_NUM = "simple_track_num";
  const std::string LAST_DESCENDANT_SIMPLE_TRACK_NUM = "last_descendant_simple_track_num";
  const std::string START_SCAN = "start_scan";
  const std::string END_SCAN = "end_scan";
  const std::string LAST_DESCENDANT_END_SCAN = "last_descendant_end_scan";
  const std::string SCAN_ORIGIN = "scan_origin";
  const std::string LAST_DESCENDANT_END_TIME = "last_descendant_end_time";
  const std::string TIME_ORIGIN = "time_origin";
  const std::string HISTORY_IN_SCANS = "history_in_scans";
  const std::string HISTORY_IN_SECS = "history_in_secs";
  const std::string DURATION_IN_SCANS = "duration_in_scans";
  const std::string DURATION_IN_SECS = "duration_in_secs";
  const std::string NPARENTS = "nparents";
  const std::string NCHILDREN = "nchildren";
  const std::string PARENT = "parent";
  const std::string CHILD = "child";
  const std::string FIRST_ENTRY_OFFSET = "first_entry_offset";

  const std::string N_SIMPLES_PER_COMPLEX = "n_simples_per_complex";
  const std::string SIMPLES_PER_COMPLEX = "simples_per_complex";
  const std::string SIMPLES_PER_COMPLEX_OFFSETS = "simples_per_complex_offsets";
  
  // complex tracks

  const std::string N_COMPLEX_TRACKS = "n_complex_tracks";
  const std::string VOLUME_AT_START_OF_SAMPLING = "volume_at_start_of_sampling";
  const std::string VOLUME_AT_END_OF_SAMPLING = "volume_at_end_of_sampling";
  const std::string COMPLEX_TRACK_NUM = "complex_track_num";
  const std::string COMPLEX_TRACK_NUMS = "complex_track_nums";
  const std::string N_TOP_MISSING = "n_top_missing";
  const std::string N_RANGE_LIMITED = "n_range_limited";
  const std::string START_MISSING = "start_missing";
  const std::string END_MISSING = "end_missing";
  const std::string MAX_COMPLEX = "max_complex";

  // track entry

  const std::string FORECAST_VALID = "forecast_valid";
  const std::string PREV_ENTRY_OFFSET = "prev_entry_offset";
  const std::string THIS_ENTRY_OFFSET = "this_entry_offset";
  const std::string NEXT_ENTRY_OFFSET = "next_entry_offset";
  const std::string NEXT_SCAN_ENTRY_OFFSET = "next_scan_entry_offset";

  // entry dval_dt
  
  const std::string DVAL_DT_PROJ_AREA_CENTROID_X =
    "entry_dval_dt_proj_area_centroid_x";
  const std::string DVAL_DT_PROJ_AREA_CENTROID_Y =
    "entry_dval_dt_proj_area_centroid_y";
  const std::string DVAL_DT_VOL_CENTROID_Z = "entry_dval_dt_vol_centroid_z";
  const std::string DVAL_DT_REFL_CENTROID_Z = "entry_dval_dt_refl_centroid_z";
  const std::string DVAL_DT_TOP = "entry_dval_dt_top";
  const std::string DVAL_DT_DBZ_MAX = "entry_dval_dt_dbz_max";
  const std::string DVAL_DT_VOLUME = "entry_dval_dt_volume";
  const std::string DVAL_DT_PRECIP_FLUX = "entry_dval_dt_precip_flux";
  const std::string DVAL_DT_MASS = "entry_dval_dt_mass";
  const std::string DVAL_DT_PROJ_AREA = "entry_dval_dt_proj_area";
  const std::string DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_X =
    "entry_dval_dt_smoothed_proj_area_centroid_x";
  const std::string DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_Y =
    "entry_dval_dt_smoothed_proj_area_centroid_y";
  const std::string DVAL_DT_SMOOTHED_SPEED = "entry_dval_dt_smoothed_speed";
  const std::string DVAL_DT_SMOOTHED_DIRECTION = "entry_dval_dt_smoothed_direction";

  // forecast bias
  
  const std::string BIAS_PROJ_AREA_CENTROID_X =
    "forecast_bias_proj_area_centroid_x";
  const std::string BIAS_PROJ_AREA_CENTROID_Y =
    "forecast_bias_proj_area_centroid_y";
  const std::string BIAS_VOL_CENTROID_Z = "forecast_bias_vol_centroid_z";
  const std::string BIAS_REFL_CENTROID_Z = "forecast_bias_refl_centroid_z";
  const std::string BIAS_TOP = "forecast_bias_top";
  const std::string BIAS_DBZ_MAX = "forecast_bias_dbz_max";
  const std::string BIAS_VOLUME = "forecast_bias_volume";
  const std::string BIAS_PRECIP_FLUX = "forecast_bias_precip_flux";
  const std::string BIAS_MASS = "forecast_bias_mass";
  const std::string BIAS_PROJ_AREA = "forecast_bias_proj_area";
  const std::string BIAS_SMOOTHED_PROJ_AREA_CENTROID_X =
    "forecast_bias_smoothed_proj_area_centroid_x";
  const std::string BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y =
    "forecast_bias_smoothed_proj_area_centroid_y";
  const std::string BIAS_SMOOTHED_SPEED = "forecast_bias_smoothed_speed";
  const std::string BIAS_SMOOTHED_DIRECTION = "forecast_bias_smoothed_direction";

  // forecast rmse
  
  const std::string RMSE_PROJ_AREA_CENTROID_X =
    "forecast_rmse_proj_area_centroid_x";
  const std::string RMSE_PROJ_AREA_CENTROID_Y =
    "forecast_rmse_proj_area_centroid_y";
  const std::string RMSE_VOL_CENTROID_Z = "forecast_rmse_vol_centroid_z";
  const std::string RMSE_REFL_CENTROID_Z = "forecast_rmse_refl_centroid_z";
  const std::string RMSE_TOP = "forecast_rmse_top";
  const std::string RMSE_DBZ_MAX = "forecast_rmse_dbz_max";
  const std::string RMSE_VOLUME = "forecast_rmse_volume";
  const std::string RMSE_PRECIP_FLUX = "forecast_rmse_precip_flux";
  const std::string RMSE_MASS = "forecast_rmse_mass";
  const std::string RMSE_PROJ_AREA = "forecast_rmse_proj_area";
  const std::string RMSE_SMOOTHED_PROJ_AREA_CENTROID_X =
    "forecast_rmse_smoothed_proj_area_centroid_x";
  const std::string RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y =
    "forecast_rmse_smoothed_proj_area_centroid_y";
  const std::string RMSE_SMOOTHED_SPEED = "forecast_rmse_smoothed_speed";
  const std::string RMSE_SMOOTHED_DIRECTION = "forecast_rmse_smoothed_direction";

  // verification contingency stats
  
  const std::string ELLIPSE_FORECAST_N_SUCCESS = "ellipse_forecast_n_success";
  const std::string ELLIPSE_FORECAST_N_FAILURE = "ellipse_forecast_n_failure";
  const std::string ELLIPSE_FORECAST_N_FALSE_ALARM = "ellipse_forecast_n_false_alarm";
  const std::string POLYGON_FORECAST_N_SUCCESS = "polygon_forecast_n_success";
  const std::string POLYGON_FORECAST_N_FAILURE = "polygon_forecast_n_failure";
  const std::string POLYGON_FORECAST_N_FALSE_ALARM = "polygon_forecast_n_false_alarm";

  // track verification
  
  const std::string VERIFICATION_PERFORMED = "verification_performed";
  const std::string VERIFY_FORECAST_LEAD_TIME = "verify_forecast_lead_time";
  const std::string VERIFY_END_TIME = "verify_end_time";
  const std::string VERIFY_FORECAST_LEAD_TIME_MARGIN =
    "verify_forecast_lead_time_margin";
  const std::string VERIFY_FORECAST_MIN_HISTORY = "verify_forecast_min_history";
  const std::string VERIFY_BEFORE_FORECAST_TIME = "verify_before_forecast_time";
  const std::string VERIFY_AFTER_TRACK_DIES = "verify_after_track_dies";

};

#endif
