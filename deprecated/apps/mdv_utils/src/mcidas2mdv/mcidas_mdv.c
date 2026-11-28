/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/*////////////////////////////////////////////////////////////
// mcidas_mdv.c
//
// Handles the output to MDV files
//
// Niles Oien
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June  1999
//
//////////////////////////////////////////////////////////////*/

#include "mcidas2mdv.h"

#include <toolsa/str.h>
#include <toolsa/mem.h>
#include <mdv/mdv_write.h>
#include <mdv/mdv_utils.h>
#include <mdv/mdv_user.h>    

#include <toolsa/umisc.h>
  

void mcidas_mdv(time_t DataTime, char *file_path,
	   char *field, char *source, ui08 *data)
{
  time_t Now;
  int i;
  MDV_handle_t _handle; 
  MDV_vlevel_header_t *vhdr;
  MDV_field_header_t *fhdr;



  Now=time(NULL);

  MDV_init_handle(&_handle);
  MDV_init_master_header(&_handle.master_hdr);


  _handle.master_hdr.struct_id=MDV_MASTER_HEAD_MAGIC_COOKIE;
  _handle.master_hdr.revision_number=MDV_REVISION_NUMBER;


  _handle.master_hdr.time_gen = Now;
  _handle.master_hdr.user_time = DataTime;
  _handle.master_hdr.time_begin = DataTime;
  _handle.master_hdr.time_end = DataTime;
  _handle.master_hdr.time_centroid = DataTime;


  _handle.master_hdr.time_expire = 0;

  _handle.master_hdr.num_data_times = 1;                
  _handle.master_hdr.index_number = 1;

  _handle.master_hdr.data_dimension = 2;

  _handle.master_hdr.data_collection_type = MDV_DATA_MEASURED;
  
  _handle.master_hdr.user_data = MDV_PROJ_FLAT;

  _handle.master_hdr.vlevel_type = MDV_VERT_TYPE_SIGMA_Z;
  _handle.master_hdr.vlevel_included = MDV_VERT_TYPE_SURFACE;
               

  _handle.master_hdr.grid_order_direction=MDV_ORIENT_SN_WE;
  _handle.master_hdr.grid_order_indices = MDV_ORDER_XYZ;

  _handle.master_hdr.n_fields = 1;
  _handle.master_hdr.max_nx = Glob->params.output_grid.nx;
  _handle.master_hdr.max_ny = Glob->params.output_grid.ny;
  _handle.master_hdr.max_nz = 1; 

  _handle.master_hdr.n_chunks = 0;

  _handle.master_hdr.field_grids_differ= 0;

  _handle.master_hdr.sensor_lon = 0; 
  _handle.master_hdr.sensor_lat = 0;   
  _handle.master_hdr.sensor_alt = 0; 
      
  sprintf(_handle.master_hdr.data_set_info,"%s",file_path);
  sprintf(_handle.master_hdr.data_set_name,"%s",field);
  sprintf(_handle.master_hdr.data_set_source,"%s",source);

  /* Now set up the field header. */

  MDV_alloc_handle_arrays(&_handle, 1, 1, 0); 
    
  /* Point at the data array. */

  _handle.field_plane[0][0] = (ui08 *) data;

  _handle.field_planes_allocated = TRUE;



  /* Reference the field header. */

  fhdr =_handle.fld_hdrs;
  MDV_init_field_header(fhdr);


  /* First fill out integers. */
  fhdr->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;
  fhdr->field_code = 0;

  fhdr->user_time1 =     Now;
  fhdr->forecast_delta = 0; /* This is not a forecast. */
  fhdr->user_time2 =     _handle.master_hdr.time_begin;
  fhdr->user_time3 =     _handle.master_hdr.time_end;
  fhdr->forecast_time =  _handle.master_hdr.time_centroid; /* Not a forecast. */
  fhdr->user_time4 =     _handle.master_hdr.time_expire; 
  fhdr->nx =  _handle.master_hdr.max_nx;
  fhdr->ny =  _handle.master_hdr.max_ny;
  fhdr->nz =  _handle.master_hdr.max_nz;


  fhdr->proj_type =   _handle.master_hdr.user_data;
  fhdr->encoding_type = MDV_INT8;

  fhdr->volume_size =  _handle.master_hdr.max_nx*_handle.master_hdr.max_ny*_handle.master_hdr.max_nz;
  /* The above assumes byte data. */

  /* Then reals */
  fhdr->proj_origin_lat =  Glob->params.output_grid.origin_lat;
  fhdr->proj_origin_lon =  Glob->params.output_grid.origin_lon;
  fhdr->proj_param[0] = 0.0;
  fhdr->proj_param[1] = 0.0;
  fhdr->proj_param[2] = 0.0;
  fhdr->proj_param[3] = 0.0;
  fhdr->proj_param[4] = 0.0;
  fhdr->proj_param[5] = 0.0;
  fhdr->proj_param[6] = 0.0;
  fhdr->proj_param[7] = 0.0;
  fhdr->vert_reference = 0.0;

  fhdr->grid_dx =  Glob->params.output_grid.dx;
  fhdr->grid_dy =  Glob->params.output_grid.dx;
  fhdr->grid_dz =  1;

  fhdr->grid_minx =  Glob->params.output_grid.minx;
  fhdr->grid_miny =  Glob->params.output_grid.miny;
  fhdr->grid_minz =  0;

  fhdr->bad_data_value = 0;
  fhdr->missing_data_value = 0;
  fhdr->proj_rotation = 0.0;
  fhdr->scale=1.0; fhdr->bias=0.0;

  STRncopy((char *)fhdr->field_name_long,
	   Glob->params.field_name,
	   MDV_LONG_FIELD_LEN);

  STRncopy((char *)fhdr->field_name,
	  Glob->params.field_name,
	  MDV_SHORT_FIELD_LEN);

  STRncopy((char *)fhdr->units,
	  Glob->params.field_units,
	  MDV_UNITS_LEN);

  STRncopy((char *)fhdr->transform, "none",
	  MDV_TRANSFORM_LEN);

  /* Fill in the vlevel header for the field. */

  vhdr = _handle.vlv_hdrs;
  MDV_init_vlevel_header(vhdr);
  vhdr->vlevel_type[0] = MDV_VERT_TYPE_SURFACE;
  vhdr->vlevel_params[0] = fhdr->grid_minz;

  i = MDV_write_to_dir(&_handle, Glob->params.output_dir, 
		       MDV_PLANE_RLE8, TRUE);

  if (i!= MDV_SUCCESS){
    fprintf(stderr,"Failed to write data to directory %s\n",
	    Glob->params.output_dir);    
  }

  MDV_handle_free_field_planes(&_handle); 
  MDV_free_handle(&_handle); 

}
  





