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
  
  // write the storm_file_header_t structure to a storm file.
  // returns 0 on success, -1 on failure
  
  int WriteStormHeader(const storm_file_header_t &storm_header);
  
  // write the storm layer property and histogram data for a storm,
  // at the end of the file.
  // returns 0 on success, -1 on failure

  int WriteProps(int storm_num);

  // write scan header and global properties for a particular scan
  // in a storm properties file.
  // Performs the writes from the end of the file.
  // returns 0 on success, -1 on failure
  
  int WriteScan(int scan_num);
     
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

  // dimensions
  
  NcxxDim _scans;
  NcxxDim _storms;
  NcxxDim _simpleTracks;
  NcxxDim _complexTracks;

  // top level groups
  
  NcxxGroup _scansGroup;
  NcxxGroup _stormsGroup;
  NcxxGroup _tracksGroup;

  // storms group
  
  NcxxGroup _stormGpropsGroup;
  NcxxGroup _stormLpropsGroup;
  NcxxGroup _stormHistGroup;
  NcxxGroup _stormRunsGroup;
  NcxxGroup _stormProjRunsGroup;

  // tracks group
  
  NcxxGroup _simpleTrackGroup;
  NcxxGroup _complexTrackGroup;
  NcxxGroup _trackEntryGroup;

  // storm header variables

  NcxxVar _fileTimeVar;
  NcxxVar _startTimeVar;
  NcxxVar _endTimeVar;
  
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
  
  // strings for netcdf groups

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
  static constexpr const char* ENTRY = "entry";
  
  // strings for storm identification parameters

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

  // strings for tracking parameters

  static constexpr const char* FORECAST_WEIGHTS = "forecast_weights";  
  static constexpr const char* WEIGHT_DISTANCE = "weight_distance";
  static constexpr const char* WEIGHT_DELTA_CUBE_ROOT_VOLUME = "weight_delta_cube_root_volume";
  static constexpr const char* MERGE_SPLIT_SEARCH_RATIO = "merge_split_search_ratio";
  static constexpr const char* MAX_TRACKING_SPEED = "max_tracking_speed";
  static constexpr const char* MAX_SPEED_FOR_VALID_FORECAST = "max_speed_for_valid_forecast";
  static constexpr const char* PARABOLIC_GROWTH_PERIOD = "parabolic_growth_period";
  static constexpr const char* SMOOTHING_RADIUS = "smoothing_radius";
  static constexpr const char* MIN_FRACTION_OVERLAP = "min_fraction_overlap";
  static constexpr const char* MIN_SUM_FRACTION_OVERLAP = "min_sum_fraction_overlap";
  static constexpr const char* SCALE_FORECASTS_BY_HISTORY = "scale_forecasts_by_history";
  static constexpr const char* USE_RUNS_FOR_OVERLAPS = "use_runs_for_overlaps";
  static constexpr const char* GRID_TYPE = "grid_type";
  static constexpr const char* NWEIGHTS_FORECAST = "nweights_forecast";
  static constexpr const char* FORECAST_TYPE = "forecast_type";
  static constexpr const char* MAX_DELTA_TIME = "max_delta_time";
  static constexpr const char* MIN_HISTORY_FOR_VALID_FORECAST = "min_history_for_valid_forecast";
  static constexpr const char* SPATIAL_SMOOTHING = "spatial_smoothing";

};

#endif


