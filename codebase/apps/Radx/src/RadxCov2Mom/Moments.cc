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
// Moments.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Moments computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#include "Moments.hh"
#include <toolsa/DateTime.hh>
#include <rapmath/trig.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
using namespace std;

pthread_mutex_t Moments::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;
string Moments::censorFlagFieldName = "CENSOR_FLAG";

// Constructor

Moments::Moments(const Params &params)  :
        _params(params)
  
{

  OK = true;
  
  // initialize moments, kdp, pid and precip objects

  _momInit();
  _kdpInit();

  // initialize noise location

  if (_noiseInit()) {
    OK = FALSE;
  }

  // initialize vel2 processing

  if (_params.load_alt_vel_alt_test_fields) {
    _altVel.setLoadTestFields(true);
  }

}

// destructor

Moments::~Moments()

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

RadxRay *Moments::compute(const RadxRay *covRay,
                          const IwrfCalib &calib,
                          double measXmitPowerDbmH,
                          double measXmitPowerDbmV,
                          double wavelengthM,
                          double radarHtKm)
{

  // create moments ray
  
  RadxRay *momRay = new RadxRay;
  momRay->copyMetaData(*covRay);
  _calib = calib;
  _measXmitPowerDbmH = measXmitPowerDbmH;
  _measXmitPowerDbmV = measXmitPowerDbmV;

  // set the measured power to the mean for the volume

  momRay->setMeasXmitPowerDbmH(measXmitPowerDbmH);
  momRay->setMeasXmitPowerDbmV(measXmitPowerDbmV);

  // set ray data
  
  _nGates = covRay->getNGates();
  _startRangeKm = covRay->getStartRangeKm();
  _gateSpacingKm = covRay->getGateSpacingKm();
  _radarHtKm = radarHtKm;
  _azimuth = covRay->getAzimuthDeg();
  _elevation = covRay->getElevationDeg();
  _timeSecs = covRay->getTimeSecs();
  _nanoSecs = covRay->getNanoSecs();
  
  _noise.setRayProps(_nGates, _calib,
                     _timeSecs, _nanoSecs,
                     _elevation, _azimuth);

  // initialize moments object

  _wavelengthM = wavelengthM;
  _rmom.setMaxGates(_nGates);
  _rmom.init(covRay->getPrtSec(), _wavelengthM,
             covRay->getStartRangeKm(), covRay->getGateSpacingKm());
  _rmom.setNSamples(covRay->getNSamples());
  if (_measXmitPowerDbmH > 0) {
    _rmom.setMeasXmitPowerDbmH(_measXmitPowerDbmH);
  }
  if (_measXmitPowerDbmV > 0) {
    _rmom.setMeasXmitPowerDbmV(_measXmitPowerDbmV);
  }
  _rmom.setCalib(_calib);
  _nyquist = covRay->getNyquistMps();

  // load up the atmospheric attenuation correction
  if (_params.atmos_atten_method == Params::ATMOS_ATTEN_NONE) {
    _atmosAtten.setAttenNone();
  } else if (_params.atmos_atten_method == Params::ATMOS_ATTEN_CONSTANT) {
    _atmosAtten.setAttenConstant(_params.atmos_atten_db_per_km);
  } else if (_params.atmos_atten_method == Params::ATMOS_ATTEN_CRPL) {
    double wavelengthCm = _wavelengthM * 100.0;
    _atmosAtten.setAttenCrpl(wavelengthCm);
  }
  _rmom.loadAtmosAttenCorrection(_nGates, _elevation, _atmosAtten);

  // allocate moments fields array, and initialize

  MomentsFields *mfields = _momFields.alloc(_nGates);
  Covars *covars = _covars.alloc(_nGates);
  _censorFlag = _censorFlag_.alloc(_nGates);
  
  for (int igate = 0; igate < _nGates; igate++) {
    mfields[igate].init();
    covars[igate].init();
    _censorFlag[igate] = false;
  }

  // load up covariances

  int iret = 0;

  switch (_params.xmit_rcv_mode) {

    case Params::DUAL_POL_ALT_HV_CO_ONLY:
      iret = _getCovariancesDpAltHvCoOnly(covRay);
      break;
      
    case Params::DUAL_POL_ALT_HV_CO_CROSS:
      iret = _getCovariancesDpAltHvCoCross(covRay);
      break;

    case Params::DUAL_POL_SIM_HV:
      iret = _getCovariancesDpSimHv(covRay);
      break;

    case Params::DUAL_POL_H_ONLY:
      iret = _getCovariancesDpHOnly(covRay);
      break;

    case Params::DUAL_POL_V_ONLY:
      iret = _getCovariancesDpVOnly(covRay);
      break;

    case Params::SINGLE_POL_V:
      iret = _getCovariancesSinglePolV(covRay);
      break;

    case Params::SINGLE_POL:
    default:
      iret = _getCovariancesSinglePolH(covRay);
      break;

  } // switch

  if (iret) {
    delete momRay;
    return NULL;
  }

  // detect noise gates if applicable
  
  bool locateNoise = false;
  if (_params.censoring_mode == Params::CENSORING_BY_NOISE_FLAG) {
    locateNoise = true;
  }
  if (_params.compute_vol_noise_stats) {
    if (_elevation >= _params.vol_noise_stats_min_elev_deg &&
        _elevation <= _params.vol_noise_stats_max_elev_deg) {
      locateNoise = true;
    }
  }

  if (locateNoise) {
    
    switch (_params.xmit_rcv_mode) {
      case Params::DUAL_POL_ALT_HV_CO_ONLY:
        _computeNoiseDpAltHvCoOnly(momRay);
        break;
      case Params::DUAL_POL_ALT_HV_CO_CROSS:
        _computeNoiseDpAltHvCoCross(momRay);
        break;
      case Params::DUAL_POL_SIM_HV:
        _computeNoiseDpSimHv(momRay);
        break;
      case Params::DUAL_POL_H_ONLY:
        _computeNoiseDpHOnly(momRay);
        break;
      case Params::DUAL_POL_V_ONLY:
        _computeNoiseDpVOnly(momRay);
        break;
      case Params::SINGLE_POL_V:
        _computeNoiseSinglePolV(momRay);
      case Params::SINGLE_POL:
      default:
        _computeNoiseSinglePolH(momRay);
    } // switch

  } // if (locateNoise) ...

  // compute moments based on mode

  switch (_params.xmit_rcv_mode) {

    case Params::DUAL_POL_ALT_HV_CO_ONLY:
      _computeMomDpAltHvCoOnly();
      break;
      
    case Params::DUAL_POL_ALT_HV_CO_CROSS:
      _computeMomDpAltHvCoCross();
      break;

    case Params::DUAL_POL_SIM_HV:
      _computeMomDpSimHv();
      break;

    case Params::DUAL_POL_H_ONLY:
      _computeMomDpHOnly();
      break;

    case Params::DUAL_POL_V_ONLY:
      _computeMomDpVOnly();
      break;

    case Params::SINGLE_POL_V:
      _computeMomSinglePolV();
      break;

    case Params::SINGLE_POL:
    default:
      _computeMomSinglePolH();
      break;

  } // switch
  
  // allocate moments arrays for computing derived fields,
  // and load them up
  
  _allocMomentsArrays();
  _loadMomentsArrays();
  
  // compute kdp
  
  switch (_params.xmit_rcv_mode) {
    case Params::DUAL_POL_ALT_HV_CO_ONLY:
    case Params::DUAL_POL_ALT_HV_CO_CROSS:
    case Params::DUAL_POL_SIM_HV:
      _kdpCompute();
      break;
    default: {}
  } // switch
  
  // censor gates as required

  if (_params.censoring_mode == Params::CENSORING_BY_NOISE_FLAG) {
    _censorByNoiseFlag();
  } else if (_params.censoring_mode == Params::CENSORING_BY_SNR_AND_NCP) {
    _censorBySnrNcp();
  } 

  // load output fields into the moments ray
  
  _loadOutputFields(momRay);

  return momRay;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Single pol - H channel

int Moments::_getCovariancesSinglePolH(const RadxRay *covRay)
  
{
  
  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0HcDb = _getField(covRay, Params::LAG0_HC_DB);
  if (fldLag0HcDb == NULL) iret = -1;
  const RadxField *fldLag1HcDb = _getField(covRay, Params::LAG1_HC_DB);
  if (fldLag1HcDb == NULL) iret = -1;
  const RadxField *fldLag1HcPhase = _getField(covRay, Params::LAG1_HC_PHASE);
  if (fldLag1HcPhase == NULL) iret = -1;
  const RadxField *fldLag2HcDb = _getField(covRay, Params::LAG2_HC_DB);
  if (fldLag2HcDb == NULL) iret = -1;
  const RadxField *fldLag2HcPhase = _getField(covRay, Params::LAG2_HC_PHASE);
  if (fldLag2HcPhase == NULL) iret = -1;
  const RadxField *fldLag3HcDb = _getField(covRay, Params::LAG3_HC_DB);
  if (fldLag3HcDb == NULL) iret = -1;
  const RadxField *fldLag3HcPhase = _getField(covRay, Params::LAG3_HC_PHASE);
  if (fldLag3HcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_getCovariancesSinglePolH" << endl;
    return -1;
  }

  Radx::fl32 *lag0HcDb = (Radx::fl32 *) fldLag0HcDb->getData();

  Radx::fl32 *lag1HcDb = (Radx::fl32 *) fldLag1HcDb->getData();
  Radx::fl32 *lag1HcPhaseDeg = (Radx::fl32 *) fldLag1HcPhase->getData();

  Radx::fl32 *lag2HcDb = (Radx::fl32 *) fldLag2HcDb->getData();
  Radx::fl32 *lag2HcPhaseDeg = (Radx::fl32 *) fldLag2HcPhase->getData();

  Radx::fl32 *lag3HcDb = (Radx::fl32 *) fldLag3HcDb->getData();
  Radx::fl32 *lag3HcPhaseDeg = (Radx::fl32 *) fldLag3HcPhase->getData();

  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];
    covar.lag0Hc = pow(10.0, lag0HcDb[igate] / 10.0);

    double lag1HcMag = pow(10.0, lag1HcDb[igate] / 20.0);
    double lag1HcPhase = lag1HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2HcMag = pow(10.0, lag2HcDb[igate] / 20.0);
    double lag2HcPhase = lag2HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3HcMag = pow(10.0, lag3HcDb[igate] / 20.0);
    double lag3HcPhase = lag3HcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;

    rap_sincos(lag1HcPhase, &sinval, &cosval);
    covar.lag1Hc.set(lag1HcMag * cosval, lag1HcMag * sinval);

    rap_sincos(lag2HcPhase, &sinval, &cosval);
    covar.lag2Hc.set(lag2HcMag * cosval, lag2HcMag * sinval);

    rap_sincos(lag3HcPhase, &sinval, &cosval);
    covar.lag3Hc.set(lag3HcMag * cosval, lag3HcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Single pol - V channel

int Moments::_getCovariancesSinglePolV(const RadxRay *covRay)
  
{
  
  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0VcDb = _getField(covRay, Params::LAG0_VC_DB);
  if (fldLag0VcDb == NULL) iret = -1;
  const RadxField *fldLag1VcDb = _getField(covRay, Params::LAG1_VC_DB);
  if (fldLag1VcDb == NULL) iret = -1;
  const RadxField *fldLag1VcPhase = _getField(covRay, Params::LAG1_VC_PHASE);
  if (fldLag1VcPhase == NULL) iret = -1;
  const RadxField *fldLag2VcDb = _getField(covRay, Params::LAG2_VC_DB);
  if (fldLag2VcDb == NULL) iret = -1;
  const RadxField *fldLag2VcPhase = _getField(covRay, Params::LAG2_VC_PHASE);
  if (fldLag2VcPhase == NULL) iret = -1;
  const RadxField *fldLag3VcDb = _getField(covRay, Params::LAG3_VC_DB);
  if (fldLag3VcDb == NULL) iret = -1;
  const RadxField *fldLag3VcPhase = _getField(covRay, Params::LAG3_VC_PHASE);
  if (fldLag3VcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_getCovariancesSinglePolV" << endl;
    return -1;
  }

  Radx::fl32 *lag0VcDb = (Radx::fl32 *) fldLag0VcDb->getData();

  Radx::fl32 *lag1VcDb = (Radx::fl32 *) fldLag1VcDb->getData();
  Radx::fl32 *lag1VcPhaseDeg = (Radx::fl32 *) fldLag1VcPhase->getData();

  Radx::fl32 *lag2VcDb = (Radx::fl32 *) fldLag2VcDb->getData();
  Radx::fl32 *lag2VcPhaseDeg = (Radx::fl32 *) fldLag2VcPhase->getData();

  Radx::fl32 *lag3VcDb = (Radx::fl32 *) fldLag3VcDb->getData();
  Radx::fl32 *lag3VcPhaseDeg = (Radx::fl32 *) fldLag3VcPhase->getData();

  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];
    covar.lag0Vc = pow(10.0, lag0VcDb[igate] / 10.0);

    double lag1VcMag = pow(10.0, lag1VcDb[igate] / 20.0);
    double lag1VcPhase = lag1VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2VcMag = pow(10.0, lag2VcDb[igate] / 20.0);
    double lag2VcPhase = lag2VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3VcMag = pow(10.0, lag3VcDb[igate] / 20.0);
    double lag3VcPhase = lag3VcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;

    rap_sincos(lag1VcPhase, &sinval, &cosval);
    covar.lag1Vc.set(lag1VcMag * cosval, lag1VcMag * sinval);

    rap_sincos(lag2VcPhase, &sinval, &cosval);
    covar.lag2Vc.set(lag2VcMag * cosval, lag2VcMag * sinval);

    rap_sincos(lag3VcPhase, &sinval, &cosval);
    covar.lag3Vc.set(lag3VcMag * cosval, lag3VcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Alternating mode dual pol, co-pol receiver only
  
int Moments::_getCovariancesDpAltHvCoOnly(const RadxRay *covRay)
    
{

  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0HcDb = _getField(covRay, Params::LAG0_HC_DB);
  if (fldLag0HcDb == NULL) iret = -1;
  const RadxField *fldLag0VcDb = _getField(covRay, Params::LAG0_VC_DB);
  if (fldLag0VcDb == NULL) iret = -1;

  const RadxField *fldLag1VcHcDb = _getField(covRay, Params::LAG1_VCHC_DB);
  if (fldLag1VcHcDb == NULL) iret = -1;
  const RadxField *fldLag1VcHcPhase = _getField(covRay, Params::LAG1_VCHC_PHASE);
  if (fldLag1VcHcPhase == NULL) iret = -1;

  const RadxField *fldLag1HcVcDb = _getField(covRay, Params::LAG1_HCVC_DB);
  if (fldLag1HcVcDb == NULL) iret = -1;
  const RadxField *fldLag1HcVcPhase = _getField(covRay, Params::LAG1_HCVC_PHASE);
  if (fldLag1HcVcPhase == NULL) iret = -1;

  const RadxField *fldLag2HcDb = _getField(covRay, Params::LAG2_HC_DB);
  if (fldLag2HcDb == NULL) iret = -1;
  const RadxField *fldLag2HcPhase = _getField(covRay, Params::LAG2_HC_PHASE);
  if (fldLag2HcPhase == NULL) iret = -1;

  const RadxField *fldLag2VcDb = _getField(covRay, Params::LAG2_VC_DB);
  if (fldLag2VcDb == NULL) iret = -1;
  const RadxField *fldLag2VcPhase = _getField(covRay, Params::LAG2_VC_PHASE);
  if (fldLag2VcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_getCovariancesDpAltHvCoOnly" << endl;
    return -1;
  }

  Radx::fl32 *lag0HcDb = (Radx::fl32 *) fldLag0HcDb->getData();
  Radx::fl32 *lag0VcDb = (Radx::fl32 *) fldLag0VcDb->getData();

  Radx::fl32 *lag1VcHcDb = (Radx::fl32 *) fldLag1VcHcDb->getData();
  Radx::fl32 *lag1VcHcPhaseDeg = (Radx::fl32 *) fldLag1VcHcPhase->getData();

  Radx::fl32 *lag1HcVcDb = (Radx::fl32 *) fldLag1HcVcDb->getData();
  Radx::fl32 *lag1HcVcPhaseDeg = (Radx::fl32 *) fldLag1HcVcPhase->getData();

  Radx::fl32 *lag2HcDb = (Radx::fl32 *) fldLag2HcDb->getData();
  Radx::fl32 *lag2HcPhaseDeg = (Radx::fl32 *) fldLag2HcPhase->getData();

  Radx::fl32 *lag2VcDb = (Radx::fl32 *) fldLag2VcDb->getData();
  Radx::fl32 *lag2VcPhaseDeg = (Radx::fl32 *) fldLag2VcPhase->getData();

  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];

    covar.lag0Hc = pow(10.0, lag0HcDb[igate] / 10.0);
    covar.lag0Vc = pow(10.0, lag0VcDb[igate] / 10.0);

    double lag1VcHcMag = pow(10.0, lag1VcHcDb[igate] / 20.0);
    double lag1VcHcPhase = lag1VcHcPhaseDeg[igate] * DEG_TO_RAD;
  
    double lag1HcVcMag = pow(10.0, lag1HcVcDb[igate] / 20.0);
    double lag1HcVcPhase = lag1HcVcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2HcMag = pow(10.0, lag2HcDb[igate] / 20.0);
    double lag2HcPhase = lag2HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2VcMag = pow(10.0, lag2VcDb[igate] / 20.0);
    double lag2VcPhase = lag2VcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;

    rap_sincos(lag1VcHcPhase, &sinval, &cosval);
    covar.lag1VcHc.set(lag1VcHcMag * cosval, lag1VcHcMag * sinval);
    
    rap_sincos(lag1HcVcPhase, &sinval, &cosval);
    covar.lag1HcVc.set(lag1HcVcMag * cosval, lag1HcVcMag * sinval);
    
    rap_sincos(lag2HcPhase, &sinval, &cosval);
    covar.lag2Hc.set(lag2HcMag * cosval, lag2HcMag * sinval);

    rap_sincos(lag2VcPhase, &sinval, &cosval);
    covar.lag2Vc.set(lag2VcMag * cosval, lag2VcMag * sinval);
    
  } // igate
  
  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Alternating mode dual pol, co- and cross-pol receivers

int Moments::_getCovariancesDpAltHvCoCross(const RadxRay *covRay)

{

  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0HcDb = _getField(covRay, Params::LAG0_HC_DB);
  if (fldLag0HcDb == NULL) iret = -1;

  const RadxField *fldLag0HxDb = _getField(covRay, Params::LAG0_HX_DB);
  if (fldLag0HxDb == NULL) iret = -1;

  const RadxField *fldLag0VcDb = _getField(covRay, Params::LAG0_VC_DB);
  if (fldLag0VcDb == NULL) iret = -1;

  const RadxField *fldLag0VxDb = _getField(covRay, Params::LAG0_VX_DB);
  if (fldLag0VxDb == NULL) iret = -1;

  const RadxField *fldLag0VcHxDb = _getField(covRay, Params::LAG0_VCHX_DB);
  if (fldLag0VcHxDb == NULL) iret = -1;

  const RadxField *fldLag0VcHxPhase = _getField(covRay, Params::LAG0_VCHX_PHASE);
  if (fldLag0VcHxPhase == NULL) iret = -1;
  
  const RadxField *fldLag0HcVxDb = _getField(covRay, Params::LAG0_HCVX_DB);
  if (fldLag0HcVxDb == NULL) iret = -1;

  const RadxField *fldLag0HcVxPhase = _getField(covRay, Params::LAG0_HCVX_PHASE);
  if (fldLag0HcVxPhase == NULL) iret = -1;

  const RadxField *fldLag1VxHxDb = _getField(covRay, Params::LAG0_VXHX_DB, false);
  const RadxField *fldLag1VxHxPhase = _getField(covRay, Params::LAG0_VXHX_PHASE, false);

  const RadxField *fldLag1VcHcDb = _getField(covRay, Params::LAG1_VCHC_DB);
  if (fldLag1VcHcDb == NULL) iret = -1;

  const RadxField *fldLag1VcHcPhase = _getField(covRay, Params::LAG1_VCHC_PHASE);
  if (fldLag1VcHcPhase == NULL) iret = -1;

  const RadxField *fldLag1HcVcDb = _getField(covRay, Params::LAG1_HCVC_DB);
  if (fldLag1HcVcDb == NULL) iret = -1;

  const RadxField *fldLag1HcVcPhase = _getField(covRay, Params::LAG1_HCVC_PHASE);
  if (fldLag1HcVcPhase == NULL) iret = -1;

  const RadxField *fldLag2HcDb = _getField(covRay, Params::LAG2_HC_DB);
  if (fldLag2HcDb == NULL) iret = -1;

  const RadxField *fldLag2HcPhase = _getField(covRay, Params::LAG2_HC_PHASE);
  if (fldLag2HcPhase == NULL) iret = -1;

  const RadxField *fldLag2VcDb = _getField(covRay, Params::LAG2_VC_DB);
  if (fldLag2VcDb == NULL) iret = -1;

  const RadxField *fldLag2VcPhase = _getField(covRay, Params::LAG2_VC_PHASE);
  if (fldLag2VcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_getCovariancesDpAltHvCoCross" << endl;
    return -1;
  }

  Radx::fl32 *lag0HcDb = (Radx::fl32 *) fldLag0HcDb->getData();
  Radx::fl32 *lag0HxDb = (Radx::fl32 *) fldLag0HxDb->getData();
  Radx::fl32 *lag0VcDb = (Radx::fl32 *) fldLag0VcDb->getData();
  Radx::fl32 *lag0VxDb = (Radx::fl32 *) fldLag0VxDb->getData();

  Radx::fl32 *lag0VcHxDb = (Radx::fl32 *) fldLag0VcHxDb->getData();
  Radx::fl32 *lag0VcHxPhaseDeg = (Radx::fl32 *) fldLag0VcHxPhase->getData();

  Radx::fl32 *lag0HcVxDb = (Radx::fl32 *) fldLag0HcVxDb->getData();
  Radx::fl32 *lag0HcVxPhaseDeg = (Radx::fl32 *) fldLag0HcVxPhase->getData();

  Radx::fl32 *lag1VxHxDb = NULL;
  if (fldLag1VxHxDb != NULL) {
    lag1VxHxDb = (Radx::fl32 *) fldLag1VxHxDb->getData();
  }

  Radx::fl32 *lag1VxHxPhaseDeg = NULL;
  if (fldLag1VxHxPhase != NULL) {
    lag1VxHxPhaseDeg = (Radx::fl32 *) fldLag1VxHxPhase->getData();
  }

  Radx::fl32 *lag1VcHcDb = (Radx::fl32 *) fldLag1VcHcDb->getData();
  Radx::fl32 *lag1VcHcPhaseDeg = (Radx::fl32 *) fldLag1VcHcPhase->getData();

  Radx::fl32 *lag1HcVcDb = (Radx::fl32 *) fldLag1HcVcDb->getData();
  Radx::fl32 *lag1HcVcPhaseDeg = (Radx::fl32 *) fldLag1HcVcPhase->getData();

  Radx::fl32 *lag2HcDb = (Radx::fl32 *) fldLag2HcDb->getData();
  Radx::fl32 *lag2HcPhaseDeg = (Radx::fl32 *) fldLag2HcPhase->getData();

  Radx::fl32 *lag2VcDb = (Radx::fl32 *) fldLag2VcDb->getData();
  Radx::fl32 *lag2VcPhaseDeg = (Radx::fl32 *) fldLag2VcPhase->getData();

  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];

    covar.lag0Hc = pow(10.0, lag0HcDb[igate] / 10.0);
    covar.lag0Hx = pow(10.0, lag0HxDb[igate] / 10.0);
    covar.lag0Vc = pow(10.0, lag0VcDb[igate] / 10.0);
    covar.lag0Vx = pow(10.0, lag0VxDb[igate] / 10.0);

    double lag0VcHxMag = pow(10.0, lag0VcHxDb[igate] / 20.0);
    double lag0VcHxPhase = lag0VcHxPhaseDeg[igate] * DEG_TO_RAD;

    double lag0HcVxMag = pow(10.0, lag0HcVxDb[igate] / 20.0);
    double lag0HcVxPhase = lag0HcVxPhaseDeg[igate] * DEG_TO_RAD;

    double lag1VxHxMag = 0.0;
    if (lag1VxHxDb != NULL) {
      lag1VxHxMag = pow(10.0, lag1VxHxDb[igate] / 20.0);
    }
    double lag1VxHxPhase = 0.0;
    if (lag1VxHxPhaseDeg != NULL) {
      lag1VxHxPhase = lag1VxHxPhaseDeg[igate] * DEG_TO_RAD;
    }

    double lag1VcHcMag = pow(10.0, lag1VcHcDb[igate] / 20.0);
    double lag1VcHcPhase = lag1VcHcPhaseDeg[igate] * DEG_TO_RAD;

    double lag1HcVcMag = pow(10.0, lag1HcVcDb[igate] / 20.0);
    double lag1HcVcPhase = lag1HcVcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2HcMag = pow(10.0, lag2HcDb[igate] / 20.0);
    double lag2HcPhase = lag2HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2VcMag = pow(10.0, lag2VcDb[igate] / 20.0);
    double lag2VcPhase = lag2VcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;

    rap_sincos(lag0VcHxPhase, &sinval, &cosval);
    covar.lag0VcHx.set(lag0VcHxMag * cosval, lag0VcHxMag * sinval);
    
    rap_sincos(lag0HcVxPhase, &sinval, &cosval);
    covar.lag0HcVx.set(lag0HcVxMag * cosval, lag0HcVxMag * sinval);
    
    rap_sincos(lag1VxHxPhase, &sinval, &cosval);
    covar.lag1VxHx.set(lag1VxHxMag * cosval, lag1VxHxMag * sinval);
    
    rap_sincos(lag1VcHcPhase, &sinval, &cosval);
    covar.lag1VcHc.set(lag1VcHcMag * cosval, lag1VcHcMag * sinval);
    
    rap_sincos(lag1HcVcPhase, &sinval, &cosval);
    covar.lag1HcVc.set(lag1HcVcMag * cosval, lag1HcVcMag * sinval);
    
    rap_sincos(lag2HcPhase, &sinval, &cosval);
    covar.lag2Hc.set(lag2HcMag * cosval, lag2HcMag * sinval);

    rap_sincos(lag2VcPhase, &sinval, &cosval);
    covar.lag2Vc.set(lag2VcMag * cosval, lag2VcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Simultaneous mode HV dual pol

int Moments::_getCovariancesDpSimHv(const RadxRay *covRay)

{

  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0HcDb = _getField(covRay, Params::LAG0_HC_DB);
  if (fldLag0HcDb == NULL) iret = -1;
  const RadxField *fldLag0VcDb = _getField(covRay, Params::LAG0_VC_DB);
  if (fldLag0VcDb == NULL) iret = -1;

  const RadxField *fldRvvhh0Db = _getField(covRay, Params::RVVHH0_DB);
  if (fldRvvhh0Db == NULL) iret = -1;
  const RadxField *fldRvvhh0Phase = _getField(covRay, Params::RVVHH0_PHASE);
  if (fldRvvhh0Phase == NULL) iret = -1;

  const RadxField *fldLag1HcDb = _getField(covRay, Params::LAG1_HC_DB);
  if (fldLag1HcDb == NULL) iret = -1;
  const RadxField *fldLag1HcPhase = _getField(covRay, Params::LAG1_HC_PHASE);
  if (fldLag1HcPhase == NULL) iret = -1;

  const RadxField *fldLag1VcDb = _getField(covRay, Params::LAG1_VC_DB);
  if (fldLag1VcDb == NULL) iret = -1;
  const RadxField *fldLag1VcPhase = _getField(covRay, Params::LAG1_VC_PHASE);
  if (fldLag1VcPhase == NULL) iret = -1;

  const RadxField *fldLag2HcDb = _getField(covRay, Params::LAG2_HC_DB);
  if (fldLag2HcDb == NULL) iret = -1;
  const RadxField *fldLag2HcPhase = _getField(covRay, Params::LAG2_HC_PHASE);
  if (fldLag2HcPhase == NULL) iret = -1;

  const RadxField *fldLag2VcDb = _getField(covRay, Params::LAG2_VC_DB);
  if (fldLag2VcDb == NULL) iret = -1;
  const RadxField *fldLag2VcPhase = _getField(covRay, Params::LAG2_VC_PHASE);
  if (fldLag2VcPhase == NULL) iret = -1;

  const RadxField *fldLag3HcDb = _getField(covRay, Params::LAG3_HC_DB);
  if (fldLag3HcDb == NULL) iret = -1;
  const RadxField *fldLag3HcPhase = _getField(covRay, Params::LAG3_HC_PHASE);
  if (fldLag3HcPhase == NULL) iret = -1;

  const RadxField *fldLag3VcDb = _getField(covRay, Params::LAG3_VC_DB);
  if (fldLag3VcDb == NULL) iret = -1;
  const RadxField *fldLag3VcPhase = _getField(covRay, Params::LAG3_VC_PHASE);
  if (fldLag3VcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_getCovariancesDpSimHv" << endl;
    return -1;
  }

  Radx::fl32 *lag0HcDb = (Radx::fl32 *) fldLag0HcDb->getData();
  Radx::fl32 *lag0VcDb = (Radx::fl32 *) fldLag0VcDb->getData();

  Radx::fl32 *rvvhh0Db = (Radx::fl32 *) fldRvvhh0Db->getData();
  Radx::fl32 *rvvhh0PhaseDeg = (Radx::fl32 *) fldRvvhh0Phase->getData();

  Radx::fl32 *lag1HcDb = (Radx::fl32 *) fldLag1HcDb->getData();
  Radx::fl32 *lag1HcPhaseDeg = (Radx::fl32 *) fldLag1HcPhase->getData();

  Radx::fl32 *lag1VcDb = (Radx::fl32 *) fldLag1VcDb->getData();
  Radx::fl32 *lag1VcPhaseDeg = (Radx::fl32 *) fldLag1VcPhase->getData();

  Radx::fl32 *lag2HcDb = (Radx::fl32 *) fldLag2HcDb->getData();
  Radx::fl32 *lag2HcPhaseDeg = (Radx::fl32 *) fldLag2HcPhase->getData();

  Radx::fl32 *lag2VcDb = (Radx::fl32 *) fldLag2VcDb->getData();
  Radx::fl32 *lag2VcPhaseDeg = (Radx::fl32 *) fldLag2VcPhase->getData();

  Radx::fl32 *lag3HcDb = (Radx::fl32 *) fldLag3HcDb->getData();
  Radx::fl32 *lag3HcPhaseDeg = (Radx::fl32 *) fldLag3HcPhase->getData();

  Radx::fl32 *lag3VcDb = (Radx::fl32 *) fldLag3VcDb->getData();
  Radx::fl32 *lag3VcPhaseDeg = (Radx::fl32 *) fldLag3VcPhase->getData();

  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];

    covar.lag0Hc = pow(10.0, lag0HcDb[igate] / 10.0);
    covar.lag0Vc = pow(10.0, lag0VcDb[igate] / 10.0);

    double rvvhh0Mag = pow(10.0, rvvhh0Db[igate] / 20.0);
    double rvvhh0Phase = rvvhh0PhaseDeg[igate] * DEG_TO_RAD;

    double lag1HcMag = pow(10.0, lag1HcDb[igate] / 20.0);
    double lag1HcPhase = lag1HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag1VcMag = pow(10.0, lag1VcDb[igate] / 20.0);
    double lag1VcPhase = lag1VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2HcMag = pow(10.0, lag2HcDb[igate] / 20.0);
    double lag2HcPhase = lag2HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2VcMag = pow(10.0, lag2VcDb[igate] / 20.0);
    double lag2VcPhase = lag2VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3HcMag = pow(10.0, lag3HcDb[igate] / 20.0);
    double lag3HcPhase = lag3HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3VcMag = pow(10.0, lag3VcDb[igate] / 20.0);
    double lag3VcPhase = lag3VcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;
    
    rap_sincos(rvvhh0Phase, &sinval, &cosval);
    covar.rvvhh0.set(rvvhh0Mag * cosval, rvvhh0Mag * sinval);
    
    rap_sincos(lag1HcPhase, &sinval, &cosval);
    covar.lag1Hc.set(lag1HcMag * cosval, lag1HcMag * sinval);

    rap_sincos(lag1VcPhase, &sinval, &cosval);
    covar.lag1Vc.set(lag1VcMag * cosval, lag1VcMag * sinval);

    rap_sincos(lag2HcPhase, &sinval, &cosval);
    covar.lag2Hc.set(lag2HcMag * cosval, lag2HcMag * sinval);

    rap_sincos(lag2VcPhase, &sinval, &cosval);
    covar.lag2Vc.set(lag2VcMag * cosval, lag2VcMag * sinval);

    rap_sincos(lag3HcPhase, &sinval, &cosval);
    covar.lag3Hc.set(lag3HcMag * cosval, lag3HcMag * sinval);

    rap_sincos(lag3VcPhase, &sinval, &cosval);
    covar.lag3Vc.set(lag3VcMag * cosval, lag3VcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Dual pol, H transmit, dual receive

int Moments::_getCovariancesDpHOnly(const RadxRay *covRay)

{

  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0HcDb = _getField(covRay, Params::LAG0_HC_DB);
  if (fldLag0HcDb == NULL) iret = -1;

  const RadxField *fldLag0VxDb = _getField(covRay, Params::LAG0_VX_DB);
  if (fldLag0VxDb == NULL) iret = -1;

  const RadxField *fldLag1HcDb = _getField(covRay, Params::LAG1_HC_DB);
  if (fldLag1HcDb == NULL) iret = -1;
  const RadxField *fldLag1HcPhase = _getField(covRay, Params::LAG1_HC_PHASE);
  if (fldLag1HcPhase == NULL) iret = -1;

  const RadxField *fldLag2HcDb = _getField(covRay, Params::LAG2_HC_DB);
  if (fldLag2HcDb == NULL) iret = -1;
  const RadxField *fldLag2HcPhase = _getField(covRay, Params::LAG2_HC_PHASE);
  if (fldLag2HcPhase == NULL) iret = -1;

  const RadxField *fldLag3HcDb = _getField(covRay, Params::LAG3_HC_DB);
  if (fldLag3HcDb == NULL) iret = -1;
  const RadxField *fldLag3HcPhase = _getField(covRay, Params::LAG3_HC_PHASE);
  if (fldLag3HcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_computeDpHOnly" << endl;
    return -1;
  }

  Radx::fl32 *lag0HcDb = (Radx::fl32 *) fldLag0HcDb->getData();
  Radx::fl32 *lag0VxDb = (Radx::fl32 *) fldLag0VxDb->getData();

  Radx::fl32 *lag1HcDb = (Radx::fl32 *) fldLag1HcDb->getData();
  Radx::fl32 *lag1HcPhaseDeg = (Radx::fl32 *) fldLag1HcPhase->getData();

  Radx::fl32 *lag2HcDb = (Radx::fl32 *) fldLag2HcDb->getData();
  Radx::fl32 *lag2HcPhaseDeg = (Radx::fl32 *) fldLag2HcPhase->getData();

  Radx::fl32 *lag3HcDb = (Radx::fl32 *) fldLag3HcDb->getData();
  Radx::fl32 *lag3HcPhaseDeg = (Radx::fl32 *) fldLag3HcPhase->getData();
  
  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];

    covar.lag0Hc = pow(10.0, lag0HcDb[igate] / 10.0);
    covar.lag0Vx = pow(10.0, lag0VxDb[igate] / 10.0);

    double lag1HcMag = pow(10.0, lag1HcDb[igate] / 20.0);
    double lag1HcPhase = lag1HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2HcMag = pow(10.0, lag2HcDb[igate] / 20.0);
    double lag2HcPhase = lag2HcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3HcMag = pow(10.0, lag3HcDb[igate] / 20.0);
    double lag3HcPhase = lag3HcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;
    
    rap_sincos(lag1HcPhase, &sinval, &cosval);
    covar.lag1Hc.set(lag1HcMag * cosval, lag1HcMag * sinval);

    rap_sincos(lag2HcPhase, &sinval, &cosval);
    covar.lag2Hc.set(lag2HcMag * cosval, lag2HcMag * sinval);

    rap_sincos(lag3HcPhase, &sinval, &cosval);
    covar.lag3Hc.set(lag3HcMag * cosval, lag3HcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// retrieve the covariances for given covariance ray
// Dual pol, V transmit, dual receive

int Moments::_getCovariancesDpVOnly(const RadxRay *covRay)
  
{

  int iret = 0;

  // identify the fields we need

  const RadxField *fldLag0VcDb = _getField(covRay, Params::LAG0_VC_DB);
  if (fldLag0VcDb == NULL) iret = -1;

  const RadxField *fldLag0HxDb = _getField(covRay, Params::LAG0_HX_DB);
  if (fldLag0HxDb == NULL) iret = -1;

  const RadxField *fldLag1VcDb = _getField(covRay, Params::LAG1_VC_DB);
  if (fldLag1VcDb == NULL) iret = -1;
  const RadxField *fldLag1VcPhase = _getField(covRay, Params::LAG1_VC_PHASE);
  if (fldLag1VcPhase == NULL) iret = -1;

  const RadxField *fldLag2VcDb = _getField(covRay, Params::LAG2_VC_DB);
  if (fldLag2VcDb == NULL) iret = -1;
  const RadxField *fldLag2VcPhase = _getField(covRay, Params::LAG2_VC_PHASE);
  if (fldLag2VcPhase == NULL) iret = -1;

  const RadxField *fldLag3VcDb = _getField(covRay, Params::LAG3_VC_DB);
  if (fldLag3VcDb == NULL) iret = -1;
  const RadxField *fldLag3VcPhase = _getField(covRay, Params::LAG3_VC_PHASE);
  if (fldLag3VcPhase == NULL) iret = -1;

  if (iret) {
    cerr << "ERROR - Moments::_computeDpHOnly" << endl;
    return -1;
  }

  Radx::fl32 *lag0VcDb = (Radx::fl32 *) fldLag0VcDb->getData();
  Radx::fl32 *lag0HxDb = (Radx::fl32 *) fldLag0HxDb->getData();

  Radx::fl32 *lag1VcDb = (Radx::fl32 *) fldLag1VcDb->getData();
  Radx::fl32 *lag1VcPhaseDeg = (Radx::fl32 *) fldLag1VcPhase->getData();

  Radx::fl32 *lag2VcDb = (Radx::fl32 *) fldLag2VcDb->getData();
  Radx::fl32 *lag2VcPhaseDeg = (Radx::fl32 *) fldLag2VcPhase->getData();

  Radx::fl32 *lag3VcDb = (Radx::fl32 *) fldLag3VcDb->getData();
  Radx::fl32 *lag3VcPhaseDeg = (Radx::fl32 *) fldLag3VcPhase->getData();
  
  Covars *covars = _covars.dat();

  for (int igate = 0; igate < _nGates; igate++) {

    Covars &covar = covars[igate];

    covar.lag0Vc = pow(10.0, lag0VcDb[igate] / 10.0);
    covar.lag0Hx = pow(10.0, lag0HxDb[igate] / 10.0);

    double lag1VcMag = pow(10.0, lag1VcDb[igate] / 20.0);
    double lag1VcPhase = lag1VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag2VcMag = pow(10.0, lag2VcDb[igate] / 20.0);
    double lag2VcPhase = lag2VcPhaseDeg[igate] * DEG_TO_RAD;

    double lag3VcMag = pow(10.0, lag3VcDb[igate] / 20.0);
    double lag3VcPhase = lag3VcPhaseDeg[igate] * DEG_TO_RAD;

    double sinval, cosval;
    
    rap_sincos(lag1VcPhase, &sinval, &cosval);
    covar.lag1Vc.set(lag1VcMag * cosval, lag1VcMag * sinval);

    rap_sincos(lag2VcPhase, &sinval, &cosval);
    covar.lag2Vc.set(lag2VcMag * cosval, lag2VcMag * sinval);

    rap_sincos(lag3VcPhase, &sinval, &cosval);
    covar.lag3Vc.set(lag3VcMag * cosval, lag3VcMag * sinval);

  } // igate

  return 0;

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Single pol - H channel

void Moments::_computeMomSinglePolH()
  
{
  
  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.computeMomSinglePolH(covar.lag0Hc,
                               covar.lag1Hc,
                               covar.lag2Hc,
                               covar.lag3Hc,
                               igate,
                               mfields[igate]);
    
  } // igate

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Single pol - V channel

void Moments::_computeMomSinglePolV()
  
{
  
  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.computeMomSinglePolV(covar.lag0Vc,
                               covar.lag1Vc,
                               covar.lag2Vc,
                               covar.lag3Vc,
                               igate,
                               mfields[igate]);
    
  } // igate

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Alternating mode dual pol, co-pol receiver only
  
void Moments::_computeMomDpAltHvCoOnly()
    
{

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();

  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];

    _rmom.computeMomDpAltHvCoOnly(covar.lag0Hc,
                                  covar.lag0Vc,
                                  covar.lag1VcHc,
                                  covar.lag1HcVc,
                                  covar.lag2Hc,
                                  covar.lag2Vc,
                                  igate,
                                  mfields[igate]);
    
  } // igate

  // compute vel2 - unfolding vel_hv using the alternating velocity
  
  _altVel.computeVelAlt(_nGates, mfields, _nyquist);

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Alternating mode dual pol, co- and cross-pol receivers

void Moments::_computeMomDpAltHvCoCross()

{

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();

  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    
    _rmom.computeMomDpAltHvCoCross(covar.lag0Hc,
                                   covar.lag0Hx,
                                   covar.lag0Vc, 
                                   covar.lag0Vx,
                                   covar.lag0VcHx, 
                                   covar.lag0HcVx,
                                   covar.lag1VxHx,
                                   covar.lag1VcHc, 
                                   covar.lag1HcVc,
                                   covar.lag2Hc, 
                                   covar.lag2Vc,
                                   igate,
                                   mfields[igate]);

  } // igate

  // compute vel2 - unfolding vel_hv using the alternating velocity
  
  _altVel.computeVelAlt(_nGates, mfields, _nyquist);

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Simultaneous mode HV dual pol

void Moments::_computeMomDpSimHv()

{

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];

    _rmom.computeMomDpSimHv(covar.lag0Hc,
                            covar.lag0Vc, 
                            covar.rvvhh0,
                            covar.lag1Hc, 
                            covar.lag1Vc,
                            covar.lag2Hc, 
                            covar.lag2Vc,
                            covar.lag3Hc, 
                            covar.lag3Vc,
                            igate,
                            mfields[igate]);
    
  } // igate

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Dual pol, H transmit, dual receive

void Moments::_computeMomDpHOnly()

{

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    
    _rmom.computeMomDpHOnly(covar.lag0Hc, 
                            covar.lag0Vx, 
                            covar.lag1Hc, 
                            covar.lag2Hc, 
                            covar.lag3Hc,
                            igate,
                            mfields[igate]);
    
  } // igate

}

//////////////////////////////////////////////////
// compute the moments for given covariance ray
// Dual pol, V transmit, dual receive

void Moments::_computeMomDpVOnly()
  
{

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();

  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];

    _rmom.computeMomDpHOnly(covar.lag0Vc, 
                            covar.lag0Hx, 
                            covar.lag1Vc, 
                            covar.lag2Vc, 
                            covar.lag3Vc, 
                            igate,
                            mfields[igate]);
    
  } // igate

}

//////////////////////////////////////////////////
// compute the noise
// Single pol - H channel

void Moments::_computeNoiseSinglePolH(RadxRay *ray)
  
{
  
  // detect noise at each gate

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.singlePolHNoisePrep(igate, covar.lag0Hc, covar.lag1Hc, mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseSinglePolH(mfields);

  // override noise for moments computations

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _rmom.setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc());
    ray->setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc() -
                                _calib.getReceiverGainDbHc());
  }

  // set the noise-related fields in moments
  
  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Single pol - V channel

void Moments::_computeNoiseSinglePolV(RadxRay *ray)
  
{
  
  // detect noise at each gate

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.singlePolVNoisePrep(igate, covar.lag0Vc, covar.lag1Vc, mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseSinglePolV(mfields);

  // override noise for moments computations

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _rmom.setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc());
    ray->setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc() -
                                _calib.getReceiverGainDbVc());
  }

  // set the noise-related fields in moments
  
  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Alternating mode dual pol, co-pol receiver only
  
void Moments::_computeNoiseDpAltHvCoOnly(RadxRay *ray)
  
{

  // detect noise at each gate

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.dpAltHvCoOnlyNoisePrep(igate,
                                 covar.lag0Hc,
                                 covar.lag0Vc,
                                 covar.lag2Hc,
                                 covar.lag2Vc,
                                 mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseDpAltHvCoOnly(mfields);

  // override noise for moments computations

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _rmom.setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc());
    _rmom.setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc());
    ray->setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc() -
                                _calib.getReceiverGainDbHc());
    ray->setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc() -
                                _calib.getReceiverGainDbVc());
  }

  // set the noise-related fields in moments

  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Alternating mode dual pol, co/cross receivers
  
void Moments::_computeNoiseDpAltHvCoCross(RadxRay *ray)
  
{

  // prepare for noise computations
  
  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.dpAltHvCoCrossNoisePrep(igate,
                                  covar.lag0Hc,
                                  covar.lag0Hx,
                                  covar.lag0Vc,
                                  covar.lag0Vx,
                                  covar.lag2Hc,
                                  covar.lag2Vc,
                                  mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseDpAltHvCoCross(mfields);

  // override noise for moments computations

  if (_params.use_estimated_noise_for_noise_subtraction) {

    _rmom.setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc());
    _rmom.setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc());

    ray->setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc() -
                                _calib.getReceiverGainDbHc());
    ray->setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc() -
                                _calib.getReceiverGainDbVc());

    // for Hx and Vx, adjust by the mean difference seen in
    // the co-polar channels
    
    double noiseBiasDbHc = _noise.getNoiseBiasDbHc();
    double noiseBiasDbVc = _noise.getNoiseBiasDbVc();
    double adjustedNoiseDbmHx = _calib.getNoiseDbmHx() + noiseBiasDbHc;
    double adjustedNoiseDbmVx = _calib.getNoiseDbmVx() + noiseBiasDbVc;
    
    _rmom.setEstimatedNoiseDbmHx(adjustedNoiseDbmHx);
    _rmom.setEstimatedNoiseDbmVx(adjustedNoiseDbmVx);

    ray->setEstimatedNoiseDbmHx(adjustedNoiseDbmHx -
                                _calib.getReceiverGainDbHx());
    ray->setEstimatedNoiseDbmVx(adjustedNoiseDbmVx -
                                _calib.getReceiverGainDbVx());

  }

  // set the noise-related fields in moments

  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Sim HV mode
  
void Moments::_computeNoiseDpSimHv(RadxRay *ray)
  
{

  // prepare for noise computations

  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.dpSimHvNoisePrep(igate,
                           covar.lag0Hc,
                           covar.lag0Vc,
                           covar.lag1Hc,
                           covar.lag1Vc,
                           mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseDpSimHv(mfields);

  // override noise for moments computations

  if (_params.use_estimated_noise_for_noise_subtraction) {
    _rmom.setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc());
    _rmom.setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc());
    ray->setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc() -
                                _calib.getReceiverGainDbHc());
    ray->setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc() -
                                _calib.getReceiverGainDbVc());
  }

  // set the noise-related fields in moments

  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Dual pol, H-transmit only
  
void Moments::_computeNoiseDpHOnly(RadxRay *ray)
  
{

  // prepare for noise computations
  
  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.dpHOnlyNoisePrep(igate,
                           covar.lag0Hc,
                           covar.lag0Vx,
                           covar.lag1Hc,
                           mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseDpHOnly(mfields);
  
  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {

    _rmom.setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc());
    ray->setEstimatedNoiseDbmHc(_noise.getMedianNoiseDbmHc() -
                                _calib.getReceiverGainDbHc());

    // for Vx, adjust by the difference seen in the co-polar channel

    double noiseBiasDbHc = _noise.getNoiseBiasDbHc();
    double adjustedNoiseDbmVx = _calib.getNoiseDbmVx() + noiseBiasDbHc;
    _rmom.setEstimatedNoiseDbmVx(adjustedNoiseDbmVx);
    ray->setEstimatedNoiseDbmVx(adjustedNoiseDbmVx -
                                _calib.getReceiverGainDbVx());

  }
  
  // set the noise-related fields in moments

  _setNoiseFields();

}

//////////////////////////////////////////////////
// compute the noise
// Dual pol, V-transmit only
  
void Moments::_computeNoiseDpVOnly(RadxRay *ray)
  
{

  // prepare for noise computations
  
  Covars *covars = _covars.dat();
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    Covars &covar = covars[igate];
    _rmom.dpVOnlyNoisePrep(igate,
                           covar.lag0Vc,
                           covar.lag0Hx,
                           covar.lag1Vc,
                           mfields[igate]);
  }

  // identify noise regions, and compute the mea noise
  // mean noise values are stored in moments

  _noise.computeNoiseDpVOnly(mfields);

  // override noise for moments computations
  
  if (_params.use_estimated_noise_for_noise_subtraction) {

    _rmom.setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc());
    ray->setEstimatedNoiseDbmVc(_noise.getMedianNoiseDbmVc() -
                                _calib.getReceiverGainDbVc());

    // for Hx, adjust by the difference seen in the co-polar channel

    double noiseBiasDbVc = _noise.getNoiseBiasDbVc();
    double adjustedNoiseDbmHx = _calib.getNoiseDbmHx() + noiseBiasDbVc;
    _rmom.setEstimatedNoiseDbmHx(adjustedNoiseDbmHx);
    ray->setEstimatedNoiseDbmHx(adjustedNoiseDbmHx -
                                _calib.getReceiverGainDbHx());

  }
  
  // set the noise-related fields in moments

  _setNoiseFields();

}

///////////////////////////////
// get a field based on the ID
 
const RadxField *Moments::_getField(const RadxRay *covRay,
                                    int fieldId,
                                    bool required /* = true */)

{
  
  string fieldName;
  for (int ivar = 0; ivar < _params.input_covars_n; ivar++) {
    if (_params._input_covars[ivar].field_id == fieldId) {
      fieldName = _params._input_covars[ivar].field_name;
      break;
    }
  }
  
  if (fieldName.size() == 0) {
    if (required) {
      pthread_mutex_lock(&_debugPrintMutex);
      cerr << "ERROR - Moments::_getField" << endl;
      cerr << "  Unspecified field ID: " << fieldId << endl;
      cerr << "                  name: " << _fieldId2Str(fieldId) << endl;
      cerr << "  This must be added to the input_covars array in the param file" << endl;
      pthread_mutex_unlock(&_debugPrintMutex);
    }
    return NULL;
  }
  
  const RadxField *field = covRay->getField(fieldName);
  if (field == NULL) {
    if (required) {
      pthread_mutex_lock(&_debugPrintMutex);
      cerr << "ERROR - Moments::_getField" << endl;
      cerr << "  Cannot find field in ray: " << fieldName<< endl;
      cerr << "  El, az: "
           << covRay->getElevationDeg() << ", "
           << covRay->getAzimuthDeg() << endl;
      cerr << "  N fields in ray: " << covRay->getFields().size() << endl;
      pthread_mutex_unlock(&_debugPrintMutex);
    }
    return NULL;
  }

  return field;
  
}

///////////////////////////////////////
// get string associated with field ID

string Moments::_fieldId2Str(int fieldId)

{

  switch (fieldId) {
    case Params::LAG0_HC_DB: return "LAG0_HC_DB";
    case Params::LAG0_HX_DB: return "LAG0_HX_DB";
    case Params::LAG0_VC_DB: return "LAG0_VC_DB";
    case Params::LAG0_VX_DB: return "LAG0_VX_DB";
    case Params::LAG0_HCVX_DB: return "LAG0_HCVX_DB";
    case Params::LAG0_HCVX_PHASE: return "LAG0_HCVX_PHASE";
    case Params::LAG0_VCHX_DB: return "LAG0_VCHX_DB";
    case Params::LAG0_VCHX_PHASE: return "LAG0_VCHX_PHASE";
    case Params::LAG1_HC_DB: return "LAG1_HC_DB";
    case Params::LAG1_HC_PHASE: return "LAG1_HC_PHASE";
    case Params::LAG1_VC_DB: return "LAG1_VC_DB";
    case Params::LAG1_VC_PHASE: return "LAG1_VC_PHASE";
    case Params::LAG1_HCVC_DB: return "LAG1_HCVC_DB";
    case Params::LAG1_HCVC_PHASE: return "LAG1_HCVC_PHASE";
    case Params::LAG1_VCHC_DB: return "LAG1_VCHC_DB";
    case Params::LAG1_VCHC_PHASE: return "LAG1_VCHC_PHASE";
    case Params::LAG1_VXHX_DB: return "LAG1_VXHX_DB";
    case Params::LAG1_VXHX_PHASE: return "LAG1_VXHX_PHASE";
    case Params::LAG2_HC_DB: return "LAG2_HC_DB";
    case Params::LAG2_HC_PHASE: return "LAG2_HC_PHASE";
    case Params::LAG2_VC_DB: return "LAG2_VC_DB";
    case Params::LAG2_VC_PHASE: return "LAG2_VC_PHASE";
    case Params::LAG3_HC_DB: return "LAG3_HC_DB";
    case Params::LAG3_HC_PHASE: return "LAG3_HC_PHASE";
    case Params::LAG3_VC_DB: return "LAG3_VC_DB";
    case Params::LAG3_VC_PHASE: return "LAG3_VC_PHASE";
    case Params::RVVHH0_DB: return "RVVHH0_DB";
    case Params::RVVHH0_PHASE: return "RVVHH0_PHASE";
    default: return "UNKNOWN";
  }

}
      
///////////////////////////////
// censor gates based on noise

void Moments::_censorByNoiseFlag()

{

  // load up censor flag from noise flag

  MomentsFields *mfields = _momFields.dat();
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (mfields[igate].noise_flag) {
      _censorFlag[igate] = true;
    } else {
      _censorFlag[igate] = false;
    }
  }

  // fill in gaps in censor flag

  _despeckleCensoring();

  // censor the fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlag[igate]) {
      _censorFields(igate);
    }
  }

}

////////////////////////////////////////////////////////////////////
// censor based on snr and ncp thresholds

void Moments::_censorBySnrNcp()

{

  // load up censor flag from snr and ncp

  MomentsFields *mfields = _momFields.dat();

  double snrThreshold = _params.censoring_snr_threshold;
  double ncpThreshold = _params.censoring_ncp_threshold;

  for (int igate = 0; igate < _nGates; igate++) {
    if (mfields[igate].snr < snrThreshold &&
        mfields[igate].ncp < ncpThreshold) {
      _censorFlag[igate] = true;
    } else {
      _censorFlag[igate] = false;
    }
  }

  // fill in gaps in censor flag

  _despeckleCensoring();

  // censor the fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlag[igate]) {
      _censorFields(igate);
    }
  }

}

///////////////////////////////////////////////////////////
// censor specified fields in a ray

void Moments::_censorFields(int gateNum)

{

  MomentsFields &mfield = _momFields.dat()[gateNum];
  mfield.init();

}

/////////////////////////////////////////////////////////////
// apply despeckling filter to censor flag

void Moments::_despeckleCensoring()
  
{
  
  int *countSet = new int[_nGates];
  int *countNot = new int[_nGates];

  // first remove small gaps from censory array

  int nSet = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlag[igate]) {
      nSet++;
    } else {
      nSet = 0;
    }
    countSet[igate] = nSet;
  }

  for (int igate = _nGates - 2; igate >= 0; igate--) {
    if (countSet[igate] != 0 &&
        countSet[igate] < countSet[igate+1]) {
      countSet[igate] = countSet[igate+1];
    }
  }

  for (int igate = 1; igate < _nGates - 1; igate++) {
    // is the gap small enough?
    nSet = countSet[igate];
    if (nSet > 0 && nSet <= 2) {
      _censorFlag[igate] = false;
    }
  }

  // compute the running count of gates which have the flag set and
  // those which do not

  // Go forward through the gates, counting up the number of gates set
  // or not set and assigning that number to the arrays as we go.

  nSet = 0;
  int nNot = 0;
  for (int igate = 0; igate < _nGates; igate++) {
    if (_censorFlag[igate]) {
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
    if (nNot > 0 && nNot <= 3) {

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
        _censorFlag[igate] = true;
      }
    }
  } // igate

  delete[] countSet;
  delete[] countNot;

}

///////////////////////////////
// load up fields in output ray

void Moments::_loadOutputFields(RadxRay *momRay)

{

  // initialize array pointers

  const double *dbzForKdp = _kdp.getDbz();
  const double *zdrForKdp = _kdp.getZdr();
  const double *rhohvForKdp = _kdp.getRhohv();
  const double *snrForKdp = _kdp.getSnr();
 
  // load up output data

  double phidpFoldLimitLower = -180.0;
  double phidpFoldLimitUpper = 180.0;
  if (_params.xmit_rcv_mode == Params::DUAL_POL_ALT_HV_CO_ONLY ||
      _params.xmit_rcv_mode == Params::DUAL_POL_ALT_HV_CO_CROSS) {
    phidpFoldLimitLower = -90.0;
    phidpFoldLimitUpper = 90.0;
  }

  for (int ifield = 0; ifield < _params.output_fields_n; ifield++) {

    const Params::output_field_t &ofld = _params._output_fields[ifield];

    bool fieldFolds = false;
    double foldLimitLower = 0.0;
    double foldLimitUpper = 0.0;
    bool isDiscrete = false;

    // fill data array
    
    TaArray<Radx::fl32> data_;
    Radx::fl32 *data = data_.alloc(momRay->getNGates());
    Radx::fl32 *datp = data;
    
    MomentsFields *fields = _momFields.dat();
    for (int igate = 0; igate < (int) momRay->getNGates(); 
         igate++, fields++, datp++) {
      
      switch (ofld.id) {

        // computed fields

        case Params::SNR:
          *datp = fields->snr;
          break;
        case Params::DBM:
          *datp = fields->dbm;
          break;
        case Params::DBZ:
          *datp = fields->dbz;
          break;
        case Params::DBZHC:
          *datp = fields->dbzhc;
          break;
        case Params::DBZVC:
          *datp = fields->dbzvc;
          break;
        case Params::DBZHX:
          *datp = fields->dbzhx;
          break;
        case Params::DBZVX:
          *datp = fields->dbzvx;
          break;
        case Params::DBZ_NO_ATMOS_ATTEN:
          *datp = fields->dbz_no_atmos_atten;
          break;
        case Params::VEL:
          *datp = fields->vel;
          fieldFolds = true;
          foldLimitLower = -_nyquist;
          foldLimitUpper = _nyquist;
          break;
        case Params::VEL_ALT:
          *datp = fields->vel_alt;
          fieldFolds = true;
          foldLimitLower = -_nyquist;
          foldLimitUpper = _nyquist;
          break;
        case Params::VEL_HV:
          *datp = fields->vel_hv;
          fieldFolds = true;
          foldLimitLower = -_nyquist / 2.0;
          foldLimitUpper = _nyquist / 2.0;
          break;
        case Params::VEL_ALT_FOLD_INTERVAL:
          *datp = fields->vel_alt_fold_interval;
          isDiscrete = true;
          break;
        case Params::VEL_ALT_FOLD_CONFIDENCE:
          *datp = fields->vel_alt_fold_confidence;
          break;
        case Params::VEL_DIFF:
          *datp = fields->vel_diff;
          break;
        case Params::VEL_UNFOLD_INTERVAL:
          *datp = fields->vel_unfold_interval;
          isDiscrete = true;
          break;
        case Params::VEL_H_ONLY:
          *datp = fields->vel_h_only;
          fieldFolds = true;
          foldLimitLower = -_nyquist / 2.0;
          foldLimitUpper = _nyquist / 2.0;
          break;
        case Params::VEL_V_ONLY:
          *datp = fields->vel_v_only;
          fieldFolds = true;
          foldLimitLower = -_nyquist / 2.0;
          foldLimitUpper = _nyquist / 2.0;
          break;
        case Params::WIDTH:
          *datp = fields->width;
          break;
        case Params::WIDTH_H_ONLY:
          *datp = fields->width_h_only;
          break;
        case Params::WIDTH_V_ONLY:
          *datp = fields->width_v_only;
          break;
        case Params::NCP:
          *datp = fields->ncp;
          break;
        case Params::NCP_H_ONLY:
          *datp = fields->ncp_h_only;
          break;
        case Params::NCP_V_ONLY:
          *datp = fields->ncp_v_only;
          break;
        case Params::NCP_H_MINUS_V:
          *datp = fields->ncp_h_minus_v;
          break;
        case Params::ZDRM:
          *datp = fields->zdrm;
          break;
        case Params::ZDR:
          *datp = fields->zdr;
          break;
        case Params::ZDR_BIAS:
          *datp = fields->zdr_bias;
          break;
        case Params::LDRHM:
          *datp = fields->ldrhm;
          break;
        case Params::LDRH:
          *datp = fields->ldrh;
          break;
        case Params::LDRVM:
          *datp = fields->ldrvm;
          break;
        case Params::LDRV:
          *datp = fields->ldrv;
          break;
        case Params::LDR_DIFF:
          *datp = fields->ldr_diff;
          break;
        case Params::LDR_MEAN:
          *datp = fields->ldr_mean;
          break;
        case Params::RHOHV:
          *datp = fields->rhohv;
          break;
        case Params::RHOHV_NNC:
          *datp = fields->rhohv_nnc;
          break;
        case Params::RHO_HCVX:
          *datp = fields->rho_hcvx;
          break;
        case Params::RHO_VCHX:
          *datp = fields->rho_vchx;
          break;
        case Params::RHO_VXHX:
          *datp = fields->rho_vxhx;
          break;
        case Params::PHIDP0:
          *datp = fields->phidp0;
          fieldFolds = true;
          foldLimitLower = phidpFoldLimitLower;
          foldLimitUpper = phidpFoldLimitUpper;
          break;
        case Params::PHIDP:
          *datp = fields->phidp;
          fieldFolds = true;
          foldLimitLower = phidpFoldLimitLower;
          foldLimitUpper = phidpFoldLimitUpper;
          break;
        case Params::PHIDP_COND:
          *datp = fields->phidp_cond;
          fieldFolds = true;
          foldLimitLower = phidpFoldLimitLower;
          foldLimitUpper = phidpFoldLimitUpper;
          break;
        case Params::PHIDP_FILT:
          *datp = fields->phidp_filt;
          fieldFolds = true;
          foldLimitLower = phidpFoldLimitLower;
          foldLimitUpper = phidpFoldLimitUpper;
          break;
        case Params::KDP:
          *datp = fields->kdp;
          break;
        case Params::PSOB:
          *datp = fields->psob;
          break;
        case Params::SNRHC:
          *datp = fields->snrhc;
          break;
        case Params::SNRHX:
          *datp = fields->snrhx;
          break;
        case Params::SNRVC:
          *datp = fields->snrvc;
          break;
        case Params::SNRVX:
          *datp = fields->snrvx;
          break;
        case Params::DBMHC:
          *datp = fields->dbmhc;
          break;
        case Params::DBMHX:
          *datp = fields->dbmhx;
          break;
        case Params::DBMVC:
          *datp = fields->dbmvc;
          break;
        case Params::DBMVX:
          *datp = fields->dbmvx;
          break;

          // noise

        case Params::PHASE_FOR_NOISE:
          *datp = RadarComplex::argDeg(fields->phase_for_noise);
          fieldFolds = true;
          foldLimitLower = -180.0;
          foldLimitUpper = 180.0;
          break;
        case Params::ACCUM_PHASE_CHANGE:
          *datp = fmod(fields->accum_phase_change, 180.0);
          break;
        case Params::PHASE_CHANGE_ERROR:
          *datp = fields->phase_change_error;
          break;
        case Params::DBM_SDEV:
          *datp = fields->dbm_sdev;
          break;
        case Params::NCP_MEAN:
          *datp = fields->ncp_mean;
          break;
        case Params::NOISE_FLAG:
          *datp = fields->noise_flag;
          isDiscrete = true;
          break;
        case Params::SIGNAL_FLAG:
          *datp = fields->signal_flag;
          isDiscrete = true;
          break;
        case Params::NOISE_BIAS_DB_HC:
          *datp = fields->noise_bias_db_hc;
          break;
        case Params::NOISE_BIAS_DB_HX:
          *datp = fields->noise_bias_db_hx;
          break;
        case Params::NOISE_BIAS_DB_VC:
          *datp = fields->noise_bias_db_vc;
          break;
        case Params::NOISE_BIAS_DB_VX:
          *datp = fields->noise_bias_db_vx;
          break;

          // test fields

        case Params::TEST:
          *datp = fields->test;
          break;
        case Params::TEST2:
          *datp = fields->test2;
          break;
        case Params::TEST3:
          *datp = fields->test3;
          break;
        case Params::TEST4:
          *datp = fields->test4;
          break;
        case Params::TEST5:
          *datp = fields->test5;
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
        case Params::PHIDP_SDEV_FOR_KDP:
          *datp = fields->phidp_sdev_4kdp;
          break;
        case Params::PHIDP_JITTER_FOR_KDP:
          *datp = fields->phidp_jitter_4kdp;
          break;
        case Params::ZDR_SDEV_FOR_KDP:
          *datp = fields->zdr_sdev_4kdp;
          break;

          // attenuation correction
          
        case Params::DBZ_ATTEN_CORRECTION:
          *datp = fields->dbz_atten_correction;
          break;
        case Params::ZDR_ATTEN_CORRECTION:
          *datp = fields->zdr_atten_correction;
          break;

        case Params::DBZ_ATTEN_CORRECTED:
          *datp = fields->dbz_atten_corrected;
          break;
        case Params::ZDR_ATTEN_CORRECTED:
          *datp = fields->zdr_atten_corrected;
          break;

      } // switch

    } // igate

    // create field
    
    RadxField *field = new RadxField(ofld.name, ofld.units);
    field->setLongName(ofld.long_name);
    field->setStandardName(ofld.standard_name);
    field->setTypeFl32(MomentsFields::missingDouble);
    field->addDataFl32(momRay->getNGates(), data);
    if (fieldFolds) {
      field->setFieldFolds(foldLimitLower, foldLimitUpper);
    }
    if (isDiscrete) {
      field->setIsDiscrete(true);
    }

    // add to ray

    momRay->addField(field);

  } // ifield

  // create and add censoring field

  if (_params.censoring_mode != Params::CENSORING_NONE) {
    TaArray<Radx::si08> cdata_;
    Radx::si08 *cdata = cdata_.alloc(momRay->getNGates());
    for (size_t ii = 0; ii < momRay->getNGates(); ii++) {
      if (_censorFlag[ii]) {
        cdata[ii] = 1;
      } else {
        cdata[ii] = 0;
      }
    }
    RadxField *cfield = new RadxField(censorFlagFieldName, "");
    cfield->setLongName("flag_to_indicate_censoring_has_been_applied");
    cfield->setStandardName("censoring_flag");
    cfield->setTypeSi08(-128, 1, 0);
    cfield->addDataSi08(momRay->getNGates(), cdata);
    cfield->setIsDiscrete(true);
    momRay->addField(cfield);
  }

}

///////////////////////////////
// set the noise fields

void Moments::_setNoiseFields()
  
{

  MomentsFields *mfields = _momFields.dat();
  const vector<bool> &noiseFlag = _noise.getNoiseFlag();
  const vector<bool> &signalFlag = _noise.getSignalFlag();
  const vector<double> &accumPhaseChange = _noise.getAccumPhaseChange();
  const vector<double> &phaseChangeError = _noise.getPhaseChangeError();
  const vector<double> &dbmSdev = _noise.getDbmSdev();
  const vector<double> &ncpMean = _noise.getNcpMean();
  
  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_flag = noiseFlag[igate];
    mfield.signal_flag = signalFlag[igate];
    mfield.accum_phase_change = accumPhaseChange[igate];
    mfield.phase_change_error = phaseChangeError[igate];
    mfield.dbm_sdev = dbmSdev[igate];
    mfield.ncp_mean = ncpMean[igate];
  }
  
}

////////////////////////////////////////////////////////////////////////
// Convert interest map points to vector
//
// Returns 0 on success, -1 on failure

int Moments::_convertInterestParamsToVector(const string &label,
                                            const Params::interest_map_point_t *map,
                                            int nPoints,
                                            vector<InterestMap::ImPoint> &pts)

{
  
  pts.clear();
  
  double prevVal = -1.0e99;
  for (int ii = 0; ii < nPoints; ii++) {
    if (map[ii].value <= prevVal) {
      pthread_mutex_lock(&_debugPrintMutex);
      cerr << "ERROR - Moments::_convertInterestParamsToVector" << endl;
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

//////////////////////////////////////
// initialize moments
  
void Moments::_momInit()
  
{
  
  // initialize moments object
  
  _rmom.setDebug(_params.debug >= Params::DEBUG_VERBOSE);
  _rmom.setVerbose(_params.debug >= Params::DEBUG_EXTRA);

  if (_params.correct_for_system_phidp) {
    _rmom.setCorrectForSystemPhidp(true);
  }
  if (_params.change_velocity_sign) {
    _rmom.setChangeVelocitySign(true);
  }
  _rmom.setMinSnrDbForZdr(_params.min_snr_db_for_zdr);
  _rmom.setMinSnrDbForLdr(_params.min_snr_db_for_ldr);
  // if (_params.adjust_dbz_for_measured_xmit_power) {
  //   _rmom.setAdjustDbzForMeasXmitPower(true);
  // }
  // if (_params.adjust_zdr_for_measured_xmit_power) {
  //   _rmom.setAdjustZdrForMeasXmitPower(true);
  // }

}

//////////////////////////////////////
// initialize noise computations
  
int Moments::_noiseInit()
  
{

  // initialize noise location
  
  _noise.setNGatesKernel(_params.noise_ngates_kernel);

  if (_params.noise_method == Params::NOISE_RUNNING_MEDIAN) {
    _noise.setComputeRunningMedian(_params.noise_ngates_for_running_median);
  } else {
    _noise.setComputeRayMedian(_params.noise_min_ngates_for_ray_median);
  }

  // interest maps for for noise
  
  vector<InterestMap::ImPoint> pts;
  if (_convertInterestParamsToVector
      ("phase_change_error_for_noise",
       _params._phase_change_error_for_noise_interest_map,
       _params.phase_change_error_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapPhaseChangeErrorForNoise
    (pts, _params.phase_change_error_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("dbm_sdev_for_noise",
       _params._dbm_sdev_for_noise_interest_map,
       _params.dbm_sdev_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapDbmSdevForNoise
    (pts, _params.dbm_sdev_for_noise_interest_weight);

  if (_convertInterestParamsToVector
      ("ncp_mean_sdev_for_noise",
       _params._ncp_mean_for_noise_interest_map,
       _params.ncp_mean_for_noise_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapNcpMeanForNoise
    (pts, _params.ncp_mean_for_noise_interest_weight);

  _noise.setInterestThresholdForNoise
    (_params.interest_threshold_for_noise);

  // interest maps for for signal
  
  if (_convertInterestParamsToVector
      ("phase_change_error_for_signal",
       _params._phase_change_error_for_signal_interest_map,
       _params.phase_change_error_for_signal_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapPhaseChangeErrorForSignal
    (pts, _params.phase_change_error_for_signal_interest_weight);

  if (_convertInterestParamsToVector
      ("dbm_sdev_for_signal",
       _params._dbm_sdev_for_signal_interest_map,
       _params.dbm_sdev_for_signal_interest_map_n,
       pts)) {
    return -1;
  }
  _noise.setInterestMapDbmSdevForSignal
    (pts, _params.dbm_sdev_for_signal_interest_weight);

  _noise.setInterestThresholdForSignal
    (_params.interest_threshold_for_signal);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    _noise.printParams(cerr);
  }

  return 0;

}

//////////////////////////////////////
// initialize KDP
  
void Moments::_kdpInit()
  
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
  _kdp.setNFiltIterUnfolded(_params.KDP_n_filt_iterations_unfolded);
  _kdp.setNFiltIterCond(_params.KDP_n_filt_iterations_conditioned);
  if (_params.KDP_use_iterative_filtering) {
    _kdp.setUseIterativeFiltering(true);
    _kdp.setPhidpDiffThreshold(_params.KDP_phidp_difference_threshold);
  }
  _kdp.setPhidpSdevMax(_params.KDP_phidp_sdev_max);
  _kdp.setPhidpJitterMax(_params.KDP_phidp_jitter_max);
  _kdp.checkSnr(_params.KDP_check_snr);
  _kdp.setSnrThreshold(_params.KDP_snr_threshold);
  _kdp.checkRhohv(_params.KDP_check_rhohv);
  _kdp.setRhohvThreshold(_params.KDP_rhohv_threshold);
  if (_params.KDP_check_zdr_sdev) {
    _kdp.checkZdrSdev(true);
  }
  _kdp.setZdrSdevMax(_params.KDP_zdr_sdev_max);
  _kdp.setMinValidAbsKdp(_params.KDP_min_valid_abs_kdp);

  // if (_params.set_max_range) {
  //   _kdp.setMaxRangeKm(true, _params.max_range_km);
  // }

  if (_params.KDP_debug) {
    _kdp.setDebug(true);
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

void Moments::_kdpCompute()
  
{

  // set up array for range
  
  TaArray<double> rangeKm_;
  double *rangeKm = rangeKm_.alloc(_nGates);
  double range = _startRangeKm;
  for (int ii = 0; ii < _nGates; ii++, range += _gateSpacingKm) {
    rangeKm[ii] = range;
  }

  // compute KDP
  
  _kdp.compute(_timeSecs, _nanoSecs,
               _elevation, _azimuth,
               _wavelengthM * 100.0,
               _nGates, _startRangeKm, _gateSpacingKm,
               _snrArray, _dbzArray, _zdrArray,
               _rhohvArray, _phidpArray,
               MomentsFields::missingDouble);
  
  const double *kdp = _kdp.getKdp();
  const double *psob = _kdp.getPsob();
  const double *phidpCond = _kdp.getPhidpCondFilt();
  const double *phidpFilt = _kdp.getPhidpFilt();
  const double *phidpSdev = _kdp.getPhidpSdev();
  const double *phidpJitter = _kdp.getPhidpJitter();
  const double *zdrSdev = _kdp.getZdrSdev();
  const double *dbzAtten = _kdp.getDbzAttenCorr();
  const double *zdrAtten = _kdp.getZdrAttenCorr();
  
  // put KDP into fields objects
  
  MomentsFields *mfields = _momFields.dat();
  for (int ii = 0; ii < _nGates; ii++) {
    MomentsFields &mfield = mfields[ii];
    if (kdp[ii] != NAN) {
      mfield.kdp = kdp[ii];
      _kdpArray[ii] = kdp[ii];
    }
    mfield.phidp_cond = phidpCond[ii];
    mfield.phidp_filt = phidpFilt[ii];
    mfield.phidp_sdev_4kdp = phidpSdev[ii];
    mfield.phidp_jitter_4kdp = phidpJitter[ii];
    mfield.zdr_sdev_4kdp = zdrSdev[ii];
    if (mfield.signal_flag) {
      mfield.psob = psob[ii];
    }
    mfield.dbz_atten_correction = dbzAtten[ii];
    mfield.zdr_atten_correction = zdrAtten[ii];
    if (_dbzArray[ii] != MomentsFields::missingDouble &&
        dbzAtten[ii] != MomentsFields::missingDouble) {
      mfield.dbz_atten_corrected = _dbzArray[ii] + dbzAtten[ii];
    }
    if (_zdrArray[ii] != MomentsFields::missingDouble &&
        zdrAtten[ii] != MomentsFields::missingDouble) {
      mfield.zdr_atten_corrected = _zdrArray[ii] + zdrAtten[ii];
    }
  }
  
}

//////////////////////////////////////
// alloc moments arrays
  
void Moments::_allocMomentsArrays()
  
{

  _snrArray = _snrArray_.alloc(_nGates);
  _dbzArray = _dbzArray_.alloc(_nGates);
  _zdrArray = _zdrArray_.alloc(_nGates);
  _kdpArray = _kdpArray_.alloc(_nGates);
  _ldrArray = _ldrArray_.alloc(_nGates);
  _rhohvArray = _rhohvArray_.alloc(_nGates);
  _phidpArray = _phidpArray_.alloc(_nGates);

}

/////////////////////////////////////////////////////
// load momemts arrays ready for KDP, PID and PRECIP
  
void Moments::_loadMomentsArrays()
  
{
  
  MomentsFields *mfields = _momFields.dat();
  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    _snrArray[igate] = mfield.snrhc;
    _dbzArray[igate] = mfield.dbz;
    _zdrArray[igate] = mfield.zdr;
    _ldrArray[igate] = mfield.ldrh;
    _rhohvArray[igate] = mfield.rhohv;
    _phidpArray[igate] = mfield.phidp;
  }
  
}

