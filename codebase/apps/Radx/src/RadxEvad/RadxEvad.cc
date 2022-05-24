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
// ** 2) Redistributions of source code must retain the above conypyright      
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
// RadxEvad.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2013
//
///////////////////////////////////////////////////////////////

#include "RadxEvad.hh"
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Ncxx/Nc3xFile.hh>
#include <Mdv/GenericRadxFile.hh>
#include <dsserver/DsLdataInfo.hh>
#include <toolsa/pmu.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <toolsa/TaArray.hh>
#include <toolsa/Path.hh>
#include <toolsa/file_io.h>
#include <rapmath/RapComplex.hh>
#include <physics/IcaoStdAtmos.hh>
#include <physics/thermo.h>
#include <Spdb/SoundingPut.hh>
#include <algorithm>

using namespace std;

const double RadxEvad::missingVal = -9999.0;
const double RadxEvad::pseudoEarthDiamKm = 17066.0;

// Constructor

RadxEvad::RadxEvad(int argc, char **argv)
  
{

  OK = TRUE;

  // set programe name

  _progName = "RadxEvad";
  
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

  // check on overriding radar location

  if (_params.override_radar_location) {
    if (_params.radar_latitude_deg < -900 ||
        _params.radar_longitude_deg < -900 ||
        _params.radar_altitude_meters < -900) {
      cerr << "ERROR: " << _progName << endl;
      cerr << "  Problem with command line or TDRP parameters." << endl;
      cerr << "  You have chosen to override radar location" << endl;
      cerr << "  You must override latitude, longitude and altitude" << endl;
      cerr << "  You must override all 3 values." << endl;
      OK = FALSE;
    }
  }

  // matrix allocation

  _AA = (double **) umalloc2(nFourierCoeff, nFourierCoeff, sizeof(double));
  _AAinverse = (double **) umalloc2(nFourierCoeff, nFourierCoeff, sizeof(double));

  _PP = (double **) umalloc2(nDivCoeff, nDivCoeff, sizeof(double));
  _PPinverse = (double **) umalloc2(nDivCoeff, nDivCoeff, sizeof(double));

  // initialize the azimuth slice geometry, making sure we have
  // an integral number of slices

  _sliceDeltaAz = _params.slice_delta_azimuth;
  _nAzSlices = (int) (360.0 / _sliceDeltaAz + 0.5);
  _sliceDeltaAz = 360.0 / _nAzSlices;

  // profile geometry
  _profileMinHt = _params.profile_min_height;
  _profileMaxHt = _params.profile_max_height;
  _profileDeltaHt = _params.profile_height_interval;
  _profileNLevels = (int) ((_profileMaxHt - _profileMinHt) / _profileDeltaHt) + 1;

  if (_profileNLevels < 1) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "  Bad vertical geometry." << endl;
    cerr << "  profile_min_height: " << _params.profile_min_height << endl;
    cerr << "  profile_max_height: " << _params.profile_max_height << endl;
    cerr << "  profile_height_interval: " << _params.profile_height_interval << endl;
    OK = FALSE;
  }

  // init process mapper registration

  if (_params.mode == Params::REALTIME) {
    PMU_auto_init( _progName.c_str(),
                  _params.instance,
                  PROCMAP_REGISTER_INTERVAL);
  }

}

// destructor

RadxEvad::~RadxEvad()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int RadxEvad::Run()
{

  if (_params.mode == Params::ARCHIVE) {
    return _runArchive();
  } else if (_params.mode == Params::FILELIST) {
    return _runFilelist();
  } else {
    return _runRealtime();
  }
}

//////////////////////////////////////////////////
// Run in filelist mode

int RadxEvad::_runFilelist()
{

  // loop through the input file list

  int iret = 0;

  for (int ii = 0; ii < (int) _args.inputFileList.size(); ii++) {

    string inputPath = _args.inputFileList[ii];
    if (_processFile(inputPath)) {
      iret = -1;
    }

  }

  return iret;

}

//////////////////////////////////////////////////
// Run in archive mode

int RadxEvad::_runArchive()
{

  // get the files to be processed

  RadxTimeList tlist;
  tlist.setDir(_params.input_dir);
  tlist.setModeInterval(_args.startTime, _args.endTime);
  if (tlist.compile()) {
    cerr << "ERROR - RadxEvad::_runFilelist()" << endl;
    cerr << "  Cannot compile time list, dir: " << _params.input_dir << endl;
    cerr << "  Start time: " << RadxTime::strm(_args.startTime) << endl;
    cerr << "  End time: " << RadxTime::strm(_args.endTime) << endl;
    cerr << tlist.getErrStr() << endl;
    return -1;
  }

  const vector<string> &paths = tlist.getPathList();
  if (paths.size() < 1) {
    cerr << "ERROR - RadxEvad::_runFilelist()" << endl;
    cerr << "  No files found, dir: " << _params.input_dir << endl;
    return -1;
  }
  
  // loop through the input file list
  
  int iret = 0;
  for (size_t ii = 0; ii < paths.size(); ii++) {
    if (_processFile(paths[ii])) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Run in realtime mode

int RadxEvad::_runRealtime()
{

  // init process mapper registration

  PMU_auto_init(_progName.c_str(), _params.instance,
                PROCMAP_REGISTER_INTERVAL);

  // watch for new data to arrive

  LdataInfo ldata(_params.input_dir,
                  _params.debug >= Params::DEBUG_VERBOSE);
  
  int iret = 0;

  while (true) {
    ldata.readBlocking(_params.max_realtime_data_age_secs,
                       1000, PMU_auto_register);
    
    const string path = ldata.getDataPath();
    if (_processFile(path)) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// Process a file
// Returns 0 on success, -1 on failure

int RadxEvad::_processFile(const string &filePath)
{

  // check we have not already processed this file
  // in the file aggregation step

  RadxPath thisPath(filePath);
  for (size_t ii = 0; ii < _readPaths.size(); ii++) {
    RadxPath rpath(_readPaths[ii]);
    if (thisPath.getFile() == rpath.getFile()) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "Skipping file: " << filePath << endl;
        cerr << "  Previously processed in aggregation step" << endl;
      }
      return 0;
    }
  }
  
  if (_params.debug) {
    cerr << "INFO - RadxEvad::Run" << endl;
    cerr << "  Input path: " << filePath << endl;
  }
  
  GenericRadxFile inFile;
  _setupRead(inFile);
  
  // read in file
  
  _readVol.clear();
  if (inFile.readFromPath(filePath, _readVol)) {
    cerr << "ERROR - RadxEvad::Run" << endl;
    cerr << inFile.getErrStr() << endl;
    return -1;
  }
  _readPaths = inFile.getReadPaths();
  _nyquist = _params.nyquist_velocity;

  // override radar location if requested

  if (_params.override_radar_location) {
    _readVol.overrideLocation(_params.radar_latitude_deg,
                              _params.radar_longitude_deg,
                              _params.radar_altitude_meters / 1000.0);
  }

  // set number of gates constant
  
  _readVol.setNGatesConstant();
  
  // convert to floats

  _readVol.convertToFl32();
  
  // process this data set

  if (_processDataSet()) {
    cerr << "ERROR - RadxEvad::Run" << endl;
    cerr << "  Cannot process data in file: " << filePath << endl;
    return -1;
  }

  // write output

  int iret = 0;
  if (_params.write_results_to_netcdf) {
    if (_writeNetcdfOutput()) {
      iret = -1;
    }
  }

  if (_params.write_results_to_spdb) {
    if (_writeSpdbOutput()) {
      iret = -1;
    }
  }

  return iret;

}

//////////////////////////////////////////////////
// set up read

void RadxEvad::_setupRead(RadxFile &file)
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    file.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.setVerbose(true);
  }

  file.addReadField(_params.VEL_field_name);
  if (_params.censor_using_thresholds) {
    file.addReadField(_params.censor_field_name);
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    file.printReadRequest(cerr);
  }

}

///////////////////////////////////////////
// process this data set

int RadxEvad::_processDataSet()
  
{

  if (_params.debug) {
    cerr << "Processing data set ..." << endl;
  }

  // set up geom

  _nGates = _readVol.getMaxNGates();
  _radxStartRange = _readVol.getStartRangeKm();
  _radxGateSpacing = _readVol.getGateSpacingKm();

  _radarName = _readVol.getInstrumentName();
  _radarLatitude = _readVol.getLatitudeDeg();
  _radarLongitude = _readVol.getLongitudeDeg();
  _radarAltitude = _readVol.getAltitudeKm();

  // clear ring array

  _rings.clear();

  //double _max_range 
   // _max_range = _params.max_range;
  double _min_range = _params.min_range;
  double _delta_range = _params.delta_range;
  _nRanges = (int) ((_params.max_range - _params.min_range) /
    _params.delta_range + 0.5);  

  if (_params.range_gate_geom_equal) {
    _min_range = _radxStartRange;
    _delta_range = _radxGateSpacing;
    _nRanges = _nGates;
  } 

  // loop through the sweeps

  int ngood = 0;

  double firstval, lastval, vallo, valmid, valhi;

  const vector<RadxSweep *> &radxSweeps = _readVol.getSweeps();
  for (size_t isweep = 0; isweep < radxSweeps.size(); isweep++) {
    
    const RadxSweep *sweep = radxSweeps[isweep];
    double elev = sweep->getFixedAngleDeg();
    
    if (elev < _params.min_elev || elev > _params.max_elev) {
      if (_params.debug) {
	      cerr << "Ignoring sweep number: " << isweep << endl;
	      cerr << "    elev out of range: " << elev << endl;
      }
      continue;
    }

    for (int irange = 0; irange < _nRanges; irange++) {

      // set up ring info

      _ring.clear();
      _ring.sweepNum = isweep;
      _ring.rangeNum = irange;
      
      // compute gates to be used
      
      _ring.startRange = _min_range + irange * _delta_range;
      _ring.endRange = _ring.startRange + _delta_range;
      
      _ring.startGate = 
        (int) ((_ring.startRange - _radxStartRange) / _radxGateSpacing + 0.5);
      if (_ring.startGate < 0) {
	_ring.startGate = 0;
      }

      _ring.endGate = (int) ((_ring.endRange - _radxStartRange) / _radxGateSpacing + 0.5);
      if (_ring.endGate > _nGates - 1) {
	_ring.endGate = _nGates - 1;
      }

      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "ring at range " << irange << " ... " << endl;
        cerr << " startRange=" << _ring.startRange << endl;
        cerr << "   endRange=" << _ring.endRange << endl;
        cerr << " startGate =" << _ring.startGate << endl;
        cerr << "   endGate =" << _ring.endGate << endl;
      }

      // set elevation, compute range and height at ring mid-pt

      _ring.elev = elev;
      _ring.midRange = (_ring.startRange + _ring.endRange) / 2.0;
      _ring.midHt = (_ring.midRange * sin(_ring.elev * DEG_TO_RAD) +
                     (_ring.midRange * _ring.midRange / pseudoEarthDiamKm));

      if (_params.compute_profile_spacing_from_data) {
        if (irange == 1) firstval = _ring.midHt; // pick the 2nd value (zero-based) to be the profileMin
        if (irange % 3 == 1) lastval = _ring.midHt;
        if (irange == 1) vallo = _ring.midHt;
        if (irange == 3) valmid = _ring.midHt;
        if (irange == 4) valhi = _ring.midHt;
        if (_params.debug > Params::DEBUG_VERBOSE) {
          cout << "ring_midHt " << _ring.midHt << endl;
        }
      }

      // compute corrected elevation and coefficient for use in
      // computing divergence and vertical velocity

      double elevCorrectionRad = _ring.midRange / pseudoEarthDiamKm;
      _ring.elevStar = _ring.elev + elevCorrectionRad * RAD_TO_DEG;
      double rangeMeters = _ring.midRange * 1000.0;
      _ring.elevCoeff = ((2.0 * sin(_ring.elevStar * DEG_TO_RAD)) /
                         (rangeMeters * cos(_ring.elevStar * DEG_TO_RAD)));

      // compute wind solution for the ring
      
      if (_computeSolutionForRing(isweep, elev, irange) == 0) {
        ngood++;
      }

      // if a valid wind was obtained, save this ring

      if (_ring.windValid) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "====>> RING RESULTS VALID <<====" << endl;
          _printResultsForRing(cerr, _ring);
        }
        _rings.push_back(_ring);
      } else {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "====>> RING RESULTS INVALID <<====" << endl;
          _printResultsForRing(cerr, _ring);
        }
      }
      
    } // ir

  } // iel

  if (ngood < 1) {
    cerr << "ERROR - _processDataSet" << endl;
    cerr << "  Probably cannot find VEL field, name: " 
         << _params.VEL_field_name << endl;
    return -1;
  }

  if (_params.compute_profile_spacing_from_data) {
    double one_delta = valhi - valmid;
    _profileMinHt = firstval;
    _profileMaxHt = lastval;
    _profileDeltaHt = (valhi - vallo);
    _profileNLevels = (int) ((_profileMaxHt - _profileMinHt) / _profileDeltaHt) + 1;

    if (_params.debug > Params::DEBUG_VERBOSE) {
      cout << "one_delta " << one_delta << endl;
      cout << "_profileMinHt " << _profileMinHt << endl;
      cout << "_profileMaxHt " << _profileMaxHt << endl;
      cout << "_profileDeltaHt " << _profileDeltaHt << endl;
      cout << "_profileNLevels " << _profileNLevels << endl;  
    }    
  }

  // load up the raw profile

  _loadProfile();

  // compute divergence

  _computeDivergence();

  // interpolate divergence across missing sections

  _interpDivergence();

  // compute the vertical velocity

  _computeVertVel(_params.w_at_top_level);

  // debug print

  if (_params.debug) {
    fprintf(stderr, "========== Interp wind profile ==============\n");
    fprintf(stderr,
            "%5s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %8s %6s\n",
            "ht", "spd", "dirn", "u", "v",
            "w", "wp", "div", "div'", "divRms",
            "rho", "rhoRmsD", "nrings");
    for (int ii = 0; ii < (int) _profile.interp.size(); ii++) {
      const ProfilePt &pt = _profile.interp[ii];
      double div = pt.div;
      double divPrime = pt.divPrime;
      if (div != missingVal) {
        div *= 1.0e5;
      }
      if (divPrime != missingVal) {
        divPrime *= 1.0e5;
      }
      fprintf(stderr,
              "%5.3f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %8.2f %6d\n",
              pt.ht, pt.windSpeed, pt.windDirn, pt.uu, pt.vv,
              pt.ww, pt.wp, div, divPrime, pt.rmsDiv,
              pt.rho, pt.rmsRhoD, pt.nrings);
    }
    fprintf(stderr, "=============================================\n");
  }

  return 0;

}

///////////////////////////////////////////
// compute wind solution for a single ring

int RadxEvad::_computeSolutionForRing(int isweep, double elev, int irange)
  
{

  // load up vel points in azimuth slices

  _ring.slices.clear();
  for (int islice = 0; islice < _nAzSlices; islice++) {
    AzSlice slice;
    slice.num = islice;
    slice.valid = false;
    slice.meanAz = islice * _sliceDeltaAz;
    slice.startAz = slice.meanAz - _sliceDeltaAz / 2.0;
    slice.endAz = slice.meanAz + _sliceDeltaAz / 2.0;
    slice.meanVel = 0;
    slice.unfoldedVel = 0;
    slice.foldInterval = 0;
    _computeFf(slice.meanAz, slice.ff);
    _ring.slices.push_back(slice);
  } // islice

  // loop through rays, loading up slices with velocity data

  double cosElev = cos(elev * DEG_TO_RAD);
  const RadxSweep *sweep = _readVol.getSweeps()[isweep];
  const vector<RadxRay *> &rays = _readVol.getRays();
  int ngood = 0;

  for (size_t iray = sweep->getStartRayIndex();
       iray <= sweep->getEndRayIndex(); iray++) {
    
    const RadxRay *ray = rays[iray];

    // get az

    double az = ray->getAzimuthDeg();
    if (az < 0) {
      az += 360;
    } else if (az > 360) {
      az -= 360;
    }

    // compute slice num

    int sliceNum = (int) ((az / _sliceDeltaAz) + 0.5);
    if (sliceNum >= _nAzSlices) {
      sliceNum = 0;
    }

    // get field pointers

    const RadxField *velFld = ray->getField(_params.VEL_field_name);
    if (velFld == NULL) {
      if (_params.debug >= Params::DEBUG_VERBOSE) {
        cerr << "WARNING - RadxEvad::_computeSolutionForRing()" << endl;
        cerr << "  Cannot find VEL field, name: " << _params.VEL_field_name << endl;
        cerr << "  Ray az: " << az << endl;
      }
      continue;
    }
    ngood++;
    const Radx::fl32 *velArray = velFld->getDataFl32();
    Radx::fl32 velMiss = velFld->getMissingFl32();
    double rayNyquist = ray->getNyquistMps();
    if (!_params.set_nyquist_velocity) {
      _nyquist = rayNyquist;
    }
    
    const Radx::fl32 *censorArray = NULL;
    Radx::fl32 censorMiss = -9999;
    if (_params.censor_using_thresholds) {
      const RadxField *censorFld = ray->getField(_params.censor_field_name);
      if (censorFld == NULL) {
        if (_params.debug >= Params::DEBUG_VERBOSE) {
          cerr << "WARNING - RadxEvad::_computeSolutionForRing()" << endl;
          cerr << "  Cannot find censor field, name: "
               << _params.censor_field_name << endl;
          cerr << "  Ray az: " << az << endl;
        }
      } else {
        censorArray = censorFld->getDataFl32();
        censorMiss = censorFld->getMissingFl32();
      }
    }
    
    // loop through gates
    
    for (int igate = _ring.startGate; igate <= _ring.endGate; igate++) {
      if (velArray[igate] == velMiss) {
        continue;
      }
      double vel = velArray[igate] * cosElev;
      double accept = true;
      if (censorArray != NULL) {
        double censor = censorArray[igate];
        if (censor == censorMiss) {
          accept = false;
        } else if (censor < _params.censor_min_value ||
                   censor > _params.censor_max_value) {
          accept = false;
        }
      }
      if (accept) {
        VelPt pt;
        pt.az = az;
        pt.vel = vel;
        _ring.slices[sliceNum].pts.push_back(pt);
      }
    } // igate

  } // iray
  
  if (ngood < 1) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "WARNING - RadxEvad::_computeSolutionForRing()" << endl;
      cerr << "  sweepNum, rangeNum: " << isweep << ", " << irange << endl;
      cerr << "  Cannot find VEL field, name: " 
           << _params.VEL_field_name << endl;
    }
    return -1;
  }

  // for each slice, compute the mean velocity on the phase circle
  // and for now set the unfolded velocity to this mean value

  for (int islice = 0; islice < _nAzSlices; islice++) {
    _computeMeanVel(_ring.slices[islice]);
    _ring.slices[islice].unfoldedVel = _ring.slices[islice].meanVel;
    _ring.slices[islice].foldInterval = 0;
  }

  // for each slice, compute the median velocity on the azimuth circle
  
  for (int islice = 0; islice < _nAzSlices; islice++) {
    _computeMedianVel(islice);
  }
  
  // copy median values into gaps

  _copyMedianIntoGaps();

  // identify the 0-isodop points and folding points
  
  _identifyFolds();
  
  if (_ring.foldIndices.size() == 0) {
    
    // no unfolding needed

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> no unfolding needed - no folds found" << endl;
    }

  } else if (_rings.size() == 0) {
    
    // first ring, unfold iteratively from the zero-isodops

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> unfolding iteratively from zero isodop" << endl;
    }
    
    _unfoldIteratively();

  } else {

    // unfold from previous circle

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "-->> unfolding from previous ring" << endl;
    }

    _unfoldFromPreviousRing(_ring.midHt);
    
  }
  
  // compute VAD

  _computeWindForRing();

  return 0;

}

///////////////////////////////////////////////////
// compute mean velocity for an azimuth slice
//
// This is done 'on the phase circle' - to take folding
// into account
  
void RadxEvad::_computeMeanVel(AzSlice &slice)

{

  if ((int) slice.pts.size() >= _params.min_vel_values_per_slice) {
    slice.valid = true;
  } else {
    slice.valid = false;
  }

  if (slice.pts.size() < 1) {
    return;
  }

  if (slice.pts.size() == 1) {
    slice.meanVel = slice.pts[0].vel;
  }


  double sumU = 0.0;
  double sumV = 0.0;
  double count = 0.0;
  
  for (int ipt = 0; ipt < (int) slice.pts.size(); ipt++) {

    const VelPt &pt = slice.pts[ipt];
    double phaseRad = (pt.vel / _nyquist) * M_PI;
    double sinPhase, cosPhase;
    ta_sincos(phaseRad, &sinPhase, &cosPhase);
    sumU += cosPhase;
    sumV += sinPhase;
    count++;
    
  }

  double meanU = sumU / count;
  double meanV = sumV / count;

  if (meanU == 0.0 && meanV == 0.0) {
    slice.meanVel = 0.0;
  } else {
    slice.meanVel = (atan2(meanV, meanU) / M_PI) * _nyquist;
  }

}

///////////////////////////////////////////////////
// compute mean velocity for an azimuth slice
//
// This is done 'on the phase circle' - to take folding
// into account
  
void RadxEvad::_computeMedianVel(int sliceNum)

{

  AzSlice &slice = _ring.slices[sliceNum];

  // load up array of velocity values in vicinity of this slice

  int nHalfMedian = _params.n_slices_for_vel_median / 2;
  int nMedian = nHalfMedian * 2 + 1;

  TaArray<double> velArray_;
  double *vel = velArray_.alloc(nMedian);

  // this slice

  vel[nHalfMedian] = slice.meanVel;

  for (int ii = 0; ii < nHalfMedian; ii++) {
    // slices below this one
    int kk = sliceNum - ii;
    if (kk < 0) {
      kk += _nAzSlices;
    }
    vel[ii] = _ring.slices[kk].meanVel;
    // slices above this one
    kk = slice.num + ii;
    if (kk >= _nAzSlices) {
      kk -= _nAzSlices;
    }
    vel[nHalfMedian + ii] = _ring.slices[kk].meanVel;
  }
  
  // compute median
  
  qsort(vel, nMedian, sizeof(double), _doubleCompare);

  // set the value

  slice.medianVel = vel[nHalfMedian];

}

///////////////////////////////////////////
// copy median values into gaps

void RadxEvad::_copyMedianIntoGaps()
  
{

  bool done = false;
  while (!done) {

    done = true;

    for (int ii = 0; ii < _nAzSlices; ii++) {
      
      AzSlice &slice = _ring.slices[ii];
      slice.foldBoundary = false;
      slice.zeroIsodop = false;
      
      int prevIndex = ii - 1;
      if (prevIndex == -1) {
        prevIndex = _nAzSlices - 1;
      }

      int nextIndex = ii + 1;
      if (nextIndex == _nAzSlices) {
        nextIndex = 0;
      }
      
      double thisVel = _ring.slices[ii].medianVel;
      double prevVel = _ring.slices[prevIndex].medianVel;
      double nextVel = _ring.slices[nextIndex].medianVel;

      if (thisVel != 0 && prevVel == 0) {
        _ring.slices[prevIndex].medianVel = thisVel;
        done = false;
      }

      if (thisVel != 0 && nextVel == 0) {
        _ring.slices[nextIndex].medianVel = thisVel;
        done = false;
      }

    } // ii

  } // while

}

///////////////////////////////////////////
// identify the folds and 0-isodop points

void RadxEvad::_identifyFolds()
  
{

  _ring.zeroIndices.clear();
  _ring.foldIndices.clear();

  for (int ii = 0; ii < _nAzSlices; ii++) {

    AzSlice &slice = _ring.slices[ii];
    slice.foldBoundary = false;
    slice.zeroIsodop = false;

    int prevIndex = ii - 1;
    if (prevIndex == -1) {
      prevIndex = _nAzSlices - 1;
    }
                 
    int nextIndex = ii + 1;
    if (nextIndex == _nAzSlices) {
      nextIndex = 0;
    }
    
    double prevVel = _ring.slices[prevIndex].medianVel;
    double nextVel = _ring.slices[nextIndex].medianVel;
    double thisVel = _ring.slices[ii].medianVel;

    if ((prevVel < 0 && thisVel > 0) ||
        (prevVel > 0 && thisVel < 0)) {

      double absDiff = fabs(thisVel - prevVel);
      if (absDiff > _nyquist) {
        // fold point
        slice.foldBoundary = true;
        _ring.foldIndices.push_back(ii);
      } else {
        // zero isodop
        slice.zeroIsodop = true;
        _ring.zeroIndices.push_back(ii);
      }

    }

    if ((nextVel < 0 && thisVel > 0) ||
        (nextVel > 0 && thisVel < 0)) {

      double absDiff = fabs(thisVel - nextVel);
      if (absDiff > _nyquist) {
        // fold point
        slice.foldBoundary = true;
      } else {
        // zero isodop
        slice.zeroIsodop = true;
      }

    }

  } // ii

  if (_params.debug >= Params::DEBUG_EXTRA) {
    for (int islice = 0; islice < _nAzSlices; islice++) {
      cerr << "slice az, count, meanVel, medianVel: "
           << _ring.slices[islice].meanAz << ", "
           << _ring.slices[islice].pts.size() << ", "
           << _ring.slices[islice].meanVel << ", "
           << _ring.slices[islice].medianVel;
      if (_ring.slices[islice].foldBoundary) {
        cerr << "  ====>> FOLD BOUNDARY <<====";
      }
      if (_ring.slices[islice].zeroIsodop) {
        cerr << "  ====>> ZERO ISODOP <<====";
      }
      cerr << endl;
    }
    for (int ii = 0; ii < (int) _ring.zeroIndices.size(); ii++) {
      cerr << "  ZERO ISODOP at index: " << _ring.zeroIndices[ii] << endl;
    }
    for (int ii = 0; ii < (int) _ring.foldIndices.size(); ii++) {
      cerr << "  FOLD BOUNDARY at index: " << _ring.foldIndices[ii] << endl;
    }
  }
  
}

///////////////////////////////////////////////////
// Unfold iteratively using zero isodops
// Use the solution which minimizes the variance
  
int RadxEvad::_unfoldIteratively()
  
{

      
  if (_ring.zeroIndices.size() == 0) {
    // no zero isodop identified, so cannot unfold
    return -1;
  }

  double minVariance = 1.0e99;
  bool unfoldingSuccess = false;
  vector<AzSlice> optimumSlices;

  for (int ii = 0; ii < (int) _ring.zeroIndices.size(); ii++) {

    if (_unfoldFromZeroIsodop(_ring.zeroIndices[ii]) == 0) {

      unfoldingSuccess = true;
      _computeWindForRing();
      
      if (_ring.fitVariance < minVariance) {
        optimumSlices = _ring.slices;
        minVariance = _ring.fitVariance;
      }
      
    }
    
  } // ii
  
  if (unfoldingSuccess) {
    _ring.slices = optimumSlices;
    _ring.fitVariance = minVariance;
    return 0;
  } else {
    return -1;
  }
  
}

///////////////////////////////////////////////////
// Unfold from zero isodop index
// Returns 0 on success, -1 on failure.
  
int RadxEvad::_unfoldFromZeroIsodop(int zeroIsodopIndex)
  
{

  // go around the circle from the starting index

  int foldInterval = 0;
  
  for (int ii = 0; ii <= _nAzSlices; ii++) {

    int thisIndex = ii + zeroIsodopIndex;
    if (thisIndex >= _nAzSlices) {
      thisIndex -= _nAzSlices;
    }

    int nextIndex = ii + zeroIsodopIndex + 1;
    if (nextIndex >= _nAzSlices) {
      nextIndex -= _nAzSlices;
    }

    AzSlice &thisSlice = _ring.slices[thisIndex];
    AzSlice &nextSlice = _ring.slices[nextIndex];

    double thisVel = thisSlice.meanVel;
    double nextVel = nextSlice.meanVel;
    
    if (nextVel < 0 && thisVel > 0) {
      double absDiff = fabs(thisVel - nextVel);
      if (absDiff > _nyquist) {
        // fold point upwards
        foldInterval++;
      }
    } else if (nextVel > 0 && thisVel < 0) {
      double absDiff = fabs(thisVel - nextVel);
      if (absDiff > _nyquist) {
        // fold point downwards
        foldInterval--;
      }
    }
    
    if (ii == 0) {
      thisSlice.foldInterval = 0;
      thisSlice.unfoldedVel = thisSlice.meanVel;
    } else if (ii < _nAzSlices) {
      nextSlice.foldInterval = foldInterval;
      nextSlice.unfoldedVel = nextSlice.meanVel + (foldInterval * 2.0 * _nyquist);
    }

  } // ii

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "============ unfolding from zero isodop ===================" << endl;
    cerr << "Starting at slice index: " << zeroIsodopIndex << endl;
    for (int islice = 0; islice < _nAzSlices; islice++) {
      cerr << "slice num, interval, az, medianVel, unfoldedVel: "
           << _ring.slices[islice].num << ", "
           << _ring.slices[islice].foldInterval << ", "
           << _ring.slices[islice].meanAz << ", "
           << _ring.slices[islice].meanVel << ", "
           << _ring.slices[islice].unfoldedVel;
      if (_ring.slices[islice].foldBoundary) {
        cerr << "  ====>> FOLD BOUNDARY <<====";
      }
      if (_ring.slices[islice].zeroIsodop) {
        cerr << "  ====>> ZERO ISODOP <<====";
      }
      cerr << endl;
    }
    cerr << "===========================================================" << endl;
  }

  // check we are back where we started

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    if (foldInterval == 0) {
      cerr << "Unfolding successful" << endl;
    } else {
      cerr << "WARNING - could not unfold" << endl;
      cerr << "  Ending foldInterval: " << foldInterval << endl;
    }
  }

  if (foldInterval == 0) {
    return 0;
  } else {
    return -1;
  }

}

///////////////////////////////////////////////////
// Unfold from model fit in a previous ring
  
void RadxEvad::_unfoldFromPreviousRing(double ht)
  
{

  // find closest ring in height to this one

  double maxDiff = 1.0e99;
  int bestRingNum = -1;
  
  for (int ii = 0; ii < (int) _rings.size(); ii++) {
    double diff = fabs(ht - _rings[ii].midHt);
    if (diff < maxDiff) {
      bestRingNum = ii;
      maxDiff = diff;
    }
  }

  const VelRing &ring = _rings[bestRingNum];

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Unfolding using ring el, range, midHt: "
         << ring.elev << ", " << ring.midRange << ", " << ring.midHt << endl;
  }
  
  for (int ii = 0; ii < _nAzSlices; ii++) {
    
    double velFromPrevFit = ring.slices[ii].modelVel;
    double thisVel = _ring.slices[ii].meanVel;

    double diff = velFromPrevFit - thisVel;
    double normDiff = fabs(diff / _nyquist);
    int foldInterval = (int) (normDiff / 2 + 0.5);
    if (diff < 0) {
      foldInterval *= -1;
    }

    double unfoldedVel = thisVel + (foldInterval * 2.0 * _nyquist);

    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "velFromPrevFit, thisVel, diff, normDiff, foldInterval, unfoldedVel: "
           << velFromPrevFit << ", "
           << thisVel << ", "
           << diff << ", "
           << normDiff << ", "
           << foldInterval << ", "
           << unfoldedVel << endl;
    }

    _ring.slices[ii].unfoldedVel = unfoldedVel;

  } // ii

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "============ unfolding from previous fit ==================" << endl;
    for (int islice = 0; islice < _nAzSlices; islice++) {
      cerr << "slice num, interval, az, medianVel, unfoldedVel: "
           << _ring.slices[islice].num << ", "
           << _ring.slices[islice].foldInterval << ", "
           << _ring.slices[islice].meanAz << ", "
           << _ring.slices[islice].meanVel << ", "
           << _ring.slices[islice].unfoldedVel;
      if (_ring.slices[islice].foldBoundary) {
        cerr << "  ====>> FOLD BOUNDARY <<====";
      }
      if (_ring.slices[islice].zeroIsodop) {
        cerr << "  ====>> ZERO ISODOP <<====";
      }
      cerr << endl;
    }
    cerr << "===========================================================" << endl;
  }

}

///////////////////////////////////////////
// compute VAD for azimuth slices

void RadxEvad::_computeWindForRing()
  
{

  // initialize matrices
  
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    _aa[ii] = 0.0;
    _bb[ii] = 0.0;
    for (int jj = 0; jj < nFourierCoeff; jj++) {
      _AA[ii][jj] = 0.0;
    }
  }

  for (int islice = 0; islice < _nAzSlices; islice++) {

    AzSlice &slice = _ring.slices[islice];
    
    // check if slice has valid data

    if (!slice.valid) {
      continue;
    }

    for (int ii = 0; ii < nFourierCoeff; ii++) {
      _bb[ii] += slice.ff[ii] * slice.unfoldedVel;
      for (int jj = 0; jj < nFourierCoeff; jj++) {
        _AA[ii][jj] += slice.ff[ii] * slice.ff[jj];
      }
    }

  } // islice

  // do we have any data to work with?

  bool haveData = false;
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    for (int jj = 0; jj < nFourierCoeff; jj++) {
      if (_AA[ii][jj] != 0) {
        haveData = true;
        break;
      }
    }
  }

  if (!haveData) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "RadxEvad::_computewindForRing()" << endl;
      cerr << "========>> No data - cannot compute <<=============" << endl;
    }
    return;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {

    cerr << "============== RAW ARRAY =============" << endl;
    for (int ii = 0; ii < nFourierCoeff; ii++) {
      fprintf(stderr, "ii, bb, _AA: %d %10.5f : ", ii, _bb[ii]);
      for (int jj = 0; jj < nFourierCoeff; jj++) {
        fprintf(stderr, " %10.5f", _AA[ii][jj]);
      }
      fprintf(stderr, "\n");
    }
    cerr << "======================================" << endl;
  }

  // invert AA

  _invertAA();

  if (_params.debug >= Params::DEBUG_EXTRA) {

    cerr << "============== INVERSE ARRAY =============" << endl;
    for (int ii = 0; ii < nFourierCoeff; ii++) {
      fprintf(stderr, "ii, bb, _AA: %d %10.5f : ", ii, _bb[ii]);
      for (int jj = 0; jj < nFourierCoeff; jj++) {
        fprintf(stderr, " %10.5f", _AAinverse[ii][jj]);
      }
      fprintf(stderr, "\n");
    }
    cerr << "==========================================" << endl;

  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {

    // check for identity matrix
    
    double identity[nFourierCoeff][nFourierCoeff];
    for (int ii = 0; ii < nFourierCoeff; ii++) {
      for (int jj = 0; jj < nFourierCoeff; jj++) {
        double sum = 0.0;
        for (int kk = 0; kk < nFourierCoeff; kk++) {
          sum += _AA[ii][kk] * _AAinverse[kk][jj];
        }
        identity[ii][jj] = sum;
      }
    }

    cerr << "============== IDENTITY MATRIX =============" << endl;
    for (int ii = 0; ii < nFourierCoeff; ii++) {
      for (int jj = 0; jj < nFourierCoeff; jj++) {
        fprintf(stderr, " %10.5f", identity[ii][jj]);
      }
      fprintf(stderr, "\n");
    }
    cerr << "============================================" << endl;

  }

  // check matrix conndition
  
  double normAA = _norm(_AA, nFourierCoeff);
  double normInv = _norm(_AAinverse, nFourierCoeff);
  double condition = normAA * normInv;
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "Checking matrix condition" << endl;
    cerr << "  normAA, normInv: " << normAA << ", " << normInv << endl;
    cerr << "  condition: " << condition << endl;
  }
  
  if (condition > 1.0e6) {
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "NOTE - matrix ill-conditioned" << endl;
      cerr << "  Will not use" << endl;
    }
    return;
  }

  // compute aa

  for (int ii = 0; ii < nFourierCoeff; ii++) {
    double sum = 0.0;
    for (int jj = 0; jj < nFourierCoeff; jj++) {
      sum += _AAinverse[ii][jj] * _bb[jj];
    }
    _aa[ii] = sum;
  }
  memcpy(_ring.aa, _aa, nFourierCoeff * sizeof(double));

  // compute YY, used later in divergence

  double rangeMeters = _ring.midRange * 1000.0;
  _ring.YY = (2.0 * _aa[1]) / (rangeMeters * cos(_ring.elevStar * DEG_TO_RAD));

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "============== aa =============" << endl;
    for (int ii = 0; ii < nFourierCoeff; ii++) {
      cerr << "ii, aa[ii]: " << ii << ", " << _aa[ii] << endl;
    }
    cerr << "===============================" << endl;
  }

  // compute the model fit velocity

  for (int islice = 0; islice < _nAzSlices; islice++) {
    AzSlice &slice = _ring.slices[islice];
    if (slice.valid) {
      double vr = 0.0;
      for (int ii = 0; ii < nFourierCoeff; ii++) {
        vr += _aa[ii] * slice.ff[ii];
      }
      slice.modelVel = vr;
    }
  } // islice

  // compute S-squared - estimated variance

  double sumSqDiff = 0.0;
  double count = 0.0;
  for (int islice = 0; islice < _nAzSlices; islice++) {
    AzSlice &slice = _ring.slices[islice];
    if (slice.valid) {
      double diff = slice.unfoldedVel - slice.modelVel;
      sumSqDiff += diff * diff;
      count++;
    }
  } // islice
  double variance = 9999;
  double rms = 99;
  if (count > nFourierCoeff) {
    variance = sumSqDiff / (count - nFourierCoeff);
    rms = sqrt(variance);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "==>> VARIANCE: " << variance << endl;
    cerr << "==>> RMS ERROR: " << rms << endl;
  }
  
  _ring.fitVariance = variance;
  _ring.fitRmsError = rms;

  // compute variance of the aa terms
  // used in computing divergence

  for (int ii = 0; ii < nFourierCoeff; ii++) {
    _ring.aaVariance[ii] = _ring.fitVariance * _AAinverse[ii][ii];
  }
  
  // compute radial wind speed for all slices, pick the max/min

  _ring.maxVr = -1.0e9;
  _ring.minVr = 1.0e9;
  _ring.azForMaxVr = 0.0;
  _ring.azForMinVr = 0.0;
  
  for (int islice = 0; islice < _nAzSlices; islice++) {
    AzSlice &slice = _ring.slices[islice];
    double az = slice.meanAz;
    double vr = slice.modelVel;
    if (vr > _ring.maxVr) {
      _ring.maxVr = vr;
      _ring.azForMaxVr = az;
    }
    if (vr < _ring.minVr) {
      _ring.minVr = vr;
      _ring.azForMinVr = az;
    }
  } // islice
  
  double recipAzForMaxVr = RapComplex::computeSumDeg(_ring.azForMaxVr, 180);
  _ring.windDirn = RapComplex::computeMeanDeg(_ring.azForMinVr, recipAzForMaxVr);
  if (_ring.windDirn < 0) {
    _ring.windDirn += 360.0;
  }
  _ring.azError = fabs(RapComplex::computeDiffDeg(_ring.azForMinVr, recipAzForMaxVr));
  if (_ring.azError < 0.1) {
    _ring.azError = 0.0;
  }
  _ring.windSpeed = (_ring.maxVr - _ring.minVr) / 2.0;

  // check for validity using to/from azimuth error

  _ring.windValid = true;
  if (_ring.azError > _params.max_to_from_direction_error) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Ring invalid, elev, minRange: "
           << _ring.elev << ", " << _ring.midRange << endl;
      cerr << "  max to/from direction error: "
           << _params.max_to_from_direction_error << endl;
      cerr << "  actual to/from error: "  << _ring.azError << endl;
    }
    _ring.windValid = false;
  }

  // check for validity using RMS error

  if (_ring.fitRmsError > _params.max_fit_rms_error) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Ring invalid, elev, minRange: "
           << _ring.elev << ", " << _ring.midRange << endl;
      cerr << "  max rms error allowed: " << _params.max_fit_rms_error << endl;
      cerr << "  actual rms error: "  << _ring.fitRmsError << endl;
    }
    _ring.windValid = false;
  }

  // determine if this ring has valid data by checking for the size of
  // missing sectors

  int maxMissingAllowed = (int)
    (_params.max_missing_sector_size / _params.slice_delta_azimuth + 0.5);

  int maxMissing = 0;
  int nmissAtStart = -1;
  int nmiss = 0;
  
  for (int islice = 0; islice < _nAzSlices; islice++) {
    AzSlice &slice = _ring.slices[islice];
    if (!slice.valid) {
      nmiss++;
    } else {
      if (nmiss > maxMissing) {
        maxMissing = nmiss;
      }
      if (nmissAtStart == -1) {
        // save nmiss at start of circle
        nmissAtStart = nmiss;
      }
      nmiss = 0;
    }
  } // islice

  // add together nmiss at start and end of circle

  nmissAtStart += nmiss;
  if (nmissAtStart > maxMissing) {
    maxMissing = nmissAtStart;
  }

  if (maxMissing > maxMissingAllowed) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Ring invalid, elev, minRange: "
           << _ring.elev << ", " << _ring.midRange << endl;
      cerr << "  max missing sector size: "
           << _params.max_missing_sector_size << endl;
      cerr << "  max missing slices: "  << maxMissingAllowed << endl;
      cerr << "  n missing slices: "  << maxMissing << endl;
    }
    _ring.windValid = false;
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "== INTERMEDIATE RESULTS ==" << endl;
    _printResultsForRing(cerr, _ring);
  }

}


///////////////////////////////////////////////////
// load vertical profile on even height intervals
  
void RadxEvad::_loadProfile()

{

  _profile.clear();

  // load up raw profile

  for (int ii = 0; ii < (int) _rings.size(); ii++) {
    const VelRing &ring = _rings[ii];
    ProfilePt pt;
    pt.ht = ring.midHt;
    pt.windSpeed = ring.windSpeed;
    pt.windDirn = ring.windDirn;
    pt.uu = pt.windSpeed * sin((pt.windDirn + 180.0) * DEG_TO_RAD);
    pt.vv = pt.windSpeed * cos((pt.windDirn + 180.0) * DEG_TO_RAD);
    _profile.raw.push_back(pt);
  }

  // sort the raw profile, lowest to highest

  sort(_profile.raw.begin(), _profile.raw.end(), ProfilePtCompare());

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "========= Sorted raw wind profile ===========\n");
    for (int ii = 0; ii < (int) _profile.raw.size(); ii++) {
      const ProfilePt &pt = _profile.raw[ii];
      fprintf(stderr,
              "ht, speed, dirn, u, v: %10.3f %10.3f %10.3f %10.3f %10.3f\n",
              pt.ht, pt.windSpeed, pt.windDirn, pt.uu, pt.vv);
    }
    fprintf(stderr, "=============================================\n");
  }

  // interpolate the profile onto a regular grid

  IcaoStdAtmos icao;
  for (int ii = 0; ii < _profileNLevels; ii++) {

    double midHt = _profileMinHt + ii * _profileDeltaHt;
    double lowerLimit = midHt - _profileDeltaHt / 2.0;
    double upperLimit = midHt + _profileDeltaHt / 2.0;

    // look for wind results within the height limits

    vector<ProfilePt> ptsFound;
    for (int jj = 0; jj < (int) _profile.raw.size(); jj++) {
      const ProfilePt &pt = _profile.raw[jj];
      if (pt.ht >= lowerLimit && pt.ht < upperLimit) {
        ptsFound.push_back(pt);
      }
    } // jj

    ProfilePt interpPt;
    interpPt.ht = midHt;
    double htMeters = midHt * 1000.0;
    double mb = icao.ht2pres(htMeters);
    double tk = icao.ht2temp(htMeters);
    double rho = PHYprestemp2density(mb, tk);
    interpPt.rho = rho;
    
    if (ptsFound.size() > 0) {
      
      // use mean wind within the height interval
      
      double sumUU = 0.0;
      double sumVV = 0.0;

      for (int kk = 0; kk < (int) ptsFound.size(); kk++) {
        sumUU += ptsFound[kk].uu;
        sumVV += ptsFound[kk].vv;
      }

      double meanUU = sumUU / ptsFound.size();
      double meanVV = sumVV / ptsFound.size();
      
      double speed = sqrt(meanUU * meanUU + meanVV * meanVV);
      double dirn = 0.0;
      if (meanUU != 0.0 || meanVV != 0.0) {
        dirn = atan2(meanUU, meanVV) * RAD_TO_DEG -180.0;
        if (dirn < 0) {
          dirn += 360.0;
        }
      }

      interpPt.windSpeed = speed;
      interpPt.windDirn = dirn;
      interpPt.uu = meanUU;
      interpPt.vv = meanVV;

    } else { // if (ptsFound.size() > 0) ...

      // look for points below and above
      
      ProfilePt ptBelow;
      ProfilePt ptAbove;
      
      for (int jj = 0; jj < (int) _profile.raw.size(); jj++) {
        const ProfilePt &pt = _profile.raw[jj];
        if (pt.ht <= midHt) {
          ptBelow = pt;
        }
        if (pt.ht >= midHt) {
          ptAbove = pt;
          break;
        }
      } // jj
      
      // interpolate between points below and above
      
      if (ptBelow.ht > 0 && ptAbove.ht > 0) {

        double htDiffBelow = midHt - ptBelow.ht;
        double htDiffAbove = ptAbove.ht - midHt;
        double htDiffTotal = htDiffBelow + htDiffAbove;
        double fractionBelow = htDiffBelow / htDiffTotal;
        double fractionAbove = htDiffAbove / htDiffTotal;

        double uu = ptBelow.uu * fractionAbove + ptAbove.uu * fractionBelow;
        double vv = ptBelow.vv * fractionAbove + ptAbove.vv * fractionBelow;
        
        double speed = sqrt(uu * uu + vv * vv);
        double dirn = 0.0;
        if (uu != 0.0 || uu != 0.0) {
          dirn = atan2(uu, vv) * RAD_TO_DEG - 180.0;
          if (dirn < 0) {
            dirn += 360.0;
          }
        }
        
        interpPt.windSpeed = speed;
        interpPt.windDirn = dirn;
        interpPt.uu = uu;
        interpPt.vv = vv;
        
      }
      
    } // if (ptsFound.size() > 0) ...

    _profile.interp.push_back(interpPt);

  } // ii

}

///////////////////////////////////////////////////
// compute divergence and precip vertical velocity
  
void RadxEvad::_computeDivergence()

{

  // find the rings within a given level and for a given range

  for (int ilevel = 0; ilevel < _profileNLevels; ilevel++) {

    double midHt = _profileMinHt + ilevel * _profileDeltaHt;
    double lowerLimit = midHt - _profileDeltaHt;
    double upperLimit = midHt + _profileDeltaHt;
    if (_params.compute_profile_spacing_from_data) {
      lowerLimit = midHt - _profileDeltaHt*0.5;
      upperLimit = midHt + _profileDeltaHt*0.5;      
    }

    if (_params.debug > Params::DEBUG_VERBOSE) {
      cout << "lowerLimit " << lowerLimit << endl;
      cout << "midHt      " << midHt << endl;
      cout << "upperLimit " << upperLimit << endl;
      cout << "----------" << endl;
    }

    // initialize matrices
    
    for (int ii = 0; ii < nDivCoeff; ii++) {
      _pp[ii] = 0.0;
      _qq[ii] = 0.0;
      for (int jj = 0; jj < nDivCoeff; jj++) {
        _PP[ii][jj] = 0.0;
        _PPinverse[ii][jj] = 0.0;
      }
    }
    
    // look for rings within the height limits
    // and expand the search to get more cases as applicable
      
    vector<VelRing *> divRings;
    for (int ii = 0; ii < (int) _rings.size(); ii++) {
      VelRing *ring = &_rings[ii];
      if (ring->midHt >= lowerLimit && ring->midHt < upperLimit) {
        // save the level number
        ring->levelNum = ilevel;
        divRings.push_back(ring);
      }
    } // ii

    _profile.interp[ilevel].nrings = (int) divRings.size();
    if (divRings.size() < 3) {
      continue;
    }
      
    // loop through points
      
    for (int kk = 0; kk < (int) divRings.size(); kk++) {
      
      VelRing *ring = divRings[kk];

      // count how many other points are from same elevation angle
      // including this one
      
      double nk = 0;
      for (int mm = 0; mm < (int) divRings.size(); mm++) {
        if (ring->sweepNum == divRings[mm]->sweepNum) {
          nk++;
        }
      }
      ring->nk = nk;
      
      // compute the weight to be given to this ring
      
      double rngm = ring->midRange * 1000;
      double elevStar = ring->elevStar;
      double cosElev = cos(elevStar * DEG_TO_RAD);
      double vara1 = ring->aaVariance[1];
      double weight = ((rngm * rngm * cosElev * cosElev) / (4.0 * vara1 * nk));
      ring->weight = weight;
      
      // increment matrices
      
      double gg0 = 1.0;
      double gg1 = ring->elevCoeff;
      double YY = ring->YY;
      
      _qq[0] += weight * gg0 * YY;
      _qq[1] += weight * gg1 * YY;
      
      _PP[0][0] += weight * gg0 * gg0;
      _PP[0][1] += weight * gg0 * gg1;
      _PP[1][0] += weight * gg1 * gg0;
      _PP[1][1] += weight * gg1 * gg1;
      
    } // kk
    
    // invert the PP matrix
    
    _invertPP();
    
    // compute divergence and precip vertical velocity
    
    _pp[0] = _PPinverse[0][0] * _qq[0] + _PPinverse[0][1] * _qq[1];
    _pp[1] = _PPinverse[1][0] * _qq[0] + _PPinverse[1][1] * _qq[1];
    
    double div = _pp[0];
    double wp = _pp[1];
    
    _profile.interp[ilevel].div = div;
    _profile.interp[ilevel].wp = wp;
    
    // compute error of estimate
    
    double sumErrorNum = 0.0;
    double sumErrorDenom = 0.0;
    
    for (int kk = 0; kk < (int) divRings.size(); kk++) {
      VelRing *ring = divRings[kk];
      double gg0 = 1.0;
      double gg1 = ring->elevCoeff;
      double yyEst = _pp[0] * gg0 + _pp[1] * gg1;
      double yyError = (ring->YY - yyEst);
      sumErrorNum += ring->weight * yyError * yyError;
      sumErrorDenom += 1.0 / ring->nk;
    }
    
    double denom = sumErrorDenom - 2.0;
    if (denom < 1) {
      denom = 1;
    }

    double divS2 = sumErrorNum / denom;

    _profile.interp[ilevel].varDiv = divS2;
    _profile.interp[ilevel].rmsDiv = sqrt(divS2);

    double rho = _profile.interp[ilevel].rho;
    double varRhoD = rho * rho * divS2 * _PPinverse[1][1];
    _profile.interp[ilevel].varRhoD = varRhoD;
    _profile.interp[ilevel].rmsRhoD = sqrt(varRhoD);
    
  } // ilevel

}

///////////////////////////////////////////////////
// interpolate divergence across missing values
  
void RadxEvad::_interpDivergence()

{

  // find start and end heights
  
  int ibot = _profileNLevels;
  for (int ilev = 0; ilev < _profileNLevels; ilev++) {
    const ProfilePt &pt = _profile.interp[ilev];
    if (pt.rmsDiv > missingVal) {
      ibot = ilev;
      break;
    }
  }
  
  int itop = 0;
  for (int ilev = _profileNLevels - 1; ilev >= 0; ilev--) {
    const ProfilePt &pt = _profile.interp[ilev];
    if (pt.rmsDiv > missingVal) {
      itop = ilev;
      break;
    }
  }

  _ibotDiv = ibot;
  _itopDiv = itop;

  if (itop - ibot < 2) {
    return;
  }

  for (int ilev = ibot + 1; ilev < itop; ilev++) {

    const ProfilePt &pt = _profile.interp[ilev];
    if (pt.rmsDiv > missingVal) {
      // good data, no need to interpolate
      continue;
    }

    // find point below

    int ibelow = ibot;
    for (int jlev = ilev - 1; jlev >= ibot; jlev--) {
      const ProfilePt &ptBelow = _profile.interp[jlev];
      if (ptBelow.rmsDiv > missingVal) {
        ibelow = jlev;
        break;
      }
    }

    // find point above
    
    int iabove = itop;
    for (int jlev = ilev + 1; jlev < itop; jlev++) {
      const ProfilePt &ptAbove = _profile.interp[jlev];
      if (ptAbove.rmsDiv > missingVal) {
        iabove = jlev;
        break;
      }
    }

    // interpolate between points above and below
    
    const ProfilePt &ptBelow = _profile.interp[ibelow];
    const ProfilePt &ptAbove = _profile.interp[iabove];

    double htThis = pt.ht;
    double htBelow = ptBelow.ht;
    double htAbove = ptAbove.ht;
    double htInterp = htAbove - htBelow;

    double wtBelow = (htAbove - htThis) / htInterp;
    double wtAbove = 1.0 - wtBelow;

    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "interp-> htThis, htBelow, HtAbove, htInterp, wtBelow, wtAbove: "
           << htThis << ", " <<  htBelow << ", " <<  htAbove << ", "
           <<  htInterp << ", " <<  wtBelow << ", " <<  wtAbove << endl;
    }

    ProfilePt &interpPt = _profile.interp[ilev];

    interpPt.wp = wtBelow * ptBelow.wp + wtAbove * ptAbove.wp;
    interpPt.div = wtBelow * ptBelow.div + wtAbove * ptAbove.div;
    interpPt.varDiv = wtBelow * ptBelow.varDiv + wtAbove * ptAbove.varDiv;
    interpPt.varRhoD = wtBelow * ptBelow.varRhoD + wtAbove * ptAbove.varRhoD;

    interpPt.rmsDiv = sqrt(interpPt.varDiv);
    interpPt.rmsRhoD = sqrt(interpPt.varRhoD);

  } // ilev

}

///////////////////////////////////////////////////
// compute vertical velocity, given wtop
  
void RadxEvad::_computeVertVel(double wtop)

{

  // assume wtop (0 for now)

  double rhoTop =  _profile.interp[_itopDiv].rho;
  double dz = _profileDeltaHt * 1000.0;
  
  // compute fixed terms
  
  double flux = 0.0;
  double sumVarRhoD = 0.0;
  for (int ii = _ibotDiv; ii <= _itopDiv; ii++) {
    const ProfilePt &pt = _profile.interp[ii];
    flux += pt.rho * pt.div * dz;
    sumVarRhoD += pt.varRhoD * dz;
  }
  
  // compute modified divergence
  
  for (int ii = _ibotDiv; ii <= _itopDiv; ii++) {
    ProfilePt &pt = _profile.interp[ii];
    double correction =
      (1.0 / pt.rho) * (pt.varRhoD / sumVarRhoD) * (rhoTop * wtop + flux);
    pt.divPrime = pt.div - correction;
  }
  
  // integrate to get w
  
  double sumW = 0.0;
  for (int ii = _ibotDiv; ii <= _itopDiv; ii++) {
    ProfilePt &pt = _profile.interp[ii];
    sumW += pt.rho * pt.divPrime * dz;
    pt.ww = (1.0 / pt.rho) * -1.0 * sumW;
  }
  
}

///////////////////////////////////////////////////
// compute fourier coefficients
  
void RadxEvad::_computeFf(double az, double *ff)

{

  double azRad = az * DEG_TO_RAD;


  ff[0] = 1.0;
  ff[1] = sin(azRad);
  ff[2] = cos(azRad);
  ff[3] = sin(azRad * 2.0);
  ff[4] = cos(azRad * 2.0);
  ff[5] = sin(azRad * 3.0);
  ff[6] = cos(azRad * 3.0);

}

///////////////////////////////////////////////////
// invert AA matrix

void RadxEvad::_invertAA()

{

  double aa[nFourierCoeff * nFourierCoeff];
  int kk = 0;
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    for (int jj = 0; jj < nFourierCoeff; jj++, kk++) {
      aa[kk] = _AA[ii][jj];
    }
  }

  _invertMatrix(aa, nFourierCoeff);

  kk = 0;
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    for (int jj = 0; jj < nFourierCoeff; jj++, kk++) {
      _AAinverse[ii][jj] = aa[kk];
    }
  }

}

///////////////////////////////////////////////////
// invert PP matrix

void RadxEvad::_invertPP()

{

  double pp[nDivCoeff * nDivCoeff];
  int kk = 0;
  for (int ii = 0; ii < nDivCoeff; ii++) {
    for (int jj = 0; jj < nDivCoeff; jj++, kk++) {
      pp[kk] = _PP[ii][jj];
    }
  }

  _invertMatrix(pp, nDivCoeff);

  kk = 0;
  for (int ii = 0; ii < nDivCoeff; ii++) {
    for (int jj = 0; jj < nDivCoeff; jj++, kk++) {
      _PPinverse[ii][jj] = pp[kk];
    }
  }

}

///////////////////////////////////////////////////
// invert square matrix, in place

void RadxEvad::_invertMatrix(double *data, int nn) const
{

  if (data[0] != 0.0) {
    for (int i=1; i < nn; i++) {
      data[i] /= data[0]; // normalize row 0
    }
  }

  for (int i=1; i < nn; i++)  { 

    for (int j=i; j < nn; j++)  { // do a column of L
      double sum = 0.0;
      for (int k = 0; k < i; k++) {
	sum += data[j*nn+k] * data[k*nn+i];
      }
      data[j*nn+i] -= sum;
    } // j

    if (i == nn-1) continue;

    for (int j=i+1; j < nn; j++)  {  // do a row of U
      double sum = 0.0;
      for (int k = 0; k < i; k++) {
	sum += data[i*nn+k]*data[k*nn+j];
      }
      data[i*nn+j] = (data[i*nn+j]-sum) / data[i*nn+i];
    }

  } // i

  // invert L
  
  for ( int i = 0; i < nn; i++ ) {
    
    for ( int j = i; j < nn; j++ )  {
      double x = 1.0;
      if ( i != j ) {
	x = 0.0;
	for ( int k = i; k < j; k++ ) {
	  x -= data[j*nn+k]*data[k*nn+i];
	}
      }
      data[j*nn+i] = x / data[j*nn+j];
    } // j

  }

  // invert U

  for ( int i = 0; i < nn; i++ ) {
    for ( int j = i; j < nn; j++ )  {
      if ( i == j ) continue;
      double sum = 0.0;
      for ( int k = i; k < j; k++ ) {
	sum += data[k*nn+j]*( (i==k) ? 1.0 : data[i*nn+k] );
      }
      data[i*nn+j] = -sum;
    }
  }
  
  // final inversion
  for ( int i = 0; i < nn; i++ ) {
    for ( int j = 0; j < nn; j++ )  {
      double sum = 0.0;
      for ( int k = ((i>j)?i:j); k < nn; k++ ) {
	sum += ((j==k)?1.0:data[j*nn+k])*data[k*nn+i];
      }
      data[j*nn+i] = sum;
    }
  }

}

////////////////////////////////////////////////
// compute the norm1 of the matrix
// this is the maximum absolute column sum

double RadxEvad::_norm(double **a,int n)

{

  double maxColSum = 0.0;

  for (int ii = 0; ii < n; ii++) {

    double colSum = 0.0;
    for (int jj = 0; jj < n; jj++) {
      colSum += fabs(a[ii][jj]);
    }
    
    if (colSum > maxColSum) {
      maxColSum = colSum;
    }

  }

  return maxColSum;

}


///////////////////////////////////////////////////
// print results for the ring
  
void RadxEvad::_printResultsForRing(ostream &out,
                                   const VelRing &ring)

{

  out << "==================== results  for ring ========================" << endl;
  
  out << "  sweepNum: " << ring.sweepNum << endl;
  out << "  rangeNum: " << ring.rangeNum << endl;
  out << "  startGate: " << ring.startGate << endl;
  out << "  endGate: " << ring.endGate << endl;
  out << "  elev: " << ring.elev << endl;
  out << "  elevStar: " << ring.elevStar << endl;
  out << "  elevCoeff: " << ring.elevCoeff << endl;
  out << "  midHt: " << ring.midHt << endl;
  out << "  startRange: " << ring.startRange << endl;
  out << "  endRange: " << ring.endRange << endl;
  out << "  midRange: " << ring.midRange << endl;
  out << "  fitVariance: " << ring.fitVariance << endl;
  out << "  fitRmsError: " << ring.fitRmsError << endl;
  out << "  maxVr: " << ring.maxVr << endl;
  out << "  minVr: " << ring.minVr << endl;
  out << "  azForMaxVr: " << ring.azForMaxVr << endl;
  out << "  azForMinVr: " << ring.azForMinVr << endl;
  out << "  azError: " << ring.azError << endl;
  out << "  windSpeed: " << ring.windSpeed << endl;
  out << "  windDirn: " << ring.windDirn << endl;
  out << "  windValid: " << ring.windValid << endl;
  out << "  aa:";
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    out << " " << ring.aa[ii];
  }
  out << endl;
  out << "  aaVariance:";
  for (int ii = 0; ii < nFourierCoeff; ii++) {
    out << " " << ring.aaVariance[ii];
  }
  out << endl;
  out << "  YY: " << ring.YY << endl;
  out << "===============================================================" << endl;
  
}

//////////////////////////////
// write Output data to netcdf

int RadxEvad::_writeNetcdfOutput()
  
{

  PMU_auto_register("Before netcdf write");

  // compute output file name
  
  RadxTime fileTime(_readVol.getStartTimeSecs());

  string outDir(_params.output_netcdf_dir);
  if (_params.append_year_dir_to_output_dir) {
    char yearStr[BUFSIZ];
    sprintf(yearStr, "%s%.4d", PATH_DELIM, fileTime.getYear());
    outDir += yearStr;
  }
  if (_params.append_day_dir_to_output_dir) {
    char dayStr[BUFSIZ];
    sprintf(dayStr, "%s%.4d%.2d%.2d", PATH_DELIM,
            fileTime.getYear(), fileTime.getMonth(), fileTime.getDay());
    outDir += dayStr;
  }

  // make sure output subdir exists
  
  if (ta_makedir_recurse(outDir.c_str())) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << "  Cannot create output dir: " << outDir << endl;
    return -1;
  }
  
  // compute file name
  
  char fileName[128];
  snprintf(fileName, 128,
           "profile.%.4d%.2d%.2d_%.2d%.2d%.2d.%s.nc",
           fileTime.getYear(), fileTime.getMonth(), fileTime.getDay(),
           fileTime.getHour(), fileTime.getMin(), fileTime.getSec(),
           _radarName.c_str());
  
  char outPath[BUFSIZ];
  snprintf(outPath, BUFSIZ, "%s%s%s",
           outDir.c_str(), PATH_DELIM,  fileName);

  // open file for writing

  Nc3xFile file;
  if (file.openWrite(outPath, Nc3File::Netcdf4)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << "  Cannot open netCDF file: " << outPath << endl;
    return -1;
  }

  // add global attributes

  if (file.addGlobAttr("featureType", "profile")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }

  // add dimensions
  
  int nZ = _getNValidLevels();
  if (_params.write_data_from_all_levels) {
    nZ = _profileNLevels;
  }

  Nc3Dim *zDim;
  if (file.addDim(zDim, "z", nZ)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }

  // add variables

  Nc3Var *profileVar;
  if (file.addMetaVar(profileVar, "profile", "profile_id", "", nc3Int)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(profileVar, "profile_role", "profile_id");
  
  Nc3Var *timeVar;
  if (file.addMetaVar(timeVar, "time", "time", "", nc3Double, 
                      "seconds since 1970-01-01 00:00:0")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(timeVar, "long_name", "time");
  
  Nc3Var *lonVar;
  if (file.addMetaVar(lonVar, "lon", "longitude", "", nc3Double, 
                      "degrees_east")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(lonVar, "long_name", "longitude_of_radar");
  
  Nc3Var *latVar;
  if (file.addMetaVar(latVar, "lat", "latitude", "", nc3Double, 
                      "degrees_north")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(latVar, "long_name", "latitude_of_radar");
  
  Nc3Var *altVar;
  if (file.addMetaVar(altVar, "alt", "altitude", "", nc3Double, "km")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(altVar, "long_name", "altitude_of_radar");
  
  Nc3Var *zVar;
  if (file.addMetaVar(zVar, "z", "altitude", "", nc3Float, zDim, "km")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(zVar, "long_name", "height above radar");
  file.addAttr(zVar, "positive", "up");
  file.addAttr(zVar, "azis", "Z");


  Nc3Var *uVar;
  if (file.addMetaVar(uVar, "u", "eastward_wind", "", nc3Float, zDim, "m s-1")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(uVar, "long_name", "wind speed eastward positive");
  file.addAttr(uVar, "_fillValue", missingVal);

  Nc3Var *vVar;
  if (file.addMetaVar(vVar, "v", "northward_wind", "", nc3Float, zDim, "m s-1")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(vVar, "long_name", "wind speed northward positive");
  file.addAttr(vVar, "_fillValue", missingVal);

  Nc3Var *wVar;
  if (file.addMetaVar(wVar, "w", "upward_wind", "", nc3Float, zDim, "m s-1")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(wVar, "long_name", "wind speed upward positive");
  file.addAttr(wVar, "_fillValue", missingVal);

  Nc3Var *divVar;
  if (file.addMetaVar(divVar, "div", "divergence_of_wind", "", 
                      nc3Float, zDim, "s-1")) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  file.addAttr(divVar, "long_name", "divergence");
  file.addAttr(divVar, "_fillValue", missingVal);

  // write the scalar variables

  double vtime = _readVol.getStartTimeSecs();
  if (file.writeVar(timeVar, vtime)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  if (file.writeVar(lonVar, _radarLongitude)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  if (file.writeVar(latVar, _radarLatitude)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  if (file.writeVar(altVar, _radarAltitude)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  // set up arrays with the data
  
  TaArray<float> ht_;
  TaArray<float> uu_;
  TaArray<float> vv_;
  TaArray<float> ww_;
  TaArray<float> div_;
  float *ht = ht_.alloc(nZ);
  float *uu = uu_.alloc(nZ);
  float *vv = vv_.alloc(nZ);
  float *ww = ww_.alloc(nZ);
  float *div = div_.alloc(nZ);

  for (int iz = 0; iz < nZ; iz++) {
    const ProfilePt &pt = _profile.interp[iz];
    ht[iz] = pt.ht;
    uu[iz] = pt.uu;
    vv[iz] = pt.vv;
    ww[iz] = pt.ww;
    if (pt.div == missingVal) {
      div[iz] = pt.div;
    } else {
      div[iz] = pt.div * 1.0e5;
    }
  }
  
  if (file.writeVar(zVar, zDim, ht)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  if (file.writeVar(uVar, zDim, uu)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  if (file.writeVar(vVar, zDim, vv)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  if (file.writeVar(wVar, zDim, ww)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  if (file.writeVar(divVar, zDim, div)) {
    cerr << "ERROR - RadxEvad::_writeNetcdfOutput" << endl;
    cerr << file.getErrStr() << endl;
    return -1;
  }
  
  // close

  file.close();

  if (_params.debug) {
    cerr << "Wrote netcdf file: " << file.getPathInUse() << endl;
  }

  // write latest data info file

  _writeLdataInfo(outPath);

  return 0;

}
  
////////////////////////////
// write Output data to spdb

int RadxEvad::_writeSpdbOutput()
  
{
  
  PMU_auto_register("Before spdb write");

  // initialize sounding put object

  SoundingPut sndg;
  sndg.init(_params.output_spdb_url,
            Sounding::VAD_ID,
            "RadxEvad",
            Spdb::hash4CharsToInt32(_radarName.c_str()),
            _radarName.c_str(),
            _radarLatitude,
            _radarLongitude,
            _radarAltitude,
            missingVal);
  
  // set up vectors with the data
  
  vector<double> ht;
  vector<double> uu;
  vector<double> vv;
  vector<double> ww;
  vector<double> div;

  int nZ = _getNValidLevels();
  if (_params.write_data_from_all_levels) {
    nZ = _profileNLevels;
  }
  if (nZ == 0) {
    nZ = 1;
  }
  
  for (int iz = 0; iz < nZ; iz++) {
    const ProfilePt &pt = _profile.interp[iz];
    ht.push_back(pt.ht);
    uu.push_back(pt.uu);
    vv.push_back(pt.vv);
    ww.push_back(pt.ww);
    if (pt.div == missingVal) {
      div.push_back(pt.div);
    } else {
      div.push_back(pt.div * 1.0e5);
    }
  }
  
  time_t vtime = _readVol.getStartTimeSecs();
  
  if (sndg.set(vtime, &ht, &uu, &vv, &ww, NULL, NULL, NULL, &div)) {
    cerr << "ERROR - RadxEvad::_writeOutput" << endl;
    cerr << "  Cannot set data in output sounding object" << endl;
    return -1;
  }

  if (sndg.writeSounding(vtime, vtime + _params.output_spdb_valid_period_secs)) {
    cerr << "ERROR - RadxEvad::_writeOutput" << endl;
    cerr << "  Cannot write sounding object" << endl;
    cerr << sndg.getSpdbMgr().getErrStr() << endl;
  }

  if (_params.debug) {
    cerr << "Wrote spdb data, URL: " << _params.output_spdb_url << endl;
    cerr << "          Valid time: " << RadxTime::strm(vtime) << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////
// get number of levels with valid data

int RadxEvad::_getNValidLevels()
{
  int nValid = 1;
  for (int iz = 1; iz < _profileNLevels; iz++) {
    const ProfilePt &pt = _profile.interp[iz];
    if (pt.uu != missingVal ||
        pt.vv != missingVal ||
        pt.ww != missingVal ||
        pt.div != missingVal) {
      nValid = iz + 1;
    }
  }
  // return _profileNLevels;
  return nValid;
}
  
/////////////////////////////////////////////////////
// define functions to be used for sorting

int RadxEvad::_doubleCompare(const void *i, const void *j)
{
  double *f1 = (double *) i;
  double *f2 = (double *) j;
  if (*f1 < *f2) {
    return -1;
  } else if (*f1 > *f2) {
    return 1;
  } else {
    return 0;
  }
}

//////////////////////////////////////
// Write LdataInfo file

void RadxEvad::_writeLdataInfo(const string &outputPath)
{

  DsLdataInfo ldata(_params.output_netcdf_dir, _params.debug);
  
  ldata.setWriter("RadxEvad");
  ldata.setDataFileExt("nc");
  ldata.setDataType("netCDF");

  string fileName;
  Path::stripDir(_params.output_netcdf_dir, outputPath, fileName);
  ldata.setRelDataPath(fileName);
  
  ldata.setIsFcast(false);
  ldata.write(_readVol.getStartTimeSecs());
  
  if (_params.debug) {
    cerr << "RadxEvad::_writeLdataInfo(): Data written to "
         << outputPath << endl;
  }

}

