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
///////////////////////////////////////////////////////////////
// DsSymprodServer.cc
//
// Server class for serving out data in the symprod format.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <spdb/DsSymprodServer.hh>
#include <spdb/spdb_products.h>
#include <toolsa/Socket.hh>
using namespace std;

//////////////////////////////////////////
// constructor
//
// Inherits from DsSpdbServer

DsSymprodServer::DsSymprodServer(const string &prog_name,
				 const string &instance,
                                 void *params,
                                 bool allowParamOverride,
				 int port,
				 int qmax,
				 int max_clients,
				 bool no_threads,
				 bool is_debug,
				 bool is_verbose)
  : DsSpdbServer(prog_name,
		 instance,
                 params,
                 allowParamOverride,
		 port,
		 qmax,
		 max_clients,
		 no_threads,
		 is_debug,
		 is_verbose)
{
}

/*********************************************************************
 * transformData() - Transform the data from the database into
 *                   symprod format.
 */

void DsSymprodServer::transformData(void *params,
                                    si32 n_chunks_in,
				    spdb_chunk_ref_t *chunk_refs_in,
				    void *chunk_data_in,
				    si32 *n_chunks_out,
				    spdb_chunk_ref_t **chunk_refs_out,
				    void **chunk_data_out)
{
  // Create the memory buffers.

  MEMbuf *trans_header_buffer = MEMbufCreate();
  MEMbuf *trans_data_buffer = MEMbufCreate();
  
  // Transform each chunk and add it to the memory buffers

  int symprod_buffer_len = 0;
  int num_chunks = 0;
  
  for (int i = 0; i < n_chunks_in; i++)
  {
    spdb_chunk_ref_t chunk_hdr = chunk_refs_in[i];
    void *chunk_data = (void *)((char *)chunk_data_in + chunk_hdr.offset);
    int symprod_len;
    
    void *trans_data = convertToSymprod(params,
                                        chunk_hdr,
					chunk_data,
					chunk_hdr.len,
					&symprod_len);
    
    if (trans_data != (void *)NULL)
    {
      chunk_hdr.prod_id = SPDB_SYMPROD_ID;
      chunk_hdr.offset = symprod_buffer_len;
      chunk_hdr.len = symprod_len;
      
      MEMbufAdd(trans_header_buffer, &chunk_hdr, sizeof(chunk_hdr));
      MEMbufAdd(trans_data_buffer, trans_data, symprod_len);
      
      num_chunks++;
      
      symprod_buffer_len += symprod_len;
      
    }
    
  }
  
  // Set the pointers to the data to send out.

  *n_chunks_out = num_chunks;

  *chunk_refs_out = new spdb_chunk_ref_t[num_chunks];
  memcpy(*chunk_refs_out, MEMbufPtr(trans_header_buffer),
	 num_chunks * sizeof(spdb_chunk_ref_t));
  
  *chunk_data_out = (void *)(new char[symprod_buffer_len]);
  memcpy(*chunk_data_out, MEMbufPtr(trans_data_buffer), symprod_buffer_len);
  
  // Delete the memory buffers

  MEMbufDelete(trans_header_buffer);
  MEMbufDelete(trans_data_buffer);
  
}
