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
//////////////////////////////////////////////////////////////////////////
// FindSurfaceVel.cc
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2016
//
//////////////////////////////////////////////////////////////////////////
//
// FindSurfaceVel is intended for use with downward-pointing airborne
// Doppler radars.
//
// FindSurfaceVel reads in Radx moments, computes the apparent velocity
// of the ground echo, and filters the apparent velocity in time to remove
// spurious spikes.
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <radar/FindSurfaceVel.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxTimeList.hh>
using namespace std;

// Coefficients for stage-1 FIR filter.
// Initial filtering step. This is normally a 21-point FIR filter.

const double FindSurfaceVel::stage1FilterDefault[Stage1FilterDefaultLen] = {
  1.6976992e-02, 2.3294990e-02, 3.0244475e-02, 3.7550056e-02, 4.4888313e-02,
  5.1908191e-02, 5.8254533e-02, 6.3592862e-02, 6.7633391e-02, 7.0152222e-02,
  7.1007947e-02, 7.0152222e-02, 6.7633391e-02, 6.3592862e-02, 5.8254533e-02,
  5.1908191e-02, 4.4888313e-02, 3.7550056e-02, 3.0244475e-02, 2.3294990e-02,
  1.6976992e-02
};

// Coefficients for spike FIR filter.
// Applying this filter onces is equivalent to applying the stage-1 filter
// five times in succession on a time series.
// Normally this will have 101 entries.

const double FindSurfaceVel::spikeFilterDefault[SpikeFilterDefaultLen] = {
  1.4102747e-09, 9.6755464e-09, 3.9114620e-08, 1.2097842e-07, 3.1591184e-07,
  7.3214001e-07, 1.5507567e-06, 3.0594395e-06, 5.6957251e-06, 1.0100617e-05,
  1.7182765e-05, 2.8192724e-05, 4.4805905e-05, 6.9211797e-05, 1.0420588e-04,
  1.5327948e-04, 2.2070176e-04, 3.1158693e-04, 4.3193918e-04, 5.8866742e-04,
  7.8956192e-04, 1.0432208e-03, 1.3589249e-03, 1.7464536e-03, 2.2158418e-03,
  2.7770754e-03, 3.4397322e-03, 4.2125742e-03, 5.1031044e-03, 6.1171018e-03,
  7.2581539e-03, 8.5272083e-03, 9.9221654e-03, 1.1437535e-02, 1.3064185e-02,
  1.4789188e-02, 1.6595812e-02, 1.8463629e-02, 2.0368790e-02, 2.2284434e-02,
  2.4181247e-02, 2.6028147e-02, 2.7793078e-02, 2.9443896e-02, 3.0949289e-02,
  3.2279725e-02, 3.3408367e-02, 3.4311933e-02, 3.4971458e-02, 3.5372918e-02,
  3.5507705e-02, 3.5372918e-02, 3.4971458e-02, 3.4311933e-02, 3.3408367e-02,
  3.2279725e-02, 3.0949289e-02, 2.9443896e-02, 2.7793078e-02, 2.6028147e-02,
  2.4181247e-02, 2.2284434e-02, 2.0368790e-02, 1.8463629e-02, 1.6595812e-02,
  1.4789188e-02, 1.3064185e-02, 1.1437535e-02, 9.9221654e-03, 8.5272083e-03,
  7.2581539e-03, 6.1171018e-03, 5.1031044e-03, 4.2125742e-03, 3.4397322e-03,
  2.7770754e-03, 2.2158418e-03, 1.7464536e-03, 1.3589249e-03, 1.0432208e-03,
  7.8956192e-04, 5.8866742e-04, 4.3193918e-04, 3.1158693e-04, 2.2070176e-04,
  1.5327948e-04, 1.0420588e-04, 6.9211797e-05, 4.4805905e-05, 2.8192724e-05,
  1.7182765e-05, 1.0100617e-05, 5.6957251e-06, 3.0594395e-06, 1.5507567e-06,
  7.3214001e-07, 3.1591184e-07, 1.2097842e-07, 3.9114620e-08, 9.6755464e-09,
  1.4102747e-09
};

// Before applying this filter, we first compute the conditioned time series.
// We compute the difference between the results of the stage-1 and spike filters
// If this difference exceeds _spikeFilterDifferenceThreshold we use the spike
// filter result. Otherwise we use the stage-1 result.

const double FindSurfaceVel::finalFilterDefault[FinalFilterDefaultLen] = {
  2.8821826e-04, 7.9095771e-04, 1.5695770e-03, 2.6840635e-03, 4.1883217e-03,
  6.1251990e-03, 8.5216287e-03, 1.1384298e-02, 1.4696240e-02, 1.8414716e-02,
  2.2470653e-02, 2.6769843e-02, 3.1195919e-02, 3.5615037e-02, 3.9882036e-02,
  4.3847698e-02, 4.7366679e-02, 5.0305553e-02, 5.2550426e-02, 5.4013588e-02,
  5.4638695e-02, 5.4013588e-02, 5.2550426e-02, 5.0305553e-02, 4.7366679e-02,
  4.3847698e-02, 3.9882036e-02, 3.5615037e-02, 3.1195919e-02, 2.6769843e-02,
  2.2470653e-02, 1.8414716e-02, 1.4696240e-02, 1.1384298e-02, 8.5216287e-03,
  6.1251990e-03, 4.1883217e-03, 2.6840635e-03, 1.5695770e-03, 7.9095771e-04,
  2.8821826e-04
};

// Constructor

FindSurfaceVel::FindSurfaceVel()
  
{

  _debug = false;
  _verbose = false;
  
  _dbzMax = NULL;
  _rangeToSurface = NULL;
  _velSurfaceArray = NULL;
  _filteredStage1 = NULL;
  _filteredSpike = NULL;
  _filteredCond = NULL;
  _filteredFinal = NULL;
  _nValid = 0;

  _dbzFieldName = "DBZ";
  _velFieldName = "VEL";
  _minRangeToSurfaceKm = 0.5;
  _minDbzForSurfaceEcho = 20.0;
  _nGatesForSurfaceEcho = 1;
  _spikeFilterDifferenceThreshold = 0.11;

  // set up the default filters
  
  initFilters(Stage1FilterDefaultLen, stage1FilterDefault,
              SpikeFilterDefaultLen, spikeFilterDefault,
              FinalFilterDefaultLen, finalFilterDefault);
  
}

// destructor

FindSurfaceVel::~FindSurfaceVel()

{

  // free memory

  for (size_t iray = 0; iray < _filtRays.size(); iray++) {
    RadxRay::deleteIfUnused(_filtRays[iray]);
  }

  if (_dbzMax) {
    delete[] _dbzMax;
  }
  if (_rangeToSurface) {
    delete[] _rangeToSurface;
  }
  if (_velSurfaceArray) {
    delete[] _velSurfaceArray;
  }
  if (_filteredStage1) {
    delete[] _filteredStage1;
  }
  if (_filteredSpike) {
    delete[] _filteredSpike;
  }
  if (_filteredCond) {
    delete[] _filteredCond;
  }
  if (_filteredFinal) {
    delete[] _filteredFinal;
  }

}

/////////////////////////////////////////////////////
// get results
// the following return NAN if results not available

double FindSurfaceVel::getVelMeasured() const
{
  if (!_velSurfaceArray || !_velIsValid) {
    return NAN;
  } else {
    return _velSurfaceArray[_finalIndex];
  }
}

double FindSurfaceVel::getRangeToSurface() const
{
  if (!_rangeToSurface || !_velIsValid) {
    return NAN;
  } else {
    return _rangeToSurface[_finalIndex];
  }
}

double FindSurfaceVel::getDbzMax() const
{
  if (!_dbzMax || !_velIsValid) {
    return NAN;
  } else {
    return _dbzMax[_finalIndex];
  }
}

double FindSurfaceVel::getVelStage1() const
{
  if (!_filteredStage1 || !_velIsValid) {
    return NAN;
  } else {
    return _filteredStage1[_finalIndex];
  }
}

double FindSurfaceVel::getVelSpike() const
{
  if (!_filteredSpike || !_velIsValid) {
    return NAN;
  } else {
    return _filteredSpike[_finalIndex];
  }
}

double FindSurfaceVel::getVelCond() const
{
  if (!_filteredCond || !_velIsValid) {
    return NAN;
  } else {
    return _filteredCond[_finalIndex];
  }
}

double FindSurfaceVel::getVelFilt() const
{
  if (!_filteredFinal || !_velIsValid) {
    return NAN;
  } else {
    return _filteredFinal[_finalIndex];
  }
}

//////////////////////////////////////////////////
// initialzie the filters from arrays

void FindSurfaceVel::initFilters(int stage1FilterLen,
                                 const double *filtCoeffStage1,
                                 int spikeFilterLen,
                                 const double *filtCoeffSpike,
                                 int finalFilterLen,
                                 const double *filtCoeffFinal)
                                 
{

  _filtCoeffStage1.clear();
  _filtCoeffSpike.clear();
  _filtCoeffFinal.clear();

  for (int ii = 0; ii < stage1FilterLen; ii++) {
    _filtCoeffStage1.push_back(filtCoeffStage1[ii]);
  }
  
  for (int ii = 0; ii < spikeFilterLen; ii++) {
    _filtCoeffSpike.push_back(filtCoeffSpike[ii]);
  }

  for (int ii = 0; ii < finalFilterLen; ii++) {
    _filtCoeffFinal.push_back(filtCoeffFinal[ii]);
  }
  
  _initFromFilters();

}

//////////////////////////////////////////////////
// initialize the filters from vectors

void FindSurfaceVel::initFilters(const vector<double> &filtCoeffStage1,
                                 const vector<double> &filtCoeffSpike,
                                 const vector<double> &filtCoeffFinal)
                                 
{

  _filtCoeffStage1.clear();
  _filtCoeffSpike.clear();
  _filtCoeffFinal.clear();

  _filtCoeffStage1 = filtCoeffStage1;
  _filtCoeffSpike = filtCoeffSpike;
  _filtCoeffFinal = filtCoeffFinal;

  _initFromFilters();

}

//////////////////////////////////////////////////
// initialize variables based on filters

void FindSurfaceVel::_initFromFilters()
  
{

  // compute filter lengths and half-lengths

  _lenStage1 = _filtCoeffStage1.size();
  _lenStage1Half = _lenStage1 / 2;
  
  _lenSpike = _filtCoeffSpike.size();
  _lenSpikeHalf = _lenSpike / 2;

  _lenFinal = _filtCoeffFinal.size();
  _lenFinalHalf = _lenFinal / 2;

  // compute indices for filter results and
  // length of buffers required for filtering
  
  if (_lenSpike > _lenStage1) {
    _condIndex = _lenSpikeHalf;
    _finalIndex = _lenSpikeHalf + _lenFinalHalf;
    _filtBufLen = _lenSpike;
    if (_finalIndex + _lenFinalHalf > _filtBufLen) {
      _filtBufLen = _finalIndex + _lenFinalHalf;
    }
  } else {
    _condIndex = _lenStage1Half;
    _finalIndex = _lenStage1Half + _lenFinalHalf;
    _filtBufLen = _lenStage1;
    if (_finalIndex + _lenFinalHalf > _filtBufLen) {
      _filtBufLen = _finalIndex + _lenFinalHalf;
    }
  }

  // set up arrays

  int nbytes = _filtBufLen * sizeof(double);
  
  _dbzMax = new double[_filtBufLen];
  _rangeToSurface = new double[_filtBufLen];
  _velSurfaceArray = new double[_filtBufLen];
  _filteredSpike = new double[_filtBufLen];
  _filteredStage1 = new double[_filtBufLen];
  _filteredCond = new double[_filtBufLen];
  _filteredFinal = new double[_filtBufLen];

  memset(_dbzMax, 0, nbytes);
  memset(_rangeToSurface, 0, nbytes);
  memset(_velSurfaceArray, 0, nbytes);
  memset(_filteredSpike, 0, nbytes);
  memset(_filteredStage1, 0, nbytes);
  memset(_filteredCond, 0, nbytes);
  memset(_filteredFinal, 0, nbytes);

  if (_verbose) {
    cerr << "Filter summary:" << endl;
    cerr << "  Filter _lenStage1, _lenStage1Half: "
         << _lenStage1 << ", " << _lenStage1Half << endl;
    cerr << "  Filter _lenSpike, _lenSpikeHalf: "
         << _lenSpike << ", " << _lenSpikeHalf << endl;
    cerr << "  Filter _lenFinal, _lenFinalHalf: "
         << _lenFinal << ", " << _lenFinalHalf << endl;
    cerr << "  Conditioned index: " << _condIndex << endl;
    cerr << "  Final index: " << _finalIndex << endl;
    cerr << "  Filter buffer len: " << _filtBufLen << endl;
  }

}

/////////////////////////////////////////////////////////////////////////
// Process an incoming ray
// Returns 0 on success, -1 on failure.
// On success, call getSurfaceVelocity() to get computed surface velocity

int FindSurfaceVel::processRay(RadxRay *ray)

{

  // convert to floats

  ray->convertToFl32();

  // trim ray deque as needed
  
  if (_filtRays.size() > _finalIndex) {
    RadxRay *toBeDeleted = _filtRays.back();
    RadxRay::deleteIfUnused(toBeDeleted);
    _filtRays.pop_back();
  }

  // shift the data arrays by 1

  _shiftArraysBy1();

  // add to ray deque
  
  ray->addClient();
  _filtRays.push_front(ray);
  
  // compute surface vel, store at end of array
  
  _computeSurfaceVel(ray);
  
  // apply the spike and stage1 filters

  _applyStage1Filter();
  _applySpikeFilter();

  // compute the conditioned velocity, based on the
  // difference between the results of the spike and stage1 filter
  
  _computeConditionedValue();
  
  // apply the final filter
  
  _applyFinalFilter();

  // was this a valid observation? - i.e. can we see the surface

  if (_rangeToSurface[0] > 0) {
    _nValid++;
  } else {
    _nValid = 0;
  }
  
  // print out?
  
  if (_verbose) {
    if (_filtRays.size() > _finalIndex) {
      RadxRay *ray = _filtRays[_finalIndex];
      if (_rangeToSurface[_finalIndex] > 0) {
        cerr << "Surface data: time range dbzMax vel filtStage1 filtSpike filtCond filtFinal: "
             << ray->getRadxTime().asString() << " "
             << _rangeToSurface[_finalIndex] << " "
             << _dbzMax[_finalIndex] << " "
             << _velSurfaceArray[_finalIndex] << " "
             << _filteredStage1[_finalIndex] <<  ""
             << _filteredSpike[_finalIndex] << " "
             << _filteredCond[_finalIndex] << " "
             << _filteredFinal[_finalIndex] << endl;
      } else {
        cerr << "Surface NOT found, time: "
             << ray->getRadxTime().asString() << endl;
      }
    }
  }

  // check for success
  
  _velIsValid = false;
  _filtRay = _filtRays[_filtRays.size()-1];
  
  if (_filtRays.size() > _finalIndex) {
    _filtRay = _filtRays[_finalIndex];
    if (_nValid > _finalIndex) {
      _velIsValid = true;
    }
    return 0;
  }

  return -1;

}

/////////////////////////////////////////////////////////////////////////
// compute surface velocity
//
// Sets vel to (0.0) if cannot determine valocity

void FindSurfaceVel::_computeSurfaceVel(RadxRay *ray)
  
{

  // init
  
  _dbzMax[0] = NAN;
  _rangeToSurface[0] = NAN;
  _velSurfaceArray[0] = 0.0;

  // check elevation
  // cannot compute gnd vel if not pointing down
  
  double elev = ray->getElevationDeg();
  if (elev > -85 || elev < -95) {
    if (_debug) {
      cerr << "Bad elevation for finding surface, time, elev(deg): "
           << ray->getRadxTime().asString() << ", "
           << elev << endl;
    }
    return;
  }

  // get dbz field

  RadxField *dbzField = ray->getField(_dbzFieldName);
  if (dbzField == NULL) {
    cerr << "ERROR - FindSurfaceVel::_computeSurfaceVel" << endl;
    cerr << "  No dbz field found, field name: " << _dbzFieldName << endl;
    return;
  }
  const Radx::fl32 *dbzArray = dbzField->getDataFl32();
  Radx::fl32 dbzMiss = dbzField->getMissingFl32();

  // get vel field
  
  RadxField *velField = ray->getField(_velFieldName);
  if (velField == NULL) {
    cerr << "ERROR - FindSurfaceVel::_computeSurfaceVel" << endl;
    cerr << "  No vel field found, field name: " << _velFieldName << endl;
    return;
  }
  const Radx::fl32 *velArray = velField->getDataFl32();
  Radx::fl32 velMiss = velField->getMissingFl32();
  
  // get gate at which max dbz occurs

  double range = dbzField->getStartRangeKm();
  double drange = dbzField->getGateSpacingKm();
  double dbzMax = -9999;
  int gateForMax = -1;
  double rangeToSurface = 0;
  double foundSurface = false;
  for (size_t igate = 0; igate < dbzField->getNPoints(); igate++, range += drange) {
    if (range < _minRangeToSurfaceKm) {
      continue;
    }
    Radx::fl32 dbz = dbzArray[igate];
    if (dbz != dbzMiss) {
      if (dbz > dbzMax) {
        dbzMax = dbz;
        gateForMax = igate;
        rangeToSurface = range;
        foundSurface = true;
      }
    }
  }
  
  // check for sufficient power

  if (foundSurface) {
    if (dbzMax < _minDbzForSurfaceEcho) {
      foundSurface = false;
      if (_debug) {
        cerr << "WARNING - FindSurfaceVel::_computeSurfaceVel" << endl;
        cerr << "  Ray at time: " << ray->getRadxTime().asString() << endl;
        cerr << "  Dbz max not high enough for surface detection: " << dbzMax << endl;
        cerr << "  Range to max dbz: " << rangeToSurface << endl;
      }
    }
  }

  size_t nEachSide = _nGatesForSurfaceEcho / 2;
  if (foundSurface) {
    for (size_t igate = gateForMax - nEachSide; igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 dbz = dbzArray[igate];
      if (dbz == dbzMiss) {
        foundSurface = false;
      }
      if (dbz < _minDbzForSurfaceEcho) {
        foundSurface = false;
      }
    }
  }

  // compute surface vel
  
  if (foundSurface) {
    double sum = 0.0;
    double count = 0.0;
    for (size_t igate = gateForMax - nEachSide; igate <= gateForMax + nEachSide; igate++) {
      Radx::fl32 vel = velArray[igate];
      if (vel == velMiss) {
        foundSurface = false;
      }
      sum += vel;
      count++;
    }
    _velSurfaceArray[0] = sum / count;
    _dbzMax[0] = dbzMax;
    _rangeToSurface[0] = rangeToSurface;
  }

}

/////////////////////////////////////////////////////////////////////////
// apply the stage1 filter
// this is applied to the tail end of the incoming data

void FindSurfaceVel::_applyStage1Filter()

{
  
  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenStage1; ii++) {
    double vel = _velSurfaceArray[ii];
    double wt = _filtCoeffStage1[ii];
    sum += wt * vel;
    sumWts += wt;
  }

  double filtVal = _velSurfaceArray[_lenStage1Half];
  if (sumWts > 0) {
    filtVal = sum / sumWts;
  }
  
  _filteredStage1[_lenStage1Half] = filtVal;

}

/////////////////////////////////////////////////////////////////////////
// apply the spike filter

void FindSurfaceVel::_applySpikeFilter()

{

  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenSpike; ii++) {
    double vel = _velSurfaceArray[ii];
    double wt = _filtCoeffSpike[ii];
    sum += wt * vel;
    sumWts += wt;
  }

  double filtVal = _velSurfaceArray[_lenSpikeHalf];
  if (sumWts > 0) {
    filtVal = sum / sumWts;
  }

  _filteredSpike[_lenSpikeHalf] = filtVal;

}

/////////////////////////////////////////////////////////////////////////
// apply the final filter to compute the final filtered velocity

void FindSurfaceVel::_applyFinalFilter()

{
  
  size_t istart = _finalIndex - _lenFinalHalf;
  
  double sum = 0.0;
  double sumWts = 0.0;
  for (size_t ii = 0; ii < _lenFinal; ii++) {
    double vel = _filteredCond[ii + istart];
    double wt = _filtCoeffFinal[ii];
    sum += wt * vel;
    sumWts += wt;
  }
  
  double finalVal = _filteredCond[_finalIndex];
  if (sumWts > 0) {
    finalVal = sum / sumWts;
  }
  
  _filteredFinal[_finalIndex] = finalVal;

}

/////////////////////////////////////////////////////////////////////////
// compute the conditioned value

void FindSurfaceVel::_computeConditionedValue()

{
  
  // we compute the conditioned value

  double filtStage1 = _filteredStage1[_condIndex];
  double filtSpike = _filteredSpike[_condIndex];
  double surfaceVel = _velSurfaceArray[_condIndex];
  double conditionedVel = surfaceVel;

  double absDiff = fabs(surfaceVel - filtStage1);
  if (absDiff > _spikeFilterDifferenceThreshold) {
    conditionedVel = filtSpike;
  }

  _filteredCond[_condIndex] = conditionedVel;
  
}

//////////////////////////////////////////////////
// shift the arrays by 1

void FindSurfaceVel::_shiftArraysBy1()
{
  
  int nbytesMove = (_filtBufLen - 1) * sizeof(double);
  
  memmove(_dbzMax + 1, _dbzMax, nbytesMove);
  memmove(_rangeToSurface + 1, _rangeToSurface, nbytesMove);
  memmove(_velSurfaceArray + 1, _velSurfaceArray, nbytesMove);
  memmove(_filteredSpike + 1, _filteredSpike, nbytesMove);
  memmove(_filteredStage1 + 1, _filteredStage1, nbytesMove);
  memmove(_filteredCond + 1, _filteredCond, nbytesMove);
  memmove(_filteredFinal + 1, _filteredFinal, nbytesMove);

}

