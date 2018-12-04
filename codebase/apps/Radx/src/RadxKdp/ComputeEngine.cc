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
  
  // allocate input arrays for computing derived fields,
  // and load them up
  
  _allocInputArrays();
  _loadInputArrays(inputRay);

  // alloc the derived field arrays

  _allocDerivedArrays();
  
  // compute kdp
  
  _kdpCompute();

  // load output fields into the moments ray
  
  _loadOutputFields(inputRay, derivedRay);

  return derivedRay;

}

//////////////////////////////////////
// initialize KDP
  
void ComputeEngine::_kdpInit()
  
{

  // initialize KDP object

  _kdp.setFromParams(_kdpFiltParams);
  
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
// alloc input arrays
  
void ComputeEngine::_allocInputArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);

}

//////////////////////////////////////
// alloc derived arrays
  
void ComputeEngine::_allocDerivedArrays()
  
{
  
  _kdpArray = _kdpArray_.alloc(_nGates);
  _kdpZZdrArray = _kdpZZdrArray_.alloc(_nGates);
  _kdpCondArray = _kdpCondArray_.alloc(_nGates);

}

/////////////////////////////////////////////////////
// load input arrays ready for KDP
  
int ComputeEngine::_loadInputArrays(RadxRay *inputRay)
  
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
  
  return 0;
  
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

///////////////////////////////
// load up fields in output ray

void ComputeEngine::_loadOutputFields(RadxRay *inputRay,
                                      RadxRay *derivedRay)

{

  // initialize array pointers

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

  _addField(derivedRay,
            "DBZ_FOR_KDP", "dBZ",
            "dbz_filtered_for_kdp_computations",
            "equivalent_reflectivity_factor",
            _kdp.getDbz());
  
  _addField(derivedRay,
            "SNR_FOR_KDP", "dB",
            "snr_filtered_for_kdp_computations",
            "signal_to_noise_ratio",
            _kdp.getSnr());
  
  _addField(derivedRay,
            "ZDR_FOR_KDP", "dB",
            "zdr_filtered_for_kdp_computations",
            "differential_reflectivity_hv",
            _kdp.getZdr());
  
  _addField(derivedRay,
            "ZDR_SDEV_FOR_KDP", "dB",
            "standard_deviation_of_zdr_for_kdp_computations",
            "differential_reflectivity_hv",
            _kdp.getZdrSdev());
  
  _addField(derivedRay,
            "RHOHV_FOR_KDP", "",
            "rhohv_filtered_for_kdp_computations",
            "cross_correlation_hv",
            _kdp.getRhohv());
  
  _addField(derivedRay,
            "VALID_FLAG_FOR_KDP", "",
            "valid_flag_after_kdp_computations",
            "valid_flag_for_kdp",
            _kdp.getValidForKdp());
  
  _addField(derivedRay,
            "PHIDP_FOR_KDP", "deg",
            "phidp_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidp());
  
  _addField(derivedRay,
            "PHIDP_MEAN", "deg",
            "phidp_mean_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpMean());
  
  _addField(derivedRay,
            "PHIDP_MEAN_UNFOLD", "deg",
            "phidp_mean_unfold_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpMeanUnfold());
  
  _addField(derivedRay,
            "PHIDP_SDEV", "deg",
            "phidp_sdev_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpSdev());
  
  _addField(derivedRay,
            "PHIDP_JITTER", "deg",
            "phidp_jitter_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpJitter());
  
  _addField(derivedRay,
            "PHIDP_UNFOLD", "deg",
            "phidp_unfold_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpUnfold());
  
  _addField(derivedRay,
            "PHIDP_FILT", "deg",
            "phidp_filtered_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpFilt());
  
  _addField(derivedRay,
            "PHIDP_COND", "deg",
            "phidp_cond_for_kdp_computations",
            "differential_phase_hv",
            _kdp.getPhidpCond());
  
}
  
//////////////////////////////////////
// add a field to the derived ray
  
void ComputeEngine::_addField(RadxRay *derivedRay,
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
  field->copyRangeGeom(*derivedRay);
  
  // add to ray
  
  derivedRay->addField(field);

}

void ComputeEngine::_addField(RadxRay *derivedRay,
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
  field->copyRangeGeom(*derivedRay);
  
  // add to ray
  
  derivedRay->addField(field);

}


