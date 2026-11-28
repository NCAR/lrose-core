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
/////////////////////////////////////////////////////////////////////
// FILLIN_MDV_MASTER_HEADER: copy applicapble contents of Kavouras 6H
// header into an mdv master header
// RETURN 0 if sucessful, RETURN -1 if failure occurred
// Note: all enumerated types are defined in mdv_macros.h
// Template by --Rachel Ames 1/96
// Kavouras Format - F. Hage 3/96
///

#include "kav6h2mdv.h"
using namespace std;

int fill_mdv_master_header(dcmp6h_header_t *k_head, MDV_master_header_t *mmh)
{
   int error_flag = 0;

// fill in fortran record size -- assume length of record_len1=length of record_len2)
   mmh->record_len1 = sizeof(MDV_master_header_t) - 2*sizeof(mmh->record_len1);

   mmh->struct_id = MDV_MASTER_HEAD_MAGIC_COOKIE; 

// time fields
   mmh->time_gen      = (k_head->generate_date -1) * 86400 + (k_head->generate_time_hi * 65536) + k_head->generate_time_lo;
   mmh->time_begin    = (k_head->scan_date -1) * 86400 + (k_head->scan_time_hi * 65536) + k_head->scan_time_lo;
   mmh->time_end      = mmh->time_begin;
   mmh->time_centroid = mmh->time_begin;
   mmh->time_expire   = 0;

// number data times associated (ie number of forecasts produced), 1 for kavouras NIDS
   mmh->num_data_times = 1;

// which data time this file represents,
   mmh->index_number = 1;

   mmh->data_dimension = 3;

// Original vertical level type.  Can be changed with inline args/parameter file
   mmh->native_vlevel_type = MDV_VERT_TYPE_ELEV;


// vlevel included -- no for NIDS Images 
   mmh->vlevel_included = FALSE;

// Grid orientation
   mmh->grid_order_direction = MDV_ORIENT_SN_WE;

// Order of indicies. 
// For datasets, write out XYZ (X moves fastest, then Y, then Z)
   mmh->grid_order_indices =  MDV_ORDER_XYZ;

// number fields
   mmh->n_fields = 1;

// projection origin information. 
   if (gd.origin_lat == 0.0)
      mmh->sensor_lat = ((k_head->radar_lat_hi * 65536) + k_head->radar_lat_lo) / 1000.0;
   else
      mmh->sensor_lat = gd.origin_lat;

   if (gd.origin_lon == 0.0)
      mmh->sensor_lon =  ((k_head->radar_lon_hi * 65536) + k_head->radar_lon_lo) / 1000.0;
   else
      mmh->sensor_lon = gd.origin_lon;

   if (gd.origin_alt == 0.0)
      mmh->sensor_alt =  ((k_head->radar_alt) * FT2METERS)/1000.;
   else
      mmh->sensor_alt = gd.origin_alt;

// dimension information
   mmh->max_nx = (gd.x2 - gd.x1) + 1;
   mmh->max_ny = (gd.y2 - gd.y1) + 1;

// dimension information
   mmh->max_nx = (gd.x2 - gd.x1) + 1;
   mmh->max_ny = (gd.y2 - gd.y1) + 1;
   mmh->max_nz = 1;

   mmh->n_chunks = 0;

// offsets
   mmh->field_hdr_offset  = sizeof(MDV_master_header_t);
   mmh->vlevel_hdr_offset = 0;
   mmh->chunk_hdr_offset  = 0;

#ifdef DBG
   fprintf(stdout,"\noffsets, fld %d, vlv %d, chunk %d",mmh->field_hdr_offset,mmh->vlevel_hdr_offset,mmh->chunk_hdr_offset);
#endif

    switch(k_head->product_code) {
    case 16:   // Base Reflectivity
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
       mmh->data_collection_type = MDV_DATA_MEASURED;
       mmh->vlevel_type = MDV_VERT_TYPE_ELEV;
       strcpy((char *)mmh->data_set_info,"This File was converted from a KAVOURAS format file");
       strcpy((char *)mmh->data_set_name,"Kavouras NIDS Base Reflectivity");
       strcpy((char *)mmh->data_set_source,"NIDS");
    break;

    case 22:  // Base Velocity
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
       mmh->data_collection_type = MDV_DATA_MEASURED;
       mmh->vlevel_type = MDV_VERT_TYPE_ELEV;
       strcpy((char *)mmh->data_set_info,"This File was converted from a KAVOURAS format file");
       strcpy((char *)mmh->data_set_name,"Kavouras NIDS Base Velocity");
       strcpy((char *)mmh->data_set_source,"NIDS");
    break;

    case 28:  // Spectral Width
    case 29:
    case 30:
       mmh->data_collection_type = MDV_DATA_MEASURED;
       mmh->vlevel_type = MDV_VERT_TYPE_ELEV;
       strcpy((char *)mmh->data_set_info,"This File was converted from a KAVOURAS format file");
       strcpy((char *)mmh->data_set_name,"Kavouras NIDS Base Velocity");
       strcpy((char *)mmh->data_set_source,"NIDS");
    break;


    default:  // For now -
       mmh->data_collection_type = MDV_DATA_EXTRAPOLATED;
       mmh->vlevel_type = MDV_VERT_TYPE_SURFACE;
       strcpy((char *)mmh->data_set_info,"This File was converted from a KAVOURAS format file");
       strcpy((char *)mmh->data_set_name,"Kavouras Distributed NIDS");
       strcpy((char *)mmh->data_set_source,"NIDS");
    break;
    }

// fill in fortran record size -- record_len1=record_len2)
   mmh->record_len2 = mmh->record_len1;

// normal exit
    return(error_flag);
 
}  


