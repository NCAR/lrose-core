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

#include "Area.hh"
#include "Props.hh"
#include "Boundary.hh"
#include "GridClump.hh"
#include "StormFile.hh"
#include "InputMdv.hh"

#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <rapmath/umath.h>
using namespace std;

#define MAX_EIG_DIM 3

//////////////
// constructor
//

Area::Area(char *prog_name, Params *params,
	   InputMdv *input_mdv, StormFile *storm_file)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;
  _inputMdv = input_mdv;
  _stormFile = storm_file;
  _boundary = new Boundary(_progName);

  // grids

  _nGridAlloc = 0;
  _compGrid = NULL;
  _precipGrid = NULL;

  // ellipse comps

  _means = (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvalues =
    (double *) umalloc (MAX_EIG_DIM * sizeof(double));
  _eigenvectors =
    (double **) umalloc2 (MAX_EIG_DIM, MAX_EIG_DIM, sizeof(double));
  _areaCoords = (double **) umalloc2 (1, 2, sizeof(double));
  _nCoordsAlloc = 1;

}

/////////////
// destructor
//

Area::~Area()

{

  if (_boundary) {
    delete (_boundary);
  }

  if (_compGrid) {
    ufree(_compGrid);
  }

  if (_precipGrid) {
    ufree(_precipGrid);
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

  STRfree(_progName);

}

////////////
// init()
//
// Initialize some variables for efficiency
//

void Area::init(double dbz_scale,
		double dbz_bias,
		double z_p_inverse_coeff,
		double z_p_inverse_exponent)

{

  _dbzScale = dbz_scale;
  _dbzBias = dbz_bias;
  _zPInverseCoeff = z_p_inverse_coeff;
  _zPInverseExpon = z_p_inverse_exponent;
  _zFactor = pow(_zPInverseCoeff, _zPInverseExpon);

}

////////////
// compute()
//

void Area::compute(GridClump *grid_clump,
		   storm_file_global_props_t *gprops,
		   dbz_hist_entry_t *dbz_hist)

{

  _gridClump = grid_clump;
  _gProps = gprops;
  int multiple_layers = FALSE;

  _gProps->bounding_min_ix = _gridClump->startIx;
  _gProps->bounding_min_iy = _gridClump->startIy;
  _gProps->bounding_max_ix = _gridClump->startIx + _gridClump->nX - 1;
  _gProps->bounding_max_iy = _gridClump->startIy + _gridClump->nY - 1;

  // compute grid sizes, and set grid params

  _nX = _gridClump->nX;
  _nY = _gridClump->nY;
  _nPoints = _nX * _nY;

  // check memory allocation and zero out grids

  _allocGrids();

  // load up grid with 1's to indicate projected area

  Interval *intvl = _gridClump->intervals;
  for (int intv = 0; intv < _gridClump->nIntervals; intv++, intvl++) {
    if (intvl->plane != 0) {
      multiple_layers = TRUE;
    }
    int offset = (intvl->row_in_plane * _nX) + intvl->begin;
    memset((_compGrid + offset), 1, intvl->len);
  }
  
  // projected area comps
  
  _ellipseCompute(_compGrid,
		  &_gProps->proj_area,
		  &_gProps->proj_area_centroid_x,
		  &_gProps->proj_area_centroid_y,
		  &_gProps->proj_area_orientation,
		  &_gProps->proj_area_major_radius,
		  &_gProps->proj_area_minor_radius);

  // compute polygon for projected area
  
  _computeProjPolygon();
  
  if (multiple_layers) {

    // load up grid with 1's to indicate precip area
    
    Interval *intvl = _gridClump->intervals;
    for (int intv = 0; intv < _gridClump->nIntervals; intv++, intvl++) {
      if (intvl->plane == 0) {
	int offset =
	  (intvl->row_in_plane * _nX) + intvl->begin;
	memset((void *) (_precipGrid + offset), 1, (int) intvl->len);
      }
    }
    
    // precip area comps
    
    _ellipseCompute(_precipGrid,
		    &_gProps->precip_area,
		    &_gProps->precip_area_centroid_x,
		    &_gProps->precip_area_centroid_y,
		    &_gProps->precip_area_orientation,
		    &_gProps->precip_area_major_radius,
		    &_gProps->precip_area_minor_radius);
    
  } else { // if (multiple_layers) 

    // only one layer - copy from proj area
    
    _gProps->precip_area = _gProps->proj_area;
    _gProps->precip_area_centroid_x = _gProps->proj_area_centroid_x;
    _gProps->precip_area_centroid_y = _gProps->proj_area_centroid_y;
    _gProps->precip_area_orientation = _gProps->proj_area_orientation;
    _gProps->precip_area_major_radius = _gProps->proj_area_major_radius;
    _gProps->precip_area_minor_radius = _gProps->proj_area_minor_radius;
    
  } // if (multiple_layers) 

  // compute precip from composite, and load up
  // dbz histogram for composite area

  _precipAndAreaHist(dbz_hist);

}

//////////////////////////////////////////////////////////
// storeProjRuns()
//
// Store the projected area runs in the storm file handle

int Area::storeProjRuns()

{

  int nIntervals = _boundary->nIntervals();

  RfAllocStormProps(&_stormFile->handle, _inputMdv->grid.nz,
		    _stormFile->nDbzHistIntervals,
		    0, nIntervals,
		    "props_compute");
  
  int start_ix = _gridClump->startIx;
  int start_iy = _gridClump->startIy;
  
  Interval *intvl = _boundary->intervals();
  storm_file_run_t *run = _stormFile->handle.proj_runs;
  
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
  }
  _nGridAlloc = _nPoints;
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

void Area::_ellipseCompute(ui08 *grid,
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

  int max_coords = _gridClump->grid.nx * _gridClump->grid.ny;
  _allocCoords(max_coords);

  // load up coords

  int n_coords = 0;
  double dy = _gridClump->grid.dy;
  double dx = _gridClump->grid.dx;
  double yy = _gridClump->grid.miny + _gridClump->startIy * dy;
  for (int iy = 0; iy < _nY; iy++, yy += dy) {
    double xx = _gridClump->grid.minx + _gridClump->startIx * dx;
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
    
    *area = (double) n_coords * _gridClump->dAreaAtCentroid;  
    double area_ellipse = (double) n_coords * _gridClump->dAreaEllipse;
    
    // obtain the principal component transformation for the coord data
    // The technique is applicable here because the first principal
    // component will lie along the axis of maximum variance, which
    // is equivalent to fitting a line through the data points,
    // minimizing the sum of the sguared perpendicular distances
    // from the data to the line.
    
    if (upct(2, n_coords, _areaCoords,
	     _means, _eigenvectors, _eigenvalues) != 0) {
      
      fprintf(stderr, "WARNING - %s:Area::_ellipseCompute\n", _progName);
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
      // line up in a straight line - use circle
      
      if (*area_major_radius / *area_minor_radius > 1000.0) {
	
	*area_major_radius  = sqrt(area_ellipse / M_PI);
	*area_minor_radius  = *area_major_radius;
	
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

void Area::_computeProjPolygon()

{
  
  // compute the proj area centroid as a reference point for
  // the shape star - we add 0.5 because the star is computed
  // relative to the lower-left corner of the grid, while the
  // other computations are relative to the center of the
  // grid rectangles.
  
  double dx = _gridClump->grid.dx;
  double dy = _gridClump->grid.dy;

  double ref_x =
    (0.5 + ((_gProps->proj_area_centroid_x - _gridClump->grid.minx) / dx) - 
     _gridClump->startIx);
  
  double ref_y =
    (0.5 + ((_gProps->proj_area_centroid_y - _gridClump->grid.miny) / dy) - 
     _gridClump->startIy);
  
  // compute the boundary

  double *radii = _boundary->computeRadii(_nX, _nY, _compGrid, 1,
					  N_POLY_SIDES, ref_x, ref_y);

  fl32 *poly = _gProps->proj_area_polygon;
  for (int i = 0; i < N_POLY_SIDES; i++) {
    poly[i] = radii[i];
  }

}

///////////////////////
// _precipAndAreaHist()
//
// compute precip and area dbz histogram
//

void Area::_precipAndAreaHist(dbz_hist_entry_t *dbz_hist)
     
{

  ui08 *comp_dbz = _inputMdv->compDbz;
  ui08 *cg = _compGrid;
  
  int *dbzInterval = _inputMdv->dbzInterval;
  double n = 0.0;
  double sum_factor = 0.0;

  for (int iy = 0; iy < _nY; iy++) {
    
    int index = (iy + _gridClump->startIy) * _inputMdv->grid.nx +
      + _gridClump->startIx;
    ui08 *dbz = comp_dbz + index;

    for (int ix = 0; ix < _nX; ix++, cg++, dbz++) {
      
      // if this point is in the composite shape, add
      // in the precip factor
      
      if (*cg) {

	double r_dbz = ((double) *dbz) * _dbzScale + _dbzBias;
	double refl = pow(10.0, r_dbz / 10.0);
	double precip_flux_factor = pow(refl, _zPInverseExpon);
	sum_factor += precip_flux_factor;
	
	// load up area dbz histogram counts
	
	int dbz_intvl = dbzInterval[*dbz];
	if (dbz_intvl >= 0) {
	  dbz_hist[dbz_intvl].n_area++;
	  n++;
	}
	
      }

    } // ix

  } // iy
  
  // compute precip flux in m/s
  
  _gProps->precip_flux =
    ((sum_factor * _gridClump->dAreaAtCentroid * _zFactor) / 3.6);
  
  // load area dbz histograms
  
  for (int ii = 0; ii < _stormFile->nDbzHistIntervals; ii++) {
    if (dbz_hist[ii].n_area > 0) {
      dbz_hist[ii].percent_area = 100.0 *
	(double) dbz_hist[ii].n_area / n;
    } else {
      dbz_hist[ii].percent_area = 0.0;
    }
  }

}
