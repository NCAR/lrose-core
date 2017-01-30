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
// InterestMap.hh
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 2005
//
///////////////////////////////////////////////////////////////
//
// Handles interest mapping. Converts a data value into an
// interest value based on a linear function.
//
///////////////////////////////////////////////////////////////

#ifndef InterestMap_hh
#define InterestMap_hh

#include <string>
#include <vector>
using namespace std;

class InterestMap {
  
public:

  class ImPoint {
  public:
    ImPoint(double val, double interest) {
      _val = val;
      _interest = interest;
    }
    double getVal() const { return _val; }
    double getInterest() const { return _interest; }
  private:
    double _val;
    double _interest;
  };

  InterestMap(const string &label,
	      const vector<ImPoint> &map,
	      double weight);

  ~InterestMap();

  // set the missing value

  void setMissingValue(double miss) { _missingDbl = miss; }
  
  // get interest for a given val

  double getInterest(double val);
  
  // get weighted interest for a given val

  void getWeightedInterest(double val, double &interest, double &wt);
  
  // accumulate weighted interest based on value
  // If interest is less than minValidInterest, the sum
  // is not performed.

  void accumWeightedInterest(double val,
                             double &sumInterest,
                             double &sumWt,
                             double minValidInterest = 0.00001);
  
  // get weight

  double getWeight() const { return _weight; }
  
  // Print interest map parameters
  
  void printParams(ostream &out);
  
protected:
private:
  
  static double _missingDbl;
  static const int _nLut = 10001;

  string _label;
  vector<ImPoint> _map;
  double _weight;

  bool _mapLoaded;
  double _minVal;
  double _maxVal;
  double _dVal;

  double *_lut;
  double *_weightedLut;

};

#ifdef _in_interest_map_cc
double InterestMap::_missingDbl = -9999.0;
#endif

#endif

