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
// DsSpdbServer.hh
//
// FileServerobject
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////


#ifndef _DsSpdbServer_HH
#define _DsSpdbServer_HH

#include <dsserver/DsServer.hh>
#include <spdb/spdb.h>
#include <spdb/SpdbMsg.hh>
#include <string>
using namespace std;

class Socket;

class DsSpdbServer : public DsServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  //   
  DsSpdbServer(const string& prog_name,
	       const string& instance,
               void *initialParams,
               bool allowParamOverride,
	       int port,
	       int qmax,
	       int max_clients,
	       bool no_threads = false,
	       bool is_debug = false,
	       bool is_verbose = false);

  // destructor

  virtual ~DsSpdbServer(){};
  
protected:

  //
  // Set in the constructor -- do not modify in threads
  //
  void *initialParams;
  bool  allowOverride;

  // Handle a client's request for data. The passed
  //   message is a decoded DsResourceMsg that is
  //   *not* a server command.
  //
  // Look at the URL and determine if a port is needed.
  // Spawn the necessary server.
  // 
  virtual int handleDataCommand(Socket * socket,
				const void * data, int dataSize);

  // Allocate, load, and free the server parameters from the specified file
  // Alloc should return 0 if successful

  virtual int allocLocalParams(const string &paramFile, 
                               void **serverParams) = 0;

  virtual void freeLocalParams( void *serverParams ) = 0;

    
  // Perform any special operations that need to happen
  //   when the server times out waiting for clients.
  // 
  // Returns: true if the server should continue to wait for clients,
  //          false if the server should return from waitForClients().

  virtual bool timeoutMethod();

  // Perform any special operations that need to happen
  //   when the server is finished handling a client.
  //
  // Returns: true if the server should continue to wait for clients,
  //          false if the server should return from waitForClients().
  //

  virtual bool postHandlerMethod();

  // Transform the data retrieved from the database before sending it back
  // to the client.  This routine should allocate space for chunk_refs_out
  // and chunk_data_out using new as these are deleted in the calling
  // routine using delete.

  virtual void transformData(void *serverParams,
                             si32 n_chunks_in,
			     spdb_chunk_ref_t *chunk_refs_in,
			     void *chunk_data_in,
			     si32 *n_chunks_out,
			     spdb_chunk_ref_t **chunk_refs_out,
			     char **chunk_data_out);
  
  // Make a put/get request -- subclasses can modify the request
  // before it gets sent off to spdb library

  virtual int _handlePut(void *serverParams, SpdbMsg &msg, 
                         string &dirPath, Socket &socket);
  virtual int _handleGet(void *serverParams, SpdbMsg &msg, 
                         string &dirPath, Socket &socket);

  //
  // Needs to be available to Server subclass

  int _doGet(void *serverParams,
             spdb_handle_t &handle,
	     int get_mode,
	     const SpdbMsg::info_t &info,
	     si32 *n_chunks,
	     spdb_chunk_ref_t **chunk_refs,
	     void **chunk_data,
	     SpdbMsg::info_t *get_info,
	     string &errStr);

private:

  // private members

  string _progName;

  // Private methods with no bodies. DO NOT USE!
  // 
  DsSpdbServer();
  DsSpdbServer(const DsSpdbServer & orig);
  DsSpdbServer & operator = (const DsSpdbServer & other);

  int _doPut(void *serverParams, 
             const string &dirPath,
	     int put_mode,
	     const int prod_id,
	     const string &prod_label,
	     const int n_chunks,
	     const spdb_chunk_ref_t *chunk_refs,
	     const void *chunk_data,
	     string &errStr);
 
};

#endif
