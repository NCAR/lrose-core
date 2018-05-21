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
// PolarInterp class - derived from Interp.
// Used for interpolation onto a regular polar grid
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#include "PolarInterp.hh"
#include "OutputMdv.hh"
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <rapformats/Cedric.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxSweep.hh>
using namespace std;

const double PolarInterp::_searchResAz = 0.1;
const double PolarInterp::_searchAzOverlapDeg = 20.0;
const double PolarInterp::_searchAzOverlapHalf =
  PolarInterp::_searchAzOverlapDeg / 2.0;

// Constructor

PolarInterp::PolarInterp(const string &progName,
                     const Params &params,
                     RadxVol &readVol,
                     vector<Field> &interpFields,
                     vector<Ray *> &interpRays) :
        Interp(progName,
               params,
               readVol,
               interpFields,
               interpRays)
        
{

  _searchLeft = NULL;
  _searchRight = NULL;

  _prevRadarLat = _prevRadarLon = _prevRadarAltKm = -9999.0;
  _outputFields = NULL;

  // set up thread objects

  _createThreads();
  
}

//////////////////////////////////////
// destructor

PolarInterp::~PolarInterp()

{

  _freeThreads();

  // TODO: check on this section
  // the following causes a segv for some reason 

  if (_params.debug >= Params::DEBUG_EXTRA) {
    if (_searchLeft) {
      ufree2((void **) _searchLeft);
    }
    if (_searchRight) {
      ufree2((void **) _searchRight);
    }
  }
  
  _freeOutputArrays();

}

//////////////////////////////////////////////////
// free the threading objects

void PolarInterp::_freeThreads()
{
  // thread pools free their threads in destructor
}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int PolarInterp::interpVol()

{

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - PolarInterp::interpVol()" << endl;
    return -1;
  }

  // compute the scan azimuth delta angle
  // will be used for the azimuth resolution
  
  _computeAzimuthDelta();
  if (_params.debug) {
    cerr << "  _scanDeltaAz: " << _scanDeltaAz << endl;
  }
  
  // locate empty sectors in azimuth
  
  if (_locateDataSector()) {
    // no data
    if (_params.debug) {
      cerr << "WARNING - no data found" << endl;
    }
    return -1;
  }

  if (_params.debug) {
    cerr << "  _isSector: " << _isSector << endl;
    cerr << "  _spansNorth: " << _spansNorth << endl;
  }

  // set the min and delta az for the grid

  _computeAzGridDetails();

  // initialize the output grid dimensions

  _initZLevels();
  _initGrid();
  _initProjection();

  // compute search matrix angle limits - keep the matrix
  // as small as possible for efficiency
  
  _computeSearchLimits();
  
  // fill the search matrix

  if (_params.debug) {
    cerr << "  Filling search matrix ... " << endl;
  }
  _fillSearchMatrix();

  // interpolate

  if (_params.debug) {
    cerr << "  Interpolating ... " << endl;
  }
  _doInterp();

  // transform for output

  _transformForOutput();

  // write out data

  if (_writeOutputFile()) {
    cerr << "ERROR - Interp::processFile" << endl;
    cerr << "  Cannot write output file" << endl;
    return -1;
  }

  return 0;

}

//////////////////////////////////////////////////
// Compute grid az details

void PolarInterp::_computeAzGridDetails()
{

  // compute min and maz az
  double minAz = 0.0;
  double maxAz = 360.0;
  if (_isSector) {
    minAz = _dataSectorStartAzDeg;
    maxAz = _dataSectorEndAzDeg;
    if (minAz >= 360) {
      minAz -= 360.0;
      maxAz -= 360.0;
    }
  }

  // comptute az range, determine number of az locations

  double azRange = maxAz - minAz;
  double deltaAz = _scanDeltaAz;
  int nAz = (int) floor (azRange / deltaAz + 0.5);
  if (!_isSector) {
    deltaAz = 360.0 / nAz;
  }

  // cerr << "111111111111 minAz: " << minAz << endl;
  // cerr << "111111111111 maxAz: " << maxAz << endl;
  // cerr << "111111111111 deltaAz: " << deltaAz << endl;
  // cerr << "111111111111 nAz: " << nAz << endl;
  
  // check for consistent offset
  
  double sum = 0.0;
  double count = 0.0;
  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    const Ray *ray = _interpRays[iray];
    int azIndex = (int) ((ray->az - minAz) / deltaAz);
    double az = minAz + azIndex * deltaAz;
    double offset = az - ray->az;
    sum += offset;
    count++;
  }
  double meanAzOffset = sum / count;
  // cerr << "22222222222 meanAzOffset: " << meanAzOffset << endl;

  // modify minAz by offset, so that on average the rays lie in the
  // center of the grid cell

  minAz -= meanAzOffset;

  _nAz = nAz;
  _deltaAz = deltaAz;
  _minAz = minAz;
    
}

//////////////////////////////////////////////////
// Compute the search matrix limits
// keeping it as small as possible for efficiency

void PolarInterp::_computeSearchLimits()
{

  // set azimuth resolution and search radius
  
  double minAzDeg = 0;
  double maxAzDeg = 360.0 + _searchAzOverlapDeg;
  
  double searchAzRangeDeg = maxAzDeg - minAzDeg;
  _searchMinAz = minAzDeg;
  _searchNAz = (int) (searchAzRangeDeg / _searchResAz + 1);
  
  if (_params.debug) {
    cerr << "  _searchMinAz: " << _searchMinAz << endl;
    cerr << "  _searchNAz: " << _searchNAz << endl;
  }

}

////////////////////////////////////////////////////////////
// Initialize Z levels

void PolarInterp::_initZLevels()

{

  _gridZLevels.clear();
  _nEl = _readVol.getNSweeps();
  _gridNz = _nEl;
  for (int ii = 0; ii < _gridNz; ii++) {
    double elev = _readVol.getSweeps()[ii]->getFixedAngleDeg();
    _gridZLevels.push_back(elev);
  }

}

////////////////////////////////////////////////////////////
// Initialize output grid

void PolarInterp::_initGrid()

{

  _gridNx = _readVol.getMaxNGates();
  _gridMinx = _readVol.getStartRangeKm();
  _gridDx = _readVol.getGateSpacingKm();
  
  _gridNy = _nAz;
  _gridMiny = _minAz;
  _gridDy = _deltaAz;

}
  
//////////////////////////////////////////////////
// initialize the threading objects

void PolarInterp::_createThreads()
{

  // initialize thread pool for interpolation

  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    PerformInterp *thread = new PerformInterp(this);
    _threadPoolInterp.addThreadToMain(thread);
  }

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void PolarInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _nEl;
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void PolarInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void PolarInterp::_initOutputArrays()
  
{

  _allocOutputArrays();

  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    for (int jj = 0; jj < _nPointsVol; jj++) {
      _outputFields[ii][jj] = missingFl32;
    }
  }

}

////////////////////////////////////////////////////////////
// Initialize projection

void PolarInterp::_initProjection()

{

  if (_params.center_grid_on_radar) {
    _gridOriginLat = _radarLat;
    _gridOriginLon = _radarLon;
  } else {
    _gridOriginLat = _params.grid_origin_lat;
    _gridOriginLon = _params.grid_origin_lon;
  }

  _proj.setGrid(_gridNx, _gridNy,
                _gridDx, _gridDy,
                _gridMinx, _gridMiny);
  
  _proj.initPolarRadar(_gridOriginLat,
                       _gridOriginLon);

}

////////////////////////////////////////////////////////////
// Allocate the search matrix

void PolarInterp::_allocSearchMatrix()

{

  _searchLeft = 
    (SearchPoint **) umalloc2(_nEl, _searchNAz, sizeof(SearchPoint));

  _searchRight = 
    (SearchPoint **) umalloc2(_nEl, _searchNAz, sizeof(SearchPoint));


}

////////////////////////////////////////////////////////////
// Free up the search matrix

void PolarInterp::_freeSearchMatrix()

{
  if (_searchLeft) {
    ufree2((void **) _searchLeft);
    _searchLeft = NULL;
  }
  if (_searchRight) {
    ufree2((void **) _searchRight);
    _searchRight = NULL;
  }
}

////////////////////////////////////////////////////////////
// Initalize the search matrix

void PolarInterp::_initSearchMatrix()

{

  for (int iel = 0; iel < _nEl; iel++) {
    for (int iaz = 0; iaz < _searchNAz; iaz++) {
      _searchLeft[iel][iaz].clear();
      _searchRight[iel][iaz].clear();
    }
  }

  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    
    const Ray *ray = _interpRays[iray];

    // get sweep index

    int iel = ray->sweepIndex;
    
    // compute azimuth index
    
    double az = ray->az;
    int iaz = _getSearchAzIndex(az);
    if (iaz < 0) {
      continue;
    }

    // set rays at index location

    _searchLeft[iel][iaz].ray = ray;
    _searchLeft[iel][iaz].rayEl = ray->el;
    _searchLeft[iel][iaz].rayAz = ray->az;

    _searchRight[iel][iaz].ray = ray;
    _searchRight[iel][iaz].rayEl = ray->el;
    _searchRight[iel][iaz].rayAz = ray->az;

  } // iray

  // copy the low azimuths into extra region at the
  // high end, so that we can interpolate across the N line
  // so search can cross N line
  
  int sourceIndexStart = _getSearchAzIndex(0.0);
  int sourceIndexEnd = _getSearchAzIndex(_searchAzOverlapDeg);
  int targetIndexStart = _getSearchAzIndex(360.0);
  int targetOffset = targetIndexStart - sourceIndexStart;
  
  for (int iaz = sourceIndexStart; iaz <= sourceIndexEnd; iaz++) {
    int jaz = iaz + targetOffset;
    if (jaz < _searchNAz) {
      for (int iel = 0; iel < _nEl; iel++) {
        _searchLeft[iel][jaz] = _searchLeft[iel][iaz];
        _searchRight[iel][jaz] = _searchRight[iel][iaz];
        _searchLeft[iel][jaz].rayAz += 360.0;
        _searchRight[iel][jaz].rayAz += 360.0;
      }
    }
  }
  
}


////////////////////////////////////////////////////////////
// Fill the search matrix, using the el/az for each ray
// in the current volume

void PolarInterp::_fillSearchMatrix()

{

  _freeSearchMatrix();
  _allocSearchMatrix();
  _initSearchMatrix();

  _fillLeft();
  _fillRight();

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printSearchMatrix(stderr, 1);
  }

}

///////////////////////////////////////////////////////////
// print search matrix

void PolarInterp::_printSearchMatrix(FILE *out, int res)
{

  for (int iel = 0; iel < _nEl; iel += res) {
    for (int iaz = 0; iaz < _searchNAz; iaz += res) {
      
      const SearchPoint &left = _searchLeft[iel][iaz];
      const SearchPoint &right = _searchRight[iel][iaz];
      
      if (!left.ray && !right.ray) {
        continue;
      }
      
      fprintf(out,
              "iel, iaz, az: "
              "%4d %4d "
              "%7.3f ",
              iel, iaz,
              iaz * _searchResAz + _searchMinAz);

      if (left.ray) {
        fprintf(out, "LEFT %7.3f %7.3f ", left.rayEl, left.rayAz);
      } else {
        fprintf(out, "LEFT %7.3f %7.3f ", -99.999, -99.999);
      }
      if (right.ray) {
        fprintf(out, "RIGHT %7.3f %7.3f ", right.rayEl, right.rayAz);
      } else {
        fprintf(out, "RIGHT %7.3f %7.3f ", -99.999, -99.999);
      }
      fprintf(out, "\n");

    } // iaz
  } // iel

}

///////////////////////////////////////////////////////////
// print point in search matrix

void PolarInterp::_printSearchMatrixPoint(FILE *out, int iel, int iaz)
{

  const SearchPoint &left = _searchLeft[iel][iaz];
  const SearchPoint &right = _searchRight[iel][iaz];
  
  cout << "----------------------------------------------" << endl;
  
  fprintf(out,
          "Search matrix point: iel, iaz, az: "
          "%4d %4d "
          "%7.3f ",
          iel, iaz,
          iaz * _searchResAz + _searchMinAz);
  
  if (left.ray) {
    fprintf(out, "LEFT %7.3f %7.3f ", left.rayEl, left.rayAz);
  } else {
    fprintf(out, "LEFT %7.3f %7.3f ", -99.999, -99.999);
  }
  if (right.ray) {
    fprintf(out, "RIGHT %7.3f %7.3f ", right.rayEl, right.rayAz);
  } else {
    fprintf(out, "RIGHT %7.3f %7.3f ", -99.999, -99.999);
  }
  fprintf(out, "\n");
  
  cout << "----------------------------------------------" << endl;

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray to the left of the search point
// We do this by propagating the ray information to the right.

void PolarInterp::_fillLeft()

{

  for (int iel = 0; iel < _nEl; iel++) {
    
    for (int iaz = 0; iaz < _searchNAz - 1; iaz++) {
      
      SearchPoint &sp = _searchLeft[iel][iaz];
      if (sp.ray == NULL) {
        continue;
      }
      
      // propagate to the right
      
      SearchPoint &right = _searchLeft[iel][iaz+1];
      if (right.ray == NULL) {
        right.ray = sp.ray;
        right.rayEl = sp.rayEl;
        right.rayAz = sp.rayAz;
      }

    } // iaz
    
  } // iel

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray to the right of the search point
// We do this by propagating the ray information to the left.

void PolarInterp::_fillRight()

{
 
  for (int iel = 0; iel < _nEl; iel++) {

    for (int iaz = _searchNAz - 1; iaz > 0; iaz--) {
      
      SearchPoint &sp = _searchRight[iel][iaz];
      if (sp.ray == NULL) {
        continue;
      }

      // propagate to the left

      SearchPoint &left = _searchRight[iel][iaz-1];
      if (left.ray == NULL) {
        left.ray = sp.ray;
        left.rayEl = sp.rayEl;
        left.rayAz = sp.rayAz;
      }

    } // iaz

  } // iel

}

/////////////////////////////////////////////////////////
// get the azimuth index for the search matrix, given
// the azimuth angle
//
// Returns -1 if out of bounds

int PolarInterp::_getSearchAzIndex(double az) 
{
  int iaz = (int) ((az - _searchMinAz) / _searchResAz + 0.5);
  if (iaz < 0) {
    iaz = -1;
  } else if (iaz > _searchNAz - 1) {
    iaz = -1;
  }
  return iaz;
}
  
/////////////////////////////////////////////////////////
// get the azimuth given the search index

double PolarInterp::_getSearchAz(int index) 
{
  return _searchMinAz + index * _searchResAz;
}
  
//////////////////////////////////////////////////
// interpolate onto the grid

void PolarInterp::_doInterp()
{

  // initialize the output field arrays

  _initOutputArrays();

  // perform the interpolation

  if (_params.use_multiple_threads) {
    _interpMultiThreaded();
  } else {
    _interpSingleThreaded();
  }

}

//////////////////////////////////////////////////
// interpolate entire volume in single thread

void PolarInterp::_interpSingleThreaded()
{
  
  // interpolate one column at a time

  for (int iz = 0; iz < _nEl; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      _interpAz(iz, iy);
    } // iy
  } // iz

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void PolarInterp::_interpMultiThreaded()
{

  _threadPoolInterp.initForRun();

  // loop through the elevation layers
  for (int iz = 0; iz < _nEl; iz++) {
    // loop through the azimuths
    for (int iy = 0; iy < _gridNy; iy++) {
      // get a thread from the pool
      bool isDone = true;
      PerformInterp *thread = 
        (PerformInterp *) _threadPoolInterp.getNextThread(true, isDone);
      if (thread == NULL) {
        break;
      }
      if (isDone) {
        // if it is a done thread, return thread to the available pool
        _threadPoolInterp.addThreadToAvail(thread);
        // reduce iy by 1 since we did not actually get a compute
        // thread yet for this row
        iy--;
      } else {
        // available thread, set it running
        thread->setElIndex(iz);
        thread->setAzIndex(iy);
        thread->signalRunToStart();
      }
    } // iy
  } // iz
    
  // collect remaining done threads

  _threadPoolInterp.setReadyForDoneCheck();
  while (!_threadPoolInterp.checkAllDone()) {
    PerformInterp *thread = 
      (PerformInterp *) _threadPoolInterp.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      _threadPoolInterp.addThreadToAvail(thread);
    }
  } // while

}

////////////////////////////////////////////////////////////
// Interpolate an azimuth at a time

void PolarInterp::_interpAz(int elIndex, int azIndex)

{
  
  // find starting location in search vector
    
  double az = _minAz + azIndex * _deltaAz;
  double condAz = _conditionAz(az);
  int iaz = _getSearchAzIndex(condAz);
  if (iaz < 0) {
    return;
  }
    
  // get rays on either side of grid point in azimuth
  
  SearchPoint left = _searchLeft[elIndex][iaz];
  SearchPoint right = _searchRight[elIndex][iaz];
  
  // we need to check that these search rays in fact do bound
  // the search point. Because our search cells have finite dimensions -
  // i.e. _searchResAz, it can happen that the ray
  // position may be on the incorrect side of the grid position.
  // If that is the case, we need to shift the search box by up
  // to one index in each dimension.
  
  // check to make sure left ray is actually
  // to the left of the grid point
  
  if (left.ray) {
    if ((left.rayAz > az) && (iaz > 0)) {
      left = _searchLeft[elIndex][iaz-1];
    }
  }
  
  // check to make sure right ray is actually
  // to the right of the grid point
  
  if (right.ray) {
    if ((right.rayAz < az) && (iaz < _searchNAz - 1)) {
      right = _searchRight[elIndex][iaz + 1];
    }
  }
  
  // count number of available rays around this point
  // initialize az and el to be used for interp
  
  int nAvail = 0;
  
  if (left.ray) {
    nAvail++;
    left.interpAz = left.rayAz;
  } else {
    left.interpAz = az;
  }
  
  if (right.ray) {
    nAvail++;
    right.interpAz = right.rayAz;
  } else {
    right.interpAz = az;
  }
  
  // make sure we have at least 1 ray from which to interpolate
  if (nAvail < 1) {
    // no rays, cannot compute for this point
    return;
  }
  
  // if we have only 1 available ray, make sure we are within the
  // beam width of the edge of the measured data
  
  if (nAvail == 1) {
    // compute angle difference
    double angleError = 0;
    if (left.ray) {
      // data is to the left
      angleError = fabs(az - _conditionAz(left.ray->azForLimits));
    } else if (right.ray) {
      // data is to the right
      angleError = fabs(az - _conditionAz(right.ray->azForLimits));
    }
    if (angleError >= 360.0) {
      angleError -= 360.0;
    }
    // if angle error exceeds the beam width, cannot process
    // this point
    double angleExtension =
      _beamWidthDegH * _params.beam_width_fraction_for_data_limit_extension;
    if (angleError > angleExtension) {
      return;
    }
  } // if (nAvail == 1)
  
  // load up wts

  Neighbors wts;
  
  if (nAvail == 1) {
    _loadWtsFor1ValidRay(left, right, wts);
  } else {
    _loadWtsFor2ValidRays(condAz, left, right, wts);
  }
  
  // normalize weights
  
  double sumWt = 0.0;
  sumWt += wts.left;
  sumWt += wts.right;
  if (sumWt == 0) {
    sumWt = 1.0;
  }
  
  wts.left /= sumWt;
  wts.right /= sumWt;
  
  // loop through gates
  
  int ptIndex = elIndex * _nPointsPlane + azIndex * _gridNx;
  for (int igate = 0; igate < _gridNx; igate++, ptIndex++) {
    
    // interpolate fields

    for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
      
      if (_interpFields[ifield].isDiscrete || _params.use_nearest_neighbor) {
        _loadNearestGridPt(ifield, ptIndex, igate,
                           left, right, wts);
      } else if (_interpFields[ifield].fieldFolds) {
        _loadFoldedGridPt(ifield, ptIndex, igate,
                          left, right, wts);
      } else {
        _loadInterpGridPt(ifield, ptIndex, igate,
                          left, right, wts);
      }

    } // ifield

  } // ix

}

////////////////////////////////////////////
// load up weights for case where we only
// have 1 valid ray

void PolarInterp::_loadWtsFor1ValidRay(const SearchPoint &left,
                                       const SearchPoint &right,
                                       Neighbors &wts)
  
{
  
  // use weights for valid ray
  
  if (left.ray) {
    wts.left = 1.0;
    wts.right = 0.0;
  } else {
    wts.left = 0.0;
    wts.right = 1.0;
  }
  
}

////////////////////////////////////////////
// load up weights for case where we
// have 2 valid rays
  
void PolarInterp::_loadWtsFor2ValidRays(double condAz,
                                        const SearchPoint &left,
                                        const SearchPoint &right,
                                        Neighbors &wts)

{
  
  // compute 'distance' in el/az space from ray to grid location
  // compute weights based on inverse of
  // distances from grid pt to surrounding rays multiplied
  // by weight for range
  
  double dist_left = _angDist(condAz - left.rayAz);
  wts.left = 1.0 / dist_left;
  
  double dist_right = _angDist(condAz - right.rayAz);
  wts.right = 1.0 / dist_right;

}

////////////////////////////////////////////
// load up grid point using nearest neighbor

void PolarInterp::_loadNearestGridPt(int ifield,
                                     int ptIndex,
                                     int igate,
                                     const SearchPoint &left,
                                     const SearchPoint &right,
                                     const Neighbors &wts)
  
{
  
  // find value with highest weight - that will be closest
  
  double maxWt = 0.0;
  double closestVal = 0.0;
  int nContrib = 0;
  
  if (left.ray) {
    _accumNearest(left.ray, ifield, igate,
                  wts.left, closestVal, maxWt, nContrib);
  }
  
  if (right.ray) {
    _accumNearest(right.ray, ifield, igate,
                  wts.right, closestVal, maxWt, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= 1) {
    _outputFields[ifield][ptIndex] = closestVal;
  } else {
    _outputFields[ifield][ptIndex] = missingFl32;
  }
  
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
  
void PolarInterp::_loadInterpGridPt(int ifield,
                                    int ptIndex,
                                    int igate,
                                    const SearchPoint &left,
                                    const SearchPoint &right,
                                    const Neighbors &wts)
  
{
  
  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWts = 0.0;
  int nContrib = 0;
  
  if (left.ray) {
    _accumInterp(left.ray, ifield, igate,
                 wts.left, sumVals, sumWts, nContrib);
  }

  if (right.ray) {
    _accumInterp(right.ray, ifield, igate,
                 wts.right, sumVals, sumWts, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= 1) {
    double interpVal = missingDouble;
    if (sumWts > 0) {
      interpVal = sumVals / sumWts;
    }
    _outputFields[ifield][ptIndex] = interpVal;
  } else {
    _outputFields[ifield][ptIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field
  
void PolarInterp::_loadFoldedGridPt(int ifield,
                                    int ptIndex,
                                    int igate,
                                    const SearchPoint &left,
                                    const SearchPoint &right,
                                    const Neighbors &wts)

{

  // sum up weighted vals
  
  double sumX = 0.0;
  double sumY = 0.0;
  double sumWts = 0.0;
  int nContrib = 0;
  
  if (left.ray) {
    _accumFolded(left.ray, ifield, igate,
                 wts.left, sumX, sumY, sumWts, nContrib);
  }
  
  
  if (right.ray) {
    _accumFolded(right.ray, ifield, igate,
                 wts.right, sumX, sumY, sumWts, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= 1) {
    const Field &intFld = _interpFields[ifield];
    double angleInterp = atan2(sumY, sumX);
    double valInterp = _getFoldValue(angleInterp,
                                     intFld.foldLimitLower, intFld.foldRange);
    _outputFields[ifield][ptIndex] = valInterp;
  } else {
    _outputFields[ifield][ptIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// condition the azimuth, if we are in sector that
// spans the north line

double PolarInterp::_conditionAz(double az)
{
  if (_isSector) {
    // sector mode
    if (_spansNorth && az < _dataSectorStartAzDeg) {
      az += 360.0;
    }
  } else {
    // 360 mode
    if (az < _searchAzOverlapHalf) {
      az += 360.0;
    }
  }
  return az;
}

/////////////////////////////////////////////////////
// write out data

int PolarInterp::_writeOutputFile()
{

  if (_params.debug) {
    cerr << "  Writing output file ... " << endl;
  }

  // all other formats go via the MDV class

  OutputMdv out(_progName, _params);
  out.setMasterHeader(_readVol);
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    const Field &ifld = _interpFields[ifield];
    out.addField(_readVol, _proj, _gridZLevels,
                 ifld.outputName, ifld.longName, ifld.units,
                 ifld.inputDataType,
                 ifld.inputScale,
                 ifld.inputOffset,
                 missingFl32,
                 _outputFields[ifield]);
  } // ifield
  
  out.addChunks(_readVol, _interpFields.size());
  
  // write out file
  
  if (out.writeVol()) {
    cerr << "ERROR - Interp::processFile" << endl;
    cerr << "  Cannot write file to output_dir: "
         << _params.output_dir << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////////////////////////////////////
// PerformInterp thread
///////////////////////////////////////////////////////////////
// Constructor
PolarInterp::PerformInterp::PerformInterp(PolarInterp *obj) :
        _this(obj)
{
}  
// run method
void PolarInterp::PerformInterp::run()
{
  _this->_interpAz(_elIndex, _azIndex);
}


