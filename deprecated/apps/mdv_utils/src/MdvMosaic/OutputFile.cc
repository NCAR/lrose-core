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
// August 1998
//
///////////////////////////////////////////////////////////////

#include "OutputFile.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>
using namespace std;

//////////////
// Constructor

OutputFile::OutputFile(char *prog_name,
		       MdvMosaic_tdrp_struct *params,
		       mdv_grid_t *grid)

{

  _progName = STRdup(prog_name);
  _params = params;
  MDV_init_handle(&_handle);
  _grid = grid;

}

/////////////
// destructor

OutputFile::~OutputFile()

{
  
  STRfree(_progName);
  MDV_free_handle(&_handle);

}

////////////////////////////////////////////////////
// initHeaders()
//
// Initialize the volume headers from the grid params and
// headers in an input file. Allocate the field data arrays.
//

void OutputFile::initHeaders(MDV_handle_t *input_mdv)
  
{

  // clear the master header
  
  MDV_master_header_t *out_mhdr = &_handle.master_hdr;
  MDV_master_header_t *in_mhdr = &input_mdv->master_hdr;
  MDV_init_master_header(out_mhdr);
  
  // fill the master header
  
  out_mhdr->num_data_times = 0;           //was 1
  if (_grid->nz == 1) {
    out_mhdr->data_dimension = 2;
  } else {
    out_mhdr->data_dimension = 3;
  }
  out_mhdr->data_collection_type = in_mhdr->data_collection_type;
  out_mhdr->native_vlevel_type = in_mhdr->native_vlevel_type;
  out_mhdr->vlevel_type = in_mhdr->vlevel_type;
  out_mhdr->vlevel_included = in_mhdr->vlevel_included;
  out_mhdr->grid_order_direction = MDV_ORIENT_SN_WE;
  out_mhdr->grid_order_indices = MDV_ORDER_XYZ;
  out_mhdr->n_fields = _params->field_list.len;
  out_mhdr->max_nx = _grid->nx;
  out_mhdr->max_ny = _grid->ny;
  out_mhdr->max_nz = _grid->nz;
  out_mhdr->n_chunks = 0;
  out_mhdr->field_grids_differ = FALSE;
  out_mhdr->sensor_lon = 0.0;
  out_mhdr->sensor_lat = 0.0;
  out_mhdr->sensor_alt = 0.0;

  // data set name and source
  
  STRncopy(out_mhdr->data_set_name, _params->data_set_name, MDV_NAME_LEN);
  STRncopy(out_mhdr->data_set_source, _params->data_set_source, MDV_NAME_LEN);
  
  // allocate MDV arrays
  
  MDV_alloc_handle_arrays(&_handle, out_mhdr->n_fields,
			  out_mhdr->max_nz, out_mhdr->n_chunks);

  // fill in field headers and vlevel headers
  
  for (int out_field = 0; out_field < out_mhdr->n_fields; out_field++) {
    
    int in_field = _params->field_list.val[out_field];
    MDV_field_header_t *out_fhdr = _handle.fld_hdrs + out_field;
    MDV_field_header_t *in_fhdr = input_mdv->fld_hdrs + in_field;
    
    MDV_init_field_header(out_fhdr);

    out_fhdr->nx = _grid->nx;
    out_fhdr->ny = _grid->ny;
    out_fhdr->nz = _grid->nz;
    out_fhdr->proj_type = _grid->proj_type;
    out_fhdr->encoding_type = MDV_INT8;
    out_fhdr->data_element_nbytes = 1;
    out_fhdr->field_data_offset = 0;
    out_fhdr->volume_size =
      out_fhdr->nx * out_fhdr->ny * out_fhdr->nz * sizeof(ui08);

    //out_fhdr->proj_origin_lat = _grid->proj_origin_lat;
    //out_fhdr->proj_origin_lon = _grid->proj_origin_lon;

    out_fhdr->proj_origin_lat = out_mhdr->sensor_lat;
    out_fhdr->proj_origin_lon = out_mhdr->sensor_lon;

    out_fhdr->grid_dx = _grid->dx;
    out_fhdr->grid_dy = _grid->dy;
    out_fhdr->grid_dz = _grid->dz;

    out_fhdr->grid_minx = _grid->minx;
    out_fhdr->grid_miny = _grid->miny;
    out_fhdr->grid_minz = _grid->minz;

    out_fhdr->bad_data_value = 0;
    out_fhdr->missing_data_value = 0;

    out_fhdr->proj_rotation = _grid->proj_params.flat.rotation;

    out_fhdr->field_code = in_fhdr->field_code;
    STRncopy(out_fhdr->field_name_long, in_fhdr->field_name_long,
	     MDV_LONG_FIELD_LEN);
    STRncopy(out_fhdr->field_name, in_fhdr->field_name, MDV_SHORT_FIELD_LEN);
    STRncopy(out_fhdr->units, in_fhdr->units, MDV_UNITS_LEN);
    STRncopy(out_fhdr->transform, in_fhdr->transform, MDV_TRANSFORM_LEN);
    
    MDV_vlevel_header_t *vhdr = _handle.vlv_hdrs + out_field;
    MDV_init_vlevel_header(vhdr);
    for (int iz = 0; iz < out_fhdr->nz; iz++) {
      vhdr->vlevel_type[iz] = MDV_VERT_TYPE_Z;
      vhdr->vlevel_params[iz] = out_fhdr->grid_minz + iz * out_fhdr->grid_dz;
    }
    
  } // out_field

  // allocate the data arrays

  for (int out_field = 0; out_field < out_mhdr->n_fields; out_field++) {
    for (int iz = 0; iz < _grid->nz; iz++) {
      _handle.field_plane[out_field][iz] = (ui08 *) umalloc
	(_grid->nx * _grid->ny);
    } // iz
  } // out_field
  _handle.field_planes_allocated = TRUE;

}

////////////////////////////////////////////////////
// addToInfo()
//
// Add a string to the info field in the header.
//

void OutputFile::addToInfo(char *info_str)
{
  STRconcat(_handle.master_hdr.data_set_info, info_str, MDV_INFO_LEN);
}
  
////////////////////////////////////////////////////
// clearVol()
//
// Clears out the volume data, initialize
//

void OutputFile::clearVol()
  
{

  int npoints_plane = _grid->nx * _grid->ny;
  for (int out_field = 0; out_field < _handle.master_hdr.n_fields; out_field++) {
    for (int iz = 0; iz < _grid->nz; iz++) {
      memset(_handle.field_plane[out_field][iz], 0, npoints_plane);
    }
  }
  memset(_handle.master_hdr.data_set_info, 0, MDV_INFO_LEN);

}

////////////////////////////////////////////////////
// loadScaleAndBias()
//
// Set scale and bias for a given field
//

void OutputFile::loadScaleAndBias(int out_field, double scale, double bias)
  
{

  MDV_field_header_t *out_fhdr = _handle.fld_hdrs + out_field;
  out_fhdr->scale = scale;
  out_fhdr->bias = bias;
    
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputFile::writeVol(time_t merge_time, time_t start_time, time_t end_time)

{

  MDV_master_header_t *out_mhdr = &_handle.master_hdr;

  out_mhdr->time_gen = time(NULL);

  out_mhdr->time_begin = start_time;
  out_mhdr->time_end = end_time;
  out_mhdr->time_centroid = merge_time;
  //out_mhdr->time_expire = out_mhdr->time_centroid +
  //  (out_mhdr->time_end - out_mhdr->time_begin);
  out_mhdr->time_expire = 0;

  // Use the first field header for the forecast time
  MDV_field_header_t *out_fhdr = _handle.fld_hdrs;
  out_fhdr->forecast_time = out_mhdr->time_centroid;
  
  if (_params->debug >= DEBUG_NORM) {
    fprintf(stderr, "Writing merged MDV file, time %s, to dir %s\n",
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

