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
////////////////////////////////////////////////
//
// Mdvx_BE.hh
//
// Byte swapping functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

// byte swapping

static void master_header_from_BE(master_header_t &m_hdr);
static void master_header_to_BE(master_header_t &m_hdr);

static void field_header_from_BE(field_header_t &f_hdr);
static void field_header_to_BE(field_header_t &f_hdr);

static void vlevel_header_from_BE(vlevel_header_t &v_hdr);
static void vlevel_header_to_BE(vlevel_header_t &v_hdr);

static void chunk_header_from_BE(chunk_header_t &c_hdr);
static void chunk_header_to_BE(chunk_header_t &c_hdr);

static void master_header_from_BE_32(master_header_32_t &m_hdr);
static void master_header_to_BE_32(master_header_32_t &m_hdr);

static void field_header_from_BE_32(field_header_32_t &f_hdr);
static void field_header_to_BE_32(field_header_32_t &f_hdr);

static void vlevel_header_from_BE_32(vlevel_header_32_t &v_hdr);
static void vlevel_header_to_BE_32(vlevel_header_32_t &v_hdr);

static void chunk_header_from_BE_32(chunk_header_32_t &c_hdr);
static void chunk_header_to_BE_32(chunk_header_32_t &c_hdr);

#endif

    
