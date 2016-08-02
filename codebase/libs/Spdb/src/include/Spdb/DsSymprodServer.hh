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
#include <dsserver/DsProcessServer.hh>
#include <Spdb/Symprod.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/Product_defines.hh>
using namespace std;

class DsSymprodServer : public DsProcessServer
{
  
public:
  
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  DsSymprodServer(const string &prog_name,
		  const string &instance,
                  const void *params,
		  int port,
		  int qmax,
		  int max_clients,
		  bool no_threads = false,
		  bool is_debug = false,
		  bool is_verbose = false);

  // destructor
  
  virtual ~DsSymprodServer(){};

  // specify uniqueness in the returned data set

  void setUnique(Spdb::get_unique_t state) { _unique = state; }
  
protected:

  // initial params set in parent - do not modify in threads
  // local params are used in threads

  const void *_initialParams;

  // copy of the read message

  DsSpdbMsg _readMsg;
  
  // horizontal and vertical limits

  bool _horizLimitsSet;
  double _minLat, _minLon, _maxLat, _maxLon;

  bool _vertLimitsSet;
  double _minHt, _maxHt;

  // auxiliary XML commands

  string _auxXml;

  // uniqueness on get

  Spdb::get_unique_t _unique;
  
  // Allocate, load, and free the server parameters from the specified file
  // Alloc should return 0 if successful

  virtual int loadLocalParams(const string &paramFile, 
			      void **serverParams) = 0;

  virtual void freeLocalParams( void *serverParams ) = 0;

  // Override base class on timeout and post handlers
  // always return true - i.e. never exit

  virtual bool timeoutMethod();
  virtual bool postHandlerMethod();
  
  // Handle a client's request for data. The passed
  // message is a decoded DsMessage.
  
  virtual int handleDataCommand(Socket *socket,
				const void *data, ssize_t dataSize);
  
  // Transform the data retrieved from the database into symprod
  // format for sending back to the client.  This must be overridden
  // by derrived classes.

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
  
  // Handle a get request 

  virtual int _handleGet(const void *serverParams, const DsSpdbMsg &msg, 
                         const string &dirPath, Socket &socket);

  // sets the prod_id and prod_label in the info appropriately

  virtual void _setProductId(DsSpdbMsg::info_t &info, const void *localParams);

  // Convert the given data chunk from the SPDB database to symprod format.
  // Load up symprod_buf.
  // Return 0  on success,
  //        -1 if the chunk can't or shouldn't be transformed.

  virtual int convertToSymprod(const void *params,
			       const string &dir_path,
			       int prod_id,
			       const string &prod_label,
			       const Spdb::chunk_ref_t &chunk_ref,
			       const Spdb::aux_ref_t &aux_ref,
			       const void *spdb_data,
			       int spdb_len,
			       MemBuf &symprod_buf) = 0;
  
  ///////////////////////////////////
  // set the limits from the message
  
  void setLimitsFromMsg(const DsSpdbMsg &msg);

  ///////////////////////////////////
  // Get the read message
  // NOTE : Only set for 'get' requests.
  
  const DsSpdbMsg &getReadMsg() const { return (_readMsg); }
  
private:

};

#endif
