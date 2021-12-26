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
// StormShapeSim.cc
//
// StormShapeSim object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2021
//
///////////////////////////////////////////////////////////////
// StormShapeSim simulates storm shapes and writes these to a
// Cartesian MDV file. It then resamples the Cartesian file
// using a prescribed radar scan strategy, and writes the
// radar-bases simulation to a CfRadial file.
///////////////////////////////////////////////////////////////

#include "StormShapeSim.hh"
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/BeamHeight.hh>

#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>

const fl32 StormShapeSim::dbzMiss = -9999.0F;

using namespace std;

// Constructor

StormShapeSim::StormShapeSim(int argc, char **argv) :
        _args("StormShapeSim")

{

  OK = TRUE;
  _dbzCart = NULL;

  // set programe name

  _progName = strdup("StormShapeSim");

  // get command line args
  
  if (_args.parse(argc, (const char **) argv)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    OK = FALSE;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv,
			   _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    OK = FALSE;
    return;
  }

  return;

}

// destructor

StormShapeSim::~StormShapeSim()

{

  if (_dbzCart) {
    delete[] _dbzCart;
  }
  
}

//////////////////////////////////////////////////
// Run

int StormShapeSim::Run()
{

  // create Cartesian DBZ volume with shapes embedded in the data

  _createDbzCart();
  
  // create a volume depending on mode
  
  RadxVol vol;
  if (_params.specify_mode == Params::SPECIFY_RADAR_PARAMS) {
    _createVol(vol);
  } else {
    if (_readFile(_params.specified_file_path, vol)) {
      cerr << "ERROR - cannot read file: " << _params.specified_file_path << endl;
      return -1;
    }
    if (_params.debug) {
      cerr << "==>> Read file: " << _params.specified_file_path << endl;
    }
  }

  // time limits, and vol duration
  
  RadxTime startTime = vol.getStartRadxTime();
  RadxTime endTime = vol.getEndRadxTime();
  double volDurationSecs = endTime - startTime;
  if (_params.debug) {
    cerr << "volStartTime : " << startTime.asString(3) << endl;
    cerr << "volEndTime   : " << endTime.asString(3) << endl;
    cerr << "duration     : " << volDurationSecs << endl;
  }
  

  // add the gate geometry fields

  _addGeomFields(vol);

  // reverse sweep order if requested

  if (_params.reverse_sweep_order_in_vol) {
    vol.reverseSweepOrder();
  }

  // write the file

  if (_writeCfRadialVol(vol)) {
    cerr << "ERROR - StormShapeSim::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }
  
  return 0;

}

//////////////////////////////////////////////////
// create dbz cartesian volume

void StormShapeSim::_createDbzCart()
{

  const Params::cart_grid_t &cgrid = _params.cart_grid;
  
  // create grid and initialize
  
  _nptsCart = (cgrid.nz * cgrid.ny * cgrid.nx);
  _dbzCart = new fl32[_nptsCart];

  for (size_t ii = 0; ii < _nptsCart; ii++) {
    _dbzCart[ii] = dbzMiss;
  }

  // loop through the points

  size_t index = 0;
  for (int iz = 0; iz < cgrid.nz; iz++) {
    double zz = cgrid.minz + iz * cgrid.dz;
    for (int iy = 0; iy < cgrid.nz; iy++) {
      double yy = cgrid.miny + iy * cgrid.dy;
      for (int ix = 0; ix < cgrid.nz; ix++, index++) {
        double xx = cgrid.minx + ix * cgrid.dx;

        // loop through the shapes

        for (int ishape = 0; ishape < _params.storm_shapes_n; ishape++) {

          const Params::storm_shape_t &ss = _params._storm_shapes[ishape];

          // compute dbz in shape body
          
          double bodyDbz = ss.body_dbz_at_base;
          if (zz > ss.body_max_z_km) {
            bodyDbz = ss.body_dbz_at_base;
          } else if (zz >= ss.body_min_z_km && zz <= ss.body_max_z_km) {
            double zFrac = ((zz - ss.body_min_z_km) /
                            (ss.body_max_z_km - ss.body_min_z_km));
            bodyDbz += zFrac * (ss.body_dbz_at_top - ss.body_dbz_at_base);
          }

          // compute dbz at the centroid, at the grid z
          
          double centroidDbz = bodyDbz;
          if (zz < ss.body_min_z_km) {
            centroidDbz -= (ss.body_min_z_km - zz) * ss.dbz_gradient_vert;
          } else if (zz > ss.body_max_z_km) {
            centroidDbz -= (zz - ss.body_max_z_km) * ss.dbz_gradient_vert;
          }

          // compute the dbz at the grid point

          double gridDbz = centroidDbz;
          
          double xOff = ss.centroid_x_km - xx;
          double yOff = ss.centroid_y_km - yy;
          double aa = ss.body_ellipse_radius_x_km;
          double bb = ss.body_ellipse_radius_y_km;
          double cc = (aa + bb) / 1.0;
          double normDist =
            sqrt((xOff * xOff / aa * aa) + (yOff * yOff / bb * bb));
          
          if (normDist > 1.0) {
            gridDbz -= ss.dbz_gradient_horiz * (normDist - 1.0) * cc;
          }

          if (gridDbz > _params.min_valid_dbz) {
            _dbzCart[index] = gridDbz;
          }
          
        } // ishape
        
      } // ix
    } // iy
  } // iz

}

//////////////////////////////////////////////////
// create a volume from specified params

void StormShapeSim::_createVol(RadxVol &vol)
{
  
  vol.clear();
  RadxTime startTime(RadxTime::NOW);
  double timeSinceStart = 0.0;
  double startRangeKm = _params.gate_spacing_m / 2000.0;
  double gateSpacingKm = _params.gate_spacing_m / 1000.0;
  double maxRangeKm = _params.max_range_km;
  int nGates = (int) (maxRangeKm / gateSpacingKm + 0.5);
  
  // loop through specified sweeps

  double prevEl = _params._sweeps[0].elev_deg;
  
  for (int isweep = 0; isweep < _params.sweeps_n; isweep++) {
    
    double el = _params._sweeps[isweep].elev_deg;

    // move to elevation angle

    double deltaEl = fabs(el - prevEl);
    double slewTime = deltaEl / _params.elev_rate_deg_per_sec;
    timeSinceStart += slewTime;
    prevEl = el;
    
    // azimuths every degree
    
    for (int iray = 0; iray < 360; iray++) {
      
      double deltaTime = 1.0 / _params._sweeps[isweep].az_rate_deg_per_sec;
      timeSinceStart += deltaTime;
      
      double az = iray;
      
      // create ray
      
      RadxRay *ray = new RadxRay;
      
      ray->setSweepNumber(isweep);
      ray->setSweepMode(Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE);
      ray->setVolumeNumber(0);

      ray->setTime(startTime + timeSinceStart);
      ray->setAzimuthDeg(az);
      ray->setElevationDeg(el);
      ray->setFixedAngleDeg(el);
      ray->setIsIndexed(true);
      ray->setAngleResDeg(1.0);
      ray->setNSamples(1);
      ray->setRangeGeom(startRangeKm, gateSpacingKm);
      ray->setNGates(nGates);

      // add ray to vol - vol will free it later
      
      vol.addRay(ray);
      
    } // iray
    
  } // isweep

  // load up vol metadata from rays
  
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();

  // set vol metadata
  
  vol.setTitle("StormShapeSim");
  vol.setInstitution("NCAR/EOL");
  vol.setSource("Written by StormShapeSim");
  vol.setComment("Stores geometry and time-based fields");

  vol.setInstrumentName(_params.radar_name);
  vol.setSiteName(_params.radar_name);
  vol.setInstrumentType(Radx::INSTRUMENT_TYPE_RADAR);
  vol.setPlatformType(Radx::PLATFORM_TYPE_FIXED);
  vol.setPrimaryAxis(Radx::PRIMARY_AXIS_Z);
  vol.setLatitudeDeg(_params.radar_location.latitudeDeg);
  vol.setLongitudeDeg(_params.radar_location.longitudeDeg);
  vol.setAltitudeKm(_params.radar_location.altitudeKm);
  vol.setWavelengthCm(_params.radar_wavelength_cm);
  vol.setRadarBeamWidthDegH(_params.beam_width_deg);
  vol.setRadarBeamWidthDegV(_params.beam_width_deg);

}

//////////////////////////////////////////////////
// Read in a specified file
// Returns 0 on success
//         -1 on failure

int StormShapeSim::_readFile(const string &readPath,
                                RadxVol &vol)
{
  
  // clear all data on volume object

  vol.clear();
  
  if (_params.debug) {
    cerr << "INFO - StormShapeSim::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setDebug(true);
  }
  inFile.setReadMaxRangeKm(_params.max_range_km);
  inFile.setReadPreserveSweeps(true);

  // read in file

  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - StormShapeSim::_readFile" << endl;
    cerr << "  path: " << readPath << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ==>> read in file: " << readPath << endl;
  }

  return 0;

}

//////////////////////////////////////////////////
// add geometry fields

void StormShapeSim::_addGeomFields(RadxVol &vol)
{

  
  RadxTime startTime = vol.getStartRadxTime();

  double beamWidthDeg = _params.beam_width_deg;
  double beamWidthDegH = vol.getRadarBeamWidthDegH();
  double beamWidthDegV = vol.getRadarBeamWidthDegV();
  if (beamWidthDegH > 0 && beamWidthDegV > 0) {
    beamWidthDeg = (beamWidthDegH + beamWidthDegV) / 2.0;
  } else if (beamWidthDegH > 0) {
    beamWidthDeg = beamWidthDegH;
  } else if (beamWidthDegV > 0) {
    beamWidthDeg = beamWidthDegV;
  }
  double beamWidthRad = beamWidthDeg * DEG_TO_RAD;

  BeamHeight beamHt; // default init to get height above radar

  // loop through rays

  for (size_t iray = 0; iray < vol.getNRays(); iray++) {
    
    RadxRay *ray = vol.getRays()[iray];
    double el = ray->getElevationDeg();
    int nGates = ray->getNGates();
    double startRangeKm = ray->getStartRangeKm();
    double gateSpacingKm = ray->getGateSpacingKm();

    // sample volume in km3
    
    RadxField *sampleVol = new RadxField("sampleVol", "km3");
    sampleVol->setStandardName("radar_sample_volume");
    sampleVol->setLongName("radar_sample_volume_per_gate");
    sampleVol->setMissingFl32(-9999.0);
    sampleVol->copyRangeGeom(*ray);
    Radx::fl32 *volData = new Radx::fl32[nGates];
    for (int ii = 0; ii < nGates; ii++) {
      double rangeKm = startRangeKm + ii * gateSpacingKm;
      double widthKm = rangeKm * beamWidthRad;
      double areaKm2 = (M_PI * widthKm * widthKm) / 4.0;
      double volKm3 = areaKm2 * gateSpacingKm;
      volData[ii] = volKm3;
    }
    sampleVol->addDataFl32(nGates, volData);
    ray->addField(sampleVol);
    
    // add height in km
    
    RadxField *height = new RadxField("height", "km");
    height->setStandardName("beam_height_above_radar");
    height->setLongName("beam_height_above_radar");
    height->setMissingFl32(-9999.0);
    height->copyRangeGeom(*ray);
    Radx::fl32 *htData = new Radx::fl32[nGates];
    for (int ii = 0; ii < nGates; ii++) {
      double rangeKm = startRangeKm + ii * gateSpacingKm;
      double beamHtKm = beamHt.computeHtKm(el, rangeKm);
      htData[ii] = beamHtKm;
    }
    height->addDataFl32(nGates, htData);
    ray->addField(height);
    
  } // iray
  
  // update metadata from rays
  
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();

}

//////////////////////////////////////////////////
// set up write for polar volume

void StormShapeSim::_setupCfRadialWrite(RadxFile &file)
{

  if (_params.debug) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setVerbose(true);
  }

  file.setWriteFileNameMode(RadxFile::FILENAME_WITH_START_AND_END_TIMES);
  file.setWriteCompressed(true);
  file.setCompressionLevel(4);
  file.setFileFormat(RadxFile::FILE_FORMAT_CFRADIAL);
  file.setNcFormat(RadxFile::NETCDF4);
  
}

//////////////////////////////////////////////////
// write out the cfradial volume

int StormShapeSim::_writeCfRadialVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupCfRadialWrite(outFile);
  
  // write to dir
  
  if (outFile.writeToDir(vol, _params.output_dir_cfradial, true, false)) {
    cerr << "ERROR - StormShapeSim::_writeCfRadialVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir_cfradial << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }
  string outputPath = outFile.getPathInUse();

  return 0;

}

