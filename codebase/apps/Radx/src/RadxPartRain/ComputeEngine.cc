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
  if (_pidInit()) {
    OK = false;
  }
  _precipInit();

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
  _zdrInIceResults.clear();
  _zdrInBraggResults.clear();
  _zdrmInIceResults.clear();
  _zdrmInBraggResults.clear();
  _selfConResults.clear();

  // create moments ray
  
  RadxRay *derivedRay = new RadxRay;
  derivedRay->copyMetaData(*inputRay);

  // initialize precip and pid for wavelength
  
  _rate.setWavelengthCm(wavelengthM * 100.0);
  _pid.setWavelengthCm(wavelengthM * 100.0);

  // allocate moments arrays for computing derived fields,
  // and load them up
  
  _allocMomentsArrays();
  _loadMomentsArrays(inputRay);
  
  // compute ZDP

  _computeZdpArray();
  
  // compute kdp if needed
  
  if (!_params.KDP_available) {
    _kdpCompute();
  } else {
    _kdp.initializeArrays(_nGates);
  }

  // locate RLAN interference

  if (_params.locate_rlan_interference) {
    _locateRlan();
  }

  // compute pid

  _allocPidArrays();
  _pidCompute();
  
  // compute precip

  _allocPrecipArrays();
  _precipCompute();

  // accumulate stats for ZDR bias

  _allocZdrBiasArrays();
  if (_params.estimate_zdr_bias_in_ice) {
    _accumForZdrBiasInIce();
  }
  if (_params.estimate_zdr_bias_in_bragg) {
    _accumForZdrBiasInBragg();
  }

  _allocSelfConArrays();
  if (_params.estimate_z_bias_using_self_consistency) {
    _runSelfConsistencyCheck();
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
  
  const double *dbzForPrecip = _rate.getDbz();
  const double *zdrForPrecip = _rate.getZdr();
  const double *kdpForPrecip = _rate.getKdp();

  const double *dbzForPid = _pid.getDbz();
  const double *zdrForPid = _pid.getZdr();
  const double *kdpForPid = _pid.getKdp();
  const double *ldrForPid = _pid.getLdr();
  const double *rhohvForPid = _pid.getRhohv();
  const double *phidpForPid = _pid.getPhidp();
  const double *sdzdrForPid = _pid.getSdzdr();
  const double *sdphidpForPid = _pid.getSdphidp();

  const double *snrRlan = _rlan.getSnr();
  const double *snrModeRlan = _rlan.getSnrMode();
  const double *snrDModeRlan = _rlan.getSnrDMode();
  const double *zdrRlan = _rlan.getZdr();
  const double *zdrModeRlan = _rlan.getZdrMode();
  const double *zdrDModeRlan = _rlan.getZdrDMode();
  const double *ncpMeanRlan = _rlan.getNcpMean();
  const double *phaseRlan = _rlan.getPhase();
  const double *phaseChangeRlan = _rlan.getPhaseChangeError();
  const bool *rlanFlag = _rlan.getRlanFlag();

  // load up output data

  double minValidPrecipRate = _params.PRECIP_min_valid_rate;

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
        case Params::KDP_BRINGI:
          *datp = _kdpBringiArray[igate];
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

          // precip rate

        case Params::PRECIP_RATE_ZH:
          {
            double rate = _rateZ[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_ZH_SNOW:
          {
            double rate = _rateZSnow[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_Z_ZDR:
          {
            double rate = _rateZZdr[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_KDP:
          {
            double rate = _rateKdp[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_KDP_ZDR:
          {
            double rate = _rateKdpZdr[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_HYBRID:
          {
            double rate = _rateHybrid[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_PID:
          {
            double rate = _ratePid[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_HIDRO:
          {
            double rate = _rateHidro[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::PRECIP_RATE_BRINGI:
          {
            double rate = _rateBringi[igate];
            if (rate < minValidPrecipRate) {
              *datp = missingDbl;
            } else {
              *datp = rate;
            }
          }
          break;
        case Params::DBZ_FOR_RATE:
          *datp = dbzForPrecip[igate];
          break;
        case Params::ZDR_FOR_RATE:
          *datp = zdrForPrecip[igate];
          break;
        case Params::KDP_FOR_RATE:
          *datp = kdpForPrecip[igate];
          break;

          // pid

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
        case Params::PARTICLE_ID2:
          {
            int pid2 = _pidArray2[igate];
            if (pid2 > 0) {
              *datp = pid2;
            } else {
              *datp = missingDbl;
            }
          }
          break;
        case Params::PID_INTEREST:
          *datp = _pidInterest[igate];
          break;
        case Params::PID_INTEREST2:
          *datp = _pidInterest2[igate];
          break;
        case Params::DBZ_FOR_PID:
          *datp = dbzForPid[igate];
          break;
        case Params::ZDR_FOR_PID:
          *datp = zdrForPid[igate];
          break;
        case Params::LDR_FOR_PID:
          *datp = ldrForPid[igate];
          break;
        case Params::PHIDP_FOR_PID:
          *datp = phidpForPid[igate];
          break;
        case Params::RHOHV_FOR_PID:
          *datp = rhohvForPid[igate];
          break;
        case Params::KDP_FOR_PID:
          *datp = kdpForPid[igate];
          break;
        case Params::SDZDR_FOR_PID:
          *datp = sdzdrForPid[igate];
          break;
        case Params::SDPHIDP_FOR_PID:
          *datp = sdphidpForPid[igate];
          break;
        case Params::TEMP_FOR_PID:
          *datp = _tempForPid[igate];
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

        case Params::ZDRM_IN_ICE:
          *datp = _zdrmInIce[igate];
          break;
        case Params::ZDRM_IN_BRAGG:
          *datp = _zdrmInBragg[igate];
          break;

        case Params::ZDR_IN_ICE:
          *datp = _zdrInIce[igate];
          break;
        case Params::ZDR_IN_BRAGG:
          *datp = _zdrInBragg[igate];
          break;

        case Params::ZDR_FLAG_IN_ICE:
          *datp = _zdrFlagInIce[igate];
          break;
        case Params::ZDR_FLAG_IN_BRAGG:
          *datp = _zdrFlagInBragg[igate];
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
    
  } // if (_needPid ...

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

  // initialize KDP BRINGI object if required

  if (_params.compute_kdp_bringi) {

    if (_params.KDP_BRINGI_fir_filter_len == Params::FIR_LEN_125) {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_125);
    } else if (_params.KDP_BRINGI_fir_filter_len == Params::FIR_LEN_60) {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_60);
    } else if (_params.KDP_BRINGI_fir_filter_len == Params::FIR_LEN_40) {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_40);
    } else if (_params.KDP_BRINGI_fir_filter_len == Params::FIR_LEN_30) {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_30);
    } else if (_params.KDP_BRINGI_fir_filter_len == Params::FIR_LEN_20) {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_20);
    } else {
      _kdpBringi.setFIRFilterLen(KdpBringi::FIR_LENGTH_10);
    }
    if (_params.set_max_range) {
      _kdpBringi.setMaxRangeKm(true, _params.max_range_km);
    }
    _kdpBringi.setPhidpDiffThreshold(_params.KDP_BRINGI_phidp_difference_threshold);
    _kdpBringi.setPhidpSdevThreshold(_params.KDP_BRINGI_phidp_sdev_threshold);
    _kdpBringi.setZdrSdevThreshold(_params.KDP_BRINGI_phidp_sdev_threshold);
    _kdpBringi.setRhohvWxThreshold(_params.KDP_BRINGI_rhohv_threshold);
    if (_params.KDP_BRINGI_apply_median_filter_to_PHIDP) {
      _kdpBringi.setApplyMedianFilterToPhidp(_params.KDP_BRINGI_median_filter_len);
    }
    if (_params.KDP_debug) {
      _kdp.setDebug(true);
    }
    if (_params.KDP_write_ray_files) {
      _kdp.setWriteRayFile(true, _params.KDP_ray_files_dir);
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

  if (_params.compute_kdp_bringi) {

    // compute KDP BRINGI
    
    double *ranges = new double[_nGates];
    double range = _startRangeKm;
    for (int igate = 0; igate < _nGates; igate++) {
      ranges[igate] = range;
      range += _gateSpacingKm;
    }

    double *ldrArray = NULL;
    if (_params.LDR_available) {
      ldrArray = _ldrArray;
    }

    _kdpBringi.compute(_elevation,
                       _azimuth,
                       _nGates,
                       ranges,
                       _dbzArray,
                       _zdrArray,
                       _phidpArray,
                       _rhohvArray,
                       _snrArray,
                       missingDbl,
                       ldrArray);

    delete[] ranges;
    
    const double *kdpb = _kdpBringi.getKdp();
    for (int ii = 0; ii < _nGates; ii++) {
      if (kdpb[ii] == NAN) {
        _kdpBringiArray[ii] = missingDbl;
      } else {
        _kdpBringiArray[ii] = kdp[ii];
      }
    }

  } // if (_params.compute_kdp_bringi)

}

//////////////////////////////////////
// Locate RLAN interference

void ComputeEngine::_locateRlan()
  
{

  // set up RLAN

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _rlan.setDebug(true);
  }

  _rlan.setNGatesKernel(9);

  _rlan.setRayProps(_timeSecs,
                    _nanoSecs,
                    _elevation,
                    _azimuth,
                    _nGates,
                    _startRangeKm,
                    _gateSpacingKm,
                    _wavelengthM,
                    _nyquist);

  _rlan.setFields(_snrArray,
                  _velArray,
                  _widthArray,
                  _ncpArray,
                  _zdrArray,
                  missingDbl);

  // locate RLAN interference

  _rlan.locate();

#ifdef JUNK
  
  // override temp profile if appropriate
  
  if (_params.use_soundings_from_spdb) {
    if (_tempProfile) {
      const vector<NcarParticleId::TmpPoint> &profile = _tempProfile->getProfile();
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
  memcpy(_pidArray2, _pid.getPid2(), _nGates * sizeof(int));
  memcpy(_pidInterest, _pid.getInterest(), _nGates * sizeof(double));
  memcpy(_pidInterest2, _pid.getInterest2(), _nGates * sizeof(double));

#endif
  
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
      const vector<NcarParticleId::TmpPoint> &profile = _tempProfile->getProfile();
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
  memcpy(_pidArray2, _pid.getPid2(), _nGates * sizeof(int));
  memcpy(_pidInterest, _pid.getInterest(), _nGates * sizeof(double));
  memcpy(_pidInterest2, _pid.getInterest2(), _nGates * sizeof(double));
  
}

//////////////////////////////////////
// initialize precip computations
  
void ComputeEngine::_precipInit()
  
{

  _rate.setSnrThresholdDb(_params.PRECIP_snr_threshold);

  // initialize precip rate object

  _rate.setZhAa(_params.zh_aa);
  _rate.setZhBb(_params.zh_bb);

  _rate.setZhAaSnow(_params.zh_aa_snow);
  _rate.setZhBbSnow(_params.zh_bb_snow);

  _rate.setZzdrAa(_params.zzdr_aa);
  _rate.setZzdrBb(_params.zzdr_bb);
  _rate.setZzdrCc(_params.zzdr_cc);
  
  _rate.setKdpAa(_params.kdp_aa);
  _rate.setKdpBb(_params.kdp_bb);

  _rate.setKdpZdrAa(_params.kdpzdr_aa);
  _rate.setKdpZdrBb(_params.kdpzdr_bb);
  _rate.setKdpZdrCc(_params.kdpzdr_cc);
  
  _rate.setPidKdpThreshold(_params.pid_rate_kdp_threshold);

  _rate.setHybridDbzThreshold(_params.hybrid_dbz_threshold);
  _rate.setHybridKdpThreshold(_params.hybrid_kdp_threshold);
  _rate.setHybridZdrThreshold(_params.hybrid_zdr_threshold);

  _rate.setHidroDbzThreshold(_params.hidro_dbz_threshold);
  _rate.setHidroKdpThreshold(_params.hidro_kdp_threshold);
  _rate.setHidroZdrThreshold(_params.hidro_zdr_threshold);

  _rate.setBringiDbzThreshold(_params.bringi_dbz_threshold);
  _rate.setBringiKdpThreshold(_params.bringi_kdp_threshold);
  _rate.setBringiZdrThreshold(_params.bringi_zdr_threshold);

  _rate.setMinValidRate(_params.PRECIP_min_valid_rate);
  _rate.setMaxValidRate(_params.PRECIP_max_valid_rate);
  _rate.setMaxValidDbz(_params.PRECIP_max_valid_dbz);
  _rate.setBrightBandDbzCorrection(_params.PRECIP_brightband_dbz_correction);

  if (_params.PRECIP_apply_median_filter_to_DBZ) {
    _rate.setApplyMedianFilterToDbz(_params.PRECIP_DBZ_median_filter_len);
  }
  if (_params.PRECIP_apply_median_filter_to_ZDR) {
    _rate.setApplyMedianFilterToZdr(_params.PRECIP_ZDR_median_filter_len);
  }

}

//////////////////////////////////////
// compute PRECIP
  
void ComputeEngine::_precipCompute()
  
{
  
  // compute rates
  
  _rate.computePrecipRates(_nGates, _snrArray,
                           _dbzArray, _zdrArray, _kdpArray, 
                           missingDbl, &_pid);

  // save results

  memcpy(_rateZ, _rate.getRateZ(), _nGates * sizeof(double));
  memcpy(_rateZSnow, _rate.getRateZSnow(), _nGates * sizeof(double));
  memcpy(_rateZZdr, _rate.getRateZZdr(), _nGates * sizeof(double));
  memcpy(_rateKdp, _rate.getRateKdp(), _nGates * sizeof(double));
  memcpy(_rateKdpZdr, _rate.getRateKdpZdr(), _nGates * sizeof(double));
  memcpy(_rateHybrid, _rate.getRateHybrid(), _nGates * sizeof(double));
  memcpy(_ratePid, _rate.getRatePid(), _nGates * sizeof(double));
  memcpy(_rateHidro, _rate.getRateHidro(), _nGates * sizeof(double));
  memcpy(_rateBringi, _rate.getRateBringi(), _nGates * sizeof(double));

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

//////////////////////////////////////
// alloc arrays for PID
  
void ComputeEngine::_allocPidArrays()
  
{

  _pidArray = _pidArray_.alloc(_nGates);
  _pidArray2 = _pidArray2_.alloc(_nGates);
  _pidInterest = _pidInterest_.alloc(_nGates);
  _pidInterest2 = _pidInterest2_.alloc(_nGates);
  _tempForPid = _tempForPid_.alloc(_nGates);

}

//////////////////////////////////////
// alloc arrays for PRECIP
  
void ComputeEngine::_allocPrecipArrays()
  
{

  _rateZ = _rateZ_.alloc(_nGates);
  _rateZSnow = _rateZSnow_.alloc(_nGates);
  _rateZZdr = _rateZZdr_.alloc(_nGates);
  _rateKdp = _rateKdp_.alloc(_nGates);
  _rateKdpZdr = _rateKdpZdr_.alloc(_nGates);
  _rateHybrid = _rateHybrid_.alloc(_nGates);
  _ratePid = _ratePid_.alloc(_nGates);
  _rateHidro = _rateHidro_.alloc(_nGates);
  _rateBringi = _rateBringi_.alloc(_nGates);

}

//////////////////////////////////////
// alloc zdr bias arrays
  
void ComputeEngine::_allocZdrBiasArrays()
  
{

  _zdrmInIce = _zdrmInIce_.alloc(_nGates);
  _zdrmInBragg = _zdrmInBragg_.alloc(_nGates);

  _zdrInIce = _zdrInIce_.alloc(_nGates);
  _zdrInBragg = _zdrInBragg_.alloc(_nGates);

  _zdrFlagInIce = _zdrFlagInIce_.alloc(_nGates);
  _zdrFlagInBragg = _zdrFlagInBragg_.alloc(_nGates);

  memset(_zdrFlagInIce, 0, _nGates * sizeof(int));
  memset(_zdrFlagInBragg, 0, _nGates * sizeof(int));

}

//////////////////////////////////////
// alloc self consistency arrays
  
void ComputeEngine::_allocSelfConArrays()
  
{

  _kdpFromFilt = _kdpFromFilt_.alloc(_nGates);
  memset(_kdpFromFilt, 0, _nGates * sizeof(int));

  _kdpEst = _kdpEst_.alloc(_nGates);
  memset(_kdpEst, 0, _nGates * sizeof(int));

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

  if (_params.estimate_zdr_bias_in_ice ||
      _params.estimate_zdr_bias_in_bragg ||
      _params.estimate_z_bias_using_self_consistency) {
    if (_loadFieldArray(inputRay, _params.VEL_field_name,
                        true, _velArray)) {
      return -1;
    }
    if (_loadFieldArray(inputRay, _params.ZDRM_field_name,
                        true, _zdrmArray)) {
      return -1;
    }
    if (_loadFieldArray(inputRay, _params.RHOHV_NNC_field_name,
                        true, _rhohvNncArray)) {
      return -1;
    }
  }

  if (_loadFieldArray(inputRay, _params.PHIDP_field_name,
                      true, _phidpArray)) {
    return -1;
  }
  
  if (_loadFieldArray(inputRay, _params.RHOHV_field_name,
                      true, _rhohvArray)) {
    return -1;
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
  
  if (_params.RHO_VXHX_available) {
    if (_loadFieldArray(inputRay, _params.RHO_VXHX_field_name,
                        true, _rhoVxHxArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _rhoVxHxArray[igate] = missingDbl;
    }
  }
  
  if (_params.KDP_available) {
    if (_loadFieldArray(inputRay, _params.KDP_field_name,
                        true, _kdpArray)) {
      return -1;
    }
  } else {
    for (int igate = 0; igate < _nGates; igate++) {
      _kdpArray[igate] = missingDbl;
    }
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

  const int *pid = _pid.getPid();
  for (int ii = 0; ii < _nGates; ii++) {
    int ptype = pid[ii];
    if (ptype == NcarParticleId::FLYING_INSECTS ||
        ptype == NcarParticleId::SECOND_TRIP ||
        ptype == NcarParticleId::GROUND_CLUTTER ||
        ptype < NcarParticleId::CLOUD ||
        ptype > NcarParticleId::SATURATED_SNR) {
      field.setGateToMissing(ii);
    }
  } // ii

}

//////////////////////////////////////////////////////////
// Accumulate stats for ZDR bias from irregular ice phase
  
void ComputeEngine::_accumForZdrBiasInIce()
  
{

  // initialize

  for (int igate = 0; igate < _nGates; igate++) {
    _zdrFlagInIce[igate] = 0;
    _zdrInIce[igate] = missingDbl;
    _zdrmInIce[igate] = missingDbl;
  }

  // check elevation angle

  if (_elevation < _params.zdr_bias_ice_min_elevation_deg ||
      _elevation > _params.zdr_bias_ice_max_elevation_deg) {
    return;
  }

  // load up valid flag field along the ray

  const double *phidpAccumArray = _kdp.getPhidpAccumFilt();

  double rangeKm = _startRangeKm;
  for (int igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {

    // initially clear flag
    
    _zdrFlagInIce[igate] = 0;
    
    // check conditions

    if (rangeKm < _params.zdr_bias_ice_min_range_km ||
        rangeKm > _params.zdr_bias_ice_max_range_km) {
      continue;
    }
    
    double zdr = _zdrmArray[igate];
    if (zdr == missingDbl ||
        fabs(zdr) > _params.zdr_bias_max_abs_zdr) {
      continue;
    }

    double zdrm = _zdrmArray[igate];
    if (zdrm == missingDbl ||
        fabs(zdrm) > _params.zdr_bias_max_abs_zdrm) {
      continue;
    }

    double rhohvNnc = _rhohvNncArray[igate];
    if (rhohvNnc == missingDbl ||
        rhohvNnc < _params.zdr_bias_min_rhohv_nnc) {
      continue;
    }
    
    double vel = _velArray[igate];
    if (vel == missingDbl ||
        fabs(vel) < _params.zdr_bias_min_abs_vel) {
      continue;
    }

    double phidpAccum = phidpAccumArray[igate];
    if (phidpAccum == missingDbl ||
        phidpAccum > _params.zdr_bias_max_phidp_accum) {
      continue;
    }

    double kdp = _kdpArray[igate];
    if (kdp == missingDbl ||
        fabs(kdp) > _params.zdr_bias_max_abs_kdp) {
      continue;
    }

    double dbz = _dbzArray[igate];
    if (dbz == missingDbl ||
        dbz < _params.zdr_bias_ice_min_dbz ||
        dbz > _params.zdr_bias_ice_max_dbz) {
      continue;
    }
    
    double snr = _snrArray[igate];
    if (snr == missingDbl ||
        snr < _params.zdr_bias_ice_min_snr ||
        snr > _params.zdr_bias_ice_max_snr) {
      continue;
    }

    double rhoVxHx = _rhoVxHxArray[igate];
    if (rhoVxHx != missingDbl) {
      if (rhoVxHx < _params.zdr_bias_ice_min_rho_vxhx ||
          rhoVxHx > _params.zdr_bias_ice_max_rho_vxhx) {
        continue;
      }
    }

    double tempForPid = _tempForPid[igate];
    if (tempForPid == missingDbl ||
        tempForPid < _params.zdr_bias_ice_min_temp_c ||
        tempForPid > _params.zdr_bias_ice_max_temp_c) {
      continue;
    }

    int pid = _pidArray[igate];
    bool pidGood = false;
    for (int ii = 0; ii < _params.zdr_bias_ice_pid_types_n; ii++) {
      if (pid == _params._zdr_bias_ice_pid_types[ii]) {
        pidGood = true;
        break;
      }
    }
    if (!pidGood) {
      continue;
    }

    // passed all tests, so set flag

    _zdrFlagInIce[igate] = 1;

  } // igate
  
  // now ensure we have runs that meet or exceed the min length

  int count = 0;
  int start = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    if (_zdrFlagInIce[igate] == 1) {
      // accumulate run
      count++;
    } else {
      if (count < _params.zdr_bias_ice_min_gate_run) {
        // run too short, clear flag
        for (int jgate = start; jgate < start + count; jgate++) {
          _zdrFlagInIce[jgate] = 0;
        } // jgate
      }
      count = 0;
      start = igate + 1;
    }
  } // igate

  // sum up for ZDR bias

  for (int igate = 0; igate < _nGates; igate++) {
    if (_zdrFlagInIce[igate] == 1) {
      double zdr = _zdrArray[igate];
      double zdrm = _zdrmArray[igate];
      _zdrInIce[igate] = zdr;
      _zdrInIceResults.push_back(zdr);
      _zdrmInIce[igate] = zdrm;
      _zdrmInIceResults.push_back(zdrm);
      // cerr << "IIII ice el, az, range, zdrm: "
      //      << _elevation << ", "
      //      << _azimuth << ", "
      //      << igate * _gateSpacingKm + _startRangeKm << ", "
      //      << zdrm << endl;
    } else {
      _zdrInIce[igate] = missingDbl;
      _zdrmInIce[igate] = missingDbl;
    }
  }

}

//////////////////////////////////////////////////////////////
// Accumulate stats for ZDR bias from Bragg regions
  
void ComputeEngine::_accumForZdrBiasInBragg()
  
{

  // initialize

  for (int igate = 0; igate < _nGates; igate++) {
    _zdrFlagInBragg[igate] = 0;
    _zdrInBragg[igate] = missingDbl;
    _zdrmInBragg[igate] = missingDbl;
  }

  // check elevation angle

  if (_elevation < _params.zdr_bias_bragg_min_elevation_deg || 
      _elevation > _params.zdr_bias_bragg_max_elevation_deg) {
    return;
  }

  // load up valid flag field along the ray

  const double *phidpAccumArray = _kdp.getPhidpAccumFilt();

  double rangeKm = _startRangeKm;

  for (int igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {
    
    _zdrFlagInBragg[igate] = 0;

    if (rangeKm < _params.zdr_bias_bragg_min_range_km ||
        rangeKm > _params.zdr_bias_bragg_max_range_km) {
      continue;
    }
    
    double zdr = _zdrArray[igate];
    if (zdr == missingDbl ||
        fabs(zdr) > _params.zdr_bias_max_abs_zdr) {
      continue;
    }

    double zdrm = _zdrmArray[igate];
    if (zdrm == missingDbl ||
        fabs(zdrm) > _params.zdr_bias_max_abs_zdrm) {
      continue;
    }

    double rhohvNnc = _rhohvNncArray[igate];
    if (rhohvNnc == missingDbl ||
        rhohvNnc < _params.zdr_bias_min_rhohv_nnc) {
      continue;
    }
    
    double vel = _velArray[igate];
    if (vel == missingDbl ||
        fabs(vel) < _params.zdr_bias_min_abs_vel) {
      continue;
    }

    double kdp = _kdpArray[igate];
    if (kdp == missingDbl ||
        fabs(kdp) > _params.zdr_bias_max_abs_kdp) {
      continue;
    }

    double phidpAccum = phidpAccumArray[igate];
    if (phidpAccum == missingDbl ||
        phidpAccum > _params.zdr_bias_max_phidp_accum) {
      continue;
    }

    double dbz = _dbzArray[igate];
    if (dbz == missingDbl ||
        dbz < _params.zdr_bias_bragg_min_dbz ||
        dbz > _params.zdr_bias_bragg_max_dbz) {
      continue;
    }
    
    double snr = _snrArray[igate];
    if (snr == missingDbl ||
        snr < _params.zdr_bias_bragg_min_snr ||
        snr > _params.zdr_bias_bragg_max_snr) {
      continue;
    }

    double rhoVxHx = _rhoVxHxArray[igate];
    if (rhoVxHx != missingDbl) {
      if (rhoVxHx < _params.zdr_bias_bragg_min_rho_vxhx ||
          rhoVxHx > _params.zdr_bias_bragg_max_rho_vxhx) {
        continue;
      }
    }

    double tempForPid = _tempForPid[igate];
    if (tempForPid == missingDbl ||
        tempForPid < _params.zdr_bias_bragg_min_temp_c ||
        tempForPid > _params.zdr_bias_bragg_max_temp_c) {
      continue;
    }

    if (_params.zdr_bias_bragg_check_pid) {
      int pid = _pidArray[igate];
      bool pidGood = false;
      for (int ii = 0; ii < _params.zdr_bias_bragg_pid_types_n; ii++) {
        if (pid == _params._zdr_bias_bragg_pid_types[ii]) {
          pidGood = true;
          break;
        }
      }
      if (!pidGood) {
        continue;
      }
    }

    // passed all tests, set flag

    _zdrFlagInBragg[igate] = 1;

  } // igate
  
  // now ensure we have runs that meet or exceed the min length
  
  int count = 0;
  int start = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    if (_zdrFlagInBragg[igate] == 1) {
      // accumulate run
      count++;
    } else {
      if (count < _params.zdr_bias_bragg_min_gate_run) {
        // run too short, clear flag
        for (int jgate = start; jgate < start + count; jgate++) {
          _zdrFlagInBragg[jgate] = 0;
        } // jgate
      }
      count = 0;
      start = igate + 1;
    }
  } // igate

  // sum up for ZDR bias

  for (int igate = 0; igate < _nGates; igate++) {
    if (_zdrFlagInBragg[igate] == 1) {
      double zdr = _zdrArray[igate];
      double zdrm = _zdrmArray[igate];
      _zdrInBragg[igate] = zdr;
      _zdrmInBragg[igate] = zdrm;
      _zdrInBraggResults.push_back(zdr);
      _zdrmInBraggResults.push_back(zdrm);
    } else {
      _zdrInBragg[igate] = missingDbl;
      _zdrmInBragg[igate] = missingDbl;
    }
  }

}

//////////////////////////////////////////////////////////////////////
// Run the self-consistency analysis

void ComputeEngine::_runSelfConsistencyCheck()
  
{

  // check elevation angle

  if (_elevation < _params.self_consistency_min_elevation_deg) {
    return;
  }

  _selfConMinNoGapNGates =
    (int) (_params.self_consistency_min_no_gap_distance_km / _gateSpacingKm + 0.5);
  _selfConMinCombinedNGates =
    (int) (_params.self_consistency_min_combined_distance_km / _gateSpacingKm + 0.5);

  // load up flag array to indicate valid gates
  // based on required criteria
  
  RadxArray<bool> gateIsValid_;
  bool *gateIsValid = gateIsValid_.alloc(_nGates);
  int firstValid = -1;
  int lastValid = -1;

  const double *phidpFilt = _kdp.getPhidpFilt();
  const double *phidpCondFilt = _kdp.getPhidpCondFilt();
  const double *kdp = _kdp.getKdp();

  // compute slope of filtered phidp

  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (phidpFilt[igate-1] == missingDbl || phidpFilt[igate+1] == missingDbl) {
      _kdpFromFilt[igate] = missingDbl;
    } else {
      _kdpFromFilt[igate] = 
        (phidpFilt[igate+1] - phidpFilt[igate-1]) / (4.0 * _gateSpacingKm);
    }
  }
  _kdpFromFilt[0] = _kdpFromFilt[1];
  _kdpFromFilt[_nGates-1] = _kdpFromFilt[_nGates-2];
  
  // loop through gates, checking for valid runs

  double rangeKm = _startRangeKm;
  for (int igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {
    
    gateIsValid[igate] = true;

    // check range

    if (rangeKm > _params.self_consistency_max_range_km) {
      gateIsValid[igate] = false;
      continue;
    }

    // check dbz

    double dbzm = _dbzArray[igate];
    double dbz = dbzm + _params.self_consistency_dbz_correction;
    if (dbzm == missingDbl ||
        dbz < _params.self_consistency_min_dbz ||
        dbz > _params.self_consistency_max_dbz) {
      gateIsValid[igate] = false;
      continue;
    }

    // check zdr

    double zdrm = _zdrmArray[igate];
    double zdr = zdrm + _params.self_consistency_zdrm_correction;
    if (zdrm == missingDbl ||
        zdr < _params.self_consistency_min_zdr ||
        zdr > _params.self_consistency_max_zdr) {
      gateIsValid[igate] = false;
      continue;
    }

    // check SNR
    
    double snr = _snrArray[igate];
    if (snr == missingDbl ||
        snr < _params.self_consistency_min_snr ||
        snr > _params.self_consistency_max_snr) {
      gateIsValid[igate] = false;
      continue;
    }
    
    // check rhohv

    double rhohv = _rhohvArray[igate];
    if (rhohv == missingDbl ||
        rhohv < _params.self_consistency_min_rhohv) {
      gateIsValid[igate] = false;
      continue;
    }

    // check KDP is not missing
    
    if (kdp[igate] == missingDbl) {
      gateIsValid[igate] = false;
      continue;
    }
    if (_kdpFromFilt[igate] == missingDbl) {
      gateIsValid[igate] = false;
      continue;
    }
    
    // check temperature

    double tempForPid = _tempForPid[igate];
    if (tempForPid != missingDbl &&
        tempForPid < _params.self_consistency_min_temp_c) {
      gateIsValid[igate] = false;
      continue;
    }
    
    // check we are in rain
    
    int pid = _pidArray[igate];
    bool isRain = false;
    for (int ii = 0; ii < _params.self_consistency_pid_types_n; ii++) {
      if (pid == _params._self_consistency_pid_types[ii]) {
        isRain = true;
        break;
      }
    }
    if (!isRain) {
      gateIsValid[igate] = false;
      continue;
    }

    // check for backscatter phase shift
    
    double backScatterShift = fabs(phidpFilt[igate] - phidpCondFilt[igate]);
    if (backScatterShift > _params.self_consistency_max_phase_shift_on_backscatter) {
      gateIsValid[igate] = false;
      continue;
    }

    // gate is valid

    if (firstValid < 0) {
      firstValid = igate;
    }
    lastValid = igate;

  } // igate

  if (firstValid < 0 || lastValid < 0) {
    return;
  }
  
  // count the length of valid runs, removing those that are too short
  
  int start = -1;
  int end = -1;
  int len = 0;
  for (int igate = firstValid; igate <= lastValid; igate++) {
    if (gateIsValid[igate]) {
      len++;
      if (start < 0) {
        start = igate;
      }
      end = igate;
    } else {
      if (len < _selfConMinNoGapNGates && start >= 0 && end >= 0) {
        for (int jj = start; jj <= end; jj++) {
          gateIsValid[jj] = false;
        }
      }
      len = 0;
      start = -1;
      end = -1;
    }
  } // igate

  if (len < _selfConMinNoGapNGates && start >= 0 && end >= 0) {
    for (int jj = start; jj <= end; jj++) {
      gateIsValid[jj] = false;
    }
  }

  // fill in acceptably small gaps

  start = -1;
  end = -1;
  len = 0;
  for (int igate = firstValid; igate <= lastValid; igate++) {
    if (!gateIsValid[igate]) {
      len++;
      if (start < 0) {
        start = igate;
      }
      end = igate;
    } else {
      if (len <= _params.self_consistency_max_gate_gap &&
          start >= 0 && end >= 0) {
        for (int jj = start; jj <= end; jj++) {
          gateIsValid[jj] = true;
        }
      }
      len = 0;
      start = -1;
      end = -1;
    }
  } // igate

  if (len <= _params.self_consistency_max_gate_gap &&
      start >= 0 && end >= 0) {
    for (int jj = start; jj <= end; jj++) {
      gateIsValid[jj] = true;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    pthread_mutex_lock(&_debugPrintMutex);
    cerr << "SELF CONSISTENCY GATE RANGE:" << endl;
    cerr << "  elev: " << _elevation << endl;
    cerr << "  az: " << _azimuth << endl;
    cerr << "  firstValid: " << firstValid << endl;
    cerr << "  lastValid: " << lastValid << endl;
    pthread_mutex_unlock(&_debugPrintMutex);
  }

  // find the resulting valid runs

  start = -1;
  end = -1;
  len = 0;
  vector<int> runStart, runEnd;
  for (int igate = firstValid; igate <= lastValid; igate++) {
    if (gateIsValid[igate]) {
      len++;
      if (start < 0) {
        start = igate;
      }
      end = igate;
    } else {
      if (start >= 0 && end >= 0) {
        runStart.push_back(start);
        runEnd.push_back(end);
      }
      len = 0;
      start = -1;
      end = -1;
    }
  } // igate

  if (start >= 0 && end >= 0) {
    // move in one gate at each end to be on the safe side
    start++;
    end--;
    int runLen = end - start + 1;
    if (runLen >= _selfConMinNoGapNGates) {
      runStart.push_back(start);
      runEnd.push_back(end);
    }
  }

  // trim the runs to limit dbz and phidp slope at the ends
  // we use the slope of the filtered phidp instead of kdp, since kdp
  // is based on the conditional phidp, which artifically cuts off the
  // peaks of phase shift on backscatter

  for (size_t irun = 0; irun < runStart.size(); irun++) {

    int rstart = runStart[irun];
    int rend = runEnd[irun];

    for (int ii = runStart[irun]; ii < runEnd[irun]; ii++) {
      bool trimRun = false;
      if (_kdpFromFilt[ii] < _params.self_consistency_min_kdp ||
          _kdpFromFilt[ii] > _params.self_consistency_max_kdp_at_run_limits) {
        trimRun = true;
      }
      double dbz = _dbzArray[ii] + _params.self_consistency_dbz_correction;
      if (dbz > _params.self_consistency_max_dbz_at_run_limits) {
        trimRun = true;
      }
      if (trimRun) {
        if (_params.self_consistency_debug >= Params::DEBUG_EXTRA) {
          pthread_mutex_lock(&_debugPrintMutex);
          if (runStart[irun] != rstart) {
            cerr << "==>> trimming run start from " 
                 << runStart[irun] << " to " << rstart << endl;
          }
          pthread_mutex_unlock(&_debugPrintMutex);
        }
        rstart = ii + 1;
      } else {
        break;
      }
    }

    for (int ii = runEnd[irun]; ii > runStart[irun]; ii--) {
      bool trimRun = false;
      if (_kdpFromFilt[ii] < _params.self_consistency_min_kdp ||
          _kdpFromFilt[ii] > _params.self_consistency_max_kdp_at_run_limits) {
        trimRun = true;
      }
      double dbz = _dbzArray[ii] + _params.self_consistency_dbz_correction;
      if (dbz > _params.self_consistency_max_dbz_at_run_limits) {
        trimRun = true;
      }
      if (trimRun) {
        if (_params.self_consistency_debug >= Params::DEBUG_EXTRA) {
          pthread_mutex_lock(&_debugPrintMutex);
          if (runEnd[irun] != rend) {
            cerr << "==>> trimming run end from "
                 << runEnd[irun] << " to " << rend << endl;
          }
          pthread_mutex_unlock(&_debugPrintMutex);
        }
        rend = ii - 1;
      } else {
        break;
      }
    }

    if (rstart > rend) {
      rstart = rend;
    }

    runStart[irun] = rstart;
    runEnd[irun] = rend;
    
  } // irun - trimming run ends for dbz and kdp

  // at start, trim off any gates over which there is no significant
  // phidp accum

  for (size_t irun = 0; irun < runStart.size(); irun++) {

    int rstart = runStart[irun];
    int rend = runEnd[irun];
    
    const double *phidpAccumArray = _kdp.getPhidpAccumFilt();
    double phidpStart = phidpAccumArray[rstart];
    double phidpEnd = phidpAccumArray[rend];
    double phidpAccumRun = phidpEnd - phidpStart;
    
    int istart = rstart;
    for (int ii = rstart + 1; ii < rend; ii++) {
      double phidpAccum = phidpAccumArray[ii];
      double fraction = phidpAccum / phidpAccumRun;
      if (fraction >= 0.01) {
        istart = ii;
        break;
      }
    } // ii

    if (istart - rstart > 3) {
      // trim run if significant
      runStart[irun] = istart;
    }

  } // irun - trimming for flat phidp at start

  // analyze the runs

  for (size_t irun = 0; irun < runStart.size(); irun++) {
    _doCheckSelfConsistency(runStart[irun], runEnd[irun]);
  }

}

////////////////////////////////////////////////////////
// Run the self-consistency analysis for a specified run
  
void ComputeEngine::_doCheckSelfConsistency(int runStart, int runEnd)
  
{
  
  // See Vivekanandan, J., G. Zhang, S. M. Ellis, D. Rajopadhyaya, and
  // S. K. Avery, Radar reflectivity calibration using differential
  // propagation phase measurement, Radio Sci., 38(3), 8049,
  // doi:10.1029/2002RS002676, 2003.

  double rangeStart = _startRangeKm + runStart * _gateSpacingKm;
  double rangeEnd = _startRangeKm + runEnd * _gateSpacingKm;
  
  // compute measured phidp accum
  
  const double *phidpAccumArray = _kdp.getPhidpAccumFilt();
  double phidpStart = phidpAccumArray[runStart];
  double phidpEnd = phidpAccumArray[runEnd];
  double phidpAccumObs = phidpEnd - phidpStart;
  if (phidpAccumObs < _params.self_consistency_min_phidp_accum) {
    return;
  }

  // set parameters

  int runLen = runEnd - runStart + 1;
  if (runLen < _selfConMinCombinedNGates) {
    return;
  }

  double aa = _params.self_consistency_kdp_z_expon;
  double bb = _params.self_consistency_kdp_zdr_expon;
  double cc = _params.self_consistency_kdp_coefficient;
  double c1 = _params.self_consistency_z_atten_coefficient;
  double c2 = _params.self_consistency_zdr_atten_coefficient;

  double a0 = _params.self_consistency_polynomial_a0;
  double a1 = _params.self_consistency_polynomial_a1;
  double a2 = _params.self_consistency_polynomial_a2;
  double a3 = _params.self_consistency_polynomial_a3;

  // correct DBZ and ZDR to get it close to reality

  double dbzCorr = _params.self_consistency_dbz_correction;
  double zdrmCorr = _params.self_consistency_zdrm_correction;
  
  // compute atmospheric attenuation
 
  RadxArray<double> atmosAtten_;
  double *atmosAtten2Way = atmosAtten_.alloc(runLen);
  for (int ii = 0, jj = runStart; jj <= runEnd; ii++, jj++) {
    double rangeKm = _startRangeKm + jj * _gateSpacingKm;
    atmosAtten2Way[ii] = _atmos.getAtten(_elevation, rangeKm);
  }

  // adjust for attenuation
  
  RadxArray<double> zAtten_, zdrAtten_;
  double *zAtten = zAtten_.alloc(runLen);
  double *zdrAtten = zdrAtten_.alloc(runLen);
  
  RadxArray<double> dbz_, zdr_, kdp_, kdpFromFilt_, phidpFilt_, rhohv_;
  double *dbz = dbz_.alloc(runLen);
  double *zdr = zdr_.alloc(runLen);
  double *kdp = kdp_.alloc(runLen);
  double *kdpFromFilt = kdpFromFilt_.alloc(runLen);
  double *phidpFilt = phidpFilt_.alloc(runLen);
  double *rhohv = rhohv_.alloc(runLen);

  RadxArray<int> pid_;
  int *pid = pid_.alloc(runLen);

  memcpy(dbz, _dbzArray + runStart, runLen * sizeof(double));
  memcpy(zdr, _zdrmArray + runStart, runLen * sizeof(double));
  memcpy(kdp, _kdpArray + runStart, runLen * sizeof(double));
  memcpy(kdpFromFilt, _kdpFromFilt + runStart, runLen * sizeof(double));
  memcpy(phidpFilt, _kdp.getPhidpFilt() + runStart, runLen * sizeof(double));
  memcpy(rhohv, _rhohvArray + runStart, runLen * sizeof(double));
  memcpy(pid, _pidArray + runStart, runLen * sizeof(int));

  for (int ii = 0; ii < runLen; ii++) {
    if (kdp[ii] < _params.self_consistency_min_kdp) {
      kdp[ii] = 0.0;
    }
  }

  zAtten[0] = kdp[0] * c1;
  zdrAtten[0] = kdp[0] * c2;

  for (int ii = 1; ii < runLen; ii++) {
    zAtten[ii] = zAtten[ii-1] + kdp[ii] * c1 * _gateSpacingKm;
    zdrAtten[ii] = zdrAtten[ii-1] + kdp[ii] * c2 * _gateSpacingKm;
    double dbzAttenCorr = 2 * zAtten[ii] + atmosAtten2Way[ii];
    double zdrAttenCorr = 2 * zdrAtten[ii];
    dbz[ii] += (dbzAttenCorr + dbzCorr);
    zdr[ii] += (zdrmCorr + zdrAttenCorr);
  }

  // linearize dbz and zdr

  RadxArray<double> zdrLin_, zzLin_;
  double *zdrLin = zdrLin_.alloc(runLen);
  double *zzLin = zzLin_.alloc(runLen);

  for (int ii = 0; ii < runLen; ii++) {
    zdrLin[ii] = pow(10.0, zdr[ii] / 10.0);
    zzLin[ii] = pow(10.0, dbz[ii] / 10.0);
  }

  // calculate theoretical phi accum from Z and ZDR
  // plus compute for 1 dBZ above and 1 dBZ below

  double phidpObsStart = phidpFilt[0];
  RadxArray<double> zdrTerm_, phidpEst_;
  double *zdrTerm = zdrTerm_.alloc(runLen);
  double *phidpEst = phidpEst_.alloc(runLen);

  RadxArray<double> obsAccum_, estAccum_;
  double *obsAccum = obsAccum_.alloc(runLen);
  double *estAccum = estAccum_.alloc(runLen);
  
  double phidpAccumEst = 0.0;
  double kdpEst = 0.0;
  for (int ii = 0; ii < runLen; ii++) {
    if (_params.self_consisteny_method == Params::SELF_CON_ZDR_POWER_LAW_METHOD) {
      kdpEst = _computeKdpPowerLaw(zzLin[ii], zdrLin[ii], aa, bb, cc);
    } else {
      kdpEst = _computeKdpPolynomial(zzLin[ii], zdr[ii], a0, a1, a2, a3);
    }
    phidpAccumEst += kdpEst * _gateSpacingKm * 2.0;
    _kdpEst[ii] = kdpEst;
    zdrTerm[ii] = pow(zdrLin[ii], bb);
    obsAccum[ii] = phidpFilt[ii] - phidpObsStart;
    estAccum[ii] = phidpAccumEst;
    phidpEst[ii] = phidpObsStart + phidpAccumEst;
  }
  double zBias = pow((phidpAccumEst / phidpAccumObs), 1.0 / aa);
  double dbzBias = 10.0 * log10(zBias);

  // get correlation coefficient between observed and estimated phidp change

  double aaa[2];
  double meanObsAccum, meanEstAccum;
  double sdevObsAccum, sdevEstAccum;
  double accumCorrelation;
  double stdErrEst, rSquared;
  uLinearFit(runLen, obsAccum, estAccum, aaa,
             &meanObsAccum, &meanEstAccum,
             &sdevObsAccum, &sdevEstAccum,
             &accumCorrelation,
             &stdErrEst, &rSquared);

  // debug prints

  {

    pthread_mutex_lock(&_debugPrintMutex);

    if (_params.self_consistency_debug) {
      cerr << "======= self consistency analysis ===========" << endl;
      RadxTime rtime(_timeSecs, _nanoSecs / 1.0e9);
      cerr << "  Time: " << rtime.asString(6) << endl;
      cerr << "  elev: " << _elevation << endl;
      cerr << "  az: " << _azimuth << endl;
      cerr << "  run start: " << runStart << endl;
      cerr << "  run end: " << runEnd << endl;
      cerr << "  range start: " << rangeStart << endl;
      cerr << "  range end: " << rangeEnd << endl;
      cerr << "  dbzCorr: " << dbzCorr << endl;
      cerr << "  zdrmCorr: " << zdrmCorr << endl;
      cerr << "  phidpAccumObs: " << phidpAccumObs << endl;
      cerr << "  phidpAccumEst: " << phidpAccumEst << endl;
      cerr << "  dbzBias: " << dbzBias << endl;
      cerr << "  accumCorrelation: " << accumCorrelation << endl;
      cerr << "=============================================" << endl;
    }
    
    if (_params.self_consistency_write_run_files) {
      _writeSelfConRunDataToFile(runStart, runEnd,
                                 rangeStart, rangeEnd,
                                 phidpAccumObs,
                                 phidpAccumEst, 
                                 dbzBias,
                                 accumCorrelation,
                                 dbz, zdr, zdrTerm, phidpEst);
    }

    pthread_mutex_unlock(&_debugPrintMutex);

  } // debug prints

  self_con_t selfCon;
  RadxTime rtime(_timeSecs, _nanoSecs / 1.0e9);
  selfCon.rtime = rtime;
  selfCon.elevation = _elevation;
  selfCon.azimuth = _azimuth;
  selfCon.runStart = runStart;
  selfCon.runEnd = runEnd;
  selfCon.rangeStart = rangeStart;
  selfCon.rangeEnd = rangeEnd;
  selfCon.dbzCorrection = dbzCorr;
  selfCon.zdrCorrection = zdrmCorr;
  selfCon.accumObs = phidpAccumObs;
  selfCon.accumEst = phidpAccumEst;
  selfCon.dbzBias = dbzBias;
  selfCon.accumCorrelation = accumCorrelation;

  _selfConResults.push_back(selfCon);

}

////////////////////////////////////////////////////////////
// Compute estimated kdp using power law

double ComputeEngine::_computeKdpPowerLaw(double zz,
                                          double zdrLin,
                                          double aa, 
                                          double bb,
                                          double cc)
  
{

  double z_term = pow(zz, aa);
  double zdr_term = pow(zdrLin, bb);
  double kdp_est = z_term * zdr_term * cc;

  return kdp_est;
  
}

////////////////////////////////////////////////////////////
// Compute estimated kdp using polynomial formulation

double ComputeEngine::_computeKdpPolynomial(double zz,
                                            double zdrDb,
                                            double a0, double a1, double a2, double a3)
  
{
  
  double zdr_term = a0 + a1 * zdrDb + a2 * zdrDb * zdrDb + a3 * zdrDb * zdrDb * zdrDb;
  double kdp_est = zdr_term * zz * 1.0e-5;
  if (kdp_est < 0) {
    kdp_est = 0.0;
  }
  return kdp_est;

}
  

////////////////////////////////////////////////////////////
// write self-consistency run to a file

void ComputeEngine::_writeSelfConRunDataToFile(int runStart,
                                               int runEnd,
                                               double rangeStart,
                                               double rangeEnd,
                                               double phidpAccumObs,
                                               double phidpAccumEst,
                                               double dbzBias,
                                               double accumCorrelation,
                                               const double *dbzCorr,
                                               const double *zdrCorr,
                                               const double *zdrTerm,
                                               const double *phidpEst)
  
{

  // make sure output dir exists
  
  if (ta_makedir_recurse(_params.self_consistency_run_files_dir)) {
    int errNum = errno;
    cerr << "ERROR - ComputeEngine::_writeSelfConRunDataToFile()" << endl;
    cerr << "  Cannot create dir: " << _params.self_consistency_run_files_dir << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  // create file name
  

  char filePath[MAX_PATH_LEN];
  RadxTime rtime(_timeSecs, _nanoSecs / 1.0e9);
  int msecs = (int) (_nanoSecs / 1000000.0 + 0.5);
  sprintf(filePath, 
          "%s%sselfcon_run_%.4d%.2d%.2d-%.2d%.2d%.2d.%.3d"
          "_srun-%d_erun-%d_el-%05.1f_az-%05.1f_.txt",
          _params.self_consistency_run_files_dir, PATH_DELIM,
          rtime.getYear(), rtime.getMonth(), rtime.getDay(),
          rtime.getHour(), rtime.getMin(), rtime.getSec(), msecs,
          runStart, runEnd, _elevation, _azimuth);
  
  // open file

  FILE *out = fopen(filePath, "w");
  if (out == NULL) {
    int errNum = errno;
    cerr << "ERROR - ComputeEngine::_writeSelfConRunDataToFile()" << endl;
    cerr << "  Cannot open file: " << filePath << endl;
    cerr << "  " << strerror(errNum) << endl;
    return;
  }

  // write header line with column labels

  fprintf(out,
          "# gateNum "
          "snr dbzObs dbzCorr zdrObs zdrCorr zdrTerm rhohv "
          "phidpObs phidpEst phidpUnfold phidpFilt phidpCondFilt "
          "psob kdp temp pid\n");

  // write meta data

  fprintf(out, "# time: %s\n", rtime.asString(6).c_str());
  fprintf(out, "# elev: %g\n", _elevation);
  fprintf(out, "# az: %g\n", _azimuth);
  fprintf(out, "# run start: %d\n", runStart);
  fprintf(out, "# run end: %d\n", runEnd);
  fprintf(out, "# range start: %g\n", rangeStart);
  fprintf(out, "# range end: %g\n", rangeEnd);
  fprintf(out, "# dbzCorr: %g\n", _params.self_consistency_dbz_correction);
  fprintf(out, "# zdrmCorr: %g\n", _params.self_consistency_zdrm_correction);
  fprintf(out, "# phidpAccumObs: %g\n", phidpAccumObs);
  fprintf(out, "# phidpAccumEst: %g\n", phidpAccumEst);
  fprintf(out, "# dbzBias: %g\n", dbzBias);
  fprintf(out, "# accumCorrelation: %g\n", accumCorrelation);

  // write data

  for (int igate = runStart; igate <= runEnd; igate++) {
    int ii = igate - runStart;
    fprintf(out,
            "%3d "
            "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10.3f %10.3f "
            "%10.3f %10.3f %10.3f %10d\n",
            igate,
            _getPlotVal(_snrArray[igate], -10),
            _getPlotVal(_dbzArray[igate], -20),
            _getPlotVal(dbzCorr[ii], -20),
            _getPlotVal(_zdrArray[igate], 0),
            _getPlotVal(zdrCorr[ii], 0),
            _getPlotVal(zdrTerm[ii], 0),
            _getPlotVal(_rhohvArray[igate], 0),
            _getPlotVal(_phidpArray[igate], 0),
            _getPlotVal(phidpEst[ii], 0),
            _getPlotVal(_kdp.getPhidpUnfold()[igate], 0),
            _getPlotVal(_kdp.getPhidpFilt()[igate], 0),
            _getPlotVal(_kdp.getPhidpCondFilt()[igate], 0),
            _getPlotVal(_kdp.getPsob()[igate], 0),
            _getPlotVal(_kdp.getKdp()[igate], 0),
            _getPlotVal(_tempForPid[igate], 0),
            _pidArray[igate]
            );
  }

  // close file

  fclose(out);

}

//////////////////////////////////////////////////////////////////////////
// Get a value suitable for plotting
// i.e. interpret missing data reasonably

double ComputeEngine::_getPlotVal(double val, double valIfMissing)
  
{
  if (val < -9990) {
    return valIfMissing;
  } else {
    return val;
  }
}
