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
// ComputeEngine.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2012
//
///////////////////////////////////////////////////////////////
//
// ComputeEngine computation - for multi-threading
// There is one object per thread.
//
///////////////////////////////////////////////////////////////

#include "ComputeEngine.hh"
#include <toolsa/DateTime.hh>
#include <toolsa/sincos.h>
#include <rapmath/trig.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <algorithm>
using namespace std;
pthread_mutex_t ComputeEngine::_debugPrintMutex = PTHREAD_MUTEX_INITIALIZER;
const double ComputeEngine::missingDbl = -9999.0;

// Constructor

ComputeEngine::ComputeEngine(const Params &params)  :
        _params(params)
  
{

  OK = true;
  
}

// destructor

ComputeEngine::~ComputeEngine()

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

RadxRay *ComputeEngine::compute(RadxRay *inputRay)
{

  // create moments ray
  
  RadxRay *outputRay = new RadxRay;
  outputRay->copyMetaData(*inputRay);

  // set ray data
  
  _nGates = inputRay->getNGates();
  _startRangeKm = inputRay->getStartRangeKm();
  _gateSpacingKm = inputRay->getGateSpacingKm();
  _azimuth = inputRay->getAzimuthDeg();
  _elevation = inputRay->getElevationDeg();
  _timeSecs = inputRay->getTimeSecs();
  _nanoSecs = inputRay->getNanoSecs();
  _nyquist = inputRay->getNyquistMps();
  if (_params.override_nyquist) {
    _nyquist = _params.nyquist_velocity;
  }

  // loop through the fields in the ray

  for (size_t ifield = 0; ifield < inputRay->getNFields(); ifield++) {
    
    const RadxField *inputField = inputRay->getField(ifield);

    // find specified filter parameters

    int filterIndex = -1;
    for (int ii = 0; ii < _params.field_filters_n; ii++) {
      const Params::field_filter_t &fieldFilter = _params._field_filters[ii];
      string searchStr(fieldFilter.partial_field_name);
      string inputName(inputField->getName());
      if (searchStr.size() == 0 ||
          inputName.find(searchStr) != string::npos) {
        // found a match
        filterIndex = ii;
        break;
      }
    }

    RadxField *outField = NULL;
    
    if (filterIndex < 0) {
      // no filter, copy the field over unchanged
      outField = new RadxField(*inputField);
    } else {
      const Params::field_filter_t &fieldFilter =
        _params._field_filters[filterIndex];
      // apply specified filter
      if (fieldFilter.filter_type == Params::FILTER_MEDIAN ||
          inputField->getIsDiscrete()) {
        outField = _applyMedian(inputField, fieldFilter.length);
      } else if (fieldFilter.filter_type == Params::FILTER_TRIANGULAR) {
        outField = _applyTriangular(inputField, fieldFilter.length);
      } else if (fieldFilter.filter_type == Params::FILTER_LEAST_SQUARES) {
        outField = _applyLeastSquares(inputField, fieldFilter.length);
      } else {
        // no filter, copy the field over unchanged
        outField = new RadxField(*inputField);
      }
    }

    outputRay->addField(outField);

  } // ifield

  // set max range

  if (_params.set_max_range) {
    outputRay->setMaxRangeKm(_params.max_range_km);
  }

  return outputRay;

}

/////////////////////////////////////////////////
// apply median filter

RadxField *ComputeEngine::_applyMedian(const RadxField *inputField,
                                       int length)

{

  // ensure we have an odd number of gates

  if (length % 2 == 0) {
    length++;
  }
  int mm = length / 2;
  int mm1 = mm + 1;

  // save input type

  Radx::DataType_t inputType = inputField->getDataType();

  // get a float field
  
  RadxField *outField = new RadxField(*inputField);
  outField->convertToFl32();
  const Radx::fl32 *inData = outField->getDataFl32();
  Radx::fl32 missing = outField->getMissingFl32();

  vector<Radx::fl32> outData_;
  outData_.resize(outField->getNPoints());
  Radx::fl32 *outData = outData_.data();
  memcpy(outData, inData, outField->getNPoints() * sizeof(Radx::fl32));
  
  // loop through points, computing median

  vector<Radx::fl32> good;
  
  for (size_t igate = mm; igate < outField->getNPoints() - mm; igate++) {
    
    good.clear();
    for (int ii = 0; ii < length; ii++) {
      Radx::fl32 inVal = inData[igate - mm + ii];
      if (inVal != missing) {
        good.push_back(inVal);
      }
    }

    if ((int) good.size() > mm1) {
      sort(good.begin(), good.end());
      outData[igate] = good[good.size()/2];
    } else {
      outData[igate] = missing;
    }

  } // igate

  // convert output to to input type

  outField->setDataFl32(outField->getNPoints(), outData, true);
  outField->convertToType(inputType);

  return outField;

}

/////////////////////////////////////////////////
// apply triangular filter

RadxField *ComputeEngine::_applyTriangular(const RadxField *inputField,
                                           int length)

{
  
  // ensure we have an odd number of gates

  if (length % 2 == 0) {
    length++;
  }
  int mm = length / 2;
  int mm1 = mm + 1;

  // save input type
  
  Radx::DataType_t inputType = inputField->getDataType();

  // get a float field
  
  RadxField *outField = new RadxField(*inputField);
  outField->convertToFl32();
  const Radx::fl32 *inData = outField->getDataFl32();
  Radx::fl32 missing = outField->getMissingFl32();

  vector<Radx::fl32> outData_;
  outData_.resize(outField->getNPoints());
  Radx::fl32 *outData = outData_.data();
  memcpy(outData, inData, outField->getNPoints() * sizeof(Radx::fl32));

  // override fold limits if required

  _setFoldLimits(outField);
  
  // compute weights

  double *wts = new double[length];
  double centerVal = 1.0 / (double) mm1;
  wts[mm] = centerVal;
  for (int ii = 0; ii < mm; ii++) {
    double val = (centerVal * (ii + 1)) / (double) mm1;
    wts[ii] = val;
    wts[length - ii -1] = val;
  }

  if (outField->getFieldFolds() == false) {

    // apply filter directly to the data

    _applyTriangular(mm, outField->getNPoints(),
                     inData, outData, missing, length, wts);

  } else {
  
    // field folds
    // resolve field into x and y components

    Radx::fl32 *xxIn = new Radx::fl32[outField->getNPoints()];
    Radx::fl32 *yyIn = new Radx::fl32[outField->getNPoints()];

    for (size_t igate = 0; igate < outField->getNPoints(); igate++) {
      if (inData[igate] == missing) {
        xxIn[igate] = missing;
        yyIn[igate] = missing;
      } else {
        double angle = _getFoldAngle(inData[igate],
                                     outField->getFoldLimitLower(),
                                     outField->getFoldRange());
        double sinVal, cosVal;
        ta_sincos(angle, &sinVal, &cosVal);
        xxIn[igate] = cosVal;
        yyIn[igate] = sinVal;
      }
    }

    // apply filter to x and y components
    
    Radx::fl32 *xxOut = new Radx::fl32[outField->getNPoints()];
    Radx::fl32 *yyOut = new Radx::fl32[outField->getNPoints()];
    
    _applyTriangular(mm, outField->getNPoints(),
                     xxIn, xxOut, missing, length, wts);
  
    _applyTriangular(mm, outField->getNPoints(),
                     yyIn, yyOut, missing, length, wts);
    
    // compute the filtered value from the filtered components
    
    for (size_t igate = 0; igate < outField->getNPoints(); igate++) {
      if (xxOut[igate] == missing || yyOut[igate] == missing) {
        outData[igate] = missing;
      } else {
        double angleFilt = atan2(yyOut[igate], xxOut[igate]);
        double valFilt = _getFoldValue(angleFilt,
                                       outField->getFoldLimitLower(),
                                       outField->getFoldRange());
        outData[igate] = valFilt;
      }
    }

    delete[] xxIn;
    delete[] yyIn;
    delete[] xxOut;
    delete[] yyOut;

  }

  // convert output to to input type

  outField->setDataFl32(outField->getNPoints(), outData, true);
  outField->convertToType(inputType);

  delete[] wts;

  return outField;

}

void ComputeEngine::_applyTriangular(int mm,
                                     int nGates,
                                     const Radx::fl32 *inData,
                                     Radx::fl32 *outData,
                                     Radx::fl32 missing,
                                     int length,
                                     const double *wts)
  
{

  // loop through gates, computing filtered value
  
  for (int igate = mm; igate < nGates - mm; igate++) {
    int count = 0;
    double sum = 0.0;
    double sumWts = 0.0;
    for (int ii = 0; ii < length; ii++) {
      Radx::fl32 inVal = inData[igate - mm + ii];
      if (inVal != missing) {
        sum += inVal * wts[ii];
        sumWts += wts[ii];
        count++;
      }
    }
    if (count > mm) {
      outData[igate] = sum / sumWts;
    } else {
      outData[igate] = missing;
    }
  } // igate

}

/////////////////////////////////////////////////
// apply leastSquares filter

RadxField *ComputeEngine::_applyLeastSquares(const RadxField *inputField,
                                             int length)

{
  
  // ensure we have an odd number of gates

  if (length % 2 == 0) {
    length++;
  }
  int mm = length / 2;
  
  // save input type
  
  Radx::DataType_t inputType = inputField->getDataType();

  // get a float field
  
  RadxField *outField = new RadxField(*inputField);
  outField->convertToFl32();
  const Radx::fl32 *inData = outField->getDataFl32();
  Radx::fl32 missing = outField->getMissingFl32();

  vector<Radx::fl32> outData_;
  outData_.resize(outField->getNPoints());
  Radx::fl32 *outData = outData_.data();
  memcpy(outData, inData, outField->getNPoints() * sizeof(Radx::fl32));

  // override fold limits if required
  
  _setFoldLimits(outField);
  
  if (outField->getFieldFolds() == false) {
    
    // apply filter directly to the data
    
    _applyLeastSquares(mm, outField->getNPoints(),
                       inData, outData, missing, length);

  } else {
  
    // field folds
    // resolve field into x and y components
    
    Radx::fl32 *xxIn = new Radx::fl32[outField->getNPoints()];
    Radx::fl32 *yyIn = new Radx::fl32[outField->getNPoints()];

    for (size_t igate = 0; igate < outField->getNPoints(); igate++) {
      if (inData[igate] == missing) {
        xxIn[igate] = missing;
        yyIn[igate] = missing;
      } else {
        double angle = _getFoldAngle(inData[igate],
                                     outField->getFoldLimitLower(),
                                     outField->getFoldRange());
        double sinVal, cosVal;
        ta_sincos(angle, &sinVal, &cosVal);
        xxIn[igate] = cosVal;
        yyIn[igate] = sinVal;
      }
    }

    // apply filter to x and y components
    
    Radx::fl32 *xxOut = new Radx::fl32[outField->getNPoints()];
    Radx::fl32 *yyOut = new Radx::fl32[outField->getNPoints()];
    
    _applyLeastSquares(mm, outField->getNPoints(),
                       xxIn, xxOut, missing, length);
    
    _applyLeastSquares(mm, outField->getNPoints(),
                       yyIn, yyOut, missing, length);
    
    // compute the filtered value from the filtered components
    
    for (size_t igate = 0; igate < outField->getNPoints(); igate++) {
      if (xxOut[igate] == missing || yyOut[igate] == missing) {
        outData[igate] = missing;
      } else {
        double angleFilt = atan2(yyOut[igate], xxOut[igate]);
        double valFilt = _getFoldValue(angleFilt,
                                       outField->getFoldLimitLower(),
                                       outField->getFoldRange());
        outData[igate] = valFilt;
      }
    }

    delete[] xxIn;
    delete[] yyIn;
    delete[] xxOut;
    delete[] yyOut;

  }

  // convert output to to input type

  outField->setDataFl32(outField->getNPoints(), outData, true);
  outField->convertToType(inputType);

  return outField;

}

void ComputeEngine::_applyLeastSquares(int mm,
                                       int nGates,
                                       const Radx::fl32 *inData,
                                       Radx::fl32 *outData,
                                       Radx::fl32 missing,
                                       int length)
  
{

  // loop through gates, computing filtered value
  
  for (int igate = mm; igate < nGates - mm; igate++) {
    double count = 0.0;
    double sumx = 0.0;
    double sumx2 = 0.0;
    double sumy = 0.0;
    double sumxy = 0.0;
    for (int ii = 0; ii < length; ii++) {
      Radx::fl32 inVal = inData[igate - mm + ii];
      if (inVal != missing) {
        double xx = ii - mm;
        double yy = inVal;
        count++;
        sumx += xx;
        sumx2 += xx * xx;
        sumy += yy;
        sumxy += xx * yy;
      }
    }
    if (count > mm) {
      double term1 = count * sumx2  - sumx * sumx;
      double term2 = sumy * sumx2 - sumx * sumxy;
      outData[igate] = term2 / term1;
    } else {
      outData[igate] = missing;
    }
  } // igate

}

/////////////////////////////////////////////////////////////////
// set folding behavior as appropriate

void ComputeEngine::_setFoldLimits(RadxField *outField)

{

  if (_params.set_fold_limits) {
    for (int ii = 0; ii < _params.folded_fields_n; ii++) {
      string radxName = _params._folded_fields[ii].input_name;
      if (radxName == outField->getName()) {
        if (_params._folded_fields[ii].field_folds) {
          double foldLimitLower = _params._folded_fields[ii].fold_limit_lower;
          double foldLimitUpper = _params._folded_fields[ii].fold_limit_upper;
          if (_params._folded_fields[ii].use_global_nyquist) {
            foldLimitLower = -_nyquist;
            foldLimitUpper = _nyquist;
          }
          outField->setFieldFolds(foldLimitLower, foldLimitUpper);
        }
        return;
      }
    } // ii
  } // if (_params.set_fold_limits)

}
  
/////////////////////////////////////////////////////////////////
// convert a value to an angle, for a field that folds

double ComputeEngine::_getFoldAngle(double val,
                                    double foldLimitLower,
                                    double foldRange)
{
  double fraction = (val - foldLimitLower) / foldRange;
  double angle = -M_PI + fraction * (M_PI * 2.0);
  return angle;
}

/////////////////////////////////////////////////////////////////
// convert an angle to a value, for a field that folds

double ComputeEngine::_getFoldValue(double angle,
                                    double foldLimitLower,
                                    double foldRange)
{
  double fraction = (angle + M_PI) / (M_PI * 2.0);
  double val = fraction * foldRange + foldLimitLower;
  return val;
}

