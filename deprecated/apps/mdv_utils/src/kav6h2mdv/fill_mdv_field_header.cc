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
// FILL_MDV_FIELD_HEADER: copy applicapble contents of Kavouras 6H
// header into an mdv field header
// RETURN 0 if sucessful, RETURN -1 if failure occurred
// Template by --Rachel Ames 1/96
// For Kavouras 6H format F. Hage 3/96
//
 
#include "kav6h2mdv.h"
using namespace std;

int fill_mdv_field_header(dcmp6h_header_t *k_head, 
                          MDV_master_header_t *mmh,
                          MDV_field_header_t *mfh)

{
   int error_flag =0;

// allocate space for header info

   if (mfh == NULL) return -1;

// fill in fortran record size -- (record_len1=record_len2)
   mfh->record_len1 = sizeof(MDV_field_header_t)-2*sizeof(mfh->record_len2);

   mfh->struct_id = MDV_FIELD_HEAD_MAGIC_COOKIE;

// Kavouras files have no forecasts
   mfh->forecast_delta = 0;
   mfh->forecast_time  = mmh->time_centroid;
 
   mfh->nx = (gd.x2 - gd.x1) +  1;
   mfh->ny = (gd.y2 - gd.y1) +  1;
   mfh->nz = 1;

// type of projection
   mfh->proj_type = MDV_PROJ_FLAT;

// data encoding
// data always starts out as MDV_INT8, data may be encoded later in write routines.
   mfh->encoding_type = MDV_INT8;

   mfh->data_element_nbytes = MDV_data_element_size(mfh->encoding_type);

// Field data offset, assume no chunk data, all fields volumes same size
   mfh->field_data_offset = sizeof(MDV_master_header_t) + (mmh->n_fields * sizeof(MDV_field_header_t)) + sizeof(mfh->record_len1);
                            

// projection origin information. 
   mfh->proj_origin_lat = mmh->sensor_lat;
   mfh->proj_origin_lon = mmh->sensor_lon;

   mfh->proj_rotation = 0.0;  // Oriented Relative to true north

   // grid information in meters or degrees
   mfh->grid_dx   = KAV_DX;
   mfh->grid_dy   = KAV_DY;
   mfh->grid_dz   = 0.0;
   mfh->grid_minx = KAV_X_START + gd.x1 * mfh->grid_dx;
   mfh->grid_miny = KAV_Y_START + gd.y1 * mfh->grid_dy;
   mfh->grid_minz = k_head->param3 / 10.0;

   mfh->bad_data_value     = 0;
   mfh->missing_data_value = 0;

    switch(k_head->product_code) {
    case 16:   // Base Reflectivity
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
       mfh->field_code = 256;
       strncpy((char *)mfh->field_name_long, "Radar Reflectivity",MDV_LONG_FIELD_LEN);
       strncpy((char *)mfh->field_name, "DBZ",MDV_SHORT_FIELD_LEN);
       strncpy((char *)mfh->units, "dBZ",MDV_UNITS_LEN);
       strncpy((char *)mfh->transform, "NIDS BASED",MDV_TRANSFORM_LEN);
    break;

    case 22:  // Base Velocity
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 55:
    case 56:
       mfh->field_code = 259;
       strncpy((char *)mfh->field_name_long, "Radar Radial Velocity",MDV_LONG_FIELD_LEN);
       strncpy((char *)mfh->field_name, "Velocity",MDV_SHORT_FIELD_LEN);
       strncpy((char *)mfh->units, "m/sec",MDV_UNITS_LEN);
       strncpy((char *)mfh->transform, "NIDS BASED",MDV_TRANSFORM_LEN);
    break;

    case 28: // Spectral width
    case 29:
    case 30:
       mfh->field_code = 260;
       strncpy((char *)mfh->field_name_long, "Radar Spectral Width",MDV_LONG_FIELD_LEN);
       strncpy((char *)mfh->field_name, "Width",MDV_SHORT_FIELD_LEN);
       strncpy((char *)mfh->units, "Hz",MDV_UNITS_LEN);
       strncpy((char *)mfh->transform, "NIDS BASED",MDV_TRANSFORM_LEN);
    break;

    default:  // For now -
       mfh->field_code = 0;
       strncpy((char *)mfh->field_name_long, "NIDS PRODUCT",MDV_LONG_FIELD_LEN);
       strncpy((char *)mfh->field_name, "UNKNOWN NIDS",MDV_SHORT_FIELD_LEN);
       strncpy((char *)mfh->units, "UNKNOWN",MDV_UNITS_LEN);
       strncpy((char *)mfh->transform, "NIDS BASED",MDV_TRANSFORM_LEN);
    break;
    }


// fill in fortran record size -- (record_len1=record_len2)
   mfh->record_len2 = mfh->record_len1;

// normal exit
    return(error_flag);
 
}  

