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
// Grid.cc
//
// Grid object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include "Grid.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/utim.h>
#include <toolsa/pjg.h>
#include <Mdv/mdv/mdv_read.h>
using namespace std;

//////////////
// Constructor

Grid::Grid(const char *prog_name, Params *params,
	   const char *label, const char *grid_file_path,
	   int field_num, double plane_ht)

{

  OK = TRUE;
  _params = params;
  byteArray = NULL;
  dataArray = NULL;
  rangeArray = NULL;

  _progName = STRdup(prog_name);
  _gridFilePath = STRdup(grid_file_path);
  _label = STRdup(label);

  // init the MDV handle

  MDV_handle_t mdv;
  MDV_init_handle(&mdv);

  // read in grid file
  
  if (MDV_read_all(&mdv, _gridFilePath, MDV_INT8)) {
    fprintf(stderr, "ERROR - %s:Grid::Grid\n", _progName);
    fprintf(stderr, "Cannot read grid file %s\n", _gridFilePath);
    OK = FALSE;
    return;
  }
  
  // set times
  
  MDV_master_header_t *mhdr = &mdv.master_hdr;
  timeStart = mhdr->time_begin;
  timeEnd = mhdr->time_end;
  timeCent = mhdr->time_centroid;

  // get plane number for grid data
  
  int plane_num;
  double actual_ht;

  if (plane_ht < 0) {
    plane_num = -1;
    actual_ht = plane_ht;
  } else {
    if (MDV_field_ht_to_num(&mdv, field_num, plane_ht,
			  &plane_num, &actual_ht)) {
      fprintf(stderr, "ERROR - %s:Grid::Grid\n", _progName);
      fprintf(stderr, "Cannot set plane num for data.\n");
      OK = FALSE;
      MDV_free_handle(&mdv);
      return;
    }
  }
  if (_params->debug >= Params::DEBUG_VERBOSE) {
    fprintf(stderr, "%s: plane_num, actual_ht: %d, %g\n",
	    label, plane_num, actual_ht);
  }

  // check that field number is valid
  
  if (field_num > mdv.master_hdr.n_fields - 1) {
    fprintf(stderr, "ERROR - %s:Grid::Grid\n", _progName);
    fprintf(stderr, "File '%s'\n", _gridFilePath);
    fprintf(stderr, "File has %d fields\n", mdv.master_hdr.n_fields);
    fprintf(stderr, "Grid field number %d out of range\n", field_num);
    OK = FALSE;
    MDV_free_handle(&mdv);
    return;
  }

  // load grid

  if (OK) {
    _load(field_num, plane_num, &mdv);
  }
  
  // free up

  MDV_free_handle(&mdv);

}

/////////////
// destructor

Grid::~Grid()

{

  // free strings

  STRfree(_progName);
  STRfree(_gridFilePath);
  STRfree(_label);

  // free grid arrays

  //  if (!(_params->output_intermediate_grids))
  _freeGrids();

}

///////////////
// _freeGrids()

void Grid::_freeGrids()

{

  if (byteArray != NULL) {
    ufree(byteArray);
    byteArray = NULL;
  }

  if (dataArray != NULL) {
    ufree(dataArray);
    dataArray = NULL;
  }

  if (rangeArray != NULL) {
    ufree(rangeArray);
    rangeArray = NULL;
  }

}

/////////////////
// checkGeom()
//
// returns 0 on success (grids are the same)
// and -1 on error (grids differ)
//

int Grid::checkGeom(Grid *other)

{

  if (nx != other->nx ||
      ny != other->ny ||
      dx != other->dx ||
      dy != other->dy ||
      minx != other->minx ||
      miny != other->miny) {

    return (-1);

  }

  if (fabs(originLat - other->originLat) > 0.0001) {
    return (-1);
  }

  if (fabs(originLon - other->originLon) > 0.0001) {
    return (-1);
  }

  return (0);

}

//////////////
// printGeom()
//
// print out grid geometry
//

void Grid::printGeom(FILE *out, const char *spacer)

{

  fprintf(out, "%snx: %d\n", spacer, nx);
  fprintf(out, "%sny: %d\n", spacer, ny);
  fprintf(out, "%sdx: %g\n", spacer, dx);
  fprintf(out, "%sdy: %g\n", spacer, dy);
  fprintf(out, "%sminx: %g\n", spacer, minx);
  fprintf(out, "%sminy: %g\n", spacer, miny);
  fprintf(out, "%soriginLat: %g\n", spacer, originLat);
  fprintf(out, "%soriginLon: %g\n", spacer, originLon);

}

/////////////////
// _load()
//

void Grid::_load(int field_num, int plane_num,
		 MDV_handle_t *mdv)

{

  MDV_field_header_t *fhdr = mdv->fld_hdrs + field_num;

  if (_params->debug) {
    cerr << _label << " grid, field name: " << fhdr->field_name << endl;
  }

  // set grid params
  
  if (_params->use_specified_grid) {

    nx = _params->specified_grid.nx;
    ny = _params->specified_grid.ny;
    minx = _params->specified_grid.minx;
    miny = _params->specified_grid.miny;
    dx = _params->specified_grid.dx;
    dy = _params->specified_grid.dy;
    originLat = _params->specified_grid.origin_lat;
    originLon = _params->specified_grid.origin_lon;

  } else {

    nx = fhdr->nx;
    ny = fhdr->ny;
    minx = fhdr->grid_minx;
    miny = fhdr->grid_miny;
    dx = fhdr->grid_dx;
    dy = fhdr->grid_dy;
    originLat = fhdr->proj_origin_lat;
    originLon = fhdr->proj_origin_lon;

  }

  npoints = nx * ny;

  // allocate data array and range array

  _freeGrids();
  byteArray = (ui08 *) ucalloc (npoints, sizeof(ui08));
  dataArray = (float *) ucalloc (npoints, sizeof(float));
  rangeArray = (float *) ucalloc (npoints, sizeof(float));

  // set up pointer to input grid. If plane_num is -1,
  // compute the composite and put into first plane

  ui08 *plane;
    
  if (plane_num < 0) {
    
    // set plane to lowest
    
    plane = (ui08 *) mdv->field_plane[field_num][0];
    
    // compute composite and put in first plane
    
    for (int i = 1; i < fhdr->nz; i++) {
      ui08 *pi = (ui08 *) mdv->field_plane[field_num][i];
      ui08 *pp = (ui08 *) plane;
      for (int j = 0; j < fhdr->nx * fhdr->ny; j++, pi++, pp++) {
	if (*pi > *pp) {
	  *pp = *pi;
	}
      } // j
    } // i
    
  } else {
    
    plane = (ui08 *) mdv->field_plane[field_num][plane_num];
    
  }

  // Save scale, bias, missing and  for the output of intermediate fields.
  Scale = fhdr->scale; Bias = fhdr->bias;
  Missing = fhdr->missing_data_value; 
  Bad = fhdr->bad_data_value;

  // compute sensor location relative to grid

  double radarX, radarY;
  PJGLatLon2DxDy(fhdr->proj_origin_lat, fhdr->proj_origin_lon,
		 mdv->master_hdr.sensor_lat, mdv->master_hdr.sensor_lon,
		 &radarX, &radarY);

  // load up grid in specified manner
    
  if (_params->use_specified_grid) {
    if (_params->load_means_in_specified_grid) {
      _loadMeans(plane, fhdr, radarX, radarY);
    } else {
      _loadNearest(plane, fhdr, radarX, radarY);
    }
  } else {
    _loadNative(plane, fhdr, radarX, radarY);
  }

}

/////////////////
// _loadNative()
//
// Load up native grid
//

void Grid::_loadNative(ui08 *plane,
		       MDV_field_header_t *fhdr,
		       double radarX, double radarY)
  
{

  double scale = fhdr->scale;
  double bias = fhdr->bias;
  int missing_val = (int) fhdr->missing_data_value;

  ui08 *pp = plane;
  ui08 *bp = byteArray;
  float *dp = dataArray;
  float *rp = rangeArray;
      
  double y = miny;
  double rely = y - radarY;
      
  for (int iy = 0; iy < ny; iy++, y += dy, rely += dy) {
    
    double x = minx;
    double rely = x - radarX;
    
    for (int ix = 0; ix < nx;
	 ix++, x += dx, rely += dx, pp++, bp++, dp++, rp++) {
      
      *bp = *pp;
      if (*pp == missing_val) {
	*dp = VGRID_MISSING;
      } else {
	*dp = *pp * scale + bias;
      }
      *rp = sqrt(rely * rely + rely * rely);
      
    } // ix
    
  } // iy

}

/////////////////
// _loadNearest()
//
// Load up grid using nearest neighbor
//

void Grid::_loadNearest(ui08 *plane,
			MDV_field_header_t *fhdr,
			double radarX, double radarY)
  
{

  double scale = fhdr->scale;
  double bias = fhdr->bias;
  int missing_val = (int) fhdr->missing_data_value;

  // compute grid offsets

  double offsetX, offsetY;
  PJGLatLon2DxDy(fhdr->proj_origin_lat, fhdr->proj_origin_lon,
		 originLat, originLon,
		 &offsetX, &offsetY);

  // resample grid using nearest neighbor

  // compute yy rel to original grid

  double yy = miny + offsetY;
  double rely = yy - radarY;

  ui08 *bp = byteArray;
  float *dp = dataArray;
  float *rp = rangeArray;

  int npoints_native = fhdr->nx * fhdr->ny;
      
  for (int iy = 0; iy < ny; iy++, yy += dy, rely += dy) {
    
    // compute xx rel to original grid

    double xx = minx + offsetX;
    double relx = xx - radarX;
    
    for (int ix = 0; ix < nx; ix++, xx += dx, relx += dx, bp++, dp++, rp++) {
      
      // compute index in original grid
      
      int jx = (int)(((xx - fhdr->grid_minx) / fhdr->grid_dx) + 0.5);
      int jy = (int)(((yy - fhdr->grid_miny) / fhdr->grid_dy) + 0.5);
      int jj = jy * fhdr->nx + jx;

      if (jj >= 0 && jj < npoints_native) {

	ui08 bval = plane[jj];
	*bp = bval;
	if (bval == missing_val) {
	  *dp = VGRID_MISSING;
	} else {
	  *dp = bval * scale + bias;
	}
	*rp = sqrt(rely * rely + rely * rely);

      } // if (jj >= 0 && jj < npoints) 
      
    } // ix
    
  } // iy

}

///////////////
// _loadMeans()
//
// Load up grid using means sampled from native grid
//

void Grid::_loadMeans(ui08 *plane,
		      MDV_field_header_t *fhdr,
		      double radarX, double radarY)

{

  double scale = fhdr->scale;
  double bias = fhdr->bias;
  int missing_val = (int) fhdr->missing_data_value;

  // compute grid offsets
  
  double offsetX, offsetY;
  PJGLatLon2DxDy(fhdr->proj_origin_lat, fhdr->proj_origin_lon,
		 originLat, originLon,
		 &offsetX, &offsetY);
  
  // compute means and load into grid
  
  double y = miny + offsetY;
  double rely = y - radarY;

  ui08 *bp = byteArray;
  float *dp = dataArray;
  float *rp = rangeArray;
      
  double high_y = miny + offsetY - 0.5 * dy;
  int low_iy;
  int high_iy = (int) floor ((high_y - fhdr->grid_miny) / fhdr->grid_dy);
      
  for (int iy = 0; iy < ny; iy++, y += dy, rely += dy) {
    
    // compute native y limits for this row
    
    low_iy = high_iy + 1;
    high_y += dy;
    high_iy = (int) floor ((high_y - fhdr->grid_miny) / fhdr->grid_dy);
    
    double x = minx + offsetX;
    double rely = x - radarX;
    
    double high_x = minx + offsetX - 0.5 * dx;
    int low_ix;
    int high_ix = (int) floor ((high_x - fhdr->grid_minx) / fhdr->grid_dx);
    
    for (int ix = 0; ix < nx; ix++, x += dx, rely += dx, bp++, dp++, rp++) {
      
      // compute native x limits for this row
      
      low_ix = high_ix + 1;
      high_x += dx;
      high_ix = (int) floor ((high_x - fhdr->grid_minx) / fhdr->grid_dx);
      
      double mean = VGRID_MISSING;
      
      if (low_ix >= 0 && high_ix < fhdr->nx) {
	
	// loop through the relevant portion of the data
	// plane, computing the mean
	
	double n = 0.0;
	double sum = 0.0;
	
	for (int jy = low_iy; jy <= high_iy; jy++) {
	  ui08 *pp = plane + jy * fhdr->nx + low_ix;
	  for (int jx = low_ix; jx <= high_ix; jx++, pp++) {
	    if (*pp != missing_val) {
	      double data_val = (double) *pp * scale + bias;
	      n++;
	      sum += data_val;
	    }
	  } /* jx */
	} /* jy */
	
	// compute mean
	
	if (n > 0) {
	  mean = sum / n;
	}
	
      } // if (low_ix >= 0 && high_ix < fhdr->nx) 
      
      // compute index in original grid

      if (mean == VGRID_MISSING) {
	*bp = missing_val;
	*dp = VGRID_MISSING;
      } else {
	int byteval = (int) ((mean - bias) / scale + 0.5);
	byteval = MIN(1, byteval);
	byteval = MAX(254, byteval);
	*bp = byteval;
	*dp = mean;
      }
      *rp = sqrt(rely * rely + rely * rely);
      
    } // ix
    
  } // iy

}

  
    
