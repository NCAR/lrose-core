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
// NoiseLocator.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// Find noise gates in Doppler radar data
//
///////////////////////////////////////////////////////////////

#include <radar/NoiseLocator.hh>
#include <toolsa/mem.h>
#include <algorithm>
using namespace std;

// mutexes

pthread_mutex_t NoiseLocator::_prevGridMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t NoiseLocator::_computeMethodMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t NoiseLocator::_runningMedianMutex = PTHREAD_MUTEX_INITIALIZER;

// method for computing the median noise

NoiseLocator::compute_method_t NoiseLocator::_computeMethod
  = NoiseLocator::RAY_BY_RAY;

// grid for storing previous noise values

NoiseLocator::noise_val_t **NoiseLocator::_previousGrid = NULL;
const double NoiseLocator::_gridResEl = 360.0 / _gridSizeEl;
const double NoiseLocator::_gridResAz = 360.0 / _gridSizeAz;

// running median method

int NoiseLocator::_nGatesRunningMedian = 2500;
int NoiseLocator::_nGatesRunningCount = 0;
vector<double> NoiseLocator::_runningValsDbmHc;
vector<double> NoiseLocator::_runningValsDbmVc;
vector<double> NoiseLocator::_runningValsDbmHx;
vector<double> NoiseLocator::_runningValsDbmVx;
double NoiseLocator::_runningMedianNoiseDbmHc = -9999;
double NoiseLocator::_runningMedianNoiseDbmVc = -9999;
double NoiseLocator::_runningMedianNoiseDbmHx = -9999;
double NoiseLocator::_runningMedianNoiseDbmVx = -9999;

// search kernel for finding most appropriate previous
// noise value

NoiseLocator::search_kernel_t NoiseLocator::_searchKernel[25] =
  {
    {0,0},   {-1,0},  {1,0},   {0,1},   {0,-1},
    {-1,1},  {1,1},   {-1,-1}, {1,-1},  {-2,0},
    {2,0},   {-2,1},  {2,1},   {0, 2},  {-1,2},
    {1,2},   {-2,2},  {2,2},   {-2,-1}, {2,-1},
    {0,-2},  {-1,-2}, {1,-2},  {-2,-2}, {2,-2}
  };

///////////////////////////////////////////////////////////////
// Constructor

NoiseLocator::NoiseLocator()
  
{

  _debug = false;
  _equalBiasInAllChannels = false;

  // create grid if not yet done
  // grid resolution is 0.5 degreed in each dimension (el, az)
  // init to 0

  pthread_mutex_lock(&_prevGridMutex);
  if (_previousGrid == NULL) {
    _previousGrid = (noise_val_t **)
      umalloc2(720, 720, sizeof(noise_val_t));
    memset(*_previousGrid, 0, 720 * 720 * sizeof(noise_val_t));
  }
  pthread_mutex_unlock(&_prevGridMutex);

  // create the default interest maps

  _interestMapPhaseChangeErrorForNoise = NULL;
  _interestMapDbmSdevForNoise = NULL;
  _interestMapNcpMeanForNoise = NULL;
  _interestMapPhaseChangeErrorForSignal = NULL;
  _interestMapDbmSdevForSignal = NULL;

  _createDefaultInterestMaps();
  
  _nGatesKernel = 9;
  _minNGatesRayMedian = 30;

}

///////////////////////////////////////////////////////////////
// destructor

NoiseLocator::~NoiseLocator()
  
{

  pthread_mutex_lock(&_prevGridMutex);
  if (_previousGrid) {
    ufree2((void **) _previousGrid);
    _previousGrid = NULL;
  }
  pthread_mutex_unlock(&_prevGridMutex);

  if (_interestMapPhaseChangeErrorForNoise) {
    delete _interestMapPhaseChangeErrorForNoise;
  }
  if (_interestMapDbmSdevForNoise) {
    delete _interestMapDbmSdevForNoise;
  }
  if (_interestMapNcpMeanForNoise) {
    delete _interestMapNcpMeanForNoise;
  }
  if (_interestMapPhaseChangeErrorForSignal) {
    delete _interestMapPhaseChangeErrorForSignal;
  }
  if (_interestMapDbmSdevForSignal) {
    delete _interestMapDbmSdevForSignal;
  }

}

///////////////////////////////////////////////////////////////
// for the ray-by-ray method, we compute noise for individual
// rays so the noise varies on a ray-by-ray basis

void NoiseLocator::setComputeRayMedian(int minNGatesRayMedian)
{
  
  pthread_mutex_lock(&_computeMethodMutex);
  _computeMethod = RAY_BY_RAY;
  _minNGatesRayMedian = minNGatesRayMedian;
  pthread_mutex_unlock(&_computeMethodMutex);

}

///////////////////////////////////////////////////////////////
// for the running-median method, we compute a running median of
// the noise which is applied to the rays in sequence
// so the estimated noise varies slowly from time to time

void NoiseLocator::setComputeRunningMedian(int nGatesMedian)
  
{ 
  
  pthread_mutex_lock(&_computeMethodMutex);

  _computeMethod = RUNNING_MEDIAN;
  _nGatesRunningMedian = nGatesMedian;

  if (nGatesMedian != (int) _runningValsDbmHc.size()) {
    _runningValsDbmHc.resize(nGatesMedian);
    _runningValsDbmVc.resize(nGatesMedian);
    _runningValsDbmHx.resize(nGatesMedian);
    _runningValsDbmVx.resize(nGatesMedian);
    _nGatesRunningCount = 0;
  }

  pthread_mutex_unlock(&_computeMethodMutex);

}

///////////////////////////////////////////////////////////////
// print parameters for debugging

void NoiseLocator::printParams(ostream &out)
  
{

  pthread_mutex_lock(&_computeMethodMutex);

  out << "Performing noise detection:" << endl;
  out << "  nGatesForNoiseDetection: " << _nGatesKernel << endl;
  out << "  minNGatesRayMedian: " << _minNGatesRayMedian << endl;
  out << "  interestThresholdForNoise: " 
      << _interestThresholdForNoise << endl;
  out << "  interestThresholdForSignal: " 
      << _interestThresholdForSignal << endl;
  
  if (_interestMapPhaseChangeErrorForNoise) {
    _interestMapPhaseChangeErrorForNoise->printParams(out);
  }
  if (_interestMapDbmSdevForNoise) {
    _interestMapDbmSdevForNoise->printParams(out);
  }
  if (_interestMapNcpMeanForNoise) {
    _interestMapNcpMeanForNoise->printParams(out);
  }

  if (_interestMapPhaseChangeErrorForSignal) {
    _interestMapPhaseChangeErrorForSignal->printParams(out);
  }
  if (_interestMapDbmSdevForSignal) {
    _interestMapDbmSdevForSignal->printParams(out);
  }

  if (_computeMethod == RAY_BY_RAY) {
    out << "Median method: RAY_BY_RAY" << endl;
  } else {
    out << "Median method: RUNNING_MEDIAN" << endl;
    out << "  nGatesRunningMedian: " << _nGatesRunningMedian << endl;
  }

  pthread_mutex_unlock(&_computeMethodMutex);

}

///////////////////////////////////////////////////////////////
// set the ray properties
// must be called before locate() or computeNoise()

void NoiseLocator::setRayProps(int nGates,
                               const IwrfCalib &calib,
                               time_t timeSecs, 
                               double nanoSecs,
                               double elevation, 
                               double azimuth)

{

  _nGates = nGates;
  _calib = calib;
  _timeSecs = timeSecs;
  _nanoSecs = nanoSecs;
  _elevation = elevation;
  _azimuth = azimuth;

  // compute index for saving data in grid

  _gridIndexAz = (int) (_azimuth / _gridResAz);
  _gridIndexAz = _gridIndexAz % _gridSizeAz;
  if (_gridIndexAz < 0) _gridIndexAz += _gridSizeAz;
  
  _gridIndexEl = (int) (_elevation / _gridResEl);
  _gridIndexEl = _gridIndexEl % _gridSizeEl;
  if (_gridIndexEl < 0) _gridIndexEl += _gridSizeEl;

}

///////////////////////////////////////////////////////////////
// locate the noise gates

void NoiseLocator::locate(const MomentsFields *mfields)
  
{

  _noiseFlag.resize(_nGates);
  _signalFlag.resize(_nGates);
  _startGate.resize(_nGates);
  _endGate.resize(_nGates);
  _accumPhaseChange.resize(_nGates);
  _phaseChangeError.resize(_nGates);
  _dbmSdev.resize(_nGates);
  _ncpMean.resize(_nGates);

  for (int igate = 0; igate < _nGates; igate++) {
    _noiseFlag[igate] = false;
    _signalFlag[igate] = false;
    _startGate[igate] = 0;
    _endGate[igate] = 0;
    _accumPhaseChange[igate] = -9999;
    _phaseChangeError[igate] = -9999;
    _dbmSdev[igate] = -9999;
    _ncpMean[igate] = -9999;
  }
  
  // first compute the absolute phase at each gate, summing up
  // the phase change from the start to end of the ray
  // the units are degrees
  
  double prevPhaseSum = RadarComplex::argDeg(mfields[0].phase_for_noise);
  
  for (int igate = 1; igate < _nGates; igate++) {
    RadarComplex_t diff =
      RadarComplex::conjugateProduct(mfields[igate].phase_for_noise,
                                     mfields[igate-1].phase_for_noise);
    double diffDeg = RadarComplex::argDeg(diff);
    double phaseSum = prevPhaseSum + diffDeg;
    _accumPhaseChange[igate] = phaseSum;
    prevPhaseSum = phaseSum;
  }
  
  // set kernel size for computing phase error
  // make sure it is odd, add 1 if necessary
  
  int kernelSize = _nGatesKernel;
  int kernelHalf = kernelSize / 2;
  kernelSize = kernelHalf * 2 + 1;

  if ((int) _nGates < kernelSize) {
    return;
  }

  // set up gate limits
  
  for (int igate = 0; igate < _nGates; igate++) {
    int istart = igate - kernelHalf;
    int iend = igate + kernelHalf;
    if (istart < 0) {
      int iadj = 0 - istart;
      istart += iadj;
      iend += iadj;
    } else if (iend > (int) _nGates - 1) {
      int iadj = iend - (_nGates - 1);
      iend -= iadj;
      istart -= iadj;
    }
    _startGate[igate] = istart;
    _endGate[igate] = iend;
  }

  // compute phase change error

  for (int igate = 0; igate < _nGates; igate++) {
    _phaseChangeError[igate] =
      _computePhaseChangeError(_startGate[igate], _endGate[igate]);
  }
  
  // compute dbm sdev

  _computeDbmSdev(mfields);
  _computeNcpMean(mfields);

  // set flags

  double sumWeightsNoise = (_weightPhaseChangeErrorForNoise + 
                            _weightDbmSdevForNoise +
                            _weightNcpMeanForNoise);
  double sumWeightsSignal = (_weightPhaseChangeErrorForSignal + 
                             _weightDbmSdevForSignal);

  for (int igate = 0; igate < _nGates; igate++) {

    double pce = _phaseChangeError[igate];
    double dbmSdev = _dbmSdev[igate];
    double ncpMean = _ncpMean[igate];

    double sumInterestNoise =
      (_interestMapPhaseChangeErrorForNoise->getInterest(pce) * 
       _weightPhaseChangeErrorForNoise) +
      (_interestMapDbmSdevForNoise->getInterest(dbmSdev) * 
       _weightDbmSdevForNoise) +
      (_interestMapNcpMeanForNoise->getInterest(ncpMean) * 
       _weightNcpMeanForNoise);
    
    double interestNoise = sumInterestNoise / sumWeightsNoise;

    double sumInterestSignal =
      (_interestMapPhaseChangeErrorForSignal->getInterest(pce) * 
       _weightPhaseChangeErrorForSignal) +
      (_interestMapDbmSdevForSignal->getInterest(dbmSdev) * 
       _weightDbmSdevForSignal);

    double interestSignal = sumInterestSignal / sumWeightsSignal;

    if (interestNoise > _interestThresholdForNoise) {
      _noiseFlag[igate] = true;
    } else {
      _noiseFlag[igate] = false;
    }

    if (interestSignal > _interestThresholdForSignal) {
      _signalFlag[igate] = false;
    } else {
      _signalFlag[igate] = true;
    }

  } // igate

  // for single gates surrounded by noise, set the noise flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (_noiseFlag[igate-1] && _noiseFlag[igate+1]) {
      _noiseFlag[igate] = true;
    }
  }
 
  // for single gates surrounded by non-noise, unset the noise flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (!_noiseFlag[igate-1] && !_noiseFlag[igate+1]) {
      _noiseFlag[igate] = false;
    }
  }
 
  // for single gates surrounded by non-signal, unset the signal flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (!_signalFlag[igate-1] && !_signalFlag[igate+1]) {
      _signalFlag[igate] = false;
    }
  }
 
}

///////////////////////////////////////////////////////////////
// compute mean phase error in range, for the specified kernel

double NoiseLocator::_computePhaseChangeError(int startGate, int endGate)
  
{

  double phaseStart = _accumPhaseChange[startGate];
  double phaseEnd = _accumPhaseChange[endGate];
  double dGates = endGate - startGate;
  double slope = (phaseEnd - phaseStart) / dGates;
  
  double linearPhase = phaseStart + slope;
  double sumAbsError = 0.0;
  double count = 0.0;
  
  for (int igate = startGate + 1; igate < endGate; 
       igate++, linearPhase += slope) {
    double phase = _accumPhaseChange[igate];
    double absError = fabs(phase - linearPhase);
    sumAbsError += absError;
    count++;
  }

  return sumAbsError / count;

}

///////////////////////////////////////////////////////////////
// compute for DBM SDEV

void NoiseLocator::_computeDbmSdev(const MomentsFields *mfields)
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nDbm = 0.0;
    double sumDbm = 0.0;
    double sumDbmSq = 0.0;
    
    for (size_t jgate = _startGate[igate]; jgate <= _endGate[igate]; jgate++) {
      
      double dbm = mfields[jgate].dbm_for_noise;
      
      if (dbm != MomentsFields::missingDouble) {
        sumDbm += dbm;
        sumDbmSq += (dbm * dbm);
        nDbm++;
      }
      
    } // jgate
    
    if (nDbm > 0) {
      double meanDbm = sumDbm / nDbm;
      if (nDbm > 2) {
        double term1 = sumDbmSq / nDbm;
        double term2 = meanDbm * meanDbm;
        if (term1 >= term2) {
          _dbmSdev[igate] = sqrt(term1 - term2);
        }
      }
    }
    
  } // igate

}

///////////////////////////////////////////////////////////////
// compute NCP mean

void NoiseLocator::_computeNcpMean(const MomentsFields *mfields)
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nNcp = 0.0;
    double sumNcp = 0.0;
    
    for (size_t jgate = _startGate[igate]; jgate <= _endGate[igate]; jgate++) {
      
      double ncp = mfields[jgate].ncp;
      
      if (ncp != MomentsFields::missingDouble) {
        sumNcp += ncp;
        nNcp++;
      }
      
    } // jgate

    if (nNcp > 0) {
      double meanNcp = sumNcp / nNcp;
      _ncpMean[igate] = meanNcp;
    }
    
  } // igate

}

///////////////////////////////////////////////////////////////
// Identify the noise, compute mean noise
// Single pol
//
// The following must be set in mfields prior to calling:
//   lag0_hc_db

void NoiseLocator::computeNoiseSinglePolH(MomentsFields *mfields)
  
  
{

  // initialize

  _medianNoiseDbmHc = _calib.getNoiseDbmHc();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHc;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHc.push_back(mfield.lag0_hc_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseHc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmHc = _computeMedian(noiseHc);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseHc = _medianNoiseDbmHc;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestHc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmHc = prev.noiseHc;
      }
      
    }

  } else {

    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);
    
    for (int igate = 0; igate < _nGates; igate++) {
      
      // add valid gates to the arrays
      
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmHc[_nGatesRunningCount] = mfield.lag0_hc_db;
        _nGatesRunningCount++;
      }
      
      if (_nGatesRunningCount == _nGatesRunningMedian) {
        
        // if reached correct size, compute the medians
        
        _runningMedianNoiseDbmHc = _computeMedian(_runningValsDbmHc);
        _nGatesRunningCount = 0;
        
      }
      
    } // igate
    
    // set the mean to the latest median
    
    if (_runningMedianNoiseDbmHc > -9990) {
      _medianNoiseDbmHc = _runningMedianNoiseDbmHc;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbHc = _medianNoiseDbmHc - _calib.getNoiseDbmHc();

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hc = _noiseBiasDbHc;
  }
  
}

// The following must be set in mfields prior to calling:
//   lag0_vc_db

void NoiseLocator::computeNoiseSinglePolV(MomentsFields *mfields)
  
  
{

  // initialize

  _medianNoiseDbmVc = _calib.getNoiseDbmVc();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseVc;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseVc.push_back(mfield.lag0_vc_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseVc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmVc = _computeMedian(noiseVc);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseVc = _medianNoiseDbmVc;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestVc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmVc = prev.noiseVc;
      }
      
    }

  } else {

    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);
    
    for (int igate = 0; igate < _nGates; igate++) {
      
      // add valid gates to the arrays
      
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmVc[_nGatesRunningCount] = mfield.lag0_vc_db;
        _nGatesRunningCount++;
      }
      
      if (_nGatesRunningCount == _nGatesRunningMedian) {
        
        // if reached correct size, compute the medians
        
        _runningMedianNoiseDbmVc = _computeMedian(_runningValsDbmVc);
        _nGatesRunningCount = 0;
        
      }
      
    } // igate
    
    // set the mean to the latest median
    
    if (_runningMedianNoiseDbmVc > -9990) {
      _medianNoiseDbmVc = _runningMedianNoiseDbmVc;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbVc = _medianNoiseDbmVc - _calib.getNoiseDbmVc();

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_vc = _noiseBiasDbVc;
  }
  
}

///////////////////////////////////////////////////////////////
// Identify the noise, compute median noise
// Alternating mode dual pol, co-pol receiver only
//
// The following must be set in mfields prior to calling:
//   lag0_hc_db
//   lag0_vc_db

void NoiseLocator::computeNoiseDpAltHvCoOnly(MomentsFields *mfields)
  
{

  // initialize

  _medianNoiseDbmHc = _calib.getNoiseDbmHc();
  _medianNoiseDbmVc = _calib.getNoiseDbmVc();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHc;
    vector<double> noiseVc;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHc.push_back(mfield.lag0_hc_db);
        noiseVc.push_back(mfield.lag0_vc_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseHc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmHc = _computeMedian(noiseHc);
      _medianNoiseDbmVc = _computeMedian(noiseVc);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseHc = _medianNoiseDbmHc;
      nval.noiseVc = _medianNoiseDbmVc;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestHc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmHc = prev.noiseHc;
        _medianNoiseDbmVc = prev.noiseVc;
      }
      
    }

  } else {

    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);
    
    for (int igate = 0; igate < _nGates; igate++) {

      // add valid gates to the arrays

      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmHc[_nGatesRunningCount] = mfield.lag0_hc_db;
        _runningValsDbmVc[_nGatesRunningCount] = mfield.lag0_vc_db;
        _nGatesRunningCount++;
      }

      if (_nGatesRunningCount == _nGatesRunningMedian) {
        // if reached correct size, compute the medians
        _runningMedianNoiseDbmHc = _computeMedian(_runningValsDbmHc);
        _runningMedianNoiseDbmVc = _computeMedian(_runningValsDbmVc);
        _nGatesRunningCount = 0;
      }

    } // igate
    
    // set the mean to the latest median

    if (_runningMedianNoiseDbmHc > -9990) {
      _medianNoiseDbmHc = _runningMedianNoiseDbmHc;
    }
    if (_runningMedianNoiseDbmVc > -9990) {
      _medianNoiseDbmVc = _runningMedianNoiseDbmVc;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbHc = _medianNoiseDbmHc - _calib.getNoiseDbmHc();
  _noiseBiasDbVc = _medianNoiseDbmVc - _calib.getNoiseDbmVc();

  // if required, set equal bias in both channels
  // use Hc bias as the master

  if (_equalBiasInAllChannels) {
    _noiseBiasDbVc = _noiseBiasDbHc;
    _medianNoiseDbmVc = _calib.getNoiseDbmVc() + _noiseBiasDbVc;
  }

  // set noise moment for each gate

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hc = _noiseBiasDbHc;
    mfield.noise_bias_db_vc = _noiseBiasDbVc;
  }
  
}

///////////////////////////////////////////////////////////////
// Identify the noise, compute median noise
// Alternating mode dual pol, co/cross receivers
//
// The following must be set in mfields prior to calling:
//   lag0_hc_db
//   lag0_vc_db
//   lag0_hx_db
//   lag0_vx_db
  
void NoiseLocator::computeNoiseDpAltHvCoCross(MomentsFields *mfields)
  
{

  // initialize

  _medianNoiseDbmHc = _calib.getNoiseDbmHc();
  _medianNoiseDbmHx = _calib.getNoiseDbmHx();
  _medianNoiseDbmVc = _calib.getNoiseDbmVc();
  _medianNoiseDbmVx = _calib.getNoiseDbmVx();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHc;
    vector<double> noiseHx;
    vector<double> noiseVc;
    vector<double> noiseVx;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHc.push_back(mfield.lag0_hc_db);
        noiseHx.push_back(mfield.lag0_hx_db);
        noiseVc.push_back(mfield.lag0_vc_db);
        noiseVx.push_back(mfield.lag0_vx_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseHc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmHc = _computeMedian(noiseHc);
      _medianNoiseDbmHx = _computeMedian(noiseHx);
      _medianNoiseDbmVc = _computeMedian(noiseVc);
      _medianNoiseDbmVx = _computeMedian(noiseVx);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseHc = _medianNoiseDbmHc;
      nval.noiseVc = _medianNoiseDbmVc;
      nval.noiseHx = _medianNoiseDbmHx;
      nval.noiseVx = _medianNoiseDbmVx;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestHc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmHc = prev.noiseHc;
        _medianNoiseDbmVc = prev.noiseVc;
        _medianNoiseDbmHx = prev.noiseHx;
        _medianNoiseDbmVx = prev.noiseVx;
      }
      
    }

  } else {

    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);
    
    for (int igate = 0; igate < _nGates; igate++) {

      // add valid gates to the arrays

      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmHc[_nGatesRunningCount] = mfield.lag0_hc_db;
        _runningValsDbmVc[_nGatesRunningCount] = mfield.lag0_vc_db;
        _runningValsDbmHx[_nGatesRunningCount] = mfield.lag0_hx_db;
        _runningValsDbmVx[_nGatesRunningCount] = mfield.lag0_vx_db;
        _nGatesRunningCount++;
      }

      if (_nGatesRunningCount == _nGatesRunningMedian) {
        // if reached correct size, compute the medians
        _runningMedianNoiseDbmHc = _computeMedian(_runningValsDbmHc);
        _runningMedianNoiseDbmVc = _computeMedian(_runningValsDbmVc);
        _runningMedianNoiseDbmHx = _computeMedian(_runningValsDbmHx);
        _runningMedianNoiseDbmVx = _computeMedian(_runningValsDbmVx);
        _nGatesRunningCount = 0;
      }

    } // igate
    
    // set the mean to the latest median

    if (_runningMedianNoiseDbmHc > -9990) {
      _medianNoiseDbmHc = _runningMedianNoiseDbmHc;
    }
    if (_runningMedianNoiseDbmVc > -9990) {
      _medianNoiseDbmVc = _runningMedianNoiseDbmVc;
    }
    if (_runningMedianNoiseDbmHx > -9990) {
      _medianNoiseDbmHx = _runningMedianNoiseDbmHx;
    }
    if (_runningMedianNoiseDbmVx > -9990) {
      _medianNoiseDbmVx = _runningMedianNoiseDbmVx;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }
  
  // set the noise bias in the moments

  _noiseBiasDbHc = _medianNoiseDbmHc - _calib.getNoiseDbmHc();
  _noiseBiasDbVc = _medianNoiseDbmVc - _calib.getNoiseDbmVc();
  _noiseBiasDbHx = _medianNoiseDbmHx - _calib.getNoiseDbmHx();
  _noiseBiasDbVx = _medianNoiseDbmVx - _calib.getNoiseDbmVx();

  // if required, set equal bias in both channels
  // use Hc bias as the master

  if (_equalBiasInAllChannels) {
    _noiseBiasDbVc = _noiseBiasDbHc;
    _medianNoiseDbmVc = _calib.getNoiseDbmVc() + _noiseBiasDbVc;
    _noiseBiasDbHx = _noiseBiasDbHc;
    _medianNoiseDbmHx = _calib.getNoiseDbmHx() + _noiseBiasDbHx;
    _noiseBiasDbVx = _noiseBiasDbHc;
    _medianNoiseDbmVx = _calib.getNoiseDbmVx() + _noiseBiasDbVx;
  }

  // set noise moment for each gate

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hc = _noiseBiasDbHc;
    mfield.noise_bias_db_vc = _noiseBiasDbVc;
    mfield.noise_bias_db_hx = _noiseBiasDbHx;
    mfield.noise_bias_db_vx = _noiseBiasDbVx;
  }
  
}

///////////////////////////////////////////////////////////////
// Identify the noise, compute median noise
// Sim HV mode
//
// The following must be set in mfields prior to calling:
//   lag0_hc_db
//   lag0_vc_db
  
void NoiseLocator::computeNoiseDpSimHv(MomentsFields *mfields)
  
{

  // initialize

  _medianNoiseDbmHc = _calib.getNoiseDbmHc();
  _medianNoiseDbmVc = _calib.getNoiseDbmVc();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHc;
    vector<double> noiseVc;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHc.push_back(mfield.lag0_hc_db);
        noiseVc.push_back(mfield.lag0_vc_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseHc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmHc = _computeMedian(noiseHc);
      _medianNoiseDbmVc = _computeMedian(noiseVc);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseHc = _medianNoiseDbmHc;
      nval.noiseVc = _medianNoiseDbmVc;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestHc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmHc = prev.noiseHc;
      _medianNoiseDbmVc = prev.noiseVc;
      }
      
    }

  } else {

    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);

    for (int igate = 0; igate < _nGates; igate++) {
      // add valid gates to the arrays
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmHc[_nGatesRunningCount] = mfield.lag0_hc_db;
        _runningValsDbmVc[_nGatesRunningCount] = mfield.lag0_vc_db;
        _nGatesRunningCount++;
      }
      if (_nGatesRunningCount == _nGatesRunningMedian) {
        // if reached correct size, compute the medians
        _runningMedianNoiseDbmHc = _computeMedian(_runningValsDbmHc);
        _runningMedianNoiseDbmVc = _computeMedian(_runningValsDbmVc);
        _nGatesRunningCount = 0;
      }
    } // igate
    
    // set the mean to the latest median
    if (_runningMedianNoiseDbmHc > -9990) {
      _medianNoiseDbmHc = _runningMedianNoiseDbmHc;
    }
    if (_runningMedianNoiseDbmVc > -9990) {
      _medianNoiseDbmVc = _runningMedianNoiseDbmVc;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbHc = _medianNoiseDbmHc - _calib.getNoiseDbmHc();
  _noiseBiasDbVc = _medianNoiseDbmVc - _calib.getNoiseDbmVc();

  // if required, set equal bias in both channels
  // use Hc bias as the master

  if (_equalBiasInAllChannels) {
    _noiseBiasDbVc = _noiseBiasDbHc;
    _medianNoiseDbmVc = _calib.getNoiseDbmVc() + _noiseBiasDbVc;
  }

  // set noise moment for each gate

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hc = _noiseBiasDbHc;
    mfield.noise_bias_db_vc = _noiseBiasDbVc;
  }
  
}

///////////////////////////////////////////////////////////////
// Identify the noise, compute median noise
// Dual pol, H-transmit only
//
// The following must be set in mfields prior to calling:
//   lag0_hc_db
//   lag0_vx_db
  
void NoiseLocator::computeNoiseDpHOnly(MomentsFields *mfields)
  
{

  // initialize

  _medianNoiseDbmHc = _calib.getNoiseDbmHc();
  _medianNoiseDbmVx = _calib.getNoiseDbmVx();
    
  // locate the noise gates
  
  locate(mfields);

  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHc;
    vector<double> noiseVx;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHc.push_back(mfield.lag0_hc_db);
        noiseVx.push_back(mfield.lag0_vx_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseHc.size() >= _minNGatesRayMedian) {

      _medianNoiseDbmHc = _computeMedian(noiseHc);
      _medianNoiseDbmVx = _computeMedian(noiseVx);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseHc = _medianNoiseDbmHc;
      nval.noiseVx = _medianNoiseDbmVx;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestHc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmHc = prev.noiseHc;
        _medianNoiseDbmVx = prev.noiseVx;
      }
      
    }

  } else {
    
    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);

    for (int igate = 0; igate < _nGates; igate++) {
      // add valid gates to the arrays
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmHc[_nGatesRunningCount] = mfield.lag0_hc_db;
        _runningValsDbmVx[_nGatesRunningCount] = mfield.lag0_vx_db;
        _nGatesRunningCount++;
      }
      if (_nGatesRunningCount == _nGatesRunningMedian) {
        // if reached correct size, compute the medians
        _runningMedianNoiseDbmHc = _computeMedian(_runningValsDbmHc);
        _runningMedianNoiseDbmVx = _computeMedian(_runningValsDbmVx);
        _nGatesRunningCount = 0;
      }
    } // igate
    
    // set the mean to the latest median

    if (_runningMedianNoiseDbmHc > -9990) {
      _medianNoiseDbmHc = _runningMedianNoiseDbmHc;
    }
    if (_runningMedianNoiseDbmVx > -9990) {
      _medianNoiseDbmVx = _runningMedianNoiseDbmVx;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbHc = _medianNoiseDbmHc - _calib.getNoiseDbmHc();
  _noiseBiasDbVx = _medianNoiseDbmVx - _calib.getNoiseDbmVx();

  // if required, set equal bias in both channels
  // use Hc bias as the master

  if (_equalBiasInAllChannels) {
    _noiseBiasDbVx = _noiseBiasDbHc;
    _medianNoiseDbmVx = _calib.getNoiseDbmVx() + _noiseBiasDbVx;
  }

  // set noise moment for each gate

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hc = _noiseBiasDbHc;
    mfield.noise_bias_db_vx = _noiseBiasDbVx;
  }
  
}

///////////////////////////////////////////////////////////////
// Identify the noise, compute median noise
// Dual pol, V-transmit only
//
// The following must be set in mfields prior to calling:
//   lag0_vc_db
//   lag0_hx_db
  
void NoiseLocator::computeNoiseDpVOnly(MomentsFields *mfields)
  
{

  // initialize

  _medianNoiseDbmHx = _calib.getNoiseDbmHx();
  _medianNoiseDbmVc = _calib.getNoiseDbmVc();

  // locate the noise gates
  
  locate(mfields);
  
  if (_computeMethod == RAY_BY_RAY) {
  
    // compute median noise for ray
    
    vector<double> noiseHx;
    vector<double> noiseVc;
    for (int igate = 0; igate < _nGates; igate++) {
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        noiseHx.push_back(mfield.lag0_hx_db);
        noiseVc.push_back(mfield.lag0_vc_db);
      }
    }
    
    // set mean noise if noise was present at a large enough number
    // of gates, otherwise use the calibrated noise
    
    if ((int) noiseVc.size() >= _minNGatesRayMedian) {
      
      _medianNoiseDbmVc = _computeMedian(noiseVc);
      _medianNoiseDbmHx = _computeMedian(noiseHx);
      
      // save the data in a grid
      
      pthread_mutex_lock(&_prevGridMutex);
      noise_val_t &nval = _previousGrid[_gridIndexEl][_gridIndexAz];
      nval.noiseVc = _medianNoiseDbmVc;
      nval.noiseHx = _medianNoiseDbmHx;
      pthread_mutex_unlock(&_prevGridMutex);
      
    } else {
      
      // check for previously saved data
      
      noise_val_t prev;
      if (_getSavedNoiseClosestVc(prev) == 0) {
        // use previously saved data
        _medianNoiseDbmVc = prev.noiseVc;
        _medianNoiseDbmHx = prev.noiseHx;
      }
      
    }

  } else {
    
    // running MEDIAN method
    
    pthread_mutex_lock(&_runningMedianMutex);

    for (int igate = 0; igate < _nGates; igate++) {
      // add valid gates to the arrays
      const MomentsFields &mfield = mfields[igate];
      if (_noiseFlag[igate]) {
        _runningValsDbmVc[_nGatesRunningCount] = mfield.lag0_vc_db;
        _runningValsDbmHx[_nGatesRunningCount] = mfield.lag0_hx_db;
        _nGatesRunningCount++;
      }
      if (_nGatesRunningCount == _nGatesRunningMedian) {
        // if reached correct size, compute the medians
        _runningMedianNoiseDbmVc = _computeMedian(_runningValsDbmVc);
        _runningMedianNoiseDbmHx = _computeMedian(_runningValsDbmHx);
        _nGatesRunningCount = 0;
      }
    } // igate
    
    // set the mean to the latest median

    if (_runningMedianNoiseDbmVc > -9990) {
      _medianNoiseDbmVc = _runningMedianNoiseDbmVc;
    }
    if (_runningMedianNoiseDbmHx > -9990) {
      _medianNoiseDbmHx = _runningMedianNoiseDbmHx;
    }

    pthread_mutex_unlock(&_runningMedianMutex);

  }

  // set the noise bias in the moments

  _noiseBiasDbHx = _medianNoiseDbmHx - _calib.getNoiseDbmHx();
  _noiseBiasDbVc = _medianNoiseDbmVc - _calib.getNoiseDbmVc();
  
  // if required, set equal bias in both channels
  // use Vc bias as the master

  if (_equalBiasInAllChannels) {
    _noiseBiasDbHx = _noiseBiasDbVc;
    _medianNoiseDbmHx = _calib.getNoiseDbmHx() + _noiseBiasDbHx;
  }

  // set noise moment for each gate

  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_bias_db_hx = _noiseBiasDbHx;
    mfield.noise_bias_db_vc = _noiseBiasDbVc;
  }
  
}

///////////////////////////////////////////////////////////////
// Compute mean noise, removing outliers
  
double NoiseLocator::_computeMean(const vector<double> &vals)
  
{

  if (vals.size() < 1) {
    return -9999;
  }

  // compute mean and standard deviation

  double sum = 0.0;
  double sumsq = 0.0;
  double count = 0.0;
  for (size_t ii = 0; ii < vals.size(); ii++) {
    double val = vals[ii];
    sum += val;
    sumsq += val * val;
    count++;
  }

  double mean = sum / count;

  if (count < 5) {
    return mean;
  }
  
  double fac = (sumsq - (sum * sum) / count) / (count - 1.0);
  double sdev = 0.0;
  if (fac >= 0.0) {
    sdev = sqrt(fac);
  }

  // recompute mean using only vals with 2 * sdev of mean

  sum = 0.0;
  count = 0.0;
  for (size_t ii = 0; ii < vals.size(); ii++) {
    double val = vals[ii];
    if (fabs(val - mean) < sdev * 5.0) {
      sum += val;
      count++;
    }
  }

  if (count > 0) {
    mean = sum / count;
  }

  return mean;

}

///////////////////////////////////////////////////////////////
// Compute median noise
  
double NoiseLocator::_computeMedian(const vector<double> &vals)

{

  if (vals.size() < 1) {
    return -9999;
  }
  if (vals.size() < 2) {
    return vals[0];
  }

  vector<double> tmp(vals);
  sort(tmp.begin(), tmp.end());
  return tmp[tmp.size() / 2];

}

///////////////////////////////////////////////////////////////
// Get saved noise power closest to current point
// and within 1 degree.
//
// Assumes Hc is active.
//
// Returns 0 on success, -1 on failure
  
int NoiseLocator::_getSavedNoiseClosestHc(noise_val_t &closest)
  
{

  // check neighboring locations, starting close and
  // going out to 2 rows/cols away

  for (int ii = 0; ii < 25; ii++) {
    
    int iel = _gridIndexEl + _searchKernel[ii].iy;
    if (iel < 0) iel += _gridSizeEl;
    if (iel >= _gridSizeEl) iel -= _gridSizeEl;
    
    int iaz = _gridIndexAz + _searchKernel[ii].iy;
    if (iaz < 0) iaz += _gridSizeAz;
    if (iaz >= _gridSizeAz) iaz -= _gridSizeAz;
    
    pthread_mutex_lock(&_prevGridMutex);
    noise_val_t prev = _previousGrid[iel][iaz];
    pthread_mutex_unlock(&_prevGridMutex);
    if (prev.noiseHc != 0) {
      closest = prev;
      return 0;
    }

  }

  // previous noise not found

  return -1;

}

///////////////////////////////////////////////////////////////
// Get saved noise power closest to current point
// and within 1 degree.
//
// Assumes Vc is active.
//
// Returns 0 on success, -1 on failure
  
int NoiseLocator::_getSavedNoiseClosestVc(noise_val_t &closest)
  
{

  // check neighboring locations, starting close and
  // going out to 2 rows/cols away

  for (int ii = 0; ii < 25; ii++) {
    
    int iel = _gridIndexEl + _searchKernel[ii].iy;
    if (iel < 0) iel += _gridSizeEl;
    if (iel >= _gridSizeEl) iel -= _gridSizeEl;
    
    int iaz = _gridIndexAz + _searchKernel[ii].iy;
    if (iaz < 0) iaz += _gridSizeAz;
    if (iaz >= _gridSizeAz) iaz -= _gridSizeAz;
    
    pthread_mutex_lock(&_prevGridMutex);
    const noise_val_t prev = _previousGrid[iel][iaz];
    pthread_mutex_unlock(&_prevGridMutex);
    if (prev.noiseVc != 0) {
      closest = prev;
      return 0;
    }

  }

  // previous noise not found

  return -1;

}

///////////////////////////////////////////////////////////////
// Create the default interest maps and weights

void NoiseLocator::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;

  pts.clear();
  pts.push_back(InterestMap::ImPoint(40.0, 0.001));
  pts.push_back(InterestMap::ImPoint(50.0, 1.0));
  setInterestMapPhaseChangeErrorForNoise(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.65, 1.0));
  pts.push_back(InterestMap::ImPoint(0.75, 0.001));
  setInterestMapDbmSdevForNoise(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.10, 1.0));
  pts.push_back(InterestMap::ImPoint(0.20, 0.001));
  setInterestMapNcpMeanForNoise(pts, 1.0);

  setInterestThresholdForNoise(0.51);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(10.0, 0.001));
  pts.push_back(InterestMap::ImPoint(20.0, 1.0));
  setInterestMapPhaseChangeErrorForSignal(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.75, 1.0));
  pts.push_back(InterestMap::ImPoint(0.85, 0.001));
  setInterestMapDbmSdevForSignal(pts, 1.0);

  setInterestThresholdForSignal(0.51);

}

///////////////////////////////////////////////////////////////
// interest maps for noise

void NoiseLocator::setInterestMapPhaseChangeErrorForNoise
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapPhaseChangeErrorForNoise) {
    delete _interestMapPhaseChangeErrorForNoise;
  }

  _interestMapPhaseChangeErrorForNoise = new InterestMap
    ("PhaseChangeErrorForNoise", pts, weight);

  _weightPhaseChangeErrorForNoise = weight;

}

void NoiseLocator::setInterestMapDbmSdevForNoise
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapDbmSdevForNoise) {
    delete _interestMapDbmSdevForNoise;
  }

  _interestMapDbmSdevForNoise = new InterestMap
    ("DbmSdevForNoise", pts, weight);

  _weightDbmSdevForNoise = weight;

}

void NoiseLocator::setInterestMapNcpMeanForNoise
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapNcpMeanForNoise) {
    delete _interestMapNcpMeanForNoise;
  }

  _interestMapNcpMeanForNoise = new InterestMap
    ("NcpMeanForNoise", pts, weight);

  _weightNcpMeanForNoise = weight;

}

void NoiseLocator::setInterestThresholdForNoise(double val)
  
{
  _interestThresholdForNoise = val;
}

///////////////////////////////////////////////////////////////
// interest maps for signal

void NoiseLocator::setInterestMapPhaseChangeErrorForSignal
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapPhaseChangeErrorForSignal) {
    delete _interestMapPhaseChangeErrorForSignal;
  }

  _interestMapPhaseChangeErrorForSignal = new InterestMap
    ("PhaseChangeErrorForSignal", pts, weight);

  _weightPhaseChangeErrorForSignal = weight;

}

void NoiseLocator::setInterestMapDbmSdevForSignal
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapDbmSdevForSignal) {
    delete _interestMapDbmSdevForSignal;
  }

  _interestMapDbmSdevForSignal = new InterestMap
    ("DbmSdevForSignal", pts, weight);

  _weightDbmSdevForSignal = weight;
  
}

void NoiseLocator::setInterestThresholdForSignal(double val)
  
{
  _interestThresholdForSignal = val;
}

//////////////////////////////////////////
// add the noise fields to a moments array

void NoiseLocator::addToMoments(MomentsFields *mfields)
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {
    MomentsFields &mfield = mfields[igate];
    mfield.noise_flag = _noiseFlag[igate];
    mfield.signal_flag = _signalFlag[igate];
    mfield.accum_phase_change = _accumPhaseChange[igate];
    mfield.phase_change_error = _phaseChangeError[igate];
    mfield.dbm_sdev = _dbmSdev[igate];
    mfield.ncp_mean = _ncpMean[igate];
  }
  
}

