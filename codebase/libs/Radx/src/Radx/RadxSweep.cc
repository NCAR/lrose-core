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
#include <Radx/ByteOrder.hh>
#include <cstring>
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


/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxSweep::serialize(RadxMsg &msg,
                          RadxMsg::RadxMsg_t msgType /* = RadxMsg::RadxSweepMsg */) 
  
{
  
  // init

  msg.clearAll();
  msg.setMsgType(msgType);

  // add metadata numbers
  
  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxSweep::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxSweepMsg &&
      msg.getMsgType() != RadxMsg::RadxSweepAsInFileMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxSweep::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata numbers
  
  const RadxMsg::Part *metaNumsPart = msg.getPartByType(_metaNumbersPartId);
  if (metaNumsPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxSweep::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxSweep::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxSweep::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set

  _metaNumbers.fixedAngle = _fixedAngle;
  _metaNumbers.targetScanRate = _targetScanRate;
  _metaNumbers.measuredScanRate = _measuredScanRate;
  _metaNumbers.angleRes = _angleRes;
  _metaNumbers.intermedFreqHz = _intermedFreqHz;
  _metaNumbers.startRayIndex = _startRayIndex;
  _metaNumbers.endRayIndex = _endRayIndex;

  _metaNumbers.volNum = _volNum;
  _metaNumbers.sweepNum = _sweepNum;
  _metaNumbers.sweepMode = _sweepMode;
  _metaNumbers.polarizationMode = _polarizationMode;
  _metaNumbers.prtMode = _prtMode;
  _metaNumbers.followMode = _followMode;
  _metaNumbers.raysAreIndexed = _raysAreIndexed;
  _metaNumbers.isLongRange = _isLongRange;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxSweep::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                       size_t bufLen,
                                       bool swap)
  
{
  
  // check size
  
  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxSweep::_setMetaNumbersFromMsg" << endl;
    cerr << "  Incorrect message size: " << bufLen << endl;
    cerr << "  Should be: " << sizeof(msgMetaNumbers_t) << endl;
    return -1;
  }

  // copy into local struct
  
  _metaNumbers = *metaNumbers;
  
  // swap as needed

  if (swap) {
    _swapMetaNumbers(_metaNumbers); 
  }

  // set members
  
  _fixedAngle = _metaNumbers.fixedAngle;
  _targetScanRate = _metaNumbers.targetScanRate;
  _measuredScanRate = _metaNumbers.measuredScanRate;
  _angleRes = _metaNumbers.angleRes;
  _intermedFreqHz = _metaNumbers.intermedFreqHz;
  _startRayIndex = _metaNumbers.startRayIndex;
  _endRayIndex = _metaNumbers.endRayIndex;

  _volNum = _metaNumbers.volNum;
  _sweepNum = _metaNumbers.sweepNum;
  _sweepMode = (Radx::SweepMode_t) _metaNumbers.sweepMode;
  _polarizationMode = (Radx::PolarizationMode_t) _metaNumbers.polarizationMode;
  _prtMode = (Radx::PrtMode_t) _metaNumbers.prtMode;
  _followMode = (Radx::FollowMode_t) _metaNumbers.followMode;
  _raysAreIndexed = _metaNumbers.raysAreIndexed;
  _isLongRange = _metaNumbers.isLongRange;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxSweep::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.fixedAngle, 16 * sizeof(Radx::fl64));
  ByteOrder::swap32(&meta.volNum, 16 * sizeof(Radx::fl32));
}
