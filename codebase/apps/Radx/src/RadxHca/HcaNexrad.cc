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
#include <radar/BeamHeight.hh>
#include <cstring>

using namespace std;

// mutex for print protection

pthread_mutex_t HcaNexrad::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;

// Constructor

HcaNexrad::HcaNexrad(const TempProfile &tempProfile) :
        _tempProfile(tempProfile)
  
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
  
  _tempAtBottomOfMeltingLayerC = 0.5;
  _tempAtTopOfMeltingLayerC = -0.5;

  _tmpMinHtMeters = 0;
  _tmpMaxHtMeters = 0;
  _tmpBottomC = 0;
  _tmpTopC = 0;

  _dbzFilterLen = 7;
  _zdrFilterLen = 11;
  _rhohvFilterLen = 11;
  _phidpFilterLen = 11;
  _phidpHvyFilterLen = 25;

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

    _tempLow[ii] = _missingDouble;
    _tempMid[ii] = _missingDouble;
    _tempHigh[ii] = _missingDouble;

    _smoothDbz[ii] = _missingDouble;
    _smoothZdr[ii] = _missingDouble;
    _smoothRhohv[ii] = _missingDouble;
    _smoothPhidp[ii] = _missingDouble;
    _hvySmoothPhidp[ii] = _missingDouble;

    _textureDbz[ii] = _missingDouble;
    _sdDbz[ii] = _missingDouble;
    _textureZdr[ii] = _missingDouble;
    _textureRhohv[ii] = _missingDouble;
    _texturePhidp[ii] = _missingDouble;
    _sdPhidp[ii] = _missingDouble;

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
    _tempCat[ii] = -1;
    
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

  _tempLow = _tempLow_.alloc(_nGates);
  _tempMid = _tempMid_.alloc(_nGates);
  _tempHigh = _tempHigh_.alloc(_nGates);

  _smoothDbz = _smoothDbz_.alloc(_nGates);
  _smoothZdr = _smoothZdr_.alloc(_nGates);
  _smoothRhohv = _smoothRhohv_.alloc(_nGates);
  _smoothPhidp = _smoothPhidp_.alloc(_nGates);
  _hvySmoothPhidp = _hvySmoothPhidp_.alloc(_nGates);

  _textureDbz = _textureDbz_.alloc(_nGates);
  _sdDbz = _sdDbz_.alloc(_nGates);
  _textureZdr = _textureZdr_.alloc(_nGates);
  _textureRhohv = _textureRhohv_.alloc(_nGates);
  _texturePhidp = _texturePhidp_.alloc(_nGates);
  _sdPhidp = _sdPhidp_.alloc(_nGates);

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
  _tempCat = _tempCat_.alloc(_nGates);

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

  _fillTempArrays();

  // compute RMSE of trend residuals for dbz, zdr and rhohv
  
  _filt.computeTrendDevInRange(_dbz,
                               _nGates,
                               _dbzFilterLen,
                               _missingDouble,
                               _smoothDbz,
                               _textureDbz);
  
  _filt.computeTrendDevInRange(_zdr,
                               _nGates,
                               _zdrFilterLen,
                               _missingDouble,
                               _smoothZdr,
                               _textureZdr);
  
  _filt.computeTrendDevInRange(_rhohv,
                               _nGates,
                               _rhohvFilterLen,
                               _missingDouble,
                               _smoothRhohv,
                               _textureRhohv);
  
  // compute standard deviation of dbz - for testing purposes
  
  _filt.computeSdevInRange(_dbz,
                           _sdDbz,
                           _nGates,
                           _dbzFilterLen,
                           _missingDouble);
  
  // compute RMSE of trend residuals for phidp
  
  _phidpFilt.computePhidpSdev(_nGates,
                              _phidpFilterLen,
                              _phidp,
                              _missingDouble,
                              true,
                              _smoothPhidp,
                              _texturePhidp);
  
  _phidpFilt.computePhidpSdev(_nGates,
                              _phidpHvyFilterLen,
                              _phidp,
                              _missingDouble,
                              true,
                              _hvySmoothPhidp,
                              NULL);
  
  // compute standard deviation of phidp for testing purposes
  
  _phidpFilt.computePhidpSdev(_nGates,
                              _phidpFilterLen,
                              _phidp,
                              _missingDouble,
                              false,
                              NULL,
                              _sdPhidp);
  
  // for (int igate = 0; igate < _nGates; igate++) {
  //   double sdPhidp = _sdPhidp[igate];
  //   double snr = _snr[igate];
  //   if (sdPhidp == _missingDouble) {
  //     if (snr != _missingDouble && snr > _snrThresholdDb) {
  //       _sdPhidp[igate] = 0.0;
  //     }
  //   } else if (sdPhidp < 0.01) {
  //     if (snr == _missingDouble || snr < _snrThresholdDb) {
  //       _sdPhidp[igate] = _missingDouble;
  //     }
  //   }
  // }

  // set up feature 2D-array
  
  double *featureVals[HcaInterestMap::nFeatures];
  featureVals[HcaInterestMap::FeatureDBZ] = _smoothDbz;
  featureVals[HcaInterestMap::FeatureZDR] = _smoothZdr;
  featureVals[HcaInterestMap::FeatureRHOHV] = _smoothRhohv;
  featureVals[HcaInterestMap::FeatureLOG_KDP] = _logKdp;
  featureVals[HcaInterestMap::FeatureSD_DBZ] = _textureDbz;
  featureVals[HcaInterestMap::FeatureSD_PHIDP] = _texturePhidp;

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
    _tempCat[igate] = -1;
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

    // load list of classes applicable at this temperature

    vector<HcaInterestMap::imap_class_t> validClasses;

    if (_tempHigh[igate] >= _tempAtBottomOfMeltingLayerC) {
      validClasses.push_back(HcaInterestMap::ClassGC);
      validClasses.push_back(HcaInterestMap::ClassBS);
      validClasses.push_back(HcaInterestMap::ClassBD);
      validClasses.push_back(HcaInterestMap::ClassRA);
      validClasses.push_back(HcaInterestMap::ClassHR);
      validClasses.push_back(HcaInterestMap::ClassRH);
      _tempCat[igate] = 1;
    } else if (_tempMid[igate] >= _tempAtBottomOfMeltingLayerC) {
      validClasses.push_back(HcaInterestMap::ClassGC);
      validClasses.push_back(HcaInterestMap::ClassBS);
      validClasses.push_back(HcaInterestMap::ClassWS);
      validClasses.push_back(HcaInterestMap::ClassGR);
      validClasses.push_back(HcaInterestMap::ClassBD);
      validClasses.push_back(HcaInterestMap::ClassRA);
      validClasses.push_back(HcaInterestMap::ClassHR);
      validClasses.push_back(HcaInterestMap::ClassRH);
      _tempCat[igate] = 2;
    } else if (_tempMid[igate] >= _tempAtTopOfMeltingLayerC) {
      validClasses.push_back(HcaInterestMap::ClassGC);
      validClasses.push_back(HcaInterestMap::ClassBS);
      validClasses.push_back(HcaInterestMap::ClassDS);
      validClasses.push_back(HcaInterestMap::ClassWS);
      validClasses.push_back(HcaInterestMap::ClassGR);
      validClasses.push_back(HcaInterestMap::ClassBD);
      validClasses.push_back(HcaInterestMap::ClassRH);
      _tempCat[igate] = 3;
    } else if (_tempLow[igate] >= _tempAtTopOfMeltingLayerC) {
      validClasses.push_back(HcaInterestMap::ClassGC);
      validClasses.push_back(HcaInterestMap::ClassBS);
      validClasses.push_back(HcaInterestMap::ClassDS);
      validClasses.push_back(HcaInterestMap::ClassWS);
      validClasses.push_back(HcaInterestMap::ClassCR);
      validClasses.push_back(HcaInterestMap::ClassGR);
      validClasses.push_back(HcaInterestMap::ClassBD);
      validClasses.push_back(HcaInterestMap::ClassRH);
      _tempCat[igate] = 4;
    } else {
      validClasses.push_back(HcaInterestMap::ClassDS);
      validClasses.push_back(HcaInterestMap::ClassCR);
      validClasses.push_back(HcaInterestMap::ClassGR);
      validClasses.push_back(HcaInterestMap::ClassRH);
      _tempCat[igate] = 5;
    }

    // determine the class with the highest weighted interest

    int mostLikelyClass = -1;
    double maxInterest = -9999.0;
    for (size_t ii = 0; ii < validClasses.size(); ii++) {
      HcaInterestMap::imap_class_t validClass = validClasses[ii];
      if(interestVals[validClass][igate] > maxInterest) {
        mostLikelyClass = validClass;
        maxInterest = interestVals[validClass][igate];
      }
    }
    _hca[igate] = mostLikelyClass + 1;

    if (_snr[igate] < _snrThresholdDb) {
      _hca[igate] = -1;
    }

    
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

//////////////////////////////////////////////
// fill temperature array, based on height

void HcaNexrad::_fillTempArrays()
  
{
  
  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_radarHtKm);
  if (_setPseudoRadiusRatio) {
    beamHt.setPseudoRadiusRatio(_pseudoRadiusRatio);
  }
  double rangeKm = _startRangeKm;
  double halfBeamWidth = _vertBeamWidthDeg / 2.0;
  if (halfBeamWidth > 1.5) {
    halfBeamWidth = 1.5;
  }
  for (int ii = 0; ii < _nGates; ii++, rangeKm += _gateSpacingKm) {
    double htLow = beamHt.computeHtKm(_elevation - halfBeamWidth, rangeKm);
    _tempLow[ii] = _tempProfile.getTempForHtKm(htLow);
    double htMid = beamHt.computeHtKm(_elevation, rangeKm);
    _tempMid[ii] = _tempProfile.getTempForHtKm(htMid);
    double htHigh = beamHt.computeHtKm(_elevation + halfBeamWidth, rangeKm);
    _tempHigh[ii] = _tempProfile.getTempForHtKm(htHigh);
  }

}
    
