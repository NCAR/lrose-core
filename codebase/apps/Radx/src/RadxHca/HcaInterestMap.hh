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
/////////////////////////////////////////////////////////////
// HcaInterestMap.hh
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

#ifndef HcaInterestMap_hh
#define HcaInterestMap_hh

#include <string>
#include <vector>
using namespace std;

class HcaInterestMap {
  
public:

  typedef struct {
    double xx1;
    double xx2;
    double xx3;
    double xx4;
    double delta12; // xx2 - xx1
    double delta34; // xx4 - xx3
    double slope12; // slope from xx1 to xx2
    double slope34; // slope from xx3 to xx4
  } imap_shape_t;

  typedef enum {
    ClassGC = 0,
    ClassBS = 1,
    ClassDS = 2,
    ClassWS = 3,
    ClassCR = 4,
    ClassGR = 5,
    ClassBD = 6,
    ClassRA = 7,
    ClassHR = 8,
    ClassRH = 9
  } imap_class_t;
  
  typedef enum {
    FeatureDBZ = 0,
    FeatureZDR = 1,
    FeatureRHOHV = 2,
    FeatureLOG_KDP = 3,
    FeatureTD_DBZ = 4,
    FeatureTD_PHIDP = 5
  } imap_feature_t;
  
  typedef enum {
    DbzFunctionConst = 0,
    DbzFunctionF1 = 1,
    DbzFunctionF2 = 2,
    DbzFunctionF3 = 3,
    DbzFunctionG1 = 4,
    DbzFunctionG2 = 5
  } imap_dbz_function_t;
  
  HcaInterestMap(imap_class_t hcaClass,
                 imap_feature_t feature,
                 double x1,
                 double x2,
                 double x3,
                 double x4,
                 double weight);
  
  ~HcaInterestMap();
  
  // get class and feature type

  imap_class_t getClass() const { return _hcaClass; }
  imap_feature_t getFeature() const { return _feature; }
  
  // get interest for a given val
  
  double getInterest(double dbz, double val);

  // get weighted interest for a given val
  
  void getWeightedInterest(double dbz, double val, double &interest, double &wt);
  
  // accumulate weighted interest based on value
  
  void accumWeightedInterest(double dbz, double val,
                             double &sumInterest, double &sumWt);
  
  // Print interest map parameters
  
  void print(ostream &out);
  
  // get string for classification
  
  static string hcaClassToStr(imap_class_t hcaClass);
  
  // get string for feature field
  
  static string hcaFeatureToStr(imap_feature_t hcaFeature);

protected:
private:
  
  static const double _missingDbl;
  static const int _nLut = 10001;

  string _label;

  imap_class_t _hcaClass;
  imap_feature_t _feature;
  imap_shape_t _shape;
  double _weight;

  bool _useLut;
  vector<imap_shape_t> _shapeLut;
  double _lutMinDbz;
  double _lutDeltaDbz;
  int _lutSize;

  // functions

  void _computeShapeSlopes(imap_shape_t &shape);

  void _loadLut(imap_dbz_function_t fx1, double cx1,
                imap_dbz_function_t fx2, double cx2,
                imap_dbz_function_t fx3, double cx3,
                imap_dbz_function_t fx4, double cx4);
  
  double _computeShapeValueForDbz(imap_dbz_function_t fx, 
                                  double cx,
                                  double dbz);
  
  imap_shape_t *_getCurrentShape(double dbz);

  double _getInterest(const imap_shape_t *shape,
                      double val);

};

#ifdef _in_interest_map_cc
const double HcaInterestMap::_missingDbl = -9999.0;
#endif

#endif

