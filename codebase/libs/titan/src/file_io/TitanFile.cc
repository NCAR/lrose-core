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
////////////////////////////////////////////////////////////////
// TitanFile.cc
//
// This class handles the NetCDF file IO for TITAN.
//
// Mike Dixon, EOL, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2025.
//
////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <cerrno>
#include <dataport/bigend.h>
#include <titan/TitanFile.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/pjg.h>
#include <toolsa/sincos.h>
#include <toolsa/TaArray.hh>
using namespace std;

////////////////////////////////////////////////////////////
// Constructor

TitanFile::TitanFile()

{

  // storms
  
  MEM_zero(_storm_header);
  MEM_zero(_scan);
  _gprops = nullptr;
  _lprops = nullptr;
  _hist = nullptr;
  _runs = nullptr;
  _proj_runs = nullptr;
  _scan_offsets = nullptr;
  _storm_num = 0;

  _storm_header_file = nullptr;
  _storm_data_file = nullptr;

  _storm_header_file_label = STORM_HEADER_FILE_TYPE;
  _storm_data_file_label = STORM_DATA_FILE_TYPE;

  _max_scans = 0;
  _max_storms = 0;
  _max_layers = 0;
  _max_dbz_intervals = 0;
  _max_runs = 0;
  _max_proj_runs = 0;

  // tracks

  MEM_zero(_track_header);
  MEM_zero(_simple_params);
  MEM_zero(_complex_params);
  MEM_zero(_entry);

  _scan_index = nullptr;
  _scan_entries = nullptr;
  _track_utime = nullptr;
  
  _complex_track_nums = nullptr;
  _complex_track_offsets = nullptr;
  _simple_track_offsets = nullptr;
  _nsimples_per_complex = nullptr;
  _simples_per_complex_offsets = nullptr;
  _simples_per_complex = nullptr;

  _track_header_file_label = TRACK_HEADER_FILE_TYPE;
  _track_data_file_label = TRACK_DATA_FILE_TYPE;

  _track_header_file = nullptr;
  _track_data_file = nullptr;

  _first_entry = true;

  _n_scan_entries = 0;
  _lowest_avail_complex_slot = 0;

  _n_simple_allocated = 0;
  _n_complex_allocated = 0;
  _n_simples_per_complex_allocated = 0;
  _n_scan_entries_allocated = 0;
  _n_scan_index_allocated = 0;
  _n_utime_allocated = 0;

  _convention = "TitanStormTracking";
  _version = "1.0";


}

////////////////////////////////////////////////////////////
// destructor

TitanFile::~TitanFile()

{

  freeStormsAll();
  closeStormFiles();
  freeTracksAll();
  closeTrackFiles();

  closeNcFile();
  
}

/////////////////////////////////////////
// Open file

int TitanFile::openNcFile(const string &path,
                            NcxxFile::FileMode mode)

{

  // ensure the directory exists
  
  // if (mode == NcxxFile::FileMode::write) {
  Path filePath(path);
  filePath.makeDirRecurse();
  // }
  
  // open file
  
  try {
    _ncFile.open(path, mode);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - TitanFile::openNcFile");
    _addErrStr("  Cannot open file: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // set up netcdf groups

  _setUpGroups();

  // set up the dimensions

  _setUpDims();
  
  // set up netcdf variables

  _setUpVars();
  
  return 0;
  
}
  
void TitanFile::closeNcFile()

{

  _ncFile.close();
  
}
     
/////////////////////////////////////////
// set up groups

void TitanFile::_setUpGroups()
{

  _scansGroup = _getGroup(SCANS, _ncFile);
  _stormsGroup = _getGroup(STORMS, _ncFile);
  _tracksGroup = _getGroup(TRACKS, _ncFile);

  _gpropsGroup = _getGroup(GPROPS, _stormsGroup);
  _lpropsGroup = _getGroup(LPROPS, _stormsGroup);
  _histGroup = _getGroup(HIST, _stormsGroup);
  _runsGroup = _getGroup(RUNS, _stormsGroup);
  _projRunsGroup = _getGroup(PROJ_RUNS, _stormsGroup);

  _simpleGroup = _getGroup(SIMPLE, _tracksGroup);
  _complexGroup = _getGroup(COMPLEX, _tracksGroup);
  _entriesGroup = _getGroup(ENTRIES, _tracksGroup);

}

/////////////////////////////////////////
// get group relative to a parent group

NcxxGroup TitanFile::_getGroup(const std::string& name,
                                 NcxxGroup &parent)
{

  // get group if it exists
  NcxxGroup group = _ncFile.getGroup(name);
  if (group.isNull()) {
    // group does not exist, create it
    group = _ncFile.addGroup(name);
  }
  return group;

}

/////////////////////////////////////////
// set up dimensions

void TitanFile::_setUpDims()

{

  _n_scans = _getDim(N_SCANS, _scansGroup);
  _n_storms = _getDim(N_STORMS, _gpropsGroup);
  _n_simple = _getDim(N_SIMPLE, _simpleGroup);
  _n_complex = _getDim(N_COMPLEX, _complexGroup);
  _n_entries = _getDim(N_ENTRIES, _entriesGroup);
  _n_poly = _getDim(N_POLY, N_POLY_SIDES, _gpropsGroup);
  _n_layers = _getDim(N_LAYERS, _lpropsGroup);
  _n_runs = _getDim(N_RUNS, _runsGroup);
  _n_proj_runs = _getDim(N_PROJ_RUNS, _projRunsGroup);
  _n_hist = _getDim(N_HIST, _histGroup);
  _max_forecast_weights = _getDim(MAX_FORECAST_WEIGHTS, MAX_NWEIGHTS_FORECAST, _tracksGroup);
  _max_parents = _getDim(MAX_PARENTS_, MAX_PARENTS, _simpleGroup);
  _max_children = _getDim(MAX_CHILDREN_, MAX_CHILDREN, _simpleGroup);
  
}

/////////////////////////////////////////
// get dimension

NcxxDim TitanFile::_getDim(const std::string& name,
                             NcxxGroup &group)
{
  NcxxDim dim = group.getDim(name, NcxxGroup::All);
  if (dim.isNull()) {
    dim = group.addDim(name);
  }
  return dim;
}

NcxxDim TitanFile::_getDim(const std::string& name,
                             size_t dimSize,
                             NcxxGroup &group)
{
  NcxxDim dim = group.getDim(name, NcxxGroup::All);
  if (dim.isNull()) {
    dim = group.addDim(name, dimSize);
  }
  return dim;
}

/////////////////////////////////////////
// set up variables

void TitanFile::_setUpVars()

{

  // top level
  
  _topLevelVars.file_time = _getVar(FILE_TIME, NcxxType::nc_INT64, _ncFile);
  _topLevelVars.start_time = _getVar(START_TIME, NcxxType::nc_INT64, _ncFile);
  _topLevelVars.end_time = _getVar(END_TIME, NcxxType::nc_INT64, _ncFile);
  _topLevelVars.n_scans = _getVar(N_SCANS, NcxxType::nc_INT, _ncFile);
  _topLevelVars.n_storms = _getVar(N_STORMS, NcxxType::nc_INT, _ncFile);
  _topLevelVars.n_simple = _getVar(N_SIMPLE, NcxxType::nc_INT, _ncFile);
  _topLevelVars.n_complex = _getVar(N_COMPLEX, NcxxType::nc_INT, _ncFile);

  // scans

  _scanVars.scan_min_z = _getVar(SCAN_MIN_Z, NcxxType::nc_FLOAT, _n_scans, _scansGroup);
  _scanVars.scan_delta_z = _getVar(SCAN_DELTA_Z, NcxxType::nc_FLOAT, _n_scans, _scansGroup);
  _scanVars.scan_num = _getVar(SCAN_NUM, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.scan_nstorms = _getVar(SCAN_NSTORMS, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.scan_time = _getVar(SCAN_TIME, NcxxType::nc_INT64, _n_scans, _scansGroup);
  _scanVars.scan_gprops_offset = _getVar(SCAN_GPROPS_OFFSET, NcxxType::nc_INT64, _n_scans, _scansGroup);
  _scanVars.scan_first_offset = _getVar(SCAN_FIRST_OFFSET, NcxxType::nc_INT64, _n_scans, _scansGroup);
  _scanVars.scan_last_offset = _getVar(SCAN_LAST_OFFSET, NcxxType::nc_INT64, _n_scans, _scansGroup);
  _scanVars.scan_ht_of_freezing = _getVar(SCAN_HT_OF_FREEZING, NcxxType::nc_FLOAT, _n_scans, _scansGroup);

  _scanVars.grid_nx = _getVar(GRID_NX, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.grid_ny = _getVar(GRID_NY, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.grid_nz = _getVar(GRID_NZ, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.grid_minx = _getVar(GRID_MINX, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_miny = _getVar(GRID_MINY, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_minz = _getVar(GRID_MINZ, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_dx = _getVar(GRID_DX, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_dy = _getVar(GRID_DY, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_dz = _getVar(GRID_DZ, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_dz_constant = _getVar(GRID_DZ_CONSTANT, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.grid_sensor_x = _getVar(GRID_SENSOR_X, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_sensor_y = _getVar(GRID_SENSOR_Y, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_sensor_z = _getVar(GRID_SENSOR_Z, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_sensor_lat = _getVar(GRID_SENSOR_LAT, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_sensor_lon = _getVar(GRID_SENSOR_LON, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.grid_unitsx = _getVar(GRID_UNITSX, NcxxType::nc_STRING, _n_scans, _scansGroup);
  _scanVars.grid_unitsy = _getVar(GRID_UNITSY, NcxxType::nc_STRING, _n_scans, _scansGroup);
  _scanVars.grid_unitsz = _getVar(GRID_UNITSZ, NcxxType::nc_STRING, _n_scans, _scansGroup);

  _scanVars.proj_type = _getVar(PROJ_TYPE, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.proj_origin_lat = _getVar(PROJ_ORIGIN_LAT, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_origin_lon = _getVar(PROJ_ORIGIN_LON, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_rotation = _getVar(PROJ_ROTATION, NcxxType::nc_FLOAT, _n_scans, _scansGroup);
  _scanVars.proj_lat1 = _getVar(PROJ_LAT1, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_lat2 = _getVar(PROJ_LAT2, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_tangent_lat = _getVar(PROJ_TANGENT_LAT, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_tangent_lon = _getVar(PROJ_TANGENT_LON, NcxxType::nc_DOUBLE, _n_scans, _scansGroup);
  _scanVars.proj_pole_type = _getVar(PROJ_POLE_TYPE, NcxxType::nc_INT, _n_scans, _scansGroup);
  _scanVars.proj_central_scale = _getVar(PROJ_CENTRAL_SCALE, NcxxType::nc_FLOAT, _n_scans, _scansGroup);

  // storm params

  _sparamsVars.low_dbz_threshold = _getVar(LOW_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.high_dbz_threshold = _getVar(HIGH_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.dbz_hist_interval = _getVar(DBZ_HIST_INTERVAL, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_dbz_threshold = _getVar(HAIL_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.base_threshold = _getVar(BASE_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.top_threshold = _getVar(TOP_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.min_storm_size = _getVar(MIN_STORM_SIZE, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.max_storm_size = _getVar(MAX_STORM_SIZE, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.morphology_erosion_threshold =
    _getVar(MORPHOLOGY_EROSION_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.morphology_refl_divisor =
    _getVar(MORPHOLOGY_REFL_DIVISOR, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.min_radar_tops = _getVar(MIN_RADAR_TOPS, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.tops_edge_margin = _getVar(TOPS_EDGE_MARGIN, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_p_coeff = _getVar(Z_P_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_p_exponent = _getVar(Z_P_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_m_coeff = _getVar(Z_M_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_m_exponent = _getVar(Z_M_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_vert_aspect = _getVar(SECTRIP_VERT_ASPECT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_horiz_aspect = _getVar(SECTRIP_HORIZ_ASPECT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_orientation_error =
    _getVar(SECTRIP_ORIENTATION_ERROR, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.poly_start_az = _getVar(POLY_START_AZ, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.poly_delta_az = _getVar(POLY_DELTA_AZ, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.check_morphology = _getVar(CHECK_MORPHOLOGY, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.check_tops = _getVar(CHECK_TOPS, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.vel_available = _getVar(VEL_AVAILABLE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.n_poly_sides = _getVar(N_POLY_SIDES_, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.ltg_count_time = _getVar(LTG_COUNT_TIME, NcxxType::nc_INT64, _stormsGroup);
  _sparamsVars.ltg_count_margin_km = _getVar(LTG_COUNT_MARGIN_KM, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_z_m_coeff = _getVar(HAIL_Z_M_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_z_m_exponent = _getVar(HAIL_Z_M_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_mass_dbz_threshold = _getVar(HAIL_MASS_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.gprops_union_type = _getVar(GPROPS_UNION_TYPE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.tops_dbz_threshold = _getVar(TOPS_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.precip_computation_mode = _getVar(PRECIP_COMPUTATION_MODE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.precip_plane_ht = _getVar(PRECIP_PLANE_HT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.low_convectivity_threshold =
    _getVar(LOW_CONVECTIVITY_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.high_convectivity_threshold =
    _getVar(HIGH_CONVECTIVITY_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);

  // storm global props

  _gpropsVars.vol_centroid_x = _getVar(VOL_CENTROID_X, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.vol_centroid_y = _getVar(VOL_CENTROID_Y, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.vol_centroid_z = _getVar(VOL_CENTROID_Z, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.refl_centroid_x = _getVar(REFL_CENTROID_X, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.refl_centroid_y = _getVar(REFL_CENTROID_Y, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.refl_centroid_z = _getVar(REFL_CENTROID_Z, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.top = _getVar(TOP, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.base = _getVar(BASE, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.volume = _getVar(VOLUME, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.area_mean = _getVar(AREA_MEAN, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_flux = _getVar(PRECIP_FLUX, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.mass = _getVar(MASS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.tilt_angle = _getVar(TILT_ANGLE, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.tilt_dirn = _getVar(TILT_DIRN, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.dbz_max = _getVar(DBZ_MAX, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.dbz_mean = _getVar(DBZ_MEAN, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.dbz_max_gradient = _getVar(DBZ_MAX_GRADIENT, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.dbz_mean_gradient = _getVar(DBZ_MEAN_GRADIENT, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.ht_of_dbz_max = _getVar(HT_OF_DBZ_MAX, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.rad_vel_mean = _getVar(RAD_VEL_MEAN, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.rad_vel_sd = _getVar(RAD_VEL_SD, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.vorticity = _getVar(VORTICITY, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area = _getVar(PRECIP_AREA, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area_centroid_x =
    _getVar(PRECIP_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area_centroid_y =
    _getVar(PRECIP_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area_orientation =
    _getVar(PRECIP_AREA_ORIENTATION, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area_minor_radius =
    _getVar(PRECIP_AREA_MINOR_RADIUS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.precip_area_major_radius =
    _getVar(PRECIP_AREA_MAJOR_RADIUS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area = _getVar(PROJ_AREA, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_centroid_x =
    _getVar(PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_centroid_y =
    _getVar(PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_orientation =
    _getVar(PROJ_AREA_ORIENTATION, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_minor_radius =
    _getVar(PROJ_AREA_MINOR_RADIUS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_major_radius =
    _getVar(PROJ_AREA_MAJOR_RADIUS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.proj_area_polygon =
    _getVar(PROJ_AREA_POLYGON, NcxxType::nc_FLOAT, _n_storms, _n_poly, _gpropsGroup);
  _gpropsVars.storm_num = _getVar(STORM_NUM, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.n_layers = _getVar(N_LAYERS, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.base_layer = _getVar(BASE_LAYER, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.n_dbz_intervals = _getVar(N_DBZ_INTERVALS, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.n_runs = _getVar(N_RUNS, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.n_proj_runs = _getVar(N_PROJ_RUNS, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.top_missing = _getVar(TOP_MISSING, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.range_limited = _getVar(RANGE_LIMITED, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.second_trip = _getVar(SECOND_TRIP, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_present = _getVar(HAIL_PRESENT, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.anom_prop = _getVar(ANOM_PROP, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.bounding_min_ix = _getVar(BOUNDING_MIN_IX, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.bounding_min_iy = _getVar(BOUNDING_MIN_IY, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.bounding_max_ix = _getVar(BOUNDING_MAX_IX, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.bounding_max_iy = _getVar(BOUNDING_MAX_IY, NcxxType::nc_INT, _n_storms, _gpropsGroup);
  _gpropsVars.layer_props_offset = _getVar(LAYER_PROPS_OFFSET, NcxxType::nc_INT64, _n_storms, _gpropsGroup);
  _gpropsVars.dbz_hist_offset = _getVar(DBZ_HIST_OFFSET, NcxxType::nc_INT64, _n_storms, _gpropsGroup);
  _gpropsVars.runs_offset = _getVar(RUNS_OFFSET, NcxxType::nc_INT64, _n_storms, _gpropsGroup);
  _gpropsVars.proj_runs_offset = _getVar(PROJ_RUNS_OFFSET, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.vil_from_maxz = _getVar(VIL_FROM_MAXZ, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.ltg_count = _getVar(LTG_COUNT, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.convectivity_median = _getVar(CONVECTIVITY_MEDIAN, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_FOKRcategory = _getVar(HAIL_FOKRCATEGORY, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_waldvogelProbability =
    _getVar(HAIL_WALDVOGELPROBABILITY, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_hailMassAloft = _getVar(HAIL_HAILMASSALOFT, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_vihm = _getVar(HAIL_VIHM, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_poh = _getVar(HAIL_POH, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_shi = _getVar(HAIL_SHI, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_posh = _getVar(HAIL_POSH, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  _gpropsVars.hail_mehs = _getVar(HAIL_MEHS, NcxxType::nc_FLOAT, _n_storms, _gpropsGroup);
  
  // storm layer props
  
  _lpropsVars.vol_centroid_x = _getVar(VOL_CENTROID_X, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.vol_centroid_y = _getVar(VOL_CENTROID_Y, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.refl_centroid_x = _getVar(REFL_CENTROID_X, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.refl_centroid_y = _getVar(REFL_CENTROID_Y, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.area = _getVar(AREA, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.dbz_max = _getVar(DBZ_MAX, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.dbz_mean = _getVar(DBZ_MEAN, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.mass = _getVar(MASS, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.rad_vel_mean = _getVar(RAD_VEL_MEAN, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.rad_vel_sd = _getVar(RAD_VEL_SD, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.vorticity = _getVar(VORTICITY, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);
  _lpropsVars.convectivity_median = _getVar(CONVECTIVITY_MEDIAN, NcxxType::nc_FLOAT, _n_layers, _lpropsGroup);

  _lpropsVars.vol_centroid_x.setCompression(false, true, 4);
  _lpropsVars.vol_centroid_y.setCompression(false, true, 4);
  _lpropsVars.refl_centroid_x.setCompression(false, true, 4);
  _lpropsVars.refl_centroid_y.setCompression(false, true, 4);
  _lpropsVars.area.setCompression(false, true, 4);
  _lpropsVars.dbz_max.setCompression(false, true, 4);
  _lpropsVars.dbz_mean.setCompression(false, true, 4);
  _lpropsVars.mass.setCompression(false, true, 4);
  _lpropsVars.rad_vel_mean.setCompression(false, true, 4);
  _lpropsVars.rad_vel_sd.setCompression(false, true, 4);
  _lpropsVars.vorticity.setCompression(false, true, 4);
  _lpropsVars.convectivity_median.setCompression(false, true, 4);

  // reflectivity histograms - set for compression

  _histVars.percent_volume = _getVar(PERCENT_VOLUME, NcxxType::nc_FLOAT, _n_hist, _histGroup);
  _histVars.percent_area = _getVar(PERCENT_AREA, NcxxType::nc_FLOAT, _n_hist, _histGroup);

  _histVars.percent_volume.setCompression(false, true, 4);
  _histVars.percent_area.setCompression(false, true, 4);

  // storm runs - set for compression

  _runsVars.run_ix = _getVar(RUN_IX, NcxxType::nc_INT, _n_runs, _runsGroup);
  _runsVars.run_iy = _getVar(RUN_IY, NcxxType::nc_INT, _n_runs, _runsGroup);
  _runsVars.run_iz = _getVar(RUN_IZ, NcxxType::nc_INT, _n_runs, _runsGroup);
  _runsVars.run_len = _getVar(RUN_LEN, NcxxType::nc_INT, _n_runs, _runsGroup);

  _runsVars.run_ix.setCompression(false, true, 4);
  _runsVars.run_iy.setCompression(false, true, 4);
  _runsVars.run_iz.setCompression(false, true, 4);
  _runsVars.run_len.setCompression(false, true, 4);
  
  // storm proj runs - set for compression

  _projRunsVars.run_ix = _getVar(RUN_IX, NcxxType::nc_INT, _n_proj_runs, _projRunsGroup);
  _projRunsVars.run_iy = _getVar(RUN_IY, NcxxType::nc_INT, _n_proj_runs, _projRunsGroup);
  _projRunsVars.run_iz = _getVar(RUN_IZ, NcxxType::nc_INT, _n_proj_runs, _projRunsGroup);
  _projRunsVars.run_len = _getVar(RUN_LEN, NcxxType::nc_INT, _n_proj_runs, _projRunsGroup);

  _projRunsVars.run_ix.setCompression(false, true, 4);
  _projRunsVars.run_iy.setCompression(false, true, 4);
  _projRunsVars.run_iz.setCompression(false, true, 4);
  _projRunsVars.run_len.setCompression(false, true, 4);
  
  // tracking parameters

  _tparamsVars.forecast_weights =
    _getVar(FORECAST_WEIGHTS, NcxxType::nc_FLOAT, _max_forecast_weights, _tracksGroup);
  _tparamsVars.weight_distance = _getVar(WEIGHT_DISTANCE, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.weight_delta_cube_root_volume
    = _getVar(WEIGHT_DELTA_CUBE_ROOT_VOLUME, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.merge_split_search_ratio = _getVar(MERGE_SPLIT_SEARCH_RATIO, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.max_speed = _getVar(MAX_SPEED, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.max_speed_for_valid_forecast =
    _getVar(MAX_SPEED_FOR_VALID_FORECAST, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.parabolic_growth_period = _getVar(PARABOLIC_GROWTH_PERIOD, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.smoothing_radius = _getVar(SMOOTHING_RADIUS, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.min_fraction_overlap = _getVar(MIN_FRACTION_OVERLAP, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.min_sum_fraction_overlap = _getVar(MIN_SUM_FRACTION_OVERLAP, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.scale_forecasts_by_history =
    _getVar(SCALE_FORECASTS_BY_HISTORY, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.use_runs_for_overlaps = _getVar(USE_RUNS_FOR_OVERLAPS, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.grid_type = _getVar(GRID_TYPE, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.nweights_forecast = _getVar(NWEIGHTS_FORECAST, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.forecast_type = _getVar(FORECAST_TYPE, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.max_delta_time = _getVar(MAX_DELTA_TIME, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.min_history_for_valid_forecast
    = _getVar(MIN_HISTORY_FOR_VALID_FORECAST, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.spatial_smoothing = _getVar(SPATIAL_SMOOTHING, NcxxType::nc_INT, _tracksGroup);

  // tracking state
  
  _tstateVars.tracking_valid = _getVar(TRACKING_VALID, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.tracking_modify_code = _getVar(TRACKING_MODIFY_CODE, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.n_samples_for_forecast_stats =
    _getVar(N_SAMPLES_FOR_FORECAST_STATS, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.last_scan_num = _getVar(LAST_SCAN_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_simple_track_num = _getVar(MAX_SIMPLE_TRACK_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_complex_track_num = _getVar(MAX_COMPLEX_TRACK_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_parents = _getVar(MAX_PARENTS_, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_children = _getVar(MAX_CHILDREN_, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_nweights_forecast = _getVar(MAX_NWEIGHTS_FORECAST_, NcxxType::nc_INT, _tracksGroup);

  // global bias for forecasts

  _globalBiasVars.proj_area_centroid_x =
    _getVar(BIAS_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.proj_area_centroid_y =
    _getVar(BIAS_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.vol_centroid_z = _getVar(BIAS_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.refl_centroid_z = _getVar(BIAS_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.top = _getVar(BIAS_TOP, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.dbz_max = _getVar(BIAS_DBZ_MAX, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.volume = _getVar(BIAS_VOLUME, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.precip_flux = _getVar(BIAS_PRECIP_FLUX, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.mass = _getVar(BIAS_MASS, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.proj_area = _getVar(BIAS_PROJ_AREA, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.smoothed_proj_area_centroid_x =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.smoothed_proj_area_centroid_y =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.smoothed_speed = _getVar(BIAS_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _tracksGroup);
  _globalBiasVars.smoothed_direction = _getVar(BIAS_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _tracksGroup);

  // global rmse for forecasts

  _globalRmseVars.proj_area_centroid_x =
    _getVar(RMSE_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.proj_area_centroid_y =
    _getVar(RMSE_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.vol_centroid_z = _getVar(RMSE_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.refl_centroid_z = _getVar(RMSE_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.top = _getVar(RMSE_TOP, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.dbz_max = _getVar(RMSE_DBZ_MAX, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.volume = _getVar(RMSE_VOLUME, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.precip_flux = _getVar(RMSE_PRECIP_FLUX, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.mass = _getVar(RMSE_MASS, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.proj_area = _getVar(RMSE_PROJ_AREA, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.smoothed_proj_area_centroid_x =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.smoothed_proj_area_centroid_y =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.smoothed_speed = _getVar(RMSE_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _tracksGroup);
  _globalRmseVars.smoothed_direction = _getVar(RMSE_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _tracksGroup);

  // global track verification contingency tables

  _globalVerifyVars.ellipse_forecast_n_success =
    _getVar(ELLIPSE_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _tracksGroup);
  _globalVerifyVars.ellipse_forecast_n_failure =
    _getVar(ELLIPSE_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _tracksGroup);
  _globalVerifyVars.ellipse_forecast_n_false_alarm =
    _getVar(ELLIPSE_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _tracksGroup);
  _globalVerifyVars.polygon_forecast_n_success =
    _getVar(POLYGON_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _tracksGroup);
  _globalVerifyVars.polygon_forecast_n_failure =
    _getVar(POLYGON_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _tracksGroup);
  _globalVerifyVars.polygon_forecast_n_false_alarm =
    _getVar(POLYGON_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _tracksGroup);

  // simple tracks

  _simpleVars.simple_track_num = _getVar(SIMPLE_TRACK_NUM, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.complex_track_num = _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.last_descendant_simple_track_num =
    _getVar(LAST_DESCENDANT_SIMPLE_TRACK_NUM, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.start_scan = _getVar(START_SCAN, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.end_scan = _getVar(END_SCAN, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.scan_origin = _getVar(SCAN_ORIGIN, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.start_time = _getVar(START_TIME, NcxxType::nc_INT64, _n_simple, _simpleGroup);
  _simpleVars.end_time = _getVar(END_TIME, NcxxType::nc_INT64, _n_simple, _simpleGroup);
  _simpleVars.last_descendant_end_scan =
    _getVar(LAST_DESCENDANT_END_SCAN, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.last_descendant_end_time =
    _getVar(LAST_DESCENDANT_END_TIME, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.time_origin = _getVar(TIME_ORIGIN, NcxxType::nc_INT64, _n_simple, _simpleGroup);
  _simpleVars.history_in_scans = _getVar(HISTORY_IN_SCANS, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.history_in_secs = _getVar(HISTORY_IN_SECS, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.duration_in_scans = _getVar(DURATION_IN_SCANS, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.duration_in_secs = _getVar(DURATION_IN_SECS, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.nparents = _getVar(NPARENTS, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.nchildren = _getVar(NCHILDREN, NcxxType::nc_INT, _n_simple, _simpleGroup);
  _simpleVars.parent = _getVar(PARENT, NcxxType::nc_INT, _n_simple, _max_parents, _simpleGroup);
  _simpleVars.child = _getVar(CHILD, NcxxType::nc_INT, _n_simple, _max_children, _simpleGroup);
  _simpleVars.first_entry_offset = _getVar(FIRST_ENTRY_OFFSET, NcxxType::nc_INT64, _n_simple, _simpleGroup);
  
  // complex tracks

  _complexVars.volume_at_start_of_sampling =
    _getVar(VOLUME_AT_START_OF_SAMPLING, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.volume_at_end_of_sampling =
    _getVar(VOLUME_AT_END_OF_SAMPLING, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.complex_track_num = _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.start_scan = _getVar(START_SCAN, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.end_scan = _getVar(END_SCAN, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.duration_in_scans = _getVar(DURATION_IN_SCANS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.duration_in_secs = _getVar(DURATION_IN_SECS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.start_time = _getVar(START_TIME, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.end_time = _getVar(END_TIME, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.n_simple_tracks = _getVar(N_SIMPLE_TRACKS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.n_top_missing = _getVar(N_TOP_MISSING, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.n_range_limited = _getVar(N_RANGE_LIMITED, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.start_missing = _getVar(START_MISSING, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.end_missing = _getVar(END_MISSING, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVars.n_samples_for_forecast_stats =
    _getVar(N_SAMPLES_FOR_FORECAST_STATS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);

  // bias for forecasts per complex track

  _complexBiasVars.proj_area_centroid_x =
    _getVar(BIAS_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.proj_area_centroid_y =
    _getVar(BIAS_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.vol_centroid_z = _getVar(BIAS_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.refl_centroid_z = _getVar(BIAS_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.top = _getVar(BIAS_TOP, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.dbz_max = _getVar(BIAS_DBZ_MAX, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.volume = _getVar(BIAS_VOLUME, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.precip_flux = _getVar(BIAS_PRECIP_FLUX, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.mass = _getVar(BIAS_MASS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.proj_area = _getVar(BIAS_PROJ_AREA, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.smoothed_proj_area_centroid_x =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.smoothed_proj_area_centroid_y =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.smoothed_speed = _getVar(BIAS_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexBiasVars.smoothed_direction = _getVar(BIAS_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _n_complex, _complexGroup);

  // rmse for forecasts per complex track

  _complexRmseVars.proj_area_centroid_x =
    _getVar(RMSE_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.proj_area_centroid_y =
    _getVar(RMSE_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.vol_centroid_z = _getVar(RMSE_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.refl_centroid_z = _getVar(RMSE_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.top = _getVar(RMSE_TOP, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.dbz_max = _getVar(RMSE_DBZ_MAX, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.volume = _getVar(RMSE_VOLUME, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.precip_flux = _getVar(RMSE_PRECIP_FLUX, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.mass = _getVar(RMSE_MASS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.proj_area = _getVar(RMSE_PROJ_AREA, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.smoothed_proj_area_centroid_x =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.smoothed_proj_area_centroid_y =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.smoothed_speed = _getVar(RMSE_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexRmseVars.smoothed_direction = _getVar(RMSE_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _n_complex, _complexGroup);

  // verification contingency tables per complex track

  _complexVerifyVars.ellipse_forecast_n_success =
    _getVar(ELLIPSE_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVerifyVars.ellipse_forecast_n_failure =
    _getVar(ELLIPSE_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVerifyVars.ellipse_forecast_n_false_alarm =
    _getVar(ELLIPSE_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_success =
    _getVar(POLYGON_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_failure =
    _getVar(POLYGON_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _n_complex, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_false_alarm =
    _getVar(POLYGON_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _n_complex, _complexGroup);

  // track entries

  _entryVars.time = _getVar(TIME, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.time_origin = _getVar(TIME_ORIGIN, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.scan_origin = _getVar(SCAN_ORIGIN, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.scan_num = _getVar(SCAN_NUM, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.storm_num = _getVar(STORM_NUM, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.simple_track_num = _getVar(SIMPLE_TRACK_NUM, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.complex_track_num = _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.history_in_scans = _getVar(HISTORY_IN_SCANS, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.history_in_secs = _getVar(HISTORY_IN_SECS, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.duration_in_scans = _getVar(DURATION_IN_SCANS, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.duration_in_secs = _getVar(DURATION_IN_SECS, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.forecast_valid = _getVar(FORECAST_VALID, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.prev_entry_offset = _getVar(PREV_ENTRY_OFFSET, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.this_entry_offset = _getVar(THIS_ENTRY_OFFSET, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.next_entry_offset = _getVar(NEXT_ENTRY_OFFSET, NcxxType::nc_INT, _n_entries, _entriesGroup);
  _entryVars.next_scan_entry_offset = _getVar(NEXT_SCAN_ENTRY_OFFSET, NcxxType::nc_INT, _n_entries, _entriesGroup);

  // track entry dval_dt for forecasts

  _entryDvalDtVars.proj_area_centroid_x =
    _getVar(DVAL_DT_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.proj_area_centroid_y =
    _getVar(DVAL_DT_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.vol_centroid_z = _getVar(DVAL_DT_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.refl_centroid_z = _getVar(DVAL_DT_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.top = _getVar(DVAL_DT_TOP, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.dbz_max = _getVar(DVAL_DT_DBZ_MAX, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.volume = _getVar(DVAL_DT_VOLUME, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.precip_flux = _getVar(DVAL_DT_PRECIP_FLUX, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.mass = _getVar(DVAL_DT_MASS, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.proj_area = _getVar(DVAL_DT_PROJ_AREA, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.smoothed_proj_area_centroid_x =
    _getVar(DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.smoothed_proj_area_centroid_y =
    _getVar(DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.smoothed_speed = _getVar(DVAL_DT_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);
  _entryDvalDtVars.smoothed_direction = _getVar(DVAL_DT_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _n_entries, _entriesGroup);

}

/////////////////////////////////////////
// get scalar variable

NcxxVar TitanFile::_getVar(const std::string& name,
                             const NcxxType& ncType,
                             NcxxGroup &group)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    var = group.addVar(name, ncType);
  }
  return var;
}

/////////////////////////////////////////
// get array variable

NcxxVar TitanFile::_getVar(const std::string& name,
                             const NcxxType& ncType,
                             const NcxxDim& dim,
                             NcxxGroup &group)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    var = group.addVar(name, ncType, dim);
  }
  return var;
}

/////////////////////////////////////////
// get 2D array variable

NcxxVar TitanFile::_getVar(const std::string& name,
                             const NcxxType& ncType,
                             const NcxxDim& dim0,
                             const NcxxDim& dim1,
                             NcxxGroup &group)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    std::vector<NcxxDim> dimVec;
    dimVec.push_back(dim0);
    dimVec.push_back(dim1);
    var = group.addVar(name, ncType, dimVec);
  }
  return var;
}

////////////////////
// error string

void TitanFile::_clearErrStr()
{
  _errStr = "";
  _addErrStr("ERROR at time: ", DateTime::str());
}

///////////////////////////////////////////////
// add labelled integer value to error string,
// with optional following carriage return.

void TitanFile::_addErrInt(string label,
                             int iarg, bool cr)
{
  _errStr += label;
  char str[32];
  sprintf(str, "%d", iarg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

///////////////////////////////////////////////
// add labelled double value to error string,
// with optional following carriage return.
// Default format is %g.

void TitanFile::_addErrDbl(string label, double darg,
                             string format, bool cr)
  
{
  _errStr += label;
  char str[128];
  sprintf(str, format.c_str(), darg);
  _errStr += str;
  if (cr) {
    _errStr += "\n";
  }
}

////////////////////////////////////////
// add labelled string to error string
// with optional following carriage return.

void TitanFile::_addErrStr(string label,
                             string strarg, bool cr)

{
  _errStr += label;
  _errStr += strarg;
  if (cr) {
    _errStr += "\n";
  }
}

//////////////////////////////////////////////////////////////
//
// allocate space for the layer props
//
//////////////////////////////////////////////////////////////

void TitanFile::allocLayers(int n_layers)
     
{
  if (n_layers > _max_layers) {
    _max_layers = n_layers;
    _lprops = (storm_file_layer_props_t *)
      urealloc(_lprops, n_layers * sizeof(storm_file_layer_props_t));
    memset(_lprops, 0, n_layers * sizeof(storm_file_layer_props_t));
  }
}

void TitanFile::freeLayers()
     
{
  if (_lprops) {
    ufree(_lprops);
    _lprops = nullptr;
    _max_layers = 0;
  }
}

//////////////////////////////////////////////////////////////
//
// allocate space for dbz hist
//
//////////////////////////////////////////////////////////////

void TitanFile::allocHist(int n_dbz_intervals)
     
{

  if (n_dbz_intervals > _max_dbz_intervals) {
    _max_dbz_intervals = n_dbz_intervals;
    _hist = (storm_file_dbz_hist_t *)
      urealloc(_hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
    memset(_hist, 0, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  }

}

void TitanFile::freeHist()
     
{

  if (_hist) {
    ufree (_hist);
    _hist = nullptr;
    _max_dbz_intervals = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the runs
//
//////////////////////////////////////////////////////////////

void TitanFile::allocRuns(int n_runs)
     
{

  if (n_runs > _max_runs) {
    _max_runs = n_runs;
    _runs = (storm_file_run_t *)
      urealloc(_runs, n_runs * sizeof(storm_file_run_t));
    memset(_runs, 0, n_runs * sizeof(storm_file_run_t));
  }

}

void TitanFile::freeRuns()
     
{

  if (_runs) {
    ufree ((char *) _runs);
    _runs = nullptr;
    _max_runs = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the proj runs
//
//////////////////////////////////////////////////////////////

void TitanFile::allocProjRuns(int n_proj_runs)
     
{

  if (n_proj_runs > _max_proj_runs) {
    _max_proj_runs = n_proj_runs;
    _proj_runs = (storm_file_run_t *)
      urealloc((char *) _proj_runs,
	       n_proj_runs * sizeof(storm_file_run_t));
  }

}

void TitanFile::freeProjRuns()
     
{

  if (_proj_runs) {
    ufree ((char *) _proj_runs);
    _proj_runs = nullptr;
    _max_proj_runs = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate gprops array
//
//////////////////////////////////////////////////////////////

void TitanFile::allocGprops(int nstorms)
     
{
  
  if (nstorms > _max_storms) {
    _max_storms = nstorms;
    _gprops = (storm_file_global_props_t *)
      urealloc(_gprops, nstorms * sizeof(storm_file_global_props_t));
  }

}

void TitanFile::freeGprops()
     
{

  if (_gprops) {
    ufree ((char *) _gprops);
    _gprops = nullptr;
    _max_storms = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// allocate space for the scan offset array.
//
//////////////////////////////////////////////////////////////

void TitanFile::allocScanOffsets(int n_scans_needed)
     
{

  // allocate the required space plus a buffer so that 
  // we do not do too many reallocs
  
  if (n_scans_needed > _max_scans) {
    _max_scans = n_scans_needed + 100;
    _scan_offsets = (si32 *) urealloc
      (_scan_offsets, (_max_scans * sizeof(si32)));
  }

}

void TitanFile::freeScanOffsets()
     
{

  if (_scan_offsets) {
    ufree(_scan_offsets);
    _scan_offsets = nullptr;
    _max_scans = 0;
  }

}

//////////////////////////////////////////////////////////////
//
// free all arrays
//
//////////////////////////////////////////////////////////////

void TitanFile::freeStormsAll()
     
{

  freeLayers();
  freeHist();
  freeRuns();
  freeProjRuns();
  freeGprops();
  freeScanOffsets();

}


//////////////////////////////////////////////////////////////
//
// Opens the storm header and data files
//
// The storm header file path must have been set
//
//////////////////////////////////////////////////////////////

int TitanFile::openStormFiles(const char *mode,
                                const char *header_file_path,
                                const char *data_file_ext /* = nullptr*/ )
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::OpenStormFiles\n";

  // close files
  
  closeStormFiles();
  
  // open the header file - file path may change if it is compressed
  
  char hdr_file_path[MAX_PATH_LEN];
  STRncopy(hdr_file_path, header_file_path, MAX_PATH_LEN);
  if ((_storm_header_file = ta_fopen_uncompress(hdr_file_path, mode)) == nullptr) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open header file: ",
		  header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _storm_header_file_path = hdr_file_path;
  
  // compute the data file name
   
  if (*mode == 'r') {

    // read the header if the file is opened for reading
    
    if (readStormHeader(false)) {
      return -1;
    }

    // compute the file path from the header file path and
    // the data file name
    
    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, _storm_header_file_path.c_str(), MAX_PATH_LEN - 1);
    
    // if dir path has slash, get pointer to that and end the sting
    // immediately after
    
    char *chptr;
    if ((chptr = strrchr(tmp_path, '/')) != nullptr) {
      *(chptr + 1) = '\0';
      _storm_data_file_path = tmp_path;
      _storm_data_file_path += _storm_header.data_file_name;
    } else {
      _storm_data_file_path = _storm_header.data_file_name;
    }
    
  } else {
    
    // file opened for writing, use ext to compute file name
    
    if (data_file_ext == nullptr) {
      _errStr += "Must provide data file extension for file creation\n";
      return -1;
    }

    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, _storm_header_file_path.c_str(), MAX_PATH_LEN - 1);

    char *chptr;
    if ((chptr = strrchr(tmp_path, '.')) == nullptr) {
      TaStr::AddStr(_errStr, "  Header file must have extension : ",
		    _storm_header_file_path);
      return -1;
    }
    
    *(chptr + 1) = '\0';
    _storm_data_file_path = tmp_path;
    _storm_data_file_path += data_file_ext;

  } // if (*mode == 'r') 
    
  // open the data file - file path may change if it is compressed
  
  char dat_file_path[MAX_PATH_LEN];
  STRncopy(dat_file_path, _storm_data_file_path.c_str(), MAX_PATH_LEN);
    
  if ((_storm_data_file = ta_fopen_uncompress(dat_file_path, mode)) == nullptr) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open storm data file: ",
		  _storm_data_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _storm_data_file_path = dat_file_path;

  // In write mode, write file labels
  
  if (*mode == 'w') {
    
    // header file
    
    char header_file_label[R_FILE_LABEL_LEN];
    MEM_zero(header_file_label);
    strcpy(header_file_label, _storm_header_file_label.c_str());
    
    if (ufwrite(header_file_label, 1, R_FILE_LABEL_LEN,
		_storm_header_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing header file label to: ",
		    _storm_header_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

    // data file
    
    char data_file_label[R_FILE_LABEL_LEN];
    MEM_zero(data_file_label);
    strcpy(data_file_label, STORM_DATA_FILE_TYPE);
    _storm_data_file_label = data_file_label;
    
    if (ufwrite(data_file_label, 1, R_FILE_LABEL_LEN,
		_storm_data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing data file label to: ",
		    _storm_data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } else {
    
    // read mode - read in data file label
    
    char data_file_label[R_FILE_LABEL_LEN];
    if (ufread(data_file_label, sizeof(char), R_FILE_LABEL_LEN,
	       _storm_data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Reading data file label from: ",
		    _storm_data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // check label
    
    if (_storm_data_file_label != data_file_label) {
      _errStr +=
	"  Data file does not have the correct label\n";
      TaStr::AddStr(_errStr, "  File label is: ", data_file_label);
      TaStr::AddStr(_errStr, "  Should be: ", _storm_data_file_label);
      return -1;
    }
    
  } // if (*mode == 'w') 

  return 0;

}

//////////////////////////////////////////////////////////////
//
// closes the storm header and data files
//
//////////////////////////////////////////////////////////////

void TitanFile::closeStormFiles()
     
{

  // unlock the header file

  unlockStormHeaderFile();

  // close the header file
  
  if (_storm_header_file != nullptr) {
    fclose(_storm_header_file);
    _storm_header_file = (FILE *) nullptr;
  }

  // close the data file
   
  if (_storm_data_file != nullptr) {
    fclose(_storm_data_file);
    _storm_data_file = (FILE *) nullptr;
  }
  
}

//////////////////////////////////////////////////////////////
//
// Flush the storm header and data files
//
//////////////////////////////////////////////////////////////

void TitanFile::flushStormFiles()
  
{

  fflush(_storm_header_file);
  fflush(_storm_data_file);

}

//////////////////////////////////////////////////////////////
//
// Put an advisory lock on the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::lockStormHeaderFile(const char *mode)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::lockStormHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_header_file_path);
  
  if (ta_lock_file_procmap(_storm_header_file_path.c_str(),
			   _storm_header_file, mode)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot lock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// Remove advisory lock from the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::unlockStormHeaderFile()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::unlockStormHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_header_file_path);
  
  if (ta_unlock_file(_storm_header_file_path.c_str(),
		     _storm_header_file)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot unlock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// reads in the storm_file_header_t structure from a storm
// properties file.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::readStormHeader(bool clear_error_str /* = true*/ )
     
{

  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanFile::readStormHeader\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _storm_header_file_path);

  // rewind file
  
  fseek(_storm_header_file, 0L, SEEK_SET);
  
  // read in header file label
  
  char header_file_label[R_FILE_LABEL_LEN];
  if (ufread(header_file_label, sizeof(char), R_FILE_LABEL_LEN,
	     _storm_header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading header file label from: ",
		  _storm_header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // check label
  
  if (_storm_header_file_label != header_file_label) {
    _errStr +=
      "  Header file does not contain correct label.\n";
    TaStr::AddStr(_errStr, "  File label is: ", header_file_label);
    TaStr::AddStr(_errStr, "  Should be: ", _storm_header_file_label);
    return -1;
  }
    
  // read in header
  
  if (ufread(&_storm_header, sizeof(storm_file_header_t),
	     1, _storm_header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading storm file header structure");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _storm_header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_storm_header, (sizeof(storm_file_header_t) - nbytes_char));
  
  // allocate space for scan offsets array
  
  int n_scans = _storm_header.n_scans;
  
  allocScanOffsets(n_scans);
  
  // read in scan offsets
  
  if (ufread(_scan_offsets, sizeof(si32), n_scans,
	     _storm_header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading storm file scan offsets");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the offset array from network byte order into host byte order
  
  BE_to_array_32(_scan_offsets, n_scans * sizeof(si32));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// read in the storm projected area runs
// Space for the array is allocated.
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::readProjRuns(int storm_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::readProjRuns\n";

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }
  
  // store storm number
  
  _storm_num = storm_num;
  
  // allocate mem
  
  int n_proj_runs = _gprops[storm_num].n_proj_runs;

  allocProjRuns(n_proj_runs);
  
  // move to proj_run data position in file
  
  fseek(_storm_data_file, _gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  // read in proj_runs
  
  if (ufread(_proj_runs, sizeof(storm_file_run_t), n_proj_runs,
	     _storm_data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading proj runs, file: ", _storm_data_file_path);
    TaStr::AddInt(_errStr, "  N runs: ", n_proj_runs);
    TaStr::AddInt(_errStr, "  Storm number: ", storm_num);
    TaStr::AddInt(_errStr, "  Scan number: ", _scan.scan_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode proj_runs from network byte order into host byte order
  
  BE_to_array_16(_proj_runs, n_proj_runs * sizeof(storm_file_run_t));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// read in the storm property data for a given storm in a scan
// Space for the arrays of structures is allocated as required.
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::readProps(int storm_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::readProps\n";
  TaStr::AddStr(_errStr, "  Reading storm props from file: ", _storm_data_file_path);
  TaStr::AddInt(_errStr, "  Storm number: ", storm_num);
  TaStr::AddInt(_errStr, "  Scan number: ", _scan.scan_num);
  
  // store storm number
  
  _storm_num = storm_num;
  
  // allocate or realloc mem
  
  int n_layers = _gprops[storm_num].n_layers;
  int n_dbz_intervals = _gprops[storm_num].n_dbz_intervals;
  int n_runs = _gprops[storm_num].n_runs;
  int n_proj_runs = _gprops[storm_num].n_proj_runs;

  allocLayers(n_layers);
  allocHist(n_dbz_intervals);
  allocRuns(n_runs);
  allocProjRuns(n_proj_runs);

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }
  
  // move to layer data position in file
  
  fseek(_storm_data_file, _gprops[storm_num].layer_props_offset, SEEK_SET);
  
  // read in layer props
  
  if (ufread(_lprops, sizeof(storm_file_layer_props_t),
	     n_layers, _storm_data_file) != n_layers) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading layer props");
    TaStr::AddInt(_errStr, "  N layers: ", n_layers);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode layer props from network byte order into host byte order
  
  BE_to_array_32(_lprops, n_layers * sizeof(storm_file_layer_props_t));
  
  // move to hist data position in file
  
  fseek(_storm_data_file, _gprops[storm_num].dbz_hist_offset, SEEK_SET);
  
  // read in histogram data
  
  if (ufread(_hist, sizeof(storm_file_dbz_hist_t),
	     n_dbz_intervals, _storm_data_file) != n_dbz_intervals) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading dbz histogram");
    TaStr::AddInt(_errStr, "  N intervals: ", n_dbz_intervals);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode histogram data from network byte order into host byte order
  
  BE_to_array_32(_hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // move to run data position in file
  
  fseek(_storm_data_file, _gprops[storm_num].runs_offset, SEEK_SET);
  
  // read in runs
  
  if (ufread(_runs, sizeof(storm_file_run_t),
	     n_runs, _storm_data_file) != n_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading runs");
    TaStr::AddInt(_errStr, "  N runs: ", n_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode runs from network byte order into host byte order
  
  BE_to_array_16(_runs, n_runs * sizeof(storm_file_run_t));
  
  // move to proj_run data position in file
  
  fseek(_storm_data_file, _gprops[storm_num].proj_runs_offset, SEEK_SET);
  
  // read in proj_runs
  
  if (ufread(_proj_runs, sizeof(storm_file_run_t),
	     n_proj_runs, _storm_data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading proj runs");
    TaStr::AddInt(_errStr, "  N proj runs: ", n_proj_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // decode proj_runs from network byte order into host byte order
  
  BE_to_array_16(_proj_runs, n_proj_runs * sizeof(storm_file_run_t));
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// Read in the scan info for a particular scan in a storm properties
// file.
//
// If storm num is set, only the gprops for that storm is swapped
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::readScan(int scan_num, int storm_num /* = -1*/ )
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::readScan\n";
  TaStr::AddStr(_errStr, "  Reading scan from file: ", _storm_data_file_path);
  TaStr::AddInt(_errStr, "  Scan number: ", scan_num);

  // move to scan position in file
  
  if (_scan_offsets && scan_num < _max_scans) {
    fseek(_storm_data_file, _scan_offsets[scan_num], SEEK_SET);
  } else {
    return -1;
  }
  
  // read in scan struct
  
  storm_file_scan_header_t scan;
  if (ufread(&scan, sizeof(storm_file_scan_header_t),
	     1, _storm_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the scan struct from network byte order into host byte order
  
  si32 nbytes_char = scan.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&scan, (sizeof(storm_file_scan_header_t) - nbytes_char));
  
  // allocate or reallocate
  
  int nstorms = scan.nstorms;
  allocGprops(nstorms);
  
  // copy scan header into storm file index

  _scan = scan;
  
  // return early if nstorms is zero
  
  if (nstorms == 0) {
    return 0;
  }
  
  // move to gprops position in file
  
  fseek(_storm_data_file, _scan.gprops_offset, SEEK_SET);
  
  // read in global props
  
  if (ufread(_gprops, sizeof(storm_file_global_props_t),
	     nstorms, _storm_data_file) != nstorms) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading gprops");
    TaStr::AddInt(_errStr, "  nstorms: ", nstorms);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode global props from network byte order into host byte order

  if (storm_num >= 0) {
    BE_to_array_32(_gprops + storm_num, sizeof(storm_file_global_props_t));
  } else {
    BE_to_array_32(_gprops, nstorms * sizeof(storm_file_global_props_t));
  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// seek to the end of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanFile::seekStormEndData()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::seekStormEndData\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_data_file_path);

  if (fseek(_storm_data_file, 0L, SEEK_END)) {

    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;

  } else {

    return 0;

  }

}

//////////////////////////////////////////////////////////////
//
// seek to the start of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanFile::seekStormStartData()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::seekStormStartData\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_data_file_path);

  if (fseek(_storm_data_file, R_FILE_LABEL_LEN, SEEK_SET) != 0) {
    
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;

  } else {

    return 0;

  }

}

//////////////////////////////////////////////////////////////
//
// write the storm_file_header_t structure to a storm file.
//
// NOTE: should be called after writeSecProps() and writeScan(),
// so that appropriate n_scans can be determined.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::writeStormHeader(const storm_file_header_t &storm_file_header)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeStormHeader\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_header_file_path);
  
  storm_file_header_t header = storm_file_header;
  
  header.major_rev = STORM_FILE_MAJOR_REV;
  header.minor_rev = STORM_FILE_MINOR_REV;
  
  // set file time to gmt
  
  header.file_time = time(nullptr);
  
  // make local copies of the global file header and scan offsets
  
  _topLevelVars.file_time.putVal((int64_t) header.file_time);
  _topLevelVars.start_time.putVal((int64_t) header.start_time);
  _topLevelVars.end_time.putVal((int64_t) header.end_time);
  _topLevelVars.n_scans.putVal((int) _n_scans.getSize());
  _topLevelVars.n_storms.putVal((int) _n_storms.getSize());
  
  const storm_file_params_t &sparams(header.params);
  
  _sparamsVars.low_dbz_threshold.putVal(sparams.low_dbz_threshold);
  _sparamsVars.high_dbz_threshold.putVal(sparams.high_dbz_threshold);
  _sparamsVars.dbz_hist_interval.putVal(sparams.dbz_hist_interval);
  _sparamsVars.hail_dbz_threshold.putVal(sparams.hail_dbz_threshold);
  _sparamsVars.base_threshold.putVal(sparams.base_threshold);
  _sparamsVars.top_threshold.putVal(sparams.top_threshold);
  _sparamsVars.min_storm_size.putVal(sparams.min_storm_size);
  _sparamsVars.max_storm_size.putVal(sparams.max_storm_size);
  _sparamsVars.morphology_erosion_threshold.putVal(sparams.morphology_erosion_threshold);
  _sparamsVars.morphology_refl_divisor.putVal(sparams.morphology_refl_divisor);
  _sparamsVars.min_radar_tops.putVal(sparams.min_radar_tops);
  _sparamsVars.tops_edge_margin.putVal(sparams.tops_edge_margin);
  _sparamsVars.z_p_coeff.putVal(sparams.z_p_coeff);
  _sparamsVars.z_p_exponent.putVal(sparams.z_p_exponent);
  _sparamsVars.z_m_coeff.putVal(sparams.z_m_coeff);
  _sparamsVars.z_m_exponent.putVal(sparams.z_m_exponent);
  _sparamsVars.sectrip_vert_aspect.putVal(sparams.sectrip_vert_aspect);
  _sparamsVars.sectrip_horiz_aspect.putVal(sparams.sectrip_horiz_aspect);
  _sparamsVars.sectrip_orientation_error.putVal(sparams.sectrip_orientation_error);
  _sparamsVars.poly_start_az.putVal(sparams.poly_start_az);
  _sparamsVars.poly_delta_az.putVal(sparams.poly_delta_az);
  _sparamsVars.check_morphology.putVal(sparams.check_morphology);
  _sparamsVars.check_tops.putVal(sparams.check_tops);
  _sparamsVars.vel_available.putVal(sparams.vel_available);
  _sparamsVars.n_poly_sides.putVal(sparams.n_poly_sides);
  _sparamsVars.ltg_count_time.putVal(sparams.ltg_count_time);
  _sparamsVars.ltg_count_margin_km.putVal(sparams.ltg_count_margin_km);
  _sparamsVars.hail_z_m_coeff.putVal(sparams.hail_z_m_coeff);
  _sparamsVars.hail_z_m_exponent.putVal(sparams.hail_z_m_exponent);
  _sparamsVars.hail_mass_dbz_threshold.putVal(sparams.hail_mass_dbz_threshold);
  _sparamsVars.gprops_union_type.putVal(sparams.gprops_union_type);
  _sparamsVars.tops_dbz_threshold.putVal(sparams.tops_dbz_threshold);
  _sparamsVars.precip_computation_mode.putVal(sparams.precip_computation_mode);
  _sparamsVars.precip_plane_ht.putVal(sparams.precip_plane_ht);
  _sparamsVars.low_convectivity_threshold.putVal(sparams.low_convectivity_threshold);
  _sparamsVars.high_convectivity_threshold.putVal(sparams.high_convectivity_threshold);

  // TaArray<si32> offsetArray;
  // si32 *scan_offsets = offsetArray.alloc(n_scans);
  // memcpy (scan_offsets, _scan_offsets, n_scans * sizeof(si32));
  
  // encode the header and scan offset array into network byte order
  
  // ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  // ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  // header.nbytes_char = STORM_FILE_HEADER_NBYTES_CHAR;
  // BE_from_array_32(&header,
  //       	   (sizeof(storm_file_header_t) - header.nbytes_char));
  // BE_from_array_32(scan_offsets, n_scans * sizeof(si32));
  
  // write label to file
  
  // fseek(_storm_header_file, 0, SEEK_SET);
  // ustr_clear_to_end(file_label, R_FILE_LABEL_LEN);

  // if (ufwrite(file_label, sizeof(char), R_FILE_LABEL_LEN,
  //             _storm_header_file) != R_FILE_LABEL_LEN) {
  //   int errNum = errno;
  //   TaStr::AddStr(_errStr, "  ", "Writing label");
  //   TaStr::AddStr(_errStr, "  ", strerror(errNum));
  //   return -1;
  // }
  
  // // write header to file
  
  // if (ufwrite(&header, sizeof(storm_file_header_t),
  //             1, _storm_header_file) != 1) {
  //   int errNum = errno;
  //   TaStr::AddStr(_errStr, "  ", "Writing header");
  //   TaStr::AddStr(_errStr, "  ", strerror(errNum));
  //   return -1;
  // }
  
  // // write scan offsets to file
  
  // if (ufwrite(scan_offsets, sizeof(si32),
  //             n_scans, _storm_header_file) != n_scans) {
  //   int errNum = errno;
  //   TaStr::AddStr(_errStr, "  ", "Writing scan offsets");
  //   TaStr::AddStr(_errStr, "  ", strerror(errNum));
  //   return -1;
  // }
  
  // flush the file buffer
  
  // flushStormFiles();

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// write scan header and storm global properties
// for a particular scan.
//
// NOTE: writeSecProps() must be called first, so that
// the appropriate offsets can be set.
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::writeStormScan(const storm_file_header_t &storm_file_header,
                              const storm_file_scan_header_t &scanHeader,
                              const storm_file_global_props_t *gprops)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeScan\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_data_file_path);
  
  // scan storage index
  
  size_t scanNum = scanHeader.scan_num;
  std::vector<size_t> scanIndex;
  scanIndex.push_back(scanNum);

  // write scan details
  
  _scanVars.scan_min_z.putVal(scanIndex, scanHeader.min_z);
  _scanVars.scan_delta_z.putVal(scanIndex, scanHeader.delta_z);
  _scanVars.scan_num.putVal(scanIndex, scanHeader.scan_num);
  _scanVars.scan_nstorms.putVal(scanIndex, scanHeader.nstorms);
  _scanVars.scan_time.putVal(scanIndex, scanHeader.time);
  _scanVars.scan_ht_of_freezing.putVal(scanIndex, scanHeader.ht_of_freezing);

  // write grid details

  _scanVars.grid_nx.putVal(scanIndex, scanHeader.grid.nx);
  _scanVars.grid_ny.putVal(scanIndex, scanHeader.grid.ny);
  _scanVars.grid_nz.putVal(scanIndex, scanHeader.grid.nz);
  _scanVars.grid_minx.putVal(scanIndex, scanHeader.grid.minx);
  _scanVars.grid_miny.putVal(scanIndex, scanHeader.grid.miny);
  _scanVars.grid_minz.putVal(scanIndex, scanHeader.grid.minz);
  _scanVars.grid_dx.putVal(scanIndex, scanHeader.grid.dx);
  _scanVars.grid_dy.putVal(scanIndex, scanHeader.grid.dy);
  _scanVars.grid_dz.putVal(scanIndex, scanHeader.grid.dz);
  _scanVars.grid_dz_constant.putVal(scanIndex, scanHeader.grid.dz_constant);
  _scanVars.grid_sensor_x.putVal(scanIndex, scanHeader.grid.sensor_x);
  _scanVars.grid_sensor_y.putVal(scanIndex, scanHeader.grid.sensor_y);
  _scanVars.grid_sensor_z.putVal(scanIndex, scanHeader.grid.sensor_z);
  _scanVars.grid_sensor_lat.putVal(scanIndex, scanHeader.grid.sensor_lat);
  _scanVars.grid_sensor_lon.putVal(scanIndex, scanHeader.grid.sensor_lon);
  
  _scanVars.grid_unitsx.putVal(scanIndex, std::string(scanHeader.grid.unitsx));
  _scanVars.grid_unitsy.putVal(scanIndex, std::string(scanHeader.grid.unitsy));
  _scanVars.grid_unitsz.putVal(scanIndex, std::string(scanHeader.grid.unitsz));

  // write projection details
  
  _scanVars.proj_type.putVal(scanIndex, scanHeader.grid.proj_type);
  _scanVars.proj_origin_lat.putVal(scanIndex, scanHeader.grid.proj_origin_lat);
  _scanVars.proj_origin_lon.putVal(scanIndex, scanHeader.grid.proj_origin_lon);
  _scanVars.proj_rotation.putVal(scanIndex, scanHeader.grid.proj_params.flat.rotation);

  _scanVars.proj_lat1.putVal(scanIndex, scanHeader.grid.proj_params.lc2.lat1);
  _scanVars.proj_lat2.putVal(scanIndex, scanHeader.grid.proj_params.lc2.lat2);
  _scanVars.proj_tangent_lat.putVal(scanIndex, 0.0);
  _scanVars.proj_tangent_lon.putVal(scanIndex, 0.0);
  _scanVars.proj_pole_type.putVal(scanIndex, 0);
  _scanVars.proj_central_scale.putVal(scanIndex, 1.0);

  // write gprops offset
  
  _scanVars.scan_gprops_offset.putVal(scanIndex, (int64_t) _n_storms.getSize());

  // write storm global props

  const storm_file_params_t &sparams(storm_file_header.params);

  for (int istorm = 0; istorm < scanHeader.nstorms; istorm++) {

    // write first and last gprops offsets for this scan
    // first_offset is the offset of the first storm in the scan
    // last_offset is the offset of the last storm in the scan
    // NOTE: last_offset is the offset OF the last storm, NOT one beyond
    
    if (istorm == 0) {
      _scanVars.scan_first_offset.putVal(scanIndex, (int64_t) _n_storms.getSize());
    }
    _scanVars.scan_last_offset.putVal(scanIndex, (int64_t) _n_storms.getSize());

    // write the global props
    
    const storm_file_global_props_t &gp = gprops[istorm];
    
    int gpropsOffset = _n_storms.getSize();
    std::vector<size_t> stormIndex;
    stormIndex.push_back(gpropsOffset);
    
    _gpropsVars.vol_centroid_x.putVal(stormIndex, gp.vol_centroid_x);
    _gpropsVars.vol_centroid_y.putVal(stormIndex, gp.vol_centroid_y);
    _gpropsVars.vol_centroid_z.putVal(stormIndex, gp.vol_centroid_z);
    _gpropsVars.refl_centroid_x.putVal(stormIndex, gp.refl_centroid_x);
    _gpropsVars.refl_centroid_y.putVal(stormIndex, gp.refl_centroid_y);
    _gpropsVars.refl_centroid_z.putVal(stormIndex, gp.refl_centroid_z);
    _gpropsVars.top.putVal(stormIndex, gp.top);
    _gpropsVars.base.putVal(stormIndex, gp.base);
    _gpropsVars.volume.putVal(stormIndex, gp.volume);
    _gpropsVars.area_mean.putVal(stormIndex, gp.area_mean);
    _gpropsVars.precip_flux.putVal(stormIndex, gp.precip_flux);
    _gpropsVars.mass.putVal(stormIndex, gp.mass);
    _gpropsVars.tilt_angle.putVal(stormIndex, gp.tilt_angle);
    _gpropsVars.tilt_dirn.putVal(stormIndex, gp.tilt_dirn);
    _gpropsVars.dbz_max.putVal(stormIndex, gp.dbz_max);
    _gpropsVars.dbz_mean.putVal(stormIndex, gp.dbz_mean);
    _gpropsVars.dbz_max_gradient.putVal(stormIndex, gp.dbz_max_gradient);
    _gpropsVars.dbz_mean_gradient.putVal(stormIndex, gp.dbz_mean_gradient);
    _gpropsVars.ht_of_dbz_max.putVal(stormIndex, gp.ht_of_dbz_max);
    _gpropsVars.rad_vel_mean.putVal(stormIndex, gp.rad_vel_mean);
    _gpropsVars.rad_vel_sd.putVal(stormIndex, gp.rad_vel_sd);
    _gpropsVars.vorticity.putVal(stormIndex, gp.vorticity);
    _gpropsVars.precip_area.putVal(stormIndex, gp.precip_area);
    _gpropsVars.precip_area_centroid_x.putVal(stormIndex, gp.precip_area_centroid_x);
    _gpropsVars.precip_area_centroid_y.putVal(stormIndex, gp.precip_area_centroid_y);
    _gpropsVars.precip_area_orientation.putVal(stormIndex, gp.precip_area_orientation);
    _gpropsVars.precip_area_minor_radius.putVal(stormIndex, gp.precip_area_minor_radius);
    _gpropsVars.precip_area_major_radius.putVal(stormIndex, gp.precip_area_major_radius);
    _gpropsVars.proj_area.putVal(stormIndex, gp.proj_area);
    _gpropsVars.proj_area_centroid_x.putVal(stormIndex, gp.proj_area_centroid_x);
    _gpropsVars.proj_area_centroid_y.putVal(stormIndex, gp.proj_area_centroid_y);
    _gpropsVars.proj_area_orientation.putVal(stormIndex, gp.proj_area_orientation);
    _gpropsVars.proj_area_minor_radius.putVal(stormIndex, gp.proj_area_minor_radius);
    _gpropsVars.proj_area_major_radius.putVal(stormIndex, gp.proj_area_major_radius);

    _gpropsVars.storm_num.putVal(stormIndex, gp.storm_num);
    _gpropsVars.n_layers.putVal(stormIndex, gp.n_layers);
    _gpropsVars.base_layer.putVal(stormIndex, gp.base_layer);
    _gpropsVars.n_dbz_intervals.putVal(stormIndex, gp.n_dbz_intervals);
    _gpropsVars.n_runs.putVal(stormIndex, gp.n_runs);
    _gpropsVars.n_proj_runs.putVal(stormIndex, gp.n_proj_runs);
    _gpropsVars.top_missing.putVal(stormIndex, gp.top_missing);
    _gpropsVars.range_limited.putVal(stormIndex, gp.range_limited);
    _gpropsVars.second_trip.putVal(stormIndex, gp.second_trip);
    _gpropsVars.hail_present.putVal(stormIndex, gp.hail_present);
    _gpropsVars.anom_prop.putVal(stormIndex, gp.anom_prop);
    _gpropsVars.bounding_min_ix.putVal(stormIndex, gp.bounding_min_ix);
    _gpropsVars.bounding_min_iy.putVal(stormIndex, gp.bounding_min_iy);
    _gpropsVars.bounding_max_ix.putVal(stormIndex, gp.bounding_max_ix);
    _gpropsVars.bounding_max_iy.putVal(stormIndex, gp.bounding_max_iy);
    _gpropsVars.vil_from_maxz.putVal(stormIndex, gp.vil_from_maxz);
    _gpropsVars.ltg_count.putVal(stormIndex, gp.ltg_count);

    if (sparams.low_convectivity_threshold != 0.0 ||
        sparams.high_convectivity_threshold != 0.0) {
      _gpropsVars.convectivity_median.putVal(stormIndex, gp.convectivity_median);
    }

    if (sparams.gprops_union_type == UNION_HAIL) {
      
      _gpropsVars.hail_FOKRcategory.putVal(stormIndex, gp.add_on.hail_metrics.FOKRcategory);
      _gpropsVars.hail_waldvogelProbability.putVal(stormIndex, gp.add_on.hail_metrics.waldvogelProbability);
      _gpropsVars.hail_hailMassAloft.putVal(stormIndex, gp.add_on.hail_metrics.hailMassAloft);
      _gpropsVars.hail_vihm.putVal(stormIndex, gp.add_on.hail_metrics.vihm);

    } else if (sparams.gprops_union_type == UNION_NEXRAD_HDA) {
      
      _gpropsVars.hail_poh.putVal(stormIndex, gp.add_on.hda.poh);
      _gpropsVars.hail_shi.putVal(stormIndex, gp.add_on.hda.shi);
      _gpropsVars.hail_posh.putVal(stormIndex, gp.add_on.hda.posh);
      _gpropsVars.hail_mehs.putVal(stormIndex, gp.add_on.hda.mehs);
      
    }

    // polygons are 2D variables
    
    std::vector<size_t> polyIndex;
    polyIndex.push_back(gpropsOffset);
    polyIndex.push_back(0);
    
    std::vector<size_t> polyCount;
    polyCount.push_back(1);
    polyCount.push_back(N_POLY_SIDES);
    
    _gpropsVars.proj_area_polygon.putVal(polyIndex, polyCount, gp.proj_area_polygon);

    // offsets into layers, histograms and runs
    
    if (istorm < (int) _layerOffsets.size()) {
      _gpropsVars.layer_props_offset.putVal(stormIndex, _layerOffsets[istorm]);
    }
    if (istorm < (int) _histOffsets.size()) {
      _gpropsVars.dbz_hist_offset.putVal(stormIndex, _histOffsets[istorm]);
    }
    if (istorm < (int) _runsOffsets.size()) {
      _gpropsVars.runs_offset.putVal(stormIndex, _runsOffsets[istorm]);
    }
    if (istorm < (int) _projRunsOffsets.size()) {
      _gpropsVars.proj_runs_offset.putVal(stormIndex, _projRunsOffsets[istorm]);
    }

  } // istorm

#ifdef JUNK
  
  // Move to the end of the file before beginning the write.

  fseek(_storm_data_file, 0, SEEK_END);

  // if nstorms is greater than zero, write global props to file
  
  int nstorms = _scan.nstorms;
  
  if (nstorms > 0) {
    
    // get gprops position in file
    
    _scan.gprops_offset = ftell(_storm_data_file);
    
    // make local copy of gprops and encode into network byte order

    TaArray<storm_file_global_props_t> gpropsArray;
    storm_file_global_props_t *gprops = gpropsArray.alloc(nstorms);
    memcpy (gprops, _gprops, nstorms * sizeof(storm_file_global_props_t));
    BE_from_array_32(gprops,
		     nstorms * sizeof(storm_file_global_props_t));
    
    // write in global props
    
    if (ufwrite(gprops, sizeof(storm_file_global_props_t),
		nstorms, _storm_data_file) != nstorms) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Writing gprops");
      TaStr::AddInt(_errStr, "  nstorms: ", nstorms);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

  } // if (nstorms > 0) 
  
  // get scan position in file
  
  allocScanOffsets(scan_num + 1);
  long offset = ftell(_storm_data_file);
  _scan_offsets[scan_num] = offset;
  
  // set last scan offset
  
  _scan.last_offset = offset + sizeof(storm_file_scan_header_t) - 1;
  
  // copy scan header to local variable, and encode. Note that the 
  // character data at the end of the struct is not encoded
  
  storm_file_scan_header_t scan = _scan;
  scan.grid.nbytes_char =  TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN;
  scan.nbytes_char = scan.grid.nbytes_char;
  
  ustr_clear_to_end(scan.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(scan.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32(&scan,
		   (sizeof(storm_file_scan_header_t) - scan.nbytes_char));
  
  // write scan struct
  
  if (ufwrite(&scan, sizeof(storm_file_scan_header_t), 1,
	      _storm_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing scan header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

#endif
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
//
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
//
//////////////////////////////////////////////////////////////

int TitanFile::writeStormAuxProps(int storm_num,
                                  const storm_file_header_t &storm_file_header,
                                  const storm_file_scan_header_t &sheader,
                                  const storm_file_global_props_t *gprops,
                                  const storm_file_layer_props_t *lprops,
                                  const storm_file_dbz_hist_t *hist,
                                  const storm_file_run_t *runs,
                                  const storm_file_run_t *proj_runs)
  
{

  // ensure we have space for the offsets
  
  _layerOffsets.resize(storm_num + 1);
  _histOffsets.resize(storm_num + 1);
  _runsOffsets.resize(storm_num + 1);
  _projRunsOffsets.resize(storm_num + 1);
  
  const storm_file_params_t &sparams(storm_file_header.params);
  const storm_file_global_props_t &gp = gprops[storm_num];

  // write layers

  _layerOffsets[storm_num] = _n_layers.getSize();
  int nLayers = gp.n_layers;
  for (int ilayer = 0; ilayer < nLayers; ilayer++) {
    const storm_file_layer_props_t &ll = lprops[ilayer];
    int lpropsOffset = _n_layers.getSize();
    std::vector<size_t> layerIndex;
    layerIndex.push_back(lpropsOffset);
    _lpropsVars.vol_centroid_x.putVal(layerIndex, ll.vol_centroid_x);
    _lpropsVars.vol_centroid_y.putVal(layerIndex, ll.vol_centroid_y);
    _lpropsVars.refl_centroid_x.putVal(layerIndex, ll.refl_centroid_x);
    _lpropsVars.refl_centroid_y.putVal(layerIndex, ll.refl_centroid_y);
    _lpropsVars.area.putVal(layerIndex, ll.area);
    _lpropsVars.dbz_max.putVal(layerIndex, ll.dbz_max);
    _lpropsVars.dbz_mean.putVal(layerIndex, ll.dbz_mean);
    _lpropsVars.mass.putVal(layerIndex, ll.mass);
    _lpropsVars.rad_vel_mean.putVal(layerIndex, ll.rad_vel_mean);
    _lpropsVars.rad_vel_sd.putVal(layerIndex, ll.rad_vel_sd);
    _lpropsVars.vorticity.putVal(layerIndex, ll.vorticity);
    if (sparams.low_convectivity_threshold != 0.0 ||
        sparams.high_convectivity_threshold != 0.0) {
      _lpropsVars.convectivity_median.putVal(layerIndex, ll.convectivity_median);
    }
  }

  // write histograms

  _histOffsets[storm_num] = _n_hist.getSize();
  int nHist = gp.n_dbz_intervals;
  for (int ihist = 0; ihist < nHist; ihist++) {
    const storm_file_dbz_hist_t &hh = hist[ihist];
    int histOffset = _n_hist.getSize();
    std::vector<size_t> histIndex;
    histIndex.push_back(histOffset);
    _histVars.percent_volume.putVal(histIndex, hh.percent_volume);
    _histVars.percent_area.putVal(histIndex, hh.percent_area);
  }
  
  // write runs
  
  _runsOffsets[storm_num] = _n_runs.getSize();
  int nRuns = gp.n_runs;
  for (int irun = 0; irun < nRuns; irun++) {
    const storm_file_run_t &run = runs[irun];
    int runOffset = _n_runs.getSize();
    std::vector<size_t> runIndex;
    runIndex.push_back(runOffset);
    _runsVars.run_ix.putVal(runIndex, run.ix);
    _runsVars.run_iy.putVal(runIndex, run.iy);
    _runsVars.run_iz.putVal(runIndex, run.iz);
    _runsVars.run_len.putVal(runIndex, run.n);
  }
  
  _projRunsOffsets[storm_num] = _n_proj_runs.getSize();
  int nProjRuns = gp.n_proj_runs;
  for (int irun = 0; irun < nProjRuns; irun++) {
    const storm_file_run_t &run = proj_runs[irun];
    int runOffset = _n_proj_runs.getSize();
    std::vector<size_t> runIndex;
    runIndex.push_back(runOffset);
    _projRunsVars.run_ix.putVal(runIndex, run.ix);
    _projRunsVars.run_iy.putVal(runIndex, run.iy);
    _projRunsVars.run_iz.putVal(runIndex, run.iz);
    _projRunsVars.run_len.putVal(runIndex, run.n);
  }
  
  // save existing dimensions as offsets

  // int runsOffset = _n_runs.getSize();
  // int projRunsOffset = _n_proj_runs.getSize();

  // get array sizes for this storm

  // int nRuns = gp.n_runs;
  // int nProjRuns = gp.n_proj_runs;
  
#ifdef JUNK
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeProps\n";
  TaStr::AddStr(_errStr, "  File: ", _storm_data_file_path);

  int n_layers = _gprops[storm_num].n_layers;
  int n_dbz_intervals = _gprops[storm_num].n_dbz_intervals;
  int n_runs = _gprops[storm_num].n_runs;
  int n_proj_runs = _gprops[storm_num].n_proj_runs;
  
  // set layer props offset
  
  fseek(_storm_data_file, 0, SEEK_END);
  int offset = ftell(_storm_data_file);
  _gprops[storm_num].layer_props_offset = offset;
  
  // if this is the first storm, store the first_offset value
  // in the scan header
  
  if (storm_num == 0) {
    _scan.first_offset = offset;
  }
  
  // copy layer props to local array
  
  TaArray<storm_file_layer_props_t> lpropsArray;
  storm_file_layer_props_t *lprops = lpropsArray.alloc(n_layers);
  memcpy (lprops, _lprops,
          n_layers * sizeof(storm_file_layer_props_t));
  
  // code layer props into network byte order from host byte order
  
  BE_from_array_32(lprops, n_layers * sizeof(storm_file_layer_props_t));
  
  // write layer props
  
  if (ufwrite(lprops, sizeof(storm_file_layer_props_t),
	      n_layers, _storm_data_file) != n_layers) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing layers");
    TaStr::AddInt(_errStr, "  n_layers: ", n_layers);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set dbz hist offset
  
  _gprops[storm_num].dbz_hist_offset = ftell(_storm_data_file);
  
  // copy histogram data to local variable

  TaArray<storm_file_dbz_hist_t> histArray;
  storm_file_dbz_hist_t *hist = histArray.alloc(n_dbz_intervals);
  memcpy (hist, _hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // encode histogram data to network byte order from host byte order
  
  BE_from_array_32(hist, n_dbz_intervals * sizeof(storm_file_dbz_hist_t));
  
  // write in histogram data
  
  if (ufwrite(hist, sizeof(storm_file_dbz_hist_t),
	      n_dbz_intervals, _storm_data_file) != n_dbz_intervals) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing hist");
    TaStr::AddInt(_errStr, "  n_dbz_intervals: ", n_dbz_intervals);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set runs offset
  
  _gprops[storm_num].runs_offset = ftell(_storm_data_file);
  
  // copy runs to local array

  TaArray<storm_file_run_t> runsArray;
  storm_file_run_t *runs = runsArray.alloc(n_runs);
  memcpy (runs, _runs, n_runs * sizeof(storm_file_run_t));
  
  // code run props into network byte order from host byte order
  
  BE_from_array_16(runs, n_runs * sizeof(storm_file_run_t));
  
  // write runs
  
  if (ufwrite(runs, sizeof(storm_file_run_t),
	      n_runs, _storm_data_file) != n_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing runs");
    TaStr::AddInt(_errStr, "  n_runs: ", n_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // set proj_runs offset
  
  _gprops[storm_num].proj_runs_offset = ftell(_storm_data_file);
  
  // copy proj_runs to local array

  TaArray<storm_file_run_t> projRunsArray;
  storm_file_run_t *proj_runs = projRunsArray.alloc(n_proj_runs);
  memcpy (proj_runs, _proj_runs,
          n_proj_runs * sizeof(storm_file_run_t));
  
  // code run props into network byte order from host byte order
  
  BE_from_array_16(proj_runs,
		   n_proj_runs * sizeof(storm_file_run_t));
  
  // write proj_runs
  
  if (ufwrite(proj_runs, sizeof(storm_file_run_t),
	      n_proj_runs, _storm_data_file) != n_proj_runs) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing proj_runs");
    TaStr::AddInt(_errStr, "  n_proj_runs: ", n_proj_runs);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

#endif

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// convert ellipse parameters from deg to km,
// for those which were computed from latlon grids.
//
//////////////////////////////////////////////////////////////

void TitanFile::_convert_ellipse_2km(const titan_grid_t &tgrid,
					  double centroid_x,
					  double centroid_y,
					  fl32 &orientation,
					  fl32 &minor_radius,
					  fl32 &major_radius)
  
{

  // only convert for latlon projection
  
  if (tgrid.proj_type == TITAN_PROJ_LATLON) {
    
    double centroid_lon, centroid_lat;
    double major_orient_rad, major_lon, major_lat;
    double minor_orient_rad, minor_lon, minor_lat;
    double dist, theta;
    double orientation_km, major_radius_km, minor_radius_km;
    double sin_major, cos_major;
    double sin_minor, cos_minor;

    centroid_lon = centroid_x;
    centroid_lat = centroid_y;
    
    major_orient_rad = orientation * DEG_TO_RAD;
    ta_sincos(major_orient_rad, &sin_major, &cos_major);
    major_lon = centroid_lon + major_radius * sin_major;
    major_lat = centroid_lat + major_radius * cos_major;
    
    minor_orient_rad = (orientation + 270.0) * DEG_TO_RAD;
    ta_sincos(minor_orient_rad, &sin_minor, &cos_minor);
    minor_lon = centroid_lon + minor_radius * sin_minor;
    minor_lat = centroid_lat + minor_radius * cos_minor;

    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     major_lat, major_lon,
		     &dist, &theta);

    orientation_km = theta;
    major_radius_km = dist;
    
    PJGLatLon2RTheta(centroid_lat, centroid_lon,
		     minor_lat, minor_lon,
		     &dist, &theta);
    
    minor_radius_km = dist;
    
    orientation = orientation_km;
    major_radius = major_radius_km;
    minor_radius = minor_radius_km;

  }

}
		     
//////////////////////////////////////////////////////////////
//
// Convert the ellipse data (orientation, major_radius and minor_radius)
// for a a gprops struct to local (km) values.
// This applies to structs which were derived from lat-lon grids, for
// which some of the fields are in deg instead of km.
// It is a no-op for other projections.
//
// See Note 3 in storms.h
//
//////////////////////////////////////////////////////////////

void TitanFile::gpropsEllipses2Km(const storm_file_scan_header_t &scan,
				       storm_file_global_props_t &gprops)
     
{
  
  // convert the ellipses as appropriate

  _convert_ellipse_2km(scan.grid,
		       gprops.precip_area_centroid_x,
		       gprops.precip_area_centroid_y,
		       gprops.precip_area_orientation,
		       gprops.precip_area_minor_radius,
		       gprops.precip_area_major_radius);
  
  _convert_ellipse_2km(scan.grid,
		       gprops.proj_area_centroid_x,
		       gprops.proj_area_centroid_y,
		       gprops.proj_area_orientation,
		       gprops.proj_area_minor_radius,
		       gprops.proj_area_major_radius);

}

//////////////////////////////////////////////////////////////
//
// Convert the (x,y) km locations in a gprops struct to lat-lon.
// This applies to structs which were computed for non-latlon 
// grids. It is a no-op for lat-lon grids.
//
// See Note 3 in storms.h
//
//////////////////////////////////////////////////////////////

void TitanFile::gpropsXY2LatLon(const storm_file_scan_header_t &scan,
				     storm_file_global_props_t &gprops)
  
{
  
  const titan_grid_t  &tgrid = scan.grid;

  switch (tgrid.proj_type) {
    
  case TITAN_PROJ_LATLON:
    break;
    
  case TITAN_PROJ_FLAT:
    {
      double lat, lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.vol_centroid_x,
			gprops.vol_centroid_y,
			&lat, &lon);
      gprops.vol_centroid_y = lat;
      gprops.vol_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.refl_centroid_x,
			gprops.refl_centroid_y,
			&lat, &lon);
      gprops.refl_centroid_y = lat;
      gprops.refl_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.precip_area_centroid_x,
			gprops.precip_area_centroid_y,
			&lat, &lon);
      gprops.precip_area_centroid_y = lat;
      gprops.precip_area_centroid_x = lon;
      PJGLatLonPlusDxDy(tgrid.proj_origin_lat,
			tgrid.proj_origin_lon,
			gprops.proj_area_centroid_x,
			gprops.proj_area_centroid_y,
			&lat, &lon);
      gprops.proj_area_centroid_y = lat;
      gprops.proj_area_centroid_x = lon;
      break;
    }
    
  case TITAN_PROJ_LAMBERT_CONF:
    {
      double lat, lon;
      PJGstruct *ps = PJGs_lc2_init(tgrid.proj_origin_lat,
				    tgrid.proj_origin_lon,
				    tgrid.proj_params.lc2.lat1,
				    tgrid.proj_params.lc2.lat2);
      if (ps != nullptr) {
	PJGs_lc2_xy2latlon(ps,
			   gprops.vol_centroid_x,
			   gprops.vol_centroid_y,
			   &lat, &lon);
	gprops.vol_centroid_y = lat;
	gprops.vol_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.refl_centroid_x,
			   gprops.refl_centroid_y,
			   &lat, &lon);
	gprops.refl_centroid_y = lat;
	gprops.refl_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.precip_area_centroid_x,
			   gprops.precip_area_centroid_y,
			   &lat, &lon);
	gprops.precip_area_centroid_y = lat;
	gprops.precip_area_centroid_x = lon;
	PJGs_lc2_xy2latlon(ps,
			   gprops.proj_area_centroid_x,
			   gprops.proj_area_centroid_y,
			   &lat, &lon);
	gprops.proj_area_centroid_y = lat;
	gprops.proj_area_centroid_x = lon;
	free(ps);
      }
      break;
    }
  
  default:
    break;
    
  } // switch 
  
}

///////////////////////////////////////////////////////////////
// Truncate header file
//
// Returns 0 on success, -1 on failure.

int TitanFile::truncateStormHeaderFile(int length)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::truncateStormHeaderFile\n";
  return (_truncateStormFiles(_storm_header_file, _storm_header_file_path, length));

}

///////////////////////////////////////////////////////////////
// Truncate data file
//
// Returns 0 on success, -1 on failure.

int TitanFile::truncateStormDataFile(int length)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::truncateStormDataFile\n";
  return (_truncateStormFiles(_storm_data_file, _storm_data_file_path, length));

}

///////////////////////////////////////////////////
//
// Truncate open file - close, truncate and reopen
//
// Returns 0 on success, -1 on failure
//

int TitanFile::_truncateStormFiles(FILE *&fd, const string &path, int length)
     
{
  
  TaStr::AddStr(_errStr, "ERROR - ", "TitanFile::_truncate");
  TaStr::AddStr(_errStr, "  File: ", path);
  
  // close the buffered file
  
  fclose(fd);
  
  // open for low-level io
  
  int low_fd;
  if ((low_fd = open(path.c_str(), O_WRONLY)) < 0) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ",
		  "Cannot open file - low level - for truncation.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // truncate the file
  
  if (ftruncate(low_fd, length) != 0) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot truncate file.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return(-1);
  }
  
  // close low-level io
  
  close(low_fd);
  
  // re-open the file for buffered i/o
  
  if ((fd = fopen(path.c_str(), "r+")) == nullptr) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot reopen file.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return(-1);
  }

  return (0);
  
}

////////////////////////////////////////////////////////////
// track data access

const simple_track_params_t &TitanFile::simple_params() const { 
  return _simple_params;
}

const complex_track_params_t &TitanFile::complex_params() const {
  return _complex_params;
}

#define N_ALLOC 20

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocSimpleArrays()
//
// allocate space for the simple track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::allocSimpleArrays(int n_simple_needed)
     
{

  if (_n_simple_allocated < n_simple_needed) {
    
    int n_start = _n_simple_allocated;

    int n_realloc = n_simple_needed + N_ALLOC;
    _n_simple_allocated = n_realloc;
    
    _simple_track_offsets = (si32 *) urealloc
      (_simple_track_offsets, n_realloc * sizeof(si32));
      
    _nsimples_per_complex = (si32 *) urealloc
      (_nsimples_per_complex, n_realloc * sizeof(si32));
    
    _simples_per_complex_offsets = (si32 *) urealloc
      (_simples_per_complex_offsets, n_realloc * sizeof(si32));
    
    _complex_track_offsets = (si32 *) urealloc
      (_complex_track_offsets, n_realloc * sizeof(si32));
    
    // initialize new elements to zero
  
    int n_new = _n_simple_allocated - n_start;

    memset (_simple_track_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_nsimples_per_complex + n_start, 0, n_new * sizeof(si32));
    memset (_simples_per_complex_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_complex_track_offsets + n_start, 0, n_new * sizeof(si32));
  
  } // if (_n_simple_allocated < n_simple_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeSimpleArrays()
//
// free space for the simple track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeSimpleArrays()
     
{
  
  if (_simples_per_complex) {
    for (int i = 0; i < _n_simples_per_complex_allocated; i++) {
      if(_simples_per_complex[i] != nullptr) {
	ufree(_simples_per_complex[i]);
      }
    }
    ufree(_simples_per_complex);
    _simples_per_complex = nullptr;
  }
  
  if (_simple_track_offsets) {
    ufree(_simple_track_offsets);
    _simple_track_offsets = nullptr;
  }

  if (_nsimples_per_complex) {
    ufree(_nsimples_per_complex);
    _nsimples_per_complex = nullptr;
  }

  if (_simples_per_complex_offsets) {
    ufree(_simples_per_complex_offsets);
    _simples_per_complex_offsets = nullptr;
  }

  if (_complex_track_offsets) {
    ufree(_complex_track_offsets);
    _complex_track_offsets = nullptr;
  }

  _n_simple_allocated = 0;

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocComplexArrays()
//
// allocate space for the complex track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::allocComplexArrays(int n_complex_needed)
     
{

  if (_n_complex_allocated < n_complex_needed) {

    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
    
    int n_start = _n_complex_allocated;
    int n_realloc = n_complex_needed + N_ALLOC;
    _n_complex_allocated = n_realloc;
    
    _complex_track_nums = (si32 *) urealloc
      (_complex_track_nums, n_realloc * sizeof(si32));

    // initialize new elements to zero
  
    int n_new = n_realloc - n_start;
    memset (_complex_track_nums + n_start, 0, n_new * sizeof(si32));
    
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeComplexArrays()
//
// free space for the complex track arrays.
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeComplexArrays()
     
{

  if (_complex_track_nums) {
    ufree(_complex_track_nums);
    _complex_track_nums = nullptr;
    _n_complex_allocated = 0;
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocSimplesPerComplex()
//
// allocate space for the array of pointers to simples_per_complex
//
///////////////////////////////////////////////////////////////////////////


void TitanFile::allocSimplesPerComplex(int n_simple_needed)
     
{

  if (_n_simples_per_complex_allocated < n_simple_needed) {
    
    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
    
    int n_start = _n_simples_per_complex_allocated;
    int n_realloc = n_simple_needed + N_ALLOC;
    _n_simples_per_complex_allocated = n_realloc;
    
    _simples_per_complex = (si32 **) urealloc
      (_simples_per_complex, n_realloc * sizeof(si32 *));
    
    // initialize new elements to zero
  
    int n_new = n_realloc - n_start;

    memset (_simples_per_complex + n_start,
	    0, n_new * sizeof(si32 *));
  
  } // if (_n_simples_per_complex_allocated < n_simple_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeSimplesPerComplex()
//
///////////////////////////////////////////////////////////////////////////


void TitanFile::freeSimplesPerComplex()
     
{
  if (_simples_per_complex) {
    for (int i = 0; i < _n_simples_per_complex_allocated; i++) {
      if (_simples_per_complex[i] != nullptr) {
	ufree(_simples_per_complex[i]);
	_simples_per_complex[i] = nullptr;
      }
    }
    ufree(_simples_per_complex);
    _simples_per_complex = nullptr;
    _n_simples_per_complex_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocScanEntries()
//
// allocate mem for scan entries array
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::allocScanEntries(int n_entries_needed)
  
{
  
  if (n_entries_needed > _n_scan_entries_allocated) {
    
    _scan_entries = (track_file_entry_t *) urealloc
      (_scan_entries, n_entries_needed * sizeof(track_file_entry_t));
    
    _n_scan_entries_allocated = n_entries_needed;
    
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeScanEntries()
//
// free scan entries
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeScanEntries()
     
{
  
  if (_scan_entries) {
    ufree(_scan_entries);
    _scan_entries = nullptr;
    _n_scan_entries_allocated = 0;
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocScanIndex()
//
// allocate space for the scan index
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::allocScanIndex(int n_scans_needed)
     
{

  if (_n_scan_index_allocated < n_scans_needed) {

    // allocate the required space plus a buffer so that 
    // we do not do too many reallocs
      
    int n_start = _n_scan_index_allocated;
    int n_realloc = n_scans_needed + N_ALLOC;
    _n_scan_index_allocated = n_realloc;

    _scan_index = (track_file_scan_index_t *) urealloc
      (_scan_index, n_realloc * sizeof(track_file_scan_index_t));
      
    // initialize new elements to zero
  
    int n_new = _n_scan_index_allocated - n_start;

    memset (_scan_index + n_start, 0,
	    n_new * sizeof(track_file_scan_index_t));
  
  } // if (_n_scan_index_allocated < n_scans_needed) 

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeScanIndex()
//
// free space for the scan index
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeScanIndex()
     
{
  if (_scan_index) {
    ufree(_scan_index);
    _scan_index = nullptr;
    _n_scan_index_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::allocUtime()
//
// allocate array of track_utime_t structs
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::allocUtime()
     
{

  if (_n_utime_allocated < _track_header.max_simple_track_num + 1) {
      
    _n_utime_allocated = _track_header.max_simple_track_num + 1;
    
    _track_utime = (track_utime_t *) urealloc
      (_track_utime, _n_utime_allocated * sizeof(track_utime_t));
    
    memset (_track_utime, 0,
	    _n_utime_allocated * sizeof(track_utime_t));
      
  }

}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeUtime()
//
// free space for the utime array
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeUtime()
     
{
  if (_track_utime) {
    ufree(_track_utime);
    _track_utime = nullptr;
    _n_utime_allocated = 0;
  }
}

///////////////////////////////////////////////////////////////////////////
//
// TitanFile::freeAll()
//
// free all arrays
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::freeTracksAll()
  
{

  freeSimpleArrays();
  freeComplexArrays();
  freeSimplesPerComplex();
  freeScanEntries();
  freeScanIndex();
  freeUtime();

}

///////////////////////////////////////////////////////////////////////////
//
// Open the track header and data files
//
// Returns 0 on success, -1 on error
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::openTrackFiles(const char *mode,
                                const char *header_file_path,
                                const char *data_file_ext /* = nullptr*/ )
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::openFiles\n";

  // close files

  closeTrackFiles();

  // open the header file - file path may change if it is compressed
  
  char hdr_file_path[MAX_PATH_LEN];
  STRncopy(hdr_file_path, header_file_path, MAX_PATH_LEN);
  if ((_track_header_file = ta_fopen_uncompress(hdr_file_path, mode)) == nullptr) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open header file: ", header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _track_header_file_path = hdr_file_path;
  
  // compute the data file name
   
  if (*mode == 'r') {

    // read the header if the file is opened for reading
   
    if (readTrackHeader(false)) {
      return -1;
    }
    
    // compute the file path from the header file path and
    // the data file name

    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, header_file_path, MAX_PATH_LEN - 1);

    // if dir path has slash, get pointer to that and end the sting
    // immediately after
    
    char *chptr;
    if ((chptr = strrchr(tmp_path, '/')) != nullptr) {
      *(chptr + 1) = '\0';
      _track_data_file_path = tmp_path;
      _track_data_file_path += _track_header.data_file_name;
    } else {
      _track_data_file_path = _track_header.data_file_name;
    }

  } else {

    // file opened for writing, use ext to compute file name

    if (data_file_ext == nullptr) {
      _errStr += "Must provide data file extension for file creation\n";
      return -1;
    }

    char tmp_path[MAX_PATH_LEN];
    strncpy(tmp_path, _track_header_file_path.c_str(), MAX_PATH_LEN - 1);
    
    char *chptr;
    if ((chptr = strrchr(tmp_path, '.')) == nullptr) {
      TaStr::AddStr(_errStr, "  Header file must have extension : ",
		    _track_header_file_path);
      return -1;
    }
    
    *(chptr + 1) = '\0';
    _track_data_file_path = tmp_path;
    _track_data_file_path += data_file_ext;

  } // if (*mode == 'r') 
    
  // open the data file
  
  char dat_file_path[MAX_PATH_LEN];
  STRncopy(dat_file_path, _track_data_file_path.c_str(), MAX_PATH_LEN);
    
  if ((_track_data_file = ta_fopen_uncompress(dat_file_path, mode)) == nullptr) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Cannot open data file: ",
		  _track_data_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  _track_data_file_path = dat_file_path;

  // In write mode, write file labels
   
  if (*mode == 'w') {

    // header file
    
    char header_file_label[R_FILE_LABEL_LEN];
    MEM_zero(header_file_label);
    strcpy(header_file_label, TRACK_HEADER_FILE_TYPE);
    _track_header_file_label = header_file_label;
    
    if (ufwrite(header_file_label, 1, R_FILE_LABEL_LEN,
		_track_header_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing header file label to: ",
		    _track_header_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

    // data file

    char data_file_label[R_FILE_LABEL_LEN];
    MEM_zero(data_file_label);
    strcpy(data_file_label, TRACK_DATA_FILE_TYPE);
    _track_data_file_label = data_file_label;
    
    if (ufwrite(data_file_label, 1, R_FILE_LABEL_LEN,
		_track_data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Writing data file label to: ",
		    _track_data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }

  } else {
    
    // read mode - read in data file label
    
    char data_file_label[R_FILE_LABEL_LEN];
    if (ufread(data_file_label, sizeof(char), R_FILE_LABEL_LEN,
	       _track_data_file) != R_FILE_LABEL_LEN) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  Reading data file label from: ",
		    _track_data_file_path);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // check label
    
    if (_track_data_file_label != data_file_label) {
      _errStr +=
	"  Data file does not have the correct label\n";
      TaStr::AddStr(_errStr, "  File label is: ", data_file_label);
      TaStr::AddStr(_errStr, "  Should be: ", _track_data_file_label);
      return -1;
    }
    
  } // if (*mode == 'w') 

  return 0;

}

//////////////////////////////////////////////////////////////
//
// closes the track header and data files
//
//////////////////////////////////////////////////////////////

void TitanFile::closeTrackFiles()
     
{

  // unlock the header file

  unlockTrackHeaderFile();

  // close the header file
  
  if (_track_header_file != nullptr) {
    fclose(_track_header_file);
    _track_header_file = (FILE *) nullptr;
  }

  // close the data file
  
  if (_track_data_file != nullptr) {
    fclose(_track_data_file);
    _track_data_file = (FILE *) nullptr;
  }
  
}

//////////////////////////////////////////////////////////////
//
// flush the track header and data files
//
//////////////////////////////////////////////////////////////

void TitanFile::flushTrackFiles()
  
{
  
  fflush(_track_header_file);
  fflush(_track_data_file);

}

//////////////////////////////////////////////////////////////
//
// TitanFile::lockHeaderFile()
//
// Put an advisory lock on the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::lockTrackHeaderFile(const char *mode)
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::lockTrackHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _track_header_file_path);
  
  if (ta_lock_file_procmap(_track_header_file_path.c_str(),
			   _track_header_file, mode)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot lock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// TitanFile::unlockHeaderFile()
//
// Remove advisory lock from the header file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::unlockTrackHeaderFile()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::unlockTrackHeaderFile\n";
  TaStr::AddStr(_errStr, "  File: ", _track_header_file_path);
  
  if (ta_unlock_file(_track_header_file_path.c_str(),
		     _track_header_file)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Cannot unlock file");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// Read in the track_file_header_t structure from a track file.
// Read in associated arrays.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readTrackHeader(bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanFile::readTrackHeader\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_header_file_path);

  // rewind file
  
  fseek(_track_header_file, 0L, SEEK_SET);
  
  // read in file label
  
  char header_file_label[R_FILE_LABEL_LEN];
  if (ufread(header_file_label, sizeof(char), R_FILE_LABEL_LEN,
	     _track_header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading header file label from: ",
		  _track_header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // check label
  
  if (_track_header_file_label != header_file_label) {
    _errStr +=
      "  Header file does not contain correct label.\n";
    TaStr::AddStr(_errStr, "  File label is: ", header_file_label);
    TaStr::AddStr(_errStr, "  Should be: ", _track_header_file_label);
    return -1;
  }
    
  // read in header
  
  if (ufread(&_track_header, sizeof(track_file_header_t),
	     1, _track_header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading file header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _track_header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_track_header, (sizeof(track_file_header_t) - nbytes_char));

  int n_complex_tracks = _track_header.n_complex_tracks;
  int n_simple_tracks = _track_header.n_simple_tracks;
  int n_scans = _track_header.n_scans;
  
  // check that the constants in use when the file was written are
  // less than or the same as those in use now
  
  if (_track_header.max_parents != MAX_PARENTS) {
    TaStr::AddStr(_errStr, "  ", "MAX_PARENTS has changed");
    TaStr::AddInt(_errStr, "  _track_header.max_parents: ", _track_header.max_parents);
    TaStr::AddInt(_errStr, "  MAX_PARENTS: ", MAX_PARENTS);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  if (_track_header.max_children != MAX_CHILDREN) {
    TaStr::AddStr(_errStr, "  ", "MAX_CHILDREN has changed");
    TaStr::AddInt(_errStr, "  _track_header.max_children: ", _track_header.max_children);
    TaStr::AddInt(_errStr, "  MAX_CHILDREN: ", MAX_CHILDREN);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  if (_track_header.max_nweights_forecast != MAX_NWEIGHTS_FORECAST) {
    TaStr::AddStr(_errStr, "  ", "MAX_NWEIGHTS_FORECAST has changed");
    TaStr::AddInt(_errStr, "  _track_header.max_nweights_forecast: ",
		  _track_header.max_nweights_forecast);
    TaStr::AddInt(_errStr, "  MAX_NWEIGHTS_FORECAST: ",
		  MAX_NWEIGHTS_FORECAST);
    TaStr::AddStr(_errStr, "  ", "Fix header and recompile");
    return -1;
  }

  // alloc arrays

  allocComplexArrays(n_complex_tracks);
  allocSimpleArrays(n_simple_tracks);
  allocScanIndex(n_scans);
  
  // read in complex track num array

  if (ufread(_complex_track_nums, sizeof(si32),
	     n_complex_tracks, _track_header_file) != n_complex_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex track nums");
    TaStr::AddInt(_errStr, "  n_complex_tracks", n_complex_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_complex_track_nums, n_complex_tracks * sizeof(si32));
  
  // read in complex track offsets
  
  if (ufread(_complex_track_offsets, sizeof(si32),
	     n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex track offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_complex_track_offsets, n_simple_tracks * sizeof(si32));
  
  // read in simple track offsets
  
  if (ufread(_simple_track_offsets, sizeof(si32),
	     n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simple track offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_simple_track_offsets, n_simple_tracks * sizeof(si32));
  
  // read in scan index array
  
  if (ufread(_scan_index, sizeof(track_file_scan_index_t),
	     n_scans, _track_header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading scan index array");
    TaStr::AddInt(_errStr, "  n_scans", n_scans);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_scan_index, n_scans * sizeof(track_file_scan_index_t));
  
  // read in nsimples_per_complex
  
  if (ufread(_nsimples_per_complex, sizeof(si32),
	     n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading nsimples_per_complex");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_nsimples_per_complex, n_simple_tracks * sizeof(si32));
  
  // read in simples_per_complex_offsets
  
  if (ufread(_simples_per_complex_offsets, sizeof(si32),
	     n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simples_per_complex_offsets");
    TaStr::AddInt(_errStr, "  n_simple_tracks", n_simple_tracks);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_simples_per_complex_offsets,
		 n_simple_tracks * sizeof(si32));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// Read in the track_file_header_t and scan_index array.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readScanIndex(bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanFile::readScanIndex\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_header_file_path);

  // rewind file
  
  fseek(_track_header_file, 0L, SEEK_SET);
  
  // read in file label
  
  char header_file_label[R_FILE_LABEL_LEN];
  if (ufread(header_file_label, sizeof(char), R_FILE_LABEL_LEN,
	     _track_header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  Reading header file label from: ",
		  _track_header_file_path);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // check label
  
  if (_track_header_file_label != header_file_label) {
    _errStr +=
      "  Header file does not contain correct label.\n";
    TaStr::AddStr(_errStr, "  File label is: ", header_file_label);
    TaStr::AddStr(_errStr, "  Should be: ", _track_header_file_label);
    return -1;
  }
    
  // read in header
  
  if (ufread(&_track_header, sizeof(track_file_header_t),
	     1, _track_header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading file header");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // decode the structure into host byte order - the file
  // is stored in network byte order
  
  si32 nbytes_char = _track_header.nbytes_char;
  BE_to_array_32(&nbytes_char, sizeof(si32));
  BE_to_array_32(&_track_header, (sizeof(track_file_header_t) - nbytes_char));

  int n_complex_tracks = _track_header.n_complex_tracks;
  int n_simple_tracks = _track_header.n_simple_tracks;
  int n_scans = _track_header.n_scans;
  
  // seek ahead

  long nbytesSkip = (n_complex_tracks * sizeof(si32) +
		     2 * n_simple_tracks * sizeof(si32));
		    
  if (fseek(_track_header_file, nbytesSkip, SEEK_CUR)) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Seeking over  arrays");
    TaStr::AddInt(_errStr, "  Cannot seek ahead by nbytes: ", nbytesSkip);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // alloc array

  allocScanIndex(n_scans);

  // read in scan index array
  
  if (ufread(_scan_index, sizeof(track_file_scan_index_t),
	     n_scans, _track_header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading scan index array");
    TaStr::AddInt(_errStr, "  n_scans", n_scans);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(_scan_index, n_scans * sizeof(track_file_scan_index_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// reads in the parameters for a complex track
//
// For normal reads, read_simples_per_complex should be set true. This
// is only set FALSE in Titan, which creates the track files.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readComplexParams(int track_num,
				      bool read_simples_per_complex,
				      bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanFile::readComplexParams\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);
  TaStr::AddInt(_errStr, "  track_num", track_num);

  // move to offset in file
  
  if (_complex_track_offsets[track_num] == 0) {
    return -1;
  }
  
  fseek(_track_data_file, _complex_track_offsets[track_num], SEEK_SET);
  
  // read in params
  
  if (ufread(&_complex_params, sizeof(complex_track_params_t),
	     1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading complex_track_params");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(&_complex_params, sizeof(complex_track_params_t));
  
  // If read_simples_per_complex is set,
  // read in simples_per_complex array, which indicates which
  // simple tracks are part of this complex track.

  if (read_simples_per_complex) {
    
    int nsimples = _nsimples_per_complex[track_num];
    
    allocSimplesPerComplex(track_num + 1);

    if (_simples_per_complex[track_num] == nullptr) {
      _simples_per_complex[track_num] = (si32 *) umalloc
	(nsimples * sizeof(si32));
    } else {
      _simples_per_complex[track_num] = (si32 *) urealloc
	(_simples_per_complex[track_num],
	 nsimples * sizeof(si32));
    }
    
    fseek(_track_header_file, _simples_per_complex_offsets[track_num], SEEK_SET);
  
    if (ufread(_simples_per_complex[track_num],
	       sizeof(si32), nsimples, _track_header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading simples per complex for");
      TaStr::AddStr(_errStr, "  ", "  complex track params.");
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    BE_to_array_32(_simples_per_complex[track_num], nsimples * sizeof(si32));

  } //   if (read_simples_per_complex) 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the parameters for a simple track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readSimpleParams(int track_num,
				     bool clear_error_str /* = true*/ )
     
{
  
  if (clear_error_str) {
    _clearErrStr();
  }
  _errStr += "ERROR - TitanFile::readSimpleParams\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);
  TaStr::AddInt(_errStr, "  track_num", track_num);

  // move to offset in file
  
  fseek(_track_data_file, _simple_track_offsets[track_num], SEEK_SET);
  
  // read in params
  
  if (ufread(&_simple_params, sizeof(simple_track_params_t),
	     1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading simple_track_params");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  BE_to_array_32(&_simple_params, sizeof(simple_track_params_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in an entry for a track
//
// If first_entry is set to TRUE, then the first entry is read in. If not
// the next entry is read in.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readEntry()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::readEntry\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);

  // move to the entry offset in the file
  
  long offset;
  if (_first_entry) {
    offset = _simple_params.first_entry_offset;
    _first_entry = false;
  } else {
    offset = _entry.next_entry_offset;
  }
  
  fseek(_track_data_file, offset, SEEK_SET);
  
  // read in entry
  
  if (ufread(&_entry, sizeof(track_file_entry_t),
	     1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Reading track entry");
    TaStr::AddInt(_errStr, "  Simple track num: ",
		  _simple_params.simple_track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  BE_to_array_32(&_entry, sizeof(track_file_entry_t));
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the array of simple tracks for each complex track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readSimplesPerComplex()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::readSimplesPerComplex\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);

  for (int itrack = 0; itrack < _track_header.n_complex_tracks; itrack++) {

    int complex_num = _complex_track_nums[itrack];
    int nsimples = _nsimples_per_complex[complex_num];

    allocSimplesPerComplex(complex_num + 1);

    _simples_per_complex[complex_num] = (si32 *) urealloc
      (_simples_per_complex[complex_num],
       (nsimples * sizeof(si32)));

    fseek(_track_header_file,
	  _simples_per_complex_offsets[complex_num], SEEK_SET);
    
    if (ufread(_simples_per_complex[complex_num], sizeof(si32), nsimples,
	       _track_header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading simples_per_complex");
      TaStr::AddInt(_errStr, "  track_num: ", complex_num);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    BE_to_array_32(_simples_per_complex[complex_num],
		   nsimples * sizeof(si32));

  } // itrack 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in entries for a scan
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readScanEntries(int scan_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::readScanEntries\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);

  // allocate as necessary
  
  _n_scan_entries = _scan_index[scan_num].n_entries;
  allocScanEntries(_n_scan_entries);

  track_file_entry_t *entry = _scan_entries;
  int next_entry_offset = _scan_index[scan_num].first_entry_offset;
  
  for (int ientry = 0; ientry < _n_scan_entries; ientry++, entry++) {
    
    // move to the next entry offset in the file
    
    fseek(_track_data_file, next_entry_offset, SEEK_SET);
  
    // read in entry
  
    if (ufread(entry, sizeof(track_file_entry_t),
	       1, _track_data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Reading track entry");
      TaStr::AddInt(_errStr, "  ientry: ", ientry);
      TaStr::AddInt(_errStr, "  scan_num: ", scan_num);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
  
    BE_to_array_32(entry, sizeof(track_file_entry_t));
    next_entry_offset = entry->next_scan_entry_offset;

  } // ientry 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in track_utime_t array
//
// Returns 0 on success or -1 on error
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readUtime()
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::readUtime\n";
  TaStr::AddStr(_errStr, "  Reading from file: ", _track_data_file_path);

  allocUtime();
  
  // read the complex and simple track params and load up
  // the start and end julian time arrays - these are used to
  // determine if a track is a valid candidate for display
  
  for (int itrack = 0; itrack < _track_header.n_complex_tracks; itrack++) {
    
    int complex_track_num = _complex_track_nums[itrack];
    
    if (readComplexParams(complex_track_num, true, false)) {
      return -1;
    }
    
    time_t start_time = _complex_params.start_time;
    time_t end_time = _complex_params.end_time;
    
    _track_utime[complex_track_num].start_complex = start_time;
    _track_utime[complex_track_num].end_complex = end_time;

  } // itrack 
  
  for (int itrack = 0; itrack < _track_header.n_simple_tracks; itrack++) {
    
    int simple_track_num = itrack;
    
    if (readSimpleParams(simple_track_num, false)) {
      return -1;
    }
    
    time_t start_time = _simple_params.start_time;
    time_t end_time = _simple_params.end_time;
    
    _track_utime[simple_track_num].start_simple = start_time;
    _track_utime[simple_track_num].end_simple = end_time;

  } // itrack 

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// Reinitialize headers and arrays
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::reinit()
     
{
  
  MEM_zero(_track_header);
  MEM_zero(_complex_params);
  MEM_zero(_simple_params);
  MEM_zero(_entry);
  
  if (_n_complex_allocated > 0) {
    memset(_complex_track_nums, 0, _n_complex_allocated * sizeof(si32));
  }

  if (_n_scan_entries_allocated > 0) {
    memset(_scan_entries, 0, _n_scan_entries * sizeof(track_file_entry_t));
  }
  
  if (_n_scan_index_allocated > 0) {
    memset(_scan_index, 0,
	   _n_scan_index_allocated * sizeof(track_file_scan_index_t));
  }
  
  if (_n_simple_allocated > 0) {
    memset (_simple_track_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_nsimples_per_complex, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_simples_per_complex_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_complex_track_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
  }
    
  if (_n_utime_allocated > 0) {
    memset(_track_utime, 0,
	   _n_utime_allocated * sizeof(track_utime_t));
  }

}

///////////////////////////////////////////////////////////////////////////
//
// Set a complex params slot in the file available for
// reuse, by setting the offset to its negative value.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::reuseComplexSlot(int track_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::ReuseComplexSlot\n";

  si32 *offset = _complex_track_offsets + track_num;

  if (*offset <= 0) {
    TaStr::AddStr(_errStr, "  ", "Slot for track not available");
    TaStr::AddInt(_errStr, "  track_num: ", track_num);
    return -1;
  }
  
  *offset *= -1;
  
  if (track_num < _lowest_avail_complex_slot)
    _lowest_avail_complex_slot = track_num;

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// prepare a simple track for reading by reading in the simple track
// params and setting the first_entry flag
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::rewindSimple(int track_num)
     
{

  _clearErrStr();
  _errStr += "ERROR - TitanFile::RewindSimpleTrack\n";
  
  // read in simple track params
  
  if (readSimpleParams(track_num, false)) {
    return -1;
  }

  // set first_entry flag
  
  _first_entry = TRUE;

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// rewrite an entry for a track in the track data file
//
// The entry is written at the file offset of the original entry
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::rewriteEntry()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::RewriteEntry\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _track_data_file_path);

  // code copy into network byte order
  
  track_file_entry_t entry = _entry;
  BE_to_array_32(&entry, sizeof(track_file_entry_t));
  
  // move to entry offset
  
  fseek(_track_data_file, _entry.this_entry_offset, SEEK_SET);
  
  // write entry
  
  if (ufwrite(&entry, sizeof(track_file_entry_t),
	      1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing track entry");
    TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// seek to the end of the track file data
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::seekTrackEndData()
  
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::seekTrackEndData\n";
  TaStr::AddStr(_errStr, "  File: ", _track_data_file_path);

  if (fseek(_track_data_file, 0L, SEEK_END) != 0) {
    
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
    
  } else {

    return 0;

  }

}

///////////////////////////////////////////////////////////////////////////
//
// seek to the start of data in track data file
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::seekTrackStartData()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::seekTrackStartData\n";
  TaStr::AddStr(_errStr, "  File: ", _track_data_file_path);

  if (fseek(_track_data_file, R_FILE_LABEL_LEN, SEEK_SET) != 0) {
    
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "seek failed");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
    
  } else {
    
    return 0;
    
  }

}

///////////////////////////////////////////////////////////////////////////
//
// write the track_file_header_t structure to a track data file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeTrackHeader()
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeTrackHeader\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _track_header_file_path);
  
  // get data file size

  fflush(_track_data_file);
  struct stat data_stat;
  ta_stat (_track_data_file_path.c_str(), &data_stat);
  _track_header.data_file_size = data_stat.st_size;
  
  // set file time to gmt
  
  _track_header.file_time = time(nullptr);
  
  // copy file label
  
  char label[R_FILE_LABEL_LEN];
  MEM_zero(label);
  strcpy(label, _track_header_file_label.c_str());
  
  // move to start of file and write label
  
  fseek(_track_header_file, (si32) 0, SEEK_SET);
  
  if (ufwrite(label, sizeof(char), R_FILE_LABEL_LEN,
	      _track_header_file) != R_FILE_LABEL_LEN) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing file label.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // create local arrays

  int n_complex_tracks = _track_header.n_complex_tracks;
  int n_simple_tracks = _track_header.n_simple_tracks;
  int n_scans = _track_header.n_scans;

  TaArray<si32> nums, coffsets, soffsets, nsimples, simples; 
  si32 *complex_track_nums = nums.alloc(n_complex_tracks);
  si32 *complex_track_offsets = coffsets.alloc(n_simple_tracks);
  si32 *simple_track_offsets = soffsets.alloc(n_simple_tracks);
  si32 *nsimples_per_complex = nsimples.alloc(n_simple_tracks);
  si32 *simples_per_complex_offsets = simples.alloc(n_simple_tracks);

  TaArray<track_file_scan_index_t> sindexArray;
  track_file_scan_index_t *scan_index = sindexArray.alloc(n_scans);

  // copy the header and arrays to local variables
  
  track_file_header_t header = _track_header;
  
  memcpy (complex_track_nums,_complex_track_nums,
          n_complex_tracks *  sizeof(si32));
  
  memcpy (complex_track_offsets, _complex_track_offsets,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (simple_track_offsets, _simple_track_offsets,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (nsimples_per_complex, _nsimples_per_complex,
          n_simple_tracks *  sizeof(si32));
  
  memcpy (scan_index, _scan_index,
          n_scans *  sizeof(track_file_scan_index_t));
  
  // code into network byte order
  
  header.nbytes_char = (TITAN_N_GRID_LABELS * TITAN_GRID_UNITS_LEN +
                        TRACK_FILE_HEADER_NBYTES_CHAR);
  
  ustr_clear_to_end(header.header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.data_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.storm_header_file_name, R_LABEL_LEN);
  ustr_clear_to_end(header.verify.grid.unitsx, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsy, TITAN_GRID_UNITS_LEN);
  ustr_clear_to_end(header.verify.grid.unitsz, TITAN_GRID_UNITS_LEN);

  BE_from_array_32(&header,
		   sizeof(track_file_header_t) - header.nbytes_char);
  
  BE_from_array_32(complex_track_nums,
		   n_complex_tracks *  sizeof(si32));
  
  BE_from_array_32(complex_track_offsets,
		   n_simple_tracks *  sizeof(si32));
    
  BE_from_array_32(simple_track_offsets,
		   n_simple_tracks *  sizeof(si32));
  
  BE_from_array_32(nsimples_per_complex,
		   n_simple_tracks *  sizeof(si32));
  
  BE_from_array_32(scan_index,
		   n_scans *  sizeof(track_file_scan_index_t));
  
  // write header
  
  if (ufwrite(&header, sizeof(track_file_header_t),
	      1, _track_header_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing header.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in complex track num array
  
  if (ufwrite(complex_track_nums, sizeof(si32),
	      n_complex_tracks, _track_header_file) != n_complex_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex_track_nums.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in complex track offset
  
  if (ufwrite(complex_track_offsets, sizeof(si32),
	      n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex_track_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in simple track offset
  
  if (ufwrite(simple_track_offsets, sizeof(si32), n_simple_tracks,
	      _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simple_track_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // write in scan index
  
  if (ufwrite(scan_index, sizeof(track_file_scan_index_t),
	      n_scans, _track_header_file) != n_scans) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing scan_index.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }

  // write in nsimples_per_complex

  if (ufwrite(nsimples_per_complex, sizeof(si32),
	      n_simple_tracks, _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing nsimples_per_complex.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  long offsets_pos = ftell(_track_header_file);
  
  // seek ahead of the simples_per_complex_offsets array
  
  fseek(_track_header_file, n_simple_tracks * sizeof(si32), SEEK_CUR);

  // clear offsets array

  memset(simples_per_complex_offsets, 0, n_simple_tracks * sizeof(si32));
  
  // loop through complex tracks

  for (int icomplex = 0; icomplex < n_complex_tracks; icomplex++) {

    int complex_num = _complex_track_nums[icomplex];
    int nsimples = _nsimples_per_complex[complex_num];
    simples_per_complex_offsets[complex_num] = ftell(_track_header_file);

    TaArray<si32> simpleArray;
    si32 *simples_per_complex = simpleArray.alloc(nsimples);
    memcpy(simples_per_complex, _simples_per_complex[complex_num],
	   nsimples * sizeof(si32));
    BE_from_array_32(simples_per_complex, nsimples * sizeof(si32));
		     
    // write out simple tracks array
    
    if (ufwrite(simples_per_complex, sizeof(si32), nsimples,
		_track_header_file) != nsimples) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ", "Writing simples_per_complex.");
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // icomplex 

  // write out simples_per_complex_offsets
  
  fseek(_track_header_file, offsets_pos, SEEK_SET);
  BE_from_array_32(simples_per_complex_offsets,
		   n_simple_tracks * sizeof(si32));
  
  if (ufwrite(simples_per_complex_offsets, sizeof(si32), n_simple_tracks,
	      _track_header_file) != n_simple_tracks) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simples_per_complex_offsets.");
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  flushTrackFiles();

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write simple track params at the end of the file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeSimpleParams(int track_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeSimpleParams\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _track_data_file_path);
  
  // Go to the end of the file.

  fseek(_track_data_file, 0, SEEK_END);
  long file_mark = ftell(_track_data_file);
  
  // if params have been written before, go to the stored offset.
  // If not, store offset as current file location
  
  bool rewrite = true;
  if (_simple_track_offsets[track_num] == 0) {
    _simple_track_offsets[track_num] = file_mark;
    rewrite = false;
  }
  
  // copy track params, encode and write to file
  
  simple_track_params_t simple_params = _simple_params;
  BE_from_array_32(&simple_params,
		   sizeof(simple_track_params_t));
  
  // for rewrite, move to stored offset
  
  if (rewrite) {
    fseek(_track_data_file, _simple_track_offsets[track_num], SEEK_SET);
  }
  
  if (ufwrite(&simple_params, sizeof(simple_track_params_t),
	      1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing simple track params.");
    TaStr::AddInt(_errStr, "  track_num", track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  fflush(_track_data_file);
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write complex track params
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeComplexParams(int track_num)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeSimpleParams\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _track_data_file_path);
  
  if (_complex_track_offsets[track_num] == 0) {
    
    // params have not been written before.
    //
    // Two steps: 1) look for a slot which has been freed
    //               up when a complex track was consolidated.
    //            2) If no available slot, use end of file
    
    int lowest_avail_slot = _lowest_avail_complex_slot;
    si32 *offset_p = _complex_track_offsets + lowest_avail_slot;
    si32 avail_offset;
    bool slot_found = false;
      
    for (int i = lowest_avail_slot; i < track_num; i++) {
      
      if (*offset_p < 0) {
	
	// avail slot found
	
	avail_offset = -(*offset_p);
	*offset_p = 0;
	_lowest_avail_complex_slot = i + 1;
	slot_found = true;
	break;
	
      } // if (*offset_p < 0) 
      
      offset_p++;
      
    } // i 
    
    if (slot_found) {
      
      _complex_track_offsets[track_num] = avail_offset;
      fseek(_track_data_file, avail_offset, SEEK_SET);
      
    } else {
      
      fseek(_track_data_file, 0, SEEK_END);
      _complex_track_offsets[track_num] = ftell(_track_data_file);
      _lowest_avail_complex_slot = track_num + 1;
      
    }
    
  } else {
    
    // params have been stored before, so go to the stored offset
    
    fseek(_track_data_file, _complex_track_offsets[track_num], SEEK_SET);
    
  }
  
  // copy track params, encode and write to file
  
  complex_track_params_t complex_params = _complex_params;
  
  BE_from_array_32(&complex_params,
		   sizeof(complex_track_params_t));
  
  if (ufwrite(&complex_params, sizeof(complex_track_params_t),
	      1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ", "Writing complex track params.");
    TaStr::AddInt(_errStr, "  track_num", track_num);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  // flush the file buffer
  
  fflush(_track_data_file);
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write an entry for a track in the track data file
//
// The entry is written at the end of the file
//
// returns offset of last entry written on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

long TitanFile::writeEntry(int prev_in_track_offset,
				int prev_in_scan_offset)
     
{
  
  _clearErrStr();
  _errStr += "ERROR - TitanFile::writeEntry\n";
  TaStr::AddStr(_errStr, "  Writing to file: ", _track_data_file_path);

  long file_mark;

  // Go to the end of the file and save the file position.

  fseek(_track_data_file, 0, SEEK_END);
  file_mark = ftell(_track_data_file);
  
  // If prev_in_track_offset is non-zero (which indicates that this is not
  // the first entry in a track) read in the entry at that location,
  // update the next_entry prev_in_track_offset with the current file
  // location and write back to file
  
  if (prev_in_track_offset != 0) {
    
    // move to the entry prev_in_track_offset in the file
    
    fseek(_track_data_file, prev_in_track_offset, SEEK_SET);
    
    // read in entry
    
    track_file_entry_t entry;
    if (ufread(&entry, sizeof(track_file_entry_t),
	       1, _track_data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Reading track entry to update in_track_offset");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // store next_entry_offset, swap
    
    entry.next_entry_offset = file_mark;
    BE_from_array_32(&entry.next_entry_offset,
		     sizeof(entry.next_entry_offset));
    
    // move back to offset
    
    fseek(_track_data_file, prev_in_track_offset, SEEK_SET);
    
    // rewrite entry
    
    if (ufwrite(&entry, sizeof(track_file_entry_t),
		1, _track_data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Re-writing track entry.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // if (prev_in_track_offset == 0) 
  
  // If prev_in_scan_offset is non-zero (which indicates that this is not
  // the first entry in a track) read in the entry at that location,
  // update the next_scan_entry_offset with the current file
  // location and write back to file
  
  if (prev_in_scan_offset != 0) {
    
    // move to the entry prev_in_scan_offset in the file
    
    fseek(_track_data_file, prev_in_scan_offset, SEEK_SET);
    
    // read in entry
    
    track_file_entry_t entry;
    if (ufread(&entry, sizeof(track_file_entry_t),
	       1, _track_data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Reading track entry to update in_track_offset.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
    // store next_entry_offset, swap
    
    entry.next_scan_entry_offset = file_mark;
    BE_from_array_32(&entry.next_scan_entry_offset,
		     sizeof(entry.next_scan_entry_offset));
    
    // move back to offset
    
    fseek(_track_data_file, prev_in_scan_offset, SEEK_SET);
    
    // rewrite entry
    
    if (ufwrite(&entry, sizeof(track_file_entry_t),
		1, _track_data_file) != 1) {
      int errNum = errno;
      TaStr::AddStr(_errStr, "  ",
		    "Re-writing track entry.");
      TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
      TaStr::AddStr(_errStr, "  ", strerror(errNum));
      return -1;
    }
    
  } // if (prev_in_scan_offset == 0) 
  
  // go to end of file to write entry structure
  
  fseek(_track_data_file, 0, SEEK_END);
  
  // copy entry to local variable
  
  track_file_entry_t entry = _entry;
  
  // set entry offsets
  
  entry.prev_entry_offset = prev_in_track_offset;
  entry.this_entry_offset = file_mark;
  entry.next_entry_offset = 0;
  
  // swap
  
  BE_from_array_32(&entry, sizeof(track_file_entry_t));
  
  // write entry
  
  if (ufwrite(&entry, sizeof(track_file_entry_t),
	      1, _track_data_file) != 1) {
    int errNum = errno;
    TaStr::AddStr(_errStr, "  ",
		  "Writing track entry.");
    TaStr::AddInt(_errStr, "  offset: ", _entry.this_entry_offset);
    TaStr::AddStr(_errStr, "  ", strerror(errNum));
    return -1;
  }
  
  return (file_mark);
  
}
