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
// IntfLocator.cc
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

#include <radar/IntfLocator.hh>
#include <radar/RadarComplex.hh>
#include <algorithm>
#include <ostream>
#include <cstring>
using namespace std;

///////////////////////////////////////////////////////////////
// Constructor

IntfLocator::IntfLocator()
  
{

  _debug = false;

  // create the default interest maps

  _interestMapPhaseChangeErrorForRlan = NULL;
  _interestMapSnrDModeForRlan = NULL;
  _interestMapNcpMeanForRlan = NULL;
  _createDefaultInterestMaps();
  _nGatesKernel = 9;

}

///////////////////////////////////////////////////////////////
// destructor

IntfLocator::~IntfLocator()
  
{

  if (_interestMapPhaseChangeErrorForRlan) {
    delete _interestMapPhaseChangeErrorForRlan;
  }

  if (_interestMapSnrDModeForRlan) {
    delete _interestMapSnrDModeForRlan;
  }

  if (_interestMapNcpMeanForRlan) {
    delete _interestMapNcpMeanForRlan;
  }

}

///////////////////////////////////////////////////////////////
// print parameters for debugging

void IntfLocator::printParams(ostream &out)
  
{

  out << "Performing rlan detection:" << endl;
  out << "  nGatesKernel: " << _nGatesKernel << endl;
  out << "  interestThreshold: " << _interestThreshold << endl;
  
  if (_interestMapPhaseChangeErrorForRlan) {
    _interestMapPhaseChangeErrorForRlan->printParams(out);
  }
  if (_interestMapSnrDModeForRlan) {
    _interestMapSnrDModeForRlan->printParams(out);
  }
  if (_interestMapNcpMeanForRlan) {
    _interestMapNcpMeanForRlan->printParams(out);
  }

}

///////////////////////////////////////////////////////////////
// set the ray properties
// must be called before locate()

void IntfLocator::setRayProps(time_t timeSecs, 
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

void IntfLocator::setFields(double *snr,
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
    for (int igate = 0; igate < _nGates; igate++) {
      if (_snr[igate] < -10) {
        _snr[igate] = missingVal;
      }
    }
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
        if (vel[ii] != missingVal) {
          double absVel = fabs(vel[ii]);
          if (_nyquist < absVel) {
            _nyquist = absVel;
          }
        } // if (vel[ii] != missingVal)
      } // ii
    }
    // estimate the phase from the vel
    for (int ii = 0; ii < _nGates; ii++) {
      _phase[ii] = (_vel[ii] / _nyquist) * 180.0;
    }
    _phaseAvail = true;
  } else {
    _phaseAvail = false;
  }

}

///////////////////////////////////////////////////////////////
// locate the rlan gates

void IntfLocator::locate()
  
{

  _startGate.resize(_nGates);
  _endGate.resize(_nGates);
  
  _rlanFlag = _rlanFlag_.alloc(_nGates);
  _accumPhaseChange = _accumPhaseChange_.alloc(_nGates);
  _phaseChangeError = _phaseChangeError_.alloc(_nGates);
  _snrMode = _snrMode_.alloc(_nGates);
  _snrDMode = _snrDMode_.alloc(_nGates);
  _zdrMode = _zdrMode_.alloc(_nGates);
  _zdrDMode = _zdrDMode_.alloc(_nGates);
  _ncpMean = _ncpMean_.alloc(_nGates);
  
  for (int igate = 0; igate < _nGates; igate++) {
    _rlanFlag[igate] = false;
    _startGate[igate] = 0;
    _endGate[igate] = 0;
    _accumPhaseChange[igate] = -9999;
    _phaseChangeError[igate] = -9999;
    _snrMode[igate] = -9999;
    _snrDMode[igate] = -9999;
    _zdrMode[igate] = -9999;
    _zdrDMode[igate] = -9999;
    _ncpMean[igate] = -9999;
  }
  
  // first compute the absolute phase at each gate, summing up
  // the phase change from the start to end of the ray
  // the units are degrees

  if (_phaseAvail) {
    double prevPhaseSum = _phase[0];
    for (int igate = 1; igate < _nGates; igate++) {
      double diffDeg = RadarComplex::diffDeg(_phase[igate], _phase[igate-1]);
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
    _computeDeltaMode("SNR", _snr, _snrMode, _snrDMode);
  }
  if (_zdrAvail) {
    _computeDeltaMode("ZDR", _zdr, _zdrMode, _zdrDMode);
  }
  if (_ncpAvail) {
    _computeMeanInRange(_ncp, _ncpMean);
  }

  // set flags
  
  double sumWeightsRlan = (_weightPhaseChangeErrorForRlan + 
                           _weightSnrDModeForRlan +
                           _weightNcpMeanForRlan);
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    double pce = _phaseChangeError[igate];
    double sndDMode = _snrMode[igate];
    double ncpMean = _ncpMean[igate];

    double sumInterestRlan =
      (_interestMapPhaseChangeErrorForRlan->getInterest(pce) * 
       _weightPhaseChangeErrorForRlan) +
      (_interestMapSnrDModeForRlan->getInterest(sndDMode) * 
       _weightSnrDModeForRlan) +
      (_interestMapNcpMeanForRlan->getInterest(ncpMean) * 
       _weightNcpMeanForRlan);
    
    double interestRlan = sumInterestRlan / sumWeightsRlan;

    if (interestRlan > _interestThreshold) {
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

double IntfLocator::_computePhaseChangeError(int startGate, int endGate)
  
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
// compute Delta relative to MODE for ray

void IntfLocator::_computeDeltaMode(const string &fieldName,
                                    const double *vals, 
                                    double *mode, double *dMode)
  
{
  
  // initialize
  
  for (int igate = 0; igate < _nGates; igate++) {
    mode[igate] = _missingVal;
    dMode[igate] = _missingVal;
  }

  // compute min and max
  
  double nValid = 0.0;
  double minVal = 1.0e99;
  double maxVal = -1.0e99;
  for (int igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (!isfinite(val)) {
      return;
    }
    if (val != _missingVal) {
      nValid++;
      if (val < minVal) {
        minVal = val;
      }
      if (val > maxVal) {
        maxVal = val;
      }
    }
  } // igate
  
  if (nValid < _nGates / 4) {
    return;
  }
  if (maxVal <= minVal) {
    return;
  }

  // create histogram with 100 bins

  int nBins = 100;
  double histDelta = (maxVal - minVal) / (double) (nBins - 1);
  TaArray<double> hist_;
  double *hist = hist_.alloc(nBins);
  memset(hist, 0, nBins * sizeof(double));
  for (int igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (val != _missingVal) {
      int bin = (int) ((val - minVal) / histDelta);
      hist[bin]++;
    }
  } // igate

  // find mode bin
  
  int modeBin = 0;
  double maxCount = 0;
  for (int ibin = 0; ibin < nBins; ibin++) {
    double count = hist[ibin];
    if (count > maxCount) {
      maxCount = count;
      modeBin = ibin;
    }
  }

  // find median
  
  // double sumCount = 0.0;
  // double half = nValid / 2.0;
  // double median = (maxVal - minVal) / 2.0;
  // for (int ibin = 0; ibin < nBins; ibin++) {
  //   double count = hist[ibin];
  //   if (sumCount + count > half) {
  //     double frac = (half - sumCount) / count;
  //     median = ((double) ibin + frac) * histDelta + minVal;
  //     break;
  //   }
  //   sumCount += count;
  // } // ibin

  // compute mean for vals within that bin and one bin on each side

  double modeLower = minVal + (modeBin - 1) * histDelta;
  double modeUpper = minVal + (modeBin + 2) * histDelta;
  double sum = 0.0;
  double count = 0.0;

  for (int igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (val != _missingVal && val >= modeLower && val <= modeUpper) {
      sum += val;
      count++;
    }
  } // igate
  double modeVal = sum / count;

  // vector<double> vvals;
  // for (int igate = 0; igate < _nGates; igate++) {
  //   double val = vals[igate];
  //   if (val != _missingVal) {
  //     vvals.push_back(val);
  //   }
  // }
  // sort(vvals.begin(), vvals.end());
  // double mmedian = vvals[vvals.size()/2];

  // set delta from mode

  for (int igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (val != _missingVal) {
      dMode[igate] = fabs(val - modeVal);
      // dMode[igate] = val - mmedian;
    } else {
      dMode[igate] = _missingVal;
    }
    mode[igate] = modeVal;
    // mode[igate] = mmedian;
  } // igate
  
}

///////////////////////////////////////////////////////////////
// compute SDEV in range

void IntfLocator::_computeSdevInRange(const double *vals, double *sdevs)
  
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

void IntfLocator::_computeMeanInRange(const double *vals, double *means)
  
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
  
double IntfLocator::_computeMean(const vector<double> &vals)
  
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
  
double IntfLocator::_computeMedian(const vector<double> &vals)

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

void IntfLocator::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;

  pts.clear();
  pts.push_back(InterestMap::ImPoint(40.0, 0.001));
  pts.push_back(InterestMap::ImPoint(50.0, 1.0));
  setInterestMapPhaseChangeErrorForRlan(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(2.0, 1.0));
  pts.push_back(InterestMap::ImPoint(2.5, 0.001));
  setInterestMapSnrDModeForRlan(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.15, 1.0));
  pts.push_back(InterestMap::ImPoint(0.20, 0.001));
  setInterestMapNcpMeanForRlan(pts, 1.0);

  setInterestThreshold(0.51);

}

///////////////////////////////////////////////////////////////
// interest maps for rlan

void IntfLocator::setInterestMapPhaseChangeErrorForRlan
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

void IntfLocator::setInterestMapSnrDModeForRlan
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{

  if (_interestMapSnrDModeForRlan) {
    delete _interestMapSnrDModeForRlan;
  }

  _interestMapSnrDModeForRlan = new InterestMap
    ("SnrDModeForRlan", pts, weight);

  _weightSnrDModeForRlan = weight;

}

void IntfLocator::setInterestMapNcpMeanForRlan
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

void IntfLocator::setInterestThreshold(double val)
  
{
  _interestThreshold = val;
}

