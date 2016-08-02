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
// KdpBringi.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2008
//
///////////////////////////////////////////////////////////////
//
// Kdp for SBand - based on Bringi code
//
////////////////////////////////////////////////////////////////

#include <iomanip>
#include <iostream>
#include <cmath>
#include <cstring>
#include <radar/KdpBringi.hh>
#include <radar/FilterUtils.hh>
#include <radar/DpolFilter.hh>
using namespace std;

const double KdpBringi::firCoeff_125[FIR_LEN_125+1] = {
  2.5107443e-003,2.6960328e-003,2.8834818e-003,3.0729344e-003,
  3.2642298e-003,3.4572038e-003,3.6516884e-003,3.8475124e-003,
  4.0445018e-003,4.2424792e-003,4.4412651e-003,4.6406770e-003,
  4.8405305e-003,5.0406391e-003,5.2408146e-003,5.4408670e-003,
  5.6406054e-003,5.8398373e-003,6.0383699e-003,6.2360093e-003,
  6.4325618e-003,6.6278331e-003,6.8216293e-003,7.0137569e-003,
  7.2040230e-003,7.3922356e-003,7.5782040e-003,7.7617385e-003,
  7.9426516e-003,8.1207572e-003,8.2958715e-003,8.4678134e-003,
  8.6364038e-003,8.8014671e-003,8.9628303e-003,9.1203239e-003,
  9.2737821e-003,9.4230427e-003,9.5679474e-003,9.7083424e-003,
  9.8440780e-003,9.9750093e-003,1.0100996e-002,1.0221904e-002,
  1.0337601e-002,1.0447965e-002,1.0552875e-002,1.0652219e-002,
  1.0745888e-002,1.0833782e-002,1.0915804e-002,1.0991867e-002,
  1.1061886e-002,1.1125785e-002,1.1183495e-002,1.1234953e-002,
  1.1280103e-002,1.1318895e-002,1.1351287e-002,1.1377243e-002,
  1.1396735e-002,1.1409742e-002,1.1416248e-002,1.1416248e-002,
  1.1409742e-002,1.1396735e-002,1.1377243e-002,1.1351287e-002,
  1.1318895e-002,1.1280103e-002,1.1234953e-002,1.1183495e-002,
  1.1125785e-002,1.1061886e-002,1.0991867e-002,1.0915804e-002,
  1.0833782e-002,1.0745888e-002,1.0652219e-002,1.0552875e-002,
  1.0447965e-002,1.0337601e-002,1.0221904e-002,1.0100996e-002,
  9.9750093e-003,9.8440780e-003,9.7083424e-003,9.5679474e-003,
  9.4230427e-003,9.2737821e-003,9.1203239e-003,8.9628303e-003,
  8.8014671e-003,8.6364038e-003,8.4678134e-003,8.2958715e-003,
  8.1207572e-003,7.9426516e-003,7.7617385e-003,7.5782040e-003,
  7.3922356e-003,7.2040230e-003,7.0137569e-003,6.8216293e-003,
  6.6278331e-003,6.4325618e-003,6.2360093e-003,6.0383699e-003,
  5.8398373e-003,5.6406054e-003,5.4408670e-003,5.2408146e-003,
  5.0406391e-003,4.8405305e-003,4.6406770e-003,4.4412651e-003,
  4.2424792e-003,4.0445018e-003,3.8475124e-003,3.6516884e-003,
  3.4572038e-003,3.2642298e-003,3.0729344e-003,2.8834818e-003,
  2.6960328e-003,2.5107443e-003};

const double KdpBringi::firCoeff_60[FIR_LEN_60+1] = {
  0.005192387815,0.006000584633,0.006826878703,
  0.007668199579,0.008521340618,0.009382975237,
  0.01024967409,0.01111792308,0.0119841421,
  0.01284470437,0.01369595621,0.01453423726,
  0.01535590085,0.01615733448,0.01693498027,
  0.01768535525,0.01840507134,0.01909085485,
  0.01973956552,0.02034821473,0.02091398302,
  0.02143423665,0.02190654305,0.02232868523,
  0.0226986749,0.02301476423,0.02327545631,
  0.02347951399,0.02362596732,0.02371411929,
  0.02374355002,0.02371411929,0.02362596732,
  0.02347951399,0.02327545631,0.02301476423,
  0.0226986749,0.02232868523,0.02190654305,
  0.02143423665,0.02091398302,0.02034821473,
  0.01973956552,0.01909085485,0.01840507134,
  0.01768535525,0.01693498027,0.01615733448,
  0.01535590085,0.01453423726,0.01369595621,
  0.01284470437,0.0119841421,0.01111792308,
  0.01024967409,0.009382975237,0.008521340618,
  0.007668199579,0.006826878703,0.006000584633,
  0.005192387815 };

const double KdpBringi::firCoeff_40[FIR_LEN_40+1] = {
  0.007806525986, 0.009628559511, 0.01150585082,
  0.01342243276, 0.01536143961, 0.01730530352,
  0.0192359639, 0.02113508696, 0.02298429218,
  0.02476538263, 0.02646057561, 0.02805273044,
  0.02952556994, 0.03086389233, 0.03205377043,
  0.03308273518, 0.03393994069, 0.03461630839,
  0.03510464806, 0.0353997539, 0.03549847428,
  0.0353997539, 0.03510464806, 0.03461630839,
  0.03393994069, 0.03308273518, 0.03205377043,
  0.03086389233, 0.02952556994, 0.02805273044,
  0.02646057561, 0.02476538263, 0.02298429218,
  0.02113508696, 0.0192359639, 0.01730530352,
  0.01536143961, 0.01342243276, 0.01150585082,
  0.009628559511, 0.007806525986 };

const double KdpBringi::firCoeff_30[FIR_LEN_30+1] = {
  0.01040850049,0.0136551033,0.01701931136,0.0204494327,
  0.0238905658,0.02728575662,0.03057723021,0.03370766631,
  0.03662148602,0.03926611662,0.04159320123,0.04355972181,
  0.04512900539,0.04627158699,0.04696590613,0.04719881804,
  0.04696590613,0.04627158699,0.04512900539,0.04355972181,
  0.04159320123,0.03926611662,0.03662148602,0.03370766631,
  0.03057723021,0.02728575662,0.0238905658,0.0204494327,
  0.01701931136,0.0136551033,0.01040850049};

const double KdpBringi::firCoeff_20[FIR_LEN_20+1] = {
  1.625807356e-2,2.230852545e-2,2.896372364e-2,3.595993808e-2,
  4.298744446e-2,4.971005447e-2,5.578764970e-2,6.089991897e-2,
  6.476934523e-2,6.718151185e-2,6.80010000e-2,6.718151185e-2,
  6.476934523e-2,6.089991897e-2,5.578764970e-2,4.971005447e-2,
  4.298744446e-2,3.595993808e-2,2.896372364e-2,2.230852545e-2,
  1.625807356e-2 };

const double KdpBringi::firCoeff_10[FIR_LEN_10+1] = {
  0.03064579383,0.0603038422,0.09022859603,0.1159074511,
  0.1332367851,0.1393550634,0.1332367851,0.1159074511,
  0.09022859603,0.0603038422,0.03064579383 };

const double KdpBringi::firGain_125 = 1.0;
const double KdpBringi::firGain_60 = 1.0;
const double KdpBringi::firGain_40 = 1.0;
const double KdpBringi::firGain_30 = 1.0;
const double KdpBringi::firGain_20 = 1.044222;
const double KdpBringi::firGain_10 = 1.0;

// Constructor

KdpBringi::KdpBringi()
  
{

  // FIR filter defaults to length 20

  setFIRFilterLen(FIR_LENGTH_20);

  _wavelengthCm = 10.0;

  _applyMedianFilterToPhidp = false;
  _phidpMedianFilterLen = 5;
  _snrThreshold = 0.0;
  _ldrThreshold = 0.0;
  _phidpDiffThreshold = 4.0;
  _phidpSdevThreshold = 15.0;
  _zdrSdevThreshold = 2.0;
  _rhohvWxThreshold = 0.7;

  _limitMaxRange = false;
  _maxRangeKm = 0.0;

  _minValidAbsKdp = 0.01;

}

// Destructor

KdpBringi::~KdpBringi()
  
{

}

/////////////////////////////////////
// set FIR filter length

void KdpBringi::setFIRFilterLen(fir_filter_len_t len)

{
  
  switch (len) {
    case FIR_LENGTH_125:
      _firLength = FIR_LEN_125;
      _firGain = firGain_125;
      _firCoeff = firCoeff_125;
      break;
    case FIR_LENGTH_30:
      _firLength = FIR_LEN_30;
      _firGain = firGain_30;
      _firCoeff = firCoeff_30;
      break;
    case FIR_LENGTH_20:
      _firLength = FIR_LEN_20;
      _firGain = firGain_20;
      _firCoeff = firCoeff_20;
      break;
    case FIR_LENGTH_10:
    default:
      _firLength = FIR_LEN_10;
      _firGain = firGain_10;
      _firCoeff = firCoeff_10;
  }

  _firLenHalf = _firLength / 2;

}

/////////////////////////////////////
// compute KDP

int KdpBringi::compute(double elev,
                       double az,
                       int nGates,
                       const double *range,
                       const double *dbz,
                       const double *zdr,
                       const double *phidp,
                       const double *rhohv,
                       const double *snr,
                       double missingValue,
                       const double *ldr /* = NULL */)
  
{

  _missingValue = missingValue;
  _elev = elev;
  _az = az;

  _nGatesMaxRange = nGates;
  if (_limitMaxRange) {
    for (int igate = 0; igate < nGates; igate++) {
      if (range[igate] > _maxRangeKm) {
        _nGatesMaxRange = igate - 1;
        break;
      }
    }
  }

  // initialize the arrays
  
  _initArrays(nGates, range, dbz, zdr, phidp, rhohv, snr, ldr);

  // compute sdev of phidp and zdr, using unfiltered data

  for (int igate = 0; igate < nGates; igate++) {

    int nhalf = N_GOOD / 2;
    int midIndex = igate;
    int startIndex = midIndex - nhalf;
    if (startIndex < 0) {
      startIndex = 0;
    }
    int endIndex = midIndex + nhalf;
    if (endIndex > nGates - 1) {
      endIndex = nGates - 1;
    }
    int nn = endIndex - startIndex + 1;
    
    double phidp_mean, phidp_sd;
    _msr(phidp_mean, phidp_sd, phidp + startIndex, nn);
    _sdphidp[igate] = phidp_sd;

    double mean_zdr, sd_zdr;
    _msr(mean_zdr, sd_zdr, zdr + startIndex, nn);
    _sdzdr[igate] = sd_zdr;

  }

  // apply median filter to phidp as appropriate
  
  if (_applyMedianFilterToPhidp) {
    FilterUtils::applyMedianFilter(_phidp, nGates, _phidpMedianFilterLen);
  }

  // find the runs with good data

  _findGoodRuns();

  // unfold phidp
  
  _unfoldPhidp(nGates);

  // load conditioned phidp array for phidp

  _loadConditionedPhidp(nGates);

  if(_goodRuns.size() < 1) {
    // NO good data in whole ray. RETURN.
    for (int ii = 0; ii < nGates; ii++) {
      _kdp[ii] = 0;
    }
    return -1;
  }
  
  int kbegin = _runFirst->ibegin;
  int kend = _runLast->iend;

  // initialize working arrays

  memset(_yyy - _arrayExtra, 0, _arrayLen * sizeof(double));

  memcpy(_xxx - _arrayExtra, _phidp - _arrayExtra,
         _arrayLen * sizeof(double));

  memcpy(_zzz - _arrayExtra, _phidpCond - _arrayExtra,
         _arrayLen * sizeof(double));

  //------------- LOOP for Phidp Adaptive Filtering --------------------
  
  for (int mm = 0; mm < _firLength; mm++) {

    // TIE DOWN THE INITIAL and EXTENDING DATA RECORD

    for (int igate = -_firLength; igate < kbegin; igate++) {   // Set the initial conditions
      _zzz[igate] = _runFirst->phidpBegin;
    }
    
    for (int igate = kend + 1; igate < nGates + _firLength; igate++) { // Extend data record
      _zzz[igate] = _runLast->phidpEnd;
    }
    
    // FIR FILTER SECTION

    for (int igate = -_firLenHalf; igate < nGates + _firLenHalf; igate++) {
      
      double acc = 0.0;
      
      int kk = igate - _firLenHalf;
      for (int jj = 0; jj <= _firLength; jj++, kk++) {
	acc = acc + _firCoeff[jj] * _zzz[kk];
      }

      _yyy[igate] = acc * _firGain;

    } // END of FIR FILTERING
    
    for (int igate = 0; igate < nGates; igate++) {
      
      double delt = fabs(_xxx[igate] - _yyy[igate]);
      if (delt >= _phidpDiffThreshold) {
	_zzz[igate] = _yyy[igate];
      } else {
	_zzz[igate] = _xxx[igate];
      }

    }

  } // END LOOP for Phidp Adaptive Filtering
  
  // save filtered phidp, extending into region before and after actual data

  for (int igate = 0; igate < nGates; igate++) {
    _phidpFilt[igate] = _yyy[igate];
  }
  double beginPhidp = _phidpFilt[0];
  for (int igate = -_arrayExtra; igate < 0; igate++) {
    _phidpFilt[igate] = beginPhidp;
  }
  double endPhidp = _phidpFilt[nGates-1];
  for (int igate = nGates; igate < nGates + _arrayExtra; igate++) {
    _phidpFilt[igate] = endPhidp;
  }

  // compute phase shift on backscatter as the difference between
  // measured and filtered phidp

  for (int igate = 0; igate < nGates; igate++) {
    _psob[igate] = _phidp[igate] - _phidpFilt[igate];
  }
  
  // CALCULATE KDP
  
  for (int igate = 0; igate < nGates; igate++) {

    if (!_goodFlag[igate]) {
      continue;
    }

    // Check Dbz range to decide the number of gates over which to
    // compute kdp. The default value for adapLen is 10.
    
    double maxDbz = -999;
    if (igate >= _firLenHalf && igate < nGates - _firLenHalf) {
      double dbz = _dbz[igate];
      if (dbz > maxDbz) {
        maxDbz = dbz;
      }
    }
    int adapLen = 8;
    if (maxDbz < 20.0) {
      adapLen = 16;
    } else if (maxDbz < 35.0) {
      adapLen = 8;
    } else {
      adapLen = 4;
    }

    int startGate = igate - adapLen / 2;
    int endGate = igate + adapLen / 2;

    double drange = _range[endGate] - _range[startGate];
    double dphi = _phidpFilt[endGate] - _phidpFilt[startGate];
    double kdp = (dphi / drange) / 2.0;
    _kdp[igate] = kdp;

#ifdef NOTNOW

    // compute Kdp based on lse fit to Adap flt Phidp

    double xx[32], yy[32];
    int count = 0;
    for (int jj = startGate; jj <= endGate; jj++) {
      xx[count] = _range[jj];
      yy[count] = _phidpFilt[jj];
      count++;
    }
    
    double aa, bb;
    _lse(aa,bb,xx,yy,count);
    
    double kdp2 = aa / 2.0;
    _kdp2[igate] = kdp2;

#endif

  } // igate

  // Remove runs of constant KDP
  
  for (int ii = 0; ii < nGates; ii++) {
    double startKdp = _kdp[ii];
    if (startKdp == missingValue) {
      continue;
    }
    int count = 0;
    for (int jj = ii + 1; jj < nGates; jj++) {
      double kdp = _kdp[jj];
      if (fabs(kdp - startKdp) < 0.00001) {
        count++;
      } else {
        break;
      }
    } // jj
    if (count > 4) {
      for (int jj = ii + 1; jj <= ii + count; jj++) {
        _kdp[jj] = 0;
      } // jj
      ii += count;
    }
  } // ii

  // set KDP and PSOB to 0
  // for small values of KDP, and non-good gates
  
  for (int ii = 0; ii < nGates; ii++) {
    if (!_goodFlag[ii] || fabs(_kdp[ii]) < _minValidAbsKdp) {
      _kdp[ii] = 0;
      _psob[ii] = 0;
    }
  }
    
  return 0;

}
  
/////////////////////////////////////
// initialize arrays

void KdpBringi::_initArrays(int nGates,
                            const double *range,
                            const double *dbz,
                            const double *zdr,
                            const double *phidp,
                            const double *rhohv,
                            const double *snr,
                            const double *ldr)
  
{

  _arrayExtra = _firLength + 1;
  _arrayLen = nGates + 2 * _arrayExtra;

  // allocate the arrays needed
  // copy input arrays, leaving extra space at the beginning
  // for negative indices and at the end for filtering as required

  _range = _range_.alloc(_arrayLen) + _arrayExtra;
  _dbz = _dbz_.alloc(_arrayLen) + _arrayExtra;
  _zdr = _zdr_.alloc(_arrayLen) + _arrayExtra;
  _phidp = _phidp_.alloc(_arrayLen) + _arrayExtra;
  _phidpCond = _phidpCond_.alloc(_arrayLen) + _arrayExtra;
  _phidpFilt = _phidpFilt_.alloc(_arrayLen) + _arrayExtra;
  _psob = _psob_.alloc(_arrayLen) + _arrayExtra;
  _rhohv = _rhohv_.alloc(_arrayLen) + _arrayExtra;
  _snr = _snr_.alloc(_arrayLen) + _arrayExtra;
  _ldr = _ldr_.alloc(_arrayLen) + _arrayExtra;
  _sdphidp = _sdphidp_.alloc(_arrayLen) + _arrayExtra;
  _sdzdr = _sdzdr_.alloc(_arrayLen) + _arrayExtra;
  _goodFlag = _goodFlag_.alloc(_arrayLen) + _arrayExtra;
  _kdp = _kdp_.alloc(_arrayLen) + _arrayExtra;

  _xxx = _xxx_.alloc(_arrayLen) + _arrayExtra;
  _yyy = _yyy_.alloc(_arrayLen) + _arrayExtra;
  _zzz = _zzz_.alloc(_arrayLen) + _arrayExtra;

  // copy data to working arrays

  memset(_range - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_range, range, nGates * sizeof(double));
  
  memset(_dbz - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_dbz, dbz, nGates * sizeof(double));
  
  memset(_zdr - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_zdr, zdr, nGates * sizeof(double));
  
  memset(_phidp - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_phidp, phidp, nGates * sizeof(double));
  
  memset(_phidpCond - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memset(_phidpFilt - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memset(_psob - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  
  memset(_rhohv - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_rhohv, rhohv, nGates * sizeof(double));
  
  memset(_snr - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memcpy(_snr, snr, nGates * sizeof(double));
  
  memset(_ldr - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  if (ldr != NULL) {
    memcpy(_ldr, ldr, nGates * sizeof(double));
  } else {
    for (int igate = 0; igate < nGates; igate++) {
      _ldr[igate] = -9999;
    }
  }

  // initialize computed arrays

  memset(_sdphidp - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memset(_sdzdr - _arrayExtra, 0, (_arrayLen) * sizeof(double));
  memset(_goodFlag - _arrayExtra, 0, (_arrayLen) * sizeof(bool));
  memset(_kdp - _arrayExtra, 0, (_arrayLen) * sizeof(double));

  // beyond max range, set input values to missing

  if (_limitMaxRange) {
    for (int igate = _nGatesMaxRange; igate < nGates; igate++) {
      _dbz[igate] = _missingValue;
      _zdr[igate] = _missingValue;
      _phidp[igate] = _missingValue;
      _rhohv[igate] = _missingValue;
      _snr[igate] = _missingValue;
      _ldr[igate] = _missingValue;
    }
  }

  // if data is missing, set it to value at previous gate
  // except for phidp

  if (_dbz[0] == _missingValue) {
    _dbz[0] = -32.0;
  }
  if (_zdr[0] == _missingValue) {
    _zdr[0] = 0.0;
  }
  if (_rhohv[0] == _missingValue) {
    _rhohv[0] = 0.0;
  }
  if (_snr[0] == _missingValue) {
    _snr[0] = -10.0;
  }
  
  for (int igate = 1; igate < nGates; igate++) {
    if (_dbz[igate] == _missingValue) {
      _dbz[igate] = _dbz[igate-1];
    }
    if (_zdr[igate] == _missingValue) {
      _zdr[igate] = _zdr[igate-1];
    }
    if (_rhohv[igate] == _missingValue) {
      _rhohv[igate] = _rhohv[igate-1];
    }
    if (_snr[igate] == _missingValue) {
      _snr[igate] = _snr[igate-1];
    }
  }
  
  for (int igate = -_arrayExtra; igate < 0; igate++) {
    _dbz[igate] = _dbz[0];
    _zdr[igate] = _zdr[0];
    _rhohv[igate] = _rhohv[0];
    _snr[igate] = _snr[0];
  }
  
  for (int igate = nGates; igate < nGates + _arrayExtra; igate++) {
    _dbz[igate] = _dbz[nGates-1];
    _zdr[igate] = _zdr[nGates-1];
    _rhohv[igate] = _rhohv[nGates-1];
    _snr[igate] = _snr[nGates-1];
  }

}

//////////////////////////////////////////////////////////////////////////
// Find 'good' runs

void KdpBringi::_findGoodRuns()
{

  int nGates = _nGatesMaxRange;
  bool inGood = false;
  int count = 0;

  int ibegin = nGates - 1;
  int iend = nGates - 1;

  _goodRuns.clear();

  for (int igate = 0; igate < nGates; igate++) {

    bool goodGate = ((_rhohv[igate] >= _rhohvWxThreshold) &&
                     (_snr[igate] >= _snrThreshold) &&
                     (_ldr[igate] <= _ldrThreshold) &&
                     (_sdphidp[igate] < _phidpSdevThreshold) &&
                     (_sdzdr[igate] < _zdrSdevThreshold));
    count++;
    
    if (!inGood) {
      
      // in bad run
      
      if (!goodGate) {
	count = 0;
        continue;
      }
      
      if (count == N_GOOD) {
        // start of good run
        ibegin = igate - N_GOOD + 1;
        iend  = nGates - 1;
        inGood = true;
        count = 0;
      } // if (count == N_GOOD)
      
    } else {
      
      // in good run
      
      if(igate == nGates - 1) {
	iend = nGates - 1;
	continue; // igate loop
      }

      if (goodGate) {
        count = 0;
        continue;
      }
      
      if (count == N_BAD) {
        
        // check to preserve hail/BB signal.
        // NOTE: BB with mean Dbz<30 dBZ may be classified as "bad" data
        
        double zSum = 0.0;
        double xx[N_BAD], yy[N_BAD];
        for (int jj = 0; jj < N_BAD; jj++) {
          int kk = igate-jj;
          zSum += pow(10.0, (0.1*_dbz[kk]));
          yy[jj] = _rhohv[kk];
          xx[jj] = _phidp[kk];
        }
        double zMean = zSum / N_BAD;
        double dbzMean = 10.0*log10(zMean);
        
        bool goodCheck = false;
        
        if (dbzMean >= 30.0) {
          
          double rhohvMean, rhohvSd;
          _msr(rhohvMean, rhohvSd, yy, N_BAD);
          double phidpMean, phidpSd;
          _msr(phidpMean, phidpSd, xx, N_BAD);
          
          // rhohv in BB could go as low as 0.6
          //  checking mean rhohv in BB/hail region
	  
          if ((rhohvMean >= 0.6) && 
              (phidpSd < _phidpSdevThreshold * 1.5)) {
            goodCheck = true;
          }
          
        } // if (dbzMean >= 30.0)
        
        if (!goodCheck) {
          iend = igate - N_BAD;
          PhidpRun run(ibegin, iend);
          _goodRuns.push_back(run);
          inGood = false;
        }
        
        count = 0;
        
      } // if (count == N_BAD ...
      
    } // if (!inGood)

  } // igate
  
  if (inGood) {
    PhidpRun run(ibegin, iend);
    _goodRuns.push_back(run);
  }

  if (_goodRuns.size() < 1) {
    return;
  }

  // set pointers to first and last runs

  _runFirst = &_goodRuns[0];
  _runLast = &_goodRuns[_goodRuns.size()-1];
  
  // compute start and end phidp for runs

  for (size_t ii = 0; ii < _goodRuns.size(); ii++) {
    PhidpRun &goodRun = _goodRuns[ii];
    goodRun.phidpBegin = _mean(_phidp + goodRun.ibegin, N_MEAN);
    goodRun.phidpEnd = _mean(_phidp + goodRun.iend - N_MEAN, N_MEAN);
  }

  // set good flag
  
  for (size_t ii = 0; ii < _goodRuns.size(); ii++) {
    const PhidpRun &goodRun = _goodRuns[ii];
    for (int igate = goodRun.ibegin; igate <= goodRun.iend; igate++) {
      _goodFlag[igate] = true;
    }
  }

}
  
/////////////////////////////////////////////
// unfold phidp
// This is Yanting's modified unfold code 

void KdpBringi::_unfoldPhidp(int nGates)

{

  double r_min = 5; /* the minimum range in km depends on radar system
                     * i.e. beyond far field */

  // find first gate without missing data and beyond the far field (5 km)
  
  int firstGate = -1;
  for (int igate = 0; igate < nGates; igate++) {
    if (_phidp[igate] != _missingValue && _range[igate] >= r_min) {
      firstGate = igate;
      break;
    }
  }

  if (firstGate < 0) {
    return;
  }
  
  // compute the beginning phidp by moving up through the gates to find
  // a set of gates over which the stdev of phidp is less than the threshold
  
  double phi_begin = _phidp[firstGate];
  int count = 0;
  for (int igate = firstGate; igate < nGates; igate++) {
    
    // compute sdev of phidp over next N_GOOD gates
    
    double phi_mean,phidp_sd;
    _msr(phi_mean, phidp_sd, _phidp + igate, N_GOOD);
    
    // is the standard deviation below the threshold?
    // if so we are ready to begin

    if(phidp_sd < _phidpSdevThreshold) {
      count++;
      if(count >= N_GOOD) {
        phi_begin = phi_mean;
        firstGate = igate;
        break;
      } else {
        count = 0;
      }
    }
    
  } // igate
        
  // for gates before the start gate, or after the last gate,
  // fill in missing regions with the surounding gates

  for (int igate = -_arrayExtra; igate < 0; igate++) {
    _phidp[igate] = phi_begin;
  }
  
  for (int igate = 0; igate < firstGate; igate++) {
    if (_phidp[igate] == _missingValue) {
      _phidp[igate] = phi_begin;
    }
  }
  
  for (int igate = firstGate; igate < nGates; igate++) {
    if (_phidp[igate] == _missingValue) {
      _phidp[igate] = _phidp[igate-1];
    }
  }
  
  // look for fold

  double phi_recent = phi_begin;
  for (int igate = firstGate; igate < nGates; igate++) {
    
    // compute mean and sdev for the previous N_GOOD gates, including
    // the current gate
    
    double phi_mean, phi_sd;
    _msr(phi_mean, phi_sd, _phidp + igate - N_GOOD, N_GOOD);

    // if smooth, store the mean as the 'recent' value

    if(phi_sd < _phidpSdevThreshold) {
      phi_recent = phi_mean;
    }

    // if the current differs from the recent appreciably, folding
    // has occurred. Unfold accordingly
    
    double diff = phi_recent - _phidp[igate];
    
    if (diff > 160 && diff < 200) {
      _phidp[igate] += 180;
    } else if (diff > 340) {
      _phidp[igate] += 360;
    } else if (diff < -160 && diff > -200) {
      _phidp[igate] -= 180;
    } else if (diff < -340) {
      _phidp[igate] -= 360;
    }
    
  } // igate

  for (int igate = nGates; igate < nGates +  _arrayExtra; igate++) {
    _phidp[igate] = _phidp[nGates-1];
  }

}
    
/////////////////////////////////////////////
// load conditioned phidp array for phidp
// interpolating between good runs

void KdpBringi::_loadConditionedPhidp(int nGates)

{
  
  if (_goodRuns.size() == 0) {
    double beginPhidp = _phidp[0];
    for (int igate = -_arrayExtra; igate < nGates + _arrayExtra; igate++) {
      _phidpCond[igate] = beginPhidp;
    }
    return;
  }

  for (int igate = -_arrayExtra; igate < nGates + _arrayExtra; igate++) {
    _phidpCond[igate] = _phidp[igate];
  }

  // before first run use initial phidp

  int ibeginGood = _runFirst->ibegin;
  double beginPhidp = _runFirst->phidpBegin;
  for (int igate = -_arrayExtra; igate < ibeginGood; igate++) {
    _phidpCond[igate] = beginPhidp;
  }
  
  // after last run use final phidp

  int iendGood = _runLast->iend;
  double endPhidp = _runLast->phidpEnd;
  for (int igate = iendGood + 1; igate < nGates + _arrayExtra; igate++) {
    _phidpCond[igate] = endPhidp;
  }

  // between runs interpolate
  
  for (size_t ii = 1; ii < _goodRuns.size(); ii++) {

    const PhidpRun &prevRun = _goodRuns[ii-1];
    const PhidpRun &thisRun = _goodRuns[ii];

    int iendPrev = prevRun.iend;
    double prevPhidp = prevRun.phidpEnd;
    
    int ibeginThis = thisRun.ibegin;
    double thisPhidp = thisRun.phidpBegin;

    double gateCount = ibeginThis - iendPrev;
    double dphi = (thisPhidp - prevPhidp) / gateCount;
    double phidpCond = prevPhidp;

    for (int igate = iendPrev + 1; igate < ibeginThis; igate++) {
      phidpCond += dphi;
      _phidpCond[igate] = phidpCond;
    }

  } // ii

}

//////////////////////////////////////////////////////////////////////////
//  To calculate the mean of the array starting with y[0]
//  i.e. (i = 0 ... n-1)

double KdpBringi::_mean(const double *y, int n)

{
  
  double ysum  = 0.0;
  double total = n;

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) > 1.e35) {
      total--;
    } else {
      ysum += y[i];
    }
  }

  if (total > 0) {
    return ysum / total;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////////////////////////////////
//  To calculate the mean (ymn) and standard deviation (sd, or,
//  mean square root, msr) of the array y(i) (i=1,...,n).
//                                               Li Liu  Sep. 19, 95

void KdpBringi::_msr(double &ymn, double &sd, const double *y, int n)

{
  
  double ysum  = 0.0;
  double yysum = 0.0;
  double total = n;

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) > 1.e35) {
      total--;
    } else {
      ysum += y[i];
    }
  }

  if (total > 0) {
    ymn = ysum / total;
  } else {
    ymn = 0;
  }

  for (int i = 0; i < n; i++) {
    if (fabs(y[i]) < 1.e35) {
      double diff = y[i] - ymn;
      yysum += diff * diff;
    }
  }
  if (total > 0) {
    sd = sqrt(yysum/total);
  } else {
    sd = 0.0;
  }

}

/////////////////////////////////////////////////////////////////////////
// This is a Linear Least Square Estimate subroutine to fit a linear
// equation for (xi,yi) (i=1,...,n), so that
//                         yi = a * xi + b
// INPUTs: x(i), y(i), n, (i=1,...,n ).
// OUTPUTs: a ( slope ), b ( intercept ).
//                                                Li Liu   Sep. 23, 92

void KdpBringi::_lse(double &a, double &b, const double *x, const double *y, int n)
  
{

  double xsum = 0.0;
  double ysum = 0.0;
  double xxsum = 0.0;
  double yysum = 0.0;
  double xysum = 0.0;
  double total = n;
  for (int i = 0; i < n; i++) {
    if (x[i] > 1.e35 || y[i] > 1.e35) {
      total--;
    } else {
      xsum += x[i];
      ysum += y[i];
      xxsum += x[i]*x[i];
      yysum += y[i]*y[i];
      xysum += x[i]*y[i];
    }
  }

  double det = total * xxsum - xsum * xsum;
  a = ( total*xysum - xsum*ysum ) / det;
  b = ( ysum*xxsum - xsum*xysum ) / det;

#ifdef NOTNOW

  // get y-on-x slope

  double num = total * xysum - xsum * ysum;
  double denomx = total * xxsum - xsum * xsum;
  double slope_y_on_x;
  
  if (denomx != 0.0) {
    slope_y_on_x = num / denomx;
  } else {
    slope_y_on_x = 0.0;
  }
  
  // get x-on-y slope

  double denomy = total * yysum - ysum * ysum;
  double slope_x_on_y;
  
  if (denomy != 0.0) {
    slope_x_on_y = num / denomy;
  } else {
    slope_x_on_y = 0.0;
  }

  // average slopes

  double slope;
  if (slope_y_on_x != 0.0 && slope_x_on_y != 0.0) {
    slope = (slope_y_on_x + 1.0 / slope_x_on_y) / 2.0;
  } else if (slope_y_on_x != 0.0) {
    slope = slope_y_on_x;
  } else if (slope_x_on_y != 0.0) {
    slope = 1.0 / slope_x_on_y;
  } else {
    slope = 0.0;
  }

  a = slope;

#endif

}

