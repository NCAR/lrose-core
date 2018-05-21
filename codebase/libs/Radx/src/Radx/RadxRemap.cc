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
// RadxRemap.cc
//
// Remap range geometry
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Feb 2011
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxRemap.hh>
#include <cmath>
using namespace std;

//////////////
// Constructor

RadxRemap::RadxRemap()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxRemap::RadxRemap(const RadxRemap &rhs)
     
{
  _copy(rhs);
}

/////////////
// destructor

RadxRemap::~RadxRemap()

{
}

/////////////////////////////
// Assignment
//

RadxRemap &RadxRemap::operator=(const RadxRemap &rhs)
  

{
  return _copy(rhs);
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxRemap::_init()
  
{
  _gateSpacingIsConstant = true;
  _startRangeKm = 0.5;
  _gateSpacingKm = 1.0;
  _remappingRequired = false;
  _rangeArray.clear();
  _nearest.clear();
  _indexBefore.clear();
  _indexAfter.clear();
  _wtBefore.clear();
  _wtAfter.clear();
  _nGatesInterp = 0;
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxRemap &RadxRemap::_copy(const RadxRemap &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }
  
  _gateSpacingIsConstant = rhs._gateSpacingIsConstant;
  _startRangeKm = rhs._startRangeKm;
  _gateSpacingKm = rhs._gateSpacingKm;
  _remappingRequired = rhs._remappingRequired;
  _rangeArray = rhs._rangeArray;
  _nearest = rhs._nearest;

  _indexBefore = rhs._indexBefore;
  _indexAfter = rhs._indexAfter;
  _wtBefore = rhs._wtBefore;
  _wtAfter = rhs._wtAfter;
  _nGatesInterp = rhs._nGatesInterp;

  return *this;
  
}

////////////////////////////////////////////////////////////////
/// Compute lookup from range array
///
/// If constant, startRange and gateSpacing are derived from the
/// vector of ranges.
///
/// If not constant, a lookup vector will be computed for
/// remapping the non-constant range data onto a constant
/// delta range.
///
/// Use getGateSpacingIsConstant() to check for constant gate spacing.
///     getStartRangeKm() and getGateSpacingKm() if spacing is constant.
///     getRangeArray() for saved range array.
///     getLookupNearest() for lookup table.
///
/// Returns 0 on success, -1 on error

int RadxRemap::computeRangeLookup(const vector<double> &rangeArray)

{
  
  _init();
  _rangeArray = rangeArray;
  
  if (_rangeArray.size() < 1) {
    _gateSpacingIsConstant = true;
    return 0;
  }
  _startRangeKm = _rangeArray[0];
  
  if (_rangeArray.size() < 2) {
    _gateSpacingIsConstant = true;
    return 0;
  }
  _gateSpacingKm = fabs(_rangeArray[1] - _rangeArray[0]);
  if (_gateSpacingKm <= 0) {
    cerr << "ERROR - RadxRemap::computeRangeLookup()" << endl;
    cerr << "  Bad range array" << endl;
    cerr << "  _rangeArray[0]: " << _rangeArray[0] << endl;
    cerr << "  _rangeArray[1]: " << _rangeArray[1] << endl;
    return -1;
  }
  
  // check for constant geometry

  _remappingRequired = false;
  double minSpacing = _gateSpacingKm;
  for (size_t ii = 2; ii < _rangeArray.size(); ii++) {
    double spacing = fabs(_rangeArray[ii] - _rangeArray[ii-1]);
    if (fabs(spacing - _gateSpacingKm) > 1.0e-6) {
      _remappingRequired = true;
    }
    if (spacing < minSpacing) {
      minSpacing = spacing;
    }
  }
  
  if (!_remappingRequired) {
    _gateSpacingIsConstant = true;
    return 0;
  }
  
  // spacing is not constant, compute remap lookup table
  // using the min spacing

  _gateSpacingKm = minSpacing;

  double maxRange = _rangeArray[_rangeArray.size()-1];
  double rangeSpan = maxRange - _startRangeKm;

  if (rangeSpan <= 0) {
    cerr << "ERROR - RadxRemap::computeRangeLookup()" << endl;
    cerr << "  Bad range span" << endl;
    cerr << "  first gate range: " << _rangeArray[0] << endl;
    cerr << "  last  gate range: " << _rangeArray[_rangeArray.size()-1] << endl;
    return -1;
  }

  // create lookup vector
  
  int nLookup = (int) (rangeSpan / _gateSpacingKm + 0.5);
  for (int ii = 0; ii < nLookup; ii++) {
    _nearest.push_back(-1);
  }

  // insert lookup info into lookup vector - this may leave gaps
  
  for (size_t irange = 0; irange < _rangeArray.size(); irange++) {
    double range = _rangeArray[irange];
    int ilut = (int) ((range - _startRangeKm) / _gateSpacingKm + 0.5);
    if (ilut < 0) {
      ilut = 0;
    } else if (ilut > nLookup - 1) {
      ilut = nLookup - 1;
      if (ilut < 0) {
        ilut = 0;
      }
    }
    _nearest[ilut] = irange;
  }

  // make sure end points are filled
  
  if (_nearest[0] < 0) {
    _nearest[0] = 0;
  }
  if (_nearest[nLookup-1] < 0) {
    _nearest[nLookup-1] = _rangeArray.size() - 1;
  }
  
  // fill in the gaps

  class FillInfo {
  public:
    FillInfo() {
      fwdIrange = 0;
      fwdDist = 0;
      revIrange = 0;
      revDist = 0;
    }
    int fwdIrange;
    int fwdDist;
    int revIrange;
    int revDist;
  };
  FillInfo *fillInfo = new FillInfo[nLookup];

  // search forward for gaps

  int dist = 0;
  int irange = 0;
  for (int ilut = 0; ilut < nLookup; ilut++) {
    if (_nearest[ilut] < 0) {
      dist++;
      fillInfo[ilut].fwdIrange = irange;
      fillInfo[ilut].fwdDist = dist;
    } else {
      dist = 0;
      // irange = ilut;
      irange = _nearest[ilut];
    }
  }

  // search in reverse for gaps

  dist = 0;
  irange = nLookup - 1;
  for (int ilut = nLookup - 1; ilut >= 0; ilut--) {
    if (_nearest[ilut] < 0) {
      dist++;
      fillInfo[ilut].revIrange = irange;
      fillInfo[ilut].revDist = dist;
    } else {
      dist = 0;
      // irange = ilut;
      irange = _nearest[ilut];
    }
  }

  // fill in gaps using fwd or reverse irange.
  // depending on which has the lower distance

  for (int ilut = 0; ilut < nLookup; ilut++) {
    if (_nearest[ilut] < 0) {
      const FillInfo &info = fillInfo[ilut];
      if (info.revDist < info.fwdDist) {
        _nearest[ilut] = info.revIrange;
      } else {
        _nearest[ilut] = info.fwdIrange;
      }
    }
  } // ilut

  delete[] fillInfo;

  _gateSpacingIsConstant = false;
  _nGatesInterp = _nearest.size();

  return 0;

}

////////////////////////////////////////////////////////////////
/// Check if gate spacing is constant, given range array.
/// Range array is distance to center of each gate.
///
/// Returns true if gate spacing is constant, false otherwise.

bool RadxRemap::checkGateSpacingIsConstant(const vector<double> &rangeArray)

{
  
  if (rangeArray.size() < 2) {
    return true;
  }
  
  // check for constant geometry

  double gateSpacingKm = fabs(rangeArray[1] - rangeArray[0]);
  for (size_t ii = 2; ii < rangeArray.size(); ii++) {
    double spacing = fabs(rangeArray[ii] - rangeArray[ii-1]);
    if (fabs(spacing - gateSpacingKm) > 1.0e-6) {
      return false;
    }
  }

  return true;

}
  
////////////////////////////////////////////////////////////////
// check if the geometry is different given start range and gate spacing
// uses an absolute difference of 0.00001 km as the check

bool RadxRemap::checkGeometryIsDifferent(double startRangeKm0,
                                         double gateSpacingKm0,
                                         double startRangeKm1,
                                         double gateSpacingKm1)

{

  double smallDouble = 0.00001;
  if (fabs(startRangeKm1 - startRangeKm0) < smallDouble &&
      fabs(gateSpacingKm1 - gateSpacingKm0) < smallDouble) {
    return false;
  }
  return true;

}
  
////////////////////////////////////////////////////////////////
/// Prepare for interpolation, given a difference in geometry.
///
/// Computes lookup tables and weights.
/// Prepares for both interpolationg and nearest neighbor remapping.
///
/// After running this, you can use:
///     remappingRequired() indicates the need to remapping
///     getStartRangeKm() and getGateSpacingKm() if spacing is constant.
///     getRangeArray() for saved range array.
///     getLookupNearest() for lookup table.
///     getIndexBefore()
///     getIndexAfter()
///     getWtBefore()
///     getWtAfter()

void RadxRemap::prepareForInterp(int oldNGates,
                                 double startRangeKm0,
                                 double gateSpacingKm0,
                                 double startRangeKm1,
                                 double gateSpacingKm1)
  
{
  
  // is geometry different

  if (!checkGeometryIsDifferent(startRangeKm0,
                                gateSpacingKm0,
                                startRangeKm1,
                                gateSpacingKm1)) {
    // no need to remap
    _startRangeKm = startRangeKm0;
    _gateSpacingKm = gateSpacingKm0;
    _remappingRequired = false;
    return;
  }

  _init();

  // max range

  double maxRange = startRangeKm0 + oldNGates * gateSpacingKm0;
  int newNGates = 
    (int) (((maxRange - startRangeKm1) / gateSpacingKm1) + 0.5);

  // compute lookup table
  
  for (int igate = 0; igate < newNGates; igate++) {

    double targetRange = startRangeKm1 + igate * gateSpacingKm1;

    // nearest neightbor

    int nearestIndex = 
      (int) ((targetRange - startRangeKm0) / gateSpacingKm0 + 0.5);
    if (nearestIndex < 0 || nearestIndex > oldNGates - 1) {
      nearestIndex = -1;
    }
    _nearest.push_back(nearestIndex);

    // interpolation

    int indexBefore = 
      (int) ((targetRange - startRangeKm0) / gateSpacingKm0);
    int indexAfter = indexBefore + 1;
    if (indexBefore < 0 || indexBefore > oldNGates - 1) {
      indexBefore = -1;
    }
    if (indexAfter < 0 || indexAfter > oldNGates - 1) {
      indexAfter = -1;
    }
    if (indexBefore < 0 || indexAfter < 0) {
      // we are at the extremes - cannot interpolate
      _indexBefore.push_back(indexBefore);
      _indexAfter.push_back(indexAfter);
      _wtBefore.push_back(0.0);
      _wtAfter.push_back(0.0);
    } else {
      double rangeBefore = indexBefore * gateSpacingKm0 + startRangeKm0;
      double wtAfter = (targetRange - rangeBefore) / gateSpacingKm0;
      double wtBefore = 1.0 - wtAfter;
      _indexBefore.push_back(indexBefore);
      _indexAfter.push_back(indexAfter);
      _wtBefore.push_back(wtBefore);
      _wtAfter.push_back(wtAfter);
    }

  } // igate

  _startRangeKm = startRangeKm1;
  _gateSpacingKm = gateSpacingKm1;
  _remappingRequired = true;
  _nGatesInterp = _indexBefore.size();

  return;

}

/////////////////////////////////////////////////////////
// print geometry

void RadxRemap::print(ostream &out) const
  
{
  
  out << "  RadxRemap:" << endl;
  out << "    remappingRequired: " << _remappingRequired << endl;
  out << "    startRangeKm: " << _startRangeKm << endl;
  out << "    gateSpacingKm: " << _gateSpacingKm << endl;
  if (_rangeArray.size() > 0) {
    out << "    nGatesIn: " << _rangeArray.size() << endl;
    for (size_t ii = 0; ii < _rangeArray.size(); ii++) {
      out << "      range[" << ii << "]: " << _rangeArray[ii] << endl;
    }
  }
  if (_nearest.size() > 0) {
    out << "    nGatesLookupNearest: " << _nearest.size() << endl;
    for (size_t ii = 0; ii < _nearest.size(); ii++) {
      out << "      nearest[" << ii << "]: " << _nearest[ii] << endl;
    }
  }
  if (_indexBefore.size() > 0) {
    out << "    indexBefore.size(): " << _indexBefore.size() << endl;
    for (size_t ii = 0; ii < _indexBefore.size(); ii++) {
      out << "      indexBefore[" << ii << "]: " << _indexBefore[ii] << endl;
    }
  }
  if (_indexAfter.size() > 0) {
    out << "    indexAfter.size(): " << _indexAfter.size() << endl;
    for (size_t ii = 0; ii < _indexAfter.size(); ii++) {
      out << "      indexAfter[" << ii << "]: " << _indexAfter[ii] << endl;
    }
  }
  if (_wtBefore.size() > 0) {
    out << "    wtBefore.size(): " << _wtBefore.size() << endl;
    for (size_t ii = 0; ii < _wtBefore.size(); ii++) {
      out << "      wtBefore[" << ii << "]: " << _wtBefore[ii] << endl;
    }
  }
  if (_wtAfter.size() > 0) {
    out << "    wtAfter.size(): " << _wtAfter.size() << endl;
    for (size_t ii = 0; ii < _wtAfter.size(); ii++) {
      out << "      wtAfter[" << ii << "]: " << _wtAfter[ii] << endl;
    }
  }

}
