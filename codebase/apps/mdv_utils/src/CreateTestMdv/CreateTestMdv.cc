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
// CreateTestMdv object
//
// Mike Dixon, EOL, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2020
//
///////////////////////////////////////////////////////////////
//
// CreateTestMdv creates a dummy MDV file for testing purposes.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <cmath>
#include <toolsa/mem.h>
#include <toolsa/ucopyright.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <Mdv/MdvxField.hh>
#include "CreateTestMdv.hh"
using namespace std;

const fl32 CreateTestMdv::_missingFloat = -9999.0;

// Constructor

CreateTestMdv::CreateTestMdv(int argc, char **argv)

{

  isOK = true;

  // set programe name

  _progName = (char *) "CreateTestMdv";
  ucopyright(_progName.c_str());
  
  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = false;
    return;
  }

  // get TDRP params
  
  _paramsPath = (char *) "unknown";
  if (_params.loadFromArgs(argc, argv, _args.override.list,
			   &_paramsPath)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with TDRP parameters" << endl;
    isOK = false;
  }

  // create the zlevel array

  if (_params.specify_individual_z_levels) {
    for (int ii = 0; ii < _params.z_level_array_n; ii++) {
      _zLevels.push_back(_params._z_level_array[ii]);
    }
  } else {
    for (int ii = 0; ii < _params.grid_z_geom.nz; ii++) {
      double zz = _params.grid_z_geom.minz + ii * _params.grid_z_geom.dz;
      _zLevels.push_back(zz);
    }
  }

  return;

}

// destructor

CreateTestMdv::~CreateTestMdv()

{

}

//////////////////////////////////////////////////
// Run

int CreateTestMdv::Run ()
{

  DsMdvx mdvx;
  if (_createMdvx(mdvx)) {
    cerr << "ERROR - CreateTestMdv::Run" << endl;
    cerr << "  creating mdvx object" << endl;
    return -1;
  }
  
  // write out file
  
  if (_writeMdvxFile(mdvx)) {
    cerr << "ERROR - CreateTestMdv::Run" << endl;
    cerr << "  writing mdvx file" << endl;
    return -1;
  }

  return 0;

}

///////////////////////////////
// create mdvx object

int CreateTestMdv::_createMdvx(DsMdvx &mdvx)

{
  
  time_t dataTime = time(NULL);
  
  if (_params.debug) {
    cerr << "Creating file at time: "
         << DateTime::strm(dataTime) << endl;
  }
  
  // initialize the master header
  
  _initMasterHeader(mdvx, dataTime);
  
  // add the fields
  
  int nFields = _params.fields_n;
  for (int ifield = 0; ifield < nFields; ifield++) {
    Params::field_t &fld = _params._fields[ifield];
    _addField(mdvx, fld);
  }

  return 0;

}

///////////////////////////////
// add a field

void CreateTestMdv::_addField(DsMdvx &mdvx,
                              const Params::field_t &fld)

{
  
  // fill in field header and vlevel header
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  MdvxProj proj;
  _initProjection(fhdr);
  
  fhdr.proj_origin_lat = _params.proj_origin_lat;
  fhdr.proj_origin_lon = _params.proj_origin_lon;

  fhdr.nx = _params.grid_xy_geom.nx;
  fhdr.ny = _params.grid_xy_geom.ny;
  fhdr.nz = _zLevels.size();
  int64_t ndata = fhdr.nx * fhdr.ny * fhdr.nz;
  
  fhdr.grid_dx = _params.grid_xy_geom.dx;
  fhdr.grid_dy = _params.grid_xy_geom.dy;
  if (_zLevels.size() < 2) {
    fhdr.grid_dz = 1.0;
  } else {
    fhdr.grid_dz = _zLevels[1] - _zLevels[0];
  }

  fhdr.grid_minx = _params.grid_xy_geom.minx;
  fhdr.grid_miny = _params.grid_xy_geom.miny;
  if (_zLevels.size() < 1) {
    fhdr.grid_dz = 0.0;
  } else {
    fhdr.grid_dz = _zLevels[0];
  }

  // int npointsPlane = fhdr.nx * fhdr.ny;
    
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = _params.vlevel_type;
  fhdr.vlevel_type = _params.vlevel_type;
  fhdr.dz_constant = false;

  fhdr.bad_data_value = _missingFloat;
  fhdr.missing_data_value = _missingFloat;
  
  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = ndata * sizeof(fl32);

  // vlevel header
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = _params.vlevel_type;
    vhdr.level[iz] = _zLevels[iz];
  }

  // alloc data array

  fl32 *data = new fl32[ndata];

  // fill it with sim data

  double fieldRange = fld.max_val - fld.min_val;
  double fieldMin = fld.min_val;

  int64_t index = 0;
  for (int iz = 0; iz < fhdr.nz; iz++) {
    for (int iy = 0; iy < fhdr.ny; iy++) {
      for (int ix = 0; ix < fhdr.nx; ix++, index++) {
        
        // compute normalized location in data space [0, 1]
        
        double zz = (double) iz / fhdr.nz;
        double yy = (double) iy / fhdr.ny;
        double xx = (double) ix / fhdr.nx;
        double frac = sqrt(zz * zz + yy * yy + xx * xx) / sqrt(3.0);

        // transform to get the float val

        fl32 val = fieldMin + frac * fieldRange;
        data[index] = val;

      } // ix
    } // iy
  } // iz

  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);
  
  // set name etc
  
  field->setFieldName(fld.name);
  field->setFieldNameLong(fld.long_name);
  field->setUnits(fld.units);
  field->setTransform("none");
  
  field->convertRounded((Mdvx::encoding_type_t) fld.encoding,
                        (Mdvx::compression_type_t) _params.compression);
  
  // add field to mdvx object
  
  mdvx.addField(field);

}

////////////////////////////////////////////////////
// init master header

void CreateTestMdv::_initMasterHeader(DsMdvx &mdvx, time_t dataTime)

{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_begin = dataTime;
  mhdr.time_end = dataTime;
  mhdr.time_centroid = dataTime;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 3;
  
  mhdr.data_collection_type = -1;
  mhdr.native_vlevel_type = _params.vlevel_type;
  mhdr.vlevel_type = _params.vlevel_type;
  if (_params.specify_individual_z_levels) {
    mhdr.max_nz = _params.z_level_array_n;
  } else {
    mhdr.max_nz = _params.grid_z_geom.nz;
  }
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;
  mhdr.max_nx = _params.grid_xy_geom.nx;
  mhdr.max_ny = _params.grid_xy_geom.ny;
  mhdr.n_fields = _params.fields_n;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  mdvx.setDataSetInfo(_params.data_set_info);
  mdvx.setDataSetName(_params.data_set_name);
  mdvx.setDataSetSource(_params.data_set_source);

  mdvx.setMasterHeader(mhdr);
  
}

////////////////////////////////////////////////////////////
// Initialize projection

void CreateTestMdv::_initProjection(Mdvx::field_header_t &fhdr)
  
{

  fhdr.proj_origin_lat = _params.proj_origin_lat;
  fhdr.proj_origin_lon = _params.proj_origin_lon;

  if (_params.projection == Params::PROJ_LATLON) {
    fhdr.proj_type = Mdvx::PROJ_LATLON;
  } else if (_params.projection == Params::PROJ_FLAT) {
    fhdr.proj_type = Mdvx::PROJ_FLAT;
    fhdr.proj_rotation = _params.proj_rotation;
    fhdr.proj_param[0] = _params.proj_rotation;
  } else if (_params.projection == Params::PROJ_LAMBERT_CONF) {
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
    fhdr.proj_param[0] = _params.proj_lat1;
    fhdr.proj_param[1] = _params.proj_lat2;
  } else if (_params.projection == Params::PROJ_POLAR_STEREO) {
    fhdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
    fhdr.proj_param[0] = _params.proj_tangent_lon;
    if (_params.proj_pole_is_north) {
      fhdr.proj_param[1] = 1;
    } else {
      fhdr.proj_param[1] = 0;
    }
    fhdr.proj_param[2] = _params.proj_central_scale;
  } else if (_params.projection == Params::PROJ_OBLIQUE_STEREO) {
    fhdr.proj_type = Mdvx::PROJ_OBLIQUE_STEREO;
    fhdr.proj_param[0] = _params.proj_tangent_lat;
    fhdr.proj_param[1] = _params.proj_tangent_lon;
    fhdr.proj_param[2] = _params.proj_central_scale;
  } else if (_params.projection == Params::PROJ_MERCATOR) {
    fhdr.proj_type = Mdvx::PROJ_MERCATOR;
  } else if (_params.projection == Params::PROJ_TRANS_MERCATOR) {
    fhdr.proj_type = Mdvx::PROJ_TRANS_MERCATOR;
    fhdr.proj_param[0] = _params.proj_central_scale;
  } else if (_params.projection == Params::PROJ_ALBERS) {
    fhdr.proj_type = Mdvx::PROJ_ALBERS;
    fhdr.proj_param[0] = _params.proj_lat1;
    fhdr.proj_param[1] = _params.proj_lat2;
  } else if (_params.projection == Params::PROJ_LAMBERT_AZIM) {
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_AZIM;
  } else if (_params.projection == Params::PROJ_VERT_PERSP) {
    fhdr.proj_type = Mdvx::PROJ_VERT_PERSP;
  }
  
  fhdr.proj_param[6] = _params.proj_false_northing;
  fhdr.proj_param[7] = _params.proj_false_easting;

}

////////////////////////////////////////////////////
// write output data
//
// returns 0 on success, -1 on failure

int CreateTestMdv::_writeMdvxFile(DsMdvx &mdvx)

{

  // write out
  
  if (mdvx.writeToDir(_params.output_dir)) {
    cerr << "ERROR - CreateTestMdv::_writeOutput" << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
  
  if (_params.debug) {
    cerr << "Wrote file: " << mdvx.getPathInUse() << endl;
  }

  return 0;
    
}

