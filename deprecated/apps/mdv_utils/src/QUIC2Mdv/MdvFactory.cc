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
// After, Paddy McCarthy, after  Mike Dixon, after Frank Hage.
//
///////////////////////////////////////////////////////////////
using namespace std;

#include "MdvFactory.hh"
// From Rong-Sheu
#define GRID_DX 0.004;  // km
#define GRID_DY 0.004;  // km
#define GRID_DZ 0.004;  // km
#define GRID_MINX 0.0;  // km
#define GRID_MINY 0.0;  // km
#define GRID_MINZ 0.0;  // km
#define GRID_ORIGIN_LAT 39.286189;  // degrees
#define GRID_ORIGIN_LON -76.623519; // degrees
#define DELTA_T 30


////////////////////////////////////////////////////
void MdvFactory::BuildHeaders(void )
{
 
  // Clear the master header workspace
  MEM_zero(_mhdr);

  // Populate the Master Header Workspace
  _mhdr.num_data_times = 1;
  _mhdr.data_dimension = 2;

  _mhdr.data_collection_type = Mdvx::DATA_FORECAST;
  _mhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  _mhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  _mhdr.vlevel_included = FALSE;
  _mhdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  _mhdr.data_ordering = Mdvx::ORDER_XYZ;

  _mhdr.n_chunks = 0;
  _mhdr.field_grids_differ = FALSE;
  _mhdr.sensor_lon = 0.0;
  _mhdr.sensor_lat = 0.0;
  _mhdr.sensor_alt = 0.0;

  STRncopy(_mhdr.data_set_info, "Image data converted from QUICK OUTPUT", MDV_NAME_LEN);
  STRncopy(_mhdr.data_set_source, "QUIC", MDV_NAME_LEN);
 
  // Clear the field header workspace
  MEM_zero(_fhdr);

  // Populate the field header
  _fhdr.data_element_nbytes = 4;
  _fhdr.proj_type = Mdvx::PROJ_FLAT;

  _fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  _fhdr.compression_type = 0; // Mdvx::COMPRESSION_NONE -clashs with libtiff define

  _fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  _fhdr.scaling_type = Mdvx::SCALING_NONE;

  _fhdr.scale = 1;
  _fhdr.bias = 0;

  _fhdr.native_vlevel_type = Mdvx::VERT_TYPE_Z;
  _fhdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  _fhdr.dz_constant = true;

  _fhdr.proj_origin_lat = GRID_ORIGIN_LAT;
  _fhdr.proj_origin_lon = GRID_ORIGIN_LON;
  _fhdr.proj_param[0] = 0.0;
  //_fhdr.proj_param[1] = params.proj.param2;
  //_fhdr.proj_param[2] = params.proj.param2;
  //_fhdr.proj_param[3] = params.proj.param3;

  _fhdr.grid_dx = GRID_DX;
  _fhdr.grid_dy = GRID_DY;
  _fhdr.grid_dz = GRID_DZ;

  _fhdr.grid_minx = GRID_MINZ;
  _fhdr.grid_miny = GRID_MINZ;
  _fhdr.grid_minz = GRID_MINZ;
 
  _fhdr.proj_rotation = 0.0;
   
  _fhdr.bad_data_value =  -9999.0;
  _fhdr.missing_data_value = -9999.99;

  _fhdr.field_code = 0;

  STRncopy(_fhdr.field_name_long, "Concentration", MDV_LONG_FIELD_LEN);
  STRncopy(_fhdr.field_name, "conc", MDV_SHORT_FIELD_LEN);
  STRncopy(_fhdr.units, "ppm", MDV_UNITS_LEN);
  STRncopy(_fhdr.transform, "From QUIC", MDV_TRANSFORM_LEN);

  // Clear and then Populate the V level header workspace
  MEM_zero(_vhdr);
  _vhdr.level[0] = 0;
  _vhdr.type[0] = Mdvx::VERT_TYPE_Z;
}

////////////////////////////////////////////////////
void MdvFactory::PutData(double *data, uint32 height, uint32 width, uint32 depth)
{

  fl32 *mdv_data;
  fl32 *mdv_ptr;
  double *data_ptr;
  int num_3dpoints;
  _mhdr.max_nx = width;
  _mhdr.max_ny = height;
  _mhdr.max_nz = depth;
  num_3dpoints = width * height * depth;

  STRncopy(_mhdr.data_set_name, "QUIC Output" , MDV_NAME_LEN);

  _fhdr.nx = width;
  _fhdr.ny = height;
  _fhdr.nz = depth;

  _fhdr.volume_size = _fhdr.nx * _fhdr.ny * _fhdr.nz * _fhdr.data_element_nbytes;

  // create field in our Mdvx Object from our field header workspace
  MdvxField *field = new MdvxField(_fhdr, _vhdr, NULL, true);

  if((mdv_data = (fl32 *)  calloc(width*height*depth,sizeof(fl32))) == NULL) {
		 perror("PutData:  Can't alloc");
		 return;
  }

  mdv_ptr = mdv_data; // Start of our float32 (output) buffer
  data_ptr = data; // Start of our input buffer
  for(int i = 0; i < num_3dpoints; i++ ) {
		  *mdv_ptr++ = *data_ptr++;
  }

  // Add the data to the  Field
  field->setVolData((void *) mdv_data, _fhdr.volume_size,Mdvx::ENCODING_FLOAT32,Mdvx::SCALING_NONE,0.0,0.0);

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

int MdvFactory::WriteFile(const time_t data_time, const char *output_dir )

{
  int ret = 0;

  // set the master header times.
  _mhdr.time_gen = data_time;
  _mhdr.time_begin = data_time;
  _mhdr.time_end = data_time ;
  _mhdr.time_centroid = data_time;
  _mhdr.time_expire = data_time + DELTA_T; 

  _Mdvx_obj.setMasterHeader(_mhdr);

  // Write the file.
  if (_Mdvx_obj.writeToDir(output_dir)) {
    cerr << "ERROR - OutputFile::writeVol" << endl;
    cerr << _Mdvx_obj.getErrStr() << endl;
    ret = -1;
  }

  if(1) {
      // Update LdataInfo file
      DsLdataInfo ldatainfo;
      ldatainfo.setDir(output_dir);
      ldatainfo.setDataFileExt("mdv");
      ldatainfo.write(data_time);
  }
  
  return ret;
}

