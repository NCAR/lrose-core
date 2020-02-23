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
// Mdvx_constants.hh
//
// Constants for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

// label and revision number

static const string FILE_LABEL;
static const int REVISION_NUMBER = 2;

// max size for signed and unsigned 32-bit ints

#define SI32_MAX 2147483647L
#define UI32_MAX 4294967295U

// magic cookies

static const int MASTER_HEAD_MAGIC_COOKIE_32 = 14142;
static const int FIELD_HEAD_MAGIC_COOKIE_32 = 14143;
static const int VLEVEL_HEAD_MAGIC_COOKIE_32 = 14144;
static const int CHUNK_HEAD_MAGIC_COOKIE_32 = 14145;

static const int MASTER_HEAD_MAGIC_COOKIE_64 = 14152;
static const int FIELD_HEAD_MAGIC_COOKIE_64 = 14153;
static const int VLEVEL_HEAD_MAGIC_COOKIE_64 = 14154;
static const int CHUNK_HEAD_MAGIC_COOKIE_64 = 14155;

#endif

