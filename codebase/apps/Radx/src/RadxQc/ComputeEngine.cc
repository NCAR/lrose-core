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
  _nWarnCensorPrint = 0;
  
  // initialize kdp, pid
  // set up interest maps etc

  _kdpInit();
  
  if (_pidInit()) {
    OK = false;
  }

  if (_rlanInit()) {
    OK = false;
  }

  if (_seaclutInit()) {
    OK = false;
  }

}
  
// destructor

ComputeEngine::~ComputeEngine()

{

}

//////////////////////////////////////////////////
// compute for given input ray
// storing results in derived ray
//
// Creates derived ray and returns it.
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

  // allocate arrays for computing derived fields,
  // and load them up
  
  _allocArrays();
  _loadMomentsArrays(inputRay);
  
  // set flag for input field censoring

  _setInputCensoringFlag(*inputRay);

  // compute ZDP

  _computeZdpArray();
  
  // compute kdp if needed
  
  if (_params.PHIDP_available) {
    _kdpCompute();
  } else {
    _kdp.initializeArrays(_nGates);
  }

  // locate RLAN interference

  if (_params.locate_rlan_interference) {
    _locateRlan();
  }

  // locate sea clutter

  if (_params.locate_sea_clutter) {
    _locateSeaClutter();
  }

  // compute pid

  if (_params.compute_pid) {
    _pidCompute();
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
  const double *zdrSdevRlan = _intf.getZdrSdev();
  const double *ncpMeanRlan = _intf.getNcpMean();
  const double *widthMeanRlan = _intf.getWidthMean();
  const double *phaseRlan = _intf.getPhase();
  const double *phaseNoiseRlan = _intf.getPhaseNoise();

  const double *phaseNoiseInterestRlan = _intf.getPhaseNoiseInterest();
  const double *ncpMeanInterestRlan = _intf.getNcpMeanInterest();
  const double *widthMeanInterestRlan = _intf.getWidthMeanInterest();
  const double *snrDModeInterestRlan = _intf.getSnrDModeInterest();
  const double *zdrSdevInterestRlan = _intf.getZdrSdevInterest();

  const bool *rlanFlag = _intf.getRlanFlag();
  
  const double *snrMeanSeaclut = _seaclut.getSnrMean();
  const double *rhohvMeanSeaclut = _seaclut.getRhohvMean();
  const double *phidpSdevSeaclut = _seaclut.getPhidpSdev();
  const double *zdrSdevSeaclut = _seaclut.getZdrSdev();

  const double *rhohvMeanInterestSeaclut = _seaclut.getRhohvMeanInterest();
  const double *phidpSdevInterestSeaclut = _seaclut.getPhidpSdevInterest();
  const double *zdrSdevInterestSeaclut = _seaclut.getZdrSdevInterest();
  const double *dbzElevGradientInterestSeaclut = _seaclut.getDbzElevGradientInterest();

  const bool *seaclutFlag = _seaclut.getClutFlag();

  RadxField *rayHtField = inputRay->getField(_params.ray_height_field_name);
  const Radx::fl32 *rayHt = NULL;
  if (rayHtField != NULL) {
    rayHtField->convertToFl32();
    rayHt = rayHtField->getDataFl32();
  }
  RadxField *dbzGradientField =
    inputRay->getField(_params.dbz_elevation_gradient_field_name);
  const Radx::fl32 *dbzGradient = NULL;
  if (dbzGradientField != NULL) {
    dbzGradientField->convertToFl32();
    dbzGradient = dbzGradientField->getDataFl32();
  }

  // load up output data

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    const Params::output_field_t &ofld = _params._output_fields[ifield];

    // fill data array
    
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(derivedRay->getNGates());
    for (size_t igate = 0; igate < derivedRay->getNGates(); igate++) {
      data[igate] = Radx::missingFl32;
    }
    Radx::fl32 *datp = data;
    
    for (size_t igate = 0; igate < derivedRay->getNGates();  igate++, datp++) {
      
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
        case Params::LDR:
          *datp = _ldrArray[igate];
          break;
        case Params::RHOHV:
          *datp = _rhohvArray[igate];
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
        case Params::ZDR_SDEV_RLAN:
          *datp = zdrSdevRlan[igate];
          break;
        case Params::NCP_MEAN_RLAN:
          *datp = ncpMeanRlan[igate];
          break;
        case Params::WIDTH_MEAN_RLAN:
          *datp = widthMeanRlan[igate];
          break;
        case Params::PHASE_RLAN:
          *datp = phaseRlan[igate];
          break;
        case Params::PHASE_NOISE_RLAN:
          *datp = phaseNoiseRlan[igate];
          break;

        case Params::RLAN_FLAG:
          if (rlanFlag[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;

        case Params::PHASE_NOISE_INTEREST_RLAN:
          *datp = phaseNoiseInterestRlan[igate];
          break;
        case Params::NCP_MEAN_INTEREST_RLAN:
          *datp = ncpMeanInterestRlan[igate];
          break;
        case Params::WIDTH_MEAN_INTEREST_RLAN:
          *datp = widthMeanInterestRlan[igate];
          break;
        case Params::SNR_DMODE_INTEREST_RLAN:
          *datp = snrDModeInterestRlan[igate];
          break;
        case Params::ZDR_SDEV_INTEREST_RLAN:
          *datp = zdrSdevInterestRlan[igate];
          break;

        case Params::RAY_HEIGHT:
          if (rayHt != NULL) {
            *datp = rayHt[igate];
          }
          break;
        case Params::DBZ_ELEV_GRADIENT_SEACLUT:
          if (dbzGradient != NULL) {
            *datp = dbzGradient[igate];
          }
          break;

        case Params::SNR_MEAN_SEACLUT:
          *datp = snrMeanSeaclut[igate];
          break;
        case Params::RHOHV_MEAN_SEACLUT:
          *datp = rhohvMeanSeaclut[igate];
          break;
        case Params::PHIDP_SDEV_SEACLUT:
          *datp = phidpSdevSeaclut[igate];
          break;
        case Params::ZDR_SDEV_SEACLUT:
          *datp = zdrSdevSeaclut[igate];
          break;
        case Params::RHOHV_MEAN_INTEREST_SEACLUT:
          *datp = rhohvMeanInterestSeaclut[igate];
          break;
        case Params::PHIDP_SDEV_INTEREST_SEACLUT:
          *datp = phidpSdevInterestSeaclut[igate];
          break;
        case Params::ZDR_SDEV_INTEREST_SEACLUT:
          *datp = zdrSdevInterestSeaclut[igate];
          break;
        case Params::DBZ_ELEV_GRADIENT_INTEREST_SEACLUT:
          *datp = dbzElevGradientInterestSeaclut[igate];
          break;
        case Params::SEACLUT_FLAG:
          if (seaclutFlag[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;

        case Params::TEMP_FOR_PID:
          *datp = _tempForPid[igate];
          break;

        case Params::PARTICLE_ID:
          {
            int pid = _pidArray[igate];
            if (pid > 0) {
              *datp = pid;
            } else {
              *datp = missingDbl;
            }
          }
          break;
          
        case Params::PID_CENSOR_FLAG:
          if (_pidCensorFlag[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;


        case Params::INPUT_FIELDS_CENSOR_FLAG:
          if (_inputCensorFlag[igate]) {
            *datp = 1.0;
          } else {
            *datp = 0.0;
          }
          break;

        case Params::COMBINED_CENSOR_FLAG:
          if (_combinedCensorFlag[igate]) {
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

  // if required, output individual PID particle interest fields

  if (_params.PID_output_particle_interest_fields) {
    
    const vector<NcarParticleId::Particle*> plist = _pid.getParticleList();
    for (size_t ii = 0; ii < plist.size(); ii++) {

      const NcarParticleId::Particle *particle = plist[ii];
      string fieldName = particle->label;
      fieldName += "_interest";

      // fill data array
      
      TaArray<Radx::fl32> data_;
      Radx::fl32 *data = data_.alloc(derivedRay->getNGates());

      const double *interest = particle->gateInterest;
      for (int igate = 0; igate < (int) derivedRay->getNGates(); igate++) {
        data[igate] = interest[igate];
      } // igate
      
      // create field
      
      RadxField *field = new RadxField(fieldName, "");
      field->setTypeFl32(missingDbl);
      field->addDataFl32(derivedRay->getNGates(), data);
      
      // add to ray
      
      derivedRay->addField(field);
      
    } // ii
    
  } // if (_params.PID_output_particle_interest_fields) ...
  
  // copy fields through as required

  if (_params.write_censored_fields_to_output) {

    for (int ii = 0; ii < _params.censored_output_fields_n; ii++) {

      // find the field

      const Params::censored_output_field_t &cField =
        _params._censored_output_fields[ii];
      string inName = cField.input_name;
      RadxField *inField = inputRay->getField(inName);
      if (inField == NULL) {
        continue;
      }

      // clear the combined censor flag

      for (int igate = 0; igate < _nGates; igate++) {
        _combinedCensorFlag[igate] = false;
      }

      // set the flag depending on censoring scheme
      
      if (cField.apply_rlan_censoring) {
        _censorOnRlan();
      }
      if (cField.apply_seaclut_censoring) {
        _censorOnSeaClutter();
      }
      if (cField.apply_pid_censoring) {
        _censorOnPid();
      }
      if (cField.apply_input_field_censoring) {
        _censorOnInputFields();
      }

      if (_params.censoring_ignore_isolated_gates) {
        _ignoreShortCensorRuns();
      }
      if (_params.censoring_fill_in_gaps) {
        _fillInCensorGaps();
      }

      // create censored field

      RadxField *outField = new RadxField(*inField);
      outField->setName(cField.output_name);
      derivedRay->addField(outField);
      _censorOnCombinedFlag(*outField);

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
// Initialize RLAN interference

int ComputeEngine::_rlanInit()
  
{

  int iret = 0;

  _intf.setMinRaySnr(_params.rlan_min_ray_snr_db);
  _intf.setMinRayFraction(_params.rlan_min_ray_fraction);
  _intf.setMinRaySnrForZdr(_params.rlan_min_ray_snr_for_zdr_sdev_db);
  _intf.setMaxElevDeg(_params.rlan_max_elev_deg);

  // set up interest maps for RLAN interference

  if (_convertInterestParamsToVector
      ("rlan_phase_noise",
       _params._rlan_phase_noise_interest_map,
       _params.rlan_phase_noise_interest_map_n,
       _rlanImapPhaseNoise)) {
    iret = -1;
  } else {
    _intf.setInterestMapPhaseNoise(_rlanImapPhaseNoise,
                                   _params.rlan_phase_noise_weight);
  }
  
  if (_convertInterestParamsToVector
      ("rlan_ncp_mean",
       _params._rlan_ncp_mean_interest_map,
       _params.rlan_ncp_mean_interest_map_n,
       _rlanImapNcpMean)) {
    iret = -1;
  } else {
    _intf.setInterestMapNcpMean(_rlanImapNcpMean,
                                _params.rlan_ncp_mean_weight);
  }
  
  if (_convertInterestParamsToVector
      ("rlan_width_mean",
       _params._rlan_width_mean_interest_map,
       _params.rlan_width_mean_interest_map_n,
       _rlanImapWidthMean)) {
    iret = -1;
  } else {
    _intf.setInterestMapWidthMean(_rlanImapWidthMean,
                                  _params.rlan_width_mean_weight);
  }
  
  if (_convertInterestParamsToVector
      ("rlan_snr_dmode",
       _params._rlan_snr_dmode_interest_map,
       _params.rlan_snr_dmode_interest_map_n,
       _rlanImapSnrDMode)) {
    iret = -1;
  } else {
    _intf.setInterestMapSnrDMode(_rlanImapSnrDMode,
                                 _params.rlan_snr_dmode_weight);
  }
  _intf.setRlanInterestThreshold(_params.rlan_interest_threshold);

  if (_convertInterestParamsToVector
      ("rlan_zdr_sdev",
       _params._rlan_zdr_sdev_interest_map,
       _params.rlan_zdr_sdev_interest_map_n,
       _rlanImapZdrSdev)) {
    iret = -1;
  } else {
    _intf.setInterestMapZdrSdev(_rlanImapZdrSdev,
                                _params.rlan_zdr_sdev_weight);
  }

  return iret;

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
  _intf.setWavelengthM(_wavelengthM);
  _intf.setRadarHtM(_radarHtKm * 1000.0);

  _intf.setRayProps(_timeSecs,
                    _nanoSecs,
                    _elevation,
                    _azimuth,
                    _nGates,
                    _startRangeKm,
                    _gateSpacingKm);

  _intf.setFieldMissingVal(missingDbl);
  
  _intf.setVelField(_velArray, _nyquist);

  if (_params.NCP_available) {
    _intf.setNcpField(_ncpArray);
  }
  if (_params.WIDTH_available) {
    _intf.setWidthField(_widthArray);
  }

  if (_params.SNR_available) {
    _intf.setSnrField(_snrArray);
  } else {
    _intf.setDbzField(_dbzArray, _params.noise_dbz_at_100km);
  }

  if (_params.ZDR_available) {
    _intf.setZdrField(_zdrArray);
  }

  // locate RLAN interference

  _intf.rlanLocate();

}

//////////////////////////////////////
// Initialize Sea Clutter

int ComputeEngine::_seaclutInit()
  
{
  
  int iret = 0;

  if (_convertInterestParamsToVector
      ("seaclut_rhohv_mean",
       _params._seaclut_rhohv_mean_interest_map,
       _params.seaclut_rhohv_mean_interest_map_n,
       _seaclutImapRhohvMean)) {
    iret = -1;
  } else {
    _seaclut.setInterestMapRhohvMean
      (_seaclutImapRhohvMean,
       _params.seaclut_rhohv_mean_weight);
  }
  
  if (_convertInterestParamsToVector
      ("seaclut_phidp_sdev",
       _params._seaclut_phidp_sdev_interest_map,
       _params.seaclut_phidp_sdev_interest_map_n,
       _seaclutImapPhidpSdev)) {
    iret = -1;
  } else {
    _seaclut.setInterestMapPhidpSdev
      (_seaclutImapPhidpSdev,
       _params.seaclut_phidp_sdev_weight);
  }
  
  if (_convertInterestParamsToVector
      ("seaclut_zdr_sdev",
       _params._seaclut_zdr_sdev_interest_map,
       _params.seaclut_zdr_sdev_interest_map_n,
       _seaclutImapZdrSdev)) {
    iret = -1;
  } else {
    _seaclut.setInterestMapZdrSdev
      (_seaclutImapZdrSdev,
       _params.seaclut_zdr_sdev_weight);
  }
  
  if (_convertInterestParamsToVector
      ("seaclut_dbz_elev_gradient",
       _params._seaclut_dbz_elev_gradient_interest_map,
       _params.seaclut_dbz_elev_gradient_interest_map_n,
       _seaclutImapDbzElevGradient)) {
    iret = -1;
  } else {
    _seaclut.setInterestMapDbzElevGradient
      (_seaclutImapDbzElevGradient,
       _params.seaclut_dbz_elev_gradient_weight);
  }
  
  _seaclut.setMinSnrDb(_params.seaclut_min_snr_db);
  _seaclut.setMaxElevDeg(_params.seaclut_max_elev_deg);

  return iret;

}

//////////////////////////////////////
// Locate Sea Clutter

void ComputeEngine::_locateSeaClutter()
  
{

  // initialize sea cutter object

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _seaclut.setDebug(true);
  }

  _seaclut.setNGatesKernel(9);
  _seaclut.setWavelengthM(_wavelengthM);
  _seaclut.setRadarHtM(_radarHtKm * 1000.0);

  _seaclut.setRayProps(_timeSecs,
                       _nanoSecs,
                       _elevation,
                       _azimuth,
                       _nGates,
                       _startRangeKm,
                       _gateSpacingKm);
  
  _seaclut.setFieldMissingVal(missingDbl);
  
  if (_params.SNR_available) {
    _seaclut.setSnrField(_snrArray);
  } else {
    _seaclut.setDbzField(_dbzArray, _params.noise_dbz_at_100km);
  }

  if (_params.RHOHV_available) {
    _seaclut.setRhohvField(_rhohvArray);
  }

  if (_params.PHIDP_available) {
    _seaclut.setPhidpField(_phidpArray);
  }

  if (_params.ZDR_available) {
    _seaclut.setZdrField(_zdrArray);
  }

  if (_dbzElevGradientAvail) {
    _seaclut.setDbzElevGradientField(_dbzElevGradientArray);
  }

  // locate sea clutter

  _seaclut.locate();

}

//////////////////////////////////////
// initialize pid computations
  
int ComputeEngine::_pidInit()
  
{

  _pid.setSnrThresholdDb(_params.PID_snr_threshold);
  _pid.setSnrUpperThresholdDb(_params.PID_snr_upper_threshold);

  if (_params.PID_apply_median_filter_to_DBZ) {
    _pid.setApplyMedianFilterToDbz(_params.PID_DBZ_median_filter_len);
  }
  if (_params.PID_apply_median_filter_to_ZDR) {
    _pid.setApplyMedianFilterToZdr(_params.PID_ZDR_median_filter_len);
  }

  if (_params.PID_apply_median_filter_to_LDR) {
    _pid.setApplyMedianFilterToLdr(_params.PID_LDR_median_filter_len);
  }
  if (_params.PID_replace_missing_LDR) {
    _pid.setReplaceMissingLdr(_params.PID_LDR_replacement_value);
  }

  if (_params.PID_apply_median_filter_to_RHOHV) {
    _pid.setApplyMedianFilterToRhohv(_params.PID_RHOHV_median_filter_len);
  }
  if (_params.apply_median_filter_to_PID) {
    _pid.setApplyMedianFilterToPid(_params.PID_median_filter_len);
  }

  _pid.setNgatesSdev(_params.PID_ngates_for_sdev);
  _pid.setMinValidInterest(_params.PID_min_valid_interest);
  
  _pid.setMissingDouble(missingDbl);

  _pid.setDebug(false);
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _pid.setDebug(true);
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _pid.setVerbose(true);
  }
  if (_pid.readThresholdsFromFile(_params.pid_thresholds_file_path)) {
    cerr << "ERROR - RadxPartRain::ComputeEngine::_run" << endl;
    cerr << "  Cannot read in pid thresholds from file: "
         << _params.pid_thresholds_file_path << endl;
    cerr << "  PID will not be computed" << endl;
    return -1;
  }
  
  return 0;
  
}

//////////////////////////////////////
// compute PID

void ComputeEngine::_pidCompute()
  
{
  
  // override temp profile if appropriate
  
  if (_params.use_soundings_from_spdb) {
    if (_tempProfile) {
      const vector<TempProfile::PointVal> &profile = _tempProfile->getProfile();
      if (profile.size() > 0) {
        _pid.setTempProfile(profile);
      }
    }
  }

  // fill temperature array
  
  _pid.fillTempArray(_radarHtKm,
                     _params.override_standard_pseudo_earth_radius,
                     _params.pseudo_earth_radius_ratio,
                     _elevation, _nGates,
                     _startRangeKm,
                     _gateSpacingKm,
                     _tempForPid);

  // compute particle ID
  
  _pid.computePidBeam(_nGates, _snrArray, _dbzArray, 
                      _zdrArray, _kdpArray, _ldrArray, 
                      _rhohvArray, _phidpArray, _tempForPid);
  
  // load results

  memcpy(_pidArray, _pid.getPid(), _nGates * sizeof(int));
  memcpy(_pidInterest, _pid.getInterest(), _nGates * sizeof(double));

  // set censoring flag

  for (int igate = 0; igate < _nGates; igate++) {
    bool setFlag = false;
    int pidVal = _pidArray[igate];
    for (int ii = 0; ii < _params.PID_censoring_flag_vals_n; ii++) {
      if (pidVal == _params._PID_censoring_flag_vals[ii]) {
        setFlag = true;
        break;
      }
    } // ii
    if (setFlag) {
      _pidCensorFlag[igate] = true;
    } else {
      _pidCensorFlag[igate] = false;
    }
  } // igate
  
}

//////////////////////////////////////
// alloc arrays
  
void ComputeEngine::_allocArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _velArray = _velArray_.alloc(_nGates);
  _widthArray = _widthArray_.alloc(_nGates);
  _ncpArray = _ncpArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _ldrArray = _ldrArray_.alloc(_nGates);
  _zdpArray = _zdpArray_.alloc(_nGates);
  _kdpArray = _kdpArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);
  _dbzElevGradientArray = _dbzElevGradientArray_.alloc(_nGates);

  _pidArray = _pidArray_.alloc(_nGates);
  _pidInterest = _pidInterest_.alloc(_nGates);
  _tempForPid = _tempForPid_.alloc(_nGates);

  _pidCensorFlag = _pidCensorFlag_.alloc(_nGates);
  _inputCensorFlag = _inputCensorFlag_.alloc(_nGates);
  _combinedCensorFlag = _combinedCensorFlag_.alloc(_nGates);

}

/////////////////////////////////////////////////////
// load momemts arrays ready for KDP, PID and PRECIP
  
int ComputeEngine::_loadMomentsArrays(RadxRay *inputRay)
  
{
  
  if (_loadFieldArray(inputRay, _params.DBZ_field_name,
                      true, _dbzArray)) {
    return -1;
  }

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
  
  if (_params.SNR_available) {
    if (_loadFieldArray(inputRay, _params.SNR_field_name,
                        true, _snrArray)) {
      return -1;
    }
  } else {
    _computeSnrFromDbz();
  }
  

  if (_params.ZDR_available) {
    if (_loadFieldArray(inputRay, _params.ZDR_field_name,
                        true, _zdrArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _zdrArray[igate] = missingDbl;
    }
  }

  if (_params.LDR_available) {
    if (_loadFieldArray(inputRay, _params.LDR_field_name,
                        true, _ldrArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _ldrArray[igate] = missingDbl;
    }
  }

  if (_params.PHIDP_available) {
    if (_loadFieldArray(inputRay, _params.PHIDP_field_name,
                        true, _phidpArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _phidpArray[igate] = missingDbl;
    }
  }

  if (_params.RHOHV_available) {
    if (_loadFieldArray(inputRay, _params.RHOHV_field_name,
                        true, _rhohvArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _rhohvArray[igate] = missingDbl;
    }
  }

  for (int igate = 0; igate < _nGates; igate++) {
    _kdpArray[igate] = missingDbl;
  }
  
  if (_loadFieldArray(inputRay, _params.dbz_elevation_gradient_field_name,
                      false, _dbzElevGradientArray) == 0) {
    _dbzElevGradientAvail = true;
  } else {
    _dbzElevGradientAvail = false;
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
// Censor gates with RLAN present

void ComputeEngine::_censorOnRlan()

{

  const bool *rlanFlag = _intf.getRlanFlag();
  for (int igate = 0; igate < _nGates; igate++) {
    if (rlanFlag[igate]) {
      _combinedCensorFlag[igate] = true;
    }
  } // igate

}

//////////////////////////////////////////////////////////////
// Censor gates with sea clutter present

void ComputeEngine::_censorOnSeaClutter()
  
{

  const bool *clutFlag = _seaclut.getClutFlag();
  for (int igate = 0; igate < _nGates; igate++) {
    if (clutFlag[igate]) {
      _combinedCensorFlag[igate] = true;
    }
  } // igate

}

//////////////////////////////////////////////////////////////
// Censor gates with given particle types

void ComputeEngine::_censorOnPid()

{

  for (int igate = 0; igate < _nGates; igate++) {
    if (_pidCensorFlag[igate]) {
      _combinedCensorFlag[igate] = true;
    }
  } // igate

}

////////////////////////////////////////////////////////////////////
// set censoring flag based on input fields in a ray

void ComputeEngine::_setInputCensoringFlag(RadxRay &inputRay)
  
{

  // allocate as needed

  _inputCensorFlag = _inputCensorFlag_.alloc(_nGates);

  // check if we need input censoring
  
  bool applyInputCensoring = false;
  if (_params.write_censored_fields_to_output) {
    for (int ii = 0; ii < _params.censored_output_fields_n; ii++) {
      if (_params._censored_output_fields[ii].apply_input_field_censoring) {
        applyInputCensoring = true;
      }
    }
  }

  // if no input censoring, set flag to false everywhere
  // and return

  if (!applyInputCensoring) {
    for (int igate = 0; igate < _nGates; igate++) {
      _inputCensorFlag[igate] = false;
    }
    return;
  }

  // initialize censoring flags to true to
  // turn censoring ON everywhere
  
  for (int igate = 0; igate < _nGates; igate++) {
    _inputCensorFlag[igate] = true;
  }

  // create temporary field copies for input data
  
  vector<RadxField *> inFields;
  for (int ifield = 0; ifield < _params.censoring_input_fields_n; ifield++) {
    
    const Params::censoring_input_field_t &inField =
      _params._censoring_input_fields[ifield];
    
    string inName = inField.input_name;
    RadxField *field = inputRay.getField(inName);
    if (field != NULL) {
      field->convertToFl32();
    }
    inFields.push_back(field);
    
  } // ifield

  // check OR fields
  // if any of these have VALID data, we turn censoring OFF

  int orFieldCount = 0;

  for (int ifield = 0; ifield < _params.censoring_input_fields_n; ifield++) {
    
    const Params::censoring_input_field_t &cfld =
      _params._censoring_input_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_OR) {
      continue;
    }

    RadxField *field = inFields[ifield];
    if (field == NULL) {
      // field missing, do not censor
      if (_nWarnCensorPrint % 360 == 0) {
        cerr << "WARNING - input censoring field missing: " << cfld.input_name << endl;
        cerr << "  Censoring will not be applied for this field." << endl;
      }
      _nWarnCensorPrint++;
      for (int igate = 0; igate < _nGates; igate++) {
        _inputCensorFlag[igate] = false;
      }
      continue;
    }
    
    orFieldCount++;
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;
    
    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (int igate = 0; igate < _nGates; igate++) {
      double val = fdata[igate];
      if (val >= minValidVal && val <= maxValidVal) {
        _inputCensorFlag[igate] = false;
      }
    }
    
  } // ifield

  // if no OR fields were found, turn off ALL censoring at this stage

  if (orFieldCount == 0) {
    for (int igate = 0; igate < _nGates; igate++) {
      _inputCensorFlag[igate] = false;
    }
  }

  // check AND fields
  // if any of these have INVALID data, we turn censoring ON

  for (int ifield = 0; ifield < _params.censoring_input_fields_n; ifield++) {
    
    const Params::censoring_input_field_t &cfld =
      _params._censoring_input_fields[ifield];
    if (cfld.combination_method != Params::LOGICAL_AND) {
      continue;
    }
    
    RadxField *field = inFields[ifield];
    if (field == NULL) {
      continue;
    }
    
    double minValidVal = cfld.min_valid_value;
    double maxValidVal = cfld.max_valid_value;
    
    const Radx::fl32 *fdata = (const Radx::fl32 *) field->getData();
    for (int igate = 0; igate < _nGates; igate++) {
      double val = fdata[igate];
      if (val < minValidVal || val > maxValidVal) {
        _inputCensorFlag[igate] = true;
      }
    }
    
  } // ifield

  // check that uncensored runs meet the minimum length
  // those which do not are censored

  int minValidRun = _params.input_field_censoring_min_valid_run;
  if (minValidRun > 1) {
    int runLength = 0;
    bool doCheck = false;
    for (int igate = 0; igate < _nGates; igate++) {
      if (!_inputCensorFlag[igate]) {
        doCheck = false;
        runLength++;
      } else {
        doCheck = true;
      }
      // last gate?
      if (igate == _nGates - 1) doCheck = true;
      // check run length
      if (doCheck) {
        if (runLength < minValidRun) {
          // clear the run which is too short
          for (int jgate = igate - runLength; jgate < igate; jgate++) {
            _inputCensorFlag[jgate] = true;
          } // jgate
        }
        runLength = 0;
      } // if (doCheck ...
    } // igate
  }

}

//////////////////////////////////////////////////////////////
// Censor gates based on input field values

void ComputeEngine::_censorOnInputFields()

{

  for (int igate = 0; igate < _nGates; igate++) {
    if (_inputCensorFlag[igate]) {
      _combinedCensorFlag[igate] = true;
    }
  } // igate

}

//////////////////////////////////////////////////////////////
// Censor gates based on combined flag value

void ComputeEngine::_censorOnCombinedFlag(RadxField &field)

{

  for (int igate = 0; igate < _nGates; igate++) {
    if (_combinedCensorFlag[igate]) {
      field.setGateToMissing(igate);
    }
  } // igate

}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int ComputeEngine::_convertInterestParamsToVector(const string &label,
                                                  const Params::interest_map_point_t *map,
                                                  int nPoints,
                                                  vector<InterestMap::ImPoint> &pts)
  
{
  
  pts.clear();
  
  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      pthread_mutex_lock(&_debugPrintMutex);
      cerr << "ERROR - ComputeEngine:_convertInterestParamsToVector" << endl;
      cerr << "  Map label: " << label << endl;
      cerr << "  Map values must increase monotonically" << endl;
      pthread_mutex_unlock(&_debugPrintMutex);
      return -1;
    }
    InterestMap::ImPoint pt(map[ii].value, map[ii].interest);
    pts.push_back(pt);
    prevVal = map[ii].value;
  } // ii
  
  return 0;

}


///////////////////////////////////////////////////////////////
// apply in in-fill filter to remove small holes in censor flag

void ComputeEngine::_fillInCensorGaps()
{

  
  int *countSet = new int[_nGates];
  int *countNot = new int[_nGates];
  
  // compute the running count of gates which have the flag set and
  // those which do not
  //
  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  int nSet = 0;
  int nNot = 0;

  for (int igate = 0; igate < _nGates; igate++) {
    if (_combinedCensorFlag[igate]) {
      nSet++;
      nNot = 0;
    } else {
      nSet = 0;
      nNot++;
    }
    countSet[igate] = nSet;
    countNot[igate] = nNot;
  }

  // Go in reverse through the gates, taking the max non-zero
  // values and copying them across the set or not-set regions.
  // This makes all the counts equal in the gaps and set areas.

  for (int igate = _nGates - 2; igate >= 0; igate--) {
    if (countSet[igate] != 0 &&
        countSet[igate] < countSet[igate+1]) {
      countSet[igate] = countSet[igate+1];
    }
    if (countNot[igate] != 0 &&
        countNot[igate] < countNot[igate+1]) {
      countNot[igate] = countNot[igate+1];
    }
  }

  // fill in gaps

  for (int igate = 1; igate < _nGates - 1; igate++) {
    
    // is the gap small enough?

    nNot = countNot[igate];
    if (nNot > 0 && nNot <= _params.censoring_max_fill_ngates) {
      
      // is it surrounded by regions at least as large as the gap?

      int minGateCheck = igate - nNot;
      if (minGateCheck < 0) {
        minGateCheck = 0;
      }
      int maxGateCheck = igate + nNot;
      if (maxGateCheck > _nGates - 1) {
        maxGateCheck = _nGates - 1;
      }
      int nAdjacentBelow = 0;
      for (int jgate = igate - 1; jgate >= minGateCheck; jgate--) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentBelow = nSet;
          break;
        }
      } // jgate
      int nAdjacentAbove = 0;
      for (int jgate = igate + 1; jgate <= maxGateCheck; jgate++) {
        nSet = countSet[jgate];
        if (nSet != 0) {
          nAdjacentAbove = nSet;
          break;
        }
      } // jgate
      int minAdjacent = nAdjacentBelow;
      minAdjacent = MIN(minAdjacent, nAdjacentAbove);

      if (minAdjacent >= nNot) {
        _combinedCensorFlag[igate] = true;
      }
    }
  } // igate

  delete[] countSet;
  delete[] countNot;

}

///////////////////////////////////////////////////////////////
// ignore short runs of censoring

void ComputeEngine::_ignoreShortCensorRuns()
{

  
  int *countSet = new int[_nGates];
  int *countNot = new int[_nGates];
  
  // compute the running count of gates which have the flag set and
  // those which do not
  //
  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  int nSet = 0;
  int nNot = 0;

  for (int igate = 0; igate < _nGates; igate++) {
    if (_combinedCensorFlag[igate]) {
      nSet++;
      nNot = 0;
    } else {
      nSet = 0;
      nNot++;
    }
    countSet[igate] = nSet;
    countNot[igate] = nNot;
  }

  // Go in reverse through the gates, taking the max non-zero
  // values and copying them across the set or not-set regions.
  // This makes all the counts equal in the gaps and set areas.

  for (int igate = _nGates - 2; igate >= 0; igate--) {
    if (countSet[igate] != 0 &&
        countSet[igate] < countSet[igate+1]) {
      countSet[igate] = countSet[igate+1];
    }
    if (countNot[igate] != 0 &&
        countNot[igate] < countNot[igate+1]) {
      countNot[igate] = countNot[igate+1];
    }
  }

  // remove isolated gates

  for (int igate = 1; igate < _nGates - 1; igate++) {
    
    // is the count small enough?

    nSet = countSet[igate];
    if (nSet > 0 && nSet <= _params.censoring_max_isolated_ngates) {
      
      // is it surrounded by regions at least as large?
      
      int minGateCheck = igate - nSet;
      if (minGateCheck < 0) {
        minGateCheck = 0;
      }
      int maxGateCheck = igate + nSet;
      if (maxGateCheck > _nGates - 1) {
        maxGateCheck = _nGates - 1;
      }
      int nAdjacentBelow = 0;
      for (int jgate = igate - 1; jgate >= minGateCheck; jgate--) {
        nNot = countNot[jgate];
        if (nNot != 0) {
          nAdjacentBelow = nNot;
          break;
        }
      } // jgate
      int nAdjacentAbove = 0;
      for (int jgate = igate + 1; jgate <= maxGateCheck; jgate++) {
        nNot = countNot[jgate];
        if (nNot != 0) {
          nAdjacentAbove = nNot;
          break;
        }
      } // jgate
      int minAdjacent = nAdjacentBelow;
      minAdjacent = MIN(minAdjacent, nAdjacentAbove);
      
      if (minAdjacent >= nSet) {
        _combinedCensorFlag[igate] = false;
      }
    }
  } // igate

  delete[] countSet;
  delete[] countNot;

}
