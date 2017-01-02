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
// HcaNexrad.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////
//
// HcaNexrad - hydrometeor classification algorithm
//
// References:
// Park HyangSuk, A. V. Ryzhhov, D. S. Zrnic, Kyung0Eak Kim.
// June 2009.
// The Hydrometeor Classification Algorithm for the Polarimetric
// WSR-88D: Description and Application to an MCS.
// AMS Weather and Forecasting, Vol 24, 730-748, June 2009.
//
///////////////////////////////////////////////////////////////

#include "HcaNexrad.hh"
#include <radar/FilterUtils.hh>
#include <radar/BeamHeight.hh>
#include <cstring>

using namespace std;

// mutex for print protection

pthread_mutex_t HcaNexrad::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;

// Constructor

HcaNexrad::HcaNexrad()
  
{

  // initialize with reasonable values

  _debug = false;
  _verbose = false;

  _missingDouble = -9999.0;
  
  _wavelengthM = 0.010;

  _nGates = 0;
  _startRangeKm = 0.125;
  _gateSpacingKm = 0.250;
  _radarHtKm = 0.0;

  _elevation = 0.0;
  _azimuth = 0.0;

  _setPseudoRadiusRatio = false;
  _pseudoRadiusRatio = 4.0 / 3.0;

  _tmpMinHtMeters = 0;
  _tmpMaxHtMeters = 0;
  _tmpBottomC = 0;
  _tmpTopC = 0;

  _dbzFilterLen = 5;
  _zdrFilterLen = 9;
  _rhohvFilterLen = 9;
  _sdDbzFilterLen = 5;
  _sdPhidpFilterLen = 9;

  // initialize interest map array with NULLS
  // interest maps must be added by calling class using addInterestMap()

  for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
    for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
      _imaps[iclass][ifeature] = NULL;
    }
  }

}
  
// destructor

HcaNexrad::~HcaNexrad()

{
  
  deleteInterestMaps();

}

/////////////////////////////////////////////////////////
// Initialize the object arrays for later use.
// Do this if you need access to the arrays, but have not yet called
// computePidBeam(), and do not plan to do so.
// For example, you may want to output missing fields that you have
// not computed, but the memory needs to be there.

void HcaNexrad::initializeArrays(int nGates)

{

  _nGates = nGates;

  // allocate local arrays

  _allocArrays();

  // set to missing
  
  for (int ii = 0; ii < _nGates; ii++) {

    _snr[ii] = _missingDouble;
    _dbz[ii] = _missingDouble;
    _zdr[ii] = _missingDouble;
    _rhohv[ii] = _missingDouble;
    _phidp[ii] = _missingDouble;
    _logKdp[ii] = _missingDouble;
    _tempC[ii] = _missingDouble;

    _sdDbz[ii] = _missingDouble;
    _sdDbz2[ii] = _missingDouble;
    _sdPhidp[ii] = _missingDouble;
    _sdPhidp2[ii] = _missingDouble;

    _gcInterest[ii] = _missingDouble;
    _bsInterest[ii] = _missingDouble;
    _dsInterest[ii] = _missingDouble;
    _wsInterest[ii] = _missingDouble;
    _crInterest[ii] = _missingDouble;
    _grInterest[ii] = _missingDouble;
    _bdInterest[ii] = _missingDouble;
    _raInterest[ii] = _missingDouble;
    _hrInterest[ii] = _missingDouble;
    _rhInterest[ii] = _missingDouble;

    _hca[ii] = -1;
    
  }
  
}

/////////////////////////
// allocate local arrays

void HcaNexrad::_allocArrays()
  
{
  
  _snr = _snr_.alloc(_nGates);
  _dbz = _dbz_.alloc(_nGates);
  _zdr = _zdr_.alloc(_nGates);
  _rhohv = _rhohv_.alloc(_nGates);
  _phidp = _phidp_.alloc(_nGates);
  _logKdp = _logKdp_.alloc(_nGates);
  _tempC = _tempC_.alloc(_nGates);

  _sdDbz = _sdDbz_.alloc(_nGates);
  _sdDbz2 = _sdDbz2_.alloc(_nGates);
  _sdPhidp = _sdPhidp_.alloc(_nGates);
  _sdPhidp2 = _sdPhidp2_.alloc(_nGates);

  _gcInterest = _gcInterest_.alloc(_nGates);
  _bsInterest = _bsInterest_.alloc(_nGates);
  _dsInterest = _dsInterest_.alloc(_nGates);
  _wsInterest = _wsInterest_.alloc(_nGates);
  _crInterest = _crInterest_.alloc(_nGates);
  _grInterest = _grInterest_.alloc(_nGates);
  _bdInterest = _bdInterest_.alloc(_nGates);
  _raInterest = _raInterest_.alloc(_nGates);
  _hrInterest = _hrInterest_.alloc(_nGates);
  _rhInterest = _rhInterest_.alloc(_nGates);

  _hca = _hca_.alloc(_nGates);

}

//////////////////////////////////////
// compute HCA

void HcaNexrad::computeHca(const double *snr,
                           const double *dbz,
                           const double *zdr,
                           const double *rhohv,
                           const double *phidpUnfolded,
                           const double *kdp)

{

  // copy the data locally

  memcpy(_snr, snr, _nGates * sizeof(double));
  memcpy(_dbz, dbz, _nGates * sizeof(double));
  memcpy(_zdr, zdr, _nGates * sizeof(double));
  memcpy(_rhohv, rhohv, _nGates * sizeof(double));
  memcpy(_phidp, phidpUnfolded, _nGates * sizeof(double));

  for (int igate = 0; igate < _nGates; igate++) {
    if (kdp[igate] > 1.0e-3) {
      _logKdp[igate] = log(kdp[igate]);
    } else {
      _logKdp[igate] = -30.0;
    }
  }

  // compute the temperature at each gate

  _fillTempArray();

  // compute trend deviation of dbz
  
  FilterUtils::computeTrendDevInRange(_dbz,
                                      _sdDbz,
                                      _nGates,
                                      _sdDbzFilterLen,
                                      _missingDouble);
  
  
  FilterUtils::computeSdevInRange(_dbz,
                                  _sdDbz2,
                                  _nGates,
                                  _sdDbzFilterLen,
                                  _missingDouble);
  
  
  // compute trend deviation of phidp
  
  FilterUtils::computeTrendDevInRange(_phidp,
                                      _sdPhidp,
                                      _nGates,
                                      _sdPhidpFilterLen,
                                      _missingDouble);
  
  FilterUtils::computeSdevInRange(_phidp,
                                  _sdPhidp2,
                                  _nGates,
                                  _sdPhidpFilterLen,
                                  _missingDouble);
  
  for (int igate = 0; igate < _nGates; igate++) {
    double sdPhidp = _sdPhidp[igate];
    double snr = _snr[igate];
    if (sdPhidp == _missingDouble) {
      if (snr != _missingDouble && snr > _snrThresholdDb) {
        _sdPhidp[igate] = 0.0;
      }
    } else if (sdPhidp < 0.01) {
      if (snr == _missingDouble || snr < _snrThresholdDb) {
        _sdPhidp[igate] = _missingDouble;
      }
    }
  }

  // set up feature 2D-array
  
  double *featureVals[HcaInterestMap::nFeatures];
  featureVals[HcaInterestMap::FeatureDBZ] = _dbz;
  featureVals[HcaInterestMap::FeatureZDR] = _zdr;
  featureVals[HcaInterestMap::FeatureRHOHV] = _rhohv;
  featureVals[HcaInterestMap::FeatureLOG_KDP] = _logKdp;
  featureVals[HcaInterestMap::FeatureSD_DBZ] = _sdDbz;
  featureVals[HcaInterestMap::FeatureSD_PHIDP] = _sdPhidp;

  // set up classes 2D-array
  
  double *interestVals[HcaInterestMap::nClasses];
  interestVals[HcaInterestMap::ClassGC] = _gcInterest;
  interestVals[HcaInterestMap::ClassBS] = _bsInterest;
  interestVals[HcaInterestMap::ClassDS] = _dsInterest;
  interestVals[HcaInterestMap::ClassWS] = _wsInterest;
  interestVals[HcaInterestMap::ClassCR] = _crInterest;
  interestVals[HcaInterestMap::ClassGR] = _grInterest;
  interestVals[HcaInterestMap::ClassBD] = _bdInterest;
  interestVals[HcaInterestMap::ClassRA] = _raInterest;
  interestVals[HcaInterestMap::ClassHR] = _hrInterest;
  interestVals[HcaInterestMap::ClassRH] = _rhInterest;

  // initialize interest to missing

  for (int igate = 0; igate < _nGates; igate++) {
    for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
      interestVals[iclass][igate] = _missingDouble;
    }
    _hca[igate] = -1;
  }

  // compute the interest for each class at each gate

  for (int igate = 0; igate < _nGates; igate++) {

    // ensure we have all feature fields

    bool missing = false;
    for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
      if (featureVals[ifeature][igate] == _missingDouble) {
        missing = true;
        break;
      }
    }
    if (missing) {
      continue;
    }

    // sum up interest and weights for each class
    
    for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
    
      double sumWtInterest = 0.0;
      double sumWt = 0.0;
      for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
        _imaps[iclass][ifeature]->accumWeightedInterest(_dbz[igate],
                                                        featureVals[ifeature][igate],
                                                        sumWtInterest,
                                                        sumWt);
      } // ifeature

      double meanInterest = sumWtInterest / sumWt;
      interestVals[iclass][igate] = meanInterest;
      
    } // iclass

    // determine the class with the highest weighted interest

    int mostLikelyClass = -1;
    double maxInterest = -9999.0;
    for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
      if(interestVals[iclass][igate] > maxInterest) {
        mostLikelyClass = iclass;
        maxInterest = interestVals[iclass][igate];
      }
    }
    _hca[igate] = mostLikelyClass + 1;
    
  } // igate

}

///////////////////////////////////////////////////////////////////
// Create and add an interest map,

int HcaNexrad::addInterestMap(HcaInterestMap::imap_class_t hcaClass,
                              HcaInterestMap::imap_feature_t feature,
                              double x1, double x2,
                              double x3, double x4,
                              double weight)
{

  int iret = 0;

  if (_imaps[hcaClass][feature] != NULL) {
    cerr << "ERROR - HcaNexrad::_addInterestMap()" << endl;
    cerr << "  Duplicate interest map" << endl;
    cerr << "                  class   : "
         << HcaInterestMap::hcaClassToStr(hcaClass) << endl;
    cerr << "                  feature : " 
         << HcaInterestMap::hcaFeatureToStr(feature) << endl;
    iret = -1;
  } else {
    _imaps[hcaClass][feature] =
      new HcaInterestMap(hcaClass, feature,
                         x1, x2, x3, x4, weight);
  }
  
  return iret;

}

////////////////////////////////////////////
// check that all interest maps are non-NULL

int HcaNexrad::checkInterestMaps()
  
{
  
  int iret = 0;

  for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
    for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
      if (_imaps[iclass][ifeature] == NULL) {
        cerr << "ERROR - HcaNexrad::_checkInterestMaps()" << endl;
        cerr << "  Missing interest map" << endl;
        cerr << "                  class   : "
             << HcaInterestMap::hcaClassToStr
          ((HcaInterestMap::imap_class_t) iclass) << endl;
        cerr << "                  feature : " 
             << HcaInterestMap::hcaFeatureToStr
          ((HcaInterestMap::imap_feature_t) ifeature) << endl;
        iret = -1;
      }
    }
  }

  return iret;

}

/////////////////////////////////////
// print all interest maps

void HcaNexrad::printInterestMaps(ostream &out)
  
{
  
  out << "=============================================" << endl;
  for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
    for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
      if (_imaps[iclass][ifeature] != NULL) {
        if (_imaps[iclass][ifeature] != NULL) {
          _imaps[iclass][ifeature]->print(out);
        }
      }
    }
  }
  out << "=============================================" << endl;

}

/////////////////////////////////////////
// delete all interest maps - set to NULL

void HcaNexrad::deleteInterestMaps()
  
{
  
  for (size_t iclass = 0; iclass < HcaInterestMap::nClasses; iclass++) {
    for (size_t ifeature = 0; ifeature < HcaInterestMap::nFeatures; ifeature++) {
      if (_imaps[iclass][ifeature] != NULL) {
        delete _imaps[iclass][ifeature];
        _imaps[iclass][ifeature] = NULL;
      }
    }
  }

}

/////////////////////////////////////////////////////////////
// Set the temperature profile.
//

void HcaNexrad::setTempProfile(const TempProfile &tempProfile)

{

  // store the profile

  _tmpProfile = tempProfile.getProfile();

  // compute the temperature height lookup table

  _computeTempHtLookup();

}
 
//////////////////////////////////////////////
// fill temperature array, based on height

void HcaNexrad::_fillTempArray()
  
{
  
  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_radarHtKm);
  if (_setPseudoRadiusRatio) {
    beamHt.setPseudoRadiusRatio(_pseudoRadiusRatio);
  }
  double rangeKm = _startRangeKm;
  for (int ii = 0; ii < _nGates; ii++, rangeKm += _gateSpacingKm) {
    double htKm = beamHt.computeHtKm(_elevation, rangeKm);
    _tempC[ii] = _computeTempC(htKm);
  }

}
    
/////////////////////////////////////////////////////
// compute temperature/ht lookup 

void HcaNexrad::_computeTempHtLookup()
  
{

  _tmpMinHtMeters =
    (int) (_tmpProfile[0].htKm * 1000.0 + 0.5);
  _tmpMaxHtMeters =
    (int) (_tmpProfile[_tmpProfile.size()-1].htKm * 1000.0 + 0.5);

  _tmpBottomC = _tmpProfile[0].tmpC;
  _tmpTopC = _tmpProfile[_tmpProfile.size()-1].tmpC;

  // fill out temp array, every meter

  int nHt = (_tmpMaxHtMeters - _tmpMinHtMeters) + 1;
  _tmpHtArray_.free();
  _tmpHtArray = _tmpHtArray_.alloc(nHt);
  
  for (int ii = 1; ii < (int) _tmpProfile.size(); ii++) {

    int minHtMeters = (int) (_tmpProfile[ii-1].htKm * 1000.0 + 0.5);
    double minTmp = _tmpProfile[ii-1].tmpC;

    int maxHtMeters = (int) (_tmpProfile[ii].htKm * 1000.0 + 0.5);
    double maxTmp = _tmpProfile[ii].tmpC;

    double deltaMeters = maxHtMeters - minHtMeters;
    double deltaTmp = maxTmp - minTmp;
    double gradient = deltaTmp / deltaMeters;
    double tmp = minTmp;
    int kk = minHtMeters - _tmpMinHtMeters;
    
    for (int jj = minHtMeters; jj <= maxHtMeters; jj++, kk++, tmp += gradient) {
      if (kk >= 0 && kk < nHt) {
	_tmpHtArray[kk] = tmp;
      }
    }

  }

}

////////////////////////////////////////
// get temperature at a given height

double HcaNexrad::_computeTempC(double htKm)

{

  int htMeters = (int) (htKm * 1000.0 + 0.5);

  if (htMeters <= _tmpMinHtMeters) {
    return _tmpBottomC;
  } else if (htMeters >= _tmpMaxHtMeters) {
    return _tmpTopC;
  }

  int kk = htMeters - _tmpMinHtMeters;
  return _tmpHtArray[kk];

}

