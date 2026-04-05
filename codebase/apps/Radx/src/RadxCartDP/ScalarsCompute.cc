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

ScalarsCompute::ScalarsCompute(RadxCartDP *parent, 
                               const Params &params,
                               const KdpFiltParams &kdpFiltParams,
                               const NcarPidParams &ncarPidParams,
                               // const PrecipRateParams &precipRateParams,
                               int id)  :
        _parent(parent),
        _params(params),
        _kdpFiltParams(kdpFiltParams),
        _ncarPidParams(ncarPidParams),
        // _precipRateParams(precipRateParams),
        _id(id)
  
{

  OK = true;
  
  // initialize kdp and pid computations

  _kdpInit();

  if (_pidInit()) {
    OK = false;
  }

  // _precipInit();

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

// void ScalarsCompute::loadTempProfile(time_t dataTime)
// {
//   _pid.loadTempProfile(dataTime);
// }

///////////////////////////////////////////////////////////
// Set the temperature profile

// void ScalarsCompute::setTempProfile(const TempProfile &profile)
// {
//   _pid.setTempProfile(profile);
// }

///////////////////////////////////////////////////////////
// Get the temperature profile

// const TempProfile &ScalarsCompute::getTempProfile() const
// {
//   return _pid.getTempProfile();
// }

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

  // if (_pid.loadTempProfile(inputRay->getTimeSecs())) {
  //   cerr << "ERROR - RadxRate::ScalarsCompute - cannot load temp profile" << endl;
  //   return NULL;
  // }
  _pidPrepare();

  // compute precip

  // _precipCompute();
  
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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _kdp.setDebug(true);
  }

}

////////////////////////////////////////////////
// compute kdp from phidp, using Bringi's method

void ScalarsCompute::_kdpCompute()
  
{

  // set up array for range
  
  // vector<double> rangeKm(_nGates);
  // double range = _startRangeKm;
  // for (size_t ii = 0; ii < _nGates; ii++, range += _gateSpacingKm) {
  //   rangeKm[ii] = range;
  // }

  // compute KDP
  
  _kdp.compute(_timeSecs,
               _nanoSecs / 1.0e9,
               _elevation,
               _azimuth,
               _wavelengthM * 100.0,
               _nGates, 
               _startRangeKm,
               _gateSpacingKm,
               _snrArray.data(),
               _dbzArray.data(),
               _zdrArray.data(),
               _rhohvArray.data(),
               _phidpArray.data(),
               missingDbl);

  const double *kdp = _kdp.getKdp();
  const double *kdpSC = _kdp.getKdpSC();
  
  // save KDP in arrays on this class
  
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

//////////////////////////////////////////
// compute fields in preparation for PID

void ScalarsCompute::_pidPrepare()
  
{
  
  // select fields

  vector<double> &kdpArray = _kdpArray;
  if (_params.PID_use_KDP_self_consistency) {
    kdpArray = _kdpScArray;
  }
  
  const double *dbzArray = _dbzArray.data();
  const double *zdrArray = _zdrArray.data();
  if (_params.PID_use_attenuation_corrected_fields) {
    dbzArray = _kdp.getDbzCorrected();
    zdrArray = _kdp.getZdrCorrected();
  }
  
  // compute particle ID

  _pid.prepareForPid(_nGates, _snrArray.data(),
                     dbzArray, zdrArray, kdpArray.data(),
                     _ldrArray.data(), _rhohvArray.data(),
                     _phidpArray.data());
  
}

#ifdef NOTNOW

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

  const double *dbzArray = _dbzArray.data();
  const double *zdrArray = _zdrArray.data();
  if (_params.RATE_use_attenuation_corrected_fields) {
    dbzArray = _kdp.getDbzCorrected();
    zdrArray = _kdp.getZdrCorrected();
  }

  // compute rates
  
  _precip.computePrecipRates(_nGates, _snrArray.data(),
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

#endif

//////////////////////////////////////
// alloc computational arrays
  
void ScalarsCompute::_allocArrays()
  
{

  _snrArray.resize(_nGates);
  _dbzArray.resize(_nGates);
  _zdrArray.resize(_nGates);
  _ldrArray.resize(_nGates);
  _rhohvArray.resize(_nGates);
  _phidpArray.resize(_nGates);

  _kdpArray.resize(_nGates);
  _kdpScArray.resize(_nGates);

  // _tempForPid.resize(_nGates);

  _sdZdr.resize(_nGates);
  _sdPhidp.resize(_nGates);

}

/////////////////////////////////////////////////////
// load input arrays
  
int ScalarsCompute::_loadInputArrays(RadxRay *inputRay)
  
{
  
  if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::DBZ),
                      _dbzArray.data())) {
    cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
    cerr << "  Cannot load DBZ field name: "
         << _parent->getRadarInputName(Params::DBZ) << endl;
    return -1;
  }
  
  if (_parent->getRadarInputName(Params::SNR).size() > 0) {
    if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::SNR),
                        _snrArray.data())) {
      cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
      cerr << "  Cannot load SNR field name: "
           << _parent->getRadarInputName(Params::SNR) << endl;
      return -1;
    }
  } else {
    _computeSnrFromDbz();
  }
  
  if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::ZDR),
                      _zdrArray.data())) {
    cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
    cerr << "  Cannot load ZDR field name: "
         << _parent->getRadarInputName(Params::ZDR) << endl;
    return -1;
  }

  if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::PHIDP),
                      _phidpArray.data())) {
    cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
    cerr << "  Cannot load PHIDP field name: "
         << _parent->getRadarInputName(Params::PHIDP) << endl;
    return -1;
  }
  
  if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::RHOHV),
                      _rhohvArray.data())) {
    cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
    cerr << "  Cannot load RHOHV field name: "
         << _parent->getRadarInputName(Params::RHOHV) << endl;
    return -1;
  }
  
  if (_loadFieldArray(inputRay, _parent->getRadarInputName(Params::LDR),
                      _ldrArray.data())) {
    cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
    cerr << "  Cannot load LDR field name: "
         << _parent->getRadarInputName(Params::LDR) << endl;
    return -1;
  }
  
  // if (_loadFieldArray(inputRay, RadxCartDP::tempFieldName, _tempForPid.data())) {
  //   cerr << "ERROR - ScalarsCompute::_loadInputArrays" << endl;
  //   cerr << "  Cannot load temp field name: "
  //        << RadxCartDP::tempFieldName << endl;
  //   return -1;
  // }
  
  return 0;
  
}

////////////////////////////////////////
// load a field array based on the name

int ScalarsCompute::_loadFieldArray(RadxRay *inputRay,
                                    const string &fieldName,
                                    double *array)

{

  RadxField *field = inputRay->getField(fieldName);
  if (field == NULL) {

    if (fieldName.size() == 0) {
      for (size_t igate = 0; igate < _nGates; igate++) {
        array[igate] = missingDbl;
      }
      return 0;
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

////////////////////////////////////////
// add a derived field to a ray

int ScalarsCompute::_addFieldToRay(RadxRay *inputRay,
                                   const string &fieldName,
                                   const string &units,
                                   double *array,
                                   double miss)

{

  RadxField *field = new RadxField(fieldName, units);
  field->setTypeFl64(miss);
  field->addDataFl64(_nGates, array);
  inputRay->addField(field);
  
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
  
  double *snr = _snrArray.data();
  const double *dbz = _dbzArray.data();
  for (size_t igate = 0; igate < _nGates; igate++, snr++, dbz++) {
    if (*dbz != missingDbl) {
      *snr = *dbz - noiseDbz[igate];
    } else {
      *snr = -20;
    }
  }

}

///////////////////////////////
// load up fields in output ray

void ScalarsCompute::_loadOutputFields(RadxRay *inputRay,
                                       RadxRay *outputRay)

{

  // memcpy(_snrArray.data(), _pid.getSnr(), _nGates * sizeof(double));
  // memcpy(_dbzArray.data(), _pid.getDbz(), _nGates * sizeof(double));
  // memcpy(_zdrArray.data(), _pid.getZdr(), _nGates * sizeof(double));
  // memcpy(_ldrArray.data(), _pid.getLdr(), _nGates * sizeof(double));
  // memcpy(_tempForPid.data(), _pid.getTempC(), _nGates * sizeof(double));
  // memcpy(_sdZdr.data(), _pid.getSdzdr(), _nGates * sizeof(double));
  // memcpy(_sdPhidp.data(), _pid.getSdphidp(), _nGates * sizeof(double));

  _addField(outputRay,
            "SNR_FOR_PID", "dB",
            "snr_filtered_for_pid_computations",
            "signal_to_noise_ratio",
            _snrArray.data());
  //            _pid.getSnr());
  
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
  
  if (_parent->getRadarInputName(Params::LDR).size() > 0) {
    _addField(outputRay,
              "LDR_FOR_PID", "dB",
              "linear_differential_refectivity_for_pid_computations",
              "linear_differential_reflectivity",
              _pid.getLdr());
  }
  
  _addField(outputRay,
            "ZDR_SDEV_FOR_PID", "dB",
            "standard_deviation_of_zdr_for_pid_computations",
            "sdev_of_differential_reflectivity_hv",
            _pid.getSdzdr());
  
  _addField(outputRay,
            "PHIDP_SDEV_FOR_PID", "deg",
            "standard_deviation_of_phidp_for_pid_computations",
            "sdev_of_differential_phase_hv",
            _pid.getSdphidp());
  
  // _addField(outputRay,
  //           "TEMP_FOR_PID", "C",
  //           "temperature_for_pid_computations",
  //           "temperature",
  //           _pid.getTempC());
  
  if (_params.PID_use_KDP_self_consistency) {
    _addField(outputRay,
              "KDP_FOR_PID", "deg/km",
              "specific_differential_phase_for_pid_computations",
              "specific_differential_phase_hv",
              _kdp.getKdpSC());
  } else {
    _addField(outputRay,
              "KDP_FOR_PID", "deg/km",
              "specific_differential_phase_for_pid_computations",
              "specific_differential_phase_hv",
              _kdp.getKdp());
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
  
  // copy fields through as required
  
  if (_params.copy_selected_input_fields_to_output) {

    for (int ii = 0; ii < _params.copy_fields_n; ii++) {
      const Params::copy_field_t &cfield = _params._copy_fields[ii];
      RadxField *inField = inputRay->getField(cfield.field_name);
      if (inField != NULL) {
        RadxField *outField = new RadxField(*inField);
        outputRay->addField(outField);
      }
    } // ii

  } // if (_params.copy_input_fields_to_output)

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


