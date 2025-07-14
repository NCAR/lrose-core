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
#include <cassert>

#include <filesystem>
using Fpath = std::filesystem::path;

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
  //  _scan_offsets = nullptr;
  // _storm_num = 0;

  // _max_scans = 0;
  _max_storms = 0;
  _max_layers = 0;
  _max_hist = 0;
  _max_runs = 0;
  _max_proj_runs = 0;
  
  _nScans = 0;
  _sumStorms = 0;
  _sumLayers = 0;
  _sumHist = 0;
  _sumRuns = 0;
  _sumProjRuns = 0;

  // tracks

  MEM_zero(_track_header);
  MEM_zero(_simple_params);
  MEM_zero(_complex_params);
  MEM_zero(_entry);

  _scan_index = nullptr;
  _scan_entries = nullptr;
  _track_utime = nullptr;
  
  _complex_track_nums = nullptr;
  // _complex_track_offsets = nullptr;
  // _simple_track_offsets = nullptr;
  _n_simples_per_complex = nullptr;
  _simples_per_complex_offsets = nullptr;
  _simples_per_complex_1D = nullptr;
  _simples_per_complex_2D = nullptr;

  _first_entry = true;

  _n_scan_entries = 0;
  _lowest_avail_complex_slot = 0;

  _n_simple_allocated = 0;
  _n_complex_allocated = 0;
  _n_simples_per_complex_2D_allocated = 0;
  _n_scan_entries_allocated = 0;
  _n_scan_index_allocated = 0;
  _n_utime_allocated = 0;

  _prevEntryOffset = 0;
  // _prev_in_scan_offset = 0;

  _horizGridUnits = KM;
  _horizGridUnitsPerHr = KM_PER_HR;
  _speedUnits = KM_PER_HR;
  _speedUnitsPerHr = KM_PER_HR_PER_HR;

  // legacy v5 file?
  
  _isLegacyV5Format = false;

  // attributes
  
  _convention = "TitanStormTracking";
  _version = "1.0";
  _title = "";
  _institution = "";
  _source = "";
  _comment = "";


}

////////////////////////////////////////////////////////////
// destructor

TitanFile::~TitanFile()

{

  closeFile();
  freeStormsAll();
  freeTracksAll();
  
}

/////////////////////////////////////////
// Open file from path

int TitanFile::openFile(const string &path,
                        NcxxFile::FileMode mode)
  
{
  
  _filePath = path;
  _fileMode = mode;
  
  // file format

  Path filePath(path);
  if (filePath.getExt() == "nc") {
    _isLegacyV5Format = false;
    _lockPath = filePath.getDirectory() + PATH_DELIM + "." + filePath.getFile() + ".lock";
  } else {
    _isLegacyV5Format = true;
    return _openLegacyFiles(path, mode);
  }
  
  // ensure the directory exists

  if (mode != NcxxFile::read) {
    filePath.makeDirRecurse();
  }
  
  // open file

  _ncFile.close();
  try {
    _ncFile.open(path, mode);
  } catch (NcxxException& e) {
    _addErrStr("ERROR - TitanFile::openFile");
    _addErrStr("  Cannot open file: ", path);
    _addErrStr("  exception: ", e.what());
    return -1;
  }

  // add global attributes

  if (mode != NcxxFile::read) {
    _setGlobalAttributes();
  }
  
  // set up netcdf groups

  _setUpGroups();

  // set up the dimensions

  _setUpDims();
  
  // set up netcdf variables

  _setUpVars();

  // open lock file at the low level

  _lockId = open(_lockPath.c_str(), O_CREAT | O_RDWR, 0666);
  if (_lockId < 0) {
    int errNum = errno;
    _addErrStr("ERROR - TitanFile::openFile");
    _addErrStr("  Cannot open lock file: ", _lockPath);
    _addErrStr(strerror(errNum));
    return -1;
  }
  
  return 0;
  
}
  
/////////////////////////////////////////
// Open file from dir and date

int TitanFile::openFile(const string &dir,
                        time_t date,
                        NcxxFile::FileMode mode,
                        bool isLegacyV5Format /* = false */)
  
{

  // compute path
  
  DateTime dtime(date);
  string pathStr = dir + PATH_DELIM + "titan_" + dtime.getDateStrPlain();
  _isLegacyV5Format = isLegacyV5Format;
  if (_isLegacyV5Format) {
    pathStr.append(".th5");
  } else {
    pathStr.append(".nc");
  }

  return openFile(pathStr, mode);

}
  
/////////////////////////////////////////////////////////
// Open legacy files from path

int TitanFile::_openLegacyFiles(const string &path,
                                NcxxFile::FileMode mode)
  
{
  
  Fpath basePath(path);
  string stormHeaderPath = basePath.replace_extension(".sh5").string();
  string trackHeaderPath = basePath.replace_extension(".th5").string();
  string fmode("r");
  switch (mode) {
    case NcxxFile::read:
      fmode = "r";
      break;
    case NcxxFile::write:
      fmode = "w+";
      break;
    case NcxxFile::replace:
    case NcxxFile::newFile:
      fmode = "w";
      break;
  }
  
  if (fmode != "r") {
    Path stormPath(stormHeaderPath);
    stormPath.makeDirRecurse();
  }
  
  if (_sFile.OpenFiles(fmode.c_str(), stormHeaderPath.c_str(), "sd5")) {
    _addErrStr("ERROR - TitanFile::openFile");
    _addErrStr(" Opening storm files, header path: ", stormHeaderPath);
    return -1;
  }

  if (_tFile.OpenFiles(fmode.c_str(), trackHeaderPath.c_str(), "td5")) {
    _addErrStr("ERROR - TitanFile::openFile");
    _addErrStr(" Opening track files, header path: ", trackHeaderPath);
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////
// close file

void TitanFile::closeFile()

{

  if (_isLegacyV5Format) {
    _sFile.CloseFiles();
    _tFile.CloseFiles();
  } else {
    _ncFile.close();
  }
  
}

/////////////////////////////////////////
// get paths

string TitanFile::getStormFileHeaderPath() const {
  if (_isLegacyV5Format) {
    return _sFile.header_file_path();
  } else {
    return "NA";
  }
}

string TitanFile::getStormFileDataPath() const {
  if (_isLegacyV5Format) {
    return _sFile.data_file_path();
  } else {
    return "NA";
  }
}

string TitanFile::getTrackFileHeaderPath() const {
  if (_isLegacyV5Format) {
    return _tFile.header_file_path();
  } else {
    return "NA";
  }
}

string TitanFile::getTrackFileDataPath() const {
  if (_isLegacyV5Format) {
    return _tFile.data_file_path();
  } else {
    return "NA";
  }
}

/////////////////////////////////////////
// set global attributes

void TitanFile::_setGlobalAttributes()
{
  if (_fileMode != NcxxFile::read) {
    _ncFile.putAtt(VERSION, _version);
    _ncFile.putAtt(CONVENTION, _convention);
    _ncFile.putAtt(TITLE, _title);
    _ncFile.putAtt(INSTITUTION, _institution);
    _ncFile.putAtt(SOURCE, _source);
    _ncFile.putAtt(COMMENT, _comment);
  }
}

/////////////////////////////////////////
// set up groups

void TitanFile::_setUpGroups()
{

  _scansGroup = _getGroup(SCANS, _ncFile);
  _stormsGroup = _getGroup(STORMS, _ncFile);
  _tracksGroup = _getGroup(TRACKS, _ncFile);

  _gpropsGroup = _getGroup(GPROPS, _stormsGroup);
  _layersGroup = _getGroup(LAYERS, _stormsGroup);
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

  _scansDim = _getDim(N_SCANS, _scansGroup);
  _stormsDim = _getDim(N_STORMS, _gpropsGroup);
  _simpleDim = _getDim(N_SIMPLE, _simpleGroup);
  _maxComplexDim = _getDim(MAX_COMPLEX, _complexGroup);
  _entriesDim = _getDim(N_ENTRIES, _entriesGroup);
  _polyDim = _getDim(N_POLY, N_POLY_SIDES, _gpropsGroup);
  _layersDim = _getDim(N_LAYERS, _layersGroup);
  _histDim = _getDim(N_HIST, _histGroup);
  _runsDim = _getDim(N_RUNS, _runsGroup);
  _projRunsDim = _getDim(N_PROJ_RUNS, _projRunsGroup);
  _maxFcastWtsDim = _getDim(MAX_FORECAST_WEIGHTS, MAX_NWEIGHTS_FORECAST, _tracksGroup);
  _maxParentsDim = _getDim(MAX_PARENTS_, MAX_PARENTS, _simpleGroup);
  _maxChildrenDim = _getDim(MAX_CHILDREN_, MAX_CHILDREN, _simpleGroup);
  
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
// get scalar variable

NcxxVar TitanFile::_getVar(const std::string& name,
                           const NcxxType& ncType,
                           NcxxGroup &group,
                           std::string units /* = "" */)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    var = group.addVar(name, ncType);
    var.setDefaultFillValue();
    if (units.size() > 0) {
      var.putAtt(UNITS, units);
    }
    _setFillValue(var);
  }
  return var;
}

/////////////////////////////////////////
// get array variable

NcxxVar TitanFile::_getVar(const std::string& name,
                           const NcxxType& ncType,
                           const NcxxDim& dim,
                           NcxxGroup &group,
                           std::string units /* = "" */)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    var = group.addVar(name, ncType, dim);
    var.setDefaultFillValue();
    var.setCompression(false, true, 4);
    if (units.size() > 0) {
      var.putAtt(UNITS, units);
    }
    _setFillValue(var);
  }
  return var;
}

/////////////////////////////////////////
// get 2D array variable

NcxxVar TitanFile::_getVar(const std::string& name,
                           const NcxxType& ncType,
                           const NcxxDim& dim0,
                           const NcxxDim& dim1,
                           NcxxGroup &group,
                           std::string units /* = "" */)
{
  NcxxVar var = group.getVar(name);
  if (var.isNull()) {
    std::vector<NcxxDim> dimVec;
    dimVec.push_back(dim0);
    dimVec.push_back(dim1);
    var = group.addVar(name, ncType, dimVec);
    var.setDefaultFillValue();
    var.setCompression(false, true, 4);
    if (units.size() > 0) {
      var.putAtt(UNITS, units);
    }
    _setFillValue(var);
  }
  return var;
}

////////////////////////////////////////
// set default fill value, based on type
// throws NcxxException on error

void TitanFile::_setFillValue(NcxxVar &var)
  
{

  nc_type vtype = var.getType().getId();
  if (vtype == NC_DOUBLE) {
    var.addScalarAttr("_FillValue", missingDouble);
    return;
  }
  if (vtype == NC_FLOAT) {
    var.addScalarAttr("_FillValue", missingFloat);
    return;
  }
  if (vtype == NC_INT64) {
    var.addScalarAttr("_FillValue", missingInt64);
    return;
  }
  if (vtype == NC_INT) {
    var.addScalarAttr("_FillValue", missingInt32);
    return;
  }
  if (vtype == NC_LONG) {
    var.addScalarAttr("_FillValue", missingInt32);
    return;
  }
  if (vtype == NC_SHORT) {
    var.addScalarAttr("_FillValue", missingInt16);
    return;
  }
  if (vtype == NC_UBYTE) {
    var.addScalarAttr("_FillValue", missingInt08);
    return;
  }
}

/////////////////////////////////////////
// set up variables

void TitanFile::_setUpVars()

{

  // top level
  
  _topLevelVars.file_time =
    _getVar(FILE_TIME, NcxxType::nc_INT64, _ncFile, TIME0);
  _topLevelVars.start_time =
    _getVar(START_TIME, NcxxType::nc_INT64, _ncFile, TIME0);
  _topLevelVars.end_time =
    _getVar(END_TIME, NcxxType::nc_INT64, _ncFile, TIME0);
  
  _topLevelVars.n_scans =
    _getVar(N_SCANS, NcxxType::nc_INT, _ncFile);

  _topLevelVars.sum_storms =
    _getVar(SUM_STORMS, NcxxType::nc_INT, _ncFile);
  _topLevelVars.sum_layers =
    _getVar(SUM_LAYERS, NcxxType::nc_INT, _ncFile);
  _topLevelVars.sum_hist =
    _getVar(SUM_HIST, NcxxType::nc_INT, _ncFile);
  _topLevelVars.sum_runs =
    _getVar(SUM_RUNS, NcxxType::nc_INT, _ncFile);
  _topLevelVars.sum_proj_runs =
    _getVar(SUM_PROJ_RUNS, NcxxType::nc_INT, _ncFile);

  _topLevelVars.max_simple_track_num =
    _getVar(MAX_SIMPLE_TRACK_NUM, NcxxType::nc_INT, _ncFile);
  _topLevelVars.max_complex_track_num =
    _getVar(MAX_COMPLEX_TRACK_NUM, NcxxType::nc_INT, _ncFile);

  // scans

  _scanVars.scan_min_z =
    _getVar(SCAN_MIN_Z, NcxxType::nc_FLOAT, _scansDim, _scansGroup, KM);
  _scanVars.scan_delta_z =
    _getVar(SCAN_DELTA_Z, NcxxType::nc_FLOAT, _scansDim, _scansGroup, KM);
  _scanVars.scan_num =
    _getVar(SCAN_NUM, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_nstorms =
    _getVar(SCAN_NSTORMS, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_time =
    _getVar(SCAN_TIME, NcxxType::nc_INT64, _scansDim, _scansGroup);
  _scanVars.scan_gprops_offset =
    _getVar(SCAN_GPROPS_OFFSET, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_first_offset =
    _getVar(SCAN_FIRST_OFFSET, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_last_offset =
    _getVar(SCAN_LAST_OFFSET, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_ht_of_freezing =
    _getVar(SCAN_HT_OF_FREEZING, NcxxType::nc_FLOAT, _scansDim, _scansGroup, KM);

  _scanVars.grid_nx =
    _getVar(GRID_NX, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.grid_ny =
    _getVar(GRID_NY, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.grid_nz =
    _getVar(GRID_NZ, NcxxType::nc_INT, _scansDim, _scansGroup);

  _scanVars.grid_minx =
    _getVar(GRID_MINX, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_miny =
    _getVar(GRID_MINY, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_minz =
    _getVar(GRID_MINZ, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, KM);

  _scanVars.grid_dx =
    _getVar(GRID_DX, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_dy =
    _getVar(GRID_DY, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_dz =
    _getVar(GRID_DZ, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, KM);

  _scanVars.grid_dz_constant =
    _getVar(GRID_DZ_CONSTANT, NcxxType::nc_INT, _scansDim, _scansGroup);

  _scanVars.grid_sensor_x =
    _getVar(GRID_SENSOR_X, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_sensor_y =
    _getVar(GRID_SENSOR_Y, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, _horizGridUnits);
  _scanVars.grid_sensor_z =
    _getVar(GRID_SENSOR_Z, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, KM);

  _scanVars.grid_sensor_lat =
    _getVar(GRID_SENSOR_LAT, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.grid_sensor_lon =
    _getVar(GRID_SENSOR_LON, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);

  _scanVars.grid_unitsx =
    _getVar(GRID_UNITSX, NcxxType::nc_STRING, _scansDim, _scansGroup);
  _scanVars.grid_unitsy =
    _getVar(GRID_UNITSY, NcxxType::nc_STRING, _scansDim, _scansGroup);
  _scanVars.grid_unitsz =
    _getVar(GRID_UNITSZ, NcxxType::nc_STRING, _scansDim, _scansGroup);

  _scanVars.proj_type =
    _getVar(PROJ_TYPE, NcxxType::nc_INT, _scansDim, _scansGroup);
  
  _scanVars.proj_origin_lat =
    _getVar(PROJ_ORIGIN_LAT, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.proj_origin_lon =
    _getVar(PROJ_ORIGIN_LON, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.proj_rotation =
    _getVar(PROJ_ROTATION, NcxxType::nc_FLOAT, _scansDim, _scansGroup, DEG);

  _scanVars.proj_lat1 =
    _getVar(PROJ_LAT1, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.proj_lat2 =
    _getVar(PROJ_LAT2, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.proj_tangent_lat =
    _getVar(PROJ_TANGENT_LAT, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);
  _scanVars.proj_tangent_lon =
    _getVar(PROJ_TANGENT_LON, NcxxType::nc_DOUBLE, _scansDim, _scansGroup, DEG);

  _scanVars.proj_pole_type =
    _getVar(PROJ_POLE_TYPE, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.proj_central_scale =
    _getVar(PROJ_CENTRAL_SCALE, NcxxType::nc_FLOAT, _scansDim, _scansGroup);

  _scanVars.scan_gprops_offset_0 =
    _getVar(SCAN_GPROPS_OFFSET_0, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_layer_offset_0 =
    _getVar(SCAN_LAYER_OFFSET_0, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_hist_offset_0 =
    _getVar(SCAN_HIST_OFFSET_0, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_runs_offset_0 =
    _getVar(SCAN_RUNS_OFFSET_0, NcxxType::nc_INT, _scansDim, _scansGroup);
  _scanVars.scan_proj_runs_offset_0 =
    _getVar(SCAN_PROJ_RUNS_OFFSET_0, NcxxType::nc_INT, _scansDim, _scansGroup);

  // add projection attributes
  
  _addProjectionFlagAttributes();

  // storm params

  _sparamsVars.low_dbz_threshold =
    _getVar(LOW_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, DBZ);
  _sparamsVars.high_dbz_threshold =
    _getVar(HIGH_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, DBZ);
  _sparamsVars.dbz_hist_interval =
    _getVar(DBZ_HIST_INTERVAL, NcxxType::nc_FLOAT, _stormsGroup, DBZ);
  _sparamsVars.hail_dbz_threshold =
    _getVar(HAIL_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, DBZ);
  
  _sparamsVars.base_threshold =
    _getVar(BASE_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.top_threshold =
    _getVar(TOP_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.min_storm_size =
    _getVar(MIN_STORM_SIZE, NcxxType::nc_FLOAT, _stormsGroup, KM3);
  _sparamsVars.max_storm_size =
    _getVar(MAX_STORM_SIZE, NcxxType::nc_FLOAT, _stormsGroup, KM3);

  _sparamsVars.morphology_erosion_threshold =
    _getVar(MORPHOLOGY_EROSION_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.morphology_refl_divisor =
    _getVar(MORPHOLOGY_REFL_DIVISOR, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.min_radar_tops =
    _getVar(MIN_RADAR_TOPS, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.tops_edge_margin =
    _getVar(TOPS_EDGE_MARGIN, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.z_p_coeff =
    _getVar(Z_P_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_p_exponent =
    _getVar(Z_P_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_m_coeff =
    _getVar(Z_M_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.z_m_exponent =
    _getVar(Z_M_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_vert_aspect =
    _getVar(SECTRIP_VERT_ASPECT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_horiz_aspect =
    _getVar(SECTRIP_HORIZ_ASPECT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.sectrip_orientation_error =
    _getVar(SECTRIP_ORIENTATION_ERROR, NcxxType::nc_FLOAT, _stormsGroup, DEG);
  _sparamsVars.poly_start_az =
    _getVar(POLY_START_AZ, NcxxType::nc_FLOAT, _stormsGroup, DEG);
  _sparamsVars.poly_delta_az =
    _getVar(POLY_DELTA_AZ, NcxxType::nc_FLOAT, _stormsGroup, DEG);
  _sparamsVars.check_morphology =
    _getVar(CHECK_MORPHOLOGY, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.check_tops =
    _getVar(CHECK_TOPS, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.vel_available =
    _getVar(VEL_AVAILABLE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.n_poly_sides =
    _getVar(N_POLY_SIDES_, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.ltg_count_time =
    _getVar(LTG_COUNT_TIME, NcxxType::nc_INT64, _stormsGroup, TIME0);
  _sparamsVars.ltg_count_margin_km =
    _getVar(LTG_COUNT_MARGIN_KM, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.hail_z_m_coeff =
    _getVar(HAIL_Z_M_COEFF, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_z_m_exponent =
    _getVar(HAIL_Z_M_EXPONENT, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.hail_mass_dbz_threshold =
    _getVar(HAIL_MASS_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.gprops_union_type =
    _getVar(GPROPS_UNION_TYPE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.tops_dbz_threshold =
    _getVar(TOPS_DBZ_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup, DBZ);
  _sparamsVars.precip_computation_mode =
    _getVar(PRECIP_COMPUTATION_MODE, NcxxType::nc_INT, _stormsGroup);
  _sparamsVars.precip_plane_ht =
    _getVar(PRECIP_PLANE_HT, NcxxType::nc_FLOAT, _stormsGroup, KM);
  _sparamsVars.low_convectivity_threshold =
    _getVar(LOW_CONVECTIVITY_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);
  _sparamsVars.high_convectivity_threshold =
    _getVar(HIGH_CONVECTIVITY_THRESHOLD, NcxxType::nc_FLOAT, _stormsGroup);

  // storm global props

  _gpropsVars.vol_centroid_x =
    _getVar(VOL_CENTROID_X, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.vol_centroid_y =
    _getVar(VOL_CENTROID_Y, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.vol_centroid_z =
    _getVar(VOL_CENTROID_Z, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM);
  _gpropsVars.refl_centroid_x =
    _getVar(REFL_CENTROID_X, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.refl_centroid_y =
    _getVar(REFL_CENTROID_Y, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.refl_centroid_z =
    _getVar(REFL_CENTROID_Z, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM);
  _gpropsVars.top =
    _getVar(TOP, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM);
  _gpropsVars.base =
    _getVar(BASE, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM);
  _gpropsVars.volume =
    _getVar(VOLUME, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM3);
  _gpropsVars.area_mean =
    _getVar(AREA_MEAN, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM2);
  _gpropsVars.precip_flux =
    _getVar(PRECIP_FLUX, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, M3_PER_SEC);
  _gpropsVars.mass =
    _getVar(MASS, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KTONS);
  _gpropsVars.tilt_angle =
    _getVar(TILT_ANGLE, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DEG);
  _gpropsVars.tilt_dirn =
    _getVar(TILT_DIRN, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DEG);
  _gpropsVars.dbz_max =
    _getVar(DBZ_MAX, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DBZ);
  _gpropsVars.dbz_mean =
    _getVar(DBZ_MEAN, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DBZ);
  _gpropsVars.dbz_max_gradient =
    _getVar(DBZ_MAX_GRADIENT, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DBZ_PER_KM);
  _gpropsVars.dbz_mean_gradient =
    _getVar(DBZ_MEAN_GRADIENT, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, DBZ_PER_KM);
  _gpropsVars.ht_of_dbz_max =
    _getVar(HT_OF_DBZ_MAX, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM);
  _gpropsVars.rad_vel_mean =
    _getVar(RAD_VEL_MEAN, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, M_PER_SEC);
  _gpropsVars.rad_vel_sd =
    _getVar(RAD_VEL_SD, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, M_PER_SEC);
  _gpropsVars.vorticity =
    _getVar(VORTICITY, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, PER_SEC);
  _gpropsVars.precip_area =
    _getVar(PRECIP_AREA, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KM2);

  _gpropsVars.precip_area_centroid_x =
    _getVar(PRECIP_AREA_CENTROID_X, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.precip_area_centroid_y =
    _getVar(PRECIP_AREA_CENTROID_Y, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.precip_area_orientation =
    _getVar(PRECIP_AREA_ORIENTATION, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, DEG);
  _gpropsVars.precip_area_minor_radius =
    _getVar(PRECIP_AREA_MINOR_RADIUS, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.precip_area_major_radius =
    _getVar(PRECIP_AREA_MAJOR_RADIUS, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);

  _gpropsVars.proj_area = _getVar(PROJ_AREA, NcxxType::nc_FLOAT,
                                  _stormsDim, _gpropsGroup, KM2);
  _gpropsVars.proj_area_centroid_x =
    _getVar(PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.proj_area_centroid_y =
    _getVar(PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.proj_area_orientation =
    _getVar(PROJ_AREA_ORIENTATION, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, DEG);
  _gpropsVars.proj_area_minor_radius =
    _getVar(PROJ_AREA_MINOR_RADIUS, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.proj_area_major_radius =
    _getVar(PROJ_AREA_MAJOR_RADIUS, NcxxType::nc_FLOAT,
            _stormsDim, _gpropsGroup, _horizGridUnits);
  _gpropsVars.proj_area_polygon =
    _getVar(PROJ_AREA_POLYGON, NcxxType::nc_FLOAT,
            _stormsDim, _polyDim, _gpropsGroup, _horizGridUnits);

  _gpropsVars.storm_num =
    _getVar(STORM_NUM, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.n_layers =
    _getVar(N_LAYERS, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.base_layer =
    _getVar(BASE_LAYER, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.n_dbz_intervals =
    _getVar(N_DBZ_INTERVALS, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.n_runs =
    _getVar(N_RUNS, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.n_proj_runs =
    _getVar(N_PROJ_RUNS, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.top_missing =
    _getVar(TOP_MISSING, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.range_limited =
    _getVar(RANGE_LIMITED, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.second_trip =
    _getVar(SECOND_TRIP, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_present =
    _getVar(HAIL_PRESENT, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.anom_prop =
    _getVar(ANOM_PROP, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.bounding_min_ix =
    _getVar(BOUNDING_MIN_IX, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.bounding_min_iy =
    _getVar(BOUNDING_MIN_IY, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.bounding_max_ix =
    _getVar(BOUNDING_MAX_IX, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.bounding_max_iy =
    _getVar(BOUNDING_MAX_IY, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.layer_props_offset =
    _getVar(LAYER_PROPS_OFFSET, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.dbz_hist_offset =
    _getVar(DBZ_HIST_OFFSET, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.runs_offset =
    _getVar(RUNS_OFFSET, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.proj_runs_offset =
    _getVar(PROJ_RUNS_OFFSET, NcxxType::nc_INT, _stormsDim, _gpropsGroup);
  _gpropsVars.vil_from_maxz =
    _getVar(VIL_FROM_MAXZ, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KG_PER_M2);
  _gpropsVars.ltg_count =
    _getVar(LTG_COUNT, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.convectivity_median =
    _getVar(CONVECTIVITY_MEDIAN, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_FOKRcategory =
    _getVar(HAIL_FOKRCATEGORY, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_waldvogelProbability =
    _getVar(HAIL_WALDVOGELPROBABILITY, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_hailMassAloft =
    _getVar(HAIL_HAILMASSALOFT, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup, KTONS);
  _gpropsVars.hail_vihm =
    _getVar(HAIL_VIHM, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_poh =
    _getVar(HAIL_POH, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_shi =
    _getVar(HAIL_SHI, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_posh =
    _getVar(HAIL_POSH, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  _gpropsVars.hail_mehs =
    _getVar(HAIL_MEHS, NcxxType::nc_FLOAT, _stormsDim, _gpropsGroup);
  
  // storm layer props
  
  _lpropsVars.vol_centroid_x =
    _getVar(VOL_CENTROID_X, NcxxType::nc_FLOAT, _layersDim, _layersGroup, _horizGridUnits);
  _lpropsVars.vol_centroid_y =
    _getVar(VOL_CENTROID_Y, NcxxType::nc_FLOAT, _layersDim, _layersGroup, _horizGridUnits);
  _lpropsVars.refl_centroid_x =
    _getVar(REFL_CENTROID_X, NcxxType::nc_FLOAT, _layersDim, _layersGroup, _horizGridUnits);
  _lpropsVars.refl_centroid_y =
    _getVar(REFL_CENTROID_Y, NcxxType::nc_FLOAT, _layersDim, _layersGroup, _horizGridUnits);
  _lpropsVars.area =
    _getVar(AREA, NcxxType::nc_FLOAT, _layersDim, _layersGroup, KM2);
  _lpropsVars.dbz_max =
    _getVar(DBZ_MAX, NcxxType::nc_FLOAT, _layersDim, _layersGroup, DBZ);
  _lpropsVars.dbz_mean =
    _getVar(DBZ_MEAN, NcxxType::nc_FLOAT, _layersDim, _layersGroup, DBZ);
  _lpropsVars.mass =
    _getVar(MASS, NcxxType::nc_FLOAT, _layersDim, _layersGroup, KTONS);
  _lpropsVars.rad_vel_mean =
    _getVar(RAD_VEL_MEAN, NcxxType::nc_FLOAT, _layersDim, _layersGroup, M_PER_SEC);
  _lpropsVars.rad_vel_sd =
    _getVar(RAD_VEL_SD, NcxxType::nc_FLOAT, _layersDim, _layersGroup, M_PER_SEC);
  _lpropsVars.vorticity =
    _getVar(VORTICITY, NcxxType::nc_FLOAT, _layersDim, _layersGroup, PER_SEC);
  _lpropsVars.convectivity_median =
    _getVar(CONVECTIVITY_MEDIAN, NcxxType::nc_FLOAT, _layersDim, _layersGroup);

  // reflectivity histograms

  _histVars.percent_volume =
    _getVar(PERCENT_VOLUME, NcxxType::nc_FLOAT, _histDim, _histGroup);
  _histVars.percent_area =
    _getVar(PERCENT_AREA, NcxxType::nc_FLOAT, _histDim, _histGroup);

  // storm runs

  _runsVars.run_ix =
    _getVar(RUN_IX, NcxxType::nc_INT, _runsDim, _runsGroup);
  _runsVars.run_iy =
    _getVar(RUN_IY, NcxxType::nc_INT, _runsDim, _runsGroup);
  _runsVars.run_iz =
    _getVar(RUN_IZ, NcxxType::nc_INT, _runsDim, _runsGroup);
  _runsVars.run_len =
    _getVar(RUN_LEN, NcxxType::nc_INT, _runsDim, _runsGroup);

  // storm proj runs

  _projRunsVars.run_ix =
    _getVar(RUN_IX, NcxxType::nc_INT, _projRunsDim, _projRunsGroup);
  _projRunsVars.run_iy =
    _getVar(RUN_IY, NcxxType::nc_INT, _projRunsDim, _projRunsGroup);
  _projRunsVars.run_iz =
    _getVar(RUN_IZ, NcxxType::nc_INT, _projRunsDim, _projRunsGroup);
  _projRunsVars.run_len =
    _getVar(RUN_LEN, NcxxType::nc_INT, _projRunsDim, _projRunsGroup);

  // tracking parameters

  _tparamsVars.forecast_weights =
    _getVar(FORECAST_WEIGHTS, NcxxType::nc_FLOAT, _maxFcastWtsDim, _tracksGroup);
  _tparamsVars.weight_distance =
    _getVar(WEIGHT_DISTANCE, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.weight_delta_cube_root_volume =
    _getVar(WEIGHT_DELTA_CUBE_ROOT_VOLUME, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.merge_split_search_ratio =
    _getVar(MERGE_SPLIT_SEARCH_RATIO, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.max_tracking_speed =
    _getVar(MAX_TRACKING_SPEED, NcxxType::nc_FLOAT, _tracksGroup, KM_PER_HR);
  _tparamsVars.max_speed_for_valid_forecast =
    _getVar(MAX_SPEED_FOR_VALID_FORECAST, NcxxType::nc_FLOAT, _tracksGroup, KM_PER_HR);
  _tparamsVars.parabolic_growth_period =
    _getVar(PARABOLIC_GROWTH_PERIOD, NcxxType::nc_FLOAT, _tracksGroup, SECONDS);
  _tparamsVars.smoothing_radius =
    _getVar(SMOOTHING_RADIUS, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _tparamsVars.min_fraction_overlap =
    _getVar(MIN_FRACTION_OVERLAP, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.min_sum_fraction_overlap =
    _getVar(MIN_SUM_FRACTION_OVERLAP, NcxxType::nc_FLOAT, _tracksGroup);
  _tparamsVars.scale_forecasts_by_history =
    _getVar(SCALE_FORECASTS_BY_HISTORY, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.use_runs_for_overlaps =
    _getVar(USE_RUNS_FOR_OVERLAPS, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.grid_type =
    _getVar(GRID_TYPE, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.nweights_forecast =
    _getVar(NWEIGHTS_FORECAST, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.forecast_type =
    _getVar(FORECAST_TYPE, NcxxType::nc_INT, _tracksGroup);
  _tparamsVars.max_delta_time =
    _getVar(MAX_DELTA_TIME, NcxxType::nc_INT, _tracksGroup, SECONDS);
  _tparamsVars.min_history_for_valid_forecast =
    _getVar(MIN_HISTORY_FOR_VALID_FORECAST, NcxxType::nc_INT, _tracksGroup, SECONDS);
  _tparamsVars.spatial_smoothing =
    _getVar(SPATIAL_SMOOTHING, NcxxType::nc_INT, _tracksGroup);

  // tracking state
  
  _tstateVars.n_simple_tracks =
    _getVar(N_SIMPLE_TRACKS, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.n_complex_tracks =
    _getVar(N_COMPLEX_TRACKS, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.tracking_valid =
    _getVar(TRACKING_VALID, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.tracking_modify_code =
    _getVar(TRACKING_MODIFY_CODE, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.n_samples_for_forecast_stats =
    _getVar(N_SAMPLES_FOR_FORECAST_STATS, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.last_scan_num =
    _getVar(LAST_SCAN_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_simple_track_num =
    _getVar(MAX_SIMPLE_TRACK_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_complex_track_num =
    _getVar(MAX_COMPLEX_TRACK_NUM, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_parents =
    _getVar(MAX_PARENTS_, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_children =
    _getVar(MAX_CHILDREN_, NcxxType::nc_INT, _tracksGroup);
  _tstateVars.max_nweights_forecast =
    _getVar(MAX_NWEIGHTS_FORECAST_, NcxxType::nc_INT, _tracksGroup);

  // global bias for forecasts

  _globalBiasVars.proj_area_centroid_x =
    _getVar(BIAS_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalBiasVars.proj_area_centroid_y =
    _getVar(BIAS_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalBiasVars.vol_centroid_z =
    _getVar(BIAS_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalBiasVars.refl_centroid_z =
    _getVar(BIAS_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalBiasVars.top =
    _getVar(BIAS_TOP, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalBiasVars.dbz_max =
    _getVar(BIAS_DBZ_MAX, NcxxType::nc_FLOAT, _tracksGroup, DBZ);
  _globalBiasVars.volume =
    _getVar(BIAS_VOLUME, NcxxType::nc_FLOAT, _tracksGroup, KM3);
  _globalBiasVars.precip_flux =
    _getVar(BIAS_PRECIP_FLUX, NcxxType::nc_FLOAT, _tracksGroup, M3_PER_SEC);
  _globalBiasVars.mass =
    _getVar(BIAS_MASS, NcxxType::nc_FLOAT, _tracksGroup, KTONS);
  _globalBiasVars.proj_area =
    _getVar(BIAS_PROJ_AREA, NcxxType::nc_FLOAT, _tracksGroup, KM2);
  _globalBiasVars.smoothed_proj_area_centroid_x =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalBiasVars.smoothed_proj_area_centroid_y =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalBiasVars.smoothed_speed =
    _getVar(BIAS_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _tracksGroup, _speedUnits);
  _globalBiasVars.smoothed_direction =
    _getVar(BIAS_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _tracksGroup, DEG);

  // global rmse for forecasts

  _globalRmseVars.proj_area_centroid_x =
    _getVar(RMSE_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalRmseVars.proj_area_centroid_y =
    _getVar(RMSE_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalRmseVars.vol_centroid_z =
    _getVar(RMSE_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalRmseVars.refl_centroid_z =
    _getVar(RMSE_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalRmseVars.top =
    _getVar(RMSE_TOP, NcxxType::nc_FLOAT, _tracksGroup, KM);
  _globalRmseVars.dbz_max =
    _getVar(RMSE_DBZ_MAX, NcxxType::nc_FLOAT, _tracksGroup, DBZ);
  _globalRmseVars.volume =
    _getVar(RMSE_VOLUME, NcxxType::nc_FLOAT, _tracksGroup, KM3);
  _globalRmseVars.precip_flux =
    _getVar(RMSE_PRECIP_FLUX, NcxxType::nc_FLOAT, _tracksGroup, M3_PER_SEC);
  _globalRmseVars.mass =
    _getVar(RMSE_MASS, NcxxType::nc_FLOAT, _tracksGroup, KTONS);
  _globalRmseVars.proj_area =
    _getVar(RMSE_PROJ_AREA, NcxxType::nc_FLOAT, _tracksGroup, KM2);
  _globalRmseVars.smoothed_proj_area_centroid_x =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalRmseVars.smoothed_proj_area_centroid_y =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _tracksGroup, _horizGridUnits);
  _globalRmseVars.smoothed_speed =
    _getVar(RMSE_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _tracksGroup, _speedUnits);
  _globalRmseVars.smoothed_direction =
    _getVar(RMSE_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _tracksGroup, DEG);

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

  _simpleVars.simple_track_num =
    _getVar(SIMPLE_TRACK_NUM, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.complex_track_num =
    _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.last_descendant_simple_track_num =
    _getVar(LAST_DESCENDANT_SIMPLE_TRACK_NUM, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.start_scan =
    _getVar(START_SCAN, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.end_scan =
    _getVar(END_SCAN, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.scan_origin =
    _getVar(SCAN_ORIGIN, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.start_time =
    _getVar(START_TIME, NcxxType::nc_INT64, _simpleDim, _simpleGroup, TIME0);
  _simpleVars.end_time =
    _getVar(END_TIME, NcxxType::nc_INT64, _simpleDim, _simpleGroup, TIME0);
  _simpleVars.last_descendant_end_scan =
    _getVar(LAST_DESCENDANT_END_SCAN, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.last_descendant_end_time =
    _getVar(LAST_DESCENDANT_END_TIME, NcxxType::nc_INT, _simpleDim, _simpleGroup, TIME0);
  _simpleVars.time_origin =
    _getVar(TIME_ORIGIN, NcxxType::nc_INT64, _simpleDim, _simpleGroup, TIME0);
  _simpleVars.history_in_scans =
    _getVar(HISTORY_IN_SCANS, NcxxType::nc_INT, _simpleDim, _simpleGroup, SCANS);
  _simpleVars.history_in_secs =
    _getVar(HISTORY_IN_SECS, NcxxType::nc_INT, _simpleDim, _simpleGroup, SECONDS);
  _simpleVars.duration_in_scans =
    _getVar(DURATION_IN_SCANS, NcxxType::nc_INT, _simpleDim, _simpleGroup, SCANS);
  _simpleVars.duration_in_secs =
    _getVar(DURATION_IN_SECS, NcxxType::nc_INT, _simpleDim, _simpleGroup, SECONDS);
  _simpleVars.nparents =
    _getVar(NPARENTS, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.nchildren =
    _getVar(NCHILDREN, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.parent =
    _getVar(PARENT, NcxxType::nc_INT, _simpleDim, _maxParentsDim, _simpleGroup);
  _simpleVars.child =
    _getVar(CHILD, NcxxType::nc_INT, _simpleDim, _maxChildrenDim, _simpleGroup);
  _simpleVars.first_entry_offset =
    _getVar(FIRST_ENTRY_OFFSET, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.n_simples_per_complex =
    _getVar(N_SIMPLES_PER_COMPLEX, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.simples_per_complex_1D =
    _getVar(SIMPLES_PER_COMPLEX, NcxxType::nc_INT, _simpleDim, _simpleGroup);
  _simpleVars.simples_per_complex_offsets =
    _getVar(SIMPLES_PER_COMPLEX_OFFSETS, NcxxType::nc_INT, _simpleDim, _simpleGroup);

  // array of complex track nums - these are in increasing order, no gaps
  
  _complexTrackNumsVar = _getVar(COMPLEX_TRACK_NUMS, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  if (_fileMode != NcxxFile::read) {
    _complexTrackNumsVar.putAtt(NOTE, "Array of complex track numbers. Monotonically increasing numbers. There will be gaps in the sequence because some complex tracks have multiple simple tracks. The complex track number is derived from the track number of the first simple track.");
  }

  // params for each complex track

  _complexVars.complex_track_num =
    _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.volume_at_start_of_sampling =
    _getVar(VOLUME_AT_START_OF_SAMPLING, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM3);
  _complexVars.volume_at_end_of_sampling =
    _getVar(VOLUME_AT_END_OF_SAMPLING, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM3);
  _complexVars.start_scan =
    _getVar(START_SCAN, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.end_scan =
    _getVar(END_SCAN, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.duration_in_scans =
    _getVar(DURATION_IN_SCANS, NcxxType::nc_INT, _maxComplexDim, _complexGroup, SCANS);
  _complexVars.duration_in_secs =
    _getVar(DURATION_IN_SECS, NcxxType::nc_INT, _maxComplexDim, _complexGroup, SECONDS);
  _complexVars.start_time =
    _getVar(START_TIME, NcxxType::nc_INT64, _maxComplexDim, _complexGroup, TIME0);
  _complexVars.end_time =
    _getVar(END_TIME, NcxxType::nc_INT64, _maxComplexDim, _complexGroup, TIME0);
  _complexVars.n_simple_tracks =
    _getVar(N_SIMPLE_TRACKS, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.n_top_missing =
    _getVar(N_TOP_MISSING, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.n_range_limited =
    _getVar(N_RANGE_LIMITED, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.start_missing =
    _getVar(START_MISSING, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.end_missing =
    _getVar(END_MISSING, NcxxType::nc_INT, _maxComplexDim, _complexGroup);
  _complexVars.n_samples_for_forecast_stats =
    _getVar(N_SAMPLES_FOR_FORECAST_STATS, NcxxType::nc_INT, _maxComplexDim, _complexGroup);

  // bias for forecasts per complex track

  _complexBiasVars.proj_area_centroid_x =
    _getVar(BIAS_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexBiasVars.proj_area_centroid_y =
    _getVar(BIAS_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexBiasVars.vol_centroid_z =
    _getVar(BIAS_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexBiasVars.refl_centroid_z =
    _getVar(BIAS_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexBiasVars.top =
    _getVar(BIAS_TOP, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexBiasVars.dbz_max =
    _getVar(BIAS_DBZ_MAX, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, DBZ);
  _complexBiasVars.volume =
    _getVar(BIAS_VOLUME, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM3);
  _complexBiasVars.precip_flux =
    _getVar(BIAS_PRECIP_FLUX, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, M3_PER_SEC);
  _complexBiasVars.mass =
    _getVar(BIAS_MASS, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KTONS);
  _complexBiasVars.proj_area =
    _getVar(BIAS_PROJ_AREA, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM2);
  _complexBiasVars.smoothed_proj_area_centroid_x =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexBiasVars.smoothed_proj_area_centroid_y =
    _getVar(BIAS_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexBiasVars.smoothed_speed =
    _getVar(BIAS_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, _speedUnits);
  _complexBiasVars.smoothed_direction =
    _getVar(BIAS_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, DEG);

  // rmse for forecasts per complex track

  _complexRmseVars.proj_area_centroid_x =
    _getVar(RMSE_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexRmseVars.proj_area_centroid_y =
    _getVar(RMSE_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexRmseVars.vol_centroid_z =
    _getVar(RMSE_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexRmseVars.refl_centroid_z =
    _getVar(RMSE_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexRmseVars.top =
    _getVar(RMSE_TOP, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM);
  _complexRmseVars.dbz_max =
    _getVar(RMSE_DBZ_MAX, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, DBZ);
  _complexRmseVars.volume =
    _getVar(RMSE_VOLUME, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM3);
  _complexRmseVars.precip_flux =
    _getVar(RMSE_PRECIP_FLUX, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, M3_PER_SEC);
  _complexRmseVars.mass =
    _getVar(RMSE_MASS, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KTONS);
  _complexRmseVars.proj_area =
    _getVar(RMSE_PROJ_AREA, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, KM2);
  _complexRmseVars.smoothed_proj_area_centroid_x =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexRmseVars.smoothed_proj_area_centroid_y =
    _getVar(RMSE_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _maxComplexDim,
            _complexGroup, _horizGridUnits);
  _complexRmseVars.smoothed_speed =
    _getVar(RMSE_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, _speedUnits);
  _complexRmseVars.smoothed_direction =
    _getVar(RMSE_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup, DEG);

  // verification contingency tables per complex track

  _complexVerifyVars.ellipse_forecast_n_success =
    _getVar(ELLIPSE_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  _complexVerifyVars.ellipse_forecast_n_failure =
    _getVar(ELLIPSE_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  _complexVerifyVars.ellipse_forecast_n_false_alarm =
    _getVar(ELLIPSE_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_success =
    _getVar(POLYGON_FORECAST_N_SUCCESS, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_failure =
    _getVar(POLYGON_FORECAST_N_FAILURE, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  _complexVerifyVars.polygon_forecast_n_false_alarm =
    _getVar(POLYGON_FORECAST_N_FALSE_ALARM, NcxxType::nc_FLOAT, _maxComplexDim, _complexGroup);
  
  // track entries

  _entryVars.time =
    _getVar(TIME, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.time_origin =
    _getVar(TIME_ORIGIN, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.scan_origin =
    _getVar(SCAN_ORIGIN, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.scan_num =
    _getVar(SCAN_NUM, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.storm_num =
    _getVar(STORM_NUM, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.simple_track_num =
    _getVar(SIMPLE_TRACK_NUM, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.complex_track_num =
    _getVar(COMPLEX_TRACK_NUM, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.history_in_scans =
    _getVar(HISTORY_IN_SCANS, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.history_in_secs =
    _getVar(HISTORY_IN_SECS, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.duration_in_scans =
    _getVar(DURATION_IN_SCANS, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.duration_in_secs =
    _getVar(DURATION_IN_SECS, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.forecast_valid =
    _getVar(FORECAST_VALID, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.prev_entry_offset =
    _getVar(PREV_ENTRY_OFFSET, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.this_entry_offset =
    _getVar(THIS_ENTRY_OFFSET, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.next_entry_offset =
    _getVar(NEXT_ENTRY_OFFSET, NcxxType::nc_INT, _entriesDim, _entriesGroup);
  _entryVars.next_scan_entry_offset =
    _getVar(NEXT_SCAN_ENTRY_OFFSET, NcxxType::nc_INT, _entriesDim, _entriesGroup);

  // track entry dval_dt for forecasts - these are rates
  
  _entryDvalDtVars.proj_area_centroid_x =
    _getVar(DVAL_DT_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, _horizGridUnitsPerHr);
  _entryDvalDtVars.proj_area_centroid_y =
    _getVar(DVAL_DT_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, _horizGridUnitsPerHr);
  _entryDvalDtVars.vol_centroid_z =
    _getVar(DVAL_DT_VOL_CENTROID_Z, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KM_PER_HR);
  _entryDvalDtVars.refl_centroid_z =
    _getVar(DVAL_DT_REFL_CENTROID_Z, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KM_PER_HR);
  _entryDvalDtVars.top =
    _getVar(DVAL_DT_TOP, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KM_PER_HR);
  _entryDvalDtVars.dbz_max =
    _getVar(DVAL_DT_DBZ_MAX, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, DBZ_PER_HR);
  _entryDvalDtVars.volume =
    _getVar(DVAL_DT_VOLUME, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KM3_PER_HR);
  _entryDvalDtVars.precip_flux =
    _getVar(DVAL_DT_PRECIP_FLUX, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, M3_PER_SEC_PER_HR);
  _entryDvalDtVars.mass =
    _getVar(DVAL_DT_MASS, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KTONS_PER_HR);
  _entryDvalDtVars.proj_area =
    _getVar(DVAL_DT_PROJ_AREA, NcxxType::nc_FLOAT, _entriesDim, _entriesGroup, KM2_PER_HR);
  _entryDvalDtVars.smoothed_proj_area_centroid_x =
    _getVar(DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_X, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, _horizGridUnitsPerHr);
  _entryDvalDtVars.smoothed_proj_area_centroid_y =
    _getVar(DVAL_DT_SMOOTHED_PROJ_AREA_CENTROID_Y, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, _horizGridUnitsPerHr);
  _entryDvalDtVars.smoothed_speed =
    _getVar(DVAL_DT_SMOOTHED_SPEED, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, _speedUnitsPerHr);
  _entryDvalDtVars.smoothed_direction =
    _getVar(DVAL_DT_SMOOTHED_DIRECTION, NcxxType::nc_FLOAT, _entriesDim,
            _entriesGroup, DEG_PER_HR);

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

void TitanFile::allocHist(int n_hist)
     
{

  if (n_hist > _max_hist) {
    _max_hist = n_hist;
    _hist = (storm_file_dbz_hist_t *)
      urealloc(_hist, n_hist * sizeof(storm_file_dbz_hist_t));
    memset(_hist, 0, n_hist * sizeof(storm_file_dbz_hist_t));
  }

}

void TitanFile::freeHist()
     
{

  if (_hist) {
    ufree (_hist);
    _hist = nullptr;
    _max_hist = 0;
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

// void TitanFile::allocScanOffsets(int n_scans_needed)
     
// {

//   // allocate the required space plus a buffer so that 
//   // we do not do too many reallocs
  
//   if (n_scans_needed > _max_scans) {
//     _max_scans = n_scans_needed + 100;
//     _scan_offsets = (si32 *) urealloc
//       (_scan_offsets, (_max_scans * sizeof(si32)));
//   }

// }

// void TitanFile::freeScanOffsets()
     
// {

//   if (_scan_offsets) {
//     ufree(_scan_offsets);
//     _scan_offsets = nullptr;
//     _max_scans = 0;
//   }

// }

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
  // freeScanOffsets();

}


//////////////////////////////////////////////////////////////
//
// Flush the file to force writes to disk
//
//////////////////////////////////////////////////////////////

void TitanFile::flushFile()
  
{

  if (_isLegacyV5Format) {
    _ncFile.sync();
  } else {
    _sFile.FlushFiles();
    _tFile.FlushFiles();
  }

}

//////////////////////////////////////////////////////////////
//
// Put an advisory lock on the file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::lockFile(const char *mode)
  
{
  
  _clearErrStr();

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_sFile.LockHeaderFile(mode)) {
      _addErrStr("ERROR - TitanFile::lockFile");
      _addErrStr("Cannot lock storm header file");
      _addErrStr(_sFile.getErrStr());
      return -1;
    }
    if (_tFile.LockHeaderFile(mode)) {
      _addErrStr("ERROR - TitanFile::lockFile");
      _addErrStr("Cannot lock track header file");
      _addErrStr(_tFile.getErrStr());
      return -1;
    }
    return 0;
  }

  if (ta_lock_file_procmap_fd(_lockPath.c_str(), _lockId, mode)) {
    int errNum = errno;
    _addErrStr("ERROR - TitanFile::lockFile");
    _addErrStr("Cannot lock file: ", _lockPath);
    _addErrStr(strerror(errNum));
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////////////////
//
// Remove advisory lock from the lock file
//
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::unlockFile()
  
{
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_sFile.UnlockHeaderFile()) {
      _addErrStr("ERROR - TitanFile::lockFile");
      _addErrStr("Cannot unlock storm header file");
      _addErrStr(_sFile.getErrStr());
      return -1;
    }
    if (_tFile.UnlockHeaderFile()) {
      _addErrStr("ERROR - TitanFile::lockFile");
      _addErrStr("Cannot unlock track header file");
      _addErrStr(_tFile.getErrStr());
      return -1;
    }
    return 0;
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::unlockStormHeaderFile");
  _addErrStr("  File: ", _lockPath);
  
  if (ta_unlock_file_fd(_lockPath.c_str(), _lockId)) {
    int errNum = errno;
    _addErrStr("Cannot unlock file: ", _lockPath);
    _addErrStr(strerror(errNum));
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

  if (_isLegacyV5Format) {
    if (_sFile.ReadHeader(clear_error_str)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    _storm_header = _sFile.header();
    _nScans = _storm_header.n_scans;
    return 0;
  }

  if (clear_error_str) {
    _clearErrStr();
  }
  _addErrStr("ERROR - TitanFile::readStormHeader");
  _addErrStr("  Reading from file: ", _filePath);

  // top level vars
  
  _topLevelVars.file_time.getVal(&_storm_header.file_time);
  _topLevelVars.start_time.getVal(&_storm_header.start_time);
  _topLevelVars.end_time.getVal(&_storm_header.end_time);

  _topLevelVars.n_scans.getVal(&_nScans);
  _storm_header.n_scans = _nScans;
  _track_header.n_scans = _nScans;

  _topLevelVars.sum_storms.getVal(&_sumStorms);
  _topLevelVars.sum_layers.getVal(&_sumLayers);
  _topLevelVars.sum_hist.getVal(&_sumHist);
  _topLevelVars.sum_runs.getVal(&_sumRuns);
  _topLevelVars.sum_proj_runs.getVal(&_sumProjRuns);

  // storm params
  
  storm_file_params_t &sparams = _storm_header.params;
  _sparamsVars.low_dbz_threshold.getVal(&sparams.low_dbz_threshold);
  _sparamsVars.high_dbz_threshold.getVal(&sparams.high_dbz_threshold);
  _sparamsVars.dbz_hist_interval.getVal(&sparams.dbz_hist_interval);
  _sparamsVars.hail_dbz_threshold.getVal(&sparams.hail_dbz_threshold);
  _sparamsVars.base_threshold.getVal(&sparams.base_threshold);
  _sparamsVars.top_threshold.getVal(&sparams.top_threshold);
  _sparamsVars.min_storm_size.getVal(&sparams.min_storm_size);
  _sparamsVars.max_storm_size.getVal(&sparams.max_storm_size);
  _sparamsVars.morphology_erosion_threshold.getVal(&sparams.morphology_erosion_threshold);
  _sparamsVars.morphology_refl_divisor.getVal(&sparams.morphology_refl_divisor);
  _sparamsVars.min_radar_tops.getVal(&sparams.min_radar_tops);
  _sparamsVars.tops_edge_margin.getVal(&sparams.tops_edge_margin);
  _sparamsVars.z_p_coeff.getVal(&sparams.z_p_coeff);
  _sparamsVars.z_p_exponent.getVal(&sparams.z_p_exponent);
  _sparamsVars.z_m_coeff.getVal(&sparams.z_m_coeff);
  _sparamsVars.z_m_exponent.getVal(&sparams.z_m_exponent);
  _sparamsVars.sectrip_vert_aspect.getVal(&sparams.sectrip_vert_aspect);
  _sparamsVars.sectrip_horiz_aspect.getVal(&sparams.sectrip_horiz_aspect);
  _sparamsVars.sectrip_orientation_error.getVal(&sparams.sectrip_orientation_error);
  _sparamsVars.poly_start_az.getVal(&sparams.poly_start_az);
  _sparamsVars.poly_delta_az.getVal(&sparams.poly_delta_az);
  _sparamsVars.check_morphology.getVal(&sparams.check_morphology);
  _sparamsVars.check_tops.getVal(&sparams.check_tops);
  _sparamsVars.vel_available.getVal(&sparams.vel_available);
  _sparamsVars.n_poly_sides.getVal(&sparams.n_poly_sides);
  _sparamsVars.ltg_count_time.getVal(&sparams.ltg_count_time);
  _sparamsVars.ltg_count_margin_km.getVal(&sparams.ltg_count_margin_km);
  _sparamsVars.hail_z_m_coeff.getVal(&sparams.hail_z_m_coeff);
  _sparamsVars.hail_z_m_exponent.getVal(&sparams.hail_z_m_exponent);
  _sparamsVars.hail_mass_dbz_threshold.getVal(&sparams.hail_mass_dbz_threshold);
  _sparamsVars.gprops_union_type.getVal(&sparams.gprops_union_type);
  _sparamsVars.tops_dbz_threshold.getVal(&sparams.tops_dbz_threshold);
  _sparamsVars.precip_computation_mode.getVal(&sparams.precip_computation_mode);
  _sparamsVars.precip_plane_ht.getVal(&sparams.precip_plane_ht);
  _sparamsVars.low_convectivity_threshold.getVal(&sparams.low_convectivity_threshold);
  _sparamsVars.high_convectivity_threshold.getVal(&sparams.high_convectivity_threshold);

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
  _addErrStr("ERROR - TitanFile::readProjRuns");

  if (_isLegacyV5Format) {
    if (_sFile.ReadProjRuns(storm_num)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    int nProjRuns = _gprops[storm_num].n_proj_runs;
    allocProjRuns(nProjRuns);
    memcpy(_proj_runs, _sFile.proj_runs(), nProjRuns * sizeof(storm_file_run_t));
    return 0;
  }

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }
  
  // store storm number
  // _storm_num = storm_num;
  
  // allocate mem
  
  int nProjRuns = _gprops[storm_num].n_proj_runs;
  allocProjRuns(nProjRuns);
  
  // read in proj runs
  
  int projRunsOffset = _gprops[storm_num].proj_runs_offset;
  for (int irun = 0; irun < nProjRuns; irun++) {
    storm_file_run_t &run = _proj_runs[irun];
    std::vector<size_t> projRunIndex = NcxxVar::makeIndex(projRunsOffset);
    _projRunsVars.run_ix.getVal(projRunIndex, &run.ix);
    _projRunsVars.run_iy.getVal(projRunIndex, &run.iy);
    _projRunsVars.run_iz.getVal(projRunIndex, &run.iz);
    _projRunsVars.run_len.getVal(projRunIndex, &run.n);
  }

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// Read the auxiliary storm properties:
//   layers, dbz histograms, runs and proj_runs
//
// Space for the arrays of structures is allocated as required.
// returns 0 on success, -1 on failure
//
//////////////////////////////////////////////////////////////

int TitanFile::readStormAux(int storm_num)
  
{
  
  if (_isLegacyV5Format) {

    // read
    
    if (_sFile.ReadProps(storm_num)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }

    // copy over the data
    
    int nLayers = _gprops[storm_num].n_layers;
    int nDbzIntervals = _gprops[storm_num].n_dbz_intervals;
    int nRuns = _gprops[storm_num].n_runs;
    int nProjRuns = _gprops[storm_num].n_proj_runs;
    
    allocLayers(nLayers);
    allocHist(nDbzIntervals);
    allocRuns(nRuns);
    allocProjRuns(nProjRuns);
    
    memcpy(_lprops, _sFile.lprops(), nLayers * sizeof(storm_file_layer_props_t));
    memcpy(_hist, _sFile.hist(), nDbzIntervals * sizeof(storm_file_dbz_hist_t));
    memcpy(_runs, _sFile.runs(), nRuns * sizeof(storm_file_run_t));
    memcpy(_proj_runs, _sFile.proj_runs(), nProjRuns * sizeof(storm_file_run_t));

    return 0;
    
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::readStormAux");
  _addErrStr("  Reading storm props from file: ", _filePath);
  _addErrInt("  Storm number: ", storm_num);
  _addErrInt("  Scan number: ", _scan.scan_num);
  
  // store storm number
  // _storm_num = storm_num;
  
  // allocate or realloc mem
  
  int nLayers = _gprops[storm_num].n_layers;
  int nDbzIntervals = _gprops[storm_num].n_dbz_intervals;
  int nRuns = _gprops[storm_num].n_runs;
  int nProjRuns = _gprops[storm_num].n_proj_runs;

  allocLayers(nLayers);
  allocHist(nDbzIntervals);
  allocRuns(nRuns);
  allocProjRuns(nProjRuns);

  // return early if nstorms is zero
  
  if (_scan.nstorms == 0) {
    return 0;
  }

  // get offsets
  
  int layerPropsOffset = _gprops[storm_num].layer_props_offset;
  int dbzHistOffset = _gprops[storm_num].dbz_hist_offset;
  int runsOffset = _gprops[storm_num].runs_offset;
  int projRunsOffset = _gprops[storm_num].proj_runs_offset;

  // read in layer props
  
  for (int ilayer = 0; ilayer < nLayers; ilayer++) {
    storm_file_layer_props_t &ll = _lprops[ilayer];
    std::vector<size_t> layerIndex = NcxxVar::makeIndex(layerPropsOffset);
    _lpropsVars.vol_centroid_x.getVal(layerIndex, &ll.vol_centroid_x);
    _lpropsVars.vol_centroid_y.getVal(layerIndex, &ll.vol_centroid_y);
    _lpropsVars.refl_centroid_x.getVal(layerIndex, &ll.refl_centroid_x);
    _lpropsVars.refl_centroid_y.getVal(layerIndex, &ll.refl_centroid_y);
    _lpropsVars.area.getVal(layerIndex, &ll.area);
    _lpropsVars.dbz_max.getVal(layerIndex, &ll.dbz_max);
    _lpropsVars.dbz_mean.getVal(layerIndex, &ll.dbz_mean);
    _lpropsVars.mass.getVal(layerIndex, &ll.mass);
    _lpropsVars.rad_vel_mean.getVal(layerIndex, &ll.rad_vel_mean);
    _lpropsVars.rad_vel_sd.getVal(layerIndex, &ll.rad_vel_sd);
    _lpropsVars.vorticity.getVal(layerIndex, &ll.vorticity);
    _lpropsVars.convectivity_median.getVal(layerIndex, &ll.convectivity_median);
  }

  // read in histogram data
  
  for (int ihist = 0; ihist < nDbzIntervals; ihist++) {
    storm_file_dbz_hist_t &hh = _hist[ihist];
    std::vector<size_t> histIndex = NcxxVar::makeIndex(dbzHistOffset);
    _histVars.percent_volume.getVal(histIndex, &hh.percent_volume);
    _histVars.percent_area.getVal(histIndex, &hh.percent_area);
  }
  
  // read in runs
  
  for (int irun = 0; irun < nRuns; irun++) {
    storm_file_run_t &run = _runs[irun];
    std::vector<size_t> runIndex = NcxxVar::makeIndex(runsOffset);
    _runsVars.run_ix.getVal(runIndex, &run.ix);
    _runsVars.run_iy.getVal(runIndex, &run.iy);
    _runsVars.run_iz.getVal(runIndex, &run.iz);
    _runsVars.run_len.getVal(runIndex, &run.n);
  }

  // read in proj_runs
  
  for (int irun = 0; irun < nProjRuns; irun++) {
    storm_file_run_t &run = _proj_runs[irun];
    std::vector<size_t> projRunIndex = NcxxVar::makeIndex(projRunsOffset);
    _projRunsVars.run_ix.getVal(projRunIndex, &run.ix);
    _projRunsVars.run_iy.getVal(projRunIndex, &run.iy);
    _projRunsVars.run_iz.getVal(projRunIndex, &run.iz);
    _projRunsVars.run_len.getVal(projRunIndex, &run.n);
  }

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

int TitanFile::readStormScan(int scan_num, int storm_num /* = -1*/ )
     
{
  
  if (_isLegacyV5Format) {
    if (_sFile.ReadScan(scan_num, storm_num)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    _scan = _sFile.scan();
    int nStorms = _scan.nstorms;
    allocGprops(nStorms);
    memcpy(_gprops, _sFile.gprops(), nStorms * sizeof(storm_file_global_props_t));
    return 0;
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::readScan");
  _addErrStr("  Reading scan from file: ", _filePath);
  _addErrInt("  Scan number: ", scan_num);

  // check

  // cerr << "1111111111111 scan_num, _max_scans: " << scan_num << ", " << _max_scans << endl;
  
  // if (scan_num >= _max_scans) {
  //   return -1;
  // }
  
  // scan header storage index in file
  
  std::vector<size_t> scanPos = NcxxVar::makeIndex(scan_num);

  // read scan details
  
  _scanVars.scan_min_z.getVal(scanPos, &_scan.min_z);
  _scanVars.scan_delta_z.getVal(scanPos, &_scan.delta_z);
  _scanVars.scan_num.getVal(scanPos, &_scan.scan_num);
  _scanVars.scan_nstorms.getVal(scanPos, &_scan.nstorms);
  _scanVars.scan_time.getVal(scanPos, &_scan.time);
  _scanVars.scan_ht_of_freezing.getVal(scanPos, &_scan.ht_of_freezing);
  _scanVars.scan_gprops_offset.getVal(scanPos, &_scan.gprops_offset);

  // read grid details

  _scanVars.grid_nx.getVal(scanPos, &_scan.grid.nx);
  _scanVars.grid_ny.getVal(scanPos, &_scan.grid.ny);
  _scanVars.grid_nz.getVal(scanPos, &_scan.grid.nz);
  _scanVars.grid_minx.getVal(scanPos, &_scan.grid.minx);
  _scanVars.grid_miny.getVal(scanPos, &_scan.grid.miny);
  _scanVars.grid_minz.getVal(scanPos, &_scan.grid.minz);
  _scanVars.grid_dx.getVal(scanPos, &_scan.grid.dx);
  _scanVars.grid_dy.getVal(scanPos, &_scan.grid.dy);
  _scanVars.grid_dz.getVal(scanPos, &_scan.grid.dz);
  _scanVars.grid_dz_constant.getVal(scanPos, &_scan.grid.dz_constant);
  _scanVars.grid_sensor_x.getVal(scanPos, &_scan.grid.sensor_x);
  _scanVars.grid_sensor_y.getVal(scanPos, &_scan.grid.sensor_y);
  _scanVars.grid_sensor_z.getVal(scanPos, &_scan.grid.sensor_z);
  _scanVars.grid_sensor_lat.getVal(scanPos, &_scan.grid.sensor_lat);
  _scanVars.grid_sensor_lon.getVal(scanPos, &_scan.grid.sensor_lon);

  char units[10000];
  _scanVars.grid_unitsx.getVal(scanPos, &units);
  STRncopy(_scan.grid.unitsx, units, TITAN_GRID_UNITS_LEN);
  _scanVars.grid_unitsy.getVal(scanPos, &units);
  STRncopy(_scan.grid.unitsy, units, TITAN_GRID_UNITS_LEN);
  _scanVars.grid_unitsz.getVal(scanPos, &units);
  STRncopy(_scan.grid.unitsz, units, TITAN_GRID_UNITS_LEN);
  
  // read projection details
  
  _scanVars.proj_type.getVal(scanPos, &_scan.grid.proj_type);
  _scanVars.proj_origin_lat.getVal(scanPos, &_scan.grid.proj_origin_lat);
  _scanVars.proj_origin_lon.getVal(scanPos, &_scan.grid.proj_origin_lon);
  _scanVars.proj_rotation.getVal(scanPos, &_scan.grid.proj_params.flat.rotation);

  _scanVars.proj_lat1.getVal(scanPos, &_scan.grid.proj_params.lc2.lat1);
  _scanVars.proj_lat2.getVal(scanPos, &_scan.grid.proj_params.lc2.lat2);
  // _scanVars.proj_tangent_lat.getVal(scanPos, 0.0);
  // _scanVars.proj_tangent_lon.getVal(scanPos, 0.0);
  // _scanVars.proj_pole_type.getVal(scanPos, 0);
  // _scanVars.proj_central_scale.getVal(scanPos, 1.0);

  // allocate or reallocate
  
  int nStorms = _scan.nstorms;
  allocGprops(nStorms);

  // return early if nstorms is zero
  
  if (nStorms == 0) {
    return 0;
  }
  
  // read in global props
  
  // _layerOffsets.resize(nStorms);
  // _histOffsets.resize(nStorms);
  // _runsOffsets.resize(nStorms);
  // _projRunsOffsets.resize(nStorms);

  for (int istorm = 0; istorm < nStorms; istorm++) {
    
    storm_file_global_props_t &gp = _gprops[istorm];
    
    int gpropsOffset = _scan.gprops_offset + istorm;
    std::vector<size_t> stormIndex = NcxxVar::makeIndex(gpropsOffset);
    
    _gpropsVars.vol_centroid_x.getVal(stormIndex, &gp.vol_centroid_x);
    _gpropsVars.vol_centroid_y.getVal(stormIndex, &gp.vol_centroid_y);
    _gpropsVars.vol_centroid_z.getVal(stormIndex, &gp.vol_centroid_z);
    _gpropsVars.refl_centroid_x.getVal(stormIndex, &gp.refl_centroid_x);
    _gpropsVars.refl_centroid_y.getVal(stormIndex, &gp.refl_centroid_y);
    _gpropsVars.refl_centroid_z.getVal(stormIndex, &gp.refl_centroid_z);
    _gpropsVars.top.getVal(stormIndex, &gp.top);
    _gpropsVars.base.getVal(stormIndex, &gp.base);
    _gpropsVars.volume.getVal(stormIndex, &gp.volume);
    _gpropsVars.area_mean.getVal(stormIndex, &gp.area_mean);
    _gpropsVars.precip_flux.getVal(stormIndex, &gp.precip_flux);
    _gpropsVars.mass.getVal(stormIndex, &gp.mass);
    _gpropsVars.tilt_angle.getVal(stormIndex, &gp.tilt_angle);
    _gpropsVars.tilt_dirn.getVal(stormIndex, &gp.tilt_dirn);
    _gpropsVars.dbz_max.getVal(stormIndex, &gp.dbz_max);
    _gpropsVars.dbz_mean.getVal(stormIndex, &gp.dbz_mean);
    _gpropsVars.dbz_max_gradient.getVal(stormIndex, &gp.dbz_max_gradient);
    _gpropsVars.dbz_mean_gradient.getVal(stormIndex, &gp.dbz_mean_gradient);
    _gpropsVars.ht_of_dbz_max.getVal(stormIndex, &gp.ht_of_dbz_max);
    _gpropsVars.rad_vel_mean.getVal(stormIndex, &gp.rad_vel_mean);
    _gpropsVars.rad_vel_sd.getVal(stormIndex, &gp.rad_vel_sd);
    _gpropsVars.vorticity.getVal(stormIndex, &gp.vorticity);
    _gpropsVars.precip_area.getVal(stormIndex, &gp.precip_area);
    _gpropsVars.precip_area_centroid_x.getVal(stormIndex, &gp.precip_area_centroid_x);
    _gpropsVars.precip_area_centroid_y.getVal(stormIndex, &gp.precip_area_centroid_y);
    _gpropsVars.precip_area_orientation.getVal(stormIndex, &gp.precip_area_orientation);
    _gpropsVars.precip_area_minor_radius.getVal(stormIndex, &gp.precip_area_minor_radius);
    _gpropsVars.precip_area_major_radius.getVal(stormIndex, &gp.precip_area_major_radius);
    _gpropsVars.proj_area.getVal(stormIndex, &gp.proj_area);
    _gpropsVars.proj_area_centroid_x.getVal(stormIndex, &gp.proj_area_centroid_x);
    _gpropsVars.proj_area_centroid_y.getVal(stormIndex, &gp.proj_area_centroid_y);
    _gpropsVars.proj_area_orientation.getVal(stormIndex, &gp.proj_area_orientation);
    _gpropsVars.proj_area_minor_radius.getVal(stormIndex, &gp.proj_area_minor_radius);
    _gpropsVars.proj_area_major_radius.getVal(stormIndex, &gp.proj_area_major_radius);

    _gpropsVars.storm_num.getVal(stormIndex, &gp.storm_num);
    _gpropsVars.n_layers.getVal(stormIndex, &gp.n_layers);
    _gpropsVars.base_layer.getVal(stormIndex, &gp.base_layer);
    _gpropsVars.n_dbz_intervals.getVal(stormIndex, &gp.n_dbz_intervals);
    _gpropsVars.n_runs.getVal(stormIndex, &gp.n_runs);
    _gpropsVars.n_proj_runs.getVal(stormIndex, &gp.n_proj_runs);
    _gpropsVars.top_missing.getVal(stormIndex, &gp.top_missing);
    _gpropsVars.range_limited.getVal(stormIndex, &gp.range_limited);
    _gpropsVars.second_trip.getVal(stormIndex, &gp.second_trip);
    _gpropsVars.hail_present.getVal(stormIndex, &gp.hail_present);
    _gpropsVars.anom_prop.getVal(stormIndex, &gp.anom_prop);
    _gpropsVars.bounding_min_ix.getVal(stormIndex, &gp.bounding_min_ix);
    _gpropsVars.bounding_min_iy.getVal(stormIndex, &gp.bounding_min_iy);
    _gpropsVars.bounding_max_ix.getVal(stormIndex, &gp.bounding_max_ix);
    _gpropsVars.bounding_max_iy.getVal(stormIndex, &gp.bounding_max_iy);
    _gpropsVars.vil_from_maxz.getVal(stormIndex, &gp.vil_from_maxz);
    _gpropsVars.ltg_count.getVal(stormIndex, &gp.ltg_count);
    _gpropsVars.convectivity_median.getVal(stormIndex, &gp.convectivity_median);

    if (_storm_header.params.gprops_union_type == UNION_HAIL) {
      
      _gpropsVars.hail_FOKRcategory.getVal
        (stormIndex, &gp.add_on.hail_metrics.FOKRcategory);
      _gpropsVars.hail_waldvogelProbability.getVal
        (stormIndex, &gp.add_on.hail_metrics.waldvogelProbability);
      _gpropsVars.hail_hailMassAloft.getVal
        (stormIndex, &gp.add_on.hail_metrics.hailMassAloft);
      _gpropsVars.hail_vihm.getVal
        (stormIndex, &gp.add_on.hail_metrics.vihm);

    } else if (_storm_header.params.gprops_union_type == UNION_NEXRAD_HDA) {
      
      _gpropsVars.hail_poh.getVal(stormIndex, &gp.add_on.hda.poh);
      _gpropsVars.hail_shi.getVal(stormIndex, &gp.add_on.hda.shi);
      _gpropsVars.hail_posh.getVal(stormIndex, &gp.add_on.hda.posh);
      _gpropsVars.hail_mehs.getVal(stormIndex, &gp.add_on.hda.mehs);
      
    }

    // polygons are 2D variables
    
    std::vector<size_t> polyIndex = NcxxVar::makeIndex(gpropsOffset, 0);
    std::vector<size_t> polyCount = NcxxVar::makeIndex(1, N_POLY_SIDES);
    
    _gpropsVars.proj_area_polygon.getVal(polyIndex, polyCount, &gp.proj_area_polygon);

    // offsets into layers, histograms and runs
    
    _gpropsVars.layer_props_offset.getVal(stormIndex, &gp.layer_props_offset);
    _gpropsVars.dbz_hist_offset.getVal(stormIndex, &gp.dbz_hist_offset);
    _gpropsVars.runs_offset.getVal(stormIndex, &gp.runs_offset);
    _gpropsVars.proj_runs_offset.getVal(stormIndex, &gp.proj_runs_offset);
    
    // _layerOffsets[istorm] = gp.layer_props_offset;
    // _histOffsets[istorm] = gp.dbz_hist_offset;
    // _runsOffsets[istorm] = gp.runs_offset;
    // _projRunsOffsets[istorm] = gp.proj_runs_offset;

    // cerr << "aaaaaaaaaa istorm offsets layer, hist, runs, proj_runs: " << istorm << ", " <<  _layerOffsets[istorm] << ", " << _histOffsets[istorm] << ", " << _runsOffsets[istorm] << ", " << _projRunsOffsets[istorm] << endl;

  } // istorm

  return 0;
  
}

//////////////////////////////////////////////////////////////
//
// seek to the end of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanFile::seekStormEndData()
     
{
  if (_isLegacyV5Format) {
    if (_sFile.SeekEndData()) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }
  return 0;
}

//////////////////////////////////////////////////////////////
//
// seek to the start of the storm data in data file
//
//////////////////////////////////////////////////////////////

int TitanFile::seekStormStartData()
     
{
  if (_isLegacyV5Format) {
    if (_sFile.SeekStartData()) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }
  return 0;
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

  // save state
  
  _storm_header = storm_file_header;
  _nScans = _storm_header.n_scans;

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_sFile.WriteHeader(storm_file_header)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeStormHeader");
  _addErrStr("  File: ", _filePath);

  // set file time to gmt
  
  _storm_header.file_time = time(nullptr);
  
  // make local copies of the global file header and scan offsets
  
  _topLevelVars.file_time.putVal((int64_t) _storm_header.file_time);
  _topLevelVars.start_time.putVal((int64_t) _storm_header.start_time);
  _topLevelVars.end_time.putVal((int64_t) _storm_header.end_time);
  _topLevelVars.n_scans.putVal(_nScans);
  _topLevelVars.sum_storms.putVal(_sumStorms);
  _topLevelVars.sum_layers.putVal(_sumLayers);
  _topLevelVars.sum_hist.putVal(_sumHist);
  _topLevelVars.sum_runs.putVal(_sumRuns);
  _topLevelVars.sum_proj_runs.putVal(_sumProjRuns);

  const storm_file_params_t &sparams(_storm_header.params);
  
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

  // save state

  _storm_header = storm_file_header;
  _scan = scanHeader;
  allocGprops(_scan.nstorms);
  memcpy(_gprops, gprops, _scan.nstorms * sizeof(storm_file_global_props_t));
    
  // handle legacy format case
  
  if (_isLegacyV5Format) {
    if (_sFile.WriteScan(scanHeader, gprops)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeStormScan");
  _addErrStr("  File: ", _filePath);

  // update attributes that depend on scan type
  
  _updateScanAttributes(_scan);
  
  // scan storage index
  
  size_t scanNum = _scan.scan_num;
  _nScans = scanNum + 1;
  std::vector<size_t> scanPos = NcxxVar::makeIndex(scanNum);

  // write scan details
  
  _topLevelVars.n_scans.putVal(_nScans);
  
  _scanVars.scan_min_z.putVal(scanPos, _scan.min_z);
  _scanVars.scan_delta_z.putVal(scanPos, _scan.delta_z);
  _scanVars.scan_num.putVal(scanPos, _scan.scan_num);
  _scanVars.scan_nstorms.putVal(scanPos, _scan.nstorms);
  _scanVars.scan_time.putVal(scanPos, _scan.time);
  _scanVars.scan_ht_of_freezing.putVal(scanPos, _scan.ht_of_freezing);

  // write grid details

  _scanVars.grid_nx.putVal(scanPos, _scan.grid.nx);
  _scanVars.grid_ny.putVal(scanPos, _scan.grid.ny);
  _scanVars.grid_nz.putVal(scanPos, _scan.grid.nz);
  _scanVars.grid_minx.putVal(scanPos, _scan.grid.minx);
  _scanVars.grid_miny.putVal(scanPos, _scan.grid.miny);
  _scanVars.grid_minz.putVal(scanPos, _scan.grid.minz);
  _scanVars.grid_dx.putVal(scanPos, _scan.grid.dx);
  _scanVars.grid_dy.putVal(scanPos, _scan.grid.dy);
  _scanVars.grid_dz.putVal(scanPos, _scan.grid.dz);
  _scanVars.grid_dz_constant.putVal(scanPos, _scan.grid.dz_constant);
  _scanVars.grid_sensor_x.putVal(scanPos, _scan.grid.sensor_x);
  _scanVars.grid_sensor_y.putVal(scanPos, _scan.grid.sensor_y);
  _scanVars.grid_sensor_z.putVal(scanPos, _scan.grid.sensor_z);
  _scanVars.grid_sensor_lat.putVal(scanPos, _scan.grid.sensor_lat);
  _scanVars.grid_sensor_lon.putVal(scanPos, _scan.grid.sensor_lon);
  
  _scanVars.grid_unitsx.putVal(scanPos, _horizGridUnits);
  _scanVars.grid_unitsy.putVal(scanPos, _horizGridUnits);
  _scanVars.grid_unitsz.putVal(scanPos, std::string(_scan.grid.unitsz));
  
  // write projection details
  
  _scanVars.proj_type.putVal(scanPos, _scan.grid.proj_type);
  _scanVars.proj_origin_lat.putVal(scanPos, _scan.grid.proj_origin_lat);
  _scanVars.proj_origin_lon.putVal(scanPos, _scan.grid.proj_origin_lon);
  _scanVars.proj_rotation.putVal(scanPos, _scan.grid.proj_params.flat.rotation);

  _scanVars.proj_lat1.putVal(scanPos, _scan.grid.proj_params.lc2.lat1);
  _scanVars.proj_lat2.putVal(scanPos, _scan.grid.proj_params.lc2.lat2);
  _scanVars.proj_tangent_lat.putVal(scanPos, 0.0);
  _scanVars.proj_tangent_lon.putVal(scanPos, 0.0);
  _scanVars.proj_pole_type.putVal(scanPos, 0);
  _scanVars.proj_central_scale.putVal(scanPos, 1.0);

  // write scan offsets
  
  _scanVars.scan_gprops_offset.putVal(scanPos, _sumStorms);
  _scanVars.scan_gprops_offset_0.putVal(scanPos, _sumStorms);

  // write storm global props
  
  const storm_file_params_t &sparams(_storm_header.params);

  for (int istorm = 0; istorm < _scan.nstorms; istorm++) {

    // write first and last gprops offsets for this scan
    // first_offset is the offset of the first storm in the scan
    // last_offset is the offset of the last storm in the scan
    // NOTE: last_offset is the offset OF the last storm, NOT one beyond
    
    if (istorm == 0) {
      _scanVars.scan_first_offset.putVal(scanPos, _sumStorms);
    }
    _scanVars.scan_last_offset.putVal(scanPos, _sumStorms);

    // write the global props
    
    const storm_file_global_props_t &gp = _gprops[istorm];
    
    int gpropsOffset = _sumStorms;
    std::vector<size_t> stormIndex = NcxxVar::makeIndex(gpropsOffset);
    
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
      
      _gpropsVars.hail_FOKRcategory.putVal
        (stormIndex, gp.add_on.hail_metrics.FOKRcategory);
      _gpropsVars.hail_waldvogelProbability.putVal
        (stormIndex, gp.add_on.hail_metrics.waldvogelProbability);
      _gpropsVars.hail_hailMassAloft.putVal
        (stormIndex, gp.add_on.hail_metrics.hailMassAloft);
      _gpropsVars.hail_vihm.putVal
        (stormIndex, gp.add_on.hail_metrics.vihm);
      
    } else if (sparams.gprops_union_type == UNION_NEXRAD_HDA) {
      
      _gpropsVars.hail_poh.putVal(stormIndex, gp.add_on.hda.poh);
      _gpropsVars.hail_shi.putVal(stormIndex, gp.add_on.hda.shi);
      _gpropsVars.hail_posh.putVal(stormIndex, gp.add_on.hda.posh);
      _gpropsVars.hail_mehs.putVal(stormIndex, gp.add_on.hda.mehs);
      
    }

    // polygons are 2D variables
    
    std::vector<size_t> polyIndex = NcxxVar::makeIndex(gpropsOffset, 0);
    std::vector<size_t> polyCount = NcxxVar::makeIndex(1, N_POLY_SIDES);
    
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

    _sumStorms++;
    _topLevelVars.sum_storms.putVal(_sumStorms);

  } // istorm

  return 0;
  
}

//////////////////////////////////////////////////////////////
// update attributes for scan type

void TitanFile::_updateScanAttributes(const storm_file_scan_header_t &scanHeader)

{

  // set horiz units
  
  if (scanHeader.grid.proj_type == TITAN_PROJ_LATLON) {
    _horizGridUnits = DEG;
    _horizGridUnitsPerHr = DEG_PER_HR;
    _speedUnits = DEG_PER_HR;
    _speedUnitsPerHr = DEG_PER_HR_PER_HR;
  } else {
    _horizGridUnits = KM;
    _horizGridUnitsPerHr = KM_PER_HR;
    _speedUnits = KM_PER_HR;
    _speedUnitsPerHr = KM_PER_HR_PER_HR;
  }

  if (_fileMode == NcxxFile::read) {
    // not for read-only
    return;
  }
  
  // update relevant attributes
  
  _scanVars.grid_minx.putAtt(UNITS, _horizGridUnits);
  _scanVars.grid_miny.putAtt(UNITS, _horizGridUnits);
  _scanVars.grid_dx.putAtt(UNITS, _horizGridUnits);
  _scanVars.grid_dy.putAtt(UNITS, _horizGridUnits);
  _scanVars.grid_sensor_x.putAtt(UNITS, _horizGridUnits);
  _scanVars.grid_sensor_y.putAtt(UNITS, _horizGridUnits);
  
  _gpropsVars.vol_centroid_x.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.vol_centroid_y.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.refl_centroid_x.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.refl_centroid_y.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.precip_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.precip_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  
  _gpropsVars.precip_area_minor_radius.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.precip_area_major_radius.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.proj_area_minor_radius.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.proj_area_major_radius.putAtt(UNITS, _horizGridUnits);
  _gpropsVars.proj_area_polygon.putAtt(UNITS, _horizGridUnits);
  
  
  _lpropsVars.vol_centroid_x.putAtt(UNITS, _horizGridUnits);
  _lpropsVars.vol_centroid_y.putAtt(UNITS, _horizGridUnits);
  _lpropsVars.refl_centroid_x.putAtt(UNITS, _horizGridUnits);
  _lpropsVars.refl_centroid_y.putAtt(UNITS, _horizGridUnits);
  
  _globalBiasVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _globalBiasVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _globalBiasVars.smoothed_proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _globalBiasVars.smoothed_proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _globalBiasVars.smoothed_speed.putAtt(UNITS, _speedUnits);
  
  _globalRmseVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _globalRmseVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _globalRmseVars.smoothed_proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _globalRmseVars.smoothed_proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _globalRmseVars.smoothed_speed.putAtt(UNITS, _speedUnits);
  
  _complexBiasVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _complexBiasVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _complexBiasVars.smoothed_proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _complexBiasVars.smoothed_proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _complexBiasVars.smoothed_speed.putAtt(UNITS, _speedUnits);
  
  _complexRmseVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _complexRmseVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _complexRmseVars.smoothed_proj_area_centroid_x.putAtt(UNITS, _horizGridUnits);
  _complexRmseVars.smoothed_proj_area_centroid_y.putAtt(UNITS, _horizGridUnits);
  _complexRmseVars.smoothed_speed.putAtt(UNITS, _speedUnits);
  
  _entryDvalDtVars.proj_area_centroid_x.putAtt(UNITS, _horizGridUnitsPerHr);
  _entryDvalDtVars.proj_area_centroid_y.putAtt(UNITS, _horizGridUnitsPerHr);
  _entryDvalDtVars.smoothed_proj_area_centroid_x.putAtt(UNITS, _horizGridUnitsPerHr);
  _entryDvalDtVars.smoothed_proj_area_centroid_y.putAtt(UNITS, _horizGridUnitsPerHr);
  _entryDvalDtVars.smoothed_speed.putAtt(UNITS, _speedUnitsPerHr);
  
}

//////////////////////////////////////////////////////////////
// add projection attributes

void TitanFile::_addProjectionFlagAttributes()
  
{

  vector<int> flagValues;
  vector<std::string> flagMeanings;
  std::string projTypeNote;
  char tmp[BUFSIZ];
  
  flagValues.push_back(TITAN_PROJ_LATLON);
  flagMeanings.push_back("latlon");
  snprintf(tmp, BUFSIZ, "latlon = %d", TITAN_PROJ_LATLON);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_LAMBERT_CONF);
  flagMeanings.push_back("lambert_conf");
  snprintf(tmp, BUFSIZ, ", lambert_conf = %d", TITAN_PROJ_LAMBERT_CONF);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_MERCATOR);
  flagMeanings.push_back("mercator");
  snprintf(tmp, BUFSIZ, ", mercator = %d", TITAN_PROJ_MERCATOR);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_POLAR_STEREO);
  flagMeanings.push_back("polar_stereo");
  snprintf(tmp, BUFSIZ, ", polar_stereo = %d", TITAN_PROJ_POLAR_STEREO);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_POLAR_ST_ELLIP);
  flagMeanings.push_back("polar_st_ellip");
  snprintf(tmp, BUFSIZ, ", polar_st_ellip = %d", TITAN_PROJ_POLAR_ST_ELLIP);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_CYL_EQUIDIST);
  flagMeanings.push_back("cyl_equidist");
  snprintf(tmp, BUFSIZ, ", cyl_equidist = %d", TITAN_PROJ_CYL_EQUIDIST);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_FLAT);
  flagMeanings.push_back("azimuthal_equidistant");
  snprintf(tmp, BUFSIZ, ", azimuthal_equidistant = %d", TITAN_PROJ_FLAT);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_OBLIQUE_STEREO);
  flagMeanings.push_back("oblique_stereo");
  snprintf(tmp, BUFSIZ, ", oblique_stereo = %d", TITAN_PROJ_OBLIQUE_STEREO);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_TRANS_MERCATOR);
  flagMeanings.push_back("trans_mercator");
  snprintf(tmp, BUFSIZ, ", trans_mercator = %d", TITAN_PROJ_TRANS_MERCATOR);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_ALBERS);
  flagMeanings.push_back("albers");
  snprintf(tmp, BUFSIZ, ", albers = %d", TITAN_PROJ_ALBERS);
  projTypeNote += tmp;
  
  flagValues.push_back(TITAN_PROJ_LAMBERT_AZIM);
  flagMeanings.push_back("lambert_azim");
  snprintf(tmp, BUFSIZ, ", lambert_azim = %d", TITAN_PROJ_LAMBERT_AZIM);
  projTypeNote += tmp;
  
  if (_fileMode != NcxxFile::read) {
    _scanVars.proj_type.putAtt(FLAG_VALUES, NcxxType::nc_INT,
                               flagValues.size(), flagValues.data());
  }

  vector<const char *> meanings;
  for (size_t ii = 0; ii < flagMeanings.size(); ii++) 
  {
    meanings.push_back(flagMeanings[ii].c_str());
  }
  
  if (_fileMode != NcxxFile::read) {

    _scanVars.proj_type.putAtt(FLAG_MEANINGS, NcxxType::nc_STRING,
                               meanings.size(), meanings.data());
    
    _scanVars.proj_type.putAtt(NOTE, projTypeNote);
  
    _scanVars.proj_origin_lat.putAtt
      (NOTE, std::string("Applies to all projection types except latlon"));
    _scanVars.proj_origin_lon.putAtt
      (NOTE, std::string("Applies to all projection types except latlon"));
    _scanVars.proj_rotation.putAtt
      (NOTE, std::string("Applies to azimuthal_equidistant projection only"));
    
    _scanVars.proj_lat1.putAtt(NOTE, std::string("Applies to lambert_conf and albers"));
    _scanVars.proj_lat2.putAtt(NOTE, std::string("Applies to lambert_conf and albers"));
    _scanVars.proj_tangent_lat.putAtt
      (NOTE, std::string("Applies to polar_stereo and oblique_stereo"));
    _scanVars.proj_tangent_lon.putAtt(NOTE, std::string("Applies to oblique_stereo"));
    _scanVars.proj_pole_type.putAtt(NOTE, std::string("0 = north, 1 = south"));
    _scanVars.proj_central_scale.putAtt
      (NOTE, std::string("Applies to polar_stereo, oblique_stereo, trans_mercator"));

  }

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

int TitanFile::writeStormAux(int storm_num,
                             const storm_file_header_t &storm_file_header,
                             const storm_file_scan_header_t &sheader,
                             const storm_file_global_props_t *gprops,
                             const storm_file_layer_props_t *lprops,
                             const storm_file_dbz_hist_t *hist,
                             const storm_file_run_t *runs,
                             const storm_file_run_t *proj_runs)
  
{

  // save state

  const storm_file_params_t &sparams(storm_file_header.params);
  const storm_file_global_props_t &gp = gprops[storm_num];

  int nLayers = gp.n_layers;
  int nDbzIntervals = gp.n_dbz_intervals;
  int nRuns = gp.n_runs;
  int nProjRuns = gp.n_proj_runs;
    
  allocLayers(nLayers);
  allocHist(nDbzIntervals);
  allocRuns(nRuns);
  allocProjRuns(nProjRuns);
    
  memcpy(_lprops, lprops, nLayers * sizeof(storm_file_layer_props_t));
  memcpy(_hist, hist, nDbzIntervals * sizeof(storm_file_dbz_hist_t));
  memcpy(_runs, runs, nRuns * sizeof(storm_file_run_t));
  memcpy(_proj_runs, proj_runs, nProjRuns * sizeof(storm_file_run_t));

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_sFile.WriteProps(storm_num, sheader.nstorms,
                          gprops, lprops, hist, runs, proj_runs)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }

  // save scan offsets
  
  std::vector<size_t> scanPos = NcxxVar::makeIndex(sheader.scan_num);
  _scanVars.scan_layer_offset_0.putVal(scanPos, _sumLayers);
  _scanVars.scan_hist_offset_0.putVal(scanPos, _sumHist);
  _scanVars.scan_runs_offset_0.putVal(scanPos, _sumRuns);
  _scanVars.scan_proj_runs_offset_0.putVal(scanPos, _sumProjRuns);
  
  // ensure we have space for the offsets
  
  _layerOffsets.resize(storm_num + 1);
  _histOffsets.resize(storm_num + 1);
  _runsOffsets.resize(storm_num + 1);
  _projRunsOffsets.resize(storm_num + 1);
  
  // write layers

  _layerOffsets[storm_num] = _sumLayers;
  for (int ilayer = 0; ilayer < nLayers; ilayer++) {
    const storm_file_layer_props_t &ll = lprops[ilayer];
    int lpropsOffset = _sumLayers;
    std::vector<size_t> layerIndex = NcxxVar::makeIndex(lpropsOffset);
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
    _sumLayers++;
  }
  _topLevelVars.sum_layers.putVal(_sumLayers);

  // write histograms

  _histOffsets[storm_num] = _sumHist;
  for (int ihist = 0; ihist < nDbzIntervals; ihist++) {
    const storm_file_dbz_hist_t &hh = hist[ihist];
    int histOffset = _sumHist;
    std::vector<size_t> histIndex = NcxxVar::makeIndex(histOffset);
    _histVars.percent_volume.putVal(histIndex, hh.percent_volume);
    _histVars.percent_area.putVal(histIndex, hh.percent_area);
    _sumHist++;
  }
  _topLevelVars.sum_hist.putVal(_sumHist);
  
  // write runs
  
  _runsOffsets[storm_num] = _sumRuns;
  for (int irun = 0; irun < nRuns; irun++) {
    const storm_file_run_t &run = runs[irun];
    int runOffset = _sumRuns;
    std::vector<size_t> runIndex = NcxxVar::makeIndex(runOffset);
    _runsVars.run_ix.putVal(runIndex, run.ix);
    _runsVars.run_iy.putVal(runIndex, run.iy);
    _runsVars.run_iz.putVal(runIndex, run.iz);
    _runsVars.run_len.putVal(runIndex, run.n);
    _sumRuns++;
  }
  _topLevelVars.sum_runs.putVal(_sumRuns);
  
  _projRunsOffsets[storm_num] = _sumProjRuns;
  for (int irun = 0; irun < nProjRuns; irun++) {
    const storm_file_run_t &run = proj_runs[irun];
    int projRunOffset = _sumProjRuns;
    std::vector<size_t> projRunIndex = NcxxVar::makeIndex(projRunOffset);
    _projRunsVars.run_ix.putVal(projRunIndex, run.ix);
    _projRunsVars.run_iy.putVal(projRunIndex, run.iy);
    _projRunsVars.run_iz.putVal(projRunIndex, run.iz);
    _projRunsVars.run_len.putVal(projRunIndex, run.n);
    _sumProjRuns++;
  }
  _topLevelVars.sum_proj_runs.putVal(_sumProjRuns);
  
  return 0;
  
}

///////////////////////////////////////////////////////////////
// Truncate storm data when rerunning.
// Keep this scan, set subsequent scans to missing.
// Clear all track data.
//
// Returns 0 on success, -1 on failure.

int TitanFile::truncateData(int lastGoodScanNum)
  
{
  
  if (_isLegacyV5Format) {
    if (_sFile.TruncateData(lastGoodScanNum)) {
      _errStr = _sFile.getErrStr();
      return -1;
    }
    return 0;
  }

  if (lastGoodScanNum >= (int) _scansDim.getSize() - 1) {
    // last good scan num is at end of file
    // so no trucation needed
    return 0;
  }
  _nScans = lastGoodScanNum + 1;

  // read the offsets at the start of the scan after the last good one
  
  std::vector<size_t> scanPos = NcxxVar::makeIndex(_nScans);
  _scanVars.scan_gprops_offset_0.getVal(scanPos, &_sumStorms);
  _scanVars.scan_layer_offset_0.getVal(scanPos, &_sumLayers);
  _scanVars.scan_hist_offset_0.getVal(scanPos, &_sumHist);
  _scanVars.scan_runs_offset_0.getVal(scanPos, &_sumRuns);
  _scanVars.scan_proj_runs_offset_0.getVal(scanPos, &_sumProjRuns);

  // clear the scans

  _clearGroupVars(_scansGroup, _nScans);
  
  // clear the storm properties from this scan forward

  _clearGroupVars(_gpropsGroup, _sumStorms);
  _clearGroupVars(_layersGroup, _sumLayers);
  _clearGroupVars(_histGroup, _sumHist);
  _clearGroupVars(_runsGroup, _sumRuns);
  _clearGroupVars(_projRunsGroup, _sumProjRuns);

  // clear all of the track properties
  
  _clearGroupVars(_simpleGroup, 0);
  _clearGroupVars(_complexGroup, 0);
  _clearGroupVars(_entriesGroup, 0);
  _track_header.n_simple_tracks = 0;
  _track_header.n_complex_tracks = 0;
  _track_header.n_scans = _nScans;
  
  // update the top level status vars
  
  _topLevelVars.n_scans.putVal(_nScans);
  _topLevelVars.sum_storms.putVal(_sumStorms);
  _topLevelVars.sum_layers.putVal(_sumLayers);
  _topLevelVars.sum_hist.putVal(_sumHist);
  _topLevelVars.sum_runs.putVal(_sumRuns);
  _topLevelVars.sum_proj_runs.putVal(_sumProjRuns);
  
  return 0;

}

////////////////////////////////////////////////////
// clear vars in group, from given index to end

void TitanFile::_clearGroupVars(NcxxGroup &group,
                                int startIndex)
  
{

  // get set of vars in group
  
  const std::multimap<std::string, NcxxVar> &vars = group.getVars();

  // loop through vars
  
  for (auto ii : vars) {

    string name(ii.first);
    NcxxVar var(ii.second);

    if (var.getDimCount() == 1) {
      _clear1DVar(var, startIndex);
    } else if (var.getDimCount() == 2) {
      _clear2DVar(var, startIndex);
    }

  } // ii
  
}

void TitanFile::_clear1DVar(NcxxVar &var,
                            int startIndex)
  
{

  NcxxDim dim = var.getDim(0);

  for (int ii = startIndex; ii < (int) dim.getSize(); ii++) {
    std::vector<size_t> index = NcxxVar::makeIndex(ii);
    nc_type vtype = var.getType().getId();
    switch (vtype) {
      case NC_DOUBLE:
        var.putVal(index, missingDouble);
        break;
      case NC_FLOAT:
        var.putVal(index, missingFloat);
        break;
      case NC_INT64:
        var.putVal(index, missingInt64);
        break;
      case NC_INT:
        var.putVal(index, missingInt32);
        break;
      case NC_SHORT:
        var.putVal(index, missingInt16);
        break;
      case NC_UBYTE:
        var.putVal(index, missingInt08);
        break;
      case NC_STRING:
        var.putVal(index, string(""));
        break;
      default: {}
    } // switch
  } // ii
    
}

void TitanFile::_clear2DVar(NcxxVar &var,
                            int startIndex)
  
{

  NcxxDim dim0 = var.getDim(0);
  NcxxDim dim1 = var.getDim(1);

  for (int ii = startIndex; ii < (int) dim0.getSize(); ii++) {
    for (int jj = 0; jj < (int) dim1.getSize(); jj++) {
      std::vector<size_t> index = NcxxVar::makeIndex(ii, jj);
      nc_type vtype = var.getType().getId();
      switch (vtype) {
        case NC_DOUBLE:
          var.putVal(index, missingDouble);
          break;
        case NC_FLOAT:
          var.putVal(index, missingFloat);
          break;
        case NC_INT64:
          var.putVal(index, missingInt64);
          break;
        case NC_INT:
          var.putVal(index, missingInt32);
          break;
        case NC_SHORT:
          var.putVal(index, missingInt16);
          break;
        case NC_UBYTE:
          var.putVal(index, missingInt08);
          break;
        case NC_STRING:
          var.putVal(index, string(""));
          break;
        default: {}
      } // switch
    } // jj
  } // ii
    
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
    
    // _simple_track_offsets = (si32 *) urealloc
    //   (_simple_track_offsets, n_realloc * sizeof(si32));
      
    _n_simples_per_complex = (si32 *) urealloc
      (_n_simples_per_complex, n_realloc * sizeof(si32));
    
    _simples_per_complex_offsets = (si32 *) urealloc
      (_simples_per_complex_offsets, n_realloc * sizeof(si32));
    
    _simples_per_complex_1D = (si32 *) urealloc
      (_simples_per_complex_1D, n_realloc * sizeof(si32));
    
    _simples_per_complex_2D = (si32 **) urealloc
      (_simples_per_complex_2D, n_realloc * sizeof(si32 *));
    
    // _complex_track_offsets = (si32 *) urealloc
    //   (_complex_track_offsets, n_realloc * sizeof(si32));
    
    // initialize new elements to zero
  
    int n_new = _n_simple_allocated - n_start;

    // memset (_simple_track_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_n_simples_per_complex + n_start, 0, n_new * sizeof(si32));
    memset (_simples_per_complex_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_simples_per_complex_1D + n_start, 0, n_new * sizeof(si32));
    // memset (_complex_track_offsets + n_start, 0, n_new * sizeof(si32));
    memset (_simples_per_complex_2D + n_start, 0, n_new * sizeof(si32 *));
  
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
  
  // if (_simple_track_offsets) {
  //   ufree(_simple_track_offsets);
  //   _simple_track_offsets = nullptr;
  // }

  if (_n_simples_per_complex) {
    ufree(_n_simples_per_complex);
    _n_simples_per_complex = nullptr;
  }

  if (_simples_per_complex_offsets) {
    ufree(_simples_per_complex_offsets);
    _simples_per_complex_offsets = nullptr;
  }

  if (_simples_per_complex_1D) {
    ufree(_simples_per_complex_1D);
    _simples_per_complex_1D = nullptr;
  }

  // if (_complex_track_offsets) {
  //   ufree(_complex_track_offsets);
  //   _complex_track_offsets = nullptr;
  // }

  //   freeSimplesPerComplex2D();

  if (_simples_per_complex_2D) {
    for (int i = 0; i < _n_simple_allocated; i++) {
      if (_simples_per_complex_2D[i] != nullptr) {
	ufree(_simples_per_complex_2D[i]);
	_simples_per_complex_2D[i] = nullptr;
      }
    }
    ufree(_simples_per_complex_2D);
    _simples_per_complex_2D = nullptr;
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
  // freeSimplesPerComplex2D();
  freeScanEntries();
  freeScanIndex();
  freeUtime();

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

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadHeader(clear_error_str)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    // save state
    _track_header = _tFile.header();
    assert(_nScans == _track_header.n_scans);
    allocComplexArrays(_track_header.n_complex_tracks);
    allocSimpleArrays(_track_header.n_simple_tracks);
    memcpy(_complex_track_nums, _tFile.complex_track_nums(),
           _track_header.n_complex_tracks * sizeof(si32));
    memcpy(_n_simples_per_complex, _tFile.nsimples_per_complex(),
           _track_header.n_simple_tracks * sizeof(si32));
    memcpy(_simples_per_complex_offsets, _tFile.simples_per_complex_offsets(),
           _track_header.n_simple_tracks * sizeof(si32));
    // read in simples per complex
    if (readSimplesPerComplex()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }

  if (clear_error_str) {
    _clearErrStr();
  }
  _addErrStr("ERROR - TitanFile::readTrackHeader");
  _addErrStr("  Reading from file: ", _filePath);
   
  // read in header data
  
  track_file_params_t &tparams = _track_header.params;
  
  _topLevelVars.max_simple_track_num.getVal(&_track_header.max_simple_track_num);
  _topLevelVars.max_complex_track_num.getVal(&_track_header.max_complex_track_num);
  
  _tparamsVars.forecast_weights.getVal(&tparams.forecast_weights);
  _tparamsVars.weight_distance.getVal(&tparams.weight_distance);
  _tparamsVars.weight_delta_cube_root_volume.getVal(&tparams.weight_delta_cube_root_volume);
  _tparamsVars.merge_split_search_ratio.getVal(&tparams.merge_split_search_ratio);
  _tparamsVars.max_tracking_speed.getVal(&tparams.max_tracking_speed);
  _tparamsVars.max_speed_for_valid_forecast.getVal(&tparams.max_speed_for_valid_forecast);
  _tparamsVars.parabolic_growth_period.getVal(&tparams.parabolic_growth_period);
  _tparamsVars.smoothing_radius.getVal(&tparams.smoothing_radius);
  _tparamsVars.min_fraction_overlap.getVal(&tparams.min_fraction_overlap);
  _tparamsVars.min_sum_fraction_overlap.getVal(&tparams.min_sum_fraction_overlap);
  _tparamsVars.scale_forecasts_by_history.getVal(&tparams.scale_forecasts_by_history);
  _tparamsVars.use_runs_for_overlaps.getVal(&tparams.use_runs_for_overlaps);
  _tparamsVars.grid_type.getVal(&tparams.grid_type);
  _tparamsVars.nweights_forecast.getVal(&tparams.nweights_forecast);
  _tparamsVars.forecast_type.getVal(&tparams.forecast_type);
  _tparamsVars.max_delta_time.getVal(&tparams.max_delta_time);
  _tparamsVars.min_history_for_valid_forecast.getVal(&tparams.min_history_for_valid_forecast);
  _tparamsVars.spatial_smoothing.getVal(&tparams.spatial_smoothing);
  
  _tstateVars.n_simple_tracks.getVal(&_track_header.n_simple_tracks);
  _tstateVars.n_complex_tracks.getVal(&_track_header.n_complex_tracks);
  _tstateVars.tracking_valid.getVal(&_track_header.file_valid);
  _tstateVars.tracking_modify_code.getVal(&_track_header.modify_code);
  _tstateVars.n_samples_for_forecast_stats.getVal(&_track_header.n_samples_for_forecast_stats);
  _tstateVars.last_scan_num.getVal(&_track_header.last_scan_num);
  _tstateVars.max_simple_track_num.getVal(&_track_header.max_simple_track_num);
  _tstateVars.max_complex_track_num.getVal(&_track_header.max_complex_track_num);
  _tstateVars.max_parents.getVal(&_track_header.max_parents);
  _tstateVars.max_children.getVal(&_track_header.max_children);
  _tstateVars.max_nweights_forecast.getVal(&_track_header.max_nweights_forecast);

  // check that the constants in use when the file was written are
  // less than or the same as those in use now
  
  if (_track_header.max_parents != MAX_PARENTS) {
    _addErrStr("  ", "MAX_PARENTS has changed");
    _addErrInt("  _track_header.max_parents: ", _track_header.max_parents);
    _addErrInt("  MAX_PARENTS: ", MAX_PARENTS);
    _addErrStr("  ", "Fix header and recompile");
    return -1;
  }

  if (_track_header.max_children != MAX_CHILDREN) {
    _addErrStr("  ", "MAX_CHILDREN has changed");
    _addErrInt("  _track_header.max_children: ", _track_header.max_children);
    _addErrInt("  MAX_CHILDREN: ", MAX_CHILDREN);
    _addErrStr("  ", "Fix header and recompile");
    return -1;
  }

  if (_track_header.max_nweights_forecast != MAX_NWEIGHTS_FORECAST) {
    _addErrStr("  ", "MAX_NWEIGHTS_FORECAST has changed");
    _addErrInt("  _track_header.max_nweights_forecast: ",
		  _track_header.max_nweights_forecast);
    _addErrInt("  MAX_NWEIGHTS_FORECAST: ",
		  MAX_NWEIGHTS_FORECAST);
    _addErrStr("  ", "Fix header and recompile");
    return -1;
  }

  // alloc arrays

  allocComplexArrays(_track_header.n_complex_tracks);
  allocSimpleArrays(_track_header.n_simple_tracks);
  allocScanIndex(_nScans);
  
  // read in complex track num array
  // complex_track_nums has dimension _n_complex.
  // The track numbers are monotonically increasing, but will have gaps
  // due to mergers and splits which means multiple simple tracks in
  // a complex track.
  
  std::vector<size_t> compNumIndex = NcxxVar::makeIndex(0);
  std::vector<size_t> compNumCount = NcxxVar::makeIndex(_track_header.n_complex_tracks);
  _complexTrackNumsVar.getVal(compNumIndex, compNumCount, _complex_track_nums);

  // read in simples_per_complex 1D array, create 2D array
  
  if (readSimplesPerComplex()) {
    return -1;
  }

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// Read in the track_file_header_t and scan_index array.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readTrackScanIndex(bool clear_error_str /* = true*/ )
  
{
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadScanIndex(clear_error_str)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    // save state
    int n_scans = _tFile.header().n_scans;
    allocScanIndex(n_scans);
    memcpy(_scan_index, _tFile.scan_index(), n_scans * sizeof(track_file_scan_index_t));
    return 0;
  }

  if (clear_error_str) {
    _clearErrStr();
  }
  _addErrStr("ERROR - TitanFile::readTrackScanIndex");
  _addErrStr("  Reading from file: ", _filePath);

  if (readTrackHeader(clear_error_str)) {
    return -1;
  }
  
  // alloc memory

  // int n_scans = _track_header.n_scans;
  allocScanIndex(_nScans);
  vector<int64_t> utimes(_nScans, 0);
  vector<int32_t> n_storms(_nScans, 0);
  vector<int32_t> first_entry_offsets(_nScans, 0);
  
  // initialize read
  
  std::vector<size_t> readIndex = NcxxVar::makeIndex(0);
  std::vector<size_t> readCount = NcxxVar::makeIndex(_nScans);
  
  // read scan details
  
  _scanVars.scan_nstorms.getVal(readIndex, readCount, n_storms.data());
  _scanVars.scan_time.getVal(readIndex, readCount, utimes.data());
  _scanVars.scan_first_offset.getVal(readIndex, readCount, first_entry_offsets.data());

  // load up scan index array
  
  for (int iscan = 0; iscan < _nScans; iscan++) {
    _scan_index[iscan].utime = utimes[iscan];
    _scan_index[iscan].n_entries = n_storms[iscan];
    _scan_index[iscan].first_entry_offset = first_entry_offsets[iscan];
  }
  
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

int TitanFile::readComplexTrackParams(int complex_track_num,
                                      bool read_simples_per_complex,
                                      bool clear_error_str /* = true*/ )
     
{
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadComplexParams(complex_track_num,
                                 false,
                                 clear_error_str)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    // save state
    _complex_params = _tFile.complex_params();
    if (read_simples_per_complex) {
      if (readSimplesPerComplex()) {
        return -1;
      }
    }
    return 0;
  }

  if (clear_error_str) {
    _clearErrStr();
  }
  _addErrStr("ERROR - TitanFile::readComplexTrackParams");
  _addErrStr("  Reading from file: ", _filePath);
  _addErrInt("  track_num", complex_track_num);

  // the complex track params are indexed from the complex track number
  // these arrays will have gaps
  
  std::vector<size_t> varIndex = NcxxVar::makeIndex(complex_track_num);
  complex_track_params_t &cp(_complex_params);
  _complexVars.complex_track_num.getVal(varIndex, &cp.complex_track_num);
  _complexVars.volume_at_start_of_sampling.getVal(varIndex, &cp.volume_at_start_of_sampling);
  _complexVars.volume_at_end_of_sampling.getVal(varIndex, &cp.volume_at_end_of_sampling);
  _complexVars.start_scan.getVal(varIndex, &cp.start_scan);
  _complexVars.end_scan.getVal(varIndex, &cp.end_scan);
  _complexVars.duration_in_scans.getVal(varIndex, &cp.duration_in_scans);
  _complexVars.duration_in_secs.getVal(varIndex, &cp.duration_in_secs);
  _complexVars.start_time.getVal(varIndex, &cp.start_time);
  _complexVars.end_time.getVal(varIndex, &cp.end_time);
  _complexVars.n_simple_tracks.getVal(varIndex, &cp.n_simple_tracks);
  _complexVars.n_top_missing.getVal(varIndex, &cp.n_top_missing);
  _complexVars.n_range_limited.getVal(varIndex, &cp.n_range_limited);
  _complexVars.start_missing.getVal(varIndex, &cp.start_missing);
  _complexVars.end_missing.getVal(varIndex, &cp.end_missing);
  _complexVars.n_samples_for_forecast_stats.getVal(varIndex, &cp.n_samples_for_forecast_stats);

  _complexVerifyVars.ellipse_forecast_n_success.getVal(varIndex, &cp.ellipse_verify.n_success);
  _complexVerifyVars.ellipse_forecast_n_failure.getVal(varIndex, &cp.ellipse_verify.n_success);
  _complexVerifyVars.ellipse_forecast_n_false_alarm.getVal(varIndex, &cp.ellipse_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_success.getVal(varIndex, &cp.polygon_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_failure.getVal(varIndex, &cp.polygon_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_false_alarm.getVal(varIndex, &cp.polygon_verify.n_success);
  
  _complexBiasVars.proj_area_centroid_x.getVal(varIndex, &cp.forecast_bias.proj_area_centroid_x);
  _complexBiasVars.proj_area_centroid_y.getVal(varIndex, &cp.forecast_bias.proj_area_centroid_y);
  _complexBiasVars.vol_centroid_z.getVal(varIndex, &cp.forecast_bias.vol_centroid_z);
  _complexBiasVars.refl_centroid_z.getVal(varIndex, &cp.forecast_bias.refl_centroid_z);
  _complexBiasVars.top.getVal(varIndex, &cp.forecast_bias.top);
  _complexBiasVars.dbz_max.getVal(varIndex, &cp.forecast_bias.dbz_max);
  _complexBiasVars.volume.getVal(varIndex, &cp.forecast_bias.volume);
  _complexBiasVars.precip_flux.getVal(varIndex, &cp.forecast_bias.precip_flux);
  _complexBiasVars.mass.getVal(varIndex, &cp.forecast_bias.mass);
  _complexBiasVars.proj_area.getVal(varIndex, &cp.forecast_bias.proj_area);
  _complexBiasVars.smoothed_proj_area_centroid_x.getVal
    (varIndex, &cp.forecast_bias.smoothed_proj_area_centroid_x);
  _complexBiasVars.smoothed_proj_area_centroid_y.getVal
    (varIndex, &cp.forecast_bias.smoothed_proj_area_centroid_y);
  _complexBiasVars.smoothed_speed.getVal(varIndex, &cp.forecast_bias.smoothed_speed);
  _complexBiasVars.smoothed_direction.getVal(varIndex, &cp.forecast_bias.smoothed_direction);
  
  _complexRmseVars.proj_area_centroid_x.getVal(varIndex, &cp.forecast_rmse.proj_area_centroid_x);
  _complexRmseVars.proj_area_centroid_y.getVal(varIndex, &cp.forecast_rmse.proj_area_centroid_y);
  _complexRmseVars.vol_centroid_z.getVal(varIndex, &cp.forecast_rmse.vol_centroid_z);
  _complexRmseVars.refl_centroid_z.getVal(varIndex, &cp.forecast_rmse.refl_centroid_z);
  _complexRmseVars.top.getVal(varIndex, &cp.forecast_rmse.top);
  _complexRmseVars.dbz_max.getVal(varIndex, &cp.forecast_rmse.dbz_max);
  _complexRmseVars.volume.getVal(varIndex, &cp.forecast_rmse.volume);
  _complexRmseVars.precip_flux.getVal(varIndex, &cp.forecast_rmse.precip_flux);
  _complexRmseVars.mass.getVal(varIndex, &cp.forecast_rmse.mass);
  _complexRmseVars.proj_area.getVal(varIndex, &cp.forecast_rmse.proj_area);
  _complexRmseVars.smoothed_proj_area_centroid_x.getVal
    (varIndex, &cp.forecast_rmse.smoothed_proj_area_centroid_x);
  _complexRmseVars.smoothed_proj_area_centroid_y.getVal
    (varIndex, &cp.forecast_rmse.smoothed_proj_area_centroid_y);
  _complexRmseVars.smoothed_speed.getVal(varIndex, &cp.forecast_rmse.smoothed_speed);
  _complexRmseVars.smoothed_direction.getVal(varIndex, &cp.forecast_rmse.smoothed_direction);
  
  // If read_simples_per_complex is set,
  // read in simples_per_complex array, which indicates which
  // simple tracks are part of this complex track.

  if (read_simples_per_complex) {
    return readSimplesPerComplex();
  }

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the parameters for a simple track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readSimpleTrackParams(int simple_track_num,
                                     bool clear_error_str /* = true */)
     
{
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadSimpleParams(simple_track_num,
                                clear_error_str)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    _simple_params = _tFile.simple_params();
    return 0;
  }

  if (clear_error_str) {
    _clearErrStr();
  }
  _addErrStr("ERROR - TitanFile::readSimpleTrackParams");
  _addErrStr("  Reading from file: ", _filePath);
  _addErrInt("  simple_track_num", simple_track_num);
  
  std::vector<size_t> simpleIndex = NcxxVar::makeIndex(simple_track_num);
  simple_track_params_t &sp(_simple_params);

  _simpleVars.simple_track_num.getVal(simpleIndex, &sp.simple_track_num);
  _simpleVars.last_descendant_simple_track_num.getVal
    (simpleIndex, &sp.last_descendant_simple_track_num);
  _simpleVars.start_scan.getVal(simpleIndex, &sp.start_scan);
  _simpleVars.end_scan.getVal(simpleIndex, &sp.end_scan);
  _simpleVars.last_descendant_end_scan.getVal(simpleIndex, &sp.last_descendant_end_scan);
  _simpleVars.scan_origin.getVal(simpleIndex, &sp.scan_origin);
  _simpleVars.start_time.getVal(simpleIndex, &sp.start_time);
  _simpleVars.end_time.getVal(simpleIndex, &sp.end_time);
  _simpleVars.last_descendant_end_time.getVal(simpleIndex, &sp.last_descendant_end_time);
  _simpleVars.time_origin.getVal(simpleIndex, &sp.time_origin);
  _simpleVars.history_in_scans.getVal(simpleIndex, &sp.history_in_scans);
  _simpleVars.history_in_secs.getVal(simpleIndex, &sp.history_in_secs);
  _simpleVars.duration_in_scans.getVal(simpleIndex, &sp.duration_in_scans);
  _simpleVars.duration_in_secs.getVal(simpleIndex, &sp.duration_in_secs);
  _simpleVars.nparents.getVal(simpleIndex, &sp.nparents);
  _simpleVars.nchildren.getVal(simpleIndex, &sp.nchildren);

  std::vector<size_t> parentIndex = NcxxVar::makeIndex(simple_track_num, 0);
  std::vector<size_t> parentCount = NcxxVar::makeIndex(1, sp.nparents);
  _simpleVars.parent.getVal(parentIndex, parentCount, &sp.parent);
  
  std::vector<size_t> childIndex = NcxxVar::makeIndex(simple_track_num, 0);
  std::vector<size_t> childCount = NcxxVar::makeIndex(1, sp.nchildren);
  _simpleVars.child.getVal(childIndex, childCount, &sp.child);

  _simpleVars.complex_track_num.getVal(simpleIndex, &sp.complex_track_num);
  _simpleVars.first_entry_offset.getVal(simpleIndex, &sp.first_entry_offset);

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

int TitanFile::readTrackEntry()
     
{
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadEntry()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    _entry = _tFile.entry();
    return 0;
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::readEntry");
  _addErrStr("  Reading from file: ", _filePath);

  // set entry offset in the file
  
  int entryOffset;
  if (_first_entry) {
    entryOffset = _simple_params.first_entry_offset;
    _first_entry = false;
  } else {
    entryOffset = _entry.next_entry_offset;
  }

  // do the read

  return _readTrackEntry(_entry, entryOffset);
  
}

int TitanFile::_readTrackEntry(track_file_entry_t &entry,
                               int entryOffset)
     
{
  
  std::vector<size_t> entryIndex = NcxxVar::makeIndex(entryOffset);
  
  _entryVars.time.getVal(entryIndex, &entry.time);
  _entryVars.time_origin.getVal(entryIndex, &entry.time_origin);
  _entryVars.scan_origin.getVal(entryIndex, &entry.scan_origin);
  _entryVars.scan_num.getVal(entryIndex, &entry.scan_num);
  _entryVars.storm_num.getVal(entryIndex, &entry.storm_num);
  _entryVars.simple_track_num.getVal(entryIndex, &entry.simple_track_num);
  _entryVars.complex_track_num.getVal(entryIndex, &entry.complex_track_num);
  _entryVars.history_in_scans.getVal(entryIndex, &entry.history_in_scans);
  _entryVars.history_in_secs.getVal(entryIndex, &entry.history_in_secs);
  _entryVars.duration_in_scans.getVal(entryIndex, &entry.duration_in_scans);
  _entryVars.duration_in_secs.getVal(entryIndex, &entry.duration_in_secs);
  _entryVars.forecast_valid.getVal(entryIndex, &entry.forecast_valid);
  _entryVars.prev_entry_offset.getVal(entryIndex, &entry.prev_entry_offset);
  _entryVars.this_entry_offset.getVal(entryIndex, &entry.this_entry_offset);
  _entryVars.next_entry_offset.getVal(entryIndex, &entry.next_entry_offset);
  _entryVars.next_scan_entry_offset.getVal(entryIndex, &entry.next_scan_entry_offset);

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in the array of simple track numbers for each complex track
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readSimplesPerComplex()
     
{

  if (_isLegacyV5Format) {
    // legacy read
    if (_tFile.ReadSimplesPerComplex()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }

    assert(_track_header.n_simple_tracks == _tFile.header().n_simple_tracks);
    assert(_track_header.n_complex_tracks == _tFile.header().n_complex_tracks);
    
    // copy over data from _tFile object
    allocSimpleArrays(_track_header.n_simple_tracks);
    for (int itrack = 0; itrack < _track_header.n_complex_tracks; itrack++) {
      int complex_num = _tFile.complex_track_nums()[itrack];
      int nsimples = _tFile.nsimples_per_complex()[complex_num];
      _simples_per_complex_2D[complex_num] = (si32 *) urealloc
        (_simples_per_complex_2D[complex_num],
         (nsimples * sizeof(si32)));
      memcpy(_simples_per_complex_2D[complex_num],
             _tFile.simples_per_complex()[complex_num],
             nsimples * sizeof(si32));
    } // itrack

    // fill 1D array

    int offset = 0;
    for (int itrack = 0; itrack < _track_header.n_complex_tracks; itrack++) {
      int complex_num = _complex_track_nums[itrack];
      int nsimples = _n_simples_per_complex[complex_num];
      memcpy(_simples_per_complex_1D + offset,
             _simples_per_complex_2D[complex_num],
             nsimples * sizeof(si32));
      offset += nsimples;
    } // itrack
    
    return 0;

  }
  
  _addErrStr("ERROR - TitanFile::readSimplesPerComplex");

  // check memory allocation
  
  allocSimpleArrays(_track_header.n_simple_tracks);
  
  // set up index and count for retrievals
  
  std::vector<size_t> nSimpIndex = NcxxVar::makeIndex(0);
  std::vector<size_t> nSimpCount = NcxxVar::makeIndex(_track_header.n_simple_tracks);

  // read in n_simples_per_complex
  
  _simpleVars.n_simples_per_complex.getVal
    (nSimpIndex, nSimpCount, _n_simples_per_complex);
  
  // read in simples_per_complex_offsets
  
  _simpleVars.simples_per_complex_offsets.getVal
    (nSimpIndex, nSimpCount, _simples_per_complex_offsets);
  
  // read in simples_per_complex 1D array
  
  _simpleVars.simples_per_complex_1D.getVal
    (nSimpIndex, nSimpCount, _simples_per_complex_1D);

  // fill simples_per_complex_2D
  
  for (int icomp = 0; icomp < _track_header.n_complex_tracks; icomp++) {

    int complex_num = _complex_track_nums[icomp];
    int n_simples = _n_simples_per_complex[complex_num];
    int simples_offset = _simples_per_complex_offsets[complex_num];
    
    _simples_per_complex_2D[complex_num] = (si32 *) urealloc
      (_simples_per_complex_2D[complex_num],
       (n_simples * sizeof(si32)));
    
    for (int isimp = 0; isimp < n_simples; isimp++) {
      _simples_per_complex_2D[complex_num][isimp] =
        _simples_per_complex_1D[simples_offset + isimp];
    } // isimp
    
  } // icomp 
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// load vector with simples per complex, in linear order
//
// in memory these are stored in a si32** 2-d array
//
///////////////////////////////////////////////////////////////////////////

void TitanFile::loadVecSimplesPerComplex(vector<si32> &simpsPerComplexLin,
                                         vector<si32> &simpsPerComplexOffsets)
  
{

  simpsPerComplexLin.clear();
  simpsPerComplexOffsets.resize(_track_header.n_simple_tracks);
  int count = 0;
  
  for (int itrack = 0; itrack < _track_header.n_complex_tracks; itrack++) {
    
    int complex_num = _complex_track_nums[itrack];
    int nsimples = _n_simples_per_complex[complex_num];
    si32 *simples = _simples_per_complex_2D[complex_num];

    for (int ii = 0; ii < nsimples; ii++) {
      simpsPerComplexLin.push_back(simples[ii]);
    }

    simpsPerComplexOffsets[complex_num] = count;
    count += nsimples;
    
  } // itrack 
  
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

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadScanEntries(scan_num)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    // save state
    _n_scan_entries = _tFile.n_scan_entries();
    allocScanEntries(_n_scan_entries);
    memcpy(_scan_entries, _tFile.scan_entries(), _n_scan_entries * sizeof(track_file_entry_t));
    return 0;
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::readScanEntries");
  _addErrStr("  Reading from file: ", _filePath);

  // get nstorms

  int nStorms;
  std::vector<size_t> scanPos = NcxxVar::makeIndex(scan_num);
  _scanVars.scan_nstorms.getVal(scanPos, &nStorms);
  
  // allocate as necessary
  
  _n_scan_entries = nStorms;
  allocScanEntries(_n_scan_entries);
  
  // loop through storms, reading in the entries
  
  track_file_entry_t *entry = _scan_entries;
  
  for (int istorm = 0; istorm < nStorms; istorm++, entry++) {
    int entryOffset = getNextScanEntryOffset(scan_num, istorm);
    // read entry into _entry
    if (_readTrackEntry(*entry, entryOffset)) {
      return -1;
    }
  } // istorm
  
  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// read in track_utime_t array
// the start and end time arrays are used to
// determine if a track is a valid candidate for display
//
// Returns 0 on success or -1 on error
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::readUtime()
     
{

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.ReadUtime()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    // save state
    _n_scan_entries = _tFile.n_scan_entries();
    allocScanEntries(_n_scan_entries);
    memcpy(_scan_entries, _tFile.scan_entries(),
           _n_scan_entries * sizeof(track_file_entry_t));
    return 0;
  }

  _clearErrStr();
  _addErrStr("ERROR - TitanFile::readUtime");
  _addErrStr("  Reading from file: ", _filePath);

  allocUtime();
  
  int nSimple = _track_header.n_simple_tracks;
  int nComplex = _track_header.n_complex_tracks;
  vector<si64> startTime, endTime;
  startTime.resize(nSimple);
  std::vector<size_t> index = NcxxVar::makeIndex(0);
  std::vector<size_t> count = NcxxVar::makeIndex(nSimple);
  
  // read the start and end times for the simple tracks
  
  _simpleVars.start_time.getVal(index, count, startTime.data());
  _simpleVars.end_time.getVal(index, count, endTime.data());
  
  // set the start and end times in the simple track params
  
  for (int isimp = 0; isimp < nSimple; isimp++) {
    int simpleTrackNum = isimp;
    _track_utime[simpleTrackNum].start_simple = startTime[isimp];
    _track_utime[simpleTrackNum].end_simple = endTime[isimp];
  } // isimp 

  // read the start and end times for the complex tracks
  
  _complexVars.start_time.getVal(index, count, startTime.data());
  _complexVars.end_time.getVal(index, count, endTime.data());

  // set the start and end times in the complex track params
  
  for (int icomp = 0; icomp < nComplex; icomp++) {
    int complexTrackNum = _complex_track_nums[icomp];
    if (complexTrackNum < nSimple) {
      _track_utime[complexTrackNum].start_complex = startTime[complexTrackNum];
      _track_utime[complexTrackNum].end_complex = endTime[complexTrackNum];
    }
  } // icomp 
  
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
  
  // if (_n_scan_index_allocated > 0) {
  //   memset(_scan_index, 0,
  //          _n_scan_index_allocated * sizeof(track_file_scan_index_t));
  // }
  
  if (_n_simple_allocated > 0) {
    // memset (_simple_track_offsets, 0,
    //         _n_simple_allocated * sizeof(si32));
    memset (_n_simples_per_complex, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_simples_per_complex_offsets, 0,
	    _n_simple_allocated * sizeof(si32));
    memset (_simples_per_complex_1D, 0,
	    _n_simple_allocated * sizeof(si32));
    // memset (_complex_track_offsets, 0,
    //         _n_simple_allocated * sizeof(si32));
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

int TitanFile::reuseComplexSlot(int complex_track_num)
     
{
  
  if (_isLegacyV5Format) {
    if (_tFile.ReuseComplexSlot(complex_track_num)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }

  // in the NetCDF implementation, we just clear the slot
  
  if (clearComplexSlot(complex_track_num)) {
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// Clear a complex params slot in the file available for
// reuse, by setting the values to missing.
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::clearComplexSlot(int complex_track_num)
  
{

  std::vector<size_t> varIndex = NcxxVar::makeIndex(complex_track_num);

  _complexVars.complex_track_num.putVal(varIndex, Ncxx::missingInt);
  _complexVars.volume_at_start_of_sampling.putVal(varIndex, Ncxx::missingFloat);
  _complexVars.volume_at_end_of_sampling.putVal(varIndex, Ncxx::missingFloat);
  _complexVars.start_scan.putVal(varIndex, Ncxx::missingInt);
  _complexVars.end_scan.putVal(varIndex, Ncxx::missingInt);
  _complexVars.duration_in_scans.putVal(varIndex, Ncxx::missingInt);
  _complexVars.duration_in_secs.putVal(varIndex, Ncxx::missingInt);
  _complexVars.start_time.putVal(varIndex, Ncxx::missingInt);
  _complexVars.end_time.putVal(varIndex, Ncxx::missingInt);
  _complexVars.n_simple_tracks.putVal(varIndex, Ncxx::missingInt);
  _complexVars.n_top_missing.putVal(varIndex, Ncxx::missingInt);
  _complexVars.n_range_limited.putVal(varIndex, Ncxx::missingInt);
  _complexVars.start_missing.putVal(varIndex, Ncxx::missingInt);
  _complexVars.end_missing.putVal(varIndex, Ncxx::missingInt);
  _complexVars.n_samples_for_forecast_stats.putVal(varIndex, Ncxx::missingInt);

  _complexVerifyVars.ellipse_forecast_n_success.putVal(varIndex, Ncxx::missingInt);
  _complexVerifyVars.ellipse_forecast_n_failure.putVal(varIndex, Ncxx::missingInt);
  _complexVerifyVars.ellipse_forecast_n_false_alarm.putVal(varIndex, Ncxx::missingInt);
  _complexVerifyVars.polygon_forecast_n_success.putVal(varIndex, Ncxx::missingInt);
  _complexVerifyVars.polygon_forecast_n_failure.putVal(varIndex, Ncxx::missingInt);
  _complexVerifyVars.polygon_forecast_n_false_alarm.putVal(varIndex, Ncxx::missingInt);
  
  _complexBiasVars.proj_area_centroid_x.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.proj_area_centroid_y.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.vol_centroid_z.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.refl_centroid_z.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.top.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.dbz_max.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.volume.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.precip_flux.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.mass.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.proj_area.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.smoothed_proj_area_centroid_x.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.smoothed_proj_area_centroid_y.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.smoothed_speed.putVal(varIndex, Ncxx::missingFloat);
  _complexBiasVars.smoothed_direction.putVal(varIndex, Ncxx::missingFloat);
  
  _complexRmseVars.proj_area_centroid_x.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.proj_area_centroid_y.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.vol_centroid_z.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.refl_centroid_z.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.top.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.dbz_max.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.volume.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.precip_flux.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.mass.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.proj_area.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.smoothed_proj_area_centroid_x.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.smoothed_proj_area_centroid_y.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.smoothed_speed.putVal(varIndex, Ncxx::missingFloat);
  _complexRmseVars.smoothed_direction.putVal(varIndex, Ncxx::missingFloat);

  if (complex_track_num < _lowest_avail_complex_slot) {
    _lowest_avail_complex_slot = complex_track_num;
  }

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

int TitanFile::rewindSimpleTrack(int simple_track_num)
     
{

  if (_isLegacyV5Format) {
    if (_tFile.RewindSimple(simple_track_num)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    _first_entry = TRUE;
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::RewindSimpleTrack");
  
  // read in simple track params
  
  if (readSimpleTrackParams(simple_track_num, false)) {
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

int TitanFile::rewriteTrackEntry()
     
{
  
  if (_isLegacyV5Format) {
    if (_tFile.RewriteEntry()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _addErrStr("ERROR - TitanFile::rewriteTrackEntry");
  _addErrStr("  Writing to file: ", _filePath);
  
  if (writeTrackEntry(_entry)) {
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
  if (_isLegacyV5Format) {
    if (_tFile.SeekEndData()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//
// seek to the start of data in track data file
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::seekTrackStartData()
     
{

  if (_isLegacyV5Format) {
    if (_tFile.SeekStartData()) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  return 0;
}

///////////////////////////////////////////////////////////////////////////
//
// write the track_file_header_t structure to a track data file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeTrackHeader(const track_file_header_t &track_file_header,
                                const si32 *complex_track_nums,
                                const si32 *n_simples_per_complex,
                                const si32 **simples_per_complex_2D)
     
{

  // save state

  _track_header = track_file_header;
  
  allocSimpleArrays(_track_header.n_simple_tracks);
  allocComplexArrays(_track_header.n_complex_tracks);

  memcpy(_n_simples_per_complex, n_simples_per_complex,
         _track_header.n_simple_tracks *  sizeof(si32));

  for (int ii = 0; ii < _track_header.n_complex_tracks; ii++) {
    int complex_num = complex_track_nums[ii];
    int nsimples = n_simples_per_complex[complex_num];
    _simples_per_complex_2D[complex_num] = (si32 *) urealloc
      (_simples_per_complex_2D[complex_num],
       (nsimples * sizeof(si32)));
    memcpy(_simples_per_complex_2D[complex_num],
           simples_per_complex_2D[complex_num],
           nsimples * sizeof(si32));
  }
  
  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.WriteHeader(track_file_header,
                           _complex_track_nums,
                           _n_simples_per_complex,
                           (const si32**) _simples_per_complex_2D)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeTrackHeader");
  _addErrStr("  File: ", _filePath);

  // make copy
  
  const track_file_params_t &tparams(_track_header.params);
  
  // set top level vars
  
  _topLevelVars.max_simple_track_num.putVal((int) _track_header.max_simple_track_num);
  _topLevelVars.max_complex_track_num.putVal((int) _track_header.max_complex_track_num);
  
  _tparamsVars.forecast_weights.putVal(tparams.forecast_weights);
  _tparamsVars.weight_distance.putVal(tparams.weight_distance);
  _tparamsVars.weight_delta_cube_root_volume.putVal(tparams.weight_delta_cube_root_volume);
  _tparamsVars.merge_split_search_ratio.putVal(tparams.merge_split_search_ratio);
  _tparamsVars.max_tracking_speed.putVal(tparams.max_tracking_speed);
  _tparamsVars.max_speed_for_valid_forecast.putVal(tparams.max_speed_for_valid_forecast);
  _tparamsVars.parabolic_growth_period.putVal(tparams.parabolic_growth_period);
  _tparamsVars.smoothing_radius.putVal(tparams.smoothing_radius);
  _tparamsVars.min_fraction_overlap.putVal(tparams.min_fraction_overlap);
  _tparamsVars.min_sum_fraction_overlap.putVal(tparams.min_sum_fraction_overlap);
  _tparamsVars.scale_forecasts_by_history.putVal(tparams.scale_forecasts_by_history);
  _tparamsVars.use_runs_for_overlaps.putVal(tparams.use_runs_for_overlaps);
  _tparamsVars.grid_type.putVal(tparams.grid_type);
  _tparamsVars.nweights_forecast.putVal(tparams.nweights_forecast);
  _tparamsVars.forecast_type.putVal(tparams.forecast_type);
  _tparamsVars.max_delta_time.putVal(tparams.max_delta_time);
  _tparamsVars.min_history_for_valid_forecast.putVal(tparams.min_history_for_valid_forecast);
  _tparamsVars.spatial_smoothing.putVal(tparams.spatial_smoothing);
  
  _tstateVars.n_simple_tracks.putVal(_track_header.n_simple_tracks);
  _tstateVars.n_complex_tracks.putVal(_track_header.n_complex_tracks);
  _tstateVars.tracking_valid.putVal(_track_header.file_valid);
  _tstateVars.tracking_modify_code.putVal(_track_header.modify_code);
  _tstateVars.n_samples_for_forecast_stats.putVal(_track_header.n_samples_for_forecast_stats);
  _tstateVars.last_scan_num.putVal(_track_header.last_scan_num);
  _tstateVars.max_simple_track_num.putVal(_track_header.max_simple_track_num);
  _tstateVars.max_complex_track_num.putVal(_track_header.max_complex_track_num);
  _tstateVars.max_parents.putVal(_track_header.max_parents);
  _tstateVars.max_children.putVal(_track_header.max_children);
  _tstateVars.max_nweights_forecast.putVal(_track_header.max_nweights_forecast);

  return 0;
  
}

///////////////////////////////////////////////////////////////////////////
//
// write simple track params at the end of the file
//
// returns 0 on success, -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeSimpleTrackParams(int simple_track_num,
                                      const simple_track_params_t &sparams)
     
{

  // save state

  _simple_params = sparams;

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.WriteSimpleParams(sparams)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeSimpleParams");
  _addErrStr("  Writing to file: ", _filePath);
  
  std::vector<size_t> simpleIndex = NcxxVar::makeIndex(simple_track_num);

  _simpleVars.simple_track_num.putVal(simpleIndex, sparams.simple_track_num);
  _simpleVars.last_descendant_simple_track_num.putVal(simpleIndex,
                                                      sparams.last_descendant_simple_track_num);
  _simpleVars.start_scan.putVal(simpleIndex, sparams.start_scan);
  _simpleVars.end_scan.putVal(simpleIndex, sparams.end_scan);
  _simpleVars.last_descendant_end_scan.putVal(simpleIndex, sparams.last_descendant_end_scan);
  _simpleVars.scan_origin.putVal(simpleIndex, sparams.scan_origin);
  _simpleVars.start_time.putVal(simpleIndex, sparams.start_time);
  _simpleVars.end_time.putVal(simpleIndex, sparams.end_time);
  _simpleVars.last_descendant_end_time.putVal(simpleIndex, sparams.last_descendant_end_time);
  _simpleVars.time_origin.putVal(simpleIndex, sparams.time_origin);
  _simpleVars.history_in_scans.putVal(simpleIndex, sparams.history_in_scans);
  _simpleVars.history_in_secs.putVal(simpleIndex, sparams.history_in_secs);
  _simpleVars.duration_in_scans.putVal(simpleIndex, sparams.duration_in_scans);
  _simpleVars.duration_in_secs.putVal(simpleIndex, sparams.duration_in_secs);
  _simpleVars.nparents.putVal(simpleIndex, sparams.nparents);
  _simpleVars.nchildren.putVal(simpleIndex, sparams.nchildren);

  std::vector<size_t> parentIndex = NcxxVar::makeIndex(simple_track_num, 0);
  std::vector<size_t> parentCount = NcxxVar::makeIndex(1, sparams.nparents);
  _simpleVars.parent.putVal(parentIndex, parentCount, sparams.parent);
  
  std::vector<size_t> childIndex = NcxxVar::makeIndex(simple_track_num, 0);
  std::vector<size_t> childCount = NcxxVar::makeIndex(1, sparams.nchildren);
  _simpleVars.child.putVal(childIndex, childCount, sparams.child);

  _simpleVars.complex_track_num.putVal(simpleIndex, sparams.complex_track_num);
  _simpleVars.first_entry_offset.putVal(simpleIndex, sparams.first_entry_offset);

  return 0;

}

///////////////////////////////////////////////////////////////////////////
//
// write complex track params.
//
// complex_index is the index of this track in the complex_track_nums array
//
// returns 0 on success, -1 on failure.
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeComplexTrackParams(int complex_index,
                                       const complex_track_params_t &cparams)
  
{

  // save state

  _complex_params = cparams;

  // handle legacy format
  
  if (_isLegacyV5Format) {
    if (_tFile.WriteComplexParams(complex_index, cparams)) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return 0;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeComplexParams");
  _addErrStr("  Writing to file: ", _filePath);
  
  // complex_track_nums are monotonically increasing, with no gaps
  // however the maxComplex dimension may be larger than the number of
  // complex tracks, if there are mergers and splits
  
  std::vector<size_t> numIndex = NcxxVar::makeIndex(complex_index);
  _complexTrackNumsVar.putVal(numIndex, cparams.complex_track_num);
  
  // variables with dimension _maxComplexDim
  
  std::vector<size_t> varIndex = NcxxVar::makeIndex(cparams.complex_track_num);
  _complexVars.complex_track_num.putVal(varIndex, cparams.complex_track_num);
  _complexVars.volume_at_start_of_sampling.putVal(varIndex, cparams.volume_at_start_of_sampling);
  _complexVars.volume_at_end_of_sampling.putVal(varIndex, cparams.volume_at_end_of_sampling);
  _complexVars.start_scan.putVal(varIndex, cparams.start_scan);
  _complexVars.end_scan.putVal(varIndex, cparams.end_scan);
  _complexVars.duration_in_scans.putVal(varIndex, cparams.duration_in_scans);
  _complexVars.duration_in_secs.putVal(varIndex, cparams.duration_in_secs);
  _complexVars.start_time.putVal(varIndex, cparams.start_time);
  _complexVars.end_time.putVal(varIndex, cparams.end_time);
  _complexVars.n_simple_tracks.putVal(varIndex, cparams.n_simple_tracks);
  _complexVars.n_top_missing.putVal(varIndex, cparams.n_top_missing);
  _complexVars.n_range_limited.putVal(varIndex, cparams.n_range_limited);
  _complexVars.start_missing.putVal(varIndex, cparams.start_missing);
  _complexVars.end_missing.putVal(varIndex, cparams.end_missing);
  _complexVars.n_samples_for_forecast_stats.putVal(varIndex, cparams.n_samples_for_forecast_stats);

  _complexVerifyVars.ellipse_forecast_n_success.putVal(varIndex, cparams.ellipse_verify.n_success);
  _complexVerifyVars.ellipse_forecast_n_failure.putVal(varIndex, cparams.ellipse_verify.n_success);
  _complexVerifyVars.ellipse_forecast_n_false_alarm.putVal(varIndex, cparams.ellipse_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_success.putVal(varIndex, cparams.polygon_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_failure.putVal(varIndex, cparams.polygon_verify.n_success);
  _complexVerifyVars.polygon_forecast_n_false_alarm.putVal(varIndex, cparams.polygon_verify.n_success);
  
  _complexBiasVars.proj_area_centroid_x.putVal(varIndex, cparams.forecast_bias.proj_area_centroid_x);
  _complexBiasVars.proj_area_centroid_y.putVal(varIndex, cparams.forecast_bias.proj_area_centroid_y);
  _complexBiasVars.vol_centroid_z.putVal(varIndex, cparams.forecast_bias.vol_centroid_z);
  _complexBiasVars.refl_centroid_z.putVal(varIndex, cparams.forecast_bias.refl_centroid_z);
  _complexBiasVars.top.putVal(varIndex, cparams.forecast_bias.top);
  _complexBiasVars.dbz_max.putVal(varIndex, cparams.forecast_bias.dbz_max);
  _complexBiasVars.volume.putVal(varIndex, cparams.forecast_bias.volume);
  _complexBiasVars.precip_flux.putVal(varIndex, cparams.forecast_bias.precip_flux);
  _complexBiasVars.mass.putVal(varIndex, cparams.forecast_bias.mass);
  _complexBiasVars.proj_area.putVal(varIndex, cparams.forecast_bias.proj_area);
  _complexBiasVars.smoothed_proj_area_centroid_x.putVal(varIndex, cparams.forecast_bias.smoothed_proj_area_centroid_x);
  _complexBiasVars.smoothed_proj_area_centroid_y.putVal(varIndex, cparams.forecast_bias.smoothed_proj_area_centroid_y);
  _complexBiasVars.smoothed_speed.putVal(varIndex, cparams.forecast_bias.smoothed_speed);
  _complexBiasVars.smoothed_direction.putVal(varIndex, cparams.forecast_bias.smoothed_direction);
  
  _complexRmseVars.proj_area_centroid_x.putVal(varIndex, cparams.forecast_rmse.proj_area_centroid_x);
  _complexRmseVars.proj_area_centroid_y.putVal(varIndex, cparams.forecast_rmse.proj_area_centroid_y);
  _complexRmseVars.vol_centroid_z.putVal(varIndex, cparams.forecast_rmse.vol_centroid_z);
  _complexRmseVars.refl_centroid_z.putVal(varIndex, cparams.forecast_rmse.refl_centroid_z);
  _complexRmseVars.top.putVal(varIndex, cparams.forecast_rmse.top);
  _complexRmseVars.dbz_max.putVal(varIndex, cparams.forecast_rmse.dbz_max);
  _complexRmseVars.volume.putVal(varIndex, cparams.forecast_rmse.volume);
  _complexRmseVars.precip_flux.putVal(varIndex, cparams.forecast_rmse.precip_flux);
  _complexRmseVars.mass.putVal(varIndex, cparams.forecast_rmse.mass);
  _complexRmseVars.proj_area.putVal(varIndex, cparams.forecast_rmse.proj_area);
  _complexRmseVars.smoothed_proj_area_centroid_x.putVal(varIndex, cparams.forecast_rmse.smoothed_proj_area_centroid_x);
  _complexRmseVars.smoothed_proj_area_centroid_y.putVal(varIndex, cparams.forecast_rmse.smoothed_proj_area_centroid_y);
  _complexRmseVars.smoothed_speed.putVal(varIndex, cparams.forecast_rmse.smoothed_speed);
  _complexRmseVars.smoothed_direction.putVal(varIndex, cparams.forecast_rmse.smoothed_direction);

  // {
  //   bool fillMode;
  //   float fillValueFloat;
  //   _globalBiasVars.smoothed_direction.getFillModeParameters(fillMode, fillValueFloat);
  //   cerr << "FFFFFFFFFFFF fillMode, fillValue: " << fillMode << ", " << fillValueFloat << endl;
  //   int fillValueInt;
  //   _complexVars.duration_in_secs.getFillModeParameters(fillMode, fillValueInt);
  //   cerr << "IIIIIIIIIIIIIII fillMode, fillValue: " << fillMode << ", " << fillValueInt << endl;
  // }
  
  return 0;

}
///////////////////////////////////////////////////////////////////////////
//
// write an entry for a track in the track data file
// on success returns the offset of the entry written
// -1 on failure
//
///////////////////////////////////////////////////////////////////////////

int TitanFile::writeTrackEntry(const track_file_entry_t &entry)
  
{
  
  // save state

  _entry = entry;

  // handle legacy format
  
  if (_isLegacyV5Format) {
    int writeOffset = _tFile.WriteEntry(entry);
    if (writeOffset < 0) {
      _errStr = _tFile.getErrStr();
      return -1;
    }
    return writeOffset;
  }
  
  _clearErrStr();
  _addErrStr("ERROR - TitanFile::writeEntry");
  _addErrStr("  Writing to file: ", _filePath);

  // compute offset for this and next entry
  
  int thisEntryOffset = getScanEntryOffset(_entry.scan_num, _entry.storm_num);
  int nextScanEntryOffset = getNextScanEntryOffset(_entry.scan_num, _entry.storm_num);
  
  // set initial prev offset
  
  if (entry.duration_in_secs == 0) {
    // first entry in a simple track - initialize
    _prevEntryOffset = -1;
  } else {
    // store this_offset as next_offset of prev entry
    std::vector<size_t> prevIndex = NcxxVar::makeIndex(_prevEntryOffset);
    _entryVars.next_entry_offset.putVal(prevIndex, thisEntryOffset);
  }

  // do the write
  
  std::vector<size_t> thisIndex = NcxxVar::makeIndex(thisEntryOffset);
  _entryVars.time.putVal(thisIndex, entry.time);
  _entryVars.time_origin.putVal(thisIndex, entry.time_origin);
  _entryVars.scan_origin.putVal(thisIndex, entry.scan_origin);
  _entryVars.scan_num.putVal(thisIndex, entry.scan_num);
  _entryVars.storm_num.putVal(thisIndex, entry.storm_num);
  _entryVars.simple_track_num.putVal(thisIndex, entry.simple_track_num);
  _entryVars.complex_track_num.putVal(thisIndex, entry.complex_track_num);
  _entryVars.history_in_scans.putVal(thisIndex, entry.history_in_scans);
  _entryVars.history_in_secs.putVal(thisIndex, entry.history_in_secs);
  _entryVars.duration_in_scans.putVal(thisIndex, entry.duration_in_scans);
  _entryVars.duration_in_secs.putVal(thisIndex, entry.duration_in_secs);
  _entryVars.forecast_valid.putVal(thisIndex, entry.forecast_valid);
  _entryVars.prev_entry_offset.putVal(thisIndex, _prevEntryOffset);
  _entryVars.this_entry_offset.putVal(thisIndex, thisEntryOffset);
  _entryVars.next_entry_offset.putVal(thisIndex, -1);
  _entryVars.next_scan_entry_offset.putVal(thisIndex, nextScanEntryOffset);

  // save offset for next time
  
  _prevEntryOffset = thisEntryOffset;

  // return this offset
  
  return thisEntryOffset;
  
}

///////////////////////////////////////////////////////////
// write arrays designating which simple tracks are
// contained in each complex track
// returns 0 on success, -1 on failure

int TitanFile::writeSimplesPerComplexArrays(int n_simple_tracks,
                                            const si32 *n_simples_per_complex,
                                            const si32 *simples_per_complex_offsets,
                                            const si32 *simples_per_complex_1D)
  
{

  if (_isLegacyV5Format) {
    return 0;
  }

  std::vector<size_t> index = NcxxVar::makeIndex(0);
  std::vector<size_t> count = NcxxVar::makeIndex(n_simple_tracks);
  
  _simpleVars.n_simples_per_complex.putVal(index, count, n_simples_per_complex);
  _simpleVars.simples_per_complex_offsets.putVal(index, count, simples_per_complex_offsets);
  _simpleVars.simples_per_complex_1D.putVal(index, count, simples_per_complex_1D);
  
  return 0;
  
}

///////////////////////////////////////////////////////////
// get the offset of storm or entry props, given the
// scan_num and storm_num.
//
// First we read the scan first offset, and then add the
// storm_num.

int TitanFile::getScanEntryOffset(int scan_num,
                                  int storm_num)
{
  std::vector<size_t> scanPos = NcxxVar::makeIndex(scan_num);
  int scanFirstOffset;
  _scanVars.scan_first_offset.getVal(scanPos, &scanFirstOffset);
  int entryOffset = scanFirstOffset + storm_num;
  return entryOffset;
}
  
///////////////////////////////////////////////////////////
// get the next offset in the scan, for a given entry
// returns -1 if this is the last entry in a scan

int TitanFile::getNextScanEntryOffset(int scan_num,
                                      int storm_num)
{
  std::vector<size_t> scanPos = NcxxVar::makeIndex(scan_num);
  int scanFirstOffset;
  _scanVars.scan_first_offset.getVal(scanPos, &scanFirstOffset);
  int scanNStorms;
  _scanVars.scan_nstorms.getVal(scanPos, &scanNStorms);
  if (storm_num < (scanNStorms - 1)) {
    int nextEntryOffset = scanFirstOffset + storm_num + 1;
    return nextEntryOffset;
  } else {
    return -1;
  }
}
  
  
//////////////////////////////////////////////////////////////
//
// convert ellipse parameters from deg to km,
// for those which were computed from latlon grids.
//
//////////////////////////////////////////////////////////////

void TitanFile::_convertEllipse2Km(const titan_grid_t &tgrid,
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

  _convertEllipse2Km(scan.grid,
                     gprops.precip_area_centroid_x,
                     gprops.precip_area_centroid_y,
                     gprops.precip_area_orientation,
                     gprops.precip_area_minor_radius,
                     gprops.precip_area_major_radius);
  
  _convertEllipse2Km(scan.grid,
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


