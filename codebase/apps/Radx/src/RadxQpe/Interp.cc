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
// Interp.cc
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////

#include "Interp.hh"
#include "BadValue.hh"
#include <algorithm>
#include <iomanip>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/LogMsg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/sincos.h>
#include <didss/LdataInfo.hh>
#include <rapmath/trig.h>
#include <rapformats/Cedric.hh>
#include <radar/BeamHeight.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxTimeList.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxFile.hh>
#include <Radx/RadxVol.hh>
#include <Radx/RadxSweep.hh>

const double Interp::PseudoDiamKm = 17066.0; // for earth curvature correction
const double Interp::missingDouble = BadValue::value();
const fl32 Interp::missingFl32 = BadValue::value();

// Constructor

Interp::Interp(const string &progName,
               const Parms &params,
               const RadxVol &readVol,
               const vector<Field> &interpFields,
               const vector<Ray *> &interpRays) :
        _progName(progName),
        _params(params),
        _readVol(readVol),
        _interpFields(interpFields),
        _interpRays(interpRays)
{

  gettimeofday(&_timeA, NULL);
  
  _radarLat = 0.0;
  _radarLon = 0.0;
  _radarAltKm = 0.0;

  _maxNGates = 0;
  _startRangeKm = 0.0;
  _gateSpacingKm = 0.0;
  _maxRangeKm = 0.0;

  _beamWidthDegH = 0.0;
  _beamWidthDegV = 0.0;

  _isSector = false;
  _spansNorth = false;

  _dataSectorStartAzDeg = 0.0;
  _dataSectorEndAzDeg = 360.0;
  _scanDeltaAz = 0.0;
  _scanDeltaEl = 0.0;

  _gridNx = _gridNy = _gridNz = 0;
  _nPointsVol = _nPointsPlane = 0;
  _gridMinx = _gridMiny = 0.0;
  _gridDx = _gridDy = 0.0;

  _outputFields= NULL;


}

//////////////////////////////////////
// destructor

Interp::~Interp()

{

}

////////////////////////////////////////////////////////////
// Initialize projection

void Interp::_initProjection()

{

  _proj.setGrid(_gridNx, _gridNy,
                _gridDx, _gridDy,
                _gridMinx, _gridMiny);
  
  if (_params.grid_projection == Params::PROJ_LATLON) {
    _proj.initLatlon();
  } else if (_params.grid_projection == Params::PROJ_FLAT) {
    _proj.initFlat(_gridOriginLat,
                   _gridOriginLon,
                   _params.grid_rotation);
  } else if (_params.grid_projection == Params::PROJ_LAMBERT_CONF) {
    _proj.initLambertConf(_gridOriginLat,
                          _gridOriginLon,
                          _params.grid_lat1,
                          _params.grid_lat2);
  } else if (_params.grid_projection == Params::PROJ_POLAR_STEREO) {
    Mdvx::pole_type_t poleType = Mdvx::POLE_NORTH;
    if (!_params.grid_pole_is_north) {
      poleType = Mdvx::POLE_SOUTH;
    }
    _proj.initPolarStereo(_gridOriginLat,
                          _gridOriginLon,
                          _params.grid_tangent_lon,
                          poleType,
                          _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_OBLIQUE_STEREO) {
    _proj.initObliqueStereo(_gridOriginLat,
                            _gridOriginLon,
                            _params.grid_tangent_lat,
                            _params.grid_tangent_lon,
                            _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_MERCATOR) {
    _proj.initMercator(_gridOriginLat,
                       _gridOriginLon);
  } else if (_params.grid_projection == Params::PROJ_TRANS_MERCATOR) {
    _proj.initTransMercator(_gridOriginLat,
                            _gridOriginLon,
                            _params.grid_central_scale);
  } else if (_params.grid_projection == Params::PROJ_ALBERS) {
    _proj.initAlbers(_gridOriginLat,
                     _gridOriginLon,
                     _params.grid_lat1,
                     _params.grid_lat2);
  } else if (_params.grid_projection == Params::PROJ_LAMBERT_AZIM) {
    _proj.initLambertAzim(_gridOriginLat,
                          _gridOriginLon);
  } else if (_params.grid_projection == Params::PROJ_VERT_PERSP) {
    _proj.initVertPersp(_gridOriginLat,
                        _gridOriginLon,
                        _params.grid_persp_radius);
  }
  
  if (_params.grid_set_offset_origin) {
    _proj.setOffsetOrigin(_params.grid_offset_origin_latitude,
                          _params.grid_offset_origin_longitude);
  } else {
    _proj.setOffsetCoords(_params.grid_false_northing,
                          _params.grid_false_easting);
  }

  if (LOG_IS_ENABLED(LogMsg::DEBUG_VERBOSE))
  {
    cerr << "============================" << endl;
    _proj.print(cerr, false);
    cerr << "============================" << endl;
  }        
}

//////////////////////////////////////////////////
// accumulate data for a pt using nearest neighbor
// this is for cart and ppi interpolation
//   i.e. interpolate between inner and outer gates

void Interp::_accumNearest(const Ray *ray,
                           int ifield,
                           int igateInner,
                           int igateOuter,
                           double wtInner,
                           double wtOuter,
                           double &closestVal,
                           double &maxWt,
                           int &nContrib)
  
{
    
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      if (igateInner >= 0 && igateInner < nGates) {
        fl32 val = field[igateInner];
        if (val != missing) {
          if (wtInner > maxWt) {
            closestVal = val;
            maxWt = wtInner;
          }
          nContrib++;
        }
      }
      
      if (igateOuter >= 0 && igateOuter < nGates) {
        fl32 val = field[igateOuter];
        if (val != missing) {
          if (wtOuter > maxWt) {
            closestVal = val;
            maxWt = wtOuter;
          }
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}

//////////////////////////////////////////////////
// accumulate data for a pt using nearest neighbor
// this is for a single gate

void Interp::_accumNearest(const Ray *ray,
                           int ifield,
                           int igate,
                           double wt,
                           double &closestVal,
                           double &maxWt,
                           int &nContrib)
  
{
    
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];
    
    if (field != NULL) {
      
      if (igate >= 0 && igate < nGates) {
        fl32 val = field[igate];
        if (val != missing) {
          if (wt > maxWt) {
            closestVal = val;
            maxWt = wt;
          }
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}

////////////////////////////////////////////////////////
// accumulate data for a grid point using interpolation
// this is for cart and ppi interpolation
//   i.e. interpolate between inner and outer gates

void Interp::_accumInterp(const Ray *ray,
                          int ifield,
                          int igateInner,
                          int igateOuter,
                          double wtInner,
                          double wtOuter,
                          double &sumVals,
                          double &sumWts,
                          int &nContrib)

{

  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      if (igateInner >= 0 && igateInner < nGates) {
        fl32 val = field[igateInner];
        if (val != missing) {
          sumVals += val * wtInner;
          sumWts += wtInner;
          nContrib++;
        }
      }
      
      if (igateOuter >= 0 && igateOuter < nGates) {
        fl32 val = field[igateOuter];
        if (val != missing) {
          sumVals += val * wtOuter;
          sumWts += wtOuter;
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}
  
////////////////////////////////////////////////////////
// accumulate data for a grid point using interpolation
// this is for a single gate

void Interp::_accumInterp(const Ray *ray,
                          int ifield,
                          int igate,
                          double wt,
                          double &sumVals,
                          double &sumWts,
                          int &nContrib)

{
    
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      if (igate >= 0 && igate < nGates) {
        fl32 val = field[igate];
        if (val != missing) {
          sumVals += val * wt;
          sumWts += wt;
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}

//////////////////////////////////////////////////
// set the radar details

int Interp::_setRadarParams()

{

  // set radar location
  
  _radarLat = _readVol.getLatitudeDeg();
  _radarLon = _readVol.getLongitudeDeg();
  _radarAltKm = _readVol.getAltitudeKm();

  // set beam width
  
  _beamWidthDegH = _readVol.getRadarBeamWidthDegH();
  _beamWidthDegV = _readVol.getRadarBeamWidthDegV();

  // set gate geometry

  _startRangeKm = _readVol.getStartRangeKm();
  _gateSpacingKm = _readVol.getGateSpacingKm();

  if (_startRangeKm <= -9990) {
    cerr << "ERROR - Interp::_setRadarParams()" << endl;
    cerr << "  Ray start range not set" << endl;
    cerr << "  startRangeKm: " << _startRangeKm << endl;
    cerr << "  You must use the option to override the gate geometry" << endl;
    cerr << "  in the param file" << endl;
    return -1;
  }

  if (_gateSpacingKm <= 0) {
    cerr << "ERROR - Interp::_setRadarParams()" << endl;
    cerr << "  Ray gate spacing not set" << endl;
    cerr << "  gateSpacingKm: " << _gateSpacingKm << endl;
    cerr << "  You must use the option to override the gate geometry" << endl;
    cerr << "  in the param file" << endl;
    return -1;
  }

  _maxNGates = 0;
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    int nGates = ray->getNGates();
    if (nGates > _maxNGates) {
      _maxNGates = nGates;
    }
  }
  _maxRangeKm = _startRangeKm + _maxNGates * _gateSpacingKm;

  return 0;

}

/////////////////////////////////////////////////////////////////
// locate empty sectors in the azimuth data
//
// Sets:
//  _isSector;
//  _dataSectorStartAzDeg
//  _dataSectorEndAzDeg
//
// Returns -1 if no data, 0 otherwise

int Interp::_locateDataSector()
            
{

  _isSector = false;
  _dataSectorStartAzDeg = 0.0;
  _dataSectorEndAzDeg = 360.0;

  // load array of number of rays per degree

  int nRaysPerDeg[360];
  for (int ii = 0; ii < 360; ii++) {
    nRaysPerDeg[ii] = 0;
  }
  const vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    const RadxRay *ray = rays[ii];
    double az = ray->getAzimuthDeg();
    int iaz = ((int) floor(az + 0.5)) % 360;
    nRaysPerDeg[iaz]++;
  }

  // find start of first empty sector
  
  int firstEmptySectorStart = -9999;
  for (int ii = 0; ii < 360; ii++) {
    if (nRaysPerDeg[ii] == 0) {
      firstEmptySectorStart = ii;
      break;
    }
  }
  if (firstEmptySectorStart < 0) {
    // no empty sectors
    return 0;
  }
  
  int firstFullSectorStart = -9999;
  for (int ii = firstEmptySectorStart + 1; ii < 360; ii++) {
    if (nRaysPerDeg[ii] > 0) {
      firstFullSectorStart = ii;
      break;
    }
  }
  if (firstFullSectorStart < 0) {
    // no data
    return -1;
  }

  // find all of the empty sectors, starting from the start of
  // the first full sector and checking for 360 degrees

  bool inGap = false;
  vector<Sector> emptySectors;
  Sector emptySector;
  for (int ii = firstFullSectorStart + 1;
       ii <= firstFullSectorStart + 360; ii++) {
    int jj = ii % 360;
    if (nRaysPerDeg[jj] == 0) {
      if (!inGap) {
        emptySector.startAzDeg = ii;
        inGap = true;
      }
    } else {
      if (inGap) {
        emptySector.endAzDeg = ii - 1;
        emptySector.computeWidth();
        emptySectors.push_back(emptySector);
        inGap = false;
      }
    }
  }

  if (emptySectors.size() < 1) {
    return 0;
  }
  
  int maxEmptyWidth = 0;
  size_t maxEmptyIndex = 0;
  for (size_t ii = 0; ii < emptySectors.size(); ii++) {
    if (emptySectors[ii].width > maxEmptyWidth) {
      maxEmptyIndex = ii;
      maxEmptyWidth = emptySectors[ii].width;
    }
    LOGF(LogMsg::DEBUG_VERBOSE, "Found sector, num: %d", ii);
    LOGF(LogMsg::DEBUG_VERBOSE, "  startAz: %lf", emptySectors[ii].startAzDeg);
    LOGF(LogMsg::DEBUG_VERBOSE, "  endAz:   %lf", emptySectors[ii].endAzDeg);
    LOGF(LogMsg::DEBUG_VERBOSE, "  width:   %lf", emptySectors[ii].width);
  }

  LOGF(LogMsg::DEBUG_VERBOSE, "-->> empty sector with max width %lf",
       maxEmptyWidth);
  LOGF(LogMsg::DEBUG_VERBOSE, "  start, end, width: %lf %lf %lf",
       emptySectors[maxEmptyIndex].startAzDeg,
       emptySectors[maxEmptyIndex].endAzDeg,
       emptySectors[maxEmptyIndex].width);

  if (maxEmptyWidth < 30) {
    // no significant empty sector
    return 0;
  }

  _isSector = true;
  double azBuffer = _scanDeltaAz + _beamWidthDegH + 1.0;
  _dataSectorStartAzDeg = 
    emptySectors[maxEmptyIndex].endAzDeg - azBuffer;
  _dataSectorEndAzDeg =
    emptySectors[maxEmptyIndex].startAzDeg + azBuffer + 360.0;
  
  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorStartAzDeg: %lf", 
       _dataSectorStartAzDeg);
  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorEndAzDeg:  %lf",
       _dataSectorEndAzDeg);
  
  if (_dataSectorStartAzDeg >= 360.0 &&
      _dataSectorEndAzDeg >= 360.0) {
    _dataSectorStartAzDeg -= 360;
    _dataSectorEndAzDeg -= 360;
  }

  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorStartAzDeg: %lf", 
       _dataSectorStartAzDeg);
  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorEndAzDeg: %lf", 
       _dataSectorEndAzDeg);
  
  _spansNorth = false;
  if (_dataSectorStartAzDeg < 360 && _dataSectorEndAzDeg > 360) {
    _spansNorth = true;
  }

  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorStartAzDeg: %lf",
       _dataSectorStartAzDeg);
  LOGF(LogMsg::DEBUG_VERBOSE, "   dataSectorEndAzDeg: %lf", 
       _dataSectorEndAzDeg);

  return 0;
  
}

/////////////////////////////////////////////////////
// Compute the azimuth difference to be used for
// searching

void Interp::_computeAzimuthDelta()
{

  // initialize to a reasonable value

  _scanDeltaAz = 2.0;

  // not an RHI volume

  // compute histogram of delta azimuths
  // use resolution of 0.1 degrees, up to a max
  // delta of 10 degrees

  static const int nHist = 100;
  int hist[nHist];
  memset(hist, 0, nHist * sizeof(int));
    
  const vector<RadxRay *> &rays = _readVol.getRays();
  double prevAz = rays[0]->getAzimuthDeg();
  int count = 0;
  for (size_t ii = 1; ii < rays.size(); ii++) {
    double az = rays[ii]->getAzimuthDeg();
    double deltaAz = fabs(az - prevAz);
    prevAz = az;
    if (deltaAz > 180.0) {
      deltaAz -= 360.0;
    }
    int index = (int) (deltaAz * 10.0 + 0.5);
    if (index >= 0 && index < nHist) {
      hist[index]++;
      count++;
    }
  }

  // find median delta

  int medianIndex = 0;
  int sum = 0;
  int halfCount = count / 2;
  for (int ii = 0; ii < nHist; ii++) {
    sum += hist[ii];
    if (sum >= halfCount) {
      medianIndex = ii;
      break;
    }
  }

  _scanDeltaAz = medianIndex / 10.0;
    
}

/////////////////////////////////////////////////////
// Compute the elevation difference to be used for
// searching

void Interp::_computeElevationDelta()
{

  // initialize to a reasonable value

  _scanDeltaEl = 10.0;

  // not an RHI volume
  // so find the max difference between sweep fixed angles
    
  if (_readVol.getNSweeps() >= 2) {
    const vector<RadxSweep *> &sweeps = _readVol.getSweeps();
    double maxDeltaEl = 1.0;
    double prevEl = sweeps[0]->getFixedAngleDeg();
    for (size_t ii = 1; ii < _readVol.getNSweeps(); ii++) {
      double el = sweeps[ii]->getFixedAngleDeg();
      double deltaEl = fabs(el - prevEl);
      prevEl = el;
      if (deltaEl > maxDeltaEl) {
	maxDeltaEl = deltaEl;
      }
    }
    _scanDeltaEl = maxDeltaEl;
  }
}

// Print the elapsed run time since the previous call, in seconds.

void Interp::_printRunTime(const string& str, bool verbose /* = false */)
{
  LogMsg::Severity_t s;
  if (verbose) {
    s = LogMsg::DEBUG_VERBOSE;
  }
  else {
    s = LogMsg::DEBUG;
  }

  struct timeval tvb;
  gettimeofday(&tvb, NULL);
  double deltaSec = tvb.tv_sec - _timeA.tv_sec
    + 1.e-6 * (tvb.tv_usec - _timeA.tv_usec);
  LOGF(s, "TIMING, task: %s, secs used: %d", str.c_str(), deltaSec);
  _timeA.tv_sec = tvb.tv_sec;
  _timeA.tv_usec = tvb.tv_usec;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////////////////
// sub class implementation

Interp::Ray::Ray(RadxRay *ray,
                 int isweep,
                 const vector<Field> &interpFields)
  
{

  inputRay = ray;
  sweepIndex = isweep;
  inputRay->convertToFl32();
  nGates = inputRay->getNGates();
  el = inputRay->getElevationDeg();
  az = inputRay->getAzimuthDeg();
  
  cosEl = cos(el * DEG_TO_RAD);
  while (az < 0) {
    az += 360.0;
  }
  while (az > 360) {
    az -= 360.0;
  }

  elForLimits = inputRay->getFixedAngleDeg();
  azForLimits = inputRay->getAzimuthDeg();

  int nFields = interpFields.size();
  fldData = new fl32*[nFields];
  missingVal = new fl32[nFields];

  for (int ii = 0; ii < nFields; ii++) {
    string fieldName = interpFields[ii].radxName;
    RadxField *fld = inputRay->getField(fieldName);
    if (fld) {
      fldData[ii] = (fl32 *) fld->getDataFl32();
      missingVal[ii] = fld->getMissingFl32();
    } else {
      fldData[ii] = NULL;
    }
  } // ii

}

Interp::Ray::~Ray()
  
{
  delete[] fldData;
  delete[] missingVal;
}

