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
// MdvFactory.cc
//
// Handles output to MDV files
//
// F. Hage, RAP NCAR 
//
// After, Paddy McCarthy, after  Mike Dixon
//
///////////////////////////////////////////////////////////////
using namespace std;

#include "MdvFactory.hh"
#include <Imlib2.h>


////////////////////////////////////////////////////
void MdvFactory::BuildHeaders(Params *params,
    const char * field_name, double min_x, double min_y, double dx, double dy)
{
 
  // Clear the master header workspace
  MEM_zero(_mhdr);

  // Populate the Master Header Workspace
  _mhdr.num_data_times = 1;
  _mhdr.data_dimension = 2;

  _mhdr.data_collection_type = Mdvx::DATA_GRAPHIC;
  _mhdr.native_vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _mhdr.vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _mhdr.vlevel_included = FALSE;
  _mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  _mhdr.data_ordering = Mdvx::ORDER_XYZ;

  _mhdr.n_chunks = 0;
  _mhdr.field_grids_differ = FALSE;
  _mhdr.sensor_lon = 0.0;
  _mhdr.sensor_lat = 0.0;
  _mhdr.sensor_alt = 0.0;

  STRncopy(_mhdr.data_set_info, "Image data converted From WMS", MDV_NAME_LEN);
  STRncopy(_mhdr.data_set_source, "WMS Image", MDV_NAME_LEN);
 
  // Clear the field header workspace
  MEM_zero(_fhdr);

  // Populate the field header
  _fhdr.data_element_nbytes = 4;
  _fhdr.proj_type = params->in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_RGBA32;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = 0;

  _fhdr.native_vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params->proj.origin_lat;
  _fhdr.proj_origin_lon = params->proj.origin_lon;

  _fhdr.grid_dx = dx;
  _fhdr.grid_dy = dy;
  _fhdr.grid_dz = 0.0;

  _fhdr.grid_minx = min_x;
  _fhdr.grid_miny = min_y;
  _fhdr.grid_minz = 0.0;
 
  _fhdr.proj_rotation = 0.0;
   
  _fhdr.bad_data_value = (fl32) 0.0;
  _fhdr.missing_data_value = (fl32) 0.0;

  _fhdr.field_code = 0;

  STRncopy(_fhdr.field_name_long, field_name, MDV_LONG_FIELD_LEN);
  STRncopy(_fhdr.field_name, field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(_fhdr.units, params->field_units, MDV_UNITS_LEN);
  STRncopy(_fhdr.transform, "From WMS", MDV_TRANSFORM_LEN);


  // Populate the field header
  _fhdr.data_element_nbytes = 4;
  _fhdr.proj_type = params->in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_RGBA32;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = 0;

  _fhdr.native_vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params->proj.origin_lat;
  _fhdr.proj_origin_lon = params->proj.origin_lon;

  _fhdr.grid_dx = dx;
  _fhdr.grid_dy = dy;
  _fhdr.grid_dz = 0.0;

  _fhdr.grid_minx = min_x;
  _fhdr.grid_miny = min_y;
  _fhdr.grid_minz = 0.0;

  // Clear and then Populate the V level header workspace
  MEM_zero(_vhdr);
  _vhdr.level[0] = 0;
  _vhdr.type[0] = Mdvx::VERT_EARTH_IMAGE;
}

////////////////////////////////////////////////////
void MdvFactory::PutRGB(DATA32 *data, uint32 height, uint32 width, const  char *ImageName,  Params *params)
{
  unsigned char r,g,b,a;
  uint32 abgr;
  uint32 bgr;
  uint32 trans_bgr;
  DATA32 *d_ptr;

  uint32 *mdv_data;
  uint32 *mdv_ptr;
  _mhdr.max_nx = width;
  _mhdr.max_ny = height;
  _mhdr.max_nz = 1;

  STRncopy(_mhdr.data_set_name, ImageName , MDV_NAME_LEN);

  _fhdr.nx = width;
  _fhdr.ny = height;
  _fhdr.nz = 1;

  _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz * _fhdr.data_element_nbytes;

  // Dx, DY should be positive. - Origin is lower left.
  if(_fhdr.grid_dy < 0.0) {  // Invert Coords.
	  _fhdr.grid_miny += height * _fhdr.grid_dy;
	  _fhdr.grid_dy =  -(_fhdr.grid_dy);
  } 
 
  if(_fhdr.grid_dx < 0.0) {  // Invert Coords.
	  _fhdr.grid_minx += width * _fhdr.grid_dx;
	  _fhdr.grid_dx =  -(_fhdr.grid_dx);
  } 

  // create field in our Mdvx Object from our field header workspace
  MdvxField *field = new MdvxField(_fhdr, _vhdr, NULL, true);

  if((mdv_data = (uint32 *)  calloc(width*height,sizeof(uint32))) == NULL) {
		 perror("PutRGB:  Can't alloc");
		 return;
  }

  mdv_ptr = mdv_data; // Start of our uint32 buffer
  d_ptr = data;       // Start of the Image ARGB data

  a = (unsigned char) (params->default_alpha * 255.0); // Set the alpha channel
  trans_bgr = params->transparent_color & 0xffffff;

  int n_pts = width * height;
  for(int i = 0; i < height; i++) {

   // Reverse the row order - using pointer math
   mdv_ptr = mdv_data + ((height -i -1) * width);

   for(int j = 0; j < width; j++) {
	   r = (*d_ptr >> 16) & 0xff; // Pull Out R
	   g = (*d_ptr >> 8) & 0xff;  //          G
	   b = (*d_ptr & 0x0FF);      //          B
	   d_ptr++;

	   bgr = r | (g <<8) | (b << 16);  // Build the 24 bit value

	   if(params->set_background_transparent) {
		 if(bgr == trans_bgr) {
		   abgr = 0;  // Alpha is 0, same with RGB.
		 } else {
		   abgr = bgr | (a << 24); // Build the uint32 value
		 }
	   }  else {
	       abgr = bgr | (a << 24); // Build the uint32 value
	   }

	   *mdv_ptr++ = abgr;  // Fill in the mdv data  buffer 
	 }
  }

  // Add the data to the  Field
  field->setVolData((void *) mdv_data, _fhdr.volume_size,
			 Mdvx::ENCODING_RGBA32,Mdvx::SCALING_NONE,0.0,0.0);

  // add field to mdvx object after clearing
  _Mdvx_obj->clearFields();
  _Mdvx_obj->addField(field);

  if(mdv_data != NULL) free(mdv_data);

}
