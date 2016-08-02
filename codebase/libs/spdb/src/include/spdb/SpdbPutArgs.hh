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
// spdb/SpdbPutArgs.hh
//
// Args needed for process thread in SpdbPut functions
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
/////////////////////////////////////////////////////////////

#ifndef SpdbPutArgs_HH
#define SpdbPutArgs_HH

#include <string>
#include <spdb/spdb.h>
#include <spdb/SpdbMsg.hh>
#include <toolsa/MemBuf.hh>
using namespace std;

class SpdbPutArgs {
  
public:
  
  // constructor
  
  SpdbPutArgs (const string &url_str,
	       const int prod_id,
	       const string &prod_label,
	       const SpdbMsg::mode_enum_t mode,
	       const int n_chunks,
	       const spdb_chunk_ref_t *chunk_refs,
	       const void *chunk_data);

  // copy constructor
  
  SpdbPutArgs (const SpdbPutArgs &other);
  
  // Destructor

  ~SpdbPutArgs();

  // public data
  
  string urlStr;
  int prodId;
  string prodLabel;
  SpdbMsg::mode_enum_t putMode;
  int nChunks;
  spdb_chunk_ref_t *chunkRefs;
  void *chunkData;
  int chunkDataLen;
  
protected:
  
private:

  MemBuf _refBuf;
  MemBuf _dataBuf;

};

#endif








