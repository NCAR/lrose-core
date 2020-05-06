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
#include <vector>
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

  // create a volume depending on mode
  
  RadxVol vol;
  if (_params.specify_mode == Params::SPECIFY_RADAR_PARAMS) {
    _createVol(vol);
  } else {
    if (_readFile(_params.specified_file_path, vol)) {
      cerr << "ERROR - cannot read file: " << _params.specified_file_path << endl;
      return -1;
    }
  }

  cerr << "==>> Read file: " << _params.specified_file_path << endl;

  // add the gate geometry fields

  _addGeomFields(vol);

  // write the file

  if (_params.write_volume_to_output_file) {
    if (_writeVol(vol)) {
      cerr << "ERROR - RadxConvert::_processFile" << endl;
      cerr << "  Cannot write volume to file" << endl;
      return -1;
    }
  }

  // set up results vectors for different max heights

  vector<double> maxHtKm;
  vector<double> meanAgeFwd, meanAgeRev;
  vector< vector<double> > cumFreqFwd;
  vector< vector<double> > cumFreqRev;

  maxHtKm.resize(_params.age_hist_max_ht_km_n);
  meanAgeFwd.resize(_params.age_hist_max_ht_km_n);
  meanAgeRev.resize(_params.age_hist_max_ht_km_n);
  cumFreqFwd.resize(_params.age_hist_max_ht_km_n);
  cumFreqRev.resize(_params.age_hist_max_ht_km_n);
  
  for (int ii = 0; ii < _params.age_hist_max_ht_km_n; ii++) {
    maxHtKm[ii] = _params._age_hist_max_ht_km[ii];
  }
  
  // compute the age histograms
  
  for (int ii = 0; ii < _params.age_hist_max_ht_km_n; ii++) {
    _computeAgeHist(vol, maxHtKm[ii],
                    meanAgeFwd[ii], meanAgeRev[ii],
                    cumFreqFwd[ii], cumFreqRev[ii]);
  } // ii

  // write results to stdout

  _writeAgeResults(vol, maxHtKm,
                   meanAgeFwd, meanAgeRev,
                   cumFreqFwd, cumFreqRev);
  
  return 0;

}

//////////////////////////////////////////////////
// create a volume from specified params

void RadxVolTimeStats::_createVol(RadxVol &vol)
{
  
  vol.clear();
  RadxTime startTime(RadxTime::NOW);
  double timeSinceStart = 0.0;
  double startRangeKm = _params.gate_spacing_m / 2000.0;
  double gateSpacingKm = _params.gate_spacing_m / 1000.0;
  double maxRangeKm = _params.max_range_km;
  int nGates = (int) (maxRangeKm / gateSpacingKm + 0.5);
  
  // loop through specified sweeps
  
  for (int isweep = 0; isweep < _params.sweeps_n; isweep++) {
    
    double el = _params._sweeps[isweep].elev_deg;

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

}

//////////////////////////////////////////////////
// Read in a specified file
// Returns 0 on success
//         -1 on failure

int RadxVolTimeStats::_readFile(const string &readPath,
                                RadxVol &vol)
{
  
  // clear all data on volume object

  vol.clear();
  
  if (_params.debug) {
    cerr << "INFO - RadxConvert::Run" << endl;
    cerr << "  Input path: " << readPath << endl;
  }
  
  GenericRadxFile inFile;
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    inFile.setDebug(true);
  }
  if (_params.set_max_range) {
    inFile.setReadMaxRangeKm(_params.max_range_km);
  }
  inFile.setReadPreserveSweeps(true);

  // read in file

  if (inFile.readFromPath(readPath, vol)) {
    cerr << "ERROR - RadxVolTimeStats::_readFile" << endl;
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

void RadxVolTimeStats::_addGeomFields(RadxVol &vol)
{

  
  RadxTime startTime = vol.getStartRadxTime();

  double beamWidthRad = _params.beam_width_deg * DEG_TO_RAD;
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

////////////////////////////////////////////////////////////////////
// compute the age histogram

void RadxVolTimeStats::_computeAgeHist(RadxVol &vol, double maxHtKm,
                                       double &meanAgeFwd, double &meanAgeRev,
                                       vector<double> &cumFreqFwd,
                                       vector<double> &cumFreqRev)
{

  // get time limits, and vol duration
  
  RadxTime startTime = vol.getStartRadxTime();
  RadxTime endTime = vol.getEndRadxTime();
  double volDurationSecs = endTime - startTime;
  
  // initialize counter arrays

  double totalVol = 0.0;
  double totalWtFwd = 0.0;
  double totalWtRev = 0.0;

  vector<double> binVolFwd, binVolRev;
  binVolFwd.resize(_params.n_bins_age_histogram);
  binVolRev.resize(_params.n_bins_age_histogram);
  for (size_t ibin = 0; ibin < binVolFwd.size(); ibin++) {
    binVolFwd[ibin] = 0.0;
    binVolRev[ibin] = 0.0;
  }
  
  // accumulate volume in each bin
  
  for (size_t iray = 0; iray < vol.getNRays(); iray++) {
    
    // get ray
    
    RadxRay *ray = vol.getRays()[iray];
    int nGates = ray->getNGates();
    RadxTime rayTime = ray->getRadxTime();
    double ageFwd = endTime - rayTime;
    double ageRev = rayTime - startTime;

    // get fields, check they are non-null

    RadxField *sampleVolField = ray->getField("sampleVol");
    RadxField *heightField = ray->getField("height");
    
    assert(sampleVolField);
    assert(heightField);

    Radx::fl32 *sampleVol = sampleVolField->getDataFl32();
    Radx::fl32 *height = heightField->getDataFl32();

    // loop through the gates

    for (int igate = 0; igate < nGates; igate++) {

      // check height
      
      if (height[igate] > maxHtKm) {
        continue;
      }

      // compute age as fraction of vol duration
      
      double ageFracFwd = ageFwd / volDurationSecs;
      if (ageFracFwd < 0.0) {
        ageFracFwd = 0.0;
      } else if (ageFracFwd >= 1.0) {
        ageFracFwd = 0.999999;
      }
      int ageBinFwd = (int) (ageFracFwd * _params.n_bins_age_histogram);
      
      double ageFracRev = ageRev / volDurationSecs;
      if (ageFracRev < 0.0) {
        ageFracRev = 0.0;
      } else if (ageFracRev >= 1.0) {
        ageFracRev = 0.999999;
      }
      int ageBinRev = (int) (ageFracRev * _params.n_bins_age_histogram);
      
      // accumulate bin vol

      double vol = sampleVol[igate];
      binVolFwd[ageBinFwd] += vol;
      binVolRev[ageBinRev] += vol;

      totalVol += vol;
      totalWtFwd += vol * ageFracFwd;
      totalWtRev += vol * ageFracRev;

    } // igate

  } // iray

  // normalize volume by total to get fraction

  vector<double> binFreqFwd;
  binFreqFwd.resize(_params.n_bins_age_histogram);
  cumFreqFwd.resize(_params.n_bins_age_histogram);
  for (size_t ibin = 0; ibin < binVolFwd.size(); ibin++) {
    binFreqFwd[ibin] = binVolFwd[ibin] / totalVol;
  }
  cumFreqFwd[0] = binFreqFwd[0];
  for (size_t ibin = 1; ibin < binVolFwd.size(); ibin++) {
    cumFreqFwd[ibin] = cumFreqFwd[ibin - 1] + binFreqFwd[ibin];
  }
  meanAgeFwd = totalWtFwd / totalVol;

  vector<double> binFreqRev;
  binFreqRev.resize(_params.n_bins_age_histogram);
  cumFreqRev.resize(_params.n_bins_age_histogram);
  for (size_t ibin = 0; ibin < binVolRev.size(); ibin++) {
    binFreqRev[ibin] = binVolRev[ibin] / totalVol;
  }
  cumFreqRev[0] = binFreqRev[0];
  for (size_t ibin = 1; ibin < binVolRev.size(); ibin++) {
    cumFreqRev[ibin] = cumFreqRev[ibin - 1] + binFreqRev[ibin];
  }
  meanAgeRev = totalWtRev / totalVol;

}

////////////////////////////////////////////////////////////////////
// Write out the results to a text file

void RadxVolTimeStats::_writeAgeResults(RadxVol &vol,
                                        vector<double> &maxHtKm,
                                        vector<double> &meanAgeFwd,
                                        vector<double> &meanAgeRev,
                                        vector< vector<double> > &cumFreqFwd,
                                        vector< vector<double> > &cumFreqRev)
  
{

  // get time limits, and vol duration

  RadxTime startTime = vol.getStartRadxTime();
  RadxTime endTime = vol.getEndRadxTime();
  double volDurationSecs = endTime - startTime;
  int nBins = cumFreqFwd[0].size();
  int nHts = maxHtKm.size();
  
  // print to stdout
  
  char scanName[128];
  if (vol.getScanId() > 0 && string(_params.scan_name) == "Unknown") {
    snprintf(scanName, 128, "VCP%d", vol.getScanId());
  } else {
    snprintf(scanName, 128, "%s", _params.scan_name);
  }

  // header

  fprintf(stdout, "#########################################################\n");
  fprintf(stdout, "# scanName   : %s\n", scanName);
  fprintf(stdout, "# duration   : %.0f\n", volDurationSecs);
  fprintf(stdout, "# elevs      : ");
  const vector<RadxSweep *> &sweeps = vol.getSweeps();
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    fprintf(stdout, "%.2f", sweeps[ii]->getFixedAngleDeg());
    if (ii != sweeps.size()-1) {
      fprintf(stdout, ",");
    } else {
      fprintf(stdout, "\n");
    }
  }
  fprintf(stdout, "# heights    : ");
  for (int ii = 0; ii < nHts; ii++) {
    fprintf(stdout, "%g", maxHtKm[ii]);
    if (ii != nHts-1) {
      fprintf(stdout, ",");
    } else {
      fprintf(stdout, "\n");
    }
  }
  for (int ii = 0; ii < nHts; ii++) {
    fprintf(stdout, "# meanAgeFwd[%g] : %.3f\n", maxHtKm[ii], meanAgeFwd[ii]);
    fprintf(stdout, "# meanAgeRev[%g] : %.3f\n", maxHtKm[ii], meanAgeRev[ii]);
  }
  
  fprintf(stdout, 
          "# %8s %8s %8s",
          "binNum", "binAge", "binPos");

  for (int ii = 0; ii < nHts; ii++) {
    char label[128];
    snprintf(label, 128, "cumFreqFwd[%g]", maxHtKm[ii]);
    fprintf(stdout, " %14s", label);
    snprintf(label, 128, "cumFreqRev[%g]", maxHtKm[ii]);
    fprintf(stdout, " %14s", label);
  }
  fprintf(stdout, "\n");
  
  for (int ibin = 0; ibin < nBins; ibin++) {
    double binAge = ((ibin + 0.5) / nBins) * volDurationSecs;
    double binPos = ((ibin + 0.5) / nBins);
    fprintf(stdout, 
            "  %8d %8.2f %8.3f",
            ibin, binAge, binPos);
    for (int ii = 0; ii < nHts; ii++) {
      fprintf(stdout, " %14.6f %14.6f", cumFreqFwd[ii][ibin], cumFreqRev[ii][ibin]);
    }
    fprintf(stdout, "\n");
  } // ibin
  fprintf(stdout, "#########################################################\n");
  
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

