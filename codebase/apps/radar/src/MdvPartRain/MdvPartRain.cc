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
// MdvPartRain.cc
//
// MdvPartRain object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// MdvPartRain reads dual pol data from an MDV file, computes
// derived fields and writes the fields to an output file.
//
///////////////////////////////////////////////////////////////

#include <dataport/bigend.h>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/TaArray.hh>
#include <toolsa/toolsa_macros.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <Mdv/MdvxRadar.hh>
#include <Spdb/SoundingGet.hh>
#include <rapformats/Sndg.hh>
#include <radar/FilterUtils.hh>
#include <radar/TempProfile.hh>
#include "MdvPartRain.hh"
using namespace std;

const double MdvPartRain::missingDouble = -9999.0;
const fl32 MdvPartRain::missingFloat = -9999.0;
const fl32 MdvPartRain::pseudoEarthDiamKm = 17066.0;

// Constructor

MdvPartRain::MdvPartRain(int argc, char **argv)

{

  isOK = true;

  // pointers into MDV volume

  _dbz = NULL;
  _snr = NULL;
  _zdr = NULL;
  _phidp = NULL;
  _rhohv = NULL;
  _kdp = NULL;

  // geometry

  _startRange = 0.0;
  _gateSpacing = 0.0;
  _isPolarRadar = false;
  _isRhiRadar = false;
  _isPpiCart = false;
  _vlevelInKm = false;

  // set programe name

  _progName = "MdvPartRain";
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

  // initialize radar params

  _wavelengthCm = _params.wavelength_cm;
  _radarHtKm = _params.radar_height_km;
  
  // initialize KDP object

  if (_params.apply_median_filter_to_PHIDP) {
    _kdpBringi.setApplyMedianFilterToPhidp(_params.PHIDP_median_filter_len);
  }
  if (_params.KDP_fir_filter_len == Params::FIR_LEN_125) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_125);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_30) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_30);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_20) {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_20);
  } else {
    _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_10);
  }
  _kdpBringi.setPhidpDiffThreshold(_params.KDP_phidp_difference_threshold);
  _kdpBringi.setPhidpSdevThreshold(_params.KDP_phidp_sdev_threshold);
  _kdpBringi.setZdrSdevThreshold(_params.KDP_zdr_sdev_threshold);
  _kdpBringi.setRhohvWxThreshold(_params.KDP_rhohv_threshold);
  _kdpBringi.setWavelengthCm(_wavelengthCm);
  
  // initialize precip rate object

  _rate.setZhAa(_params.zh_aa);
  _rate.setZhBb(_params.zh_bb);

  _rate.setZzdrAa(_params.zzdr_aa);
  _rate.setZzdrBb(_params.zzdr_bb);
  _rate.setZzdrCc(_params.zzdr_cc);
  
  _rate.setKdpAa(_params.kdp_aa);
  _rate.setKdpBb(_params.kdp_bb);

  _rate.setKdpZdrAa(_params.kdpzdr_aa);
  _rate.setKdpZdrBb(_params.kdpzdr_bb);
  _rate.setKdpZdrCc(_params.kdpzdr_cc);
  
  _rate.setHybridDbzThreshold(_params.hybrid_dbz_threshold);
  _rate.setHybridKdpThreshold(_params.hybrid_kdp_threshold);
  _rate.setHybridZdrThreshold(_params.hybrid_zdr_threshold);

  _rate.setMinValidRate(_params.min_valid_rate);
  _rate.setWavelengthCm(_wavelengthCm);

  // initialize particle ID object

  if (_params.apply_median_filter_to_DBZ) {
    _rate.setApplyMedianFilterToDbz(_params.DBZ_median_filter_len);
    _partId.setApplyMedianFilterToDbz(_params.DBZ_median_filter_len);
  }
  if (_params.apply_median_filter_to_ZDR) {
    _rate.setApplyMedianFilterToZdr(_params.ZDR_median_filter_len);
    _partId.setApplyMedianFilterToZdr(_params.ZDR_median_filter_len);
  }
  if (_params.apply_median_filter_to_LDR) {
    _partId.setApplyMedianFilterToLdr(_params.LDR_median_filter_len);
  }
  if (_params.replace_missing_LDR) {
    _partId.setReplaceMissingLdr(_params.LDR_replacement_value);
  }
  if (_params.apply_median_filter_to_RHOHV) {
    _partId.setApplyMedianFilterToRhohv(_params.RHOHV_median_filter_len);
  }
  if (_params.apply_median_filter_to_PID) {
    _partId.setApplyMedianFilterToPid(_params.PID_median_filter_len);
  }

  _partId.setNgatesSdev(_params.ngates_for_sdev);
  _partId.setMinValidInterest(_params.PID_min_valid_interest);
  _partId.setSnrThresholdDb(_params.PID_snr_threshold);
  _partId.setSnrUpperThresholdDb(_params.PID_snr_upper_threshold);
  _partId.setWavelengthCm(_wavelengthCm);

  // init process mapper registration

  PMU_auto_init((char *) _progName.c_str(),
		_params.instance,
		PROCMAP_REGISTER_INTERVAL);

  // initialize the data input object

  if (_params.mode == Params::REALTIME) {
    if (_input.setRealtime(_params.input_dir, _params.max_valid_age,
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

MdvPartRain::~MdvPartRain()

{

  // unregister process

  PMU_auto_unregister();

}

//////////////////////////////////////////////////
// Run

int MdvPartRain::Run ()
{

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
      cerr << "ERROR - MdvPartRain::Run" << endl;
      cerr << "  Ignoring file because fields do not have same geometry." << endl;
      cerr << "  File: " << _inMdvx.getPathInUse() << endl;
      continue;
    }
    
    // set array pointers to the input field data
    
    if (_setInputPointers()) {
      cerr << "ERROR - MdvPartRain::Run()" << endl;
      cerr << "  Cannot set input pointers" << endl;
      continue;
    }
    
//     // compute KDP

// Dave Albo: Put this code into the _setInputPointers method
//            That part of the code was not working when compute_kdp=true
// 
//     if (_params.compute_kdp) {
//       _computeKdp();
//     }
    
    // compute precip rates
    
    _computeRates();
    
    // compute particle ID
    
    if (_params.compute_pid) {
      _computePid();
    }

    // create the output MDV file and set the data fields

    if (_params.debug >= Params::DEBUG_NORM) {
      cerr << "Writing file ..." << endl;
    }

    DsMdvx outMdvx;
    _fillOutput(outMdvx);

    // write output

    if (_writeOutput(outMdvx)) {
      cerr << "ERROR - MdvPartRain::Run()" << endl;
      cerr << "  Cannot write output file" << endl;
    }
    
  } // while

  return 0;

}

///////////////////////////////////////////
// set up the read for the next MDV file

void MdvPartRain::_setupRead()
  
{

  _inMdvx.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  _inMdvx.clearRead();
  
  if (_params.set_elev_limits) {
    _inMdvx.setReadVlevelLimits(_params.lower_elev, _params.upper_elev);
  }

  _inMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _inMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  _inMdvx.addReadField(_params.DBZ_field_name);
  if (!_params.compute_snr_from_dbz) {
    _inMdvx.addReadField(_params.SNR_field_name);
  }
  _inMdvx.addReadField(_params.ZDR_field_name);
  if (_params.LDR_available) {
    _inMdvx.addReadField(_params.LDR_field_name);
  }
  _inMdvx.addReadField(_params.PHIDP_field_name);
  _inMdvx.addReadField(_params.RHOHV_field_name);
  if (!_params.compute_kdp) {
    _inMdvx.addReadField(_params.KDP_field_name);
  }
  
  for (int ii = 0; ii < _params.echo_field_names_n; ii++) {
    _inMdvx.addReadField(_params._echo_field_names[ii]);
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Reading input MDV file" << endl;
    _inMdvx.printReadRequest(cerr);
  }

}

///////////////////////////
// read in next volume file

int MdvPartRain::_readNextVolume()
  
{

  PMU_auto_register("Before read");

  // set up read
  
  _setupRead();

  // read the volume
  
  if (_input.readVolumeNextWithMaxValidAge(_inMdvx)) {
    cerr << "ERROR - MdvPartRain::_readInput" << endl;
    cerr << "  Cannot read in MDV data." << endl;
    cerr << _input.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Working on file: " << _inMdvx.getPathInUse() << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Input file fields:" << endl;
    for (int ifield = 0;
	 ifield < _inMdvx.getMasterHeader().n_fields; ifield++) {
      MdvxField *fld = _inMdvx.getField(ifield);
      const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
      cerr << "Field " << ifield << ", name " << fhdr.field_name << endl;
    }
  }

  // radar parameters - wavelength
  
  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(_inMdvx) == 0 && mdvxRadar.radarParamsAvail()) {
    const DsRadarParams &rparams = mdvxRadar.getRadarParams();
    if (!_params.override_wavelength) {
      _wavelengthCm = rparams.wavelength;
    }
    if (!_params.override_radar_height) {
      _radarHtKm = rparams.altitude;
    }
  }
  _kdpBringi.setWavelengthCm(_wavelengthCm);
  _rate.setWavelengthCm(_wavelengthCm);
  _partId.setWavelengthCm(_wavelengthCm);

  return 0;

}

///////////////////////////////////////////
// check that the fields are uniform in size

bool MdvPartRain::_checkFields()
  
{
  
  _mhdr = &(_inMdvx.getMasterHeader());
  const MdvxField *fld0 = _inMdvx.getField(0);
  _fhdr0 = &(fld0->getFieldHeader());
  _vhdr0 = &(fld0->getVlevelHeader());
  
  _nGates = _fhdr0->nx;
  _nBeams = _fhdr0->ny;
  _nTilts = _fhdr0->nz;
  _nPoints = _nGates * _nBeams * _nTilts;

  for (int i = 1; i < _mhdr->n_fields; i++) {
    const MdvxField *fld = _inMdvx.getField(i);
    const Mdvx::field_header_t &fhdr = fld->getFieldHeader();
    int nGates = fhdr.nx;
    int nBeams = fhdr.ny;
    int nTilts = fhdr.nz;
    if (nGates != _nGates ||
	nBeams != _nBeams ||
	nTilts != _nTilts) {
      cerr << "ERROR - MdvPartRain::_checkFields()" << endl;
      cerr << "  Fields are not uniform in geometry" << endl;
      cerr << "  Geometry in first field, name, nGates, nBeams, nTilts: "
	   << _fhdr0->field_name << ", "
	   << _nGates << ", " << _nBeams << ", " << _nTilts << endl;
      cerr << "  Geometry in this field, name, nGates, nBeams, nTilts: "
	   << fhdr.field_name << ", "
	   << nGates << ", " << nBeams << ", " << nTilts << endl;
      cerr << "  File: " << _inMdvx.getPathInUse() << endl;
      return false;
    }
  }

  _startRange = _fhdr0->grid_minx;
  _gateSpacing = _fhdr0->grid_dx;
  _startAz = _fhdr0->grid_miny;
  _deltaAz = _fhdr0->grid_dy;

  _elevs.clear();
  for (int ii = 0; ii < _nTilts; ii++) {
    _elevs.push_back(_vhdr0->level[ii]);
  }

  if (_fhdr0->proj_type == Mdvx::PROJ_RHI_RADAR) {
    _isPolarRadar = true;
    _isRhiRadar = true;
    _rhiMinEl = _fhdr0->grid_miny;
    _rhiDeltaEl = _fhdr0->grid_dy;
  } else if (_fhdr0->proj_type == Mdvx::PROJ_POLAR_RADAR) {
    _isPolarRadar = true;
    _isRhiRadar = false;
  } else {
    _isPolarRadar = false;
    _isRhiRadar = false;
  }

  if (_fhdr0->proj_type == Mdvx::PROJ_FLAT && 
      _fhdr0->vlevel_type == Mdvx::VERT_TYPE_ELEV) {
    _isPpiCart = true;
  } else {
    _isPpiCart = false;
  }

  if (_fhdr0->vlevel_type == Mdvx::VERT_TYPE_Z) {
    _vlevelInKm = true;
  } else {
    _vlevelInKm = false;
  }

  if (!_isPolarRadar && !_isRhiRadar && !_isPpiCart && !_vlevelInKm) {
    cerr << "ERROR - MdvPartRain::_checkFields()" << endl;
    cerr << "  Unsuitable projection or vertical levels for PartRain" << endl;
    cerr << "  Must be one of:" << endl;
    cerr << "    * radar in polar coords" << endl;
    cerr << "    * radar in Cartesian PPIs" << endl;
    cerr << "    * vertical units in km" << endl;
    return false;
  }

  if (!_isPolarRadar && _params.compute_kdp) {
    cerr << "ERROR - MdvPartRain::_checkFields()" << endl;
    cerr << "  Non-polar RADAR data found in file," << endl;
    cerr << "    but compute_kdp is true" << endl;
    cerr << "  For non-polar, KDP must be a field in the input data" << endl;
    cerr << "  See params 'compute_kdp' and 'KDP_field_name'" << endl;
    return false;
  }

  return true;

}

///////////////////////////////////////////
// set up the input pointers

int MdvPartRain::_setInputPointers()
  
{

  const MdvxField *dbzFld = _inMdvx.getField(_params.DBZ_field_name);
  if (dbzFld == NULL) {
    cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
    cerr << "  Cannot find DBZ field, name: " << _params.DBZ_field_name << endl;
    return -1;
  }
  _dbz = (fl32 *) dbzFld->getVol();
  _dbzMiss = dbzFld->getFieldHeader().missing_data_value;

  if (_params.compute_snr_from_dbz) {
    _computeSnrFromDbz();
  } else {
    const MdvxField *snrFld = _inMdvx.getField(_params.SNR_field_name);
    if (snrFld == NULL) {
      cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
      cerr << "  Cannot find SNR field, name: "
           << _params.SNR_field_name << endl;
      return -1;
    }
    _snr = (fl32 *) snrFld->getVol();
    _snrMiss = snrFld->getFieldHeader().missing_data_value;
  }
  
  const MdvxField *zdrFld = _inMdvx.getField(_params.ZDR_field_name);
  if (zdrFld == NULL) {
    cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
    cerr << "  Cannot find ZDR field, name: " << _params.ZDR_field_name << endl;
    return -1;
  }
  _zdr = (fl32 *) zdrFld->getVol();
  _zdrMiss = zdrFld->getFieldHeader().missing_data_value;
  
  if (_params.LDR_available) {
    const MdvxField *ldrFld = _inMdvx.getField(_params.LDR_field_name);
    if (ldrFld == NULL) {
      cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
      cerr << "  Cannot find LDR field, name: " << _params.LDR_field_name << endl;
      return -1;
    }
    _ldr = (fl32 *) ldrFld->getVol();
    _ldrMiss = ldrFld->getFieldHeader().missing_data_value;
  } else {
    _ldr = NULL;
    _ldrMiss = missingDouble;
  }
  
  const MdvxField *phidpFld = _inMdvx.getField(_params.PHIDP_field_name);
  if (phidpFld == NULL) {
    cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
    cerr << "  Cannot find PHIDP field, name: " << _params.PHIDP_field_name << endl;
    return -1;
  }
  _phidp = (fl32 *) phidpFld->getVol();
  _phidpMiss = phidpFld->getFieldHeader().missing_data_value;
  
  const MdvxField *rhohvFld = _inMdvx.getField(_params.RHOHV_field_name);
  if (rhohvFld == NULL) {
    cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
    cerr << "  Cannot find RHOHV field, name: " << _params.RHOHV_field_name << endl;
    return -1;
  }
  _rhohv = (fl32 *) rhohvFld->getVol();
  _rhohvMiss = rhohvFld->getFieldHeader().missing_data_value;
  
  if (_params.compute_kdp) {
    _computeKdp();
  } else {
    const MdvxField *kdpFld = _inMdvx.getField(_params.KDP_field_name);
    if (kdpFld == NULL) {
      cerr << "ERROR - MdvPartRain::_setInputPointers()" << endl;
      cerr << "  Cannot find KDP field, name: " << _params.KDP_field_name << endl;
      return -1;
    }
    _kdp = (fl32 *) kdpFld->getVol();
    _kdpMiss = kdpFld->getFieldHeader().missing_data_value;
  }
  
  return 0;

}

///////////////////////////////////////////
// compute KDP

void MdvPartRain::_computeKdp()
  
{

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Computing KDP ..." << endl;
  }

  // allocate array for MDV volume
  
  _kdp = _kdp_.alloc(_nPoints);
  _dbzForKdp = _dbzForKdp_.alloc(_nPoints);
  _snrForKdp = _snrForKdp_.alloc(_nPoints);
  _zdrForKdp = _zdrForKdp_.alloc(_nPoints);
  _rhohvForKdp = _rhohvForKdp_.alloc(_nPoints);
  _phidpForKdp = _phidpForKdp_.alloc(_nPoints);
  _sdphidpForKdp = _sdphidpForKdp_.alloc(_nPoints);
  
  // alloc local arrays

  TaArray<double> range_, dbz_, snr_, zdr_, phidp_, rhohv_, kdp_;
  double *range = range_.alloc(_nGates);
  double *dbz = dbz_.alloc(_nGates);
  double *snr = snr_.alloc(_nGates);
  double *zdr = zdr_.alloc(_nGates);
  double *phidp = phidp_.alloc(_nGates);
  double *rhohv = rhohv_.alloc(_nGates);
  double *kdp = kdp_.alloc(_nGates);

  // compute range array

  for (int ii = 0; ii < _nGates; ii++) {
    range[ii] = _startRange + ii * _gateSpacing;
    kdp[ii] = missingDouble;
  }

  // deal with a beam at a time

  for (int itilt = 0; itilt < _nTilts; itilt++) {
    
    double elev = _elevs[itilt];
    
    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
      
      double az = _startAz + ibeam * _deltaAz;
      if (az > 360) {
        az -= 360;
      }

      int beamNum = itilt * _nBeams + ibeam;
      
      // load up local arrays
      
      int kk = beamNum * _nGates;
      for (int igate = 0; igate < _nGates; igate++) {
        int mm = kk + igate;
	dbz[igate] = _dbz[mm];
	snr[igate] = _snr[mm];
	zdr[igate] = _zdr[mm];
	phidp[igate] = _phidp[mm];
	rhohv[igate] = _rhohv[mm];
      }
      
      // compute KDP
      
      _kdpBringi.compute(elev, az, _nGates, range,
                         dbz, zdr, phidp, rhohv, snr, missingDouble);
    
      // store KDP
      
      const double *kdp = _kdpBringi.getKdp();
      const double *dbz = _kdpBringi.getDbz();
      const double *snr = _kdpBringi.getSnr();
      const double *zdr = _kdpBringi.getZdr();
      const double *rhohv = _kdpBringi.getRhohv();
      const double *phidp = _kdpBringi.getPhidp();
      const double *sdphidp = _kdpBringi.getSdPhidp();
      
      for (int igate = 0; igate < _nGates; igate++) {
        
        int mm = kk + igate;
        
        double kdpVal = kdp[igate];
        if (kdpVal == missingDouble) {
          _kdp[mm] = missingFloat;
        } else if (std::isnan(kdpVal) || std::isinf(kdpVal)) {
          cerr << "WARNING - bad kdp: " << kdpVal << endl;
          _kdp[mm] = missingFloat;
        } else {
          _kdp[mm] = kdpVal;
        }
        
        double dbzVal = dbz[igate];
        if (dbzVal == missingDouble) {
          _dbzForKdp[mm] = missingFloat;
        } else {
          _dbzForKdp[mm] = dbzVal;
        }
        
        double snrVal = snr[igate];
        if (snrVal == missingDouble) {
          _snrForKdp[mm] = missingFloat;
        } else {
          _snrForKdp[mm] = snrVal;
        }
        
        double zdrVal = zdr[igate];
        if (zdrVal == missingDouble) {
          _zdrForKdp[mm] = missingFloat;
        } else {
          _zdrForKdp[mm] = zdrVal;
        }
        
        double rhohvVal = rhohv[igate];
        if (rhohvVal == missingDouble) {
          _rhohvForKdp[mm] = missingFloat;
        } else {
          _rhohvForKdp[mm] = rhohvVal;
        }
        
        double phidpVal = phidp[igate];
        if (phidpVal == missingDouble) {
          _phidpForKdp[mm] = missingFloat;
        } else {
          _phidpForKdp[mm] = phidpVal;
        }
        
        double sdVal = sdphidp[igate];
        if (sdVal == missingDouble) {
          _sdphidpForKdp[mm] = missingFloat;
        } else {
          _sdphidpForKdp[mm] = sdVal;
        }
        
      }
      
    } // ibeam
  } // itilt

}

///////////////////////////////////////////
// compute precip rates

void MdvPartRain::_computeRates()
  
{

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Computing Rain rates ..." << endl;
  }

  // allocate output arrays for MDV volume

  _dbzForRate = _dbzForRate_.alloc(_nPoints);
  _zdrForRate = _zdrForRate_.alloc(_nPoints);
  _kdpForRate = _kdpForRate_.alloc(_nPoints);
  _rateZh = _rateZh_.alloc(_nPoints);
  _rateZZdr = _rateZZdr_.alloc(_nPoints);
  _rateKdp = _rateKdp_.alloc(_nPoints);
  _rateKdpZdr = _rateKdpZdr_.alloc(_nPoints);
  _rateHybrid = _rateHybrid_.alloc(_nPoints);
  
  // alloc input arrays

  TaArray<double> dbz_, zdr_, kdp_, snr_;
  double *snr = snr_.alloc(_nGates);
  double *dbz = dbz_.alloc(_nGates);
  double *zdr = zdr_.alloc(_nGates);
  double *kdp = kdp_.alloc(_nGates);

  // deal with a beam at a time

  int nBeamsTotal = _nBeams * _nTilts;
  for (int beamNum = 0; beamNum < nBeamsTotal; beamNum++) {
    
    // load up local arrays
    
    int kk = beamNum * _nGates;
    for (int igate = 0; igate < _nGates; igate++) {

      int mm = kk + igate;
      
      if (_snr[mm] == _snrMiss) {
	snr[igate] = missingDouble;
      } else {
	snr[igate] = _snr[mm];
      }

      if (_dbz[mm] == _dbzMiss) {
	dbz[igate] = missingDouble;
      } else {
	dbz[igate] = _dbz[mm];
      }

      if (_zdr[mm] == _zdrMiss) {
	zdr[igate] = missingDouble;
      } else {
	zdr[igate] = _zdr[mm];
      }

      if (_kdp[mm] == missingFloat) {
	kdp[igate] = missingDouble;
      } else {
	kdp[igate] = _kdp[mm];
      }
      
    } // igate

    // compute precip
    
    _rate.computePrecipRates(_nGates, snr, dbz, zdr, kdp, missingDouble);
    
    // set pointers to computed arrays
    
    const double *dbzForRate = _rate.getDbz();
    const double *zdrForRate = _rate.getZdr();
    const double *kdpForRate = _rate.getKdp();

    const double *rateZ = _rate.getRateZ();
    const double *rateKdp = _rate.getRateKdp();
    const double *rateKdpZdr = _rate.getRateKdpZdr();
    const double *rateZZdr = _rate.getRateZZdr();
    const double *rateHybrid = _rate.getRateHybrid();

    // store output fields

    for (int igate = 0; igate < _nGates; igate++) {

      int mm = kk + igate;

      if (dbzForRate[igate] == missingDouble) {
	_dbzForRate[mm] = missingFloat;
      } else {
	_dbzForRate[mm] = dbzForRate[igate];
      }

      if (zdrForRate[igate] == missingDouble) {
	_zdrForRate[mm] = missingFloat;
      } else {
	_zdrForRate[mm] = zdrForRate[igate];
      }

      if (kdpForRate[igate] == missingDouble) {
	_kdpForRate[mm] = missingFloat;
      } else {
	_kdpForRate[mm] = kdpForRate[igate];
      }

      if (rateZ[igate] == missingDouble) {
	_rateZh[mm] = missingFloat;
      } else {
	_rateZh[mm] = rateZ[igate];
      }

      if (rateZZdr[igate] == missingDouble) {
	_rateZZdr[mm] = missingFloat;
      } else {
	_rateZZdr[mm] = rateZZdr[igate];
      }

      if (rateKdp[igate] == missingDouble) {
	_rateKdp[mm] = missingFloat;
      } else {
	_rateKdp[mm] = rateKdp[igate];
      }

      if (rateKdpZdr[igate] == missingDouble) {
	_rateKdpZdr[mm] = missingFloat;
      } else {
	_rateKdpZdr[mm] = rateKdpZdr[igate];
      }

      if (rateHybrid[igate] == missingDouble) {
	_rateHybrid[mm] = missingFloat;
      } else {
	_rateHybrid[mm] = rateHybrid[igate];
      }
    }

  } // beamNum

}

///////////////////////////////////////////
// compute particle ID

int MdvPartRain::_computePid()
  
{

  if (_params.debug >= Params::DEBUG_NORM) {
    cerr << "Computing PID ..." << endl;
  }

  // read in the pid thresholds from file

  _partId.setMissingDouble(missingDouble);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _partId.setDebug(true);
  }
  if (_partId.readThresholdsFromFile(_params.pid_thresholds_file_path)) {
    cerr << "ERROR - MdvPartRain::_computePid" << endl;
    cerr << "  Cannot read in pid thresholds from file: "
	 << _params.pid_thresholds_file_path << endl;
    return -1;
  }

  if (_params.use_soundings_from_spdb) {
    _overrideTempProfile(_inMdvx.getValidTime());
  }

  // allocate arrays for MDV volume
  
  _pid = _pid_.alloc(_nPoints);
  _pidInterest = _pidInterest_.alloc(_nPoints);
  
  _pid2 = _pid2_.alloc(_nPoints);
  _pidInterest2 = _pidInterest2_.alloc(_nPoints);
  _pidConfidence = _pidConfidence_.alloc(_nPoints);
  
  _tempForPid = _tempForPid_.alloc(_nPoints);
  _dbzForPid = _dbzForPid_.alloc(_nPoints);
  _zdrForPid = _zdrForPid_.alloc(_nPoints);
  _ldrForPid = _ldrForPid_.alloc(_nPoints);
  _phidpForPid = _phidpForPid_.alloc(_nPoints);
  _rhohvForPid = _rhohvForPid_.alloc(_nPoints);
  _kdpForPid = _kdpForPid_.alloc(_nPoints);
  _sdzdrForPid = _sdzdrForPid_.alloc(_nPoints);
  _sdphidpForPid = _sdphidpForPid_.alloc(_nPoints);

  // allocate MDV space for individual particle interest fields

  if (_params.output_particle_interest_fields) {
    _pIntArray_.clear();
    _pIntArray.clear();
    const vector<NcarParticleId::Particle*> plist = _partId.getParticleList();
    for (int ii = 0; ii < (int) plist.size(); ii++) {
      TaArray<fl32> tmp;
      tmp.alloc(_nPoints);
      _pIntArray_.push_back(tmp);
    } // ii
    for (int ii = 0; ii < (int) plist.size(); ii++) {
      TaArray<fl32> &tmp = _pIntArray_[ii];
      _pIntArray.push_back(tmp.buf());
    } // ii
  }

  // alloc beam arrays

  TaArray<double> temp_, dbz_, snr_, zdr_, ldr_, phidp_, rhohv_, kdp_;
  double *temp = temp_.alloc(_nGates);
  double *dbz = dbz_.alloc(_nGates);
  double *snr = snr_.alloc(_nGates);
  double *zdr = zdr_.alloc(_nGates);
  double *ldr = ldr_.alloc(_nGates);
  double *phidp = phidp_.alloc(_nGates);
  double *rhohv = rhohv_.alloc(_nGates);
  double *kdp = kdp_.alloc(_nGates);

  // process a beam at a time
  
  for (int itilt = 0; itilt < _nTilts; itilt++) {

    if (_isPolarRadar) {
      double elev = _elevs[itilt];
      _fillTempArrayPolar(elev, temp);
    } else if (_vlevelInKm) {
      // ht in km
      double htKm = _elevs[itilt];
      _fillTempArrayCart(htKm, temp);
    }
    
    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {

      if (_isRhiRadar) {
	double elev = _rhiMinEl + ibeam * _rhiDeltaEl;
	_fillTempArrayPolar(elev, temp);
      } else if (_isPpiCart) {
        double elev = _elevs[itilt];
        _fillTempArrayPpi(ibeam, elev, temp);
      }
      
      int beamNum = itilt * _nBeams + ibeam;
      
      // load up beam arrays
      
      int kk = beamNum * _nGates;
      for (int igate = 0; igate < _nGates; igate++) {
	
	int mm = kk + igate;

        if (_snr[mm] == _snrMiss) {
          snr[igate] = missingDouble;
        } else {
          snr[igate] = _snr[mm];
        }

 	if (_dbz[mm] == _dbzMiss) {
 	  dbz[igate] = missingDouble;
 	} else {
          dbz[igate] = _dbz[mm];
 	}
	
	if (_zdr[mm] == _zdrMiss) {
	  zdr[igate] = missingDouble;
	} else {
	  zdr[igate] = _zdr[mm];
	}
	
	if (_params.LDR_available) {
	  if (_ldr[mm] == _ldrMiss) {
	    ldr[igate] = missingDouble;
	  } else {
	    ldr[igate] = _ldr[mm];
	  }
	} else {
	  ldr[igate] = missingDouble;
	}
	
	if (_phidp[mm] == _phidpMiss) {
	  phidp[igate] = missingDouble;
	} else {
	  phidp[igate] = _phidp[mm];
	}
	
	if (_rhohv[mm] == _rhohvMiss) {
	  rhohv[igate] = missingDouble;
	} else {
	  rhohv[igate] = _rhohv[mm];
	}
	
	if (_kdp[mm] == missingFloat) {
	  kdp[igate] = missingDouble;
	} else {
	  kdp[igate] = _kdp[mm];
	}
	
      } // igate
      
      // compute pid
      
      _partId.computePidBeam(_nGates, snr, dbz, zdr, kdp,
                             ldr, rhohv, phidp, temp);
      
      // set pointers to computed arrays
      
      const double *dbzForPid = _partId.getDbz();
      const double *zdrForPid = _partId.getZdr();
      const double *ldrForPid = _partId.getLdr();
      const double *phidpForPid = _partId.getPhidp();
      const double *rhohvForPid = _partId.getRhohv();
      const double *kdpForPid = _partId.getKdp();
      const double *tempForPid = _partId.getTempC();
      const double *sdzdrForPid = _partId.getSdzdr();
      const double *sdphidpForPid = _partId.getSdphidp();

      const int *pid = _partId.getPid();
      const double *interest = _partId.getInterest();
      const int *pid2 = _partId.getPid2();
      const double *interest2 = _partId.getInterest2();
      const double *confidence = _partId.getConfidence();

      // store output fields
      
      for (int igate = 0; igate < _nGates; igate++) {
	
	int mm = kk + igate;
	
        _pid[mm] = pid[igate];
        _pidInterest[mm] = interest[igate];
        
        _pid2[mm] = pid2[igate];
        _pidInterest2[mm] = interest2[igate];
        _pidConfidence[mm] = confidence[igate];
        
	_tempForPid[mm] = temp[igate];
        
	if (dbzForPid[igate] == missingDouble) {
	  _dbzForPid[mm] = missingFloat;
	} else {
	  _dbzForPid[mm] = dbzForPid[igate];
	}
	
	if (zdrForPid[igate] == missingDouble) {
	  _zdrForPid[mm] = missingFloat;
	} else {
	  _zdrForPid[mm] = zdrForPid[igate];
	}

	if (ldrForPid[igate] == missingDouble) {
	  _ldrForPid[mm] = missingFloat;
	} else {
	  _ldrForPid[mm] = ldrForPid[igate];
	}

	if (phidpForPid[igate] == missingDouble) {
	  _phidpForPid[mm] = missingFloat;
	} else {
	  _phidpForPid[mm] = phidpForPid[igate];
	}
        
	if (rhohvForPid[igate] == missingDouble) {
	  _rhohvForPid[mm] = missingFloat;
	} else {
	  _rhohvForPid[mm] = rhohvForPid[igate];
	}
        
	if (kdpForPid[igate] == missingDouble) {
	  _kdpForPid[mm] = missingFloat;
	} else {
	  _kdpForPid[mm] = kdpForPid[igate];
	}
        
	if (tempForPid[igate] == missingDouble) {
	  _tempForPid[mm] = missingFloat;
	} else {
	  _tempForPid[mm] = tempForPid[igate];
	}
        
	if (sdzdrForPid[igate] == missingDouble) {
	  _sdzdrForPid[mm] = missingFloat;
	} else {
	  _sdzdrForPid[mm] = sdzdrForPid[igate];
	}

	if (sdphidpForPid[igate] == missingDouble) {
	  _sdphidpForPid[mm] = missingFloat;
	} else {
	  _sdphidpForPid[mm] = sdphidpForPid[igate];
	}

      } // igate
      
      if (_params.output_particle_interest_fields) {
        // store individual interest fields
        for (int igate = 0; igate < _nGates; igate++) {
          int mm = kk + igate;
          const vector<NcarParticleId::Particle*> plist = _partId.getParticleList();
          for (int ii = 0; ii < (int) plist.size(); ii++) {
            fl32 *pinterest = _pIntArray[ii];
            const double *gateInterest = plist[ii]->gateInterest;
            pinterest[mm] = gateInterest[igate];
          } // ii
        }
      }

    } // ibeam
    
  } // itilt

  return 0;

}

////////////////////////////////////////////////////////////
// fill temperature array for polar data
// computing ht from elevation angle and range

void MdvPartRain::_fillTempArrayPolar(double elev, double *temp)

{

  double sinelev = sin(elev * DEG_TO_RAD);
  double range = _startRange;

  for (int ii = 0; ii < _nGates; ii++, range += _gateSpacing) {
    double ht = range * sinelev +
      (range * range / pseudoEarthDiamKm) + _radarHtKm;
    temp[ii] = _partId.getTmpC(ht);
  }

}
    
////////////////////////////////////////////////////////////
// fill temperature array for cart PPI
// computing ht from elevation angle and range from radar

void MdvPartRain::_fillTempArrayPpi(int iy, double elev, double *temp)

{
  
  double yy = _fhdr0->grid_miny * iy * _fhdr0->grid_dy;
  double sinelev = sin(elev * DEG_TO_RAD);
  
  double xx = _fhdr0->grid_minx;
  for (int ix = 0; ix < _nGates; ix++, xx += _fhdr0->grid_dx) {
    double range = sqrt(xx * xx + yy * yy);
    double ht = range * sinelev +
      (range * range / pseudoEarthDiamKm) + _radarHtKm;
    temp[ix] = _partId.getTmpC(ht);
  }

}
    
////////////////////////////////////////////////////////////
// fill temperature array, for Cartesian volume

void MdvPartRain::_fillTempArrayCart(double htKm, double *temp)

{

  for (int ii = 0; ii < _nGates; ii++) {
    temp[ii] = _partId.getTmpC(htKm);
  }

}
    
//////////////////////////////////////////
// fill output MDV object

void MdvPartRain::_fillOutput(DsMdvx &outMdvx)
 
{

  // fields

  outMdvx.clearFields();

  if (_params.KDP_output_field.write) {
    _addField(outMdvx,
	      _params.KDP_output_field.name, 
	      _params.KDP_output_field.long_name, 
	      _params.KDP_output_field.units,
	      _params.KDP_output_field.encoding,
	      _kdp);
  }
  
  if (_params.DBZ_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.DBZ_FOR_KDP_output_field.name, 
              _params.DBZ_FOR_KDP_output_field.long_name, 
              _params.DBZ_FOR_KDP_output_field.units,
              _params.DBZ_FOR_KDP_output_field.encoding,
              _dbzForKdp);
  }
  
  if (_params.SNR_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.SNR_FOR_KDP_output_field.name, 
              _params.SNR_FOR_KDP_output_field.long_name, 
              _params.SNR_FOR_KDP_output_field.units,
              _params.SNR_FOR_KDP_output_field.encoding,
              _snrForKdp);
  }
  
  if (_params.ZDR_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.ZDR_FOR_KDP_output_field.name, 
              _params.ZDR_FOR_KDP_output_field.long_name, 
              _params.ZDR_FOR_KDP_output_field.units,
              _params.ZDR_FOR_KDP_output_field.encoding,
              _zdrForKdp);
  }
  
  if (_params.RHOHV_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.RHOHV_FOR_KDP_output_field.name, 
              _params.RHOHV_FOR_KDP_output_field.long_name, 
              _params.RHOHV_FOR_KDP_output_field.units,
              _params.RHOHV_FOR_KDP_output_field.encoding,
              _rhohvForKdp);
  }
  
  if (_params.PHIDP_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.PHIDP_FOR_KDP_output_field.name, 
              _params.PHIDP_FOR_KDP_output_field.long_name, 
              _params.PHIDP_FOR_KDP_output_field.units,
              _params.PHIDP_FOR_KDP_output_field.encoding,
              _phidpForKdp);
  }
  
  if (_params.SDPHIDP_FOR_KDP_output_field.write) {
    _addField(outMdvx,
              _params.SDPHIDP_FOR_KDP_output_field.name, 
              _params.SDPHIDP_FOR_KDP_output_field.long_name, 
              _params.SDPHIDP_FOR_KDP_output_field.units,
              _params.SDPHIDP_FOR_KDP_output_field.encoding,
              _sdphidpForKdp);
  }
  
  if (_params.DBZ_FOR_RATE_output_field.write) {
    _addField(outMdvx,
	      _params.DBZ_FOR_RATE_output_field.name, 
	      _params.DBZ_FOR_RATE_output_field.long_name, 
	      _params.DBZ_FOR_RATE_output_field.units,
	      _params.DBZ_FOR_RATE_output_field.encoding,
	      _dbzForRate);
  }

  if (_params.ZDR_FOR_RATE_output_field.write) {
    _addField(outMdvx,
	      _params.ZDR_FOR_RATE_output_field.name, 
	      _params.ZDR_FOR_RATE_output_field.long_name, 
	      _params.ZDR_FOR_RATE_output_field.units,
	      _params.ZDR_FOR_RATE_output_field.encoding,
	      _zdrForRate);
  }

  if (_params.KDP_FOR_RATE_output_field.write) {
    _addField(outMdvx,
	      _params.KDP_FOR_RATE_output_field.name, 
	      _params.KDP_FOR_RATE_output_field.long_name, 
	      _params.KDP_FOR_RATE_output_field.units,
	      _params.KDP_FOR_RATE_output_field.encoding,
	      _kdpForRate);
  }

  if (_params.RATE_ZH_output_field.write) {
    _addField(outMdvx,
	      _params.RATE_ZH_output_field.name, 
	      _params.RATE_ZH_output_field.long_name, 
	      _params.RATE_ZH_output_field.units,
	      _params.RATE_ZH_output_field.encoding,
	      _rateZh);
  }

  if (_params.RATE_Z_ZDR_output_field.write) {
    _addField(outMdvx,
	      _params.RATE_Z_ZDR_output_field.name, 
	      _params.RATE_Z_ZDR_output_field.long_name, 
	      _params.RATE_Z_ZDR_output_field.units,
	      _params.RATE_Z_ZDR_output_field.encoding,
	      _rateZZdr);
  }

  if (_params.RATE_KDP_output_field.write) {
    _addField(outMdvx,
	      _params.RATE_KDP_output_field.name, 
	      _params.RATE_KDP_output_field.long_name, 
	      _params.RATE_KDP_output_field.units,
	      _params.RATE_KDP_output_field.encoding,
	      _rateKdp);
  }

  if (_params.RATE_KDP_ZDR_output_field.write) {
    _addField(outMdvx,
	      _params.RATE_KDP_ZDR_output_field.name, 
	      _params.RATE_KDP_ZDR_output_field.long_name, 
	      _params.RATE_KDP_ZDR_output_field.units,
	      _params.RATE_KDP_ZDR_output_field.encoding,
	      _rateKdpZdr);
  }

  if (_params.RATE_HYBRID_output_field.write) {
    _addField(outMdvx,
	      _params.RATE_HYBRID_output_field.name, 
	      _params.RATE_HYBRID_output_field.long_name, 
	      _params.RATE_HYBRID_output_field.units,
	      _params.RATE_HYBRID_output_field.encoding,
	      _rateHybrid);
  }

  if (_params.compute_pid) {

    if (_params.PID_output_field.write) {
      _addField(outMdvx,
		_params.PID_output_field.name, 
		_params.PID_output_field.long_name, 
		_params.PID_output_field.units,
		_params.PID_output_field.encoding,
		_pid);
    }

    if (_params.PID_INT_output_field.write) {
      _addField(outMdvx,
		_params.PID_INT_output_field.name, 
		_params.PID_INT_output_field.long_name, 
		_params.PID_INT_output_field.units,
		_params.PID_INT_output_field.encoding,
		_pidInterest);
    }

    if (_params.PID2_output_field.write) {
      _addField(outMdvx,
		_params.PID2_output_field.name, 
		_params.PID2_output_field.long_name, 
		_params.PID2_output_field.units,
		_params.PID2_output_field.encoding,
		_pid2);
    }

    if (_params.PID_INT2_output_field.write) {
      _addField(outMdvx,
		_params.PID_INT2_output_field.name, 
		_params.PID_INT2_output_field.long_name, 
		_params.PID_INT2_output_field.units,
		_params.PID_INT2_output_field.encoding,
		_pidInterest2);
    }

    if (_params.PID_CONFIDENCE_output_field.write) {
      _addField(outMdvx,
		_params.PID_CONFIDENCE_output_field.name, 
		_params.PID_CONFIDENCE_output_field.long_name, 
		_params.PID_CONFIDENCE_output_field.units,
		_params.PID_CONFIDENCE_output_field.encoding,
		_pidConfidence);
    }
    
    if (_params.DBZ_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.DBZ_FOR_PID_output_field.name, 
		_params.DBZ_FOR_PID_output_field.long_name, 
		_params.DBZ_FOR_PID_output_field.units,
		_params.DBZ_FOR_PID_output_field.encoding,
		_dbzForPid);
    }
    
    if (_params.ZDR_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.ZDR_FOR_PID_output_field.name, 
		_params.ZDR_FOR_PID_output_field.long_name, 
		_params.ZDR_FOR_PID_output_field.units,
		_params.ZDR_FOR_PID_output_field.encoding,
		_zdrForPid);
    }
     
    if (_params.LDR_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.LDR_FOR_PID_output_field.name, 
		_params.LDR_FOR_PID_output_field.long_name, 
		_params.LDR_FOR_PID_output_field.units,
		_params.LDR_FOR_PID_output_field.encoding,
		_ldrForPid);
    }
     
    if (_params.PHIDP_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.PHIDP_FOR_PID_output_field.name, 
		_params.PHIDP_FOR_PID_output_field.long_name, 
		_params.PHIDP_FOR_PID_output_field.units,
		_params.PHIDP_FOR_PID_output_field.encoding,
		_phidpForPid);
    }
     
    if (_params.RHOHV_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.RHOHV_FOR_PID_output_field.name, 
		_params.RHOHV_FOR_PID_output_field.long_name, 
		_params.RHOHV_FOR_PID_output_field.units,
		_params.RHOHV_FOR_PID_output_field.encoding,
		_rhohvForPid);
    }
     
    if (_params.KDP_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.KDP_FOR_PID_output_field.name, 
		_params.KDP_FOR_PID_output_field.long_name, 
		_params.KDP_FOR_PID_output_field.units,
		_params.KDP_FOR_PID_output_field.encoding,
		_kdpForPid);
    }
     
    if (_params.SDZDR_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.SDZDR_FOR_PID_output_field.name, 
		_params.SDZDR_FOR_PID_output_field.long_name, 
		_params.SDZDR_FOR_PID_output_field.units,
		_params.SDZDR_FOR_PID_output_field.encoding,
		_sdzdrForPid);
    }
     
    if (_params.SDPHIDP_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.SDPHIDP_FOR_PID_output_field.name, 
		_params.SDPHIDP_FOR_PID_output_field.long_name, 
		_params.SDPHIDP_FOR_PID_output_field.units,
		_params.SDPHIDP_FOR_PID_output_field.encoding,
		_sdphidpForPid);
    }
     
    if (_params.TEMP_FOR_PID_output_field.write) {
      _addField(outMdvx,
		_params.TEMP_FOR_PID_output_field.name, 
		_params.TEMP_FOR_PID_output_field.long_name, 
		_params.TEMP_FOR_PID_output_field.units,
		_params.TEMP_FOR_PID_output_field.encoding,
		_tempForPid);
    }

  }

  // optionally write interest fields for individual particles

  _writeParticleInterestFields(outMdvx);

  // optionally echo input fields into output file

  _echoInputFields(outMdvx);

  // chunks

  outMdvx.clearChunks();

  for (int ii = 0; ii < _inMdvx.getNChunks(); ii++) {
    MdvxChunk *chunk = new MdvxChunk(*_inMdvx.getChunkByNum(ii));
    outMdvx.addChunk(chunk);
  }

  // master header

  Mdvx::master_header_t inMhdr = *_mhdr;
  inMhdr.n_fields = outMdvx.getNFields();
  inMhdr.n_chunks = outMdvx.getNChunks();
  outMdvx.setMasterHeader(inMhdr);

  string info = "Derived fields computed by MdvPartRain\n";
  info += inMhdr.data_set_info;
  info += "\n";
  outMdvx.setDataSetInfo(info.c_str());

}
  
//////////////////////////////////////////
// add output field to output MDVX object

void MdvPartRain::_addField(DsMdvx &outMdvx,
			    const string &field_name,
			    const string &long_field_name,
			    const string &units,
			    int encoding_type,
			    const fl32 *data)
  
{
  
  if (_params.debug) {
    cerr << "  Adding fl32 field: " << field_name << endl;
  }

  const MdvxField *fld0 = _inMdvx.getField(0);
  
  // copy field header, set members which change
  
  Mdvx::field_header_t fhdr = fld0->getFieldHeader();

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = _nPoints * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = missingFloat;
  fhdr.missing_data_value = missingFloat;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN);
  STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
  
  // copy vlevel header
  
  Mdvx::vlevel_header_t vhdr = fld0->getVlevelHeader();

  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, data);

  // convert to output encoding type, and compress

  fld->convertDynamic((Mdvx::encoding_type_t) encoding_type, Mdvx::COMPRESSION_GZIP);

  // add to object
  
  outMdvx.addField(fld);

}
  
//////////////////////////////////////////
// add integer output field to working MDVX object

void MdvPartRain::_addField(DsMdvx &outMdvx,
			    const string &field_name,
			    const string &long_field_name,
			    const string &units,
			    int encoding_type,
			    const ui16 *data)
  
{
  
  if (_params.debug) {
    cerr << "  Adding ui16 field: " << field_name << endl;
  }

  const MdvxField *fld0 = _inMdvx.getField(0);
  
  // copy field header, set members which change
  
  Mdvx::field_header_t fhdr = fld0->getFieldHeader();
  
  fhdr.encoding_type = Mdvx::ENCODING_INT16;
  fhdr.data_element_nbytes = 2;
  fhdr.volume_size = _nPoints * sizeof(ui16);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = 0;
  fhdr.missing_data_value = 0;
  
  fhdr.min_value = 0;
  fhdr.max_value = 0;
  fhdr.min_value_orig_vol = 0;
  fhdr.max_value_orig_vol = 0;
  
  STRncopy(fhdr.field_name_long, long_field_name.c_str(), MDV_LONG_FIELD_LEN);
  STRncopy(fhdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.units, units.c_str(), MDV_UNITS_LEN);
  STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
  
  // copy vlevel header
  
  Mdvx::vlevel_header_t vhdr = fld0->getVlevelHeader();

  // create field
  
  MdvxField *fld = new MdvxField(fhdr, vhdr, data);

  // compress output

  fld->compress(Mdvx::COMPRESSION_GZIP);

  // add to object
  
  outMdvx.addField(fld);

}
  
////////////////////////////////////
// echo input fields to output file

int MdvPartRain::_echoInputFields(DsMdvx &outMdvx)
  
{

  if (!_params.echo_input_fields_in_output_file) {
    return 0;
  }

  int iret = 0;

  for (int ii = 0; ii < _params.echo_field_names_n; ii++) {
    char *fname = _params._echo_field_names[ii];
    MdvxField *fld = _inMdvx.getField(fname);
    if (_params.debug) {
      cerr << "  Echoing input field: " << fname << endl;
    }
    if (fld == NULL) {
      cerr << "WARNING - void MdvPartRain::_fillOutput" << endl;
      cerr << "  Trying to echo input field to output file: "
           << fname << endl;
      cerr << "  Cannot find field in input file" << endl;
      iret = -1;
    } else {
      MdvxField *echoFld = new MdvxField(*fld);
      outMdvx.addField(echoFld);
    }
  } // ii

  return iret;

}

/////////////////////////////////////////////////////
// write interest fields for individual particles

void MdvPartRain::_writeParticleInterestFields(DsMdvx &outMdvx)

{

  if (!_params.compute_pid || !_params.output_particle_interest_fields) {
    return;
  }
  
  const vector<NcarParticleId::Particle*> plist = _partId.getParticleList();
  for (int ii = 0; ii < (int) plist.size(); ii++) {
    string fieldName = plist[ii]->label;
    fieldName += "_interest";
    fl32 *interest = _pIntArray[ii];
    _addField(outMdvx,
	      fieldName, fieldName, "",
              _params.encoding_for_particle_interest_fields,
              interest);
  }

}

///////////////////////////
// write Output file

int MdvPartRain::_writeOutput(DsMdvx &outMdvx)
  
{

  PMU_auto_register("Before write");
  outMdvx.setWriteLdataInfo();
  outMdvx.setAppName(_progName);
  if(outMdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - MdvPartRain::_writeOutput" << endl;
    cerr << "  Cannot write data set." << endl;
    cerr << outMdvx.getErrStr() << endl;
    return -1;
  }
  if (_params.debug) {
    cerr << "Wrote file: " << outMdvx.getPathInUse() << endl;
  }

  return 0;

}


////////////////////////////////////////////////////////////////////////
// override temperature profile using sounding from SPDB

int MdvPartRain::_overrideTempProfile(time_t dataTime)

{

  // get temperature profile

  TempProfile tempProfile;

  tempProfile.setSoundingLocationName
    (_params.sounding_location_name);
  tempProfile.setSoundingSearchTimeMarginSecs
    (_params.sounding_search_time_margin_secs);
  
  tempProfile.setCheckPressureRange
    (_params.sounding_check_pressure_range);
  tempProfile.setSoundingRequiredMinPressureHpa
    (_params.sounding_required_pressure_range_hpa.min_val);
  tempProfile.setSoundingRequiredMaxPressureHpa
    (_params.sounding_required_pressure_range_hpa.max_val);
  
  tempProfile.setCheckHeightRange
    (_params.sounding_check_height_range);
  tempProfile.setSoundingRequiredMinHeightM
    (_params.sounding_required_height_range_m.min_val);
  tempProfile.setSoundingRequiredMaxHeightM
    (_params.sounding_required_height_range_m.max_val);
  
  tempProfile.setCheckPressureMonotonicallyDecreasing
    (_params.sounding_check_pressure_monotonically_decreasing);

  tempProfile.setHeightCorrectionKm
    (_params.sounding_height_correction_km);

  if (_params.sounding_use_wet_bulb_temp) {
    tempProfile.setUseWetBulbTemp(true);
  }
  
  time_t retrievedTime;
  if (tempProfile.loadFromSpdb(_params.sounding_spdb_url,
                               dataTime,
                               retrievedTime)) {
    cerr << "ERROR - MdvPartRain::tempProfileInit" << endl;
    cerr << "  Cannot retrive profile for time: "
         << DateTime::strm(dataTime) << endl;
    cerr << "  url: " << _params.sounding_spdb_url << endl;
    cerr << "  station name: " << _params.sounding_location_name << endl;
    cerr << "  time margin secs: " << _params.sounding_search_time_margin_secs << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "=====================================" << endl;
    cerr << "Got temp profile, URL: " << _params.sounding_spdb_url << endl;
    cerr << "Overriding temperature profile" << endl;
    cerr << "  vol time: " << DateTime::strm(dataTime) << endl;
    cerr << "  retrievedTime: " << DateTime::strm(retrievedTime) << endl;
    cerr << "  freezingLevel: " << tempProfile.getFreezingLevel() << endl;
  }
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    const vector<TempProfile::PointVal> &retrievedProfile = tempProfile.getProfile();
    cerr << "=====================================" << endl;
    cerr << "Temp  profile" << endl;
    int nLevels = (int) retrievedProfile.size();
    int nPrint = 50;
    int printInterval = nLevels / nPrint;
    if (nLevels < nPrint) {
      printInterval = 1;
    }
    for (size_t ii = 0; ii < retrievedProfile.size(); ii++) {
      bool doPrint = false;
      if (ii % printInterval == 0) {
        doPrint = true;
      }
      if (ii < retrievedProfile.size() - 1) {
        if (retrievedProfile[ii].tmpC * retrievedProfile[ii+1].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (ii > 0) {
        if (retrievedProfile[ii-1].tmpC * retrievedProfile[ii].tmpC <= 0) {
          doPrint = true;
        }
      }
      if (doPrint) {
        cerr << "  ilevel, press(Hpa), alt(km), temp(C): " << ii << ", "
             << retrievedProfile[ii].pressHpa << ", "
             << retrievedProfile[ii].htKm << ", "
             << retrievedProfile[ii].tmpC << endl;
      }
    }
    cerr << "=====================================" << endl;
  }
  
  // accept the profile

  _partId.setTempProfile(tempProfile);
  
  return 0;
  
}

////////////////////////////////////////////////////////////////////////
// compute the SNR field from the DBZ field

void MdvPartRain::_computeSnrFromDbz()

{

  // free up previous array

  _freeSnr();

  // allocate array

  const MdvxField *dbzFld = _inMdvx.getField(_params.DBZ_field_name);
  _snr = new fl32[_nPoints];
  _snrMiss = dbzFld->getFieldHeader().missing_data_value;

  // copy in the dbz field, including missing values
  
  memcpy(_snr, _dbz, _nPoints * sizeof(fl32));

  // compute noise at each gate

  TaArray<double> noiseDbz_;
  double *noiseDbz = noiseDbz_.alloc(_nGates);
  double range = _startRange;
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacing) {
    noiseDbz[igate] = _params.noise_dbz_at_100km +
      20.0 * (log10(range) - log10(100.0));
  }

  // compute snr from dbz

  fl32 *snr = _snr;
  const fl32 *dbz = _dbz;
  for (int itilt = 0; itilt < _nTilts; itilt++) {
    for (int ibeam = 0; ibeam < _nBeams; ibeam++) {
      for (int igate = 0; igate < _nGates; igate++, snr++, dbz++) {
        if (*dbz != _dbzMiss) {
          *snr = *dbz - noiseDbz[igate];
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////
// free the SNR field, if it has been allocated locally

void MdvPartRain::_freeSnr()

{
  if (_params.compute_snr_from_dbz) {
    if (_snr) {
      delete[] _snr;
      _snr = NULL;
    }
  }
}
