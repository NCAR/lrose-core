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
// MdvDataFactory.cc
//
// Handles output to MDV files in data format (not RGBA).
//   This is a hard-coded class that takes image pixel colors
//   and inteprets them as data values (useful for grayscale
//   images). It can serve as an example of how to do this,
//   but if there are other uses for this, the conversion
//   method and scale/bias need to be parameterized.
//
// Paddy McCarthy, RAP NCAR 
//
// After F. Hage, after, Paddy McCarthy, after  Mike Dixon
//
///////////////////////////////////////////////////////////////
using namespace std;

#include "MdvDataFactory.hh"

////////////////////////////////////////////////////
void MdvDataFactory::BuildHeaders(Params &params)
{
 
  // Clear the master header workspace
  MEM_zero(_mhdr);

  // Populate the Master Header Workspace
  _mhdr.num_data_times = 1;
  _mhdr.data_dimension = 2;

  _mhdr.data_collection_type = Mdvx::DATA_MEASURED;
  _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _mhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _mhdr.vlevel_included = FALSE;
  _mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  _mhdr.data_ordering = Mdvx::ORDER_XYZ;

  _mhdr.n_chunks = 0;
  _mhdr.field_grids_differ = FALSE;
  _mhdr.sensor_lon = 0.0;
  _mhdr.sensor_lat = 0.0;
  _mhdr.sensor_alt = 0.0;

  STRncopy(_mhdr.data_set_info, "Data file created from image using Image2Mdv", MDV_NAME_LEN);
  STRncopy(_mhdr.data_set_source, "Earth Conformal Data", MDV_NAME_LEN);
 
  // Clear the field header workspace
  MEM_zero(_fhdr);

  // Populate the field header
  _fhdr.data_element_nbytes = 1;
  _fhdr.proj_type = params.in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_INT8;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = -35;

  _fhdr.native_vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _fhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params.proj.origin_lat;
  _fhdr.proj_origin_lon = params.proj.origin_lon;
  _fhdr.proj_param[0] = params.proj.param1;
  _fhdr.proj_param[1] = params.proj.param2;
  _fhdr.proj_param[2] = params.proj.param2;
  _fhdr.proj_param[3] = params.proj.param3;

  _fhdr.grid_dx = params.grid.dx;
  _fhdr.grid_dy = params.grid.dy;
  _fhdr.grid_dz = 1.0;

  _fhdr.grid_minx = params.grid.start_x;
  _fhdr.grid_miny = params.grid.start_y;
  _fhdr.grid_minz = 0.0;
 
  _fhdr.proj_rotation = 0.0;
   
  _fhdr.bad_data_value = (fl32) params.missing_data_color;
  _fhdr.missing_data_value = (fl32) params.missing_data_color;

  _fhdr.field_code = 0;

  STRncopy(_fhdr.field_name_long, params.field_name_long, MDV_LONG_FIELD_LEN);
  STRncopy(_fhdr.field_name, params.field_name, MDV_SHORT_FIELD_LEN);
  STRncopy(_fhdr.units, params.field_units, MDV_UNITS_LEN);
  STRncopy(_fhdr.transform, "From Image", MDV_TRANSFORM_LEN);


  // Populate the field header
  _fhdr.data_element_nbytes = 1;
  _fhdr.proj_type = params.in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_INT8;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = -35;

  _fhdr.native_vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _fhdr.vlevel_type = Mdvx::VERT_TYPE_COMPOSITE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params.proj.origin_lat;
  _fhdr.proj_origin_lon = params.proj.origin_lon;

  _fhdr.grid_dx = params.grid.dx;
  _fhdr.grid_dy = params.grid.dy;
  _fhdr.grid_dz = 1.0;

  _fhdr.grid_minx = params.grid.start_x;
  _fhdr.grid_miny = params.grid.start_y;
  _fhdr.grid_minz = 0.0;

  // Clear and then Populate the V level header workspace
  MEM_zero(_vhdr);
  _vhdr.level[0] = 0;
  _vhdr.type[0] = Mdvx::VERT_TYPE_COMPOSITE;
}

////////////////////////////////////////////////////
void MdvDataFactory::PutRGB(DATA32 *data, ui32 height, ui32 width, const  char *ImageName,  Params &params)
{
  unsigned char r,g,b,a;
  DATA32 *d_ptr;
  si08 *mdv_char_data;
  si08 *mdv_char_ptr;

  _mhdr.max_nx = width;
  _mhdr.max_ny = height;
  _mhdr.max_nz = 1;

  STRncopy(_mhdr.data_set_name, ImageName , MDV_NAME_LEN);

  _fhdr.nx = width;
  _fhdr.ny = height;
  _fhdr.nz = 1;

  _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz * _fhdr.data_element_nbytes;

  // create field in our Mdvx Object from our field header workspace
  MdvxField *field = new MdvxField(_fhdr, _vhdr, NULL, true);

  if((mdv_char_data = (si08 *)  calloc(width*height,sizeof(si08))) == NULL) {
		 perror("PutRGB:  Can't alloc char array");
		 return;
  }

  mdv_char_ptr = mdv_char_data;
  d_ptr = data;       // Start of the Image Pixbuf data

  a = (unsigned char) (params.default_alpha * 255.0); // Set the alpha channel

  // int n_pts = width * height;
  for(int i = 0; i < (int) height; i++) {

   mdv_char_ptr = mdv_char_data + ((height -i -1) * width);

   for(int j = 0; j < (int) width; j++) {
	   b = *d_ptr & 0xff ; // Pull Out B
	   g = *d_ptr >> 8 & 0xff; //      G
	   r = *d_ptr >> 16 & 0xff; //     R
	   d_ptr++; // move to next pixel.

       // int value = (int) ( ( ((float)r) - 20.0 ) / 5.0 );
	   si08 value = (si08) r;

	   // if (value != -4)
	   //     cerr << "Orig value: " << ((int) r) << " Computed value: " << value << endl;;

	   *mdv_char_ptr++ = value;  // Fill in the mdv data  buffer 
	 }
  }

  // Add the data to the  Field
  field->setVolData((void *) mdv_char_data, _fhdr.volume_size,Mdvx::ENCODING_INT8,Mdvx::SCALING_NONE,1.0,-35.0);

  // add field to mdvx object after clearing
  _Mdvx_obj.clearFields();
  _Mdvx_obj.addField(field);

  if(mdv_char_data != NULL) free(mdv_char_data);

}

