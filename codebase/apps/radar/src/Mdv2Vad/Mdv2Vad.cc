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
// Mdv2Vad.cc
//
// Mdv2Vad object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2009
//
///////////////////////////////////////////////////////////////
//
// Mdv2Vad reads in Doppler data from an MDV polar radar file,
// computes volumetric VAD (VVP) winds and writes them out to SPDB.
//
// The implementation in Mdv2Vad is based on the paper 'An Improved
// Version of the Extended Velocity-Azimuth Display Analysis of
// Single-Doppler Radar Data' by Thomas Metejka and
// Ramesh C. Srivastava, Journal of Atmospheric and Oceanic Technology,
// Vol 8, No 4, August 1991.
//
// The code is designed to match the terminology in the paper as
// fas as is posible. Please refer to the paper for a detailed
// explanation of the method.
//
////////////////////////////////////////////////////////////////

#include <algorithm>
#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <rapmath/trig.h>
#include <rapmath/RapComplex.hh>
#include <physics/IcaoStdAtmos.hh>
#include <physics/thermo.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <Spdb/SoundingPut.hh>
#include "Mdv2Vad.hh"
using namespace std;

const double Mdv2Vad::missingVal = SNDG_VALUE_UNKNOWN;
const double Mdv2Vad::pseudoEarthDiamKm = 17066.0;

// Constructor

Mdv2Vad::Mdv2Vad(int argc, char **argv)

{

  isOK = true;

  // pointers into MDV volume

  _dbz = NULL;
  _snr = NULL;
  _vel = NULL;
  _censor = NULL;
  _AA = NULL;
  _AAinverse = NULL;

  // geometry

  _mdvStartRange = 0.0;
  _mdvGateSpacing = 0.0;
  _mdvMinAz = 0.0;
  _mdvDeltaAz = 0.0;

  // set programe name

  _progName = "Mdv2Vad";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // check that start and end time is set in archive mode

  if (_params.mode == Params::ARCHIVE) {
    if (_args.startTime == 0 || _args.endTime == 0) {
      cerr << "ERROR - must specify start and end dates." << endl << endl;
      _args.usage(_progName, cerr);
      isOK = false;
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

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_dir, 600,
			   PMU_auto_register)) {
      isOK = false;
    }
  } else if (_params.mode == Params::ARCHIVE) {
    if (_input.setArchive(_params.input_dir,
			  _args.startTime,
			  _args.endTime)) {
      isOK = false;
    }
  } else if (_params.mode == Params::FILELIST) {
    if (_input.setFilelist(_args.inputFileList)) {
      isOK = false;
    }
  }

  return;

}

// destructor

Mdv2Vad::~Mdv2Vad()

{

  if (_AA) {
    ufree2((void **) _AA);
  }

  if (_AAinverse) {
    ufree2((void **) _AAinverse);
  }

  if (_PP) {
    ufree2((void **) _PP);
  }

  if (_PPinverse) {
    ufree2((void **) _PPinverse);
  }

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int Mdv2Vad::Run ()
{

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "_nAzSlices, _sliceDeltaAz: "
         << _nAzSlices << ", " << _sliceDeltaAz << endl;
  }

  // register with procmap
  
  PMU_auto_register("Run");
  
  // loop until end of data
  
  while (!_input.endOfData()) {
    
    PMU_auto_register("In main loop");

    // read the next MDV volume
    
    if (_readNextVolume()) {
      continue;
    }
    
    // check for uniform field geometry
    // Sets _nGates, _nBeams, _nTilts
    
    if (!_checkFields()) {
      cerr << "ERROR - Mdv2Vad::Run" << endl;
      cerr << "  Ignoring file because fields do not have same geometry." << endl;
      cerr << "  File: " << _mdvx.getPathInUse() << endl;
      continue;
    }
    
    // set array pointers to the input field data
    
    if (_setInputPointers()) {
      cerr << "ERROR - Mdv2Vad::Run()" << endl;
      cerr << "  Cannot set input pointers" << endl;
      continue;
    }
    
    // process this input data set
    
    _processDataSet();
    
    // write output

    if (_writeOutput()) {
      cerr << "ERROR - Mdv2Vad::Run()" << endl;
      cerr << "  Cannot write output data" << endl;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////
// set up the read for the next MDV file

void Mdv2Vad::_setupRead()
  
{

  _mdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  _mdvx.clearRead();
  
  if (_params.set_elev_limits) {
    _mdvx.setReadVlevelLimits(_params.lower_elev, _params.upper_elev);
  }

  _mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.SNR_available) {
    _mdvx.addReadField(_params.SNR_field_name);
  }
  _mdvx.addReadField(_params.DBZ_field_name);
  _mdvx.addReadField(_params.VEL_field_name);

  if (_params.censor_using_thresholds) {
    _mdvx.addReadField(_params.censor_field_name);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading input MDV file" << endl;
    _mdvx.printReadRequest(cerr);
  }

}

///////////////////////////
// read in next volume file

int Mdv2Vad::_readNextVolume()
  
{

  PMU_auto_register("Before read");

  // set up read
  
  _setupRead();

  // read the volume
  
  if (_input.readVolumeNext(_mdvx)) {
    cerr << "ERROR - Mdv2Vad::_readInput" << endl;
    cerr << "  Cannot read in MDV data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Working on file: " << _mdvx.getPathInUse() << endl;
  }

  // radar parameters

  _nyquist = _params.nyquist_velocity;
  _radarHtMeters = 0;

  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(_mdvx) == 0 && mdvxRadar.radarParamsAvail()) {
    
    const DsRadarParams &rparams = mdvxRadar.getRadarParams();
    _radarHtMeters = rparams.altitude * 1000.0;
    if (!_params.set_nyquist_velocity) {
      _nyquist = rparams.unambigVelocity;
    }

    _radarName = rparams.radarName;
    _radarLatitude = rparams.latitude;
    _radarLongitude = rparams.longitude;
    _radarAltitude = rparams.altitude;
  
  } else {

    _radarName = _params.radar_name;
    _radarLatitude = _params.radar_latitude;
    _radarLongitude = _params.radar_longitude;
    _radarAltitude = _params.radar_altitude;

  }

  if (_params.override_radar_name) {
    _radarName = _params.radar_name;
  }
  
  if (_params.override_radar_location) {
    _radarLatitude = _params.radar_latitude;
    _radarLongitude = _params.radar_longitude;
    _radarAltitude = _params.radar_altitude;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  nyquist velocity: " << _nyquist << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input file fields:" << endl;
    for (int ifield = 0;
	 ifield < _mdvx.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = _mdvx.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  return 0;

}

///////////////////////////////////////////
// check that the fields are uniform in size

bool Mdv2Vad::_checkFields()
  
{

  const Mdvx::master_header_t &mhdr = _mdvx.getMasterHeader();
  const MdvxField *fld0 = _mdvx.getField(0);
  const Mdvx::field_header_t &fhdr0 = fld0->getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr0 = fld0->getVlevelHeader();
  
  _nGates = fhdr0.nx;
  _nBeams = fhdr0.ny;
  _nTilts = fhdr0.nz;
  _nPointsTilt = _nGates * _nBeams;

  for (int i = 1; i < mhdr.n_fields; i++) {
    const MdvxField *fld = _mdvx.getField(i);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    int nGates = fhdr.nx;
    int nBeams = fhdr.ny;
    int nTilts = fhdr.nz;
    if (nGates != _nGates ||
	nBeams != _nBeams ||
	nTilts != _nTilts) {
      cerr << "ERROR - Mdv2Vad::_checkFields()" << endl;
      cerr << "  Fields are not uniform in geometry" << endl;
      cerr << "  Geometry in first field, name, nGates, nBeams, nTilts: "
	   << fhdr0.field_name << ", "
	   << _nGates << ", " << _nBeams << ", " << _nTilts << endl;
      cerr << "  Geometry in this field, name, nGates, nBeams, nTilts: "
	   << fhdr.field_name << ", "
	   << nGates << ", " << nBeams << ", " << nTilts << endl;
      cerr << "  File: " << _mdvx.getPathInUse() << endl;
      return false;
    }
    if (fhdr.proj_type != Mdvx::PROJ_POLAR_RADAR) {
      cerr << "ERROR - Mdv2Vad::_checkFields()" << endl;
      cerr << "  Fields are not in POLAR_RADAR projection" << endl;
      cerr << "  Projection is: " << Mdvx::projType2Str(fhdr.proj_type) << endl;
      cerr << "  This field name: " << fhdr.field_name << endl;
      cerr << "  File: " << _mdvx.getPathInUse() << endl;
      return false;
    }
  }

  _mdvStartRange = fhdr0.grid_minx;
  _mdvGateSpacing = fhdr0.grid_dx;

  _mdvMinAz = fhdr0.grid_miny;
  _mdvDeltaAz = fhdr0.grid_dy;

  _elevs.clear();
  for (int ii = 0; ii < _nTilts; ii++) {
    _elevs.push_back(vhdr0.level[ii]);
  }

  return true;

}

///////////////////////////////////////////
// set up the input pointers

int Mdv2Vad::_setInputPointers()
  
{

  const MdvxField *dbzFld = _mdvx.getField(_params.DBZ_field_name);
  if (dbzFld == NULL) {
    cerr << "ERROR - Mdv2Vad::_setInputPointers()" << endl;
    cerr << "  Cannot find DBZ field, name: " << _params.DBZ_field_name << endl;
    return -1;
  }
  _dbz = (fl32 *) dbzFld->getVol();
  _dbzMiss = dbzFld->getFieldHeader().missing_data_value;

  if (_params.SNR_available) {
    const MdvxField *snrFld = _mdvx.getField(_params.SNR_field_name);
    if (snrFld == NULL) {
      cerr << "ERROR - Mdv2Vad::_setInputPointers()" << endl;
      cerr << "  Cannot find SNR field, name: " << _params.SNR_field_name << endl;
      return -1;
    }
    _snr = (fl32 *) snrFld->getVol();
    _snrMiss = snrFld->getFieldHeader().missing_data_value;
  }
  
  const MdvxField *velFld = _mdvx.getField(_params.VEL_field_name);
  if (velFld == NULL) {
    cerr << "ERROR - Mdv2Vad::_setInputPointers()" << endl;
    cerr << "  Cannot find VEL field, name: " << _params.VEL_field_name << endl;
    return -1;
  }
  _vel = (fl32 *) velFld->getVol();
  _velMiss = velFld->getFieldHeader().missing_data_value;
  
  if (_params.censor_using_thresholds) {
    const MdvxField *censorFld = _mdvx.getField(_params.censor_field_name);
    if (censorFld == NULL) {
      cerr << "ERROR - Mdv2Vad::_setInputPointers()" << endl;
      cerr << "  Cannot find censor field, name: " << _params.censor_field_name << endl;
      return -1;
    }
    _censor = (fl32 *) censorFld->getVol();
    _censorMiss = censorFld->getFieldHeader().missing_data_value;
  }

  return 0;

}

///////////////////////////////////////////
// process this data set

void Mdv2Vad::_processDataSet()
  
{

  if (_params.debug) {
    cerr << "Processing data set ..." << endl;
  }

  // clear ring array

  _rings.clear();

  for (int iel = 0; iel < (int) _elevs.size(); iel++) {

    if (_elevs[iel] < _params.min_elev || _elevs[iel] > _params.max_elev) {
      if (_params.debug) {
	cerr << "Ignoring tilt, elev out of range: " << _elevs[iel] << endl;
      }
      continue;
    }

    _nRanges = (int) ((_params.max_range - _params.min_range) /
                      _params.delta_range + 0.5);

    for (int irange = 0; irange < _nRanges; irange++) {

      // set up ring info

      _ring.clear();
      _ring.elNum = iel;
      _ring.rangeNum = irange;
      
      // compute gates to be used
      
      _ring.startRange = _params.min_range + irange * _params.delta_range;
      _ring.endRange = _ring.startRange + _params.delta_range;

      _ring.startGate = (int) ((_ring.startRange - _mdvStartRange) / _mdvGateSpacing + 0.5);
      if (_ring.startGate < 0) {
	_ring.startGate = 0;
      }

      _ring.endGate = (int) ((_ring.endRange - _mdvStartRange) / _mdvGateSpacing + 0.5);
      if (_ring.endGate > _nGates - 1) {
	_ring.endGate = _nGates - 1;
      }

      // set elevation, compute range and height at ring mid-pt

      _ring.elev = _elevs[_ring.elNum];
      _ring.midRange = (_ring.startRange + _ring.endRange) / 2.0;
      _ring.midHt = (_ring.midRange * sin(_ring.elev * DEG_TO_RAD) +
                     (_ring.midRange * _ring.midRange / pseudoEarthDiamKm));

      // compute corrected elevation and coefficient for use in
      // computing divergence and vertical velocity

      double elevCorrectionRad = _ring.midRange / pseudoEarthDiamKm;
      _ring.elevStar = _ring.elev + elevCorrectionRad * RAD_TO_DEG;
      double rangeMeters = _ring.midRange * 1000.0;
      _ring.elevCoeff = ((2.0 * sin(_ring.elevStar * DEG_TO_RAD)) /
                         (rangeMeters * cos(_ring.elevStar * DEG_TO_RAD)));

      // compute wind solution for the ring
      
      _computeSolutionForRing();

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

}

///////////////////////////////////////////
// compute wind solution for a single ring

void Mdv2Vad::_computeSolutionForRing()
  
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

  // load up slices with velocity data

  double cosElev = cos(_ring.elev * DEG_TO_RAD);
  for (int iaz = 0; iaz < _nBeams; iaz++) {

    // compute azimuth

    double az = _mdvMinAz + iaz * _mdvDeltaAz;
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

    int gateIndex = _ring.elNum * _nPointsTilt + iaz * _nGates + _ring.startGate;
    for (int igate = _ring.startGate; igate <= _ring.endGate; igate++, gateIndex++) {
      double vel = _vel[gateIndex] * cosElev;
      double acceptVal = true;
      if (_censor != NULL) {
        double censorVal = _censor[gateIndex];
        if (censorVal < _params.censor_min_value ||
            censorVal > _params.censor_max_value) {
          acceptVal = false;
        }
      }
      if (acceptVal) {
        VelPt pt;
        pt.az = az;
        pt.vel = vel;
        _ring.slices[sliceNum].pts.push_back(pt);
      }
    } // igate

  } // iaz

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

}

///////////////////////////////////////////////////
// compute mean velocity for an azimuth slice
//
// This is done 'on the phase circle' - to take folding
// into account
  
void Mdv2Vad::_computeMeanVel(AzSlice &slice)

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
    rap_sincos(phaseRad, &sinPhase, &cosPhase);
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
  
void Mdv2Vad::_computeMedianVel(int sliceNum)

{

  AzSlice &slice = _ring.slices[sliceNum];

  // load up array of velocity values in vicinity of this slice

  int nHalfMedian = _params.n_slices_for_vel_median / 2;
  int nMedian = nHalfMedian * 2 + 1;

  TaArray<double> _velArray;
  double *vel = _velArray.alloc(nMedian);

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

void Mdv2Vad::_copyMedianIntoGaps()
  
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

void Mdv2Vad::_identifyFolds()
  
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
  
int Mdv2Vad::_unfoldIteratively()
  
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
  
int Mdv2Vad::_unfoldFromZeroIsodop(int zeroIsodopIndex)
  
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
  
void Mdv2Vad::_unfoldFromPreviousRing(double ht)
  
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

void Mdv2Vad::_computeWindForRing()
  
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
      cerr << "Mdv2Vad::_computewindForRing()" << endl;
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
  
void Mdv2Vad::_loadProfile()

{

  _profile.clear();

  // load up raw profile

  for (int ii = 0; ii < (int) _rings.size(); ii++) {
    const VelRing &ring = _rings[ii];
    ProfilePt pt;
    pt.ht = ring.midHt;
    pt.windSpeed = ring.windSpeed;
    pt.windDirn = ring.windDirn;
    pt.uu = pt.windSpeed * cos(pt.windDirn * DEG_TO_RAD);
    pt.vv = pt.windSpeed * sin(pt.windDirn * DEG_TO_RAD);
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
        dirn = atan2(meanVV, meanUU) * RAD_TO_DEG;
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
          dirn = atan2(vv, uu) * RAD_TO_DEG;
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
  
void Mdv2Vad::_computeDivergence()

{

  // find the rings within a given level and for a given range

  for (int ilevel = 0; ilevel < _profileNLevels; ilevel++) {

    double midHt = _profileMinHt + ilevel * _profileDeltaHt;
    double lowerLimit = midHt - _profileDeltaHt;
    double upperLimit = midHt + _profileDeltaHt;

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
        if (ring->elNum == divRings[mm]->elNum) {
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
  
void Mdv2Vad::_interpDivergence()

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
  
void Mdv2Vad::_computeVertVel(double wtop)

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
  
void Mdv2Vad::_computeFf(double az, double *ff)

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

void Mdv2Vad::_invertAA()

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

void Mdv2Vad::_invertPP()

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

void Mdv2Vad::_invertMatrix(double *data, int nn) const
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

double Mdv2Vad::_norm(double **a,int n)

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
  
void Mdv2Vad::_printResultsForRing(ostream &out,
                                   const VelRing &ring)

{

  out << "==================== results  for ring ========================" << endl;
  
  out << "  elNum: " << ring.elNum << endl;
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

///////////////////////////
// write Output data

int Mdv2Vad::_writeOutput()
  
{
  
  PMU_auto_register("Before write");

  // initialize sounding put object

  SoundingPut sndg;
  sndg.init(_params.output_spdb_url,
            Sounding::VAD_ID,
            "Mdv2Vad",
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

  for (int ilev = 0; ilev < _profileNLevels; ilev++) {
    const ProfilePt &pt = _profile.interp[ilev];
    if (ilev == 0 ||
        pt.uu != missingVal ||
        pt.vv != missingVal ||
        pt.ww != missingVal ||
        pt.div != missingVal) {
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
  }
  
  time_t vtime = _mdvx.getValidTime();
  
  if (sndg.set(vtime, &ht, &uu, &vv, &ww, NULL, NULL, NULL, &div)) {
    cerr << "ERROR - Mdv2Vad::_writeOutput" << endl;
    cerr << "  Cannot set data in output sounding object" << endl;
    return -1;
  }

  if (sndg.writeSounding(vtime, vtime + _params.valid_period_secs)) {
    cerr << "ERROR - Mdv2Vad::_writeOutput" << endl;
    cerr << "  Cannot write sounding object" << endl;
    cerr << sndg.getSpdbMgr().getErrStr() << endl;
  }

  if (_params.debug) {
    cerr << "Wrote spdb data, URL: " << _params.output_spdb_url << endl;
    cerr << "          Valid time: " << DateTime::strm(vtime) << endl;
  }

  return 0;

}

/////////////////////////////////////////////////////
// define functions to be used for sorting

int Mdv2Vad::_doubleCompare(const void *i, const void *j)
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
