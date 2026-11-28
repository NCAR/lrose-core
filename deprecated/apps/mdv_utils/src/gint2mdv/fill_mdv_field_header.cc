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
 * FILL_MDV_FIELD_HEADER: copy applicapble contents of gint
 * header into an mdv field header
 * Returns FILL_SUCCESS or RETURN FILL_FAILURE. 
 * --Rachel Ames 1/96, RAP/NCAR
 */
 
#ifdef __cplusplus
extern "C" {
#endif

#include "gint2mdv.h"
using namespace std;

int fill_mdv_field_header(Tvolume_header *gh, 
                          MDV_master_header_t *mmh,
                          MDV_field_header_t *mfh, 
                          int ifield)
{
   int vlevel = 0;		/* loop variable */
   float zmin=0.;		/* temporary variable */

/* allocate space for header info if needed*/
   if (mfh == NULL) {
      if ((mfh = (MDV_field_header_t *)
                  ucalloc(1,sizeof(MDV_field_header_t))) == NULL) {
         fprintf(stderr,"\nError occurred during calloc of mdv field header\n");
         return(FILL_FAILURE);
      }
   }

/* fill in fortran record size -- (record_len1=record_len2) */
   mfh->record_len1 = sizeof(MDV_field_header_t)-2*sizeof(mfh->record_len2);

/* fill in magic cookie value */
   mfh->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;

/* FIELD_CODE */
   /* fill this in later mfh->field_code= */

/* for gint files, there is no forecast */
/*   mfh->forecast_delta    = 0; */
/*   mfh->forecast_time     = mmh->time_centroid; */
 
/* In gint files, each field has same dimensions.  Copy what's in master header */
   mfh->nx = mmh->max_nx;
   mfh->ny = mmh->max_ny;
   mfh->nz = mmh->max_nz;

/* Projection type.  Can be changed with tdrp configure */ 
   if (gd->params.projection_type == -2) { /* user has not specified */
      if (gh->vh->proj_type == -1)
         mfh->proj_type = MDV_PROJ_FLAT; 
      else
         mfh->proj_type = MDV_PROJ_LATLON;
   } /* endif read info from header */
   else  /* user has specified a projection */
      mfh->proj_type = gd->params.projection_type;

/* encoding type.  MDV_INT8 is unsigned ints. All gint data
 * read in as MDV_INT8.  Output may either be MDV_PLANE_RLE8 or MDV_INT8.
 * MDV_PLANE_RLE8 is MDV plane by plane compression format */
   mfh->encoding_type = MDV_INT8;

/* data element size in bytes -- MDV_INT8 is the only one implemented right now*/
   mfh->data_element_nbytes = MDV_data_element_size(MDV_INT8);

/* volume size -- size of all the field data */
   if (mfh->encoding_type == MDV_INT8) {
      mfh->volume_size = (mfh->nx)*(mfh->ny)*(mfh->nz)*mfh->data_element_nbytes;
   }
   else {
      fprintf(stdout,"\nDon't know what volume size is. Fill in later?");
   }

/* Field data offset, assume no chunk data, all field volumes same size.
 * If data written out encoded then this will need to be updated later. */
   mfh->field_data_offset = sizeof(MDV_master_header_t) + 
                            mmh->n_fields * 
                            (sizeof(MDV_field_header_t) + 
                            mmh->vlevel_included * sizeof(MDV_vlevel_header_t)) +
                            ifield*mfh->volume_size +
                            (2*ifield+1)*sizeof(mfh->record_len2);

/* projection origin information.  
 * Sometimes wrong in gint so configurable in parameter file
 */
   if (gd->params.sensor_latitude == 0.0)
      mfh->proj_origin_lat = gh->vh->origin_lat/1000000.;
   else
      mfh->proj_origin_lat = gd->params.sensor_latitude;

   if (gd->params.sensor_longitude == 0.0)
      mfh->proj_origin_lon = gh->vh->origin_lon/1000000.;     
   else
      mfh->proj_origin_lon = gd->params.sensor_longitude;

/* projection parameters - not needed for gint files */
/* mfh->proj_param[MDV_MAX_VLEVELS] = ; */

/* Vertical reference point -- put in origin altitude */
   mfh->vert_reference = gd->params.sensor_altitude;                                          
/* grid information in meters or degrees, meters converted to km.
 * mfh->grid_dz = unknown and not filled in.
 * gint data is offset so subtract 1/2 of a grid point 
 */
   if (mfh->proj_type == MDV_PROJ_FLAT)  {
      mfh->grid_dx   = gh->vh->xss*METERS2KM;
      mfh->grid_dy   = gh->vh->yss*METERS2KM;
      mfh->grid_minx = gh->vh->xstt*METERS2KM - mfh->grid_dx/2.;
      mfh->grid_miny = gh->vh->ystt*METERS2KM - mfh->grid_dy/2.;
   }
   else if (mfh->proj_type == MDV_PROJ_LATLON)  {
      mfh->grid_dx = (fl32)gh->vh->xss/1048576.0;
      mfh->grid_dy = (fl32)gh->vh->yss/1048576.0;
      mfh->grid_minx = (fl32)gh->vh->xstt/1048576.0 - mfh->grid_dx/2.;
      mfh->grid_miny = (fl32)gh->vh->ystt/1048576.0 - mfh->grid_dy/2.;
   }
   mfh->grid_dz = 0;
   
/* find min vertical level */
   zmin = gh->ai[0].z;
   for (vlevel = 1; vlevel < gh->vh->nz; vlevel++) {
      if (gh->ai[vlevel].z < zmin) zmin = gh->ai[vlevel].z;
   }
 
/* for gint data, z always in meters  and is relative to sensor altitude */
   mfh->grid_minz = zmin*METERS2KM + gd->params.sensor_altitude;

/* scale and bias */
   mfh->scale = gh->fi[ifield].scale/65536.;
   mfh->bias  = gh->fi[ifield].offset/65536.;

/* in gint files, bad and missing are same value */
   mfh->bad_data_value     = (float)gh->fi[0].bad_data_value;
   mfh->missing_data_value = mfh->bad_data_value;

/* field name information */
   strncpy((char *)mfh->field_name_long,gh->fi[ifield].field_names,
           strlen(gh->fi[ifield].field_names));
   strncpy((char *)mfh->field_name, gh->fi[ifield].field_names,
           strlen(gh->fi[ifield].field_names));
   strncpy((char *)mfh->units, gh->fi[ifield].unit_names,
           strlen(gh->fi[ifield].unit_names));

/* fill in fortran record size -- (record_len1=record_len2) */
   mfh->record_len2 = mfh->record_len1;

/* normal exit */
    return(FILL_SUCCESS);

}  
 
#ifdef __cplusplus
}
#endif

