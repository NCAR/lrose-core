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
////////////////////////////////////////////////////////////////////////
// PrecipRate.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////////////
//
// Compute rain rate
//
///////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <radar/PrecipRate.hh>
#include <radar/FilterUtils.hh>
#include <radar/DpolFilter.hh>
#include <toolsa/TaArray.hh>

using namespace std;

// Constructor

PrecipRate::PrecipRate()

{

  // initialize coefficients to NCAR defaults - Florida (CAPE)

  _zh_aa = 0.0262;
  _zh_bb = 0.687;

  _zh_aa_snow = 0.0365; // marshall palmer
  _zh_bb_snow = 0.625;

  _zzdr_aa = 0.00786;
  _zzdr_bb = 0.967;
  _zzdr_cc = -4.98;

  _kdp_aa = 54.3;
  _kdp_bb = 0.806;

  _kdpzdr_aa = 136;
  _kdpzdr_bb = 0.968;
  _kdpzdr_cc = -2.86;

  _pid_kdp_threshold = 0.3;

  _hybrid_dbz_threshold = 38.0;
  _hybrid_kdp_threshold = 0.3;
  _hybrid_zdr_threshold = 0.5;

  _hidro_dbz_threshold = 38.0;
  _hidro_kdp_threshold = 0.3;
  _hidro_zdr_threshold = 0.5;

  _bringi_dbz_threshold = 38.0;
  _bringi_kdp_threshold = 0.3;
  _bringi_zdr_threshold = 0.5;

  _brightBandDbzCorrection = -10.0;

  // initialize min/max valid precip rate (mm/hr)

  _min_valid_rate = 0.1;
  _max_valid_rate = 250.0;
  _max_valid_dbz = 53.0;

  // constraints

  _wavelengthCm = 10.0;

  // signal/noise threshold (dB)

  _snrThreshold = 3.0;

}

/////////////////////////////////////////////////////////
// destructor

PrecipRate::~PrecipRate()

{

}

////////////////////////////////////////////
// Set processing options from params object

void PrecipRate::setFromParams(const PrecipRateParams &params)
{
  
  _params = params;

  setSnrThresholdDb(_params.RATE_snr_threshold);

  // initialize precip rate object

  setZhAa(_params.RATE_zh_aa);
  setZhBb(_params.RATE_zh_bb);

  setZhAaSnow(_params.RATE_zh_aa_snow);
  setZhBbSnow(_params.RATE_zh_bb_snow);

  setZzdrAa(_params.RATE_zzdr_aa);
  setZzdrBb(_params.RATE_zzdr_bb);
  setZzdrCc(_params.RATE_zzdr_cc);
  
  setKdpAa(_params.RATE_kdp_aa);
  setKdpBb(_params.RATE_kdp_bb);

  setKdpZdrAa(_params.RATE_kdpzdr_aa);
  setKdpZdrBb(_params.RATE_kdpzdr_bb);
  setKdpZdrCc(_params.RATE_kdpzdr_cc);
  
  setPidKdpThreshold(_params.RATE_pid_rate_kdp_threshold);

  setHybridDbzThreshold(_params.RATE_hybrid_dbz_threshold);
  setHybridKdpThreshold(_params.RATE_hybrid_kdp_threshold);
  setHybridZdrThreshold(_params.RATE_hybrid_zdr_threshold);

  setHidroDbzThreshold(_params.RATE_hidro_dbz_threshold);
  setHidroKdpThreshold(_params.RATE_hidro_kdp_threshold);
  setHidroZdrThreshold(_params.RATE_hidro_zdr_threshold);

  setBringiDbzThreshold(_params.RATE_bringi_dbz_threshold);
  setBringiKdpThreshold(_params.RATE_bringi_kdp_threshold);
  setBringiZdrThreshold(_params.RATE_bringi_zdr_threshold);

  setMinValidRate(_params.RATE_min_valid_rate);
  setMaxValidRate(_params.RATE_max_valid_rate);
  setMaxValidDbz(_params.RATE_max_valid_dbz);
  setBrightBandDbzCorrection(_params.RATE_brightband_dbz_correction);

  if (_params.RATE_apply_median_filter_to_DBZ) {
    setApplyMedianFilterToDbz(_params.RATE_DBZ_median_filter_len);
  }
  if (_params.RATE_apply_median_filter_to_ZDR) {
    setApplyMedianFilterToZdr(_params.RATE_ZDR_median_filter_len);
  }

}

///////////////////////
// compute precip rate
//
// cflag: censor flag
// dbz, zdr, kdp: input
// rateZ, rateKdp, rateKdpZdr, rateZZdr: simple rates, output
// rateHybrid: rate based on simple rates
// rateHidro: CSU HIDRO algorithm
// rateBringi: BRINGI algorithm

void PrecipRate::computePrecipRates
  (int nGates,
   const double *snr,
   const double *dbz,
   const double *zdr,
   const double *kdp,
   double missingVal,
   const NcarParticleId *pid /* = NULL */)
  
{

  // set state

  _missingVal = missingVal;
  _nGates = nGates;

  // allocate local arrays

  _initArrays();

  // copy input data to local arrays
  
  memcpy(_snr, snr, _nGates * sizeof(double));
  memcpy(_dbz, dbz, _nGates * sizeof(double));
  memcpy(_zdr, zdr, _nGates * sizeof(double));
  memcpy(_kdp, kdp, _nGates * sizeof(double));
  
  // limit dbz values

  for (int ii = 0; ii < _nGates; ii++) {
    if (_dbz[ii] > _max_valid_dbz) {
      _dbz[ii] = _max_valid_dbz;
    }
  }

  // censor

  for (int ii = 0; ii < _nGates; ii++) {
    double sn = snr[ii];
    if (sn == _missingVal || sn < _snrThreshold) {
      _cflags[ii] = true;
    } else {
      _cflags[ii] = false;
    }
  }

  // median filter as appropriate

  if (_applyMedianFilterToDbz) {
    FilterUtils::applyMedianFilter(_dbz, _nGates, _dbzMedianFilterLen);
  }
  
  if (_applyMedianFilterToZdr) {
    FilterUtils::applyMedianFilter(_zdr, _nGates, _zdrMedianFilterLen);
  }
  
  const NcarParticleId::category_t *category = NULL;
  if (pid != NULL) {
    category = pid->getCategory();
  }
  
  for (int ii = 0; ii < _nGates; ii++) {
    
    if (_cflags[ii]) {
      // censor
      _dbz[ii] = _missingVal;
      _zdr[ii] = _missingVal;
      _kdp[ii] = _missingVal;
      _rateZ[ii] = _missingVal;
      _rateZSnow[ii] = _missingVal;
      _rateZMixed[ii] = _missingVal;
      _rateKdp[ii] = _missingVal;
      _rateKdpZdr[ii] = _missingVal;
      _rateZZdr[ii] = _missingVal;
      _rateHybrid[ii] = _missingVal;
      _ratePid[ii] = _missingVal;
      _rateHidro[ii] = _missingVal;
      _rateBringi[ii] = _missingVal;
      continue;
    }

    NcarParticleId::category_t thisCategory = NcarParticleId::CATEGORY_RAIN;
    if (category) {
      thisCategory = category[ii];
    }

    _computeRates(_dbz[ii], 
                  _zdr[ii],
                  _kdp[ii],
                  thisCategory,
		  _rateZ[ii],
		  _rateZSnow[ii],
		  _rateZMixed[ii],
                  _rateKdp[ii],
                  _rateKdpZdr[ii],
                  _rateZZdr[ii],
		  _rateHybrid[ii],
                  _rateHidro[ii],
                  _rateBringi[ii]);
    
    if (_rateZ[ii] <= 0.0) _rateZ[ii] = _missingVal;
    if (_rateZSnow[ii] <= 0.0) _rateZSnow[ii] = _missingVal;
    if (_rateZMixed[ii] <= 0.0) _rateZMixed[ii] = _missingVal;
    if (_rateKdp[ii] <= 0.0) _rateKdp[ii] = _missingVal;
    if (_rateKdpZdr[ii] <= 0.0) _rateKdpZdr[ii] = _missingVal;
    if (_rateZZdr[ii] <= 0.0) _rateZZdr[ii] = _missingVal;
    if (_rateHybrid[ii] <= 0.0) _rateHybrid[ii] = _missingVal;
    if (_rateHidro[ii] <= 0.0) _rateHidro[ii] = _missingVal;
    if (_rateBringi[ii] <= 0.0) _rateBringi[ii] = _missingVal;

  }

  if (pid != NULL) {
    _computeHybridRates(pid);
    _computePidFuzzyRate(pid);
  }

}
  
////////////////////////////////////////////////////////////////
// compute rain rate

void PrecipRate::_computeRates(double dbz, 
                               double zdrdb,
                               double kdp,
                               NcarParticleId::category_t category,
			       double &rateZh,
			       double &rateZhSnow,
			       double &rateZhMixed,
                               double &rateKdp,
			       double &rateKdpZdr,
                               double &rateZZdr,
			       double &rateHybrid,
                               double &rateHidro,
                               double &rateBringi)
  
{

  // NOTE: we use the field indices to determine whether data is available

  double zh = pow(10.0, dbz / 10.0);
  double zhMixed = pow(10.0, (dbz + _brightBandDbzCorrection) / 10.0);
  double zdr = pow(10.0, zdrdb / 10.0);

  // NOTE: following are for linear parameters
  //   zh in mm6m-3
  //   kdp deg/km
  //   zdr unitless ratio

  // rate from zh

  if (zh != _missingVal) {

    rateZh = _zh_aa * pow(zh, _zh_bb);
    if (rateZh < _min_valid_rate) {
      rateZh = _missingVal;
    } else if (rateZh > _max_valid_rate) {
      rateZh = _max_valid_rate;
    }
    rateZhSnow = _zh_aa_snow * pow(zh, _zh_bb_snow);
    if (rateZhSnow < _min_valid_rate) {
      rateZhSnow = _missingVal;
    } else if (rateZhSnow > _max_valid_rate) {
      rateZhSnow = _max_valid_rate;
    }
    rateZhMixed = _zh_aa * pow(zhMixed, _zh_bb);
    if (rateZhMixed < _min_valid_rate) {
      rateZhMixed = _missingVal;
    } else if (rateZhMixed > _max_valid_rate) {
      rateZhMixed = _max_valid_rate;
    }
  }

  // rate from zh and zdr

  if (zh != _missingVal && zdr != _missingVal) {
    
    if (zdrdb < 0.1) {
      rateZZdr = rateZh;
    } else {
      rateZZdr =
	_zzdr_aa * pow(zh, _zzdr_bb) * pow(zdr, _zzdr_cc);
    }
    if (rateZZdr < _min_valid_rate) {
      rateZZdr = _missingVal;
    } else if (rateZZdr > _max_valid_rate) {
      rateZZdr = _max_valid_rate;
    }
    
  }

  // rate from kdp

  if (kdp != _missingVal) {

    double signKdp = 1.0;
    if (kdp < 0) {
      signKdp = -1.0;
    }
    rateKdp = signKdp * _kdp_aa * pow(fabs(kdp), _kdp_bb);
    if (rateKdp < _min_valid_rate) {
      rateKdp = _missingVal;
    } else if (rateKdp > _max_valid_rate) {
      rateKdp = _max_valid_rate;
    }
    
    if (zdr != _missingVal) {
      if (zdrdb < 0.1) {
        rateZZdr = rateZh;
      } else {
        rateKdpZdr = (signKdp * _kdpzdr_aa *
                      pow(fabs(kdp), _kdpzdr_bb) *
                      pow(zdr, _kdpzdr_cc));
      }
      if (rateKdpZdr < _min_valid_rate) {
        rateKdpZdr = _missingVal;
      } else if (rateKdpZdr > _max_valid_rate) {
        rateKdpZdr = _max_valid_rate;
      }
    } // if (zdr != _missingVal)
      
  } // if (kdp != _missingVal)

}

////////////////////////////////////////////////////////////////
// allocate local arrays

void PrecipRate::_allocArrays()

{  
  _snr = _snr_.alloc(_nGates);
  _cflags = _cflags_.alloc(_nGates);
  _dbz = _dbz_.alloc(_nGates);
  _zdr = _zdr_.alloc(_nGates);
  _kdp = _kdp_.alloc(_nGates);
  _rateZ = _rateZ_.alloc(_nGates);
  _rateZSnow = _rateZSnow_.alloc(_nGates);
  _rateZMixed = _rateZMixed_.alloc(_nGates);
  _rateKdp = _rateKdp_.alloc(_nGates);
  _rateKdpZdr = _rateKdpZdr_.alloc(_nGates);
  _rateZZdr = _rateZZdr_.alloc(_nGates);
  _rateHybrid = _rateHybrid_.alloc(_nGates);
  _ratePid = _ratePid_.alloc(_nGates);
  _rateHidro = _rateHidro_.alloc(_nGates);
  _rateBringi = _rateBringi_.alloc(_nGates);
}

////////////////////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// computePrecipRates(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void PrecipRate::_initArrays()
  
{
  _allocArrays();
  for (int ii = 0; ii < _nGates; ii++) {
    _snr[ii] = _missingVal;
    _cflags[ii] = false;
    _dbz[ii] = _missingVal;
    _zdr[ii] = _missingVal;
    _kdp[ii] = _missingVal;
    _rateZ[ii] = _missingVal;
    _rateZSnow[ii] = _missingVal;
    _rateZMixed[ii] = _missingVal;
    _rateKdp[ii] = _missingVal;
    _rateKdpZdr[ii] = _missingVal;
    _rateZZdr[ii] = _missingVal;
    _rateHybrid[ii] = _missingVal;
    _ratePid[ii] = _missingVal;
    _rateHidro[ii] = _missingVal;
    _rateBringi[ii] = _missingVal;
  }
}

////////////////////////////////////////////////////////////
// compute hybrid rates
//
// This must be called AFTER computePrecipRates().

void PrecipRate::_computeHybridRates(const NcarParticleId *pid)
  
{

  const int *PID = pid->getPid();

  for (int igate = 0; igate < _nGates; igate++) {

    double rateZ = _rateZ[igate];
    double rateZSnow = _rateZSnow[igate];
    double rateZMixed = _rateZMixed[igate];
    double rateZZdr = _rateZZdr[igate];
    double rateKdpZdr = _rateKdpZdr[igate];
    double rateKdp = _rateKdp[igate];
    double dbz = _dbz[igate];
    double zdr = _zdr[igate];
    double kdp = _kdp[igate];
    int ptype = PID[igate];
    
    // first check for clutter etc

    if (ptype == NcarParticleId::FLYING_INSECTS ||
        ptype == NcarParticleId::SECOND_TRIP ||
        ptype == NcarParticleId::GROUND_CLUTTER) {
      _rateHybrid[igate] = 0.0;
      _rateHidro[igate] = 0.0;
      _rateBringi[igate] = 0.0;
      continue;
    }

    // NCAR hybrid

    switch (ptype) {
      case NcarParticleId::HAIL:
      case NcarParticleId::RAIN_HAIL_MIXTURE:
      case NcarParticleId::GRAUPEL_SMALL_HAIL: {
      case NcarParticleId::HEAVY_RAIN:
        // hail or rain/hail - use KDP if avail
        if (kdp >= _hybrid_kdp_threshold &&
            rateKdp != _missingVal) {
          _rateHybrid[igate] = rateKdp;
        } else {
          _rateHybrid[igate] = rateZZdr;
        }
        break;
      }
      case NcarParticleId::DRY_SNOW:
      case NcarParticleId::ICE_CRYSTALS:
      case NcarParticleId::IRREG_ICE_CRYSTALS: {
        // ice/snow
        _rateHybrid[igate] = rateZSnow;
        break;
      }
      case NcarParticleId::GRAUPEL_RAIN:
      case NcarParticleId::WET_SNOW: {
        // melting snow - i.e. brightband
        _rateHybrid[igate] = rateZMixed;
        break;
      }
      case NcarParticleId::DRIZZLE:
      case NcarParticleId::LIGHT_RAIN:
      case NcarParticleId::MODERATE_RAIN:
      case NcarParticleId::SUPERCOOLED_DROPS: {
        // default to rain
        if (dbz > _hybrid_dbz_threshold &&
            kdp >= _hybrid_kdp_threshold) {
          // heavy rain - use KDP if available
          if (rateKdp != _missingVal) {
            _rateHybrid[igate] = rateKdp;
          } else {
            _rateHybrid[igate] = rateZZdr;
          }
        } else {
          if (zdr >= _hybrid_zdr_threshold) {
            // moderate rain - use ZZDR
            _rateHybrid[igate] = rateZZdr;
          } else {
            // light rain - use ZH
            _rateHybrid[igate] = rateZ;
          }
        }
        break;
      }
    } // switch
    if (_rateHybrid[igate] == _missingVal && rateZ != _missingVal) {
      _rateHybrid[igate] = rateZ;
    }

    // hidro
    
    switch (ptype) {
      case NcarParticleId::HAIL:
      case NcarParticleId::RAIN_HAIL_MIXTURE:
      case NcarParticleId::GRAUPEL_SMALL_HAIL:
      case NcarParticleId::GRAUPEL_RAIN:
      case NcarParticleId::WET_SNOW: {
        if (kdp >= _hidro_kdp_threshold && rateKdp != _missingVal) {
          _rateHidro[igate] = rateKdp;
        } else {
          _rateHidro[igate] = rateZ;
        }
        break;
      }
      case NcarParticleId::DRIZZLE:
      case NcarParticleId::LIGHT_RAIN:
      case NcarParticleId::MODERATE_RAIN:
      case NcarParticleId::HEAVY_RAIN:
      case NcarParticleId::SUPERCOOLED_DROPS: {
        if (kdp >= _hidro_kdp_threshold &&
            dbz >= _hidro_dbz_threshold) {
          if (zdr >= _hidro_zdr_threshold && rateKdpZdr != _missingVal) {
            _rateHidro[igate] = rateKdpZdr;
          } else if (rateKdp != _missingVal) {
            _rateHidro[igate] = rateKdp;
          } else {
            _rateHidro[igate] = rateZZdr;
          }
        } else {
          if (zdr >= _hidro_zdr_threshold) {
            _rateHidro[igate] = rateZZdr;
          } else {
            _rateHidro[igate] = rateZ;
          }
        }
        break;
      }
      default: {} // do nothing for ice
    }
    
    // bringi
    
    if ((ptype == NcarParticleId::HAIL ||
         ptype == NcarParticleId::RAIN_HAIL_MIXTURE) &&
        rateKdp != _missingVal) {
      _rateBringi[igate] = rateKdp;
    } else if (dbz > _bringi_dbz_threshold &&
               kdp >= _bringi_kdp_threshold &&
               rateKdp != _missingVal) {
      _rateBringi[igate] = rateKdp;
    } else {
      if (zdr >= _bringi_zdr_threshold) {
        _rateBringi[igate] = rateZZdr;
      } else {
        _rateBringi[igate] = rateZ;
      }
    }

  } // igate

}
  
////////////////////////////////////////////////////////////
// compute rate based on PID fuzzy logic
//
// We take the interest associated with each PID type,
// and use that to weight the precip rate associated with
// each type.
//
// This must be called AFTER computePrecipRates().

void PrecipRate::_computePidFuzzyRate(const NcarParticleId *pid)
  
{

  const int *PID = pid->getPid();
  const double *cloud = pid->getParticleCloud()->gateInterest;
  const double *drizzle = pid->getParticleDrizzle()->gateInterest;
  const double *lightRain = pid->getParticleLightRain()->gateInterest;
  const double *modRain = pid->getParticleModerateRain()->gateInterest;
  const double *hvyRain = pid->getParticleHeavyRain()->gateInterest;
  const double *hail = pid->getParticleHail()->gateInterest;
  const double *rainHail = pid->getParticleRainHail()->gateInterest;
  const double *graupelHail = pid->getParticleGraupelSmallHail()->gateInterest;
  const double *graupelRain = pid->getParticleGraupelRain()->gateInterest;
  const double *drySnow = pid->getParticleDrySnow()->gateInterest;
  const double *wetSnow = pid->getParticleWetSnow()->gateInterest;
  const double *ice = pid->getParticleIce()->gateInterest;
  const double *irregIce = pid->getParticleIrregIce()->gateInterest;
  const double *sld = pid->getParticleSuperCooledDrops()->gateInterest;
  // const double *bugs = pid->getParticleBugs()->gateInterest;
  // const double *trip2 = pid->getParticleSecondTrip()->gateInterest;
  // const double *clut = pid->getParticleClutter()->gateInterest;

  for (int igate = 0; igate < _nGates; igate++) {

    // first check for clutter etc

    if (PID[igate] == NcarParticleId::FLYING_INSECTS ||
        PID[igate] == NcarParticleId::SECOND_TRIP ||
        PID[igate] == NcarParticleId::GROUND_CLUTTER) {
      _ratePid[igate] = 0.0;
      continue;
    }

    // compute interest for each precip category
    
    double interestLightRain = 0.0;
    interestLightRain += cloud[igate];
    interestLightRain += drizzle[igate];
    interestLightRain += lightRain[igate];
    interestLightRain += sld[igate];
    
    double interestModRain = 0.0;
    interestModRain += modRain[igate];
    interestModRain += graupelRain[igate];
    
    double interestHvyRain = 0.0;
    interestHvyRain += hvyRain[igate];

    double interestHail = 0.0;
    interestHail += hail[igate];
    interestHail += rainHail[igate];
    interestHail += graupelHail[igate];

    double interestSnow = 0.0;
    interestSnow += drySnow[igate];
    interestSnow += ice[igate];
    interestSnow += irregIce[igate];

    double interestMixed = 0.0;
    interestMixed += wetSnow[igate];
    
    double sumInterest =
      (interestLightRain + interestModRain + interestHvyRain +
       interestHail + interestSnow + interestMixed);
    
    double wtLightRain = interestLightRain / sumInterest;
    double wtModRain = interestModRain / sumInterest;
    double wtHvyRain = interestHvyRain / sumInterest;
    double wtSnow = interestSnow / sumInterest;
    double wtHail = interestHail / sumInterest;
    double wtMixed = interestMixed / sumInterest;
    
    double rateLightRain = _rateZ[igate];
    double rateModRain = _rateZZdr[igate];
    double rateSnow = _rateZSnow[igate];
    double rateMixed = _rateZMixed[igate];

    double rateHail = _rateZZdr[igate];
    double rateHvyRain = _rateZZdr[igate];
    double rateKdp = _rateKdp[igate];
    
    if (_kdp[igate] > _pid_kdp_threshold && rateKdp != _missingVal) {
      rateHail = _rateKdp[igate];
      rateHvyRain = _rateKdp[igate];
    }
    
    double rate = 0.0;
    rate += rateLightRain * wtLightRain;
    rate += rateModRain * wtModRain;
    rate += rateSnow * wtSnow;
    rate += rateMixed * wtMixed;
    rate += rateHail * wtHail;
    rate += rateHvyRain * wtHvyRain;

    if (rate > 0) {
      _ratePid[igate] = rate;
    }

  } // igate

}
  
////////////////////////////////////////////////////////////////
// compute rain rate - reference
//
// Documentation on the formulas in use

double PrecipRate::_computeRainRateRef()

{

  double zh = 1.0;
  double kdp = 1.0;
  double zdr = 1.0;
  double zdrdb = 1.0;
  double r = 1.0;

  // following are for
  //  linear parameters
  // zh in mm6m-3
  // kdp deg/km
  // zdr unitless

  r = 2.62e-2 * pow(zh, 0.687);

  double signKdp = 1.0;
  if (kdp < 0) {
    signKdp = -1.0;
  }

  r = signKdp * 54.3 * pow(fabs(kdp), 0.806);

  r = signKdp * 136 * pow(fabs(kdp), 0.968) * pow(zdr, -2.86);

  r = 7.86e-3 * pow(zh, 0.967) * pow(zdr, -4.98);

  // Alternate form for zh, zdr
  // zh in linear units
  // zdr in dB

  r = 7.60e-3 * zh * pow(10.0, (0.165 * zdrdb * zdrdb - 0.897 * zdrdb));

  return r;

}

