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
// Mdvx_vsect.hh
//
// VSECT functions for Mdvx class
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh
#ifdef _in_Mdvx_hh

// assemble/disassemble vertical section chunks

//////////////////////////////////////////////////
// assemble vsection waypoint buffer
// handle byte-swapping into BE order
// 32-bit and 64-bit version

static void
  assembleVsectWayPtsBuf32(const vector<Mdvx::vsect_waypt_t> &wayPts,
                           MemBuf &buf);

static void
  assembleVsectWayPtsBuf64(const vector<Mdvx::vsect_waypt_t> &wayPts,
                           MemBuf &buf);

//////////////////////////////////////////////////
// assemble vsection sample pt buffer
// handle byte-swapping into BE order
// 32-bit and 64-bit version

static void
  assembleVsectSamplePtsBuf32(const vector<Mdvx::vsect_samplept_t> &samplePts,
                              double dx_km,
                              MemBuf &buf);

static void
  assembleVsectSamplePtsBuf64(const vector<Mdvx::vsect_samplept_t> &samplePts,
                              double dx_km,
                              MemBuf &buf);

//////////////////////////////////////////////////
// assemble vsection segments buffer
// handle byte-swapping into BE order
// 32-bit and 64-bit version

static void
  assembleVsectSegmentsBuf32(const vector<Mdvx::vsect_segment_t> &segments,
                             double totalLength,
                             MemBuf &buf);

static void
  assembleVsectSegmentsBuf64(const vector<Mdvx::vsect_segment_t> &segments,
                             double totalLength,
                             MemBuf &buf);

//////////////////////////////////////////////////
// disassemble vsection waypoints buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string
// 32-bit and 64-bit version

static int
  disassembleVsectWayPtsBuf32(const MemBuf &buf,
                              vector<Mdvx::vsect_waypt_t> &wayPts,
                              string &errStr);

static int
  disassembleVsectWayPtsBuf64(const MemBuf &buf,
                              vector<Mdvx::vsect_waypt_t> &wayPts,
                              string &errStr);

//////////////////////////////////////////////////
// disassemble vsection samplepts buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string
// 32-bit and 64-bit version

static int
  disassembleVsectSamplePtsBuf32(const MemBuf &buf,
                                 vector<Mdvx::vsect_samplept_t> &samplePts,
                                 double &dxKm,
                                 string &errStr);

static int
  disassembleVsectSamplePtsBuf64(const MemBuf &buf,
                                 vector<Mdvx::vsect_samplept_t> &samplePts,
                                 double &dxKm,
                                 string &errStr);

//////////////////////////////////////////////////
// disassemble vsection segments buffer
// handle byte-swapping from BE order
// returns 0 on success, -1 on failure
// on failure, sets error string
// 32-bit and 64-bit version

static int
  disassembleVsectSegmentsBuf32(const MemBuf &buf,
                                vector<Mdvx::vsect_segment_t> &segments,
                                double &totalLength,
                                string &errStr);
  
static int
  disassembleVsectSegmentsBuf64(const MemBuf &buf,
                                vector<Mdvx::vsect_segment_t> &segments,
                                double &totalLength,
                                string &errStr);
  
#endif

    
