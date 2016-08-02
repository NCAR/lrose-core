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
// Area.cc
//
// Area class - storm area computations
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Props.hh"
#include "Area.hh"
#include "GridClump.hh"
#include "InputMdv.hh"

#include <toolsa/umisc.h>
#include <rapmath/umath.h>
using namespace std;

#define MAX_EIG_DIM 3

//////////////
// constructor
//

Area::Area(const string &prog_name, const Params &params,
	   const InputMdv &input_mdv, TitanStormFile &storm_file) :
  Worker(prog_name, params),
  _inputMdv(input_mdv),
  _sfile(storm_file),
  _boundary(prog_name, params)
  
{

  OK = TRUE;

  // grids

  _nGridAlloc = 0;
  _compGrid = NULL;
  _precipGrid = NULL;
  _dbzForPrecip = NULL;

  // ellipse comps

  _means = (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvalues =
    (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvectors =
    (double **) umalloc2 (MAX_EIG_DIM, MAX_EIG_DIM, sizeof(double));
  _areaCoords = (double **) umalloc2 (1, 2, sizeof(double));
  _nCoordsAlloc = 1;

  // compute number of dBZ hist intervals
  
  _nDbzHistIntervals = (int)
    ((_params.high_dbz_threshold - _params.low_dbz_threshold) /
     (_params.dbz_hist_interval) + 1);
  
  // precip
  
  _zPInverseCoeff = 1.0 / _params.ZR.coeff;
  _zPInverseExpon = 1.0 / _params.ZR.expon;
  _zFactor = pow(_zPInverseCoeff, _zPInverseExpon);

}

/////////////
// destructor
//

Area::~Area()

{

  if (_compGrid) {
    ufree(_compGrid);
  }

  if (_precipGrid) {
    ufree(_precipGrid);
  }

  if (_dbzForPrecip) {
    ufree(_dbzForPrecip);
  }

  if (_areaCoords) {
    ufree2((void **) _areaCoords);
  }

  if (_eigenvectors) {
    ufree2((void **) _eigenvectors);
  }
  if (_eigenvalues) {
    ufree(_eigenvalues);
  }
  if (_means) {
    ufree(_means);
  }

}

////////////
// compute()
//

void Area::compute(const GridClump &grid_clump,
		   storm_file_global_props_t *gprops,
		   dbz_hist_entry_t *dbz_hist)

{

  _gProps = gprops;
  _gProps->bounding_min_ix = grid_clump.startIx;
  _gProps->bounding_min_iy = grid_clump.startIy;
  _gProps->bounding_max_ix = grid_clump.startIx + grid_clump.nX - 1;
  _gProps->bounding_max_iy = grid_clump.startIy + grid_clump.nY - 1;

  // compute grid sizes, and set grid params

  _nX = grid_clump.nX;
  _nY = grid_clump.nY;
  _nPoints = _nX * _nY;

  // check memory allocation and zero out grids

  _allocGrids();

  // load up composite grid with 1's to indicate projected area

  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {
    const Interval &intvl = grid_clump.intervals[intv];
    int offset = (intvl.row_in_plane * _nX) + intvl.begin;
    memset((_compGrid + offset), 1, intvl.len);
  }
  
  // projected area comps
  
  _ellipseCompute(grid_clump,
		  _compGrid,
		  &_gProps->proj_area,
		  &_gProps->proj_area_centroid_x,
		  &_gProps->proj_area_centroid_y,
		  &_gProps->proj_area_orientation,
		  &_gProps->proj_area_major_radius,
		  &_gProps->proj_area_minor_radius);

  // compute polygon for projected area
  
  _computeProjPolygon(grid_clump);

  // precip

  // load up grid with 1's to indicate precip area
  
  Params::precip_mode_t precipMode = _params.precip_computation_mode;
  
  for (int intv = 0; intv < grid_clump.nIntervals; intv++) {
    const Interval &intvl = grid_clump.intervals[intv];
    int izPlane = intvl.plane + _inputMdv.minValidLayer;
    if (precipMode == Params::PRECIP_FROM_COLUMN_MAX ||
        precipMode == Params::PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
      if (izPlane >= _inputMdv.minPrecipLayer &&
          izPlane <= _inputMdv.maxPrecipLayer) {
        int offset =
          (intvl.row_in_plane * _nX) + intvl.begin;
        memset((void *) (_precipGrid + offset), 1, (int) intvl.len);
      }
    } else {
      if (izPlane == _inputMdv.specifiedPrecipLayer) {
        int offset =
          (intvl.row_in_plane * _nX) + intvl.begin;
        memset((void *) (_precipGrid + offset), 1, (int) intvl.len);
      }
    }
  }
  
  // precip area comps
  
  _ellipseCompute(grid_clump,
                  _precipGrid,
                  &_gProps->precip_area,
                  &_gProps->precip_area_centroid_x,
                  &_gProps->precip_area_centroid_y,
                  &_gProps->precip_area_orientation,
                  &_gProps->precip_area_major_radius,
                  &_gProps->precip_area_minor_radius);
  
  // compute precip and 2D reflectivity histogram
  
  _computePrecip(grid_clump);
  _compute2dDbzHist(dbz_hist);

  // compute tops

  if (_params.set_dbz_threshold_for_tops) {
    _computeTops(grid_clump);
  }

}

//////////////////////////////////////////////////////////
// storeProjRuns()
//
// Store the projected area runs in the storm file handle

int Area::storeProjRuns(const GridClump &grid_clump)

{

  int nIntervals = _boundary.nIntervals();

  _sfile.AllocLayers(_inputMdv.grid.nz);
  _sfile.AllocHist(_nDbzHistIntervals);
  _sfile.AllocProjRuns(nIntervals);
  
  int start_ix = grid_clump.startIx;
  int start_iy = grid_clump.startIy;
  
  Interval *intvl = _boundary.intervals();
  storm_file_run_t *run = _sfile._proj_runs;
  
  for (int irun = 0; irun < nIntervals; irun++, run++, intvl++) {
    
    run->ix = intvl->begin + start_ix;
    run->iy = intvl->row_in_plane + start_iy;
    run->iz = 0;
    run->n = intvl->len;
	
  } // irun

  return (nIntervals);

}

/////////////////
// _allocGrids()

void Area::_allocGrids()

{
  if (_nPoints > _nGridAlloc) {
    _compGrid = (ui08 *) urealloc(_compGrid, _nPoints);
    _precipGrid = (ui08 *) urealloc(_precipGrid, _nPoints);
    _dbzForPrecip = (fl32 *) urealloc(_dbzForPrecip, _nPoints * sizeof(fl32));
    _nGridAlloc = _nPoints;
  }
  memset(_compGrid, 0, _nPoints);
  memset(_precipGrid, 0, _nPoints);
}

/////////////////////////////////////////////////////////
// _ellipseCompute()
//
// Compute ellipse from  principal component analysis
//
// Note: area is in sq km.
//       ellipse_area is in sq grid_units.
//

void Area::_ellipseCompute(const GridClump &grid_clump,
			   ui08 *grid,
			   fl32 *area,
			   fl32 *area_centroid_x,
			   fl32 *area_centroid_y,
			   fl32 *area_orientation,
			   fl32 *area_major_radius,
			   fl32 *area_minor_radius)
  
{


  /*
   * check coords allocation
   */

  int max_coords = grid_clump.grid.nx * grid_clump.grid.ny;
  _allocCoords(max_coords);

  // load up coords

  int n_coords = 0;
  double dy = grid_clump.grid.dy;
  double dx = grid_clump.grid.dx;
  double yy = grid_clump.grid.miny + grid_clump.startIy * dy;
  for (int iy = 0; iy < _nY; iy++, yy += dy) {
    double xx = grid_clump.grid.minx + grid_clump.startIx * dx;
    for (int ix = 0; ix < _nX; ix++, xx += dx) {
      if (*grid) {
	_areaCoords[n_coords][0] = xx;
	_areaCoords[n_coords][1] = yy;
	n_coords++;
      }
      grid++;
    } // ix
  } // iy
    
  if (n_coords == 0) {
   
    *area = 0.0;
    *area_centroid_x = 0.0;
    *area_centroid_y = 0.0;
    *area_orientation = 0.0;
    *area_major_radius = 0.0;
    *area_minor_radius = 0.0;
    
  } else {
    
    *area = (double) n_coords * grid_clump.dAreaAtCentroid;  
    double area_ellipse = (double) n_coords * grid_clump.dAreaEllipse;
    
    // obtain the principal component transformation for the coord data
    // The technique is applicable here because the first principal
    // component will lie along the axis of maximum variance, which
    // is equivalent to fitting a line through the data points,
    // minimizing the sum of the sguared perpendicular distances
    // from the data to the line.
    
    if (upct(2, n_coords, _areaCoords,
	     _means, _eigenvectors, _eigenvalues) != 0) {
      
      fprintf(stderr, "WARNING - %s:Area::_ellipseCompute\n",
	      _progName.c_str());
      fprintf(stderr, "Computing pct for precip area shape.\n");
      
      *area_orientation = MISSING_VAL;
      *area_major_radius = MISSING_VAL;
      *area_minor_radius = MISSING_VAL;
      
    } else {
      
      double area_u = _eigenvectors[0][0];
      double area_v = _eigenvectors[1][0];
      
      if (area_u == 0 && area_v == 0) {
	
	*area_orientation = 0.0;
	
      } else {
	
	*area_orientation = atan2(area_u, area_v) * RAD_TO_DEG;
	
	if (*area_orientation < 0)
	  *area_orientation += 180.0;
	
      }
      
      *area_centroid_x = _means[0];
      *area_centroid_y = _means[1];
      
      double area_major_sd;
      if (_eigenvalues[0] > 0)
	area_major_sd = sqrt(_eigenvalues[0]);
      else
	area_major_sd = 0.1;
      
      double area_minor_sd;
      if (_eigenvalues[1] > 0)
	area_minor_sd = sqrt(_eigenvalues[1]);
      else
	area_minor_sd = 0.1;
      
      double scale_factor =
	sqrt(area_ellipse /
	     (M_PI * area_minor_sd * area_major_sd));
      
      *area_major_radius = area_major_sd * scale_factor;
      *area_minor_radius = area_minor_sd * scale_factor;
      
      // check for ridiculous results, which occur when all points
      // line up in a straight line
      
      if (*area_major_radius / *area_minor_radius > 1000.0) {
	
	*area_major_radius  = sqrt(area_ellipse / M_PI);
	*area_minor_radius  = sqrt(area_ellipse / M_PI);
	
      } // if (*area_major_radius ...
      
    } // if (upct.....
    
  } // if (n_coords == 0)

}

/////////////////
// _allocCoords()
//

void Area::_allocCoords(const int n_coords)
     
{

  if (n_coords > _nCoordsAlloc) {
    // delete old
    ufree2((void **) _areaCoords);
    // new allocation
    _areaCoords =
      (double **) umalloc2 (n_coords, 2, sizeof(double));
    // save size allocated
    _nCoordsAlloc = n_coords;
  }

}

///////////////////////
// _computeProjPolygon
//

void Area::_computeProjPolygon(const GridClump &grid_clump)

{
  
  // compute the proj area centroid as a reference point for
  // the shape star - we add 0.5 because the star is computed
  // relative to the lower-left corner of the grid, while the
  // other computations are relative to the center of the
  // grid rectangles.
  
  double dx = grid_clump.grid.dx;
  double dy = grid_clump.grid.dy;

  double ref_x =
    (0.5 + ((_gProps->proj_area_centroid_x - grid_clump.grid.minx) / dx) - 
     grid_clump.startIx);
  
  double ref_y =
    (0.5 + ((_gProps->proj_area_centroid_y - grid_clump.grid.miny) / dy) - 
     grid_clump.startIy);
  
  // compute the boundary

  double *radii = _boundary.computeRadii(_nX, _nY, _compGrid, 1,
					 N_POLY_SIDES, ref_x, ref_y);

  fl32 *poly = _gProps->proj_area_polygon;
  for (int i = 0; i < N_POLY_SIDES; i++) {
    poly[i] = radii[i];
  }

}

///////////////////////////////
// compute precip

void Area::_computePrecip(const GridClump &grid_clump)
     
{

  // initialize dbz grid for precip

  fl32 dbzMiss = _inputMdv.dbzMiss;
  int ii = 0;
  for (int iy = 0; iy < _nY; iy++) {
    for (int ix = 0; ix < _nX; ix++, ii++) {
      _dbzForPrecip[ii] = dbzMiss;
    }
  }

  // load up dbz grid for precip, depending on mode

  int nPointsPlane = _inputMdv.grid.nx * _inputMdv.grid.ny;
  Params::precip_mode_t precipMode = _params.precip_computation_mode;
  int nZ = _inputMdv.grid.nz;

  ii = 0;
  for (int iy = 0; iy < _nY; iy++) {
    
    int mm = (iy + grid_clump.startIy) * _inputMdv.grid.nx +
      + grid_clump.startIx;
    
    for (int ix = 0; ix < _nX; ix++, ii++, mm++) {

      if (!_precipGrid[ii]) {
        // only for points which have been tagged as within the grid
        continue;
      }
      
      if (precipMode == Params::PRECIP_FROM_COLUMN_MAX) {
        
	for (int iz = 0; iz < nZ; iz++) {
          int jj = iz * nPointsPlane + mm;
          fl32 dbz = _inputMdv.dbzVol[jj];
	  if (dbz != dbzMiss) {
            if (dbz > _dbzForPrecip[ii]) {
              _dbzForPrecip[ii] = dbz;
            }
	  }
	} // iz

      } else if (precipMode == Params::PRECIP_FROM_LOWEST_AVAILABLE_REFL) {
        
	// starting at the bottom, compute precip from the first non-missing
	// reflectivity point in the column
	
	for (int iz = 0; iz < nZ; iz++) {
          int jj = iz * nPointsPlane + mm;
          fl32 dbz = _inputMdv.dbzVol[jj];
	  if (dbz != dbzMiss) {
            _dbzForPrecip[ii] = dbz;
	    break;
	  }
	} // iz

      } else {
        
        int jj = _inputMdv.specifiedPrecipLayer * nPointsPlane + mm;
        _dbzForPrecip[ii] = _inputMdv.dbzVol[jj];
        
      }

    } // ix

  } // iy

  // accumulate precip

  double sum_factor = 0.0;
  ii = 0;
  for (int iy = 0; iy < _nY; iy++) {
    for (int ix = 0; ix < _nX; ix++, ii++) {
      if (_dbzForPrecip[ii] != dbzMiss) {
	double dbz = _dbzForPrecip[ii];
        double refl = pow(10.0, dbz / 10.0);
	double precip_flux_factor = pow(refl, _zPInverseExpon);
	sum_factor += precip_flux_factor;
      }
    }
  }

  // compute precip flux in m/s
  
  _gProps->precip_flux =
    ((sum_factor * grid_clump.dAreaAtCentroid * _zFactor) / 3.6);

}

///////////////////////////////////////////////////////////
// precip 2D dbz histogram, based on dbz values for precip
//

void Area::_compute2dDbzHist(dbz_hist_entry_t *dbz_hist)
     
{

  fl32 dbzMiss = _inputMdv.dbzMiss;
  double lowDbzThreshold = _params.low_dbz_threshold;
  double histInterval = _params.dbz_hist_interval;
  double n = 0.0;
  
  int ii = 0;
  for (int iy = 0; iy < _nY; iy++) {
    for (int ix = 0; ix < _nX; ix++, ii++) {
      fl32 dbz = _dbzForPrecip[ii];
      if (dbz != dbzMiss) {
	int dbz_intvl = (int) ((dbz - lowDbzThreshold) / histInterval);
	if (dbz_intvl >= 0 && dbz_intvl < _nDbzHistIntervals) {
	  dbz_hist[dbz_intvl].n_area++;
	  n++;
	}
      }
    }
  }

  // load area dbz histograms
  
  for (int ii = 0; ii < _nDbzHistIntervals; ii++) {
    if (dbz_hist[ii].n_area > 0) {
      dbz_hist[ii].percent_area = 100.0 *
	(double) dbz_hist[ii].n_area / n;
    } else {
      dbz_hist[ii].percent_area = 0.0;
    }
  }

}

/////////////////////////////////////////////////////
// compute storm tops based on 'tops_dbz_threshold'
//

void Area::_computeTops(const GridClump &grid_clump)
     
{

  double top = 0.0;
  int nPointsPlane = _inputMdv.grid.nx * _inputMdv.grid.ny;
  
  for (int iz = 0; iz < _inputMdv.grid.nz; iz++) {
    
    double ht = _inputMdv.grid.minz + (iz + 0.5) * _inputMdv.grid.dz;
    const fl32 *dbzPlane = _inputMdv.dbzVol + iz * nPointsPlane;
    ui08 *flag = _compGrid;

    // compute max dbz at this ht
    
    double maxDbzInLayer = -9999;
    for (int iy = 0; iy < _nY; iy++) {

      int index =
        (iy + grid_clump.startIy) * _inputMdv.grid.nx + grid_clump.startIx;

      const fl32 *dbz = dbzPlane + index;
      for (int ix = 0; ix < _nX; ix++, flag++, dbz++) {
        
        // if this point is in the composite shape, use it
        
        if (*flag) {
          if (*dbz >= _params.tops_dbz_threshold) {
            if (*dbz > maxDbzInLayer) {
              maxDbzInLayer = *dbz;
            }
          }
        } // if(*flag)
        
      } // ix
    } // iy

    if (maxDbzInLayer >= _params.tops_dbz_threshold) {
      top = ht;
    }

  } // iz

  _gProps->top = top;
  
}

