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
// ComputeEngine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// ComputeEngine computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#include "ComputeEngine.hh"
#include <toolsa/os_config.h>
#include <toolsa/file_io.h>
#include <rapmath/trig.h>
#include <rapmath/umath.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <cerrno>
using namespace std;
pthread_mutex_t ComputeEngine::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;
const double ComputeEngine::missingDbl = -9999.0;

// Constructor

ComputeEngine::ComputeEngine(const Params &params,
                             int id)  :
        _params(params),
        _id(id)
  
{

  OK = true;
  
  // initialize moments, kdp, pid and precip objects

  _kdpInit();

}

// destructor

ComputeEngine::~ComputeEngine()

{

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// storing results in moments ray
//
// Creates moments ray and returns it.
// It must be freed by caller.
//
// Returns NULL on error.

RadxRay *ComputeEngine::compute(RadxRay *inputRay,
                                double radarHtKm,
                                double wavelengthM,
                                const TempProfile *tempProfile)
{

  // set ray-specific metadata
  
  _nGates = inputRay->getNGates();
  _startRangeKm = inputRay->getStartRangeKm();
  _gateSpacingKm = inputRay->getGateSpacingKm();
  _azimuth = inputRay->getAzimuthDeg();
  _elevation = inputRay->getElevationDeg();
  _timeSecs = inputRay->getTimeSecs();
  _nanoSecs = inputRay->getNanoSecs();
  _nyquist = inputRay->getNyquistMps();

  // initialize

  _radarHtKm = radarHtKm;
  _wavelengthM = wavelengthM;
  _tempProfile = tempProfile;
  _atmos.setAttenCrpl(_wavelengthM * 100.0);

  // create moments ray
  
  RadxRay *derivedRay = new RadxRay;
  derivedRay->copyMetaData(*inputRay);

  // allocate moments arrays for computing derived fields,
  // and load them up
  
  _allocMomentsArrays();
  _loadMomentsArrays(inputRay);
  
  // compute ZDP

  _computeZdpArray();
  
  // compute kdp if needed
  
  if (_params.compute_KDP) {
    _kdpCompute();
  } else {
    _kdp.initializeArrays(_nGates);
  }

  // locate RLAN interference

  if (_params.locate_rlan_interference) {
    _locateRlan();
  }

  // load output fields into the moments ray
  
  _loadOutputFields(inputRay, derivedRay);

  // set max range

  if (_params.set_max_range) {
    derivedRay->setMaxRangeKm(_params.max_range_km);
  }

  return derivedRay;

}

///////////////////////////////
// load up fields in output ray

void ComputeEngine::_loadOutputFields(RadxRay *inputRay,
                                      RadxRay *derivedRay)

{

  // initialize array pointers

  const double *dbzForKdp = _kdp.getDbz();
  const double *zdrForKdp = _kdp.getZdr();
  const double *rhohvForKdp = _kdp.getRhohv();
  const double *snrForKdp = _kdp.getSnr();
  const double *zdrSdevForKdp = _kdp.getZdrSdev();
  const bool *validFlagForKdp = _kdp.getValidForKdp();

  const double *phidpForKdp = _kdp.getPhidp();
  const double *phidpMeanForKdp = _kdp.getPhidpMean();
  const double *phidpMeanUnfoldForKdp = _kdp.getPhidpMeanUnfold();
  const double *phidpSdevForKdp = _kdp.getPhidpSdev();
  const double *phidpJitterForKdp = _kdp.getPhidpJitter();
  const double *phidpUnfoldForKdp = _kdp.getPhidpUnfold();
  const double *phidpFiltForKdp = _kdp.getPhidpFilt();
  const double *phidpCondForKdp = _kdp.getPhidpCond();
  const double *phidpCondFiltForKdp = _kdp.getPhidpCondFilt();
  const double *psob = _kdp.getPsob();

  const double *dbzAtten = _kdp.getDbzAttenCorr();
  const double *zdrAtten = _kdp.getZdrAttenCorr();
  
  const double *snrRlan = _intf.getSnr();
  const double *snrModeRlan = _intf.getSnrMode();
  const double *snrDModeRlan = _intf.getSnrDMode();
  const double *zdrRlan = _intf.getZdr();
  const double *zdrModeRlan = _intf.getZdrMode();
  const double *zdrDModeRlan = _intf.getZdrDMode();
  const double *ncpMeanRlan = _intf.getNcpMean();
  const double *phaseRlan = _intf.getPhase();
  const double *phaseChangeRlan = _intf.getPhaseChangeError();
  const bool *rlanFlag = _intf.getRlanFlag();

  // load up output data

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    const Params::output_field_t &ofld = _params._output_fields[ifield];

    // fill data array
    
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(derivedRay->getNGates());
    Radx::fl32 *datp = data;
    
    for (int igate = 0; igate < (int) derivedRay->getNGates(); 
         igate++, datp++) {
      
      switch (ofld.id) {

        // computed fields

        case Params::SNR:
          *datp = _snrArray[igate];
          break;
        case Params::DBZ:
          *datp = _dbzArray[igate];
          break;
        case Params::VEL:
          *datp = _velArray[igate];
          break;
        case Params::WIDTH:
          *datp = _widthArray[igate];
          break;
        case Params::NCP:
          *datp = _ncpArray[igate];
          break;
        case Params::ZDR:
          *datp = _zdrArray[igate];
          break;
        case Params::ZDRM:
          *datp = _zdrmArray[igate];
          break;
        case Params::LDR:
          *datp = _ldrArray[igate];
          break;
        case Params::RHOHV:
          *datp = _rhohvArray[igate];
          break;
        case Params::RHOHV_NNC:
          *datp = _rhohvNncArray[igate];
          break;
        case Params::PHIDP:
          *datp = _phidpArray[igate];
          break;
        case Params::KDP:
          *datp = _kdpArray[igate];
          break;
        case Params::PSOB:
          *datp = psob[igate];
          break;
        case Params::ZDP:
          *datp = _zdpArray[igate];
          break;
          
          // kdp
          
        case Params::DBZ_FOR_KDP:
          *datp = dbzForKdp[igate];
          break;
        case Params::ZDR_FOR_KDP:
          *datp = zdrForKdp[igate];
          break;
        case Params::RHOHV_FOR_KDP:
          *datp = rhohvForKdp[igate];
          break;
        case Params::SNR_FOR_KDP:
          *datp = snrForKdp[igate];
          break;
        case Params::ZDR_SDEV_FOR_KDP:
          *datp = zdrSdevForKdp[igate];
          break;
        case Params::VALID_FLAG_FOR_KDP:
          if (validFlagForKdp[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;

        case Params::PHIDP_FOR_KDP:
          *datp = phidpForKdp[igate];
          break;
        case Params::PHIDP_MEAN_FOR_KDP:
          *datp = phidpMeanForKdp[igate];
          break;
        case Params::PHIDP_MEAN_UNFOLD_FOR_KDP:
          *datp = phidpMeanUnfoldForKdp[igate];
          break;
        case Params::PHIDP_SDEV_FOR_KDP:
          *datp = phidpSdevForKdp[igate];
          break;
        case Params::PHIDP_JITTER_FOR_KDP:
          *datp = phidpJitterForKdp[igate];
          break;
        case Params::PHIDP_UNFOLD_FOR_KDP:
          *datp = phidpUnfoldForKdp[igate];
          break;
        case Params::PHIDP_FILT_FOR_KDP:
          *datp = phidpFiltForKdp[igate];
          break;
        case Params::PHIDP_COND_FOR_KDP:
          *datp = phidpCondForKdp[igate];
          break;
        case Params::PHIDP_COND_FILT_FOR_KDP:
          *datp = phidpCondFiltForKdp[igate];
          break;

          // attenuation

        case Params::DBZ_ATTEN_CORRECTION:
          *datp = dbzAtten[igate];
          break;
        case Params::ZDR_ATTEN_CORRECTION:
          *datp = zdrAtten[igate];
          break;
        case Params::DBZ_ATTEN_CORRECTED:
          if (dbzAtten[igate] != missingDbl &&
              _dbzArray[igate] != missingDbl) {
            *datp = dbzAtten[igate] + _dbzArray[igate];
          }
          break;
        case Params::ZDR_ATTEN_CORRECTED:
          if (zdrAtten[igate] != missingDbl &&
              _zdrArray[igate] != missingDbl) {
            *datp = zdrAtten[igate] + _zdrArray[igate];
          }
          break;

        case Params::SNR_RLAN:
          *datp = snrRlan[igate];
          break;
        case Params::SNR_MODE_RLAN:
          *datp = snrModeRlan[igate];
          break;
        case Params::SNR_DMODE_RLAN:
          *datp = snrDModeRlan[igate];
          break;
        case Params::ZDR_RLAN:
          *datp = zdrRlan[igate];
          break;
        case Params::ZDR_MODE_RLAN:
          *datp = zdrModeRlan[igate];
          break;
        case Params::ZDR_DMODE_RLAN:
          *datp = zdrDModeRlan[igate];
          break;
        case Params::NCP_MEAN_RLAN:
          *datp = ncpMeanRlan[igate];
          break;
        case Params::PHASE_RLAN:
          *datp = phaseRlan[igate];
          break;
        case Params::PHASE_CHANGE_RLAN:
          *datp = phaseChangeRlan[igate];
          break;
        case Params::RLAN_FLAG:
          if (rlanFlag[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;

      } // switch

    } // igate

    // create field
    
    RadxField *field = new RadxField(ofld.name, ofld.units);
    field->setLongName(ofld.long_name);
    field->setStandardName(ofld.standard_name);
    field->setTypeFl32(missingDbl);
    field->addDataFl32(derivedRay->getNGates(), data);
    field->copyRangeGeom(*inputRay);

    // add to ray

    derivedRay->addField(field);

  } // ifield

  // copy fields through as required

  if (_params.copy_input_fields_to_output) {

    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      const Params::copy_field_t &cfield = _params._copy_fields[ii];
      string inName = cfield.input_name;
      RadxField *inField = inputRay->getField(inName);
      if (inField != NULL) {
        RadxField *outField = new RadxField(*inField);
        outField->setName(cfield.output_name);
        if (cfield.censor_non_precip) {
          _censorNonPrecip(*outField);
        }
        derivedRay->addField(outField);
      }
    } // ii

  } // if (_params.copy_input_fields_to_output)

}

//////////////////////////////////////
// initialize KDP
  
void ComputeEngine::_kdpInit()
  
{

  // initialize KDP object

  if (_params.KDP_fir_filter_len == Params::FIR_LEN_125) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_125);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_60) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_60);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_40) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_40);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_30) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_30);
  } else if (_params.KDP_fir_filter_len == Params::FIR_LEN_20) {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_20);
  } else {
    _kdp.setFIRFilterLen(KdpFilt::FIR_LENGTH_10);
  }
  _kdp.setNGatesStats(_params.KDP_ngates_for_stats);
  _kdp.setMinValidAbsKdp(_params.KDP_min_valid_abs_kdp);
  if (_params.set_max_range) {
    _kdp.setMaxRangeKm(true, _params.max_range_km);
  }
  _kdp.setNFiltIterUnfolded(_params.KDP_n_filt_iterations_unfolded);
  _kdp.setNFiltIterCond(_params.KDP_n_filt_iterations_conditioned);
  if (_params.KDP_use_iterative_filtering) {
    _kdp.setUseIterativeFiltering(true);
    _kdp.setPhidpDiffThreshold(_params.KDP_phidp_difference_threshold);
  }
  _kdp.setPhidpSdevMax(_params.KDP_phidp_sdev_max);
  _kdp.setPhidpJitterMax(_params.KDP_phidp_jitter_max);
  _kdp.setMinValidAbsKdp(_params.KDP_min_valid_abs_kdp);
  _kdp.checkSnr(_params.KDP_check_snr);
  _kdp.setSnrThreshold(_params.KDP_snr_threshold);
  _kdp.checkRhohv(_params.KDP_check_rhohv);
  _kdp.setRhohvThreshold(_params.KDP_rhohv_threshold);
  if (_params.KDP_check_zdr_sdev) {
    _kdp.checkZdrSdev(true);
  }
  _kdp.setZdrSdevMax(_params.KDP_zdr_sdev_max);
  if (_params.KDP_debug) {
    _kdp.setDebug(true);
  }
  if (_params.KDP_write_ray_files) {
    _kdp.setWriteRayFile(true, _params.KDP_ray_files_dir);
  }

  if (_params.apply_precip_attenuation_correction) {
    if (_params.specify_coefficients_for_attenuation_correction) {
      _kdp.setAttenCoeffs(_params.dbz_attenuation_coefficient,
                          _params.dbz_attenuation_exponent,
                          _params.zdr_attenuation_coefficient,
                          _params.zdr_attenuation_exponent);
    } else {
      _kdp.setComputeAttenCorr(true);
    }
  }

}

////////////////////////////////////////////////
// compute kdp from phidp, using Bringi's method

void ComputeEngine::_kdpCompute()
  
{

  // set up array for range
  
  TaArray<double> rangeKm_;
  double *rangeKm = rangeKm_.alloc(_nGates);
  double range = _startRangeKm;
  for (int ii = 0; ii < _nGates; ii++, range += _gateSpacingKm) {
    rangeKm[ii] = range;
  }

  // compute KDP
  
  _kdp.compute(_timeSecs,
               _nanoSecs / 1.0e9,
               _elevation,
               _azimuth,
               _wavelengthM * 100.0,
               _nGates, 
               _startRangeKm,
               _gateSpacingKm,
               _snrArray,
               _dbzArray,
               _zdrArray,
               _rhohvArray,
               _phidpArray,
               missingDbl);

  const double *kdp = _kdp.getKdp();
  
  // put KDP into fields objects
  
  for (int ii = 0; ii < _nGates; ii++) {
    if (kdp[ii] == NAN) {
      _kdpArray[ii] = missingDbl;
    } else {
      _kdpArray[ii] = kdp[ii];
    }
  }

}

//////////////////////////////////////
// Locate RLAN interference

void ComputeEngine::_locateRlan()
  
{

  // set up RLAN

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _intf.setDebug(true);
  }

  _intf.setNGatesKernel(9);

  _intf.setRayProps(_timeSecs,
                    _nanoSecs,
                    _elevation,
                    _azimuth,
                    _nGates,
                    _startRangeKm,
                    _gateSpacingKm,
                    _wavelengthM,
                    _nyquist);

  _intf.setFields(_snrArray,
                  _velArray,
                  _widthArray,
                  _ncpArray,
                  _zdrArray,
                  missingDbl);

  // locate RLAN interference

  _intf.locate();

}

//////////////////////////////////////
// alloc moments arrays
  
void ComputeEngine::_allocMomentsArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _velArray = _velArray_.alloc(_nGates);
  _widthArray = _widthArray_.alloc(_nGates);
  _ncpArray = _ncpArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _zdrmArray = _zdrmArray_.alloc(_nGates);
  _zdpArray = _zdpArray_.alloc(_nGates);
  _kdpArray = _kdpArray_.alloc(_nGates);
  _ldrArray = _ldrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _rhohvNncArray = _rhohvNncArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);
  _rhoVxHxArray = _rhoVxHxArray_.alloc(_nGates);

}

/////////////////////////////////////////////////////
// load momemts arrays ready for KDP, PID and PRECIP
  
int ComputeEngine::_loadMomentsArrays(RadxRay *inputRay)
  
{
  
  if (_loadFieldArray(inputRay, _params.DBZ_field_name,
                      true, _dbzArray)) {
    return -1;
  }

  if (_params.SNR_available) {
    if (_loadFieldArray(inputRay, _params.SNR_field_name,
                        true, _snrArray)) {
      return -1;
    }
  } else {
    _computeSnrFromDbz();
  }
  
  if (_loadFieldArray(inputRay, _params.ZDR_field_name,
                      true, _zdrArray)) {
    return -1;
  }

  if (_loadFieldArray(inputRay, _params.VEL_field_name,
                      true, _velArray)) {
    return -1;
  }

  if (_loadFieldArray(inputRay, _params.PHIDP_field_name,
                      true, _phidpArray)) {
    return -1;
  }
  
  if (_loadFieldArray(inputRay, _params.RHOHV_field_name,
                      true, _rhohvArray)) {
    return -1;
  }
  
  for (int igate = 0; igate < _nGates; igate++) {
    _kdpArray[igate] = missingDbl;
  }

  if (_params.locate_rlan_interference) {

    if (_loadFieldArray(inputRay, _params.VEL_field_name,
                        true, _velArray)) {
      return -1;
    }

    if (_params.WIDTH_available) {
      if (_loadFieldArray(inputRay, _params.WIDTH_field_name,
                          true, _widthArray)) {
        return -1;
      }
    } else {
      for (int igate = 0; igate < _nGates; igate++) {
        _widthArray[igate] = missingDbl;
      }
    }
    
    if (_params.NCP_available) {
      if (_loadFieldArray(inputRay, _params.NCP_field_name,
                          true, _ncpArray)) {
        return -1;
      }
    } else {
      for (int igate = 0; igate < _nGates; igate++) {
        _ncpArray[igate] = missingDbl;
      }
    }

  }

  return 0;
  
}

/////////////////////////////////////////////////////
// compute zdp from dbz and zdr
//
// See Bringi and Chandrasekar, p438.
  
void ComputeEngine::_computeZdpArray()
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {

    double dbz = _dbzArray[igate];
    double zh = pow(10.0, dbz / 10.0);
    double zdr = _zdrArray[igate];
    double zdrLinear = pow(10.0, zdr / 10.0);
    double zv = zh / zdrLinear;
    double zdp = missingDbl;
    if (zh > zv) {
      zdp = 10.0 * log10(zh - zv);
    }
    _zdpArray[igate] = zdp;

  } // igate

}

////////////////////////////////////////
// load a field array based on the name

int ComputeEngine::_loadFieldArray(RadxRay *inputRay,
                                   const string &fieldName,
                                   bool required,
                                   double *array)

{
  
  RadxField *field = inputRay->getField(fieldName);
  if (field == NULL) {

    if (!required) {
      for (int igate = 0; igate < _nGates; igate++) {
        array[igate] = missingDbl;
      }
      return -1;
    }

    pthread_mutex_lock(&_debugPrintMutex);
    cerr << "ERROR - ComputeEngine::_getField" << endl;
    cerr << "  Cannot find field in ray: " << fieldName<< endl;
    cerr << "  El, az: "
         << inputRay->getElevationDeg() << ", "
         << inputRay->getAzimuthDeg() << endl;
    cerr << "  N fields in ray: " << inputRay->getNFields() << endl;
    pthread_mutex_unlock(&_debugPrintMutex);
    return -1;
  }
  
  // convert field data to floats

  field->convertToFl64();
  const double *vals = field->getDataFl64();
  double missingVal = field->getMissingFl64();
  for (int igate = 0; igate < _nGates; igate++, vals++) {
    double val = *vals;
    if (val == missingVal) {
      array[igate] = missingDbl;
    } else {
      array[igate] = val;
    }
  }
  
  return 0;
  
}

//////////////////////////////////////////////////////////////
// Compute the SNR field from the DBZ field

void ComputeEngine::_computeSnrFromDbz()

{

  // compute noise at each gate

  TaArray<double> noiseDbz_;
  double *noiseDbz = noiseDbz_.alloc(_nGates);
  double range = _startRangeKm;
  if (range == 0) {
    range = _gateSpacingKm / 10.0;
  }
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacingKm) {
    noiseDbz[igate] = _params.noise_dbz_at_100km +
      20.0 * (log10(range) - log10(100.0));
  }

  // compute snr from dbz
  
  double *snr = _snrArray;
  const double *dbz = _dbzArray;
  for (int igate = 0; igate < _nGates; igate++, snr++, dbz++) {
    if (*dbz != missingDbl) {
      *snr = *dbz - noiseDbz[igate];
    } else {
      *snr = -20;
    }
  }

}

//////////////////////////////////////////////////////////////
// Censor gates with non-precip particle types

void ComputeEngine::_censorNonPrecip(RadxField &field)

{

  // const int *pid = _pid.getPid();
  // for (int ii = 0; ii < _nGates; ii++) {
  //   int ptype = pid[ii];
  //   if (ptype == NcarParticleId::FLYING_INSECTS ||
  //       ptype == NcarParticleId::SECOND_TRIP ||
  //       ptype == NcarParticleId::GROUND_CLUTTER ||
  //       ptype < NcarParticleId::CLOUD ||
  //       ptype > NcarParticleId::SATURATED_SNR) {
  //     field.setGateToMissing(ii);
  //   }
  // } // ii

}

