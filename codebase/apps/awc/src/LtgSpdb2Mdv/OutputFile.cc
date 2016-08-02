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
// OutputFile.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2000
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <toolsa/udatetime.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>

using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(const string &prog_name,
		       const Params &params,
		       time_t trigger_time) :
  _progName(prog_name), _params(params)
  
{
  
  _initMdvx(trigger_time);

}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
}

////////////////////////////////////////
// writeVol()
//
// Write out volume in MDV format.
//

int OutputFile::writeVol()
  
{

  PMU_auto_register("In OutputFile::writeVol");
  
  if (_params.debug) {
    fprintf(stderr, "Writing MDV file, time %s, to url %s\n",
	    utimstr(_mdvx.getMasterHeader().time_centroid), _params.output_url);
  }

  // convert for output

  for (int ii = 0; ii < _mdvx.getNFields(); ii++) {
    
    MdvxField *fld = _mdvx.getFieldByNum(ii);
    
    double scale;
    double bias;

    if (_params.output_scaling_info.SpecifyScaling) {
      scale = _params.output_scaling_info.scale;
      bias = _params.output_scaling_info.bias;
    
      fld->convertType((Mdvx::encoding_type_t)_params.encoding_type,
		       (Mdvx::compression_type_t)_params.compression_type,
		       Mdvx::SCALING_SPECIFIED,
		       scale,
		       bias);

    }
    else
    {
      
      fld->convertType((Mdvx::encoding_type_t)_params.encoding_type,
		       (Mdvx::compression_type_t)_params.compression_type,
		       Mdvx::SCALING_ROUNDED);

    }
    
  
  }

  // write to directory
  
  _mdvx.setWriteLdataInfo();
  if (_mdvx.writeToDir(_params.output_url)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _mdvx.getErrStr() << endl;
    return -1;
  } else {
    return 0;
  }

}

////////////////////////////////////////////////////
// _initMdvx()
//
// Initialize the MDVX object
//

void OutputFile::_initMdvx(time_t trigger_time)
  
{

  // set the master header
  
  Mdvx::master_header_t mhdr;
  MEM_zero(mhdr);

  mhdr.time_gen = time(NULL);
  mhdr.time_begin = trigger_time - _params.rate_compute_period;
  mhdr.time_end = trigger_time + _params.rate_extend_period;
  
  switch (_params.time_stamp_flag) {
  case Params::BEGIN_TIME:
    mhdr.time_centroid = mhdr.time_begin;
    break;
  case Params::MID_TIME:
    mhdr.time_centroid = (int)(((double)mhdr.time_begin/2.0) +
			       ((double)mhdr.time_end/2.0));
    break;
  case Params::END_TIME:
    mhdr.time_centroid = mhdr.time_end;
    break;
  } // switch

  mhdr.time_expire = mhdr.time_centroid + _params.rate_compute_period;
    
  mhdr.num_data_times = 1;
  mhdr.data_dimension = 2;
  
  mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  mhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  mhdr.vlevel_included = TRUE;
  mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  mhdr.data_ordering = Mdvx::ORDER_XYZ;

  mhdr.max_nx = _params.output_grid.nx;
  mhdr.max_ny = _params.output_grid.ny;
  mhdr.max_nz = 1;
  mhdr.n_chunks = 0;
  mhdr.field_grids_differ = FALSE;
  mhdr.sensor_lon = 0.0;
  mhdr.sensor_lat = 0.0;
  mhdr.sensor_alt = 0.0;

  STRncopy(mhdr.data_set_info, _params.data_set_info, MDV_INFO_LEN);
  STRncopy(mhdr.data_set_name, _params.data_set_name, MDV_NAME_LEN);
  STRncopy(mhdr.data_set_source, _params.data_set_source, MDV_NAME_LEN);

  _mdvx.setMasterHeader(mhdr);
  
  // fill in field headers and vlevel headers
  
  _mdvx.clearFields();
  
  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);

  fhdr.nx = _params.output_grid.nx;
  fhdr.ny = _params.output_grid.ny;
  fhdr.nz = 1;

  _npointsPlane = fhdr.nx * fhdr.ny;

  if (_params.output_projection == Params::PROJ_FLAT) {
    fhdr.proj_type = Mdvx::PROJ_FLAT;
  } else if (_params.output_projection == Params::PROJ_LAMBERT_CONF) {
    fhdr.proj_type = Mdvx::PROJ_LAMBERT_CONF;
  } else if (_params.output_projection == Params::PROJ_LATLON) {
    fhdr.proj_type = Mdvx::PROJ_LATLON;
  }

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;

  fhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fhdr.dz_constant = true;

  if (_params.output_projection == Params::PROJ_FLAT) {

    fhdr.proj_origin_lat = _params.grid_origin_lat;
    fhdr.proj_origin_lon = _params.grid_origin_lon;
    
  } else if (_params.output_projection == Params::PROJ_LAMBERT_CONF) {
    
    fhdr.proj_origin_lat = _params.grid_origin_lat;
    fhdr.proj_origin_lon = _params.grid_origin_lon;
    fhdr.proj_param[0] = _params.grid_lat1;
    fhdr.proj_param[1] = _params.grid_lat2;

  }

  fhdr.grid_dx = _params.output_grid.dx;
  fhdr.grid_dy = _params.output_grid.dy;
  fhdr.grid_dz = 1.0;
  fhdr.grid_minx = _params.output_grid.minx;
  fhdr.grid_miny = _params.output_grid.miny;
  fhdr.grid_minz = 0.0;
  
  fhdr.proj_rotation = 0.0;
    
  if (_params.use_missing_for_no_strikes)
  {
    fhdr.bad_data_value = 0.0;
    fhdr.missing_data_value = 0.0;
  }
  else
  {
    fhdr.bad_data_value = -999.0;
    fhdr.missing_data_value = -999.0;
  }
  
  STRncopy(fhdr.field_name, _params.rate_field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr.field_name_long,
	   _params.rate_field_name_long, MDV_LONG_FIELD_LEN);
  char units[64];
  sprintf(units, "strikes/%ds", _params.rate_compute_period);
  STRncopy(fhdr.units, units, MDV_UNITS_LEN);
  STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);

  // vlevel header
  
  vhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vhdr.level[0] = 0;

  // create the data for the rate field.  We want to create a field with
  // all 0.0 values regardless of what missing data value we're using.

  int grid_size = fhdr.nx * fhdr.ny;
  fl32 *rate_data = new fl32[grid_size];
  memset(rate_data, 0, grid_size * sizeof(fl32));
  
  // create rate field
  
  MdvxField *rateField = new MdvxField(fhdr, vhdr, (void *)rate_data, true);

  delete [] rate_data;
  
  // add rate field to mdvx object
  
  _mdvx.addField(rateField);

  if(_params.add_distance_field) {

    // create distance field, add to object

    fhdr.bad_data_value = 0.0;
    fhdr.missing_data_value = 0.0;
    
    STRncopy(fhdr.field_name, _params.distance_field_name,
	     MDV_SHORT_FIELD_LEN);
    STRncopy(fhdr.field_name_long, _params.distance_field_name_long,
	     MDV_LONG_FIELD_LEN);
    STRncopy(fhdr.units, "km", MDV_UNITS_LEN);
    STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
    
    MdvxField *distField = new MdvxField(fhdr, vhdr, NULL, true);
    
    _mdvx.addField(distField);

  }

  if(_params.add_derived_field) {

    // create derived field, add to object

    fhdr.bad_data_value = 0.0;
    fhdr.missing_data_value = 0.0;
    
    STRncopy(fhdr.field_name, _params.derived_field_name,
	     MDV_SHORT_FIELD_LEN);
    STRncopy(fhdr.field_name_long, _params.derived_field_name_long,
	     MDV_LONG_FIELD_LEN);
    STRncopy(fhdr.units, _params.derived_field_units,
	     MDV_UNITS_LEN);
    STRncopy(fhdr.transform, "none", MDV_TRANSFORM_LEN);
    
    MdvxField *derivedField = new MdvxField(fhdr, vhdr, NULL, true);
    
    _mdvx.addField(derivedField);

  }

  
}

