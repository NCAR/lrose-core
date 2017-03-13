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
// SatInterp class - derived from Interp.
// Handles satellite RADAR or LIDAR data
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2013
//
///////////////////////////////////////////////////////////////

#include "SatInterp.hh"
#include "OutputMdv.hh"
#include <algorithm>
#include <map>
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
#include <Radx/RadxGeoref.hh>
using namespace std;

// Constructor

SatInterp::SatInterp(const string &progName,
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
  _zSearchRatio = _params.reorder_z_search_ratio;

  _kdTree = NULL;
  _kdMat = NULL;

  _tagStartRangeKm = -9999;
  _tagGateSpacingKm = -9999;

  // initialize the output grid dimensions

  _initZLevels();
  _initGrid();

  // set up thread objects

  _createThreads();
  
}

//////////////////////////////////////
// destructor

SatInterp::~SatInterp()

{

  // threading

  _freeThreads();
  pthread_mutex_destroy(&_kdTreeMutex);

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
  _instrPoints.clear();

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int SatInterp::interpVol()

{

  _printRunTime("initialization");

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - SatInterp::interpVol()" << endl;
    return -1;
  }

  // compute search radius at max range

  _maxSearchRadius = _params.reorder_search_radius_km;

  // compute grid locations relative to instr
  
  if (_params.debug) {
    cerr << "  Computing grid relative to instr ... " << endl;
  }
  _computeGridRelative();

  // compute the cartesian points for each gate

  _computeInstrPoints();
  
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

void SatInterp::_initZLevels()

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

void SatInterp::_initGrid()

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
// create the threading objects

void SatInterp::_createThreads()
{

  // initialize compute object

  pthread_mutex_init(&_kdTreeMutex, NULL);
  
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
// free the threading objects

void SatInterp::_freeThreads()
{

  // NOTE - thread pools free their threads in the destructor

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void SatInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _gridNz;
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void SatInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void SatInterp::_initOutputArrays()
  
{

  _allocOutputArrays();

  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    for (int jj = 0; jj < _nPointsVol; jj++) {
      _outputFields[ii][jj] = missingFl32;
    }
  }

  _printRunTime("initOutputArrays");

}

//////////////////////////////////////////////////
// Compute instr points in Cart space

void SatInterp::_computeInstrPoints()
{
  
  // Build the KD tree for 3D instr observations
  
  instr_point_t instrPt;
  _instrPoints.clear();

  // loop through rays
  
  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    
    const Ray *ray = _interpRays[iray];
    const RadxGeoref *georef = ray->inputRay->getGeoreference();
    double lat = _radarLat;
    double lon = _radarLon;
    if (georef) {
      lat = georef->getLatitude();
      lon = georef->getLongitude();
    }
    
    int nGates = ray->nGates;
    double range = _startRangeKm;
    double deltaRange  = _gateSpacingKm;
    if (_params.sat_data_invert_in_range) {
      range += nGates * _gateSpacingKm;
      deltaRange *= -1.0;
    }
    int rayStartIndex = (int) _instrPoints.size();

    _computeTagGates(nGates);

    for (int igate = 0; igate < nGates; igate++, range += deltaRange) {
      
      double xx, yy;
      _proj.latlon2xy(lat, lon, xx, yy);

      double zz = range;
      
      instrPt.xx = xx;
      instrPt.yy = yy;
      instrPt.zz = zz;
      instrPt.iray = iray;
      instrPt.igate = igate;
      instrPt.ray = ray;

      // tag the gates that will be used in the
      // kd tree to find the closest rays
      
      if (_tagGates[igate]) {
        instrPt.isTagPt = true;
      } else {
        instrPt.isTagPt = false;
      }
      
      instrPt.index = (int) _instrPoints.size();
      _instrPoints.push_back(instrPt);

    } // igate


    // set ray index limits on each point

    int rayEndIndex = (int) _instrPoints.size() - 1;
    
    for (int ii = rayStartIndex; ii <= rayEndIndex; ii++) {
      instr_point_t &pt = _instrPoints[ii];
      pt.rayStartIndex = rayStartIndex;
      pt.rayEndIndex = rayEndIndex;
    }

  } // iray
  
  _printRunTime("Reorder - computing instr point locations");
  
}

////////////////////////////////////////////////////////////
// Compute the tag gate locations - these are used for
// identifying the rays closest to a grid point

void SatInterp::_computeTagGates(int nGates)

{

  _tagGateSpacingKm = _gateSpacingKm;
  _tagStartRangeKm = _startRangeKm;
  _tagGates.clear();

  // loop through gates, starting at gate 5
  
  int nextTagPos = 1;
  
  for (int ii = 0; ii < nGates; ii++) {
    
    if (ii == nextTagPos) {
      
      _tagGates.push_back(true);

      // skip over the number of gates equal to the search margin
      
      int nskip = (int) ((_maxSearchRadius / _gateSpacingKm) + 0.5);
      if (nskip < 1) {
        nskip = 1;
      }
      nextTagPos += nskip;

    } else {

      _tagGates.push_back(false);

    }

  } // ii
  
}


////////////////////////////////////////////////////////////
// Compute grid locations relative to instr

void SatInterp::_computeGridRelative()

{

  _gridOriginLat = _params.grid_origin_lat;
  _gridOriginLon = _params.grid_origin_lon;
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
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

void SatInterp::_computeGridRelMultiThreaded()
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
// Compute grid relative locations for one row

void SatInterp::_computeGridRelRow(int iz, int iy)

{

  // loop through the row

  double zz = _gridZLevels[iz];
  double yy = _gridMiny + iy * _gridDy;
  double xx = _gridMinx;

  for (int ix = 0; ix < _gridNx; ix++, xx += _gridDx) {
    
    GridLoc *loc = _gridLoc[iz][iy][ix];
    
    loc->xxInstr = xx;
    loc->yyInstr = yy;
    loc->zzInstr = zz;

  } // ix

}

//////////////////////////////////////////////////
// build the KD tree for ray tag points

void SatInterp::_buildKdTree()
{

  for (size_t ipt = 0; ipt < _instrPoints.size(); ipt++) {
    
    instr_point_t instrPt = _instrPoints[ipt];
    if (!instrPt.isTagPt) {
      continue;
    }
    
    double xx = instrPt.xx;
    double yy = instrPt.yy;

    KD_real *kdPt = new KD_real[KD_DIM];
    kdPt[0] = instrPt.zz / _zSearchRatio;
    kdPt[1] = yy;
    kdPt[2] = xx;
    _kdVec.push_back(kdPt);
    _tagPoints.push_back(instrPt);
      
  } // ipt
  
  // create array of pointers to the pts in the vector
  
  _kdMat = new KD_real*[_kdVec.size()];
  for (size_t ii = 0; ii < _kdVec.size(); ii++) {
    _kdMat[ii] = _kdVec[ii];
  }
  
  _kdTree = new KD_tree((const KD_real **) _kdMat, _kdVec.size(), KD_DIM);
  _printRunTime("building KD tree");

  
}

//////////////////////////////////////////////////
// free the KD tree

void SatInterp::_freeKdTree()
{

  delete _kdTree;
  _kdTree = NULL;
  
  delete[] _kdMat;
  _kdMat = NULL;
  
  for (size_t ii = 0; ii < _kdVec.size(); ii++) {
    delete[] _kdVec[ii];
  }
  _kdVec.clear();
  _tagPoints.clear();

}

//////////////////////////////////////////////////
// interpolate onto the grid

void SatInterp::_doInterp()
{

  // initialize the output field arrays

  _initOutputArrays();

  // build the KD tree

  _buildKdTree();
  
  // perform interpolation

  if (_params.use_multiple_threads) {
    _interpMultiThreaded();
  } else {
    _interpSingleThreaded();
  }

  // free up the KD tree

  _freeKdTree();
  
}

//////////////////////////////////////////////////
// interpolate entire volume in single thread

void SatInterp::_interpSingleThreaded()
{
  
  // interpolate one plane at a time

  for (int iz = 0; iz < _gridNz; iz++) {
    _interpPlane(iz);
  } // iz

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void SatInterp::_interpMultiThreaded()
{

  _threadPoolInterp.initForRun();

  // loop through the Z layers
  for (int iz = 0; iz < _gridNz; iz++) {
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
      // reduce iz by 1 since we did not actually get a compute
      // thread yet for this row
      iz--;
    } else {
      // available thread, set it running
      thread->setZIndex(iz);
      thread->signalRunToStart();
    }
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
// Interpolate a plane at a time

void SatInterp::_interpPlane(int iz)

{

  // copy the KD tree

  pthread_mutex_lock(&_kdTreeMutex);
  KD_tree kdTree(*_kdTree);
  pthread_mutex_unlock(&_kdTreeMutex);
  
  // init
  
  int nNeighbors = _params.reorder_npoints_search;
  
  TaArray<int> tagIndexes_;
  int *tagIndexes = tagIndexes_.alloc(nNeighbors);
  
  TaArray<KD_real> distSq_;
  KD_real *distSq = distSq_.alloc(nNeighbors);

  // create a vector of neighbor details, one for each
  // point in the row
  
  vector<NeighborProps *> neighbors;
  
  for (int iy = 0; iy < _gridNy; iy++) {
    for (int ix = 0; ix < _gridNx; ix++) {
    
      NeighborProps *neighborProps = new NeighborProps;
      
      // get the grid location
      
      neighborProps->iz = iz;
      neighborProps->iy = iy;
      neighborProps->ix = ix;
      neighborProps->loc = _gridLoc[iz][iy][ix];
      
      // set the query location
      
      KD_real queryLoc[KD_DIM];
      queryLoc[0] = neighborProps->loc->zzInstr / _zSearchRatio;
      queryLoc[1] = neighborProps->loc->yyInstr;
      queryLoc[2] = neighborProps->loc->xxInstr;
      
      // initialize tagIndexes
      
      for (int ii = 0; ii < nNeighbors; ii++) {
        tagIndexes[ii] = -1;
      }
      
      double dtestSq = _maxSearchRadius * _maxSearchRadius;
      
      // Find nearest neighbors

      kdTree.nnquery(queryLoc, // query location
                     nNeighbors, // number of neighbors to search for
                     KD_EUCLIDEAN, // search metric
                     1, // Minkowski parameter
                     tagIndexes, // out: indices of nearest nbrs
                     distSq); // out: squares of distances of nbrs
      
      for (int jj = 0; jj < nNeighbors; jj++) {
        int tagIndex = tagIndexes[jj];
        if (tagIndex < 0) {
          continue;
        }
        if (distSq[jj] <= dtestSq) {
          neighborProps->tagIndexes.push_back(tagIndex);
          neighborProps->distSq.push_back(distSq[jj]);
        } else {
          // no more
          break;
        }
      }
      
      neighbors.push_back(neighborProps);
      
    } // ix
  } // iy
  
  // interp the points
  
  for (size_t ii = 0; ii < neighbors.size(); ii++) {
    _interpPoint(*neighbors[ii]);
  } // ix

  // free up

  for (size_t ii = 0; ii < neighbors.size(); ii++) {
    delete neighbors[ii];
  } // ix
  
  _printRunTime("interpolating plane", true);
  
}

////////////////////////////////////////////////////////////
// Interpolate fields for specified point

void SatInterp::_interpPoint(const NeighborProps &neighborProps)
  
{

  // load up points for interpolation, if they are within the
  // required distance
  
  map<int, const instr_point_t> rayTags;
  for (size_t jj = 0; jj < neighborProps.tagIndexes.size(); jj++) {
    int tagIndex = neighborProps.tagIndexes[jj];
    instr_point_t thisPt = _tagPoints[tagIndex];
    pair<int, const instr_point_t> val(thisPt.iray, _tagPoints[tagIndex]);
    rayTags.insert(val);
  }
  
  vector<instr_point_t> interpPts;
  const GridLoc &loc = *neighborProps.loc;

  bool dataAbove = false;
  bool dataBelow = false;
    
  for (map<int, const instr_point_t>::iterator kk = rayTags.begin();
       kk != rayTags.end(); kk++) {
    
    // find the closest gates to the grid point
    
    ray_closest_t closestPts;
    _findClosestGates(loc, kk->second, closestPts);
    
    interpPts.push_back(closestPts.first);
    interpPts.push_back(closestPts.second);
    
    double meanzz = (closestPts.first.zz + closestPts.second.zz) / 2.0;
    if (meanzz > loc.zzInstr) {
      dataAbove = true;
    } else {
      dataBelow = true;
    }
    
  } // for (map ...

  if (interpPts.size() == 0) {
    // no points
    return;
  }

  if (_params.reorder_bound_grid_point_vertically) {
    if (!dataAbove || !dataBelow) {
      // not bounded in the vertical
      return;
    }
  }

  // interpolate fields
  
  int iz = neighborProps.iz;
  int iy = neighborProps.iy;
  int ix = neighborProps.ix;

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
// find gates in ray closest to tag point

void SatInterp::_findClosestGates(const GridLoc &loc,
                                  const instr_point_t &pt,
                                  ray_closest_t &closest)

{

  int thisIndex = pt.index;
  if (thisIndex == pt.rayEndIndex) {
    thisIndex--;
  }
  int nextIndex = thisIndex + 1;

  double thisDistSq = _computeDistSq(loc, _instrPoints[thisIndex]);
  double nextDistSq = _computeDistSq(loc, _instrPoints[nextIndex]);

  bool searchForward = true;
  if (nextDistSq > thisDistSq) {
    // distance increasing, switch search direction
    searchForward = false;
  }

  if (!searchForward) {
    // swap variables
    double dtmp = thisDistSq;
    thisDistSq = nextDistSq;
    nextDistSq = dtmp;
    int itmp = thisIndex;
    thisIndex = nextIndex;
    nextIndex = itmp;
  }

  if (searchForward) {

    while (nextIndex < pt.rayEndIndex) {
      thisIndex++;
      nextIndex++;
      thisDistSq = _computeDistSq(loc, _instrPoints[thisIndex]);
      nextDistSq = _computeDistSq(loc, _instrPoints[nextIndex]);
      if (nextDistSq > thisDistSq) {
        break;
      }
    }

  } else {

    while (nextIndex > pt.rayStartIndex) {
      thisIndex--;
      nextIndex--;
      thisDistSq = _computeDistSq(loc, _instrPoints[thisIndex]);
      nextDistSq = _computeDistSq(loc, _instrPoints[nextIndex]);
      if (nextDistSq > thisDistSq) {
        break;
      }
    }

  }

  closest.first = _instrPoints[thisIndex];
  closest.first.distSq = thisDistSq;
  closest.first.wt = 1.0 / thisDistSq;
  closest.second = _instrPoints[nextIndex];
  closest.second.distSq = nextDistSq;
  closest.second.wt = 1.0 / nextDistSq;
  
}

///////////////////////////////////////////////////////
// compute distance between instr pt and grid loc

double SatInterp::_computeDistSq(const GridLoc &loc,
                                 const instr_point_t &pt)

{
  double dx = loc.xxInstr - pt.xx;
  double dy = loc.yyInstr - pt.yy;
  double dz = loc.zzInstr - pt.zz;
  double distSq = dx * dx + dy * dy + dz * dz;
  return distSq;
}

///////////////////////////////////////////////////////
// load up data for a grid point using nearest neighbor

void SatInterp::_computeNearestGridPt(int ifield,
                                          int iz, int iy, int ix,
                                          const vector<instr_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double closestVal = 0.0;
  double maxWt = 0.0;
  int nContrib = 0;
  
  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    const instr_point_t &pt = interpPts[ii];
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

void SatInterp::_computeInterpGridPt(int ifield,
                                     int iz, int iy, int ix,
                                     const vector<instr_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWtsValid = 0.0;
  double sumWtsTotal = 0.0;
  int nFound = 0;
  int nContrib = 0;

  for (size_t ii = 0; ii < interpPts.size(); ii++) {

    instr_point_t rpt(interpPts[ii]);
    GridLoc loc(*_gridLoc[iz][iy][ix]);

    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      _accumInterp(ray, ifield, rpt.igate, rpt.wt,
                   sumVals, sumWtsValid, nContrib);
      sumWtsTotal += rpt.wt;
    }
    nFound++;

  } // ii

  // compute ratio weight

  double wtRatio = sumWtsValid / sumWtsTotal;

  // compute weighted mean
  
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;

  if (wtRatio < _params.reorder_min_valid_wt_ratio) {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  } else if (nContrib >= _params.min_nvalid_for_interp) {
    double interpVal = missingDouble;
    if (sumWtsValid > 0) {
      interpVal = sumVals / sumWtsValid;
    }
    _outputFields[ifield][gridPtIndex] = interpVal;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }

}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field

void SatInterp::_computeFoldedGridPt(int ifield,
                                     int iz, int iy, int ix,
                                     const vector<instr_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double sumX = 0.0;
  double sumY = 0.0;
  double sumWtsValid = 0.0;
  double sumWtsTotal = 0.0;
  int nFound = 0;
  int nContrib = 0;
  
  for (size_t ii = 0; ii < interpPts.size(); ii++) {

    instr_point_t rpt(interpPts[ii]);
    GridLoc loc(*_gridLoc[iz][iy][ix]);

    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      _accumFolded(ray, ifield, rpt.igate, rpt.wt,
                   sumX, sumY, sumWtsValid, nContrib);
    }
    nFound++;
    sumWtsTotal += rpt.wt;

  } // ii

  // compute ratio weight

  double wtRatio = sumWtsValid / sumWtsTotal;

  // compute weighted mean
  
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;

  if (wtRatio < _params.reorder_min_valid_wt_ratio) {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  } else if (nContrib >= _params.min_nvalid_for_interp) {
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

int SatInterp::_writeOutputFile()
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
    cerr << "ERROR - SatInterp::processFile" << endl;
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
SatInterp::ComputeGridRelative::ComputeGridRelative(SatInterp *obj) :
        _this(obj)
{
}  
// run method
void SatInterp::ComputeGridRelative::run()
{
  _this->_computeGridRelRow(_zIndex, _yIndex);
}

///////////////////////////////////////////////////////////////
// PerformInterp thread
///////////////////////////////////////////////////////////////
// Constructor
SatInterp::PerformInterp::PerformInterp(SatInterp *obj) :
        _this(obj)
{
}  
// run method
void SatInterp::PerformInterp::run()
{
  _this->_interpPlane(_zIndex);
}


