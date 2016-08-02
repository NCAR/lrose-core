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
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <didss/DsURL.hh>
#include <Mdv/MdvxRadar.hh>
using namespace std;

//////////////
// constructor
//

InputMdv::InputMdv(const string &prog_name, const Params &params) :
  Worker(prog_name, params)

{

  dbzField = NULL;
  velField = NULL;
  dbzVol = NULL;
  velVol = NULL;
  compDbz = NULL;

  dbzScale = 1.0;
  dbzBias = 0.0;
  dbzMiss = 0.0;
  velScale = 1.0;
  velBias = 0.0;

  _prevDbzScale = -1.0;
  _prevDbzBias = 0.0;

  // create radar tops object
  // if required, mask out areas with low tops
  
  if (_params.check_tops) {
    _tops = new Tops(_progName, _params, *this);
  } else {
    _tops = NULL;
  }

}

/////////////
// destructor
//

InputMdv::~InputMdv()

{

  if (_tops) {
    delete(_tops);
  }
  
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

  mdvx.clearRead();

  mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
		    _params.input_url,
		    0, data_time);

  bool useFieldNames;
  if (strlen(_params.dbz_field.name) > 0) {
    useFieldNames = true;
    mdvx.addReadField(_params.dbz_field.name);
    if (_params.vel_available) {
      mdvx.addReadField(_params.vel_field.name);
    }
  } else {
    useFieldNames = false;
    mdvx.addReadField(_params.dbz_field.num);
    if (_params.vel_available) {
      mdvx.addReadField(_params.vel_field.num);
    }
  }

  mdvx.setReadEncodingType(Mdvx::ENCODING_INT8);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (_params.debug >= Params::DEBUG_VERBOSE) {
    mdvx.printReadRequest(cerr);
  }

  if (mdvx.readVolume()) {
    cerr << "ERROR - " << _progName << ": InputMdv::read" << endl;
    cerr << mdvx.getErrStr();
    return -1;
  }

  // check that the vertical levels are evenly spaced, since TITAN
  // requires that

  if (_params.remap_z_to_constant_grid) {
    for (int ii = 0; ii < mdvx.getNFields(); ii++) {
      MdvxField *field = mdvx.getField(ii);
      if (field) {
        field->remapVlevels(_params.remap_z_grid.nz,
                            _params.remap_z_grid.minz,
                            _params.remap_z_grid.dz);
      }
    }
  }

  // set headers, fields etc
  
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  if (useFieldNames) {
    dbzField = mdvx.getField(_params.dbz_field.name);
    if (_params.vel_available) {
      velField = mdvx.getField(_params.vel_field.name);
    }
  } else {
    if (_params.dbz_field.num < _params.vel_field.num ||
	!_params.vel_available) {
      dbzField = mdvx.getField(0);
      if (_params.vel_available) {
	velField = mdvx.getField(1);
      }
    } else {
      dbzField = mdvx.getField(1);
      if (_params.vel_available) {
	velField = mdvx.getField(0);
      }
    }
  }
  if (_params.negate_dbz_field) {
    dbzField->negate();
  }

  const Mdvx::field_header_t &dbzFhdr = dbzField->getFieldHeader();

  if (!dbzField->isDzConstant()) {
    cerr << "WARNING - InputMdv::read()" << endl;
    cerr << "  DBZ field does not have constant delta Z in height" << endl;
    cerr << "  File: " << mdvx.getPathInUse() << endl;
    cerr << "  Vertical levels will not be properly handled." << endl;
    cerr << "  Please set the 'remap_z_to_constant_grid' parameter" << endl;
  }

  // set up the projection coordinate grid

  proj.init(mdvx);
  coord = proj.getCoord();
  
  // make sure dz is not zero - if it is, set to 1.0
  
  if (coord.dz <= 0) {
    coord.dz = 1.0;
  }

  // copy to titan grid

  _copyCoord2Grid(coord, grid);

  // set number of fields

  nFields = mhdr.n_fields;

  // set min cartesian layer
  
  minValidLayer =
    (int) ((_params.base_threshold - coord.minz) / coord.dz + 0.5);
  
  if(minValidLayer < 0) {
    minValidLayer = 0;
  } else if(minValidLayer > coord.nz - 1) {
    minValidLayer = coord.nz - 1;
  }
  
  // set max cartesian layer at which data is considered valid
  
  maxValidLayer =
    (int) ((_params.top_threshold - coord.minz) / coord.dz + 0.5);
  
  if(maxValidLayer < 0) {
    maxValidLayer = 0;
  } else if(maxValidLayer > coord.nz - 1) {
    maxValidLayer = coord.nz - 1;
  }
  
  nPlanes = maxValidLayer - minValidLayer + 1;
  
  // set precip layer limits
  
  minPrecipLayer =
    (int) ((_params.precip_min_ht - coord.minz) / coord.dz + 0.5);
  if(minPrecipLayer < 0) {
    minPrecipLayer = 0;
  } else if(minPrecipLayer > coord.nz - 1) {
    minPrecipLayer = coord.nz - 1;
  }
  
  maxPrecipLayer =
    (int) ((_params.precip_max_ht - coord.minz) / coord.dz + 0.5);
  if(maxPrecipLayer < 0) {
    maxPrecipLayer = 0;
  } else if(maxPrecipLayer > coord.nz - 1) {
    maxPrecipLayer = coord.nz - 1;
  }
  
  // set specified precip layer
  
  specifiedPrecipLayer = minValidLayer;
  if (_params.precip_computation_mode == Params::PRECIP_AT_SPECIFIED_HT) {
    specifiedPrecipLayer =
      (int) ((_params.precip_plane_ht - coord.minz) / coord.dz + 0.5);
  }
  
  if(specifiedPrecipLayer < 0) {
    specifiedPrecipLayer = 0;
  } else if(specifiedPrecipLayer > coord.nz - 1) {
    specifiedPrecipLayer = coord.nz - 1;
  }
  
  // set Dbz_byte_data
  
  double dbz_scale = dbzFhdr.scale;
  double dbz_bias = dbzFhdr.bias;
  
  lowDbzByte =
    (int) ((_params.low_dbz_threshold - dbz_bias) / dbz_scale + 0.5);
  
  highDbzByte =
    (int) ((_params.high_dbz_threshold - dbz_bias) / dbz_scale + 0.5);

  if (_params.use_dual_threshold) {
    dualDbzByte =
      (int) ((_params.dual_threshold.dbz_threshold - dbz_bias) /
	     dbz_scale + 0.5);
  }
  
  // if scale or bias has changed, as will always be the case on the
  // first call, set the Glob->dbz_interval array. If the dbz val
  // is outside the histogram range, the index is set to -1
  
  if (dbz_scale != _prevDbzScale || dbz_bias != _prevDbzBias) {
    
    for (int i = 0; i < N_BYTE_DATA_VALS; i++) {
      
      double dbz = (double) i * dbz_scale + dbz_bias;
      
      if (dbz < _params.low_dbz_threshold ||
	  dbz >= _params.high_dbz_threshold) {

	dbzInterval[i] = -1;

      } else {

	dbzInterval[i] =
	  (int) ((dbz - _params.low_dbz_threshold) /
		 _params.dbz_hist_interval);

      }
      
    } // i
    
  } // if (dbz_scale != _prevDbzScale ...
  
  // save scale and bias values for checking next time

  _prevDbzScale = dbz_scale;
  _prevDbzBias = dbz_bias;
  dbzScale = dbz_scale;
  dbzBias = dbz_bias;
  if (velField) {
    velScale = velField->getFieldHeader().scale;
    velBias = velField->getFieldHeader().bias;
  }

  dbzMiss = dbzFhdr.missing_data_value;

  // set the dbz volume to the start of the first valid plane
  // since this is where the clumping should start
  
  dbzVol = (ui08 *) dbzField->getVol();
  if (velField) {
    velVol = (ui08 *) velField->getVol();
  }
  
  // if required, mask out dbz regions with low tops
  
  if (_tops) {
    _tops->maskLowTops();
  }

  // if we are going to include storms which have reflectivity exceeding
  // the upper threshold, set high-value bytes to 0
  
  if (!_params.discard_high_threshold_storms) {
    int npoints = dbzFhdr.nx * dbzFhdr.ny * dbzFhdr.nz;
    ui08 *dbyte = dbzVol;
    for (int i = 0; i < npoints; i++, dbyte++) {
      if (*dbyte > highDbzByte) {
	*dbyte = 0;
      }
    }
  }
    
  // create composite grid
  
  compField = *dbzField;
  compField.convert2Composite();
  compDbz = (ui08 *) compField.getVol();
  
  return (0);
  
}

/////////////////////////////////
// _copyCoord2Grid
//

void InputMdv::_copyCoord2Grid(const Mdvx::coord_t &coord,
			       titan_grid_t &grid)

{

  MEM_zero(grid);

  grid.proj_origin_lat = coord.proj_origin_lat;
  grid.proj_origin_lon = coord.proj_origin_lon;
  
  grid.minx = coord.minx;
  grid.miny = coord.miny;
  grid.minz = coord.minz;

  grid.dx = coord.dx;
  grid.dy = coord.dy;
  grid.dz = coord.dz;

  grid.sensor_x = coord.sensor_x;
  grid.sensor_y = coord.sensor_y;
  grid.sensor_z = coord.sensor_z;

  grid.sensor_lat = coord.sensor_lat;
  grid.sensor_lon = coord.sensor_lon;

  grid.proj_type = coord.proj_type;
  grid.dz_constant = coord.dz_constant;

  grid.nx = coord.nx;
  grid.ny = coord.ny;
  grid.nz = coord.nz;
  
  grid.nbytes_char = 3 * TITAN_GRID_UNITS_LEN;

  memcpy(grid.unitsx, coord.unitsx,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  memcpy(grid.unitsy, coord.unitsy,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  memcpy(grid.unitsz, coord.unitsz,
	 MIN(TITAN_GRID_UNITS_LEN, MDV_COORD_UNITS_LEN));
  
  if (coord.proj_type == Mdvx::PROJ_FLAT) {
    grid.proj_params.flat.rotation = coord.proj_params.flat.rotation;
  } else if (coord.proj_type == Mdvx::PROJ_LAMBERT_CONF) {
    grid.proj_params.lc2.lat1 = coord.proj_params.lc2.lat1;
    grid.proj_params.lc2.lat2 = coord.proj_params.lc2.lat2;
//     grid.proj_params.lc2.SW_lat = coord.proj_params.lc2.SW_lat;
//     grid.proj_params.lc2.SW_lon = coord.proj_params.lc2.SW_lon;
//     grid.proj_params.lc2.origin_x = coord.proj_params.lc2.origin_x;
//     grid.proj_params.lc2.origin_y = coord.proj_params.lc2.origin_y;
  }

}

