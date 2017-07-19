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
/*********************************************************************
 * SpdbServer.cc: SpdbServer object code.  This object serves out
 *                data from an SPDB database.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1996
 *
 * Nancy Rehak
 *
 *********************************************************************/


#include <cstdio>
#include <cerrno>
#include <cassert>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>

#include <dataport/bigend.h>

#include <symprod/spdb.h>
#include <symprod/spdb_client.h>

#include <toolsa/mem.h>
#include <toolsa/membuf.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/smu.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <SpdbServer/SpdbServer.h>
using namespace std;


#define SPDB_WAIT_MSECS  1000

/*
 * Global variables
 */

static int Forever = TRUE;


/*********************************************************************
 * Constructor
 */

SpdbServer::SpdbServer(const int port,
		       const char *database_product_label,
		       const int database_product_id,
		       const char *database_dir,
		       const char *prog_name,
		       const char *servmap_type,
		       const char *servmap_subtype,
		       const char *servmap_instance,
		       const int max_children,
		       SpdbServerTransform input_transform,
		       const si32 input_product_id,
		       SpdbServerTransform output_transform,
		       const si32 output_product_id,
		       const int wait_msecs,
		       const int realtime_avail_flag,
		       SpdbServerDebugLevel debug_level,
		       const int latest_only_flag)
{
  static char *routine_name = "Constructor";
  
  if (debug_level >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (debug_level >= SPDB_SERVER_DEBUG_MSGS)
    fprintf(stderr,
	    "%s::%s:  Creating SpdbServer on port %d\n",
	    _className(), routine_name, port);
  
  // Store the socket information

  _port = port;
  
  // Store the server mapper information

  _programName = STRdup(prog_name);
  _servmapType = STRdup(servmap_type);
  _servmapSubtype = STRdup(servmap_subtype);
  _servmapInstance = STRdup(servmap_instance);
  _waitMsecs = wait_msecs;
  _realtimeAvailFlag = realtime_avail_flag;
  _latestOnlyFlag = latest_only_flag;

  // Initialize the communications process information

  _numChildren = 0;
  _maxChildren = max_children;
  
  // Store the database information

  _databaseProductLabel = STRdup(database_product_label);
  _databaseProductId = database_product_id;
  _databaseDirName = STRdup(database_dir);
  
  // Store the transform routines

  _inputTransform = input_transform;
  _outputTransform = output_transform;
  
  // Store the product ids

  if (_inputTransform == NULL)
    _inputProductId = database_product_id;
  else
    _inputProductId = input_product_id;

  if (_outputTransform == NULL)
    _outputProductId = database_product_id;
  else
    _outputProductId = output_product_id;
  
  // Initialize the transformed data buffer.  This buffer is used by
  // several routines to manage the transformed data and is kept in
  // a global variable for convenience and to save some memory.

  _transformBuf = MEMbufCreate();
  _transformHdrs = MEMbufCreate();

  // Store the debug level

  _debugLevel = debug_level;
  
  // Initialize the database

  if (SPDB_init(&_spdbHandle,
		_databaseProductLabel,
		_databaseProductId,
		_databaseDirName) != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error initializing SPDB database in directory <%s>.\n",
	      _databaseDirName);
    }
    
    exit(-1);
  }
  
  // Open the socket as a server

  if ((_serverfd = SKU_open_server(_port)) < 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error opening server socket on port %d\n",
	      _port);
    }
    
    exit(-1);
  }
  
  // Initialize the client socket information

  _clientfd = -1;
  
  // Initialize SMU usage

  SMU_auto_init(_servmapType,
		_servmapSubtype,
		_servmapInstance,
		_databaseDirName,
		_port,
		_realtimeAvailFlag,
		NULL);
  
  // Initialize PMU usage

  PMU_auto_init(_programName,
		_servmapInstance,
		PROCMAP_REGISTER_INTERVAL);
  
  if (debug_level >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
}


/*********************************************************************
 * Destructor
 */

SpdbServer::~SpdbServer(void)
{
  static char *routine_name = "Destructor";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  if (_debugLevel >= SPDB_SERVER_DEBUG_MSGS)
    fprintf(stderr,
	    "%s:  Destroying object\n",
	    _className());

  // Free the strings

  STRfree(_databaseProductLabel);
  STRfree(_databaseDirName);
  STRfree(_programName);
  STRfree(_servmapType);
  STRfree(_servmapSubtype);
  STRfree(_servmapInstance);
  
  // Close the database

  SPDB_close(&_spdbHandle);
  SPDB_free(&_spdbHandle);
  
  // Unregister from the server mapper

  SMU_auto_unregister();
  
  // Unregister from the process mapper

  PMU_auto_unregister();
  
  // Close the server socket

  SKU_close(_serverfd);
  
  // Delete the transformed data buffer.
  
  MEMbufDelete(_transformBuf);
  MEMbufDelete(_transformHdrs);

  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
}


/*********************************************************************
 * operate() - main operational loop.  This routine just does all of
 *             the work of the server and never returns to the caller.
 */

void SpdbServer::operate(void)
{
  static char *routine_name = "operate";
  static time_t last_data_time_update = 0;
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Operate forever

  while (Forever)
  {
    // Register with the server mapper and process mapper
    
    SMU_auto_register();
    PMU_auto_register("waiting for request");
    
    // Reap any outstanding children to keep from having a bunch
    // of "defunct" processes.

#ifndef DEBUG_COMM

    _reapChildren();

#endif

    // Register the latest data time with the server mapper
    // every SPDB_SERVER_DATA_TIME_UPDATE seconds

    time_t current_time = time(NULL);
    
    if (current_time - last_data_time_update > SPDB_SERVER_DATA_TIME_UPDATE)
    {
      si32 last_time;
      si32 begin_time, end_time;
      
      if (SPDB_last_valid_time(&_spdbHandle,
			       &last_time) == 0)
	SMU_auto_last_data(last_time);

      if (SPDB_first_and_last_times(&_spdbHandle,
				    &begin_time, &end_time) == 0)
	SMU_auto_start_and_end_data(begin_time, end_time);
      
      last_data_time_update = current_time;
    }
    
    // Wait for a client

    if ((_clientfd = SKU_get_client_timed(_serverfd,
					 _waitMsecs)) < 0)
    {
      if (_clientfd != -1)      // timed out
      {
	if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
	{
	  fprintf(stderr, "ERROR - %s::%s\n",
		  _className(), routine_name);
	  fprintf(stderr, "Error getting client on port %d\n",
		  _port);
	}
      }
      
      continue;
    }
    
#ifndef DEBUG_COMM

    // Make sure we don't have too many child processes running

    while (_numChildren > _maxChildren)
    {
      // Register with the server mapper and process mapper again,
      // in case there is a problem with getting a child process.

      SMU_auto_register();
      PMU_auto_register("waiting for available child process");
    
      _reapChildren();
      sleep(1);
    }

    // Spawn a child process to process the received message

    int pid;
    
    if ((pid = fork()) != 0)
    {
      // This is the parent process

      // Keep track of the number of child process

      _numChildren++;
      
      // Close the client socket.  The child process will
      // handle the request.  Make sure you don't "hang up"
      // the socket or the child won't be able to use it.

      SKU_close_no_hangup(_clientfd);

    } /* endif - parent process */
    else
    {
      // This is the child process

#endif
      
      // Process the message received from the client

      _handleClient();

      // close connection
      
      SKU_close(_clientfd);
	
#ifndef DEBUG_COMM

      // child's work is done - exit
      
      _exit(0);

    } /* endelse - child process */
    
    // Reap any outstanding children (there should just be the one)
    // to keep from having a bunch of "defunct" processes.

    _reapChildren();

#endif
    
  } /* endwhile - Forever */
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return;
}


/*********************************************************************
 * handleSigpipe() - Close all open sockets when a SIGPIPE signal is
 *                   received.  The application should call this
 *                   routine in its SIGPIPE handler.
 */

void SpdbServer::handleSigpipe(void)
{
  return;
}


/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * _handleClient() - handle the client request communications
 */

void SpdbServer::_handleClient()

{

  static char *routine_name = "_handleClient";
  
  SKU_header_t sku_header;
  int sku_status;
  char *msg_buffer;
  char *buf_ptr;
  long msg_buffer_len;
  
  si32 request;
  
  // Process the message received from the client
  
  if ((sku_status = SKU_read_message(_clientfd,
				     &sku_header,
				     &msg_buffer,
				     &msg_buffer_len,
				     -1)) != 1)
    {
      if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
      {
	fprintf(stderr, "ERROR - %s::%s\n",
		_className(), routine_name);
	fprintf(stderr, "Error %d reading request from client "
		"on port %d (_clientfd = %d)\n",
		sku_status, _port, _clientfd);
      }
      
      return;
    }

  // Extract the message
  
  buf_ptr = msg_buffer;
  
  request = *(si32 *)buf_ptr;
  request = BE_to_si32(request);
  buf_ptr += sizeof(si32);
  
  _processRequest(request,
		  sku_header.id,
		  buf_ptr,
		  msg_buffer_len - sizeof(si32),
		  _clientfd);
  
  // Close the socket
  
  return;

}


/*********************************************************************
 * _processRequest() - process a request from a client.
 */

SpdbServerStatus SpdbServer::_processRequest(const int request,
					     const int product_id,
					     char *request_info,
					     int request_info_len,
					     int clientfd)
{
  static char *routine_name = "_processRequest";
  
  SpdbServerStatus request_return;
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_MSGS)
    fprintf(stderr, "%s::%s - Received %s request\n",
	    _className(), routine_name,
	    SPDB_request2string(request));
    
  switch(request)
  {
  case SPDB_GET_DATA :
    request_return = _processGetDataRequest(product_id,
					    request_info,
					    request_info_len,
					    clientfd);
    break;
    
  case SPDB_GET_DATA_CLOSEST :
  case SPDB_GET_DATA_FIRST_BEFORE :
  case SPDB_GET_DATA_FIRST_AFTER :
    request_return = _processGetDataClosestRequest(request,
						   product_id,
						   request_info,
						   request_info_len,
						   clientfd);
    break;
    
  case SPDB_GET_DATA_INTERVAL :
    request_return = _processGetDataIntervalRequest(product_id,
						    request_info,
						    request_info_len,
						    clientfd);
    break;
    
  case SPDB_GET_DATA_VALID :
    request_return = _processGetDataValidRequest(product_id,
						 request_info,
						 request_info_len,
						 clientfd);
    break;
    
  case SPDB_PUT_DATA :
  case SPDB_PUT_DATA_ADD :
  case SPDB_PUT_DATA_OVER :
    request_return = _processPutDataRequest(product_id,
					    request_info,
					    request_info_len,
					    request,
					    clientfd);
    break;
    
  default:
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Invalid request %d received\n",
	      request);
    }
    
    _sendSimpleReply(clientfd, SPDB_REQUEST_ERROR);
    
    return(SPDB_SERVER_INVALID_REQUEST);
    
    break;
    
  } /* endswitch - request */

  if (_debugLevel >= SPDB_SERVER_DEBUG_MSGS)
  {
    umalloc_count();
  }
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(request_return);
}    


/*********************************************************************
 * _processGetDataRequest() - process an SPDB_GET_DATA request from
 *                            a client.
 */

SpdbServerStatus
SpdbServer::_processGetDataRequest(int product_id,
				   char *request_info,
				   int request_info_len,
				   int clientfd)
{
  static char *routine_name = "_processGetDataRequest";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure the request is for the correct product id
  
  if (product_id != 0 &&
      product_id != _outputProductId)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Received request for product %d, outputing product %d\n",
	      product_id, _outputProductId);
    }
      
    _sendSimpleReply(_clientfd, SPDB_REQUEST_ERROR);
      
    return(SPDB_SERVER_REQUEST_ERROR);
  }
   
  si32 data_type;
  si32 request_time;
  
  // Check the request buffer to make sure it contains the correct
  // information.

  if (request_info_len != 2 * sizeof(si32))
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len), (long int)(sizeof(si32)));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  // Extract the request time from the buffer, remembering to byte-swap
  
  data_type = *(si32 *)request_info;
  data_type = BE_to_si32(data_type);
  
  request_time = *((si32 *)request_info + 1);
  request_time = BE_to_si32(request_time);
  
  // Get the requested data from the database

  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  
  if (SPDB_fetch(&_spdbHandle,
		 data_type,
		 request_time,
		 &nchunks,
		 &chunk_hdrs,
		 &chunk_data) != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error extracting data for request time %s\n",
	      utimstr(request_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Make sure we were able to fetch the data

  if (nchunks <= 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr,
	      "ERROR - %s::%s - No data from SPDB_fetch - request_time = %s",
	      _className(), routine_name, utimstr(request_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Transform the data, if necessary

  void *transformed_data;
  spdb_chunk_ref_t *transformed_chunk_hdrs;
  si32 transformed_nchunks;

  if ((transformed_data =
       _transformOutputData(nchunks,
			    chunk_hdrs,
			    chunk_data,
			    &transformed_nchunks,
			    &transformed_chunk_hdrs)) == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Make sure the output product id is correct

  for (int i = 0; i < transformed_nchunks; i++)
    transformed_chunk_hdrs[i].prod_id = _outputProductId;
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 transformed_nchunks,
						 transformed_chunk_hdrs,
						 transformed_data);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(reply_status);
}


/*********************************************************************
 * _processGetDataClosestRequest() - process an SPDB_GET_DATA_CLOSEST
 *                                   request from a client.
 */

SpdbServerStatus
SpdbServer::_processGetDataClosestRequest(const int request,
					  int product_id,
					  char *request_info,
					  int request_info_len,
					  int clientfd)
{
  static char *routine_name = "_processGetDataClosestRequest";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure the request is for the correct product id
  
  if (product_id != 0 &&
      product_id != _outputProductId)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Received request for product %d, outputing product %d\n",
	      product_id, _outputProductId);
    }
      
    _sendSimpleReply(_clientfd, SPDB_REQUEST_ERROR);
      
    return(SPDB_SERVER_REQUEST_ERROR);
  }
   
  si32 data_type;
  si32 request_time;
  si32 time_margin;
  
  // Check the request buffer to make sure it contains the correct
  // information.

  if (request_info_len != 3 * sizeof(si32))
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len),
	      (long int)(2 * sizeof(si32)));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  // Extract the request time and time margin from the buffer,
  // remembering to byte-swap
  
  data_type = *(si32 *)request_info;
  data_type = BE_to_si32(data_type);
  
  request_time = *((si32 *)request_info + 1);
  request_time = BE_to_si32(request_time);
  
  time_margin = *((si32 *)request_info + 2);
  time_margin = BE_to_si32(time_margin);
  
  // Get the requested data from the database

  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  int iret;

  switch (request) {

  case SPDB_GET_DATA_CLOSEST :
    iret = SPDB_fetch_closest(&_spdbHandle,
			      data_type,
			      request_time,
			      time_margin,
			      &nchunks,
			      &chunk_hdrs,
			      &chunk_data);
    break;

  case SPDB_GET_DATA_FIRST_BEFORE :
    iret = SPDB_fetch_first_before(&_spdbHandle,
				   data_type,
				   request_time,
				   time_margin,
				   &nchunks,
				   &chunk_hdrs,
				   &chunk_data);
    break;


  case SPDB_GET_DATA_FIRST_AFTER :
    iret = SPDB_fetch_first_after(&_spdbHandle,
				  data_type,
				  request_time,
				  time_margin,
				  &nchunks,
				  &chunk_hdrs,
				  &chunk_data);
    break;
    
  } // switch

  if (iret != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error extracting data for request time %s, time margin is %d\n",
	      utimstr(request_time), time_margin);
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Make sure we were able to fetch the data

  if (nchunks <= 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr,
	      "ERROR - %s::%s - No data from SPDB_fetch - time_margin = %d, request_time = %s\n",
	      _className(), routine_name, time_margin, utimstr(request_time));
    }

    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Transform the data, if necessary

  void *transformed_data;
  spdb_chunk_ref_t *transformed_chunk_hdrs;
  si32 transformed_nchunks;

  if ((transformed_data =
       _transformOutputData(nchunks,
			    chunk_hdrs,
			    chunk_data,
			    &transformed_nchunks,
			    &transformed_chunk_hdrs)) == NULL)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
      fprintf(stderr,
	      "ERROR - %s::%s - Output transformation failed\n",
	      _className(), routine_name);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Make sure the output product id is correct

  for (int i = 0; i < transformed_nchunks; i++)
    transformed_chunk_hdrs[i].prod_id = _outputProductId;
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 transformed_nchunks,
						 transformed_chunk_hdrs,
						 transformed_data);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(reply_status);
}


/*********************************************************************
 * _processGetDataIntervalRequest() - process an SPDB_GET_DATA_INTERVAL
 *                                    request from a client.
 */

SpdbServerStatus
SpdbServer::_processGetDataIntervalRequest(int product_id,
					   char *request_info,
					   int request_info_len,
					   int clientfd)
{
  static char *routine_name = "_processGetDataIntervalRequest";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure the request is for the correct product id
  
  if (product_id != 0 &&
      product_id != _outputProductId)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Received request for product %d, outputing product %d\n",
	      product_id, _outputProductId);
    }
      
    _sendSimpleReply(_clientfd, SPDB_REQUEST_ERROR);
      
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  si32 data_type;
  si32 start_time;
  si32 end_time;
  
  // Check the request buffer to make sure it contains the correct
  // information.

  if (request_info_len != 3 * sizeof(si32))
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len),
	      (long int)(2 * sizeof(si32)));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  // Extract the start and end times from the buffer, remembering
  // to byte-swap
  
  data_type = *(si32 *)request_info;
  data_type = BE_to_si32(data_type);
  
  start_time = *((si32 *)request_info + 1);
  start_time = BE_to_si32(start_time);
  
  end_time = *((si32 *)request_info + 2);
  end_time = BE_to_si32(end_time);
  
  // Get the requested data from the database

  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  
  if (SPDB_fetch_interval(&_spdbHandle,
			  data_type,
			  start_time,
			  end_time,
			  &nchunks,
			  &chunk_hdrs,
			  &chunk_data) != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error extracting data for start time %s, end time %s\n",
	      utimstr(start_time), utimstr(end_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Make sure we were able to fetch the data

  if (nchunks <= 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr,
	      "ERROR - %s::%s - No data from SPDB_fetch_interval - start_time = %s\n",
	      _className(), routine_name, utimstr(start_time));

      fprintf(stderr,
	      "                               end_time = %s\n",
	      utimstr(end_time));

    }
    
    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Transform the data, if necessary

  void *transformed_data;
  spdb_chunk_ref_t *transformed_chunk_hdrs;
  si32 transformed_nchunks;

  if ((transformed_data =
       _transformOutputData(nchunks,
			    chunk_hdrs,
			    chunk_data,
			    &transformed_nchunks,
			    &transformed_chunk_hdrs)) == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Make sure the output product id is correct

  for (int i = 0; i < transformed_nchunks; i++)
    transformed_chunk_hdrs[i].prod_id = _outputProductId;
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 transformed_nchunks,
						 transformed_chunk_hdrs,
						 transformed_data);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(reply_status);
}


/*********************************************************************
 * _processGetDataValidRequest() - process an SPDB_GET_DATA_VALID
 *                                 request from a client.
 */

SpdbServerStatus
SpdbServer::_processGetDataValidRequest(int product_id,
					char *request_info,
					int request_info_len,
					int clientfd)
{
  static char *routine_name = "_processGetDataValidRequest";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure the request is for the correct product id
  
  if (product_id != 0 &&
      product_id != _outputProductId)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Received request for product %d, outputing product %d\n",
	      product_id, _outputProductId);
    }
      
    _sendSimpleReply(_clientfd, SPDB_REQUEST_ERROR);
      
    return(SPDB_SERVER_REQUEST_ERROR);
  }
   
  si32 data_type;
  si32 search_time;

  // Check the request buffer to make sure it contains the correct
  // information.

  if (request_info_len != 2 * sizeof(si32))
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len), (long int)(sizeof(si32)));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  // Extract the search time from the buffer, remembering to byte-swap
  
  data_type = *(si32 *)request_info;
  data_type = BE_to_si32(data_type);
  
  search_time = *((si32 *)request_info + 1);
  search_time = BE_to_si32(search_time);
  
  // Get the requested data from the database

  si32 nchunks;
  spdb_chunk_ref_t *chunk_hdrs;
  void *chunk_data;
  
  if (SPDB_fetch_valid(&_spdbHandle,
		       data_type,
		       search_time,
		       &nchunks,
		       &chunk_hdrs,
		       &chunk_data) != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error extracting data for searcj time %s\n",
	      utimstr(search_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_DATA_ERROR);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Make sure we were able to fetch the data

  if (nchunks <= 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr,
	      "ERROR - %s::%s - No data from SPDB_fetch_valid - search_time = %s",
	      _className(), routine_name, utimstr(search_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Transform the data, if necessary

  void *transformed_data;
  spdb_chunk_ref_t *transformed_chunk_hdrs;
  si32 transformed_nchunks;

  if ((transformed_data =
       _transformOutputData(nchunks,
			    chunk_hdrs,
			    chunk_data,
			    &transformed_nchunks,
			    &transformed_chunk_hdrs)) == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Make sure the output product id is correct

  for (int i = 0; i < transformed_nchunks; i++)
    transformed_chunk_hdrs[i].prod_id = _outputProductId;
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 transformed_nchunks,
						 transformed_chunk_hdrs,
						 transformed_data);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(reply_status);
}


/*********************************************************************
 * _transformOutputData() - apply the output transformation to the
 *                          SPDB data, if there is one
 */

void *SpdbServer::_transformOutputData(si32 nchunks,
				       spdb_chunk_ref_t *chunk_hdrs,
				       void *chunk_data,
				       si32 *trans_nchunks,
				       spdb_chunk_ref_t **trans_chunk_hdrs)
{
  static char *routine_name = "_transformOutputData";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  void *transformed_data;
  
  // Initialize returned values

  *trans_nchunks = 0;
  *trans_chunk_hdrs = NULL;

  // Get rid of duplicate records, if requested

  if (_latestOnlyFlag)
  {
    int next_chunk = 0;
    int next_chunk_offset = 0;

    for (int i = 0; i < nchunks; i++)
    {
      int delete_flag = FALSE;

      if (chunk_hdrs[i].data_type != 0)
      {
	for (int j = i+1; j < nchunks; j++)
	{
	  if (chunk_hdrs[i].data_type == chunk_hdrs[j].data_type)
	  {
	    delete_flag = TRUE;
	    if (_debugLevel >= SPDB_SERVER_DEBUG_MSGS)
	    {
	      fprintf(stderr,
		      "%s::%s\n",
		      _className(), routine_name);
	      fprintf(stderr,
		      "Skipping duplicate record for data type %d\n",
		      chunk_hdrs[i].data_type);
	    }
	  } 
	} /* endfor - j */

      } /* endif - data_type != 0 */

      // Move this record up to the earliest available slot

      if (!delete_flag)
      {
	// Move the chunk data

	memcpy((char *)chunk_data + next_chunk_offset,
	       (char *)chunk_data + chunk_hdrs[i].offset,
	       chunk_hdrs[i].len);

	// Move the chunk header information

	chunk_hdrs[next_chunk] = chunk_hdrs[i];
	chunk_hdrs[next_chunk].offset = next_chunk_offset;

	// Update the offsets

	next_chunk_offset += chunk_hdrs[i].len;
	next_chunk++;

      } /* endif - !delete_flag */

    } /* endfor - i */

    nchunks = next_chunk;

  } /* endif - _latestOnlyFLag */

  // Now transform the data

  if (_outputTransform == NULL)
  {
    transformed_data = chunk_data;
    *trans_nchunks = nchunks;
    *trans_chunk_hdrs = chunk_hdrs;
  }
  else
  {
    int transformed_len;
    int transformed_offset = 0;
    void *transformed_chunk;
    spdb_chunk_ref_t transformed_chunk_hdr;

    MEMbufReset(_transformBuf);
    MEMbufReset(_transformHdrs);

    for (int i = 0; i < nchunks; i++)
    {
      if ((transformed_chunk =
	   _outputTransform(&chunk_hdrs[i],
			    (void *)((char *)chunk_data +
				     chunk_hdrs[i].offset),
			    chunk_hdrs[i].len,
			    &transformed_len)) == NULL)
      {
	if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
	{
	  fprintf(stderr, "%s::%s\n", _className(), routine_name);
	  fprintf(stderr, "WARNING - transformed output for chunk %d is NULL\n",
		  i);
	}
	
	continue;
      }
      
      MEMbufAdd(_transformBuf, transformed_chunk, transformed_len);

      transformed_chunk_hdr = chunk_hdrs[i];
      transformed_chunk_hdr.len = transformed_len;
      transformed_chunk_hdr.offset = transformed_offset;
      MEMbufAdd(_transformHdrs, &transformed_chunk_hdr,
		sizeof(spdb_chunk_ref_t));

      transformed_offset += transformed_len;
      (*trans_nchunks)++;

    } /* endfor - i */

    transformed_data = MEMbufPtr(_transformBuf);
    *trans_chunk_hdrs = (spdb_chunk_ref_t *)MEMbufPtr(_transformHdrs);
  }

  return(transformed_data);
  
}

  
/*********************************************************************
 * _processPutDataRequest() - process an SPDB_PUT_DATA request from a
 *                            client.
 */

SpdbServerStatus
SpdbServer::_processPutDataRequest(int product_id,
				   char *request_info,
				   int request_info_len,
				   const int request_type,
				   int clientfd)
{
  static char *routine_name = "_processPutDataRequest";
  
  char *request_ptr = request_info;
  
  time_t latest_data_time = 0;
  
  // Make sure the request is for the correct product id
  
  if (product_id != 0 &&
      product_id != _inputProductId)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Received request for product %d, inputing product %d\n",
	      product_id, _inputProductId);
    }
      
    _sendSimpleReply(_clientfd, SPDB_REQUEST_ERROR);
      
    return(SPDB_SERVER_REQUEST_ERROR);
  }
   
  si32 nchunks;
  si32 data_length;
  int expected_msg_length;
  spdb_chunk_ref_t *input_hdrs;
  spdb_chunk_ref_t chunk_hdr;
  void *input_data;
  void *chunk_data;
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Get the number of chunks from the request info -- remember
  // to byte-swap

  nchunks = *(si32 *)request_ptr;
  nchunks = BE_to_si32(nchunks);
  request_ptr += sizeof(si32);
  
  // Get the data length from the request info -- remember to byte-swap

  data_length = *(si32 *)request_ptr;
  data_length = BE_to_si32(data_length);
  request_ptr += sizeof(si32);
  
  // Set the other input pointers

  input_hdrs = (spdb_chunk_ref_t *)request_ptr;
  input_data = (void *)((char *)input_hdrs +
			(nchunks * sizeof(spdb_chunk_ref_t)));
  
  // Check the request buffer to make sure it contains enough
  // information.

  expected_msg_length = (2 * sizeof(si32)) +
    (nchunks * sizeof(spdb_chunk_ref_t)) + data_length;
  
  if (request_info_len != expected_msg_length)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr,
	      "Request information contains %d bytes, should contain %d bytes.\n",
	      request_info_len, expected_msg_length);
    }
    
    _sendSimpleReply(clientfd, SPDB_PUT_FAILED);
    
    return(SPDB_SERVER_REQUEST_ERROR);
  }
  
  // Store the data in the database

  for (int i = 0; i < nchunks; i++)
  {
    int store_return;
    
    // Get the header information for this chunk -- remember to byte-swap

    chunk_hdr = input_hdrs[i];
    BE_to_array_32(&chunk_hdr, sizeof(chunk_hdr));
    
    chunk_data = (void *)((char *)input_data + chunk_hdr.offset);
    
    // Transform the data, if necessary

    void *transformed_data;
    int transformed_len;
    
    if (_inputTransform == NULL)
    {
      transformed_data = chunk_data;
      transformed_len = chunk_hdr.len;
    }
    else
    {
      if ((transformed_data = _inputTransform(&chunk_hdr,
					      chunk_data,
					      chunk_hdr.len,
					      &transformed_len)) == NULL)
      {
	if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
	{
	  fprintf(stderr, "%s::%s\n",
		  _className(), routine_name);
	  fprintf(stderr, "ERROR - Error transforming chunk %d for input\n",
		  i);
	}
	
	return(SPDB_SERVER_DATA_ERROR);
      }
    }
    
    // Store the chunk information -- don't need to byte-swap because data
    // comes over socket in big-endian format and that's what we want to
    // store it in.

    switch(request_type)
    {
    case SPDB_PUT_DATA :
      store_return = SPDB_store(&_spdbHandle,
				chunk_hdr.data_type,
				chunk_hdr.valid_time,
				chunk_hdr.expire_time,
				transformed_data,
				transformed_len);
      break;
      
    case SPDB_PUT_DATA_ADD :
      store_return = SPDB_store_add(&_spdbHandle,
				    chunk_hdr.data_type,
				    chunk_hdr.valid_time,
				    chunk_hdr.expire_time,
				    transformed_data,
				    transformed_len);
      break;
      
    case SPDB_PUT_DATA_OVER :
      store_return = SPDB_store_over(&_spdbHandle,
				     chunk_hdr.data_type,
				     chunk_hdr.valid_time,
				     chunk_hdr.expire_time,
				     transformed_data,
				     transformed_len);
      break;
      
    default:
      if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
      {
	fprintf(stderr, "ERROR - %s::%s\n",
		_className(), routine_name);
	fprintf(stderr, "Invalid request type %s\n",
		SPDB_request2string(request_type));
      }
      
      _sendSimpleReply(clientfd, SPDB_PUT_FAILED);
      
      return(SPDB_SERVER_REQUEST_ERROR);
      
      break;
    } /* endswitch - request_type */

    // Compute the latest data time to be sent to the server mapper

    if (chunk_hdr.valid_time > latest_data_time)
      latest_data_time = chunk_hdr.valid_time;
    
    if (store_return != 0)
    {
      if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
      {
	fprintf(stderr, "ERROR - %s::%s\n",
		_className(), routine_name);
	fprintf(stderr, "Error storing data for chunk %d\n", i);
      }
	
      _sendSimpleReply(clientfd, SPDB_PUT_FAILED);
	
      return(SPDB_SERVER_DATA_ERROR);
    }
  }
  
  // register latest data

  if (latest_data_time > 0)
    SMU_auto_last_data(latest_data_time);
    
  // Send the reply to the client

  _sendSimpleReply(clientfd, SPDB_PUT_SUCCESSFUL);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(SPDB_SERVER_SUCCESS);
}


/*********************************************************************
 * _sendSimpleReply() - Send a simple reply (a reply with no data) to
 *                      the client.
 */

SpdbServerStatus SpdbServer::_sendSimpleReply(int clientfd,
					      spdb_reply_t reply)
{
  static char *routine_name = "_sendSimpleReply";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_MSGS)
  {
    fprintf(stderr,
	    "%s::%s\n",
	    _className(), routine_name);
    fprintf(stderr,
	    "Sending reply %s to client\n",
	    SPDB_reply2string(reply));
  }
  
  // Byte swap the reply

  si32 send_reply = reply;
  send_reply = BE_from_si32(send_reply);
  
  // Send the reply to the client

  if (SKU_write_message(clientfd,
			_databaseProductId,
			(char *)&send_reply,
			sizeof(send_reply)) != 1)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error writing simple reply to client on port %d\n",
	      _port);
    }
    
    return(SPDB_SERVER_WRITE_ERROR);
  }

  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(SPDB_SERVER_SUCCESS);
}


/*********************************************************************
 * _sendDataReply() - Send an SPDB_DATA reply to a client.
 */

SpdbServerStatus SpdbServer::_sendDataReply(int clientfd,
					    si32 nchunks,
					    spdb_chunk_ref_t *chunk_hdrs,
					    void *chunk_data)
{
  static char *routine_name = "_sendDataReply";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  // Make sure there was some data for this time

  if (nchunks <= 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
    {
      fprintf(stderr, "%s::%s - No data available for request\n",
	      _className(), routine_name);
    }
    
    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Create the reply buffer and byte-swap where necessary.  Note that
  // the actual chunk data will be retrieved from the database in
  // big-endian format.

  ui08 *reply_buffer;
  ui08 *buf_ptr;
  si32 reply;
  
  ui32 chunk_data_size = 0;
  
  for (int i = 0; i < nchunks; i++)
    chunk_data_size += chunk_hdrs[i].len;
  
  ui32 reply_len = sizeof(si32) + (2 * sizeof(ui32)) +
    (nchunks * sizeof(spdb_chunk_ref_t)) + chunk_data_size;
  
  reply_buffer = (ui08 *)umalloc(reply_len);
  buf_ptr = reply_buffer;
  
  // reply
  reply = SPDB_DATA;
  memcpy(buf_ptr, &reply, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  // nchunks
  memcpy(buf_ptr, &nchunks, sizeof(ui32));
  BE_from_array_32(buf_ptr, sizeof(ui32));
  buf_ptr += sizeof(ui32);
  
  // data length
  memcpy(buf_ptr, &chunk_data_size, sizeof(ui32));
  BE_from_array_32(buf_ptr, sizeof(ui32));
  buf_ptr += sizeof(ui32);
  
  // chunk headers
  memcpy(buf_ptr, chunk_hdrs, nchunks * sizeof(spdb_chunk_ref_t));
  BE_from_array_32(buf_ptr, nchunks * sizeof(spdb_chunk_ref_t));
  buf_ptr += nchunks * sizeof(spdb_chunk_ref_t);

  // chunk data (no byte-swapping)
  memcpy(buf_ptr, chunk_data, chunk_data_size);

  // Send the reply to the client

  if (SKU_write_message(clientfd,
			_outputProductId,
			(char *)reply_buffer,
			reply_len) != 1)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error writing reply to client on port %d\n",
	      _port);
    }
    
    ufree(reply_buffer);
    
    return(SPDB_SERVER_WRITE_ERROR);
  }

  ufree(reply_buffer);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(SPDB_SERVER_SUCCESS);
}


/*********************************************************************
 * _reapChildren() - Reap any outstanding child processes.
 */

void SpdbServer::_reapChildren(void)
{
  while (waitpid((pid_t)-1,
		 (int *)NULL,
		 (int)(WNOHANG | WUNTRACED)) > 0)
  {
    _numChildren--;
  }
  
  return;
}
