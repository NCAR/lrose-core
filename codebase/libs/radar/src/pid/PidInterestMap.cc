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
// PidInterestMap.cc
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
#include <radar/PidInterestMap.hh>
using namespace std;

// Constructor

PidInterestMap::PidInterestMap(const string &label,
			       double minDbz,
			       double maxDbz,
			       const vector<ImPoint> &map,
			       double weight,
			       double missingVal) :
  _label(label),
  _minDbz(minDbz),
  _maxDbz(maxDbz),
  _weight(weight),
  _missingDouble(missingVal)

{
  
  _lut = NULL;
  _weightedLut = NULL;
  _mapLoaded = false;
  _minVal = 0.0;
  _maxVal = 0.0;

  if (map.size() < 2) {
    cerr << "WARNING - PidInterestMap, label: " << _label << endl;
    cerr << "  Map must have at least 2 points.";
    return;
  }

  // build up map points, checking for bad ones
  
  bool warnings = false;
  _map.push_back(map[0]);
  for (int ii = 1; ii < (int) map.size(); ii++) {
    if (map[ii].getVal() == map[ii-1].getVal() &&
        map[ii].getInterest() == map[ii-1].getInterest()) {
      cerr << "WARNING - PidInterestMap - error in interest map" << endl;
      cerr << "  Label: " << _label << endl;
      cerr << "  2 consecutive points are identical" << endl;
      cerr << "  Map index: " << ii << endl;
      cerr << "  val, interest: "
           << map[ii].getVal() << ", " << map[ii].getInterest() << endl;
      cerr << "  Ignoring this point" << endl;
      warnings = true;
      continue;
    }
    _map.push_back(map[ii]);
  }

  if (_map.size() < 2) {
    cerr << "WARNING - PidInterestMap, label: " << _label << endl;
    cerr << "  Map must have at least 2 points.";
    return;
  }
  
  // warn on bad interest values

  for (int ii = 0; ii < (int) _map.size(); ii++) {
    if (_map[ii].getInterest() < 0.0 || _map[ii].getInterest() > 1.0) {
      cerr << "WARNING - PidInterestMap - error in interest map" << endl;
      cerr << "  Label: " << _label << endl;
      cerr << "  Interest is not in range [0.0, 1.0]" << endl;
      cerr << "  Map index: " << ii << endl;
      cerr << "  val, interest: "
           << _map[ii].getVal() << ", " << _map[ii].getInterest() << endl;
      warnings = true;
    }
  }

  // warn on non increasing x values

  for (int ii = 1; ii < (int) _map.size(); ii++) {
    if (_map[ii].getVal() <= _map[ii-1].getVal()) {
      cerr << "WARNING - PidInterestMap - error in interest map" << endl;
      cerr << "  Label: " << _label << endl;
      cerr << "  Interest map is not monotonically increasing" << endl;
      cerr << "  Map index: " << ii << endl;
      cerr << "  This val, interest: "
           << _map[ii].getVal() << ", " << _map[ii].getInterest() << endl;
      cerr << "  Prev val, interest: "
           << _map[ii-1].getVal() << ", " << _map[ii-1].getInterest() << endl;
      cerr << "  Setting to previous value plus a small value" << endl;
      _map[ii].setVal(_map[ii-1].getVal() + 0.0001);
      warnings = true;
    }
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
    
    double interest = (_map[mapIndex-1].getInterest() +
                       (val - _map[mapIndex-1].getVal()) * slope);
    
    _lut[ii] = interest;
    _weightedLut[ii] = interest * _weight;

  }

  _mapLoaded = true;

  if (warnings) {
    cerr << "======= WARNING - modified interest map =========" << endl;
    print(cerr);
    cerr << "=================================================" << endl;
  }

}

// destructor

PidInterestMap::~PidInterestMap()
  
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

double PidInterestMap::getInterest(double val) const

{

  if (!_mapLoaded || val == _missingDouble) {
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

void PidInterestMap::getWeightedInterest(double val,
					 double &interest,
					 double &wt) const

{

  if (!_mapLoaded || val == _missingDouble) {
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

void PidInterestMap::accumWeightedInterest(double val,
					   double &sumWtInterest,
					   double &sumWt) const
  
{
  
  if (!_mapLoaded || val == _missingDouble || fabs(_weight) < 0.001) {
    return;
  }

  int index = (int) floor((val - _minVal) / _dVal + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _nLut - 1) {
    index = _nLut - 1;
  }

  double weightedInterest = _weightedLut[index];

  sumWtInterest += weightedInterest;
  sumWt += _weight;

}

///////////////////////////////////////////////////////////
// print

void PidInterestMap::print(ostream &out) const
  
{

  out << "------ interest map ------" << endl;
  out << "  label: " << _label << endl;
  out << "  minDbz: " << _minDbz << endl;
  out << "  maxDbz: " << _maxDbz << endl;
  out << "  weight: " << _weight << endl;
  for (int ii = 0; ii < (int) _map.size(); ii++) {
    out << "  pt: val, interest: "
	<< _map[ii].getVal() << ", " << _map[ii].getInterest() << endl;
  }

}  
