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
// InterestMap.cc
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

#include <iostream>
#include <cmath>
#include <cassert>
#define _in_interest_map_cc
#include <radar/InterestMap.hh>
using namespace std;

// Constructor

InterestMap::InterestMap(const string &label,
			 const vector<ImPoint> &map,
			 double weight) :
  _label(label),
  _map(map),
  _weight(weight)

{
  
  _lut = NULL;
  _weightedLut = NULL;
  _mapLoaded = false;
  _minVal = 0.0;
  _maxVal = 0.0;

  // check we have at least 2 points

  if (_map.size() < 2) {
    cerr << "ERROR - InterestMap, label: " << _label << endl;
    cerr << "  Map must have at least 2 points.";
    assert(false);
  }

  // check vals increase monotonically
  
  double prevVal = map[0].getVal();
  bool success = true;
  for (size_t ii = 1; ii < map.size(); ii++) {
    double val = map[ii].getVal();
    if (val <= prevVal) {
      cerr << "ERROR - InterestMap, label: " << _label << endl;
      cerr << "  Map values must increase monotonically.";
      cerr << "  index: " << ii << endl;
      cerr << "  prevVal: " << prevVal << endl;
      cerr << "  thisVal: " << val << endl;
      success = false;
    }
  }
  assert(success);

  _minVal = _map[0].getVal();
  _maxVal = _map[_map.size() - 1].getVal();
  _dVal = (_maxVal - _minVal) / (_nLut - 1.0);
  _lut = new double[_nLut];
  _weightedLut = new double[_nLut];

  int mapIndex = 1;
  double slope =
    ((_map[mapIndex].getInterest() - _map[mapIndex-1].getInterest()) /
     (_map[mapIndex].getVal() - _map[mapIndex-1].getVal()));

  for (int ii = 0; ii < _nLut; ii++) {

    double val = _minVal + ii * _dVal;
    if ((val > _map[mapIndex].getVal()) &&
	(mapIndex < (int) _map.size() - 1)) {
      mapIndex++;
      slope =
	((_map[mapIndex].getInterest() - _map[mapIndex-1].getInterest()) /
	 (_map[mapIndex].getVal() - _map[mapIndex-1].getVal()));
    }

    double interest = _map[mapIndex-1].getInterest() +
      (val - _map[mapIndex-1].getVal()) * slope;

    _lut[ii] = interest;
    _weightedLut[ii] = interest * _weight;

  }

  _mapLoaded = true;
  
#ifdef DEBUG_PRINT
  cerr << "Interest map, label: " << _label << endl;
  for (double val = -10.0; val < 10.0; val+= 0.1) {
    double interest, wt;
    getInterest(val, interest, wt);
    cerr << "val, interest: " << val << ", " << interest << endl;
  }
  for (int ii = 0; ii < _nLut; ii++) {
    cerr << "val, interest: " << _minVal + ii * _dVal << ", " << _lut[ii] << endl;
  }
#endif
  
}

// destructor

InterestMap::~InterestMap()
  
{
  if (_lut) {
    delete[] _lut;
  }
  if (_weightedLut) {
    delete[] _weightedLut;
  }
}

///////////////////////////////////////////////////////////
// compute interest from value

double InterestMap::getInterest(double val)

{

  if (!_mapLoaded || val == _missingDbl) {
    return 0.0;
  }

  int index = (int) floor((val - _minVal) / _dVal + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _nLut - 1) {
    index = _nLut - 1;
  }
  
  return _lut[index];

}

///////////////////////////////////////////////////////////
// compute weighted interest from value

void InterestMap::getWeightedInterest(double val,
                                      double &interest,
                                      double &wt)

{

  if (!_mapLoaded || val == _missingDbl) {
    interest = 0.0;
    wt = 0.0;
    return;
  }

  int index = (int) floor((val - _minVal) / _dVal + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _nLut - 1) {
    index = _nLut - 1;
  }
  
  interest = _weightedLut[index];
  wt = _weight;

}

///////////////////////////////////////////////////////////
// accumulate weighted interest based on value
// If interest is less than minValidInterest, the sum
// is not performed.

void InterestMap::accumWeightedInterest(double val,
                                        double &sumInterest,
                                        double &sumWt,
                                        double minValidInterest /* = 0.00001 */)
  
{
  
  if (!_mapLoaded || val == _missingDbl || fabs(_weight) < 0.001) {
    return;
  }

  int index = (int) floor((val - _minVal) / _dVal + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _nLut - 1) {
    index = _nLut - 1;
  }

  double interest = _weightedLut[index];

  if (fabs(interest) > minValidInterest) {
    sumInterest += interest;
    sumWt += _weight;
  }

#ifdef DEBUG_PRINT  
  cerr << "label, val, interest, sumInt, sumWt: "
       << _label << ", " << val << ", " << interest
       << ", " << sumInterest << ", " << sumWt << endl;
#endif

}

///////////////////////////////////////////////////////////
// Print interest map parameters

void InterestMap::printParams(ostream &out)
  
{

  out << "=============================================" << endl;
  out << "Interest map: " << _label << endl;
  out << "      weight: " << _weight << endl;
  out << "      points: " << endl;
  for (size_t ii = 0; ii < _map.size(); ii++) {
    out << "      val, interest: "
        << _map[ii].getVal() << ", " << _map[ii].getInterest() << endl;
  }
  if (!_mapLoaded) {
    out << "NOTE - InterestMap::printParams" << endl;
    out << "  Map not yet loaded" << endl;
    return;
  }

}
