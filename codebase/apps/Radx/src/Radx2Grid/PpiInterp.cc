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
// PpiInterp class - derived from Interp.
// Used for 2-D Cartesian interpolation on ppi cones.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#include "PpiInterp.hh"
#include "OutputMdv.hh"
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/toolsa_macros.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxSweep.hh>
using namespace std;

const double PpiInterp::_searchResAz = 0.1;
const double PpiInterp::_searchAzOverlapDeg = 20.0;
const double PpiInterp::_searchAzOverlapHalf =
  PpiInterp::_searchAzOverlapDeg / 2.0;

// Constructor

PpiInterp::PpiInterp(const string &progName,
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
  _gridLoc = NULL;
  _nzAlloc = _nyAlloc = _nxAlloc = 0;
  _outputFields = NULL;

  // set up thread objects

  _createThreads();
  
}

//////////////////////////////////////
// destructor

PpiInterp::~PpiInterp()

{

  _freeThreads();
  _freeZLevels();
  _freeSearchMatrix();
  _freeGridLoc();
  _freeOutputArrays();

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int PpiInterp::interpVol()

{

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - PpiInterp::interpVol()" << endl;
    return -1;
  }

  // set the Z levels

  _initZLevels();

  // compute the scan azimuth delta angle
  // this is used in the search process

  _computeAzimuthDelta();

  if (_params.debug) {
    cerr << "  _scanDeltaAz: " << _scanDeltaAz << endl;
  }

  // initialize the output grid dimensions if things have changed
  
  if (_geomHasChanged() || _gridLoc == NULL) {

    // initialize grid

    _initGrid();

    // compute grid locations relative to radar
    
    if (_params.debug) {
      cerr << "  Computing grid relative to radar ... " << endl;
    }
    _computeGridRelative();
    
  }

  // compute search matrix angle limits - keep the matrix
  // as small as possible for efficiency
  
  _computeSearchLimits();
  
  // fill the search matrix

  if (_params.debug) {
    cerr << "  Filling search matrix ... " << endl;
  }
  _allocSearchMatrix();
  _initSearchMatrix();
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

  // free up

  _freeSearchMatrix();
  _freeOutputArrays();

  if (_params.free_memory_between_files) {
    _freeGridLoc();
  }

  return 0;

}

//////////////////////////////////////////////////
// create the threading objects

void PpiInterp::_createThreads()
{

  // initialize thread pool for grid relative to radar

  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    ComputeGridRelative *thread = new ComputeGridRelative(this);
    _threadPoolGridRel.addThreadToMain(thread);
  }

  // initialize thread pool for interpolation

  for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    PerformInterp *thread = new PerformInterp(this);
    _threadPoolInterp.addThreadToMain(thread);
  }

}

//////////////////////////////////////////////////
// free the threads

void PpiInterp::_freeThreads()
{
  // thread pools free up threads in destructor
}

////////////////////////////////////////////////////////////
// Check if radar or scan geometry has changed

bool PpiInterp::_geomHasChanged()

{

  bool hasChanged = false;
  
  // check if radar has moved
  
  if (fabs(_prevRadarLat - _radarLat) < 0.00001 &&
      fabs(_prevRadarLon - _radarLon) < 0.00001 &&
      fabs(_prevRadarAltKm - _radarAltKm) < 0.00001) {
    _prevRadarLat = _radarLat;
    _prevRadarLon = _radarLon;
    _prevRadarAltKm = _radarAltKm;
    hasChanged = true;
  }

  // check elevation angles
  
  bool elevChanged = false;
  if (_gridZLevels.size() == 0 || _gridZLevels.size() == 0) {
    elevChanged = true;
  } else if (_prevZLevels.size() != _gridZLevels.size()) {
    elevChanged = true;
  } else {
    for (size_t ii = 0; ii < _gridZLevels.size(); ii++) {
      if (fabs(_prevZLevels[ii] - _gridZLevels[ii]) > 0.001) {
        elevChanged = true;
        break;
      }
    }
  }

  if (elevChanged) {
    _prevZLevels = _gridZLevels;
    hasChanged = true;
    if (_params.debug) {
      cerr << "==>> Elevs have changed" << endl;
    }
  } else {
    if (_params.debug) {
      cerr << "==>> Elevs have NOT changed" << endl;
    }
  }

  return hasChanged;
  
}

  
//////////////////////////////////////////////////
// Compute the search matrix limits
// keeping it as small as possible for efficiency

void PpiInterp::_computeSearchLimits()
{

  // set azimuth resolution and search radius
  
  double minAzDeg = 0;
  double maxAzDeg = 360.0 + _searchAzOverlapDeg;
  
  double searchAzRangeDeg = maxAzDeg - minAzDeg;
  _searchMinAz = minAzDeg;
  _searchNAz = (int) (searchAzRangeDeg / _searchResAz + 1);
  
  // set search radius
  
  _searchRadiusAz = _scanDeltaAz + _beamWidthDegH + 1.0;
  _searchMaxDistAz = (int) (_searchRadiusAz / _searchResAz + 0.5);

  if (_params.debug) {
    cerr << "  _searchMinAz: " << _searchMinAz << endl;
    cerr << "  _searchNAz: " << _searchNAz << endl;
    cerr << "  _searchRadiusAz: " << _searchRadiusAz << endl;
    cerr << "  _searchMaxDistAz: " << _searchMaxDistAz << endl;
  }

}

////////////////////////////////////////////////////////////
// Initialize Z levels

void PpiInterp::_initZLevels()

{
  _freeZLevels();
  _nEl = _readVol.getNSweeps();
  _gridNz = _nEl;
  for (int ii = 0; ii < _gridNz; ii++) {
    _gridZLevels.push_back
      (_readVol.getSweeps()[ii]->getFixedAngleDeg());
  }
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "----------------------------------" << endl;
    cerr << "PpiInterp::_initZLevels" << endl;
    cerr << "  nEl: " << _nEl << endl;
    for (size_t ii = 0; ii < _gridZLevels.size(); ii++) {
      cerr << "  ielev, elev: " << ii << ", " << _gridZLevels[ii] << endl;
    }
    cerr << "----------------------------------" << endl;
  }
}

////////////////////////////////////////////////////////////
// Free Z levels

void PpiInterp::_freeZLevels()

{
  _gridZLevels.clear();
}

////////////////////////////////////////////////////////////
// Initialize output grid

void PpiInterp::_initGrid()

{

  _freeGridLoc();

  _gridNx = _params.grid_xy_geom.nx;
  _gridMinx = _params.grid_xy_geom.minx;
  _gridDx = _params.grid_xy_geom.dx;
  
  _gridNy = _params.grid_xy_geom.ny;
  _gridMiny = _params.grid_xy_geom.miny;
  _gridDy = _params.grid_xy_geom.dy;

  _gridLoc = (GridLoc ****)
    umalloc3(_nEl, _gridNy, _gridNx, sizeof(GridLoc *));
  
  for (int iz = 0; iz < _nEl; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      for (int ix = 0; ix < _gridNx; ix++) {
        _gridLoc[iz][iy][ix] = new GridLoc;
      }
    }
  }

  _nzAlloc = _nEl;
  _nyAlloc = _gridNy;
  _nxAlloc = _gridNx;

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "PpiInterp::_initGrid - grid initialized" << endl;
    cerr << "  nz: " << _nzAlloc << endl;
    cerr << "  ny: " << _nyAlloc << endl;
    cerr << "  nx: " << _nxAlloc << endl;
  }

}
  
////////////////////////////////////////////////////////////
// Free up the grid locations

void PpiInterp::_freeGridLoc()
  
{
  
  if (_gridLoc) {
    for (int iz = 0; iz < _nzAlloc; iz++) {
      for (int iy = 0; iy < _nyAlloc; iy++) {
        for (int ix = 0; ix < _nxAlloc; ix++) {
          delete _gridLoc[iz][iy][ix];
        }
      }
    }
    ufree3((void ***) _gridLoc);
    _gridLoc = NULL;
    _nzAlloc = _nyAlloc = _nxAlloc = 0;
  }

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void PpiInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _nEl;
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void PpiInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void PpiInterp::_initOutputArrays()
  
{

  _allocOutputArrays();

  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    for (int jj = 0; jj < _nPointsVol; jj++) {
      _outputFields[ii][jj] = missingFl32;
    }
  }

}

////////////////////////////////////////////////////////////
// Compute grid locations relative to radar

void PpiInterp::_computeGridRelative()

{

  if (_params.center_grid_on_radar) {
    _gridOriginLat = _radarLat;
    _gridOriginLon = _radarLon;
  } else {
    _gridOriginLat = _params.grid_origin_lat;
    _gridOriginLon = _params.grid_origin_lon;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  _radarLat: " << _radarLat << endl;
    cerr << "  _radarLon: " << _radarLon << endl;
    cerr << "  _radarAltKm: " << _radarAltKm << endl;
    cerr << "  _gridOriginLat: " << _gridOriginLat << endl;
    cerr << "  _gridOriginLon: " << _gridOriginLon << endl;
  }

  // initialize the projection

  _initProjection();

  if (_params.use_multiple_threads) {

    _computeGridRelMultiThreaded();

  } else {

    // loop through the grid
    
    for (int iz = 0; iz < _nEl; iz++) {
      for (int iy = 0; iy < _gridNy; iy++) {
        _computeGridRow(iz, iy);
      } // iy
    } // iz

  }

}

//////////////////////////////////////////////////////
// compute grid rows in multi-threaded mode

void PpiInterp::_computeGridRelMultiThreaded()
{

  _threadPoolGridRel.initForRun();

  // loop through the Z layers
  for (int iz = 0; iz < _gridNz; iz++) {
    // loop through the Y columns
    for (int iy = 0; iy < _gridNy; iy++) {
      // get a thread from the pool
      bool isDone = true;
      ComputeGridRelative *thread = 
        (ComputeGridRelative *) _threadPoolGridRel.getNextThread(true, isDone);
      if (thread == NULL) {
        break;
      }
      if (isDone) {
        // if it is a done thread, return thread to the available pool
        _threadPoolGridRel.addThreadToAvail(thread);
        // reduce iy by 1 since we did not actually get a compute
        // thread yet for this row
        iy--;
      } else {
        // available thread, set it running
        thread->setZIndex(iz);
        thread->setYIndex(iy);
        thread->signalRunToStart();
      }
    } // iy
  } // iz
  
  // collect remaining done threads

  _threadPoolGridRel.setReadyForDoneCheck();
  while (!_threadPoolGridRel.checkAllDone()) {
    ComputeGridRelative *thread = 
      (ComputeGridRelative *) _threadPoolGridRel.getNextDoneThread();
    if (thread == NULL) {
      break;
    } else {
      _threadPoolGridRel.addThreadToAvail(thread);
    }
  } // while

}

////////////////////////////////////////////////////////////
// Compute grid locations for one row

void PpiInterp::_computeGridRow(int iz, int iy)

{

  // loop through the row

  double elev = _gridZLevels[iz];
  double cosElev = cos(elev * DEG_TO_RAD);
  double yy = _gridMiny + iy * _gridDy;
  double xx = _gridMinx;

  for (int ix = 0; ix < _gridNx; ix++, xx += _gridDx) {
    
    // get the latlon of the (x,y) point in the output grid
    
    double gridLat, gridLon;
    _proj.xy2latlon(xx, yy, gridLat, gridLon);
    
    // get the azimuth and distance from the radar
    
    double gndRange, azimuth;
    PJGLatLon2RTheta(_radarLat, _radarLon,
                     gridLat, gridLon,
                     &gndRange, &azimuth);
    if (azimuth < 0) {
      azimuth += 360.0;
    }
    
    // compute elevation
    
    GridLoc *loc = _gridLoc[iz][iy][ix];
    loc->az = azimuth;
    loc->slantRange = gndRange / cosElev;
    loc->gndRange = gndRange;
    
  } // ix

}

////////////////////////////////////////////////////////////
// Allocate the search matrix

void PpiInterp::_allocSearchMatrix()

{

  _freeSearchMatrix();

  _searchLeft = 
    (SearchPoint **) umalloc2(_nEl, _searchNAz, sizeof(SearchPoint));

  _searchRight = 
    (SearchPoint **) umalloc2(_nEl, _searchNAz, sizeof(SearchPoint));


}

////////////////////////////////////////////////////////////
// Free up the search matrix

void PpiInterp::_freeSearchMatrix()

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

void PpiInterp::_initSearchMatrix()

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

void PpiInterp::_fillSearchMatrix()

{

  _fillLeft();
  _fillRight();

  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printSearchMatrix(stderr, 1);
  }

}

///////////////////////////////////////////////////////////
// print search matrix

void PpiInterp::_printSearchMatrix(FILE *out, int res)
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

void PpiInterp::_printSearchMatrixPoint(FILE *out, int iel, int iaz)
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

void PpiInterp::_fillLeft()

{

  for (int iel = 0; iel < _nEl; iel++) {
    
    for (int iaz = 0; iaz < _searchNAz - 1; iaz++) {
      
      SearchPoint &sp = _searchLeft[iel][iaz];
      if (sp.ray == NULL) {
        continue;
      }
      if (sp.azDist >= _searchMaxDistAz) {
        continue;
      }

      // propagate to the right
      
      SearchPoint &right = _searchLeft[iel][iaz+1];
      if (right.ray == NULL) {
        right.azDist = sp.azDist + 1;
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

void PpiInterp::_fillRight()

{
 
  for (int iel = 0; iel < _nEl; iel++) {

    for (int iaz = _searchNAz - 1; iaz > 0; iaz--) {
      
      SearchPoint &sp = _searchRight[iel][iaz];
      if (sp.ray == NULL) {
        continue;
      }
      if (sp.azDist >= _searchMaxDistAz) {
        continue;
      }

      // propagate to the left

      SearchPoint &left = _searchRight[iel][iaz-1];
      if (left.ray == NULL) {
        left.azDist = sp.azDist + 1;
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

int PpiInterp::_getSearchAzIndex(double az) 
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

double PpiInterp::_getSearchAz(int index) 
{
  return _searchMinAz + index * _searchResAz;
}
  
//////////////////////////////////////////////////
// interpolate onto the grid

void PpiInterp::_doInterp()
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

void PpiInterp::_interpSingleThreaded()
{
  
  // interpolate one column at a time

  for (int iz = 0; iz < _nEl; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      _interpRow(iz, iy);
    } // iy
  } // iz

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void PpiInterp::_interpMultiThreaded()
{

  _threadPoolInterp.initForRun();

  // loop through the Z layers
  for (int iz = 0; iz < _gridNz; iz++) {
    // loop through the Y columns
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
        thread->setZIndex(iz);
        thread->setYIndex(iy);
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
// Interpolate a row at a time

void PpiInterp::_interpRow(int iel, int iy)

{

  int ptIndex = iel * _nPointsPlane + iy * _gridNx;

  for (int ix = 0; ix < _gridNx; ix++, ptIndex++) {

    // get the grid location
    
    const GridLoc *loc = _gridLoc[iel][iy][ix];
    
    // find starting location in search vector

    double az = _conditionAz(loc->az);
    int iaz = _getSearchAzIndex(az);
    
    if (iaz < 0) {
      continue;
    }

    // get rays on either side of grid point in azimuth
    
    SearchPoint left = _searchLeft[iel][iaz];
    SearchPoint right = _searchRight[iel][iaz];

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
        left = _searchLeft[iel][iaz-1];
      }
    }
    
    // check to make sure right ray is actually
    // to the right of the grid point
    
    if (right.ray) {
      if ((right.rayAz < az) && (iaz < _searchNAz - 1)) {
        right = _searchRight[iel][iaz + 1];
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
      continue;
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
      // if angle error exceeds the beam width, cannot process
      // this point
      double angleExtension =
        _beamWidthDegH * _params.beam_width_fraction_for_data_limit_extension;
      if (angleError > angleExtension) {
        continue;
      }
    } // if (nAvail == 1)

    // get gate indices, compute weights based on range

    double rangeKm = loc->slantRange;
    double dgate = (rangeKm - _startRangeKm) / _gateSpacingKm;
    int igateInner = (int) floor(dgate);
    int igateOuter = igateInner + 1;
    double wtOuter = dgate - igateInner;
    double wtInner = 1.0 - wtOuter;
    Neighbors wts;

    if (nAvail == 1) {
      
      // if we only have 1 valid ray, use an inverse distance interp
      
      _loadWtsFor1ValidRay(left, right, wtInner, wtOuter, wts);
      
    } else {
      
      // we have 2 valid rays, use an inverse distance interp
      
      _loadWtsFor2ValidRays(loc, left, right, wtInner, wtOuter, wts);
      
    } // if (nAvail == 1) 

    // normalize weights

    double sumWt = 0.0;
    sumWt += wts.left_inner;
    sumWt += wts.left_outer;
    sumWt += wts.right_inner;
    sumWt += wts.right_outer;
    if (sumWt == 0) {
      sumWt = 1.0;
    }

    wts.left_inner /= sumWt;
    wts.left_outer /= sumWt;
    wts.right_inner /= sumWt;
    wts.right_outer /= sumWt;
    
    // interpolate fields

    for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {

      if (_interpFields[ifield].isDiscrete || _params.use_nearest_neighbor) {
        _loadNearestGridPt(ifield, ptIndex, igateInner, igateOuter,
                           left, right, wts);
      } else if (_interpFields[ifield].fieldFolds) {
        _loadFoldedGridPt(ifield, ptIndex, igateInner, igateOuter,
                          left, right, wts);
      } else {
        _loadInterpGridPt(ifield, ptIndex, igateInner, igateOuter,
                          left, right, wts);
      }

    } // ifield

  } // ix

}

////////////////////////////////////////////
// load up weights for case where we only
// have 1 valid ray

void PpiInterp::_loadWtsFor1ValidRay(const SearchPoint &left,
                                     const SearchPoint &right,
                                     double wtInner,
                                     double wtOuter, 
                                     Neighbors &wts)

{
  
  // use weights for valid ray
  
  if (left.ray) {
    wts.left_inner = wtInner;
    wts.left_outer = wtOuter;
  } else {
    wts.left_inner = 0.0;
    wts.left_outer = 0.0;
  }
  
  if (right.ray) {
    wts.right_inner = wtInner;
    wts.right_outer = wtOuter;
  } else {
    wts.right_inner = 0.0;
    wts.right_outer = 0.0;
  }
  
}

////////////////////////////////////////////
// load up weights for case where we
// have 2 valid rays
  
void PpiInterp::_loadWtsFor2ValidRays(const GridLoc *loc,
                                      const SearchPoint &left,
                                      const SearchPoint &right,
                                      double wtInner,
                                      double wtOuter, 
                                      Neighbors &wts)

{
  
  double az = _conditionAz(loc->az);

  // compute 'distance' in el/az space from ray to grid location
  // compute weights based on inverse of
  // distances from grid pt to surrounding rays multiplied
  // by weight for range
  
  if (left.ray) {
    double dist_left = _angDist(az - left.rayAz);
    double wtDist = 1.0 / dist_left;
    wts.left_inner = wtDist * wtInner;
    wts.left_outer = wtDist * wtOuter;
  } else {
    wts.left_inner = 0.0;
    wts.left_outer = 0.0;
  }
  
  if (right.ray) {
    double dist_right = _angDist(az - right.rayAz);
    double wtDist = 1.0 / dist_right;
    wts.right_inner = wtDist * wtInner;
    wts.right_outer = wtDist * wtOuter;
  } else {
    wts.right_inner = 0.0;
    wts.right_outer = 0.0;
  }
  
}

////////////////////////////////////////////
// load up grid point using nearest neighbor
  
void PpiInterp::_loadNearestGridPt(int ifield,
                                   int ptIndex,
                                   int igateInner,
                                   int igateOuter,
                                   const SearchPoint &left,
                                   const SearchPoint &right,
                                   const Neighbors &wts)
  
{

  // find value with highest weight - that will be closest
        
  double maxWt = 0.0;
  double closestVal = 0.0;
  int nContrib = 0;
  
  if (left.ray) {
    _accumNearest(left.ray, ifield, igateInner, igateOuter,
                  wts.left_inner, wts.left_outer, closestVal, maxWt, nContrib);
  }
  
  if (right.ray) {
    _accumNearest(right.ray, ifield, igateInner, igateOuter,
                  wts.right_inner, wts.right_outer, closestVal, maxWt, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= _params.min_nvalid_for_interp) {
    _outputFields[ifield][ptIndex] = closestVal;
  } else {
    _outputFields[ifield][ptIndex] = missingFl32;
  }
  
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
  
void PpiInterp::_loadInterpGridPt(int ifield,
                                  int ptIndex,
                                  int igateInner,
                                  int igateOuter,
                                  const SearchPoint &left,
                                  const SearchPoint &right,
                                  const Neighbors &wts)

{

  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWts = 0.0;
  int nContrib = 0;
  
  if (left.ray) {
    _accumInterp(left.ray, ifield, igateInner, igateOuter,
                 wts.left_inner, wts.left_outer, sumVals, sumWts, nContrib);
  }

  if (right.ray) {
    _accumInterp(right.ray, ifield, igateInner, igateOuter,
                 wts.right_inner, wts.right_outer, sumVals, sumWts, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= _params.min_nvalid_for_interp) {
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
  
void PpiInterp::_loadFoldedGridPt(int ifield,
                                  int ptIndex,
                                  int igateInner,
                                  int igateOuter,
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
    _accumFolded(left.ray, ifield, igateInner, igateOuter,
                 wts.left_inner, wts.left_outer, sumX, sumY, sumWts, nContrib);
  }
  
  
  if (right.ray) {
    _accumFolded(right.ray, ifield, igateInner, igateOuter,
                 wts.right_inner, wts.right_outer, sumX, sumY, sumWts, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= _params.min_nvalid_for_interp) {
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
// condition the azimuth so we are in the 
// correct part of the array

double PpiInterp::_conditionAz(double az)
{
  if (az < _searchAzOverlapHalf) {
    az += 360.0;
  }
  return az;
}

/////////////////////////////////////////////////////
// write out data

int PpiInterp::_writeOutputFile()
{

  if (_params.debug) {
    cerr << "  Writing output file ... " << endl;
  }

  // cedric is a special case
  
  if (_params.output_format == Params::CEDRIC) {
    return _writeCedricFile(true);
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
// ComputeGridRelative thread
///////////////////////////////////////////////////////////////
// Constructor
PpiInterp::ComputeGridRelative::ComputeGridRelative(PpiInterp *obj) :
        _this(obj)
{
}  
// run method
void PpiInterp::ComputeGridRelative::run()
{
  _this->_computeGridRow(_zIndex, _yIndex);
}

///////////////////////////////////////////////////////////////
// PerformInterp thread
///////////////////////////////////////////////////////////////
// Constructor
PpiInterp::PerformInterp::PerformInterp(PpiInterp *obj) :
        _this(obj)
{
}  
// run method
void PpiInterp::PerformInterp::run()
{
  _this->_interpRow(_zIndex, _yIndex);
}
