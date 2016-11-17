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
// SeaClutter.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2016
//
///////////////////////////////////////////////////////////////
//
// Locate gates contaminated by sea clutter
//
///////////////////////////////////////////////////////////////

#include <radar/SeaClutter.hh>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <math.h>
using namespace std;

///////////////////////////////////////////////////////////////
// Constructor

SeaClutter::SeaClutter()
  
{

  _debug = false;

  // create the default interest maps

  _interestMapRhohvMean = NULL;
  _interestMapPhidpSdev = NULL;
  _interestMapZdrSdev = NULL;
  _interestMapDbzElevGradient = NULL;

  setMaxElevDeg(3.0);

  _createDefaultInterestMaps();
  _nGatesKernel = 9;
  
  _radarHtM = -9999.0;
  _wavelengthM = -9999.0;
  _minSnrDb = 0.0;

}

///////////////////////////////////////////////////////////////
// destructor

SeaClutter::~SeaClutter()
  
{

  if (_interestMapRhohvMean) {
    delete _interestMapRhohvMean;
  }

  if (_interestMapPhidpSdev) {
    delete _interestMapPhidpSdev;
  }

  if (_interestMapZdrSdev) {
    delete _interestMapZdrSdev;
  }

  if (_interestMapDbzElevGradient) {
    delete _interestMapDbzElevGradient;
  }

}

///////////////////////////////////////////////////////////////
// print parameters for debugging

void SeaClutter::printParams(ostream &out)
  
{

  out << "Performing sea clutter detection:" << endl;
  out << "  nGatesKernel: " << _nGatesKernel << endl;
  out << "  clutInterestThreshold: " << _clutInterestThreshold << endl;
  
  if (_interestMapRhohvMean) {
    _interestMapRhohvMean->printParams(out);
  }
  if (_interestMapPhidpSdev) {
    _interestMapPhidpSdev->printParams(out);
  }
  if (_interestMapZdrSdev) {
    _interestMapZdrSdev->printParams(out);
  }
  if (_interestMapDbzElevGradient) {
    _interestMapDbzElevGradient->printParams(out);
  }

}

///////////////////////////////////////////////////////////////
// set the ray properties
// must be called before locate()

void SeaClutter::setRayProps(time_t timeSecs, 
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

  _snrAvail = false;
  _dbzAvail = false;
  _rhohvAvail = false;
  _phidpAvail = false;
  _zdrAvail = false;
  _dbzElevGradientAvail = false;

  _snr = _snr_.alloc(_nGates);
  _dbz = _dbz_.alloc(_nGates);
  _phidp = _phidp_.alloc(_nGates);
  _rhohv = _rhohv_.alloc(_nGates);
  _zdr = _zdr_.alloc(_nGates);
  _dbzElevGradient = _dbzElevGradient_.alloc(_nGates);

}

///////////////////////////////////////////////////////////////
// set the SNR field, if available
// must be called after setRayProps()

void SeaClutter::setSnrField(double *vals)
{
  memcpy(_snr, vals, _nGates * sizeof(double));
  _snrAvail = true;
}

///////////////////////////////////////////////////////////////
// set the DBZ field, if no SNR available
// SNR will be estimated from DBZ
// must be called after setRayProps()

void SeaClutter::setDbzField(double *vals,
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
// set the RHOHV field
// must be called after setRayProps()

void SeaClutter::setRhohvField(double *vals)
{
  memcpy(_rhohv, vals, _nGates * sizeof(double));
  _rhohvAvail = true;
}

///////////////////////////////////////////////////////////////
// set the PHIDP field
// must be called after setRayProps()

void SeaClutter::setPhidpField(double *vals)
{
  memcpy(_phidp, vals, _nGates * sizeof(double));
  _phidpAvail = true;
}

///////////////////////////////////////////////////////////////
// set the ZDR field
// must be called after setRayProps()

void SeaClutter::setZdrField(double *vals)
{
  memcpy(_zdr, vals, _nGates * sizeof(double));
  _zdrAvail = true;
}

///////////////////////////////////////////////////////////////
// set the DBZ ELEV GRADIEND field
// must be called after setRayProps()

void SeaClutter::setDbzElevGradientField(double *vals)
{
  memcpy(_dbzElevGradient, vals, _nGates * sizeof(double));
  _dbzElevGradientAvail = true;
}

//////////////////////////////////////////////////////////////
// Compute the SNR field from the DBZ field

void SeaClutter::_computeSnrFromDbz(double noiseDbzAt100km)

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
// locate the clutter gates
//
// Returns 0 on success, -1 on failure

int SeaClutter::locate()
  
{

  bool iret = 0;
  
  if (!_snrAvail) {
    cerr << "ERROR - SeaClutter::clutLocate()" << endl;
    cerr << "  Neither SNR nor DBZ available" << endl;
    cerr << "  Cannot locate sea clutter without measured SNR" << endl;
    cerr << "    or SNR estimated from DBZ" << endl;
    iret = -1;
  }

  if (!_rhohvAvail) {
    cerr << "ERROR - SeaClutter::clutLocate()" << endl;
    cerr << "  RHOHV not available" << endl;
    cerr << "  Cannot locate sea clutter without RHOHV field" << endl;
    iret = -1;
  }

  if (!_phidpAvail) {
    cerr << "ERROR - SeaClutter::clutLocate()" << endl;
    cerr << "  PHIDP not available" << endl;
    cerr << "  Cannot locate sea clutter without phidp field" << endl;
    iret = -1;
  }

  if (!_zdrAvail) {
    cerr << "ERROR - SeaClutter::clutLocate()" << endl;
    cerr << "  ZDR not available" << endl;
    cerr << "  Cannot locate sea clutter without zdr field" << endl;
    iret = -1;
  }

  if (iret) {
    return -1;
  }

  _startGate.resize(_nGates);
  _endGate.resize(_nGates);
  
  _clutFlag = _clutFlag_.alloc(_nGates);
  _snrMean = _snrMean_.alloc(_nGates);
  _rhohvMean = _rhohvMean_.alloc(_nGates);
  _phidpSdev = _phidpSdev_.alloc(_nGates);
  _zdrSdev = _zdrSdev_.alloc(_nGates);

  _rhohvMeanInterest = _rhohvMeanInterest_.alloc(_nGates);
  _phidpSdevInterest = _phidpSdevInterest_.alloc(_nGates);
  _zdrSdevInterest = _zdrSdevInterest_.alloc(_nGates);
  _dbzElevGradientInterest = _dbzElevGradientInterest_.alloc(_nGates);
  
  for (int igate = 0; igate < _nGates; igate++) {
    _clutFlag[igate] = false;
    _startGate[igate] = 0;
    _endGate[igate] = 0;
    _snrMean[igate] = -9999;
    _rhohvMean[igate] = -9999;
    _phidpSdev[igate] = -9999;
    _zdrSdev[igate] = -9999;
    _rhohvMeanInterest[igate] = -9999;
    _phidpSdevInterest[igate] = -9999;
    _zdrSdevInterest[igate] = -9999;
    _dbzElevGradientInterest[igate] = -9999;
  }
  
  // set kernel size for computing phase error
  // make sure it is odd, add 1 if necessary
  
  int kernelSize = _nGatesKernel;
  int kernelHalf = kernelSize / 2;
  kernelSize = kernelHalf * 2 + 1;
  
  if ((int) _nGates < kernelSize) {
    cerr << "ERROR - SeaClutter::clutLocate()" << endl;
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

  // compute means and sdev
  
  _computeMeanInRange(_snr, _snrMean);
  _computeMeanInRange(_rhohv, _rhohvMean);
  _computeSdevInRange(_zdr, _zdrSdev);

  // sdev of phidp is a special case since we
  // need to compute it around the circle

  _phidpProc.setRangeGeometry(_startRangeKm, _gateSpacingKm);
  _phidpProc.computePhidpSdev(_nGates, _nGatesKernel,
                              _phidp, _missingVal);
  memcpy(_phidpSdev, _phidpProc.getPhidpSdev(),
         _nGates * sizeof(double));
  
  // compute interest fields
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    _rhohvMeanInterest[igate] =
      _interestMapRhohvMean->getInterest(_rhohvMean[igate]);

    _phidpSdevInterest[igate] =
      _interestMapPhidpSdev->getInterest(_phidpSdev[igate]);

    _zdrSdevInterest[igate] =
      _interestMapZdrSdev->getInterest(_zdrSdev[igate]);

    _dbzElevGradientInterest[igate] =
      _interestMapDbzElevGradient->getInterest(_dbzElevGradient[igate]);
    
  }

  // compute sum weights
  
  double sumWeightsClut = (_weightRhohvMean + _weightPhidpSdev +
                           _weightZdrSdev + _weightDbzElevGradient);
  
  // set flags
  
  for (int igate = 0; igate < _nGates; igate++) {
    
    double sumInterestClut =
      (_rhohvMeanInterest[igate] * _weightRhohvMean) +
      (_phidpSdevInterest[igate] * _weightPhidpSdev) +
      (_zdrSdevInterest[igate] * _weightZdrSdev) +
      (_dbzElevGradientInterest[igate] * _weightDbzElevGradient);

    double interestClut = sumInterestClut / sumWeightsClut;
    if (interestClut > _clutInterestThreshold && _snrMean[igate] >= _minSnrDb) {
      _clutFlag[igate] = true;
    } else {
      _clutFlag[igate] = false;
    }

  } // igate

  // for single gates surrounded by clut, set the clut flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (_clutFlag[igate-1] && _clutFlag[igate+1]) {
      _clutFlag[igate] = true;
    }
  }
 
  // for single gates surrounded by non-clut, unset the clut flag
  
  for (int igate = 1; igate < _nGates - 1; igate++) {
    if (!_clutFlag[igate-1] && !_clutFlag[igate+1]) {
      _clutFlag[igate] = false;
    }
  }

  // if elevation angle is too high, clear flag

  if (_elevation > _maxElevDeg) {
    for (int igate = 0; igate < _nGates; igate++) {
      _clutFlag[igate] = false;
    }
  }

  return 0;
 
}

///////////////////////////////////////////////////////////////
// compute SDEV in range

void SeaClutter::_computeSdevInRange(const double *vals, double *sdevs)
  
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

void SeaClutter::_computeMeanInRange(const double *vals, double *means)
  
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

void SeaClutter::_createDefaultInterestMaps()
  
{

  vector<InterestMap::ImPoint> pts;

  pts.clear();
  pts.push_back(InterestMap::ImPoint(0.35, 1.0));
  pts.push_back(InterestMap::ImPoint(0.40, 0.001));
  setInterestMapRhohvMean(pts, 0.5);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(40.0, 0.001));
  pts.push_back(InterestMap::ImPoint(45.0, 1.0));
  setInterestMapPhidpSdev(pts, 1.0);

  pts.clear();
  pts.push_back(InterestMap::ImPoint(1.5, 0.001));
  pts.push_back(InterestMap::ImPoint(2.5, 1.0));
  setInterestMapZdrSdev(pts, 1.0);
  
  pts.clear();
  pts.push_back(InterestMap::ImPoint(-15, 1.0));
  pts.push_back(InterestMap::ImPoint(-5, 0.001));
  setInterestMapDbzElevGradient(pts, 1.0);
  
  setClutInterestThreshold(0.51);

}

/////////////////////////////////////////////////////////
// set interest map and weight for rhohv mean

void SeaClutter::setInterestMapRhohvMean
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapRhohvMean) {
    delete _interestMapRhohvMean;
  }
  _interestMapRhohvMean = new InterestMap("RhohvMean", pts, weight);
  _weightRhohvMean = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for phidp sdev

void SeaClutter::setInterestMapPhidpSdev
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapPhidpSdev) {
    delete _interestMapPhidpSdev;
  }
  _interestMapPhidpSdev = new InterestMap("PhidpSdev", pts, weight);
  _weightPhidpSdev = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for zdr sdev

void SeaClutter::setInterestMapZdrSdev
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapZdrSdev) {
    delete _interestMapZdrSdev;
  }
  _interestMapZdrSdev = new InterestMap("ZdrSdev", pts, weight);
  _weightZdrSdev = weight;
}

/////////////////////////////////////////////////////////
// set interest map and weight for zdr sdev

void SeaClutter::setInterestMapDbzElevGradient
  (const vector<InterestMap::ImPoint> &pts,
   double weight)
  
{
  if (_interestMapDbzElevGradient) {
    delete _interestMapDbzElevGradient;
  }
  _interestMapDbzElevGradient = new InterestMap("DbzElevGradient", pts, weight);
  _weightDbzElevGradient = weight;
}

/////////////////////////////////////////////////////////
// set combined interest threshold for sea clutter

void SeaClutter::setClutInterestThreshold(double val)
{
  _clutInterestThreshold = val;
}

