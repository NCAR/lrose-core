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
#include "OutputMdv.hh"
#include <algorithm>
#include <iomanip>
#include <toolsa/pmu.h>
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
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
using namespace std;

const double Interp::PseudoDiamKm = 17066.0; // for earth curvature correction
const double Interp::missingDouble = -9999.0;
const fl32 Interp::missingFl32 = -9999.0;

// Constructor

Interp::Interp(const string &progName,
               const Params &params,
               RadxVol &readVol,
               vector<Field> &interpFields,
               vector<Ray *> &interpRays) :
        _progName(progName),
        _params(params),
        _readVol(readVol),
        _interpFields(interpFields),
        _interpRays(interpRays)
        
{

  _rhiMode = false;

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

  _proj.latlon2xy(_readVol.getLatitudeDeg(), _readVol.getLongitudeDeg(),
                  _radarX, _radarY);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "============ Interp::_initProjection() ===============" << endl;
    _proj.print(cerr, false);
    cerr << "  radarX: " << _radarX << endl;
    cerr << "  radarY: " << _radarY << endl;
    cerr << "======================================================" << endl;
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
  
////////////////////////////////////////////////////////
// accumulate data for a grid point using interpolation
// for a folded field
// this is for cart and ppi interpolation
//   i.e. interpolate between inner and outer gates

void Interp::_accumFolded(const Ray *ray,
                          int ifield,
                          int igateInner,
                          int igateOuter,
                          double wtInner,
                          double wtOuter,
                          double &sumX,
                          double &sumY,
                          double &sumWts,
                          int &nContrib)

{
    
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      const Field &intFld = _interpFields[ifield];

      if (igateInner >= 0 && igateInner < nGates) {
        fl32 val = field[igateInner];
        if (val != missing) {
          double angle =
            _getFoldAngle(val, intFld.foldLimitLower, intFld.foldRange);
          double sinVal, cosVal;
          ta_sincos(angle, &sinVal, &cosVal);
          sumX += cosVal * wtInner;
          sumY += sinVal * wtInner;
          sumWts += wtInner;
          nContrib++;
        }
      }
      
      if (igateOuter >= 0 && igateOuter < nGates) {
        fl32 val = field[igateOuter];
        if (val != missing) {
          double angle = 
            _getFoldAngle(val, intFld.foldLimitLower, intFld.foldRange);
          double sinVal, cosVal;
          ta_sincos(angle, &sinVal, &cosVal);
          sumX += cosVal * wtOuter;
          sumY += sinVal * wtOuter;
          sumWts += wtOuter;
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}
  
////////////////////////////////////////////////////////
// accumulate data for a grid point using interpolation
// for a folded field
// this is for a single gate

void Interp::_accumFolded(const Ray *ray,
                          int ifield,
                          int igate,
                          double wt,
                          double &sumX,
                          double &sumY,
                          double &sumWts,
                          int &nContrib)

{
    
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      const Field &intFld = _interpFields[ifield];

      if (igate >= 0 && igate < nGates) {
        fl32 val = field[igate];
        if (val != missing) {
          double angle =
            _getFoldAngle(val, intFld.foldLimitLower, intFld.foldRange);
          double sinVal, cosVal;
          ta_sincos(angle, &sinVal, &cosVal);
          sumX += cosVal * wt;
          sumY += sinVal * wt;
          sumWts += wt;
          nContrib++;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  
}
  
/////////////////////////////////////////////////////////////////
// convert a value to an angle, for a field that folds

double Interp::_getFoldAngle(double val,
                             double foldLimitLower,
                             double foldRange) const
{
  double fraction = (val - foldLimitLower) / foldRange;
  double angle = -M_PI + fraction * (M_PI * 2.0);
  return angle;
}

/////////////////////////////////////////////////////////////////
// convert an angle to a value, for a field that folds

double Interp::_getFoldValue(double angle,
                             double foldLimitLower,
                             double foldRange) const
{
  double fraction = (angle + M_PI) / (M_PI * 2.0);
  double val = fraction * foldRange + foldLimitLower;
  return val;
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

  if (_params.interp_mode != Params::INTERP_MODE_CART_SAT) {
    if (_beamWidthDegH <= 0 || _beamWidthDegV <= 0) {
      cerr << "ERROR - Interp::_setRadarParams()" << endl;
      cerr << "  Radar beam width not set" << endl;
      cerr << "  beamWidthDegH: " << _beamWidthDegH << endl;
      cerr << "  beamWidthDegV: " << _beamWidthDegV << endl;
      cerr << "  You must use the option to override the beam width" << endl;
      cerr << "  in the param file" << endl;
      return -1;
    }
  }

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
    if (_params.debug >= Params::DEBUG_EXTRA) {
      cerr << "Found sector, num: " << ii << endl;
      cerr << "  startAz: " << emptySectors[ii].startAzDeg << endl;
      cerr << "  endAz: " << emptySectors[ii].endAzDeg << endl;
      cerr << "  width: " << emptySectors[ii].width << endl;
    }
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "-->> empty sector with max width" << endl;
    cerr << "  start, end, width: "
         << emptySectors[maxEmptyIndex].startAzDeg << ", " 
         << emptySectors[maxEmptyIndex].endAzDeg << ", "
         << emptySectors[maxEmptyIndex].width << endl;
  }

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
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "   dataSectorStartAzDeg: " << _dataSectorStartAzDeg << endl;
    cerr << "   dataSectorEndAzDeg: " << _dataSectorEndAzDeg << endl;
  }
  
  if (_dataSectorStartAzDeg >= 360.0 &&
      _dataSectorEndAzDeg >= 360.0) {
    _dataSectorStartAzDeg -= 360;
    _dataSectorEndAzDeg -= 360;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "   dataSectorStartAzDeg: " << _dataSectorStartAzDeg << endl;
    cerr << "   dataSectorEndAzDeg: " << _dataSectorEndAzDeg << endl;
  }
  
  _spansNorth = false;
  if (_dataSectorStartAzDeg < 360 && _dataSectorEndAzDeg > 360) {
    _spansNorth = true;
  }

  if (_params.debug >= Params::DEBUG_EXTRA) {
    cerr << "   dataSectorStartAzDeg: " << _dataSectorStartAzDeg << endl;
    cerr << "   dataSectorEndAzDeg: " << _dataSectorEndAzDeg << endl;
  }

  return 0;
  
}

/////////////////////////////////////////////////////
// Compute the azimuth difference to be used for
// searching

void Interp::_computeAzimuthDelta()
{

  // initialize to a reasonable value

  _scanDeltaAz = 2.0;

  if (_rhiMode) {

    // this is an RHI volume
    // so find the median difference between sweep fixed angles

    if (_readVol.getNSweeps() >= 2) {
      const vector<RadxSweep *> &sweeps = _readVol.getSweeps();
      vector<double> deltas;
      double prevAz = sweeps[0]->getFixedAngleDeg();
      for (size_t ii = 1; ii < _readVol.getNSweeps(); ii++) {
        double az = sweeps[ii]->getFixedAngleDeg();
        double deltaAz = fabs(az - prevAz);
        prevAz = az;
        if (deltaAz > 180.0) {
          deltaAz -= 360.0;
        }
        deltas.push_back(deltaAz);
      }
      sort(deltas.begin(), deltas.end());
      _scanDeltaAz = deltas[deltas.size()/2];
    }
    
  } else {

    // not an RHI volume

    // compute histogram of delta azimuths
    // use resolution of 0.1 degrees, up to a max
    // delta of 10 degrees

    static const int nHist = 100;
    int hist[nHist];
    memset(hist, 0, nHist * sizeof(int));
    
    vector<RadxRay *> &rays = _readVol.getRays();
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

    if (medianIndex == 0)
      _scanDeltaAz = 1.0;
    else
      _scanDeltaAz = medianIndex / 10.0;
    
  } // if (_rhiMode)

}

/////////////////////////////////////////////////////
// Compute the elevation difference to be used for
// searching

void Interp::_computeElevationDelta()
{

  // initialize to a reasonable value

  _scanDeltaEl = 10.0;

  if (_rhiMode) {

    // this is an RHI volume
    // compute histogram of delta elevations
    // use resolution of 0.1 degrees, up to a max
    // delta of 10 degrees

    static const int nHist = 100;
    int hist[nHist];
    memset(hist, 0, nHist * sizeof(int));
    
    vector<RadxRay *> &rays = _readVol.getRays();
    double prevEl = rays[0]->getElevationDeg();
    int count = 0;
    for (size_t ii = 1; ii < rays.size(); ii++) {
      double el = rays[ii]->getElevationDeg();
      double deltaEl = fabs(el - prevEl);
      prevEl = el;
      int index = (int) (deltaEl * 10.0 + 0.5);
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
    
    _scanDeltaEl = medianIndex / 10.0;
    
  } else {

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
    
  } // if (_rhiMode)

}

// Print the elapsed run time since the previous call, in seconds.

void Interp::_printRunTime(const string& str, bool verbose /* = false */)
{
  if (verbose) {
    if (_params.debug < Params::DEBUG_VERBOSE) {
      return;
    }
  } else if (!_params.debug) {
    return;
  }
  struct timeval tvb;
  gettimeofday(&tvb, NULL);
  double deltaSec = tvb.tv_sec - _timeA.tv_sec
    + 1.e-6 * (tvb.tv_usec - _timeA.tv_usec);
  cerr << "TIMING, task: " << str << ", secs used: " << deltaSec << endl;
  _timeA.tv_sec = tvb.tv_sec;
  _timeA.tv_usec = tvb.tv_usec;
}

/////////////////////////////////////////////////////
// write out data in CEDRIC format

int Interp::_writeCedricFile(bool isPpi)
{

  if (_params.debug) {
    cerr << "  Writing CEDRIC file ... " << endl;
  }

  const RadxRay *ray0 = _readVol.getRays()[0];

  Cedric ced;
  if (_params.debug) {
    ced.setDebug(true);
  }

  // set meta data
  
  ced.setDateTimeRun(time(NULL));
  ced.setVolStartTime(_readVol.getStartTimeSecs());
  ced.setVolEndTime(_readVol.getEndTimeSecs());
  ced.setTapeStartTime(_readVol.getStartTimeSecs());
  ced.setTapeEndTime(_readVol.getEndTimeSecs());
  ced.setTimeZone("UTC");
  
  ced.setVolumeNumber(_readVol.getVolumeNumber());
  ced.setVolName(_readVol.getInstrumentName());
  ced.setRadar(_readVol.getInstrumentName());
  ced.setProject(_readVol.getSource());
  ced.setProgram(_readVol.getHistory());
  if (isPpi) {
    if (_params.grid_projection == Params::PROJ_LATLON) {
      ced.setCoordType("LLE ");
    } else {
      ced.setCoordType("ELEV");
    }
  } else {
    if (_params.grid_projection == Params::PROJ_LATLON) {
      ced.setCoordType("LLZ ");
    } else {
      ced.setCoordType("CRT ");
    }
  }
  ced.setXaxisAngleFromNDeg(90.0);
  ced.setOriginX(0.0);
  ced.setOriginY(0.0);

  // ced.setLongitudeDeg(_readVol.getLongitudeDeg());
  // ced.setLatitudeDeg(_readVol.getLatitudeDeg());
  ced.setLongitudeDeg(_gridOriginLon);
  ced.setLatitudeDeg(_gridOriginLat);
  ced.setOriginHtKm(0);
  ced.setNyquistVel(ray0->getNyquistMps());

  ced.addLandmark(_readVol.getInstrumentName(),
                  _radarX, _radarY, _readVol.getAltitudeKm());

  ced.setMinRangeKm(_readVol.getStartRangeKm());
  ced.setMaxRangeKm(_readVol.getStartRangeKm() +
                    _readVol.getGateSpacingKm() * ray0->getNGates());
  ced.setNumGatesBeam(ray0->getNGates());
  ced.setGateSpacingKm(_readVol.getGateSpacingKm());
  ced.setMinAzimuthDeg(_dataSectorStartAzDeg);
  ced.setMaxAzimuthDeg(_dataSectorEndAzDeg);

  // mhdr.sensor_alt = _readVol.getAltitudeKm();
  
  ced.setMinX(_gridMinx);
  ced.setMaxX(_gridMinx + (_gridNx - 1) * _gridDx);
  ced.setNx(_gridNx);
  ced.setDx(_gridDx);
  ced.setFastAxis(1);
  
  ced.setMinY(_gridMiny);
  ced.setMaxY(_gridMiny + (_gridNy - 1) * _gridDy);
  ced.setNy(_gridNy);
  ced.setDy(_gridDy);
  ced.setMidAxis(2);

  ced.setMinZ(_gridZLevels[0]);
  ced.setMaxZ(_gridZLevels[_gridZLevels.size()-1]);
  ced.setNz(_gridZLevels.size());
  ced.setDz((_gridZLevels[1] - _gridZLevels[0]));
  ced.setSlowAxis(3);
  ced.setPlaneType("PP");

  const vector<RadxSweep *>sweeps = _readVol.getSweeps();
  ced.setNumElevs(sweeps.size());
  double minElev = 9999;
  double maxElev = -9999;
  double sumElev = 0.0;
  double sumDeltaElev = 0.0;
  double prevElev = -9999;
  for (size_t ii = 0; ii < sweeps.size(); ii++) {
    double elev = sweeps[ii]->getFixedAngleDeg();
    sumElev += elev;
    if (elev > maxElev) maxElev = elev;
    if (elev < minElev) minElev = elev;
    if (prevElev > -9990) {
      double deltaElev = fabs(elev - prevElev);
      sumDeltaElev += deltaElev;
    }
    prevElev = elev;
  }
  ced.setMinElevDeg(minElev);
  ced.setMaxElevDeg(maxElev);
  ced.setAveElevDeg(sumElev / sweeps.size());
  ced.setAveDeltaElevDeg(sumDeltaElev / sweeps.size());

  for (size_t ii = 0; ii < _gridZLevels.size(); ii++) {
    ced.addVlevel(ii, _gridZLevels[ii],
                  _interpFields.size(),
                  _gridNx, _gridNy,
                  ray0->getNyquistMps());
  }

  ced.setNumRadars(1);
  if (_readVol.getNRcalibs() > 0) {
    ced.setRadarConst(_readVol.getRcalibs()[0]->getRadarConstantH());
  }

  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    const Field &ifld = _interpFields[ifield];
    ced.addField(ifld.outputName, _outputFields[ifield], missingFl32);
  }

  if (_params.specify_output_filename) {
    string outputPath = _params.output_dir;
    outputPath += PATH_DELIM;
    outputPath += _params.output_filename;
    if (ced.writeToDir(outputPath, _progName)) {
      cerr << "ERROR - CartInterp::_writeCedricFile()" << endl;
      return -1;
    }
  } else {
    if (ced.writeToDir(_params.output_dir, _progName)) {
      cerr << "ERROR - CartInterp::_writeCedricFile()" << endl;
      return -1;
    }
  }

  return 0;

}

//////////////////////////////////////////////////////////////////
// transform fields back as required for output

void Interp::_transformForOutput()

{

  if (!_params.transform_fields_for_interpolation) {
    return;
  }

  // loop through interp fields
  
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    
    Field &ifld = _interpFields[ifield];
    string &radxName = ifld.radxName;
    
    // loop through transform fields
    
    for (int jfield = 0; jfield < _params.transform_fields_n; jfield++) {

      const Params::transform_field_t &transform =
        _params._transform_fields[jfield];
      string transName = transform.output_name;
      if (radxName == transName) {
        
        if (transform.transform ==
            Params::TRANSFORM_DB_TO_LINEAR_AND_BACK) {
          
          fl32 *data = _outputFields[ifield];
          for (int ipt = 0; ipt < _nPointsVol; ipt++) {
            fl32 val = data[ipt];
            if (val != Interp::missingFl32) {
              if (val <= 0.0) {
                data[ipt] = Interp::missingFl32;
              } else {
                data[ipt] = 10.0 * log10(val);
              }
            }
          } // ipt
          
        } else if (transform.transform ==
                   Params::TRANSFORM_LINEAR_TO_DB_AND_BACK) {
          
          fl32 *data = _outputFields[ifield];
          for (int ipt = 0; ipt < _nPointsVol; ipt++) {
            fl32 val = data[ipt];
            if (val != Interp::missingFl32) {
              data[ipt] = pow(10.0, val / 10.0);
            }
          } // ipt
        
        } // if (transform.transform ...

      } // if (outputName == transName)

    } // jfield

  } // ifield

}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////////////////
// sub class implementation

Interp::Ray::Ray(RadxRay *ray,
                 int isweep,
                 const vector<Field> &interpFields,
                 bool use_fixed_angle_for_interpolation,
                 bool use_fixed_angle_for_data_limits)
  
{

  inputRay = ray;
  sweepIndex = isweep;
  inputRay->convertToFl32();

  bool ppiMode = true;
  if (inputRay->getSweepMode() == Radx::SWEEP_MODE_RHI ||
      inputRay->getSweepMode() == Radx::SWEEP_MODE_ELEVATION_SURVEILLANCE) {
    ppiMode = false;
  }

  nGates = inputRay->getNGates();

  el = inputRay->getElevationDeg();
  az = inputRay->getAzimuthDeg();
  
  if (use_fixed_angle_for_interpolation) {
    if (ppiMode) {
      el = inputRay->getFixedAngleDeg();
    } else {
      az = inputRay->getFixedAngleDeg();
    }
  }

  cosEl = cos(el * DEG_TO_RAD);
  while (az < 0) {
    az += 360.0;
  }
  while (az > 360) {
    az -= 360.0;
  }

  elForLimits = inputRay->getElevationDeg();
  azForLimits = inputRay->getAzimuthDeg();
  if (use_fixed_angle_for_data_limits) {
    if (ppiMode) {
      elForLimits = inputRay->getFixedAngleDeg();
    } else {
      azForLimits = inputRay->getFixedAngleDeg();
    }
  }

  nFields = interpFields.size();
  fldData = new fl32*[nFields];
  missingVal = new fl32[nFields];
  
  for (int ii = 0; ii < nFields; ii++) {
    string fieldName = interpFields[ii].radxName;
    RadxField *fld = inputRay->getField(fieldName);
    if (fld) {
      fldData[ii] = new fl32[fld->getNPoints()];
      memcpy(fldData[ii], fld->getDataFl32(), fld->getNPoints() * sizeof(fl32));
      // fldData[ii] = (fl32 *) fld->getDataFl32();
      missingVal[ii] = fld->getMissingFl32();
    } else {
      fldData[ii] = NULL;
      missingVal[ii] = Radx::missingFl32;
    }
  } // ii

}

Interp::Ray::~Ray()
  
{

  for (int ii = 0; ii < nFields; ii++) {
    if (fldData[ii] != NULL) {
      delete[] fldData[ii];
    }
  } // ii

  delete[] fldData;
  delete[] missingVal;
}

