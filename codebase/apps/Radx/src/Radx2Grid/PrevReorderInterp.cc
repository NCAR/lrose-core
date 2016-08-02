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
// PrevReorderInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation, following the 
// REORDER strategy of interpolation using the closest N points.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#include "PrevReorderInterp.hh"
#include "OutputMdv.hh"
#include <algorithm>
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/sincos.h>
#include <toolsa/toolsa_macros.h>
#include <toolsa/TaArray.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxSweep.hh>
using namespace std;

// Constructor

PrevReorderInterp::PrevReorderInterp(const string &progName,
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

  _prevRadarLat = _prevRadarLon = _prevRadarAltKm = -9999.0;
  _gridLoc = NULL;
  _outputFields = NULL;
  _ncols = _params.reorder_blocks_ncols;
  _nrows = _params.reorder_blocks_nrows;
  _zSearchRatio = _params.reorder_z_search_ratio;

  // initialize the output grid dimensions

  _initZLevels();
  _initGrid();

  // set up thread objects

  _initThreads();
  
}

//////////////////////////////////////
// destructor

PrevReorderInterp::~PrevReorderInterp()

{

  // wait for active thread pool to complete

  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

  // signal all threads to exit

  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->setExitFlag(true);
    _availThreads[ii]->signalWorkToStart();
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->setExitFlag(true);
    _activeThreads[ii]->signalWorkToStart();
  }

  // wait for all threads to exit
  
  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    _availThreads[ii]->waitForWorkToComplete();
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    _activeThreads[ii]->waitForWorkToComplete();
  }

  // delete all threads
  
  for (size_t ii = 0; ii < _availThreads.size(); ii++) {
    delete _availThreads[ii];
  }
  for (size_t ii = 0; ii < _activeThreads.size(); ii++) {
    delete _activeThreads[ii];
  }

  pthread_mutex_destroy(&_debugPrintMutex);

  // free up grid

  if (_gridLoc) {
    for (int iz = 0; iz < _gridNz; iz++) {
      for (int iy = 0; iy < _gridNy; iy++) {
        for (int ix = 0; ix < _gridNx; ix++) {
          delete _gridLoc[iz][iy][ix];
        }
      }
    }
    ufree3((void ***) _gridLoc);
  }

  _freeOutputArrays();
  _radarPoints.clear();

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int PrevReorderInterp::interpVol()

{

  _printRunTime("initialization");

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - PrevReorderInterp::interpVol()" << endl;
    return -1;
  }

  // compute search radius at max range

  _maxSearchRadius = _params.reorder_search_radius_km;
  if (_params.reorder_scale_search_radius_with_range) {
    _maxSearchRadius *=
      (_maxSearchRadius / _params.reorder_nominal_range_for_search_radius_km);
  }

  // compute the scan azimuth and elevation delta angle
  // these are used in the search process
  
  _computeAzimuthDelta();
  _computeElevationDelta();
  if (_params.debug) {
    cerr << "  _scanDeltaAz: " << _scanDeltaAz << endl;
    cerr << "  _scanDeltaEl: " << _scanDeltaEl << endl;
  }
  _printRunTime("computing scan props");

  // compute grid locations relative to radar
  
  if (_params.debug) {
    cerr << "  Computing grid relative to radar ... " << endl;
  }
  _computeGridRelative();

  // compute the cartesian points for each gate

  _computeRadarPoints();
  
  // interpolate

  if (_params.debug) {
    cerr << "  Interpolating ... " << endl;
  }
  _doInterp();
  _printRunTime("interpolation");

  // write out data

  if (_writeOutputFile()) {
    cerr << "ERROR - Interp::processFile" << endl;
    cerr << "  Cannot write output file" << endl;
    return -1;
  }

  return 0;

}

////////////////////////////////////////////////////////////
// Initialize Z levels

void PrevReorderInterp::_initZLevels()

{

  if (_params.specify_individual_z_levels) {
    _gridNz = _params.z_level_array_n;
  } else {
    _gridNz = _params.grid_z_geom.nz;
  }

  if (_params.specify_individual_z_levels) {
    for (int ii = 0; ii < _gridNz; ii++) {
      _gridZLevels.push_back(_params._z_level_array[ii]);
    }
  } else {
    for (int ii = 0; ii < _gridNz; ii++) {
      _gridZLevels.push_back
        (_params.grid_z_geom.minz + ii * _params.grid_z_geom.dz);
    }
  }

  // make sure the Zlevels are in ascending order

  sort(_gridZLevels.begin(), _gridZLevels.end());

  // set max Z

  _maxZ = _gridZLevels[_gridZLevels.size()-1];

}

////////////////////////////////////////////////////////////
// Initialize output grid

void PrevReorderInterp::_initGrid()

{

  _gridNx = _params.grid_xy_geom.nx;
  _gridMinx = _params.grid_xy_geom.minx;
  _gridDx = _params.grid_xy_geom.dx;
  
  _gridNy = _params.grid_xy_geom.ny;
  _gridMiny = _params.grid_xy_geom.miny;
  _gridDy = _params.grid_xy_geom.dy;
  
  _gridLoc = (GridLoc ****)
    umalloc3(_gridNz, _gridNy, _gridNx, sizeof(GridLoc *));
  
  for (int iz = 0; iz < _gridNz; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      for (int ix = 0; ix < _gridNx; ix++) {
        _gridLoc[iz][iy][ix] = new GridLoc;
      }
    }
  }

}
  
//////////////////////////////////////////////////
// initialize the threading objects

void PrevReorderInterp::_initThreads()
{

  // initialize compute object

  pthread_mutex_init(&_debugPrintMutex, NULL);
  
  if (_params.use_multiple_threads) {
    
    // set up compute thread pool
    
    for (int ii = 0; ii < _params.n_compute_threads; ii++) {
      
      PrevReorderThread *thread = new PrevReorderThread();
      thread->setContext(this);
      
      pthread_t pth = 0;
      pthread_create(&pth, NULL, _computeInThread, thread);
      thread->setThreadId(pth);
      _availThreads.push_back(thread);
      
    }
    
  }

}

///////////////////////////////////////////////////////////
// Thread function to perform computations

void *PrevReorderInterp::_computeInThread(void *thread_data)
  
{
  
  // get thread data from args

  PrevReorderThread *reorderThread = (PrevReorderThread *) thread_data;
  PrevReorderInterp *context = reorderThread->getContext();
  assert(context);
  
  while (true) {

    // wait for main to unlock start mutex on this thread
    
    reorderThread->waitForStartSignal();
    
    // if exit flag is set, context is done, exit now
    
    if (reorderThread->getExitFlag()) {
      if (context->getParams().debug >= Params::DEBUG_VERBOSE) {
        pthread_mutex_t *debugPrintMutex = context->getDebugPrintMutex();
        pthread_mutex_lock(debugPrintMutex);
        cerr << "====>> compute thread exiting" << endl;
        pthread_mutex_unlock(debugPrintMutex);
      }
      reorderThread->signalParentWorkIsComplete();
      return NULL;
    }
    
    // perform computations
    
    if (reorderThread->getTask() == PrevReorderThread::INTERP) {
      context->_interpBlock(reorderThread->getYIndex(), 
                            reorderThread->getXIndex());
    } else if (reorderThread->getTask() == PrevReorderThread::GRID_LOC) {
      context->_computeGridRelRow(reorderThread->getZIndex(), 
                                  reorderThread->getYIndex());
    }
    
    // unlock done mutex
    
    reorderThread->signalParentWorkIsComplete();
    
  } // while

  return NULL;

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void PrevReorderInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _gridNz;
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void PrevReorderInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void PrevReorderInterp::_initOutputArrays()
  
{

  _allocOutputArrays();

  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    for (int jj = 0; jj < _nPointsVol; jj++) {
      _outputFields[ii][jj] = missingFl32;
    }
  }

}

//////////////////////////////////////////////////
// Compute radar points in Cart space

void PrevReorderInterp::_computeRadarPoints()
{

  // set max ht for points in tree
  
  double searchMaxZ = _maxZ + _maxSearchRadius;
  
  // initialize beamHeight computations
  
  _beamHt.setInstrumentHtKm(_radarAltKm);

  // Build the KD tree for 3D radar observations
  
  radar_point_t radarPt;
  _radarPoints.clear();

  // loop through rays
  
  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    
    const Ray *ray = _interpRays[iray];
    double el = ray->el;
    double az = ray->az;
    
    double sinAz, cosAz;
    ta_sincos(az * DEG_TO_RAD, &sinAz, &cosAz);
    
    int nGates = ray->nGates;
    double range = _startRangeKm;
    for (int igate = 0; igate < nGates; igate++, range += _gateSpacingKm) {
      
      double xx = range * sinAz;
      double yy = range * cosAz;
      double zz = beamHt.computeHtKm(el, range);

      if (zz > searchMaxZ) {
        continue;
      }

      radarPt.xx = xx;
      radarPt.yy = yy;
      radarPt.zz = zz;
      radarPt.iray = iray;
      radarPt.igate = igate;
      _radarPoints.push_back(radarPt);

    } // igate
    
  } // iray
  
  _printRunTime("Reorder - computing radar point locations");
  
}

////////////////////////////////////////////////////////////
// Compute grid locations relative to radar

void PrevReorderInterp::_computeGridRelative()

{

  // check if radar has moved

  if (fabs(_prevRadarLat - _radarLat) < 0.00001 &&
      fabs(_prevRadarLon - _radarLon) < 0.00001 &&
      fabs(_prevRadarAltKm - _radarAltKm) < 0.00001) {
    return;
  }

  _prevRadarLat = _radarLat;
  _prevRadarLon = _radarLon;
  _prevRadarAltKm = _radarAltKm;
  
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
    
    for (int iz = 0; iz < _gridNz; iz++) {
      for (int iy = 0; iy < _gridNy; iy++) {
        _computeGridRelRow(iz, iy);
      } // iy
    } // iz

  }

  _printRunTime("computing grid");

}

//////////////////////////////////////////////////////
// compute grid rows in multi-threaded mode

void PrevReorderInterp::_computeGridRelMultiThreaded()
{

  // loop through the Z layers
  
  for (int iz = 0; iz < _gridNz; iz++) {

    // loop through the Y columns
  
    for (int iy = 0; iy < _gridNy; iy++) {
      
      // is a thread available? if not wait for one
    
      PrevReorderThread *thread = NULL;
      if (_availThreads.size() > 0) {
        // get thread from available pool
        // it is doing no work
        thread = _availThreads.front();
        _availThreads.pop_front();
      } else {
        // get thread from active pool
        thread = _activeThreads.front();
        _activeThreads.pop_front();
        // wait for current work to complete
        thread->waitForWorkToComplete();
      }
    
      // set thread going to compute moments
      
      thread->setTask(PrevReorderThread::GRID_LOC);
      thread->setZIndex(iz);
      thread->setYIndex(iy);
      thread->signalWorkToStart();
      
      // push onto active pool
      
      _activeThreads.push_back(thread);

    } // iy

  } // iz
    
  // wait for all active threads to complete
  
  while (_activeThreads.size() > 0) {
    PrevReorderThread *thread = _activeThreads.front();
    _activeThreads.pop_front();
    _availThreads.push_back(thread);
    thread->waitForWorkToComplete();
  }

}

////////////////////////////////////////////////////////////
// Compute grid relative locations for one row

void PrevReorderInterp::_computeGridRelRow(int iz, int iy)

{

  // initialize beamHeight computations

  _beamHt.setInstrumentHtKm(_radarAltKm);

  // loop through the row

  double zz = _gridZLevels[iz];
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
    
    double elevDeg = beamHt.computeElevationDeg(zz, gndRange);

    // compute coords relative to radar

    double sinAz, cosAz;
    ta_sincos(azimuth * DEG_TO_RAD, &sinAz, &cosAz);
    double xxInstr = gndRange * sinAz;
    double yyInstr = gndRange * cosAz;
    double zzInstr = zz - _radarAltKm;

    GridLoc *loc = _gridLoc[iz][iy][ix];
    loc->el = elevDeg;
    loc->az = azimuth;
    loc->slantRange = beamHt.getSlantRangeKm();

    loc->xxInstr = xxInstr;
    loc->yyInstr = yyInstr;
    loc->zzInstr = zzInstr;
    
  } // ix

}

//////////////////////////////////////////////////
// interpolate onto the grid

void PrevReorderInterp::_doInterp()
{

  // initialize the output field arrays

  _initOutputArrays();

  // perform interpolation

  if (_params.use_multiple_threads) {
    _interpMultiThreaded();
  } else {
    _interpSingleThreaded();
  }

}

//////////////////////////////////////////////////
// interpolate entire volume in single thread

void PrevReorderInterp::_interpSingleThreaded()
{
  
  // perform the interpolation for each data block in order

  for (int blocky = 0; blocky < _nrows; blocky++) {
    for (int blockx = 0; blockx < _ncols; blockx++) {
      _interpBlock(blocky, blockx);
    } // blockx
  } // blocky

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void PrevReorderInterp::_interpMultiThreaded()
{

  // loop through the data blocks in Y
  
  for (int blocky = 0; blocky < _nrows; blocky++) {

    // loop through the data blocks in X

    for (int blockx = 0; blockx < _ncols; blockx++) {

      // is a thread available? if not wait for one
      
      PrevReorderThread *thread = NULL;
      if (_availThreads.size() > 0) {
        // get thread from available pool
        // it is doing no work
        thread = _availThreads.front();
        _availThreads.pop_front();
      } else {
        // get thread from active pool
        thread = _activeThreads.front();
        _activeThreads.pop_front();
        // wait for current work to complete
        thread->waitForWorkToComplete();
      }
      
      // set thread going to compute moments
      
      thread->setTask(PrevReorderThread::INTERP);
      thread->setXIndex(blockx);
      thread->setYIndex(blocky);
      thread->signalWorkToStart();
      
      // push onto active pool
      
      _activeThreads.push_back(thread);

    } // blockx

  } // blocky
    
  // wait for all active threads to complete
  
  while (_activeThreads.size() > 0) {
    PrevReorderThread *thread = _activeThreads.front();
    _activeThreads.pop_front();
    _availThreads.push_back(thread);
    thread->waitForWorkToComplete();
  }

}

//////////////////////////////////////////////////
// interpolate a block of grid cells

void PrevReorderInterp::_interpBlock(int blocky, int blockx)
{

  int nyPerBlock = (_gridNy - 1) / _nrows + 1;
  int nxPerBlock = (_gridNx - 1) / _ncols + 1;

  int startIy = blocky * nyPerBlock;
  int endIy = startIy + nyPerBlock - 1;
  if (endIy > _gridNy - 1) {
    endIy = _gridNy - 1;
  }

  int startIx = blockx * nxPerBlock;
  int endIx = startIx + nxPerBlock - 1;
  if (endIx > _gridNx - 1) {
    endIx = _gridNx - 1;
  }

  // compute xy search limits for creating kdtree
  // only including the points in this block and some
  // surrounding margin

  double searchMiny = _gridMiny + (startIy - 0.5) * _gridDy - _maxSearchRadius * 2;
  double searchMaxy = _gridMiny + (endIy + 0.5) * _gridDy + _maxSearchRadius * 2;

  double searchMinx = _gridMinx + (startIx - 0.5) * _gridDx - _maxSearchRadius * 2;
  double searchMaxx = _gridMinx + (endIx + 0.5) * _gridDx + _maxSearchRadius * 2;

  // Build the KD tree

  vector<KD_real *> kdVec;
  vector<radar_point_t> radarPoints;
  
  for (size_t ipt = 0; ipt < _radarPoints.size(); ipt++) {
    
    const radar_point_t &radarPt = _radarPoints[ipt];
    
    double xx = radarPt.xx;
    double yy = radarPt.yy;

    if (yy >= searchMiny && yy <= searchMaxy &&
        xx >= searchMinx && xx <= searchMaxx) {

      KD_real *kdPt = new KD_real[KD_DIM];
      kdPt[0] = radarPt.zz / _zSearchRatio;
      kdPt[1] = yy;
      kdPt[2] = xx;
      kdVec.push_back(kdPt);
      radarPoints.push_back(radarPt);
      
    } // if (yy >= searchMiny && yy <= searchMaxy ...
    
  } // ipt
  
  _printRunTime("KD tree - setting up radarPt array", true);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Processing blocky, blockx: " << blocky << ", " << blockx << endl;
    cerr << "  ncols, gridNy, nyPerBlock, startIy, endIy: "
         << _ncols << ", "
         << _gridNy << ", "
         << nyPerBlock << ", "
         << startIy << ", "
         << endIy << endl;
    cerr << "  nrows, gridNx, nxPerBlock, startIx, endIx: "
         << _nrows << ", "
         << _gridNx << ", "
         << nxPerBlock << ", "
         << startIx << ", "
         << endIx << endl;
  }

  if ((int) kdVec.size() < _params.reorder_npoints_search) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Too few points in block: " << kdVec.size() << endl;
      cerr << "  Will not be processed" << endl;
    }
    // free up KD vector of points
    for (size_t ii = 0; ii < kdVec.size(); ii++) {
      delete[] kdVec[ii];
    }
    kdVec.clear();
    return;
  }
  
  // create array of pointers to the pts in the vector
  
  KD_real **kdMat = new KD_real*[kdVec.size()];
  for (size_t ii = 0; ii < kdVec.size(); ii++) {
    kdMat[ii] = kdVec[ii];
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "  ==>> number of points in block: " << kdVec.size() << endl;
  }

  KD_tree kdTree((const KD_real **) kdMat, kdVec.size(), KD_DIM);
  _printRunTime("KD tree - building KD tree", true);
  
  // interpolate each point

  for (int iz = 0; iz < _gridNz; iz++) {
    for (int iy = startIy; iy <= endIy; iy++) {
      for (int ix = startIx; ix <= endIx; ix++) {
        _interpPoint(iz, iy, ix, kdTree, radarPoints);
      }
    } // iy
  } // iz

  // free up

  delete[] kdMat;
  for (size_t ii = 0; ii < kdVec.size(); ii++) {
    delete[] kdVec[ii];
  }
  kdVec.clear();
  radarPoints.clear();

}

////////////////////////////////////////////////////////////
// Interpolate fields for specified point

void PrevReorderInterp::_interpPoint(int iz, int iy, int ix,
                                 KD_tree &kdTree,
                                 const vector<radar_point_t> &radarPoints)

{

  int nNeighbors = _params.reorder_npoints_search;

  TaArray<int> indices_;
  int *indices = indices_.alloc(nNeighbors);
  
  TaArray<KD_real> distSq_;
  KD_real *distSq = distSq_.alloc(nNeighbors);
  
  // get the grid location
  
  const GridLoc *loc = _gridLoc[iz][iy][ix];
  
  if (loc->slantRange > _maxRangeKm) {
    return;
  }

  // set the query location

  KD_real queryLoc[KD_DIM];
  queryLoc[0] = loc->zzInstr / _zSearchRatio;
  queryLoc[1] = loc->yyInstr;
  queryLoc[2] = loc->xxInstr;
    
  // initialize indices
  
  for (int ii = 0; ii < nNeighbors; ii++) {
    indices[ii] = -1;
  }
  
  // get closest neighbor, check distance for sanity
  
  kdTree.nnquery(queryLoc, // query location
                 1, // get only 1 point
                 KD_EUCLIDEAN, // search metric
                 1, // Minkowski parameter
                 indices, // out: indices of nearest nbrs
                 distSq); // out: squares of distances of nbrs
  
  const radar_point_t &closestPt = radarPoints[indices[0]];
  double range = _startRangeKm + closestPt.igate * _gateSpacingKm;
  double dtest = _params.reorder_search_radius_km;
  if (_params.reorder_scale_search_radius_with_range) {
    dtest *= (range / _params.reorder_nominal_range_for_search_radius_km);
  }
  if (dtest < 1.0) {
    dtest = 1.0;
  }
  double dtestSq = dtest * dtest;
  if (distSq[0] > dtestSq) {
    // the closest point is greater than dtest away from cell
    // so don't process this cell
    return;
  }

  // Find nearest neighbors
  
  kdTree.nnquery(queryLoc, // query location
                 nNeighbors, // number of neighbors to search for
                 KD_EUCLIDEAN, // search metric
                 1, // Minkowski parameter
                 indices, // out: indices of nearest nbrs
                 distSq); // out: squares of distances of nbrs
  
  // load up points for interpolation, if they are within the
  // required distance

  vector<radar_point_t> interpPts;
  for (int jj = 0; jj < nNeighbors; jj++) {
    int index = indices[jj];
    if (index < 0) {
      continue;
    }
    if (distSq[jj] <= dtestSq) {
      radar_point_t thisPt = radarPoints[index];
      thisPt.wt = 1.0 / distSq[jj];
      interpPts.push_back(thisPt);
    } else {
      // no more
      break;
    }
  }

  if (interpPts.size() == 0) {
    // no points
    return;
  }

  // interpolate fields
  
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    
    if (_interpFields[ifield].isDiscrete || _params.use_nearest_neighbor) {
      _computeNearestGridPt(ifield, iz, iy, ix, interpPts);
    } else if (_interpFields[ifield].fieldFolds) {
      _computeFoldedGridPt(ifield, iz, iy, ix, interpPts);
    } else {
      _computeInterpGridPt(ifield, iz, iy, ix, interpPts);
    }
    
  } // ifield

}

///////////////////////////////////////////////////////
// load up data for a grid point using nearest neighbor

void PrevReorderInterp::_computeNearestGridPt(int ifield,
                                          int iz, int iy, int ix,
                                          const vector<radar_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double closestVal = 0.0;
  double maxWt = 0.0;
  int nContrib = 0;
  
  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    const radar_point_t &pt = interpPts[ii];
    const Ray *ray = _interpRays[pt.iray];
    if (ray) {
      // points are in order of closest first
      _accumNearest(ray, ifield, pt.igate, pt.wt,
                    closestVal, maxWt, nContrib);
      break;
    }
  }
  
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;

  if (nContrib > 0) {
    _outputFields[ifield][gridPtIndex] = closestVal;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation

void PrevReorderInterp::_computeInterpGridPt(int ifield,
                                         int iz, int iy, int ix,
                                         const vector<radar_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWts = 0.0;
  int nFound = 0;
  int nContrib = 0;
  int cumulativeFlag = 0;

  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    
    radar_point_t rpt(interpPts[ii]);
    GridLoc loc(*_gridLoc[iz][iy][ix]);

    if (_params.reorder_only_use_one_point_per_octant) {
      int flag = 1;
      if (rpt.zz > loc.zzInstr) {
        flag = flag << 4;
      }
      if (rpt.yy > loc.yyInstr) {
        flag = flag << 2;
      }
      if (rpt.xx > loc.xxInstr) {
        flag = flag << 1;
      }
      if ((flag & cumulativeFlag) > 0) {
        // already have a point in this quadrant
        continue;
      }
      cumulativeFlag |= flag;
    }

    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      _accumInterp(ray, ifield, rpt.igate, rpt.wt,
                   sumVals, sumWts, nContrib);
    }
    nFound++;

  } // ii

  // compute weighted mean
  
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;

  if (nContrib >= _params.min_nvalid_for_interp) {
    double interpVal = missingDouble;
    if (sumWts > 0) {
      interpVal = sumVals / sumWts;
    }
    _outputFields[ifield][gridPtIndex] = interpVal;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field

void PrevReorderInterp::_computeFoldedGridPt(int ifield,
                                         int iz, int iy, int ix,
                                         const vector<radar_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double sumX = 0.0;
  double sumY = 0.0;
  double sumWts = 0.0;
  int nFound = 0;
  int nContrib = 0;
  int cumulativeFlag = 0;
  
  for (size_t ii = 0; ii < interpPts.size(); ii++) {

    radar_point_t rpt(interpPts[ii]);
    GridLoc loc(*_gridLoc[iz][iy][ix]);

    if (_params.reorder_only_use_one_point_per_octant) {
      int flag = 1;
      if (rpt.zz > loc.zzInstr) {
        flag = flag << 4;
      }
      if (rpt.yy > loc.yyInstr) {
        flag = flag << 2;
      }
      if (rpt.xx > loc.xxInstr) {
        flag = flag << 1;
      }
      if ((flag & cumulativeFlag) > 0) {
        // already have a point in this quadrant
        continue;
      }
      cumulativeFlag |= flag;
    }

    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      _accumFolded(ray, ifield, rpt.igate, rpt.wt,
                   sumX, sumY, sumWts, nContrib);
    }
    nFound++;

  } // ii

  // compute weighted mean
  
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;

  if (nContrib >= _params.min_nvalid_for_interp) {
    const Field &intFld = _interpFields[ifield];
    double angleInterp = atan2(sumY, sumX);
    double valInterp = _getFoldValue(angleInterp,
                                     intFld.foldLimitLower, intFld.foldRange);
    _outputFields[ifield][gridPtIndex] = valInterp;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// write out data

int PrevReorderInterp::_writeOutputFile()
{

  if (_params.debug) {
    cerr << "  Writing output file ... " << endl;
  }

  // cedric is a special case
  
  if (_params.output_format == Params::CEDRIC) {
    return _writeCedricFile(false);
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
    cerr << "ERROR - PrevReorderInterp::processFile" << endl;
    cerr << "  Cannot write file to output_dir: "
         << _params.output_dir << endl;
    return -1;
  }

  return 0;

}


