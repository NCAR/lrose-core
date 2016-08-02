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

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <Mdv/mdv/mdv_write.h>
#include <Mdv/mdv/mdv_utils.h>
#include <Mdv/mdv/mdv_user.h> 
#include <math.h>   

#include <toolsa/umisc.h>
using namespace std;
  
  // constructor
  // Sets up a master header and makes room for 
  // N field headers in a handle.
  // Specific to the needs of the surface interpolation.
  //
Output::Output(date_time_t DataTime,
               time_t DataDuration,
               int NumFields, int nx, int ny, int nz,
               float lon, float lat, float alt,
               const char *SetInfo, const char *SetName, const char *SetSource,
	       int flat)
{


  MDV_init_handle(&_handle);

  MDV_init_master_header(&_handle.master_hdr);

  _handle.master_hdr.struct_id=MDV_MASTER_HEAD_MAGIC_COOKIE;
  _handle.master_hdr.revision_number=MDV_REVISION_NUMBER;

  time_t Now;
  Now=time(NULL);;

  _handle.master_hdr.time_gen = Now;
  _handle.master_hdr.user_time = DataTime.unix_time;
  _handle.master_hdr.time_begin = DataTime.unix_time - DataDuration;
  _handle.master_hdr.time_end = DataTime.unix_time;
  _handle.master_hdr.time_centroid = DataTime.unix_time;


  _handle.master_hdr.time_expire = DataTime.unix_time + DataDuration;

  _handle.master_hdr.num_data_times = 1;                
  _handle.master_hdr.index_number = 1;

  if (nz == 1){
    _handle.master_hdr.data_dimension = 2;
  } else {
    _handle.master_hdr.data_dimension = 3;
  }

  _handle.master_hdr.data_collection_type = MDV_DATA_MEASURED;
  
  if (flat){
    _handle.master_hdr.user_data = MDV_PROJ_FLAT;
  } else { 
    _handle.master_hdr.user_data = MDV_PROJ_LATLON;
  }

  _handle.master_hdr.vlevel_type = MDV_VERT_TYPE_SURFACE;
  _handle.master_hdr.vlevel_included = 1;
               

  _handle.master_hdr.grid_order_direction=MDV_ORIENT_SN_WE;
  _handle.master_hdr.grid_order_indices = MDV_ORDER_XYZ;

  _handle.master_hdr.n_fields = NumFields;
  _handle.master_hdr.max_nx = nx;
  _handle.master_hdr.max_ny = ny;
  _handle.master_hdr.max_nz = nz; 

  _handle.master_hdr.n_chunks = 0;

  _handle.master_hdr.field_grids_differ= 0;

  _handle.master_hdr.sensor_lon = lon; 
  _handle.master_hdr.sensor_lat = lat;   
  _handle.master_hdr.sensor_alt = alt; 
      
  sprintf(_handle.master_hdr.data_set_info,"%s",SetInfo);
  sprintf(_handle.master_hdr.data_set_name,"%s",SetName);
  sprintf(_handle.master_hdr.data_set_source,"%s",SetSource); /* Should put didss URL here */

  /* Now set up the array of NumFields field headers. */

  MDV_alloc_handle_arrays(&_handle, NumFields, nz, 0); // Zero chunks.
    
  // allocate the data arrays

  for (int out_field = 0; out_field < NumFields; out_field++) {
    for (int iz = 0; iz < nz; iz++) {
      _handle.field_plane[out_field][iz] = (ui08 *) umalloc(nx*ny);
    } // iz
  } // out_field
  _handle.field_planes_allocated = TRUE;


}


///////////////////////////////////////////////////////////////////////////////////

  // AddField
  // Sets up field header number Field where 0 <= Field < NumFields
  void Output::AddField(const char *var_name, const char *short_var_name, const char *var_units,
			int Field, int NumFields,
			float *data, float bad, float missing,
			float dx, float dy, float dz,
			float x0, float y0, float z0,
			int FieldCode, float Bad)
{


  // Reference the right field header.
  MDV_field_header_t *fhdr =_handle.fld_hdrs + Field;

  MDV_init_field_header(fhdr);

  date_time_t Now; ulocaltime (&Now);


  // First fill out integers.
  fhdr->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
  fhdr->field_code = FieldCode;

  fhdr->user_time1 =     Now.unix_time;
  fhdr->forecast_delta = 0; /* This is not a forecast. */
  fhdr->user_time2 =     _handle.master_hdr.time_begin;
  fhdr->user_time3 =     _handle.master_hdr.time_end;
  fhdr->forecast_time =  _handle.master_hdr.time_centroid; /* Not a forecast. */
  fhdr->user_time4 =     _handle.master_hdr.time_expire; 
  fhdr->nx =  _handle.master_hdr.max_nx;
  fhdr->ny =  _handle.master_hdr.max_ny;
  fhdr->nz =  _handle.master_hdr.max_nz;

  if (FieldCode == 5){ // Terrain. Never expires.
    _handle.master_hdr.time_expire += 20*365*86400;
  }


  fhdr->proj_type =   _handle.master_hdr.user_data;
  fhdr->encoding_type = MDV_INT8;

  fhdr->volume_size =  _handle.master_hdr.max_nx*_handle.master_hdr.max_ny*_handle.master_hdr.max_nz;
  // The above assume byte data.

  // Then reals
  fhdr->proj_origin_lat =  _handle.master_hdr.sensor_lat;
  fhdr->proj_origin_lon =  _handle.master_hdr.sensor_lon;
  fhdr->proj_param[0] = 0.0;
  fhdr->proj_param[1] = 0.0;
  fhdr->proj_param[2] = 0.0;
  fhdr->proj_param[3] = 0.0;
  fhdr->proj_param[4] = 0.0;
  fhdr->proj_param[5] = 0.0;
  fhdr->proj_param[6] = 0.0;
  fhdr->proj_param[7] = 0.0;
  fhdr->vert_reference = 0.0;

  fhdr->grid_dx =  dx;
  fhdr->grid_dy =  dy;
  fhdr->grid_dz =  dz;

  fhdr->grid_minx =  x0;
  fhdr->grid_miny =  y0;
  fhdr->grid_minz =  z0;

  fhdr->bad_data_value = bad;
  fhdr->missing_data_value = missing;
  fhdr->proj_rotation = 0.0;

  // Then characters
  sprintf(fhdr->field_name_long,"%s",var_name);
  sprintf(fhdr->field_name,"%s",short_var_name);
  sprintf(fhdr->units,"%s",var_units);
  sprintf(fhdr->transform,"%s"," ");


  // Fill in the vlevel header for this field.

  MDV_vlevel_header_t *vhdr = _handle.vlv_hdrs + Field;
  MDV_init_vlevel_header(vhdr);
  for (int iz = 0; iz < fhdr->nz; iz++) {
    vhdr->vlevel_type[iz] = MDV_VERT_TYPE_SURFACE;
    vhdr->vlevel_params[iz] = fhdr->grid_minz + iz * fhdr->grid_dz;
  }
  


  // Finally, put the data in - allocate space
  // for the bytes, and then calculate the bytes
  // from the data.

 
  float Scale,Bias;

  //
  // Get the scale and bias.
  //
  GetScaleAndBias(data,Bad,fhdr->nx*fhdr->ny*fhdr->nz,
		  &Scale, &Bias);

  //
  // Write them to the field header.
  //
  fhdr->scale = Scale;
  fhdr->bias = Bias;
 
  //
  // Use them to get the output bytes.
  //

  ui08 *outbytes;

  for (int i=0; i<fhdr->nz; i++){
    outbytes= (ui08 *)(_handle.field_plane[Field][i]);

    ApplyScaleAndBias(data, Bad, fhdr->nx*fhdr->ny,
		      outbytes, Bias, Scale,FieldCode);

  }

}

///////////////////////////////////////////////////////////////////////////////////

  // Compute scale and bias.
void Output::GetScaleAndBias(float *data, float bad, int size, float *Scale, float *Bias)
{

  // get min and max for field 
  float min,max;
  int i,j;

  i=-1;
  do{
    i++;
  }while((i<size) && (data[i]==bad));

  if (i==size){ // All data were bad - scale and bias irrelevant.
    *Bias=0.0; *Scale =1.0; return;
  }

  min=data[i]; max=data[i];

  for (j = i+1; j < size; j++) {
    if (data[j]!=bad) {
 
      if (data[j] < min) min=data[j];
      if (data[j] > max) max=data[j];

    }
  }

  // compute scale and bias
  double range = max - min;

  if (range < 1.0e-99){ // Array is constant.
    *Scale = 10.0;
    // *Bias = min - *Scale * 2.0;
  } else {
    *Scale = range / 250.0;
    // *Bias = min - *Scale * 2.0;
  }

  //
  // Round Scale up to nearest factor of 10.0
  //
  *Scale=10.0*ceil(*Scale/10.0);

  //
  // Round Bias down to nearest factor of 10.0
  //
  *Bias= 10.0*floor(min/10.0) - *Scale*2.0;


  return;

}

/////////////////////////////////////////////////////////////////

void Output::ApplyScaleAndBias(float *data, float bad, int size,
		       unsigned char *OutBytes,float Bias, float Scale,
			       int FieldCode)

{

  unsigned char zb;

  for (int i=0; i<size; i++){
    if (data[i]==bad){
      OutBytes[i]=0;
    } else {
      OutBytes[i]=(unsigned char)((data[i]-Bias)/Scale);
      if ((FieldCode == 5) && (Bias <= 0.0)){ //Terrain
	zb=(unsigned char)((0.0-Bias)/Scale);
	if ((OutBytes[i]==zb) && (data[i]>0.05)){ 
	  //
	  // Fix it so that small values of altitude don't
	  // get mapped to 0.0 elevation.
	  //
	  OutBytes[i]=OutBytes[i]+1;
	}
      }
    }
  }

}


/////////////////////////////////////////////////////////////////

  // destructor
  // Actually writes the file, then deallocates all
  // the memory. The variable outdir must have been
// set at this point.

Output::~Output()
{

  int i = MDV_write_all(&_handle, outpath, MDV_PLANE_RLE8);

  if (i!= MDV_SUCCESS){
    fprintf(stderr,"Failed to write data to directory %s\n",outpath);    
  }

  MDV_handle_free_field_planes(&_handle); 
  MDV_free_handle(&_handle); 
 


}
  





