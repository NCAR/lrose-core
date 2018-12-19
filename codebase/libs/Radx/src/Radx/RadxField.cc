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
// RadxField.cc
//
// Field object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxField.hh>
#include <Radx/RadxArray.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
using namespace std;

/////////////////////////////////////////////////////////
// RadxField constructor

RadxField::RadxField(const string &name /*= "not-set" */,
                     const string &units /* = "" */) :
        _name(name),
        _units(units)
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxField::RadxField(const RadxField &rhs)
     
{
  _init();
  _copy(rhs);
}

/////////////////////////////////////////////////////////
// RadxField destructor

RadxField::~RadxField()
  
{
  clearData();
}

/////////////////////////////
// Assignment
//

RadxField &RadxField::operator=(const RadxField &rhs)
  

{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// initialize

void RadxField::_init()
  
{

  RadxRangeGeom::_init();
  RadxPacking::_init();

  _dataType = Radx::FL32;
  _byteWidth = sizeof(Radx::fl32);

  _scale = 1.0;
  _offset = 0.0;

  _samplingRatio = 1.0;

  _fieldFolds = false;
  _foldLimitLower = 0.0;
  _foldLimitUpper = 0.0;
  _foldRange = 0.0;

  _isDiscrete = false;
  
  _minVal = Radx::missingMetaDouble;
  _maxVal = Radx::missingMetaDouble;

  _setMissingToDefaults();
  _buf.clear();
  _data = NULL;
  _dataIsLocal = true;

  _thresholdValue = Radx::missingMetaDouble;

}

//////////////////////////////////////////////////
// copy metadata, but leave the data array empty

RadxField &RadxField::copyMetaData(const RadxField &rhs)
  
{

  if (&rhs == this) {
    return *this;
  }
  
  copyRangeGeom(rhs);

  _name = rhs._name;
  _longName = rhs._longName;
  _standardName = rhs._standardName;
  _units = rhs._units;
  _legendXml = rhs._legendXml;
  _thresholdingXml = rhs._thresholdingXml;
  _comment = rhs._comment;

  _dataType = rhs._dataType;
  _byteWidth = rhs._byteWidth;

  _scale = rhs._scale;
  _offset = rhs._offset;

  _samplingRatio = rhs._samplingRatio;

  _fieldFolds = rhs._fieldFolds;
  _foldLimitLower = rhs._foldLimitLower;
  _foldLimitUpper = rhs._foldLimitUpper;
  _foldRange = rhs._foldRange;

  _isDiscrete = rhs._isDiscrete;

  _missingFl64 = rhs._missingFl64;
  _missingFl32 = rhs._missingFl32;
  _missingSi32 = rhs._missingSi32;
  _missingSi16 = rhs._missingSi16;
  _missingSi08 = rhs._missingSi08;

  _thresholdFieldName = rhs._thresholdFieldName;
  _thresholdValue = rhs._thresholdValue;

  // for copy, always make local copy of data

  clearData();
  
  return *this;
  
}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxField &RadxField::_copy(const RadxField &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  copyMetaData(rhs);

  // for copy, always make local copy of data

  _buf.reset();
  _data = _buf.add(rhs._data, rhs.getNBytes());
  _dataIsLocal = true;
  copyPacking(rhs);

  _minVal = rhs._minVal;
  _maxVal = rhs._maxVal;
  
  return *this;
  
}

/////////////////////////////////////////////////////////
// clear the data array in the object

void RadxField::clearData()
  
{
  
  _buf.clear();
  _data = NULL;
  _dataIsLocal = true;
  _minVal = Radx::missingMetaDouble;
  _maxVal = Radx::missingMetaDouble;
  clearPacking();

}

/////////////////////////////////////////////////////////
/// Set missing value for fl64 data.
///
/// Only changes the missing value, and any points which have
/// missing values. The remainder of the data is unchanged.

void RadxField::setMissingFl64(Radx::fl64 missingValue)
  
{
  
  if (_dataType != Radx::FL64) {
    cerr << "WARNING - RadxField::setMissingFl64" << endl;
    cerr << "  Incorrect data type: " << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: " << Radx::dataTypeToStr(Radx::FL64) << endl;
    _missingFl64 = missingValue;
    return;
  }

  if (_missingFl64 == missingValue) {
    return;
  }

  Radx::fl64 *xdata = getDataFl64();
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (xdata[ii] == _missingFl64) {
      xdata[ii] = missingValue;
    }
  }

  _missingFl64 = missingValue;
  return;

}

/////////////////////////////////////////////////////////
/// Set missing value for fl32 data.
///
/// Only changes the missing value, and any points which have
/// missing values. The remainder of the data is unchanged.

void RadxField::setMissingFl32(Radx::fl32 missingValue)
  
{
  
  if (_dataType != Radx::FL32) {
    cerr << "WARNING - RadxField::setMissingFl32" << endl;
    cerr << "  Incorrect data type: " << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: " << Radx::dataTypeToStr(Radx::FL32) << endl;
    _missingFl32 = missingValue;
    return;
  }

  if (_missingFl32 == missingValue) {
    return;
  }

  Radx::fl32 *xdata = getDataFl32();
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (xdata[ii] == _missingFl32) {
      xdata[ii] = missingValue;
    }
  }

  _missingFl32 = missingValue;
  return;

}

/////////////////////////////////////////////////////////
/// Set missing value for si32 data.
///
/// Only changes the missing value, and any points which have
/// missing values. The remainder of the data is unchanged.

void RadxField::setMissingSi32(Radx::si32 missingValue)
  
{
  
  if (_dataType != Radx::SI32) {
    cerr << "WARNING - RadxField::setMissingSi32" << endl;
    cerr << "  Incorrect data type: " << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: " << Radx::dataTypeToStr(Radx::SI32) << endl;
    _missingSi32 = missingValue;
    return;
  }

  if (_missingSi32 == missingValue) {
    return;
  }

  Radx::si32 *xdata = getDataSi32();
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (xdata[ii] == _missingSi32) {
      xdata[ii] = missingValue;
    }
  }

  _missingSi32 = missingValue;
  return;

}

/////////////////////////////////////////////////////////
/// Set missing value for si16 data.
///
/// Only changes the missing value, and any points which have
/// missing values. The remainder of the data is unchanged.

void RadxField::setMissingSi16(Radx::si16 missingValue)
  
{
  
  if (_dataType != Radx::SI16) {
    cerr << "WARNING - RadxField::setMissingSi16" << endl;
    cerr << "  Incorrect data type: " << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: " << Radx::dataTypeToStr(Radx::SI16) << endl;
    _missingSi16 = missingValue;
    return;
  }

  if (_missingSi16 == missingValue) {
    return;
  }

  Radx::si16 *xdata = getDataSi16();
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (xdata[ii] == _missingSi16) {
      xdata[ii] = missingValue;
    }
  }

  _missingSi16 = missingValue;
  return;

}

/////////////////////////////////////////////////////////
/// Set missing value for si08 data.
///
/// Only changes the missing value, and any points which have
/// missing values. The remainder of the data is unchanged.

void RadxField::setMissingSi08(Radx::si08 missingValue)
  
{
  
  if (_dataType != Radx::SI08) {
    cerr << "WARNING - RadxField::setMissingSi08" << endl;
    cerr << "  Incorrect data type: " << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: " << Radx::dataTypeToStr(Radx::SI08) << endl;
    _missingSi08 = missingValue;
    return;
  }

  if (_missingSi08 == missingValue) {
    return;
  }

  Radx::si08 *xdata = getDataSi08();
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (xdata[ii] == _missingSi08) {
      xdata[ii] = missingValue;
    }
  }

  _missingSi08 = missingValue;
  return;

}

/////////////////////////////////////////////////////////
// Set data type to 64-bit floating point.
// Clears existing data.

void RadxField::setTypeFl64(Radx::fl64 missingValue)
  
{
  clearData();
  _dataType = Radx::FL64;
  _byteWidth = sizeof(Radx::fl64);
  _scale = 1.0;
  _offset = 0.0;
  _setMissingToDefaults();
  _missingFl64 = missingValue;
}

/////////////////////////////////////////////////////////
// Set data type to 32-bit floating point.
// Clears existing data.

void RadxField::setTypeFl32(Radx::fl32 missingValue)
  
{
  clearData();
  _dataType = Radx::FL32;
  _byteWidth = sizeof(Radx::fl32);
  _scale = 1.0;
  _offset = 0.0;
  _setMissingToDefaults();
  _missingFl32 = missingValue;
}

/////////////////////////////////////////////////////////
// set the data type to 32-bit integer
// Clears existing data.

void RadxField::setTypeSi32(Radx::si32 missingValue,
                            double scale,
                            double offset)
{
  clearData();
  _dataType = Radx::SI32;
  _byteWidth = sizeof(Radx::si32);
  _scale = scale;
  if (fabs(offset) == 0.0) {
    _offset = 0.0;
  } else {
    _offset = offset;
  }
  _setMissingToDefaults();
  _missingSi32 = missingValue;
}

/////////////////////////////////////////////////////////
// set the data type to 16-bit integer
// Clears existing data.

void RadxField::setTypeSi16(Radx::si16 missingValue,
                            double scale,
                            double offset)
{
  clearData();
  _dataType = Radx::SI16;
  _byteWidth = sizeof(Radx::si16);
  _scale = scale;
  if (fabs(offset) == 0.0) {
    _offset = 0.0;
  } else {
    _offset = offset;
  }
  _setMissingToDefaults();
  _missingSi16 = missingValue;
}

/////////////////////////////////////////////////////////
// set the data type to 08-bit integer
// Clears existing data.

void RadxField::setTypeSi08(Radx::si08 missingValue,
                            double scale,
                            double offset)
{
  clearData();
  _dataType = Radx::SI08;
  _byteWidth = sizeof(Radx::si08);
  _scale = scale;
  if (fabs(offset) == 0.0) {
    _offset = 0.0;
  } else {
    _offset = offset;
  }
  _setMissingToDefaults();
  _missingSi08 = missingValue;
}

/////////////////////////////////////////////////////////
// Add a ray of 64-bit floating point data to the field

void RadxField::addDataFl64(size_t nGates,
                            const Radx::fl64 *data)
  
{

  // check the correct type has been set,
  // and the data is managed locally

  _printTypeMismatch("addDataFl64", Radx::FL64);
  assert(_dataType == Radx::FL64);
  assert(_dataIsLocal);
  
  // add data to the buffer
  
  _data = _buf.add(data, nGates * sizeof(Radx::fl64));

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
// Add a ray of 32-bit floating point data to the field

void RadxField::addDataFl32(size_t nGates,
                            const Radx::fl32 *data)
  
{

  // check the correct type has been set,
  // and the data is managed locally

  _printTypeMismatch("addDataFl32", Radx::FL32);
  assert(_dataType == Radx::FL32);
  assert(_dataIsLocal);
  
  // add data to the buffer
  
  _data = _buf.add(data, nGates * sizeof(Radx::fl32));

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
// Add a ray of 32-bit integer data to the field

void RadxField::addDataSi32(size_t nGates,
                            const Radx::si32 *data)
  
{

  // check the correct type has been set,
  // and the data is managed locally

  _printTypeMismatch("addDataSi32", Radx::SI32);
  assert(_dataType == Radx::SI32);
  assert(_dataIsLocal);
  
  // add data to the buffer
  
  _data = _buf.add(data, nGates * sizeof(Radx::si32));

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
// Add a ray of 16-bit integer data to the field

void RadxField::addDataSi16(size_t nGates,
                            const Radx::si16 *data)
  
{

  // check the correct type has been set,
  // and the data is managed locally

  _printTypeMismatch("addDataSi16", Radx::SI16);
  assert(_dataType == Radx::SI16);
  assert(_dataIsLocal);
  
  // add data to the buffer
  
  _data = _buf.add(data, nGates * sizeof(Radx::si16));

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
// Add a ray of 08-bit integer data to the field

void RadxField::addDataSi08(size_t nGates,
                            const Radx::si08 *data)
  
{

  // check the correct type has been set,
  // and the data is managed locally

  _printTypeMismatch("addDataSi08", Radx::SI08);
  assert(_dataType == Radx::SI08);
  assert(_dataIsLocal);
  
  // add data to the buffer
  
  _data = _buf.add(data, nGates * sizeof(Radx::si08));

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
// Add a ray of missing data

void RadxField::addDataMissing(size_t nGates)
  
{
  
  switch (_dataType) {
    case Radx::FL64: {
      Radx::fl64 *data = new Radx::fl64[nGates];
      for (size_t ii = 0; ii < nGates; ii++) {
        data[ii] = _missingFl64;
      }
      _data = _buf.add(data, nGates * sizeof(Radx::fl64));
      delete[] data;
      break;
    }
    case Radx::FL32: {
      Radx::fl32 *data = new Radx::fl32[nGates];
      for (size_t ii = 0; ii < nGates; ii++) {
        data[ii] = _missingFl32;
      }
      _data = _buf.add(data, nGates * sizeof(Radx::fl32));
      delete[] data;
      break;
    }
    case Radx::SI32: {
      Radx::si32 *data = new Radx::si32[nGates];
      for (size_t ii = 0; ii < nGates; ii++) {
        data[ii] = _missingSi32;
      }
      _data = _buf.add(data, nGates * sizeof(Radx::si32));
      delete[] data;
      break;
    }
    case Radx::SI16: {
      Radx::si16 *data = new Radx::si16[nGates];
      for (size_t ii = 0; ii < nGates; ii++) {
        data[ii] = _missingSi16;
      }
      _data = _buf.add(data, nGates * sizeof(Radx::si16));
      delete[] data;
      break;
    }
    case Radx::SI08: {
      Radx::si08 *data = new Radx::si08[nGates];
      for (size_t ii = 0; ii < nGates; ii++) {
        data[ii] = _missingSi08;
      }
      _data = _buf.add(data, nGates * sizeof(Radx::si08));
      delete[] data;
      break;
    }
    default: {
      return;
    }
  }

  // update the ray geometry

  addToPacking(nGates);

}

/////////////////////////////////////////////////////////
/// Set value at a specified gate to missing

void RadxField::setGateToMissing(size_t gateNum)
  
{

  if (gateNum >= _nPoints) {
    return;
  }

  switch (_dataType) {
    case Radx::SI08: {
      Radx::si08 *sdata = (Radx::si08 *) _data;
      sdata[gateNum] = _missingSi08;
      break;
    }
    case Radx::SI16: {
      Radx::si16 *sdata = (Radx::si16 *) _data;
      sdata[gateNum] = _missingSi16;
      break;
    }
    case Radx::SI32: {
      Radx::si32 *sdata = (Radx::si32 *) _data;
      sdata[gateNum] = _missingSi32;
      break;
    }
    case Radx::FL32: {
      Radx::fl32 *sdata = (Radx::fl32 *) _data;
      sdata[gateNum] = _missingFl32;
      break;
    }
    case Radx::FL64: {
      Radx::fl64 *sdata = (Radx::fl64 *) _data;
      sdata[gateNum] = _missingFl64;
      break;
    }
    default: {}
  } // switch

}

/////////////////////////////////////////////////////////
/// Set values at specified gates to missing

void RadxField::setGatesToMissing(size_t startGate, size_t endGate)
{
  for (size_t ii = startGate; ii <= endGate; ii++) {
    setGateToMissing(ii);
  }
}

/////////////////////////////////////////////////////////
/// Set values within specified range limits to missing

void RadxField::setRangeIntervalToMissing(double startRangeKm, double endRangeKm)
{
  int startGate =
    (int) floor((startRangeKm - _startRangeKm) / _gateSpacingKm + 0.5);
  int endGate =
    (int) floor((endRangeKm - _startRangeKm) / _gateSpacingKm + 0.5);
  for (int ii = startGate; ii <= endGate; ii++) {
    setGateToMissing(ii);
  }
}

//////////////////////////////////////////////////////////////////////////
/// Set the number of gates.
///
/// If more gates are needed, extend the field data out to a set number of
/// gates. The data for extra gates are set to missing values.
///
/// If fewer gates are needed, the data is truncated.

void RadxField::setNGates(size_t nGates)
  
{

  int nExtra = nGates - _nPoints;
  if (nExtra == 0) {
    // no change
    return;
  }

  if (nExtra < 0) {

    // shrink

    // make temp copy

    RadxBuf tmp(_buf);

    // copy data back in

    _buf.clear();
    _data = _buf.add(tmp.getPtr(), nGates * _byteWidth);

    // set number of gates

    clearPacking();
    addToPacking(nGates);

    return;

  }

  // grow - fill with missing

  switch (_dataType) {
    
    case Radx::FL64: {
      Radx::fl64 *extra = new Radx::fl64[nExtra];
      for (int ii = 0; ii < nExtra; ii++) {
        extra[ii] = _missingFl64;
      }
      _data = _buf.add(extra, nExtra * sizeof(Radx::fl64));
      delete[] extra;
      break;
    }
      
    case Radx::FL32: {
      Radx::fl32 *extra = new Radx::fl32[nExtra];
      for (int ii = 0; ii < nExtra; ii++) {
        extra[ii] = _missingFl32;
      }
      _data = _buf.add(extra, nExtra * sizeof(Radx::fl32));
      delete[] extra;
      break;
    }
      
    case Radx::SI32: {
      Radx::si32 *extra = new Radx::si32[nExtra];
      for (int ii = 0; ii < nExtra; ii++) {
        extra[ii] = _missingSi32;
      }
      _data = _buf.add(extra, nExtra * sizeof(Radx::si32));
      delete[] extra;
      break;
    }
      
    case Radx::SI16: {
      Radx::si16 *extra = new Radx::si16[nExtra];
      for (int ii = 0; ii < nExtra; ii++) {
        extra[ii] = _missingSi16;
      }
      _data = _buf.add(extra, nExtra * sizeof(Radx::si16));
      delete[] extra;
      break;
    }
      
    case Radx::SI08: 
    default: {
      Radx::si08 *extra = new Radx::si08[nExtra];
      for (int ii = 0; ii < nExtra; ii++) {
        extra[ii] = _missingSi08;
      }
      _data = _buf.add(extra, nExtra * sizeof(Radx::si08));
      delete[] extra;
      break;
    }
      
  } // switch (_dataType)

  // set packing to reflect npoints

  clearPacking();
  addToPacking(nGates);
  
}

/////////////////////////////////////////////////////////
// set fl64 data for a single ray
//
// If isLocal is false, the data pointer will be stored
// and the data memory is owned by the calling object.
//
// If isLocal is true, the data will be copied to the
// local buffer.

void RadxField::setDataFl64(size_t nGates,
                            const Radx::fl64 *data,
                            bool isLocal)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataFl64", Radx::FL64);
  assert(_dataType == Radx::FL64);

  // clear

  clearData();

  // set geometry
  
  addToPacking(nGates);
  
  // set data

  if (isLocal) {
    _data = _buf.add(data, getNBytes());
    _dataIsLocal = true;
  } else {
    _data = data;
    _dataIsLocal = false;
  }

}

//////////////////////////////////////////////////////////////
// set fl64 data for a vector of rays
//
// The data is copied to the local buffer, and managed locally.

void RadxField::setDataFl64(const vector<size_t> &rayNGates,
                            const Radx::fl64 *data)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataFl64", Radx::FL64);
  assert(_dataType == Radx::FL64);
  
  // clear
  
  _buf.clear();
  
  // set geometry
  
  setPacking(rayNGates);

  // set data

  _data = _buf.add(data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////
// set fl32 data for a single ray
//
// If isLocal is false, the data pointer will be stored
// and the data memory is owned by the calling object.
//
// If isLocal is true, the data will be copied to the
// local buffer.

void RadxField::setDataFl32(size_t nGates,
                            const Radx::fl32 *data,
                            bool isLocal)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataFl32", Radx::FL32);
  assert(_dataType == Radx::FL32);

  // clear

  clearData();

  // set geometry
  
  addToPacking(nGates);
  
  // set data

  if (isLocal) {
    _data = _buf.add(data, getNBytes());
    _dataIsLocal = true;
  } else {
    _data = data;
    _dataIsLocal = false;
  }

}

//////////////////////////////////////////////////////////////
// set fl32 data for a vector of rays
//
// The data is copied to the local buffer, and managed locally.

void RadxField::setDataFl32(const vector<size_t> &rayNGates,
                            const Radx::fl32 *data)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataFl32", Radx::FL32);
  assert(_dataType == Radx::FL32);
  
  // clear
  
  _buf.clear();
  
  // set geometry
  
  setPacking(rayNGates);

  // set data

  _data = _buf.add(data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////
// set si32 data for a single ray
//
// If isLocal is false, the data pointer will be stored
// and the data memory is owned by the calling object.
//
// If isLocal is true, the data will be copied to the
// local buffer.

void RadxField::setDataSi32(size_t nGates,
                            const Radx::si32 *data,
                            bool isLocal)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi32", Radx::SI32);
  assert(_dataType == Radx::SI32);

  // clear

  clearData();

  // set geometry
  
  addToPacking(nGates);
  
  // set data

  if (isLocal) {
    _data = _buf.add(data, getNBytes());
    _dataIsLocal = true;
  } else {
    _data = data;
    _dataIsLocal = false;
  }

}

//////////////////////////////////////////////////////////////
// set si32 data for a vector of rays
//
// The data is copied to the local buffer, and managed locally.

void RadxField::setDataSi32(const vector<size_t> &rayNGates,
                            const Radx::si32 *data)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi32", Radx::SI32);
  assert(_dataType == Radx::SI32);
  
  // clear
  
  _buf.clear();
  
  // set geometry
  
  setPacking(rayNGates);

  // set data

  _data = _buf.add(data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////
// set si16 data for a single ray
//
// If isLocal is false, the data pointer will be stored
// and the data memory is owned by the calling object.
//
// If isLocal is true, the data will be copied to the
// local buffer.

void RadxField::setDataSi16(size_t nGates,
                            const Radx::si16 *data,
                            bool isLocal)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi16", Radx::SI16);
  assert(_dataType == Radx::SI16);

  // clear

  clearData();

  // set geometry
  
  addToPacking(nGates);
  
  // set data

  if (isLocal) {
    _data = _buf.add(data, getNBytes());
    _dataIsLocal = true;
  } else {
    _data = data;
    _dataIsLocal = false;
  }
  
  // TODO - follow up on this in the future - Mike Dixon
  //        2016/01/10
  // check for missing val that is 1 off from theoretical value
  // and set to missing
  // in some nexrad data (ZDR for example) we get field vals that are
  // -32767 instead of -32768

  if (_missingSi16 == -32768) {
    Radx::si16 *dbuf = (Radx::si16 *) _data;
    for (size_t ii = 0; ii < getNPoints(); ii++) {
      if (dbuf[ii] == -32767) {
        dbuf[ii] = _missingSi16;
      }
    }
  } else if (_missingSi16 == -32767) {
    Radx::si16 *dbuf = (Radx::si16 *) _data;
    for (size_t ii = 0; ii < getNPoints(); ii++) {
      if (dbuf[ii] == -32768 || dbuf[ii] == -21766) {
        dbuf[ii] = _missingSi16;
      }
    }
  }

}

//////////////////////////////////////////////////////////////
// set si16 data for a vector of rays
//
// The data is copied to the local buffer, and managed locally.

void RadxField::setDataSi16(const vector<size_t> &rayNGates,
                            const Radx::si16 *data)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi16", Radx::SI16);
  assert(_dataType == Radx::SI16);
  
  // clear
  
  _buf.clear();
  
  // set geometry
  
  setPacking(rayNGates);

  // set data

  _data = _buf.add(data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////
// set si08 data for a single ray
//
// If isLocal is false, the data pointer will be stored
// and the data memory is owned by the calling object.
//
// If isLocal is true, the data will be copied to the
// local buffer.

void RadxField::setDataSi08(size_t nGates,
                            const Radx::si08 *data,
                            bool isLocal)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi08", Radx::SI08);
  assert(_dataType == Radx::SI08);

  // clear

  clearData();

  // set geometry
  
  addToPacking(nGates);
  
  // set data

  if (isLocal) {
    _data = _buf.add(data, getNBytes());
    _dataIsLocal = true;
  } else {
    _data = data;
    _dataIsLocal = false;
  }

}

//////////////////////////////////////////////////////////////
// set si08 data for a vector of rays
//
// The data is copied to the local buffer, and managed locally.

void RadxField::setDataSi08(const vector<size_t> &rayNGates,
                            const Radx::si08 *data)
  
{

  // check the correct type has been set
  
  _printTypeMismatch("setDataSi08", Radx::SI08);
  assert(_dataType == Radx::SI08);
  
  // clear
  
  _buf.clear();
  
  // set geometry
  
  setPacking(rayNGates);

  // set data

  _data = _buf.add(data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////
// Set the object so that the data is locally managed.
//
// If the data points to a remote object, allocate 
// a local data array and copy the remote data into it.
  
void RadxField::setDataLocal()
  
{

  if (_dataIsLocal) {
    // already local
    return;
  }

  _buf.clear();
  _data = _buf.add(_data, getNBytes());
  _dataIsLocal = true;

}

/////////////////////////////////////////////////////////////////
// Set data on the object to point to data managed by a different
// field object. Therefore the data is not managed by this object.

void RadxField::setDataRemote(const RadxField &other,
                              const void *data,
                              size_t nGates)
  
{

  // clear the local data array in the object
  
  _buf.clear();
  _dataIsLocal = false;
  clearPacking();
  addToPacking(nGates);

  // point to data managed by other field
  
  _data = data;
  _scale = other._scale;
  _offset = other._offset;
  _samplingRatio = other._samplingRatio;

  _fieldFolds = other._fieldFolds;
  _foldLimitLower = other._foldLimitLower;
  _foldLimitUpper = other._foldLimitUpper;
  _foldRange = other._foldRange;
  _isDiscrete = other._isDiscrete;

  _dataType = other._dataType;
  _byteWidth = other._byteWidth;
  _minVal = other._minVal;
  _maxVal = other._maxVal;
  _missingFl64 = other._missingFl64;
  _missingFl32 = other._missingFl32;
  _missingSi32 = other._missingSi32;
  _missingSi16 = other._missingSi16;
  _missingSi08 = other._missingSi08;

}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// If the data type is an integer type, dynamic scaling
/// is used - i.e. the min and max value is computed and
/// the scale and offset are set to values which maximize the
/// dynamic range.
///
/// If dataType is Radx::ASIS, no type conversion is performed.
///
/// If a string argument has zero length, the value on the
/// field will be left unchanged.

void RadxField::convert(Radx::DataType_t dtype,
                        const string &name,
                        const string &units,
                        const string &standardName,
                        const string &longName)
  
{

  if ((_dataType != Radx::ASIS) &&
      (_dataType != dtype)) {
    convertToType(dtype);
  }
  if (name.length() > 0) {
    _name = name;
  }
  _units = units;
  _standardName = standardName;
  _longName = longName;
}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// If the data type is an integer type, the specified
/// scale and offset are used.
///
/// If dataType is Radx::ASIS, no type conversion is performed.
///
/// If a string argument has zero length, the value on the
/// field will be left unchanged.

void RadxField::convert(Radx::DataType_t dtype,
                        double scale,
                        double offset,
                        const string &name,
                        const string &units,
                        const string &standardName,
                        const string &longName)
  
{
  
  bool typeHasChanged = false;
  if ((_dataType != dtype) ||
      (fabs(scale - _scale) > 1.0e-5) ||
      (fabs(offset - _offset) > 1.0e-5)) {
    typeHasChanged = true;
  }
  if ((_dataType != Radx::ASIS) && typeHasChanged) {
    convertToType(dtype, scale, offset);
  }
  if (name.length() > 0) {
    _name = name;
  }
  if (units.length() > 0) {
    _units = units;
  }
  if (standardName.length() > 0) {
    _standardName = standardName;
  }
  if (longName.length() > 0) {
    _longName = longName;
  }
}

/////////////////////////////////////////////////////////
// convert to fl64

void RadxField::convertToFl64()
  
{

  if (_dataType == Radx::FL64) {
    return;
  }

  // make sure we are managing the data

  setDataLocal();
  
  switch (_dataType) {
    case Radx::FL32: {
      Radx::fl32 *fdata = (Radx::fl32 *) _data;
      Radx::fl64 *ddata = new Radx::fl64[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (fdata[ii] == _missingFl32) {
          ddata[ii] = Radx::missingFl64;
        } else {
          ddata[ii] = fdata[ii];
        }
      }
      _buf.clear();
      _data = _buf.add(ddata, _nPoints * sizeof(Radx::fl64));
      delete[] ddata;
      break;
    }
    case Radx::SI32: {
      Radx::si32 *idata = (Radx::si32 *) _data;
      Radx::fl64 *ddata = new Radx::fl64[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (idata[ii] == _missingSi32) {
          ddata[ii] = Radx::missingFl64;
        } else {
          ddata[ii] = idata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(ddata, _nPoints * sizeof(Radx::fl64));
      delete[] ddata;
      break;
    }
    case Radx::SI16: {
      Radx::si16 *sdata = (Radx::si16 *) _data;
      Radx::fl64 *ddata = new Radx::fl64[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (sdata[ii] == _missingSi16) {
          ddata[ii] = Radx::missingFl64;
        } else {
          ddata[ii] = sdata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(ddata, _nPoints * sizeof(Radx::fl64));
      delete[] ddata;
      break;
    }
    case Radx::SI08: {
      Radx::si08 *bdata = (Radx::si08 *) _data;
      Radx::fl64 *ddata = new Radx::fl64[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (bdata[ii] == _missingSi08) {
          ddata[ii] = Radx::missingFl64;
        } else {
          ddata[ii] = bdata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(ddata, _nPoints * sizeof(Radx::fl64));
      delete[] ddata;
      break;
    }
    default: {
      return;
    }
  }
  
  _dataType = Radx::FL64;
  _byteWidth = sizeof(Radx::fl64);
  _scale = 1.0;
  _offset = 0.0;
  _setMissingToDefaults();

}

/////////////////////////////////////////////////////////
// convert to fl32

void RadxField::convertToFl32()
  
{
  
  if (_dataType == Radx::FL32) {
    return;
  }

  // make sure we are managing the data locally

  setDataLocal();
  
  switch (_dataType) {
    case Radx::FL64: {
      Radx::fl64 *ddata = (Radx::fl64 *) _data;
      Radx::fl32 *fdata = new Radx::fl32[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (ddata[ii] == _missingFl64) {
          fdata[ii] = Radx::missingFl32;
        } else {
          fdata[ii] = ddata[ii];
        }
      }
      _buf.clear();
      _data = _buf.add(fdata, _nPoints * sizeof(Radx::fl32));
      delete[] fdata;
      break;
    }
    case Radx::SI32: {
      Radx::si32 *idata = (Radx::si32 *) _data;
      Radx::fl32 *fdata = new Radx::fl32[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (idata[ii] == _missingSi32) {
          fdata[ii] = Radx::missingFl32;
        } else {
          fdata[ii] = idata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(fdata, _nPoints * sizeof(Radx::fl32));
      delete[] fdata;
      break;
    }
    case Radx::SI16: {
      Radx::si16 *sdata = (Radx::si16 *) _data;
      Radx::fl32 *fdata = new Radx::fl32[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (sdata[ii] == _missingSi16) {
          fdata[ii] = Radx::missingFl32;
        } else {
          fdata[ii] = sdata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(fdata, _nPoints * sizeof(Radx::fl32));
      delete[] fdata;
      break;
    }
    case Radx::SI08: {
      Radx::si08 *bdata = (Radx::si08 *) _data;
      Radx::fl32 *fdata = new Radx::fl32[_nPoints];
      for (size_t ii = 0; ii < _nPoints; ii++) {
        if (bdata[ii] == _missingSi08) {
          fdata[ii] = Radx::missingFl32;
        } else {
          fdata[ii] = bdata[ii] * _scale + _offset;
        }
      }
      _buf.clear();
      _data = _buf.add(fdata, _nPoints * sizeof(Radx::fl32));
      delete[] fdata;
      break;
    }
    default: {
      return;
    }
  }
  
  _dataType = Radx::FL32;
  _byteWidth = sizeof(Radx::fl32);
  _scale = 1.0;
  _offset = 0.0;
  _setMissingToDefaults();

}

/////////////////////////////////////////////////////////
// convert to si32

void RadxField::convertToSi32(double scale,
                              double offset)
  
{

  if (_dataType == Radx::SI32 &&
      fabs(scale - _scale) < 0.00001 &&
      fabs(offset - _offset) < 0.00001) {
    return;
  }

  convertToFl32();
  
  Radx::fl32 *fdata = (Radx::fl32 *) _data;
  Radx::si32 *idata = new Radx::si32[_nPoints];
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (fdata[ii] == _missingFl32) {
      idata[ii] = Radx::missingSi32;
    } else {
      long long int ival =
        (long long int) floor((fdata[ii] - offset) / scale + 0.5);
      if (ival < -2147483647 || ival > 2147483647) {
        idata[ii] = Radx::missingSi32;
      } else {
        idata[ii] = (Radx::si32) ival;
      }
    }
  }
  _buf.clear();
  _data = _buf.add(idata, _nPoints * sizeof(Radx::si32));
  delete[] idata;
  
  _dataType = Radx::SI32;
  _byteWidth = sizeof(Radx::si32);
  _scale = scale;
  _offset = offset;

}

/////////////////////////////////////////////////////////
// convert to si16
// specify the scale and offset

void RadxField::convertToSi16(double scale,
                              double offset)
  
{

  if (_dataType == Radx::SI16 &&
      fabs(scale - _scale) < 0.00001 &&
      fabs(offset - _offset) < 0.00001) {
    return;
  }

  convertToFl32();

  Radx::fl32 *fdata = (Radx::fl32 *) _data;
  Radx::si16 *sdata = new Radx::si16[_nPoints];
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (fdata[ii] == _missingFl32) {
      sdata[ii] = Radx::missingSi16;
    } else {
      int idata = (int) floor((fdata[ii] - offset) / scale + 0.5);
      if (idata < -32767 || idata > 32767) {
        sdata[ii] = Radx::missingSi16;
      } else {
        sdata[ii] = (Radx::si16) idata;
      }
    }
  }
  _buf.clear();
  _data = _buf.add(sdata, _nPoints * sizeof(Radx::si16));
  delete[] sdata;

  _dataType = Radx::SI16;
  _byteWidth = sizeof(Radx::si16);
  _scale = scale;
  _offset = offset;

}

/////////////////////////////////////////////////////////
// convert to byte

void RadxField::convertToSi08(double scale,
                              double offset)
  
{
  
  if (_dataType == Radx::SI08 &&
      fabs(scale - _scale) < 0.00001 &&
      fabs(offset - _offset) < 0.00001) {
    return;
  }
  
  convertToFl32();
  
  Radx::fl32 *fdata = (Radx::fl32 *) _data;
  Radx::si08 *bdata = new Radx::si08[_nPoints];
  for (size_t ii = 0; ii < _nPoints; ii++) {
    if (fdata[ii] == _missingFl32) {
      bdata[ii] = Radx::missingSi08;
    } else {
      int ival = (int) floor((fdata[ii] - offset) / scale + 0.5);
      if (ival < -127 || ival > 127) {
        bdata[ii] = Radx::missingSi08;
      } else {
        bdata[ii] = (Radx::si08) ival;
      }
    }
  }
  _buf.clear();
  _data = _buf.add(bdata, _nPoints * sizeof(Radx::si08));
  delete[] bdata;
  
  _dataType = Radx::SI08;
  _byteWidth = sizeof(Radx::si08);
  _scale = scale;
  _offset = offset;

}

/////////////////////////////////////////////////////////
// convert to si32
// dynamically compute the scale and offset

void RadxField::convertToSi32()
  
{
  
  if (_dataType == Radx::SI32) {
    return;
  }
  
  // convert to fl32s

  convertToFl32();

  // compute min and max

  double scale = 1.0;
  double offset = 0.0;

  if (computeMinAndMax() == 0) {

    // compute scale and offset
    //
    // We map the valid range (_minVal, _maxVal) to ( -2^(n-1)+ 1, 2^(n-1) -1)
    // and leave -2^(n-1) for the fill value.
    //
    // add_offset = (_maxVal + _minVal)/2
    // scale_factor = (_maxVal - _minVal)/(2^n - 2)
    // packedVal = (unpacked - offset)/scaleFactor
    // where n is the number of bits of the packed (integer) data type
    
    scale = (_maxVal - _minVal) / (pow(2.0, 32.0) - 2);
    offset = (_maxVal + _minVal) / 2.0;

  }

  // perform conversion

  convertToSi32(scale, offset);

}

/////////////////////////////////////////////////////////
// convert to si16
// dynamically compute the scale and offset

void RadxField::convertToSi16()
  
{
  
  if (_dataType == Radx::SI16) {
    return;
  }
  
  // convert to fl32s

  convertToFl32();

  // compute min and max

  double scale = 1.0;
  double offset = 0.0;

  if (computeMinAndMax() == 0) {

    // compute scale and offset
    //
    // We map the valid range (_minVal, _maxVal) to ( -2^(n-1)+ 1, 2^(n-1) -1)
    // and leave -2^(n-1) for the fill value.
    //
    // add_offset = (_maxVal + _minVal)/2
    // scale_factor = (_maxVal - _minVal)/(2^n - 2)
    // packedVal = (unpacked - offset)/scaleFactor
    // where n is the number of bits of the packed (integer) data type
    
    scale = (_maxVal - _minVal) / (pow(2.0, 16.0) - 2);
    offset = (_maxVal + _minVal) / 2.0;

  }

  // perform conversion

  convertToSi16(scale, offset);

}

/////////////////////////////////////////////////////////
// convert to byte
// dynamically compute the scale and offset

void RadxField::convertToSi08()
  
{
  
  if (_dataType == Radx::SI08) {
    return;
  }
  
  // convert to fl32s

  convertToFl32();

  // compute min and max

  double scale = 1.0;
  double offset = 0.0;

  if (computeMinAndMax() == 0) {

    // compute scale and offset
    //
    // We map the valid range (_minVal, _maxVal) to ( -2^(n-1)+ 1, 2^(n-1) -1)
    // and leave -2^(n-1) for the fill value.
    //
    // add_offset = (_maxVal + _minVal)/2
    // scale_factor = (_maxVal - _minVal)/(2^n - 2)
    // packedVal = (unpacked - offset)/scaleFactor
    // where n is the number of bits of the packed (integer) data type
    
    scale = (_maxVal - _minVal) / (pow(2.0, 8.0) - 2);
    offset = (_maxVal + _minVal) / 2.0;

  }

  // perform conversion

  convertToSi08(scale, offset);

}

/////////////////////////////////////////////////////////
/// convert to specified type
///
/// If the data type is an integer type, dynamic scaling
/// is used - i.e. the min and max value is computed and
/// the scale and offset are set to values which maximize the
/// dynamic range.
///
/// If targetType is Radx::ASIS, no conversion is performed.

void RadxField::convertToType(Radx::DataType_t targetType)
  
{

  if (targetType == Radx::ASIS) {
    return;
  }

  // if no data, just set the type and return
  
  if (_nPoints == 0) {
    _dataType = targetType;
    _byteWidth = Radx::getByteWidth(_dataType);
    return;
  }

  switch (targetType) {
    case Radx::FL64:
      convertToFl64();
      return;
    case Radx::SI32:
      convertToSi32();
      return;
    case Radx::SI16:
      convertToSi16();
      return;
    case Radx::SI08:
      convertToSi08();
      return;
    case Radx::FL32:
    default:
      convertToFl32();
      return;
  }
}

/////////////////////////////////////////////////////////
/// convert to specified type
///
/// For integer types, the specified scale and offset are
/// used.
///
/// If targetType is Radx::ASIS, no conversion is performed.

void RadxField::convertToType(Radx::DataType_t targetType,
                              double scale,
                              double offset)
  
{

  if (targetType == Radx::ASIS) {
    return;
  }

  // if no data, just set the type and return
  
  if (_nPoints == 0) {
    _dataType = targetType;
    _byteWidth = Radx::getByteWidth(_dataType);
    return;
  }

  switch (targetType) {
    case Radx::FL64:
      convertToFl64();
      return;
    case Radx::SI32:
      convertToSi32(scale, offset);
      return;
    case Radx::SI16:
      convertToSi16(scale, offset);
      return;
    case Radx::SI08:
      convertToSi08(scale, offset);
      return;
    case Radx::FL32:
    default:
      convertToFl32();
      return;
  }
}

////////////////////////////////////////////////////////////
// Get pointer to data for specified ray

const void *RadxField::getData(size_t rayNum, size_t &nGates) const

{

  if (rayNum >= _rayStartIndex.size()) {
    cerr << "ERROR - RadxField::getData(rayNum)" << endl;
    cerr << "  specified rayNum: " << rayNum << endl;
    cerr << "  exceeds max: " << _rayStartIndex.size()-1 << endl;
  }
  assert(rayNum < _rayStartIndex.size());
  size_t index = _rayStartIndex[rayNum];
  nGates = _rayNGates[rayNum];
  
  switch (_dataType) {
    case Radx::FL64:
      return ((Radx::fl64 *) _data) + index;
    case Radx::FL32:
      return ((Radx::fl32 *) _data) + index;
    case Radx::SI32:
      return ((Radx::si32 *) _data) + index;
    case Radx::SI16:
      return ((Radx::si16 *) _data) + index;
    case Radx::SI08:
    default:
      return ((Radx::si08 *) _data) + index;
  }

}

void *RadxField::getData(size_t rayNum, size_t &nGates)

{

  if (rayNum >= _rayStartIndex.size()) {
    cerr << "ERROR - RadxField::getData(rayNum)" << endl;
    cerr << "  specified rayNum: " << rayNum << endl;
    cerr << "  exceeds max: " << _rayStartIndex.size()-1 << endl;
  }
  assert(rayNum < _rayStartIndex.size());
  size_t index = _rayStartIndex[rayNum];
  nGates = _rayNGates[rayNum];
  
  switch (_dataType) {
    case Radx::FL64:
      return ((Radx::fl64 *) _data) + index;
    case Radx::FL32:
      return ((Radx::fl32 *) _data) + index;
    case Radx::SI32:
      return ((Radx::si32 *) _data) + index;
    case Radx::SI16:
      return ((Radx::si16 *) _data) + index;
    case Radx::SI08:
    default:
      return ((Radx::si08 *) _data) + index;
  }

}

////////////////////////////////////////////////////////////
// Get pointer to 64-bit floating point data.
// Note - this assumes data is stored in this type.
// An assert will check this assumption, and exit if false.

const Radx::fl64 *RadxField::getDataFl64() const
{
  // first check that the data type is correct
  _printTypeMismatch("getDataFl64", Radx::FL64);
  assert(_dataType == Radx::FL64);
  return (const Radx::fl64 *) _data;
}

Radx::fl64 *RadxField::getDataFl64()
{
  // first check that the data type is correct
  _printTypeMismatch("getDataFl64", Radx::FL64);
  assert(_dataType == Radx::FL64);
  return (Radx::fl64 *) _data;
}

////////////////////////////////////////////////////////////
// Get pointer to 32-bit floating point data.
// Note - this assumes data is stored in this type.
// An assert will check this assumption, and exit if false.

const Radx::fl32 *RadxField::getDataFl32() const 
{ 
  // first check that the data type is correct
  _printTypeMismatch("getDataFl32", Radx::FL32);
  assert(_dataType == Radx::FL32);
  return (const Radx::fl32 *) _data; 
}

Radx::fl32 *RadxField::getDataFl32()
{ 
  // first check that the data type is correct
  _printTypeMismatch("getDataFl32", Radx::FL32);
  assert(_dataType == Radx::FL32);
  return (Radx::fl32 *) _data; 
}

////////////////////////////////////////////////////////////
// Get pointer to 32-bit scaled integer data.
// Note - this assumes data is stored in this type.
// An assert will check this assumption, and exit if false.

const Radx::si32 *RadxField::getDataSi32() const 
{ 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi32", Radx::SI32);
  assert(_dataType == Radx::SI32);
  return (const Radx::si32 *) _data; 
}

Radx::si32 *RadxField::getDataSi32() { 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi32", Radx::SI32);
  assert(_dataType == Radx::SI32);
  return (Radx::si32 *) _data; 
}

////////////////////////////////////////////////////////////
// Get pointer to 16-bit scaled integer data.
// Note - this assumes data is stored in this type.
// An assert will check this assumption, and exit if false.

const Radx::si16 *RadxField::getDataSi16() const { 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi16", Radx::SI16);
  assert(_dataType == Radx::SI16);
  return (const Radx::si16 *) _data; 
}

Radx::si16 *RadxField::getDataSi16() { 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi16", Radx::SI16);
  assert(_dataType == Radx::SI16);
  return (Radx::si16 *) _data; 
}

////////////////////////////////////////////////////////////
// Get pointer to 8-bit scaled integer data.
// Note - this assumes data is stored in this type.
// An assert will check this assumption, and exit if false.

const Radx::si08 *RadxField::getDataSi08() const { 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi08", Radx::SI08);
  assert(_dataType == Radx::SI08);
  return (const Radx::si08 *) _data; 
}

Radx::si08 *RadxField::getDataSi08() { 
  // first check that the data type is correct
  _printTypeMismatch("getDataSi08", Radx::SI08);
  assert(_dataType == Radx::SI08);
  return (Radx::si08 *) _data; 
}

/////////////////////////////////////////////////////////
// compute min and max

int RadxField::computeMinAndMax() const
  
{

  _minVal = 1.0e99;
  _maxVal = -1.0e99;
  
  // use floats as they are
  
  if (_dataType == Radx::FL64) {

    const Radx::fl64 *data = (Radx::fl64*) _data;
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      Radx::fl64 val = *data;
      if (val != _missingFl64) {
        if (val < _minVal) {
          _minVal = val;
        }
        if (val > _maxVal) {
          _maxVal = val;
        }
      }
    }

  } else if (_dataType == Radx::FL32) {

    const Radx::fl32 *data = (Radx::fl32*) _data;
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      Radx::fl32 val = *data;
      if (val != _missingFl32) {
        if (val < _minVal) {
          _minVal = val;
        }
        if (val > _maxVal) {
          _maxVal = val;
        }
      }
    }

  } else {

    for (size_t ii = 0; ii < _nPoints; ii++) {
      double val = getDoubleValue(ii);
      if (val != _missingFl64) {
        if (val < _minVal) {
          _minVal = val;
        }
        if (val > _maxVal) {
          _maxVal = val;
        }
      }
    }

  }

  // all missing?

  if (!isfinite(_minVal)) {
    _minVal = 1.0e99;
  }
  if (!isfinite(_maxVal)) {
    _maxVal = -1.0e99;
  }

  if (_minVal == 1.0e99 || _maxVal == -1.0e99) {

    _minVal = Radx::missingMetaDouble;
    _maxVal = Radx::missingMetaDouble;
    return -1;

  } else {

    // make sure they are not equal
    
    if (_minVal == _maxVal) {
      if (_minVal == 0) {
        _maxVal = 1.0;
      } else {
        _maxVal = _minVal * 2.0;
      }
    }

    return 0;

  }

}

/////////////////////////////////////////////////////////////
/// Apply a linear transformation to the data values.
/// Transforms x to y as follows:
///   y = x * scale + offset
/// After operation, leaves type unchanged.

void RadxField::applyLinearTransform(double scale, double offset)

{

  // save existing type

  Radx::DataType_t origType = getDataType();

  // convert to floats

  convertToFl32();

  // apply transformation
  
  Radx::fl32 *data = (Radx::fl32*) _data;
  for (size_t ii = 0; ii < _nPoints; ii++, data++) {
    Radx::fl32 val = *data;
    if (val != _missingFl32) {
      Radx::fl32 newVal = val * scale + offset;
      *data = newVal;
    }
  } // ii

  // convert back to original type

  convertToType(origType);

}

/////////////////////////////////////////////////////////////
/// Transorm from db to linear units
/// Note - will convert to fl32

void RadxField::transformDbToLinear()

{

  convertToFl32();
  Radx::fl32 *fdat = (Radx::fl32 *) _data;
  for (size_t ii = 0; ii < _nPoints; ii++) {
    Radx::fl32 val = fdat[ii];
    if (val != Radx::missingFl32) {
      fdat[ii] = pow(10.0, val / 10.0);
    }
  }

}

/////////////////////////////////////////////////////////////
/// Transorm from linear to db units
/// Note - will convert to fl32

void RadxField::transformLinearToDb()

{

  convertToFl32();
  Radx::fl32 *fdat = (Radx::fl32 *) _data;
  for (size_t ii = 0; ii < _nPoints; ii++) {
    Radx::fl32 val = fdat[ii];
    if (val != Radx::missingFl32) {
      if (val <= 0.0) {
        fdat[ii] = Radx::missingFl32;
      } else {
        fdat[ii] = 10.0 * log10(val);
      }
    }
  }

}

/////////////////////////////////////////////////
// set the missing values to the default values

void RadxField::_setMissingToDefaults()

{

  _missingFl64 = Radx::missingFl64;
  _missingFl32 = Radx::missingFl32;
  _missingSi32 = Radx::missingSi32;
  _missingSi16 = Radx::missingSi16;
  _missingSi08 = Radx::missingSi08;

}

/////////////////////////////////////////////////
// get the missing value as a double

double RadxField::getMissing() const

{

  switch (_dataType) {
    case Radx::FL64:
      return _missingFl64;
    case Radx::FL32:
      return _missingFl32;
    case Radx::SI32:
      return _missingSi32;
    case Radx::SI16:
      return _missingSi16;
    case Radx::SI08:
      return _missingSi08;
    default:
      return _missingFl64;
  }

}

///////////////////////////////////////////////////////////
/// Check if the data at all gates is missing?
/// Returns true if all missing, false otherwise.

bool RadxField::checkDataAllMissing() const
  
{
  
  if (_dataType == Radx::FL64) {
    
    const Radx::fl64 *data = ((Radx::fl64*) _data);
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      if (*data != _missingFl64) {
        return false;
      }
    }

  } else if (_dataType == Radx::FL32) {
    
    const Radx::fl32 *data = ((Radx::fl32*) _data);
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      if (*data != _missingFl32) {
        return false;
      }
    }

  } else if (_dataType == Radx::SI32) {
    
    const Radx::si32 *data = ((Radx::si32*) _data);
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      if (*data != _missingSi32) {
        return false;
      }
    }

  } else if (_dataType == Radx::SI16) {
    
    const Radx::si16 *data = ((Radx::si16*) _data);
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      if (*data != _missingSi16) {
        return false;
      }
    }

  } else if (_dataType == Radx::SI08) {
    
    const Radx::si08 *data = ((Radx::si08*) _data);
    for (size_t ii = 0; ii < _nPoints; ii++, data++) {
      if (*data != _missingSi08) {
        return false;
      }
    }

  }

  return true;

}

///////////////////////////////////////////////////////////
// compute the number of gates without missing data
// i.e. all gates beyond this number have missing data

int RadxField::computeNGatesNonMissing(size_t rayNum) const
  
{
  
  if (rayNum >= _rayStartIndex.size()) {
    cerr << "ERROR - RadxField::computeNGatesNonMissing(rayNum)" << endl;
    cerr << "  specified rayNum: " << rayNum << endl;
    cerr << "  exceeds max: " << _rayStartIndex.size()-1 << endl;
  }
  assert(rayNum < _rayStartIndex.size());
  
  // for efficiency, search from the end back to the start

  int startIndex = _rayStartIndex[rayNum];
  int endIndex = startIndex + _rayNGates[rayNum] - 1;
  
  if (_dataType == Radx::FL64) {
    
    const Radx::fl64 *data = ((Radx::fl64*) _data) + endIndex;
    for (int ii = endIndex; ii >= startIndex; ii--, data--) {
      Radx::fl64 val = *data;
      if (val != _missingFl64) {
        return ii - startIndex + 1;
      }
    }

  } else if (_dataType == Radx::FL32) {
    
    const Radx::fl32 *data = ((Radx::fl32*) _data) + endIndex;
    for (int ii = endIndex; ii >= startIndex; ii--, data--) {
      Radx::fl32 val = *data;
      if (val != _missingFl32) {
        return ii - startIndex + 1;
      }
    }

  } else if (_dataType == Radx::SI32) {
    
    const Radx::si32 *data = ((Radx::si32*) _data) + endIndex;
    for (int ii = endIndex; ii >= startIndex; ii--, data--) {
      Radx::si32 val = *data;
      if (val != _missingSi32) {
        return ii - startIndex + 1;
      }
    }

  } else if (_dataType == Radx::SI16) {
    
    const Radx::si16 *data = ((Radx::si16*) _data) + endIndex;
    for (int ii = endIndex; ii >= startIndex; ii--, data--) {
      Radx::si16 val = *data;
      if (val != _missingSi16) {
        return ii - startIndex + 1;
      }
    }

  } else if (_dataType == Radx::SI08) {
    
    const Radx::si08 *data = ((Radx::si08*) _data) + endIndex;
    for (int ii = endIndex; ii >= startIndex; ii--, data--) {
      Radx::si08 val = *data;
      if (val != _missingSi08) {
        return ii - startIndex + 1;
      }
    }

  }

  return 0;

}

/////////////////////////////////////////////////
// Remap data for a single ray onto new range
// geometry using the remap object.
// If interp is set, use interpolation if appropriate.

void RadxField::remapRayGeom(const RadxRemap &remap,
                             bool interp /* = false */)
  
{

  // no data yet?
  
  assert(_data != NULL);

  // remap data using lookup tables
  
  if (interp &&
      !_isDiscrete &&
      _name.find("_FLAG") == string::npos &&
      _standardName.find("hydrometeor_type") == string::npos) {
    // interpolate if requested, and this is not a
    // discrete field
    _remapDataInterp(remap);
  } else {
    _remapDataNearest(remap);
  }

}

//////////////////////////////////////////////////////
// remap the data field using a passed-in lookup table
// for nearest neighbor

void RadxField::_remapDataNearest(const RadxRemap &remap)

{
  
  if (remap.getLookupNearest().size() < 1) {
    cerr << "WARNING - RadxField::_remapDataNearest()" << endl;
    cerr << "  Remap object not initialized for remapping" << endl;
    cerr << "  Field will be unchanged: " << _name << endl;
    return;
  }
  
  // make sure we are managing the data
  
  setDataLocal();

  const vector<int> &lookup = remap.getLookupNearest();
  int nPointsInterp = (int) lookup.size();

  switch (_dataType) {
    
    case Radx::FL64: {
      Radx::fl64 *newData = new Radx::fl64[nPointsInterp];
      Radx::fl64 *oldData = (Radx::fl64 *) _data;
      for (int ii = 0; ii < nPointsInterp; ii++) {
        int index = lookup[ii];
        if (index < 0) {
          newData[ii] = _missingFl64;
        } else {
          newData[ii] = oldData[index];
        }
      } // ii
      _buf.clear();
      _data = _buf.add(newData, nPointsInterp * sizeof(Radx::fl64));
      delete[] newData;
      break;
    }
      
    case Radx::FL32: {
      Radx::fl32 *newData = new Radx::fl32[nPointsInterp];
      Radx::fl32 *oldData = (Radx::fl32 *) _data;
      for (int ii = 0; ii < nPointsInterp; ii++) {
        int index = lookup[ii];
        if (index < 0) {
          newData[ii] = _missingFl32;
        } else {
          newData[ii] = oldData[index];
        }
      } // ii
      _buf.clear();
      _data = _buf.add(newData, nPointsInterp * sizeof(Radx::fl32));
      delete[] newData;
      break;
    }
        
    case Radx::SI32: {
      Radx::si32 *newData = new Radx::si32[nPointsInterp];
      Radx::si32 *oldData = (Radx::si32 *) _data;
      for (int ii = 0; ii < nPointsInterp; ii++) {
        int index = lookup[ii];
        if (index < 0) {
          newData[ii] = _missingSi32;
        } else {
          newData[ii] = oldData[index];
        }
      } // ii
      _buf.clear();
      _data = _buf.add(newData, nPointsInterp * sizeof(Radx::si32));
      delete[] newData;
      break;
    }
        
    case Radx::SI16: {
      Radx::si16 *newData = new Radx::si16[nPointsInterp];
      Radx::si16 *oldData = (Radx::si16 *) _data;
      for (int ii = 0; ii < nPointsInterp; ii++) {
        int index = lookup[ii];
        if (index < 0) {
          newData[ii] = _missingSi16;
        } else {
          newData[ii] = oldData[index];
        }
      } // ii
      _buf.clear();
      _data = _buf.add(newData, nPointsInterp * sizeof(Radx::si16));
      delete[] newData;
      break;
    }
        
    case Radx::SI08:
    default: {
      Radx::si08 *newData = new Radx::si08[nPointsInterp];
      Radx::si08 *oldData = (Radx::si08 *) _data;
      for (int ii = 0; ii < nPointsInterp; ii++) {
        int index = lookup[ii];
        if (index < 0) {
          newData[ii] = _missingSi08;
        } else {
          newData[ii] = oldData[index];
        }
      } // ii
      _buf.clear();
      _data = _buf.add(newData, nPointsInterp * sizeof(Radx::si08));
      delete[] newData;
      break;
    }
      
  } // switch (_dataType)
  
  // set geometry

  setRangeGeom(remap.getStartRangeKm(), remap.getGateSpacingKm());
  
  // set numebr of gates etc.
  
  clearPacking();
  addToPacking(remap.getLookupNearest().size());

}

//////////////////////////////////////////////////////
// remap the data field in range, using interpolation

void RadxField::_remapDataInterp(const RadxRemap &remap)

{

  // if interpolation has not been initialized, use nearest neighbor

  if (remap.getIndexBefore().size() == 0 ||
      remap.getIndexBefore().size() < remap.getLookupNearest().size()) {
    cerr << "WARNING - RadxField::_remapDataInterp()" << endl;
    cerr << "  Remap object not initialized for interpolation" << endl;
    cerr << "  Using nearest neighbor remapping instead" << endl;
    _remapDataNearest(remap);
    return;
  }
  
  // make sure we are managing the data
  
  setDataLocal();

  // save data type etc

  Radx::DataType_t dataTypeOrig = _dataType;
  double scaleOrig = _scale;
  double offsetOrig = _offset;

  // convert to floats
  
  convertToFl32();

  // perform the interpolation

  const vector<int> &indexBefore = remap.getIndexBefore();
  const vector<int> &indexAfter = remap.getIndexAfter();
  const vector<double> &wtBefore = remap.getWtBefore();
  const vector<double> &wtAfter = remap.getWtAfter();
  int nPointsInterp = (int) indexBefore.size();

  Radx::fl32 *newData = new Radx::fl32[nPointsInterp];
  Radx::fl32 *oldData = (Radx::fl32 *) _data;
  
  for (int ii = 0; ii < nPointsInterp; ii++) {
    int iBefore = indexBefore[ii];
    int iAfter = indexAfter[ii];
    if (iBefore < 0 || iAfter < 0) {
      newData[ii] = _missingFl32;
    } else {
      Radx::fl32 valBefore = oldData[iBefore];
      Radx::fl32 valAfter = oldData[iAfter];
      if (valBefore != _missingFl32 && valAfter != _missingFl32) {
        if (_fieldFolds) {
          newData[ii] =
            _interpFolded(valBefore, valAfter, wtBefore[ii], wtAfter[ii]);
        } else {
          newData[ii] = valBefore * wtBefore[ii] + valAfter * wtAfter[ii];
        }
      } else if (valBefore != _missingFl32) {
        newData[ii] = valBefore;
      } else if (valAfter != _missingFl32) {
        newData[ii] = valAfter;
      } else {
        newData[ii] = _missingFl32;
      }
    }
  } // ii
  _buf.clear();
  _data = _buf.add(newData, nPointsInterp * sizeof(Radx::fl32));
  delete[] newData;

  // set geometry

  setRangeGeom(remap.getStartRangeKm(), remap.getGateSpacingKm());
  
  // set numebr of gates etc.
  
  clearPacking();
  addToPacking(remap.getLookupNearest().size());

  // convert back to original type

  convertToType(dataTypeOrig, scaleOrig, offsetOrig);
  
}

/////////////////////////////////////////////////
// Remap using fewer rays
// from minRayIndex to maxRayIndex, inclusive

void RadxField::remapRays(int minRayIndex,
                          int maxRayIndex)
  
{

  // assert reasonableness

  if (minRayIndex < 0) {
    cerr << "ERROR - RadxField::remapRays" << endl;
    cerr << "  minRayIndex: " << minRayIndex << endl;
    cerr << "  min allowed: 0" << endl;
  } else if (maxRayIndex >= (int) getNRays()) {
    cerr << "ERROR - RadxField::remapRays" << endl;
    cerr << "  maxRayIndex: " << maxRayIndex << endl;
    cerr << "  max allowed: " << getNRays()-1 << endl;
  }
  assert(minRayIndex >= 0);
  assert(maxRayIndex < (int) getNRays());

  // copy into tmp objects

  RadxBuf tmpBuf(_buf);
  char *tmpData = (char *) tmpBuf.getPtr();
  vector<size_t> tmpNGates(_rayNGates);
  vector<size_t> tmpIndex(_rayStartIndex);

  // clear

  _buf.clear();
  _rayNGates.clear();
  _rayStartIndex.clear();
  _nPoints = 0;

  // copy over
  
  for (int ii = minRayIndex; ii <= maxRayIndex; ii++) {
    
    size_t nGates = tmpNGates[ii];
    size_t index = tmpIndex[ii];

    _data = _buf.add(tmpData + index * _byteWidth, nGates * _byteWidth);
    _rayNGates.push_back(nGates);
    _rayStartIndex.push_back(index);

    _nPoints += nGates;

  } // ii

}

/////////////////////////////////////////////////////////
// print

void RadxField::print(ostream &out) const
  
{
  
  out << "=============== RadxField ===============" << endl;
  out << "  name: " << _name << endl;
  if (_longName.size() > 0) {
    out << "  longName: " << _longName << endl;
  }
  if (_standardName.size() > 0) {
    out << "  standardName: " << _standardName << endl;
  }
  if (_comment.size() > 0) {
    out << "  comment: " << _comment << endl;
  }
  out << "  units: " << _units << endl;
  out << "  nRays: " << getNRays() << endl;
  out << "  nPoints: " << _nPoints << endl;
  out << "  nBytes: " << getNBytes() << endl;
  out << "  dataType: " << Radx::dataTypeToStr(_dataType) << endl;
  out << "  byteWidth: " << _byteWidth << endl;
  out << "  samplingRatio: " << _samplingRatio << endl;
  out << "  fieldFolds: " << (_fieldFolds? "Y":"N") << endl;
  if (_fieldFolds) {
    out << "    foldLimitLower: " << _foldLimitLower << endl;
    out << "    foldLimitUpper: " << _foldLimitUpper << endl;
    out << "    foldRange: " << _foldRange << endl;
  }
  out << "  isDiscrete: " << (_isDiscrete? "Y":"N") << endl;
  if (_minVal!= Radx::missingMetaDouble) {
    out << "  minVal: " << _minVal << endl;
  }
  if (_dataType != Radx::FL64 && _dataType != Radx::FL32) {
    out << "  scale: " << _scale << endl;
    out << "  offset: " << _offset << endl;
  }
  switch (_dataType) {
    case Radx::FL64:
      out << "  missingFl64: " << _missingFl64 << endl;
      break;
    case Radx::FL32:
      out << "  missingFl32: " << _missingFl32 << endl;
      break;
    case Radx::SI32:
      out << "  missingSi32: " << (int) _missingSi32 << endl;
      break;
    case Radx::SI16:
      out << "  missingSi16: " << (int) _missingSi16 << endl;
      break;
    case Radx::SI08:
      out << "  missingSi08: " << (int) _missingSi08 << endl;
      break;
    default: {}
  }
  out << "  dataIsLocal: " << _dataIsLocal << endl;
  RadxRangeGeom::print(out);
  RadxPacking::printSummary(out);
  if (_legendXml.size() > 0) {
    out << "---------------- legendXml ----------------" <<  endl;
    out << _legendXml << endl;
    out << "-------------------------------------------" <<  endl;
  }
  if (_thresholdingXml.size() > 0) {
    out << "------------- thresholdingXml -------------" <<  endl;
    out << _thresholdingXml << endl;
    out << "-------------------------------------------" <<  endl;
  }
  if (_comment.size() > 0) {
    out << "----------------- comment -----------------" <<  endl;
    out << _comment << endl;
    out << "-------------------------------------------" <<  endl;
  }
  out << "=========================================" << endl;

}

/////////////////////////////////////////////////////////
// print with data

void RadxField::printWithData(ostream &out) const
  
{

  print(out);
  out << "================== Data ===================" << endl;
  if (_data == NULL) {
    out << "========= currently no data =========" << endl;
  } else {
    int printed = 0;
    int count = 1;
    double prevVal = getDoubleValue(0);
    for (size_t ii = 1; ii < _nPoints; ii++) {
      double dval = getDoubleValue(ii);
      if (dval != prevVal) {
        _printPacked(out, count, prevVal);
        printed++;
        if (printed > 6) {
          out << endl;
          printed = 0;
        }
        prevVal = dval;
        count = 1;
      } else {
        count++;
      }
    } // ii
    _printPacked(out, count, prevVal);
    out << endl;
  }
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////////////
// print in packed format, using count for identical data values

void RadxField::_printPacked(ostream &out, int count, double val) const

{
  
  char outstr[1024];
  if (count > 1) {
    out << count << "*";
  }
  if (val == _missingFl64) {
    out << "MISS ";
  } else {
    if (fabs(val) > 0.01) {
      sprintf(outstr, "%.3f ", val);
      out << outstr;
    } else if (val == 0.0) {
      out << "0.0 ";
    } else {
      sprintf(outstr, "%.3e ", val);
      out << outstr;
    }
  }
}

/////////////////////////////////////////////////////////////////
// print data type mismatch

void RadxField::_printTypeMismatch(const string &methodName,
                                   Radx::DataType_t dtype) const
{
  if (_dataType != dtype) {
    cerr << "ERROR - RadxField::" << methodName << endl;
    cerr << "  Stored data type is incorrect: "
         << Radx::dataTypeToStr(_dataType) << endl;
    cerr << "  Should be: "
         << Radx::dataTypeToStr(dtype) << endl;
  }
}

/////////////////////////////////////////////////////////////////
// interpolate a folded value

double RadxField::_interpFolded(double val0, double val1,
                                double wt0, double wt1)
{

  // get the sin and cos components for each value

  double angle0 = _getFoldAngle(val0);
  double sin0, cos0;
  Radx::sincos(angle0, sin0, cos0);

  double angle1 = _getFoldAngle(val1);
  double sin1, cos1;
  Radx::sincos(angle1, sin1, cos1);

  // compute the weighted (x,y) interp unit vector

  double xxInterp = (wt0 * cos0) + (wt1 * cos1);
  double yyInterp = (wt0 * sin0) + (wt1 * sin1);

  double angleInterp = atan2(yyInterp, xxInterp);
  double valInterp = _getFoldValue(angleInterp);

  return valInterp;

}

/////////////////////////////////////////////////////////////////
// convert a value to an angle, for a field that folds

double RadxField::_getFoldAngle(double val)
{
  double fraction = (val - _foldLimitLower) / _foldRange;
  double angle = -M_PI + fraction * (M_PI * 2.0);
  return angle;
}

/////////////////////////////////////////////////////////////////
// convert an angle to a value, for a field that folds

double RadxField::_getFoldValue(double angle)
{
  double fraction = (angle + M_PI) / (M_PI * 2.0);
  double val = fraction * _foldRange + _foldLimitLower;
  return val;
}

/////////////////////////////////////////////////////////////////
// convert a value to an angle, for a field that folds
// static method

double RadxField::_getFoldAngle(double val,
                                double foldLimitLower,
                                double foldRange)
{
  double fraction = (val - foldLimitLower) / foldRange;
  double angle = -M_PI + fraction * (M_PI * 2.0);
  return angle;
}

/////////////////////////////////////////////////////////////////
// convert an angle to a value, for a field that folds
// static method

double RadxField::_getFoldValue(double angle,
                                double foldLimitLower,
                                double foldRange)
{
  double fraction = (angle + M_PI) / (M_PI * 2.0);
  double val = fraction * foldRange + foldLimitLower;
  return val;
}

/////////////////////////////////////////////////////////////////
/// compute stats from a series of fields
///
/// Pass in a method type, and a vector of fields
///
/// Compute the requested stats on those fields, on a point-by-point basis.
/// Create a field, fill it with the results, and return it.
///
/// If the number of points in the field is not constant, use the minumum number
/// of points in the supplied fields.
///
/// maxFractionMissing indicates the maximum fraction of the input data field
/// that can be missing for valid statistics. Should be between 0 and 1.
///
/// Returns NULL if fieldIn.size() == 0.
/// Otherwise, returns field containing results.

RadxField *RadxField::computeStats(RadxField::StatsMethod_t method,
                                   const vector<const RadxField *> &fieldsIn,
                                   double maxFractionMissing /* = 0.25 */)

{

  if (fieldsIn.size() == 0) {
    return NULL;
  }

  // get the mid field

  size_t iMid = fieldsIn.size() / 2;
  const RadxField *fieldMid = fieldsIn[iMid];

  // compute the number of points to use

  size_t nPoints = fieldsIn[iMid]->getNPoints();
  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
    if (fieldsIn[ifield]->getNPoints() < nPoints) {
      nPoints = fieldsIn[ifield]->getNPoints();
    }
  }

  // create the return field, copying over the metadata
  
  RadxField *stats =
    new RadxField(fieldMid->getName(), fieldMid->getUnits());
  stats->copyMetaData(*fieldMid);
  stats->setTypeFl64(Radx::missingFl64);

  // save the incoming data type

  Radx::DataType_t dataTypeIn = fieldMid->getDataType();

  // create results array, using doubles

  RadxArray<Radx::fl64> data_;
  Radx::fl64 *data = data_.alloc(nPoints);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    data[ipt] = Radx::missingFl64;
  }

  // compute the stats

  switch (method) {

    case STATS_METHOD_MEAN:
      if (stats->getIsDiscrete()) {
        _computeMedian(nPoints,
                       fieldsIn, 
                       data,
                       maxFractionMissing);
      } else if (stats->getFieldFolds()) {
        _computeMeanFolded(nPoints,
                           stats->getFoldLimitLower(),
                           stats->getFoldRange(),
                           fieldsIn,
                           data,
                           maxFractionMissing);
      } else {
        _computeMean(nPoints,
                     fieldsIn,
                     data,
                     maxFractionMissing);
      }
      break;
      
    case STATS_METHOD_MEDIAN:
      _computeMedian(nPoints,
                     fieldsIn,
                     data,
                     maxFractionMissing);
      break;

    case STATS_METHOD_MAXIMUM:
      _computeMaximum(nPoints,
                      fieldsIn,
                      data,
                      maxFractionMissing);
      break;

    case STATS_METHOD_MINIMUM:
      _computeMinimum(nPoints, 
                      fieldsIn,
                      data,
                      maxFractionMissing);
      break;
      
    case STATS_METHOD_MIDDLE:
    default:
      _computeMiddle(nPoints, 
                     fieldsIn,
                     data,
                     maxFractionMissing);

  } // switch

  // add data to stats field

  stats->addDataFl64(nPoints, data);
  
  // convert to incoming data type
  
  stats->convertToType(dataTypeIn);
  
  // return the created field - must be freed by caller

  return stats;

}

//////////////////////////////////////////////////////////
// compute mean

void RadxField::_computeMean(size_t nPoints,
                             const vector<const RadxField *> &fieldsIn,
                             Radx::fl64 *data,
                             double maxFractionMissing)

{

  // for fields in dB space, convert to linear, compute the
  // mean, and convert back

  string lowCaseName(_name);
  for (size_t ii = 0; ii < lowCaseName.size(); ii++) {
    lowCaseName[ii] = std::tolower(lowCaseName[ii]);
  }
  bool convertToLinear = false;
  if (lowCaseName == "db" ||
      lowCaseName == "dbm" ||
      lowCaseName == "dbz") {
    convertToLinear = true;
  }

  RadxArray<Radx::fl64> sum_;
  Radx::fl64 *sum = sum_.alloc(nPoints);
  memset(sum, 0, nPoints * sizeof(Radx::fl64));
      
  RadxArray<Radx::fl64> count_;
  Radx::fl64 *count = count_.alloc(nPoints);
  memset(count, 0, nPoints * sizeof(Radx::fl64));
  
  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
        
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
        
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (convertToLinear) {
        val = pow(10.0, val / 10.0);
      }
      if (val != miss) {
        sum[ipt] += val;
        count[ipt]++;
      }
    }
    
  } // ifield

  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    if (count[ipt] >= minValid) {
      double mean = sum[ipt] / count[ipt];
      if (convertToLinear) {
        mean = 10.0 * log10(mean);
      }
      data[ipt] = mean;
    }
  }

}

//////////////////////////////////////////////////////////
// compute mean for folded data

void RadxField::_computeMeanFolded(size_t nPoints,
                                   double foldLimitLower,
                                   double foldRange,
                                   const vector<const RadxField *> &fieldsIn,
                                   Radx::fl64 *data,
                                   double maxFractionMissing)

{
                             
  RadxArray<Radx::fl64> sumx_;
  Radx::fl64 *sumx = sumx_.alloc(nPoints);
  memset(sumx, 0, nPoints * sizeof(Radx::fl64));
      
  RadxArray<Radx::fl64> sumy_;
  Radx::fl64 *sumy = sumy_.alloc(nPoints);
  memset(sumy, 0, nPoints * sizeof(Radx::fl64));
      
  RadxArray<Radx::fl64> count_;
  Radx::fl64 *count = count_.alloc(nPoints);
  memset(count, 0, nPoints * sizeof(Radx::fl64));
  
  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
        
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
        
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (val != miss) {
        double angle =
          _getFoldAngle(val, foldLimitLower, foldRange);
        double sinVal, cosVal;
        Radx::sincos(angle, sinVal, cosVal);
        sumx[ipt] += cosVal;
        sumy[ipt] += sinVal;
        count[ipt]++;
      }
    }
    
  } // ifield
      
  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    if (count[ipt] >= minValid) {
      double angleMean = atan2(sumy[ipt], sumx[ipt]);
      data[ipt] = _getFoldValue(angleMean, foldLimitLower, foldRange);
    }
  }

}

//////////////////////////////////////////////////////////
// compute median

void RadxField::_computeMedian(size_t nPoints,
                               const vector<const RadxField *> &fieldsIn,
                               Radx::fl64 *data,
                               double maxFractionMissing)

{

  vector< vector<double> > seriesArray;
  seriesArray.resize(nPoints);

  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
    
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
    
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (val != miss) {
        seriesArray[ipt].push_back(val);
      }
    }
    
  } // ifield
  
  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    vector<double> &series = seriesArray[ipt];
    if ((int) series.size() >= minValid) {
      sort(series.begin(), series.end());
      double median = series[series.size()/2];
      data[ipt] = median;
    }
  } // ipt

}

        
//////////////////////////////////////////////////////////
// compute maximum

void RadxField::_computeMaximum(size_t nPoints,
                                const vector<const RadxField *> &fieldsIn,
                                Radx::fl64 *data,
                                double maxFractionMissing)

{
  
  RadxArray<Radx::fl64> max_;
  Radx::fl64 *max = max_.alloc(nPoints);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    max[ipt] = -1.0e99;
  }

  RadxArray<Radx::fl64> count_;
  Radx::fl64 *count = count_.alloc(nPoints);
  memset(count, 0, nPoints * sizeof(Radx::fl64));
  
  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
    
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
        
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (val != miss) {
        if (val > max[ipt]) {
          max[ipt] = val;
        }
        count[ipt]++;
      }
    }
    
  } // ifield
      
  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    if (max[ipt] > -1.0e98 && count[ipt] >= minValid) {
      data[ipt] = max[ipt];
    }
  }

}

        
//////////////////////////////////////////////////////////
// compute minimum

void RadxField::_computeMinimum(size_t nPoints,
                                const vector<const RadxField *> &fieldsIn,
                                Radx::fl64 *data,
                                double maxFractionMissing)

{
                             
  RadxArray<Radx::fl64> min_;
  Radx::fl64 *min = min_.alloc(nPoints);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    min[ipt] = 1.0e99;
  }
      
  RadxArray<Radx::fl64> count_;
  Radx::fl64 *count = count_.alloc(nPoints);
  memset(count, 0, nPoints * sizeof(Radx::fl64));
  
  for (size_t ifield = 0; ifield < fieldsIn.size(); ifield++) {
    
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
        
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (val != miss) {
        if (val < min[ipt]) {
          min[ipt] = val;
        }
        count[ipt]++;
      }
    }
    
  } // ifield
      
  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    if (min[ipt] < 1.0e98 && count[ipt] >= minValid) {
      data[ipt] = min[ipt];
    }
  }

}

//////////////////////////////////////////////////////////
// compute middle value

void RadxField::_computeMiddle(size_t nPoints,
                               const vector<const RadxField *> &fieldsIn,
                               Radx::fl64 *data,
                               double maxFractionMissing)

{
                             
  RadxArray<Radx::fl64> mid_;
  Radx::fl64 *mid = mid_.alloc(nPoints);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    mid[ipt] = NAN;
  }
      
  RadxArray<Radx::fl64> count_;
  Radx::fl64 *count = count_.alloc(nPoints);
  memset(count, 0, nPoints * sizeof(Radx::fl64));
  
  int midIndex = (int) fieldsIn.size() / 2;
  for (int ifield = 0; ifield < (int) fieldsIn.size(); ifield++) {
    
    RadxField copy(*fieldsIn[ifield]);
    copy.convertToFl64();
    const Radx::fl64 *vals = copy.getDataFl64();
    Radx::fl64 miss = copy.getMissingFl64();
        
    for (size_t ipt = 0; ipt < nPoints; ipt++, vals++) {
      Radx::fl64 val = *vals;
      if (val != miss) {
        count[ipt]++;
      }
      if (ifield == midIndex) {
        mid[ipt] = val;
      }
    }
    
  } // ifield
      
  int minValid = _computeMinValid(fieldsIn.size(), maxFractionMissing);
  for (size_t ipt = 0; ipt < nPoints; ipt++) {
    if (!std::isnan(mid[ipt]) && count[ipt] >= minValid) {
      data[ipt] = mid[ipt];
    }
  }

}

/////////////////////////////////////////////////////////
// compute the minimum number of valid points, given
// the n and the max missing fraction

int RadxField::_computeMinValid(int nn,
                                double maxFractionMissing)

{

  int minValid = (int) ((1.0 - maxFractionMissing) * nn + 0.5);
  if (minValid < 1) {
    minValid = 1;
  } else if (minValid > nn) {
    minValid = nn;
  }
  return minValid;

}

///////////////////////////////////////////
// convert enums to strings

string RadxField::statsMethodToStr(StatsMethod_t method)

{

  switch (method) {
    case STATS_METHOD_MEAN:
      return "mean";
    case STATS_METHOD_MEDIAN:
      return "median";
    case STATS_METHOD_MAXIMUM:
      return "maximum";
    case STATS_METHOD_MINIMUM:
      return "minimum";
    case STATS_METHOD_MIDDLE:
      return "middle";
    default:
      return "unknown";
  }
  
}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxField::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxFieldMsg);

  // add metadata strings as xml part
  // include null at string end

  string xml;
  _loadMetaStringsToXml(xml);
  msg.addPart(_metaStringsPartId, xml.c_str(), xml.size() + 1);

  // add metadata numbers

  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

  // add field data

  msg.addPart(_dataPartId, _data, _nPoints * _byteWidth);

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxField::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxFieldMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata strings

  const RadxMsg::Part *metaStringPart = msg.getPartByType(_metaStringsPartId);
  if (metaStringPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  No metadata string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaStringsFromXml((char *) metaStringPart->getBuf(),
                             metaStringPart->getLength())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "  Bad string XML for metadata: " << endl;
    string bufStr((char *) metaStringPart->getBuf(),
                  metaStringPart->getLength());
    cerr << "  " << bufStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata numbers
  
  const RadxMsg::Part *metaNumsPart = msg.getPartByType(_metaNumbersPartId);
  if (metaNumsPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    cerr << "  Field name: " << _name << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  Field name: " << _name << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the data part

  const RadxMsg::Part *dataPart = msg.getPartByType(_dataPartId);
  if (dataPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  No data part in message" << endl;
    cerr << "  Field name: " << _name << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  
  // check the length

  size_t nGates = _nPoints;
  size_t requiredLen = nGates * _byteWidth;
  if (dataPart->getLength() != requiredLen) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::deserialize" << endl;
    cerr << "  Incorrect field len (nbytes): " << dataPart->getLength() << endl;
    cerr << "  Should be: " << requiredLen << endl;
    cerr << "  Field name: " << _name << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // add field data

  clearData();

  switch (_dataType) {
    case Radx::FL64: {
      addDataFl64(nGates, (const Radx::fl64 *) dataPart->getBuf());
      if (msg.getSwap()) {
        ByteOrder::swap64((Radx::fl64 *) _data, requiredLen);
      }
      break;
    }
    case Radx::FL32: {
      addDataFl32(nGates, (const Radx::fl32 *) dataPart->getBuf());
      if (msg.getSwap()) {
        ByteOrder::swap32((Radx::fl32 *) _data, requiredLen);
      }
      break;
    }
    case Radx::SI32: {
      addDataSi32(nGates, (const Radx::si32 *) dataPart->getBuf());
      if (msg.getSwap()) {
        ByteOrder::swap32((Radx::si32 *) _data, requiredLen);
      }
      break;
    }
    case Radx::SI16: {
      addDataSi16(nGates, (const Radx::si16 *) dataPart->getBuf());
      if (msg.getSwap()) {
        ByteOrder::swap16((Radx::si16 *) _data, requiredLen);
      }
      break;
    }
    case Radx::SI08:
    default: {
      addDataSi08(nGates, (const Radx::si08 *) dataPart->getBuf());
      break;
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// convert string metadata to XML

void RadxField::_loadMetaStringsToXml(string &xml, int level /* = 0 */)  const
  
{

  xml.clear();
  xml += RadxXml::writeStartTag("RadxField", level);
  
  xml += RadxXml::writeString("name", level + 1, _name);
  xml += RadxXml::writeString("longName", level + 1, _longName);
  xml += RadxXml::writeString("standardName", level + 1, _standardName);
  xml += RadxXml::writeString("units", level + 1, _units);
  xml += RadxXml::writeString("legendXml", level + 1, _legendXml);
  xml += RadxXml::writeString("thresholdingXml", level + 1, _thresholdingXml);
  xml += RadxXml::writeString("thresholdFieldName", level + 1, _thresholdFieldName);
  xml += RadxXml::writeString("comment", level + 1, _comment);

  xml += RadxXml::writeEndTag("RadxField", level);


}

/////////////////////////////////////////////////////////
// set metadata strings from XML
// returns 0 on success, -1 on failure

int RadxField::_setMetaStringsFromXml(const char *xml,
                                      size_t bufLen)

{

  // check for NULL

  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::_setMetaStringsFromXml" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  string xmlStr(xml);
  
  string contents;
  if (RadxXml::readString(xmlStr, "RadxField", contents)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::_setMetaStringsFromXml" << endl;
    cerr << "  XML not delimited by 'RadxField' tags" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  int iret = 0;
  vector<string> missingTags;
  if (RadxXml::readString(contents, "name", _name)) {
    missingTags.push_back("name");
    iret = -1;
  }
  if (RadxXml::readString(contents, "longName", _longName)) {
    missingTags.push_back("longName");
    iret = -1;
  }
  if (RadxXml::readString(contents, "standardName", _standardName)) {
    missingTags.push_back("standardName");
    iret = -1;
  }
  if (RadxXml::readString(contents, "units", _units)) {
    missingTags.push_back("units");
    iret = -1;
  }
  if (RadxXml::readString(contents, "legendXml", _legendXml)) {
    missingTags.push_back("legendXml");
    iret = -1;
  }
  if (RadxXml::readString(contents, "thresholdingXml", _thresholdingXml)) {
    missingTags.push_back("thresholdingXml");
    iret = -1;
  }
  if (RadxXml::readString(contents, "thresholdFieldName", _thresholdFieldName)) {
    missingTags.push_back("thresholdFieldName");
    iret = -1;
  }
  if (RadxXml::readString(contents, "comment", _comment)) {
    missingTags.push_back("comment");
    iret = -1;
  }

  if (iret) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::_setMetaStringsFromXml" << endl;
    cerr << "  Tags missing from Xml str:" << endl;
    cerr << "  " << xmlStr << endl;
    for (size_t ii = 0; ii < missingTags.size(); ii++) {
      cerr << "    missing tag: " << missingTags[ii] << endl;
    }
    cerr << "=======================================" << endl;
    return -1;
  }
  
  return iret;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxField::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set fl64

  _metaNumbers.startRangeKm = _startRangeKm;
  _metaNumbers.gateSpacingKm = _gateSpacingKm;
  _metaNumbers.scale = _scale;
  _metaNumbers.offset = _offset;
  _metaNumbers.samplingRatio = _samplingRatio;
  _metaNumbers.foldLimitLower = _foldLimitLower;
  _metaNumbers.foldLimitUpper = _foldLimitUpper;
  _metaNumbers.foldRange = _foldRange;
  _metaNumbers.minVal = _minVal;
  _metaNumbers.maxVal = _maxVal;
  _metaNumbers.missingFl64 = _missingFl64;
  _metaNumbers.thresholdValue = _thresholdValue;

  // set fl32

  _metaNumbers.missingFl32 = _missingFl32;

  // set si32

  _metaNumbers.rangeGeomSet = _rangeGeomSet;
  _metaNumbers.nGates = _nPoints;
  _metaNumbers.dataType = _dataType;
  _metaNumbers.byteWidth = _byteWidth;
  _metaNumbers.fieldFolds = _fieldFolds;
  _metaNumbers.isDiscrete = _isDiscrete;
  _metaNumbers.missingSi32 = _missingSi32;
  _metaNumbers.missingSi16 = _missingSi16;
  _metaNumbers.missingSi08 = _missingSi08;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxField::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                      size_t bufLen,
                                      bool swap)
  
{

  // check size

  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxField::_setMetaNumbersFromMsg" << endl;
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

  setRangeGeom(_metaNumbers.startRangeKm,
               _metaNumbers.gateSpacingKm);

  clearPacking();
  addToPacking(_metaNumbers.nGates);
  
  _scale = _metaNumbers.scale;
  _offset = _metaNumbers.offset;
  _samplingRatio = _metaNumbers.samplingRatio;
  _foldLimitLower = _metaNumbers.foldLimitLower;
  _foldLimitUpper = _metaNumbers.foldLimitUpper;
  _foldRange = _metaNumbers.foldRange;
  _minVal = _metaNumbers.minVal;
  _maxVal = _metaNumbers.maxVal;
  _missingFl64 = _metaNumbers.missingFl64;
  _thresholdValue = _metaNumbers.thresholdValue;
  
  _missingFl32 = _metaNumbers.missingFl32;
  
  _rangeGeomSet = _metaNumbers.rangeGeomSet;
  _dataType = (Radx::DataType_t) _metaNumbers.dataType;
  _byteWidth = _metaNumbers.byteWidth;
  _fieldFolds = _metaNumbers.fieldFolds;
  _isDiscrete = _metaNumbers.isDiscrete;
  _missingSi32 = _metaNumbers.missingSi32;
  _missingSi16 = (Radx::si16) _metaNumbers.missingSi16;
  _missingSi08 = (Radx::si08) _metaNumbers.missingSi08;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxField::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.scale, 16 * sizeof(Radx::si64));
  ByteOrder::swap32(&meta.missingFl32, 16 * sizeof(Radx::si32));
}
          

