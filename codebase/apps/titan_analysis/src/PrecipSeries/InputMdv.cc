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
// March 1999
//
///////////////////////////////////////////////////////////////

#include "InputMdv.hh"
#include <toolsa/umisc.h>
#include <mdv/mdv_read.h>
#include <iostream>
using namespace std;

//////////////
// constructor
//

InputMdv::InputMdv(string &prog_name, Params &params) :
  _progName(prog_name),
  _params(params)
  
{

  isOK = TRUE;

  if (MDV_init_handle(&_handle)) {
    isOK = FALSE;
    return;
  }

  _prevDbzScale = -1.0;
  _prevDbzBias = 0.0;

  _compDbz = NULL;
  _ncompDbzAlloc = 0;

  // create radar tops object

}

/////////////
// destructor
//

InputMdv::~InputMdv()

{

  if (_compDbz) {
    ufree(_compDbz);
  }
  MDV_free_handle(&_handle);

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
	  _params.input_rdata_dir, PATH_DELIM,
	  dtime.year, dtime.month, dtime.day,
	  PATH_DELIM,
	  dtime.hour, dtime.min, dtime.sec, "mdv");
  cout << _progName << ": processing file " << filePath << endl;
	  
  // read in file
  
  if (MDV_read_all(&_handle, filePath, MDV_INT8)) {
    return (-1);
  }

  // copy the grid

  _grid = _handle.grid;

  // initialize  grid computations

  if(_grid.proj_type == MDV_PROJ_FLAT) {
    MDV_init_flat(_grid.proj_origin_lat,
		  _grid.proj_origin_lon,
		  _grid.proj_params.flat.rotation,
		  &_gridComps);
  } else {
    MDV_init_latlon(&_gridComps);
  }

  // make sure dz is not zero - if it is, set to 1.0

  if (_grid.dz <= 0) {
    _grid.dz = 1.0;
  }

  // set number of fields

  _nFields = _handle.master_hdr.n_fields;

  // check that requested dbz field number is valid
  
  if (_params.dbz_field > _nFields - 1) {
    cerr << "ERROR - " << _progName << ":InputMdv::read" << endl;
    cerr << "Dbz field number " << _params.dbz_field
	 << " invalid - only " << _nFields << " fields available." << endl;
    cerr << "Remember - field numbers start at 0." << endl;
    return(-1);
  }
  
  // set min cartesian layer

  _minValidLayer =
    (int) ((_params.composite_min_altitude - _grid.minz) / _grid.dz + 0.5);
  
  if(_minValidLayer < 0) {
    _minValidLayer = 0;
  } else if(_minValidLayer > _grid.nz - 1) {
    _minValidLayer = _grid.nz - 1;
  }

  // set max cartesian layer at which data is considered valid
  
  _maxValidLayer =
    (int) ((_params.composite_max_altitude - _grid.minz) / _grid.dz + 0.5);
  
  if(_maxValidLayer < 0) {
    _maxValidLayer = 0;
  } else if(_maxValidLayer > _grid.nz - 1) {
    _maxValidLayer = _grid.nz - 1;
  }

  _nPlanes = _maxValidLayer - _minValidLayer + 1;
  
  // set Dbz_byte_data
  
  MDV_field_header_t *fhdr = _handle.fld_hdrs + _params.dbz_field;
  double dbz_scale = fhdr->scale;
  double dbz_bias = fhdr->bias;
  
  _lowDbzByte =
    (int) ((_params.low_dbz_threshold - dbz_bias) / dbz_scale + 0.5);
  
  _hailDbzByte =
    (int) ((_params.hail_dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  // if scale or bias has changed, as will always be the case on the
  // first call, compute the dbz_byte to precip_rate lookup table
  
  if (dbz_scale != _prevDbzScale || dbz_bias != _prevDbzBias) {
    
    for (int i = 1; i < 256; i++) {
      double dbz = (double) i * dbz_scale + dbz_bias;
      if (dbz < _params.low_dbz_threshold) {
	_precipLookup[i] = 0.0;
      } else {
	if (dbz > _params.hail_dbz_threshold) {
	  dbz = _params.hail_dbz_threshold;
	}
	double z = pow(10.0, dbz / 10.0);
	double precip_rate =
	  pow((z / _params.zr.coeff),
	      (1.0 / _params.zr.expon)) / 3600.0; // mm/sec
	if (precip_rate < 0.001) {
	  _precipLookup[i] = 0.0;
	} else {
	  _precipLookup[i] = precip_rate;
	}
      }
    } // i
    _precipLookup[(ui08) fhdr->missing_data_value] = 0.0;
    _precipLookup[(ui08) fhdr->bad_data_value] = 0.0;

  }

  // save scale and bias values for checking next time
  
  _prevDbzScale = dbz_scale;
  _prevDbzBias = dbz_bias;
  _dbzScale = dbz_scale;
  _dbzBias = dbz_bias;

  // set the dbzPlanes

  _dbzPlanes = (ui08 **) _handle.field_plane[_params.dbz_field];

  // fill out the composite grid
 
  _compDbzFill();

  return (0);
  
}

///////////////////////////////////////
// precipRateCompute()
//
// Compute precip rate at a point in mm/sec
//
// returns the precip
//

double InputMdv::precipRateCompute(double lat, double lon)

{

  // compute the x,y location
  
  double x, y;
  MDV_latlon2xy(&_gridComps, lat, lon, &x, &y);

  int ix = (int) ((x - _grid.minx) / _grid.dx + 0.5);
  int iy = (int) ((y - _grid.miny) / _grid.dy + 0.5);

  int index = ix + (iy * _grid.nx);
  double rate = _precipLookup[_compDbz[index]];
  
  return (rate);

}


///////////////////////////////
// _compDbzAlloc()
//
// allocate the composite grid
//
 
void InputMdv::_compDbzAlloc()
  
{
  
  int nbytes = _grid.nx * _grid.ny;
  
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

  _compDbzAlloc();
  int nPointsPlane = _grid.nx * _grid.ny;
    
  if (_grid.nz == 1) {
    
    // only 1 plane - use this

    memcpy(_compDbz, _dbzPlanes[0], nPointsPlane);
    
  } else {
    
    // compute the composite reflectivity grid - this is the
    // max reflectivity at any height

    for (int iz = 0; iz < _grid.nz; iz++) {

      ui08 *dbz = _dbzPlanes[iz];
      ui08 *comp = _compDbz;
      
      for (int i = 0; i < nPointsPlane; i++, dbz++, comp++) {
	if (*dbz > *comp) {
	  *comp = *dbz;
	}
      } // i

    } // iz

  }
    
}
