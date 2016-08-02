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
// RadxSweep.cc
//
// Sweep object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxSweep.hh>
using namespace std;

/////////////////////////////////////////////////////////
// RadxSweep constructor

RadxSweep::RadxSweep()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxSweep::RadxSweep(const RadxSweep &rhs)
     
{
  _init();
  _copy(rhs);
}

/////////////////////////////////////////////////////////
// RadxSweep destructor

RadxSweep::~RadxSweep()
  
{
}

/////////////////////////////
// Assignment
//

RadxSweep &RadxSweep::operator=(const RadxSweep &rhs)
  

{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxSweep &RadxSweep::_copy(const RadxSweep &rhs)
  
{

  if (&rhs == this) {
    return *this;
  }
  
  clear();

  _volNum = rhs._volNum;
  _sweepNum = rhs._sweepNum;

  _startRayIndex = rhs._startRayIndex;
  _endRayIndex = rhs._endRayIndex;

  _sweepMode = rhs._sweepMode;
  _polarizationMode = rhs._polarizationMode;
  _prtMode = rhs._prtMode;
  _followMode = rhs._followMode;

  _fixedAngle = rhs._fixedAngle;
  _targetScanRate = rhs._targetScanRate;
  _measuredScanRate = rhs._measuredScanRate;

  _raysAreIndexed = rhs._raysAreIndexed;
  _angleRes = rhs._angleRes;

  _intermedFreqHz = rhs._intermedFreqHz;

  _isLongRange = rhs._isLongRange;

  return *this;
  
}

//////////////////////////////////////////////////
// initialize

void RadxSweep::_init()
  
{

  _volNum = Radx::missingMetaInt;
  _sweepNum = Radx::missingMetaInt;

  _startRayIndex = Radx::missingMetaInt;
  _endRayIndex = Radx::missingMetaInt;

  _sweepMode = Radx::missingSweepMode;
  _polarizationMode = Radx::missingPolarizationMode;
  _prtMode = Radx::missingPrtMode;
  _followMode = Radx::missingFollowMode;

  _fixedAngle = Radx::missingMetaDouble;
  _targetScanRate = Radx::missingMetaDouble;
  _measuredScanRate = Radx::missingMetaDouble;

  _raysAreIndexed = false;
  _angleRes = Radx::missingMetaDouble;

  _intermedFreqHz = Radx::missingMetaDouble;

  _isLongRange = false;

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxSweep::clear()
  
{

}

/////////////////////////////////////////////////////////
// print

void RadxSweep::print(ostream &out) const
  
{
  
  out << "=============== RadxSweep ===============" << endl;
  out << "  volNum: " << _volNum << endl;
  out << "  sweepNum: " << _sweepNum << endl;
  out << "  nRays: " << _endRayIndex - _startRayIndex + 1 << endl;
  out << "  startRayIndex: " << _startRayIndex << endl;
  out << "  endRayIndex: " << _endRayIndex << endl;
  out << "  sweepMode: "
      << Radx::sweepModeToStr(_sweepMode) << endl;
  out << "  polarizationMode: "
      << Radx::polarizationModeToStr(_polarizationMode) << endl;
  out << "  prtMode: "
      << Radx::prtModeToStr(_prtMode) << endl;
  out << "  followMode: "
      << Radx::followModeToStr(_followMode) << endl;
  out << "  fixedAngle: " << _fixedAngle << endl;
  out << "  targetScanRate: " << _targetScanRate << endl;
  out << "  measuredScanRate: " << _measuredScanRate << endl;
  out << "  raysAreIndexed: " << (_raysAreIndexed? "Y":"N") << endl;
  out << "  angleRes: " << _angleRes << endl;
  if (_intermedFreqHz != Radx::missingMetaDouble) {
    out << "  intermedFreqHz: " << _intermedFreqHz << endl;
  }
  if (_isLongRange) {
    out << "  isLongRange: Y" << endl;
  }
  out << "===========================================" << endl;

}


