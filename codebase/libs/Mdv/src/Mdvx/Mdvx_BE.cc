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
//////////////////////////////////////////////////////////
// Mdvx_BE.cc
//
// Byte swapping routines for Mdvx class
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 1999
//
//////////////////////////////////////////////////////////

#include <Mdv/Mdvx.hh>
#include <dataport/bigend.h>
#include <rapformats/ds_radar.h>
#include <rapformats/var_elev.h>
using namespace std;

/////////////////////////////////////////////////////////////////////
// master_header_from_BE
// Converts master header from big endian format to native format.
//

void Mdvx::master_header_from_BE(master_header_t &m_hdr) 

{

  BE_to_array_32(&m_hdr.record_len1, 4 * sizeof(si32));
  BE_to_array_64(&m_hdr.time_gen, 6 * sizeof(si64));
  BE_to_array_32(&m_hdr.num_data_times, 12 * sizeof(si32));
  BE_to_array_64(&m_hdr.max_nx, 6 * sizeof(si64));
  BE_to_array_32(&m_hdr.field_grids_differ, 10 * sizeof(si32));
  BE_to_array_64(&m_hdr.time_written, 4 * sizeof(si64));
  BE_to_array_32(m_hdr.unused_si32, 8 * sizeof(si32));
  BE_to_array_64(&m_hdr.sensor_lon, 3 * sizeof(si64));
  BE_to_array_32(m_hdr.unused_fl32, 12 * sizeof(fl32));
  BE_to_array_32(&m_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// master_header_to_BE
// Converts master header to big endian format from native format.
//

void Mdvx::master_header_to_BE(master_header_t &m_hdr) 

{

  // set magic cookie and fortran record lengths

  m_hdr.struct_id = MASTER_HEAD_MAGIC_COOKIE_64;
  m_hdr.revision_number = REVISION_NUMBER;
  m_hdr.record_len1 = sizeof(m_hdr) - (2 * sizeof(si32));
  m_hdr.record_len2 = sizeof(m_hdr) - (2 * sizeof(si32));

  // swap
  
  BE_from_array_32(&m_hdr.record_len1, 4 * sizeof(si32));
  BE_from_array_64(&m_hdr.time_gen, 6 * sizeof(si64));
  BE_from_array_32(&m_hdr.num_data_times, 12 * sizeof(si32));
  BE_from_array_64(&m_hdr.max_nx, 6 * sizeof(si64));
  BE_from_array_32(&m_hdr.field_grids_differ, 10 * sizeof(si32));
  BE_from_array_64(&m_hdr.time_written, 4 * sizeof(si64));
  BE_from_array_32(m_hdr.unused_si32, 8 * sizeof(si32));
  BE_from_array_64(&m_hdr.sensor_lon, 3 * sizeof(si64));
  BE_from_array_32(m_hdr.unused_fl32, 12 * sizeof(fl32));
  BE_from_array_32(&m_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// field_header_from_BE
// Converts field header from big endian format to native format.
//

void Mdvx::field_header_from_BE(field_header_t &f_hdr)

{

  BE_to_array_32(&f_hdr.record_len1, 4 * sizeof(si32));
  BE_to_array_64(&f_hdr.user_time1, 9 * sizeof(si64));
  
  BE_to_array_32(&f_hdr.proj_type, 2 * sizeof(si32));
  BE_to_array_64(&f_hdr.data_element_nbytes, 3 * sizeof(si64));
  
  BE_to_array_32(f_hdr.user_data_si32, 24 * sizeof(si32));

  BE_to_array_64(&f_hdr.proj_origin_lat, 2 * sizeof(si64));
  BE_to_array_64(f_hdr.proj_param, sizeof(f_hdr.proj_param));
  BE_to_array_64(&f_hdr.vert_reference, 7 * sizeof(si64));
  
  BE_to_array_32(&f_hdr.scale, 24 * sizeof(si32));

  BE_to_array_32(&f_hdr.record_len2, 1 * sizeof(si32));
  
  // make consistent with new encoding and compression types

  if (f_hdr.encoding_type == PLANE_RLE8) {
    f_hdr.encoding_type = ENCODING_INT8;
    f_hdr.compression_type = COMPRESSION_RLE;
  }

  if (f_hdr.compression_type < COMPRESSION_NONE ||
      f_hdr.compression_type >= COMPRESSION_TYPES_N) {
    f_hdr.compression_type = COMPRESSION_NONE;
  }
 
}

/////////////////////////////////////////////////////////////////////
// field_header_to_BE
// Converts field header to big endian format from native format.
//

void Mdvx::field_header_to_BE(field_header_t &f_hdr)

{

  // set magic cookie and fortran record lengths

  f_hdr.struct_id = FIELD_HEAD_MAGIC_COOKIE_64;
  f_hdr.record_len1 = sizeof(f_hdr) - (2 * sizeof(si32));
  f_hdr.record_len2 = sizeof(f_hdr) - (2 * sizeof(si32));

 // make consistent with new encoding and compression types

  if (f_hdr.encoding_type == PLANE_RLE8) {
    f_hdr.encoding_type = ENCODING_INT8;
    f_hdr.compression_type = COMPRESSION_RLE;
  }
 
  BE_from_array_32(&f_hdr.record_len1, 4 * sizeof(si32));
  BE_from_array_64(&f_hdr.user_time1, 9 * sizeof(si64));
  
  BE_from_array_32(&f_hdr.proj_type, 2 * sizeof(si32));
  BE_from_array_64(&f_hdr.data_element_nbytes, 3 * sizeof(si64));
  
  BE_from_array_32(f_hdr.user_data_si32, 24 * sizeof(si32));

  BE_from_array_64(&f_hdr.proj_origin_lat, 2 * sizeof(si64));

  BE_from_array_64(f_hdr.proj_param, sizeof(f_hdr.proj_param));
  BE_from_array_64(&f_hdr.vert_reference, 7 * sizeof(si64));
  
  BE_from_array_32(&f_hdr.scale, 24 * sizeof(si32));
  BE_from_array_32(&f_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// vlevel_header_from_BE
// Converts vlevel header from big endian format to native format.
//

void Mdvx::vlevel_header_from_BE(vlevel_header_t &v_hdr)

{

  // swap entire struct, it is all 32-bit
  
  BE_to_array_32(&v_hdr, sizeof(vlevel_header_t));
 
}

/////////////////////////////////////////////////////////////////////
// vlevel_header_to_BE
// Converts vlevel header to big endian format from native format.
//

void Mdvx::vlevel_header_to_BE(vlevel_header_t &v_hdr)

{

  // set magic cookie and fortran record lengths

  v_hdr.struct_id = VLEVEL_HEAD_MAGIC_COOKIE_64;
  v_hdr.record_len1 = sizeof(v_hdr) - (2 * sizeof(si32));
  v_hdr.record_len2 = sizeof(v_hdr) - (2 * sizeof(si32));

  // swap entire struct, it is all 32-bit
  
  BE_from_array_32(&v_hdr, sizeof(vlevel_header_t));

}

/////////////////////////////////////////////////////////////////////
// chunk_header_from_BE
// Converts chunk header from big endian format to native format.
//

void Mdvx::chunk_header_from_BE(chunk_header_t &c_hdr)

{

  BE_to_array_32(&c_hdr.record_len1, 4 * sizeof(si32));
  BE_to_array_64(&c_hdr.chunk_data_offset, 2 * sizeof(si64));
  BE_to_array_32(c_hdr.unused_si32, sizeof(c_hdr.unused_si32));
  BE_to_array_32(&c_hdr.record_len2, 1 * sizeof(si32));
 
}

/////////////////////////////////////////////////////////////////////
// chunk_header_to_BE
// Converts chunk header to big endian format from native format.
//

void Mdvx::chunk_header_to_BE(chunk_header_t &c_hdr)

{

  // set magic cookie and fortran record lengths

  c_hdr.struct_id = CHUNK_HEAD_MAGIC_COOKIE_64;
  c_hdr.record_len1 = sizeof(c_hdr) - (2 * sizeof(si32));
  c_hdr.record_len2 = sizeof(c_hdr) - (2 * sizeof(si32));

  // swap
  
  BE_from_array_32(&c_hdr.record_len1, 4 * sizeof(si32));
  BE_from_array_64(&c_hdr.chunk_data_offset, 2 * sizeof(si64));
  BE_from_array_32(c_hdr.unused_si32, sizeof(c_hdr.unused_si32));
  BE_from_array_32(&c_hdr.record_len2, 1 * sizeof(si32));
 
}

/////////////////////////////////////////////////////////////////////
// master_header_from_BE_32
// Converts master header from big endian format to native format.
//

void Mdvx::master_header_from_BE_32(master_header_32_t &m_hdr) 

{

  BE_to_array_32(&m_hdr.record_len1, 63 * sizeof(si32));
  BE_to_array_32(&m_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// master_header_to_BE_32
// Converts master header to big endian format from native format.
//

void Mdvx::master_header_to_BE_32(master_header_32_t &m_hdr) 

{

  // set magic cookie and fortran record lengths

  m_hdr.struct_id = MASTER_HEAD_MAGIC_COOKIE_32;
  m_hdr.revision_number = REVISION_NUMBER;
  m_hdr.record_len1 = sizeof(m_hdr) - (2 * sizeof(si32));
  m_hdr.record_len2 = sizeof(m_hdr) - (2 * sizeof(si32));

  BE_from_array_32(&m_hdr.record_len1, 63 * sizeof(si32));
  BE_from_array_32(&m_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// field_header_from_BE_32
// Converts field header from big endian format to native format.
//

void Mdvx::field_header_from_BE_32(field_header_32_t &f_hdr)

{

  BE_to_array_32(&f_hdr.record_len1, 71 * sizeof(si32));
  BE_to_array_32(&f_hdr.record_len2, 1 * sizeof(si32));
  
  // make consistent with encoding and compression types

  if (f_hdr.encoding_type == PLANE_RLE8) {
    f_hdr.encoding_type = ENCODING_INT8;
    f_hdr.compression_type = COMPRESSION_RLE;
  }

  if (f_hdr.compression_type < COMPRESSION_NONE ||
      f_hdr.compression_type >= COMPRESSION_TYPES_N) {
    f_hdr.compression_type = COMPRESSION_NONE;
  }
 
}

/////////////////////////////////////////////////////////////////////
// field_header_to_BE_32
// Converts field header to big endian format from native format.
//

void Mdvx::field_header_to_BE_32(field_header_32_t &f_hdr)

{

  // set magic cookie and fortran record lengths

  f_hdr.struct_id = FIELD_HEAD_MAGIC_COOKIE_32;
  f_hdr.record_len1 = sizeof(f_hdr) - (2 * sizeof(si32));
  f_hdr.record_len2 = sizeof(f_hdr) - (2 * sizeof(si32));

 // make consistent with new encoding and compression types

  if (f_hdr.encoding_type == PLANE_RLE8) {
    f_hdr.encoding_type = ENCODING_INT8;
    f_hdr.compression_type = COMPRESSION_RLE;
  }
 
  BE_from_array_32(&f_hdr.record_len1, 71 * sizeof(si32));
  BE_from_array_32(&f_hdr.record_len2, 1 * sizeof(si32));

}

/////////////////////////////////////////////////////////////////////
// vlevel_header_from_BE_32
// Converts vlevel header from big endian format to native format.
//

void Mdvx::vlevel_header_from_BE_32(vlevel_header_32_t &v_hdr)

{

  // swap entire struct, it is all 32-bit vals

  BE_to_array_32(&v_hdr, sizeof(v_hdr));
  
}

/////////////////////////////////////////////////////////////////////
// vlevel_header_to_BE_32
// Converts vlevel header to big endian format from native format.
//

void Mdvx::vlevel_header_to_BE_32(vlevel_header_32_t &v_hdr)

{

  // set magic cookie and fortran record lengths

  v_hdr.struct_id = VLEVEL_HEAD_MAGIC_COOKIE_32;
  v_hdr.record_len1 = sizeof(v_hdr) - (2 * sizeof(si32));
  v_hdr.record_len2 = sizeof(v_hdr) - (2 * sizeof(si32));

  // swap entire struct, it is all 32-bit vals

  BE_from_array_32(&v_hdr, sizeof(v_hdr));
 
}

/////////////////////////////////////////////////////////////////////
// chunk_header_from_BE_32
// Converts chunk header from big endian format to native format.
//

void Mdvx::chunk_header_from_BE_32(chunk_header_32_t &c_hdr)

{

  BE_to_array_32(&c_hdr.record_len1, 7 * sizeof(si32));
  BE_to_array_32(&c_hdr.record_len2, 1 * sizeof(si32));
 
}

/////////////////////////////////////////////////////////////////////
// chunk_header_to_BE_32
// Converts chunk header to big endian format from native format.
//

void Mdvx::chunk_header_to_BE_32(chunk_header_32_t &c_hdr)

{

  // set magic cookie and fortran record lengths

  c_hdr.struct_id = CHUNK_HEAD_MAGIC_COOKIE_32;
  c_hdr.record_len1 = sizeof(c_hdr) - (2 * sizeof(si32));
  c_hdr.record_len2 = sizeof(c_hdr) - (2 * sizeof(si32));

  BE_from_array_32(&c_hdr.record_len1, 7 * sizeof(si32));
  BE_from_array_32(&c_hdr.record_len2, 1 * sizeof(si32));

}

