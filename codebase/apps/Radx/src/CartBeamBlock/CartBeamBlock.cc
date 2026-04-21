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
///////////////////////////////////////////////////////////////
//
// CartBeamBlock
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2026
//
///////////////////////////////////////////////////////////////
//
// CartBeamBlock computes beam blockage for a specified radar
// type and location, and for a specified 3D Cartesian grid.
//
///////////////////////////////////////////////////////////////

#include "CartBeamBlock.hh"
#include "DemProvider.hh"
#include "BlockageCalc.hh"
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <cmath>
#include <toolsa/umisc.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/pjg_flat.h>
#include <didss/DsInputPath.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxRadar.hh>

using namespace rainfields;
using namespace rainfields::ancilla;
namespace fs = std::filesystem;
const fl32 CartBeamBlock::missingFl32 = -9999.0;
  
// Constructor

CartBeamBlock::CartBeamBlock(int argc, char **argv) :
        _dem(nullptr),
        _pattern(nullptr),
        _calc(nullptr)
  
{

  OK = TRUE;

  // set programe name

  _progName = "CartBeamBlock";
  ucopyright((char *) _progName.c_str());
  
  // parse command line args
  
  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args." << endl;
    OK = FALSE;
    return;
  }
  
  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list, &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters." << endl;
    OK = FALSE;
    return;
  }

  // create computational objects
  
  _dem = new DemProvider(_params);
  _pattern = new BeamPowerPattern;
  _calc = new BlockageCalc(_params, *_dem, *_pattern);

}

// destructor

CartBeamBlock::~CartBeamBlock()

{

  delete _dem;
  delete _pattern;
  delete _calc;
  
}

//////////////////////////////////////////////////
// Run

int CartBeamBlock::Run()
{

  // read a Cart file to get the grid details

  if (_readGridTemplate(_params.grid_template_path)) {
    cerr << "ERROR - CartBeamBlock::Run()" << endl;
    cerr << "  Cannot find template file, path: " << _params.grid_template_path << endl;
    return -1;
  }

  // get digital terrain data

  if (_readDem(_params.dem_path)) {
    cerr << "ERROR - CartBeamBlock::Run()" << endl;
    cerr << "  Cannot read DEM data, path: " << _params.dem_path << endl;
    return -1;
  }

  // compute beam blockage for each point in the Cartesian grid
  
  if (_computeBlockage()) {
    cerr << "ERROR - CartBeamBlock::Run()" << endl;
    cerr << "  Cannot compute beam blockage" << endl;
    return -1;
  }

  // write out blockage data to MDV

  if (_writeBlockage()) {
    cerr << "ERROR - CartBeamBlock::Run()" << endl;
    cerr << "  Cannot write beam blockage file" << endl;
    return -1;
  }

  return 0;
  
}

//////////////////////////////////////////////////
// Read in the grid details from a template file

int CartBeamBlock::_readGridTemplate(const string &path)
{

  // check whether we have a file or directory
  
  if (fs::is_directory(path)) {

    DsInputPath input(_progName,
                      _params.debug >= Params::DEBUG_VERBOSE,
                      path,
                      0, time(NULL));

    const char *filePath = input.next();
    if (filePath == nullptr) {
      cerr << "ERROR - CartBeamBlock::_readGridTemplate" << endl;
      cerr << "  Cannot find any files, dir: " << path << endl;
      return -1;
    }
    
    if (_readTemplateFile(filePath)) {
      cerr << "ERROR - CartBeamBlock::_readGridTemplate" << endl;
      cerr << "  Cannot read template from dir, file path: " << filePath << endl;
      return -1;
    }

  } else if (fs::is_regular_file(path)) {
    
    if (_readTemplateFile(path)) {
      cerr << "ERROR - CartBeamBlock::_readGridTemplate" << endl;
      cerr << "  Cannot read specified template file: " << path << endl;
      return -1;
    }

  } else {

    cerr << "ERROR - CartBeamBlock::_readGridTemplate" << endl;
    cerr << "  Path does not exist as dir or file: " << path << endl;
    return -1;

  }

  return 0;
  
}

int CartBeamBlock::_readTemplateFile(const string &path)
{

  _templateMdvx.clear();
  _templateMdvx.setReadPath(path);
  _templateMdvx.addReadField(_params.template_field_name);
  if (_templateMdvx.readVolume()) {
    cerr << "ERROR - CartBeamBlock::_readTemplateFile" << endl;
    cerr << "  Cannot read template file: " << path << endl;
    cerr << "  " << _templateMdvx.getErrStr() << endl;
    return -1;
  }
  if (_templateMdvx.getNFieldsFile() < 1) {
    cerr << "ERROR - CartBeamBlock::_readTemplateFile" << endl;
    cerr << "  Cannot find specified field in template file: " << path << endl;
    cerr << "  Field name: " << _params.template_field_name << endl;
    return -1;
  }
  _templateField = _templateMdvx.getField(_params.template_field_name);
  if (_templateField == nullptr) {
    cerr << "  Cannot find specified field in template file: " << path << endl;
    cerr << "  Field name: " << _params.template_field_name << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "INFO - read template file: " << path << endl;
  }
  
  _proj.init(_templateField->getFieldHeader());
  if (_params.debug) {
    cerr << "INFO - PROJECTION" << path << endl;
    _proj.print(cerr);
  }

  _templateMhdr = _templateMdvx.getMasterHeader();
  _templateFhdr = _templateField->getFieldHeader();
  _templateVhdr = _templateField->getVlevelHeader();
  _sensorLat = _templateMhdr.sensor_lat;
  _sensorLon = _templateMhdr.sensor_lon;
  _sensorHtKm = _templateMhdr.sensor_alt;
  _sensorHtM = _sensorHtKm * 1000.0;
  _proj.setSensorPosn(_sensorLat, _sensorLon, _sensorHtKm);

  _zKm.clear();
  for (int iz = 0; iz < _templateFhdr.nz; iz++) {
    _zKm.push_back(_templateVhdr.level[iz]);
  }
  
  _radarLat = _sensorLat;
  _radarLon = _sensorLon;
  _radarHtKm = _sensorHtKm;
  _radarWavelengthCm = _params.radar_wavelength_cm;
  _horizBeamWidthDeg = _params.horiz_beam_width_deg;
  _vertBeamWidthDeg = _params.vert_beam_width_deg;
  
  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(_templateMdvx) == 0 &&
      mdvxRadar.radarParamsAvail()) {
    
    DsRadarParams rparams = mdvxRadar.getRadarParams();

    if (_params.override_radar_location) {
      _radarLat = _params.radar_location.latitudeDeg;
      _radarLon = _params.radar_location.longitudeDeg;
      _radarHtKm = _params.radar_location.heightKm;
    } else {
      _radarLat = rparams.latitude;
      _radarLon = rparams.longitude;
      _radarHtKm = rparams.altitude;
    }

    if (!_params.override_radar_wavelength) {
      _radarWavelengthCm = rparams.wavelength;
    }

    if (!_params.override_radar_beamwidth) {
      _horizBeamWidthDeg = rparams.horizBeamWidth;
      _vertBeamWidthDeg = rparams.vertBeamWidth;
    }

  } // if (mdvxRadar.loadFromMdvx(_templateMdvx) == 0 ....

  // compute lat/lon limits
  
  int margin = 5;
  _proj.getEdgeExtrema(_minLat, _minLon, _maxLat, _maxLon, margin);

  // compute max range limit from radar
  
  double maxRangeKm = 0.0;
  double range = 0.0, bearing = 0.0;
  // SW corner
  PJGLatLon2RTheta(_radarLat, _radarLon, _minLat, _minLon, &range, &bearing);
  maxRangeKm = std::max(maxRangeKm, range);
  // SE corner
  PJGLatLon2RTheta(_radarLat, _radarLon, _minLat, _maxLon, &range, &bearing);
  maxRangeKm = std::max(maxRangeKm, range);
  // NE corner
  PJGLatLon2RTheta(_radarLat, _radarLon, _maxLat, _maxLon, &range, &bearing);
  maxRangeKm = std::max(maxRangeKm, range);
  // NW corner
  PJGLatLon2RTheta(_radarLat, _radarLon, _maxLat, _minLon, &range, &bearing);
  maxRangeKm = std::max(maxRangeKm, range);
  
  // set the beam pattern object

  _pattern->set(euclid::EuclidAngle::fromDegrees(_vertBeamWidthDeg),
                euclid::EuclidAngle::fromDegrees(_horizBeamWidthDeg),
                _params.n_pattern_vert,
                _params.n_pattern_horiz,
                euclid::EuclidAngle::fromDegrees(_vertBeamWidthDeg * 3.0),
                euclid::EuclidAngle::fromDegrees(_horizBeamWidthDeg * 3.0));
  
  _pattern->makeVerticalIntegration();
  
  // initialize blockage calculations
  
  _calc->initGeom(maxRangeKm, _params.range_res_m / 1000.0,
                  _zKm,
                  _params.n_pattern_vert,
                  _params.n_pattern_horiz);

  _calc->setRadarLoc(_radarLat, _radarLon, _radarHtKm);

  return 0;
  
}

//////////////////////////////////////////////////
// Read in the DEM

int CartBeamBlock::_readDem(const string &path)
{

  if (!fs::is_directory(path) && (!fs::is_regular_file(path))) {
    cerr << "ERROR - CartBeamBlock::_readDem" << endl;
    cerr << "  DEM path does not exist: " << path << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "INFO: reading DEM from path: " << path << endl;
    cerr << "      DEM model: " << _dem->ModelName(_params.dem_data_format) << endl;
  }
  
  // compute safe projection lat/lon limits, with a margin of 5 grid points
  
  if (_params.debug) {
    cerr << "INFO: retrieving DEM data for lat/lon bounding box" << endl;
    cerr << "      minLat, maxLat: " << _minLat << ", " << _maxLat << endl;
    cerr << "      minLon, maxLon: " << _minLon << ", " << _maxLon << endl;
  }
    
  // initialize dem for reading
  
  std::pair<double,double> sw(_minLat, _minLon);
  std::pair<double,double> ne(_maxLat, _maxLon);
  if (_dem->set(sw, ne)) {
    cerr << "ERROR - CartBeamBlock::_readDem" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////////////////
// compute beam blockage for each point in the Cartesian grid

int CartBeamBlock::_computeBlockage()
  
{

  const Mdvx::coord_t &coord = _proj.getCoord();

  // allocate output array

  size_t nPtsPlane = _templateFhdr.nx * _templateFhdr.ny;
  _blockage.resize(nPtsPlane * _templateFhdr.nz);
  std::fill(_blockage.begin(), _blockage.end(), missingFl32);

  // compute max range from radar to grid corners
  // needed for initializing each thread-local BlockageCalc

  double maxRangeKm = 0.0;
  double rangeKm, bearingDeg;

  PJGLatLon2RTheta(_radarLat, _radarLon,
                   _minLat, _minLon,
                   &rangeKm, &bearingDeg);
  maxRangeKm = std::max(maxRangeKm, rangeKm);

  PJGLatLon2RTheta(_radarLat, _radarLon,
                   _minLat, _maxLon,
                   &rangeKm, &bearingDeg);
  maxRangeKm = std::max(maxRangeKm, rangeKm);

  PJGLatLon2RTheta(_radarLat, _radarLon,
                   _maxLat, _maxLon,
                   &rangeKm, &bearingDeg);
  maxRangeKm = std::max(maxRangeKm, rangeKm);

  PJGLatLon2RTheta(_radarLat, _radarLon,
                   _maxLat, _minLon,
                   &rangeKm, &bearingDeg);
  maxRangeKm = std::max(maxRangeKm, rangeKm);
  
  // determine number of compute threads

  int nThreads = _params.n_compute_threads;
  if (nThreads <= 0) {
    nThreads = (int) std::thread::hardware_concurrency();
    if (nThreads < 1) {
      nThreads = 1;
    }
  }
  if (nThreads > coord.ny) {
    nThreads = coord.ny;
  }
  if (nThreads < 1) {
    nThreads = 1;
  }

  if (_params.debug) {
    cerr << "INFO - CartBeamBlock::_computeBlockage()" << endl;
    cerr << "  nx, ny, nz: "
         << coord.nx << ", "
         << coord.ny << ", "
         << coord.nz << endl;
    cerr << "  nThreads: " << nThreads << endl;
    cerr << "  maxRangeKm: " << maxRangeKm << endl;
  }

  // divide rows among threads in contiguous chunks

  vector<int> iyStart(nThreads, 0);
  vector<int> iyEnd(nThreads, 0);
  vector<int> threadStatus(nThreads, 0);
  vector<string> threadErrStr(nThreads);
  vector<std::thread> workers;

  int rowsPerThread = coord.ny / nThreads;
  int rowsExtra = coord.ny % nThreads;
  int iyBegin = 0;

  for (int ii = 0; ii < nThreads; ii++) {
    int nRows = rowsPerThread;
    if (ii < rowsExtra) {
      nRows++;
    }
    iyStart[ii] = iyBegin;
    iyEnd[ii] = iyBegin + nRows;
    iyBegin += nRows;
    if (_params.debug) {
      cerr << "thread num, iyStart, iyEnd: "
           << ii << ", " << iyStart[ii] << ", " << iyEnd[ii] << ", " << endl;
    }
  }

  // lamda function
  // worker to process a contiguous block of rows
  // designed to allow multithreading

  auto computeRows = [&](int threadNum, int iy0, int iy1) {

    // thread-local blockage calculator

    BlockageCalc calc(_params, *_dem, *_pattern);
    calc.initGeom(maxRangeKm,
                  _params.range_res_m / 1000.0,
                  _zKm,
                  _params.n_pattern_vert,
                  _params.n_pattern_horiz);
    calc.setRadarLoc(_radarLat, _radarLon, _radarHtKm);

    // scratch array reused for each grid point

    vector<double> fractionBlocked(_templateFhdr.nz, 0.0);

    for (int iy = iy0; iy < iy1; iy++) {

      if (_params.debug) {
        cerr << "--->> thread starting to process row, threadNum, rowNum: "
             << threadNum << ", " << iy << endl;
      }
      
      for (int ix = 0; ix < coord.nx; ix++) {
        
        // get lat/lon of grid point

        double lat, lon;
        _proj.xyIndex2latlon(ix, iy, lat, lon);

        // compute ground range and azimuth from radar

        double gndRangeKm, azDeg;
        PJGLatLon2RTheta(_radarLat, _radarLon,
                         lat, lon,
                         &gndRangeKm, &azDeg);

        // compute blockage for all planes at this grid point

        if (calc.getBlockage(lat, lon, gndRangeKm, azDeg,
                             fractionBlocked)) {
          threadStatus[threadNum] = -1;
          threadErrStr[threadNum] =
            "ERROR - BlockageCalc::getBlockage() failed";
          return;
        }

        // copy to output 3D array
        // layout is plane-major in Z, so planes are separated by nPtsPlane

        size_t index = iy * _templateFhdr.nx + ix;
        for (int iz = 0; iz < coord.nz; iz++, index += nPtsPlane) {
          _blockage[index] = fractionBlocked[iz];
        }

      } // ix
    } // iy

  }; // auto computeRows .....

  // run single-threaded or multi-threaded

  if (nThreads == 1) {

    computeRows(0, 0, coord.ny);

  } else {

    for (int ii = 0; ii < nThreads; ii++) {
      workers.emplace_back(computeRows, ii, iyStart[ii], iyEnd[ii]);
    }

    for (size_t ii = 0; ii < workers.size(); ii++) {
      workers[ii].join();
    }

  }

  // check thread status

  for (int ii = 0; ii < nThreads; ii++) {
    if (threadStatus[ii]) {
      cerr << "ERROR - CartBeamBlock::_computeBlockage()" << endl;
      if (!threadErrStr[ii].empty()) {
        cerr << "  " << threadErrStr[ii] << endl;
      }
      cerr << "  thread num: " << ii << endl;
      cerr << "  iy start, iy end: "
           << iyStart[ii] << ", " << iyEnd[ii] << endl;
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////
// write blockage data to MDV

int CartBeamBlock::_writeBlockage()
  
{

  // create MDV object
  
  _outMdvx.clear();
  _setMasterHeader(_outMdvx);
  _addBlockageField(_outMdvx);
  _addTerrainField(_outMdvx);
  if (_params.create_hi_res_terrain_grid) {
    _addHiResTerrainField(_outMdvx);
  }
  
  // write it out
  
  _outMdvx.setWriteFormat(Mdvx::FORMAT_NCF);
  string outputDir(_params.output_dir);
  if (_params.append_radar_name_to_output_dir) {
    outputDir += PATH_DELIM;
    outputDir += _params.radar_name;
  }
  
  if (_outMdvx.writeToDir(outputDir.c_str())) {
    cerr << "ERROR - CartBeamBlock::_createTerrainGrid" << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote blockage MDV NetCDF file: " << _outMdvx.getPathInUse() << endl;
  }

  return 0;

}

//////////////////////////////////////////////
// set the master header for blockage file

void CartBeamBlock::_setMasterHeader(Mdvx &mdvx)

{
  
  // set master header
  
  Mdvx::master_header_t mhdr(_templateMhdr);

  DateTime ttime(_params.output_time_stamp.year,
                 _params.output_time_stamp.month,
                 _params.output_time_stamp.day,
                 _params.output_time_stamp.hour,
                 _params.output_time_stamp.min,
                 _params.output_time_stamp.sec);
  
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = ttime.utime();
  mhdr.time_end = ttime.utime();
  mhdr.time_centroid = ttime.utime();
  mhdr.time_expire = mhdr.time_begin + 999999999L;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = Mdvx::DATA_SYNTHESIS;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  
  mdvx.setMasterHeader(mhdr);
  
  mdvx.setDataSetInfo("Beam blockage and DEM data");
  mdvx.setDataSetSource("CartBeamBlock");
  mdvx.setDataSetName(_params.data_set_source);
  
}

//////////////////////////////////////////////////////////////////////
// add the blockage extinction field

void CartBeamBlock::_addBlockageField(Mdvx &mdvx)
  
{
  
  if (_params.debug) {
    cerr << "-->> Adding blockage field: " << _params.blockage_field_name << endl;
  }

  // field header
  
  Mdvx::field_header_t fhdr(_templateFhdr);
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  
  fhdr.bad_data_value = missingFl32;
  fhdr.missing_data_value = missingFl32;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  // vlevel header
  
  Mdvx::vlevel_header_t vhdr(_templateVhdr);
  
  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, _blockage.data());
  fld->convertType(Mdvx::ENCODING_INT16, Mdvx::COMPRESSION_GZIP);
  // set strings
  
  fld->setFieldName("BEAM_E");
  fld->setFieldNameLong("beam_extinction_from_terrain");
  fld->setUnits("");
  
  // add to object
  
  mdvx.addField(fld);

}

////////////////////////////////////////////////////////
// add 2D terrain height fieldd
//

void CartBeamBlock::_addTerrainField(Mdvx &mdvx)
  
{
  
  if (_params.debug) {
    cerr << "-->> Adding 2D terrain ht field: " << _params.terrain_ht_field_name << endl;
  }

  // field header

  Mdvx::field_header_t fhdr(_templateFhdr);
  fhdr.nz = 1;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0;

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;

  fhdr.bad_data_value = missingFl32;
  fhdr.missing_data_value = missingFl32;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // create terrain data

  vector<fl32> height;
  height.resize(fhdr.nx * fhdr.ny, missingFl32);
  int ii = 0;
  for (int iy = 0; iy < fhdr.ny; iy++) {
    for (int ix = 0; ix < fhdr.nx; ix++, ii++) {
      double latDeg, lonDeg;
      _proj.xyIndex2latlon(ix, iy, latDeg, lonDeg);
      fl32 ht = _dem->getTerrainHtM(latDeg, lonDeg);
      height[ii] = ht;
    }
  }

  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, height.data());
  fld->convertType(Mdvx::ENCODING_FLOAT32,
                   Mdvx::COMPRESSION_GZIP);
  // set strings
  
  fld->setFieldName(_params.terrain_ht_field_name);
  fld->setFieldNameLong("Terrain height from DEM data");
  fld->setUnits("m");
  
  // add to object
  
  mdvx.addField(fld);

}

////////////////////////////////////////////////////////
// add high res 2D terrain height fieldd
//

void CartBeamBlock::_addHiResTerrainField(Mdvx &mdvx)
  
{

  if (_params.debug) {
    cerr << "  Adding high resolution terrain field: "
         << _params.hi_res_terrain_ht_field_name << endl;
  }

  // field header
  
  Mdvx::field_header_t fhdr(_templateFhdr);
  fhdr.nz = 1;
  
  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.grid_dz = 1.0;
  fhdr.grid_minz = 0;

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;

  fhdr.bad_data_value = missingFl32;
  fhdr.missing_data_value = missingFl32;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;

  double llx = fhdr.grid_minx - fhdr.grid_dx / 2.0;
  double lly = fhdr.grid_miny - fhdr.grid_dy / 2.0;

  double resRatioX = _params.terrain_grid_hi_res_deg / fhdr.grid_dx;
  double resRatioY = _params.terrain_grid_hi_res_deg / fhdr.grid_dy;

  fhdr.grid_dx = _params.terrain_grid_hi_res_deg;
  fhdr.grid_dy = _params.terrain_grid_hi_res_deg;

  fhdr.grid_minx = llx + fhdr.grid_dx / 2.0;
  fhdr.grid_miny = lly + fhdr.grid_dy / 2.0;
  
  fhdr.nx = std::round(fhdr.nx / resRatioX);
  fhdr.ny = std::round(fhdr.ny / resRatioY);
  
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(fl32);

  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  vhdr.level[0] = 0;
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;

  // create terrain data
  
  vector<fl32> height;
  height.resize(fhdr.nx * fhdr.ny, missingFl32);
  int ii = 0;
  for (int iy = 0; iy < fhdr.ny; iy++) {
    double yy = fhdr.grid_miny + iy * fhdr.grid_dy;
    for (int ix = 0; ix < fhdr.nx; ix++, ii++) {
      double xx = fhdr.grid_minx + ix * fhdr.grid_dx;
      double latDeg, lonDeg;
      _proj.xy2latlon(xx, yy, latDeg, lonDeg);
      fl32 ht = _dem->getTerrainHtM(latDeg, lonDeg);
      height[ii] = ht;
    }
  }
  
  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, height.data());
  fld->convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_GZIP);
  // set strings
  
  fld->setFieldName(_params.hi_res_terrain_ht_field_name);
  fld->setFieldNameLong("High resolution terrain ht data from DEM");
  fld->setUnits("m");
  
  // add to object
  
  mdvx.addField(fld);

}

