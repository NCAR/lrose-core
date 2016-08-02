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
// Octiber 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/pmu.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(char *prog_name,
		       Params *params)

{

  _progName = STRdup(prog_name);
  _params = params;
  MDV_init_handle(&_handle);
  _initHeaders();

}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
  STRfree(_progName);
  MDV_free_handle(&_handle);

}

////////////////////////////////////////////////////
// clearVol()
//
// Clears out the volume data, initialize
//

void OutputFile::clearVol()
  
{

  for (int out_field = 0; out_field < _handle.master_hdr.n_fields;
       out_field++) {
    for (int iz = 0; iz < _handle.master_hdr.max_nz; iz++) {
      memset(_handle.field_plane[out_field][iz], 0, _npointsPlane);
    }
  }

}

////////////////////////////////////////
// writeVol()
//
// Write out volume in MDV format.
//

int OutputFile::writeVol(time_t forecast_time,
			 time_t start_time,
			 time_t end_time)
  
{

  PMU_auto_register("In OutputFile::writeVol");

  MDV_master_header_t *out_mhdr = &_handle.master_hdr;
  out_mhdr->time_gen = time(NULL);
  out_mhdr->time_begin = start_time;
  out_mhdr->time_end = end_time;
  out_mhdr->time_centroid = forecast_time;
  out_mhdr->time_expire = out_mhdr->time_centroid +
    (out_mhdr->time_end - out_mhdr->time_begin);
  
  for (int out_field = 0; out_field < out_mhdr->n_fields; out_field++) {
    MDV_field_header_t *out_fhdr = _handle.fld_hdrs + out_field;
    out_fhdr->forecast_time = forecast_time;
  }
  
  if (_params->debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Writing MDV file, time %s, to dir %s\n",
	    utimstr(out_mhdr->time_centroid), _params->output_dir);
  }
  
  // write to directory
  
  int iret = MDV_write_to_dir(&_handle, _params->output_dir,
			      MDV_PLANE_RLE8, TRUE);
  
  if (iret == MDV_SUCCESS) {
    return (0);
  } else {
    return (-1);
  }

}

////////////////////////////////////////////////////
// setScaleAndBias()
//
// Set scale and bias for a given field
//

void OutputFile::setScaleAndBias(char *field_name, double scale, double bias)
  
{

  int field_found = FALSE;
  for (int out_field = 0;
       out_field < _handle.master_hdr.n_fields; out_field++) {
    MDV_field_header_t *fhdr = _handle.fld_hdrs + out_field;
    if (!strcmp(field_name, fhdr->field_name)) {
      fhdr->scale = scale;
      fhdr->bias = bias;
      field_found = TRUE;
      break;
    }
  }
  if (!field_found) {
    fprintf(stderr, "ERROR - %s:OutputFile::setScaleAndBias\n", _progName);
    fprintf(stderr, "Cannot find field %s\n", field_name);
    fprintf(stderr, "Scale and bias not set.\n");
  }
    
}

////////////////////////////////////////////////////
// _initHeaders()
//
// Initialize the volume headers from the parameters.
//

void OutputFile::_initHeaders()
  
{

  // clear the master header
  
  MDV_master_header_t *out_mhdr = &_handle.master_hdr;
  MDV_init_master_header(out_mhdr);
  
  // fill the master header
  
  out_mhdr->num_data_times = 1;
  out_mhdr->data_dimension = 3;
  
  out_mhdr->data_collection_type = MDV_DATA_FORECAST;
  out_mhdr->native_vlevel_type = MDV_VERT_FLIGHT_LEVEL;
  out_mhdr->vlevel_type = MDV_VERT_FLIGHT_LEVEL;
  out_mhdr->vlevel_included = TRUE;
  out_mhdr->grid_order_direction = MDV_ORIENT_SN_WE;
  out_mhdr->grid_order_indices = MDV_ORDER_XYZ;
  out_mhdr->n_fields = _params->output_fields_n;
  out_mhdr->max_nx = _params->output_grid.nx;
  out_mhdr->max_ny = _params->output_grid.ny;
  out_mhdr->max_nz = _params->flight_levels_n;
  out_mhdr->n_chunks = 0;
  out_mhdr->field_grids_differ = FALSE;
  out_mhdr->sensor_lon = 0.0;
  out_mhdr->sensor_lat = 0.0;
  out_mhdr->sensor_alt = 0.0;

  // data set name and source
  
  STRncopy(out_mhdr->data_set_info, _params->data_set_info, MDV_INFO_LEN);
  STRncopy(out_mhdr->data_set_name, _params->data_set_name, MDV_NAME_LEN);
  STRncopy(out_mhdr->data_set_source, _params->data_set_source, MDV_NAME_LEN);
  
  // allocate MDV arrays
  
  MDV_alloc_handle_arrays(&_handle, out_mhdr->n_fields,
			  out_mhdr->max_nz, out_mhdr->n_chunks);
  
  // fill in field headers and vlevel headers
  
  _npointsPlane = _params->output_grid.nx * _params->output_grid.ny;

  for (int out_field = 0; out_field < out_mhdr->n_fields; out_field++) {
    
    MDV_field_header_t *out_fhdr = _handle.fld_hdrs + out_field;

    MDV_init_field_header(out_fhdr);

    out_fhdr->nx = _params->output_grid.nx;
    out_fhdr->ny = _params->output_grid.ny;
    out_fhdr->nz = _params->flight_levels_n;
    if (_params->output_projection == Params::OUTPUT_PROJ_FLAT) {
      out_fhdr->proj_type = MDV_PROJ_FLAT;
    } else if (_params->output_projection == Params::OUTPUT_PROJ_LATLON) {
      out_fhdr->proj_type = MDV_PROJ_LATLON;
    }
    out_fhdr->encoding_type = MDV_INT8;
    out_fhdr->data_element_nbytes = 1;
    out_fhdr->field_data_offset = 0;
    out_fhdr->volume_size =
      out_fhdr->nx * out_fhdr->ny * out_fhdr->nz * sizeof(ui08);

    out_fhdr->proj_origin_lat = _params->output_origin.lat;
    out_fhdr->proj_origin_lon = _params->output_origin.lon;

    out_fhdr->grid_dx = _params->output_grid.dx;
    out_fhdr->grid_dy = _params->output_grid.dy;
    out_fhdr->grid_dz = 10.0;

    out_fhdr->grid_minx = _params->output_grid.minx;
    out_fhdr->grid_miny = _params->output_grid.miny;
    out_fhdr->grid_minz = _params->_flight_levels[0];

    out_fhdr->bad_data_value = 0;
    out_fhdr->missing_data_value = 0;
    
    out_fhdr->proj_rotation = 0.0;
    
    switch (_params->_output_fields[out_field]) {
      
    case Params::U_FIELD:
      _setFieldName(out_fhdr, "U", "u-component of wind", "kts", "none",
		    MDV_get_field_code_from_abbrev("U GRD"),
		    1.0, -127.0);
      break;

    case Params::V_FIELD:
      _setFieldName(out_fhdr, "V", "v-component of wind", "kts", "none",
		    MDV_get_field_code_from_abbrev("V GRD"),
		    1.0, -127.0);
      break;
      
    case Params::W_FIELD:
      _setFieldName(out_fhdr, "W", "w-component of wind", "m/s", "none",
		    MDV_get_field_code_from_abbrev("DZDT"),
		    0.01, -1.27);
      break;

    case Params::WSPD_FIELD:
      _setFieldName(out_fhdr, "Speed", "Wind speed", "kts", "none",
		    MDV_get_field_code_from_abbrev("WIND"),
		    1.0, 0.0);
      break;

    case Params::TEMP_FIELD:
      _setFieldName(out_fhdr, "Temp", "Temperature", "C", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    0.5, -75.0);
      break;
      
    case Params::HUMIDITY_FIELD:
      _setFieldName(out_fhdr, "RH", "Relative humidity", "%", "none",
		    MDV_get_field_code_from_abbrev("R H"),
		    0.5, 0.0);
      break;

    case Params::CLOUD_FIELD:
      _setFieldName(out_fhdr, "Cloud", "Cloud mixing ratio", "g/kg", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    0.05, 0.05);
      break;

    case Params::PRECIP_FIELD:
      _setFieldName(out_fhdr, "Precip", "Precip mixing ratio", "g/kg", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    0.05, 0.05);
      break;

    case Params::ICING_FIELD:
      _setFieldName(out_fhdr, "Icing", "Icing severity index", "", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    1.0, 0.0);
      break;

    case Params::TURB_FIELD:
      _setFieldName(out_fhdr, "Turb", "turbulence severity index", "", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    1.0, 0.0);
      break;
      
    case Params::FZLEVEL_FIELD:
      _setFieldName(out_fhdr, "FZLevel", "freezing level (flight level)",
		    "", "none",
		    MDV_get_field_code_from_abbrev("UNDEFINED"),
		    2.5, -20.0);
      out_fhdr->nz = 1;
      break;
      
    } // switch

    // vlevel headers

    MDV_vlevel_header_t *vhdr = _handle.vlv_hdrs + out_field;
    MDV_init_vlevel_header(vhdr);
    for (int iz = 0; iz < out_fhdr->nz; iz++) {
      vhdr->vlevel_type[iz] = MDV_VERT_FLIGHT_LEVEL;
      vhdr->vlevel_params[iz] = _params->_flight_levels[iz];
    }
    
    // allocate the data arrays

    for (int iz = 0; iz < out_fhdr->nz; iz++) {
      _handle.field_plane[out_field][iz] =
	(ui08 *) umalloc (_npointsPlane);
    } // iz

  } // out_field

  _handle.field_planes_allocated = TRUE;

}

////////////////////////////////////////////////////
// _setFieldName()
//
// Sets the field name, units etc
//

void OutputFile::_setFieldName(MDV_field_header_t *fhdr,
			       char *name,
			       char *name_long,
			       char *units,
			       char *transform,
			       int field_code,
			       double scale,
			       double bias)
  
{

  STRncopy(fhdr->field_name, name, MDV_SHORT_FIELD_LEN);
  STRncopy(fhdr->field_name_long, name_long, MDV_LONG_FIELD_LEN);
  STRncopy(fhdr->units, units, MDV_UNITS_LEN);
  STRncopy(fhdr->transform, transform, MDV_TRANSFORM_LEN);
  fhdr->field_code = field_code;
  fhdr->scale = scale;
  fhdr->bias = bias;
  
}
