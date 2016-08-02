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
#define _in_interest_map_cc
#include "InterestMap.hh"
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

  if (_map.size() < 2) {
    cerr << "WARNING - InterestMap, label: " << _label << endl;
    cerr << "  Map must have at least 2 points.";
    return;
  }

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
  
//   cerr << "Interest map, label: " << _label << endl;
//   for (double val = -10.0; val < 10.0; val+= 0.1) {
//     double interest, wt;
//     getInterest(val, interest, wt);
//     cerr << "val, interest: " << val << ", " << interest << endl;
//   }
//   for (int ii = 0; ii < _nLut; ii++) {
//     cerr << "val, interest: " << _minVal + ii * _dVal << ", " << _lut[ii] << endl;
//   }
  
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

void InterestMap::accumWeightedInterest(double val,
                                        double &sumInterest,
                                        double &sumWt)
  
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
  
  // cerr << "val, interest: " << val << ", " << interest << endl;

  if (fabs(interest) > 0.00001) {
    sumInterest += interest;
    sumWt += _weight;
  }

}

