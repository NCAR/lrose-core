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
#include <filesystem>
#include <iostream>
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
        _dem(_params)
  
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

}

// destructor

CartBeamBlock::~CartBeamBlock()

{


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

  _origin.set(angle(_radarLat, true), angle(_radarLon, true), _radarHtKm);
  
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
    cerr << "      DEM model: " << _dem.ModelName(_params.dem_data_format) << endl;
  }
    
  // compute safe projection lat/lon limits, with a margin of 5 grid points
  
  double minLat, minLon, maxLat, maxLon;
  int margin = 5;
  _proj.getEdgeExtrema(minLat, minLon, maxLat, maxLon, margin);

  if (_params.debug) {
    cerr << "INFO: retrieving DEM data for lat/lon bounding box" << endl;
    cerr << "      minLat, maxLat: " << minLat << ", " << maxLat << endl;
    cerr << "      minLon, maxLon: " << minLon << ", " << maxLon << endl;
  }
    
  // initialize dem for reading

  std::pair<double,double> sw(minLat, minLon);
  std::pair<double,double> ne(maxLat, maxLon);
  _dem.set(sw, ne);

  // create the terrain grid
  
  if (_createTerrainGrid(minLat, minLon, maxLat, maxLon)) {
    cerr << "ERROR - CartBeamBlock::_readDem" << endl;
    cerr << "  Cannot create terrain output grid file" << endl;
    return -1;
  }
  
  return 0;

}

///////////////////////////////////////////////////////////////////////////
// compute beam blockage for each point in the Cartesian grid

int CartBeamBlock::_computeBlockage()
  
{

  // initialize BeamHeight computations
  
  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_sensorHtKm);
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }

  // setup beam power model

  angle height, width;
  height.set_degrees(_vertBeamWidthDeg);
  width.set_degrees(_horizBeamWidthDeg);
  beam_power powerModel(width, height);

  // set up beam cross section

  angle azWidth(_horizBeamWidthDeg * 2.0, true);
  angle beamWidthH(_horizBeamWidthDeg, true);
  angle beamWidthV(_vertBeamWidthDeg, true);

  beam_power_cross_section csec
    (powerModel, azWidth,
     static_cast<size_t>(_params.num_elev_subsample),
     static_cast<size_t>(_params.num_range_subsample),
     beamWidthV, beamWidthH);
  csec.make_vertical_integration();

  // compute radar x and y coords in km
  
  double radarX, radarY;
  _proj.latlon2xy(_sensorLat, _sensorLon, radarX, radarY);
  
  const Mdvx::coord_t &coord = _proj.getCoord();

  // array for extinction

  size_t nPtsPlane = _templateFhdr.nx * _templateFhdr.ny;
  vector<fl32> extinct;
  extinct.resize(nPtsPlane * _templateFhdr.nz, missingFl32);

  // loop through the XY grid
  
  for (int iy = 0; iy < coord.ny; iy++) {
    for (int ix = 0; ix < coord.nx; ix++) {

      // get lat/lon of grid point
      
      double lat, lon;
      _proj.xyIndex2latlon(ix, iy, lat, lon);

      // get terrain height
      
      angle alat, alon;
      alat.set_degrees(lat);
      alon.set_degrees(lon);
      latlon loc(alat, alon);
      fl32 terrainHt = _dem.getElevation(loc);
      if (!std::isfinite(terrainHt)) {
        continue;
      }
        
      // compute range and azimuth from radar
      
      double gndRangeKm, azDeg;
      PJGLatLon2RTheta(_radarLat, _radarLon, lat, lon, &gndRangeKm, &azDeg);
      
      // double xx, yy;
      // _proj.latlon2xy(lat, lon, xx, yy);
      // double azDeg = atan2(xx, yy) * RAD_TO_DEG;
      // double deltaX = xx - radarX;
      // double deltaY = yy - radarY;
      // double gndRangeKm = sqrt(deltaX * deltaX + deltaY * deltaY);
      
      // loop through the planes
      
      size_t index = iy * _templateFhdr.nx + ix;
      for (int iz = 0; iz < coord.nz; iz++, index += nPtsPlane) {
        
        double zKm = _templateVhdr.level[iz];
        double elDeg = beamHt.computeElevationDeg(zKm, gndRangeKm);
        
        double ext = _computeExtinction(elDeg, azDeg,
                                        zKm, gndRangeKm,
                                        beamHt, powerModel, csec);
        extinct[index] = ext;
        
        if (ext == 0.0) {
          // no blockage at this elevation, so none above either
          break;
        }
        
      } // iz
        
    } // ix
  } // iy
  
  return 0;

}

///////////////////////////////////////////////////////////
// compute extinction for a given range and elevation

double CartBeamBlock::_computeExtinction(double elDeg,
                                         double azDeg,
                                         double zKm,
                                         double gndRangeKm,
                                         const BeamHeight &beamHt,
                                         const beam_power &powerModel,
                                         const beam_power_cross_section &csec)
  
{

  // initialize
  
  double extinction = 0.0;
  angle azAngle(azDeg, true);
  angle elAngle(elDeg, true);
  double dRangeM = _params.blockage_range_resolution_m;
  
  // we compute the beam blockage at intervals along the ray
  
  double rangeM = 0.0;
  while (rangeM <= gndRangeKm * 1000.0) {
    
    // compute lat/lon
    
    double lat, lon;
    PJGLatLonPlusRTheta(_radarLat, _radarLon, rangeM / 1000.0, azDeg, &lat, &lon);
    
    // get terrain ht

    rainfields::angle alat, alon;
    alat.set_degrees(lat);
    alon.set_degrees(lon);
    rainfields::latlon loc(alat, alon);
    fl32 terrainHt = _dem.getElevation(loc);

    if (std::isfinite(terrainHt)) {

      // compute beam ht
      
      double slantRngKm = (rangeM / 1000.0) / cos(elDeg * DEG_TO_RAD);
      double htKm = beamHt.computeHtKm(elDeg, slantRngKm);

      // compute blockage
      
      // accumulate extinction
      
      extinction += htKm;
      
      // get maximum height of DEM along ray in segment bounded by this bin

      real peak_ground_range = 0.0_r;
      real peak_altitude;
      real progressive_loss = 0.0_r;
      _dem.determine_dem_segment_peak(_origin, azAngle, rangeM,
                                      rangeM + dRangeM,
                                      peak_ground_range, peak_altitude,
                                      _params.num_range_subsample);

    }

    // increment
    
    rangeM += dRangeM;
    
  } // while (gndRngKm <= gndRangeKm) 
  
  return extinction;
  
}

#ifdef NOTNOW

//------------------------------------------------------------------
void CartBeamBlock::_processBeam(RayHandler &ray, latlonalt origin, 
				 const beam_propagation &bProp,
				 const beam_power_cross_section &csec,
                                 bool &foundBlockage)
  
{
  angle azAngle = ray.azimuth();
  angle elevAngle = ray.elev();

  LOGF(LogMsg::DEBUG, "  processing beam, el, az, nSoFar: %7.2f, %7.2f, %d", 
       ray.elevDegrees(), ray.azDegrees(), _nGatesBlocked);

  // subsample each azimuth based on the number of horizontal cells in our
  // cross section
  for (size_t iray = 0; iray < csec.cols(); ++iray) {

    const angle bearing = azAngle + csec.offset_azimuth(iray);
    angle max_ray_theta = -90.0_deg;  // updated as we go
    
    // walk out along our ray, keeping track of the total power loss at each bin
    for (auto & gate : ray) {
      _processGate(gate, elevAngle, iray, origin, bProp, bearing, csec,
		   max_ray_theta);
      if (gate.getBeamL() > 0) {
        foundBlockage = true;
      }
    }

  } // iray

}

//------------------------------------------------------------------
void CartBeamBlock::_processGate(GateHandler &gate, angle elevAngle,
				 size_t iray, latlonalt origin, 
				 const beam_propagation &bProp, angle bearing,
				 const beam_power_cross_section &csec,
				 angle &max_ray_theta)
{

  if (gate.getBeamL() > 0) {
    _nGatesBlocked++;
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      LOGF(LogMsg::DEBUG,
           "elev, bearing, range, blocakge: %g, %g, %g, %g", 
           elevAngle, bearing, gate.meters(), gate.getBeamL());
    }
  }
  
  double gateMeters = gate.meters();

  // get maximum height of DEM along ray in segment bounded by this bin
  real peak_ground_range = 0.0_r;
  real peak_altitude;
  real progressive_loss = 0.0_r;
  _dem.determine_dem_segment_peak(origin, bearing, gateMeters,
				  gateMeters + _params.gates.delta*1000.0,
				  peak_ground_range, peak_altitude,
				  _params.num_range_subsample);

  // if DEM gave us no valid values we can fail this condition
  // this is usually due to sea regions not being included in the DEM
  if (peak_ground_range > 0.0_r) {
    _adjustValues(iray, bProp, peak_ground_range, peak_altitude, elevAngle,
		  csec, max_ray_theta, progressive_loss);

    // update the highest peak observed for the bin
    gate.adjustPeak(peak_altitude);
  }

  // add the loss in this ray,bin to the gate,bin total
  gate.incrementLoss(progressive_loss);

}

#endif

//////////////////////////////////////////
// create a terrain grid in Cart coords

int CartBeamBlock::_createTerrainGrid(double minLat, double minLon,
                                      double maxLat, double maxLon)
  
{

  // compute grid details

  minLat = (floor(minLat / _params.cart_terrain_grid_res) - 1.0) *
    _params.cart_terrain_grid_res;
  minLon = (floor(minLon / _params.cart_terrain_grid_res) - 1.0) *
    _params.cart_terrain_grid_res;

  maxLat = (floor(maxLat / _params.cart_terrain_grid_res) + 1.0) *
    _params.cart_terrain_grid_res;
  maxLon = (floor(maxLon / _params.cart_terrain_grid_res) + 1.0) *
    _params.cart_terrain_grid_res;

  if (_params.debug) {
    cerr << "createTerrainGrid():" << endl;
    cerr << "  minLat, minLon: " << minLat << ", " << minLon << endl;
    cerr << "  maxLat, maxLon: " << maxLat << ", " << maxLon << endl;
  }

  // create MDV object

  _outMdvx.clear();
  _setTerrainMdvMasterHeader(_outMdvx);
  if (_addTerrainMdvField(_outMdvx, minLat, minLon, maxLat, maxLon)) {
    return -1;
  }

  // write it out

  _outMdvx.setWriteFormat(Mdvx::FORMAT_NCF);

  string outputDir(_params.output_dir);
  if (_params.append_radar_name_to_output_dir) {
    outputDir += PATH_DELIM;
    outputDir += _params.radar_name;
  }
  outputDir += PATH_DELIM;
  outputDir += _params.cart_terrain_grid_subdir;

  if (_outMdvx.writeToDir(outputDir.c_str())) {
    cerr << "ERROR - CartBeamBlock::_createTerrainGrid" << endl;
    cerr << _outMdvx.getErrStr() << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "Wrote terrain NetCDF file: " << _outMdvx.getPathInUse() << endl;
  }

  return 0;

}
//////////////////////////////////////////////
// set the master header for terrain file

void CartBeamBlock::_setTerrainMdvMasterHeader(Mdvx &mdv)

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
  // mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  // mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  
  mdv.setMasterHeader(mhdr);
  
  mdv.setDataSetInfo("Terrain data from SRTM30");
  mdv.setDataSetName("SRTM30");
  mdv.setDataSetSource("CartBeamBlock");
  
}

////////////////////
// addTerrainField()
//

int CartBeamBlock::_addTerrainMdvField(Mdvx &mdv,
                                       double minLat, double minLon,
                                       double maxLat, double maxLon)
  
{
  
  if (_params.debug) {
    cerr << "  Adding terrain field: " << _params.cart_terrain_field_name << endl;
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
      rainfields::angle alat, alon;
      alat.set_degrees(latDeg);
      alon.set_degrees(lonDeg);
      rainfields::latlon loc(alat, alon);
      fl32 ht = _dem.getElevation(loc);
      if (!std::isfinite(ht)) {
        ht = missingFl32;
      }
      height[ii] = ht;
    }
  }

  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, height.data());
  fld->convertType(Mdvx::ENCODING_FLOAT32,
                   Mdvx::COMPRESSION_GZIP);
  // set strings
  
  fld->setFieldName(_params.cart_terrain_field_name);
  fld->setFieldNameLong(_params.cart_terrain_field_name);
  fld->setUnits("m");
  
  // add to object
  
  mdv.addField(fld);

  return 0;
  
}

#ifdef JUNK

//------------------------------------------------------------------
CartBeamBlock::CartBeamBlock(int argc, char **argv) :

{

  // initialize checks for still blocked

  _nGatesBlocked = 0;
  
  for (int i=0; i<_params.nazimuth(); ++i) {
    _azBlocked.push_back(true);
  }

}

//------------------------------------------------------------------
CartBeamBlock::~CartBeamBlock()
{
}

//------------------------------------------------------------------
int CartBeamBlock::Run(void)
{

  // pull out extremes in latitude/longitude, which are needed for setup
  // in some cases
  pair<double, double> sw, ne;
  _params.latlonExtrema(sw, ne);

  if (!ta_stat_is_dir(_params.input_dem_path)) {
    LOGF(LogMsg::ERROR, "DEM dir does not exist: %s", _params.input_dem_path);
    exit(1);
  }
  
  // set up the digital elevation object
  if (!_dem.set(sw, ne))
  {
    return 1;
  }

  // create latlon terrain grid if requested

  if (_params.create_cart_terrain_grid) {
    _createCartTerrainGrid(sw.first, sw.second, ne.first, ne.second);
  }
  
  angle lat(_params.radar_location.latitudeDeg, true);
  angle lon(_params.radar_location.longitudeDeg, true);

  double elevation;
  if (_params.do_lookup_radar_altitude) {
    latlon ll(lat, lon);
    elevation = _dem.getElevation(ll);
  } else {
    elevation = _params.radar_location.altitudeKm*1000.0;
  }
  latlonalt origin(lat, lon, elevation);

  // convert the site location of the volume into the native spheroid of the DEM
  origin = _dem.radarOrigin(origin); 

  // setup our beam power models
  angle height, width;
  height.set_degrees(_params.vert_beam_width_deg);
  width.set_degrees(_params.horiz_beam_width_deg);
  beam_power power_model(width, height);

  // process each scan
  size_t nScans = 0;
  for (auto &scan : _vol) {
    bool done = _processScan(scan, power_model, origin);
    nScans++;
    if (done) {
      break;
    }
  }

  if (nScans < _vol.nScans()) {
    cerr << "SUCCESS - completed early, nscans used: " << nScans << endl;
  } else {
    cerr << "WARNING - terrain still visible at top scan, elev: "
         << _vol.getElev(nScans - 1) << endl;
  }

  _vol.finish(nScans);
  return 0;

}

//------------------------------------------------------------------
bool CartBeamBlock::_processScan(ScanHandler &scan,
				 const beam_power &power_model,
				 latlonalt origin)
{

  angle elevAngle = scan.elevation();
  LOGF(LogMsg::DEBUG, "  occluding tilt: %lf", scan.elevDegrees());

  beam_propagation bProp(elevAngle, _params.radar_location.altitudeKm*1000.0);
  angle beam_width_v, beam_width_h, angle_width;
  beam_width_v.set_degrees(3*_params.vert_beam_width_deg);
  beam_width_h.set_degrees(3*_params.horiz_beam_width_deg +
			   _params.azimuths.delta);
  angle_width.set_degrees(_params.azimuths.delta);
  beam_power_cross_section csec
    {
      power_model, angle_width,
      static_cast<size_t>(_params.num_elev_subsample),
      static_cast<size_t>(_params.num_range_subsample),
      beam_width_v, beam_width_h
    };
  csec.make_vertical_integration();
  
  bool foundBlockage = false;
  for (size_t iray = 0; iray < scan.nRays(); iray++) {

    if (!_azBlocked[iray]) {
      // no remaining blockage on this azimuth
      continue;
    }

    RayHandler &ray = scan.getRay(iray);
    bool thisAzBlocked = false;
    _processBeam(ray, origin, bProp, csec, thisAzBlocked);
    if (thisAzBlocked) {
      foundBlockage = true;
    } else {
      _azBlocked[iray] = false;
    }

  } // iray

  if (foundBlockage) {
    // not done
    return false;
  } else {
    // done
    return true;
  }
  
}


//------------------------------------------------------------------
void CartBeamBlock::_adjustValues(size_t ray, const beam_propagation &bProp,
				  real peak_ground_range, real peak_altitude,
				  angle elevAngle,
				  const beam_power_cross_section &csec,
				  angle &max_ray_theta, real &progressive_loss)
{
  // convert geometric height into an elevation above radar horizon
  angle max_bin_theta = 
    bProp.required_elevation_angle(peak_ground_range, peak_altitude) -
    elevAngle;
	      
  // TODO apply part of fractional loss to this bin based on distance through
  //      bin where peak was found?

  // if we've got a new highest obstruction, then we need to lookup the new 
  // progressive fractional loss for this ray from the cross-section model
  if (max_bin_theta > max_ray_theta) {
    max_ray_theta = max_bin_theta;
              
    // what is the closest vertical bin
    const angle csec_delta = csec.height() / csec.rows();
    const real csec_y_offset = csec.rows() / 2.0_r;
    int y = std::floor(max_ray_theta / csec_delta + csec_y_offset);
    if (y >= static_cast<int>(csec.rows())) {
      progressive_loss = csec.power(csec.rows() - 1, ray);
    } else if (y >= 0) {
      progressive_loss = csec.power(y, ray);
    } else {
      ; // y < 0 - no loss - do nothing
    }
  }
}

//------------------------------------------------------------------
int CartBeamBlock::Write(void)
{
  return _vol.write();
}


#endif

