// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2018                                         
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
// ScalarsCompute.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Nov 2025
//
///////////////////////////////////////////////////////////////
//
// ScalarsCompute computation engine
// Computes dual pol scalars in polar coords
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#include "ScalarsCompute.hh"
#include "RadxCartDP.hh"
#include <toolsa/os_config.h>
#include <toolsa/file_io.h>
#include <rapmath/trig.h>
#include <rapmath/umath.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <cerrno>
using namespace std;
pthread_mutex_t ScalarsCompute::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;
const double ScalarsCompute::missingDbl = -9999.0;

// Constructor

ScalarsCompute::ScalarsCompute(const Params &params,
                               const KdpFiltParams &kdpFiltParams,
                               const NcarPidParams &ncarPidParams,
                               const PrecipRateParams &precipRateParams,
                               int id)  :
        _params(params),
        _kdpFiltParams(kdpFiltParams),
        _ncarPidParams(ncarPidParams),
        _precipRateParams(precipRateParams),
        _id(id)
  
{

  OK = true;
  
  // initialize kdp and pid computations

  _kdpInit();

  if (_pidInit()) {
    OK = false;
  }

  _precipInit();

}

// destructor

ScalarsCompute::~ScalarsCompute()

{

}

///////////////////////////////////////////////////////////
// Load the temperature profile for PID computations,
// for the specified time.
// This reads in a new sounding if needed.
// If no sounding is available, the static profile is used

void ScalarsCompute::loadTempProfile(time_t dataTime)
{
  _pid.loadTempProfile(dataTime);
}

///////////////////////////////////////////////////////////
// Set the temperature profile

void ScalarsCompute::setTempProfile(const TempProfile &profile)
{
  _pid.setTempProfile(profile);
}

///////////////////////////////////////////////////////////
// Get the temperature profile

const TempProfile &ScalarsCompute::getTempProfile() const
{
  return _pid.getTempProfile();
}

//////////////////////////////////////////////////
// compute the derived fields for given input ray
// storing results in output ray
//
// Creates output ray and returns it.
// It must be freed by caller.
//
// Returns NULL on error.

RadxRay *ScalarsCompute::doCompute(RadxRay *inputRay,
                                   double radarHtKm,
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
  _nyquist = inputRay->getNyquistMps();

  // initialize

  _radarHtKm = radarHtKm;
  _wavelengthM = wavelengthM;

  // initialize pid for wavelength
  
  _pid.setWavelengthCm(wavelengthM * 100.0);

  // alloc computation arrays

  _allocArrays();
  
  // load input arrays
  
  if (_loadInputArrays(inputRay)) {
    cerr << "ERROR - RadxRate::ScalarsCompute - cannot load input arrays" << endl;
    return NULL;
  }

  // create output ray
  
  RadxRay *outputRay = new RadxRay;
  outputRay->copyMetaData(*inputRay);
  
  // compute kdp

  _kdpCompute();

  // compute pid

  if (_pid.loadTempProfile(inputRay->getTimeSecs())) {
    cerr << "ERROR - RadxRate::ScalarsCompute - cannot load temp profile" << endl;
    return NULL;
  }
  _pidCompute();

  // compute precip

  _precipCompute();
  
  // load output fields into the moments ray
  
  _loadOutputFields(inputRay, outputRay);

  return outputRay;

}

//////////////////////////////////////
// initialize KDP
  
void ScalarsCompute::_kdpInit()
  
{

  // initialize KDP object

  _kdp.setFromParams(_kdpFiltParams);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _kdp.setDebug(true);
  }

}

////////////////////////////////////////////////
// compute kdp from phidp, using Bringi's method

void ScalarsCompute::_kdpCompute()
  
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
  const double *kdpSC = _kdp.getKdpSC();
  
  // put KDP into fields objects
  
  for (size_t ii = 0; ii < _nGates; ii++) {
    if (kdp[ii] == NAN) {
      _kdpArray[ii] = missingDbl;
    } else {
      _kdpArray[ii] = kdp[ii];
    }
    _kdpScArray[ii] = kdpSC[ii];
  }

}

//////////////////////////////////////
// initialize PID
  
int ScalarsCompute::_pidInit()
  
{

  // initialize PID object

  if (_pid.setFromParams(_ncarPidParams)) {
    return -1;
  }
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _pid.setVerbose(true);
  } else if (_params.debug >= Params::DEBUG_VERBOSE) {
    _pid.setDebug(true);
  }

  return 0;
  
}

//////////////////////////////////////
// compute PID

void ScalarsCompute::_pidCompute()
  
{
  
  // select fields

  const double *kdpArray = _kdpArray;
  if (_params.PID_use_KDP_self_consistency) {
    kdpArray = _kdpScArray;
  }

  const double *dbzArray = _dbzArray;
  const double *zdrArray = _zdrArray;
  if (_params.PID_use_attenuation_corrected_fields) {
    dbzArray = _kdp.getDbzCorrected();
    zdrArray = _kdp.getZdrCorrected();
  }

  // compute particle ID

  _pid.computePidBeam(_nGates, _snrArray,
                      dbzArray, zdrArray, kdpArray,
                      _ldrArray, _rhohvArray,
                      _phidpArray, _tempForPid);
  
  // load results

  memcpy(_pidArray, _pid.getPid(), _nGates * sizeof(int));
  memcpy(_pidInterest, _pid.getInterest(), _nGates * sizeof(double));
  
}

//////////////////////////////////////
// initialize PRECIP
  
void ScalarsCompute::_precipInit()
  
{

  // initialize PRECIP object

  _precip.setFromParams(_precipRateParams);

}

//////////////////////////////////////
// compute PRECIP
  
void ScalarsCompute::_precipCompute()
  
{
  
  // select fields

  const double *kdpArray = _kdpArray;
  if (_params.RATE_use_KDP_self_consistency) {
    kdpArray = _kdpScArray;
  }

  const double *dbzArray = _dbzArray;
  const double *zdrArray = _zdrArray;
  if (_params.RATE_use_attenuation_corrected_fields) {
    dbzArray = _kdp.getDbzCorrected();
    zdrArray = _kdp.getZdrCorrected();
  }

  // compute rates
  
  _precip.computePrecipRates(_nGates, _snrArray,
                             dbzArray, zdrArray,
                             kdpArray, missingDbl,
                             &_pid);
    
  // save results

  memcpy(_rateZ, _precip.getRateZ(), _nGates * sizeof(double));
  memcpy(_rateZSnow, _precip.getRateZSnow(), _nGates * sizeof(double));
  memcpy(_rateZZdr, _precip.getRateZZdr(), _nGates * sizeof(double));
  memcpy(_rateKdp, _precip.getRateKdp(), _nGates * sizeof(double));
  memcpy(_rateKdpZdr, _precip.getRateKdpZdr(), _nGates * sizeof(double));
  memcpy(_rateHybrid, _precip.getRateHybrid(), _nGates * sizeof(double));
  memcpy(_ratePid, _precip.getRatePid(), _nGates * sizeof(double));
  memcpy(_rateHidro, _precip.getRateHidro(), _nGates * sizeof(double));
  memcpy(_rateBringi, _precip.getRateBringi(), _nGates * sizeof(double));

}

//////////////////////////////////////
// alloc computational arrays
  
void ScalarsCompute::_allocArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _ldrArray = _ldrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);

  _kdpArray = _kdpArray_.alloc(_nGates);
  _kdpScArray = _kdpScArray_.alloc(_nGates);

  _pidArray = _pidArray_.alloc(_nGates);
  _pidInterest = _pidInterest_.alloc(_nGates);
  _tempForPid = _tempForPid_.alloc(_nGates);

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

/////////////////////////////////////////////////////
// load input arrays ready for KDP
  
int ScalarsCompute::_loadInputArrays(RadxRay *inputRay)
  
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
  
  _loadFieldArray(inputRay, "TEMPC", true, _tempForPid);

  return 0;
  
}

////////////////////////////////////////
// load a field array based on the name

int ScalarsCompute::_loadFieldArray(RadxRay *inputRay,
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
    cerr << "ERROR - ScalarsCompute::_getField" << endl;
    cerr << "  Cannot find field in ray: " << fieldName<< endl;
    cerr << "  El, az: "
         << inputRay->getElevationDeg() << ", "
         << inputRay->getAzimuthDeg() << endl;
    cerr << "  N fields in ray: " << inputRay->getFields().size() << endl;
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

void ScalarsCompute::_computeSnrFromDbz()

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
// Censor gates with non-weather particle types

void ScalarsCompute::_censorNonWeather(RadxField &field)

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

///////////////////////////////
// load up fields in output ray

void ScalarsCompute::_loadOutputFields(RadxRay *inputRay,
                                       RadxRay *outputRay)

{

  // initialize array pointers

  const double *dbzAtten = _kdp.getDbzAttenCorr();
  const double *zdrAtten = _kdp.getZdrAttenCorr();
  const double *dbzCorrected = _kdp.getDbzCorrected();
  const double *zdrCorrected = _kdp.getZdrCorrected();
  
  // load up output data
  
  for (int ifield = 0; ifield < _params.pid_output_fields_n; ifield++) {
    
    const Params::pid_output_field_t &ofld = _params._pid_output_fields[ifield];
    if (!ofld.do_write) {
      continue;
    }
    
    // fill data array
    
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(_nGates);
    Radx::fl32 *datp = data;
    
    double minValidPrecipRate = _precipRateParams.RATE_min_valid_rate;

    for (size_t igate = 0; igate < _nGates; igate++, datp++) {
    
      switch (ofld.id) {

        // PRECIP RATE
        
        case Params::RATE_ZH: {
          double rate = _rateZ[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }
          
        case Params::RATE_ZH_SNOW: {
          double rate = _rateZSnow[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }
          
        case Params::RATE_Z_ZDR: {
          double rate = _rateZZdr[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }
          
        case Params::RATE_KDP: {
          double rate = _rateKdp[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

        case Params::RATE_KDP_ZDR: {
          double rate = _rateKdpZdr[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

        case Params::RATE_HYBRID: {
          double rate = _rateHybrid[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

        case Params::RATE_PID: {
          double rate = _ratePid[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

        case Params::RATE_HIDRO: {
          double rate = _rateHidro[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

        case Params::RATE_BRINGI: {
          double rate = _rateBringi[igate];
          if (rate < minValidPrecipRate) {
            *datp = missingDbl;
          } else {
            *datp = rate;
          }
          break;
        }

          // PID
        
        case Params::PID_OUT:
          *datp = _pidArray[igate];
          break;
        case Params::PID_INTEREST:
          *datp = _pidInterest[igate];
          break;
        case Params::TEMP_FOR_PID:
          *datp = _tempForPid[igate];
          break;
          
          // computed KDP
        
        case Params::KDP_OUT:
          *datp = _kdpArray[igate];
          break;
        case Params::KDP_SC:
          *datp = _kdpScArray[igate];
          break;
          
          // attenuation
          
        case Params::DBZ_ATTEN_CORRECTION:
          *datp = dbzAtten[igate];
          break;
        case Params::ZDR_ATTEN_CORRECTION:
          *datp = zdrAtten[igate];
          break;
        case Params::DBZ_ATTEN_CORRECTED:
          *datp = dbzCorrected[igate];
          break;
        case Params::ZDR_ATTEN_CORRECTED:
          *datp = zdrCorrected[igate];
          break;

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

    outputRay->addField(field);

  } // ifield
  
  // copy fields through as required

  if (_params.copy_selected_input_fields_to_output) {

    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      const Params::copy_field_t &cfield = _params._copy_fields[ii];
      RadxField *inField = inputRay->getField(cfield.field_name);
      if (inField != NULL) {
        RadxField *outField = new RadxField(*inField);
        if (cfield.censor_non_weather) {
          _censorNonWeather(*outField);
        }
        outputRay->addField(outField);
      }
    } // ii

  } // if (_params.copy_input_fields_to_output)

  // add debug fields if required

  if (_params.PID_write_debug_fields) {
    _addPidDebugFields(inputRay, outputRay);
  }

  if (_params.KDP_write_debug_fields) {
    _addKdpDebugFields(outputRay);
  }

}

//////////////////////////////////////
// add the debug fields
  
void ScalarsCompute::_addPidDebugFields(const RadxRay *inputRay, 
                                        RadxRay *outputRay)

{

  _addField(outputRay,
            "DBZ_FOR_PID", "dBZ",
            "dbz_filtered_for_pid_computations",
            "equivalent_reflectivity_factor",
            _pid.getDbz());
  
  _addField(outputRay,
            "ZDR_FOR_PID", "dB",
            "zdr_filtered_for_pid_computations",
            "differential_reflectivity_hv",
            _pid.getZdr());
  
  _addField(outputRay,
            "KDP_FOR_PID", "deg/km",
            "specific_differential_phase_for_pid_computations",
            "specific_differential_phase_hv",
            _pid.getKdp());
  
  if (_params.LDR_available) {
    _addField(outputRay,
              "LDR_FOR_PID", "dB",
              "linear_differential_refectivity_for_pid_computations",
              "linear_differential_reflectivity",
              _pid.getLdr());
  }

  _addField(outputRay,
            "RHOHV_FOR_PID", "",
            "rhohv_filtered_for_pid_computations",
            "cross_correlation_hv",
            _pid.getRhohv());
  
  _addField(outputRay,
            "PHIDP_FOR_PID", "deg",
            "phidp_for_pid_computations",
            "differential_phase_hv",
            _pid.getPhidp());
  
  _addField(outputRay,
            "ZDR_SDEV_FOR_PID", "dB",
            "standard_deviation_of_zdr_for_pid_computations",
            "differential_reflectivity_hv",
            _pid.getSdzdr());
  
  _addField(outputRay,
            "PHIDP_SDEV_FOR_PID", "deg",
            "standard_deviation_of_phidp_for_pid_computations",
            "differential_phase_hv",
            _pid.getSdphidp());
  
  _addField(outputRay,
            "ML_INTEREST", "",
            "melting_layer_interest",
            "melting_layer_interest",
            _pid.getMlInterest());
  
  _addField(outputRay,
            "BEAM_HT", "km",
            "beam_height",
            "height_to_center_of_beam_msl",
            inputRay->getField("beam_height")->getDataFl32());
  
  _addField(outputRay,
            "RANGE", "km",
            "range",
            "range_to_center_of_gate",
            inputRay->getField("range")->getDataFl32());
  
  _addField(outputRay,
            "ELEVATION", "deg",
            "elevation",
            "elevation_angle",
            inputRay->getField("elevation")->getDataFl32());
  
  
}
  
//////////////////////////////////////
// add the debug fields
  
void ScalarsCompute::_addKdpDebugFields(RadxRay *outputRay)

{

  _addField(outputRay,
            "KDP_ZZDR", "deg/km",
            "specific_differential_phase_theoretical_from_z_and_zdr",
            "specific_differential_phase_hv",
            _kdp.getKdpZZdr());
  
  _addField(outputRay,
            "PSOB", "deg",
            "phase_shift_on_backscatter",
            "phase_shift_on_backscatter",
            _kdp.getPsob());
  
  _addField(outputRay,
            "DBZ_FOR_KDP", "dBZ",
            "dbz_filtered_for_kdp_computations",
            "equivalent_reflectivity_factor",
            _kdp.getDbz());
  
  _addField(outputRay,
            "SNR_FOR_KDP", "dB",
            "snr_filtered_for_kdp_computations",
            "signal_to_noise_ratio",
            _kdp.getSnr());
  
  _addField(outputRay,
            "ZDR_FOR_KDP", "dB",
            "zdr_filtered_for_kdp_computations",
            "differential_reflectivity_hv",
            _kdp.getZdr());
  
  _addField(outputRay,
            "ZDR_SDEV_FOR_KDP", "dB",
            "standard_deviation_of_zdr_for_kdp_computations",
            "differential_reflectivity_hv",
            _kdp.getZdrSdev());
  
  _addField(outputRay,
            "RHOHV_FOR_KDP", "",
            "rhohv_filtered_for_kdp_computations",
            "cross_correlation_hv",
            _kdp.getRhohv());
  
  _addField(outputRay,
            "VALID_FLAG_FOR_KDP", "",
            "valid_flag_after_kdp_computations",
            "valid_flag_for_kdp",
            _kdp.getValidForKdp());
  
  _addField(outputRay,
            "PHIDP_FOR_KDP", "deg",
            "phidp_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidp());
  
  _addField(outputRay,
            "PHIDP_MEAN", "deg",
            "phidp_mean_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpMean());
  
  _addField(outputRay,
            "PHIDP_MEAN_UNFOLD", "deg",
            "phidp_mean_unfold_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpMeanUnfold());
  
  _addField(outputRay,
            "PHIDP_SDEV", "deg",
            "phidp_sdev_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpSdev());
  
  _addField(outputRay,
            "PHIDP_JITTER", "deg",
            "phidp_jitter_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpJitter());
  
  _addField(outputRay,
            "PHIDP_UNFOLD", "deg",
            "phidp_unfold_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpUnfold());
  
  _addField(outputRay,
            "PHIDP_FILT", "deg",
            "phidp_filtered_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpFilt());
  
  _addField(outputRay,
            "PHIDP_COND", "deg",
            "phidp_cond_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpCond());
  
}
  
//////////////////////////////////////
// add a field to the output ray
  
void ScalarsCompute::_addField(RadxRay *outputRay,
                               const string &name,
                               const string &units,
                               const string &longName,
                               const string standardName,
                               const double *array64)

{

  // load up data as fl32
  
  TaArray<Radx::fl32> data32_;
  Radx::fl32 *data32 = data32_.alloc(_nGates);
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    if (array64[igate] == missingDbl) {
      data32[igate] = Radx::missingFl32;
    } else {
      data32[igate] = array64[igate];
    }
  }
    
  // create field
  
  RadxField *field = new RadxField(name, units);
  field->setLongName(longName);
  field->setStandardName(standardName);
  field->setTypeFl32(Radx::missingFl32);
  field->addDataFl32(_nGates, data32);
  field->copyRangeGeom(*outputRay);
  
  // add to ray
  
  outputRay->addField(field);

}

//////////////////////////////////////
// add a field to the output ray
  
void ScalarsCompute::_addField(RadxRay *outputRay,
                               const string &name,
                               const string &units,
                               const string &longName,
                               const string standardName,
                               const Radx::fl32 *array32)
  
{

  // load up data as fl32
  
  TaArray<Radx::fl32> data32_;
  Radx::fl32 *data32 = data32_.alloc(_nGates);
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    if (array32[igate] == missingDbl) {
      data32[igate] = Radx::missingFl32;
    } else {
      data32[igate] = array32[igate];
    }
  }
    
  // create field
  
  RadxField *field = new RadxField(name, units);
  field->setLongName(longName);
  field->setStandardName(standardName);
  field->setTypeFl32(Radx::missingFl32);
  field->addDataFl32(_nGates, data32);
  field->copyRangeGeom(*outputRay);
  
  // add to ray
  
  outputRay->addField(field);

}

void ScalarsCompute::_addField(RadxRay *outputRay,
                               const string &name,
                               const string &units,
                               const string &longName,
                               const string standardName,
                               const bool *arrayBool)

{

  // load up data as fl32
  
  TaArray<Radx::fl32> data32_;
  Radx::fl32 *data32 = data32_.alloc(_nGates);
  
  for (size_t igate = 0; igate < _nGates; igate++) {
    if (arrayBool[igate]) {
      data32[igate] = 1.0;
    } else {
      data32[igate] = 0.0;
    }
  }
    
  // create field
  
  RadxField *field = new RadxField(name, units);
  field->setLongName(longName);
  field->setStandardName(standardName);
  field->setTypeFl32(Radx::missingFl32);
  field->addDataFl32(_nGates, data32);
  field->copyRangeGeom(*outputRay);
  
  // add to ray
  
  outputRay->addField(field);

}


