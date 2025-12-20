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
// InterpToCart class - derived from Interp.
// Used for full 3-D Cartesian interpolation.
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2021
//
///////////////////////////////////////////////////////////////

#include "InterpToCart.hh"
#include "Orient.hh"
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

const double InterpToCart::_searchResAz = 0.1;
const double InterpToCart::_searchResEl = 0.1;
const double InterpToCart::_searchAzOverlapDeg = 20.0;
const double InterpToCart::_searchAzOverlapHalf = InterpToCart::_searchAzOverlapDeg / 2.0;

// Constructor

InterpToCart::InterpToCart(const string &progName,
                           const Params &params,
                           RadxVol &readVol,
                           vector<Field> &interpFields,
                           vector<Ray *> &interpRays) :
        BaseInterp(progName,
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

  _orient = NULL;
  _echoOrientationAvailable = false;

  // create dual-pol derived fields

  _addDpDerivedFields();
  
  // create debug fields if needed

  if (_params.output_debug_fields) {
    _createDebugFields();
  }
  
  // initialize the output grid dimensions
  
  _initGrid();

  // set up thread objects

  _createThreads();

  // set up ConvStratFinder object

  if (_params.identify_convective_stratiform_split) {
    if (_params.debug >= Params::DEBUG_VERBOSE) {
      _convStrat.setVerbose(true);
    } else if (_params.debug) {
      _convStrat.setDebug(true);
    }
    _convStrat.setMinValidHtKm(_params.conv_strat_min_valid_height);
    _convStrat.setMaxValidHtKm(_params.conv_strat_max_valid_height);
    _convStrat.setMinValidDbz(_params.conv_strat_min_valid_dbz);
    _convStrat.setTextureRadiusKm(_params.conv_strat_texture_radius_km);
    _convStrat.setMinValidFractionForTexture
      (_params.conv_strat_min_valid_fraction_for_texture);
    _convStrat.setMinConvectivityForConvective
      (_params.conv_strat_min_convectivity_for_convective);
    _convStrat.setMaxConvectivityForStratiform
      (_params.conv_strat_max_convectivity_for_stratiform);
    _convStrat.setMinGridOverlapForClumping
      (_params.conv_strat_min_overlap_for_convective_clumps);
  }
  _gotConvStrat = false;

  // set up orientation object

  if (_params.use_echo_orientation) {
    _orient = new Orient(params, 
                         readVol,
                         _gridNx,
                         _gridNy,
                         _gridZLevels);
  }
  
}

//////////////////////////////////////
// destructor

InterpToCart::~InterpToCart()

{

  _freeThreads();
  _freeSearchMatrix();
  _freeGridLoc();
  _freeOutputArrays();
  _freeDerivedFields();
  if (_orient) {
    delete _orient;
  }

}

//////////////////////////////////////////////////
// interpolate a volume
// assumes volume has been read
// and _interpFields and _interpRays vectors are populated
// returns 0 on succes, -1 on failure

int InterpToCart::interpVol()

{

  _printRunTime("Cart interp - reading data");

  if (_gridLoc == NULL) {
    _initGrid();
  }

  // set radar params from volume

  if (_setRadarParams()) {
    cerr << "ERROR - InterpToCart::interpVol()" << endl;
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

  // determine echo orientation
  // for now this only works in PPI mode

  if (_orient && !_rhiMode) {
    _orient->setRhiMode(_rhiMode);
    if (_orient->findEchoOrientation() == 0) {
      _echoOrientationAvailable = true;
    } else {
      _echoOrientationAvailable = false;
    }
    if (_echoOrientationAvailable) {
      _orient->loadSdevFields(_gridLoc,
                              _sdevDbzH,
                              _sdevDbzV);
    }
  }

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
  
  // write out data

  _printRunTime("Cart interp - before _writeOutputFile");

  if (_writeOutputFile()) {
    cerr << "ERROR - InterpToCart::interpVol" << endl;
    cerr << "  Cannot write output file" << endl;
    return -1;
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

void InterpToCart::_createThreads()
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

void InterpToCart::_freeThreads()
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
// add dual pol derived fields

void InterpToCart::_addDpDerivedFields()

{

  

}
  
//////////////////////////////////////////////////
// create the debug Cart fields

void InterpToCart::_createDebugFields()

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

  if (_params.use_echo_orientation) {
    _sdevDbzH = new DerivedField("SdevDbzH", "sdev_of_dbz_horizontal", "dBZ", true);
    _derived3DFields.push_back(_sdevDbzH);
    _sdevDbzV = new DerivedField("SdevDbzV", "sdev_of_dbz_vertical", "dBZ", true);
    _derived3DFields.push_back(_sdevDbzV);
  }
  
}

//////////////////////////////////////////////////
// free the derived fields

void InterpToCart::_freeDerivedFields()
  
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

void InterpToCart::_initZLevels()

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

void InterpToCart::_freeZLevels()

{
  _gridZLevels.clear();
}

////////////////////////////////////////////////////////////
// Initialize output grid

void InterpToCart::_initGrid()

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

}
  
////////////////////////////////////////////////////////////
// Free grid loc array

void InterpToCart::_freeGridLoc()
  
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

void InterpToCart::_allocOutputArrays()
  
{

  _freeOutputArrays();
  _outputFields = (fl32 **) umalloc2(_interpFields.size(),
                                     _nPointsVol, sizeof(fl32));
  
}

////////////////////////////////////////////////////////////
// Free up the output arrays for the gridded fields

void InterpToCart::_freeOutputArrays()
  
{
  if (_outputFields) {
    ufree2((void **) _outputFields);
  }
  _outputFields = NULL;
}

////////////////////////////////////////////////////////////
// init the output arrays for the gridded fields

void InterpToCart::_initOutputArrays()
  
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

void InterpToCart::_computeSearchLimits()
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

void InterpToCart::_computeGridRelative()

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

void InterpToCart::_computeGridRelMultiThreaded()
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

void InterpToCart::_computeGridRow(int iz, int iy)

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

void InterpToCart::_allocSearchMatrix()

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

void InterpToCart::_freeSearchMatrix()

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

void InterpToCart::_initSearchMatrix()

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

void InterpToCart::_fillSearchMatrix()

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

void InterpToCart::_printSearchMatrix(FILE *out, int res)
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

void InterpToCart::_printSearchMatrixPoint(FILE *out, int iel, int iaz)
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

int InterpToCart::_fillSearchLowerLeft(int level,
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

int InterpToCart::_fillSearchUpperLeft(int level,
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

int InterpToCart::_fillSearchLowerRight(int level,
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

int InterpToCart::_fillSearchUpperRight(int level,
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

int InterpToCart::_getSearchElIndex(double el) 
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

int InterpToCart::_getSearchAzIndex(double az) 
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

double InterpToCart::_getSearchEl(int index) 
{
  return _searchMinEl + index * _searchResEl;
}
  
/////////////////////////////////////////////////////////
// get the azimuth given the search index

double InterpToCart::_getSearchAz(int index) 
{
  return _searchMinAz + index * _searchResAz;
}
  
//////////////////////////////////////////////////
// interpolate onto the grid

void InterpToCart::_doInterp()
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

void InterpToCart::_interpSingleThreaded()
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

void InterpToCart::_interpMultiThreaded()
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

void InterpToCart::_interpRow(int iz, int iy)

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
  
void InterpToCart::_loadWtsFor2ValidRays(const GridLoc *loc,
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
  
void InterpToCart::_loadWtsFor3Or4ValidRays(const GridLoc *loc,
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
  
int InterpToCart::_loadNearestGridPt(int ifield,
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
  
int InterpToCart::_loadInterpGridPt(int ifield,
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
  
int InterpToCart::_loadFoldedGridPt(int ifield,
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

double InterpToCart::_conditionAz(double az)
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

int InterpToCart::_writeOutputFile()
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

//////////////////////////////////////////////////
// Compute convective/stratiform split

int InterpToCart::_convStratCompute()
{

  // set the grid in the ConvStratFinder object

  bool isLatLon = (_params.grid_projection == Params::PROJ_LATLON);
  _convStrat.setGrid(_gridNx, _gridNy,
                     _gridDx, _gridDy,
                     _gridMinx, _gridMiny,
                     _gridZLevels,
                     isLatLon);

  // get the dbz field for ConvStratFinder

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
    cerr << "ERROR - InterpToCart::_convStratCompute()" << endl;
    cerr << "  Cannot find dbz field: " << dbzName << endl;
    cerr << "  conv/strat partition will not be computed" << endl;
    return -1;
  }

  // compute the convective/stratiform partition
  
  if (_convStrat.computeEchoType(dbzVals, missingFl32)) {
    cerr << "ERROR - InterpToCart::_convStratCompute()" << endl;
    cerr << "  _convStrat.computePartition() failed" << endl;
    return -1;
  }

  _printRunTime("InterpToCart::_convStratCompute");

  return 0;
  
}

///////////////////////////////////////////////////////////////
// FillSearchLowerLeft thread
///////////////////////////////////////////////////////////////
// Constructor
InterpToCart::FillSearchLowerLeft::FillSearchLowerLeft(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::FillSearchLowerLeft::run()
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
InterpToCart::FillSearchLowerRight::FillSearchLowerRight(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::FillSearchLowerRight::run()
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
InterpToCart::FillSearchUpperLeft::FillSearchUpperLeft(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::FillSearchUpperLeft::run()
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
InterpToCart::FillSearchUpperRight::FillSearchUpperRight(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::FillSearchUpperRight::run()
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
InterpToCart::ComputeGridRelative::ComputeGridRelative(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::ComputeGridRelative::run()
{
  _this->_computeGridRow(_zIndex, _yIndex);
}

///////////////////////////////////////////////////////////////
// PerformInterp thread
///////////////////////////////////////////////////////////////
// Constructor
InterpToCart::PerformInterp::PerformInterp(InterpToCart *obj) :
        _this(obj)
{
}  
// run method
void InterpToCart::PerformInterp::run()
{
  _this->_interpRow(_zIndex, _yIndex);
}
