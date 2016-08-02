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

    _lut[ii] = interest * _weight;

  }

  _mapLoaded = true;

//    if (_label == "velocity") {
//      for (double val = -5.0; val < 5.0; val+= 0.1) {
//        cerr << "val, interest: " << val << ", " << getInterest(val) << endl;
//      }
//  //      for (int ii = 0; ii < _nLut; ii++) {
//  //        cerr << "val, interest: " << _minVal + ii * _dVal << ", " << _lut[ii] << endl;
//  //      }
//    }

}

// destructor

InterestMap::~InterestMap()
  
{
  if (_lut) {
    delete[] _lut;
  }
}

///////////////////////////////////////////////////////////
// compute interest from value

void InterestMap::getInterest(double val, double &interest, double &wt)

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
  
  interest = _lut[index];
  wt = _weight;

}

///////////////////////////////////////////////////////////
// accumulate interest based on value
//

void InterestMap::accumInterest(double val,
				double &sumInterest, double &sumWt)
  
{
  
  if (!_mapLoaded || val == _missingDbl) {
    return;
  }

  int index = (int) floor((val - _minVal) / _dVal + 0.5);
  if (index < 0) {
    index = 0;
  } else if (index > _nLut - 1) {
    index = _nLut - 1;
  }
  
  sumInterest += _lut[index];
  sumWt += _weight;

}

