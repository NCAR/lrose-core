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

#include <toolsa/MemBuf.hh>
#include <dsserver/DsProcessServer.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/Product_defines.hh>
#include <string>
using namespace std;

class Socket;

class DsSpdbServer : public DsProcessServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  //   
  DsSpdbServer(const string& prog_name,
	       const string& instance,
               const void *initialParams,
               bool allowParamOverride,
	       int port,
	       int qmax,
	       int max_clients,
	       bool no_threads = false,
	       bool is_debug = false,
	       bool is_verbose = false,
	       bool is_secure = false,
               bool is_read_only = false);

  // destructor

  virtual ~DsSpdbServer(){};
  
protected:

  //
  // Set in the constructor -- do not modify in threads
  //
  const void *initialParams;
  const bool  allowOverride;
  
  // horizontal and vertical limits

  bool _horizLimitsSet;
  double _minLat, _minLon, _maxLat, _maxLon;

  bool _vertLimitsSet;
  double _minHt, _maxHt;

  // auxiliary XML commands

  string _auxXml;
  
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

  virtual int loadLocalParams(const string &paramFile, 
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
  // to the client.
  
  virtual void transformData(const void *serverParams,
			     const string &dir_path,
			     int prod_id,
			     const string &prod_label,
                             int n_chunks_in,
			     const Spdb::chunk_ref_t *chunk_refs_in,
			     const Spdb::aux_ref_t *aux_refs_in,
			     const void *chunk_data_in,
			     int &n_chunks_out,
			     MemBuf &refBufOut,
			     MemBuf &auxBufOut,
			     MemBuf &dataBufOut);
  
  // Make a put/get request -- subclasses can modify the request
  // before it gets sent off to spdb library

  virtual int _handlePut(void *serverParams, DsSpdbMsg &msg, 
                         string &dirPath, Socket &socket);
  virtual int _handleGet(void *serverParams, DsSpdbMsg &msg, 
                         string &dirPath, Socket &socket);

  // Needs to be available to Server subclass

  int _doGet(void *serverParams,
             Spdb &spdb,
	     const string &dir_path,
	     int get_mode,
	     const DsSpdbMsg::info_t &info,
             const DsSpdbMsg::info2_t &info2,
	     DsSpdbMsg::info_t *get_info,
	     string &errStr);

  int _doPut(void *serverParams, 
             const string &dirPath,
	     int put_mode,
	     Spdb::lead_time_storage_t lead_time_storage,
	     int prod_id,
	     const string &prod_label,
	     int n_chunks,
	     const Spdb::chunk_ref_t *chunk_refs,
	     const Spdb::aux_ref_t *aux_refs,
	     const void *chunk_data,
             bool respect_zero_types,
	     string &errStr);
 
  const string _progName;

  ///////////////////////////////////
  // set the limits from the message
  
  void setLimitsFromMsg(DsSpdbMsg &msg);

  ///////////////////////////////////
  // Get the read message
  // NOTE : Only set for 'get' requests.
  
  const DsSpdbMsg &getReadMsg() const { return (_readMsg); }

private:

  // copy of the read message
  // only set for get messages, not put messages

  DsSpdbMsg _readMsg;

  // Private methods with no bodies. DO NOT USE!
  // 
  DsSpdbServer();
  DsSpdbServer(const DsSpdbServer & orig);
  DsSpdbServer & operator = (const DsSpdbServer & other);


};

#endif
