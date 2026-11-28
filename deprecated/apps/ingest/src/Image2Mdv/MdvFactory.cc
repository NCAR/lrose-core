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

////////////////////////////////////////////////////
void MdvFactory::BuildHeaders(Params &params)
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

  STRncopy(_mhdr.data_set_info, "Image data converted using Image2Mdv", MDV_NAME_LEN);
  STRncopy(_mhdr.data_set_source, "Earth Conformal Image", MDV_NAME_LEN);
 
  // Clear the field header workspace
  MEM_zero(_fhdr);

  // Populate the field header
  _fhdr.data_element_nbytes = 4;
  _fhdr.proj_type = params.in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_RGBA32;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = 0;

  _fhdr.native_vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params.proj.origin_lat;
  _fhdr.proj_origin_lon = params.proj.origin_lon;
  _fhdr.proj_param[0] = params.proj.param1;
  _fhdr.proj_param[1] = params.proj.param2;
  _fhdr.proj_param[2] = params.proj.param2;
  _fhdr.proj_param[3] = params.proj.param3;

  _fhdr.grid_dx = params.grid.dx;
  _fhdr.grid_dy = params.grid.dy;
  _fhdr.grid_dz = 0.0;

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
  _fhdr.data_element_nbytes = 4;
  _fhdr.proj_type = params.in_projection;

  _fhdr.encoding_type = Mdvx::ENCODING_RGBA32;

  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = 0;

  _fhdr.native_vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.vlevel_type = Mdvx::VERT_EARTH_IMAGE;
  _fhdr.dz_constant = true;


  _fhdr.proj_origin_lat = params.proj.origin_lat;
  _fhdr.proj_origin_lon = params.proj.origin_lon;

  _fhdr.grid_dx = params.grid.dx;
  _fhdr.grid_dy = params.grid.dy;
  _fhdr.grid_dz = 0.0;

  _fhdr.grid_minx = params.grid.start_x;
  _fhdr.grid_miny = params.grid.start_y;
  _fhdr.grid_minz = 0.0;

  // Clear and then Populate the V level header workspace
  MEM_zero(_vhdr);
  _vhdr.level[0] = 0;
  _vhdr.type[0] = Mdvx::VERT_EARTH_IMAGE;
}

////////////////////////////////////////////////////
void MdvFactory::PutRGB(DATA32 *data, ui32 height, ui32 width, const  char *ImageName,  Params &params)
{
  unsigned char r,g,b,a;
  ui32 abgr;
  DATA32 *d_ptr;

  ui32 *mdv_data;
  ui32 *mdv_ptr;
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

  if((mdv_data = (ui32 *)  calloc(width*height,sizeof(ui32))) == NULL) {
		 perror("PutRGB:  Can't alloc");
		 return;
  }

  mdv_ptr = mdv_data; // Start of our ui32 buffer
  d_ptr = data;       // Start of the Image Pixbuf data

  a = (unsigned char) (params.default_alpha * 255.0); // Set the alpha channel

  // int n_pts = width * height;
  for(int i = 0; i < (int) height; i++) {

   // Reverse the row order - using pointer math
   mdv_ptr = mdv_data + ((height -i -1) * width);

   for(int j = 0; j < (int) width; j++) {
	   b = *d_ptr & 0xff ; // Pull Out B
	   g = *d_ptr >> 8 & 0xff; //      G
	   r = *d_ptr >> 16 & 0xff; //     R
	   d_ptr++; // move to next pixel.

	   if(r == params.RGB_transparent_value.rval &&
		  g == params.RGB_transparent_value.gval &&
		  b == params.RGB_transparent_value.bval) {

	      abgr = r | (g <<8) | (b << 16) ; 

		} else {
	      abgr = r | (g <<8) | (b << 16) | (a << 24); // Build the ui32 value
		}

	   *mdv_ptr++ = abgr;  // Fill in the mdv data  buffer 
	 }
  }

  // Add the data to the  Field
  field->setVolData((void *) mdv_data, _fhdr.volume_size,Mdvx::ENCODING_RGBA32,Mdvx::SCALING_NONE,0.0,0.0);

  // add field to mdvx object after clearing
  _Mdvx_obj.clearFields();
  _Mdvx_obj.addField(field);

  if(mdv_data != NULL) free(mdv_data);

}

////////////////////////////////////////
// WriteFile()
//
// Write out merged volume in MDV format.
//

int MdvFactory::WriteFile(const time_t image_time, string output_fname, Params &params)

{
  int ret = 0;

  // set the master header times.
  _mhdr.time_gen = image_time;
  _mhdr.time_begin = image_time;
  _mhdr.time_end = image_time ;
  _mhdr.time_centroid = image_time;
  _mhdr.time_expire = image_time + 86400000;  // 1000 days

  _Mdvx_obj.setMasterHeader(_mhdr);

  if(params.write_compressed_images) {

      // compress the Image 
      MdvxField *fld = _Mdvx_obj.getFieldByNum(0);
      fld->convertType(Mdvx::ENCODING_ASIS,
                   Mdvx::COMPRESSION_BZIP,
                   Mdvx::SCALING_NONE);
  }

  if(output_fname.size() > 3) {

    if ( params.write_ldata_info )
      _Mdvx_obj.setWriteLdataInfo();
    else
      _Mdvx_obj.clearWriteLdataInfo();

    // Write the file.
    if (_Mdvx_obj.writeToPath(output_fname.c_str())) {
      cerr << "ERROR - Failed to write " << output_fname << endl;
      cerr << _Mdvx_obj.getErrStr() << endl;
      ret = -1;
    }
  } else {
    // Write the file.
    if (_Mdvx_obj.writeToDir(params.output_dir)) {
      cerr << "ERROR - Failed to write to " << params.output_dir  << endl;
      cerr << _Mdvx_obj.getErrStr() << endl;
      ret = -1;
    }

    // Mdvx does this automagically.
    //   Taking it out... -ptm.
    // 
    // if(params.write_ldata_info) {
    //   // Update LdataInfo file
    //   DsLdataInfo ldatainfo;
    //   ldatainfo.setDir(params.output_dir);
    //   ldatainfo.setDataFileExt("mdv");
    //   ldatainfo.write(image_time);
    // }
  }
  
  return ret;
}

