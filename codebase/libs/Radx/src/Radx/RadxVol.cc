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
// RadxVol.cc
//
// RadxVol object
//
// NetCDF file data for radar radial data in CF-compliant file
//
// Mike Dixon, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Dec 2009
//
///////////////////////////////////////////////////////////////

#include <Radx/RadxVol.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxCfactors.hh>
#include <Radx/RadxRcalib.hh>
#include <Radx/RadxAngleHist.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/PseudoRhi.hh>
#include <Radx/RadxXml.hh>
#include <Radx/ByteOrder.hh>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <sys/stat.h>
using namespace std;

const double RadxVol::_searchAngleRes = 360.0 / _searchAngleN;

//////////////
// Constructor

RadxVol::RadxVol()
  
{
  _init();
}

/////////////////////////////
// Copy constructor
//

RadxVol::RadxVol(const RadxVol &rhs)
     
{
  _init();
  _copy(rhs);
}

////////////////////////////////////
// Copy constructor, one sweep only

RadxVol::RadxVol(const RadxVol &rhs, int sweepNum)
     
{
  _init();
  copy(rhs, sweepNum);
}

/////////////
// destructor

RadxVol::~RadxVol()

{

  clear();

}

/////////////////////////////
// Assignment
//

RadxVol &RadxVol::operator=(const RadxVol &rhs)
  

{
  return _copy(rhs);
}

// set radar and lidar parameters

void RadxVol::setRadarBeamWidthDegH(double val) { 
  _platform.setRadarBeamWidthDegH(val);
}
void RadxVol::setRadarBeamWidthDegV(double val) {
  _platform.setRadarBeamWidthDegV(val);
}
void RadxVol::setRadarAntennaGainDbH(double val) {
  _platform.setRadarAntennaGainDbH(val);
}
void RadxVol::setRadarAntennaGainDbV(double val) {
  _platform.setRadarAntennaGainDbV(val);
}
void RadxVol::setRadarReceiverBandwidthMhz(double val) {
  _platform.setRadarReceiverBandwidthMhz(val);
}

void RadxVol::setIsLidar(bool val) {
  _platform.setIsLidar(val);
}

void RadxVol::setLidarConstant(double val) {
  _platform.setLidarConstant(val);
}
void RadxVol::setLidarPulseEnergyJ(double val) {
  _platform.setLidarPulseEnergyJ(val);
}
void RadxVol::setLidarPeakPowerW(double val) {
  _platform.setLidarPeakPowerW(val);
}
void RadxVol::setLidarApertureDiamCm(double val) {
  _platform.setLidarApertureDiamCm(val);
}
void RadxVol::setLidarApertureEfficiency(double val) {
  _platform.setLidarApertureEfficiency(val);
}
void RadxVol::setLidarFieldOfViewMrad(double val) {
  _platform.setLidarFieldOfViewMrad(val);
}
void RadxVol::setLidarBeamDivergenceMrad(double val) {
  _platform.setLidarBeamDivergenceMrad(val);
}

/////////////////////////////////////////////////////////
// initialize data members

void RadxVol::_init()
  
{

  _debug = false;
  _cfactors = NULL;
  _searchRays.resize(_searchAngleN);

  clear();
  
  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

}

//////////////////////////////////////////////////
// copy - used by copy constructor and operator =
//

RadxVol &RadxVol::_copy(const RadxVol &rhs)
  
{
  
  if (&rhs == this) {
    return *this;
  }

  // copy meta data

  copyMeta(rhs);

  // copy the range geometry and data packing

  copyRangeGeom(rhs);
  copyPacking(rhs);
  
  // sweeps
  
  clearSweeps();
  for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
    RadxSweep *sweep = new RadxSweep(*rhs._sweeps[ii]);
    _sweeps.push_back(sweep);
  }

  // rays

  clearRays();
  for (size_t ii = 0; ii < rhs._rays.size(); ii++) {
    RadxRay *ray = new RadxRay(*rhs._rays[ii]);
    addRay(ray);
  }

  // fields

  clearFields();
  for (size_t ii = 0; ii < rhs._fields.size(); ii++) {
    RadxField *field = new RadxField(*rhs._fields[ii]);
    _fields.push_back(field);
  }

  return *this;
  
}

//////////////////////////////////////////////////
// copy following meta data only:
//   main members, calibs, sweeps as in file
//
// does not copy sweeps, rays and fields

void RadxVol::copyMeta(const RadxVol &rhs)
  
{
  
  if (&rhs == this) {
    return;
  }
  
  // copy the base class metadata

  _debug = rhs._debug;

  _version = rhs._version;
  _title = rhs._title;
  _institution = rhs._institution;
  _references = rhs._references;
  _source = rhs._source;
  _history = rhs._history;
  _comment = rhs._comment;
  _statusXml = rhs._statusXml;
  
  _volNum = rhs._volNum;

  _scanName = rhs._scanName;
  _scanId = rhs._scanId;

  _platform = rhs._platform;

  _sweepModeFromAnglesChecked = rhs._sweepModeFromAnglesChecked;
  _predomSweepModeFromAngles = rhs._predomSweepModeFromAngles;

  _startTimeSecs = rhs._startTimeSecs;
  _endTimeSecs = rhs._endTimeSecs;
  _startNanoSecs = rhs._startNanoSecs;
  _endNanoSecs = rhs._endNanoSecs;

  _rayTimesIncrease = rhs._rayTimesIncrease;
  _transitionFlags = rhs._transitionFlags;
  _pathInUse = rhs._pathInUse;

  // sweeps as in orig file

  clearSweepsAsInFile();
  for (size_t ii = 0; ii < rhs._sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep = new RadxSweep(*rhs._sweepsAsInFile[ii]);
    _sweepsAsInFile.push_back(sweep);
  }

  // radar calibs

  clearRcalibs();
  for (size_t ii = 0; ii < rhs._rcalibs.size(); ii++) {
    RadxRcalib *calib = new RadxRcalib(*rhs._rcalibs[ii]);
    _rcalibs.push_back(calib);
  }

  // correction factors

  clearCfactors();
  if (rhs._cfactors != NULL) {
    _cfactors = new RadxCfactors(*rhs._cfactors);
  } else {
    _cfactors = NULL;
  }

  // ray search

  _searchMaxWidth = rhs._searchMaxWidth;
  _searchRays = rhs._searchRays;

}

//////////////////////////////////////////////////
// copy one sweep, clearing other sweeps as needed
//

void RadxVol::copy(const RadxVol &rhs, int sweepNum)
  
{
  
  if (&rhs == this) {
    return;
  }

  // copy meta data
  
  copyMeta(rhs);
  
  // copy the range geometry and data packing

  copyRangeGeom(rhs);
  copyPacking(rhs);
  
  // find sweep index corresponding to the sweep number
  
  int sweepIndex = -1;
  for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
    if (rhs._sweeps[ii]->getSweepNumber() == sweepNum) {
      sweepIndex = ii;
      break;
    }
  }

  // if we did not get an exact match in sweep number, choose
  // the closest one to that requested
  
  if (sweepIndex < 0) {
    int minDiff = 9999;
    sweepIndex = 0;
    for (size_t ii = 0; ii < rhs._sweeps.size(); ii++) {
      int num = rhs._sweeps[ii]->getSweepNumber();
      int diff = abs(num - sweepNum);
      if (diff < minDiff) {
        minDiff = diff;
        sweepIndex = ii;
      }
    }
  }

  // get min and max ray indexes

  int minRayIndex = rhs._sweeps[sweepIndex]->getStartRayIndex();
  int maxRayIndex = rhs._sweeps[sweepIndex]->getEndRayIndex();

  // clear the packing info

  clearPacking();

  // add the rays to this volume

  const vector<RadxRay *> &rhsRays = rhs.getRays();
  for (int ii = minRayIndex; ii <= maxRayIndex; ii++) {
    RadxRay *ray = new RadxRay(*rhsRays[ii]);
    addRay(ray);
  }

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

  // check for indexed rays, set info on rays
  
  checkForIndexedRays();

}

/////////////////////////////////////////////////////////
// clear the data in the object

void RadxVol::clear()
  
{

  _sweepModeFromAnglesChecked = false;
  _predomSweepModeFromAngles = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;

  _version.clear();
  _title.clear();
  _institution.clear();
  _references.clear();
  _source.clear();
  _history.clear();
  _comment.clear();
  _statusXml.clear();
  _userGlobAttr.clear();

  _volNum = Radx::missingMetaInt;
  
  _scanName.clear();
  _scanId = 0;

  _platform.clear();

  _startTimeSecs = 0;
  _endTimeSecs = 0;
  _startNanoSecs = 0;
  _endNanoSecs = 0;

  _rayTimesIncrease = true;

  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

  clearRays();
  clearSweeps();
  clearSweepsAsInFile();
  clearRcalibs();
  clearFields();
  clearCfactors();
  clearFrequency();
  clearPacking();
  clearPseudoRhis();

}

///////////////
// set debug

void RadxVol::setDebug(bool val) {
  _debug = val;
}

////////////////////////////////////////////////////////////////
// add a ray

void RadxVol::addRay(RadxRay *ray)
  
{

  if (!ray->getRangeGeomSet()) {
    cerr << "WARNING - Range geom has not been set on ray" << endl;
  }
  _rays.push_back(ray);
  ray->addClient();  // take responsibility for ray memory
  addToPacking(ray->getNGates());
  copyRangeGeom(*ray);

}

//////////////////////////////////////////////////////////////
// add a sweep object
// Volume takes responsibility for freeing the sweep object.

void RadxVol::addSweep(RadxSweep *sweep)
  
{
  _sweeps.push_back(sweep);
}
 
//////////////////////////////////////////////////////////////
/// Add sweep to 'as-in-file' vector
/// These are the sweeps as originally in the file before selected
/// sweeps were requested.
/// A copy of the object is made, and is managed by this object.

void RadxVol::addSweepAsInFile(RadxSweep *sweep)
  
{
  RadxSweep *sweepf = new RadxSweep(*sweep);
  _sweepsAsInFile.push_back(sweepf);
}
 
//////////////////////////////////////////////////////////////
// add a calibration object
// Volume takes responsibility for freeing the cal object

void RadxVol::addCalib(RadxRcalib *calib)
  
{

  if (calib->getCalibTime() <= 0) {
    calib->setCalibTime(getStartTimeSecs());
  }

  if (calib->getRadarName().size() == 0) {
    calib->setRadarName(getInstrumentName());
  }
  if (calib->getWavelengthCm() < 0) {
    calib->setWavelengthCm(getWavelengthCm());
  }
  if (calib->getBeamWidthDegH() < 0) {
    calib->setBeamWidthDegH(getRadarBeamWidthDegH());
  }
  if (calib->getBeamWidthDegV() < 0) {
    calib->setBeamWidthDegV(getRadarBeamWidthDegV());
  }
  if (calib->getAntennaGainDbH() < 0) {
    calib->setAntennaGainDbH(getRadarAntennaGainDbH());
  }
  if (calib->getAntennaGainDbV() < 0) {
    calib->setAntennaGainDbV(getRadarAntennaGainDbV());
  }
  
  _rcalibs.push_back(calib);
}

//////////////////////////////////////////////////////////////
// add a field object
// Volume takes responsibility for freeing the field object

void RadxVol::addField(RadxField *field)
  
{
  _fields.push_back(field);
  copyRangeGeom(*field);
}

////////////////////////////////////////////////////////////////
/// compute the max number of gates
/// Also determines if number of gates vary by ray.
/// After this call, use the following to get the results:
///   size_t getMaxNGates() - see RadxPacking.
///   bool nGatesVary() - see RadxPacking

void RadxVol::computeMaxNGates() const
  
{
  _maxNGates = 0;
  _nGatesVary = false;
  size_t prevNGates = 0;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay &ray = *_rays[iray];
    size_t rayNGates = ray.getNGates();
    if (rayNGates > _maxNGates) {
      _maxNGates = rayNGates;
    }
    if (iray > 0) {
      if (rayNGates != prevNGates) {
        _nGatesVary = true;
      }
    }
    prevNGates = rayNGates;
  }
}

////////////////////////////////////////////////////////////////
/// compute the max number of gates in a sweep index.
/// Return max number of gates.

int RadxVol::computeMaxNGatesSweep(const RadxSweep *sweep) const
  
{

  int maxNGates = 0;

  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();

  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    RadxRay &ray = *_rays[iray];
    int rayNGates = ray.getNGates();
    if (rayNGates > maxNGates) {
      maxNGates = rayNGates;
    }
  } // iray

  return maxNGates;

}

//////////////////////////////////////////////////////////////
// set the correction factors

void RadxVol::setCfactors(RadxCfactors &cfac)
  
{
  clearCfactors();
  _cfactors = new RadxCfactors(cfac);
}

/////////////////////////////////////////
// set the target scan rate

void RadxVol::setTargetScanRateDegPerSec(double rate)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _sweeps[ii]->setTargetScanRateDegPerSec(rate);
  }
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setTargetScanRateDegPerSec(rate);
  }
}
  
void RadxVol::setTargetScanRateDegPerSec(int sweepNum, double rate)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      RadxSweep *sweep = _sweeps[ii];
      sweep->setTargetScanRateDegPerSec(rate);
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        _rays[iray]->setTargetScanRateDegPerSec(rate);
      }
      return;
    }
  }
}

//////////////////////////////////////////////////////////////
/// Set the number of gates on the volume.
///
/// If more gates are needed, extend the field data out to a set number of
/// gates. The data for extra gates are set to missing values.
///
/// If fewer gates are needed, the data is truncated.

void RadxVol::setNGates(size_t nGates)

{
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setNGates(nGates);
  }
}

//////////////////////////////////////////////////////////////
/// Set the number of gates on a sweep.
///
/// If more gates are needed, extend the field data out to a set number of
/// gates. The data for extra gates are set to missing values.
///
/// If fewer gates are needed, the data is truncated.

void RadxVol::setNGatesSweep(RadxSweep *sweep,
                             size_t nGates)

{
  for (size_t iray = sweep->getStartRayIndex();
       iray <= sweep->getEndRayIndex(); iray++) {
    _rays[iray]->setNGates(nGates);
  }
}

//////////////////////////////////////////////////////////////
/// Set to constant number of gates per ray.
/// 
/// First we determine the max number of gates, and also check
/// for a variable number of gates. If the number of gates does
/// vary, the shorter rays are padded out with missing data.

void RadxVol::setNGatesConstant()

{
  
  computeMaxNGates();
  if (_nGatesVary) {
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      _rays[iray]->setNGates(_maxNGates);
    }
  }
  _nGatesVary = false;
  
}
  
/////////////////////////////////////////////////////////////////////////
/// Set the maximum range.
/// Removes excess gates as needed.
/// Does nothing if the current max range is less than that specified.

void RadxVol::setMaxRangeKm(double maxRangeKm)

{
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setMaxRangeKm(maxRangeKm);
  }
  setPackingFromRays();
}
  
//////////////////////////////////////////////////////////////
/// Set the packing from the rays

void RadxVol::setPackingFromRays()

{
  clearPacking();
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    addToPacking(_rays[iray]->getNGates());
  }
}
  
//////////////////////////////////////////////////////////////////
/// get the predominant sweep mode
/// i.e. the most common sweep mode in the volume
/// this makes use of the scan flags set on the rays
/// if the scan flags are not accurately set, use
/// getPredomSweepModeFromAngles() instead.

Radx::SweepMode_t RadxVol::getPredomSweepMode() const

{

  // set up a map for the various sweep modes

  map<Radx::SweepMode_t, int> modeMap;

  modeMap[Radx::SWEEP_MODE_NOT_SET] = 0;
  modeMap[Radx::SWEEP_MODE_CALIBRATION] = 0;
  modeMap[Radx::SWEEP_MODE_SECTOR] = 0;
  modeMap[Radx::SWEEP_MODE_COPLANE] = 0;
  modeMap[Radx::SWEEP_MODE_RHI] = 0;
  modeMap[Radx::SWEEP_MODE_VERTICAL_POINTING] = 0;
  modeMap[Radx::SWEEP_MODE_IDLE] = 0;
  modeMap[Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE] = 0;
  modeMap[Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE] = 0;
  modeMap[Radx::SWEEP_MODE_SUNSCAN] = 0;
  modeMap[Radx::SWEEP_MODE_POINTING] = 0;
  modeMap[Radx::SWEEP_MODE_FOLLOW_VEHICLE] = 0;
  modeMap[Radx::SWEEP_MODE_EL_SURV] = 0;
  modeMap[Radx::SWEEP_MODE_MANUAL_PPI] = 0;
  modeMap[Radx::SWEEP_MODE_MANUAL_RHI] = 0;
  modeMap[Radx::SWEEP_MODE_SUNSCAN_RHI] = 0;
  modeMap[Radx::SWEEP_MODE_DOPPLER_BEAM_SWINGING] = 0;
  modeMap[Radx::SWEEP_MODE_COMPLEX_TRAJECTORY] = 0;
  modeMap[Radx::SWEEP_MODE_ELECTRONIC_STEERING] = 0;

  // accumulate the number of rays for each mode
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    if (!ray.getAntennaTransition()) {
      Radx::SweepMode_t sweepMode = ray.getSweepMode();
      if (fabs(ray.getElevationDeg() - 90.0) < 2.5) {
	// elev between 87.5 and 92.5
	sweepMode = Radx::SWEEP_MODE_VERTICAL_POINTING;
      }
      modeMap[sweepMode]++;
    }
  }
  
  // now find the one with the max count

  Radx::SweepMode_t predomMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
  int maxCount = 0;
  map<Radx::SweepMode_t, int>::iterator ii;
  for (ii = modeMap.begin(); ii != modeMap.end(); ii++) {
    Radx::SweepMode_t mode = ii->first;
    int count = ii->second;
    if (count > maxCount) {
      predomMode = mode;
      maxCount = count;
    }
  }

  return predomMode;

}

//////////////////////////////////////////////////////////////////
/// get the predominant range geometry

void RadxVol::getPredomRayGeom(double &startRangeKm,
                               double &gateSpacingKm) const

{
  RayGeom predom = _getPredomGeom();
  startRangeKm = predom.startRange;
  gateSpacingKm = predom.gateSpacing;
}

//////////////////////////////////////////////////////////////////
/// get the finest range geometry

void RadxVol::getFinestRayGeom(double &startRangeKm,
                               double &gateSpacingKm) const

{
  RayGeom finest = _getFinestGeom();
  startRangeKm = finest.startRange;
  gateSpacingKm = finest.gateSpacing;
}

//////////////////////////////////////////////////////////////////
/// Get the list of unique field names, compiled by
/// searching through all rays.
///
/// The order of the field names found is preserved

vector<string> RadxVol::getUniqueFieldNameList() const

{
  
  vector<string> fieldNames;

  // find the set of fields names

  set<string> nameSet;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    for (size_t ifield = 0; ifield < ray.getNFields(); ifield++) {
      string name = ray.getField(ifield)->getName();
      pair<set<string>::const_iterator, bool> ret = nameSet.insert(name);
      if (ret.second == true) {
        // field name not previously in set, so add to vector
        fieldNames.push_back(name);
      }
    }
  }

  return fieldNames;

}

//////////////////////////////////////////////////
/// convert all fields to same data type
/// widening as required

void RadxVol::widenFieldsToUniformType()

{

  // check if fields are owned by rays instead of volume

  bool fieldsOwnedByRays = true;
  if (_fields.size() > 0) {
    fieldsOwnedByRays = false;
  } else {
    loadFieldsFromRays(true);
  }

  // search for the narrowest data type which works for all
  // fields
  
  int dataByteWidth = 1;
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    const RadxField &fld = *_fields[ii];
    if (fld.getByteWidth() > dataByteWidth) {
      dataByteWidth = fld.getByteWidth();
    }
  }
  
  if (dataByteWidth == 2) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16();
    }
  } else if (dataByteWidth == 4) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl32();
    }
  } else if (dataByteWidth == 8) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl64();
    }
  }

  // do we need to put back into ray ownership

  if (fieldsOwnedByRays) {
    loadRaysFromFields();
  } else {
    setRayFieldPointers();
  }
  
}

//////////////////////////////////////////////////
/// set all fields to same data type

void RadxVol::setFieldsToUniformType(Radx::DataType_t dataType)

{

  // check if fields are owned by rays instead of volume

  bool fieldsOwnedByRays = true;
  if (_fields.size() > 0) {
    fieldsOwnedByRays = false;
  } else {
    loadFieldsFromRays(true);
  }

  // convert to requested type

  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(dataType);
  }
  
  // do we need to put back into ray ownership

  if (fieldsOwnedByRays) {
    loadRaysFromFields();
  } else {
    setRayFieldPointers();
  }
  
}

//////////////////////////////////////////////////////////////////
/// Get a field from a ray, given the name.
/// Find the first available field on a suitable ray.
/// Returns field pointer on success, NULL on failure.

const RadxField *RadxVol::getFieldFromRay(const string &name) const

{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const vector<RadxField *> &fields = ray.getFields();
    for (size_t ifield = 0; ifield < fields.size(); ifield++) {
      if (fields[ifield]->getName() == name) {
        return fields[ifield];
      }
    }
  }

  return NULL;

}

//////////////////////////////////////////////////////////////////
/// Load contiguous fields on the volume, from fields in the rays.
///
/// The fields in the rays are then set to point to the contiguous
/// fields.
///
/// The memory for the fields is managed by the fields on the volume.
/// The fields on the rays point into these contiguous arrys.
///
/// nFieldsConstantPerRay:
///   only applicable if not all input fields are present in the
///   input data.
/// If true, each ray will have the same number of fields, even if in
/// the incoming data some rays do not have all of the fields. If
/// false, a field will only be included in a ray if that field
/// also exists in the incoming data.
///
/// See also: loadRaysFromFields()

void RadxVol::loadFieldsFromRays(bool nFieldsConstantPerRay /* = false */)
  
{

  // check we have data

  if (_rays.size() < 1) {
    return;
  }

  // has this already been done?

  if (_fields.size() > 0) {
    return;
  }

  // ensure all rays have local data
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setDataLocal();
  }

  // free any existing fields on the volume

  clearFields();
  
  // set vol number and times

  loadVolumeInfoFromRays();
  
  // get the set of unique fields names in this volume

  vector<string> fieldNames = getUniqueFieldNameList();

  // make copies of the fields, and add them to the volume

  for (size_t ii = 0; ii < fieldNames.size(); ii++) {
    RadxField *field = copyField(fieldNames[ii]);
    if (field != NULL) {
      addField(field);
    }
  } // ii
      
  // Free up field data in rays, point them to the contiguous fields.
  // This process also adjusts the scale and bias in the ray fields to
  // the same as in the global fields.

  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    RadxField &field = *_fields[ifield];
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      RadxField *rayField = ray.getField(field.getName());
      if (rayField != NULL) {
        size_t nGates;
        const void *data = field.getData(iray, nGates);
        rayField->setDataRemote(field, data, nGates);
      } else {
        if (nFieldsConstantPerRay) {
          // add any missing fields
          rayField = new RadxField(field.getName(), field.getUnits());
          rayField->copyMetaData(field);
          size_t nGates;
          const void *data = field.getData(iray, nGates);
          rayField->setDataRemote(field, data, nGates);
          ray.addField(rayField);
        }
      }
    }      
  } // ifield

  // load the sweep info from rays
  
  loadSweepInfoFromRays();

  // copy packing from first field to the volume

  if (_fields.size() > 0) {
    copyPacking(*_fields[0]);
  }

}

//////////////////////////////////////////////////////////////
/// Load up the ray fields from the contiguous fields in the volume.
/// This is the inverse of loadFieldsFromRays()

void RadxVol::loadRaysFromFields()
  
{

  if (_fields.size() < 1) {
    return;
  }

  // for fields on all rays, set them to manage their own data

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setDataLocal();
  } // iray
  
  // clear fields on the volume

  clearFields();

}

//////////////////////////////////////////////////////////////
/// Set field data pointers in the rays to point into the
/// main contiguous fields on the volume.
///
/// If the rays that have been added to the volume do not hold the
/// data themselves, call this method to set the pointers in the ray
/// fields so that they refer to the main fields on the volume.
  
void RadxVol::setRayFieldPointers()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay *ray = _rays[iray];
    ray->clearFields();
    ray->setRangeGeom(_startRangeKm, _gateSpacingKm);

    for (size_t ifield = 0; ifield < _fields.size(); ifield++) {

      const RadxField *field = _fields[ifield];
      const string &name = field->getName();
      const string &standardName = field->getStandardName();
      const string &longName = field->getLongName();
      const string &comment = field->getComment();
      const string &units = field->getUnits();
      const string &thresholdFieldName = field->getThresholdFieldName();
      double thresholdValue = field->getThresholdValue();
      int nGates = field->getRayNGates(iray);
      int index = field->getRayStartIndex(iray);
      
      switch (field->getDataType()) {
        case Radx::FL64: {
          const Radx::fl64 *dptr = (Radx::fl64 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingFl64(),
                                          dptr + index, false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setComment(comment);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::FL32: {
          const Radx::fl32 *dptr = (Radx::fl32 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingFl32(),
                                          dptr + index, false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setComment(comment);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI32: {
          const Radx::si32 *dptr = (Radx::si32 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi32(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setComment(comment);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI16: {
          const Radx::si16 *dptr = (Radx::si16 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi16(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setComment(comment);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        case Radx::SI08: {
          const Radx::si08 *dptr = (Radx::si08 *) field->getData();
          RadxField *rfld = ray->addField(name, units, nGates,
                                          field->getMissingSi08(),
                                          dptr + index,
                                          field->getScale(),
                                          field->getOffset(),
                                          false);
          rfld->setStandardName(standardName);
          rfld->setLongName(longName);
          rfld->setComment(comment);
          rfld->setThresholdFieldName(thresholdFieldName);
          rfld->setThresholdValue(thresholdValue);
          break;
        }
        default: {}
      } // switch

    } // ifield

  } // iray

}

//////////////////////////////////////////////////////////////////
/// Make a copy of the field with the specified name.
///
/// This forms a contiguous field from the ray data.
///
/// Returns a pointer to the field on success, NULL on failure.

RadxField *RadxVol::copyField(const string &fieldName) const
  
{

  // check we have data
  
  if (_rays.size() < 1) {
    return NULL;
  }
  
  // create the field
  // use the first available field in any ray as a template
  
  RadxField *copy = NULL;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const RadxField *rayField = ray.getField(fieldName);
    if (rayField != NULL) {
      // create new field using name, units and type
      copy = new RadxField(rayField->getName(), rayField->getUnits());
      copy->copyMetaData(*rayField);
      break;
    }
    if (copy != NULL) {
      break;
    }
  } // iray
  
  if (copy == NULL) {
    // no suitable field
    return NULL;
  }

  // check if the fields on the rays are uniform -
  // i.e. all have the same type, scale and offset
  
  bool fieldsAreUniform = true;
  Radx::DataType_t dataType = copy->getDataType();
  double scale = copy->getScale();
  double offset = copy->getOffset();
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    const RadxField *rayField = ray.getField(fieldName);
    if (rayField == NULL) {
      continue;
    }
    if (rayField->getDataType() != dataType) {
      // different field data types
      fieldsAreUniform = false;
      break;
    }
    if (dataType == Radx::FL32 || dataType == Radx::FL64) {
      // for float types don't worry about scale and bias
      continue;
    }
    if (fabs(rayField->getScale() - scale) > 1.0e-5) {
      // different scale
      fieldsAreUniform = false;
      break;
    }
    if (fabs(rayField->getOffset() - offset) > 1.0e-5) {
      // different offset
      fieldsAreUniform = false;
      break;
    }
  } // iray

  // load up data from the rays

  if (fieldsAreUniform) {

    // type, scale and offset constant

    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        if (dataType == Radx::FL64) {
          rcopy.setMissingFl64(copy->getMissingFl64());
          copy->addDataFl64(nGates, rcopy.getDataFl64());
        } else if (dataType == Radx::FL32) {
          rcopy.setMissingFl32(copy->getMissingFl32());
          copy->addDataFl32(nGates, rcopy.getDataFl32());
        } else if (dataType == Radx::SI32) {
          rcopy.setMissingSi32(copy->getMissingSi32());
          copy->addDataSi32(nGates, rcopy.getDataSi32());
        } else if (dataType == Radx::SI16) {
          rcopy.setMissingSi16(copy->getMissingSi16());
          copy->addDataSi16(nGates, rcopy.getDataSi16());
        } else if (dataType == Radx::SI08) {
          rcopy.setMissingSi08(copy->getMissingSi08());
          copy->addDataSi08(nGates, rcopy.getDataSi08());
        }
      }
    } // iray

    return copy;

  }

  // fields are not uniform and must be converted to a common type

  if (dataType == Radx::FL64) {
    
    // 64-bit floats
    
    copy->setTypeFl64(Radx::missingFl64);
    
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        rcopy.convertToFl64();
        copy->addDataFl64(nGates, rcopy.getDataFl64());
      }
    } // iray
    
    // convert to final type
    copy->convertToType(dataType);

  } else {
    
    // all others
    // convert to fl32 for now
    
    copy->setTypeFl32(Radx::missingFl32);
    
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxRay &ray = *_rays[iray];
      size_t nGates = ray.getNGates();
      RadxField *rfld = ray.getField(fieldName);
      if (rfld == NULL) {
        copy->addDataMissing(nGates);
      } else {
        RadxField rcopy(*rfld);
        rcopy.convertToFl32();
        copy->addDataFl32(nGates, rcopy.getDataFl32());
      }
    } // iray
    
    // convert to final type
    copy->convertToType(dataType);

  } // if (dataType == Radx::FL64)

  return copy;

}

/////////////////////////////////////////////////////////////////
/// Rename a field
/// returns 0 on success, -1 if field does not exist in any ray

int RadxVol::renameField(const string &oldName, const string &newName)
  
{

  int iret = -1;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay &ray = *_rays[iray];
    if (ray.renameField(oldName, newName) == 0) {
      iret = 0;
    }
  }

  return iret;
  
}

//////////////////////////////////////////////////////////////
// Load up modes from sweeps to rays
// This assumes the sweeps meta data is filled out, but the
// modes are missing from the rays.

void RadxVol::loadModesFromSweepsToRays()
  
{

  if (_rays.size() < 1) {
    return;
  }

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay &ray = *_rays[iray];
      ray.setSweepNumber(sweep->getSweepNumber());
      ray.setSweepMode(sweep->getSweepMode());
      ray.setPolarizationMode(sweep->getPolarizationMode());
      ray.setPrtMode(sweep->getPrtMode());
      ray.setFollowMode(sweep->getFollowMode());
    } // iray
  } // isweep

}

//////////////////////////////////////////////////////////////
// Load up fixed angle from sweeps to rays
// This assumes the sweeps meta data is filled out, but the
// fixed angles are missing from the rays.

void RadxVol::loadFixedAnglesFromSweepsToRays()
  
{

  if (_rays.size() < 1) {
    return;
  }

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay &ray = *_rays[iray];
      ray.setFixedAngleDeg(sweep->getFixedAngleDeg());
    } // iray
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Load the ray metadata from sweep information.
///
/// This loops through all of the sweeps, setting the
/// sweep-related info on the rays

void RadxVol::loadMetadataFromSweepsToRays()
  
{
  
  if (_rays.size() < 1) {
    return;
  }
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {

      RadxRay &ray = *_rays[iray];
      ray.setMetadataFromSweep(*sweep);
    } // iray
  } // isweep

}

//////////////////////////////////////////////////////////////
// Set the calbration index on the rays,
// using pulse width to determine which calibration is relevant

void RadxVol::loadCalibIndexOnRays()
{

  if (_rcalibs.size() < 1) {
    return;
  }

  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay &ray = *_rays[iray];
    double rayPulseWidth = ray.getPulseWidthUsec();

    // find the calibration which minimizes the difference between the
    // ray pulse width and calib pulse width

    int calIndex = 0;
    double minDiff = fabs(rayPulseWidth - _rcalibs[0]->getPulseWidthUsec());
    
    for (size_t icalib = 1; icalib < _rcalibs.size(); icalib++) {
      double calibPulseWidth = _rcalibs[icalib]->getPulseWidthUsec();
      double diff = fabs(rayPulseWidth - calibPulseWidth);
      if (diff < minDiff) {
        calIndex = icalib;
        minDiff = diff;
      }
      
    } // icalib

    ray.setCalibIndex(calIndex);

  } // iray

}

/////////////////////////////////////////////////////////
// clear rays

void RadxVol::clearRays()
  
{

  // delete rays

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay::deleteIfUnused(_rays[ii]);
  }
  _rays.clear();

}

/////////////////////////////////////////////////////////
// clear ray field data
// retain the ray meta-data

void RadxVol::clearRayFields()
  
{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->clearFields();
  }

}

/////////////////////////////////////////////////////////
// clear sweep info - for this object

void RadxVol::clearSweeps()
  
{

  // delete sweeps

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    delete _sweeps[ii];
  }
  _sweeps.clear();

}

/////////////////////////////////////////////////////////
// clear sweep info - as they were in file

void RadxVol::clearSweepsAsInFile()
  
{
  for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
    delete _sweepsAsInFile[ii];
  }
  _sweepsAsInFile.clear();
}

/////////////////////////////////////////////////////////
// clear radar calib info

void RadxVol::clearRcalibs()
  
{

  // delete calibs

  for (size_t ii = 0; ii < _rcalibs.size(); ii++) {
    delete _rcalibs[ii];
  }
  _rcalibs.clear();

}

/////////////////////////////////////////////////////////
// clear the field data in the object, and the
// field vector

void RadxVol::clearFields()
  
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    delete _fields[ii];
  }
  _fields.clear();
}

/////////////////////////////////////////////////////////
// clear the correction factors

void RadxVol::clearCfactors()
  
{
  if (_cfactors) {
    delete _cfactors;
  }
  _cfactors = NULL;
}

/////////////////////////////////////////////////////////
// clear the frequency list

void RadxVol::clearFrequency()
  
{
  _platform.clearFrequency();
}

/////////////////////////////////////////////////////////
// print

void RadxVol::print(ostream &out) const
  
{
  
  out << "=============== RadxVol ===============" << endl;
  out << "  version: " << _version << endl;
  out << "  title: " << _title << endl;
  out << "  institution: " << _institution << endl;
  out << "  references: " << _references << endl;
  out << "  source: " << _source << endl;
  out << "  history: " << _history << endl;
  out << "  comment: " << _comment << endl;
  out << "  volNum: " << _volNum << endl;
  out << "  scanName: " << _scanName << endl;
  out << "  scanId(VCP): " << _scanId << endl;

  _platform.print(out);

  if (checkIsRhi()) {
    out << "  rhiMode? Y" << endl;
  } else {
    out << "  rhiMode? N" << endl;
  }
  out << "  startTimeSecs: " << RadxTime::strm(_startTimeSecs) << endl;
  out << "  startNanoSecs: " << _startNanoSecs << endl;
  out << "  endTimeSecs: " << RadxTime::strm(_endTimeSecs) << endl;
  out << "  endNanoSecs: " << _endNanoSecs << endl;
  if (_rayTimesIncrease) {
    out << "  ray times are in increasing order" << endl;
  } else {
    out << "  NOTE: ray times are NOT in increasing order" << endl;
  }
  out << "  n sweeps: " << _sweeps.size() << endl;
  out << "  n rays: " << _rays.size() << endl;
  out << "  n calibs: " << _rcalibs.size() << endl;
  vector<string> fieldNames = getUniqueFieldNameList();
  if (_fields.size() > 0) {
    out << "  n fields: " << _fields.size() << endl;
  } else {
    out << "  n fields: " << fieldNames.size() << endl;
    for (size_t ii = 0; ii < fieldNames.size(); ii++) {
      out << "    field[" << ii << "]: " << fieldNames[ii] << endl;
    }
  }
  RadxRangeGeom::print(out);
  RadxPacking::printSummary(out);
  out << "===========================================" << endl;

  if (_sweepsAsInFile.size() != _sweeps.size()) {
    out << "===========>> SWEEPS AS IN FILE <<===============" << endl;
    for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
      _sweepsAsInFile[ii]->print(out);
    }
    out << "=========>> END SWEEPS AS IN FILE <<=============" << endl;
    out << "=========>>  SWEEPS AS IN OBJECT  <<=============" << endl;
  }

  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _sweeps[ii]->print(out);
  }

  if (_sweepsAsInFile.size() != _sweeps.size()) {
    out << "=========>> END SWEEPS AS IN OBJECT <<===========" << endl;
  }

  if (_fields.size() > 0) {
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->print(out);
    }
  } else {
    for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {
      string fieldName = fieldNames[ifield];
      const RadxField *fld = getFieldFromRay(fieldName);
      if (fld != NULL) {
        out << "===== NOTE: Field is from first ray =====" << endl;
        fld->print(out);
        out << "=========================================" << endl;
      }
    }
  }

  for (size_t ii = 0; ii < _rcalibs.size(); ii++) {
    _rcalibs[ii]->print(out);
  }

  if (_cfactors) {
    _cfactors->print(out);
  }

  out << "=========== statusXml ===================" << endl;
  out << _statusXml << endl;
  out << "=========================================" << endl;

}

///////////////////////////////////
// print with ray meta data

void RadxVol::printWithRayMetaData(ostream &out) const

{
  
  print(out);
  
  // print rays
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->print(out);
  }

}

///////////////////////////////////
// print with ray summary

void RadxVol::printRaySummary(ostream &out) const

{
  
  print(out);
  
  // print rays

  out << "================ RAY SUMMARY =================" << endl;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->printSummary(out);
  }

}

//////////////////////////  
// print with field data

void RadxVol::printWithFieldData(ostream &out) const

{
  
  print(out);

  // check if rays have fields

  bool raysHaveFields = false;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->getFields().size() > 0) {
      raysHaveFields = true;
      break;
    }
  }

  // print rays

  if (raysHaveFields) {
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->printWithFieldData(out);
    }
  } else {
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->print(out);
    }
  }

}
  
///////////////////////////////////////////
// nested ray geometry class - constructor

RadxVol::RayGeom::RayGeom() :
        startRange(0.0),
        gateSpacing(0.0),
        rayCount(0)
{
  
}

RadxVol::RayGeom::RayGeom(double start_range,
                          double gate_spacing) :
        startRange(start_range),
        gateSpacing(gate_spacing),
        rayCount(1)
{
  
}

///////////////////////////////////////////
// nested ray geometry class - print

void RadxVol::RayGeom::print(ostream &out) const
  
{
  
  out << "== RAY GEOM ==" << endl;
  out << "  startRange: " << startRange << endl;
  out << "  gateSpacing: " << gateSpacing << endl;
  out << "  rayCount: " << rayCount << endl;

}

///////////////////////////////////////////////////////////
/// Remap all fields and rays onto the specified geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.
///
/// If interp is true, uses interpolation if appropriate.
/// Otherwise uses nearest neighbor.

void RadxVol::remapRangeGeom(double startRangeKm,
                             double gateSpacingKm,
                             bool interp /* = false */)
  
{

  // set the ray fields to manage their own data

  loadRaysFromFields();

  // loop through rays, remapping geometry
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->remapRangeGeom(startRangeKm, gateSpacingKm, interp);
  }
  
  // save geometry members
  
  setRangeGeom(startRangeKm, gateSpacingKm);

  // set the packing from the rays, to update packing table

  setPackingFromRays();

}

////////////////////////////////////////////////////////////
/// Remap data in all rays to the predominant range geometry.
///
/// A search is made through the rays, to identify which is the
/// predominant range geometry.  All rays which do not match this
/// are then remapped to this predominant geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::remapToPredomGeom()

{

  if (_rays.size() < 3) {
    return;
  }

  // remap rays to predominant geom
  
  RayGeom predom = _getPredomGeom();
  remapRangeGeom(predom.startRange, predom.gateSpacing);
  
}

////////////////////////////////////////////////////////////
/// Remap data in all rays to the finest range geometry.
///
/// A search is made through the rays, to identify which has the
/// finest gate spacing.  All rays which do not match this
/// are then remapped to this predominant geometry.
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::remapToFinestGeom()

{

  if (_rays.size() < 2) {
    return;
  }

  // remap rays to finest geom
  
  RayGeom finest = _getFinestGeom();
  remapRangeGeom(finest.startRange, finest.gateSpacing);
  
}

///////////////////////////////////////////////////////////
/// filter on the predominant geometry
///
/// Remove rays which do not conform to the 
/// predominant geometry
///
/// This leaves the memory managed by the rays.
/// Call loadFieldsFromRays() if you need the field data
/// to be managed by the volume.

void RadxVol::filterOnPredomGeom()

{

  // set the ray fields to manage their own data

  loadRaysFromFields();

  // find predominant geom
  
  RayGeom predom = _getPredomGeom();

  // create vector with good rays, deleting bad ones

  vector<RadxRay *> good;
  double smallDiff = 0.0001;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {

    RadxRay *ray = _rays[iray];

    // check the geometry
    if (fabs(ray->getStartRangeKm() - predom.startRange) < smallDiff &&
        fabs(ray->getGateSpacingKm() - predom.gateSpacing) < smallDiff) {
      good.push_back(ray);
    } else {
      RadxRay::deleteIfUnused(ray);
    }

  }

  // replace rays vector

  _rays = good;

  // set geometry

  _startRangeKm = predom.startRange;
  _gateSpacingKm = predom.gateSpacing;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

}

///////////////////////////////////////////
// find predominant geometry

RadxVol::RayGeom RadxVol::_getPredomGeom() const

{

  double smallDiff = 0.0001;
  vector<RayGeom> geoms;

  // loop through rays, accumulating geometry information

  for (size_t iray = 0; iray < _rays.size(); iray++) {

    const RadxRay &ray = *_rays[iray];

    // check if we already have a matching geometry
    
    bool found = false;
    for (size_t ii = 0; ii < geoms.size(); ii++) {
      RayGeom &geom = geoms[ii];
      // check the geometry
      if (fabs(ray.getStartRangeKm() - geom.startRange) < smallDiff &&
          fabs(ray.getGateSpacingKm() - geom.gateSpacing) < smallDiff) {
        // matches
        geom.rayCount++;
        found = true;
        break;
      }
    } // ii 
    
    if (!found) {
      // no match, add one
      RayGeom geom(ray.getStartRangeKm(),
                   ray.getGateSpacingKm());
      geoms.push_back(geom);
    }
    
  } // iray

  // find the predominant geometry - i.e with highest ray count

  RayGeom predom(0.0, 0.0);
  for (size_t ii = 0; ii < geoms.size(); ii++) {
    RayGeom &geom = geoms[ii];
    if (geom.rayCount > predom.rayCount) {
      predom = geom;
    }
  } // ii 

  return predom;

}

///////////////////////////////////////////
// find geometry with finest resolytion

RadxVol::RayGeom RadxVol::_getFinestGeom() const

{
  
  RayGeom finest;
  finest.gateSpacing = 1.0e99;
  finest.startRange = 0.0;
  
  // loop through rays, accumulating geometry information
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    
    const RadxRay &ray = *_rays[iray];

    if (ray.getGateSpacingKm() < finest.gateSpacing) {
      finest.gateSpacing = ray.getGateSpacingKm();
      finest.startRange = ray.getStartRangeKm();
    }

  }

  return finest;

}

//////////////////////////////////////////////////////////////
/// Copy the range geom from the fields to the rays, provided
/// the fields have consistent in geometry

void RadxVol::copyRangeGeomFromFieldsToRays()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    ray->copyRangeGeomFromFields();
    copyRangeGeom(*ray);
  }

}
  
//////////////////////////////////////////////////////////////
/// Copy the range geom from the rays to the fields

void RadxVol::copyRangeGeomFromRaysToFields()
  
{
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    ray->copyRangeGeomToFields();
  }

}
  
///////////////////////////////////////////////////////////
/// remove rays with utility flag set

void RadxVol::removeFlaggedRays()

{
  
  vector<RadxRay *> goodRays;
  vector<RadxRay *> badRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->getUtilityFlag()) {
      badRays.push_back(_rays[ii]);
    } else {
      goodRays.push_back(_rays[ii]);
    }
  }

  removeBadRays(goodRays, badRays);

}

///////////////////////////////////////////////////////////
/// filter based on ray vectors
/// Keep the good rays, remove the bad rays

void RadxVol::removeBadRays(vector<RadxRay *> &goodRays,
                            vector<RadxRay *> &badRays)

{

  // free up the bad rays

  for (size_t ii = 0; ii < badRays.size(); ii++) {
    RadxRay::deleteIfUnused(badRays[ii]);
  }
  _rays = goodRays;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();

}

///////////////////////////////////////////////////////////
/// Clear antenna transition flag on all rays

void RadxVol::clearTransitionFlagOnAllRays()

{

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    if (ray->getAntennaTransition()) {
      ray->setAntennaTransition(false);
    }
  }

  // load up the volume and sweep information from the rays

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();
  
}

///////////////////////////////////////////////////////////
/// Remove rays with the antenna transition flag set.

void RadxVol::removeTransitionRays()

{

  vector<RadxRay *> goodRays;

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    if (ray->getAntennaTransition()) {
      // free up transition ray
      RadxRay::deleteIfUnused(ray);
    } else {
      // add to good list
      goodRays.push_back(ray);
    }
  }

  _rays = goodRays;

  // load up the volume and sweep information from the rays

  loadVolumeInfoFromRays();
  loadSweepInfoFromRays();
  
}

////////////////////////////////////////////////////////////
// Remove rays with transitions, with the specified margin.
//
// Sometimes the transition flag is turned on too early in
// a transition, on not turned off quickly enough after a transition.
// If you set this to a number greater than 0, that number of rays
// will be included at each end of the transition, i.e. the
// transition will effectively be shorter at each end by this
// number of rays

void RadxVol::removeTransitionRays(int nRaysMargin)

{

  // find any marked transitions

  _findTransitions(nRaysMargin);
  bool transitionsExist = false;
  for (size_t ii = 0; ii < _transitionFlags.size(); ii++) {
    if (_transitionFlags[ii]) {
      transitionsExist = true;
      break;
    }
  }

  if (!transitionsExist) {
    return;
  }

  // load up good rays with non-transitions

  vector<RadxRay *> goodRays;
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    if (_transitionFlags[iray]) {
      // free up transition ray
      RadxRay::deleteIfUnused(ray);
    } else {
      // add to good list
      ray->setAntennaTransition(false);
      goodRays.push_back(ray);
    }
  }

  // save to _rays

  _rays = goodRays;

  // load up current information

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

////////////////////////////////////////////////////////////
// Find the transitions in the rays

void RadxVol::_findTransitions(int nRaysMargin)

{

  _transitionFlags.clear();

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _transitionFlags.push_back(_rays[iray]->getAntennaTransition());
  } // iray

  // widen good data by the specified margin

  if (nRaysMargin > 0) {
    
    // widen at start of transitions
    
    for (int ii = _transitionFlags.size() - 1; ii > 0; ii--) {
      if (_transitionFlags[ii] && !_transitionFlags[ii-1]) {
        for (int jj = ii; jj < ii + nRaysMargin; jj++) {
          if (jj < (int) _transitionFlags.size()) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
    // widen at end of transitions
    
    for (int ii = 1; ii < (int) _transitionFlags.size(); ii++) {
      if (_transitionFlags[ii-1] && !_transitionFlags[ii]) {
        for (int jj = ii - nRaysMargin; jj < ii; jj++) {
          if (jj >= 0) {
            _transitionFlags[jj] = false;
          }
        }
      }
    } // ii
    
  }

}

/////////////////////////////////////////////////////////////
/// Check transitions in surveillance mode, ensuring that the
/// pointing angle error is within the  given margin and that
/// the ray belongs to the correct sweep.

void RadxVol::optimizeSurveillanceTransitions(double maxFixedAngleErrorDeg)

{
  
  if (_sweeps.size() < 2) {
    return;
  }
  
  /// ensure we are in surveillance mode
  
  int nSur = 0;
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      nSur++;
    }
  }
  if (nSur < ((int) _sweeps.size() / 2)) {
    // not predominantly surveillance
    return;
  }

  // ensure sweep info is up to date

  loadSweepInfoFromRays();

  // loop through sweeps, working on pairs of sweeps
  // we consider the transition from one sweep to the next

  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {

    // check modes
    
    const RadxSweep *sweep1 = _sweeps[isweep];
    if (sweep1->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }
    int sweepNum1 = sweep1->getSweepNumber();

    const RadxSweep *sweep2 = _sweeps[isweep + 1];
    if (sweep2->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }
    int sweepNum2 = sweep2->getSweepNumber();

    // check the sweeps are not primarily in transition
    
    if (_computeSweepFractionInTransition(isweep) > 0.8) {
      continue;
    }
    if (_computeSweepFractionInTransition(isweep + 1) > 0.8) {
      continue;
    }

    double fixedAngle1 = sweep1->getFixedAngleDeg();
    double fixedAngle2 = sweep2->getFixedAngleDeg();

    int index1 = sweep1->getEndRayIndex();
    int index2 = sweep2->getStartRayIndex();
    
    RadxRay *ray1 = _rays[index1];
    RadxRay *ray2 = _rays[index2];

    double elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
    double elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

    if (elError1 < maxFixedAngleErrorDeg &&
        elError2 < maxFixedAngleErrorDeg) {
      // no problem - do nothing
      continue;
    }
    
    if (elError1 < elError2) {
      
      // transition is early, search ahead for correct transition point

      while (elError1 < elError2) {
        
        // ray2 belongs to this sweep, not the next
        
        ray2->setSweepNumber(sweepNum1);
        ray2->setFixedAngleDeg(fixedAngle1);
        
        // update transition flag
        
        double elError = fabs(ray2->getElevationDeg() - fixedAngle1);
        if (elError < maxFixedAngleErrorDeg) {
          ray2->setAntennaTransition(false);
        } else {
          ray2->setAntennaTransition(true);
        }
        
        // move one ray forward

        index1++;
        index2++;
        if (index2 >= (int) sweep2->getEndRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        ray2 = _rays[index2];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

      }

      // update transition flags in next sweep
      
      while (elError2 > maxFixedAngleErrorDeg || ray2->getAntennaTransition()) {
        if (elError2 > maxFixedAngleErrorDeg) {
          ray2->setAntennaTransition(true);
        } else {
          ray2->setAntennaTransition(false);
        }
        index2++;
        if (index2 > (int) sweep2->getEndRayIndex()) {
          break;
        }
        ray2 = _rays[index2];
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);
      }

    } else { // if (elError1 < elError2) {

      // transition is late, search back for correct transition point

      while (elError2 < elError1) {

        // ray1 belongs to next sweep, not this one
        
        ray1->setSweepNumber(sweepNum2);
        ray1->setFixedAngleDeg(fixedAngle2);
       
        // update transition flags
        
        double elError = fabs(ray1->getElevationDeg() - fixedAngle2);
        if (elError < maxFixedAngleErrorDeg) {
          ray1->setAntennaTransition(false);
        } else {
          ray1->setAntennaTransition(true);
        }

        // move one ray back

        index1--;
        index2--;
        if (index1 <= (int) sweep1->getStartRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        ray2 = _rays[index2];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
        elError2 = fabs(ray2->getElevationDeg() - fixedAngle2);

      }

      // update transition flags in this sweep

      while (elError1 > maxFixedAngleErrorDeg || ray1->getAntennaTransition()) {
        if (elError1 > maxFixedAngleErrorDeg) {
          ray1->setAntennaTransition(true);
        } else {
          ray1->setAntennaTransition(false);
        }
        index1--;
        if (index1 < (int) sweep1->getStartRayIndex()) {
          break;
        }
        ray1 = _rays[index1];
        elError1 = fabs(ray1->getElevationDeg() - fixedAngle1);
      }

    } // if (elError1 < elError2) {

  } // isweep
  
  // reload sweep info, since sweep numbers on rays have changed
  
  loadSweepInfoFromRays();
  
}

///////////////////////////////////////////////////////////
/// Trim surveillance sweeps to 360 deg
///
/// Remove extra rays in each surveillance sweep

void RadxVol::trimSurveillanceSweepsTo360Deg()

{

  // does not apply to RHIs

  if (!checkIsRhi()) {
    return;
  }
  
  // clear utility flags

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->setUtilityFlag(false);
  }

  // loop through sweeps
  // If the sweep covers more than 360 degrees, set flag
  // on extra rays. Use the elevation angle to determine which 
  // rays to remove.

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];
    if (sweep->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      continue;
    }

    double azCovered = computeAzCovered(sweep);
    if (azCovered <= 360.0) {
      continue;
    }

    // initialize

    int startIndex = (int) sweep->getStartRayIndex();
    int endIndex = (int) sweep->getEndRayIndex();
    int midIndex = (startIndex + endIndex) / 2;

    // sanity check

    if (endIndex - startIndex < 10) {
      continue;
    }

    // check rotation sense - clockwise or not?
    
    bool clockwise = true;
    double azm0 = _rays[midIndex]->getAzimuthDeg();
    double azm1 = _rays[midIndex+1]->getAzimuthDeg();
    double dAz0 = Radx::conditionAngleDelta(azm1 - azm0);
    if (dAz0 == 0) {
      continue;
    }
    if (dAz0 < 0) {
      clockwise = false;
    }

    // compute median elevation

    vector<double> elevs;
    for (int iray = startIndex; iray <= endIndex; iray++) {
      elevs.push_back(_rays[iray]->getElevationDeg());
    }
    sort(elevs.begin(), elevs.end());
    double medianElev = elevs[elevs.size()/2];
    
    // iterate, looking for 360 wraps, and setting
    // flags until there is no wrap
    
    size_t lowIndex = sweep->getStartRayIndex();
    size_t highIndex = sweep->getEndRayIndex();
    
    while (lowIndex < highIndex) {

      double lowAz = _rays[lowIndex]->getAzimuthDeg();
      double highAz = _rays[highIndex]->getAzimuthDeg();

      double dAz = Radx::conditionAngleDelta(highAz - lowAz);
      if (!clockwise) {
        dAz *= -1.0;
      }

      if (dAz < 0) {
        // no wrap;
        break;
      }

      // we have wrapped, so set one flag

      double elErrorLow = 
        fabs(medianElev - _rays[lowIndex]->getElevationDeg());
      double elErrorHigh =
        fabs(medianElev - _rays[highIndex]->getElevationDeg());
      if (elErrorLow > elErrorHigh) {
        _rays[lowIndex]->setUtilityFlag(true);
        lowIndex++;
      } else {
        _rays[highIndex]->setUtilityFlag(true);
        highIndex--;
      }
      
    } // while (lowIndex < highIndex)

    azCovered = computeAzCovered(sweep);

  } // isweep

  // remove rays which we have flagged with utility flag

  removeFlaggedRays();
  
}

////////////////////////////////////////////////////////////
/// Remove sweeps with fewer that the given number of rays
  
void RadxVol::removeSweepsWithTooFewRays(size_t minNRays)

{

  vector<RadxRay *> goodRays, badRays;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep = _sweeps[ii];
    size_t istart = sweep->getStartRayIndex();
    size_t iend = sweep->getEndRayIndex();
    if (sweep->getNRays() < minNRays) {
      for (size_t ii = istart; ii <= iend; ii++) {
        badRays.push_back(_rays[ii]);
      }
    } else {
      for (size_t ii = istart; ii <= iend; ii++) {
        goodRays.push_back(_rays[ii]);
      }
    }
  }

  removeBadRays(goodRays, badRays);

}

////////////////////////////////////////////////////////////
// Reorder the sweeps into ascending angle order
//
// If the sweeps are reordered, this means that the rays times
// will no longer be monotonically increasing

void RadxVol::reorderSweepsAscendingAngle()
{

  if (_sweeps.size() < 1) {
    return;
  }

  // check sweeps are in ascending order, do nothing if they are

  bool ascending = true;
  for (size_t ii = 1; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep0 = _sweeps[ii-1];
    RadxSweep *sweep1 = _sweeps[ii];
    if (sweep0->getFixedAngleDeg() > sweep1->getFixedAngleDeg()) {
      ascending = false;
      break;
    }
  }
  if (ascending) {
    return;
  }
  
  // create a map of sweeps with angles in ascending order

  multimap< double, RadxSweep* > sortedSweeps;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep = _sweeps[ii];
    double angle = sweep->getFixedAngleDeg();
    sortedSweeps.insert(pair< double, RadxSweep* >(angle, sweep));
  }

  if (_rays.size() < 1) {

    // if only metadata was read, just reorder the sweeps

    _sweeps.clear();
    multimap< double, RadxSweep* >::iterator iter;
    for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
      RadxSweep *sweep = iter->second;
      _sweeps.push_back(sweep);
    }

  } else {

    // load up sorted array vector with elevation angles in ascending order
    
    vector<RadxRay *> sortedRays;
    multimap< double, RadxSweep* >::iterator iter;
    for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
      RadxSweep *sweep = iter->second;
      for (size_t iray = sweep->getStartRayIndex();
           iray <= sweep->getEndRayIndex(); iray++) {
        RadxRay *ray = _rays[iray];
        sortedRays.push_back(ray);
      }
    }
    
    // copy sorted rays to main array
    
    _rays = sortedRays;
    checkRayTimesIncrease();
    
    // load up the sweep information from the rays
    
    loadSweepInfoFromRays();
    loadVolumeInfoFromRays();

  }

}

////////////////////////////////////////////////////////////
// Reorder the sweeps as in file into ascending angle order

void RadxVol::reorderSweepsAsInFileAscendingAngle()
{

  if (_sweepsAsInFile.size() < 1) {
    return;
  }

  // check sweeps are in ascending order, do nothing if they are

  bool ascending = true;
  for (size_t ii = 1; ii < _sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep0 = _sweepsAsInFile[ii-1];
    RadxSweep *sweep1 = _sweepsAsInFile[ii];
    if (sweep0->getFixedAngleDeg() > sweep1->getFixedAngleDeg()) {
      ascending = false;
      break;
    }
  }
  if (ascending) {
    return;
  }
  
  // create a map of sweeps with angles in ascending order

  multimap< double, RadxSweep* > sortedSweeps;
  for (size_t ii = 0; ii < _sweepsAsInFile.size(); ii++) {
    RadxSweep *sweep = _sweepsAsInFile[ii];
    double angle = sweep->getFixedAngleDeg();
    sortedSweeps.insert(pair< double, RadxSweep* >(angle, sweep));
  }

  _sweepsAsInFile.clear();
  multimap< double, RadxSweep* >::iterator iter;
  for (iter = sortedSweeps.begin(); iter != sortedSweeps.end(); iter++) {
    RadxSweep *sweep = iter->second;
    _sweepsAsInFile.push_back(sweep);
  }

}

//////////////////////////////////////////////////////////////////
/// Apply a time offset, in seconds to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads.
/// The offset is ADDED to the ray times.

void RadxVol::applyTimeOffsetSecs(double offsetSecs)

{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    RadxTime rayTime = ray->getRadxTime();
    RadxTime newTime = rayTime + offsetSecs;
    ray->setTime(newTime);
  } // ii

  // reload sweep info and volume info,
  // since these are affected by the times
  
  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

  // update history

  time_t now = time(NULL);
  char note[1024];
  sprintf(note, "Applying time offset (secs): %g, mod time %s\n",
          offsetSecs, RadxTime::strm(now).c_str());
  _history += note;

}

//////////////////////////////////////////////////////////////////
/// Apply an azimuth offset to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads

void RadxVol::applyAzimuthOffset(double offset)

{

  bool isRhi = checkIsRhi();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    double az = Radx::conditionAz(ray->getAzimuthDeg() + offset);
    ray->setAzimuthDeg(az);
    if (isRhi) {
      double fixedAng = Radx::conditionAz(ray->getFixedAngleDeg() + offset);
      ray->setFixedAngleDeg(fixedAng);
    }
  } // ii

  // reload sweep info, since sweep numbers on rays have changed
  
  loadSweepInfoFromRays();

  // update history

  time_t now = time(NULL);
  char note[1024];
  sprintf(note, "Applying azimuth offset: %g, mod time %s\n",
          offset, RadxTime::strm(now).c_str());
  _history += note;

}

//////////////////////////////////////////////////////////////////
/// Apply an elevation offset to all rays in the volume
/// This applies to the rays currently in the volume, not to
/// any future reads

void RadxVol::applyElevationOffset(double offset)

{

  bool isRhi = checkIsRhi();

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxRay *ray = _rays[ii];
    double el = Radx::conditionEl(ray->getElevationDeg() + offset);
    ray->setElevationDeg(el);
    if (!isRhi) {
      double fixedAng = Radx::conditionEl(ray->getFixedAngleDeg() + offset);
      ray->setFixedAngleDeg(fixedAng);
    }
  } // ii

  // update history

  time_t now = time(NULL);
  char note[1024];
  sprintf(note, "Applying elevation offset: %g, mod time %s\n",
          offset, RadxTime::strm(now).c_str());
  _history += note;

}

////////////////////////////////////////////  
/// Set the fixed angle for a sweep
/// Also sets the fixed angle for the rays in the sweep

void RadxVol::setFixedAngleDeg(int sweepNum, double fixedAngle)

{

  // get sweep

  RadxSweep *sweep = getSweepByNumber(sweepNum);
  if (sweep == NULL) {
    cerr << "WARNING - RadxVol::setFixedAngleDeg" << endl;
    cerr << "  Trying to set fixed angle: " << fixedAngle << endl;
    cerr << "  on sweepNumber: " << sweepNum << endl;
    cerr << "  sweep does not exist" << endl;
    return;
  }

  // set the fixed angle on the rays
  
  for (size_t ii = sweep->getStartRayIndex();
       ii <= sweep->getEndRayIndex(); ii++) {
    _rays[ii]->setFixedAngleDeg(fixedAngle);
  }

  // set the fixed angle on the sweep

  sweep->setFixedAngleDeg(fixedAngle);

}

//////////////////////////////////////////////////////////  
/// Set the sweep scan modes from ray angles
///
/// Deduce the antenna scan mode from the ray angles in each
/// sweep, and set the scan mode on the rays and sweep objects.

void RadxVol::setSweepScanModeFromRayAngles()

{

  // load sweep info if needed
  
  if (_sweeps.size() < 1) {
    loadSweepInfoFromRays();
  }

  // loop through sweeps

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    Radx::SweepMode_t sweepMode = sweep->getSweepMode();
    size_t startRayIndex = sweep->getStartRayIndex();
    size_t endRayIndex = sweep->getEndRayIndex();
    
    if (checkIsRhi(startRayIndex, endRayIndex)) {
      sweepMode = Radx::SWEEP_MODE_RHI;
    } else if (checkIsSurveillance(startRayIndex, endRayIndex)) {
      sweepMode = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
    } else {
      sweepMode = Radx::SWEEP_MODE_SECTOR;
    }
    
    for (size_t iray = startRayIndex; iray <= endRayIndex; iray++) {
      RadxRay *ray = _rays[iray];
      ray->setSweepMode(sweepMode);
    }
    sweep->setSweepMode(sweepMode);

  } // isweep

}

//////////////////////////////////////////////////////////////////
/// Set the volume number.
/// This increments with every volume, and may wrap.

void RadxVol::setVolumeNumber(int volNum)

{

  _volNum = volNum; 

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    _sweeps[isweep]->setVolumeNumber(_volNum);
  }

  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setVolumeNumber(_volNum);
  }

}
  
//////////////////////////////////////////////////////////////////
/// Load volume information from the rays.
///
/// This sets the volume number and the start and end times.

void RadxVol::loadVolumeInfoFromRays()
  
{
  
  // check we have data
  
  if (_rays.size() < 1) {
    return;
  }
  
  // set vol number from first ray

  _volNum = _rays[_rays.size()/2]->getVolumeNumber();

  // set start and end times

  double startTime = 1.0e99;
  double endTime = -1.0e99;

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    const RadxRay *ray = _rays[ii];
    double dtime = ray->getTimeDouble();
    if (dtime < startTime) {
      startTime = dtime;
      _startTimeSecs = ray->getTimeSecs();
      _startNanoSecs = ray->getNanoSecs();
    }
    if (dtime > endTime) {
      endTime = dtime;
      _endTimeSecs = ray->getTimeSecs();
      _endNanoSecs = ray->getNanoSecs();
    }
  }

  // check that ray times increase
  
  checkRayTimesIncrease();

  // set geom from predominant

  RayGeom predom = _getPredomGeom();
  setRangeGeom(predom.startRange, predom.gateSpacing);
  setPackingFromRays();

}

///////////////////////////////////////////////////////////////
// check that the ray times increase, set flag accordingly

void RadxVol::checkRayTimesIncrease()

{

  _rayTimesIncrease = true;

  if (_rays.size() < 1) {
    return;
  }

  double prevTime = _rays[0]->getTimeDouble();

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    const RadxRay *ray = _rays[ii];
    double rayTime = ray->getTimeDouble();
    if (rayTime < prevTime) {
      _rayTimesIncrease = false;
      return;
    }
    prevTime = rayTime;
  }

}

///////////////////////////////////////////////////////////////
/// Check through the rays, and increment the sweep number
/// if the polarization mode changes in the middle of a sweep

void RadxVol::incrementSweepOnPolModeChange()

{

  if (_rays.size() < 2) {
    return;
  }

  int nIncr = 0;

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    Radx::PolarizationMode_t prevPolMode = _rays[ii-1]->getPolarizationMode();
    Radx::PolarizationMode_t polMode = _rays[ii]->getPolarizationMode();

    int prevSweepNum = _rays[ii-1]->getSweepNumber();
    int sweepNum = _rays[ii]->getSweepNumber();

    if ((polMode != prevPolMode) && (sweepNum == prevSweepNum)) {
      nIncr++;
    }

    if (nIncr > 0) {
      _rays[ii]->setSweepNumber(_rays[ii]->getSweepNumber() + nIncr);
    }

  } // ii

}
  
///////////////////////////////////////////////////////////////
/// Check through the rays, and increment the sweep number
/// if the prt mode changes in the middle of a sweep

void RadxVol::incrementSweepOnPrtModeChange()

{

  if (_rays.size() < 2) {
    return;
  }

  int nIncr = 0;

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    Radx::PrtMode_t prevPrtMode = _rays[ii-1]->getPrtMode();
    Radx::PrtMode_t prtMode = _rays[ii]->getPrtMode();

    int prevSweepNum = _rays[ii-1]->getSweepNumber();
    int sweepNum = _rays[ii]->getSweepNumber();

    if ((prtMode != prevPrtMode) && (sweepNum == prevSweepNum)) {
      nIncr++;
    }

    if (nIncr > 0) {
      _rays[ii]->setSweepNumber(_rays[ii]->getSweepNumber() + nIncr);
    }

  } // ii

}
  
///////////////////////////////////////////////////////////////
/// Load the sweep information from the rays.
///
/// This loops through all of the rays, and determines the sweep
/// information from them. The resulting information is stored
/// in the sweeps array on the volume.
///
/// Also sets the start/end of sweep/volume flags

void RadxVol::loadSweepInfoFromRays()
  
{

  clearSweeps();
  int prevSweepNum = -9999;
  RadxSweep *sweep = NULL;
  int rayIndex = 0;

  // do we need to fill in sweep numbers?

  RadxAngleHist hist;
  if (hist.checkSweepsNumbersAllMissing(_rays)) {
    hist.fillInSweepNumbers(_rays);
  }
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    
    RadxRay &ray = *_rays[ii];
    int sweepNum = ray.getSweepNumber();
    rayIndex = ii;

    if (ii == 0) {
      ray.setStartOfSweepFlag(true);
      ray.setStartOfVolumeFlag(true);
    } else if (ii == _rays.size() - 1) {
      ray.setEndOfSweepFlag(true);
      ray.setEndOfVolumeFlag(true);
    }

    if (sweepNum != prevSweepNum) {

      ray.setStartOfSweepFlag(true);
      if (ii > 0) {
        _rays[ii-1]->setEndOfSweepFlag(true);
      }

      sweep = new RadxSweep();

      sweep->setVolumeNumber(ray.getVolumeNumber());
      sweep->setSweepNumber(sweepNum);

      sweep->setStartRayIndex(rayIndex);
      sweep->setEndRayIndex(rayIndex);

      sweep->setSweepMode(ray.getSweepMode());
      sweep->setPolarizationMode(ray.getPolarizationMode());
      sweep->setPrtMode(ray.getPrtMode());
      sweep->setFollowMode(ray.getFollowMode());
      
      sweep->setFixedAngleDeg(ray.getFixedAngleDeg());
      sweep->setTargetScanRateDegPerSec(ray.getTargetScanRateDegPerSec());
      
      sweep->setRaysAreIndexed(ray.getIsIndexed());
      sweep->setAngleResDeg(ray.getAngleResDeg());

      _sweeps.push_back(sweep);
      
      prevSweepNum = sweepNum;
      
    } else {
      
      if (sweep) {
        sweep->setEndRayIndex(rayIndex);
      }

    }

  }

  // last sweep ends at last ray
  
  if (_sweeps.size() > 0) {
    _sweeps[_sweeps.size()-1]->setEndRayIndex(rayIndex);
  }

  // set sweep flags using median value for rays in sweep
  // also check long range

  vector<Radx::SweepMode_t> sweepModes;
  vector<Radx::PolarizationMode_t> polModes;
  vector<Radx::PrtMode_t> prtModes;
  vector<Radx::FollowMode_t> followModes;
  vector<double> fixedAngles;
  vector<double> scanRates;
  vector<bool> raysAreIndexed;
  vector<double> angleRes;
  vector<bool> isLongRange;
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    // find sweep

    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();
    if (startIndex == endIndex) {
      continue;
    }

    // initialize sweep counters and vectors

    sweepModes.clear();
    polModes.clear();
    prtModes.clear();
    followModes.clear();
    fixedAngles.clear();
    scanRates.clear();
    raysAreIndexed.clear();
    angleRes.clear();
    isLongRange.clear();

    // accum data for sweep

    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      const RadxRay &ray = *_rays[iray];
      sweepModes.push_back(ray.getSweepMode());
      polModes.push_back(ray.getPolarizationMode());
      prtModes.push_back(ray.getPrtMode());
      followModes.push_back(ray.getFollowMode());
      fixedAngles.push_back(ray.getFixedAngleDeg());
      scanRates.push_back(ray.getTargetScanRateDegPerSec());
      raysAreIndexed.push_back(ray.getIsIndexed());
      angleRes.push_back(ray.getAngleResDeg());
      isLongRange.push_back(ray.getIsLongRange());
    } // iray

    // sort vectors to prepare for median
    
    sort(sweepModes.begin(), sweepModes.end());
    sort(polModes.begin(), polModes.end());
    sort(prtModes.begin(), prtModes.end());
    sort(followModes.begin(), followModes.end());
    sort(fixedAngles.begin(), fixedAngles.end());
    sort(scanRates.begin(), scanRates.end());
    sort(raysAreIndexed.begin(), raysAreIndexed.end());
    sort(angleRes.begin(), angleRes.end());
    sort(isLongRange.begin(), isLongRange.end());

    // set sweep info

    int nRaysSweep = endIndex - startIndex + 1;
    int nRaysHalf = nRaysSweep / 2;
    
    sweep->setSweepMode(sweepModes[nRaysHalf]);
    sweep->setPolarizationMode(polModes[nRaysHalf]);
    sweep->setPrtMode(prtModes[nRaysHalf]);
    sweep->setFollowMode(followModes[nRaysHalf]);
    sweep->setFixedAngleDeg(fixedAngles[nRaysHalf]);
    sweep->setTargetScanRateDegPerSec(scanRates[nRaysHalf]);
    sweep->setRaysAreIndexed(raysAreIndexed[nRaysHalf]);
    sweep->setAngleResDeg(angleRes[nRaysHalf]);
    sweep->setIsLongRange(isLongRange[nRaysHalf]);

  } // isweep

  // copy into sweep array as in file, if not previously done
  
  if (_sweepsAsInFile.size() < _sweeps.size()) {
    clearSweepsAsInFile();
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      addSweepAsInFile(_sweeps[ii]);
    }
  }

}

///////////////////////////////////////////////////////////////
/// Adjust the limits of sweeps, by comparing the measured angles
/// to the fixed angles.
///
/// Sometimes the transitions from one fixed angle to another are
/// not accurately described by the scan flags, and as a result rays
/// are not correctly assigned to the sweeps.
///
/// This routine goes through the volume in ray order, and adjusts
/// the way rays are associated with each sweep. It does this by
/// comparing the actual angle with the fixed angle, and minimizes
/// the angular difference.
///
/// Before calling this routine you need to ensure that the fixed
/// sweep angle and measured ray angle has been set on the rays.

void RadxVol::adjustSweepLimitsUsingAngles()
  
{

  // load up the sweep info from the rays
  
  loadSweepInfoFromRays();
  
  if (_sweeps.size() < 2) {
    // no action required
    return;
  }

  if (checkIsRhi()) {
    _adjustSweepLimitsRhi();
  } else {
    _adjustSweepLimitsPpi();
  }

}

void RadxVol::_adjustSweepLimitsPpi()

{
    
  // loop through sweep pairs
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // get fixed angles, and compute the change between them
    
    double fixedAngleThis = sweepThis->getFixedAngleDeg();
    double fixedAngleNext = sweepNext->getFixedAngleDeg();
    double delta = fabs(fixedAngleNext - fixedAngleThis);
    if (delta > 180.0) {
      delta = fabs(delta - 360.0);  // correct for north crossing in PPI
    }
    if (delta < 0.01) {
      continue; // fixed angles essentially the same
    }
    double halfDelta = delta / 2.0;

    // find transition location
    
    size_t iray0 = sweepThis->getStartRayIndex();
    size_t iray1 = sweepThis->getEndRayIndex();
    size_t iray2 = sweepNext->getStartRayIndex();
    size_t iray3 = sweepNext->getEndRayIndex();
    
    size_t irayTrans = 0;
    for (size_t iray = iray0; iray < iray3; iray++) {
      RadxRay *ray = _rays[iray];
      double fixedAngle = ray->getElevationDeg();
      double diff = fabs(fixedAngleNext - fixedAngle);
      if (diff > 180.0) {
        diff = fabs(diff - 360.0); // correct for north crossing
      }
      if (diff < halfDelta) {
        // have moved to next sweep
        irayTrans = iray;
        break;
      }
    }
    
    if (irayTrans == 0) {
      // no transition found, do nothing
      continue;
    }
    
    // adjust sweep info to match transition
    
    if (irayTrans <= iray1) {
      for (size_t iray = irayTrans; iray <= iray1; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepNext->getSweepNumber());
        ray->setFixedAngleDeg(sweepNext->getFixedAngleDeg());
      }
    } else if (irayTrans >= iray2) {
      for (size_t iray = iray2; iray < irayTrans; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepThis->getSweepNumber());
        ray->setFixedAngleDeg(sweepThis->getFixedAngleDeg());
      }
    }
    
    // adjust start/end indices
    
    sweepThis->setEndRayIndex(irayTrans - 1);
    sweepNext->setStartRayIndex(irayTrans);
    
  } // isweep

}

void RadxVol::_adjustSweepLimitsRhi()

{
    
  // loop through sweep pairs
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // check motion direction
    
    size_t iray0 = sweepThis->getStartRayIndex();
    size_t iray1 = sweepThis->getEndRayIndex();
    size_t iray2 = sweepNext->getStartRayIndex();
    size_t iray3 = sweepNext->getEndRayIndex();

    RadxRay *ray0 = _rays[iray0];
    RadxRay *ray1 = _rays[iray1];
    RadxRay *ray2 = _rays[iray2];
    RadxRay *ray3 = _rays[iray3];
    
    double elev0 = ray0->getElevationDeg();
    double elev1 = ray1->getElevationDeg();
    double elev2 = ray2->getElevationDeg();
    double elev3 = ray3->getElevationDeg();

    double deltaElevThis = elev1 - elev0;
    double deltaElevNext = elev3 - elev2;

    double goingUpThis = true;
    if (deltaElevThis < 0.0 && deltaElevNext > 0) {
      goingUpThis = false;
    }

    // find transition location
    // find min or max elevation depending on whether this sweep is going up or down
    // we will use this inflection point as the transition to the next sweep

    size_t irayTrans = 0;
    
    if (goingUpThis) {
      
      double maxElev = -999.0;
      for (size_t iray = iray0; iray < iray3; iray++) {
        RadxRay *ray = _rays[iray];
        double elev = ray->getElevationDeg();
        if (elev > maxElev) {
          irayTrans = iray;
          maxElev = elev;
        }
      }

    } else {

      double minElev = 999.0;
      for (size_t iray = iray0; iray < iray3; iray++) {
        RadxRay *ray = _rays[iray];
        double elev = ray->getElevationDeg();
        if (elev < minElev) {
          irayTrans = iray;
          minElev = elev;
        }
      }

    }

    if (irayTrans == 0) {
      // no transition found, do nothing
      continue;
    }
    
    // adjust sweep info to match transition
    
    if (irayTrans <= iray1) {
      for (size_t iray = irayTrans; iray <= iray1; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepNext->getSweepNumber());
        ray->setFixedAngleDeg(sweepNext->getFixedAngleDeg());
      }
    } else if (irayTrans >= iray2) {
      for (size_t iray = iray2; iray < irayTrans; iray++) {
        RadxRay *ray = _rays[iray];
        ray->setSweepNumber(sweepThis->getSweepNumber());
        ray->setFixedAngleDeg(sweepThis->getFixedAngleDeg());
      }
    }
    
    // adjust start/end indices
    
    sweepThis->setEndRayIndex(irayTrans - 1);
    sweepNext->setStartRayIndex(irayTrans);
    
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Adjust surveillance sweep limits based on azimuth.
///
/// Some radars (e.g. DOWs) always change elevation angle at a
/// fixed theorerical azimuth.
/// 
/// This function sets the transitions between sweeps based on a
/// designated azimuth.

void RadxVol::adjustSurSweepLimitsToFixedAzimuth(double azimuth)
  
{

  // load up the sweep info from the rays
  
  loadSweepInfoFromRays();
  
  if (_sweeps.size() < 2) {
    // no action required
    return;
  }

  if (checkIsRhi()) {
    // not applicable in RHI mode
    return;
  }
  
  if (!checkIsSurveillance()) {
    // not applicable unless in surveillance mode
    return;
  }

  // loop through sweep pairs, looking for the transitions across
  // the specified azimuth

  vector<int> azTransIndex;
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];

    // check for surveillance

    if (sweepThis->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE ||
        sweepNext->getSweepMode() != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
      return;
    }

    // get point for each sweep where it crosses the azimuth
    
    int azIndexThis = _getTransIndex(sweepThis, azimuth);
    int azIndexNext = _getTransIndex(sweepNext, azimuth);

    if (azIndexThis < 0 && azIndexNext < 0) {
      // no transition found
      azTransIndex.push_back(-1);
      continue;
    }

    if (azIndexThis < 0 && azIndexNext >= 0) {
      // no azimuth transition this sweep, but got one for next sweep
      int nFwd = azIndexNext - sweepNext->getStartRayIndex();
      if (nFwd < (int) sweepNext->getNRays() / 2) {
        // use this transition
        azTransIndex.push_back(azIndexNext);
      } else {
        azTransIndex.push_back(-1);
      }
      continue;
    }

    if (azIndexThis >= 0 && azIndexNext < 0) {
      // no azimuth transition next sweep, but got one for this sweep
      int nBack = sweepThis->getEndRayIndex() - azIndexThis;
      if (nBack < (int) sweepThis->getNRays() / 2) {
        // use this transition
        azTransIndex.push_back(azIndexThis);
      } else {
        azTransIndex.push_back(-1);
      }
      continue;
    }

    int nBack = sweepThis->getEndRayIndex() - azIndexThis;
    int nFwd = azIndexNext - sweepNext->getStartRayIndex();
    if (nBack < nFwd) {
      azTransIndex.push_back(azIndexThis);
    } else {
      azTransIndex.push_back(azIndexNext);
    }
    
  } // isweep
    
  // loop through sweep pairs, adjusting the transitions
  
  for (size_t isweep = 0; isweep < _sweeps.size() - 1; isweep++) {

    if (azTransIndex[isweep] < 0) {
      // do not adjust this one
      continue;
    }
    
    RadxSweep *sweepThis = _sweeps[isweep];
    RadxSweep *sweepNext = _sweeps[isweep + 1];
    int transIndex = azTransIndex[isweep];

    sweepThis->setEndRayIndex(transIndex);
    sweepNext->setStartRayIndex(transIndex + 1);

  } // isweep

  // set the ray metadata to match
 
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    RadxSweep *sweep = _sweeps[isweep];
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay &ray = *_rays[iray];
      ray.setSweepNumber(sweep->getSweepNumber());
      ray.setFixedAngleDeg(sweep->getFixedAngleDeg());
    } // iray
  } // isweep

}

//////////////////////////////////////////////////////////  
/// Compute the fixed angle for each sweep from the rays.
/// Also sets the fixed angle on rays and sweeps.
///
/// If useMean is true, computes using the mean
/// If useMean is false, uses the median
///
/// If force is true, the angles will be computed for all sweeps.
/// If force is false, the angles will only be computed for
/// sweeps with a missing fixed angle.

void RadxVol::computeFixedAnglesFromRays(bool force /* = true */,
                                         bool useMean /* = true */)

{

  // load sweep info if needed
  
  if (_sweeps.size() < 1) {
    loadSweepInfoFromRays();
  }

  // sweep mode
  
  bool isRhi = checkIsRhi();

  // loop through sweeps

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();

    // if force false, check to see if we have a fixed angle
    
    if (!force) {
      if (sweep->getFixedAngleDeg() != Radx::missingMetaDouble) {
        continue;
      }
    }
    
    // sum up (x,y) coords of measured angles

    double sumx = 0.0;
    double sumy = 0.0;
    vector<double> angles;
    double fixedAngle = 0.0;

    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      
      const RadxRay *ray = _rays[iray];
      double angle;
      if (isRhi) {
        angle = ray->getAzimuthDeg();
      } else {
        angle = ray->getElevationDeg();
      }
      
      angles.push_back(angle);
      
      if (useMean) {
        double sinVal, cosVal;
        Radx::sincos(angle * Radx::DegToRad, sinVal, cosVal);
        sumy += sinVal;
        sumx += cosVal;
      }
      
    } // iray

    if (angles.size() > 0) {
      if (useMean) {
        // compute mean angle
        double meanAngleDeg = atan2(sumy, sumx) * Radx::RadToDeg;
        fixedAngle = meanAngleDeg;
      } else {
        // compute median
        sort(angles.begin(), angles.end());
        double medianAngleDeg = angles[angles.size() / 2];
        fixedAngle = medianAngleDeg;
      }
    }

    // set fixed angle on sweep

    sweep->setFixedAngleDeg(fixedAngle);

    // set on rays in this sweep
    
    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      _rays[iray]->setFixedAngleDeg(fixedAngle);
    }
    
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Compute sweep scan rates from ray data - in deg/sec.
///
/// This is done using the angle information on the rays.
/// Sets the measureScanRate on the sweeps.
/// Use sweep->getMeasuredScanRateDegPerSec() to retrieve
/// the scan rates after this call completes.

void RadxVol::computeSweepScanRatesFromRays()
  
{

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();

    // sweep mode

    bool isRhi = checkIsRhi();

    // sum up angle movement

    double sumDeltaAngle = 0.0;
    double prevAngle = 0.0;
    if (isRhi) {
      prevAngle = _rays[startIndex]->getElevationDeg();
    } else {
      prevAngle = _rays[startIndex]->getAzimuthDeg();
    }

    for (size_t iray = startIndex + 1; iray <= endIndex; iray++) {
      const RadxRay &ray = *_rays[iray];
      double angle = 0.0;
      if (isRhi) {
        angle = ray.getElevationDeg();
      } else {
        angle = ray.getAzimuthDeg();
      }
      double delta = Radx::conditionAngleDelta(angle - prevAngle);
      sumDeltaAngle += delta;
      prevAngle = angle;
    } // iray

    // get elapsed time

    RadxTime startTime = _rays[startIndex]->getRadxTime();
    RadxTime endTime = _rays[endIndex]->getRadxTime();
    double deltaSecs = endTime - startTime;

    double rate = sumDeltaAngle / deltaSecs;
    sweep->setMeasuredScanRateDegPerSec(rate);
    
  } // isweep

}

///////////////////////////////////////////////////////////////
/// Compute median angle for specified sweep from ray data.
/// Does not set the angle on the sweep or ray objects.
///
/// Returns computed median angle.

double RadxVol::computeSweepMedianFixedAngle(const RadxSweep *sweep) const

{

  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();

  // sweep mode
  
  bool isRhi = checkIsRhi();
  
  // compute median angle
  
  vector<double> angles;
  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    const RadxRay &ray = *_rays[iray];
    if (isRhi) {
      angles.push_back(ray.getAzimuthDeg());
    } else {
      angles.push_back(ray.getElevationDeg());
    }
  } // iray

  if (angles.size() > 2) {
    sort(angles.begin(), angles.end());
    double medianAngle = angles[angles.size() / 2];
    return medianAngle;
  } else {
    return Radx::missingMetaDouble;
  }

}

///////////////////////////////////////////////////////////////
/// Get fraction of sweep with transition rays.
/// Returns the fraction.

double RadxVol::_computeSweepFractionInTransition(int sweepIndex)
  
{
  
  if (sweepIndex < 0 || sweepIndex > (int) _sweeps.size()) {
    return 0.0;
  }

  RadxSweep *sweep = _sweeps[sweepIndex];
  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();

  int nTrans = 0;
  int nTotal = 0;

  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    const RadxRay *ray = _rays[iray];
    if (ray->getAntennaTransition()) {
      nTrans++;
    }
    nTotal++;
  }

  double fraction = (double) nTrans / (double) nTotal;
  return fraction;

}

///////////////////////////////////////////
// compute the geometry limits from rays

void RadxVol::computeGeomLimitsFromRays(double &minElev,
                                        double &maxElev,
                                        double &minRange,
                                        double &maxRange)
  
{

  if (_rays.size() < 1) {
    return;
  }

  const RadxRay &ray0 = *_rays[0];

  minElev = ray0.getElevationDeg();
  maxElev = ray0.getElevationDeg();
  minRange = ray0.getStartRangeKm();
  maxRange = minRange + ray0.getNGates() * ray0.getGateSpacingKm();

  // loop through rays, accumulating geometry information
  
  for (size_t iray = 1; iray < _rays.size(); iray++) {

    const RadxRay &ray = *_rays[iray];

    double elev = ray.getElevationDeg();
    double startRange = ray.getStartRangeKm();
    double gateSpacing = ray.getGateSpacingKm();
    double endRange = startRange + ray.getNGates() * gateSpacing;
    
    if (elev < minElev) {
      minElev = elev;
    }
    if (elev > maxElev) {
      maxElev = elev;
    }
    if (startRange < minRange) {
      minRange = startRange;
    }
    if (endRange > maxRange) {
      maxRange = endRange;
    }
    
  } // iray

}

///////////////////////////////////////////////////////////////
/// Estimate nyquist per sweep from velocity field
///
/// If nyquist values are missing, we can estimate the nyquist
/// finding the max absolute velocity in each sweep.

void RadxVol::estimateSweepNyquistFromVel(const string &velFieldName)
  
{

  // estimate nyquist for each sweep

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();
    double maxAbsVel = 0;

    for (size_t iray = startIndex; iray <= endIndex; iray++) {
      const RadxRay &ray = *_rays[iray];
      const RadxField *velField = ray.getField(velFieldName);
      if (velField != NULL) {
        RadxField velf(*velField);
        velf.convertToFl32();
        const Radx::fl32 *vel = velf.getDataFl32();
        Radx::fl32 miss = velf.getMissingFl32();
        for (size_t igate = 0; igate < velf.getNPoints(); igate++) {
          if (vel[igate] != miss) {
            double absVel = fabs(vel[igate]);
            if (absVel > maxAbsVel) {
              maxAbsVel = absVel;
            }
          } // if (vel[igate] != miss)
        } // igate
      } // if (velField != NULL)
    } // iray

    if (maxAbsVel > 0) {
      double estimatedNyquist = maxAbsVel;
      for (size_t iray = startIndex; iray <= endIndex; iray++) {
        RadxRay &ray = *_rays[iray];
        if (ray.getNyquistMps() <= 0) {
          ray.setNyquistMps(estimatedNyquist);
        }
      } // iray
    } // if (maxAbsVel > 0) 
    
  } // isweep
  
}

////////////////////////////////////////////////////////////
// Constrain the data by specifying fixedAngle limits
//
// This operation will remove unwanted rays from the
// data set, remap the field arrays for the remaining
// rays and set the field pointers in the rays to
// the remapped fields.
///
/// If strictChecking is TRUE, we only get rays within the specified limits.
/// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
///
/// Returns 0 on success, -1 on failure

int RadxVol::constrainByFixedAngle(double minFixedAngleDeg,
                                   double maxFixedAngleDeg,
                                   bool strictChecking /* = false */)
{

  // find sweep indexes which lie within the fixedAngle limits

  vector<int> sweepIndexes;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    double angle = _sweeps[ii]->getFixedAngleDeg();
    if (angle > (minFixedAngleDeg - 0.01) &&
        angle < (maxFixedAngleDeg + 0.01)) {
      sweepIndexes.push_back(ii);
    }
  }

  // make sure we have at least one index

  if (sweepIndexes.size() == 0) {
    if (strictChecking) {
      // require at least 1 sweep within limits
      return -1;
    }
    double minDiff = 1.0e99;
    double meanAngle = (minFixedAngleDeg + maxFixedAngleDeg) / 2.0;
    if (maxFixedAngleDeg - minFixedAngleDeg < 0) {
      meanAngle -= 180.0;
    }
    if (meanAngle < 0) {
      meanAngle += 360.0;
    }
    int index = 0;
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      double angle = _sweeps[ii]->getFixedAngleDeg();
      double diff = fabs(angle - meanAngle);
      if (diff < minDiff) {
        minDiff = diff;
        index = ii;
      }
    }
    sweepIndexes.push_back(index);
  }

  // constrain based on sweep indexes

  _constrainBySweepIndex(sweepIndexes);

  return 0;

}

////////////////////////////////////////////////////////////
// constrain the data by specifying sweep number limits.
//
// This operation will remove unwanted rays from the
// data set, remap the field arrays for the remaining
// rays and set the field pointers in the rays to
// the remapped fields.
///
/// If strictChecking is TRUE, we only get rays within the specified limits.
/// If strictChecking is FALSE, we are guaranteed to get at least 1 sweep.
///
/// Returns 0 on success, -1 on failure

int RadxVol::constrainBySweepNum(int minSweepNum,
                                 int maxSweepNum,
                                 bool strictChecking /* = false */)

{

  // find sweep indexes which lie within the sweep number limits

  vector<int> sweepIndexes;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    int num = _sweeps[ii]->getSweepNumber();
    if (num >= minSweepNum && num <= maxSweepNum) {
      sweepIndexes.push_back(ii);
    }
  }

  // make sure we have at least one index

  if (sweepIndexes.size() == 0) {
    if (strictChecking) {
      // require at least 1 sweep within limits
      return -1;
    }
    double minDiff = 1.0e99;
    double meanSweepNum = (minSweepNum + maxSweepNum) / 2.0;
    int index = 0;
    for (size_t ii = 0; ii < _sweeps.size(); ii++) {
      double num = _sweeps[ii]->getSweepNumber();
      double diff = fabs(num - meanSweepNum);
      if (diff < minDiff) {
        minDiff = diff;
        index = ii;
      }
    }
    sweepIndexes.push_back(index);
  }

  // constrain based on sweep indexes

  _constrainBySweepIndex(sweepIndexes);

  return 0;

}

/////////////////////////////////////////////////////////////////
// Ensure ray times are monotonically increasing by
// interpolating the times if there are duplicates

void RadxVol::interpRayTimes()

{

  if (_rays.size() < 3) {
    return;
  }
  
  time_t prevSecs =  _rays[0]->getTimeSecs();
  int prevNano =  (int) (_rays[0]->getNanoSecs() + 0.5);
  size_t prevIndex = 0;

  for (size_t iray = 1; iray < _rays.size(); iray++) {
    
    RadxRay &ray = *_rays[iray];
    time_t secs =  ray.getTimeSecs();
    int nano =  (int) (ray.getNanoSecs() + 0.5);

    if (secs != prevSecs || nano != prevNano) {

      int nRaysSame = iray - prevIndex;
      
      if (nRaysSame > 0) {
        // interpolate
        double prevTime = (double) prevSecs + prevNano / 1.0e9;
        double thisTime = (double) secs + nano / 1.0e9;
        double delta = (thisTime - prevTime) / (double) nRaysSame;
        for (size_t jj = prevIndex; jj < iray; jj++) {
          RadxRay &jray = *_rays[jj];
          jray.setTime(jray.getTimeDouble() + delta * (jj - prevIndex));
        }
      }

      prevSecs = secs;
      prevNano = nano;
      prevIndex = iray;
      
    } // if (secs != prevSecs || nano != prevNano)

  } // iray
    
}

/////////////////////////////////////////////////////////////////
// Sort rays by time

bool RadxVol::SortByRayTime::operator()
  (const RayPtr &lhs, const RayPtr &rhs) const
{
  return lhs.ptr->getRadxTime() < rhs.ptr->getRadxTime();
}

void RadxVol::sortRaysByTime()

{

  // sanity check

  if (_rays.size() < 2) {
    return;
  }

  // create set with sorted ray pointers

  multiset<RayPtr, SortByRayTime> sortedRayPtrs;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RayPtr rptr(_rays[iray]);
    sortedRayPtrs.insert(rptr);
  }

  // reload _rays array in time-sorted order

  _rays.clear();
  for (multiset<RayPtr, SortByRayTime>::iterator ii = sortedRayPtrs.begin();
       ii != sortedRayPtrs.end(); ii++) {
    _rays.push_back(ii->ptr);
  }

  // set sweep info from rays

  loadSweepInfoFromRays();
    
}

/////////////////////////////////////////////////////////////////
// Set ray numbers in order in the volume

void RadxVol::setRayNumbersInOrder()

{
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    _rays[iray]->setRayNumber(iray);
  }
}

/////////////////////////////////////////////////////////////////
// Sort rays by number

bool RadxVol::SortByRayNumber::operator()
  (const RayPtr &lhs, const RayPtr &rhs) const
{
  return lhs.ptr->getRayNumber() < rhs.ptr->getRayNumber();
}

void RadxVol::sortRaysByNumber()

{

  // sanity check
  
  if (_rays.size() < 2) {
    return;
  }

  // create set with sorted ray pointers

  multiset<RayPtr, SortByRayNumber> sortedRayPtrs;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RayPtr rptr(_rays[iray]);
    sortedRayPtrs.insert(rptr);
  }

  // reload _rays array in time-sorted order

  _rays.clear();
  for (multiset<RayPtr, SortByRayNumber>::iterator ii = sortedRayPtrs.begin();
       ii != sortedRayPtrs.end(); ii++) {
    _rays.push_back(ii->ptr);
  }
    
  // set sweep info from rays

  loadSweepInfoFromRays();
    
}

/////////////////////////////////////////////////////////////////
// Sort sweeps by fixed angle, reordering the rays accordingly

bool RadxVol::SortByFixedAngle::operator()
  (const SweepPtr &lhs, const SweepPtr &rhs) const
{
  return lhs.ptr->getFixedAngleDeg() < rhs.ptr->getFixedAngleDeg();
}

void RadxVol::sortSweepsByFixedAngle()
  
{

  // sanity check
  
  if (_sweeps.size() < 2) {
    return;
  }

  // create set with sweep pointers sorted by fixed angle

  multiset<SweepPtr, SortByFixedAngle> sortedSweepPtrs;
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    SweepPtr sptr(_sweeps[isweep]);
    sortedSweepPtrs.insert(sptr);
  }

  // create ray vector reordered by the sorted sweeps

  vector<RadxSweep *> sortedSweeps;
  vector<RadxRay *> sortedRays;
  for (multiset<SweepPtr, SortByFixedAngle>::iterator ii = sortedSweepPtrs.begin();
       ii != sortedSweepPtrs.end(); ii++) {
    RadxSweep *sweep = ii->ptr;
    sortedSweeps.push_back(sweep);
    for (size_t iray = sweep->getStartRayIndex(); 
         iray <= sweep->getEndRayIndex(); iray++) {
      sortedRays.push_back(_rays[iray]);
    }
  }
  
  // replace original with sorted

  _rays = sortedRays;

  // load sweep info from rays

  checkForIndexedRays();
  loadSweepInfoFromRays();
  
}

/////////////////////////////////////////////////////////////////
// Sort rays in each sweep by azimuth
// Assumes sweep info is already set

bool RadxVol::SortByRayAzimuth::operator()
  (const RayPtr &lhs, const RayPtr &rhs) const
{
  return lhs.ptr->getAzimuthDeg() < rhs.ptr->getAzimuthDeg();
}

void RadxVol::sortSweepRaysByAzimuth()
  
{
  
  // sanity check
  
  if (_rays.size() < 2) {
    return;
  }

  // loop through sweeps

  vector<RadxRay *> sortedRays;
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    
    RadxSweep *sweep = _sweeps[isweep];
    
    // create set with sorted ray pointers for this sweep
    
    set<RayPtr, SortByRayAzimuth> sortedRayPtrs;
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RayPtr rptr(_rays[iray]);
      sortedRayPtrs.insert(rptr);
    }
    
    // add sortedRays array in az-sorted order
    
    for (set<RayPtr, SortByRayAzimuth>::iterator ii = sortedRayPtrs.begin();
         ii != sortedRayPtrs.end(); ii++) {
      sortedRays.push_back(ii->ptr);
    }
    
  } // isweep

  // set _rays to sorted vector

  _rays = sortedRays;

}

////////////////////////////////////////////////////////////
/// set or add frequency or wavelength
/// The set methods clear the list first, and then add the value
/// The add methods do not clear the list first

void RadxVol::setFrequencyHz(double val)
{
  _platform.setFrequencyHz(val);
}

void RadxVol::setWavelengthM(double val)
{
  _platform.setWavelengthM(val);
}

void RadxVol::setWavelengthCm(double val)
{
  _platform.setWavelengthCm(val);
}

void RadxVol::addFrequencyHz(double val)
{
  _platform.addFrequencyHz(val);
}

void RadxVol::addWavelengthM(double val)
{
  _platform.addWavelengthM(val);
}

void RadxVol::addWavelengthCm(double val)
{
  _platform.addWavelengthCm(val);
}

////////////////////////////////////////////////////////////
// get wavelength

double RadxVol::getWavelengthM() const
{
  return _platform.getWavelengthM();
}

double RadxVol::getWavelengthCm() const
{
  return _platform.getWavelengthCm();
}

////////////////////////////////////////////////////////////
// get sweep by sweep number (not the index)
// returns NULL on failure

const RadxSweep *RadxVol::getSweepByNumber(int sweepNum) const

{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      return _sweeps[ii];
    }
  } // ii
  return NULL;
}

RadxSweep *RadxVol::getSweepByNumber(int sweepNum)
{
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    if (_sweeps[ii]->getSweepNumber() == sweepNum) {
      return _sweeps[ii];
    }
  } // ii
  return NULL;
}

////////////////////////////////////////////////////////////
// get sweep by fixed angle
// returns sweep closest to fixed angle
// returns NULL on failure

const RadxSweep *RadxVol::getSweepByFixedAngle(double requestedAngle) const
  
{
  if (_sweeps.size() < 1) {
    return NULL;
  }
  double minDiff = 9999.0;
  int sweepNum = -1;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    const RadxSweep *sweep = _sweeps[ii];
    double diff = 
      fabs(Radx::computeAngleDiff(requestedAngle, sweep->getFixedAngleDeg()));
    if (diff < minDiff) {
      minDiff = diff;
      sweepNum = ii;
    }
  } // ii
  if (sweepNum < 0) {
    sweepNum = 0;
  }
  return getSweepByNumber(sweepNum);
}

RadxSweep *RadxVol::getSweepByFixedAngle(double requestedAngle) 
  
{
  if (_sweeps.size() < 1) {
    return NULL;
  }
  double minDiff = 9999.0;
  int sweepNum = -1;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    RadxSweep *sweep = _sweeps[ii];
    double diff = 
      fabs(Radx::computeAngleDiff(requestedAngle, sweep->getFixedAngleDeg()));
    if (diff < minDiff) {
      minDiff = diff;
      sweepNum = ii;
    }
  } // ii
  if (sweepNum < 0) {
    sweepNum = 0;
  }
  return getSweepByNumber(sweepNum);
}

//////////////////////////////////////////////////////////////////////////
/// check if all rays in a sweep are in an antenna transition

bool RadxVol::checkAllSweepRaysInTransition(const RadxSweep *sweep) const

{
  int startIndex = sweep->getStartRayIndex();
  int endIndex = sweep->getEndRayIndex();
  for (int ii = startIndex; ii <= endIndex; ii++) {
    const RadxRay &ray = *_rays[ii];
    if (!ray.getAntennaTransition()) {
      return false;
    }
  }
  return true;
}
 
bool RadxVol::checkAllSweepRaysInTransition(int sweepNum) const

{
  const RadxSweep *sweep = getSweepByNumber(sweepNum);
  if (sweep == NULL) {
    return false;
  }
  return checkAllSweepRaysInTransition(sweep);
}

///////////////////////////////////////////////////////
// internal implementation of sweep constraint

void RadxVol::_constrainBySweepIndex(vector<int> &sweepIndexes)

{

  if (sweepIndexes.size() < 1) {
    return;
  }

  // ensure we always return 1

  if (_sweeps.size() < 2) {
    return;
  }
  
  // sort the vector
  
  if (sweepIndexes.size() > 1) {
    sort(sweepIndexes.begin(), sweepIndexes.end());
  }

  // get min and max sweep indexes

  int minSweepIndex = sweepIndexes[0];
  int maxSweepIndex = sweepIndexes[sweepIndexes.size()-1];

  if (minSweepIndex == 0 &&
      maxSweepIndex == (int) (_sweeps.size() - 1)) {
    // nothing to do - all sweeps are needed
    return;
  }

  // get min and max ray indexes

  size_t minRayIndex = _sweeps[minSweepIndex]->getStartRayIndex();
  size_t maxRayIndex = _sweeps[maxSweepIndex]->getEndRayIndex();

  // remap field data

  //   for (int ii = 0; ii < (int) _fields.size(); ii++) {
  //     _fields[ii]->remapRays(minRayIndex, maxRayIndex);
  //   }
  
  // remap ray vector

  vector<RadxRay *> goodRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (ii >= minRayIndex && ii <= maxRayIndex) {
      goodRays.push_back(_rays[ii]);
    } else {
      RadxRay::deleteIfUnused(_rays[ii]);
    }
  }
  _rays = goodRays;

  // load up sweep info from the revised list of rays

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

///////////////////////////////////////////////////////
/// remove rays with all missing data

void RadxVol::removeRaysWithDataAllMissing()

{

  vector<RadxRay *> goodRays;
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_rays[ii]->checkDataAllMissing()) {
      RadxRay::deleteIfUnused(_rays[ii]);
    } else {
      goodRays.push_back(_rays[ii]);
    }
  }
  _rays = goodRays;

  // load up sweep info from the revised list of rays

  loadSweepInfoFromRays();
  loadVolumeInfoFromRays();

}

////////////////////////////////////////////////////////////
// Determine whether rays are indexed in angle, and what
// the predominant angular resolution is.
//
// This is performed by sweep.

bool RadxVol::checkForIndexedRays() const

{
  bool allIndexed = true;
  for (size_t ii = 0; ii < _sweeps.size(); ii++) {
    _checkForIndexedRays(_sweeps[ii]);
    if (!_sweeps[ii]->getRaysAreIndexed()) {
      allIndexed = false;
    }
  } // ii
  return allIndexed;
}

void RadxVol::_checkForIndexedRays(const RadxSweep *sweep) const

{

  // get sweep mode

  Radx::SweepMode_t mode = sweep->getSweepMode();
  bool isRhi = false;
  if (mode == Radx::SWEEP_MODE_RHI ||
      mode == Radx::SWEEP_MODE_MANUAL_RHI) {
    isRhi = true;
  }
  
  // compute histogram of angular differences
  // from 0 to 10 degrees, at 0.01 degree resolution

  double res = 0.005;
  int nn = 2000;
  int startIndex = sweep->getStartRayIndex();
  int endIndex = sweep->getEndRayIndex();
  int nRays = endIndex - startIndex + 1;

  int *hist = new int[nn];
  memset(hist, 0, nn * sizeof(int));
  int count = 0;

  for (int ii = startIndex; ii < endIndex; ii++) {
    const RadxRay &ray0 = *_rays[ii];
    const RadxRay &ray1 = *_rays[ii+1];
    double diff = 0;
    if (isRhi) {
      diff = fabs(ray1.getElevationDeg() - ray0.getElevationDeg());
    } else {
      diff = fabs(ray1.getAzimuthDeg() - ray0.getAzimuthDeg());
    }
    if (diff > 180) {
      diff = fabs(diff - 360);
    }
    int index = (int) (diff / res + 0.5);
    if (index >= 0 && index < nn) {
      hist[index]++;
      count++;
    }
  }

  // find index for mode

  int modeIndex = -1;
  int maxHist = 0;
  for (int ii = 0; ii < nn; ii++) {
    int histVal = hist[ii];
    if (histVal > maxHist) {
      modeIndex = ii;
      maxHist = histVal;
    }
  }
  
  // calculate the angular resolution of the mode

  double modeRes = modeIndex * res;
  
  // count up number of ray pairs with diffs within 0.05 degrees of
  // of the mode

  int modeCount = 0;
  for (int ii = modeIndex - 10; ii < modeIndex + 10; ii++) {
    if (ii >= 0 && ii < nn) {
      modeCount += hist[ii];
    }
  }

  // are more than 90% within 0.05 degrees of the mode

  double fraction = (double) modeCount / ((double) nRays - 1);
  bool isIndexed = false;
  if (fraction > 0.9) {
    isIndexed = true;
  }

  // set ray meta-data

  double roundedRes = _computeRoundedAngleRes(modeRes);

  for (int ii = startIndex; ii <= endIndex; ii++) {
    RadxRay &ray = *_rays[ii];
    ray.setIsIndexed(isIndexed);
    ray.setAngleResDeg(roundedRes);
  }

  // clean up

  delete[] hist;

}

/////////////////////////////////////////////////
// data type conversions

void RadxVol::convertToFl64()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl64();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToFl64();
    }
  }
}

void RadxVol::convertToFl32()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToFl32();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToFl32();
    }
  }
}

void RadxVol::convertToSi32()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi32();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi32();
    }
  }
}

void RadxVol::convertToSi32(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi32(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi32(scale, offset);
    }
  }
}

void RadxVol::convertToSi16()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi16();
    }
  }
}

void RadxVol::convertToSi16(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi16(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi16(scale, offset);
    }
  }
}

void RadxVol::convertToSi08()
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi08();
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi08();
    }
  }
}

void RadxVol::convertToSi08(double scale, double offset)
{
  // check if fields are managed by the vol or the rays
  if (_fields.size() > 0) {
    // managed by vol
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      _fields[ii]->convertToSi08(scale, offset);
    }
    setRayFieldPointers();
  } else {
    // managed by rays
    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertToSi08(scale, offset);
    }
  }
}

void RadxVol::convertToType(Radx::DataType_t targetType)
{
  for (size_t ii = 0; ii < _fields.size(); ii++) {
    _fields[ii]->convertToType(targetType);
  }
  setRayFieldPointers();
}

////////////////////////////////////////////////////////////////
/// Apply a linear transformation to the data values in a field.
/// Transforms x to y as follows:
///   y = x * scale + offset
/// After operation, field type is unchanged.
/// Nothing is done if field does not exist.

void RadxVol::applyLinearTransform(const string &name,
                                   double scale, double offset)

{
  
  if (_fields.size() > 0) {

    // fields are contiguous - i.e. not in rays

    RadxField *field = getField(name);
    if (field) {
      field->applyLinearTransform(scale, offset);
    }
    
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->applyLinearTransform(name, scale, offset);
    }
    
  } // if (_fields.size() > 0) {

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

void RadxVol::convertField(const string &name,
                           Radx::DataType_t type,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  if (_fields.size() > 0) {

    // fields are contiguous

    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField &field = *_fields[ii];
      if (field.getName() == name) {
        field.convert(type, newName, units, standardName, longName);
      }
    }
  
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertField(name, type, 
                              newName, units,
                              standardName, longName);
    }
    
  } // if (_fields.size() > 0) {

}

//////////////////////////////////////////////////////
/// Converts field type, and optionally changes the
/// names.
///
/// For integer types, the specified scale and offset
/// are used.
///
/// If targetType is Radx::ASIS, no conversion is performed.
///
/// If a string is empty, the value on the field will
/// be left unchanged.

void RadxVol::convertField(const string &name,
                           Radx::DataType_t type,
                           double scale,
                           double offset,
                           const string &newName,
                           const string &units,
                           const string &standardName,
                           const string &longName)

{

  if (_fields.size() > 0) {

    // fields are contiguous

    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField &field = *_fields[ii];
      if (field.getName() == name) {
        field.convert(type, scale, offset,
                      newName, units, standardName, longName);
      }
    }
  
  } else {

    // fields are on rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      _rays[ii]->convertField(name, type, scale, offset,
                              newName, units,
                              standardName, longName);
    }
    
  } // if (_fields.size() > 0) {

}

///////////////////////////////////////////////////////////////////
/// compute field stats for rays currently in the volume
///
/// Pass in a stats method type, and a vector of fields
///
/// The requested stats on computed for each field,
/// and on a point-by-point basis.
///
/// If the geometry is not constant, remap to the predominant geom.
///
/// maxFractionMissing indicates the maximum fraction of the input data field
/// that can be missing for valid statistics. Should be between 0 and 1.
/// 
/// Returns NULL if no rays are present in the volume.
/// Otherwise, returns ray containing results.

RadxRay *RadxVol::computeFieldStats(RadxField::StatsMethod_t method,
                                    double maxFractionMissing /* = 0.25 */)

{

  // check we have some data

  if (_rays.size() == 0) {
    return NULL;
  }

  // remap rays to predominant geometry

  remapToPredomGeom();

  // find middle ray, copy the metadata

  size_t iMid = _rays.size() / 2;
  RadxRay *result = new RadxRay;
  result->copyMetaData(*_rays[iMid]);

  // compute and set the number of samples

  int nSamplesSum = 0;
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    nSamplesSum += _rays[iray]->getNSamples();
  }
  result->setNSamples(nSamplesSum);
  
  // get the field name list, and loop through them
  
  vector<string> fieldNames = getUniqueFieldNameList();
  for (size_t ifield = 0; ifield < fieldNames.size(); ifield++) {

    // assemble vector of this field on the ray

    RadxField *field = _rays[0]->getField(fieldNames[ifield]);

    vector<const RadxField *> rayFields;
    for (size_t iray = 0; iray < _rays.size(); iray++) {
      RadxField *rayField = _rays[iray]->getField(fieldNames[ifield]);
      if (rayField != NULL) {
        rayFields.push_back(rayField);
      }
    }

    // compute the stats for this field
    // add field to ray

    RadxField *statsField = 
      field->computeStats(method, rayFields, maxFractionMissing);
    if (statsField != NULL) {
      result->addField(statsField);
    }

  } // ifield

  // return resulting ray

  return result;
  
}

//////////////////////////////////////////////////////
// if possible, find a rounded angle resolution evenly
// divisible into 360.0

double RadxVol::_computeRoundedAngleRes(double res) const
{

  static double canonAngles[36] =
    {
      0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.075,
      0.1, 0.12, 0.125, 0.15, 0.18, 0.2, 0.25, 0.3, 0.36,
      0.4, 0.45, 0.5, 0.6, 0.75, 0.8, 0.9, 1.0, 1.2, 1.25,
      1.5, 1.5, 1.8, 2.0, 2.5, 3.6, 4.0, 4.5, 5.0, 6.0
    };

  if (res < 0.01 || res > 6.0) {
    return res;
  }

  double minDiff = 9999.0;
  double canon = res;
  for (int ii = 0; ii < 36; ii++) {
    double diff = fabs(res - canonAngles[ii]);
    if (diff < minDiff) {
      canon = canonAngles[ii];
      minDiff = diff;
    }
  }

  return canon;

}

//////////////////////////////////////////////////////////////////
/// get the predominant sweep mode from checking the angles

Radx::SweepMode_t RadxVol::getPredomSweepModeFromAngles() const

{

  if (!_sweepModeFromAnglesChecked) {
    _setPredomSweepModeFromAngles();
  }
  
  return _predomSweepModeFromAngles;

}

/////////////////////////////////////////////////////
// set predominant sweep mode by checking the angles
// only detects RHI, SURVEILLANCE or SECTOR

void RadxVol::_setPredomSweepModeFromAngles() const
{
  
  if (_sweepModeFromAnglesChecked) {
    return;
  }

  // sanity check
  
  if (_rays.size() < 2) {
    _predomSweepModeFromAngles = _rays[0]->getSweepMode();
    _sweepModeFromAnglesChecked = true;
    return;
  }

  // check for RHI first
    
  if (RadxAngleHist::checkIsRhi(_rays)) {
    _predomSweepModeFromAngles = Radx::SWEEP_MODE_RHI;
    _sweepModeFromAnglesChecked = true;
    return;
  }

  // get start and end ray indices for sweeps
  // this is a shortened version of loadSweepInfoFromRays()
  // but is const
  
  vector<RadxSweep> sweeps;
  RadxSweep sweep;
  sweep.setSweepNumber(-9999);
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    const RadxRay &ray = *_rays[iray];
    int sweepNum = ray.getSweepNumber();
    if (sweepNum != sweep.getSweepNumber()) {
      if (iray != 0) {
        sweeps.push_back(sweep);
      }
      sweep.setSweepNumber(sweepNum);
      sweep.setStartRayIndex(iray);
      sweep.setEndRayIndex(iray);
    } else {
      sweep.setEndRayIndex(iray);
    }
  }
  sweeps.push_back(sweep);

  // check for any sweep with an azimuth covered of 345 or more
  
  for (size_t isweep = 0; isweep < sweeps.size(); isweep++) {
    const RadxSweep &sweep = sweeps[isweep];
    double azCovered = computeAzCovered(sweep.getStartRayIndex(),
                                        sweep.getEndRayIndex());
    if (azCovered >= 345.0) {
      _predomSweepModeFromAngles = Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE;
      _sweepModeFromAnglesChecked = true;
      return;
    }
  }
  
  // not rhi or surveillance, set to sector

  _predomSweepModeFromAngles = Radx::SWEEP_MODE_SECTOR;
  _sweepModeFromAnglesChecked = true;
  return;

}

/////////////////////////////////////////////////////
// check whether volume is predominantly in RHI mode
//
// Returns true if RHI, false otherwise
//
// Also sets 

bool RadxVol::checkIsRhi() const
{

  if (getPredomSweepModeFromAngles() == Radx::SWEEP_MODE_RHI) {
    return true;
  } else {
    return false;
  }
  
}

//////////////////////////////////////////////////////////////
// check whether a series of rays is predominantly in RHI mode
// using the angles
// Returns true if RHI, false otherwise

bool RadxVol::checkIsRhi(size_t startRayIndex,
                         size_t endRayIndex)
{

  vector<RadxRay *> rayList;
  for (size_t ii = startRayIndex; ii <= endRayIndex; ii++) {
    rayList.push_back(_rays[ii]);
  }
         
  return RadxAngleHist::checkIsRhi(rayList);

}

///////////////////////////////////////////////////////
/// check if rays are predominantly in
/// SURVEILLANCE mode i.e. 360's azimuth rotation
/// Returns true if surveillance, false otherwise

bool RadxVol::checkIsSurveillance() const
{
  
  if (getPredomSweepModeFromAngles() ==
      Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
    return true;
  } else {
    return false;
  }
  
}

///////////////////////////////////////////////////////
/// check if sequence of rays are predominantly in
/// SURVEILLANCE mode i.e. 360's azimuth rotation
/// Returns true if surveillance, false otherwise

bool RadxVol::checkIsSurveillance(size_t startRayIndex,
                                  size_t endRayIndex) const
{
  
  double azCovered = computeAzCovered(startRayIndex, endRayIndex);
  if (azCovered >= 345.0) {
    return true;
  }
  
  return false;

}

///////////////////////////////////////////////////////
/// check if rays are predominantly in SECTOR mode.
/// Returns true if surveillance, false otherwise

bool RadxVol::checkIsSector() const
{
  
  if (getPredomSweepModeFromAngles() ==
      Radx::SWEEP_MODE_SECTOR) {
    return true;
  } else {
    return false;
  }
  
}

///////////////////////////////////////////////////////
// Compute the azimuth swept out by a sweep
// Returns the azimuth covered.

double RadxVol::computeAzCovered(const RadxSweep *sweep) const
{
  size_t startIndex = sweep->getStartRayIndex();
  size_t endIndex = sweep->getEndRayIndex();
  return computeAzCovered(startIndex, endIndex);
}
  
///////////////////////////////////////////////////////
// Compute the azimuth swept out by sequence of rays
// Returns the azimuth covered.

double RadxVol::computeAzCovered(size_t startRayIndex,
                                 size_t endRayIndex) const
{

  if (endRayIndex <= startRayIndex) {
    return 0.0;
  }
  
  // sum up azimuth covered by the sweep
  
  double count = 0.0;
  double sumDeltaAz = 0.0;
  double prevAz = _rays[startRayIndex]->getAzimuthDeg();

  for (size_t ii = startRayIndex + 1; ii <= endRayIndex; ii++) {
    double az = _rays[ii]->getAzimuthDeg();
    double deltaAz = Radx::conditionAngleDelta(az - prevAz);
    sumDeltaAz += fabs(deltaAz);
    count++;
    prevAz = az;
  } // ii

  // account for the width of the end rays

  double meanDeltaAz = sumDeltaAz / count;
  sumDeltaAz += meanDeltaAz;

  // return

  return sumDeltaAz;

}

/////////////////////////////////////////////////////
/// check whether start_range and gate_spacing varies per ray
///
/// Returns true if gate geom varies by ray, false otherwise

bool RadxVol::gateGeomVariesByRay() const

{

  if (_rays.size() < 2) {
    return false;
  }

  double startRange0 = _rays[0]->getStartRangeKm();
  double gateSpacing0 = _rays[0]->getGateSpacingKm();

  for (size_t ii = 1; ii < _rays.size(); ii++) {
    
    if (_rays[ii]->getStartRangeKm() != startRange0) {
      return true;
    }
    
    if (_rays[ii]->getGateSpacingKm() != gateSpacing0) {
      return true;
    }

  }

  return false;

}

/////////////////////////////////////////////////////////////
// combine rays from sweeps with common fixed angle and
// gate geometry, but with different fields

void RadxVol::combineSweepsAtSameFixedAngleAndGeom
  (bool keepLongRange /* = false */)
  
{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  // make sure sweep info is up to date

  loadSweepInfoFromRays();

  // find sweeps that should be combined

  vector<Combo> combos;
  set<int> sources;

  for (int ii = (int) _sweeps.size() - 1; ii > 0; ii--) {
    
    RadxSweep *sweep1 = _sweeps[ii];
    RadxRay *ray1 = _rays[sweep1->getStartRayIndex()];
    Combo combo(ii);
    
    for (int jj = ii - 1; jj >= 0; jj--) {
      
      RadxSweep *sweep0 = _sweeps[jj];
      RadxRay *ray0 = _rays[sweep0->getStartRayIndex()];
      
      if ((fabs(sweep0->getFixedAngleDeg() -
                sweep1->getFixedAngleDeg()) < 0.001) &&
          (fabs(ray0->getStartRangeKm() -
                ray1->getStartRangeKm()) < 0.001) &&
          (fabs(ray0->getGateSpacingKm() -
                ray1->getGateSpacingKm()) < 0.001)) {
        
        if (sources.find(jj) == sources.end()) {
          // source sweep not previously used
          sources.insert(jj);
          combo.sources.push_back(jj);
        }

      }
      
    } // jj
    
    combos.push_back(combo);
    
  } // ii
  
  // combine the data from the sweeps

  for (size_t ii = 0; ii < combos.size(); ii++) {
    const Combo &combo = combos[ii];
    for (size_t jj = 0; jj < combo.sources.size(); jj++) {
      _augmentSweepFields(combo.target, combo.sources[jj]);
    }
  }

  // select the rays to keep
  
  vector<RadxRay *> keepRays;
  for (int ii = 0; ii < (int) _sweeps.size(); ii++) {
    if (sources.find(ii) == sources.end()) {
      // not a source, so keep these rays
      RadxSweep *sweepTarget = _sweeps[ii];
      for (size_t kk = sweepTarget->getStartRayIndex();
           kk <= sweepTarget->getEndRayIndex(); kk++) {
        keepRays.push_back(_rays[kk]);
      }
    } else {
      // discard the source sweeps if no longer needed
      RadxSweep *sweepSource = _sweeps[ii];
      for (size_t kk = sweepSource->getStartRayIndex();
           kk <= sweepSource->getEndRayIndex(); kk++) {
        if (keepLongRange && _rays[kk]->getIsLongRange()) {
          keepRays.push_back(_rays[kk]);
        } else {
          RadxRay::deleteIfUnused(_rays[kk]);
        }
      }
    }
  } // ii

  _rays = keepRays;

  // reload the sweep info

  loadSweepInfoFromRays();

}

////////////////////////////////////////////////////////
/// Augment fields in a sweep, by copying in fields from
/// another sweep

void RadxVol::_augmentSweepFields(size_t targetIndex, size_t sourceIndex)
{

  RadxSweep *sweepTarget = _sweeps[targetIndex];
  RadxSweep *sweepSource = _sweeps[sourceIndex];

  _setupAngleSearch(sourceIndex);

  // check sweep mode

  Radx::SweepMode_t sweepMode = sweepTarget->getSweepMode();
  if (sweepMode != sweepSource->getSweepMode()) {
    // sweep modes do not match
    return;
  }
  
  // loop through the target rays
  
  for (size_t iray = sweepTarget->getStartRayIndex();
       iray <= sweepTarget->getEndRayIndex(); iray++) {
    
    RadxRay *rayTarget = _rays[iray];
    double angle = rayTarget->getAzimuthDeg();
    if (sweepMode == Radx::SWEEP_MODE_RHI) {
      angle = rayTarget->getElevationDeg();
    }

    // get the source ray

    int angleIndex = _getSearchAngleIndex(angle);
    const RadxRay *raySource = _searchRays[angleIndex];
    if (raySource != NULL) {

      // got a valid ray in source
      // loop through the fields in the source
      
      for (size_t ifield = 0; ifield < raySource->getNFields(); ifield++) {
        
        const RadxField *fldSource = raySource->getField(ifield);
        
        // make a copy of the field
        
        RadxField *copy = new RadxField(*fldSource);

        // does this field exist in the target?
        
        RadxField *fldTarget = rayTarget->getField(fldSource->getName());
        
        if (fldTarget != NULL) {
          // field already exists on the target, so modify its name
          // based on the sweep number
          char newName[128];
          sprintf(newName, "%s-s%d",
                  fldTarget->getName().c_str(), (int) targetIndex);
          fldTarget->setName(newName);
        }
        
        // add field to target ray
        
        rayTarget->addField(copy);
        
      } // ifield
      
    } // if (raySource != NULL)

  } // iray


}

////////////////////////////////////////////////////////
/// Make fields uniform in the volume.
/// This ensures that all rays in the volume have the same fields
/// and that they are in the same order in each ray.
/// If fields a missing from a ray, a suitable field is added
/// containing missing data.

void RadxVol::makeFieldsUniform()

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  // make uniform

  _makeFieldsUniform(0, _rays.size() - 1);

}

////////////////////////////////////////////////////////
/// Make fields uniform for each sweep.
/// This ensures that all rays in a sweep have the same fields.
/// and that they are in the same order in each ray.
/// If fields a missing from a ray, a suitable field is added
/// containing missing data.

void RadxVol::makeFieldsUniformPerSweep()

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {

    const RadxSweep *sweep = _sweeps[isweep];
    size_t startIndex = sweep->getStartRayIndex();
    size_t endIndex = sweep->getEndRayIndex();
    
    _makeFieldsUniform(startIndex, endIndex);
    
  } // isweep

}

void RadxVol::_makeFieldsUniform(size_t startIndex, size_t endIndex)

{

  // create field template by searching through the rays
  // for examples of all the field names
  
  set<string> nameSet;
  vector<const RadxField *> tplate;
  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    const RadxRay &ray = *_rays[iray];
    for (size_t ifield = 0; ifield < ray.getNFields(); ifield++) {
      const RadxField *fld = ray.getField(ifield);
      string name = fld->getName();
      pair<set<string>::const_iterator, bool> ret = nameSet.insert(name);
      if (ret.second == true) {
        // field name not previously in set, so add field to template
        tplate.push_back(fld);
      }
    }
  }
  
  // make rays match the field template
  
  for (size_t iray = startIndex; iray <= endIndex; iray++) {
    _rays[iray]->makeFieldsMatchTemplate(tplate);
  }
  
}

////////////////////////////////////////////////////////
/// Reorder the fields, by name, removing any extra fields.

void RadxVol::reorderFieldsByName(const vector<string> &names)

{

  // ensure fields are owned by rays

  loadRaysFromFields();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->reorderFieldsByName(names);
  }

}

////////////////////////////////////////////////////////
/// Remove a specifed field
/// Returns 0 on success, -1 on failure

int RadxVol::removeField(const string &name)
  
{
  
  int iret = 0;

  if (_fields.size()) {
    
    // fields are contiguous

    // copy good fields to temp array

    vector<RadxField *> temp;
    for (size_t ii = 0; ii < _fields.size(); ii++) {
      RadxField *field = _fields[ii];
      if (field->getName() == name) {
        delete field;
      } else {
        temp.push_back(field);
      }
    } // ii

    if (_fields.size() == temp.size()) {
      // field not found
      iret = -1;
    }

    _fields = temp;

  } else {

    // fields are managed by rays

    for (size_t ii = 0; ii < _rays.size(); ii++) {
      if (_rays[ii]->removeField(name)) {
        return iret;
      }
    }

  }

  return iret;

}

////////////////////////////////////////////////////////
/// Set the latitude of the platform in degrees.
///
/// Used for non-mobile platforms.

void RadxVol::setLatitudeDeg(double val) 
{

  _platform.setLatitudeDeg(val);

}

////////////////////////////////////////////////////////
/// Set the longitude of the platform in degrees.
///
/// Used for non-mobile platforms.

void RadxVol::setLongitudeDeg(double val) 
{

  _platform.setLongitudeDeg(val);

}

////////////////////////////////////////////////////////
/// Set the altitude of the platform in km.
///
/// Used for non-mobile platforms.

void RadxVol::setAltitudeKm(double val) 
{

  _platform.setAltitudeKm(val);

}

////////////////////////////////////////////////////////
/// Set the sensor ht above the surface

void RadxVol::setSensorHtAglM(double val) 
{

  _platform.setSensorHtAglM(val);

}

////////////////////////////////////////////////
/// set the radar location

void RadxVol::setLocation(double latitudeDeg,
                          double longitudeDeg,
                          double altitudeKm)

{

  _platform.setLatitudeDeg(latitudeDeg);
  _platform.setLongitudeDeg(longitudeDeg);
  _platform.setAltitudeKm(altitudeKm);

}

////////////////////////////////////////////////
/// override the radar location
/// this also sets the location in any georeference objects
/// attached to the rays

void RadxVol::overrideLocation(double latitudeDeg,
                               double longitudeDeg,
                               double altitudeKm)

{
  
  setLocation(latitudeDeg, longitudeDeg, altitudeKm);
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxGeoref *georef = _rays[ii]->getGeoreference();
    if (georef) {
      georef->setLatitude(latitudeDeg);
      georef->setLongitude(longitudeDeg);
      georef->setAltitudeKmMsl(altitudeKm);
    }
  }

}

////////////////////////////////////////////////
/// set the radar location from the start ray
/// provided georefs are active

void RadxVol::setLocationFromStartRay()

{

  if (_rays.size() < 1) {
    return;
  }

  const RadxRay *ray0 = _rays[0];
  const RadxGeoref *georef = ray0->getGeoreference();
  if (georef) {
    _platform.setLatitudeDeg(georef->getLatitude());
    _platform.setLongitudeDeg(georef->getLongitude());
    _platform.setAltitudeKm(georef->getAltitudeKmMsl());
  }

}

////////////////////////////////////////////////
/// override the radar ht AGL
/// this also sets the location in any georeference objects
/// attached to the rays

void RadxVol::overrideSensorHtAglM(double val)

{
  
  setSensorHtAglM(val);
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    RadxGeoref *georef = _rays[ii]->getGeoreference();
    if (georef) {
      georef->setAltitudeKmAgl(val / 1000.0);
    }
  }

}

////////////////////////////////////////////////////////
/// apply the georeference corrections for moving platforms
/// to compute the earth-relative azimuth and elevation
///
/// If force is true, the georefs are always applied.
/// If force is false, the georefs are applied only if
/// they have not been applied previously.

void RadxVol::applyGeorefs(bool force /* = true */)

{

  for (size_t ii = 0; ii < _rays.size(); ii++) {
    if (_cfactors) {
      _rays[ii]->setCfactors(*_cfactors);
    }
    _rays[ii]->applyGeoref(_platform.getPrimaryAxis());
  }

}

//////////////////////////////////////////////////////////
/// count the number of rays in which each georef element
/// is not missing

void RadxVol::countGeorefsNotMissing(RadxGeoref &count) const

{

  count.setToZero();
  
  for (size_t ii = 0; ii < _rays.size(); ii++) {
    _rays[ii]->incrementGeorefNotMissingCount(count);
  }

}

///////////////////////////////////////////////////////////////
/// Load up pseudo RHIs, by analyzing the rays in the volume.
/// Only relevant for surveillance and sector ppi-type volumes.
/// Returns 0 on success
/// Returns -1 on error - i.e. if not ppi-type scan.
/// After success, you can call getPseudoRhis().

int RadxVol::loadPseudoRhis()

{

  // initialize

  clearPseudoRhis();

  if (checkIsRhi()) {
    return _loadPseudoFromRealRhis();
  }

  // check scan type

  Radx::SweepMode_t sweepMode = getPredomSweepModeFromAngles();
  if (sweepMode != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE &&
      sweepMode != Radx::SWEEP_MODE_SECTOR) {
    if (_debug) {
      cerr << "WARNING - RadxVol::loadPseudoRhis()" << endl;
      cerr << "  Sweep mode invalid: " << Radx::sweepModeToStr(sweepMode) << endl;
    }
    return -1;
  }

  // trim surveillance to 360 deg sweeps

  if (sweepMode != Radx::SWEEP_MODE_AZIMUTH_SURVEILLANCE) {
    trimSurveillanceSweepsTo360Deg();
  }

  // get the sweep with lowest elevation angle
  
  size_t lowSweepIndex = 0;
  const RadxSweep *lowSweep = _sweeps[0];
  double minElev = lowSweep->getFixedAngleDeg();
  for (size_t isweep = 1; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];
    double elev = sweep->getFixedAngleDeg();
    if (elev < minElev) {
      lowSweep = sweep;
      lowSweepIndex = isweep;
      minElev = elev;
    }
  }
  if (lowSweep->getNRays() < 10) {
    if (_debug) {
      cerr << "WARNING - RadxVol::loadPseudoRhis()" << endl;
      cerr << "  Low sweep has too few rays, nRays: lowSweep->getNRays()" << endl;
      cerr << "  Cannot determine pseudo RHIs" << endl;
    }
    return -1;
  }

  // compute mean delta azimuth for low sweep

  size_t lowSweepStartRayIndex = lowSweep->getStartRayIndex();
  size_t lowSweepEndRayIndex = lowSweep->getEndRayIndex();
  double prevAz = _rays[lowSweepStartRayIndex]->getAzimuthDeg();
  double sumDeltaAz = 0.0;
  double count = 0.0;

  for (size_t iray = lowSweepStartRayIndex + 1; iray <= lowSweepEndRayIndex; iray++) {
    double az = _rays[iray]->getAzimuthDeg();
    double deltaAz = fabs(az - prevAz);
    if (deltaAz > 180.0) {
      deltaAz = fabs(deltaAz - 360.0);
    }
    sumDeltaAz += deltaAz;
    count++;
    prevAz = az;
  } // iray
  double meanDeltaAz = sumDeltaAz / count;

  // compute azimuth margin for finding RHI rays

  double azMargin = meanDeltaAz * 2.5;
  
  // go through the low sweep, adding rays to pseudo RHIs
  
  for (size_t iray = lowSweepStartRayIndex; iray <= lowSweepEndRayIndex; iray++) {
    RadxRay *lowRay = _rays[iray];
    PseudoRhi *rhi = new PseudoRhi;
    rhi->addRay(lowRay);
    _pseudoRhis.push_back(rhi);
    for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
      if (isweep == lowSweepIndex) {
        // low sweep, ignore this one, already added
        continue;
      }
      RadxSweep *sweep = _sweeps[isweep];
      RadxRay *bestRay = NULL;
      double minDeltaAz = 9999.0;
      for (size_t jray = sweep->getStartRayIndex(); 
           jray <= sweep->getEndRayIndex(); jray++) {
        RadxRay *ray = _rays[jray];
        double deltaAz = fabs(lowRay->getAzimuthDeg() - ray->getAzimuthDeg());
        if (deltaAz > 180.0) {
          deltaAz = fabs(deltaAz - 360.0);
        }
        if (deltaAz < azMargin && deltaAz < minDeltaAz) {
          bestRay = ray;
          minDeltaAz = deltaAz;
        }
      } // jray
      if (bestRay != NULL) {
        rhi->addRay(bestRay);
      }
    } // isweep;
    
  } // iray

  // sort the RHIs in elevation

  for (size_t ii = 0; ii < _pseudoRhis.size(); ii++) {
    _pseudoRhis[ii]->sortRaysByElevation();
  }

  return 0;

}

///////////////////////////////////////////////////////////////
/// Load up pseudo RHIs from a real RHI volume
/// The RHI sweeps are ordered in increasing elevation angle
/// Returns 0 on success, -1 on error.
/// After success, you can call getPseudoRhis().

int RadxVol::_loadPseudoFromRealRhis()

{

  // loop through the sweeps
  
  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    const RadxSweep *sweep = _sweeps[isweep];

    // create pseudo RHI from sweep rays
    // that are not in transition

    PseudoRhi *rhi = new PseudoRhi;
    for (size_t iray = sweep->getStartRayIndex();
         iray <= sweep->getEndRayIndex(); iray++) {
      RadxRay *ray = _rays[iray];
      if (!ray->getAntennaTransition()) {
        rhi->addRay(_rays[iray]);
      }
    }

    // set the rays by elevation

    rhi->sortRaysByElevation();

    // add to vector

    _pseudoRhis.push_back(rhi);
    
  } // isweep

  return 0;

}

///////////////////////////////////////////////////////////////
/// clear vector of pseudo RHIs

void RadxVol::clearPseudoRhis()

{

  for (size_t ii = 0; ii < _pseudoRhis.size(); ii++) {
    delete _pseudoRhis[ii];
  }
  _pseudoRhis.clear();

}

////////////////////////////////////////////////////////////////////
// Load up a 2D field fl32 array from a vector of rays.
// The ray data for the specified field will be converted to fl32.
// This is a static method, does not use any vol members.
//
// Returns 0 on success, -1 on failure

int RadxVol::load2DFieldFromRays(const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 RadxArray2D<Radx::fl32> &array,
                                 Radx::fl32 missingValue /* = -9999.0 */)
  
{

  // check field exists

  bool fieldFound = false;
  size_t maxNGates = 0;
  for (size_t iray = 0; iray < rays.size(); iray++) {
    const RadxRay *ray = rays[iray];
    if (ray->getField(fieldName) != NULL) {
      fieldFound = true;
      if (ray->getNGates() > maxNGates) {
        maxNGates = ray->getNGates();
      }
    }
  }

  if (!fieldFound) {
    cerr << "ERROR - RadxVol::load2DFieldFromRays()" << endl;
    cerr << "  Field not found: " << fieldName << endl;
    return -1;
  }

  // allocate array

  array.alloc(rays.size(), maxNGates);
  Radx::fl32 **data = array.dat2D();

  // initialize to missing

  for (size_t iray = 0; iray < rays.size(); iray++) {
    for (size_t igate = 0; igate < maxNGates; igate++) {
      data[iray][igate] = missingValue;
    }
  }

  // fill array in ray order
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    size_t nGates = ray->getNGates();
    RadxField *fld = ray->getField(fieldName);
    if (fld == NULL) {
      continue;
    }
    fld->convertToFl32();
    Radx::fl32 miss = fld->getMissingFl32();
    const Radx::fl32 *vals = fld->getDataFl32();
    for (size_t igate = 0; igate < nGates; igate++) {
      Radx::fl32 val = vals[igate];
      if (val != miss) {
        data[iray][igate] = val;
      }
    } // igate
  } // iray

  return 0;

}

////////////////////////////////////////////////////////////////////
// Load up a 2D field si32 array from a vector of rays
// The ray data for the specified field will be converted to si32.
// This is a static method, does not use any vol members.
// Returns 0 on success, -1 on failure

int RadxVol::load2DFieldFromRays(const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 RadxArray2D<Radx::si32> &array,
                                 Radx::si32 missingValue /* = -9999.0 */)
  
{

  // check field exists

  bool fieldFound = false;
  size_t maxNGates = 0;
  for (size_t iray = 0; iray < rays.size(); iray++) {
    const RadxRay *ray = rays[iray];
    if (ray->getField(fieldName) != NULL) {
      fieldFound = true;
      if (ray->getNGates() > maxNGates) {
        maxNGates = ray->getNGates();
      }
    }
  }

  if (!fieldFound) {
    cerr << "ERROR - RadxVol::load2DFieldFromRays()" << endl;
    cerr << "  Field not found: " << fieldName << endl;
    return -1;
  }

  // allocate array

  array.alloc(rays.size(), maxNGates);
  Radx::si32 **data = array.dat2D();

  // initialize to missing

  for (size_t iray = 0; iray < rays.size(); iray++) {
    for (size_t igate = 0; igate < maxNGates; igate++) {
      data[iray][igate] = missingValue;
    }
  }

  // fill array in ray order
  
  for (size_t iray = 0; iray < rays.size(); iray++) {
    RadxRay *ray = rays[iray];
    size_t nGates = ray->getNGates();
    RadxField *fld = ray->getField(fieldName);
    if (fld == NULL) {
      continue;
    }
    fld->convertToSi32();
    Radx::si32 miss = fld->getMissingSi32();
    const Radx::si32 *vals = fld->getDataSi32();
    for (size_t igate = 0; igate < nGates; igate++) {
      Radx::si32 val = vals[igate];
      if (val != miss) {
        data[iray][igate] = val;
      }
    } // igate
  } // iray

  return 0;

}
                             
//////////////////////////////////////////////////////////////
// Load up ray fields from 2D fl32 field array.
// This is a static method, does not use any vol members.
// Returns 0 on success, -1 on failure

int RadxVol::loadRaysFrom2DField(const RadxArray2D<Radx::fl32> &array,
                                 const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 const string &units,
                                 Radx::fl32 missingValue)

  
{

  if (array.sizeMajor() != (int) rays.size()) {
    cerr << "ERROR - RadxVol::loadRaysFrom2DField()" << endl;
    cerr << "  Array major dimension does not match nRays" << endl;
    cerr << "  Array major size: " << array.sizeMajor() << endl;
    cerr << "  nRays: " << rays.size() << endl;
    cerr << "  Field: " << fieldName << endl;
    return -1;
  }

  // fill rays in order

  Radx::fl32 **data = array.dat2D();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    if (nGates > array.sizeMinor()) {
      nGates = array.sizeMinor();
    }

    RadxField *fld = ray->getField(fieldName);

    if (fld == NULL) {
      // field does not exist, create it
      fld = new RadxField(fieldName, units);
      fld->setTypeFl32(missingValue);
      fld->setMissingFl32(missingValue);
      fld->addDataFl32(nGates, data[iray]);
      ray->addField(fld);
    } else {
      // copy the data in
      fld->clearData();
      fld->setMissingFl32(missingValue);
      fld->addDataFl32(nGates, data[iray]);
    }

  } // iray

  return 0;

}

//////////////////////////////////////////////////////////////
// Load up ray fields from 2D si32 field array.
// This is a static method, does not use any vol members.
// Returns 0 on success, -1 on failure

int RadxVol::loadRaysFrom2DField(const RadxArray2D<Radx::si32> &array,
                                 const vector<RadxRay *> &rays,
                                 const string &fieldName,
                                 const string &units,
                                 Radx::si32 missingValue)

  
{

  if (array.sizeMajor() != (int) rays.size()) {
    cerr << "ERROR - RadxVol::loadRaysFrom2DField()" << endl;
    cerr << "  Array major dimension does not match nRays" << endl;
    cerr << "  Array major size: " << array.sizeMajor() << endl;
    cerr << "  nRays: " << rays.size() << endl;
    cerr << "  Field: " << fieldName << endl;
    return -1;
  }

  // fill rays in order

  Radx::si32 **data = array.dat2D();
  
  for (size_t iray = 0; iray < rays.size(); iray++) {

    RadxRay *ray = rays[iray];
    int nGates = ray->getNGates();
    if (nGates > array.sizeMinor()) {
      nGates = array.sizeMinor();
    }

    RadxField *fld = ray->getField(fieldName);

    if (fld == NULL) {
      // field does not exist, create it
      fld = new RadxField(fieldName, units);
      fld->setTypeSi32(missingValue, 1.0, 0.0);
      fld->setMissingSi32(missingValue);
      fld->addDataSi32(nGates, data[iray]);
      ray->addField(fld);
    } else {
      // copy the data in
      fld->clearData();
      fld->setMissingSi32(missingValue);
      fld->addDataSi32(nGates, data[iray]);
    }

  } // iray

  return 0;

}
                  
////////////////////////////////////////////  
/// Set up angle search, for a given sweep
/// Return 0 on success, -1 on failure

int RadxVol::_setupAngleSearch(size_t sweepNum)
  
{

  // check sweep num is valid

  assert(sweepNum < _sweeps.size());

  // init rays to NULL - i.e. no match
  
  for (int ii = 0; ii < _searchAngleN; ii++) {
    _searchRays[ii] = NULL;
  }

  // set the sweep pointers

  const RadxSweep *sweep = _sweeps[sweepNum];
  size_t startRayIndex = sweep->getStartRayIndex();
  size_t endRayIndex = sweep->getEndRayIndex();
  
  double prevAngle = _rays[startRayIndex]->getAzimuthDeg();
  if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
    prevAngle = _rays[startRayIndex]->getElevationDeg();
  }
  double sumDeltaAngle = 0.0;
  double count = 0.0;
  
  for (size_t ii = startRayIndex; ii <= endRayIndex; ii++) {
    
    RadxRay *ray = _rays[ii];

    // get angle depending on sweep mode

    double angle = ray->getAzimuthDeg();
    if (sweep->getSweepMode() == Radx::SWEEP_MODE_RHI) {
      angle = ray->getElevationDeg();
    }

    // condition angle

    while (angle < 0) {
      angle += 360.0;
    }
    while (angle >= 360.0) {
      angle -= 360.0;
    }

    // compute angle change
    
    double deltaAngle = fabs(angle - prevAngle);
    if (deltaAngle > 180) {
      deltaAngle = fabs(deltaAngle - 360.0);
    }
    prevAngle = angle;
    sumDeltaAngle += deltaAngle;
    count++;
    
    // get angle index
    
    int iangle = _getSearchAngleIndex(angle);
    
    // set ray at index location
    
    _searchRays[iangle] = ray;
    
  } // iray
  
  // compute mean delta angle, and search width
  
  double meanDeltaAngle = sumDeltaAngle / count;
  _searchMaxWidth = (int) ((meanDeltaAngle / _searchAngleRes) * 1.5);

  // populate the search array for rays in sweep

  int firstIndex = -1;
  int lastIndex = -1;
  for (int ii = 0; ii < _searchAngleN; ii++) {
    // find active index
    const RadxRay *rayii = _searchRays[ii];
    if (rayii != NULL) {
      if (firstIndex < 0) {
        firstIndex = ii;
      }
      // find next active index
      for (int jj = ii + 1; jj < _searchAngleN; jj++) {
        const RadxRay *rayjj = _searchRays[jj];
        if (rayjj != NULL) {
          lastIndex = jj;
          _populateSearchRays(ii, jj);
          ii = jj - 1;
          break;
        }
      } // jj
    }
  } // ii

  // populate search across 360 line
  
  _populateSearchAcross360(firstIndex, lastIndex);

  return 0;

}
  
/////////////////////////////////////////////////////////
// get the angle index for the search matrix, given
// the angle angle
//
// Returns -1 if out of bounds

int RadxVol::_getSearchAngleIndex(double angle) 
{
  int iangle = (int) (angle / _searchAngleRes + 0.5);
  if (iangle < 0) {
    iangle = 0;
  } else if (iangle > _searchAngleN - 1) {
    iangle = _searchAngleN - 1;
  }
  return iangle;
}
  
/////////////////////////////////////////////////////////
// get the angle given the search index

double RadxVol::_getSearchAngle(int index) 
{
  return index * _searchAngleRes;
}
  
/////////////////////////////////////////////////////////
// populate the search matrix between given indices

void RadxVol::_populateSearchRays(int start, int end)
{
  
  int len = end - start;
  int extend = len / 2;
  if (extend > _searchMaxWidth) {
    extend = _searchMaxWidth;
  }

  const RadxRay *rayStart = _searchRays[start];
  for (int ii = start + 1; ii <= start + extend; ii++) {
    _searchRays[ii] = rayStart;
  }

  const RadxRay *rayEnd = _searchRays[end];
  for (int ii = end - 1; ii >= end - extend; ii--) {
    _searchRays[ii] = rayEnd;
  }

}

/////////////////////////////////////////////////////////
// populate the search across the 360 line

void RadxVol::_populateSearchAcross360(int first, int last)
{
  
  int len = first + _searchAngleN - last;
  int extend = len / 2;
  if (extend > _searchMaxWidth) {
    extend = _searchMaxWidth;
  }
  
  const RadxRay *rayLast = _searchRays[last];
  for (int ii = last + 1; ii <= last + extend; ii++) {
    int jj = ii;
    if (jj >= _searchAngleN) jj -= _searchAngleN;
    _searchRays[jj] = rayLast;
  }

  const RadxRay *rayFirst = _searchRays[first];
  for (int ii = first - 1; ii >= first - extend; ii--) {
    int jj = ii;
    if (jj < 0) jj += _searchAngleN;
    _searchRays[jj] = rayFirst;
  }

}

/////////////////////////////////////////////////////////
// Get the transition index in a sweep, where it crosses
// a specified azimuth
//
// Returns -1 if no transition found.

int RadxVol::_getTransIndex(const RadxSweep *sweep, double azimuth)
{

  for (size_t iray = sweep->getStartRayIndex();
       iray < sweep->getEndRayIndex(); iray++) {

    RadxRay *ray1 = _rays[iray];
    RadxRay *ray2 = _rays[iray + 1];

    double az1 = ray1->getAzimuthDeg();
    double az2 = ray2->getAzimuthDeg();

    double delta1 = Radx::conditionAngleDelta(az1 - azimuth);
    double delta2 = Radx::conditionAngleDelta(az2 - azimuth);

    if (delta1 <= 0.0 && delta2 >= 0.0) {
      return (int) iray;
    }
    
    if (delta2 <= 0.0 && delta1 >= 0.0) {
      return (int) iray;
    }

  } // iray

  // no transition found

  return -1;

}

/////////////////////////////////////////////////////////
// serialize into a RadxMsg

void RadxVol::serialize(RadxMsg &msg)
  
{

  // init
  
  msg.clearAll();
  msg.setMsgType(RadxMsg::RadxVolMsg);

  // add metadata strings as xml part
  // include null at string end
  
  string xml;
  _loadMetaStringsToXml(xml);
  msg.addPart(_metaStringsPartId, xml.c_str(), xml.size() + 1);

  // add metadata numbers

  _loadMetaNumbersToMsg();
  msg.addPart(_metaNumbersPartId, &_metaNumbers, sizeof(msgMetaNumbers_t));

  // add platform
  
  RadxMsg platformMsg;
  _platform.serialize(platformMsg);
  platformMsg.assemble();
  msg.addPart(_platformPartId,
              platformMsg.assembledMsg(), 
              platformMsg.lengthAssembled());

  // add calibrations
  
  for (size_t icalib = 0; icalib < _rcalibs.size(); icalib++) {
    RadxRcalib *rcalib = _rcalibs[icalib];
    RadxMsg rcalibMsg(RadxMsg::RadxRcalibMsg);
    rcalib->serialize(rcalibMsg);
    rcalibMsg.assemble();
    msg.addPart(_rcalibPartId,
                rcalibMsg.assembledMsg(), 
                rcalibMsg.lengthAssembled());
  }

  // add sweeps

  for (size_t isweep = 0; isweep < _sweeps.size(); isweep++) {
    RadxMsg sweepMsg;
    _sweeps[isweep]->serialize(sweepMsg);
    sweepMsg.assemble();
    msg.addPart(_sweepPartId,
                sweepMsg.assembledMsg(), 
                sweepMsg.lengthAssembled());
  }
  
  // add sweeps as in file, if they exist
  
  for (size_t isweep = 0; isweep < _sweepsAsInFile.size(); isweep++) {
    RadxMsg sweepMsg;
    _sweepsAsInFile[isweep]->serialize(sweepMsg, RadxMsg::RadxSweepAsInFileMsg);
    sweepMsg.assemble();
    msg.addPart(_sweepAsInFilePartId,
                sweepMsg.assembledMsg(), 
                sweepMsg.lengthAssembled());
  }
  
  // add rays
  
  for (size_t iray = 0; iray < _rays.size(); iray++) {
    RadxRay *ray = _rays[iray];
    RadxMsg rayMsg(RadxMsg::RadxRayMsg);
    ray->serialize(rayMsg);
    rayMsg.assemble();
    msg.addPart(_rayPartId,
                rayMsg.assembledMsg(), 
                rayMsg.lengthAssembled());
  }

  // add correction factors part if needed
  
  if (_cfactors != NULL) {
    RadxMsg cfactorsMsg;
    _cfactors->serialize(cfactorsMsg);
    cfactorsMsg.assemble();
    msg.addPart(_cfactorsPartId,
                cfactorsMsg.assembledMsg(), 
                cfactorsMsg.lengthAssembled());
  }

  // add fields if set
  // normally the fields will be attached to the rays instead of the volume
  
  for (size_t ifield = 0; ifield < _fields.size(); ifield++) {
    RadxField *field = _fields[ifield];
    RadxMsg fieldMsg(RadxMsg::RadxFieldMsg);
    field->serialize(fieldMsg);
    fieldMsg.assemble();
    msg.addPart(_fieldPartId,
                fieldMsg.assembledMsg(), 
                fieldMsg.lengthAssembled());
  }

}

/////////////////////////////////////////////////////////
// deserialize from a RadxMsg
// return 0 on success, -1 on failure

int RadxVol::deserialize(const RadxMsg &msg)
  
{
  
  // initialize object

  _init();

  // check type

  if (msg.getMsgType() != RadxMsg::RadxVolMsg) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::deserialize" << endl;
    cerr << "  incorrect message type" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get the metadata strings

  const RadxMsg::Part *metaStringPart = msg.getPartByType(_metaStringsPartId);
  if (metaStringPart == NULL) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::deserialize" << endl;
    cerr << "  No metadata string part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaStringsFromXml((char *) metaStringPart->getBuf(),
                             metaStringPart->getLength())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::deserialize" << endl;
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
    cerr << "ERROR - RadxVol::deserialize" << endl;
    cerr << "  No metadata numbers part in message" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }
  if (_setMetaNumbersFromMsg((msgMetaNumbers_t *) metaNumsPart->getBuf(),
                             metaNumsPart->getLength(),
                             msg.getSwap())) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::deserialize" << endl;
    msg.printHeader(cerr, "  ");
    cerr << "=======================================" << endl;
    return -1;
  }

  // get platform
  
  const RadxMsg::Part *platformPart = msg.getPartByType(_platformPartId);
  if (platformPart != NULL) {
    RadxMsg platformMsg;
    platformMsg.disassemble(platformPart->getBuf(), platformPart->getLength());
    if (_platform.deserialize(platformMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxVol::deserialize" << endl;
      cerr << "  Cannot dserialize platform" << endl;
      platformMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      return -1;
    }
  }
  
  // get rcalibs if available
  
  clearRcalibs();
  size_t nRcalibs = msg.partExists(_rcalibPartId);
  for (size_t ircalib = 0; ircalib < nRcalibs; ircalib++) {
    // get rcalib part
    const RadxMsg::Part *rcalibPart =
      msg.getPartByType(_rcalibPartId, ircalib);
    // create a message from the rcalib part
    RadxMsg rcalibMsg;
    rcalibMsg.disassemble(rcalibPart->getBuf(), rcalibPart->getLength());
    // create a rcalib, dserialize from the message
    RadxRcalib *rcalib = new RadxRcalib;
    if (rcalib->deserialize(rcalibMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Adding rcalib num: " << ircalib << endl;
      rcalibMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete rcalib;
      return -1;
    }
    // add the rcalib
    _rcalibs.push_back(rcalib);
  } // isweep
  
  // get sweeps
  
  clearSweeps();
  size_t nSweeps = msg.partExists(_sweepPartId);
  for (size_t isweep = 0; isweep < nSweeps; isweep++) {
    // get sweep part
    const RadxMsg::Part *sweepPart =
      msg.getPartByType(_sweepPartId, isweep);
    // create a message from the sweep part
    RadxMsg sweepMsg;
    sweepMsg.disassemble(sweepPart->getBuf(), sweepPart->getLength());
    // create a sweep, dserialize from the message
    RadxSweep *sweep = new RadxSweep;
    if (sweep->deserialize(sweepMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Adding sweep, num: " << isweep << endl;
      sweepMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete sweep;
      return -1;
    }
    // add the sweep
    addSweep(sweep);
  } // isweep
  
  // get sweeps as in file
  
  clearSweepsAsInFile();
  size_t nSweepsAsInFile = msg.partExists(_sweepAsInFilePartId);
  for (size_t isweep = 0; isweep < nSweepsAsInFile; isweep++) {
    // get sweep part
    const RadxMsg::Part *sweepPart =
      msg.getPartByType(_sweepAsInFilePartId, isweep);
    // create a message from the sweep part
    RadxMsg sweepMsg;
    sweepMsg.disassemble(sweepPart->getBuf(), sweepPart->getLength());
    // create a sweep, dserialize from the message
    RadxSweep *sweep = new RadxSweep;
    if (sweep->deserialize(sweepMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Adding sweep as in file, num: " << isweep << endl;
      sweepMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete sweep;
      return -1;
    }
    // add the sweep
    addSweepAsInFile(sweep);
  } // isweep

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
      cerr << "ERROR - RadxVol::deserialize" << endl;
      cerr << "  Cannot dserialize cfactors" << endl;
      cfactorsMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete _cfactors;
      _cfactors = NULL;
      return -1;
    }
  }
  
  // get rays
  
  clearRays();
  size_t nRays = msg.partExists(_rayPartId);
  for (size_t iray = 0; iray < nRays; iray++) {
    // get ray part
    const RadxMsg::Part *rayPart =
      msg.getPartByType(_rayPartId, iray);
    // create a message from the ray part
    RadxMsg rayMsg;
    rayMsg.disassemble(rayPart->getBuf(), rayPart->getLength());
    // create a ray, dserialize from the message
    RadxRay *ray = new RadxRay;
    if (ray->deserialize(rayMsg)) {
      cerr << "=======================================" << endl;
      cerr << "ERROR - RadxRay::deserialize" << endl;
      cerr << "  Adding ray, num: " << iray << endl;
      rayMsg.printHeader(cerr, "  ");
      cerr << "=======================================" << endl;
      delete ray;
      return -1;
    }
    // add the ray
    addRay(ray);
  } // iray
  
  // get fields
  // normally this will not be active
  // since the fields are normally included in the rays
  
  clearFields();
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
      cerr << "ERROR - RadxField::deserialize" << endl;
      cerr << "  Adding field, num: " << ifield << endl;
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

void RadxVol::_loadMetaStringsToXml(string &xml, int level /* = 0 */)  const
  
{
  xml.clear();
  xml += RadxXml::writeStartTag("RadxVol", level);
  xml += RadxXml::writeString("version", level + 1, _version);
  xml += RadxXml::writeString("title", level + 1, _title);
  xml += RadxXml::writeString("institution", level + 1, _institution);
  xml += RadxXml::writeString("references", level + 1, _references);
  xml += RadxXml::writeString("source", level + 1, _source);
  xml += RadxXml::writeString("history", level + 1, _history);
  xml += RadxXml::writeString("comment", level + 1, _comment);
  xml += RadxXml::writeString("author", level + 1, _author);
  xml += RadxXml::writeString("driver", level + 1, _driver);
  xml += RadxXml::writeString("created", level + 1, _created);
  xml += RadxXml::writeString("origFormat", level + 1, _origFormat);
  xml += RadxXml::writeString("statusXml", level + 1, _statusXml);
  xml += RadxXml::writeString("scanName", level + 1, _scanName);
  xml += RadxXml::writeString("pathInUse", level + 1, _pathInUse);
  for (size_t iattr = 0; iattr < _userGlobAttr.size(); iattr++) {
    xml += RadxXml::writeStartTag("UserGlobAttr", level + 1);
    xml += RadxXml::writeString("attrName", level + 2, _userGlobAttr[iattr].name);
    switch (_userGlobAttr[iattr].attrType) {
      case UserGlobAttr::ATTR_STRING:
        xml += RadxXml::writeString("attrType", level + 2, "ATTR_STRING");
        break;
      case UserGlobAttr::ATTR_INT:
        xml += RadxXml::writeString("attrType", level + 2, "ATTR_INT");
        break;
      case UserGlobAttr::ATTR_DOUBLE:
        xml += RadxXml::writeString("attrType", level + 2, "ATTR_DOUBLE");
        break;
      case UserGlobAttr::ATTR_INT_ARRAY:
        xml += RadxXml::writeString("attrType", level + 2, "ATTR_INT_ARRAY");
        break;
      case UserGlobAttr::ATTR_DOUBLE_ARRAY:
        xml += RadxXml::writeString("attrType", level + 2, "ATTR_DOUBLE_ARRAY");
        break;
    }
    xml += RadxXml::writeString("attrVal", level + 2, _userGlobAttr[iattr].val);
    xml += RadxXml::writeEndTag("UserGlobAttr", level + 1);
  }
  xml += RadxXml::writeEndTag("RadxVol", level);
}

/////////////////////////////////////////////////////////
// set metadata strings from XML
// returns 0 on success, -1 on failure

int RadxVol::_setMetaStringsFromXml(const char *xml,
                                    size_t bufLen)

{

  // check for NULL
  
  if (xml[bufLen - 1] != '\0') {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::_setMetaStringsFromXml" << endl;
    cerr << "  XML string not null terminated" << endl;
    string xmlStr(xml, bufLen);
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;    
  }

  string xmlStr(xml);
  string contents;

  if (RadxXml::readString(xmlStr, "RadxVol", contents)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::_setMetaStringsFromXml" << endl;
    cerr << "  XML not delimited by 'RadxVol' tags" << endl;
    cerr << "  " << xmlStr << endl;
    cerr << "=======================================" << endl;
    return -1;
  }

  RadxXml::readString(contents, "version", _version);
  RadxXml::readString(contents, "title", _title);
  RadxXml::readString(contents, "institution", _institution);
  RadxXml::readString(contents, "references", _references);
  RadxXml::readString(contents, "source", _source);
  RadxXml::readString(contents, "history", _history);
  RadxXml::readString(contents, "comment", _comment);
  RadxXml::readString(contents, "author", _author);
  RadxXml::readString(contents, "driver", _driver);
  RadxXml::readString(contents, "created", _created);
  RadxXml::readString(contents, "origFormat", _origFormat);
  RadxXml::readString(contents, "statusXml", _statusXml);
  RadxXml::readString(contents, "scanName", _scanName);
  RadxXml::readString(contents, "pathInUse", _pathInUse);

  clearUserGlobAttr();
  vector<string> userAttrXml;
  if (RadxXml::readStringArray(contents, "UserGlobAttr", userAttrXml) == 0) {
    for (size_t iattr = 0; iattr < userAttrXml.size(); iattr++) {
      string attrName, attrType, attrVal;
      RadxXml::readString(userAttrXml[iattr], "attrName", attrName);
      RadxXml::readString(userAttrXml[iattr], "attrType", attrType);
      RadxXml::readString(userAttrXml[iattr], "attrVal", attrVal);
      if (attrType == "ATTR_INT") {
        addUserGlobAttr(attrName, UserGlobAttr::ATTR_INT, attrVal);
      } else if (attrType == "ATTR_DOUBLE") {
        addUserGlobAttr(attrName, UserGlobAttr::ATTR_DOUBLE, attrVal);
      } else if (attrType == "ATTR_INT_ARRAY") {
        addUserGlobAttr(attrName, UserGlobAttr::ATTR_INT_ARRAY, attrVal);
      } else if (attrType == "ATTR_DOUBLE_ARRAY") {
        addUserGlobAttr(attrName, UserGlobAttr::ATTR_DOUBLE_ARRAY, attrVal);
      } else {
        addUserGlobAttr(attrName, UserGlobAttr::ATTR_STRING, attrVal);
      }
    }
  }

  return 0;

}

/////////////////////////////////////////////////////////
// load the meta number to the message struct

void RadxVol::_loadMetaNumbersToMsg()
  
{

  // clear

  memset(&_metaNumbers, 0, sizeof(_metaNumbers));
  
  // set 64 bit values

  _metaNumbers.startTimeSecs = _startTimeSecs;
  _metaNumbers.endTimeSecs = _endTimeSecs;
  _metaNumbers.startNanoSecs = _startNanoSecs;
  _metaNumbers.endNanoSecs = _endNanoSecs;

  // set 32-bit values

  _metaNumbers.volNum = _volNum;
  _metaNumbers.scanId = _scanId;
  _metaNumbers.rayTimesIncrease = _rayTimesIncrease;

}

/////////////////////////////////////////////////////////
// set the meta number data from the message struct

int RadxVol::_setMetaNumbersFromMsg(const msgMetaNumbers_t *metaNumbers,
                                    size_t bufLen,
                                    bool swap)
  
{

  // check size

  if (bufLen != sizeof(msgMetaNumbers_t)) {
    cerr << "=======================================" << endl;
    cerr << "ERROR - RadxVol::_setMetaNumbersFromMsg" << endl;
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

  // set 64 bit values

  _startTimeSecs = _metaNumbers.startTimeSecs;
  _endTimeSecs = _metaNumbers.endTimeSecs;
  _startNanoSecs = _metaNumbers.startNanoSecs;
  _endNanoSecs = _metaNumbers.endNanoSecs;

  // set 32-bit values

  _volNum = _metaNumbers.volNum;
  _scanId = _metaNumbers.scanId;
  _rayTimesIncrease = _metaNumbers.rayTimesIncrease;

  return 0;

}

/////////////////////////////////////////////////////////
// swap meta numbers

void RadxVol::_swapMetaNumbers(msgMetaNumbers_t &meta)
{
  ByteOrder::swap64(&meta.startTimeSecs, 4 * sizeof(Radx::si64));
  ByteOrder::swap32(&meta.scanId, 3 * sizeof(Radx::si32));
}
