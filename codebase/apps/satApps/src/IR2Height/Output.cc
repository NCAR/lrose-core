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
// Output.cc
//
// Output class - handles the output to MDV files
//
// Niles Oien
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan  1999
//
///////////////////////////////////////////////////////////////

#include "Output.hh"
#include "Params.hh"

#include <toolsa/str.h>
#include <toolsa/mem.h>

#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxFieldCode.hh>   

#include <toolsa/umisc.h>
  
  // constructor
  // Sets up a master header and makes room for 
  // N field headers in a handle.
  // Specific to the needs of the surface interpolation.
  //
Output::Output(time_t DataTime,
	       time_t DataDuration,
	       int NumFields, int nx, int ny, int nz,
	       float lon, float lat, float alt,
	       const char *SetInfo, const char *SetName, const char *SetSource,
	       Params::projection_t proj)
{


  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();;
 

  master_hdr.struct_id=Mdvx::MASTER_HEAD_MAGIC_COOKIE_64;
  master_hdr.revision_number=Mdvx::REVISION_NUMBER;

  time_t Now;
  Now=time(NULL);;

  master_hdr.time_gen = Now;
  master_hdr.user_time = DataTime;
  master_hdr.time_begin = DataTime - DataDuration;
  master_hdr.time_end = DataTime;
  master_hdr.time_centroid = DataTime;


  master_hdr.time_expire = DataTime + DataDuration;

  master_hdr.num_data_times = 1;                
  master_hdr.index_number = 1;

  if (nz == 1){
    master_hdr.data_dimension = 2;
  } else {
    master_hdr.data_dimension = 3;
  }

  master_hdr.data_collection_type = Mdvx::DATA_MEASURED;
  
  if (proj == Params::FLAT)    master_hdr.user_data = Mdvx::PROJ_FLAT;
  if (proj == Params::LATLON)  master_hdr.user_data = Mdvx::PROJ_LATLON;
  if (proj == Params::LAMBERT) master_hdr.user_data = Mdvx::PROJ_LAMBERT_CONF;
 

  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
               
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;  

  master_hdr.n_fields = NumFields;
  master_hdr.max_nx = nx;
  master_hdr.max_ny = ny;
  master_hdr.max_nz = nz; 

  master_hdr.n_chunks = 0;

  master_hdr.field_grids_differ= 0;

  master_hdr.sensor_lon = lon; 
  master_hdr.sensor_lat = lat;   
  master_hdr.sensor_alt = alt; 
      
  sprintf(master_hdr.data_set_info,"%s",SetInfo);
  sprintf(master_hdr.data_set_name,"%s",SetName);
  sprintf(master_hdr.data_set_source,"%s",SetSource); /* Should put didss URL here */


  mdvx.setMasterHeader(master_hdr); 
  mdvx.clearFields();

}

/////////////////////////////////////////////////////////////////
// destructor

Output::~Output()
{

}
  

///////////////////////////////////////////////////////////////////////////////////

  // AddField
  // Sets up field header number Field where 0 <= Field < NumFields
  void Output::AddField(const char *var_name, const char *short_var_name, const char *var_units,
			int Field, int NumFields,
			float *data, float bad, float missing,
			float dx, float dy, float dz,
			float x0, float y0, float z0,
			int FieldCode)
{


  Mdvx::master_header_t master_hdr = mdvx.getMasterHeader();

  Mdvx::field_header_t fhdr;
  MEM_zero(fhdr);
  Mdvx::vlevel_header_t vhdr;
  MEM_zero(vhdr);               

  date_time_t Now; ulocaltime (&Now);


  // First fill out integers.
  fhdr.struct_id = Mdvx::FIELD_HEAD_MAGIC_COOKIE_64;
  fhdr.field_code = FieldCode;

  fhdr.user_time1 =     Now.unix_time;
  fhdr.forecast_delta = 0; /* This is not a forecast. */
  fhdr.user_time2 =     master_hdr.time_begin;
  fhdr.user_time3 =     master_hdr.time_end;
  fhdr.forecast_time =  master_hdr.time_centroid; /* Not a forecast. */
  fhdr.user_time4 =     master_hdr.time_expire; 
  fhdr.nx =  master_hdr.max_nx;
  fhdr.ny =  master_hdr.max_ny;
  fhdr.nz =  master_hdr.max_nz;

  if (FieldCode == 5){ // Terrain. Never expires.
    master_hdr.time_expire += 20*365*86400;
  }


  fhdr.proj_type =   master_hdr.user_data;


  // Then reals
  fhdr.proj_origin_lat =  master_hdr.sensor_lat;
  fhdr.proj_origin_lon =  master_hdr.sensor_lon;
  fhdr.proj_param[0] = 0.0;
  fhdr.proj_param[1] = 0.0;
  fhdr.proj_param[2] = 0.0;
  fhdr.proj_param[3] = 0.0;
  fhdr.proj_param[4] = 0.0;
  fhdr.proj_param[5] = 0.0;
  fhdr.proj_param[6] = 0.0;
  fhdr.proj_param[7] = 0.0;
  fhdr.vert_reference = 0.0;

  fhdr.grid_dx =  dx;
  fhdr.grid_dy =  dy;
  fhdr.grid_dz =  dz;

  fhdr.grid_minx =  x0;
  fhdr.grid_miny =  y0;
  fhdr.grid_minz =  z0;

  fhdr.bad_data_value = bad;
  fhdr.missing_data_value = bad;
  fhdr.proj_rotation = 0.0;

  // Then characters
  sprintf(fhdr.field_name_long,"%s",var_name);
  sprintf(fhdr.field_name,"%s",short_var_name);
  sprintf(fhdr.units,"%s",var_units);
  sprintf(fhdr.transform,"%s"," ");

  fhdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fhdr.data_element_nbytes = sizeof(fl32);
  fhdr.volume_size = fhdr.nx * fhdr.ny * fhdr.nz * sizeof(fl32);
  fhdr.compression_type = Mdvx::COMPRESSION_NONE;
  fhdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  fhdr.scaling_type = Mdvx::SCALING_NONE;     
  
  for (int iz = 0; iz < fhdr.nz; iz++) {
    vhdr.type[iz] = Mdvx::VERT_TYPE_SURFACE;
    vhdr.level[iz] = fhdr.grid_minz + iz * fhdr.grid_dz;
  }
  
  // create field
  
  MdvxField *field = new MdvxField(fhdr, vhdr, data);    
  

  if (field->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
   fprintf(stderr, "convertRounded failed - I cannot go on.\n");
    exit(-1);
  }

  // add field to mdvx object

  mdvx.addField(field);       
                 
}


/////////////////////////////////////////////////////////////////
// Write the file.
//
// Returns 0 on success, -1 on failure

int Output::write(char *output_url)
{
  
  mdvx.setWriteLdataInfo();
  if (mdvx.writeToDir(output_url)) {
    cerr << "ERROR - Output::write" << endl;
    cerr << "  Cannot write to url: " << output_url << endl;
    cerr << mdvx.getErrStr() << endl;
    return -1;
  }
          
  return 0;

}
  



