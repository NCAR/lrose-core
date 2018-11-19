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
// RadxRay.cc
//
// Field object for Radx data
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRemap.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cassert>
using namespace std;

/////////////////////////////////////////////////////////
// RadxRay constructor

RadxRay::RadxRay()
  
{

  // initialize fields
  
  _init();

  // initialize client counting

  _nClients = 0;

#if defined (__linux__)
  _nClientsMutex = PTHREAD_MUTEX_INITIALIZER;
#else
  pthread_mutex_init(&_nClientsMutex, NULL);
#endif
  
}

/////////////////////////////
// Copy constructor
//

RadxRay::RadxRay(const RadxRay &rhs)
     
{
  // initialize fields
  
  _init();

  // initialize client counting

  _nClients = 0;

#if defined (__linux__)
  _nClientsMutex = PTHREAD_MUTEX_INITIALIZER;
#else
  pthread_mutex_init(&_nClientsMutex, NULL);
#endif

  // copy
  
  _copy(rhs);

}

/////////////////////////////////////////////////////////
// RadxRay destructor

RadxRay::~RadxRay()
  
{
  clearFields();
  clearGeoref();
  clearCfactors();
  pthread_mutex_destroy(&_nClientsMutex);
}

/////////////////////////////
// Assignment
//

RadxRay &RadxRay::operator=(const RadxRay &rhs)
  
  
{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// initialize

void RadxRay::_init()
  
{

  _rayNum = Radx::missingMetaInt;
  _volNum = Radx::missingMetaInt;
  _sweepNum = Radx::missingMetaInt;
  _calibIndex = -1;

  _sweepMode = Radx::missingSweepMode;
  _polarizationMode = Radx::missingPolarizationMode;
  _prtMode = Radx::missingPrtMode;
  _followMode = Radx::missingFollowMode;
 
  _timeSecs = 0;
  _nanoSecs = 0;

  _az = Radx::missingMetaDouble;
  _elev = Radx::missingMetaDouble;
  _fixedAngle = Radx::missingMetaDouble;
  _targetScanRate = Radx::missingMetaDouble;
  _trueScanRate = Radx::missingMetaDouble;

  _isIndexed = false;
  _angleRes = Radx::missingMetaDouble;

  _antennaTransition = false;
  _nSamples = 0;

  _pulseWidthUsec = Radx::missingMetaDouble;
  _prtSec = Radx::missingMetaDouble;
  _prtRatio = Radx::missingMetaDouble;
  _nyquistMps = Radx::missingMetaDouble;
  _unambigRangeKm = Radx::missingMetaDouble;
  
  _measXmitPowerDbmH = Radx::missingMetaDouble;
  _measXmitPowerDbmV = Radx::missingMetaDouble;

  _estimatedNoiseDbmHc = Radx::missingMetaDouble;
  _estimatedNoiseDbmVc = Radx::missingMetaDouble;
  _estimatedNoiseDbmHx = Radx::missingMetaDouble;
  _estimatedNoiseDbmVx = Radx::missingMetaDouble;

  clearEventFlags();

  _isLongRange = false;

  _georef = NULL;
  _georefApplied = false;
  _cfactors = NULL;
  
  _nGates = 0;

  clearRangeGeom();
  clearFields();

}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxRay &RadxRay::_copy(const RadxRay &rhs)

{

  if (&rhs == this) {
    return *this;
  }

  // copy the meta data

  copyMetaData(rhs);

  // copy the fields

  clearFields();
  for (size_t ii = 0; ii < rhs._fields.size(); ii++) {
    RadxField *field = new RadxField(*rhs._fields[ii]);
    _fields.push_back(field);
  }

  // load up map of field names

  loadFieldNameMap();

  return *this;
  
}

//////////////////////////////////////////////////
// copy meta data only
//

void RadxRay::copyMetaData(const RadxRay &rhs)

{

  if (&rhs == this) {
    return;
  }
  
  _rayNum = rhs._rayNum;
  _volNum = rhs._volNum;
  _sweepNum = rhs._sweepNum;
  _calibIndex = rhs._calibIndex;

  _sweepMode = rhs._sweepMode;
  _polarizationMode = rhs._polarizationMode;
  _prtMode = rhs._prtMode;
  _followMode = rhs._followMode;

  _timeSecs = rhs._timeSecs;
  _nanoSecs = rhs._nanoSecs;
  
  _az = rhs._az;
  _elev = rhs._elev;
  _fixedAngle = rhs._fixedAngle;
  _targetScanRate = rhs._targetScanRate;
  _trueScanRate = rhs._trueScanRate;

  _isIndexed = rhs._isIndexed;
  _angleRes = rhs._angleRes;

  _antennaTransition = rhs._antennaTransition;
  _nSamples = rhs._nSamples;

  _pulseWidthUsec = rhs._pulseWidthUsec;
  _prtSec = rhs._prtSec;
  _prtRatio = rhs._prtRatio;
  _nyquistMps = rhs._nyquistMps;
  _unambigRangeKm = rhs._unambigRangeKm;

  _measXmitPowerDbmH = rhs._measXmitPowerDbmH;
  _measXmitPowerDbmV = rhs._measXmitPowerDbmV;

  _estimatedNoiseDbmHc = rhs._estimatedNoiseDbmHc;
  _estimatedNoiseDbmVc = rhs._estimatedNoiseDbmVc;
  _estimatedNoiseDbmHx = rhs._estimatedNoiseDbmHx;
  _estimatedNoiseDbmVx = rhs._estimatedNoiseDbmVx;

  _eventFlagsSet = rhs._eventFlagsSet;
  _startOfSweepFlag = rhs._startOfSweepFlag;
  _endOfSweepFlag = rhs._endOfSweepFlag;
  _startOfVolumeFlag = rhs._startOfVolumeFlag;
  _endOfVolumeFlag = rhs._endOfVolumeFlag;

  _utilityFlag = rhs._utilityFlag;

  _isLongRange = rhs._isLongRange;

  if (rhs._georef != NULL) {
    _georef = new RadxGeoref(*rhs._georef);
  } else {
    _georef = NULL;
  }
  _georefApplied = rhs._georefApplied;
  
  if (rhs._cfactors != NULL) {
    _cfactors = new RadxCfactors(*rhs._cfactors);
  } else {
    _cfactors = NULL;
  }
  
  _nGates = rhs._nGates;
  copyRangeGeom(rhs);
  
  clearFields();

}

//////////////////////////////////////////////////
/// copy meta data from sweep
///

void RadxRay::setMetadataFromSweep(const RadxSweep &sweep)

{
  _sweepNum = sweep.getSweepNumber();
  _volNum = sweep.getVolumeNumber();
  if (_sweepMode == Radx::SWEEP_MODE_NOT_SET) {
    _sweepMode = sweep.getSweepMode();
  }
  if (_polarizationMode == Radx::POL_MODE_NOT_SET) {
    _polarizationMode = sweep.getPolarizationMode();
  }
  if (_prtMode == Radx::PRT_MODE_NOT_SET) {
    _prtMode = sweep.getPrtMode();
  }
  if (_followMode == Radx::FOLLOW_MODE_NOT_SET) {
    _followMode = sweep.getFollowMode();
  }
  _isIndexed = sweep.getRaysAreIndexed();
  _angleRes = sweep.getAngleResDeg();
  _targetScanRate = sweep.getTargetScanRateDegPerSec();
  _trueScanRate = sweep.getMeasuredScanRateDegPerSec();
  _isLongRange = sweep.getIsLongRange();
}

/////////////////////////////////////////////////////////////////
/// Load up the field name map.
/// Generally this will be done automatically by the ray object.
/// However, if you rename fields which have previously been added,
/// you will need to call this method to update the field name map.

void RadxRay::loadFieldNameMap()
  
{
  _fieldNameMap.clear();
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    string name = _fields[ii]->getName();
    string mapName = _addToFieldNameMap(name, ii);
    if (mapName != name) {
      // rename the field
      _fields[ii]->setName(mapName);
    }
  }
}

/////////////////////////////////////////////////////////
// add an entry to the field name map
//
// returns the unique name actually added
// this may be altered to avoid duplicates

string RadxRay::_addToFieldNameMap(const string &name, int index)
  
{
  
  // try to insert name
  
  pair<FieldNameMapIt, bool> insRet =
    _fieldNameMap.insert(FieldNameMapPair(name, index));

  if (insRet.second == true) {
    // success
    return name;
  }

  // there are duplicates, so modify the name and add the 
  // modified name

  int count = 2;
  while (true) {

    char numStr[128];
    sprintf(numStr, "_%d", count);

    string modName(name);
    modName += numStr;

    pair<FieldNameMapIt, bool> insRet =
      _fieldNameMap.insert(FieldNameMapPair(modName, index));

    if (insRet.second == true) {
      // success
      return modName;
    }

    count++;
    if (count > 10) {
      break;
    }

  } // while

  return "failure";

}

/////////////////////////////////////////////////////////
// clear the field data in the object, and the
// field vector

void RadxRay::clearFields()
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
  _fieldNameMap.clear();
}

/////////////////////////////////////////////////////////
// Remove a field from the field vector, given its name

int RadxRay::removeField(const string &name)
  
{
  for (vector<RadxField*>::iterator ii = _fields.begin();
       ii != _fields.end(); ii++) {
    RadxField *field = *ii;
    if (field->getName() == name) {
      delete field;
      _fields.erase(ii);
      loadFieldNameMap();
      return 0;
    }
  }
  cerr << "ERROR - RadxRay::removeField" << endl;
  cerr << "  Cannot find field: " << name << endl;
  cerr << "  Field not removed" << endl;
  return -1;
}

/////////////////////////////////////////////////////////
/// Replace a field, using its name to find the field
/// to be replaced.
/// Returns 0 on success, -1 on failure.
/// On success, replaced field is deleted from memory.

int RadxRay::replaceField(RadxField *newField)

{

  string newName = newField->getName();

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    RadxField *fld = _fields[ii];
    if (fld->getName() == newName) {
      delete fld;
      _fields[ii] = newField;
      loadFieldNameMap();
      return 0;
    }
  }
  cerr << "ERROR - RadxRay::replaceField" << endl;
  cerr << "  Cannot find field: " << newName << endl;
  cerr << "  Field not replaced" << endl;
  return -1;

}
  
/////////////////////////////////////////////////////////
/// Trim the fields to only include those specified in
/// the wanted list

void RadxRay::trimToWantedFields(const vector<string> &wantedNames)
  
{

  // determine which fields to accept and which to reject

  vector<RadxField *> accept, reject;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    RadxField *field = _fields[ii];
    bool wanted = false;
    for (size_t jj = 0; jj < wantedNames.size(); jj++) {
      if (field->getName() == wantedNames[jj]) {
        wanted = true;
        break;
      }
    } // jj
    if (wanted) {
      accept.push_back(field);
    } else {
      reject.push_back(field);
    }
  } // ii

  if (reject.size() < 1) {
    return;
  }

  // delete the rejected fields

  for (size_t ii = 0; ii < reject.size(); ii++) {
    delete reject[ii];
  }

  // copy the accepted fields to the field vector

  _fields = accept;
  loadFieldNameMap();

}

///////////////////////////////////////////////////////////
/// Reorder the fields, by name, removing any extra fields.

void RadxRay::reorderFieldsByName(const vector<string> &names)
  
{

  // trim to this list

  trimToWantedFields(names);

  // Are the fields already in order?

  bool ordered = true;
  int prevPos = -1;
  for (int ii = 0; ii < (int) names.size(); ii++) {
    for (int jj = 0; jj < (int) _fields.size(); jj++) {
      if (_fields[jj]->getName() == names[ii]) {
        if (jj > prevPos) {
          prevPos = jj;
        } else {
          ordered = false;
          break;
        }
      }
    } // jj
  } // ii
  if (ordered) {
    // fields already exist and are in the correct order
    return;
  }
  
  // store fields in order
  
  vector<RadxField *> reordered;
  for (int ii = 0; ii < (int) names.size(); ii++) {
    for (int jj = 0; jj < (int) _fields.size(); jj++) {
      if (_fields[jj]->getName() == names[ii]) {
        reordered.push_back(_fields[jj]);
        break;
      }
    } // jj
  } // ii

  // copy the reordered fields to the field vector

  _fields = reordered;

  loadFieldNameMap();

}

/////////////////////////////////////////////////////////
/// Rename a field
/// returns 0 on success, -1 if field does not exist

int RadxRay::renameField(const string &oldName, const string &newName)
  
{

  RadxField *fld = getField(oldName);
  if (fld == NULL) {
    return -1;
  }
  fld->setName(newName);
  return 0;

}

/////////////////////////////////////////////////////////
/// Check that the fields match those in the template, both
/// in name and order. Rearrange, delete and add fields as
/// appropriate.

void RadxRay::makeFieldsMatchTemplate(const vector<const RadxField *> &tplate)
  
{
  
  // Do the fields already match

  if (_fields.size() == tplate.size()) {
    bool matched = true;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      if (_fields[ii]->getName() != tplate[ii]->getName()) {
        matched = false;
        break;
      }
    }
    if (matched) {
      // fields already exist and are in the correct order
      return;
    }
  }

  // trim to fields in template

  vector<string> wantedNames;
  for (size_t ii = 0; ii < tplate.size(); ii++) {
    wantedNames.push_back(tplate[ii]->getName());
  }
  trimToWantedFields(wantedNames);
  
  // go through the template, finding the fields we want,
  // adding them in order, and filling in missing ones
  
  vector<RadxField *> reordered;
  for (size_t ii = 0; ii < tplate.size(); ii++) {
    const RadxField *tfield = tplate[ii];
    bool found = false;
    for (size_t jj = 0; jj < _fields.size(); jj++) {
      if (_fields[jj]->getName() == tfield->getName()) {
        reordered.push_back(_fields[jj]);
        found = true;
        break;
      }
    } // jj
    if (!found) {
      // create a new field, fill with missing and add it
      RadxField *fld = new RadxField(tfield->getName(), tfield->getUnits());
      fld->copyMetaData(*tfield);
      fld->setNGates(_nGates);
      reordered.push_back(fld);
    }
  } // ii

  _fields = reordered;
  loadFieldNameMap();

}

//////////////////////////////////////////////////////////////
// set the georeference info

void RadxRay::setGeoref(const RadxGeoref &ref)
  
{
  clearGeoref();
  _georef = new RadxGeoref(ref);
}

/////////////////////////////////////////////////////////
// clear the georeference info

void RadxRay::clearGeoref()
  
{
  if (_georef) {
    delete _georef;
  }
  _georef = NULL;
}

//////////////////////////////////////////////////////////////
// set cfactors info

void RadxRay::setCfactors(const RadxCfactors &cfac)
  
{
  clearCfactors();
  _cfactors = new RadxCfactors(cfac);
}

/////////////////////////////////////////////////////////
// clear the cfactors info

void RadxRay::clearCfactors()
  
{
  if (_cfactors) {
    delete _cfactors;
  }
  _cfactors = NULL;
}

/////////////////////////////////////////////////////////
// add a field of doubles, returns pointer to field added

RadxField* RadxRay::addField(const string &name,
                             const string &units,
                             size_t nGates,
                             Radx::fl64 missingValue,
                             const Radx::fl64 *data,
                             bool isLocal)
  
{
  if (_fields.size() > 0) {
    _nGates = _fields[0]->getNPoints();
    assert(_nGates == nGates);
  } else {
    _nGates = nGates;
  }
  RadxField *fld = new RadxField(name, units);
  fld->copyRangeGeom(*this);
  fld->setTypeFl64(missingValue);
  fld->setDataFl64(nGates, data, isLocal);
  string mapName = _addToFieldNameMap(name, _fields.size());
  if (mapName != name) {
    fld->setName(mapName);
  }
  _fields.push_back(fld);
  return fld;
}

/////////////////////////////////////////////////////////
// add a field of floats, returns pointer to field added

RadxField* RadxRay::addField(const string &name,
                             const string &units,
                             size_t nGates,
                             Radx::fl32 missingValue,
                             const Radx::fl32 *data,
                             bool isLocal)

{
  if (_fields.size() > 0) {
    _nGates = _fields[0]->getNPoints();
    assert(_nGates == nGates);
  } else {
    _nGates = nGates;
  }
  RadxField *fld = new RadxField(name, units);
  fld->copyRangeGeom(*this);
  fld->setTypeFl32(missingValue);
  fld->setDataFl32(nGates, data, isLocal);
  string mapName = _addToFieldNameMap(name, _fields.size());
  if (mapName != name) {
    fld->setName(mapName);
  }
  _fields.push_back(fld);
  return fld;
}

/////////////////////////////////////////////////////////
// add a field of ints, returns pointer to field added

RadxField* RadxRay::addField(const string &name,
                             const string &units,
                             size_t nGates,
                             Radx::si32 missingValue,
                             const Radx::si32 *data,
                             double scale,
                             double offset,
                             bool isLocal)

{
  if (_fields.size() > 0) {
    _nGates = _fields[0]->getNPoints();
    assert(_nGates == nGates);
  } else {
    _nGates = nGates;
  }
  RadxField *fld = new RadxField(name, units);
  fld->copyRangeGeom(*this);
  fld->setTypeSi32(missingValue, scale, offset);
  fld->setDataSi32(nGates, data, isLocal);
  string mapName = _addToFieldNameMap(name, _fields.size());
  if (mapName != name) {
    fld->setName(mapName);
  }
  _fields.push_back(fld);
  return fld;
}

/////////////////////////////////////////////////////////
// add a field of shorts, returns pointer to field added

RadxField* RadxRay::addField(const string &name,
                             const string &units,
                             size_t nGates,
                             Radx::si16 missingValue,
                             const Radx::si16 *data,
                             double scale,
                             double offset,
                             bool isLocal)

{
  if (_fields.size() > 0) {
    _nGates = _fields[0]->getNPoints();
    assert(_nGates == nGates);
  } else {
    _nGates = nGates;
  }
  RadxField *fld = new RadxField(name, units);
  fld->copyRangeGeom(*this);
  fld->setTypeSi16(missingValue, scale, offset);
  fld->setDataSi16(nGates, data, isLocal);
  string mapName = _addToFieldNameMap(name, _fields.size());
  if (mapName != name) {
    fld->setName(mapName);
  }
  _fields.push_back(fld);
  return fld;
}

/////////////////////////////////////////////////////////
// add a field of bytes, returns pointer to field added

RadxField* RadxRay::addField(const string &name,
                             const string &units,
                             size_t nGates,
                             Radx::si08 missingValue,
                             const Radx::si08 *data,
                             double scale,
                             double offset,
                             bool isLocal)

{
  if (_fields.size() > 0) {
    _nGates = _fields[0]->getNPoints();
    assert(_nGates == nGates);
  } else {
    _nGates = nGates;
  }
  RadxField *fld = new RadxField(name, units);
  fld->copyRangeGeom(*this);
  fld->setTypeSi08(missingValue, scale, offset);
  fld->setDataSi08(nGates, data, isLocal);
  string mapName = _addToFieldNameMap(name, _fields.size());
  if (mapName != name) {
    fld->setName(mapName);
  }
  _fields.push_back(fld);
  return fld;
}

////////////////////////////////////////////////////////////////////
/// Add a previously-created field to the ray. The field must have
/// been dynamically allocted using new(). Memory management for
/// this field passes to the ray, which will free the field object
/// using delete().
///
///

void RadxRay::addField(RadxField *field,
                       bool addToFront /* = false */)
{

  if (_fields.size() > 0) {
    size_t fieldNGates = field->getNPoints();
    if (fieldNGates < _nGates) {
      // pad the field to be added to match prev ngates
      field->setNGates(_nGates);
    } else if (fieldNGates > _nGates) {
      // pad the previously added fields to match new ngates
      _nGates = fieldNGates;
      for (size_t ii = 0; ii < _fields.size(); ii++) {
        _fields[ii]->setNGates(_nGates);
      }
    }
  } else {
    _nGates = field->getNPoints();
  }

  if (addToFront) {
    vector<RadxField *> tmp = _fields;
    _fields.clear();
    _fields.push_back(field);
    for (size_t ii = 0; ii < tmp.size(); ii++) {
      _fields.push_back(tmp[ii]);
    }
  } else {
    _fields.push_back(field);
  }

  loadFieldNameMap();

}

/////////////////////////////////////////////////////////
// set the fields so that they manage their data locally
// rather than pointing to another object

void RadxRay::setDataLocal()
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setDataLocal();
  }
}

/////////////////////////////////////////////////////////
// set unambiguous range, if is it not already set.
//
// Before calling this, make sure the prt mode is set,
// along with the prt values

void RadxRay::setUnambigRange()
  
{

  if (_unambigRangeKm != Radx::missingMetaDouble) {
    return;
  }

  if (_prtSec == Radx::missingMetaDouble) {
    // cannot compute range without prt
    return;
  }

  double maxRange = 0;

  switch (_prtMode) {
    case Radx::PRT_MODE_DUAL:
    case Radx::PRT_MODE_STAGGERED: {
      if (_prtRatio == Radx::missingMetaDouble) {
        // ratio should be set
        return;
      }
      // use the short PRT
      double shortPrt = _prtSec;
      maxRange = (shortPrt * Radx::LIGHT_SPEED) / 2.0;
      break;
    }
    default: {
      maxRange = (_prtSec * Radx::LIGHT_SPEED) / 2.0;
    }
  }

  _unambigRangeKm = maxRange  / 1000.0;

}

/////////////////////////////////////////////////
// Remap data onto new gate geometry.
// If no mapping is possible, the gate is set to missing
// If interp is true, interpolation is used.
// If interp is false, nearest neighbor is used.

void RadxRay::remapRangeGeom(double newStartRangeKm,
                             double newGateSpacingKm,
                             bool interp /* = false */)
  
{

  RadxRemap remap;
  if (remap.checkGeometryIsDifferent(_startRangeKm, _gateSpacingKm,
                                     newStartRangeKm, newGateSpacingKm)) {
    remap.prepareForInterp(_nGates,
                           _startRangeKm, _gateSpacingKm,
                           newStartRangeKm, newGateSpacingKm);
    
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->remapRayGeom(remap, interp);
    }

    setNGates(remap.getNGatesInterp());
    RadxRangeGeom::setRangeGeom(newStartRangeKm, newGateSpacingKm);
    
  }

}

/////////////////////////////////////////////////
// Remap data onto new gate geometry,
// given the remap object.
// If interp is true, interpolation is used.
// If interp is false, nearest neighbor is used.

void RadxRay::remapRangeGeom(RadxRemap &remap,
                             bool interp /* = false */)
  
{
  remapRangeGeom(remap.getStartRangeKm(),
                 remap.getGateSpacingKm(),
                 interp);
}

////////////////////////////////////////////////////////
// Remap field data onto new gate geometry using finest
// resolution in any of the fields.
// If all fields are the same, there is no change.

void RadxRay::remapRangeGeomToFinest(bool interp /* = false */)

{

  if (_fields.size() < 1) {
    return;
  }

  // check through the fields for geometry differences

  double gateSpacing0 = _fields[0]->getGateSpacingKm();
  double startRange0 = _fields[0]->getStartRangeKm();
  bool fieldsDiffer = false;
  double minGateSpacing = gateSpacing0;
  double minStartRange = startRange0;
  double smallDouble = 0.00001;
  
  for (size_t ii = 1; ii < _fields.size(); ii++) {
    double gateSpacing = _fields[ii]->getGateSpacingKm();
    if (fabs(gateSpacing0 - gateSpacing) > smallDouble) {
      fieldsDiffer = true;
    }
    if (minGateSpacing > gateSpacing) {
      minGateSpacing = gateSpacing;
    }
    double startRange = _fields[ii]->getStartRangeKm();
    if (fabs(startRange0 - startRange) > smallDouble) {
      fieldsDiffer = true;
    }
    if (minStartRange > startRange) {
      minStartRange = startRange;
    }
  }

  if (!fieldsDiffer) {
    // fields all have same geometry
    // make sure ray has same geometry as fields
    copyRangeGeom(*_fields[0]);
    return;
  }

  // copy range geom from first field

  copyRangeGeom(*_fields[0]);

  // remap to finest geometry, and largest number of gates
  
  remapRangeGeom(minStartRange, minGateSpacing, interp);

}

/////////////////////////////////////////////////
/// Copy the range geom from the fields, if
/// the fields are consistent in geometry

void RadxRay::copyRangeGeomFromFields()
  
{
  
  if (_fields.size() < 1) {
    return;
  }

  double startRangeKm = _fields[0]->getStartRangeKm();
  double gateSpacingKm = _fields[0]->getGateSpacingKm();
  
  for (size_t ii = 1; ii < _fields.size(); ii++) {

    if (fabs(startRangeKm - _fields[ii]->getStartRangeKm()) > 0.001) {
      // not consistent
      return;
    }

    if (fabs(gateSpacingKm - _fields[ii]->getGateSpacingKm()) > 0.001) {
      // not consistent
      return;
    }
    
  }

  if (fabs(startRangeKm - _startRangeKm) > 0.001) {
    _startRangeKm = startRangeKm;
  }
  if (fabs(gateSpacingKm - _gateSpacingKm) > 0.001) {
    _gateSpacingKm = gateSpacingKm;
  }

  _rangeGeomSet = true;

}

/////////////////////////////////////////////////////////
// override set geometry for constant gate spacing

void RadxRay::setRangeGeom(double startRangeKm,
                           double gateSpacingKm)
  
{
  
  // set on the base class

  RadxRangeGeom::setRangeGeom(startRangeKm, gateSpacingKm);

  // fix fields

  copyRangeGeomToFields();

}

/////////////////////////////////////////////////
/// Copy the range geom to the fields

void RadxRay::copyRangeGeomToFields()
  
{
  
  if (_fields.size() < 1) {
    return;
  }

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->copyRangeGeom(*this);
  }
  
}

/////////////////////////////////////////////////////////////////////////
/// Set the number of gates.
///
/// If more gates are needed, extend the field data out to a set number of
/// gates. The data for extra gates are set to missing values.
///
/// If fewer gates are needed, the data is truncated.
  
void RadxRay::setNGates(size_t nGates)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setNGates(nGates);
  }
  _nGates = nGates;
}

/////////////////////////////////////////////////////////
/// Set value at a specified gate to missing

void RadxRay::setGateToMissing(size_t gateNum)
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setGateToMissing(gateNum);
  }
}

/////////////////////////////////////////////////////////
/// Set values at specified gates to missing

void RadxRay::setGatesToMissing(size_t startGate, size_t endGate)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setGatesToMissing(startGate, endGate);
  }
}

/////////////////////////////////////////////////////////
/// Set values within specified range limits to missing

void RadxRay::setRangeIntervalToMissing(double startRangeKm, double endRangeKm)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->setRangeIntervalToMissing(startRangeKm, endRangeKm);
  }
}

/////////////////////////////////////////////////////////////////////////
/// Set the maximum range.
/// Removes excess gates as needed.
/// Does nothing if the current max range is less than that specified.

void RadxRay::setMaxRangeKm(double maxRangeKm)
{
  int maxNGates =
    (int) ((maxRangeKm - _startRangeKm) / _gateSpacingKm + 0.5);
  if (maxNGates < 1) {
    maxNGates = 1;
  }
  if (maxNGates < (int) _nGates) {
    setNGates(maxNGates);
  }
}

//////////////////////////////////////////////////////
/// Set to constant number of gates on all fields.
/// 
/// First we determine the max number of gates.
/// If the number of gates does between fields,
/// the shorter fields are padded out with missing data.

void RadxRay::setNGatesConstant()

{

  size_t maxNGates = 0;
  
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    RadxField &field = *_fields[ii];
    if (field.getNPoints() > maxNGates) {
      maxNGates = field.getNPoints();
    }
  }

  if (maxNGates != _nGates) {
    setNGates(maxNGates);
  }

}

////////////////////////////////////////////////////////////////
/// Apply a linear transformation to the data values in a field.
/// Transforms x to y as follows:
///   y = x * scale + offset
/// After operation, field type is unchanged.
/// Nothing is done if field does not exist.

void RadxRay::applyLinearTransform(const string &name,
                                   double scale, double offset)

{
  RadxField *field = getField(name);
  if (field == NULL) {
    return;
  }
  field->applyLinearTransform(scale, offset);
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
/// If targetType is Radx::ASIS, no conversion is performed.
///
/// If a string is empty, the value on the field will
/// be left unchanged.

void RadxRay::convertField(const string &name,
                           Radx::DataType_t type,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    RadxField &field = *_fields[ii];
    if (field.getName() == name) {
      field.convert(type, newName, units, standardName, longName);
    }
  }
  loadFieldNameMap();
  
}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// If the data type is an integer type, the specified
/// scale an offset are used.
///
/// If targetType is Radx::ASIS, no conversion is performed.
///
/// If a string is empty, the value on the field will
/// be left unchanged.

void RadxRay::convertField(const string &name,
                           Radx::DataType_t type,
                           double scale,
                           double offset,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    RadxField &field = *_fields[ii];
    if (field.getName() == name) {
      field.convert(type, scale, offset,
                    newName, units, standardName, longName);
    }
  }
  loadFieldNameMap();
  
}

/////////////////////////////////////////////////
// data type conversions

void RadxRay::convertToFl64()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToFl64();
  }
}

void RadxRay::convertToFl32()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToFl32();
  }
}

void RadxRay::convertToSi32()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi32();
  }
}

void RadxRay::convertToSi32(double scale, double offset)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi32(scale, offset);
  }
}

void RadxRay::convertToSi16()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi16();
  }
}

void RadxRay::convertToSi16(double scale, double offset)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi16(scale, offset);
  }
}

void RadxRay::convertToSi08()
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi08();
  }
}

void RadxRay::convertToSi08(double scale, double offset)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToSi08(scale, offset);
  }
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

void RadxRay::convertToType(Radx::DataType_t targetType)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(targetType);
  }
}

/////////////////////////////////////////////////////////
/// convert to specified type
///
/// If the data type is an integer type, the specified
/// scale and offset are used.
///
/// If targetType is Radx::ASIS, no conversion is performed.

void RadxRay::convertToType(Radx::DataType_t targetType,
                            double scale,
                            double offset)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(targetType, scale, offset);
  }
}

///////////////////////////////////////////////////////////
/// Check if the data at all gates in all fields is missing?
/// Returns true if all missing, false otherwise.

bool RadxRay::checkDataAllMissing() const
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    if (!_fields[ii]->checkDataAllMissing()) {
      return false;
    }
  }
  return true;
}
  
/////////////////////////////////////////////////////////
// print

void RadxRay::print(ostream &out) const
  
{
  
  out << "=============== RadxRay ===============" << endl;
  if (_rayNum != Radx::missingMetaInt) {
    out << "  rayNum: " << _rayNum << endl;
  }
  out << "  volNum: " << _volNum << endl;
  out << "  sweepNum: " << _sweepNum << endl;
  out << "  calibIndex: " << _calibIndex << endl;
  out << "  sweepMode: "
      << Radx::sweepModeToStr(_sweepMode) << endl;
  out << "  polarizationMode: "
      << Radx::polarizationModeToStr(_polarizationMode) << endl;
  out << "  prtMode: " 
      << Radx::prtModeToStr(_prtMode) << endl;
  out << "  followMode: "
      << Radx::followModeToStr(_followMode) << endl;
  if (_nanoSecs == 0) {
    out << "  timeSecs: " << RadxTime::strm(_timeSecs) << ".0" << endl;
  } else {
    char text[128];
    sprintf(text, "  timeSecs: %s.%.6d",
            RadxTime::strm(_timeSecs).c_str(),
            (int) ((_nanoSecs / 1000.0) + 0.5));
    out << text << endl;
  }
  out << "  az: " << _az << endl;
  out << "  elev: " << _elev << endl;
  out << "  fixedAngle: " << _fixedAngle << endl;
  out << "  targetScanRate: " << _targetScanRate << endl;
  out << "  trueScanRate: " << _trueScanRate << endl;
  out << "  isIndexed: " << _isIndexed << endl;
  out << "  angleRes: " << _angleRes << endl;
  out << "  antennaTransition: " << _antennaTransition << endl;
  out << "  nSamples: " << _nSamples << endl;
  out << "  pulseWidthUsec: " << _pulseWidthUsec << endl;
  out << "  prtSec: " << _prtSec << endl;
  out << "  prtRatio: " << _prtRatio << endl;
  out << "  nyquistMps: " << _nyquistMps << endl;
  out << "  unambigRangeKm: " << _unambigRangeKm << endl;
  out << "  measXmitPowerDbmH: " << _measXmitPowerDbmH << endl;
  out << "  measXmitPowerDbmV: " << _measXmitPowerDbmV << endl;
  if (_estimatedNoiseDbmHc > -9990) {
    out << "  estimatedNoiseDbmHc: " << _estimatedNoiseDbmHc << endl;
  }
  if (_estimatedNoiseDbmVc > -9990) {
    out << "  estimatedNoiseDbmVc: " << _estimatedNoiseDbmVc << endl;
  }
  if (_estimatedNoiseDbmHx > -9990) {
    out << "  estimatedNoiseDbmHx: " << _estimatedNoiseDbmHx << endl;
  }
  if (_estimatedNoiseDbmVx > -9990) {
    out << "  estimatedNoiseDbmVx: " << _estimatedNoiseDbmVx << endl;
  }
  out << "  eventFlagsSet: "
      << string(_eventFlagsSet?"Y":"N") << endl;
  if (_eventFlagsSet) {
    out << "  startOfSweepFlag: "
        << string(_startOfSweepFlag?"Y":"N") << endl;
    out << "  endOfSweepFlag: "
        << string(_endOfSweepFlag?"Y":"N") << endl;
    out << "  startOfVolumeFlag: "
        << string(_startOfVolumeFlag?"Y":"N") << endl;
    out << "  endOfVolumeFlag: "
        << string(_endOfVolumeFlag?"Y":"N") << endl;
  }
  // if (_utilityFlag) {
  //   out << "  utilityFlag: Y"<< endl;
  // }
  if (_isLongRange) {
    out << "  isLongRange: Y" << endl;
  }
  if (_georefApplied) {
    out << "  georefApplied: Y" << endl;
  }
  out << "  nGates: " << _nGates << endl;
  RadxRangeGeom::print(out);
  out << "  nFields: " << _fields.size() << endl;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    out << "    " << _fields[ii]->getName() 
        << ":" << _fields[ii]->getUnits() << endl;
  }
  out << "===========================================" << endl;
  if (_georef) {
    _georef->print(out);
  }
  if (_cfactors) {
    _cfactors->print(out);
  }
  
}

/////////////////////////////////////////////////////////
// print one-line summary

void RadxRay::printSummary(ostream &out) const
  
{

  char line[1024];
  char trans[16];

  if (_antennaTransition) {
    sprintf(trans, "*");
  } else {
    sprintf(trans, " ");
  }

  sprintf(line,
          "Time el az mode prf ngates sweep# vol#: "
          "%s.%.3d %6.2f %6.2f %s %4d %4d %2d %5d %s",
          RadxTime::strm(_timeSecs).c_str(),
          (int) (_nanoSecs / 1000000),
          _elev,
          _az,
          Radx::sweepModeToShortStr(_sweepMode).c_str(),
          (int) (1.0 / _prtSec + 0.5),
          (int) _nGates,
          _sweepNum,
          _volNum,
          trans);
          
  out << line << endl;

}

/////////////////////////////////////////////////////////
// print with field meta data

void RadxRay::printWithFieldMeta(ostream &out) const
  
{
  
  print(out);
  out << "=========== RadxRay Fields ================" << endl;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->print(out);
  }
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////
// print with field data

void RadxRay::printWithFieldData(ostream &out) const
  
{
  
  print(out);
  out << "=========== RadxRay Fields ================" << endl;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->printWithData(out);
  }
  out << "===========================================" << endl;

}

/////////////////////////////////////////////////////////
// print the field name map

void RadxRay::printFieldNameMap(ostream &out) const
  
{
  
  out << "======== RadxRay FieldNameMap ===========" << endl;
  for (FieldNameMapConstIt ii = _fieldNameMap.begin();
       ii != _fieldNameMap.end(); ii++) {
    out << "  name, index: " << ii->first << ", " << ii->second << endl;
  }
  out << "=========================================" << endl;

}

/////////////////////////////////////////////////////////
// set sweep and volume event flags

void RadxRay::clearEventFlags()

{
  _eventFlagsSet = false;
  _startOfSweepFlag = false;
  _endOfSweepFlag = false;
  _startOfVolumeFlag = false;
  _endOfVolumeFlag = false;
}

void RadxRay::setStartOfSweepFlag(bool state)
{
  _startOfSweepFlag = state;
  if (state) {
    _eventFlagsSet = true;
  }
}

void RadxRay::setEndOfSweepFlag(bool state) 
{
  _endOfSweepFlag = state; 
  if (state) {
    _eventFlagsSet = true;
  }
}

void RadxRay::setStartOfVolumeFlag(bool state) 
{ 
  _startOfVolumeFlag = state; 
  if (state) {
    _eventFlagsSet = true;
  }
}

void RadxRay::setEndOfVolumeFlag(bool state) 
{
  _endOfVolumeFlag = state; 
  if (state) {
    _eventFlagsSet = true;
  }
}

/////////////////////////////////////////////////////////
// set utility flag

void RadxRay::setUtilityFlag(bool state) 
{
  _utilityFlag = state; 
}

/////////////////////////////////////////////////////////
/// compute elevation and azimuth from the georeference
/// data and correction factors.
///
/// If the georeference is null, no action is taken.
/// If the correction factors is null, no corrections are
/// applied, and setGeorefApplied() is called.
///
/// If force is true, the georefs are always applied.
/// If force is false, the georefs are applied only if
/// they have not been applied previously,
/// i.e. if _georefApplied = false.

void RadxRay::applyGeoref(Radx::PrimaryAxis_t axis, bool force /* = true */)

{

  // cannot apply if _georef is NULL

  if (!_georef) {
    return;
  }

  // do not apply if previously applied and force is false

  if (_georefApplied && !force) {
    return;
  }

  // get georeference values

  double rollRad = _georef->getRoll() * Radx::DegToRad;
  double pitchRad = _georef->getPitch() * Radx::DegToRad;
  double headingRad = _georef->getHeading() * Radx::DegToRad;
  double tiltRad = _georef->getTilt() * Radx::DegToRad;
  double rotRad = _georef->getRotation() * Radx::DegToRad;

  // apply corrections if appropriate

  if (_cfactors) {
    rollRad += _cfactors->getRollCorr() * Radx::DegToRad;
    pitchRad += _cfactors->getPitchCorr() * Radx::DegToRad;
    headingRad += _cfactors->getHeadingCorr() * Radx::DegToRad;
    tiltRad += _cfactors->getTiltCorr() * Radx::DegToRad;
    rotRad += _cfactors->getRotationCorr() * Radx::DegToRad;
  }

  // compute m matrix

  double cosR, sinR;
  double cosP, sinP;
  double cosH, sinH;
  double cosTilt, sinTilt;
  double cosRot, sinRot;

  Radx::sincos(rollRad, sinR, cosR);
  Radx::sincos(pitchRad, sinP, cosP);
  Radx::sincos(headingRad, sinH, cosH);
  Radx::sincos(tiltRad, sinTilt, cosTilt);
  Radx::sincos(rotRad, sinRot, cosRot);

  double m11 = cosH * cosR + sinH * sinP * sinR;
  double m12 = sinH * cosP;
  double m13 = cosH * sinR - sinH * sinP * cosR;

  double m21 = -sinH * cosR + cosH * sinP * sinR;
  double m22 = cosH * cosP;
  double m23 = -sinH * sinR - cosH * sinP * cosR;
  
  double m31 = -cosP * sinR;
  double m32 = sinP;
  double m33 = cosP * cosR;

  // compute x, y, z for a range of unity

  double xx, yy, zz;

  if (axis == Radx::PRIMARY_AXIS_Z) {

    xx = m11 * sinRot * cosTilt + m12 * cosRot * cosTilt + m13 * sinTilt;
    yy = m21 * sinRot * cosTilt + m22 * cosRot * cosTilt + m23 * sinTilt;
    zz = m31 * sinRot * cosTilt + m32 * cosRot * cosTilt + m33 * sinTilt;

  } else if (axis == Radx::PRIMARY_AXIS_Y) {

    xx = m11 * cosRot * cosTilt + m12 * sinTilt + m13 * sinRot * cosTilt;
    yy = m21 * cosRot * cosTilt + m22 * sinTilt + m23 * sinRot * cosTilt;
    zz = m31 * cosRot * cosTilt + m32 * sinTilt + m33 * sinRot * cosTilt;

  } else if (axis == Radx::PRIMARY_AXIS_Y_PRIME) {

    xx = m11 * sinRot * cosTilt + m12 * sinTilt + m13 * cosRot * cosTilt;
    yy = m21 * sinRot * cosTilt + m22 * sinTilt + m23 * cosRot * cosTilt;
    zz = m31 * sinRot * cosTilt + m32 * sinTilt + m33 * cosRot * cosTilt;

  } else if (axis == Radx::PRIMARY_AXIS_X) {

    xx = m11 * sinTilt + m12 * sinRot * cosTilt + m13 * cosRot * cosTilt;
    yy = m21 * sinTilt + m22 * sinRot * cosTilt + m23 * cosRot * cosTilt;
    zz = m31 * sinTilt + m32 * sinRot * cosTilt + m33 * cosRot * cosTilt;

  } else {

    // assume Z

    xx = m11 * sinRot * cosTilt + m12 * cosRot * cosTilt + m13 * sinTilt;
    yy = m21 * sinRot * cosTilt + m22 * cosRot * cosTilt + m23 * sinTilt;
    zz = m31 * sinRot * cosTilt + m32 * cosRot * cosTilt + m33 * sinTilt;

  }

  // compute earth-referenced azimuth and elevation
  
  double azimuthDeg = atan2(xx, yy) * Radx::RadToDeg;
  if (azimuthDeg < 0) {
    azimuthDeg += 360.0;
  }
  double elevationDeg = asin(zz) * Radx::RadToDeg;

#ifdef DEBUG_PRINT
  cerr << "====>> roll, pitch, hdg, tilt, rot: "
       << _georef->getRoll() << ", "
       << _georef->getPitch() << ", "
       << _georef->getHeading() << ", "
       << _georef->getTilt() << ", "
       << _georef->getRotation() << endl;

  cerr << "====>> xx, yy, zz, az, el: "
       << xx << ", "
       << yy << ", "
       << zz << ", "
       << azimuthDeg << ", "
       << elevationDeg << endl;
#endif

  _az = azimuthDeg;
  _elev = elevationDeg;

  _georefApplied = true;

}

//////////////////////////////////////////////////////////
/// set the flag to indicate that georef has been applied

void RadxRay::setGeorefApplied(bool state)

{
  _georefApplied = state;
}

//////////////////////////////////////////////////////////
/// count the number of rays in which each georef element
/// is not missing

void RadxRay::incrementGeorefNotMissingCount(RadxGeoref &count)

{
  if (_georef != NULL) {
    _georef->incrementIfNotMissing(count);
  }
}

/////////////////////////////////////////////////////////////////////////
// Memory management.
// This class optionally uses the notion of clients to decide when it
// should be deleted.
// If removeClient() returns 0, the object should be deleted.
// These functions are protected by a mutex for multi-threaded ops

int RadxRay::addClient() const
  
{
  pthread_mutex_lock(&_nClientsMutex);
  _nClients++;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

int RadxRay::removeClient() const

{
  pthread_mutex_lock(&_nClientsMutex);
  if (_nClients > 0) {
    _nClients--;
  }
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

int RadxRay::removeAllClients() const

{
  pthread_mutex_lock(&_nClientsMutex);
  _nClients = 0;
  pthread_mutex_unlock(&_nClientsMutex);
  return _nClients;
}

void RadxRay::deleteIfUnused(const RadxRay *ray)
  
{
  if (ray->removeClient() == 0) {
    delete ray;
  }
}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxRay::serialize(RadxMsg &msg)
  
{

  // init

  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxRayMsg);

  // add metadata strings as xml part
  // include null at string end
  
  string xml;
  _loadMetaStringsToXml(xml);
  msg.addPart(_metaStringsPartId, xml.c_str(), xml.size() + 1);

  // add metadata numbers

  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

  // add georef part if needed

  if (_georef != NULL) {
    RadxMsg georefMsg;
    _georef->serialize(georefMsg);
    msg.addPart(_georefPartId,
                georefMsg.assembledMsg(), 
                georefMsg.lengthAssembled());
  }

  // add correction factors part if needed
  
  if (_cfactors != NULL) {
    RadxMsg cfactorsMsg;
    _cfactors->serialize(cfactorsMsg);
    msg.addPart(_cfactorsPartId,
                cfactorsMsg.assembledMsg(), 
                cfactorsMsg.lengthAssembled());
  }

  // add field data

  for (size_t ii = 0; ii < _fields.size(); ii++) {

    // get field

    RadxField *field = _fields[ii];

    // serialize

    RadxMsg fieldMsg(RadxMsg::RadxFieldMsg);
    field->serialize(fieldMsg);
    fieldMsg.assemble();
    msg.addPart(_fieldPartId,
                fieldMsg.assembledMsg(), 
                fieldMsg.lengthAssembled());

  } // ifield

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxRay::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxRayMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata strings

  const RadxMsg::Part *metaStringPart = msg.getPartByType(_metaStringsPartId);
  if (metaStringPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::deserialize" << endl;
    cerr << "  No metadata string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaStringsFromXml((char *) metaStringPart->getBuf(),
                             metaStringPart->getLength())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::deserialize" << endl;
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
    cerr << "ERROR - RadxRay::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get georefs if available
  
  const RadxMsg::Part *georefPart = msg.getPartByType(_georefPartId);
  if (georefPart != NULL) {
    RadxMsg georefMsg;
    georefMsg.disassemble(georefPart->getBuf(), georefPart->getLength());
    if (_georef) {
      delete _georef;
    }
    _georef = new RadxGeoref;
    if (_georef->deserialize(georefMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Cannot dserialize georef" << endl;
      georefMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete _georef;
      _georef = NULL;
      return -1;
    }
  }
  
  // get cfactors if available
  
  const RadxMsg::Part *cfactorsPart = msg.getPartByType(_cfactorsPartId);
  if (cfactorsPart != NULL) {
    RadxMsg cfactorsMsg;
    cfactorsMsg.disassemble(cfactorsPart->getBuf(), cfactorsPart->getLength());
    if (_cfactors) {
      delete _cfactors;
    }
    _cfactors = new RadxCfactors;
    if (_cfactors->deserialize(cfactorsMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Cannot dserialize cfactors" << endl;
      cfactorsMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete _cfactors;
      _cfactors = NULL;
      return -1;
    }
  }
  
  // add fields

  size_t nFields = msg.partExists(_fieldPartId);
  for (size_t ifield = 0; ifield < nFields; ifield++) {
    
    // get field part

    const RadxMsg::Part *fieldPart =
      msg.getPartByType(_fieldPartId, ifield);

    // create a message from the field part

    RadxMsg fieldMsg;
    fieldMsg.disassemble(fieldPart->getBuf(), fieldPart->getLength());
    
    // create a field, dserialize from the message
    
    RadxField *field = new RadxField;
    if (field->deserialize(fieldMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Adding field num: " << ifield << endl;
      fieldMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete field;
      return -1;
    }

    // add the field

    addField(field);

  } // ifield

  return 0;

}

/////////////////////////////////////////////////////////
// convert string metadata to XML

void RadxRay::_loadMetaStringsToXml(string &xml, int level /* = 0 */)  const
  
{
  xml.clear();
  xml += RadxXml::writeStartTag("RadxRay", level);
  xml += RadxXml::writeString("scanName", level + 1, _scanName);
  xml += RadxXml::writeEndTag("RadxRay", level);
}

/////////////////////////////////////////////////////////
// set metadata strings from XML
// returns 0 on success, -1 on failure

int RadxRay::_setMetaStringsFromXml(const char *xml,
                                    size_t bufLen)

{

  // check for NULL
  
  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::_setMetaStringsFromXml" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  string xmlStr(xml);
  string contents;

  if (RadxXml::readString(xmlStr, "RadxRay", contents)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::_setMetaStringsFromXml" << endl;
    cerr << "  XML not delimited by 'RadxRay' tags" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  if (RadxXml::readString(contents, "scanName", _scanName)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::_setMetaStringsFromXml" << endl;
    cerr << "  Cannot find 'scanName' tag" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxRay::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set 64 bit values

  _metaNumbers.startRangeKm = _startRangeKm;
  _metaNumbers.gateSpacingKm = _gateSpacingKm;
  _metaNumbers.timeSecs = _timeSecs;
  _metaNumbers.nanoSecs = _nanoSecs;
  _metaNumbers.az = _az;
  _metaNumbers.elev = _elev;
  _metaNumbers.fixedAngle = _fixedAngle;
  _metaNumbers.targetScanRate = _targetScanRate;
  _metaNumbers.trueScanRate = _trueScanRate;
  _metaNumbers.angleRes = _angleRes;
  _metaNumbers.pulseWidthUsec = _pulseWidthUsec;
  _metaNumbers.prtSec = _prtSec;
  _metaNumbers.prtRatio = _prtRatio;
  _metaNumbers.nyquistMps = _nyquistMps;
  _metaNumbers.unambigRangeKm = _unambigRangeKm;
  _metaNumbers.measXmitPowerDbmH = _measXmitPowerDbmH;
  _metaNumbers.measXmitPowerDbmV = _measXmitPowerDbmV;
  _metaNumbers.estimatedNoiseDbmHc = _estimatedNoiseDbmHc;
  _metaNumbers.estimatedNoiseDbmVc = _estimatedNoiseDbmVc;
  _metaNumbers.estimatedNoiseDbmHx = _estimatedNoiseDbmHx;
  _metaNumbers.estimatedNoiseDbmVx = _estimatedNoiseDbmVx;
  _metaNumbers.nGates = _nGates;

  // set 32-bit values

  _metaNumbers.rayNum = _rayNum;
  _metaNumbers.volNum = _volNum;
  _metaNumbers.sweepNum = _sweepNum;
  _metaNumbers.calibIndex = _calibIndex;
  _metaNumbers.sweepMode = _sweepMode;
  _metaNumbers.polarizationMode = _polarizationMode;
  _metaNumbers.prtMode = _prtMode;
  _metaNumbers.followMode = _followMode;
  _metaNumbers.isIndexed = _isIndexed;
  _metaNumbers.antennaTransition = _antennaTransition;
  _metaNumbers.nSamples = _nSamples;
  _metaNumbers.eventFlagsSet = _eventFlagsSet;
  _metaNumbers.startOfSweepFlag = _startOfSweepFlag;
  _metaNumbers.endOfSweepFlag = _endOfSweepFlag;
  _metaNumbers.startOfVolumeFlag = _startOfVolumeFlag;
  _metaNumbers.endOfVolumeFlag = _endOfVolumeFlag;
  _metaNumbers.utilityFlag = _utilityFlag;
  _metaNumbers.isLongRange = _isLongRange;
  _metaNumbers.georefApplied = _georefApplied;
  
}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxRay::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                    size_t bufLen,
                                    bool swap)
  
{

  // check size

  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxRay::_setMetaNumbersFromMsg" << endl;
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

  // set 64 bit values

  _timeSecs = _metaNumbers.timeSecs;
  _nanoSecs = _metaNumbers.nanoSecs;
  _az = _metaNumbers.az;
  _elev = _metaNumbers.elev;
  _fixedAngle = _metaNumbers.fixedAngle;
  _targetScanRate = _metaNumbers.targetScanRate;
  _trueScanRate = _metaNumbers.trueScanRate;
  _angleRes = _metaNumbers.angleRes;
  _pulseWidthUsec = _metaNumbers.pulseWidthUsec;
  _prtSec = _metaNumbers.prtSec;
  _prtRatio = _metaNumbers.prtRatio;
  _nyquistMps = _metaNumbers.nyquistMps;
  _unambigRangeKm = _metaNumbers.unambigRangeKm;
  _measXmitPowerDbmH = _metaNumbers.measXmitPowerDbmH;
  _measXmitPowerDbmV = _metaNumbers.measXmitPowerDbmV;
  _estimatedNoiseDbmHc = _metaNumbers.estimatedNoiseDbmHc;
  _estimatedNoiseDbmVc = _metaNumbers.estimatedNoiseDbmVc;
  _estimatedNoiseDbmHx = _metaNumbers.estimatedNoiseDbmHx;
  _estimatedNoiseDbmVx = _metaNumbers.estimatedNoiseDbmVx;
  _nGates = _metaNumbers.nGates;

  // set 32-bit values

  _rayNum = _metaNumbers.rayNum;
  _volNum = _metaNumbers.volNum;
  _sweepNum = _metaNumbers.sweepNum;
  _calibIndex = _metaNumbers.calibIndex;
  _sweepMode = (Radx::SweepMode_t) _metaNumbers.sweepMode;
  _polarizationMode = (Radx::PolarizationMode_t) _metaNumbers.polarizationMode;
  _prtMode = (Radx::PrtMode_t) _metaNumbers.prtMode;
  _followMode = (Radx::FollowMode_t) _metaNumbers.followMode;
  _isIndexed = _metaNumbers.isIndexed;
  _antennaTransition = _metaNumbers.antennaTransition;
  _nSamples = _metaNumbers.nSamples;
  _eventFlagsSet = _metaNumbers.eventFlagsSet;
  _startOfSweepFlag = _metaNumbers.startOfSweepFlag;
  _endOfSweepFlag = _metaNumbers.endOfSweepFlag;
  _startOfVolumeFlag = _metaNumbers.startOfVolumeFlag;
  _endOfVolumeFlag = _metaNumbers.endOfVolumeFlag;
  _utilityFlag = _metaNumbers.utilityFlag;
  _isLongRange = _metaNumbers.isLongRange;
  _georefApplied = _metaNumbers.georefApplied;
  
  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxRay::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.startRangeKm, 32 * sizeof(Radx::si64));
  ByteOrder::swap32(&meta.rayNum, 32 * sizeof(Radx::si32));
}
