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
// InputMdv.cc
//
// InputMdv class
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 1998
//
///////////////////////////////////////////////////////////////

#include "InputMdv.hh"
#include "Tops.hh"
#include <toolsa/str.h>
#include <mdv/mdv_read.h>
using namespace std;

//////////////
// constructor
//

InputMdv::InputMdv(char *prog_name, Params *params)

{

  OK = TRUE;
  _progName = STRdup(prog_name);
  _params = params;

  if (MDV_init_handle(&handle)) {
    OK = FALSE;
    return;
  }

  _prevDbzScale = -1.0;
  _prevDbzBias = 0.0;

  _compDbz = NULL;
  _ncompDbzAlloc = 0;

  _dbzPlanes = (ui08 **) NULL;
  _velPlanes = (ui08 **) NULL;
  _nPlanesAlloc = 0;
  _nBytesPlaneAlloc = 0;

  // create radar tops object

  // if required, mask out areas with low tops
  
  if (_params->check_tops) {
    _tops = new Tops(_progName, _params);
    if (!_tops->OK) {
      OK = FALSE;
      return;
    }
  } else {
    _tops = NULL;
  }

}

/////////////
// destructor
//

InputMdv::~InputMdv()

{

  if (_dbzPlanes) {
    ufree2((void **) _dbzPlanes);
  }
  
  if (_velPlanes) {
    ufree2((void **) _velPlanes);
  }

  if (_tops) {
    delete(_tops);
  }
  if (_compDbz) {
    ufree(_compDbz);
  }
  MDV_free_handle(&handle);
  STRfree(_progName);

}

///////////////////////////////////////
// read()
//
// Read in file for given time
//
// returns 0 on success, -1 on failure
//

int InputMdv::read(time_t data_time)

{

  // compute file path

  date_time_t dtime;
  dtime.unix_time = data_time;
  uconvert_from_utime(&dtime);

  char filePath[MAX_PATH_LEN];
  sprintf(filePath, "%s%s%.4d%.2d%.2d%s%.2d%.2d%.2d.%s",
	  _params->rdata_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day,
	  PATH_DELIM,
	  dtime.hour, dtime.min, dtime.sec, "mdv");
  fprintf(stdout, "%s: processing file %s\n", _progName, filePath);
	  
  // read in file

  if (MDV_read_all(&handle, filePath, MDV_INT8)) {
    return (-1);
  }

  // copy the grid

  grid = handle.grid;

  // make sure dz is not zero - if it is, set to 1.0

  if (grid.dz <= 0) {
    grid.dz = 1.0;
  }

  // set number of fields

  nFields = handle.master_hdr.n_fields;

  // check that requested field numbers are valid
  
  if (_params->dbz_field > nFields - 1) {
    fprintf(stderr, "ERROR - %s:InputMdv::read\n", _progName);
    fprintf(stderr,
	    "Dbz field number %d too great - only %d fields available.\n",
	    _params->dbz_field, nFields);
    fprintf(stderr, "Remember - field numbers start at 0\n");
    return(-1);
  }
  
  if (_params->vel_available && _params->vel_field > nFields - 1) {
    fprintf(stderr, "ERROR - %s:InputMdv::read\n", _progName);
    fprintf(stderr,
	    "Vel field number %d invalid - only %d fields available.\n",
	    _params->vel_field, nFields);
    fprintf(stderr, "Remember - field numbers start at 0\n");
    return(-1);
  }
  
  // set min cartesian layer

  minValidLayer =
    (int) ((_params->base_threshold - grid.minz) / grid.dz + 0.5);
  
  if(minValidLayer < 0) {
    minValidLayer = 0;
  } else if(minValidLayer > grid.nz - 1) {
    minValidLayer = grid.nz - 1;
  }

  // set max cartesian layer at which data is considered valid
  
  maxValidLayer =
    (int) ((_params->top_threshold - grid.minz) / grid.dz + 0.5);
  
  if(maxValidLayer < 0) {
    maxValidLayer = 0;
  } else if(maxValidLayer > grid.nz - 1) {
    maxValidLayer = grid.nz - 1;
  }

  nPlanes = maxValidLayer - minValidLayer + 1;
  
  // set Dbz_byte_data
  
  double dbz_scale = handle.fld_hdrs[_params->dbz_field].scale;
  double dbz_bias = handle.fld_hdrs[_params->dbz_field].bias;
  
  lowDbzByte =
    (int) ((_params->low_dbz_threshold - dbz_bias) / dbz_scale + 0.5);
  
  highDbzByte =
    (int) ((_params->high_dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  if (_params->use_dual_threshold) {
    dualDbzByte =
      (int) ((_params->dual_threshold.dbz_threshold - dbz_bias) /
	     dbz_scale + 0.5);
  }
  
  // if scale or bias has changed, as will always be the case on the
  // first call, set the Glob->dbz_interval array. If the dbz val
  // is outside the histogram range, the index is set to -1
  
  if (dbz_scale != _prevDbzScale || dbz_bias != _prevDbzBias) {
    
    for (int i = 0; i < N_BYTE_DATA_VALS; i++) {
      
      double dbz = (double) i * dbz_scale + dbz_bias;
      
      if (dbz < _params->low_dbz_threshold ||
	  dbz >= _params->high_dbz_threshold) {

	dbzInterval[i] = -1;

      } else {

	dbzInterval[i] =
	  (int) ((dbz - _params->low_dbz_threshold) /
		 _params->dbz_hist_interval);

      }
      
    } // i
    
  } // if (dbz_scale != _prevDbzScale ...
  
  // save scale and bias values for checking next time

  _prevDbzScale = dbz_scale;
  _prevDbzBias = dbz_bias;
  dbzScale = dbz_scale;
  dbzBias = dbz_bias;

  // fill out the planes of data

  _planesFill();
  
  // if required, mask out dbz regions with low tops
  
  if (_tops) {
    _tops->maskLowTops(this);
  }
  
  // fill out the composite grid
 
  _compDbzFill();

  return (0);
  
}


///////////////////////////////
// _compDbzAlloc()
//
// allocate the composite grid
//
 
void InputMdv::_compDbzAlloc()
  
{
  
  int nbytes = grid.nx * grid.ny;
  
  if (nbytes > _ncompDbzAlloc) {
    _compDbz = (ui08 *) urealloc(_compDbz, nbytes);
    _ncompDbzAlloc = nbytes;
  }

  memset(_compDbz, 0, nbytes);

}

///////////////////////////////
// _compDbzFill()
//
// fill out the composite grid
//
 
void InputMdv::_compDbzFill()
  
{

  if (grid.nz == 1) {

    // only 1 plane - use this

    compDbz = dbzPlanes[0];
    
  } else {
    
    _compDbzAlloc();
    
    // compute the composite reflectivity grid - this is the
    // max reflectivity at any height

    int nPointsPlane = grid.nx * grid.ny;

    for (int iz = 0; iz < grid.nz; iz++) {

      ui08 *dbz = dbzPlanes[iz];
      ui08 *comp = _compDbz;
      
      for (int i = 0; i < nPointsPlane; i++, dbz++, comp++) {
	if (*dbz > *comp) {
	  *comp = *dbz;
	}
      } // i

    } // iz

    compDbz = _compDbz;

  }
    
}

///////////////////////////////
// _planesAlloc()
//
// allocate the planes
//
 
void InputMdv::_planesAlloc()
  
{
  
  int nbytesPlane = grid.nx * grid.ny;
  
  if (nbytesPlane > _nBytesPlaneAlloc ||
      grid.nz > _nPlanesAlloc) {
    
    if (_dbzPlanes) {
      ufree2((void **) _dbzPlanes);
    }
    
    if (_velPlanes) {
      ufree2((void **) _velPlanes);
    }

    // umalloc2 allocates a contiguous space for a 1D array and then
    // allocates a 2D which points into the 1D array

    _dbzPlanes = (ui08 **) umalloc2(grid.nz, nbytesPlane, sizeof(ui08));
    if (_params->vel_available) {
      _velPlanes = (ui08 **) umalloc2(grid.nz, nbytesPlane, sizeof(ui08));
    }
    _nPlanesAlloc = grid.nz;
    _nBytesPlaneAlloc = nbytesPlane;
    
  }
  
}

///////////////////////////////
// _planesFill()
//
// fill out the planes
//
 
void InputMdv::_planesFill()
  
{

  if (grid.nz == 1) {
    
    // only 1 plane - use this
    
    _dbzPlanes = (ui08 **) handle.field_plane[_params->dbz_field];
    if (_params->vel_available) {
      _velPlanes = (ui08 **) handle.field_plane[_params->vel_field];
    }

  } else {
    
    _planesAlloc();
    
    // put data into contiguous grid

    int npoints_plane = grid.nx * grid.ny;

    for (int iz = 0; iz < grid.nz; iz++) {

      memcpy(_dbzPlanes[iz], handle.field_plane[_params->dbz_field][iz],
	     npoints_plane);
      
      if (_params->vel_available) {
	memcpy(_velPlanes[iz], handle.field_plane[_params->vel_field][iz],
	       npoints_plane);
      }
      
    } // iz

  }

  // set the dbz volume to the start of the first valid plane
  // since this is where the clumping should start
  
  dbzVol = _dbzPlanes[minValidLayer];

  // set public pointers

  dbzPlanes = _dbzPlanes;
  velPlanes = _velPlanes;
    
}
