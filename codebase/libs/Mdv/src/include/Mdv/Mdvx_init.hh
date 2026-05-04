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
// Mdvx_init.hh
//
// Initialize structs
//
////////////////////////////////////////////////

// This header file can only be included from within Mdvx.hh

#ifdef _in_Mdvx_hh

void initStruct(master_header_t &val);
void initStruct(field_header_t &val);
void initStruct(vlevel_header_t &val);
void initStruct(chunk_header_t &val);

void initStruct(vsect_waypt_t &val);
void initStruct(vsect_samplept_t &val);
void initStruct(vsect_segment_t &val);

void initStruct(chunkVsectWayPtHdr_t &val);
void initStruct(chunkVsectWayPt_t &val);
void initStruct(chunkVsectSamplePtHdr_t &val);
void initStruct(chunkVsectSamplePt_t &val);
void initStruct(chunkVsectSegmentHdr_t &val);
void initStruct(chunkVsectSegment_t &val);

void initStruct(flat_params_t &val);
void initStruct(albers_params_t &val);
void initStruct(lc2_params_t &val);
void initStruct(os_params_t &val);
void initStruct(ps_params_t &val);
void initStruct(trans_merc_params_t &val);
void initStruct(vert_persp_params_t &val);

void initStruct(coord_t &val);

#endif

    
