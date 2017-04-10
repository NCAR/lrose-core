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
// ReorderInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation, following the 
// REORDER strategy of interpolation using the closest N points.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Oct 2012
//
///////////////////////////////////////////////////////////////

#include "ReorderInterp.hh"
#include "SvdData.hh"
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
using namespace std;

////////////////////////////////////////////////////////////////
// constructor

ReorderInterp::ReorderInterp(const string &progName,
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
  _gridLoc = NULL;        // not used here
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

ReorderInterp::~ReorderInterp()

{

  // free up threads

  _freeThreads();

  // check grid loc

  if (_gridLoc != NULL)  {
    cerr << "ERROR - ReorderInterp destructor, _gridLoc not NULL" << endl;
  }

  // free up memory

  _freeOutputArrays();
  _radarPoints.clear();

}

//////////////////////////////////////////////////
// free the threading objects

void ReorderInterp::_freeThreads()
{

  pthread_mutex_destroy(&_kdTreeMutex);
  
  // NOTE - thread pools free their threads in the destructor

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int ReorderInterp::interpVol()

{

  cerr << "!!!!!!!!!!!!!!!!!! IMPORTANT NOTE !!!!!!!!!!!!!!!!!!!!!!" << endl;
  cerr << "!!! REORDER should NOT be used for FIXED platforms !!!!!" << endl;
  cerr << "!!! Only use this for mobile platforms !!!!!!!!!!!!!!!!!" << endl;
  cerr << "!!! For fixed platforms use INTERP_MODE_CART instead !!!" << endl;
  cerr << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;

  _printRunTime("initialization");

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - ReorderInterp::interpVol()" << endl;
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

  // initialize projection for the radar location
  _initProjectionLocal();

  // compute the cartesian points for each gate

  _computeRadarPoints();
  
  // interpolate

  if (_params.debug) {
    cerr << "  Interpolating ... " << endl;
  }
  _doInterp();
  _printRunTime("interpolation");

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

////////////////////////////////////////////////////////////
// Initialize Z levels

void ReorderInterp::_initZLevels()

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

void ReorderInterp::_initGrid()

{

  _gridNx = _params.grid_xy_geom.nx;
  _gridMinx = _params.grid_xy_geom.minx;
  _gridDx = _params.grid_xy_geom.dx;
  
  _gridNy = _params.grid_xy_geom.ny;
  _gridMiny = _params.grid_xy_geom.miny;
  _gridDy = _params.grid_xy_geom.dy;
  
}
  
//////////////////////////////////////////////////
// create and initialize the threading objects

void ReorderInterp::_createThreads()
{

  // initialize mutex for kd tree since it is not thread safe

  pthread_mutex_init(&_kdTreeMutex, NULL);
  
  // initialize thread pool for interpolation
  // use same number of threads as vert levels
  // since we compute a plane in each thread

  for (int ii = 0; ii < _gridNz; ii++) {
    // for (int ii = 0; ii < _params.n_compute_threads; ii++) {
    PerformInterp *thread = new PerformInterp(this);
    _threadPoolInterp.addThreadToMain(thread);
  }

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void ReorderInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _gridNz;
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void ReorderInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void ReorderInterp::_initOutputArrays()
  
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
// Compute radar points in Cart space

void ReorderInterp::_computeRadarPoints()
{

  // set max ht for points in tree
  
  double searchMaxZ = _maxZ + 4 * _maxSearchRadius;
  
  // initialize beamHeight computations

  BeamHeight beamHt;
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }
  beamHt.setInstrumentHtKm(_radarAltKm);

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
    int rayStartIndex = (int) _radarPoints.size();

    _computeTagGates(nGates);

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
      radarPt.ray = ray;

      // tag the gates that will be used in the
      // kd tree to find the closest rays
      
      if (_tagGates[igate]) {
        radarPt.isTagPt = true;
      } else {
        radarPt.isTagPt = false;
      }
      
      radarPt.index = (int) _radarPoints.size();
      _radarPoints.push_back(radarPt);

    } // igate


    for (size_t ii = 0; ii < _tagGates.size(); ii++) {
    }

    // set ray index limits on each point

    int rayEndIndex = (int) _radarPoints.size() - 1;
    
    for (int ii = rayStartIndex; ii <= rayEndIndex; ii++) {
      radar_point_t &pt = _radarPoints[ii];
      pt.rayStartIndex = rayStartIndex;
      pt.rayEndIndex = rayEndIndex;
    }

  } // iray
  
  _printRunTime("Reorder - computing radar point locations");
  
}

////////////////////////////////////////////////////////////
// Compute the tag gate locations - these are used for
// identifying the rays closest to a grid point

void ReorderInterp::_computeTagGates(int nGates)
  
{

  // check if geom has changed

  if (_startRangeKm == _tagStartRangeKm &&
      _gateSpacingKm == _tagGateSpacingKm &&
      (int) _tagGates.size() >= nGates) {
    // geom has not changed
    return;
  }

  _tagGateSpacingKm = _gateSpacingKm;
  _tagStartRangeKm = _startRangeKm;
  _tagGates.clear();

  // loop through gates, starting at gate 5

  int nextTagPos = 1;
  
  for (int ii = 0; ii < nGates; ii++) {
    
    if (ii == nextTagPos) {

      _tagGates.push_back(true);

      // compute the search margin for this range
      
      double range = _startRangeKm + ii * _gateSpacingKm;
      double searchRadiusKm = _params.reorder_search_radius_km * 
        (range / _params.reorder_nominal_range_for_search_radius_km);
      
      // skip over the number of gates equal to the search margin
      
      int nskip = (int) ((searchRadiusKm / _gateSpacingKm) + 0.5);
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
// Initialize the projection for computations

void ReorderInterp::_initProjectionLocal()

{
  if (_params.debug) {
    cerr << "  Initializing projection ... " << endl;
  }

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
    cerr << "ReorderInterp::_initProjection()" << endl;
    cerr << "  _radarLat: " << _radarLat << endl;
    cerr << "  _radarLon: " << _radarLon << endl;
    cerr << "  _radarAltKm: " << _radarAltKm << endl;
    cerr << "  _gridOriginLat: " << _gridOriginLat << endl;
    cerr << "  _gridOriginLon: " << _gridOriginLon << endl;
  }

  // call the method on the base class
  
  _initProjection();

}

////////////////////////////////////////////////////////////
// Compute grid relative locations for one row

void ReorderInterp::_computeGridRelRow(int iz, int iy, GridLoc **locIn)
{

  // initialize beamHeight computations

  BeamHeight beamHt;
  beamHt.setInstrumentHtKm(_radarAltKm);

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

    GridLoc *loc = locIn[ix];
    loc->el = elevDeg;
    loc->az = azimuth;
    loc->slantRange = beamHt.getSlantRangeKm();

    loc->xxInstr = xxInstr;
    loc->yyInstr = yyInstr;
    loc->zzInstr = zzInstr;
    loc->zz = zz;
    
  } // ix

}

//////////////////////////////////////////////////
// build the KD tree for ray tag points

void ReorderInterp::_buildKdTree()
{

  for (size_t ipt = 0; ipt < _radarPoints.size(); ipt++) {
    
    radar_point_t radarPt = _radarPoints[ipt];
    if (!radarPt.isTagPt) {
      continue;
    }
    
    double xx = radarPt.xx;
    double yy = radarPt.yy;

    KD_real *kdPt = new KD_real[KD_DIM];
    kdPt[0] = radarPt.zz / _zSearchRatio;
    kdPt[1] = yy;
    kdPt[2] = xx;
    _kdVec.push_back(kdPt);
    _tagPoints.push_back(radarPt);
      
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

void ReorderInterp::_freeKdTree()
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

void ReorderInterp::_doInterp()
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

void ReorderInterp::_interpSingleThreaded()
{

  for (int iz = 0; iz < _gridNz; iz++) {
    _interpPlane(iz);
  } // iz

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void ReorderInterp::_interpMultiThreaded()
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

void ReorderInterp::_interpPlane(int iz)
  
{

  // allocate the GridLoc array for this plane and set values
  GridLoc ***gridLoc = 
    (GridLoc ***)umalloc2(_gridNy, _gridNx, sizeof(GridLoc *));
  
  for (int iy = 0; iy < _gridNy; iy++) {
    for (int ix = 0; ix < _gridNx; ix++) {
      gridLoc[iy][ix] = new GridLoc;
    }
    _computeGridRelRow(iz, iy, gridLoc[iy]);
  }

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
  
  for (int iy = 0; iy < _gridNy; iy++) {
    for (int ix = 0; ix < _gridNx; ix++) {
    
      NeighborProps *neighborProps = new NeighborProps;
      
      // get the grid location
      
      neighborProps->iz = iz;
      neighborProps->iy = iy;
      neighborProps->ix = ix;
      neighborProps->loc = gridLoc[iy][ix];
      if (neighborProps->loc->slantRange > _maxRangeKm) {
        delete neighborProps;
        continue;
      }
      
      // set the query location
      
      KD_real queryLoc[KD_DIM];
      queryLoc[0] = neighborProps->loc->zz / _zSearchRatio;
      queryLoc[1] = neighborProps->loc->yyInstr;
      queryLoc[2] = neighborProps->loc->xxInstr;
      
      // initialize tagIndexes
      for (int ii = 0; ii < nNeighbors; ii++) {
        tagIndexes[ii] = -1;
      }
      
      // get closest point from KD tree, and check distance
      
      kdTree.nnquery(queryLoc, // query location
                     1, // get only 1 point
                     KD_EUCLIDEAN, // search metric
                     1, // Minkowski parameter
                     tagIndexes, // out: indices of nearest nbrs
                     distSq); // out: squares of distances of nbrs

      if (tagIndexes[0] >= (int) _tagPoints.size()) {
        continue;
      }
      const radar_point_t &closestPt = _tagPoints[tagIndexes[0]];
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
        delete neighborProps;
        continue;
      }
      
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
      
      _interpPoint(*neighborProps, *gridLoc[iy][ix]);
      delete neighborProps;
    } // ix
  } // iy
  

  //
  // delete the GridLoc array
  //
  for (int iy = 0; iy < _gridNy; iy++) {
    for (int ix = 0; ix < _gridNx; ix++) {
      delete gridLoc[iy][ix];
    }
  }
  ufree2((void **) gridLoc);

  char buf[1000];
  sprintf(buf, "interpolating plane %d", iz);
  string s = buf;
  _printRunTime(s, false);
  
}

////////////////////////////////////////////////////////////
// Interpolate fields for specified point

void ReorderInterp::_interpPoint(const NeighborProps &neighborProps,
				 const GridLoc &pointLoc)
  
{

  // load up points for interpolation, if they are within the
  // required distance
  
  map<int, const radar_point_t> rayTags;
  for (size_t jj = 0; jj < neighborProps.tagIndexes.size(); jj++) {
    int tagIndex = neighborProps.tagIndexes[jj];
    radar_point_t thisPt = _tagPoints[tagIndex];
    pair<int, const radar_point_t> val(thisPt.iray, _tagPoints[tagIndex]);
    rayTags.insert(val);
  }

#ifdef ONEPTPEROCTANT  
  int cumulativeFlag = 0;
#endif

  vector<radar_point_t> interpPts;
  const GridLoc &loc = *neighborProps.loc;

  bool dataAbove = false;
  bool dataBelow = false;
    
  for (map<int, const radar_point_t>::iterator kk = rayTags.begin();
       kk != rayTags.end(); kk++) {
    
    // cerr << "222222222222, iray, zz, yy, xx, igate, tagIndex: "
    //      << kk->second.iray << ", "
    //      << kk->second.zz << ", "
    //      << kk->second.yy << ", "
    //      << kk->second.xx << ", "
    //      << kk->second.igate << ", "
    //      << kk->second.index << endl;

    // find the closest gates to the grid point
    
    ray_closest_t closestPts;
    _findClosestGates(loc, kk->second, closestPts);

#ifdef ONEPTPEROCTANT
    if (_params.reorder_only_use_one_point_per_octant) {

      // check if the octant is already occupied
      
      double meanzz = (closestPts.first.zz + closestPts.second.zz) / 2.0;
      double meanyy = (closestPts.first.yy + closestPts.second.yy) / 2.0;
      double meanxx = (closestPts.first.xx + closestPts.second.xx) / 2.0;
      
      int flag = 1;
      if (meanzz > loc.zz) {
        dataAbove = true;
        flag = flag << 4;
      } else {
        dataBelow = true;
      }
      if (meanyy > loc.yyInstr) {
        flag = flag << 2;
      }
      if (meanxx > loc.xxInstr) {
        flag = flag << 1;
      }
      if ((flag & cumulativeFlag) == 0) {
        // no point in this octant yet
        // closestArray.push_back(closestPts);
        if (closestPts.first.distSq < closestPts.second.distSq) {
          interpPts.push_back(closestPts.first);
        } else {
          interpPts.push_back(closestPts.second);
        }
        cumulativeFlag |= flag;
      } 
      
    } else {
#endif

      // use all points
      // closestArray.push_back(closestPts);

      // if (closestPts.first.distSq < closestPts.second.distSq) {
      //   interpPts.push_back(closestPts.first);
      // } else {
      //   interpPts.push_back(closestPts.second);
      // }

      interpPts.push_back(closestPts.first);
      interpPts.push_back(closestPts.second);

      double meanzz = (closestPts.first.zz + closestPts.second.zz) / 2.0;
      if (meanzz > loc.zz) {
        dataAbove = true;
      } else {
        dataBelow = true;
      }

#ifdef ONEPTPEROCTANT
    } // if (_params.reorder_only_use_one_point_per_octant)
#endif
      
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

  int iz = neighborProps.iz;
  int iy = neighborProps.iy;
  int ix = neighborProps.ix;

  //
  // If sharing Svd between all fields, create object now, otherwise
  // leave NULL
  //
  // SvdData *svd = NULL;
  // if (!_params.full_svd)
  // {
  //   svd = new SvdData(interpPts, pointLoc.xxInstr, pointLoc.yyInstr,
  // 		      pointLoc.zz);
  // }

  // interpolate fields
  
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    
    if (_interpFields[ifield].isDiscrete || _params.use_nearest_neighbor) {
      _computeNearestGridPt(ifield, iz, iy, ix, interpPts);
    } else {
      if (_params.reorder_weighted_interpolation) {
	if (_interpFields[ifield].fieldFolds) {
	  _computeWeightedFoldedGridPt(ifield, iz, iy, ix, interpPts);
	} else {
	  _computeWeightedInterpGridPt(ifield, iz, iy, ix, interpPts);
	}
      } else {
	if (_interpFields[ifield].fieldFolds) {
	  _computeFoldedGridPt(ifield, iz, iy, ix, pointLoc, interpPts);
	} else {
	  _computeInterpGridPt(ifield, iz, iy, ix, pointLoc, interpPts);
	}
      }
    }
    
  } // ifield

  // if (!_params.full_svd)
  // {
  //   delete svd;
  // }
}

///////////////////////////////////////////////////////
// find gates in ray closest to tag point

void ReorderInterp::_findClosestGates(const GridLoc &loc,
                                      const radar_point_t &pt,
                                      ray_closest_t &closest)

{
  
  // cerr << "00000000000000000000000 initIndex: "
  //      << pt.index << endl;
  // cerr << "00000000000000000000000 rayStartIndex: "
  //      << pt.rayStartIndex << endl;
  // cerr << "00000000000000000000000 rayEndIndex: "
  //      << pt.rayEndIndex << endl;

  int thisIndex = pt.index;
  if (thisIndex == pt.rayEndIndex) {
    thisIndex--;
  }
  int nextIndex = thisIndex + 1;

  double thisDistSq = _computeDistSq(loc, _radarPoints[thisIndex]);
  double nextDistSq = _computeDistSq(loc, _radarPoints[nextIndex]);
  
  // cerr << "11111111 thisIndex, thisDist: "
  //      << thisIndex << ", " << thisDist << endl;
  // cerr << "11111111 thisIndex, nextDist: "
  //      << nextIndex << ", " << nextDist << endl;

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
      thisDistSq = _computeDistSq(loc, _radarPoints[thisIndex]);
      nextDistSq = _computeDistSq(loc, _radarPoints[nextIndex]);
      if (nextDistSq > thisDistSq) {
        break;
      }
    }

  } else {

    while (nextIndex > pt.rayStartIndex) {
      thisIndex--;
      nextIndex--;
      thisDistSq = _computeDistSq(loc, _radarPoints[thisIndex]);
      nextDistSq = _computeDistSq(loc, _radarPoints[nextIndex]);
      if (nextDistSq > thisDistSq) {
        break;
      }
    }

  }

  closest.first = _radarPoints[thisIndex];
  closest.first.distSq = thisDistSq;
  closest.first.wt = 1.0 / thisDistSq;
  closest.second = _radarPoints[nextIndex];
  closest.second.distSq = nextDistSq;
  closest.second.wt = 1.0 / nextDistSq;
  
  // set distances
  
  // cerr << "22222222 nextIndex, nextDist: "
  //      << nextIndex << ", " << nextDist << endl;
  // cerr << "22222222 thisIndex, thisDist: "
  //      << thisIndex << ", " << thisDist << endl;

  // _printRadarPoint(cerr, _radarPoints[thisIndex]);

  // check

  // double minDist = 1.0e99;
  // int minIndex = -1;
  // for (int ii = pt.rayStartIndex; ii <= pt.rayEndIndex; ii++) {
  //   double dist = _computeDist(loc, _radarPoints[ii]);
  //   if (dist < minDist) {
  //     minDist = dist;
  //     minIndex = ii;
  //   }
  // }
  
  // if (minDist != thisDist && minDist != nextDist) {
  //   cerr << "XXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
  //   cerr << "333333333 minIndex, minDist: " 
  //        << minIndex << ", " << minDist << endl;
  //   _printRadarPoint(cerr, _radarPoints[minIndex]);
  //   cerr << "XXXXXXXXXXXXXXXXXXXXXXXXXX" << endl;
  // }

}

///////////////////////////////////////////////////////
// compute distance between radar pt and grid loc

double ReorderInterp::_computeDistSq(const GridLoc &loc,
                                     const radar_point_t &pt)

{
  double dx = loc.xxInstr - pt.xx;
  double dy = loc.yyInstr - pt.yy;
  double dz = loc.zz - pt.zz;
  double distSq = dx * dx + dy * dy + dz * dz;
  return distSq;
}

///////////////////////////////////////////////////////
// print a radar point

void ReorderInterp::_printRadarPoint(ostream &out,
                                     const radar_point_t &pt)

{
  out << "=========== ray el, az: " << pt.ray->el << ", " 
      << pt.ray->az << endl;
  out << "  index: " << pt.index << endl;
  out << "  rayStartIndex: " << pt.rayStartIndex << endl;
  out << "  rayEndIndex: " << pt.rayEndIndex << endl;
  out << "  iray: " << pt.iray << endl;
  out << "  igate: " << pt.igate << endl;
  out << "  xx: " << pt.xx << endl;
  out << "  yy: " << pt.yy << endl;
  out << "  zz: " << pt.zz << endl;
  out << "  wt: " << pt.wt << endl;
  out << "  el: " << pt.ray->el << endl;
  out << "  az: " << pt.ray->az << endl;
  out << "  isTagPt: " << pt.isTagPt << endl;
}

///////////////////////////////////////////////////////
// load up data for a grid point using nearest neighbor

void ReorderInterp::_computeNearestGridPt
  (int ifield,
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

void ReorderInterp::
  _computeInterpGridPt(int ifield,
                       int iz, int iy, int ix,
                       const GridLoc &loc,
                       const vector<radar_point_t> &interpPts)
  
{
  double v;
  bool good = _computeSvd(ifield, iz, iy, ix, loc, interpPts, v);
  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;
  if (good) {
    _outputFields[ifield][gridPtIndex] = v;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }
} 

bool
ReorderInterp::_computeSvd(int ifield, int iz, int iy, int ix, 
			   const GridLoc &loc,
			   const vector<radar_point_t> &interpPts,
			   double &v)
{
  // copy the interp points
  vector<radar_point_t> newInterpPts(interpPts);

  // try to collect all data
  vector<double> b;
  bool good = _collectLocalData(ifield, newInterpPts, loc, b);
  if (good) {
    SvdData svd(newInterpPts, loc.xxInstr, loc.yyInstr, loc.zz);
    if (!svd.isOk()) {
      cerr << "ERROR _computeSvd - Could not create SVD" << endl;
      return false;
    }
    if (svd.compute(b)) {
      // return constant term which is the last (3rd) one
      v = svd.getTerm(3);
      const Field &intFld = _interpFields[ifield];
      if (intFld.isBounded) {
	if (v < intFld.boundLimitLower || v > intFld.boundLimitUpper) {
	  if (_params.debug >= Params::DEBUG_VERBOSE) {
	    cerr << "Data field " << ifield << " out of bounds " << v << endl;
	  }
	  return false;
	}
      }
      // check limits
      double minVal, maxVal;
      _computeMinMax(b, minVal, maxVal);
      if (v < minVal || v > maxVal) {
        v = missingDouble;
        return false;
      }
      return true;
    } else {
      cerr << "ERROR _computeSvd - Could not compute SVD" << endl;
      return false;
    }
  } else {
    if (_params.debug  >= Params::DEBUG_VERBOSE) {
      cerr << "Not enough local data" << endl;
    }
    return false;
  }
}

// compute min and max of an array

void ReorderInterp::_computeMinMax(const vector<double> &bb,
                                   double &minVal, double &maxVal)
{
  if (bb.size() < 1) {
    minVal = missingDouble;
    maxVal = missingDouble;
    return;
  }
  minVal = bb[0];
  maxVal = bb[0];
  for (size_t ii = 1; ii < bb.size(); ii++) {
    double val = bb[ii];
    if (val < minVal) {
      minVal = val;
    }
    if (val > maxVal) {
      maxVal = val;
    }
  }
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation

void
ReorderInterp::
  _computeWeightedInterpGridPt(int ifield,
                               int iz, int iy, int ix,
                               const vector<radar_point_t> &interpPts)
{

  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWtsValid = 0.0;
  double sumWtsTotal = 0.0;
  int nFound = 0;
  int nContrib = 0;

  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    
    radar_point_t rpt(interpPts[ii]);

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

void ReorderInterp::
  _computeFoldedGridPt(int ifield,
                       int iz, int iy, int ix,
                       const GridLoc &loc,
                       const vector<radar_point_t> &interpPts)
  
{
  // copy interp points
  vector<radar_point_t> newInterpPts(interpPts);

  // try to collect all data
  vector<double> x, y;
  bool good = _collectLocalFoldedData(ifield, newInterpPts, x, y);
  double xfit=0, yfit=0;
  if (good) {
    SvdData info(newInterpPts, loc.xxInstr, loc.yyInstr, loc.zz);
    if (info.isOk())
    {
      if (info.compute(x)) {
	// return constant term which is the last (3rd) one
	xfit = info.getTerm(3);
        // check limits
        double minVal, maxVal;
        _computeMinMax(x, minVal, maxVal);
        if (xfit < minVal || xfit > maxVal) {
          good = false;
        }
      } else {
	cerr << "ERROR _computeFoldedGridPt " << endl;
	cerr << "Could not compute SVD result" << endl;
	good = false;
      }
      if (info.compute(y)) {
	// return constant term which is the last (3rd) one
	yfit = info.getTerm(3);
        // check limits
        double minVal, maxVal;
        _computeMinMax(y, minVal, maxVal);
        if (yfit < minVal || yfit > maxVal) {
          good = false;
        }
      } else {
	cerr << "ERROR _computeFoldedGridPt " << endl;
	cerr << "Could not compute SVD result" << endl;
	good = false;
      }
    } else {
      good = false;
      cerr << "ERROR _computeFoldedGridPt " << endl;
      cerr << "Could not create SVD data" << endl;
    }
  }

  int gridPtIndex = iz * _nPointsPlane + iy * _gridNx + ix;
  if (good) {
    double angleInterp = atan2(yfit, xfit);
    const Field &intFld = _interpFields[ifield];
    double valInterp =
      _getFoldValue(angleInterp, intFld.foldLimitLower, intFld.foldRange);
    _outputFields[ifield][gridPtIndex] = valInterp;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field

void
ReorderInterp::
  _computeWeightedFoldedGridPt(int ifield,
                               int iz, int iy, int ix,
                               const vector<radar_point_t> &interpPts)
  
{
  
  // sum up weighted vals
  
  double sumX = 0.0;
  double sumY = 0.0;
  double sumWtsValid = 0.0;
  double sumWtsTotal = 0.0;
  int nFound = 0;
  int nContrib = 0;
  
  for (size_t ii = 0; ii < interpPts.size(); ii++) {

    radar_point_t rpt(interpPts[ii]);

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
    double valInterp =
      _getFoldValue(angleInterp,
                    intFld.foldLimitLower, intFld.foldRange);
    _outputFields[ifield][gridPtIndex] = valInterp;
  } else {
    _outputFields[ifield][gridPtIndex] = missingFl32;
  }

}

///////////////////////////////////////////////////////////////
// get data for a gate

bool ReorderInterp::_getData(int ifield, const Interp::Ray *ray, 
                             int igate, double &v)
{
  if (ray->fldData) {
    
    int nGates = ray->nGates;
    fl32 missing = ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];

    if (field != NULL) {
      
      if (igate >= 0 && igate < nGates) {
        fl32 val = field[igate];
        if (val != missing) {
          v = val;
	  return true;
        }
      }
      
    } // if (field != NULL) 
    
  } // if (ray->fldData)
  return false;
}

///////////////////////////////////////////////////////////////
// get data for a gate, allow missing values

bool ReorderInterp::
  _getDataAllowMissing(int ifield, const Interp::Ray *ray,
                       int igate, double &v, double &missing,
                       bool &isMissing)
{
  if (ray->fldData) {
    int nGates = ray->nGates;
    missing = (double)ray->missingVal[ifield];
    fl32 *field = ray->fldData[ifield];
    if (field != NULL) {
      if (igate >= 0 && igate < nGates) {
        fl32 val = field[igate];
 	v = val;
        isMissing = (val == missing);
 	return true;
      }
    } // if (field != NULL) 
  } // if (ray->fldData)
  return false;
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation

bool ReorderInterp::_collectLocalData(int ifield, 
				      vector<radar_point_t> &interpPts,
				      const GridLoc &loc,
				      vector<double> &b)
{
  bool good = true;
  int numGood = 0;
  b.clear();
  vector<radar_point_t> iPts;
  bool above=false;
  bool below=false;
  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    
    radar_point_t rpt(interpPts[ii]);
    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      double v;
      if (_getData(ifield, ray, rpt.igate, v)) {
	if (interpPts[ii].zz >= loc.zz) {
	  above = true;
	}
	else if (interpPts[ii].zz <= loc.zz) {
	  below = true;
	}
	b.push_back(v);
	iPts.push_back(interpPts[ii]);
	numGood ++;
      }
    } else {
      if (_params.debug >= Params::DEBUG_NORM) {
	cerr << "No Ray" << endl;
      }
      good = false;
      break;
    }
  }
  if (numGood < _params.reorder_min_nvalid_for_interp) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Too few surrounding points " << numGood << ", no interpolation" 
	   << endl;
    }
    good = false;
  }
  if (_params.reorder_bound_grid_point_vertically) {
    if (!above || !below) {
      // not bounded in the vertical
      good = false;
    }
  }
  interpPts = iPts;
  return good;
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field

bool ReorderInterp::_collectLocalFoldedData(int ifield, 
					    vector<radar_point_t> &interpPts,
					    vector<double> &x,
					    vector<double> &y)
{
  bool good = true;

  x.clear();
  y.clear();
  vector<radar_point_t> iPts;
  const Field &intFld = _interpFields[ifield];

  for (size_t ii = 0; ii < interpPts.size(); ii++) {
    
    radar_point_t rpt(interpPts[ii]);
    const Ray *ray = _interpRays[rpt.iray];
    if (ray) {
      double v;
      if (_getData(ifield, ray, rpt.igate, v)) {
	double angle =
	  _getFoldAngle(v, intFld.foldLimitLower, intFld.foldRange);
	double sinVal, cosVal;
	ta_sincos(angle, &sinVal, &cosVal);
	x.push_back(cosVal);
	y.push_back(sinVal);
	iPts.push_back(interpPts[ii]);
      }
    } else {
      good = false;
      if (_params.debug) {
	cerr << "No Ray" << endl;
      }
      break;
    }
  }
  if (static_cast<int>(x.size()) < _params.reorder_min_nvalid_for_interp) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      cerr << "Too few surrounding points " << x.size() << 
	", no interpolation" << endl;
    }
    good = false;
  }
  interpPts = iPts;
  return good;
}

/////////////////////////////////////////////////////
// write out data

int ReorderInterp::_writeOutputFile()
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
    cerr << "ERROR - ReorderInterp::processFile" << endl;
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
ReorderInterp::PerformInterp::PerformInterp(ReorderInterp *obj) :
        _this(obj)
{
}  
// run method
void ReorderInterp::PerformInterp::run()
{
  _this->_interpPlane(_zIndex);
}

