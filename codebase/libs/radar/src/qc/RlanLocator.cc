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
// RlanLocator.cc
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

#include <radar/RlanLocator.hh>
#include <radar/RadarComplex.hh>
#include <algorithm>
#include <ostream>
#include <cstring>
using namespace std;

///////////////////////////////////////////////////////////////
// Constructor

RlanLocator::RlanLocator()
  
{

  _debug = false;

  // create the default interest maps

  _interestMapPhaseChangeErrorForRlan = NULL;
  _interestMapSnrSdevForRlan = NULL;
  _interestMapNcpMeanForRlan = NULL;

  _createDefaultInterestMaps();
  
  _nGatesKernel = 9;
  _minNGatesRayMedian = 30;

}

///////////////////////////////////////////////////////////////
// destructor

RlanLocator::~RlanLocator()
  
{

  if (_interestMapPhaseChangeErrorForRlan) {
    delete _interestMapPhaseChangeErrorForRlan;
  }
  if (_interestMapSnrSdevForRlan) {
    delete _interestMapSnrSdevForRlan;
  }
  if (_interestMapNcpMeanForRlan) {
    delete _interestMapNcpMeanForRlan;
  }

}

///////////////////////////////////////////////////////////////
// set min ngates for computing median

void RlanLocator::setMinNGatesRayMedian(int minNGatesRayMedian)
{
  
  _minNGatesRayMedian = minNGatesRayMedian;

}

///////////////////////////////////////////////////////////////
// print parameters for debugging

void RlanLocator::printParams(ostream &out)
  
{

  out << "Performing rlan detection:" << endl;
  out << "  nGatesForRlanDetection: " << _nGatesKernel << endl;
  out << "  minNGatesRayMedian: " << _minNGatesRayMedian << endl;
  out << "  interestThresholdForRlan: " 
      << _interestThresholdForRlan << endl;
  
  if (_interestMapPhaseChangeErrorForRlan) {
    _interestMapPhaseChangeErrorForRlan->printParams(out);
  }
  if (_interestMapSnrSdevForRlan) {
    _interestMapSnrSdevForRlan->printParams(out);
  }
  if (_interestMapNcpMeanForRlan) {
    _interestMapNcpMeanForRlan->printParams(out);
  }

}

///////////////////////////////////////////////////////////////
// set the ray properties
// must be called before locate()

void RlanLocator::setRayProps(time_t timeSecs, 
                              double nanoSecs,
                              double elevation, 
                              double azimuth,
                              int nGates,
                              double startRangeKm,
                              double gateSpacingKm,
                              double wavelengthM,
                              double nyquist /* = -9999.0 */)
                              
{

  _timeSecs = timeSecs;
  _nanoSecs = nanoSecs;
  _elevation = elevation;
  _azimuth = azimuth;

  _nGates = nGates;
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  _wavelengthM = wavelengthM;
  _nyquist = nyquist;

  _snr = _snr_.alloc(_nGates);
  _vel = _vel_.alloc(_nGates);
  _phase = _phase_.alloc(_nGates);
  _width = _width_.alloc(_nGates);
  _ncp = _ncp_.alloc(_nGates);
  _zdr = _zdr_.alloc(_nGates);

}

///////////////////////////////////////////////////////////////
// set the available fields
// if field is not available, set to NULL
// must be called before locate()

void RlanLocator::setFields(double *snr,
                            double *vel,
                            double *width,
                            double *ncp,
                            double *zdr,
                            double missingVal)
  
{

  _missingVal = missingVal;

  if (snr != NULL) {
    memcpy(_snr, snr, _nGates * sizeof(double));
    _snrAvail = true;
  } else {
    _snrAvail = false;
  }

  if (vel != NULL) {
    memcpy(_vel, vel, _nGates * sizeof(double));
    _velAvail = true;
  } else {
    _velAvail = false;
  }

  if (width != NULL) {
    memcpy(_width, width, _nGates * sizeof(double));
    _widthAvail = true;
  } else {
    _widthAvail = false;
  }

  if (ncp != NULL) {
    memcpy(_ncp, ncp, _nGates * sizeof(double));
    _ncpAvail = true;
  } else {
    _ncpAvail = false;
  }

  if (zdr != NULL) {
    memcpy(_zdr, zdr, _nGates * sizeof(double));
    _zdrAvail = true;
  } else {
    _zdrAvail = false;
  }

  // compute phase from velocity
  
  if (_velAvail) {
    if (_nyquist < -9990) {
      // estimate the nyquist from the vel
      for (int ii = 0; ii < _nGates; ii++) {
        double absVel = fabs(vel[ii]);
        if (_nyquist < absVel) {
          _nyquist = absVel;
        }
      }
    }
    // estimate the phase from the vel
    for (int ii = 0; ii < _nGates; ii++) {
      _phase[ii] = (_vel[ii] / _nyquist) * M_PI;
    }
    _phaseAvail = true;
  } else {
    _phaseAvail = false;
  }

}

///////////////////////////////////////////////////////////////
// locate the rlan gates

void RlanLocator::locate()
  
{

  _startGate.resize(_nGates);
  _endGate.resize(_nGates);
  
  _rlanFlag = _rlanFlag_.alloc(_nGates);
  _accumPhaseChange = _accumPhaseChange_.alloc(_nGates);
  _phaseChangeError = _phaseChangeError_.alloc(_nGates);
  _snrSdev = _snrSdev_.alloc(_nGates);
  _zdrSdev = _zdrSdev_.alloc(_nGates);
  _ncpMean = _ncpMean_.alloc(_nGates);
  
  for (int igate = 0; igate < _nGates; igate++) {
    _rlanFlag[igate] = false;
    _startGate[igate] = 0;
    _endGate[igate] = 0;
    _accumPhaseChange[igate] = -9999;
    _phaseChangeError[igate] = -9999;
    _snrSdev[igate] = -9999;
    _zdrSdev[igate] = -9999;
    _ncpMean[igate] = -9999;
  }
  
  // first compute the absolute phase at each gate, summing up
  // the phase change from the start to end of the ray
  // the units are degrees

  if (_phaseAvail) {
    double prevPhaseSum = RadarComplex::argDeg(_phase[0]);
    for (int igate = 1; igate < _nGates; igate++) {
      RadarComplex_t diff =
        RadarComplex::conjugateProduct(_phase[igate], _phase[igate-1]);
      double diffDeg = RadarComplex::argDeg(diff);
      double phaseSum = prevPhaseSum + diffDeg;
      _accumPhaseChange[igate] = phaseSum;
      prevPhaseSum = phaseSum;
    }
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
  
  // compute snr sdev

  if (_snrAvail) {
    _computeSdevInRange(_snr, _snrSdev);
  }
  if (_zdrAvail) {
    _computeSdevInRange(_zdr, _zdrSdev);
  }
  if (_ncpAvail) {
    _computeMeanInRange(_ncp, _ncpMean);
  }

  // set flags

  double sumWeightsRlan = (_weightPhaseChangeErrorForRlan + 
                            _weightSnrSdevForRlan +
                            _weightNcpMeanForRlan);

  for (int igate = 0; igate < _nGates; igate++) {

    double pce = _phaseChangeError[igate];
    double snrSdev = _snrSdev[igate];
    double ncpMean = _ncpMean[igate];

    double sumInterestRlan =
      (_interestMapPhaseChangeErrorForRlan->getInterest(pce) * 
       _weightPhaseChangeErrorForRlan) +
      (_interestMapSnrSdevForRlan->getInterest(snrSdev) * 
       _weightSnrSdevForRlan) +
      (_interestMapNcpMeanForRlan->getInterest(ncpMean) * 
       _weightNcpMeanForRlan);
    
    double interestRlan = sumInterestRlan / sumWeightsRlan;

    if (interestRlan > _interestThresholdForRlan) {
      _rlanFlag[igate] = true;
    } else {
      _rlanFlag[igate] = false;
    }

  } // igate

  // for single gates surrounded by rlan, set the rlan flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (_rlanFlag[igate-1] && _rlanFlag[igate+1]) {
      _rlanFlag[igate] = true;
    }
  }
 
  // for single gates surrounded by non-rlan, unset the rlan flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (!_rlanFlag[igate-1] && !_rlanFlag[igate+1]) {
      _rlanFlag[igate] = false;
    }
  }
 
}

///////////////////////////////////////////////////////////////
// compute mean phase error in range, for the specified kernel

double RlanLocator::_computePhaseChangeError(int startGate, int endGate)
  
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
// compute SDEV in range

void RlanLocator::_computeSdevInRange(const double *vals, double *sdevs)
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nVal = 0.0;
    double sumVal = 0.0;
    double sumValSq = 0.0;
    
    for (size_t jgate = _startGate[igate]; jgate <= _endGate[igate]; jgate++) {
      
      double val = vals[jgate];
      
      if (val != _missingVal) {
        sumVal += val;
        sumValSq += (val * val);
        nVal++;
      }
      
    } // jgate
    
    if (nVal > 0) {
      double meanVal = sumVal / nVal;
      if (nVal > 2) {
        double term1 = sumValSq / nVal;
        double term2 = meanVal * meanVal;
        if (term1 >= term2) {
          sdevs[igate] = sqrt(term1 - term2);
        }
      }
    }
    
  } // igate

}

///////////////////////////////////////////////////////////////
// compute MEAN in range

void RlanLocator::_computeMeanInRange(const double *vals, double *means)
  
{
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    // compute sums etc. for stats over the kernel space
    
    double nVal = 0.0;
    double sumVal = 0.0;
    
    for (size_t jgate = _startGate[igate]; jgate <= _endGate[igate]; jgate++) {
      
      double val = vals[jgate];
      
      if (val != _missingVal) {
        sumVal += val;
        nVal++;
      }
      
    } // jgate

    if (nVal > 0) {
      double meanVal = sumVal / nVal;
      means[igate] = meanVal;
    }
    
  } // igate

}

///////////////////////////////////////////////////////////////
// Compute mean rlan, removing outliers
  
double RlanLocator::_computeMean(const vector<double> &vals)
  
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
// Compute median rlan
  
double RlanLocator::_computeMedian(const vector<double> &vals)

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
// Create the default interest maps and weights

void RlanLocator::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;

  pts.clear();
  pts.push_back(InterestMap::ImPoint(40.0, 0.001));
  pts.push_back(InterestMap::ImPoint(50.0, 1.0));
  setInterestMapPhaseChangeErrorForRlan(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.65, 1.0));
  pts.push_back(InterestMap::ImPoint(0.75, 0.001));
  setInterestMapSnrSdevForRlan(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.10, 1.0));
  pts.push_back(InterestMap::ImPoint(0.20, 0.001));
  setInterestMapNcpMeanForRlan(pts, 1.0);

  setInterestThresholdForRlan(0.51);

}

///////////////////////////////////////////////////////////////
// interest maps for rlan

void RlanLocator::setInterestMapPhaseChangeErrorForRlan
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapPhaseChangeErrorForRlan) {
    delete _interestMapPhaseChangeErrorForRlan;
  }

  _interestMapPhaseChangeErrorForRlan = new InterestMap
    ("PhaseChangeErrorForRlan", pts, weight);

  _weightPhaseChangeErrorForRlan = weight;

}

void RlanLocator::setInterestMapSnrSdevForRlan
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapSnrSdevForRlan) {
    delete _interestMapSnrSdevForRlan;
  }

  _interestMapSnrSdevForRlan = new InterestMap
    ("SnrSdevForRlan", pts, weight);

  _weightSnrSdevForRlan = weight;

}

void RlanLocator::setInterestMapNcpMeanForRlan
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapNcpMeanForRlan) {
    delete _interestMapNcpMeanForRlan;
  }

  _interestMapNcpMeanForRlan = new InterestMap
    ("NcpMeanForRlan", pts, weight);

  _weightNcpMeanForRlan = weight;

}

void RlanLocator::setInterestThresholdForRlan(double val)
  
{
  _interestThresholdForRlan = val;
}

