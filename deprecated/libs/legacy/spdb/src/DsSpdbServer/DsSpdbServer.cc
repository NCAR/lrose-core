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
// DsSpdbServer.cc
//
// File Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1999
//
///////////////////////////////////////////////////////////////

#include <spdb/DsSpdbServer.hh>
#include <toolsa/Socket.hh>
#include <didss/RapDataDir.hh>
#include <dsserver/DsLocator.hh>
#include <cassert>
using namespace std;

//////////////////////////////////////////
// constructor
//
// Inherits from DsServer

DsSpdbServer::DsSpdbServer(const string& prog_name,
			   const string& instance,
                           void *params,
                           bool allowParamOverride,
			   int port,
			   int qmax,
			   int max_clients,
			   bool no_threads,
			   bool is_debug,
			   bool is_verbose)
  : DsServer(prog_name,
	     instance,
	     port,
	     qmax,
	     max_clients,
	     false,
	     is_debug,
	     is_verbose),
    _progName(prog_name)

{
  allowOverride = allowParamOverride;
  initialParams = params;
  assert( initialParams != NULL );
  setNoThreadDebug(no_threads);
}

// virtual 
int DsSpdbServer::handleDataCommand(Socket * socket,
				    const void * data, int dataSize)
{

  if (_isDebug) {
    cerr << "Entering DsSpdbServer::handleDataCommand()." << endl;
  }

  SpdbMsg msg;
  if (_isVerbose) {
    cerr << "Client thread disassembling message..." << endl;
  }
  
  if (msg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - DsSpdbServer::handleDataCommand" << endl;
    cerr << "Invalid DsSpdbMsg message" << endl;
    return(-1);
  }
  
  if (_isVerbose) {
    cerr << "------------------------------------" << endl;
    msg.print(cerr);
  }

  // verify the url and determine the spdb directory path

  string url_str( msg.getUrlStr());

  DsURL url(url_str);
  if (!url.isValid()) {
    cerr << "ERROR - DsSpdbServer::handleDataCommand" << endl;
    cerr << "   Invalid URL: '" << url_str <<  "'\n";
    return (-1);
  }

  string dirPath;
  RapDataDir.fillPath( url, dirPath );

  //
  // Check for local copy of params
  // Override initial params if params file exists in the datatype directory
  //
  void    *serverParams   = initialParams;
  bool     paramsAreLocal = false;

  if ( allowOverride ) {
    //
    // NOTE: the DsLocator will resolve the parameter file name 
    //       and modify the url
    //
    bool  paramsExist;

    if ( DsLocator.resolveParam( url, _executableName,
                                 &paramsExist ) != 0 ) {
      cerr << "ERROR - DsSpdbServer::handleDataCommand\n"
           << "Cannot resolve parameter specification in url:\n"
           << url.getURLStr()
           << endl;
      return( -1 );
    }

    //
    // The application-specfic code must load the override parameters
    //
    if ( paramsExist ) {

      if ( allocLocalParams( url.getParamFile(), 
                             &serverParams ) != 0 ) {
        cerr << "ERROR - DsSpdbServer::handleDataCommand\n"
             << "Cannot load parameter file:\n"
             << url.getParamFile()
             << endl;
        return( -1 );
      }
      else {
        paramsAreLocal = true;
      }
    }
  }

  // handle message

  int iret = 0;
  if (msg.getSubType() == SpdbMsg::DS_SPDB_PUT) {
    if (_handlePut(serverParams, msg, dirPath, *socket)) {
      iret = -1;
    }
  } else if (msg.getSubType() == SpdbMsg::DS_SPDB_GET) {
    if (_handleGet(serverParams, msg, dirPath, *socket)) {
      iret = -1;
    }
  } else {
    cerr << "ERROR - DsSpdbServer::handleDataCommand\n"
         << "Unexpected message" << endl;
    iret = -1;
  }
  
  //
  // Free override params if necessary
  //
  if ( paramsAreLocal ) {
    freeLocalParams( serverParams );
  }

  if (_isDebug) {
    cerr << "Exiting DsSpdbServer::handleDataCommand()." << endl;
  }
  
  return (iret);

}
    
// virtual
bool DsSpdbServer::timeoutMethod()
{
  if (_isVerbose) {
    cout << "In DsSpdbServer::timeoutMethod()." << endl;
  }
  DsServer::timeoutMethod();
  return true; // Continue to wait for clients.
}

// virtual
bool DsSpdbServer::postHandlerMethod()
{
  if (_isVerbose) {
    cout << "In DsSpdbServer::postHandlerMethod()." << endl;
  }
  DsServer::postHandlerMethod();
  return true; // Continue to wait for clients.
}

// virtual
void DsSpdbServer::transformData(void *serverParams,
                                 si32 n_chunks_in,
				 spdb_chunk_ref_t *chunk_refs_in,
				 void *chunk_data_in,
				 si32 *n_chunks_out,
				 spdb_chunk_ref_t **chunk_refs_out,
				 char **chunk_data_out)
{
  // Copy the data to the output pointers.

  *n_chunks_out = n_chunks_in;
  
  *chunk_refs_out = new spdb_chunk_ref_t[n_chunks_in];
  memcpy(*chunk_refs_out, chunk_refs_in,
	 n_chunks_in * sizeof(spdb_chunk_ref_t));
  
  int buffer_size = 0;
  
  for (int i = 0; i < n_chunks_in; i++)
    buffer_size += chunk_refs_in[i].len;
  *chunk_data_out = new char[buffer_size];
  memcpy(*chunk_data_out, chunk_data_in, buffer_size);
}

////////////////
// _handlePut()

int DsSpdbServer::_handlePut(void *serverParams, SpdbMsg &msg, 
                             string &dirPath, Socket &socket)
  
{

  string errStr = "ERROR - DsSpdbServer::_handlePut()\n";

  if (_doPut(serverParams,
             dirPath,
	     msg.getMode(),
	     msg.getInfo().prod_id,
	     msg.getInfo().prod_label,
	     msg.getNChunks(),
	     msg.getChunkRefs(),
	     msg.getChunkData(),
	     errStr)) {

    errStr += "  URL: ";
    errStr += msg.getUrlStr();
    errStr += "\n";
    msg.assemblePutReturn(true, errStr.c_str());

  } else {

    msg.assemblePutReturn();

  }

  // send reply

  void *replyMsg = msg.assembledMsg();
  int replyBuflen = msg.lengthAssembled();
  
  if (socket.writeMessage(SpdbMsg::DS_MESSAGE_TYPE_SPDB,
			  replyMsg, replyBuflen, 1000)) {
    cerr << "ERROR - DsSpdbServer::_handlePut" << endl;
    cerr << "  Writing reply" << endl;
    return (-1);
  }

  return (0);

}

/////////////////////////////////////////////
// function for actually doing the put to disk

int DsSpdbServer::_doPut(void *serverParams,
                         const string &dirPath,
			 int put_mode,
			 const int prod_id,
			 const string &prod_label,
			 const int n_chunks,
			 const spdb_chunk_ref_t *chunk_refs,
			 const void *chunk_data,
			 string &errStr)
  
{

  spdb_handle_t handle;
  if (SPDB_init(&handle, (char *) prod_label.c_str(),
		prod_id, (char *) dirPath.c_str())) {
    errStr += "  Cannot init SPDB_handle.\n";
    return (-1);
  }
  
  spdb_chunk_ref_t *ref = (spdb_chunk_ref_t *) chunk_refs;
  for (int i = 0; i < n_chunks; i++, ref++) {
    if (put_mode == SpdbMsg::DS_SPDB_PUT_MODE_OVER) {
      if (SPDB_store_over(&handle, ref->data_type,
			  ref->valid_time, ref->expire_time,
			  ((char *) chunk_data + ref->offset),
			  ref->len)) {
	SPDB_free(&handle);
 	errStr += "  Put mode OVER failed.\n";
	return (-1);
      }
    } else if (put_mode == SpdbMsg::DS_SPDB_PUT_MODE_ADD) {
      if (SPDB_store_add(&handle, ref->data_type,
			 ref->valid_time, ref->expire_time,
			 ((char *) chunk_data + ref->offset),
			 ref->len)) {
	SPDB_free(&handle);
 	errStr += "  Put mode ADD failed.\n";
	return (-1);
      }
    } else {
      if (SPDB_store(&handle, ref->data_type,
		     ref->valid_time, ref->expire_time,
		     ((char *) chunk_data + ref->offset),
		     ref->len)) {
	SPDB_free(&handle);
 	errStr += "  Put mode ONCE failed.\n";
	return (-1);
      }
    }
  } // i
  
  SPDB_free(&handle);

  return (0);

}

////////////////
// _handleGet()

int DsSpdbServer::_handleGet(void *serverParams, SpdbMsg &msg, 
                             string &dirPath, Socket &socket)
  
{

  string errStr = "ERROR - DsSpdbServer::_handleGet()\n";
  
  spdb_handle_t handle;
  if (SPDB_init(&handle, NULL,
		msg.getInfo().prod_id, (char *) dirPath.c_str())) {
    errStr += "  Cannot init SPDB_handle.\n";
    return (-1);
  }
  
  si32 n_chunks_in;
  spdb_chunk_ref_t *chunk_refs_in;
  void *chunk_data_in;

  si32 n_chunks_out;
  spdb_chunk_ref_t *chunk_refs_out;
  char *chunk_data_out;
  
  SpdbMsg::info_t get_info;
  
  if (_doGet(serverParams,
             handle,
	     msg.getMode(),
	     msg.getInfo(),
	     &n_chunks_in,
	     &chunk_refs_in,
	     &chunk_data_in,
	     &get_info,
	     errStr)) {
    errStr += "  URL: ";
    errStr += msg.getUrlStr();
    errStr += "\n";
    msg.assembleGetErrorReturn(errStr.c_str());
  }
  else
  {
    if (msg.getMode() == SpdbMsg::DS_SPDB_GET_MODE_TIMES)
    {
      msg.assembleGetTimesSuccessReturn(get_info);
    }
    else
    {
      transformData(serverParams, n_chunks_in, chunk_refs_in, chunk_data_in,
		    &n_chunks_out, &chunk_refs_out, &chunk_data_out);
      get_info.n_chunks = n_chunks_out;
      msg.assembleGetDataSuccessReturn(get_info,
				       chunk_refs_out, chunk_data_out);
    }
    
  }
    
  // send reply

  void *replyMsg = msg.assembledMsg();
  int replyBuflen = msg.lengthAssembled();
  
  if (socket.writeMessage(SpdbMsg::DS_MESSAGE_TYPE_SPDB,
			  replyMsg, replyBuflen, 1000)) {
    cerr << "ERROR - DsSpdbServer::_handleGet" << endl;
    cerr << "  Writing reply" << endl;
    return (-1);
  }

  // Free the data

  SPDB_free(&handle);
  delete []chunk_refs_out;
  delete []chunk_data_out;
  
  return (0);

}

///////////////////////////////////////////////
// function for actually doing the get from disk

int DsSpdbServer::_doGet(void *serverParams,
                         spdb_handle_t &handle,
			 int get_mode,
			 const SpdbMsg::info_t &info,
			 si32 *n_chunks,
			 spdb_chunk_ref_t **chunk_refs,
			 void **chunk_data,
			 SpdbMsg::info_t *get_info,
			 string &errStr)
  
{

  *get_info = info;
  
  switch (get_mode) {

  case SpdbMsg::DS_SPDB_GET_MODE_EXACT:
    if (SPDB_fetch(&handle, info.data_type, info.request_time,
		   n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get EXACT mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_CLOSEST:
    if (SPDB_fetch_closest(&handle, info.data_type,
			   info.request_time, info.time_margin,
			   n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get CLOSEST mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_INTERVAL:
    if (SPDB_fetch_interval(&handle, info.data_type,
			    info.start_time, info.end_time,
			    n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get INTERVAL mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_VALID:
    if (SPDB_fetch_valid(&handle, info.data_type,
			 info.request_time,
			 n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get VALID mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_LATEST:
    {
      si32 lastValid;
      if (SPDB_last_valid_time(&handle, &lastValid)) {
	errStr += "  get LATEST mode failed\n";
	errStr += "  Error in SPDB_last_valid_time\n";
	SPDB_free(&handle);
	return (-1);
      }
      if (SPDB_fetch_interval(&handle, info.data_type,
			      lastValid - info.time_margin,
			      lastValid,
			      n_chunks, chunk_refs, chunk_data)) {
	errStr += "  get LATEST mode failed\n";
	errStr += "  Error in SPDB_fetch_closest\n";
	SPDB_free(&handle);
	return (-1);
      }
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_FIRST_BEFORE:
    if (SPDB_fetch_first_before(&handle, info.data_type,
				info.request_time, info.time_margin,
				n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get FIRST_BEFORE mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_FIRST_AFTER:
    if (SPDB_fetch_first_after(&handle, info.data_type,
			       info.request_time, info.time_margin,
			       n_chunks, chunk_refs, chunk_data)) {
      errStr += "  get FIRST_AFTER mode failed\n";
      SPDB_free(&handle);
      return (-1);
    }
    get_info->n_chunks = *n_chunks;
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  case SpdbMsg::DS_SPDB_GET_MODE_TIMES:
    {
      si32 first, last;
      si32 lastValid;
      if (SPDB_first_and_last_times(&handle, &first, &last)) {
	errStr += "  get TIMES mode failed\n";
	SPDB_free(&handle);
	return (-1);
      }
      if (SPDB_last_valid_time(&handle, &lastValid)) {
	errStr += "  get TIMES mode failed\n";
	SPDB_free(&handle);
	return (-1);
      }
      get_info->start_time = first;
      get_info->end_time = last;
      get_info->last_valid_time = lastValid;
    }
    get_info->prod_id = handle.prod_id;
    memcpy(get_info->prod_label, handle.prod_label, SPDB_LABEL_MAX);
    break;

  default:
    break;

  } // switch

  return (0);

}
