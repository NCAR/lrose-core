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
/////////////////////////////////////////////////////////////
// SpdbPutArgs.cc
//
// Args needed for process thread in SpdbPut functions
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
/////////////////////////////////////////////////////////////

#include <spdb/SpdbPutArgs.hh>
#include <rapmath/math_macros.h>
using namespace std;

///////////////
// Constructor

SpdbPutArgs::SpdbPutArgs (const string &url_str,
			  const int prod_id,
			  const string &prod_label,
			  const SpdbMsg::mode_enum_t mode,
			  const int n_chunks,
			  const spdb_chunk_ref_t *chunk_refs,
			  const void *chunk_data)
  
{

  urlStr = url_str;
  prodId = prod_id;
  prodLabel = prod_label;
  putMode = mode;
  nChunks = n_chunks;
  
  // load up chunk refs

  int refNbytes = nChunks * sizeof(spdb_chunk_ref_t);
  chunkRefs = (spdb_chunk_ref_t *) _refBuf.prepare(refNbytes);
  memcpy(chunkRefs, chunk_refs, refNbytes);

  // compute data length

  chunkDataLen = 0;
  for (int i = 0; i < n_chunks; i++) {
    int endPos = chunk_refs[i].offset + chunk_refs[i].len;
    chunkDataLen = MAX(chunkDataLen, endPos);
  }

  chunkData = _dataBuf.prepare(chunkDataLen);
  memcpy(chunkData, chunk_data, chunkDataLen);
  
}

//////////////  
// Destructor

SpdbPutArgs::~SpdbPutArgs()
{
}
