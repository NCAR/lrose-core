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
/////////////////////////////////////////////////////////////
// Props.cc
//
// Props class - storm properties computations
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Props.hh"
#include "InputMdv.hh"
#include "GridClump.hh"
#include "Verify.hh"
#include "Sounding.hh"

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <toolsa/pmu.h>
#include <rapmath/math_macros.h>
#include <rapmath/umath.h>
#include <physics/vil.h>
#include <Mdv/MdvxRadar.hh>
using namespace std;

#define MAX_EIG_DIM 3

//////////////
// constructor
//

Props::Props(const string &prog_name, const Params &params,
	     const InputMdv &input_mdv, TitanStormFile &storm_file,
	     Verify *verify) :
        Worker(prog_name, params),
        _inputMdv(input_mdv),
        _sfile(storm_file),
        _verify(verify),
        _area(_progName, _params, _inputMdv, _sfile)

{
  
  // alloc arrays to initial sizes
  
  _nZAlloc = 1;
  _nHistAlloc = 1;
  
  _layer =
    (layer_stats_t *) umalloc (_nZAlloc * sizeof(layer_stats_t));
  _dbzHist =
    (dbz_hist_entry_t *) umalloc (_nHistAlloc * sizeof(dbz_hist_entry_t));
  _tiltData =
    (double **) umalloc2 (_nZAlloc, MAX_EIG_DIM, sizeof(double));
  _dbzGradientData =
    (double **) umalloc2 (_nZAlloc, MAX_EIG_DIM, sizeof(double));

  _means = (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvalues =
    (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvectors =
    (double **) umalloc2 (MAX_EIG_DIM, MAX_EIG_DIM, sizeof(double));

  // compute number of dBZ hist intervals
  
  _nDbzHistIntervals = (int)
    ((_params.high_dbz_threshold - _params.low_dbz_threshold) /
     (_params.dbz_hist_interval) + 1);
  
  // hail mass parameters

  _hailZM.setRelationship(params.hail_ZM.coeff, params.hail_ZM.expon,
                          params.hail_mass_dbz_threshold);
}

/////////////
// destructor
//

Props::~Props()

{

  if (_layer) {
    ufree(_layer);
  }

  if (_dbzHist) {
    ufree(_dbzHist);
  }

  if (_tiltData) {
    ufree2((void **) _tiltData);
  }

  if (_dbzGradientData) {
    ufree2((void **) _dbzGradientData);
  }

  if (_means) {
    ufree(_means);
  }

  if (_eigenvalues) {
    ufree(_eigenvalues);
  }

  if (_eigenvectors) {
    ufree2((void **) _eigenvectors);
  }

}

///////////////////////////////////////
// initialize for latest MDV input file
//

void Props::init()

{

  // min valid ht and number of valid planes
  
  _minValidZ = _inputMdv.grid.minz +
    _inputMdv.minValidLayer * _inputMdv.grid.dz;
  _nzValid = _inputMdv.grid.nz - _inputMdv.minValidLayer;
  
  // compute test sampling range - if any points in a storm are at
  // longer range, set flag indicating it is range limited. The test
  // range is the max range less twice the diagonal of a grid cell.
  // For this test, it is only necessary to consider x and y, since the
  // elevation angle will be low.

  DsRadarParams *radar = NULL;
  DsRadarElev *elevs = NULL;
  MdvxRadar mdvxRadar;
  if (mdvxRadar.loadFromMdvx(_inputMdv.mdvx) == 0) {
    radar = &mdvxRadar.getRadarParams();
    elevs = &mdvxRadar.getRadarElev();
  }
  
  if(radar && radar->numGates > 0) {
    double max_range =
      (radar->startRange + radar->gateSpacing * radar->numGates);
    double range_margin =
      2.0 * pow(((double) _inputMdv.grid.dx * _inputMdv.grid.dx +
		 _inputMdv.grid.dy * _inputMdv.grid.dy), 0.5);
    _testRangeKm = max_range - range_margin;
  } else {
    _testRangeKm = 0.0;
  }
  
  // compute test elevation angle - if any points in a storm are at
  // higher elevation angle, set flag to indicate that the storm top
  // has not been sampled. The test angle is the highest elevation angle
  // in the volume scan less some margin.

  if (elevs && elevs->getNElev() > 0) {
    double elev_margin = radar->vertBeamWidth;
    double max_elev = elevs->getElevArray()[elevs->getNElev() - 1];
    _testElevRad = (max_elev - elev_margin) * DEG_TO_RAD;
  } else {
    _testElevRad = 0.0;
  }
  
  // inverse coefficients for the Z-R and Z-M relationships
  
  _ZPInverseCoeff = 1.0 / _params.ZR.coeff;
  _ZPInverseExpon = 1.0 / _params.ZR.expon;
  
  _ZMInverseCoeff = 1.0 / _params.ZM.coeff;
  _ZMInverseExpon = 1.0 / _params.ZM.expon;
  
  // vorticity factor to take into account the fact that
  // positive vorticity is anti-clockwise in the northern
  // hemisphere and clockwise in the southern hemisphere.
  // Northern - latitude positive. Southern - latitude negative.
  
  if (radar && radar->latitude < 0) {
    _vortHemisphereFactor = -1.0;
  } else {
    _vortHemisphereFactor = 1.0;
  }
    
  // min vorticity distance limits the calculations to a certain
  // distance from the cemtroid. to avoid very close points from
  // having a dis-proportional effect on the calculation
  
  _minVortDist = 2.0 * sqrt(_inputMdv.grid.dx * _inputMdv.grid.dx +
			    _inputMdv.grid.dy * _inputMdv.grid.dy);

  // heights of various temperatures
  
  Sounding &sndg = Sounding::inst();
  _freezingLevel = sndg.getProfile().getFreezingLevel();
  _htMinus20 = sndg.getProfile().getHtKmForTempC(-20.0);

}

////////////////////////////////////////////////
// compute()
//
// Returns 0 if success (clump is a valid storm)
//         -1 if failure
//

int Props::compute(const GridClump &grid_clump, int storm_num)

      
{

  PMU_auto_register("Props::compute - computing properties");

  // allocate

  _alloc(_inputMdv.grid.nz, _nDbzHistIntervals);

  // initialize
    
  _baseLayer = _nzValid;
  _topLayer = 0;
  _rangeLimited = FALSE;
  _topMissing = FALSE;
  _hailPresent = FALSE;
  _secondTrip = FALSE;
  
  memset (&_gprops, 0, sizeof(storm_file_global_props_t));
  memset (_layer, 0, _nzValid * sizeof(layer_stats_t));
  memset (_dbzHist, 0,
	  _nDbzHistIntervals * sizeof(dbz_hist_entry_t));
  memset (&_sum, 0, sizeof(sum_stats_t));

  for (int ii = 0; ii < _nzValid; ii++) {
    _layer[ii].htKm = _minValidZ + ii * _inputMdv.grid.dz;
  }

  // first pass through the clumps, computing the relevant things
  // from which to compute the storm properties.
  // Also, count the number of data runs for this storm.
  
  if (_computeFirstPass(grid_clump)) {
    return (-1);
  }

  // get ht of 45 dBZ
  
  _ht45AboveFreezing = _topOfDbz(45.0, grid_clump) - _freezingLevel;

  // now that we have the necessary first pass info, compute hail metrics.

  _computeHailMetrics(grid_clump);

  // perform the areal computations for precip and projected
  // areas, including dbz histogram for area

  _area.compute(grid_clump, &_gprops, _dbzHist);

  // compute other props during second pass through intervals
  
  _computeSecondPass(grid_clump);

  // tilt angle computations

  _tiltCompute();

  // compute dbz gradient with height

  _dbzGradientCompute();

  // decide whether this is a second trip echo
   
  if (_params.check_second_trip) {
    _secondTrip = _checkSecondTrip();
  }
      
  // write valid_storms verification file
  
  if (_verify) {
    _verify->updateValidStormsGrid(grid_clump);
  }

  // load up global storm properties

  storm_file_global_props_t *gprops = _sfile._gprops + storm_num;
  *gprops = _gprops;
  
  _loadGprops(gprops,
 	      storm_num, _nLayers, _baseLayer, _nDbzIntvls,
 	      _rangeLimited, _topMissing,
 	      _hailPresent, _secondTrip);
  
  // load layer props structure, for each layer
  
  for (int iz = _baseLayer; iz <= _topLayer; iz++) {
    _loadLprops(_layer + iz, _sfile._lprops + iz - _baseLayer);
  }
  
  // load dbz histogram struct, for each histogram interval
    
  for (int dbz_intvl = 0; dbz_intvl < _nDbzIntvls; dbz_intvl++) {
    _loadDbzHist(_dbzHist + dbz_intvl, _sfile._hist + dbz_intvl);
  }
  
  // store runs
  
  if (_params.store_storm_runs) {
    gprops->n_runs = _storeRuns(grid_clump);
  } else {
    gprops->n_runs = 0;
  }

  gprops->n_proj_runs = _area.storeProjRuns(grid_clump);

  return (0);

}

////////////////////
// _alloc()
//

void Props::_alloc(int nz, int nhist)

{

  if (nz > _nZAlloc) {

    _layer = (layer_stats_t *) urealloc
      (_layer, nz * sizeof(layer_stats_t));

    ufree2((void **) _tiltData);
    _tiltData = (double **) umalloc2 (nz, MAX_EIG_DIM, sizeof(double));
    
    ufree2((void **) _dbzGradientData);
    _dbzGradientData = (double **) umalloc2 (nz, MAX_EIG_DIM, sizeof(double));
    
    _nZAlloc = nz;

  }

  if (nhist > _nHistAlloc) {

    _dbzHist = (dbz_hist_entry_t *) urealloc
      (_dbzHist, nhist * sizeof(dbz_hist_entry_t));
    
    _nHistAlloc = nhist;

  }
    
  _sfile.AllocLayers(_inputMdv.grid.nz);
  _sfile.AllocHist(_nDbzHistIntervals);

}

//////////////////////////////////////
// _computeFirstPass()
//
// First pass through data
//
// Returns 0 if successful, -1 if not
//

// FUDGE_X is the distance x is moved to avoid errors from
// computing arctan when the points are coincident
#define FUDGE_X (0.1)

int Props::_computeFirstPass(const GridClump &grid_clump)

{

  int max_dbz_intvl = 0;
  const titan_grid_t &grid = grid_clump.grid;
  int nptsPlane = grid.nx * grid.ny;

  // initialize

  double highDbzThreshold = _params.high_dbz_threshold;
  double lowDbzThreshold = _params.low_dbz_threshold;
  double histInterval = _params.dbz_hist_interval;

  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {
    
    const Interval &intvl = grid_clump.intervals[intv];

    int iz = intvl.plane;
    int jz = iz + _inputMdv.minValidLayer;
    int iy = intvl.row_in_plane;
    _layer[iz].n += intvl.len;
	
    int index = ((iy + grid_clump.startIy) * grid.nx +
		 intvl.begin + grid_clump.startIx);
    
    fl32 *dbz_ptr = _inputMdv.dbzVol + jz * nptsPlane + index;
    fl32 *vel_ptr = NULL;
    if (_params.vel_available) {
      vel_ptr = _inputMdv.velVol + jz * nptsPlane + index;
    }
      
    for (int ix = intvl.begin; ix <= intvl.end;
	 ix++, dbz_ptr++, vel_ptr++) {
      
      // if dbz value exceeds max threshold, don't
      // process this clump
	
      if ((int) *dbz_ptr > highDbzThreshold) {
	return (-1);
      }
      
      // get the dbz, vel and precip mass_factor
	
      double dbz = *dbz_ptr;
      double r_dbz;

      if (dbz > _params.hail_dbz_threshold) {
	r_dbz = _params.hail_dbz_threshold;
	_hailPresent = TRUE;
      } else {
	r_dbz = dbz;
      }
	
      double refl = pow(10.0, r_dbz / 10.0);
      
      index = iy * grid.nx + ix;
	
      double vel = 0.0;
      if (_params.vel_available) {
	vel = *vel_ptr;
      }
      
      double mass_factor = pow(refl, _ZMInverseExpon);
	
      // get the dbz histogram index number
      
      int dbz_intvl = (int) ((dbz - lowDbzThreshold) / histInterval);
      
      // increment quantities
      
      // number of grid points per layer
      
      _layer[iz].sum_x += ix;
      _layer[iz].sum_y += iy;
      
      // reflectivity
      
      _layer[iz].sum_refl_x += (double) ix * refl;
      _layer[iz].sum_refl_y += (double) iy * refl;
      _layer[iz].sum_refl += refl;
      
      if (dbz > _layer[iz].dbz_max) {
	_layer[iz].dbz_max = dbz;
      }
      
      // dbz histograms
      
      if (dbz_intvl >= 0 && dbz_intvl < _nDbzHistIntervals) {
	_dbzHist[dbz_intvl].n_vol++;
      }
      
      // precip mass
      
      _layer[iz].sum_mass += mass_factor;
      
      // velocity
      
      if (_params.vel_available) {
	_layer[iz].sum_vel += vel;
	_layer[iz].sum_vel2 += vel * vel;
      }
      
    } // ix
    
  } // intv  - first pass
  
  // sum up variables over layers
  
  int dbz_max_layer = 0;

  for (int iz = 0; iz < _nzValid; iz++) {
    
    if (_layer[iz].n > 0) {
      
      _sum.n += _layer[iz].n;
      _sum.x += _layer[iz].sum_x;
      _sum.y += _layer[iz].sum_y;
      _sum.z += iz * _layer[iz].n;
      
      _sum.refl += _layer[iz].sum_refl;
      _sum.refl_x += _layer[iz].sum_refl_x;
      _sum.refl_y += _layer[iz].sum_refl_y;
      _sum.refl_z += (double) iz * _layer[iz].sum_refl;
	
      if (_layer[iz].dbz_max >= _gprops.dbz_max) {
	_gprops.dbz_max = _layer[iz].dbz_max;
	dbz_max_layer = iz;
      }
	
      _sum.mass += _layer[iz].sum_mass;
	
      if (iz > _topLayer)
	_topLayer = iz;
	
      if (iz < _baseLayer)
	_baseLayer = iz;
	
      if (_params.vel_available) {
	_sum.vel += _layer[iz].sum_vel;
	_sum.vel2 += _layer[iz].sum_vel2;
      }
      
    }
      
  } // iz
    
  _nLayers = _topLayer - _baseLayer + 1;

  // compute the properties
    
  double dn = (double) _sum.n;

  _gprops.vol_centroid_x =
    grid_clump.startX + ((double) _sum.x / dn) * grid.dx; // km
  _gprops.vol_centroid_y =
    grid_clump.startY + ((double) _sum.y / dn) * grid.dy; // km
  _gprops.vol_centroid_z =
    _minValidZ + ((double) _sum.z / dn) * grid.dz; // km
  
  _gprops.refl_centroid_x =
    grid_clump.startX + (_sum.refl_x / _sum.refl) * grid.dx; // km
    
  _gprops.refl_centroid_y =
    grid_clump.startY + (_sum.refl_y / _sum.refl) * grid.dy; // km
  
  _gprops.refl_centroid_z =
    _minValidZ + (_sum.refl_z / _sum.refl) * grid.dz; // km
  
  _gprops.volume = grid_clump.stormSize; // km3 or km2
  
  _gprops.area_mean =
    (dn / (double) _nLayers) * grid_clump.dAreaAtCentroid; // km2
  
  _gprops.top = _minValidZ +
    ((double) _topLayer + 0.5) * grid.dz; // km
    
  _gprops.base = _minValidZ +
    ((double) _baseLayer - 0.5) * grid.dz; // km
    
  _gprops.ht_of_dbz_max = _minValidZ +
    (double) dbz_max_layer * grid.dz; // km
    
  _gprops.dbz_mean = log10(_sum.refl / dn) * 10.0; // dbz
  
  _gprops.rad_vel_mean = (_sum.vel / dn); // m/s
  
  _gprops.rad_vel_sd = usdev(_sum.vel, _sum.vel2, dn);

  _gprops.mass = (_sum.mass * grid_clump.dVolAtCentroid *
                  pow(_ZMInverseCoeff, _ZMInverseExpon)); // ktons
    
  // layer properties

  for (int iz = 0; iz < _nzValid; iz++) {
    
    _layer[iz].area =
      (double) _layer[iz].n * grid_clump.dAreaAtCentroid; // km2
    
    if (_layer[iz].n > 0) {
      
      double dnperlayer = (double) _layer[iz].n;
	
      _layer[iz].vol_centroid_x = grid_clump.startX +
	((double) _layer[iz].sum_x / dnperlayer) * grid.dx; // km
      _layer[iz].vol_centroid_y = grid_clump.startY +
	((double) _layer[iz].sum_y / dnperlayer) * grid.dy; // km
	
      _layer[iz].refl_centroid_x = grid_clump.startX +
	(_layer[iz].sum_refl_x / _layer[iz].sum_refl) * grid.dx; // km
      _layer[iz].refl_centroid_y = grid_clump.startY +
	(_layer[iz].sum_refl_y / _layer[iz].sum_refl) * grid.dy; // km
	
      _layer[iz].dbz_mean = log10(_layer[iz].sum_refl / dnperlayer) * 10.0;
      
      _layer[iz].mass =
	(_layer[iz].sum_mass * grid_clump.dVolAtCentroid
	 * pow(_ZMInverseCoeff, _ZMInverseExpon)); // ktons
      
      if (_params.vel_available) {
	_layer[iz].vel_mean = _layer[iz].sum_vel / dnperlayer;
	_layer[iz].vel_sd = usdev(_layer[iz].sum_vel,
				 _layer[iz].sum_vel2, dnperlayer);
      }

    } // if (_layer[iz].n .......
    
  } // iz

  // vil - computed from maz dbz in each layer

  vil_init();
  for (int iz = 0; iz < _nzValid; iz++) {
    if (_layer[iz].n > 0) {
      vil_add(_layer[iz].dbz_max, grid.dz);
    }
  } // iz
  _gprops.vil_from_maxz = vil_compute();
  
  // dbz histograms
  
  for (int dbz_intvl = 0;
       dbz_intvl < _nDbzHistIntervals; dbz_intvl++) {
    
    if (_dbzHist[dbz_intvl].n_vol > 0) {
      _dbzHist[dbz_intvl].percent_vol = 100.0 *
	(double) _dbzHist[dbz_intvl].n_vol / dn;
      max_dbz_intvl = dbz_intvl;
    } else {
      _dbzHist[dbz_intvl].percent_vol = 0.0;
    }
      
  } // dbz_intvl
    
  _nDbzIntvls = max_dbz_intvl + 1;
    
  // compute the azimuth and range from the radar to the layer centroids
    
  for (int iz = _baseLayer; iz <= _topLayer; iz++) {
      
    if (grid.sensor_x == _layer[iz].vol_centroid_x &&
	grid.sensor_y == _layer[iz].vol_centroid_y)
      _layer[iz].vol_centroid_x += FUDGE_X;
      
    double cent_x = _layer[iz].vol_centroid_x - grid.sensor_x;
    double cent_y = _layer[iz].vol_centroid_y - grid.sensor_y;
      
    double range = sqrt(cent_x * cent_x + cent_y * cent_y);
    _layer[iz].vol_centroid_range = range;
    if (range == 0.0)
      _layer[iz].vol_centroid_az = 0.0;
    else
      _layer[iz].vol_centroid_az = atan2(cent_y, cent_x);
      
  }

  return (0);

}

//////////////////////////////////////
// _computeSecondPass()
//
// Second pass through data, computing vorticity and checking
// range limits as required.
//

// vorticity model assumes the storm has a 
// circular structure
#define VORTICITY_MODEL_FACTOR 2.0

void Props::_computeSecondPass(const GridClump &grid_clump)

{

  const titan_grid_t &grid = _inputMdv.grid;
  int nptsPlane = grid.nx * grid.ny;

  // initialize

  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {

    const Interval intvl = grid_clump.intervals[intv];

    int iz = intvl.plane;
    int jz = iz + _inputMdv.minValidLayer;
    int iy = intvl.row_in_plane;
    
    double grid_x = (double) intvl.begin * grid.dx + grid_clump.startX;
    double grid_y = (double) iy * grid.dy + grid_clump.startY;
    double grid_z = (double) jz * grid.dz + grid.minz;
    double rel_y = grid_y - grid.sensor_y;
    double rel_z = grid_z - grid.sensor_z;

    int index = ((iy + grid_clump.startIy) * grid.nx +
		 intvl.begin + grid_clump.startIx);

    fl32 *vel_ptr = NULL;
    if (_params.vel_available) {
      vel_ptr = _inputMdv.velVol + jz * nptsPlane + index;
    }

    for (int ix = intvl.begin; ix <= intvl.end;
	 ix++, grid_x += grid.dx, vel_ptr++) {
      
      // compute grid x and y
      
      if (_params.check_range_limits) {

	// Test the range of the run end_points, and flag the storm if the
	// range exceeds the test range - only perform this if not
	// already flagged. Only (x, y) is considered relevant -
	// z will have negligible effect on range computations at
	// long range.

	// Also, test the elevation angle of the run end_points, and flag
	// the storm if the angle exceeds the test angle - only
	// perform this if not already flagged.
	// For these computations, the curvature of the earth and
	// beam may be ignored, since range is short, and elevation
	// angle high.
      
	if (ix == intvl.begin || ix == intvl.end) {
	
	  double rel_x = grid_x - grid.sensor_x;
	
	  double gnd_range = pow((rel_x * rel_x + rel_y * rel_y), 0.5);
	  
	  if (_testRangeKm > 0.0 && !_rangeLimited) {
	    
	    if (gnd_range > _testRangeKm) {
	      _rangeLimited = TRUE;
            }
	    
	    if (_params.debug >= Params::DEBUG_EXTRA && _rangeLimited) {
	      fprintf(stderr,
		      "***** Range limited, x, y, range = "
		      "%g, %g, %g\n",
		      grid_x, grid_y, gnd_range);
	    }
	    
	  } // if (!_rangeLimited)

	  if (_testElevRad > 0.0 && !_topMissing) {
	    
	    double elev;

	    if (rel_z == 0.0 && gnd_range == 0.0)
	      elev = 0.0;
	    else
	      elev = atan2(rel_z, gnd_range);
	    
	    if (elev > _testElevRad) {
              
	      _topMissing = TRUE;
              
	      if (_params.debug >= Params::DEBUG_EXTRA) {
		fprintf(stderr,
			"** Top missing,x,y,z, elev = "
			"%g,%g,%g,%g\n",
			grid_x, grid_y, grid_z,
			elev * RAD_TO_DEG);
	      }
	    
	    } // if (elev > _testElevRad)
	  
	  } // if (!_topMissing)
	
	} // if (ix == intvl.begin ...
	
      } // if (_params.check_range_limits)

      if (_params.vel_available) {

	// compute grid azimuth from radar,
	// and delta azimuth from vol_centroid azimuth
	
	double grid_az;
	if (grid_x == grid.sensor_x && grid_y == grid.sensor_y) {
	  grid_az = 0.0;
	} else {
	  grid_az = atan2(grid_y - grid.sensor_y, grid_x - grid.sensor_x);
	}
	
	double delta_az = _layer[iz].vol_centroid_az - grid_az;
	
	if (delta_az > M_PI) {
	  delta_az -= TWO_PI;
	}
	
	if (delta_az < M_PI) {
	  delta_az += TWO_PI;
	}
	
	double vorticity_dist = _layer[iz].vol_centroid_range * sin(delta_az);

	if (fabs(vorticity_dist) >= _minVortDist) {
	  double vel = *vel_ptr;
	  _layer[iz].n_vorticity++;
	  _layer[iz].sum_vorticity +=
	    (vel - _layer[iz].vel_mean) / (vorticity_dist * 1000.0);
	} // if (fabs ....
	
      } // if ((_params.vel_available)
      
    } // ix
    
  } // intv
  
  if (_params.vel_available) {
    
    // compute the vorticity at each layer and for the entire volume
    
    for (int iz = _baseLayer; iz <= _topLayer; iz++) {
      
      _sum.vorticity += _layer[iz].sum_vorticity;
      _sum.n_vorticity += _layer[iz].n_vorticity;
      
      if (_layer[iz].n_vorticity > 0) {
	_layer[iz].vorticity = ((_layer[iz].sum_vorticity /
				 (double) _layer[iz].n_vorticity) *
				VORTICITY_MODEL_FACTOR *
			       _vortHemisphereFactor);
      } else {
	_layer[iz].vorticity = 0.0;
      }
      
    }
    
    if (_sum.n_vorticity > 0) {
      _gprops.vorticity =
	((_sum.vorticity / (double) _sum.n_vorticity) *
	 VORTICITY_MODEL_FACTOR * _vortHemisphereFactor);
    } else {
      _gprops.vorticity = 0.0;
    }
    
  } else {
    
    for (int iz = _baseLayer; iz <= _topLayer; iz++)
      _layer[iz].vorticity = 0.0;
    
    _gprops.vorticity = 0.0;
    
  } // if (_params.vel_available)

}

////////////////////////////////////////////      
// _storeRuns()
//
// Store the storm runs
//
// Returns the number of runs in the clump.
//

int Props::_storeRuns(const GridClump &grid_clump)

{
  
  // make sure there is space for the runs

  _sfile.AllocLayers(grid_clump.grid.nz);
  _sfile.AllocHist(_nDbzHistIntervals);
  _sfile.AllocRuns(grid_clump.nIntervals);
  
  storm_file_run_t *run = _sfile._runs;
  int start_ix = grid_clump.startIx;
  int start_iy = grid_clump.startIy;
  
  for (int intv = 0; intv < grid_clump.nIntervals; intv++, run++) {

    const Interval &intvl = grid_clump.intervals[intv];
  
    run->ix = intvl.begin + start_ix;
    run->iy = intvl.row_in_plane + start_iy;
    run->iz = intvl.plane + _inputMdv.minValidLayer;
    run->n = intvl.len;
	
  } // intv

  return (grid_clump.nIntervals);

}

//////////////////
// _tiltCompute()
//
// Compute tilt from  principal component analysis
//
// Note: area is in sq km.
//       tilt_area is in sq grid_units.
//

void Props::_tiltCompute()
     
{

  // check for more than 1 layer
  
  if (_nLayers < 2) {
    return;
  }

  // tilt angle computations
      
  // load up tilt_data array with the centroid positions for
  // each layer. The z values are exaggerated by a factor of 10.0 to
  // force the vertical to be the direction of dominant variance
      
  for (int iz = _baseLayer; iz <= _topLayer; iz++) {
    _tiltData[iz][0] = _layer[iz].vol_centroid_x;
    _tiltData[iz][1] = _layer[iz].vol_centroid_y;
    _tiltData[iz][2] = _minValidZ +
      (double) iz * _inputMdv.grid.dz * 10.0;
  }
      
  // obtain the principal component transformation for this data - the
  // technique is applicable here because the first principal component
  // will lie along the axis of maximum variance, which is equivalent
  // to fitting a line through the data points, minimizing the sum of
  // the squared perpendicular distances from the data to the line.
  
  if (upct(3, _nLayers, _tiltData + _baseLayer,
	   _means, _eigenvectors, _eigenvalues) != 0) {
    
    fprintf(stderr, "WARNING - %s:Props::_tiltCompute\n", _progName.c_str());
    fprintf(stderr, "Computing pct for tilt angle and direction.\n");
    _gprops.tilt_dirn = MISSING_VAL;
    _gprops.tilt_angle = MISSING_VAL;
    
  } else {
    
    // compute the tilt angle and direction from the first eigenvector
    
    double tilt_x = _eigenvectors[0][0];
    double tilt_y = _eigenvectors[1][0];
    double tilt_z = _eigenvectors[2][0] / 10.0;
    double tilt_r = sqrt(tilt_x * tilt_x + tilt_y * tilt_y);
    
    if (tilt_x == 0 && tilt_y == 0) {
      _gprops.tilt_dirn = 0.0;
      _gprops.tilt_angle = 0.0;
    } else {
      _gprops.tilt_dirn = atan2(tilt_x, tilt_y) * RAD_TO_DEG;
      if (_gprops.tilt_dirn < 0) {
	_gprops.tilt_dirn += 360.0;
      }
      _gprops.tilt_angle = 90.0 - atan2(tilt_z, tilt_r) * RAD_TO_DEG;
    }
    
  } // if (upct(3, _nLayers, _tiltData + _baseLayer, ...

}

/////////////////////////////////////////////////////////
// _dbzGradientCompute()
//
// Compute dbz_gradient from  principal component analysis
//

void Props::_dbzGradientCompute()
     
{
  
  // check for more than 1 layer

  if (_nLayers < 2) {
    return;
  }

  // load up dbz_gradient_data array with dbz_max.
  // The z values are exaggerated by a factor of 1000.0 to
  // force the vertical to be the direction of dominant variance
      
  for (int iz = _baseLayer; iz <= _topLayer; iz++) {
    _dbzGradientData[iz][0] = _layer[iz].dbz_max;
    _dbzGradientData[iz][1] = _minValidZ +
      (double) iz * _inputMdv.grid.dz * 1000.0;
  }
      
  // obtain the principal component transformation for this data - the
  // technique is applicable here because the first principal component
  // will lie along the axis of maximum variance, which is equivalent
  // to fitting a line through the data points, minimizing the sum of
  // the sguared perpendicular distances from the data to the line.
  
  if (upct(2, _nLayers,
	   _dbzGradientData + _baseLayer,
	   _means, _eigenvectors, _eigenvalues) != 0) {
    fprintf(stderr, "WARNING - %s:Props::_dbzGradientCompute\n",
	    _progName.c_str());
    fprintf(stderr, "Computing pct for dbz_max_gradient.\n");
    _gprops.dbz_max_gradient = MISSING_VAL;
  } else {
    _gprops.dbz_max_gradient = _eigenvectors[0][0] * 1000.0;
  } // if (upct ........

  // Repeat for dbz_mean.
      
  for (int iz = _baseLayer; iz <= _topLayer; iz++) {
    _dbzGradientData[iz][0] = _layer[iz].dbz_mean;
    _dbzGradientData[iz][1] = _minValidZ +
      (double) iz * _inputMdv.grid.dz * 1000.0;
  }
      
  if (upct( 2, _nLayers,
	    _dbzGradientData + _baseLayer,
	    _means, _eigenvectors, _eigenvalues) != 0) {
    fprintf(stderr, "WARNING - %s:Props::_dbzGradientCompute\n",
	    _progName.c_str());
    fprintf(stderr, "Computing pct for dbz_mean_gradient.\n");
    _gprops.dbz_mean_gradient = MISSING_VAL;
  } else {
    _gprops.dbz_mean_gradient = _eigenvectors[0][0] * 1000.0;
  }

}

////////////////////////////////////////////////////////////////////
// _checkSecondTrip()
//
// Checks for second trip  - returns TRUE or FALSE
//
// Check is based on aspect ratio of the echo, and how closely the
// major axis of the fitted ellipse lies to the radial from the
// radar to the storm centroid.
//

int Props::_checkSecondTrip()

{

  double top = _gprops.top;
  double base = _gprops.base;
  double centroid_x = _gprops.proj_area_centroid_x;
  double centroid_y = _gprops.proj_area_centroid_y;
  double major_radius = _gprops.proj_area_major_radius;
  double minor_radius = _gprops.proj_area_minor_radius;
  double orientation =  _gprops.proj_area_orientation;

  double radial_azimuth;
  if (centroid_x == 0.0 && centroid_y == 0.0) {
    radial_azimuth = 0.0;
  } else {
    radial_azimuth = atan2(centroid_x, centroid_y) * RAD_TO_DEG;
  }
  if (radial_azimuth < 0.0) {
    radial_azimuth += 180.0;
  }

  double delta_orientation = fabs(radial_azimuth - orientation);
  
  int orientation_OK;
  if (delta_orientation < _params.sectrip_orientation_error ||
      180.0 - delta_orientation < _params.sectrip_orientation_error) {
    orientation_OK = TRUE;
  } else {
    orientation_OK = FALSE;
  }

  double vert_aspect = major_radius / (top - base);
  double horiz_aspect = major_radius / minor_radius;

  int vert_aspect_OK;
  if (vert_aspect > _params.sectrip_vert_aspect) {
    vert_aspect_OK = TRUE;
  } else {
    vert_aspect_OK = FALSE;
  }

  int horiz_aspect_OK;
  if (horiz_aspect > _params.sectrip_horiz_aspect) {
    horiz_aspect_OK = TRUE;
  } else {
    horiz_aspect_OK = FALSE;
  }

  if (_params.debug >= Params::DEBUG_EXTRA &&
      orientation_OK && vert_aspect_OK && horiz_aspect_OK) {

    fprintf(stderr, "\n++++++++++ SECOND_TRIP ++++++++++++++\n");

    fprintf(stderr, "x,y : %g, %g\n", centroid_x, centroid_y);
    fprintf(stderr, "top, base : %g, %g\n", top, base);
    fprintf(stderr, "maj, min rad : %g, %g\n", major_radius,
	    minor_radius);
    fprintf(stderr, "orientation : %g\n", orientation);

    fprintf(stderr, "rad az : %g\n", radial_azimuth);
    fprintf(stderr, "delta orientation : %g\n", delta_orientation);
    fprintf(stderr, "orientation OK : %d\n", orientation_OK);
    fprintf(stderr, "vert, horiz aspect: %g, %g\n",
	    vert_aspect, horiz_aspect);
    fprintf(stderr, "vert_OK, horiz_OK : %d, %d\n\n",
	   vert_aspect_OK, horiz_aspect_OK);

  }

  if (orientation_OK && vert_aspect_OK && horiz_aspect_OK) {
    return (TRUE);
  } else {
    return (FALSE);
  }

}

/***************************************************************************
 * load_props.c
 *
 * Loads up the storm props
 *
 * Mike Dixon
 *
 * RAP, NCAR, Boulder, Colorado, USA
 *
 * April 1995
 *
 ****************************************************************************/

//////////////////////////////////
// _loadGprops()
//
// Load global storm properties
//

void Props::_loadGprops(storm_file_global_props_t *gprops,
			int storm_num,
			int n_layers,
			int base_layer,
			int n_dbz_intvls,
			int range_limited,
			int top_missing,
			int hail_present,
			int second_trip)

{

  gprops->storm_num = storm_num;
  gprops->n_layers = n_layers;
  gprops->base_layer = base_layer;
  gprops->n_dbz_intervals = n_dbz_intvls;
  
  gprops->range_limited = range_limited;
  gprops->top_missing = top_missing;
  gprops->hail_present = hail_present;
  gprops->second_trip = second_trip;
  
}


//////////////////////////////////
// _loadLprops()
//
// Load layer storm properties
//

void Props::_loadLprops(layer_stats_t *layer,
			 storm_file_layer_props_t *lprops)
     
{
  
  lprops->vol_centroid_x = layer->vol_centroid_x;
  lprops->vol_centroid_y = layer->vol_centroid_y;
  lprops->refl_centroid_x = layer->refl_centroid_x;
  lprops->refl_centroid_y = layer->refl_centroid_y;
  
  lprops->area = layer->area;
  lprops->dbz_max = layer->dbz_max;
  lprops->dbz_mean = layer->dbz_mean;
  lprops->mass = layer->mass;
  lprops->rad_vel_mean = layer->vel_mean;
  lprops->rad_vel_sd = layer->vel_sd;
  lprops->vorticity = layer->vorticity;
  
}

//////////////////////////////
// _loadDbzHist()
//
// Load dbz histogram props

void Props::_loadDbzHist(dbz_hist_entry_t *dbz_hist,
			 storm_file_dbz_hist_t *hist)

{

  hist->percent_volume = dbz_hist->percent_vol;
  hist->percent_area = dbz_hist->percent_area;

}

//////////////////////////////////////////////////////////////
// get top of dbz contour in clump

double Props::_topOfDbz(double dbz, const GridClump &grid_clump)
{

  // Returns the top, i.e. max height in km, of a dbz contour
  // If no values are found which exceed the dbz value, 
  // a height of 0.0km is returned

  int maxLayer = -1;
  double maxHeight;
  
  for (int iz = _topLayer; iz >= _baseLayer; iz--) {
    if (_layer[iz].dbz_max >= dbz) {
      maxLayer = iz;
      break;
    }
  }

  if (maxLayer == -1) {
    // specified dbz not found
    maxHeight = 0.0;
  } else if ( maxLayer == _topLayer ) {
    // specified dbz at the top layer
    maxHeight = _layer[maxLayer].htKm;
  } else {
    // interpolate between two layers
    int lowerLayer = maxLayer;
    int upperLayer = maxLayer + 1;
    double lowerDbzMax = _layer[lowerLayer].dbz_max;
    double upperDbzMax = _layer[upperLayer].dbz_max;
    double lowerHeight = _layer[lowerLayer].htKm;
    double upperHeight = _layer[upperLayer].htKm;
    maxHeight = ((upperHeight-lowerHeight)/(upperDbzMax-lowerDbzMax)*
                 (dbz-lowerDbzMax)) + lowerHeight;
  }
  return maxHeight;
}

//////////////////////////////////////////////////////////////
// compute hail metrics for the clump

void Props::_computeHailMetrics(const GridClump &grid_clump)
{

  // initialize
  
  const titan_grid_t &grid = grid_clump.grid;
  int nptsPlane = grid.nx * grid.ny;

  if (_params.debug_hail_metrics >= Params::DEBUG_VERBOSE) {
    cerr << "=====>> Layer temp/dbz profile for hail <<=====" << endl;
    for (int iz = 0; iz < _nzValid; iz++) {
      if (_layer[iz].n > 0) {
        double ht = _layer[iz].htKm;
        Sounding &sndg = Sounding::inst();
        double temp = sndg.getProfile().getTempForHtKm(ht);
        fprintf(stderr, "  iz, ht, temp, dbzmax: %3d, %8.3f %7.2f %7.2f\n",
                iz, ht, temp, _layer[iz].dbz_max);
      }
    } // iz
  }

  // HMA - hail mass aloft (2km above freezing)
  // sums up hail mass over clump
  
  _hailZM.initIntegration();
  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {
    const Interval &intvl = grid_clump.intervals[intv];
    int iz = intvl.plane;
    double ht = _layer[iz].htKm;
    if (ht > _freezingLevel + 2.0) {
      int jz = iz + _inputMdv.minValidLayer;
      int iy = intvl.row_in_plane;
      int index = ((iy + grid_clump.startIy) * grid.nx +
                   intvl.begin + grid_clump.startIx);
      fl32 *dbz_ptr = _inputMdv.dbzVol + jz * nptsPlane + index;
      for (int ix = intvl.begin; ix <= intvl.end; ix++, dbz_ptr++) {
        double dbz = *dbz_ptr;
        _hailZM.addDbz(dbz);
      }
    } // if (ht ...
  } // intv
  double hma = _hailZM.integralOfX(grid_clump.dVolAtCentroid);

  // VIHM - vertically integrated hail mass
  // uses the max dbz for each layer
  
  _hailZM.initIntegration();
  for (int iz = 0; iz < _nzValid; iz++) {
    if (_layer[iz].n > 0) {
      _hailZM.addDbz(_layer[iz].dbz_max);
    }
  } // iz
  double vihm = _hailZM.integralOfX(grid.dz); 

  // FOKR category

  int fokr = _getFokrCategory(grid_clump);
  
  // waldvogel probability of hail
  
  double wpoh = _getWaldvogelProbability(grid_clump);

  // nexrad hail detection algorithm
  
  double poh, shi, posh, mehs;
  _computeNexradHda(grid_clump, poh, shi, posh, mehs);

  if ((_params.debug_hail_metrics >= Params::DEBUG_VERBOSE) ||
      (_params.debug_hail_metrics && poh > 0)) {
    fprintf(stderr, 
            "Hail metrics - "
            "%5.1f:dHt45 %6.1f:HMA %6.2f:VIHM %2d:FOKR "
            "%4.0f:POH %8.1f:SHI %4.0f:POSH %5.1f:MEHS\n",
            _ht45AboveFreezing, hma, vihm, fokr, poh, shi, posh, mehs);
  }
  
  if (_params.hail_detection_mode == Params::HAIL_METRICS) {

    _gprops.add_on.hail_metrics.hailMassAloft = hma;
    _gprops.add_on.hail_metrics.vihm = vihm;
    _gprops.add_on.hail_metrics.FOKRcategory = fokr;
    _gprops.add_on.hail_metrics.waldvogelProbability = wpoh;
    
  } else if (_params.hail_detection_mode == Params::NEXRAD_HDA) {

    _gprops.add_on.hda.poh = poh;
    _gprops.add_on.hda.shi = shi;
    _gprops.add_on.hda.posh = posh;
    _gprops.add_on.hda.mehs = mehs;

  }

}

/////////////////////////////////////////////////////////////////////////
// Foote Krauss (FOKR) storm classification: FOKR is intended to separate 
// non-hailstorms (Category 0 and 1) from potentially developing hailers 
// (Cat. 2), likely hailstorms (Cat. 3) and severe hailstorms (Cat. 4)

int Props::_getFokrCategory(const GridClump &grid_clump)
{

  // Check for degenerate case

  if (_ht45AboveFreezing <= 0.0) {
    return 0;
  }
  
  if (_ht45AboveFreezing >= 4.0 &&
      _gprops.dbz_max >= _params.FOKR_cat4_zmax_thresh) {
    return 4;
  } else if (_ht45AboveFreezing >= 3.0 &&
             _gprops.dbz_max >= _params.FOKR_cat3_zmax_thresh) {
    return 3;
  } else if (_gprops.dbz_max >= _params.FOKR_cat2_zmax_thresh) {
    return 2;
  } else if (_gprops.dbz_max >= _params.FOKR_cat1_zmax_thresh) {
    return 1;
  } else {
    return 0;
  }

}

//////////////////////////////////////////////////////////////
// Waldvogel and Federer probability of hail as a function of 
// height of 45 dBZ above freezing
//

const Props::heightProb_t Props::HEIGHT_PROB[] =
{
  { 5.80, 1.0 },
  { 5.00, 0.9 },
  { 4.20, 0.8 },
  { 3.55, 0.7 },
  { 3.07, 0.6 },
  { 2.70, 0.5 },
  { 2.40, 0.4 },
  { 2.17, 0.3 },
  { 1.97, 0.2 },
  { 1.80, 0.1 },
  { 1.65, 0.0 }
};

double Props::_getWaldvogelProbability(const GridClump &grid_clump)
{

  // Check for degenerate case
  
  if (_ht45AboveFreezing <= 0.0) {
    return 0.0;
  }
  
  // Move down the height-probability curve until we find our spot

  size_t ii = 0;
  double curveHeight = HEIGHT_PROB[ii].height;
  double curveProb = HEIGHT_PROB[ii].probability;
  
  while(curveProb > 0.0) {
    if (_ht45AboveFreezing >= curveHeight) {
      break;
    }
    ii++;
    curveHeight = HEIGHT_PROB[ii].height;
    curveProb   = HEIGHT_PROB[ii].probability;
  } // while
  
  return curveProb;

}

//////////////////////////////////////////////////////////////
// compute NEXRAD Hail Detection Algorithm for the clump
//
//  poh : probability of hail (%)
//  shi : severe hail index (Jm-1s-1)
//  posh: probability of severe hail (%)
//  mehs: maximum expected hail size (mm)
//
// Arthur Witt, Michael D. Eilts, Gregory J. Stumph,
// J. T. Johnson, E DeWayne Mitchell and Kevin W Thomas:
// An Enhanced Hail Detection Algorithm for the WSR-88D.
// Weather and Forecasting, Volume 13, June 1998

void Props::_computeNexradHda(const GridClump &grid_clump,
                              double &poh,
                              double &shi,
                              double &posh,
                              double &mehs)
{

  const titan_grid_t &grid = grid_clump.grid;

  // probability of hail (POH) is from Waldvogel et al
  // we comvert probability from fraction to percent

  poh = _getWaldvogelProbability(grid_clump) * 100.0;

  // compute Severe Hail Index (SHI)
  
  double dbzLower = 40.0;
  double dbzUpper = 50.0;
  double dbzDelta = dbzUpper - dbzLower;

  double keCoeff = 5.0e-6;
  double keExpon = 0.084;
  double shiSum = 0.0;

  // loop through layers
  
  for (int iz = 0; iz < _nzValid; iz++) {

    // check this layer is active for this storm

    if (_layer[iz].n == 0) {
      continue;
    }

    // get height
    
    double ht = _layer[iz].htKm;

    // compute weight based on height

    double wtHt = 0.0;
    if (ht >= _freezingLevel && ht <= _htMinus20) {
      wtHt = (ht - _freezingLevel) / (_htMinus20 - _freezingLevel);
    } else if (ht > _htMinus20) {
      wtHt = 1.0;
    }

    // get max dbz in layer
    
    double dbzMax = _layer[iz].dbz_max;

    // compute weight based on Z

    double wtZ = 0.0;
    if (dbzMax >= dbzLower && dbzMax <= dbzUpper) {
      wtZ = (dbzMax - dbzLower) / dbzDelta;
    } else if (dbzMax > dbzUpper) {
      wtZ = 1.0;
    }

    // compute hail kinetic energy
    
    double ke = keCoeff * pow(10.0, keExpon * dbzMax) * wtZ;

    // add to shi sum
    
    shiSum += wtHt * ke;
    
  } // iz

  shi = 0.1 * shiSum * (grid.dz * 1000.0);

  // compute Probability Of Severe Hail (POSH)
  // and maximum expected hail size
  
  posh = 0.0;
  mehs = 0.0;
  
  if (poh > 0.0 && shi > 0.0) {
    
    double warningThreshold = 57.5 * _freezingLevel - 121.0;
    if (warningThreshold < 20.0) {
      warningThreshold = 20.0;
    }
    
    posh = 29.0 * log(shi / warningThreshold) + 50.0;
    if (posh < 0.0) {
      posh = 0.0;
    } else if (posh > 100.0) {
      posh = 100.0;
    }

    mehs = 2.54 * sqrt(shi);
    
  }

}


