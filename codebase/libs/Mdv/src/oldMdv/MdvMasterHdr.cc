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

#include <Mdv/mdv/MdvFile.hh>
#include <Mdv/mdv/MdvMasterHdr.hh>
#include <toolsa/str.h>
using namespace std;

MdvMasterHdr::MdvMasterHdr()
{
   MDV_init_master_header( &info );

   info.num_data_times       = 0;

   info.index_number         = -1;
   info.data_collection_type = MDV_DATA_MIXED;

   info.sensor_lat           = 0;
   info.sensor_lon           = 0;
   info.sensor_alt           = 0;

   info.grid_order_direction = MDV_ORIENT_SN_WE;
   info.grid_order_indices   = MDV_ORDER_XYZ;

   info.native_vlevel_type   = 0;
   info.vlevel_type          = 0;
   info.vlevel_included      = 0;
   info.n_chunks             = 0;

   info.data_set_info[0]     = '\0';
   info.data_set_name[0]     = '\0';
   info.data_set_source[0]   = '\0';

   clearTime();
   clearGeometry();
   calcOffsets();
}

MdvMasterHdr::MdvMasterHdr( const MdvMasterHdr &source )
{
   copy( source );
}

MdvMasterHdr& 
MdvMasterHdr::operator= ( const MdvMasterHdr &source )
{
   copy( source );
   return *this;
}

void 
MdvMasterHdr::copy( const MdvMasterHdr &source )
{
   info = source.info;

/*
   //
   // NOTE: user_ and unused_ fields are not copied!
   //
   info.time_gen             = source.info.time_gen;
   info.time_expire          = source.info.time_expire;
   info.time_begin           = source.info.time_begin;
   info.time_centroid        = source.info.time_centroid;
   info.time_end             = source.info.time_end;
   info.num_data_times       = source.info.num_data_times;

   info.index_number         = source.info.index_number;
   info.data_collection_type = source.info.data_collection_type;

   info.sensor_lat           = source.info.sensor_lat;
   info.sensor_lon           = source.info.sensor_lon;
   info.sensor_alt           = source.info.sensor_alt;

   info.grid_order_direction = source.info.grid_order_direction;
   info.grid_order_indices   = source.info.grid_order_indices;

   info.native_vlevel_type   = source.info.native_vlevel_type;
   info.vlevel_type          = source.info.vlevel_type;
   info.vlevel_included      = source.info.vlevel_included;
   info.n_chunks             = source.info.n_chunks;

   info.n_fields             = source.info.n_fields;
   info.max_nx               = source.info.max_nx;
   info.max_ny               = source.info.max_ny;
   info.max_nz               = source.info.max_nz;
   info.data_dimension       = source.info.data_dimension;
   info.field_grids_differ   = source.info.field_grids_differ;

   info.field_hdr_offset     = source.info.field_hdr_offset;
   info.vlevel_hdr_offset    = source.info.vlevel_hdr_offset;
   info.chunk_hdr_offset     = source.info.chunk_hdr_offset;

   strcpy( info.data_set_info, source.info.data_set_info );
   strcpy( info.data_set_name, source.info.data_set_name );
   strcpy( info.data_set_source, source.info.data_set_source );
*/
}

void 
MdvMasterHdr::setTime( time_t timeStamp )
{
   if ( info.time_gen == MdvFile::NEVER )
      info.time_gen = (si32)time( NULL );

   if ( info.time_begin == MdvFile::NEVER )
      info.time_begin = timeStamp;

   if ( info.time_end == MdvFile::NEVER )
      info.time_end = timeStamp;

   if ( info.time_centroid == MdvFile::NEVER )
      info.time_centroid  = timeStamp;

   if ( info.time_expire == MdvFile::NEVER )
      info.time_expire  = timeStamp;
}

void
MdvMasterHdr::clearTime()
{
   info.time_gen      = MdvFile::NEVER;
   info.time_begin    = MdvFile::NEVER;
   info.time_end      = MdvFile::NEVER;
   info.time_centroid = MdvFile::NEVER;
   info.time_expire   = MdvFile::NEVER;
}

void 
MdvMasterHdr::setSensor( double lat, double lon, double alt )
{
   info.sensor_lat = (fl32)lat;
   info.sensor_lon = (fl32)lon;
   info.sensor_alt = (fl32)alt;
}

void 
MdvMasterHdr::setDescription( const char *name, const char *source,
                              const char *desc )
{
   if ( name )
      STRncopy( info.data_set_name,   name,   MDV_NAME_LEN );

   if ( source )
      STRncopy( info.data_set_source, source, MDV_NAME_LEN );

   if ( desc )
      STRncopy( info.data_set_info,   desc,   MDV_INFO_LEN );
}

void
MdvMasterHdr::setGeometry( size_t nxMax, size_t nyMax, size_t nzMax,
                           size_t dimension, bool differ )
{
   info.max_nx             = (si32)nxMax;
   info.max_ny             = (si32)nyMax;
   info.max_nz             = (si32)nzMax;
   info.data_dimension     = (si32)dimension;
   info.field_grids_differ = differ ? 1 : 0;
}

void
MdvMasterHdr::clearGeometry()
{
   info.n_fields           = 0;
   info.max_nx             = 0;
   info.max_ny             = 0;
   info.max_nz             = 0;
   info.data_dimension     = 0;
   info.field_grids_differ = 0;
}

void
MdvMasterHdr::getGeometry( size_t *nxMax, size_t *nyMax, size_t *nzMax,
                           size_t *dimension, bool *differ )
{
   if ( nxMax )
      *nxMax = (size_t)info.max_nx;
   if ( nyMax )
      *nyMax = (size_t)info.max_ny;
   if ( nzMax )
      *nzMax = (size_t)info.max_nz;
   if ( dimension )
      *dimension = (size_t)info.data_dimension;
   if ( differ )
      *differ = info.field_grids_differ ? true : false;
}

void MdvMasterHdr::calcOffsets()
{
   MDV_set_master_hdr_offsets( &info );
}
