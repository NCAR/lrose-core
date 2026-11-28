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
//////////////////////////////////////////////////////////////////
// FILL_MDV_FIELD_HEADER: copy applicapble contents of Kavouras 6L
// header into an mdv field header
// RETURN 0 if sucessful, RETURN -1 if failure occurred
// Template by --Rachel Ames 1/96
// For Kavouras 6L format F. Hage 3/96
//
 
#ifdef __cplusplus
extern "C" {
#endif

#include "kav6l2mdv.h"
using namespace std;

int fill_mdv_field_header(dcmp6h_header_t *k_head, 
                          MDV_master_header_t *mmh,
                          MDV_field_header_t *mfh, 
                          int ifield)
{
   int error_flag =0;

// allocate space for header info

   if (mfh == NULL) return -1;

// fill in fortran record size -- (record_len1=record_len2)
   mfh->record_len1 = sizeof(MDV_field_header_t)-2*sizeof(mfh->record_len2);

   mfh->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;

// for Kavouras files, there is no forecast
   mfh->forecast_delta = 0;
   mfh->forecast_time  = mmh->time_centroid;
 
   mfh->nx = (gd.x2 - gd.x1) +  1;
   mfh->ny = (gd.y2 - gd.y1) +  1;
   mfh->nz = 1;

// type of projection
   mfh->proj_type = MDV_PROJ_FLAT;

// data encoding
// user enters 0 = None, 1=URL 8Bit encoding called MDV_PLANE_RLE8
   if (gd.compress) {
      mfh->encoding_type = MDV_PLANE_RLE8;
   } else  {
      mfh->encoding_type = MDV_INT8;
   }

   mfh->data_element_nbytes = MDV_data_element_size(mfh->encoding_type);

// Field data offset, assume no chunk data, all fields volumes same size
   mfh->field_data_offset = sizeof(MDV_master_header_t) + (mmh->n_fields * sizeof(MDV_field_header_t)) + 4;
                            

// projection origin information. 
   if (gd.origin_lat == 0.0)
      mfh->proj_origin_lat = ((k_head->radar_lat_hi * 65536) + k_head->radar_lat_lo) / 1000.0;
   else
      mfh->proj_origin_lat = gd.origin_lat;

   if (gd.origin_lon == 0.0)
      mfh->proj_origin_lon =  ((k_head->radar_lon_hi * 65536) + k_head->radar_lon_lo) / 1000.0;
   else
      mfh->proj_origin_lon = gd.origin_lon;

   mfh->proj_param[0] = 0.0;  // Oriented Relative to true north

   // grid information in meters or degrees
   mfh->grid_dx = KAV_DX;
   mfh->grid_dy = KAV_DY;
   mfh->grid_dz = 0.0;
   mfh->grid_minx = KAV_X_START;
   mfh->grid_miny = KAV_Y_START;
   mfh->grid_minz = 0.0;

   mfh->bad_data_value     = 0;
   mfh->missing_data_value = 0;

    switch(k_head->product_code) {

    case 35:  // Composite Reflectivity
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
       mfh->field_code = 257;
    break;

    case 41:   // Echo Tops
    case 42:
       mfh->field_code = 257;
    break;

    case 63:  // Layer composite reflectivity
    case 64:
    case 65:
    case 66:
    case 89:
    case 90:
       mfh->field_code = 258;
    break;

    case 22:  // Base Velocity
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
       mfh->field_code = 259;
    break;

    default:  // For now -
       mfh->field_code = 0;
    break;
    }

    strncpy((char *)mfh->field_name_long, MDV_get_field_name(mfh->field_code),MDV_LONG_FIELD_LEN);
    strncpy((char *)mfh->field_name, MDV_get_field_abbrev(mfh->field_code),MDV_SHORT_FIELD_LEN);
    strncpy((char *)mfh->units, MDV_get_field_units(mfh->field_code),MDV_UNITS_LEN);
    strncpy((char *)mfh->transform, "NIDS BASED",MDV_TRANSFORM_LEN);


// fill in fortran record size -- (record_len1=record_len2)
   mfh->record_len2 = mfh->record_len1;

// normal exit
    return(error_flag);
 
}  
 

#ifdef __cplusplus
}
#endif

