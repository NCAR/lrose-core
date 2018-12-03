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
// Dec 2018
//
///////////////////////////////////////////////////////////////
//
// ComputeEngine computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#include "ComputeEngine.hh"
#include "RadxKdp.hh"
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
                             const KdpFiltParams &kdpFiltParams,
                             int id)  :
        _params(params),
        _kdpFiltParams(kdpFiltParams),
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
// compute the derived fields for given input ray
// storing results in derived ray
//
// Creates derived ray and returns it.
// It must be freed by caller.
//
// Returns NULL on error.

RadxRay *ComputeEngine::compute(RadxRay *inputRay,
                                double wavelengthM)
{

  // set ray-specific metadata
  
  _nGates = inputRay->getNGates();
  _startRangeKm = inputRay->getStartRangeKm();
  _gateSpacingKm = inputRay->getGateSpacingKm();
  _azimuth = inputRay->getAzimuthDeg();
  _elevation = inputRay->getElevationDeg();
  _timeSecs = inputRay->getTimeSecs();
  _nanoSecs = inputRay->getNanoSecs();

  // initialize

  _wavelengthM = wavelengthM;

  // create moments ray
  
  RadxRay *derivedRay = new RadxRay;
  derivedRay->copyMetaData(*inputRay);

  // allocate moments arrays for computing derived fields,
  // and load them up
  
  _allocMomentsArrays();
  _loadMomentsArrays(inputRay);
  
  // compute kdp
  
  _kdpCompute();

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
  
  // load up output data
  
  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {
    
    const Params::output_field_t &ofld = _params._output_fields[ifield];
    
    // fill data array
    
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(_nGates);
    Radx::fl32 *datp = data;
    
    for (size_t igate = 0; igate < _nGates; igate++, datp++) {
    
      switch (ofld.id) {
        
        // computed KDP
        
        case Params::KDP:
        default:
          *datp = _kdpArray[igate];
          break;
        case Params::KDP_ZZDR:
          *datp = _kdpZZdrArray[igate];
          break;
        case Params::KDP_COND:
          *datp = _kdpCondArray[igate];
          break;
        case Params::PSOB:
          *datp = psob[igate];
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

#ifdef JUNK

        case Params::SNR:
          *datp = _snrArray[igate];
          break;
        case Params::DBZ:
          *datp = _dbzArray[igate];
          break;
        case Params::ZDR:
          *datp = _zdrArray[igate];
          break;
        case Params::RHOHV:
          *datp = _rhohvArray[igate];
          break;
        case Params::PHIDP:
          *datp = _phidpArray[igate];
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

        case Params::ZDP:
          *datp = _zdpArray[igate];
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
#endif

      } // switch

    } // igate

    // create field
    
    RadxField *field = new RadxField(ofld.name, ofld.units);
    field->setLongName(ofld.long_name);
    field->setStandardName(ofld.standard_name);
    field->setTypeFl32(missingDbl);
    field->addDataFl32(_nGates, data);
    field->copyRangeGeom(*inputRay);

    // add to ray

    derivedRay->addField(field);

  } // ifield
  
  // copy fields through as required

  if (_params.copy_selected_input_fields_to_output) {

    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      const Params::copy_field_t &cfield = _params._copy_fields[ii];
      string inName = cfield.input_name;
      RadxField *inField = inputRay->getField(inName);
      if (inField != NULL) {
        RadxField *outField = new RadxField(*inField);
        outField->setName(cfield.output_name);
        derivedRay->addField(outField);
      }
    } // ii

  } // if (_params.copy_input_fields_to_output)

  // add debug fields if required

  if (_params.write_debug_fields) {
    _addDebugFields(derivedRay);
  }

}

//////////////////////////////////////
// add the debug fields
  
void ComputeEngine::_addDebugFields(RadxRay *derivedRay)

{


}
  
//////////////////////////////////////
// initialize KDP
  
void ComputeEngine::_kdpInit()
  
{

  // initialize KDP object

  _kdp.setFromParams(_kdpFiltParams);
  if (_params.set_max_range) {
    _kdp.setMaxRangeKm(true, _params.max_range_km);
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
  for (size_t ii = 0; ii < _nGates; ii++, range += _gateSpacingKm) {
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
  const double *kdpZZdr = _kdp.getKdpZZdr();
  const double *kdpCond = _kdp.getKdpCond();
  
  // put KDP into fields objects
  
  for (size_t ii = 0; ii < _nGates; ii++) {
    if (kdp[ii] == NAN) {
      _kdpArray[ii] = missingDbl;
    } else {
      _kdpArray[ii] = kdp[ii];
    }
    _kdpZZdrArray[ii] = kdpZZdr[ii];
    _kdpCondArray[ii] = kdpCond[ii];
  }

}

//////////////////////////////////////
// alloc moments arrays
  
void ComputeEngine::_allocMomentsArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _velArray = _velArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _zdrmArray = _zdrmArray_.alloc(_nGates);
  _zdpArray = _zdpArray_.alloc(_nGates);
  _kdpArray = _kdpArray_.alloc(_nGates);
  _kdpBringiArray = _kdpBringiArray_.alloc(_nGates);
  _kdpZZdrArray = _kdpZZdrArray_.alloc(_nGates);
  _kdpCondArray = _kdpCondArray_.alloc(_nGates);
  _ldrArray = _ldrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _rhohvNncArray = _rhohvNncArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);
  _rhoVxHxArray = _rhoVxHxArray_.alloc(_nGates);
  _tempForPid = _tempForPid_.alloc(_nGates);
}

//////////////////////////////////////
// alloc arrays for PID
  
void ComputeEngine::_allocPidArrays()
  
{

  _pidArray = _pidArray_.alloc(_nGates);
  _pidArray2 = _pidArray2_.alloc(_nGates);
  _pidInterest = _pidInterest_.alloc(_nGates);
  _pidInterest2 = _pidInterest2_.alloc(_nGates);

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
    for (size_t igate = 0; igate < _nGates; igate++) {
      _ldrArray[igate] = missingDbl;
    }
  }
  
  if (_params.RHO_VXHX_available) {
    if (_loadFieldArray(inputRay, _params.RHO_VXHX_field_name,
                        true, _rhoVxHxArray)) {
      return -1;
    }
  } else {
    for (size_t igate = 0; igate < _nGates; igate++) {
      _rhoVxHxArray[igate] = missingDbl;
    }
  }
  
  if (_params.KDP_available) {
    if (_loadFieldArray(inputRay, _params.KDP_field_name,
                        true, _kdpArray)) {
      return -1;
    }
  } else {
    for (size_t igate = 0; igate < _nGates; igate++) {
      _kdpArray[igate] = missingDbl;
    }
  }

  _loadFieldArray(inputRay, "temperature", true, _tempForPid);

  return 0;
  
}

/////////////////////////////////////////////////////
// compute zdp from dbz and zdr
//
// See Bringi and Chandrasekar, p438.
  
void ComputeEngine::_computeZdpArray()
  
{
  
  for (size_t igate = 0; igate < _nGates; igate++) {

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
      for (size_t igate = 0; igate < _nGates; igate++) {
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

  // Set array values
  // fields are already fl32

  const Radx::fl32 *vals = field->getDataFl32();
  double missingFl32 = field->getMissingFl32();
  for (size_t igate = 0; igate < _nGates; igate++, vals++) {
    Radx::fl32 val = *vals;
    if (val == missingFl32) {
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
  for (size_t igate = 0; igate < _nGates; igate++, range += _gateSpacingKm) {
    noiseDbz[igate] = _params.noise_dbz_at_100km +
      20.0 * (log10(range) - log10(100.0));
  }

  // compute snr from dbz
  
  double *snr = _snrArray;
  const double *dbz = _dbzArray;
  for (size_t igate = 0; igate < _nGates; igate++, snr++, dbz++) {
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
  for (size_t ii = 0; ii < _nGates; ii++) {
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

  for (size_t igate = 0; igate < _nGates; igate++) {
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
  for (size_t igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {

    // initially clear flag
    
    _zdrFlagInIce[igate] = 0;
    
    // check conditions

    if (rangeKm < _params.zdr_bias_ice_min_range_km ||
        rangeKm > _params.zdr_bias_ice_max_range_km) {
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
  for (int igate = 0; igate < (int) _nGates; igate++) {
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

  for (size_t igate = 0; igate < _nGates; igate++) {
    if (_zdrFlagInIce[igate] == 1) {
      double zdr = _zdrArray[igate];
      double zdrm = _zdrmArray[igate];
      _zdrInIce[igate] = zdr;
      _zdrInIceResults.push_back(zdr);
      _zdrmInIce[igate] = zdrm;
      _zdrmInIceResults.push_back(zdrm);
      _zdrInIceElev.push_back(_elevation);
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

  for (size_t igate = 0; igate < _nGates; igate++) {
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

  for (size_t igate = 0; igate < _nGates; igate++, rangeKm += _gateSpacingKm) {
    
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
  for (size_t igate = 0; igate < _nGates; igate++) {
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

  for (size_t igate = 0; igate < _nGates; igate++) {
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

