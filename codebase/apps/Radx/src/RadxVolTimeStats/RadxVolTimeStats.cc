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
// RadxVolTimeStats.cc
//
// RadxVolTimeStats object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2014
//
///////////////////////////////////////////////////////////////
//
// RadxVolTimeStats creates a RadxVol object from scratch.
//
///////////////////////////////////////////////////////////////

#include "RadxVolTimeStats.hh"
#include <string>
#include <iostream>
#include <cmath>
#include <toolsa/uusleep.h>
#include <toolsa/DateTime.hh>
#include <toolsa/toolsa_macros.h>
#include <radar/BeamHeight.hh>

#include <Radx/RadxVol.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>

using namespace std;

// Constructor

RadxVolTimeStats::RadxVolTimeStats(int argc, char **argv) :
  _args("RadxVolTimeStats")

{

  OK = TRUE;

  // set programe name

  _progName = strdup("RadxVolTimeStats");

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

RadxVolTimeStats::~RadxVolTimeStats()

{

  
}

//////////////////////////////////////////////////
// Run

int RadxVolTimeStats::Run()
{

  
  // create a volume

  RadxVol vol;
  RadxTime startTime(RadxTime::NOW);
  double timeSinceStart = 0.0;
  double startRangeKm = _params.gate_spacing_m / 2000.0;
  double gateSpacingKm = _params.gate_spacing_m / 1000.0;
  int nGates = (int) (_params.max_range_km / gateSpacingKm + 0.5);
  cerr << "111111111111111111111 nGates: " << nGates << endl;
  double beamWidthRad = _params.beam_width_deg * DEG_TO_RAD;
  BeamHeight beamHt;

  // loop through sweeps
  
  for (int isweep = 0; isweep < _params.sweeps_n; isweep++) {
    
    double el = _params._sweeps[isweep].elev_deg;

    // loop through azimuths
    
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

      // add ray to vol - vol will free it later
      
      vol.addRay(ray);

      // add the fields

      // range in km

      {
        RadxField *range = new RadxField("gateRange", "km");
        range->setStandardName("range_to_center_of_gate");
        range->setLongName("range_to_center_of_gate");
        range->setMissingFl32(-9999.0);
        range->setRangeGeom(startRangeKm, gateSpacingKm);
        Radx::fl32 *data = new Radx::fl32[nGates];
        for (int ii = 0; ii < nGates; ii++) {
          data[ii] = startRangeKm + ii * gateSpacingKm;
        }
        range->addDataFl32(nGates, data);
        ray->addField(range);
      }
      
      // sample volume in km3

      {
        RadxField *sampleVol = new RadxField("sampleVol", "km3");
        sampleVol->setStandardName("radar_sample_volume");
        sampleVol->setLongName("radar_sample_volume_per_gate");
        sampleVol->setMissingFl32(-9999.0);
        sampleVol->setRangeGeom(startRangeKm, gateSpacingKm);
        Radx::fl32 *data = new Radx::fl32[nGates];
        for (int ii = 0; ii < nGates; ii++) {
          double rangeKm = startRangeKm + ii * gateSpacingKm;
          double widthKm = rangeKm * beamWidthRad;
          double areaKm2 = (M_PI * widthKm * widthKm) / 4.0;
          double volKm3 = areaKm2 * gateSpacingKm;
          data[ii] = volKm3;
        }
        sampleVol->addDataFl32(nGates, data);
        ray->addField(sampleVol);
      }

      // height in km

      {
        RadxField *height = new RadxField("height", "km");
        height->setStandardName("beam_height_above_radar");
        height->setLongName("beam_height_above_radar");
        height->setMissingFl32(-9999.0);
        height->setRangeGeom(startRangeKm, gateSpacingKm);
        Radx::fl32 *data = new Radx::fl32[nGates];
        for (int ii = 0; ii < nGates; ii++) {
          double rangeKm = startRangeKm + ii * gateSpacingKm;
          double beamHtKm = beamHt.computeHtKm(el, rangeKm);
          data[ii] = beamHtKm;
        }
        height->addDataFl32(nGates, data);
        ray->addField(height);
      }

      // age in secs since start

      {
        RadxField *age = new RadxField("age", "secs");
        age->setStandardName("age_since_start_of_vol");
        age->setLongName("age_since_start_of_vol");
        age->setMissingFl32(-9999.0);
        age->setRangeGeom(startRangeKm, gateSpacingKm);
        Radx::fl32 *data = new Radx::fl32[nGates];
        for (int ii = 0; ii < nGates; ii++) {
          data[ii] = timeSinceStart;
        }
        age->addDataFl32(nGates, data);
        ray->addField(age);
      }
      
      
    } // iray
    
  } // isweep

  // load up vol metadata from rays
  
  vol.loadVolumeInfoFromRays();
  vol.loadSweepInfoFromRays();

  // set vol metadata
  
  vol.setTitle("RadxVolTimeStats");
  vol.setInstitution("NCAR/EOL");
  vol.setSource("Written by RadxVolTimeStats");
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

  // write the file

  if (_writeVol(vol)) {
    cerr << "ERROR - RadxConvert::_processFile" << endl;
    cerr << "  Cannot write volume to file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// set up write

void RadxVolTimeStats::_setupWrite(RadxFile &file)
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
// write out the volume

int RadxVolTimeStats::_writeVol(RadxVol &vol)
{

  // output file

  GenericRadxFile outFile;
  _setupWrite(outFile);
  
  // write to dir
  
  if (outFile.writeToDir(vol, _params.output_dir, true, false)) {
    cerr << "ERROR - RadxConvert::_writeVol" << endl;
    cerr << "  Cannot write file to dir: " << _params.output_dir << endl;
    cerr << outFile.getErrStr() << endl;
    return -1;
  }
  string outputPath = outFile.getPathInUse();

  return 0;

}

