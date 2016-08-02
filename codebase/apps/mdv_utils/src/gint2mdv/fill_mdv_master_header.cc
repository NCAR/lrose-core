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
/*****************************************************************
 * FILL_MDV_MASTER_HEADER: copy applicapble contents of gint
 * header into an mdv master header.
 * Return FILL_SUCCESS or FILL_FAILURE 
 * Note: all enumerated types are defined in mdv_macros.h/mdv_file.h.
 * Make sure these are current with this code!
 * --Rachel Ames 1/96, RAP/NCAR
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
using namespace std;

int fill_mdv_master_header(Tvolume_header *gh, MDV_master_header_t *mmh)
{

/* fill in fortran record size -- assume length of record_len1=length of record_len2) */
   mmh->record_len1 = sizeof(MDV_master_header_t) - 2*sizeof(mmh->record_len1);

/* fill in magic cookie */
   mmh->struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE; 

/* fill in revision code */
   mmh->revision_number = MDV_REVISION_NUMBER;

/* time fields */
   mmh->time_gen      = gh->vh->time;
   mmh->time_begin    = gh->vh->l_time;
   mmh->time_end      =  gh->vh->time;
   mmh->time_centroid = (int)((gh->vh->time + gh->vh->l_time)/2.);
   mmh->time_expire   = 0;

/* number data times associated (ie number of forecasts produced), 1 for gint */
   mmh->num_data_times = 1;

/* which data time this file represents, 1 for gint */
   mmh->index_number = 1;

/* dimensionallity of data, 3 for gint */
   mmh->data_dimension = 3;

/* Collection method, gint files are typically measured 
 * User may change with tdrp configure                  
 */
   mmh->data_collection_type = gd->params.collection_type;

/* Original vertical level type.  Can be changed with tdrp configure */	
   mmh->native_vlevel_type = gd->params.vertical_type;

/* This data's vertical level type -- set to original vertical level type */
   mmh->vlevel_type = mmh->native_vlevel_type; 		

/* Vlevel included -- yes for gint  */
   mmh->vlevel_included = TRUE;

/* Grid orientation */
   mmh->grid_order_direction = MDV_ORIENT_SN_WE;

/* Order of indicies. 
 * For basic mdv datasets, write out XYZ (X moves fastest, then Y, then Z) */
   mmh->grid_order_indices =  MDV_ORDER_XYZ;

/* number fields */
   mmh->n_fields = gh->vh->n_fields;

/* grid dimensions, always the same in gint files */
   mmh->max_nx = gh->vh->nx;
   mmh->max_ny = gh->vh->ny;
   mmh->max_nz = gh->vh->nz;

/* number of chunks in file, 0 for gint */
   mmh->n_chunks = 0;

/* offsets, in bytes from beginning of dataset 
 * Must be in order Master Header, Field Headers, Vlevel Headers, Chunk Headers
 * (chunk headers optional) */
   mmh->field_hdr_offset  = sizeof(MDV_master_header_t);

   mmh->vlevel_hdr_offset = mmh->field_hdr_offset + 
                           (mmh->n_fields) * sizeof(MDV_field_header_t);

   if (mmh->n_chunks != 0)
      mmh->chunk_hdr_offset  = mmh->vlevel_hdr_offset + 
                              (mmh->n_fields) * sizeof(MDV_vlevel_header_t);
   else
      mmh->chunk_hdr_offset  = 0;

/* make sure number of gint vertical levels less than max allowed in mdv */
   if (gh->vh->nz > MDV_MAX_VLEVELS) {
      fprintf(stderr,"\nToo many vertical levels in gint file.");
      fprintf(stderr,"\nNlevels = %d",(int)gh->vh->nz);
      fprintf(stderr,"\nMAX ALLOWED in MDV file: %d.",(int)MDV_MAX_VLEVELS);
      return(FILL_FAILURE);
   }

/* sensor origin information.
 * Sometimes wrong in gint so configurable in parameter file.
 * Sensor altitude info not included in gint.
 */
   if (gd->params.sensor_latitude == 0.0)
      mmh->sensor_lat = gh->vh->origin_lat/1000000.;
   else  /* user has specified one */
      mmh->sensor_lat = gd->params.sensor_latitude;
 
   if (gd->params.sensor_longitude == 0.0)
      mmh->sensor_lon = gh->vh->origin_lon/1000000.;
   else  /* user has specified one */
      mmh->sensor_lon = gd->params.sensor_longitude;

   mmh->sensor_alt = gd->params.sensor_altitude;

/* data set info */
   memcpy(mmh->data_set_info,gh->vh->note,sizeof(gh->vh->note));
   memcpy(mmh->data_set_name,gh->vh->file_name,sizeof(gh->vh->file_name));
   memcpy(mmh->data_set_source,gd->params.data_source,
            sizeof(char)*strlen(gd->params.data_source));

/* fill in fortran record size -- record_len1=record_len2) */
   mmh->record_len2 = mmh->record_len1;

/* normal exit */
    return(FILL_SUCCESS);
 
}  

#ifdef __cplusplus
}
#endif

