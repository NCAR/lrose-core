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
// CartInterp class - derived from Interp.
// Used for full 3-D Cartesian interpolation.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2012
//
///////////////////////////////////////////////////////////////

#include "CartInterp.hh"
#include "OutputMdv.hh"
#include <toolsa/pjg.h>
#include <toolsa/mem.h>
#include <toolsa/sincos.h>
#include <toolsa/toolsa_macros.h>
#include <Radx/RadxRay.hh>
#include <Radx/RadxField.hh>
#include <Radx/RadxTime.hh>
#include <Radx/RadxPath.hh>
#include <Radx/RadxSweep.hh>
#include <Mdv/DsMdvx.hh>
#include <algorithm>
using namespace std;

const double CartInterp::_searchResAz = 0.1;
const double CartInterp::_searchResEl = 0.1;
const double CartInterp::_searchAzOverlapDeg = 20.0;
const double CartInterp::_searchAzOverlapHalf = CartInterp::_searchAzOverlapDeg / 2.0;

// Constructor

CartInterp::CartInterp(const string &progName,
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
  
  _searchMatrixLowerLeft = NULL;
  _searchMatrixUpperLeft = NULL;
  _searchMatrixLowerRight = NULL;
  _searchMatrixUpperRight = NULL;

  _threadFillSearchLowerLeft = NULL;
  _threadFillSearchLowerRight = NULL;
  _threadFillSearchUpperLeft = NULL;
  _threadFillSearchUpperRight = NULL;

  _prevRadarLat = _prevRadarLon = _prevRadarAltKm = -9999.0;
  _gridLoc = NULL;
  _outputFields = NULL;

  _nContribDebug = NULL;
  _gridAzDebug = NULL;
  _gridElDebug = NULL;
  _gridRangeDebug = NULL;

  _llElDebug = NULL;
  _llAzDebug = NULL;
  _lrElDebug = NULL;
  _lrAzDebug = NULL;
  _ulElDebug = NULL;
  _ulAzDebug = NULL;
  _urElDebug = NULL;
  _urAzDebug = NULL;

  // create debug fields if needed

  if (_params.output_debug_fields) {
    _createDebugFields();
  }
  
  // initialize the output grid dimensions
  
  _initGrid();

  // set up thread objects

  _createThreads();

  // set up ConvStrat object

#ifdef NOTNOW
  // create convective/stratiform fields if needed
  if (_params.identify_convective_stratiform_split) {
    _createConvStratFields();
  }
#endif
  
  if (_params.identify_convective_stratiform_split) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _convStrat.setVerbose(true);
    } else if (_params.debug) {
      _convStrat.setDebug(true);
    }
    _convStrat.setMinValidHtKm(_params.conv_strat_min_valid_height);
    _convStrat.setMaxValidHtKm(_params.conv_strat_max_valid_height);
    _convStrat.setMinValidDbz(_params.conv_strat_min_valid_dbz);
    _convStrat.setDbzForDefiniteConvection
      (_params.conv_strat_dbz_threshold_for_definite_convection);
    _convStrat.setConvectiveRadiusKm(_params.conv_strat_convective_radius_km);
    _convStrat.setTextureRadiusKm(_params.conv_strat_texture_radius_km);
    _convStrat.setMinValidFractionForTexture
      (_params.conv_strat_min_valid_fraction_for_texture);
    _convStrat.setMinTextureForConvection
      (_params.conv_strat_min_texture_for_convection);
  }
  _gotConvStrat = false;

}

//////////////////////////////////////
// destructor

CartInterp::~CartInterp()

{

  _freeThreads();
  _freeSearchMatrix();
  _freeGridLoc();
  _freeOutputArrays();
  _freeDerivedFields();

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int CartInterp::interpVol()

{

  _printRunTime("Cart interp - reading data");

  if (_gridLoc == NULL) {
    _initGrid();
  }

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - CartInterp::interpVol()" << endl;
    return -1;
  }

  // initialize the output field arrays
  
  _printRunTime("Cart interp - before _initOutputArrays");
  _initOutputArrays();
  _printRunTime("Cart interp - after _initOutputArrays");

  // compute the scan azimuth and elevation delta angle
  // these are used in the search process

  _computeAzimuthDelta();
  _computeElevationDelta();

  if (_params.debug) {
    cerr << "  _scanDeltaAz: " << _scanDeltaAz << endl;
    cerr << "  _scanDeltaEl: " << _scanDeltaEl << endl;
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
    cerr << "  _spansNorth: " << (char *) (_spansNorth? "Y":"N") << endl;
  }
  
  // compute search matrix angle limits - keep the matrix
  // as small as possible for efficiency
  
  _printRunTime("Cart interp - before computeSearchLimits");
  _computeSearchLimits();
  _printRunTime("Computing search limits");

  // fill the search matrix

  if (_params.debug) {
    cerr << "  Filling search matrix ... " << endl;
  }
  _printRunTime("Cart interp - before fillSearchMatrix");
  _fillSearchMatrix();
  _printRunTime("Filling search matrix");
  
  // compute grid locations relative to radar

  if (_params.debug) {
    cerr << "  Computing grid relative to radar ... " << endl;
  }
  _printRunTime("Cart interp - before _computeGridRelative");
  _computeGridRelative();
  _printRunTime("Computing grid relative to radar");

  // interpolate

  if (_params.debug) {
    cerr << "  Interpolating ... " << endl;
  }
  _printRunTime("Cart interp - before doInterp");
  _doInterp();
  _printRunTime("Interpolating");

  // compute convective stratiform split

  _gotConvStrat = false;
  if (_params.identify_convective_stratiform_split) {
    // convective / stratiform split
    _printRunTime("Cart interp - before strat/conv");
    if (_convStratCompute() == 0) {
      _gotConvStrat = true;
    }
    _printRunTime("Cart interp - after strat/conv");
  }
  
  // transform for output

  _transformForOutput();

  // write out data

  _printRunTime("Cart interp - before _writeOutputFile");

  if (_writeOutputFile()) {
    cerr << "ERROR - CartInterp::interpVol" << endl;
    cerr << "  Cannot write output file" << endl;
    return -1;
  }

  // if requested, write out the search matrices

  if (_params.write_search_matrix_files) {
    if (_writeSearchMatrices()) {
      cerr << "ERROR - CartInterp::interpVol" << endl;
      cerr << "  Cannot write output file" << endl;
      return -1;
    }
  }

  _printRunTime("Writing output files");

  // clean up

  _freeSearchMatrix();
   if (_params.free_memory_between_files) {
    _freeGridLoc();
  }
 
  return 0;

}

//////////////////////////////////////////////////
// create the threading objects

void CartInterp::_createThreads()
{

  // threads for search

  _threadFillSearchLowerLeft = new FillSearchLowerLeft(this);
  _threadFillSearchLowerRight = new FillSearchLowerRight(this);
  _threadFillSearchUpperLeft = new FillSearchUpperLeft(this);
  _threadFillSearchUpperRight = new FillSearchUpperRight(this);

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

void CartInterp::_freeThreads()
{

  // free threads we created for search

  if (_threadFillSearchLowerLeft) {
    delete _threadFillSearchLowerLeft;
  }
  if (_threadFillSearchLowerRight) {
    delete _threadFillSearchLowerRight;
  }
  if (_threadFillSearchUpperLeft) {
    delete _threadFillSearchUpperLeft;
  }
  if (_threadFillSearchUpperRight) {
    delete _threadFillSearchUpperRight;
  }

  // NOTE - thread pools free their threads in the destructor

}

//////////////////////////////////////////////////
// create the debug Cart fields

void CartInterp::_createDebugFields()

{
  
  _nContribDebug = new DerivedField("nContrib", "n_points_contrib", "count", true);
  _derived3DFields.push_back(_nContribDebug);
  
  _gridAzDebug = new DerivedField("gridAz", "grid_azimiuth", "deg", true);
  _derived3DFields.push_back(_gridAzDebug);
  
  _gridElDebug = new DerivedField("gridEl", "grid_elevation", "deg", true);
  _derived3DFields.push_back(_gridElDebug);
  
  _gridRangeDebug = new DerivedField("gridRange", "grid_slant_range", "km", true);
  _derived3DFields.push_back(_gridRangeDebug);
  
  _llElDebug = new DerivedField("llEl", "lower_left_el", "deg", true);
  _derived3DFields.push_back(_llElDebug);
  _llAzDebug = new DerivedField("llAz", "lower_left_az", "deg", true);
  _derived3DFields.push_back(_llAzDebug);
  
  _lrElDebug = new DerivedField("lrEl", "lower_right_el", "deg", true);
  _derived3DFields.push_back(_lrElDebug);
  _lrAzDebug = new DerivedField("lrAz", "lower_right_az", "deg", true);
  _derived3DFields.push_back(_lrAzDebug);
  
  _ulElDebug = new DerivedField("ulEl", "upper_left_el", "deg", true);
  _derived3DFields.push_back(_ulElDebug);
  _ulAzDebug = new DerivedField("ulAz", "upper_left_az", "deg", true);
  _derived3DFields.push_back(_ulAzDebug);
  
  _urElDebug = new DerivedField("urEl", "upper_right_el", "deg", true);
  _derived3DFields.push_back(_urElDebug);
  _urAzDebug = new DerivedField("urAz", "upper_right_az", "deg", true);
  _derived3DFields.push_back(_urAzDebug);
  
}

#ifdef NOTNOW
  
//////////////////////////////////////////////////
// create the conv-strat fields

void CartInterp::_createConvStratFields()

{
  
  bool writeDebug = _params.conv_strat_write_debug_fields;

  _convStratDbzMax = new DerivedField("DbzMax",
                                      "max_dbz_for_conv_strat", 
                                      "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzMax);
  
  _convStratDbzCount = new DerivedField("DbzCount",
                                        "n_points_dbz_for_conv_strat", 
                                        "count", writeDebug);
  _derived3DFields.push_back(_convStratDbzCount);
  
  _convStratDbzSum = new DerivedField("DbzSum",
                                      "sum_dbz_for_conv_strat",
                                      "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzSum);
  
  _convStratDbzSqSum = new DerivedField("DbzSqSum",
                                        "sum_dbz_sq_for_conv_strat", 
                                        "dbz^2", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqSum);
  
  _convStratDbzSqSqSum = new DerivedField("DbzSqSqSum",
                                          "sum_dbz_sq_sq_for_conv_strat",
                                          "dbz^4", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqSqSum);
  
  _convStratDbzTexture = new DerivedField("DbzTexture",
                                          "dbz_texture_for_conv_strat",
                                          "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzTexture);

  _convStratFilledTexture = new DerivedField("FilledTexture",
                                             "filled_dbz_texture_for_conv_strat",
                                             "dbz", writeDebug);
  _derived3DFields.push_back(_convStratFilledTexture);

  _convStratDbzSqTexture = new DerivedField("DbzSqTexture",
                                            "dbz_sq_texture_for_conv_strat",
                                            "dbz", writeDebug);
  _derived3DFields.push_back(_convStratDbzSqTexture);
  
  _convStratFilledSqTexture = new DerivedField("FilledSqTexture",
                                               "filled_dbz_sq_texture_for_conv_strat",
                                               "dbz", writeDebug);
  _derived3DFields.push_back(_convStratFilledSqTexture);

  _convStratDbzColMax = new DerivedField("DbzColMax",
                                         "col_max_dbz_for_conv_strat",
                                         "dbz", writeDebug);
  _derived2DFields.push_back(_convStratDbzColMax);

  _convStratMeanTexture = new DerivedField("MeanTexture",
                                           "mean_dbz_texture_for_conv_strat",
                                           "dbz", writeDebug);
  _derived2DFields.push_back(_convStratMeanTexture);

  _convStratMeanSqTexture = new DerivedField("MeanSqTexture",
                                             "mean_dbz_sq_texture_for_conv_strat",
                                             "dbz", writeDebug);
  _derived2DFields.push_back(_convStratMeanSqTexture);

  _convStratCategory = new DerivedField("ConvStrat",
                                        "category_for_conv_strat",
                                        "", true);
  _derived2DFields.push_back(_convStratCategory);
  
}

#endif
  
//////////////////////////////////////////////////
// free the derived fields

void CartInterp::_freeDerivedFields()
  
{
  for (size_t ii = 0; ii < _derived3DFields.size(); ii++) {
    delete _derived3DFields[ii];
  }
  _derived3DFields.clear();
  for (size_t ii = 0; ii < _derived2DFields.size(); ii++) {
    delete _derived2DFields[ii];
  }
  _derived2DFields.clear();
}


////////////////////////////////////////////////////////////
// Initialize Z levels

void CartInterp::_initZLevels()

{

  _freeZLevels();

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

}

////////////////////////////////////////////////////////////
// Free Z levels

void CartInterp::_freeZLevels()

{
  _gridZLevels.clear();
}

////////////////////////////////////////////////////////////
// Initialize output grid

void CartInterp::_initGrid()

{

  // initialize the Z levels

  _initZLevels();

  // init the xy grid

  _gridNx = _params.grid_xy_geom.nx;
  _gridMinx = _params.grid_xy_geom.minx;
  _gridDx = _params.grid_xy_geom.dx;
  
  _gridNy = _params.grid_xy_geom.ny;
  _gridMiny = _params.grid_xy_geom.miny;
  _gridDy = _params.grid_xy_geom.dy;

  _nPointsPlane = _gridNx * _gridNy;
  _nPointsVol = _nPointsPlane * _gridNz;

  _freeGridLoc();
  
  _gridLoc = (GridLoc ****)
    umalloc3(_gridNz, _gridNy, _gridNx, sizeof(GridLoc *));
  
  for (int iz = 0; iz < _gridNz; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      for (int ix = 0; ix < _gridNx; ix++) {
        _gridLoc[iz][iy][ix] = new GridLoc;
      }
    }
  }

  for (size_t ii = 0; ii < _derived3DFields.size(); ii++) {
    _derived3DFields[ii]->alloc(_nPointsVol, _gridZLevels);
  }

  vector<double> singleLevel;
  singleLevel.push_back(0.0);
  for (size_t ii = 0; ii < _derived2DFields.size(); ii++) {
    _derived2DFields[ii]->alloc(_nPointsPlane, singleLevel);
  }

#ifdef JUNK
  if (_params.identify_convective_stratiform_split) {
    _convStratDbzCount->setToZero();
    _convStratDbzSum->setToZero();
    _convStratDbzSqSum->setToZero();
    _convStratDbzSqSqSum->setToZero();
  }
#endif

}
  
////////////////////////////////////////////////////////////
// Free grid loc array

void CartInterp::_freeGridLoc()
  
{
  
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

  _gridLoc = NULL;

  _prevRadarLat = _prevRadarLon = _prevRadarAltKm = -9999.0;

}

////////////////////////////////////////////////////////////
// Allocate the output arrays for the gridded fields

void CartInterp::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void CartInterp::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void CartInterp::_initOutputArrays()
  
{

  _allocOutputArrays();

  for (size_t ii = 0; ii < _interpFields.size(); ii++) {
    for (int jj = 0; jj < _nPointsVol; jj++) {
      _outputFields[ii][jj] = missingFl32;
    }
  }

}

//////////////////////////////////////////////////
// Compute the search matrix limits
// keeping it as small as possible for efficiency

void CartInterp::_computeSearchLimits()
{

  // elevation

  double minEl = 9999.0;
  double maxEl = -9999.0;
  
  vector<RadxRay *> &rays = _readVol.getRays();
  for (size_t ii = 0; ii < rays.size(); ii++) {
    double el = rays[ii]->getElevationDeg();
    if (el < minEl) {
      minEl = el;
    }
    if (el > maxEl) {
      maxEl = el;
    }
  }

  double elevExtension = _beamWidthDegV *
    _params.beam_width_fraction_for_data_limit_extension * 2.0;

  minEl -= elevExtension;
  maxEl += elevExtension;

  // set elevation resolution and search radius
  
  _searchRadiusEl = _scanDeltaEl + _beamWidthDegV + 1.0;
  
  _searchMinEl =
    (int) floor((minEl - _searchRadiusEl) / _searchResEl) * _searchResEl;
  _searchMaxEl =
    (int) floor((maxEl + _searchRadiusEl) / _searchResEl + 1.0) * _searchResEl;
  _searchNEl = (int) ((_searchMaxEl - _searchMinEl) / _searchResEl + 1);
  _searchMaxDistEl = (int) (_searchRadiusEl / _searchResEl + 0.5);
  _searchMaxCount = _searchMaxDistEl;
  
  // set azimuth resolution and search radius
  
  _searchRadiusAz = _scanDeltaAz + _beamWidthDegH + 1.0;

  if (_params.debug) {
    cerr << "  _searchRadiusEl: " << _searchRadiusEl << endl;
    cerr << "  _searchRadiusAz: " << _searchRadiusAz << endl;
  }

  if (_isSector) {
    
    if (_params.debug) {
      cerr << "  _dataSectorStartAzDeg: " << _dataSectorStartAzDeg << endl;
      cerr << "  _dataSectorEndAzDeg: " << _dataSectorEndAzDeg << endl;
    }

    double searchAzRangeDeg = _dataSectorEndAzDeg - _dataSectorStartAzDeg;
    _searchMinAz = _dataSectorStartAzDeg;
    _searchNAz = (int) (searchAzRangeDeg / _searchResAz + 1);
    _searchMaxDistAz = (int) (_searchRadiusAz / _searchResAz + 0.5);

  } else {
    
    // full 360

    double minAzDeg = 0;
    double maxAzDeg = 360.0 + _searchAzOverlapDeg;
    
    double searchAzRangeDeg = maxAzDeg - minAzDeg;
    _searchMinAz = minAzDeg;
    _searchNAz = (int) (searchAzRangeDeg / _searchResAz + 1);
    _searchMaxDistAz = (int) (_searchRadiusAz / _searchResAz + 0.5);

  }
  
  if (_searchMaxDistAz > _searchMaxCount) {
    _searchMaxCount = _searchMaxDistAz;
  }

  if (_params.debug) {
    cerr << "  _searchMinAz: " << _searchMinAz << endl;
    cerr << "  _searchNAz: " << _searchNAz << endl;
    cerr << "  _searchMaxDistAz: " << _searchMaxDistAz << endl;
  }

}

////////////////////////////////////////////////////////////
// Compute grid locations relative to radar

void CartInterp::_computeGridRelative()

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
        _computeGridRow(iz, iy);
      } // iy
    } // iz
    
  }

}

//////////////////////////////////////////////////////
// compute grid rows in multi-threaded mode

void CartInterp::_computeGridRelMultiThreaded()
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

void CartInterp::_computeGridRow(int iz, int iy)

{

  // initialize beamHeight computations

  BeamHeight beamHt;
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }
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

    GridLoc *loc = _gridLoc[iz][iy][ix];
    loc->el = elevDeg;
    loc->az = azimuth;
    loc->slantRange = beamHt.getSlantRangeKm();
    loc->gndRange = beamHt.getGndRangeKm();

    loc->xxInstr = xxInstr;
    loc->yyInstr = yyInstr;
    loc->zzInstr = zzInstr;
    
  } // ix

}

////////////////////////////////////////////////////////////
// Allocate the search matrix

void CartInterp::_allocSearchMatrix()

{

  _searchMatrixLowerLeft = 
    (SearchPoint **) umalloc2(_searchNEl, _searchNAz, sizeof(SearchPoint));

  _searchMatrixUpperLeft = 
    (SearchPoint **) umalloc2(_searchNEl, _searchNAz, sizeof(SearchPoint));

  _searchMatrixLowerRight = 
    (SearchPoint **) umalloc2(_searchNEl, _searchNAz, sizeof(SearchPoint));

  _searchMatrixUpperRight = 
    (SearchPoint **) umalloc2(_searchNEl, _searchNAz, sizeof(SearchPoint));

}

////////////////////////////////////////////////////////////
// Free up the search matrix

void CartInterp::_freeSearchMatrix()

{
  if (_searchMatrixLowerLeft) {
    ufree2((void **) _searchMatrixLowerLeft);
    _searchMatrixLowerLeft = NULL;
  }
  if (_searchMatrixUpperLeft) {
    ufree2((void **) _searchMatrixUpperLeft);
    _searchMatrixUpperLeft = NULL;
  }
  if (_searchMatrixLowerRight) {
    ufree2((void **) _searchMatrixLowerRight);
    _searchMatrixLowerRight = NULL;
  }
  if (_searchMatrixUpperRight) {
    ufree2((void **) _searchMatrixUpperRight);
    _searchMatrixUpperRight = NULL;
  }
}

////////////////////////////////////////////////////////////
// Initalize the search matrix

void CartInterp::_initSearchMatrix()

{

  for (int iel = 0; iel < _searchNEl; iel++) {
    for (int iaz = 0; iaz < _searchNAz; iaz++) {
      _searchMatrixLowerLeft[iel][iaz].clear();
      _searchMatrixUpperLeft[iel][iaz].clear();
      _searchMatrixLowerRight[iel][iaz].clear();
      _searchMatrixUpperRight[iel][iaz].clear();
    }
  }

  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    
    const Ray *ray = _interpRays[iray];

    // compute elevation index
    
    double el = ray->el;
    if (el < _searchMinEl || el > _searchMaxEl) {
      continue;
    }
    int iel = _getSearchElIndex(el);
    
    // compute azimuth index
    
    double az = ray->az;
    if (_isSector) {
      az = _conditionAz(az);
    }
    int iaz = _getSearchAzIndex(az);
    if (iaz < 0) {
      continue;
    }

    // set rays at index location

    _searchMatrixLowerLeft[iel][iaz].ray = ray;
    _searchMatrixLowerLeft[iel][iaz].rayEl = ray->el;
    _searchMatrixLowerLeft[iel][iaz].rayAz = ray->az;

    _searchMatrixUpperLeft[iel][iaz].ray = ray;
    _searchMatrixUpperLeft[iel][iaz].rayEl = ray->el;
    _searchMatrixUpperLeft[iel][iaz].rayAz = ray->az;

    _searchMatrixLowerRight[iel][iaz].ray = ray;
    _searchMatrixLowerRight[iel][iaz].rayEl = ray->el;
    _searchMatrixLowerRight[iel][iaz].rayAz = ray->az;

    _searchMatrixUpperRight[iel][iaz].ray = ray;
    _searchMatrixUpperRight[iel][iaz].rayEl = ray->el;
    _searchMatrixUpperRight[iel][iaz].rayAz = ray->az;

  } // iray

  if (_isSector) {
    
    // condition the ray azimuth to the correct 360
    
    if (_spansNorth) {
      for (int iaz = 0; iaz < _searchNAz; iaz++) {
        for (int iel = 0; iel < _searchNEl; iel++) {
          if (_searchMatrixLowerLeft[iel][iaz].ray) {
            _searchMatrixLowerLeft[iel][iaz].rayAz =
              _conditionAz(_searchMatrixLowerLeft[iel][iaz].rayAz);
          }
          if (_searchMatrixLowerRight[iel][iaz].ray) {
            _searchMatrixLowerRight[iel][iaz].rayAz =
              _conditionAz(_searchMatrixLowerRight[iel][iaz].rayAz);
          }
          if (_searchMatrixUpperLeft[iel][iaz].ray) {
            _searchMatrixUpperLeft[iel][iaz].rayAz =
              _conditionAz(_searchMatrixUpperLeft[iel][iaz].rayAz);
          }
          if (_searchMatrixUpperRight[iel][iaz].ray) {
            _searchMatrixUpperRight[iel][iaz].rayAz =
              _conditionAz(_searchMatrixUpperRight[iel][iaz].rayAz);
          }
        } // iel
      } // iaz
    } // if (_spansNorth)

  } else {
    
    // for full 360, copy the low azimuths into extra region at the
    // high end, so that we can interpolate across the N line
    // so search can cross N line
    
    int sourceIndexStart = _getSearchAzIndex(0.0);
    int sourceIndexEnd = _getSearchAzIndex(_searchAzOverlapDeg);
    int targetIndexStart = _getSearchAzIndex(360.0);
    int targetOffset = targetIndexStart - sourceIndexStart;
    
    for (int iaz = sourceIndexStart; iaz <= sourceIndexEnd; iaz++) {
      int jaz = iaz + targetOffset;
      if (jaz < _searchNAz) {
        for (int iel = 0; iel < _searchNEl; iel++) {
          _searchMatrixLowerLeft[iel][jaz] = _searchMatrixLowerLeft[iel][iaz];
          _searchMatrixLowerRight[iel][jaz] = _searchMatrixLowerRight[iel][iaz];
          _searchMatrixUpperLeft[iel][jaz] = _searchMatrixUpperLeft[iel][iaz];
          _searchMatrixUpperRight[iel][jaz] = _searchMatrixUpperRight[iel][iaz];
          _searchMatrixLowerLeft[iel][jaz].rayAz += 360.0;
          _searchMatrixLowerRight[iel][jaz].rayAz += 360.0;
          _searchMatrixUpperLeft[iel][jaz].rayAz += 360.0;
          _searchMatrixUpperRight[iel][jaz].rayAz += 360.0;
        }
      }
    }
    
  } // if (_isSector) 

}


////////////////////////////////////////////////////////////
// Fill the search matrix, using the el/az for each ray
// in the current volume

void CartInterp::_fillSearchMatrix()

{

  _freeSearchMatrix();
  _allocSearchMatrix();
  _initSearchMatrix();

  // we compute the matrix in each of 4 threads

  if (_params.use_multiple_threads) {

    // start all threads

    _threadFillSearchLowerLeft->signalRunToStart();
    _threadFillSearchUpperLeft->signalRunToStart();
    _threadFillSearchLowerRight->signalRunToStart();
    _threadFillSearchUpperRight->signalRunToStart();

    // wait for each thread to complete

    _threadFillSearchLowerLeft->waitForRunToComplete();
    _threadFillSearchUpperLeft->waitForRunToComplete();
    _threadFillSearchLowerRight->waitForRunToComplete();
    _threadFillSearchUpperRight->waitForRunToComplete();

  } else {

    // start each thread and wait to complete
    // before starting next one

    _threadFillSearchLowerLeft->signalRunToStart();
    _threadFillSearchLowerLeft->waitForRunToComplete();

    _threadFillSearchUpperLeft->signalRunToStart();
    _threadFillSearchUpperLeft->waitForRunToComplete();

    _threadFillSearchLowerRight->signalRunToStart();
    _threadFillSearchLowerRight->waitForRunToComplete();

    _threadFillSearchUpperRight->signalRunToStart();
    _threadFillSearchUpperRight->waitForRunToComplete();

  }
     
  if (_params.debug >= Params::DEBUG_EXTRA) {
    _printSearchMatrix(stderr, 1);
  }

}

///////////////////////////////////////////////////////////
// print search matrix

void CartInterp::_printSearchMatrix(FILE *out, int res)
{

  for (int iel = 0; iel < _searchNEl; iel += res) {
    for (int iaz = 0; iaz < _searchNAz; iaz += res) {

      const SearchPoint &ll = _searchMatrixLowerLeft[iel][iaz];
      const SearchPoint &ul = _searchMatrixUpperLeft[iel][iaz];
      const SearchPoint &lr = _searchMatrixLowerRight[iel][iaz];
      const SearchPoint &ur = _searchMatrixUpperRight[iel][iaz];

      if (!ll.ray && !ul.ray && !lr.ray && !ur.ray) {
        continue;
      }

      fprintf(out,
              "iel, iaz, el, az: "
              "%4d %4d "
              "%7.3f %7.3f ",
              iel, iaz,
              iel * _searchResEl + _searchMinEl,
              iaz * _searchResAz + _searchMinAz);

      if (ll.ray) {
        fprintf(out, "LL %7.3f %7.3f %3d ", ll.rayEl, ll.rayAz, ll.ray->sweepIndex);
      } else {
        fprintf(out, "LL %7.3f %7.3f %3d ", -99.999, -99.999, -99);
      }
      if (ul.ray) {
        fprintf(out, "UL %7.3f %7.3f %3d ", ul.rayEl, ul.rayAz, ul.ray->sweepIndex);
      } else {
        fprintf(out, "UL %7.3f %7.3f %3d ", -99.999, -99.999, -99);
      }
      if (lr.ray) {
        fprintf(out, "LR %7.3f %7.3f %3d ", lr.rayEl, lr.rayAz, lr.ray->sweepIndex);
      } else {
        fprintf(out, "LR %7.3f %7.3f %3d ", -99.999, -99.999, -99);
      }
      if (ur.ray) {
        fprintf(out, "UR %7.3f %7.3f %3d ", ur.rayEl, ur.rayAz, ur.ray->sweepIndex);
      } else {
        fprintf(out, "UR %7.3f %7.3f %3d ", -99.999, -99.999, -99);
      }
      fprintf(out, "\n");

    } // iaz
  } // iel

}

///////////////////////////////////////////////////////////
// print point in search matrix

void CartInterp::_printSearchMatrixPoint(FILE *out, int iel, int iaz)
{

  const SearchPoint &ll = _searchMatrixLowerLeft[iel][iaz];
  const SearchPoint &ul = _searchMatrixUpperLeft[iel][iaz];
  const SearchPoint &lr = _searchMatrixLowerRight[iel][iaz];
  const SearchPoint &ur = _searchMatrixUpperRight[iel][iaz];
  
  cout << "----------------------------------------------" << endl;
  
  fprintf(out,
          "Search matrix point: iel, iaz, el, az: "
          "%4d %4d "
          "%7.3f %7.3f ",
          iel, iaz,
          iel * _searchResEl + _searchMinEl,
          iaz * _searchResAz + _searchMinAz);
  
  if (ll.ray) {
    fprintf(out, "LL %7.3f %7.3f ", ll.rayEl, ll.rayAz);
  } else {
    fprintf(out, "LL %7.3f %7.3f ", -99.999, -99.999);
  }
  if (ul.ray) {
    fprintf(out, "UL %7.3f %7.3f ", ul.rayEl, ul.rayAz);
  } else {
    fprintf(out, "UL %7.3f %7.3f ", -99.999, -99.999);
  }
  if (lr.ray) {
    fprintf(out, "LR %7.3f %7.3f ", lr.rayEl, lr.rayAz);
  } else {
    fprintf(out, "LR %7.3f %7.3f ", -99.999, -99.999);
  }
  if (ur.ray) {
    fprintf(out, "UR %7.3f %7.3f ", ur.rayEl, ur.rayAz);
  } else {
    fprintf(out, "UR %7.3f %7.3f ", -99.999, -99.999);
  }
  fprintf(out, "\n");
  
  cout << "----------------------------------------------" << endl;

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray down and left of the search point
//
// We do this by propagating the ray information up and
// to the right.

int CartInterp::_fillSearchLowerLeft(int level,
                                     vector<SearchIndex> &thisSearch,
                                     vector<SearchIndex> &nextSearch)
  
{

  // for initial level, prepare initial search vector
  
  if (level == 0) {
    thisSearch.clear();
    for (int iel = 0; iel < _searchNEl - 1; iel++) {
      for (int iaz = 0; iaz < _searchNAz - 1; iaz++) {
        SearchPoint &sp = _searchMatrixLowerLeft[iel][iaz];
        if (sp.ray != NULL) {
          SearchIndex si(iel, iaz);
          thisSearch.push_back(si);
        }
      }
    }
  } else {
    thisSearch = nextSearch;
  }
  
  // clear next search

  nextSearch.clear();
  
  // loop through the search points

  int count = 0;
  for (size_t ii = 0; ii < thisSearch.size(); ii++) {
    
    int iel = thisSearch[ii].elIndex;
    int iaz = thisSearch[ii].azIndex;

    if (iel >= _searchNEl - 1) {
      continue;
    }
    if (iaz >= _searchNAz - 1) {
      continue;
    }

    SearchPoint &sp = _searchMatrixLowerLeft[iel][iaz];
    if (sp.elDist >= _searchMaxDistEl) {
      continue;
    }
    if (sp.azDist >= _searchMaxDistAz) {
      continue;
    }

    // propagate to the right
    
    SearchPoint &right = _searchMatrixLowerLeft[iel][iaz+1];
    if (right.ray == NULL) {
      right.level = level + 1;
      right.azDist = sp.azDist + 1;
      right.elDist = sp.elDist;
      right.ray = sp.ray;
      right.rayEl = sp.rayEl;
      right.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel, iaz+1);
      nextSearch.push_back(si);
    }
    
    // propagate up
    
    SearchPoint &up = _searchMatrixLowerLeft[iel+1][iaz];
    if (up.ray == NULL) {
      up.level = level + 1;
      up.elDist = sp.elDist + 1;
      up.azDist = sp.azDist;
      up.ray = sp.ray;
      up.rayEl = sp.rayEl;
      up.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel+1, iaz);
      nextSearch.push_back(si);
    }
    
  } // ii

  return count;

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray up and left of the search point
//
// We do this by propagating the ray information down and
// to the right.

int CartInterp::_fillSearchUpperLeft(int level,
                                     vector<SearchIndex> &thisSearch,
                                     vector<SearchIndex> &nextSearch)

{

  // for initial level, prepare initial search vector
  
  if (level == 0) {
    thisSearch.clear();
    for (int iel = _searchNEl - 1; iel > 0; iel--) {
      for (int iaz = 0; iaz < _searchNAz - 1; iaz++) {
        SearchPoint &sp = _searchMatrixUpperLeft[iel][iaz];
        if (sp.ray != NULL) {
          SearchIndex si(iel, iaz);
          thisSearch.push_back(si);
        }
      }
    }
  } else {
    thisSearch = nextSearch;
  }

  // clear next search

  nextSearch.clear();
  
  // loop through the search points

  int count = 0;
  for (size_t ii = 0; ii < thisSearch.size(); ii++) {
    
    int iel = thisSearch[ii].elIndex;
    int iaz = thisSearch[ii].azIndex;

    if (iel <= 0) {
      continue;
    }
    if (iaz >= _searchNAz - 1) {
      continue;
    }


    SearchPoint &sp = _searchMatrixUpperLeft[iel][iaz];
    if (sp.elDist >= _searchMaxDistEl) {
      continue;
    }
    if (sp.azDist >= _searchMaxDistAz) {
      continue;
    }

    // propagate to the right
    
    SearchPoint &right = _searchMatrixUpperLeft[iel][iaz+1];
    if (right.ray == NULL) {
      right.level = level + 1;
      right.azDist = sp.azDist + 1;
      right.elDist = sp.elDist;
      right.ray = sp.ray;
      right.rayEl = sp.rayEl;
      right.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel, iaz+1);
      nextSearch.push_back(si);
    }

    // propagate down
    
    SearchPoint &down = _searchMatrixUpperLeft[iel-1][iaz];
    if (down.ray == NULL) {
      down.level = level + 1;
      down.elDist = sp.elDist + 1;
      down.azDist = sp.azDist;
      down.ray = sp.ray;
      down.rayEl = sp.rayEl;
      down.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel-1, iaz);
      nextSearch.push_back(si);
    }
    
  } // ii

  return count;

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray down and right of the search point
//
// We do this by propagating the ray information up and
// to the left.

int CartInterp::_fillSearchLowerRight(int level,
                                      vector<SearchIndex> &thisSearch,
                                      vector<SearchIndex> &nextSearch)

{

  // for initial level, prepare initial search vector
  
  if (level == 0) {
    thisSearch.clear();
    for (int iel = 0; iel < _searchNEl - 1; iel++) {
      for (int iaz = _searchNAz - 1; iaz > 0; iaz--) {
        SearchPoint &sp = _searchMatrixLowerRight[iel][iaz];
        if (sp.ray != NULL) {
          SearchIndex si(iel, iaz);
          thisSearch.push_back(si);
        }
      }
    }
  } else {
    thisSearch = nextSearch;
  }

  // clear next search

  nextSearch.clear();
  
  // loop through the search points

  int count = 0;
  for (size_t ii = 0; ii < thisSearch.size(); ii++) {
    
    int iel = thisSearch[ii].elIndex;
    int iaz = thisSearch[ii].azIndex;

    if (iel >= _searchNEl - 1) {
      continue;
    }
    if (iaz <= 0) {
      continue;
    }

    SearchPoint &sp = _searchMatrixLowerRight[iel][iaz];
    if (sp.elDist >= _searchMaxDistEl) {
      continue;
    }
    if (sp.azDist >= _searchMaxDistAz) {
      continue;
    }
    
    // propagate to the left
    
    SearchPoint &left = _searchMatrixLowerRight[iel][iaz-1];
    if (left.ray == NULL) {
      left.level = level + 1;
      left.azDist = sp.azDist + 1;
      left.elDist = sp.elDist;
      left.ray = sp.ray;
      left.rayEl = sp.rayEl;
      left.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel, iaz-1);
      nextSearch.push_back(si);
    }
    
    // propagate up
    
    SearchPoint &up = _searchMatrixLowerRight[iel+1][iaz];
    if (up.ray == NULL) {
      up.level = level + 1;
      up.elDist = sp.elDist + 1;
      up.azDist = sp.azDist;
      up.ray = sp.ray;
      up.rayEl = sp.rayEl;
      up.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel+1, iaz);
      nextSearch.push_back(si);
    }
    
  } // ii

  return count;

}

////////////////////////////////////////////////////////////
// Fill the matrix for ray up and right of the search point
//
// We do this by propagating the ray information below and
// to the left.

int CartInterp::_fillSearchUpperRight(int level,
                                      vector<SearchIndex> &thisSearch,
                                      vector<SearchIndex> &nextSearch)

{

  // for initial level, prepare initial search vector
  
  if (level == 0) {
    thisSearch.clear();
    for (int iel = _searchNEl - 1; iel > 0; iel--) {
      for (int iaz = _searchNAz - 1; iaz > 0; iaz--) {
        SearchPoint &sp = _searchMatrixUpperRight[iel][iaz];
        if (sp.ray != NULL) {
          SearchIndex si(iel, iaz);
          thisSearch.push_back(si);
        }
      }
    }
  } else {
    thisSearch = nextSearch;
  }

  // clear next search

  nextSearch.clear();
  
  // loop through the search points

  int count = 0;
  for (size_t ii = 0; ii < thisSearch.size(); ii++) {
    
    int iel = thisSearch[ii].elIndex;
    int iaz = thisSearch[ii].azIndex;

    if (iel <= 0) {
      continue;
    }
    if (iaz <= 0) {
      continue;
    }

    SearchPoint &sp = _searchMatrixUpperRight[iel][iaz];
    if (sp.elDist >= _searchMaxDistEl) {
      continue;
    }
    if (sp.azDist >= _searchMaxDistAz) {
      continue;
    }
    
    // propagate to the left
    
    SearchPoint &left = _searchMatrixUpperRight[iel][iaz-1];
    if (left.ray == NULL) {
      left.level = level + 1;
      left.azDist = sp.azDist + 1;
      left.elDist = sp.elDist;
      left.ray = sp.ray;
      left.rayEl = sp.rayEl;
      left.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel, iaz-1);
      nextSearch.push_back(si);
    }
    
    // propagate down
    
    SearchPoint &down = _searchMatrixUpperRight[iel-1][iaz];
    if (down.ray == NULL) {
      down.level = level + 1;
      down.elDist = sp.elDist + 1;
      down.azDist = sp.azDist;
      down.ray = sp.ray;
      down.rayEl = sp.rayEl;
      down.rayAz = sp.rayAz;
      count++;
      SearchIndex si(iel-1, iaz);
      nextSearch.push_back(si);
    }
    
  } // ii

  return count;

}

/////////////////////////////////////////////////////////
// get the elevation index for the search matrix, given
// the elevation angle
//
// Returns -1 if out of bounds

int CartInterp::_getSearchElIndex(double el) 
{ 
  int iel = (int) floor((el - _searchMinEl) / _searchResEl + 0.5);
  if (iel < 0) {
    iel = -1;
  } else if (iel > _searchNEl - 1) {
    iel = -1;
  }
  return iel;
}

/////////////////////////////////////////////////////////
// get the azimuth index for the search matrix, given
// the azimuth angle
//
// Returns -1 if out of bounds

int CartInterp::_getSearchAzIndex(double az) 
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
// get the elevation given the search index

double CartInterp::_getSearchEl(int index) 
{
  return _searchMinEl + index * _searchResEl;
}
  
/////////////////////////////////////////////////////////
// get the azimuth given the search index

double CartInterp::_getSearchAz(int index) 
{
  return _searchMinAz + index * _searchResAz;
}
  
//////////////////////////////////////////////////
// interpolate onto the grid

void CartInterp::_doInterp()
{

  // perform the interpolation

  if (_params.use_multiple_threads) {
    _interpMultiThreaded();
  } else {
    _interpSingleThreaded();
  }

}

//////////////////////////////////////////////////
// interpolate entire volume in single thread

void CartInterp::_interpSingleThreaded()
{
  
  // interpolate one column at a time

  for (int iz = 0; iz < _gridNz; iz++) {
    for (int iy = 0; iy < _gridNy; iy++) {
      _interpRow(iz, iy);
    } // iy
  } // iz

}

//////////////////////////////////////////////////////
// interpolate volume in threads, one X row at a time

void CartInterp::_interpMultiThreaded()
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

void CartInterp::_interpRow(int iz, int iy)

{

  int ptIndex = iz * _nPointsPlane + iy * _gridNx;

  for (int ix = 0; ix < _gridNx; ix++, ptIndex++) {

    // get the grid location

    const GridLoc *loc = _gridLoc[iz][iy][ix];
    
    if (_gridAzDebug) {
      _gridAzDebug->data[ptIndex] = loc->az;
    }
    if (_gridElDebug) {
      _gridElDebug->data[ptIndex] = loc->el;
    }
    if (_gridRangeDebug) {
      _gridRangeDebug->data[ptIndex] = loc->slantRange;
    }

    // find starting location in search matrix

    int iel = _getSearchElIndex(loc->el);
    double az = _conditionAz(loc->az);
    int iaz = _getSearchAzIndex(az);

    if (iel < 0 || iaz < 0) {
      continue;
    }

    // get rays around the grid point

    SearchPoint ll = _searchMatrixLowerLeft[iel][iaz];
    SearchPoint ul = _searchMatrixUpperLeft[iel][iaz];
    SearchPoint lr = _searchMatrixLowerRight[iel][iaz];
    SearchPoint ur = _searchMatrixUpperRight[iel][iaz];

    // we need to check that these search rays in fact do bound
    // the search point. Because our search cells have finite dimensions -
    // i.e. _searchResEl x _searchResAz, it can happen that the ray
    // position may be on the incorrect side of the grid position.
    // If that is the case, we need to shift the search box by up
    // to one index in each dimension.

    // we loop twice to make sure we cover the case in which a move
    // in one dimension then requires a move in the other dimension

    // check to make sure ll ray is actually below and
    // to the left of the grid point
    
    if (ll.ray) {
      int jel = iel;
      int jaz = iaz;
      for (int ii = 0; ii < 2; ii++) {
        if ((ll.rayEl > loc->el) && (jel > 0)) {
          jel--;
          ll = _searchMatrixLowerLeft[jel][jaz];
          if (!ll.ray) break;
        }
        if ((ll.rayAz > az) && (jaz > 0)) {
          jaz--;
          ll = _searchMatrixLowerLeft[jel][jaz];
          if (!ll.ray) break;
        }
      } // ii
    }
    
    // check to make sure ul ray is actually above and
    // to the left of the grid point
    
    if (ul.ray) {
      int jel = iel;
      int jaz = iaz;
      for (int ii = 0; ii < 2; ii++) {
        if ((ul.rayEl < loc->el) && (jel < _searchNEl - 1)) {
          jel++;
          ul = _searchMatrixUpperLeft[jel][jaz];
          if (!ul.ray) break;
        }
        if ((ul.rayAz > az) && (jaz > 0)) {
          jaz--;
          ul = _searchMatrixUpperLeft[jel][jaz];
          if (!ul.ray) break;
        }
      } // ii
    }
    
    // check to make sure lr ray is actually below and
    // to the right of the grid point
    
    if (lr.ray) {
      int jel = iel;
      int jaz = iaz;
      for (int ii = 0; ii < 2; ii++) {
        if ((lr.rayEl > loc->el) && (jel > 0)) {
          jel--;
          lr = _searchMatrixLowerRight[jel][jaz];
          if (!lr.ray) break;
        }
        if ((lr.rayAz < az) && (jaz < _searchNAz - 1)) {
          jaz++;
          lr = _searchMatrixLowerRight[jel][jaz];
          if (!lr.ray) break;
        }
      } // ii
    }
    
    // check to make sure ur ray is actually above and
    // to the right of the grid point
    
    if (ur.ray) {
      int jel = iel;
      int jaz = iaz;
      for (int ii = 0; ii < 2; ii++) {
        if ((ur.rayEl < loc->el) && (jel < _searchNEl - 1)) {
          jel++;
          ur = _searchMatrixUpperRight[jel][jaz];
          if (!ur.ray) break;
        }
        if ((ur.rayAz < az) && (jaz < _searchNAz - 1)) {
          jaz++;
          ur = _searchMatrixUpperRight[jel][jaz];
          if (!ur.ray) break;
        }
      } // ii
    }
    
    // count number of available rays around this point
    // initialize az and el to be used for interp

    int nAvail = 0;

    if (ll.ray) {
      nAvail++;
      ll.interpEl = ll.rayEl;
      ll.interpAz = ll.rayAz;
    } else {
      ll.interpEl = loc->el;
      ll.interpAz = az;
    }

    if (ul.ray) {
      nAvail++;
      ul.interpEl = ul.rayEl;
      ul.interpAz = ul.rayAz;
    } else {
      ul.interpEl = loc->el;
      ul.interpAz = az;
    }

    if (lr.ray) {
      nAvail++;
      lr.interpEl = lr.rayEl;
      lr.interpAz = lr.rayAz;
    } else {
      lr.interpEl = loc->el;
      lr.interpAz = az;
    }

    if (ur.ray) {
      nAvail++;
      ur.interpEl = ur.rayEl;
      ur.interpAz = ur.rayAz;
    } else {
      ur.interpEl = loc->el;
      ur.interpAz = az;
    }

    // set debugging data as required
    
    if (ll.ray) {
      if (_llElDebug) {
        _llElDebug->data[ptIndex] = ll.rayEl;
      }
      if (_llAzDebug) {
        _llAzDebug->data[ptIndex] = ll.rayAz;
      }
    }

    if (ul.ray) {
      if (_ulElDebug) {
        _ulElDebug->data[ptIndex] = ul.rayEl;
      }
      if (_ulAzDebug) {
        _ulAzDebug->data[ptIndex] = ul.rayAz;
      }
    }

    if (lr.ray) {
      if (_lrElDebug) {
        _lrElDebug->data[ptIndex] = lr.rayEl;
      }
      if (_lrAzDebug) {
        _lrAzDebug->data[ptIndex] = lr.rayAz;
      }
    }

    if (ur.ray) {
      if (_urElDebug) {
        _urElDebug->data[ptIndex] = ur.rayEl;
      }
      if (_urAzDebug) {
        _urAzDebug->data[ptIndex] = ur.rayAz;
      }
    }

    // make sure we have at least 2 rays from which to interpolate
    if (nAvail < 2) {
      // only 1 ray, cannot compute for this point
      continue;
    }

    // if we have exactly 2 available rays, make sure we are within the
    // beam width of the edge of the measured data
    
    if (nAvail == 2) {
      // determine whether we have the point on the side or
      // above/below
      double angleError = 0;
      double beamWidth = 1.0;
      if (ll.ray && ul.ray) {
        // data is to the left
        angleError = MIN(fabs(az - _conditionAz(ll.ray->azForLimits)),
                         fabs(az - _conditionAz(ul.ray->azForLimits)));
        beamWidth = _beamWidthDegH;
      } else if (lr.ray && ur.ray) {
        // data is to the right
        angleError = MIN(fabs(az - _conditionAz(lr.ray->azForLimits)),
                         fabs(az - _conditionAz(ur.ray->azForLimits)));
        beamWidth = _beamWidthDegH;
      } else if (ll.ray && lr.ray) {
        // data is below
        angleError = MIN(fabs(loc->el - ll.ray->elForLimits),
                         fabs(loc->el - lr.ray->elForLimits));
        beamWidth = _beamWidthDegV;
      } else if (ul.ray && ur.ray) {
        // data is above
        angleError = MIN(fabs(loc->el - ul.ray->elForLimits),
                         fabs(loc->el - ur.ray->elForLimits));
        beamWidth = _beamWidthDegV;
      }
      // if angle error exceeds the beam width, cannot process
      // this point
      double angleExtension =
        beamWidth * _params.beam_width_fraction_for_data_limit_extension;
      if (angleError > angleExtension) {
        continue;
      }
    } // if (nAvail == 2)

    // get gate indices, compute weights based on range

    double rangeKm = loc->slantRange;
    double dgate = (rangeKm - _startRangeKm) / _gateSpacingKm;
    int igateInner = (int) floor(dgate);
    int igateOuter = igateInner + 1;
    double wtOuter = dgate - igateInner;
    double wtInner = 1.0 - wtOuter;
    Neighbors wts;

    if (nAvail == 2) {
      
      // if we only have 2 valid rays, use an inverse distance interp

      _loadWtsFor2ValidRays(loc, ll, ul, lr, ur, wtInner, wtOuter, wts);
      
    } else {

      if (nAvail == 3) {
        // fill in az and el for missing corner
        if (!ll.ray) {
          ll.interpEl = lr.interpEl;
          ll.interpAz = ul.interpAz;
        } else if (!ul.ray) {
          ul.interpEl = ur.interpEl;
          ul.interpAz = ll.interpAz;
        } else if (!lr.ray) {
          lr.interpEl = ll.interpEl;
          lr.interpAz = ur.interpAz;
        } else if (!ur.ray) {
          ur.interpEl = ul.interpEl;
          ur.interpAz = lr.interpAz;
        }
      } // if (nAvail == 3)

      _loadWtsFor3Or4ValidRays(loc, ll, ul, lr, ur, wtInner, wtOuter, wts);
      
    } // if (nAvail == 2) 

    // normalize weights

    double sumWt = 0.0;
    sumWt += wts.ll_inner;
    sumWt += wts.ll_outer;
    sumWt += wts.ul_inner;
    sumWt += wts.ul_outer;
    sumWt += wts.lr_inner;
    sumWt += wts.lr_outer;
    sumWt += wts.ur_inner;
    sumWt += wts.ur_outer;
    if (sumWt == 0) {
      sumWt = 1.0;
    }

    wts.ll_inner /= sumWt;
    wts.ll_outer /= sumWt;
    wts.ul_inner /= sumWt;
    wts.ul_outer /= sumWt;
    wts.lr_inner /= sumWt;
    wts.lr_outer /= sumWt;
    wts.ur_inner /= sumWt;
    wts.ur_outer /= sumWt;

    // interpolate fields

    int maxContrib = 0;
    for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {

      int nContrib = 0;
      if (_interpFields[ifield].isDiscrete || _params.use_nearest_neighbor) {
        nContrib = _loadNearestGridPt(ifield, ptIndex, igateInner, igateOuter,
                                      ll, ul, lr, ur, wts);
      } else if (_interpFields[ifield].fieldFolds) {
        nContrib = _loadFoldedGridPt(ifield, ptIndex, igateInner, igateOuter,
                                     ll, ul, lr, ur, wts);
      } else {
        nContrib = _loadInterpGridPt(ifield, ptIndex, igateInner, igateOuter,
                                     ll, ul, lr, ur, wts);
      }
      if (nContrib > maxContrib) {
        maxContrib = nContrib;
      }

    } // ifield

    if (_nContribDebug) {
      _nContribDebug->data[ptIndex] = maxContrib;
    }

  } // ix

}

////////////////////////////////////////////
// load up weights for case where we only
// have 2 valid rays
  
void CartInterp::_loadWtsFor2ValidRays(const GridLoc *loc,
                                       const SearchPoint &ll,
                                       const SearchPoint &ul,
                                       const SearchPoint &lr,
                                       const SearchPoint &ur,
                                       double wtInner,
                                       double wtOuter, 
                                       Neighbors &wts)

{
  
  double az = _conditionAz(loc->az);

  // compute 'distance' in el/az space from ray to grid location
  // compute weights based on inverse of
  // distances from grid pt to surrounding rays multiplied
  // by weight for range
  
  if (ll.ray) {
    double dist_ll = _angDist(loc->el - ll.rayEl, az - ll.rayAz);
    double wtDist = 1.0 / dist_ll;
    wts.ll_inner = wtDist * wtInner;
    wts.ll_outer = wtDist * wtOuter;
  } else {
    wts.ll_inner = 0.0;
    wts.ll_outer = 0.0;
  }
  
  if (ul.ray) {
    double dist_ul = _angDist(loc->el - ul.rayEl, az - ul.rayAz);
    double wtDist = 1.0 / dist_ul;
    wts.ul_inner = wtDist * wtInner;
    wts.ul_outer = wtDist * wtOuter;
  } else {
    wts.ul_inner = 0.0;
    wts.ul_outer = 0.0;
  }
  
  if (lr.ray) {
    double dist_lr = _angDist(loc->el - lr.rayEl, az - lr.rayAz);
    double wtDist = 1.0 / dist_lr;
    wts.lr_inner = wtDist * wtInner;
    wts.lr_outer = wtDist * wtOuter;
  } else {
    wts.lr_inner = 0.0;
    wts.lr_outer = 0.0;
  }
  
  if (ur.ray) {
    double dist_ur = _angDist(loc->el - ur.rayEl, az - ur.rayAz);
    double wtDist = 1.0 / dist_ur;
    wts.ur_inner = wtDist * wtInner;
    wts.ur_outer = wtDist * wtOuter;
  } else {
    wts.ur_inner = 0.0;
    wts.ur_outer = 0.0;
  }

}

////////////////////////////////////////////
// load up weights for 3 or 4 valid rays
  
void CartInterp::_loadWtsFor3Or4ValidRays(const GridLoc *loc,
                                          const SearchPoint &ll,
                                          const SearchPoint &ul,
                                          const SearchPoint &lr,
                                          const SearchPoint &ur,
                                          double wtInner,
                                          double wtOuter, 
                                          Neighbors &wts)

{

  double az = _conditionAz(loc->az);

  // compute wts for interpolating based on azimuth lower

  double dazLower = lr.interpAz - ll.interpAz;
  double wtAzLr = 0.5;
  if (dazLower != 0.0) {
    wtAzLr = (az - ll.interpAz) / dazLower;
  }
  double wtAzLl = 1.0 - wtAzLr;
  double elLowerInterp = ll.interpEl * wtAzLl + lr.interpEl * wtAzLr;
  
  // compute wts for interpolating based on azimuth upper

  double dazUpper = ur.interpAz - ul.interpAz;
  double wtAzUr = 0.5;
  if (dazUpper != 0.0) {
    wtAzUr = (az - ul.interpAz) / dazUpper;
  }
  double wtAzUl = 1.0 - wtAzUr;
  double elUpperInterp = ul.interpEl * wtAzUl + ur.interpEl * wtAzUr;

  // compute wts for interpolating based on interpolated elevation

  double dEl = elUpperInterp - elLowerInterp;
  double wtElUpper = 0.5;
  if (dEl != 0) {
    wtElUpper = (loc->el - elLowerInterp) / dEl;
  }
  double wtElLower = 1.0 - wtElUpper;

  // compute final wts as product of these weights

  if (ll.ray) {
    double wtAng = wtAzLl * wtElLower;
    wts.ll_inner = wtAng * wtInner;
    wts.ll_outer = wtAng * wtOuter;
  } else {
    wts.ll_inner = 0.0;
    wts.ll_outer = 0.0;
  }
  
  if (ul.ray) {
    double wtAng = wtAzUl * wtElUpper;
    wts.ul_inner = wtAng * wtInner;
    wts.ul_outer = wtAng * wtOuter;
  } else {
    wts.ul_inner = 0.0;
    wts.ul_outer = 0.0;
  }
  
  if (lr.ray) {
    double wtAng = wtAzLr * wtElLower;
    wts.lr_inner = wtAng * wtInner;
    wts.lr_outer = wtAng * wtOuter;
  } else {
    wts.lr_inner = 0.0;
    wts.lr_outer = 0.0;
  }
  
  if (ur.ray) {
    double wtAng = wtAzUr * wtElUpper;
    wts.ur_inner = wtAng * wtInner;
    wts.ur_outer = wtAng * wtOuter;
  } else {
    wts.ur_inner = 0.0;
    wts.ur_outer = 0.0;
  }

}
    
////////////////////////////////////////////
// load up grid point using nearest neighbor
// returns the number of points contributing
  
int CartInterp::_loadNearestGridPt(int ifield,
                                   int ptIndex,
                                   int igateInner,
                                   int igateOuter,
                                   const SearchPoint &ll,
                                   const SearchPoint &ul,
                                   const SearchPoint &lr,
                                   const SearchPoint &ur,
                                   const Neighbors &wts)
  
{

  // find value with highest weight - that will be closest
        
  double maxWt = 0.0;
  double closestVal = 0.0;
  int nContrib = 0;
  
  if (ll.ray) {
    _accumNearest(ll.ray, ifield, igateInner, igateOuter,
                  wts.ll_inner, wts.ll_outer, closestVal, maxWt, nContrib);
  }
  
  if (ul.ray) {
    _accumNearest(ul.ray, ifield, igateInner, igateOuter,
                  wts.ul_inner, wts.ul_outer, closestVal, maxWt, nContrib);
  }
  
  if (lr.ray) {
    _accumNearest(lr.ray, ifield, igateInner, igateOuter,
                  wts.lr_inner, wts.lr_outer, closestVal, maxWt, nContrib);
  }
  
  if (ur.ray) {
    _accumNearest(ur.ray, ifield, igateInner, igateOuter,
                  wts.ur_inner, wts.ur_outer, closestVal, maxWt, nContrib);
  }
  
  // compute weighted mean
  
  if (nContrib >= _params.min_nvalid_for_interp) {
    _outputFields[ifield][ptIndex] = closestVal;
  } else {
    _outputFields[ifield][ptIndex] = missingFl32;
  }

  return nContrib;
  
}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// returns the number of points contributing
  
int CartInterp::_loadInterpGridPt(int ifield,
                                  int ptIndex,
                                  int igateInner,
                                  int igateOuter,
                                  const SearchPoint &ll,
                                  const SearchPoint &ul,
                                  const SearchPoint &lr,
                                  const SearchPoint &ur,
                                  const Neighbors &wts)

{

  // sum up weighted vals
  
  double sumVals = 0.0;
  double sumWts = 0.0;
  int nContrib = 0;
  
  if (ll.ray) {
    _accumInterp(ll.ray, ifield, igateInner, igateOuter,
                 wts.ll_inner, wts.ll_outer, sumVals, sumWts, nContrib);
  }

  if (ul.ray) {
    _accumInterp(ul.ray, ifield, igateInner, igateOuter,
                 wts.ul_inner, wts.ul_outer, sumVals, sumWts, nContrib);
  }
  
  if (lr.ray) {
    _accumInterp(lr.ray, ifield, igateInner, igateOuter,
                 wts.lr_inner, wts.lr_outer, sumVals, sumWts, nContrib);
  }
  
  if (ur.ray) {
    _accumInterp(ur.ray, ifield, igateInner, igateOuter,
                 wts.ur_inner, wts.ur_outer, sumVals, sumWts, nContrib);
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

  return nContrib;

}

/////////////////////////////////////////////////////
// load up data for a grid point using interpolation
// for a folded field
// returns the number of points contributing
  
int CartInterp::_loadFoldedGridPt(int ifield,
                                  int ptIndex,
                                  int igateInner,
                                  int igateOuter,
                                  const SearchPoint &ll,
                                  const SearchPoint &ul,
                                  const SearchPoint &lr,
                                  const SearchPoint &ur,
                                  const Neighbors &wts)
  
{

  // sum up weighted vals
  
  double sumX = 0.0;
  double sumY = 0.0;
  double sumWts = 0.0;
  int nContrib = 0;
  
  if (ll.ray) {
    _accumFolded(ll.ray, ifield, igateInner, igateOuter,
                 wts.ll_inner, wts.ll_outer, sumX, sumY, sumWts, nContrib);
  }
  
  
  if (ul.ray) {
    _accumFolded(ul.ray, ifield, igateInner, igateOuter,
                 wts.ul_inner, wts.ul_outer, sumX, sumY, sumWts, nContrib);
  }
  
  if (lr.ray) {
    _accumFolded(lr.ray, ifield, igateInner, igateOuter,
                 wts.lr_inner, wts.lr_outer, sumX, sumY, sumWts, nContrib);
  }
  
  if (ur.ray) {
    _accumFolded(ur.ray, ifield, igateInner, igateOuter,
                 wts.ur_inner, wts.ur_outer, sumX, sumY, sumWts, nContrib);
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

  return nContrib;

}

/////////////////////////////////////////////////////
// condition the azimuth, if we are in sector that
// spans the north line

double CartInterp::_conditionAz(double az)
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

int CartInterp::_writeOutputFile()
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

  // debug (test) fields

  for (size_t ii = 0; ii < _derived3DFields.size(); ii++) {
    const DerivedField *dfld = _derived3DFields[ii];
    if (dfld->writeToFile) {
      out.addField(_readVol, _proj, dfld->vertLevels,
                   dfld->name, dfld->longName, dfld->units,
                   Radx::FL32, 1.0, 0.0, missingFl32, dfld->data);
    }
  }

  for (size_t ii = 0; ii < _derived2DFields.size(); ii++) {
    const DerivedField *dfld = _derived2DFields[ii];
    if (dfld->writeToFile) {
      out.addField(_readVol, _proj, dfld->vertLevels,
                   dfld->name, dfld->longName, dfld->units,
                   Radx::FL32, 1.0, 0.0, missingFl32, dfld->data);
    }
  }

  // convective stratiform split

  if (_params.identify_convective_stratiform_split && _gotConvStrat) {
    out.addConvStratFields(_convStrat, _readVol,
                           _proj, _gridZLevels);
  }

  // chunks

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

/////////////////////////////////////////////////////
// write out search matrix data

int CartInterp::_writeSearchMatrices()
{

  if (_params.debug) {
    cerr << "  Writing search matrix debug files ... " << endl;
  }

  // create MDVX object

  DsMdvx mdvx;
  
  // set master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);
  
  mhdr.index_number = _readVol.getVolumeNumber();
  mhdr.time_gen = time(NULL);
  mhdr.time_begin = _readVol.getStartTimeSecs();
  mhdr.time_end = _readVol.getEndTimeSecs();
  mhdr.time_centroid = _readVol.getEndTimeSecs();
  mhdr.time_expire = mhdr.time_centroid;
  
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = _readVol.getLongitudeDeg();
  mhdr.sensor_lat = _readVol.getLatitudeDeg();
  mhdr.sensor_alt = _readVol.getAltitudeKm();
  
  mdvx.setMasterHeader(mhdr);
  
  mdvx.setDataSetInfo(_readVol.getHistory().c_str());
  mdvx.setDataSetName("Radx2Grid search matrices");
  mdvx.setDataSetSource(_readVol.getSource().c_str());

  // load up field data

  int npts = _searchNEl * _searchNAz;
  fl32 *llEl = new fl32[npts];
  fl32 *llAz = new fl32[npts];
  fl32 *llSwpNum = new fl32[npts];
  fl32 *lrEl = new fl32[npts];
  fl32 *lrAz = new fl32[npts];
  fl32 *lrSwpNum = new fl32[npts];
  fl32 *ulEl = new fl32[npts];
  fl32 *ulAz = new fl32[npts];
  fl32 *ulSwpNum = new fl32[npts];
  fl32 *urEl = new fl32[npts];
  fl32 *urAz = new fl32[npts];
  fl32 *urSwpNum = new fl32[npts];
  fl32 missing = -9999.0;
  
  int count = 0;
  for (int iel = 0; iel < _searchNEl; iel++) {
    for (int iaz = 0; iaz < _searchNAz; iaz++, count++) {
      if (_searchMatrixLowerLeft[iel][iaz].ray) {
        llEl[count] = _searchMatrixLowerLeft[iel][iaz].ray->el;
        llAz[count] = _searchMatrixLowerLeft[iel][iaz].ray->az / 10.0;
        llSwpNum[count] = _searchMatrixLowerLeft[iel][iaz].ray->sweepIndex;
      } else {
        llEl[count] = missing;
        llAz[count] = missing;
        llSwpNum[count] = missing;
      }
      if (_searchMatrixLowerRight[iel][iaz].ray) {
        lrEl[count] = _searchMatrixLowerRight[iel][iaz].ray->el;
        lrAz[count] = _searchMatrixLowerRight[iel][iaz].ray->az / 10.0;
        lrSwpNum[count] = _searchMatrixLowerRight[iel][iaz].ray->sweepIndex;
      } else {
        lrEl[count] = missing;
        lrAz[count] = missing;
        lrSwpNum[count] = missing;
      }
      if (_searchMatrixUpperLeft[iel][iaz].ray) {
        ulEl[count] = _searchMatrixUpperLeft[iel][iaz].ray->el;
        ulAz[count] = _searchMatrixUpperLeft[iel][iaz].ray->az / 10.0;
        ulSwpNum[count] = _searchMatrixUpperLeft[iel][iaz].ray->sweepIndex;
      } else {
        ulEl[count] = missing;
        ulAz[count] = missing;
        ulSwpNum[count] = missing;
      }
      if (_searchMatrixUpperRight[iel][iaz].ray) {
        urEl[count] = _searchMatrixUpperRight[iel][iaz].ray->el;
        urAz[count] = _searchMatrixUpperRight[iel][iaz].ray->az / 10.0;
        urSwpNum[count] = _searchMatrixUpperRight[iel][iaz].ray->sweepIndex;
      } else {
        urEl[count] = missing;
        urAz[count] = missing;
        urSwpNum[count] = missing;
      }
    }
  }

  // add the fields

  _addMatrixField(mdvx, llEl, missing, "LL_El", "LowerLeftElevation", "deg");
  _addMatrixField(mdvx, llAz, missing, "LL_Az", "LowerLeftAzimuth", "deg");
  _addMatrixField(mdvx, llSwpNum, missing, "LL_SwpNum", "LowerLeftSweepNum", "");
  
  _addMatrixField(mdvx, lrEl, missing, "LR_El", "LowerRightElevation", "deg");
  _addMatrixField(mdvx, lrAz, missing, "LR_Az", "LowerRightAzimuth", "deg");
  _addMatrixField(mdvx, lrSwpNum, missing, "LR_SwpNum", "LowerRightSweepNum", "");
  
  _addMatrixField(mdvx, ulEl, missing, "UL_El", "UpperLeftElevation", "deg");
  _addMatrixField(mdvx, ulAz, missing, "UL_Az", "UpperLeftAzimuth", "deg");
  _addMatrixField(mdvx, ulSwpNum, missing, "UL_SwpNum", "UpperLeftSweepNum", "");
  
  _addMatrixField(mdvx, urEl, missing, "UR_El", "UpperRightElevation", "deg");
  _addMatrixField(mdvx, urAz, missing, "UR_Az", "UpperRightAzimuth", "deg");
  _addMatrixField(mdvx, urSwpNum, missing, "UR_SwpNum", "UpperRightSweepNum", "");
  
  // free up arrays

  delete[] llEl;
  delete[] llAz;
  delete[] llSwpNum;
  delete[] lrEl;
  delete[] lrAz;
  delete[] lrSwpNum;
  delete[] ulEl;
  delete[] ulAz;
  delete[] ulSwpNum;
  delete[] urEl;
  delete[] urAz;
  delete[] urSwpNum;
  
  // write the file

  if (mdvx.writeToDir(_params.search_matrix_dir)) {
    cerr << "ERROR - CartInterp::_writeSearchMatrices" << endl;
    cerr << "  Cannot write matrix file to dir: "
         << _params.search_matrix_dir << endl;
    return -1;
  }

  if (_params.debug) {
    cerr << "  Wrote search matrix debug file: " << mdvx.getPathInUse() << endl;
  }
  
  return 0;

}

////////////////////////////////////////////////
// add a field to the search matrix mdv object

void CartInterp::_addMatrixField(DsMdvx &mdvx,
                                 const fl32 *data,
                                 fl32 missing,
                                 const string &name,
                                 const string &longName,
                                 const string &units)

{

  int npts = _searchNEl * _searchNAz;
  int nz = 1;

  // field header

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  
  fhdr.nx = _searchNAz;
  fhdr.ny = _searchNEl;
  fhdr.nz = nz;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_ELEV;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  
  fhdr.proj_type = Mdvx::PROJ_LATLON;
  fhdr.dz_constant = true;

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = 4;
  fhdr.volume_size = npts * nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;
  
  fhdr.proj_origin_lat = 0.0;
  fhdr.proj_origin_lon = 0.0;
  fhdr.user_data_fl32[0] = 0.0;
  
  fhdr.grid_dx = _searchResAz / 10.0;
  fhdr.grid_dy = _searchResEl;
  fhdr.grid_dz = 1.0;
  
  fhdr.grid_minx = _searchMinAz / 10.0;
  fhdr.grid_miny = _searchMinEl;
  fhdr.grid_minz = 0.0;

  fhdr.scale = 1.0;
  fhdr.bias = 0.0;
  fhdr.bad_data_value = missing;
  fhdr.missing_data_value = missing;
  
  // vlevel header
  
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);
  int nLevels = nz;
  for (int i = 0; i < nLevels; i++) {
    vhdr.level[i] = fhdr.grid_minz;
    vhdr.type[i] = Mdvx::VERT_TYPE_SURFACE;
  }
  
  // create field

  MdvxField *fld = new MdvxField(fhdr, vhdr, data);
  fld->convertType(Mdvx::ENCODING_INT16,
                   Mdvx::COMPRESSION_GZIP,
                   Mdvx::SCALING_DYNAMIC);

  // set strings
  
  fld->setFieldName(name.c_str());
  fld->setFieldNameLong(longName.c_str());
  fld->setUnits(units.c_str());
  fld->setTransform("");
  
  // add to object
  
  mdvx.addField(fld);

}

#ifdef JUNK

//////////////////////////////////////////////////
// Prepare for convective/stratiform analysis

void CartInterp::_convStratPrepare()
{

  // initialize beamHeight computations
  
  BeamHeight beamHt;
  if (_params.override_standard_pseudo_earth_radius) {
    beamHt.setPseudoRadiusRatio(_params.pseudo_earth_radius_ratio);
  }
  beamHt.setInstrumentHtKm(_radarAltKm);

  // loop through rays
  
  for (size_t iray = 0; iray < _interpRays.size(); iray++) {
    
    const Ray *ray = _interpRays[iray];

    // get dbz field
    
    RadxRay *radxRay = ray->inputRay;
    RadxField *dbzField = radxRay->getField(_params.conv_strat_dbz_field_name);
    if (dbzField == NULL) {
      continue;
    }
    const Radx::fl32 *dbzArray = dbzField->getDataFl32();
    Radx::fl32 missing = dbzField->getMissingFl32();

    // initialize meta data
    
    double el = ray->el;
    double az = ray->az;
    
    double sinAz, cosAz;
    ta_sincos(az * DEG_TO_RAD, &sinAz, &cosAz);
    
    int nGates = ray->nGates;
    double range = _startRangeKm;

    // loop through the gates

    for (int igate = 0; igate < nGates; igate++, range += _gateSpacingKm) {

      // check for good dbz data
      
      Radx::fl32 dbz = dbzArray[igate];
      if (dbz == missing ||
          dbz < _params.conv_strat_min_valid_dbz) {
        continue;
      }
      double dbzSq = dbz * dbz;
      double dbzSqSq = dbzSq * dbzSq;

      // compute (zz, yy, xx) location for each gate
    
      double xx = _radarX + range * sinAz;
      double yy = _radarY + range * cosAz;
      double zz = beamHt.computeHtKm(el, range);

      // compute grid indices

      int ix = (int) floor((xx - _gridMinx) / _gridDx + 0.5);
      if (ix < 0 || ix > _gridNx - 1) {
        continue;
      }

      int iy = (int) floor((yy - _gridMiny) / _gridDy + 0.5);
      if (iy < 0 || iy > _gridNy - 1) {
        continue;
      }

      int iLookup = (int) (zz * 100.0 + 0.5);
      if (iLookup >= 0 && iLookup < NLOOKUP) {
        for (int iz = _textureIzLower[iLookup];
             iz <= _textureIzUpper[iLookup]; iz++) {
          
          int index = iz * _nPointsPlane + iy * _gridNx + ix;
          
          fl32 dbzMax = _convStratDbzMax->data[index];
          if (dbzMax == missingFl32 || dbzMax < dbz) {
            _convStratDbzMax->data[index] = dbz;
          }
          _convStratDbzCount->data[index] += 1.0;
          _convStratDbzSum->data[index] += dbz;
          _convStratDbzSqSum->data[index] += dbzSq;
          _convStratDbzSqSqSum->data[index] += dbzSqSq;
        }
      }

    } // igate
    
  } // iray
  
}

#endif

//////////////////////////////////////////////////
// Compute convective/stratiform split

int CartInterp::_convStratCompute()
{

  // set the grid in the ConvStrat object

  bool isLatLon = (_params.grid_projection == Params::PROJ_LATLON);
  _convStrat.setGrid(_gridNx, _gridNy,
                     _gridDx, _gridDy,
                     _gridMinx, _gridMiny,
                     _gridZLevels,
                     isLatLon);

  // get the dbz field for ConvStrat

  string dbzName(_params.conv_strat_dbz_field_name);
  fl32 *dbzVals = NULL;
  for (size_t ifield = 0; ifield < _interpFields.size(); ifield++) {
    const Field &ifld = _interpFields[ifield];
    if (ifld.radxName == dbzName) {
      dbzVals = _outputFields[ifield];
      break;
    }
  }
  if (dbzVals == NULL) {
    cerr << "ERROR - CartInterp::_convStratCompute()" << endl;
    cerr << "  Cannot find dbz field: " << dbzName << endl;
    cerr << "  conv/strat partition will not be computed" << endl;
    return -1;
  }

  // compute the convective/stratiform partition
  
  if (_convStrat.computePartition(dbzVals, missingFl32)) {
    cerr << "ERROR - CartInterp::_convStratCompute()" << endl;
    cerr << "  _convStrat.computePartition() failed" << endl;
    return -1;
  }

#ifdef JUNK
  
  // prepare the data

  _convStratComputeVertLookups();
  _convStratPrepare();

  // compute kernels

  _convStratComputeKernels();
  
  // set array pointers

  fl32 *dbzCount = _convStratDbzCount->data;
  fl32 *dbzSum = _convStratDbzSum->data;
  fl32 *dbzSqSum = _convStratDbzSqSum->data;
  fl32 *dbzSqSqSum = _convStratDbzSqSqSum->data;
  fl32 *texture = _convStratDbzTexture->data;
  fl32 *sqTexture = _convStratDbzSqTexture->data;
  fl32 *filledTexture = _convStratFilledTexture->data;
  fl32 *filledSqTexture = _convStratFilledSqTexture->data;
  
  // initialize
  
  memset(dbzCount, 0, _nPointsPlane * sizeof(fl32));
  memset(dbzSum, 0, _nPointsPlane * sizeof(fl32));
  memset(dbzSqSum, 0, _nPointsPlane * sizeof(fl32));
  memset(dbzSqSqSum, 0, _nPointsPlane * sizeof(fl32));
  
  for (int ii = 0; ii < _nPointsVol; ii++) {
    texture[ii] = missingFl32;
    sqTexture[ii] = missingFl32;
    filledTexture[ii] = missingFl32;
    filledSqTexture[ii] = missingFl32;
  }
  
  // set up threads for computing texture at each level
  
  vector<ComputeTexture *> threads;
  for (int iz = 0; iz <= _gridNz; iz++) {
    size_t zoffset = iz * _nPointsPlane;
    ComputeTexture *thread = new ComputeTexture(iz);
    thread->setGridSize(_gridNx, _gridNy);
    thread->setKernelSize(_nxTexture, _nyTexture);
    thread->setKernel(_textureKernel);
    thread->setMinValidFraction(_params.conv_strat_min_valid_fraction_for_texture);
    thread->setFields(dbzCount + zoffset,
                      dbzSum + zoffset,
                      dbzSqSum + zoffset,
                      dbzSqSqSum + zoffset,
                      texture + zoffset,
                      sqTexture + zoffset,
                      filledTexture + zoffset,
                      filledSqTexture + zoffset);
    threads.push_back(thread);
  } // iz

  // set threads going

  for (size_t ii = 0; ii < threads.size(); ii++) {
    threads[ii]->signalRunToStart();
  }

  // wait until they are done

  for (size_t ii = 0; ii < threads.size(); ii++) {
    threads[ii]->waitForRunToComplete();
  }

  // delete threads

  for (size_t ii = 0; ii < threads.size(); ii++) {
    delete threads[ii];
  }
  threads.clear();
  
  // compute min and max vert indices

  int minIz = 0;
  int maxIz = _gridNz - 1;
  for (int iz = 0; iz < _gridNz; iz++) {
    double zz = _gridZLevels[iz];
    if (zz <= _params.conv_strat_min_valid_height) {
      minIz = iz;
    }
    if (zz <= _params.conv_strat_max_valid_height) {
      maxIz = iz;
    }
  } // iz

  // compute mean texture at each point, and the col max
  
  fl32 *dbzMax = _convStratDbzMax->data;
  fl32 *dbzColMax = _convStratDbzColMax->data;
  fl32 *meanTexture = _convStratMeanTexture->data;
  fl32 *meanSqTexture = _convStratMeanSqTexture->data;
  
  int jj = 0;
  for (int iy = 0; iy < _gridNy; iy++) {
    for (int ix = 0; ix < _gridNx; ix++, jj++) {
      int ii = jj + minIz * _nPointsPlane;
      double nn = 0.0;
      double sumTexture = 0.0;
      double sumSqTexture = 0.0;
      double colMax = missingFl32;
      for (int iz = minIz; iz <= maxIz; iz++, ii += _nPointsPlane) {
        fl32 text = texture[ii];
        fl32 sqText = sqTexture[ii];
        if (text != missingFl32) {
          sumTexture += text;
          sumSqTexture += sqText;
          nn++;
        }
        if (dbzMax[ii] > colMax) {
          colMax = dbzMax[ii];
        }
      } // iz
      if (nn > 0) {
        meanTexture[jj] = sumTexture / nn;
        meanSqTexture[jj] = sumSqTexture / nn;
      }
      dbzColMax[jj] = colMax;
    } // ix
  } // iy

#ifdef NOTNOW
  // fill in col max gaps using nearest neighbor

  fl32 *tmpColMax = new fl32[_nPointsPlane];
  memcpy(tmpColMax, dbzColMax, _nPointsPlane * sizeof(fl32));
  for (int iy = _nyTexture; iy < _gridNy - _nyTexture; iy++) {
    int icenter = _nxTexture + iy * _gridNx;
    for (int ix = _nxTexture; ix < _gridNx - _nxTexture; ix++, icenter++) {
      if (dbzColMax[icenter] != missingFl32) {
        continue;
      }
      for (size_t ii = 1; ii < _textureKernel.size(); ii++) {
        int kk = icenter + _textureKernel[ii].offset;
        if (dbzColMax[kk] != missingFl32) {
          tmpColMax[icenter] = dbzColMax[kk];
          break;
        }
      }
    } // ix
  } // iy
  memcpy(dbzColMax, tmpColMax, _nPointsPlane * sizeof(fl32));
  delete[] tmpColMax;
#endif
  
  // compute the category
  
  fl32 *category = _convStratCategory->data;

  for (int iy = _nyConv; iy < _gridNy - _nyConv; iy++) {
    for (int ix = _nxConv; ix < _gridNx - _nxConv; ix++) {
      
      int ii = ix + iy * _gridNx;

      // set convective flag from texture

      if (meanTexture[ii] >= _params.conv_strat_min_texture_for_convection) {
        for (size_t kk = 0; kk < _convKernel.size(); kk++) {
          int jj = ii + _convKernel[kk].offset;
          category[jj] = CATEGORY_CONVECTIVE;
        }
      }
      
      // set convective flag from col max dbz

      if (dbzColMax[ii] >= _params.conv_strat_dbz_threshold_for_definite_convection) {
        for (size_t kk = 0; kk < _convKernel.size(); kk++) {
          int jj = ii + _convKernel[kk].offset;
          category[jj] = CATEGORY_CONVECTIVE;
        }
      }
      
    } // ix
  } // iy

  // set stratiform flag for what is left
  
  for (int iy = _nyConv; iy < _gridNy - _nyConv; iy++) {
    for (int ix = _nxConv; ix < _gridNx - _nxConv; ix++) {
      int ii = ix + iy * _gridNx;
      // if (dbzColMax[ii] < _params.conv_strat_min_valid_dbz) {
      //   category[ii] = missingFl32;
      // }
      if (category[ii] == missingFl32 && meanTexture[ii] != missingFl32) {
        category[ii] = CATEGORY_STRATIFORM;
      }
    }
  }

#endif

  _printRunTime("CartInterp::_convStratCompute");

  return 0;
  
}

#ifdef JUNK

//////////////////////////////////////
// compute the kernels for this grid

bool CartInterp::_compareKernels(kernel_t x, kernel_t y) 
{
  return x.distance < y.distance; 
}

void CartInterp::_convStratComputeKernels()

{

  _convectiveRadiusKm = _params.conv_strat_convective_radius_km;
  _textureRadiusKm = _params.conv_strat_texture_radius_km;

  // texture kernel

  _textureKernel.clear();

  _nyTexture = (int) floor(_textureRadiusKm / _gridDy + 0.5);
  _nxTexture = (int) floor(_textureRadiusKm / _gridDx + 0.5);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Texture kernel size:" << endl;
    cerr << "  idy: " << _nyTexture << endl;
    cerr << "  nx: " << _nxTexture << endl;
  }
  
  kernel_t entry;
  for (int jdy = -_nyTexture; jdy <= _nyTexture; jdy++) {
    double yy = jdy * _gridDy;
    for (int jdx = -_nxTexture; jdx <= _nxTexture; jdx++) {
      double xx = jdx * _gridDx;
      entry.distance = sqrt(yy * yy + xx * xx);
      if (entry.distance <= _textureRadiusKm) {
        entry.offset = jdx + jdy * _gridNx;
        _textureKernel.push_back(entry);
      }
    }
  }

  // sort

  sort(_textureKernel.begin(), _textureKernel.end(), _compareKernels);

  // convective kernel

  _convKernel.clear();

  _nyConv = (int) floor(_convectiveRadiusKm / _gridDy + 0.5);
  _nxConv = (int) floor(_convectiveRadiusKm / _gridDx + 0.5);

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "Convective kernel size:" << endl;
    cerr << "  idy: " << _nyConv << endl;
    cerr << "  nx: " << _nxConv << endl;
  }
  
  for (int jdy = -_nyConv; jdy <= _nyConv; jdy++) {
    double yy = jdy * _gridDy;
    for (int jdx = -_nxConv; jdx <= _nxConv; jdx++) {
      double xx = jdx * _gridDx;
      entry.distance = sqrt(yy * yy + xx * xx);
      if (entry.distance <= _convectiveRadiusKm) {
        entry.offset = jdx + jdy * _gridNx;
        _convKernel.push_back(entry);
      }
    }
  }

  // sort

  sort(_convKernel.begin(), _convKernel.end(), _compareKernels);

}

/////////////////////////////////////////////
// compute the vertical level lookup tables

void CartInterp::_convStratComputeVertLookups()

{

  for (int ii = 0; ii < NLOOKUP; ii++) {
    _textureIzLower[ii] = _gridNz;
    _textureIzUpper[ii] = 0;
  }

  double conv_strat_texture_depth_km = 1.0;

  for (int ii = 0; ii < NLOOKUP; ii++) {
    
    double zz = ii * 0.01; // ht in km
    double zLowerLimit = zz - conv_strat_texture_depth_km / 2.0;
    double zUpperLimit = zz + conv_strat_texture_depth_km / 2.0;
    
    for (int jj = 0; jj < (int) _gridZLevels.size(); jj++) {
      double zLevel = _gridZLevels[jj];
      if (zLevel >= zLowerLimit) {
        _textureIzLower[ii] = jj;
        break;
      }
    }

    for (int jj = (int) _gridZLevels.size() - 1; jj >= 0; jj--) {
      double zLevel = _gridZLevels[jj];
      if (zLevel <= zUpperLimit) {
        _textureIzUpper[ii] = jj;
        break;
      }
    }

  }

}

#endif

///////////////////////////////////////////////////////////////
// FillSearchLowerLeft thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::FillSearchLowerLeft::FillSearchLowerLeft(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::FillSearchLowerLeft::run()
{
  vector<SearchIndex> thisSearch, nextSearch;
  for (int level = 0; level < _this->_searchMaxCount; level++) {
    if (_this->_fillSearchLowerLeft(level, thisSearch, nextSearch) == 0) {
      break;
    }
    thisSearch = nextSearch;
  }
}

///////////////////////////////////////////////////////////////
// FillSearchLowerRight thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::FillSearchLowerRight::FillSearchLowerRight(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::FillSearchLowerRight::run()
{
  vector<SearchIndex> thisSearch, nextSearch;
  for (int level = 0; level < _this->_searchMaxCount; level++) {
    if (_this->_fillSearchLowerRight(level, thisSearch, nextSearch) == 0) {
      break;
    }
    thisSearch = nextSearch;
  }
}

///////////////////////////////////////////////////////////////
// FillSearchUpperLeft thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::FillSearchUpperLeft::FillSearchUpperLeft(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::FillSearchUpperLeft::run()
{
  vector<SearchIndex> thisSearch, nextSearch;
  for (int level = 0; level < _this->_searchMaxCount; level++) {
    if (_this->_fillSearchUpperLeft(level, thisSearch, nextSearch) == 0) {
      break;
    }
    thisSearch = nextSearch;
  }
}

///////////////////////////////////////////////////////////////
// FillSearchUpperRight thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::FillSearchUpperRight::FillSearchUpperRight(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::FillSearchUpperRight::run()
{
  vector<SearchIndex> thisSearch, nextSearch;
  for (int level = 0; level < _this->_searchMaxCount; level++) {
    if (_this->_fillSearchUpperRight(level, thisSearch, nextSearch) == 0) {
      break;
    }
    thisSearch = nextSearch;
  }
}

///////////////////////////////////////////////////////////////
// ComputeGridRelative thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::ComputeGridRelative::ComputeGridRelative(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::ComputeGridRelative::run()
{
  _this->_computeGridRow(_zIndex, _yIndex);
}

///////////////////////////////////////////////////////////////
// PerformInterp thread
///////////////////////////////////////////////////////////////
// Constructor
CartInterp::PerformInterp::PerformInterp(CartInterp *obj) :
        _this(obj)
{
}  
// run method
void CartInterp::PerformInterp::run()
{
  _this->_interpRow(_zIndex, _yIndex);
}

#ifdef JUNK

///////////////////////////////////////////////////////////////
// ComputeTexture thread
//
// Compute texture for 1 level in a thread
//
///////////////////////////////////////////////////////////////

// Constructor

CartInterp::ComputeTexture::ComputeTexture(int iz) :
        TaThread(),
        _iz(iz)
{
  char name[128];
  sprintf(name, "ComputeTexture-level-%d", _iz);
  setThreadName(name);
  _dbzCount = NULL;
  _dbzSum = NULL;
  _dbzSqSum = NULL;
  _dbzSqSqSum = NULL;
  _dbzTexture = NULL;
  _dbzSqTexture = NULL;
  _nx = _ny = 0;
  _nxTexture = _nyTexture = 0;
}  


CartInterp::ComputeTexture::~ComputeTexture() 
{
}

// override run method
// compute texture at each point in plane

void CartInterp::ComputeTexture::run()
{
  
  // check for validity
  
  if (_dbzCount == NULL) {
    cerr << "ERROR - ComputeTexture::run" << endl;
    cerr << "  Initialization not complete" << endl;
    return;
  }
  
  // compute texture at each point in the plane

  int minPtsForTexture = 
    (int) (_minValidFraction * _kernel.size() + 0.5);
  
  for (int iy = _nyTexture; iy < _ny - _nyTexture; iy++) {
    
    int icenter = _nxTexture + iy * _nx;
    
    for (int ix = _nxTexture; ix < _nx - _nxTexture; ix++, icenter++) {
      
      // compute texture in circular kernel around point
      // first we compute the standard deviation of the square of dbz
      // then we take the square root of the sdev
      
      double nn = 0.0;
      double sum = 0.0;
      double sumSq = 0.0;
      double sumSqSq = 0.0;
      
      for (size_t ii = 0; ii < _kernel.size(); ii++) {
        int kk = icenter + _kernel[ii].offset;
        if (_dbzCount[kk] > 0) {
          nn += _dbzCount[kk];
          sum += _dbzSum[kk];
          sumSq += _dbzSqSum[kk];
          sumSqSq += _dbzSqSqSum[kk];
        }
      } // ii
      if (nn >= minPtsForTexture) {

        double mean = sum / nn;
        double var = sumSq / nn - (mean * mean);
        if (var < 0.0) {
          var = 0.0;
        }
        double sdev = sqrt(var);
        _dbzTexture[icenter] = sdev;

        double meanSq = sumSq / nn;
        double varSq = sumSqSq / nn - (meanSq * meanSq);
        if (varSq < 0.0) {
          varSq = 0.0;
        }
        double sdevSq = sqrt(varSq);
        _dbzSqTexture[icenter] = sqrt(sdevSq);

      }
      
    } // ix
    
  } // iy
  
  // fill in gaps in texture using closest non-missing point

  for (int iy = _nyTexture; iy < _ny - _nyTexture; iy++) {
    int icenter = _nxTexture + iy * _nx;
    for (int ix = _nxTexture; ix < _nx - _nxTexture; ix++, icenter++) {
      for (size_t ii = 0; ii < _kernel.size(); ii++) {
        int kk = icenter + _kernel[ii].offset;
        if (_dbzTexture[kk] != missingFl32) {
          _filledTexture[icenter] = _dbzTexture[kk];
          _filledSqTexture[icenter] = _dbzSqTexture[kk];
          break;
        }
      }
    } // ix
  } // iy
  
}

#endif
