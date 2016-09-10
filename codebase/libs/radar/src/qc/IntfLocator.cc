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
// Sept 2016
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

  _interestMapPhaseNoise = NULL;
  _interestMapNcpMean = NULL;
  _interestMapWidthMean = NULL;
  _interestMapSnrDMode = NULL;
  _interestMapSnrSdev = NULL;

  _createDefaultInterestMaps();
  _nGatesKernel = 9;
  
  _radarHtM = -9999.0;
  _wavelengthM = -9999.0;
  _nyquist = -9999.0;

}

///////////////////////////////////////////////////////////////
// destructor

IntfLocator::~IntfLocator()
  
{

  if (_interestMapPhaseNoise) {
    delete _interestMapPhaseNoise;
  }

  if (_interestMapNcpMean) {
    delete _interestMapNcpMean;
  }

  if (_interestMapWidthMean) {
    delete _interestMapWidthMean;
  }

  if (_interestMapSnrDMode) {
    delete _interestMapSnrDMode;
  }

  if (_interestMapSnrSdev) {
    delete _interestMapSnrSdev;
  }

}

///////////////////////////////////////////////////////////////
// print parameters for debugging

void IntfLocator::printParams(ostream &out)
  
{

  out << "Performing rlan detection:" << endl;
  out << "  nGatesKernel: " << _nGatesKernel << endl;
  out << "  rlanInterestThreshold: " << _rlanInterestThreshold << endl;
  out << "  noiseInterestThreshold: " << _noiseInterestThreshold << endl;
  
  if (_interestMapPhaseNoise) {
    _interestMapPhaseNoise->printParams(out);
  }
  if (_interestMapNcpMean) {
    _interestMapNcpMean->printParams(out);
  }
  if (_interestMapWidthMean) {
    _interestMapWidthMean->printParams(out);
  }
  if (_interestMapSnrDMode) {
    _interestMapSnrDMode->printParams(out);
  }
  if (_interestMapSnrSdev) {
    _interestMapSnrSdev->printParams(out);
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
                              double gateSpacingKm)
  
{
  
  _timeSecs = timeSecs;
  _nanoSecs = nanoSecs;
  _elevation = elevation;
  _azimuth = azimuth;

  _nGates = nGates;
  _startRangeKm = startRangeKm;
  _gateSpacingKm = gateSpacingKm;

  _dbzAvail = false;
  _velAvail = false;
  _phaseAvail = false;
  _widthAvail = false;
  _ncpAvail = false;
  _snrAvail = false;

  _dbz = _dbz_.alloc(_nGates);
  _vel = _vel_.alloc(_nGates);
  _phase = _phase_.alloc(_nGates);
  _width = _width_.alloc(_nGates);
  _ncp = _ncp_.alloc(_nGates);
  _snr = _snr_.alloc(_nGates);

}

///////////////////////////////////////////////////////////////
// set the DBZ field, if no SNR available
// SNR will be estimated from DBZ
// must be called after setRayProps()

void IntfLocator::setDbzField(double *vals,
                              double noiseDbzAt100km)
{

  // set DBZ field

  memcpy(_dbz, vals, _nGates * sizeof(double));
  _dbzAvail = true;

  // compute SNR from DBZ

  _computeSnrFromDbz(noiseDbzAt100km);
  _snrAvail = true;

}

///////////////////////////////////////////////////////////////
// set the VEL field
// phase will be estimated from VEL and nyquist
// must be called after setRayProps()

void IntfLocator::setVelField(double *vals,
                              double nyquist  /* = -9999.0 */)
{

  memcpy(_vel, vals, _nGates * sizeof(double));
  _velAvail = true;
  
  // estimate the nyquist if it has not been set
  
  if (_nyquist < -9990) {
    for (int ii = 0; ii < _nGates; ii++) {
      if (_vel[ii] != _missingVal) {
        double absVel = fabs(_vel[ii]);
        if (_nyquist < absVel) {
          _nyquist = absVel;
        }
      } // if (vel[ii] != _missingVal)
    } // ii
  }
  
  // estimate the phase from the vel

  for (int ii = 0; ii < _nGates; ii++) {
    if (_vel[ii] != _missingVal) {
      _phase[ii] = (_vel[ii] / _nyquist) * 180.0;
    } else {
      _phase[ii] = _missingVal;
    }
  }

  if (_nyquist > 0) {
    _phaseAvail = true;
  }

}

///////////////////////////////////////////////////////////////
// set the DBZ field
// must be called after setRayProps()

void IntfLocator::setNcpField(double *vals)
{
  memcpy(_ncp, vals, _nGates * sizeof(double));
  _ncpAvail = true;
}

///////////////////////////////////////////////////////////////
// set the WIDTH field
// width is used instead of NCP, if NCP not available
// must be called after setRayProps()

void IntfLocator::setWidthField(double *vals)
{
  memcpy(_width, vals, _nGates * sizeof(double));
  _widthAvail = true;
}

///////////////////////////////////////////////////////////////
// set the SNR field, if available
// must be called after setRayProps()

void IntfLocator::setSnrField(double *vals)
{
  memcpy(_snr, vals, _nGates * sizeof(double));
  _snrAvail = true;
}

//////////////////////////////////////////////////////////////
// Compute the SNR field from the DBZ field

void IntfLocator::_computeSnrFromDbz(double noiseDbzAt100km)

{

  // compute noise at each gate

  TaArray<double> noiseDbz_;
  double *noiseDbz = noiseDbz_.alloc(_nGates);
  double range = _startRangeKm;
  if (range == 0) {
    range = _gateSpacingKm / 10.0;
  }
  for (int igate = 0; igate < _nGates; igate++, range += _gateSpacingKm) {
    noiseDbz[igate] = noiseDbzAt100km + 20.0 * (log10(range) - log10(100.0));
  }

  // compute snr from dbz
  
  double *snr = _snr;
  const double *dbz = _dbz;
  for (int igate = 0; igate < _nGates; igate++, snr++, dbz++) {
    if (*dbz != _missingVal) {
      *snr = *dbz - noiseDbz[igate];
    } else {
      *snr = -20;
    }
  }

}

///////////////////////////////////////////////////////////////
// locate the rlan gates
// also locates the noise-only gates
//
// Returns 0 on success, -1 on failure

int IntfLocator::rlanLocate()
  
{

  bool iret = 0;
  
  if (!_velAvail) {
    cerr << "ERROR - IntfLocator::rlanLocate()" << endl;
    cerr << "  VEL not available" << endl;
    cerr << "  Cannot locate RLAN interference without VEL field" << endl;
    iret = -1;
  }

  if (!_phaseAvail) {
    cerr << "ERROR - IntfLocator::rlanLocate()" << endl;
    cerr << "  Phase estimated from VEL not available, not enough vel gates" << endl;
    cerr << "  Cannot locate RLAN interference without phase field" << endl;
    iret = -1;
  }

  if (!_snrAvail) {
    cerr << "ERROR - IntfLocator::rlanLocate()" << endl;
    cerr << "  Neither SNR nor DBZ available" << endl;
    cerr << "  Cannot locate RLAN interference without measured SNR" << endl;
    cerr << "    or SNR estimated from DBZ" << endl;
    iret = -1;
  }

  if (!_ncpAvail && !_widthAvail) {
    cerr << "ERROR - IntfLocator::rlanLocate()" << endl;
    cerr << "  Neither NCP nor WIDTH is available" << endl;
    cerr << "  Cannot locate RLAN interference" << endl;
    iret = -1;
  }

  if (iret) {
    return -1;
  }

  _startGate.resize(_nGates);
  _endGate.resize(_nGates);
  
  _rlanFlag = _rlanFlag_.alloc(_nGates);
  _noiseFlag = _noiseFlag_.alloc(_nGates);
  _accumPhaseChange = _accumPhaseChange_.alloc(_nGates);
  _phaseNoise = _phaseNoise_.alloc(_nGates);
  _ncpMean = _ncpMean_.alloc(_nGates);
  _widthMean = _widthMean_.alloc(_nGates);
  _snrMode = _snrMode_.alloc(_nGates);
  _snrDMode = _snrDMode_.alloc(_nGates);
  _snrSdev = _snrSdev_.alloc(_nGates);

  _phaseNoiseInterest = _phaseNoiseInterest_.alloc(_nGates);
  _ncpMeanInterest = _ncpMeanInterest_.alloc(_nGates);
  _widthMeanInterest = _widthMeanInterest_.alloc(_nGates);
  _snrDModeInterest = _snrDModeInterest_.alloc(_nGates);
  _snrSdevInterest = _snrSdevInterest_.alloc(_nGates);
  
  for (int igate = 0; igate < _nGates; igate++) {
    _rlanFlag[igate] = false;
    _noiseFlag[igate] = false;
    _startGate[igate] = 0;
    _endGate[igate] = 0;
    _accumPhaseChange[igate] = -9999;
    _phaseNoise[igate] = -9999;
    _snrMode[igate] = -9999;
    _snrDMode[igate] = -9999;
    _snrSdev[igate] = -9999;
    _ncpMean[igate] = -9999;
    _widthMean[igate] = -9999;
    _phaseNoiseInterest[igate] = -9999;
    _ncpMeanInterest[igate] = -9999;
    _widthMeanInterest[igate] = -9999;
    _snrDModeInterest[igate] = -9999;
    _snrSdevInterest[igate] = -9999;
  }
  
  // first compute the absolute phase at each gate, summing up
  // the phase change from the start to end of the ray
  // the units are degrees

  double prevPhaseSum = _phase[0];
  for (int igate = 1; igate < _nGates; igate++) {
    double diffDeg = RadarComplex::diffDeg(_phase[igate], _phase[igate-1]);
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
    cerr << "ERROR - IntfLocator::rlanLocate()" << endl;
    cerr << "  nGates too small: " << _nGates << endl;
    cerr << "  nGates must exceed kernelSize: " << kernelSize << endl;
    return -1;
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

  // compute phase noisiness relative to a constant phase slope

  for (int igate = 0; igate < _nGates; igate++) {
    _phaseNoise[igate] =
      _computePhaseNoise(_startGate[igate], _endGate[igate]);
  }
  
  // compute snr sdev and delta mode
  
  _computeSdevInRange(_snr, _snrSdev);
  _computeDeltaMode(_snr, _snrMode, _snrDMode);

  if (_ncpAvail) {
    _computeMeanInRange(_ncp, _ncpMean);
  }
  if (_widthAvail) {
    _computeMeanInRange(_width, _widthMean);
  }

  // compute interest fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    _phaseNoiseInterest[igate] =
      _interestMapPhaseNoise->getInterest(_phaseNoise[igate]);
    
    _ncpMeanInterest[igate] =
      _interestMapNcpMean->getInterest(_ncpMean[igate]);
    
    _widthMeanInterest[igate] =
      _interestMapWidthMean->getInterest(_widthMean[igate]);

    _snrDModeInterest[igate] =
      _interestMapSnrDMode->getInterest(_snrDMode[igate]);

    _snrSdevInterest[igate] =
      _interestMapSnrSdev->getInterest(_snrSdev[igate]);

  }

  // compute sum weights

  double sumWeightsRlan = _weightPhaseNoise + _weightSnrDMode;
  double sumWeightsNoise = _weightPhaseNoise + _weightSnrSdev;
  if (_ncpAvail) {
    sumWeightsRlan += _weightNcpMean;
    sumWeightsNoise += _weightNcpMean;
  } else {
    sumWeightsRlan += _weightWidthMean;
    sumWeightsNoise += _weightWidthMean;
  }
  
  // set flags
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    double sumInterestRlan =
      (_phaseNoiseInterest[igate] * _weightPhaseNoise) +
      (_snrDModeInterest[igate] * _weightSnrDMode);

    double sumInterestNoise =
      (_phaseNoiseInterest[igate] * _weightPhaseNoise) +
      (_snrSdevInterest[igate] * _weightSnrSdev);

    if (_ncpAvail) {
      sumInterestRlan += (_ncpMeanInterest[igate] * _weightNcpMean);
      sumInterestNoise += (_ncpMeanInterest[igate] * _weightNcpMean);
    } else {
      sumInterestRlan += (_widthMeanInterest[igate] * _weightWidthMean);
      sumInterestNoise += (_widthMeanInterest[igate] * _weightWidthMean);
    }
    
    double interestRlan = sumInterestRlan / sumWeightsRlan;
    if (interestRlan > _rlanInterestThreshold) {
      _rlanFlag[igate] = true;
    } else {
      _rlanFlag[igate] = false;
    }

    double interestNoise = sumInterestNoise / sumWeightsNoise;
    if (interestNoise > _noiseInterestThreshold) {
      _noiseFlag[igate] = true;
    } else {
      _noiseFlag[igate] = false;
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

  return 0;
 
}

///////////////////////////////////////////////////////////////
// compute phase noisiness relative to a constant phase slope

double IntfLocator::_computePhaseNoise(int startGate, int endGate)
  
{

  double phaseStart = _accumPhaseChange[startGate];
  double phaseEnd = _accumPhaseChange[endGate];
  double dGates = endGate - startGate;
  double slope = (phaseEnd - phaseStart) / dGates;
  
  double linearPhase = phaseStart + slope;
  double sumAbsNoise = 0.0;
  double count = 0.0;
  
  for (int igate = startGate + 1; igate < endGate; 
       igate++, linearPhase += slope) {
    double phase = _accumPhaseChange[igate];
    double absNoise = fabs(phase - linearPhase);
    sumAbsNoise += absNoise;
    count++;
  }

  return sumAbsNoise / count;

}

///////////////////////////////////////////////////////////////
// compute Delta relative to MODE for ray

void IntfLocator::_computeDeltaMode(const double *vals, 
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

  // estimate median from histogram (not currently used)
  
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

  // set delta from mode

  for (int igate = 0; igate < _nGates; igate++) {
    double val = vals[igate];
    if (val != _missingVal) {
      dMode[igate] = fabs(val - modeVal);
      // dMode[igate] = val - median;
    } else {
      dMode[igate] = _missingVal;
    }
    mode[igate] = modeVal;
    // mode[igate] = median;
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
// Create the default interest maps and weights

void IntfLocator::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;

  pts.clear();
  pts.push_back(InterestMap::ImPoint(35.0, 0.001));
  pts.push_back(InterestMap::ImPoint(45.0, 1.0));
  setInterestMapPhaseNoise(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.10, 1.0));
  pts.push_back(InterestMap::ImPoint(0.20, 0.001));
  setInterestMapNcpMean(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(4.0, 0.001));
  pts.push_back(InterestMap::ImPoint(5.0, 1.0));
  setInterestMapWidthMean(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(2.0, 1.0));
  pts.push_back(InterestMap::ImPoint(2.5, 0.001));
  setInterestMapSnrDMode(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.65, 1.0));
  pts.push_back(InterestMap::ImPoint(0.75, 0.001));
  setInterestMapSnrSdev(pts, 1.0);

  setRlanInterestThreshold(0.51);
  setNoiseInterestThreshold(0.51);

}

/////////////////////////////////////////////////////////
// set interest map and weight for phase noise

void IntfLocator::setInterestMapPhaseNoise
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapPhaseNoise) {
    delete _interestMapPhaseNoise;
  }
  _interestMapPhaseNoise = new InterestMap("PhaseNoise", pts, weight);
  _weightPhaseNoise = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for ncp mean

void IntfLocator::setInterestMapNcpMean
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapNcpMean) {
    delete _interestMapNcpMean;
  }
  _interestMapNcpMean = new InterestMap("NcpMean", pts, weight);
  _weightNcpMean = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for width mean

void IntfLocator::setInterestMapWidthMean
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapWidthMean) {
    delete _interestMapWidthMean;
  }
  _interestMapWidthMean = new InterestMap("WidthMean", pts, weight);
  _weightWidthMean = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for snr sdev

void IntfLocator::setInterestMapSnrSdev
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapSnrSdev) {
    delete _interestMapSnrSdev;
  }
  _interestMapSnrSdev = new InterestMap("SnrSdev", pts, weight);
  _weightSnrSdev = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for snr delta from mode

void IntfLocator::setInterestMapSnrDMode
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapSnrDMode) {
    delete _interestMapSnrDMode;
  }
  _interestMapSnrDMode = new InterestMap("SnrDMode", pts, weight);
  _weightSnrDMode = weight;
}

/////////////////////////////////////////////////////////
// set combined interest threshold for RLAN interference

void IntfLocator::setRlanInterestThreshold(double val)
{
  _rlanInterestThreshold = val;
}

//////////////////////////////////////////////////////////
// set combined interest threshold for noise and no signal

void IntfLocator::setNoiseInterestThreshold(double val)
{
  _noiseInterestThreshold = val;
}

