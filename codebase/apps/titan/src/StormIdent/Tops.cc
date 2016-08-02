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
// Tops.cc
//
// Tops class
//
// Masks out radar data which does not have the specified tops.
// Optionally writes out tops-related data to MDV files for
// debugging.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "Tops.hh"
#include "InputMdv.hh"
#include "OutputMdv.hh"
#include "Clumping.hh"
#include <toolsa/umisc.h>
#include <toolsa/str.h>
using namespace std;

#define HEIGHT_SCALE 0.1
#define COUNT_SCALE  1.0

//////////////
// constructor
//

Tops::Tops(char *prog_name, Params *params)

{
  
  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

  _clumping = new Clumping(_progName);

  _nBytesGridAlloc = 0;

  _topsGrid = NULL;
  _shallowGrid = NULL;
  _shallowEdmGrid = NULL;
  _shallowMaskGrid = NULL;

}

/////////////
// destructor
//

Tops::~Tops()

{

  STRfree(_progName);

  if (_clumping) {
    delete (_clumping);
  }

  if (_topsGrid) {
    ufree(_topsGrid);
  }
  if (_shallowGrid) {
    ufree(_shallowGrid);
  }
  if (_shallowEdmGrid) {
    ufree(_shallowEdmGrid);
  }
  if (_shallowMaskGrid) {
    ufree(_shallowMaskGrid);
  }

}

////////////////////////////////////////
// maskLowTops()
//
// Mask out areas in grid with low tops

void Tops::maskLowTops(InputMdv *input_mdv)

{

  mdv_grid_t *grid = &input_mdv->handle.grid;
  int npoints_per_plane = grid->nx * grid->ny;

  // check memory allocation

  _allocGrids(input_mdv);
  
  // load tops grid

  int dbz_threshold = input_mdv->lowDbzByte;

  for (int iz = 0; iz < grid->nz; iz++) {

    double plane_height = grid->minz + grid->dz * iz;
    
    int height_byte_val = (int) (plane_height / HEIGHT_SCALE + 0.5);
    if (height_byte_val > 255) {
      height_byte_val = 255;
    }
    
    ui08 *tops = _topsGrid;
    ui08 *dbz = input_mdv->dbzPlanes[iz];
    
    for (int i = 0; i < npoints_per_plane; i++, tops++, dbz++) {
      if (*dbz > dbz_threshold) {
	*tops = height_byte_val;
      }
    } /* i */
    
  } /* iz */
  
  // load up grid with only shallow tops

  int height_byte_threshold =
    (int) (_params->min_radar_tops / HEIGHT_SCALE + 0.5);
  if (height_byte_threshold > 255) {
    height_byte_threshold = 255;
  }
  
  ui08 *tops = _topsGrid;
  ui08 *shallow = _shallowGrid;
  for (int i = 0; i < npoints_per_plane; i++, tops++, shallow++) {
    if (*tops < height_byte_threshold) {
      *shallow = *tops;
    }
  }
  
  // get the intervals in shallow grid
  
  int num_intervals = _clumping->findIntervals(grid->nx, grid->ny,
					       _shallowGrid, 1);
  
  if (num_intervals == 0 && !_params->create_tops_files) {
    return;
  }

  // compute the edm of the shallow field
  
  _clumping->edm2D(grid->nx, grid->ny, _shallowEdmGrid);
  
  // threshold the edm field to get mask field
  
  {

    double mean_grid_res = (grid->dx + grid->dy) / 2.0;
    int edm_grid_threshold =
      (int) ((_params->tops_edge_margin / mean_grid_res) + 0.5);

    ui08 *shallow_edm = _shallowEdmGrid;
    ui08 *shallow_mask = _shallowMaskGrid;
    for (int i = 0; i < npoints_per_plane;
	 i++, shallow_edm++, shallow_mask++) {
      if (*shallow_edm >= edm_grid_threshold) {
	*shallow_mask = 1;
      }
    } // i
  }

  // mask out reflectivity data for areas of low tops in 
  // the dbz volume

  for (int iz = 0; iz < grid->nz; iz++) {
    
    ui08 *dbz = input_mdv->dbzPlanes[iz];
    ui08 *shallow_mask = _shallowMaskGrid;
    
    for (int i = 0; i < npoints_per_plane; i++, dbz++, shallow_mask++) {
      if (*shallow_mask) {
	*dbz = 0;
      }
    } // i
    
  } // iz
  
  // if required, write out tops file for debugging
  
  if (_params->create_tops_files) {
    _writeOutputMdv(input_mdv);
  }

  return;

}

/////////////////
// _allocGrids()

void Tops::_allocGrids(InputMdv *input_mdv)

{

  int nbytesPlane =
    input_mdv->handle.grid.nx * input_mdv->handle.grid.ny * sizeof(ui08);
  
  // adjust grid allocation as necessary
  
  if (nbytesPlane > _nBytesGridAlloc) {
    
    _topsGrid = (ui08 *) urealloc (_topsGrid, nbytesPlane);
    _shallowGrid = (ui08 *) urealloc (_shallowGrid, nbytesPlane);
    _shallowEdmGrid = (ui08 *) urealloc (_shallowEdmGrid, nbytesPlane);
    _shallowMaskGrid = (ui08 *) urealloc (_shallowMaskGrid, nbytesPlane);
      
    _nBytesGridAlloc = nbytesPlane;

  } //if (nbytesPlane < _nBytesGridAlloc)
    
  // zero out grids

  memset(_topsGrid, 0, _nBytesGridAlloc);
  memset(_shallowEdmGrid, 0, _nBytesGridAlloc);
  memset(_shallowGrid, 0, _nBytesGridAlloc);
  memset(_shallowMaskGrid, 0, _nBytesGridAlloc);

}

////////////////////
// _writeOutputMdv()
//

int Tops::_writeOutputMdv(InputMdv *input_mdv)

{

  // create output MDV object

  char note[256];
  
  sprintf(note,
	  "\nTops data, dbz threshold %g, "
	  "min_radar_tops %g, tops_edge_margin %g", 
	  _params->low_dbz_threshold, _params->min_radar_tops,
	  _params->tops_edge_margin);
  
  OutputMdv *out = new OutputMdv(_progName, _params,
				 input_mdv, N_TOPS_FIELDS, note,
				 _params->tops_dir);
  
  if (!out) {
    return (-1);
  }
    
  // fill in output file field info
  
  out->fillFieldHeader(TOPS_FIELD, "Storm tops", "Tops",
		       "km", "none");
  out->fillFieldHeader(SHALLOW_FIELD, "Shallow echo", "Shallow echo",
		       "km", "none");
  out->fillFieldHeader(SHALLOW_EDM_FIELD, "Shallow edm", "Shallow edm",
		       "count", "none");
  out->fillFieldHeader(SHALLOW_MASK_FIELD, "Shallow mask", "Shallow mask",
		       "count", "none");
  
  // load field data
  
  out->loadFieldData(TOPS_FIELD, _topsGrid, HEIGHT_SCALE, 0.0);
  out->loadFieldData(SHALLOW_FIELD, _shallowGrid, HEIGHT_SCALE, 0.0);
  out->loadFieldData(SHALLOW_EDM_FIELD, _shallowEdmGrid, COUNT_SCALE, 0.0);
  out->loadFieldData(SHALLOW_MASK_FIELD, _shallowMaskGrid, COUNT_SCALE, 0.0);

  // write out file

  out->writeVol();

  // clean up

  delete (out);
  
  return(0);

}


