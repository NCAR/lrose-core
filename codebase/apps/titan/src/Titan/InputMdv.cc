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
#include <toolsa/umisc.h>
#include <toolsa/str.h>
#include <didss/DsURL.hh>
#include <Mdv/MdvxRadar.hh>
using namespace std;

//////////////
// constructor
//

InputMdv::InputMdv(const string &prog_name, const Params &params) :
        Worker(prog_name, params),
        _convFinder(prog_name, params)

{

  dbzField = NULL;
  velField = NULL;
  dbzVol = NULL;
  velVol = NULL;
  compDbz = NULL;
  dbzMiss = -9999;

}

/////////////
// destructor
//

InputMdv::~InputMdv()

{
  if (compDbz) {
    delete[] compDbz;
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

  // set up ther read

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

  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (_params.use_column_max_dbz) {
    mdvx.setReadVlevelLimits(_params.column_min_ht_km, _params.column_max_ht_km);
    mdvx.setReadComposite();
  }
  
  if (_params.debug >= Params::DEBUG_EXTRA) {
    mdvx.printReadRequest(cerr);
  }
  
  // perform the read
  
  if (mdvx.readVolume()) {
    cerr << "ERROR - " << _progName << ": InputMdv::read" << endl;
    cerr << mdvx.getErrStr();
    return -1;
  }

  // set headers, fields etc
  
  const Mdvx::master_header_t &mhdr = mdvx.getMasterHeader();

  if (useFieldNames) {
    dbzField = mdvx.getField(_params.dbz_field.name);
    if (_params.vel_available) {
      velField = mdvx.getField(_params.vel_field.name);
    }
  } else {
    dbzField = mdvx.getField(0);
    if (_params.vel_available) {
      velField = mdvx.getField(1);
    }
  }
  if (_params.negate_dbz_field) {
    dbzField->negate();
  }

  // set missing value
  
  const Mdvx::field_header_t &dbzFhdr = dbzField->getFieldHeader();
  dbzMiss = dbzFhdr.missing_data_value;

  // create composite grid

  _computeComposite();

  // if required, find the convective regions
  
  if (_params.identify_convective_regions) {
    if (_convFinder.run(mdvx, *dbzField)) {
      cerr << "WARNING - InputMdv" << endl;
      cerr << "  Cannot identify convective regions - this will be disabled" << endl;
    } else {
      _removeStratiform();
    }
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
  
  // set the dbz volume to the start of the first valid plane
  // since this is where the clumping should start
  
  dbzVol = (fl32 *) dbzField->getVol();
  if (velField) {
    velVol = (fl32 *) velField->getVol();
  }
  
  return 0;
  
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
  }

}

/////////////////////////////////////////////
// create composite grid

void InputMdv::_computeComposite()

{

  const Mdvx::field_header_t &fhdr = dbzField->getFieldHeader();
  int nxy = fhdr.nx * fhdr.ny;

  if (compDbz) {
    delete[] compDbz;
  }
  compDbz = new fl32[nxy];
  
  for (int ii = 0; ii < nxy; ii++) {
    compDbz[ii] = -9999.0;
  }
  
  const fl32 *dbz = (fl32 *) dbzField->getVol();
  for (int iz = 0; iz < fhdr.nz; iz++) {
    fl32 *comp = compDbz;
    for (int iy = 0; iy < fhdr.ny; iy++) {
      for (int ix = 0; ix < fhdr.nx; ix++, dbz++, comp++) {
        if (*dbz != dbzMiss && *dbz > *comp) {
          *comp = *dbz;
        }
      } // ix
    } // iy
  } // iz

}

/////////////////////////////////////////////
// remove stratiform regions

void InputMdv::_removeStratiform()

{

  const Mdvx::field_header_t &fhdr = dbzField->getFieldHeader();

  fl32 *dbz = (fl32 *) dbzField->getVol();
  for (int iz = 0; iz < fhdr.nz; iz++) {
    const ui08 *partition = _convFinder.getConvStrat().getPartition();
    for (int iy = 0; iy < fhdr.ny; iy++) {
      for (int ix = 0; ix < fhdr.nx; ix++, dbz++, partition++) {
        if (*partition != ConvStrat::CATEGORY_CONVECTIVE) {
          *dbz = dbzMiss;
        }
      } // ix
    } // iy
  } // iz


}


