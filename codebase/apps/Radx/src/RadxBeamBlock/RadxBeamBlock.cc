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
/**
 * @file RadxBeamBlock.cc
 */

#include "RadxBeamBlock.hh"
#include <euclid/Grid2d.hh>
#include <euclid/GridAlgs.hh>
#include <toolsa/LogMsg.hh>
#include <toolsa/file_io.h>
#include <toolsa/mem.h>
#include <Mdv/MdvxField.hh>

using namespace rainfields;
using namespace ancilla;

//------------------------------------------------------------------
RadxBeamBlock::RadxBeamBlock(const Parms &params) : _params(params),
						    _vol(params),
						    _dem(params)
{

  // initialize checks for still blocked

  _nGatesBlocked = 0;
  
  for (int i=0; i<_params.nazimuth(); ++i) {
    _azBlocked.push_back(true);
  }

}

//------------------------------------------------------------------
RadxBeamBlock::~RadxBeamBlock()
{
}

//------------------------------------------------------------------
int RadxBeamBlock::Run(void)
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
bool RadxBeamBlock::_processScan(ScanHandler &scan,
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
  beam_power_cross_section csec{power_model, angle_width,
      static_cast<size_t>(_params.num_elev_subsample),
      static_cast<size_t>(_params.num_range_subsample),
      beam_width_v, beam_width_h};
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
void RadxBeamBlock::_processBeam(RayHandler &ray, latlonalt origin, 
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
void RadxBeamBlock::_processGate(GateHandler &gate, angle elevAngle,
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

//------------------------------------------------------------------
void RadxBeamBlock::_adjustValues(size_t ray, const beam_propagation &bProp,
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
int RadxBeamBlock::Write(void)
{
  return _vol.write();
}


//////////////////////////////////////////
// create a terrain grid in Cart corrds
int RadxBeamBlock::_createCartTerrainGrid(double minLat, double minLon,
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
    cerr << "createCartTerrainGrid():" << endl;
    cerr << "  minLat, minLon: " << minLat << ", " << minLon << endl;
    cerr << "  maxLat, maxLon: " << maxLat << ", " << maxLon << endl;
  }

  // create MDV object

  DsMdvx mdv;
  _setTerrainMdvMasterHeader(mdv);
  _addTerrainMdvField(mdv, minLat, minLon, maxLat, maxLon);

  // write it out

  mdv.setWriteFormat(Mdvx::FORMAT_NCF);

  string outputDir(_params.output_dir);
  if (_params.append_radar_name_to_output_dir) {
    outputDir += PATH_DELIM;
    outputDir += _params.radar_name;
  }
  outputDir += PATH_DELIM;
  outputDir += _params.cart_terrain_grid_subdir;

  if (mdv.writeToDir(outputDir.c_str())) {
    cerr << "ERROR - RadxBeamBlock::_createCartTerrainGrid" << endl;
    cerr << mdv.getErrStr() << endl;
  }

  if (_params.debug) {
    cerr << "Wrote terrain NetCDF file: " << mdv.getPathInUse() << endl;
  }

  return 0;

}
//////////////////////////////////////////////
// set the master header for terrain file

void RadxBeamBlock::_setTerrainMdvMasterHeader(DsMdvx &mdv)

{
  
  // set master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  RadxTime ttime(_params.output_time_stamp.year,
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
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;

  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  
  mdv.setMasterHeader(mhdr);
  
  mdv.setDataSetInfo("Terrain height from SRTM30");
  mdv.setDataSetName("SRTM30");
  mdv.setDataSetSource("RadxBeamBlock");
 
}

////////////////////
// addTerrainField()
//

void RadxBeamBlock::_addTerrainMdvField(DsMdvx &mdv,
                                        double minLat, double minLon,
                                        double maxLat, double maxLon)
  
{
  
  if (_params.debug) {
    cerr << "  Adding terrain field: " << _params.cart_terrain_field_name << endl;
  }

  int nLat = (int) floor((maxLat - minLat) / _params.cart_terrain_grid_res + 0.5) + 1;
  int nLon = (int) floor((maxLon - minLon) / _params.cart_terrain_grid_res + 0.5) + 1;

  // field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);

  fhdr.nx = nLon;
  fhdr.ny = nLat;
  fhdr.nz = 1;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.dz_constant = true;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = fhdr.nx * fhdr.ny * 1 * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.proj_origin_lat = minLat;
  fhdr.proj_origin_lon = minLon;

  fhdr.grid_dx = _params.cart_terrain_grid_res;
  fhdr.grid_dy = _params.cart_terrain_grid_res;
  fhdr.grid_dz = 1.0;

  fhdr.grid_minx = minLon;
  fhdr.grid_miny = minLat;
  fhdr.grid_minz = 0;

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;

  fl32 missingVal = -9999.0;
  fhdr.bad_data_value = missingVal;
  fhdr.missing_data_value = missingVal;
  
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

  fl32 *height = new fl32[fhdr.nx * fhdr.ny];
  int ii = 0;
  for (int ilat = 0; ilat < nLat; ilat++) {
    double latDeg = minLat + ilat * _params.cart_terrain_grid_res;
    for (int ilon = 0; ilon < nLon; ilon++, ii++) {
      double lonDeg = minLon + ilon * _params.cart_terrain_grid_res;
      angle alat, alon;
      alat.set_degrees(latDeg);
      alon.set_degrees(lonDeg);
      latlon loc(alat, alon);
      double ht = _dem.getElevation(loc);
      if (!std::isfinite(ht)) {
        ht = missingVal;
      }
      height[ii] = ht;
    }
  }

  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, height);
  fld->convertType(Mdvx::ENCODING_FLOAT32,
                   Mdvx::COMPRESSION_GZIP);
  delete[] height;

  // set strings
  
  fld->setFieldName(_params.cart_terrain_field_name);
  fld->setFieldNameLong(_params.cart_terrain_field_name);
  fld->setUnits("m");
  
  // add to object
  
  mdv.addField(fld);

}

