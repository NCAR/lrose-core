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
static const int REVISION_NUMBER;

// magic cookies

static const int MASTER_HEAD_MAGIC_COOKIE;
static const int FIELD_HEAD_MAGIC_COOKIE;
static const int VLEVEL_HEAD_MAGIC_COOKIE;
static const int CHUNK_HEAD_MAGIC_COOKIE;

// Number of 32 bit elements in headers (before the
// first character).  The _32 values are used for
// swapping the header fields and the _SI32 and_FL32
// values are used for for copying the fields in FORTRAN
// read and write routines.
// Note that the _32 values should equal the corresponding
// _SI32 value + the corresponding _FL32 value + 1. The
// extra 1 in this formula is the FORTRAN record length
// field at the beginning of each header.  This field is
// swapped in an array with the rest of the header fields,
// but is not copied into the FORTRAN arrays on reads and
// writes.

static const int NUM_MASTER_HEADER_SI32;
static const int NUM_MASTER_HEADER_FL32;
static const int NUM_MASTER_HEADER_32;

static const int NUM_FIELD_HEADER_SI32;
static const int NUM_FIELD_HEADER_FL32;
static const int NUM_FIELD_HEADER_32;

static const int NUM_VLEVEL_HEADER_SI32;
static const int NUM_VLEVEL_HEADER_FL32;
static const int NUM_VLEVEL_HEADER_32;

static const int NUM_CHUNK_HEADER_SI32;
static const int NUM_CHUNK_HEADER_32;

#endif

