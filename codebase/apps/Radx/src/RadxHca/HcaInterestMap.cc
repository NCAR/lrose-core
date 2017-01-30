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
// HcaInterestMap.cc
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2016
//
///////////////////////////////////////////////////////////////
//
// Handles interest mapping. Converts a data value into an
// interest value based on a piece-wise linear function.
//
///////////////////////////////////////////////////////////////

#include <iostream>
#include <cmath>
#define _in_interest_map_cc
#include "HcaInterestMap.hh"
using namespace std;

// Constructor

HcaInterestMap::HcaInterestMap(imap_class_t hcaClass,
                               imap_feature_t feature,
                               double x1,
                               double x2,
                               double x3,
                               double x4,
                               double weight) :
        _hcaClass(hcaClass),
        _feature(feature),
        _weight(weight)
        
{

  // compute label

  _label = hcaClassToStr(hcaClass);
  _label += "-";
  _label += hcaFeatureToStr(feature);
  
  // set shape parameters

  _shape.xx1 = x1;
  _shape.xx2 = x2;
  _shape.xx3 = x3;
  _shape.xx4 = x4;
  _computeShapeSlopes(_shape);

  // compute lookup table as appropriate
  // for particular features and classes
  // for dbz from -40 to +100
  
  _useLut = false;
  _lutMinDbz = -40.0;
  _lutDeltaDbz = 1.0;
  _lutSize = 140;
  
  if (_feature == FeatureZDR) {
    if (_hcaClass == ClassGR) {
      _loadLut(DbzFunctionConst, -0.3,
               DbzFunctionConst, 0.0,
               DbzFunctionF1, 0.0,
               DbzFunctionF1, 0.3);
    } else if (_hcaClass == ClassBD) {
      _loadLut(DbzFunctionF2, -0.3,
               DbzFunctionF2, 0.0,
               DbzFunctionF3, 0.0,
               DbzFunctionF3, 1.0);
    } else if (_hcaClass == ClassRA) {
      _loadLut(DbzFunctionF1, -0.3,
               DbzFunctionF1, 0.0,
               DbzFunctionF2, 0.0,
               DbzFunctionF2, 0.5);
    } else if (_hcaClass == ClassHR) {
      _loadLut(DbzFunctionF1, -0.3,
               DbzFunctionF1, 0.0,
               DbzFunctionF2, 0.0,
               DbzFunctionF2, 0.5);
    } else if (_hcaClass == ClassRH) {
      _loadLut(DbzFunctionConst, -0.3,
               DbzFunctionConst, 0.0,
               DbzFunctionF1, 0.0,
               DbzFunctionF1, 0.5);
    }
  } else if (feature == FeatureLOG_KDP) {
    if (_hcaClass == ClassBD) {
      _loadLut(DbzFunctionG1, -1.0,
               DbzFunctionG1, 0.0,
               DbzFunctionG2, 0.0,
               DbzFunctionG2, 1.0);
    } else if (_hcaClass == ClassRA) {
      _loadLut(DbzFunctionG1, -1.0,
               DbzFunctionG1, 0.0,
               DbzFunctionG2, 0.0,
               DbzFunctionG2, 1.0);
    } else if (_hcaClass == ClassHR) {
      _loadLut(DbzFunctionG1, -1.0,
               DbzFunctionG1, 0.0,
               DbzFunctionG2, 0.0,
               DbzFunctionG2, 1.0);
    } else if (_hcaClass == ClassRH) {
      _loadLut(DbzFunctionConst, -10.0,
               DbzFunctionConst, -4.0,
               DbzFunctionG1, 0.0,
               DbzFunctionG1, 1.0);
    }
  }

  // if using LUT, load up default shape at 35 dBZ

  if (_useLut) {
    _shape = *_getCurrentShape(35.0);
  }
  
  // check vals increase monotonically

  if (_shape.xx2 <= _shape.xx1 || 
      _shape.xx3 <= _shape.xx2 ||
      _shape.xx4 <= _shape.xx3) {
    cerr << "WARNING - HcaInterestMap, label: " << _label << endl;
    cerr << "  Map values must increase monotonically.";
    print(cerr);
  }

}

// destructor

HcaInterestMap::~HcaInterestMap()
  
{
  _shapeLut.clear();
}

///////////////////////////////////////////////////////////
// compute interest from value

double HcaInterestMap::getInterest(double dbz, double val)
  
{

  imap_shape_t *shape = _getCurrentShape(dbz);
  double interest = _getInterest(shape, val);
  return interest;

}

///////////////////////////////////////////////////////////
// compute weighted interest from value

void HcaInterestMap::getWeightedInterest(double dbz,
                                         double val,
                                         double &interest,
                                         double &wt)

{

  imap_shape_t *shape = _getCurrentShape(dbz);
  interest = _getInterest(shape, val) * _weight;
  wt = _weight;

}

///////////////////////////////////////////////////////////
// accumulate weighted interest based on value

void HcaInterestMap::accumWeightedInterest(double dbz,
                                           double val,
                                           double &sumInterest,
                                           double &sumWt)
  
{
  
  imap_shape_t *shape = _getCurrentShape(dbz);
  double interest = _getInterest(shape, val);
  if (fabs(interest) > 0.00001) {
    sumInterest += interest * _weight;
  }
  sumWt += _weight;

  // cerr << "label, dbz, val, wt, interest: "
  //      << _label << ", " << dbz << ", "
  //      << val << ", " << _weight << ", " << interest << endl;

}

///////////////////////////////////////////////////////////
// Print interest map parameters

void HcaInterestMap::print(ostream &out)
  
{

  out << "---------------------------------------------" << endl;
  out << "Interest map: " << _label << endl;
  out << "       class: " << hcaClassToStr(_hcaClass) << endl;
  out << "     feature: " << hcaFeatureToStr(_feature) << endl;
  out << "         xx1: " << _shape.xx1 << endl;
  out << "         xx2: " << _shape.xx2 << endl;
  out << "         xx3: " << _shape.xx3 << endl;
  out << "         xx4: " << _shape.xx4 << endl;
  out << "     delta12: " << _shape.delta12 << endl;
  out << "     delta34: " << _shape.delta34 << endl;
  out << "     slope12: " << _shape.slope12 << endl;
  out << "     slope34: " << _shape.slope34 << endl;
  out << "      weight: " << _weight << endl;
  out << "      useLut: " << (_useLut?"Y":"N") << endl;

}

///////////////////////////////////////////////////////////
// Compute shape slopes based on x values

void HcaInterestMap::_computeShapeSlopes(imap_shape_t &shape)
{
  shape.delta12 = shape.xx2 - shape.xx1;
  shape.delta34 = shape.xx4 - shape.xx3;
  shape.slope12 = +1.0 / shape.delta12;
  shape.slope34 = -1.0 / shape.delta34;
}

///////////////////////////////////////////////////////////
// Load up LUT by dbz value

void HcaInterestMap::_loadLut(imap_dbz_function_t fx1, double cx1,
                              imap_dbz_function_t fx2, double cx2,
                              imap_dbz_function_t fx3, double cx3,
                              imap_dbz_function_t fx4, double cx4)
  
{

  for (int ii = 0; ii < _lutSize; ii++) {
    
    double dbz = _lutMinDbz + ii * _lutDeltaDbz;

    imap_shape_t shape;
    shape.xx1 = _computeShapeValueForDbz(fx1, cx1, dbz);
    shape.xx2 = _computeShapeValueForDbz(fx2, cx2, dbz);
    shape.xx3 = _computeShapeValueForDbz(fx3, cx3, dbz);
    shape.xx4 = _computeShapeValueForDbz(fx4, cx4, dbz);

    // ensure monotonicity

    if (shape.xx2 <= shape.xx1) {
      shape.xx2 = shape.xx1 + fabs(shape.xx1 * 1.01) + 0.001;
    }
    if (shape.xx3 <= shape.xx2) {
      shape.xx3 = shape.xx2 + fabs(shape.xx2 * 1.01) + 0.001;
    }
    if (shape.xx4 <= shape.xx3) {
      shape.xx4 = shape.xx3 + fabs(shape.xx3 * 1.01) + 0.001;
    }

    // compute deltas and slopes

    _computeShapeSlopes(shape);

    _shapeLut.push_back(shape);

  }

  _useLut = true;

}

double HcaInterestMap::_computeShapeValueForDbz(imap_dbz_function_t fx, 
                                                double cx,
                                                double dbz)
  
{
  
  switch (fx) {
    case DbzFunctionConst:
      return cx;
    case DbzFunctionF1:
      return (cx - 0.50 + 2.50e-3 * dbz + 7.50e-4 * dbz * dbz);
    case DbzFunctionF2:
      return (cx + 0.68 + -4.81e-2 * dbz + 2.920e-3 * dbz * dbz);
    case DbzFunctionF3:
      return (cx + 1.42 + 6.67e-2 * dbz + 4.85e-4 * dbz * dbz);
    case DbzFunctionG1:
      return (cx - 44.0 + 0.8 * dbz);
    case DbzFunctionG2:
      return (cx = cx - 22.0 + 0.5 * dbz);
  }
  
  return 0.0;

}
  
///////////////////////////////////////////////////////////
// get shape for a given DBZ value

HcaInterestMap::imap_shape_t *HcaInterestMap::_getCurrentShape(double dbz)
{
  if (!_useLut) {
    // use as is
    return &_shape;
  }
  // set _shape from LUT
  int index = (int) ((dbz - _lutMinDbz) / _lutDeltaDbz + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _lutSize - 1) {
    index = _lutSize - 1;
  }
  return &_shapeLut[index];
}

///////////////////////////////////////////////////////////
// get interest for val and shape

double HcaInterestMap::_getInterest(const imap_shape_t *shape,
                                    double val)
{

  if (val <= shape->xx1 || val >= shape->xx4) {
    return 0.0;
  }

  if (val >= shape->xx2 && val <= shape->xx3) {
    return 1.0;
  }

  if (val >= shape->xx1 && val <= shape->xx2) {
    return (val - shape->xx1) * shape->slope12;
  }

  if (val >= shape->xx3 && val <= shape->xx4) {
    return 1.0 + (val - shape->xx3) * shape->slope34;
  }

  return 0.0;

}
  
/////////////////////////////////
// get string for classification

string HcaInterestMap::hcaClassToStr(imap_class_t hcaClass)
{
  switch (hcaClass) {
    case ClassGC:
      return "GC";
      break;
    case ClassBS:
      return "BS";
      break;
    case ClassDS:
      return "DS";
      break;
    case ClassWS:
      return "WS";
      break;
    case ClassCR:
      return "CR";
      break;
    case ClassGR:
      return "GR";
      break;
    case ClassBD:
      return "BD";
      break;
    case ClassRA:
      return "RA";
      break;
    case ClassHR:
      return "HR";
      break;
    case ClassRH:
      return "RH";
  }
  return "UNKNOWN";
}

/////////////////////////////////
// get string for featureification

string HcaInterestMap::hcaFeatureToStr(imap_feature_t hcaFeature)
{
  switch (hcaFeature) {
    case FeatureDBZ:
      return "DBZ";
      break;
    case FeatureZDR:
      return "ZDR";
      break;
    case FeatureRHOHV:
      return "RHOHV";
      break;
    case FeatureLOG_KDP:
      return "LOG_KDP";
      break;
    case FeatureSD_DBZ:
      return "SD_DBZ";
      break;
    case FeatureSD_PHIDP:
      return "SD_PHIPD";
      break;
  }
  return "UNKNOWN";
}

