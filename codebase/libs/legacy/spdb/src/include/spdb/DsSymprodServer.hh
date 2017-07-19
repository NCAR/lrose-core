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
// DsSymprodServer.hh
//
// FileServerobject
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////


#ifndef _DsSymprodServer_HH
#define _DsSymprodServer_HH

#include <string>

#include <spdb/DsSpdbServer.hh>
using namespace std;

class DsSymprodServer : public DsSpdbServer
{

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  DsSymprodServer(const string &prog_name,
		  const string &instance,
                  void *params,
                  bool allowParamOverride,
		  int port,
		  int qmax,
		  int max_clients,
		  bool no_threads = false,
		  bool is_debug = false,
		  bool is_verbose = false);

  // destructor

  virtual ~DsSymprodServer(){};
  
protected:

  // Transform the data retrieved from the database into symprod
  // format for sending back to the client.  This must be overridden
  // by derrived classes.

  virtual void transformData(void *params,
                             si32 n_chunks_in,
			     spdb_chunk_ref_t *chunk_refs_in,
			     void *chunk_data_in,
			     si32 *n_chunks_out,
			     spdb_chunk_ref_t **chunk_refs_out,
			     void **chunk_data_out);
  
  // Convert the given data chunk from the SPDB database to symprod format.
  // Return NULL if the chunk can't or shouldn't be transformed.

  virtual void *convertToSymprod(void *params,
                                 spdb_chunk_ref_t &spdb_hdr,
				 void *spdb_data,
				 int spdb_len,
				 int *symprod_len) = 0;
  
private:

};

#endif
