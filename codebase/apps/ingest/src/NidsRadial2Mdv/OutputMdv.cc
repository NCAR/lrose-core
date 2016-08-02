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
// OutputMdv.cc
//
// Handles output to MDV files
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include "OutputMdv.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <toolsa/udatetime.h>
#include <Mdv/mdv/mdv_write.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_user.h>
#include <dsserver/DsLdataInfo.hh>
using namespace std;

//////////////
// Constructor

OutputMdv::OutputMdv(const string &prog_name, const Params &params) :
  _progName(prog_name), _params(params)

{
  
  MDV_init_handle(&_handle);
  memset(&_grid, 0, sizeof(mdv_grid_t));

  _grid.nx = _params.output_grid.nx;
  _grid.ny = _params.output_grid.ny;
  _grid.nz = 1;

  _grid.dx = _params.output_grid.dx;
  _grid.dy = _params.output_grid.dy;
  _grid.dz = 1.0;
  
  _grid.minx = _params.output_grid.minx;
  _grid.miny = _params.output_grid.miny;

  _grid.proj_type = MDV_PROJ_FLAT;

  strcpy(_grid.unitsx, "km");
  strcpy(_grid.unitsy, "km");
  strcpy(_grid.unitsz, "deg");

  // allocate MDV arrays
  
  MDV_alloc_handle_arrays(&_handle, 1, 1, 0);

  // allocate the data array
  
  _handle.field_plane[0][0] =
    (ui08 *) umalloc (_grid.nx * _grid.ny);
  _handle.field_planes_allocated = TRUE;

}

/////////////
// destructor

OutputMdv::~OutputMdv()

{
  
  MDV_free_handle(&_handle);

}

///////////////////////////////////////////////////
// setRadarPos
//
// Set the radar position in the grid struct
//

void OutputMdv::setRadarPos(const double radar_lat,
			    const double radar_lon,
			    const double radar_ht,
			    const double elev_angle)

{
  _grid.proj_origin_lat = radar_lat;
  _grid.proj_origin_lon = radar_lon;
  _grid.sensor_lat = radar_lat;
  _grid.sensor_lon = radar_lon;
  _grid.sensor_z = radar_ht;
  _grid.minz = elev_angle;
}

////////////////////////////////////////////////////
// initHeaders()
//
// Initialize the volume headers.

void OutputMdv::initHeaders(const char *radar_name,
			    const char *field_name,
			    const char *field_units,
			    const char *field_transform,
			    const int field_code)
  
{

  // clear the master header
  
  MDV_master_header_t *out_mhdr = &_handle.master_hdr;
  MDV_init_master_header(out_mhdr);
  
  // fill the master header
  
  out_mhdr->num_data_times = 1;
  out_mhdr->data_dimension = 2;
  out_mhdr->data_collection_type = MDV_DATA_MEASURED;
  out_mhdr->native_vlevel_type = MDV_VERT_TYPE_ELEV;
  out_mhdr->vlevel_type = MDV_VERT_TYPE_ELEV;
  out_mhdr->vlevel_included = TRUE;
  out_mhdr->grid_order_direction = MDV_ORIENT_SN_WE;
  out_mhdr->grid_order_indices = MDV_ORDER_XYZ;
  out_mhdr->n_fields = 1;
  out_mhdr->max_nx = _grid.nx;
  out_mhdr->max_ny = _grid.ny;
  out_mhdr->max_nz = _grid.nz;
  out_mhdr->n_chunks = 0;
  out_mhdr->field_grids_differ = FALSE;
  out_mhdr->sensor_lon = _grid.sensor_lon;
  out_mhdr->sensor_lat = _grid.sensor_lat;
  out_mhdr->sensor_alt = _grid.sensor_z;

  // data set name and source
  
  STRncopy(out_mhdr->data_set_info,
	   "Nids data remapped using NidsRadial2Mdv", MDV_NAME_LEN);
  STRncopy(out_mhdr->data_set_name, radar_name, MDV_NAME_LEN);
  STRncopy(out_mhdr->data_set_source, "Nids", MDV_NAME_LEN);
  
  // fill in field header and vlevel header
  
  MDV_field_header_t *out_fhdr = _handle.fld_hdrs;
  MDV_init_field_header(out_fhdr);

  out_fhdr->nx = _grid.nx;
  out_fhdr->ny = _grid.ny;
  out_fhdr->nz = _grid.nz;
  out_fhdr->proj_type = _grid.proj_type;
  out_fhdr->encoding_type = MDV_INT8;
  out_fhdr->data_element_nbytes = 1;
  out_fhdr->field_data_offset = 0;
  out_fhdr->volume_size =
    out_fhdr->nx * out_fhdr->ny * out_fhdr->nz * sizeof(ui08);

  out_fhdr->proj_origin_lat = _grid.proj_origin_lat;
  out_fhdr->proj_origin_lon = _grid.proj_origin_lon;

  out_fhdr->grid_dx = _grid.dx;
  out_fhdr->grid_dy = _grid.dy;
  out_fhdr->grid_dz = _grid.dz;

  out_fhdr->grid_minx = _grid.minx;
  out_fhdr->grid_miny = _grid.miny;
  out_fhdr->grid_minz = _grid.minz;
  
  out_fhdr->bad_data_value = 0;
  out_fhdr->missing_data_value = 0;

  out_fhdr->proj_rotation = _grid.proj_params.flat.rotation;

  out_fhdr->field_code = field_code;
  STRncopy(out_fhdr->field_name_long, field_name, MDV_LONG_FIELD_LEN);
  STRncopy(out_fhdr->field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(out_fhdr->units, field_units, MDV_UNITS_LEN);
  STRncopy(out_fhdr->transform, field_transform, MDV_TRANSFORM_LEN);
  
  MDV_vlevel_header_t *vhdr = _handle.vlv_hdrs;
  MDV_init_vlevel_header(vhdr);
  vhdr->vlevel_type[0] = MDV_VERT_TYPE_ELEV;
  vhdr->vlevel_params[0] = out_fhdr->grid_minz;
  
}

////////////////////////////////////////////////////
// clearVol()
//
// Clears out the volume data, initialize
//

void OutputMdv::clearVol()
  
{
  
  int npoints_plane = _grid.nx * _grid.ny;
  memset(_handle.field_plane[0][0], 0, npoints_plane);

}

////////////////////////////////////////////////////
// loadScaleAndBias()
//
// Set scale and bias for a given field
//

void OutputMdv::loadScaleAndBias(const double scale, const double bias)
  
{

  MDV_field_header_t *out_fhdr = _handle.fld_hdrs;
  out_fhdr->scale = scale;
  out_fhdr->bias = bias;
    
}

////////////////////////////////////////
// writeVol()
//
// Write out merged volume in MDV format.
//

int OutputMdv::writeVol(const time_t scan_time,
			const char *output_dir)

{

  MDV_master_header_t *out_mhdr = &_handle.master_hdr;

  out_mhdr->time_gen = time(NULL);

  out_mhdr->time_begin = scan_time;
  out_mhdr->time_end = scan_time;
  out_mhdr->time_centroid = scan_time;
  out_mhdr->time_expire = scan_time + 600;
  
  if (_params.debug >= Params::DEBUG_NORM) {
    fprintf(stderr, "Writing merged MDV file, time %s, to dir %s\n",
	    utimstr(out_mhdr->time_centroid), output_dir);
  }
  
  // write to directory
  
  if (MDV_write_to_dir(&_handle, (char *) output_dir,
		       MDV_PLANE_RLE8, FALSE) != MDV_SUCCESS) {
    return (-1);
  }

  // write DsLdata info

  DsLdataInfo ldata(output_dir);
  ldata.setDataFileExt("mdv");
  if (ldata.write(scan_time)) {
    return (-1);
  }

  return (0);

}

