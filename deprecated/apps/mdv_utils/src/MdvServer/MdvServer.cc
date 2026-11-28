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
// MdvServer.cc
//
// MdvServer object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <sys/wait.h>

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <mdv/mdv_client.h>
#include <mdv/mdv_handle.h>
#include <mdv/mdv_utils.h>
#include <toolsa/globals.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/servmap.h>
#include <toolsa/smu.h>
#include <toolsa/sockutil.h>
#include <toolsa/str.h>

#include "MdvServer.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;


const int Forever = TRUE;


/**************************************************************
 * Constructor
 */

MdvServer::MdvServer(int argc, char **argv)
{
  char *routine_name = "Constructor()";
  
  okay = TRUE;
  done = FALSE;

  // Set program name

  char *slash_pos = strrchr(argv[0], '/');
  
  if (slash_pos == (char *)NULL)
    _programName = STRdup(argv[0]);
  else
    _programName = STRdup(slash_pos + 1);
  
  // Get command line args

  _args = new Args(argc, argv, _programName);

  if (!_args->okay)
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with command line args\n");
    okay = FALSE;
    return;
  }

  if (_args->done)
  {
    done = TRUE;
    return;
  }

  // Get the data directory

  if ((_dataDirectory = getenv("RAP_DATA_DIR")) == (char *)NULL)
  {
    fprintf(stderr,
	    "ERROR:  $RAP_DATA_DIR not set\n");
    exit(0);
  }
  
  // Get TDRP params

  _params = new Params();
  _paramsPath = (char *) "unknown";

  if (_params->loadFromArgs(argc, argv,
                            _args->override.list,
                            &_paramsPath))
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with TDRP parameters\n");
    okay = FALSE;
    return;
  }

  if (!okay)
    return;

  // Create the DsInputPath object for the input directory

  _inputPath = new DsInputPath(_programName,
			       _params->debug_level >= Params::DEBUG_VERBOSE,
			       _dataDirectory,
			       0,
			       PMU_auto_register);
  
  // Initialize the communications process information

  _numChildren = 0;
  _maxChildren = 10;
  
  // Open the server socket

  if ((_serverfd = SKU_open_server(_params->port)) < 0)
  {
    fprintf(stderr, "ERROR: %s::%s\n", _programName, routine_name);
    fprintf(stderr, "Error opening server socket on port %ld\n",
	    _params->port);
    
    exit(-1);
  }
  
  // Initialize the client socket information

  _clientfd = -1;
  
  // Initialize server registration

  SMU_auto_init("MDV",
		"MDV",
		_params->instance,
		_dataDirectory,
		_params->port,
		_params->realtime_avail,
		(SMU_fill_info_t)NULL);
  
  // Initialize process registration
  
  PMU_auto_init(_programName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);
  
}


/**************************************************************
 * Destructor
 */

MdvServer::~MdvServer()
{
  // Unregister the server

  SMU_auto_unregister();
  
  // Unregister the process

  PMU_auto_unregister();

  // Close the server socket

  SKU_close(_serverfd);
  
  // Free strings

  STRfree(_programName);
  
  // Call destructors

  delete _params;
  delete _args;

}


/**************************************************************
 * run()
 */

void MdvServer::run()
{
  static char *routine_name = "run()";
  
  static time_t last_data_time_update = 0;
  
  // Process new input files forever.
  
  while (Forever)
  {
    // Register with the server mapper and process mapper

    SMU_auto_register();
    PMU_auto_register("Waiting for requests");
     
    // Reap any outstanding children to keep from having a bunch
    // of "defunct" processes.

    _reapChildren();
    
    // Register the latest data time with the server mapper
    // every MDV_SERVER_DATA_TIME_UPDATE seconds

    time_t current_time = time((time_t *)NULL);
    
    if (current_time - last_data_time_update > MDV_SERVER_DATA_TIME_UPDATE)
    {
      time_t last_time, begin_time, end_time;
      
      if ((last_time = _inputPath->getLastTime()) > 0)
	SMU_auto_last_data(last_time);
      
      if (_inputPath->getBeginAndEndTimes(&begin_time, &end_time) == 0)
	SMU_auto_start_and_end_data(begin_time, end_time);
      
      last_data_time_update = current_time;
    }
    
    // Wait for a client

    if ((_clientfd = SKU_get_client_timed(_serverfd,
					  MDV_SERVER_WAIT_MSECS)) < 0)
    {
      if (_clientfd != -1)    // error other than timeout
      {
	if (_params->debug_level >= Params::DEBUG_NORM)
	{
	  fprintf(stderr, "ERROR: %s::%s\n", _programName, routine_name);
	  fprintf(stderr, "Error getting client on port %ld\n",
		  _params->port);
	}
	
      }

      continue;
    }
    
    // Make sure we don't have too many child processes running

    while (_numChildren > _maxChildren)
    {
      // Register with the server mapper and process mapper again,
      // in case there is a problem getting a child process.
      
      SMU_auto_register();
      PMU_auto_register("waiting for available child process");
      
      _reapChildren();
      sleep(1);
    }
    
    // Spawn a child process ot process the received message

    int pid;
    
    if ((pid = fork()) != 0)
    {
      /*
       * This is the parent process
       */

      // Keep track of the number of child processes

      _numChildren++;
      
      // Close the client socket.  The child process will
      // handle the request.  Make sure you don't "hang up"
      // the socket or the child won't be able to use it.

      SKU_close_no_hangup(_clientfd);
      
    }
    else
    {
      /*
       * This is the child process
       */

      // Process the message received from the client

      _handleClient();
      
      // Close the connection.

      SKU_close(_clientfd);
      
      // Child's work is done -- exit

      exit(0);
    }
    
    // Reap any outstanding children (there should just be the one)
    // to keep from haveing a bunch of "defunct" processes.

    _reapChildren();
    
  } /* endwhile - Forever */
    
  return;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/*********************************************************************
 * _handleClient() - handle the client request communications
 */

void MdvServer::_handleClient()
{
  static char *routine_name = "_handleClient()";
  
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
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR: %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr,
	      "Error %d reading request from client on port %ld (_clientfd = %d)\n",
	      sku_status, _params->port, _clientfd);
    }
      
    return;
  }

  // Extract the message
  
  buf_ptr = msg_buffer;
  
  request = *(si32 *)buf_ptr;
  request = BE_to_si32(request);
  buf_ptr += sizeof(si32);
  
  _processRequest(request,
		  buf_ptr,
		  msg_buffer_len - sizeof(si32),
		  _clientfd);
  
  return;

}


/*********************************************************************
 * _processRequest() - process a request from a client.
 */

MdvServerStatus MdvServer::_processRequest(const int request,
					   char *request_info,
					   int request_info_len,
					   int clientfd)
{
  static char *routine_name = "_processRequest";
  
  MdvServerStatus request_return;
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr, "%s::%s - Received %s request\n",
	    _programName, routine_name,
	    MDV_request2string(request));
    
  switch(request)
  {
  case MDV_GET_CLOSEST :
  case MDV_GET_FIRST_BEFORE :
  case MDV_GET_FIRST_AFTER :
  case MDV_GET_LATEST :
  case MDV_GET_NEW :
    request_return = _processGetRequest(request,
					request_info,
					request_info_len,
					clientfd);
    break;

  case MDV_INFO_DATASET_TIMES_REQUEST :
    request_return = _processInfoDatasetTimesRequest((MDV_dataset_time_request_t *)request_info,
						     request_info_len,
						     clientfd);
    break;
    
//  case MDV_PUT_DATA :
//    request_return = _processPutRequest(product_id,
//					    request_info,
//					    request_info_len,
//					    request,
//					    clientfd);
//    break;
    
  default:
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr, "Invalid request %d received\n",
	      request);
    }
    
    _sendSimpleReply(clientfd, MDV_REQUEST_ERROR);
    
    return(MDV_SERVER_INVALID_REQUEST);
    
    break;
    
  } /* endswitch - request */

  return(request_return);
}    


/*********************************************************************
 * _processGetRequest() - process one of the  MDV_GET_xxx requests from
 *                        a client.
 */

MdvServerStatus MdvServer::_processGetRequest(const int request,
					      char *request_info,
					      int request_info_len,
					      int clientfd)
{
  static char *routine_name = "_processGetRequest";
  
  MDV_handle_t *mdv_handle;
  int iret;

  // Check the request buffer to make sure it contains the correct
  // amount of information.

  if (request_info_len != MDV_request_size(request))
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len),
	      (long int)(MDV_request_size(request)));
    }
    
    _sendSimpleReply(clientfd, MDV_REQUEST_ERROR);
      
    return(MDV_SERVER_REQUEST_ERROR);
  }
  
  // Byte-swap the request information.
  
  MDV_request_from_BE(request, request_info, request_info_len);
  
  // Get the requested data based on the request information.

  switch (request)
  {
  case MDV_GET_CLOSEST :
  {
    MDV_request_closest_t *request_ptr =
      (MDV_request_closest_t *)request_info;

    // Get the requested data.

    if (request_ptr->gen_info.crop_flag)
      iret = MDV_get_closest(_dataDirectory,
			     request_ptr->time,
			     request_ptr->margin,
			     request_ptr->gen_info.return_type,
			     (char *)request_ptr->gen_info.field_name,
			     request_ptr->gen_info.field_num,
			     request_ptr->gen_info.plane_height_type,
			     request_ptr->gen_info.plane_height,
			     &request_ptr->gen_info.crop_info,
			     request_ptr->gen_info.composite_type,
			     &mdv_handle);
    else
      iret = MDV_get_closest(_dataDirectory,
			     request_ptr->time,
			     request_ptr->margin,
			     request_ptr->gen_info.return_type,
			     (char *)request_ptr->gen_info.field_name,
			     request_ptr->gen_info.field_num,
			     request_ptr->gen_info.plane_height_type,
			     request_ptr->gen_info.plane_height,
			     (MDV_request_crop_t *)NULL,
			     request_ptr->gen_info.composite_type,
			     &mdv_handle);
    break;
  } /* endcase - MDV_GET_CLOSEST */
  
  case MDV_GET_FIRST_BEFORE :
  {
    MDV_request_closest_t *request_ptr =
      (MDV_request_closest_t *)request_info;

    // Get the requested data.

    if (request_ptr->gen_info.crop_flag)
      iret = MDV_get_first_before(_dataDirectory,
				  request_ptr->time,
				  request_ptr->margin,
				  request_ptr->gen_info.return_type,
				  (char *)request_ptr->gen_info.field_name,
				  request_ptr->gen_info.field_num,
				  request_ptr->gen_info.plane_height_type,
				  request_ptr->gen_info.plane_height,
				  &request_ptr->gen_info.crop_info,
				  request_ptr->gen_info.composite_type,
				  &mdv_handle);
    else
      iret = MDV_get_first_before(_dataDirectory,
				  request_ptr->time,
				  request_ptr->margin,
				  request_ptr->gen_info.return_type,
				  (char *)request_ptr->gen_info.field_name,
				  request_ptr->gen_info.field_num,
				  request_ptr->gen_info.plane_height_type,
				  request_ptr->gen_info.plane_height,
				  (MDV_request_crop_t *)NULL,
				  request_ptr->gen_info.composite_type,
				  &mdv_handle);
    break;
  } /* endcase - MDV_GET_FIRST_BEFORE */
  
  case MDV_GET_FIRST_AFTER :
  {
    MDV_request_closest_t *request_ptr =
      (MDV_request_closest_t *)request_info;

    // Get the requested data.

    if (request_ptr->gen_info.crop_flag)
      iret = MDV_get_first_after(_dataDirectory,
				 request_ptr->time,
				 request_ptr->margin,
				 request_ptr->gen_info.return_type,
				 (char *)request_ptr->gen_info.field_name,
				 request_ptr->gen_info.field_num,
				 request_ptr->gen_info.plane_height_type,
				 request_ptr->gen_info.plane_height,
				 &request_ptr->gen_info.crop_info,
				 request_ptr->gen_info.composite_type,
				 &mdv_handle);
    else
      iret = MDV_get_first_after(_dataDirectory,
				 request_ptr->time,
				 request_ptr->margin,
				 request_ptr->gen_info.return_type,
				 (char *)request_ptr->gen_info.field_name,
				 request_ptr->gen_info.field_num,
				 request_ptr->gen_info.plane_height_type,
				 request_ptr->gen_info.plane_height,
				 (MDV_request_crop_t *)NULL,
				 request_ptr->gen_info.composite_type,
				 &mdv_handle);
    break;
  } /* endcase - MDV_GET_FIRST_AFTER */
  
  case MDV_GET_LATEST :
  {
    MDV_request_latest_t *request_ptr =
      (MDV_request_latest_t *)request_info;

    // Get the requested data.

    if (request_ptr->gen_info.crop_flag)
      iret = MDV_get_latest(_dataDirectory,
			    request_ptr->gen_info.return_type,
			    (char *)request_ptr->gen_info.field_name,
			    request_ptr->gen_info.field_num,
			    request_ptr->gen_info.plane_height_type,
			    request_ptr->gen_info.plane_height,
			    &request_ptr->gen_info.crop_info,
			    request_ptr->gen_info.composite_type,
			    &mdv_handle);
    else
      iret = MDV_get_latest(_dataDirectory,
			    request_ptr->gen_info.return_type,
			    (char *)request_ptr->gen_info.field_name,
			    request_ptr->gen_info.field_num,
			    request_ptr->gen_info.plane_height_type,
			    request_ptr->gen_info.plane_height,
			    (MDV_request_crop_t *)NULL,
			    request_ptr->gen_info.composite_type,
			    &mdv_handle);
    break;
  } /* endcase - MDV_GET_LATEST */
  
  case MDV_GET_NEW :
  {
    MDV_request_new_t *request_ptr =
      (MDV_request_new_t *)request_info;

    // Get the requested data.

    if (request_ptr->gen_info.crop_flag)
      iret = MDV_get_new(_dataDirectory,
			 request_ptr->last_data_time,
			 request_ptr->gen_info.return_type,
			 (char *)request_ptr->gen_info.field_name,
			 request_ptr->gen_info.field_num,
			 request_ptr->gen_info.plane_height_type,
			 request_ptr->gen_info.plane_height,
			 &request_ptr->gen_info.crop_info,
			 request_ptr->gen_info.composite_type,
			 &mdv_handle);
    else
      iret = MDV_get_new(_dataDirectory,
			 request_ptr->last_data_time,
			 request_ptr->gen_info.return_type,
			 (char *)request_ptr->gen_info.field_name,
			 request_ptr->gen_info.field_num,
			 request_ptr->gen_info.plane_height_type,
			 request_ptr->gen_info.plane_height,
			 (MDV_request_crop_t *)NULL,
			 request_ptr->gen_info.composite_type,
			 &mdv_handle);
    break;
  } /* endcase - MDV_GET_NEW */
  
  } /* endswitch - request */

  if (iret != 0)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr, "Error extracting data for request.\n");
    }
    
    _sendSimpleReply(clientfd, MDV_DATA_ERROR);
    
    return(MDV_SERVER_DATA_ERROR);
  }
  
  // Make sure we were able to fetch the data

  if (mdv_handle == (MDV_handle_t *)NULL)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr,
	      "ERROR - %s::%s - No data from MDV_get.\n",
	      _programName, routine_name);
    }

    _sendSimpleReply(clientfd, MDV_NO_DATA);
    
    return(MDV_SERVER_SUCCESS);
  }
  
  // Send the reply to the client

  MdvServerStatus reply_status = _sendDataReply(clientfd,
						mdv_handle);
  
  return(reply_status);
}


/*********************************************************************
 * _processInfoDatasetTimesRequest() - process a MDV_INFO_DATASET_TIMES_REQUEST
 *                                     request from a client.
 */

MdvServerStatus MdvServer::_processInfoDatasetTimesRequest(MDV_dataset_time_request_t *request_info,
							   int request_info_len,
							   int clientfd)
{
  static char *routine_name = "_processInfoDatasetTimesRequest";
  
  // Check the request buffer to make sure it contains the correct
  // amount of information.

  if (request_info_len != sizeof(MDV_dataset_time_request_t))
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr,
	      "Request information contains %ld bytes, should contain %ld bytes.\n",
	      (long int)(request_info_len),
	      (long int)(sizeof(MDV_dataset_time_request_t)));
    }
    
    _sendSimpleReply(clientfd, MDV_REQUEST_ERROR);
      
    return(MDV_SERVER_REQUEST_ERROR);
  }
  
  // Byte-swap the request information.
  
  MDV_dataset_time_request_from_BE(request_info);
  
  // Get the requested data.

  MDV_dataset_time_t *data_times;
  int num_datasets;
  
  data_times = MDV_get_dataset_times(_dataDirectory,
				     request_info->begin_gen_time,
				     request_info->end_gen_time,
				     &num_datasets);
  
  // Send the reply to the client

  MdvServerStatus reply_status = _sendDatasetTimesReply(clientfd,
							data_times,
							num_datasets);
  
  return(reply_status);
}


/*********************************************************************
 * _sendSimpleReply() - Send a simple reply (a reply with no data) to
 *                      the client.
 */

MdvServerStatus MdvServer::_sendSimpleReply(int clientfd,
					    int reply)
{
  static char *routine_name = "_sendSimpleReply";
  
  if (_params->debug_level >= Params::DEBUG_VERBOSE)
  {
    fprintf(stderr,
	    "%s::%s\n",
	    _programName, routine_name);
    fprintf(stderr,
	    "Sending reply %s to client\n",
	    MDV_reply2string(reply));
  }
  
  // Byte swap the reply

  si32 send_reply = reply;
  send_reply = BE_from_si32(send_reply);
  
  // Send the reply to the client

  if (SKU_write_message(clientfd,
			send_reply,
			(char *)&send_reply,
			sizeof(send_reply)) != 1)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr, "Error writing simple reply to client on port %ld\n",
	      _params->port);
    }
    
    return(MDV_SERVER_WRITE_ERROR);
  }

  return(MDV_SERVER_SUCCESS);
}


/*********************************************************************
 * _sendDataReply() - Send an MDV_DATA reply to a client.
 */

MdvServerStatus MdvServer::_sendDataReply(int clientfd,
					  MDV_handle_t *mdv_handle)
{
  static char *routine_name = "_sendDataReply";
  
  // Make sure there was some data for this time

  if (mdv_handle == (MDV_handle_t *)NULL)
  {
    if (_params->debug_level >= Params::DEBUG_VERBOSE)
    {
      fprintf(stderr, "%s::%s - No data available for request\n",
	      _programName, routine_name);
    }
    
    _sendSimpleReply(clientfd, MDV_NO_DATA);
    
    return(MDV_SERVER_SUCCESS);
  }
  
  // Create the reply buffer and byte-swap where necessary.

  ui08 *reply_buffer;
  ui08 *buf_ptr;
  si32 reply;
  
  ui32 reply_len = MDV_calc_buffer_size(mdv_handle) + sizeof(si32);
  
  reply_buffer = (ui08 *)umalloc(reply_len);
  buf_ptr = reply_buffer;
  
  // reply
  reply = MDV_DATA;
  memcpy(buf_ptr, &reply, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  // MDV data
  if (MDV_load_buffer(mdv_handle, (char *)buf_ptr) != 0)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR: %s::%s\n", _programName, routine_name);
      fprintf(stderr, "Error converting MDV data to buffer\n");
    }
    
    _sendSimpleReply(clientfd, MDV_DATA_ERROR);
    
    return(MDV_SERVER_DATA_ERROR);
  }
  
  // Send the reply to the client

  if (SKU_write_message(clientfd,
			reply,
			(char *)reply_buffer,
			reply_len) != 1)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr, "Error writing reply to client on port %ld\n",
	      _params->port);
    }
    
    ufree(reply_buffer);
    
    return(MDV_SERVER_WRITE_ERROR);
  }

  ufree(reply_buffer);
  
  return(MDV_SERVER_SUCCESS);
}


/*********************************************************************
 * _sendDatasetTimesReply() - Send an MDV_INFO_DATASET_TIMES_REPLY
 *                            reply to a client.
 */

MdvServerStatus MdvServer::_sendDatasetTimesReply(int clientfd,
						  MDV_dataset_time_t *data_times,
						  int num_datasets)
{
  static char *routine_name = "_sendDatasetTimesReply";
  
  // Create the reply buffer and byte-swap where necessary.

  ui08 *reply_buffer;
  ui08 *buf_ptr;
  si32 reply;
  
  ui32 reply_len = (2 * sizeof(si32)) +
    (num_datasets * sizeof(MDV_dataset_time_t));
  
  reply_buffer = (ui08 *)umalloc(reply_len);
  buf_ptr = reply_buffer;
  
  // reply
  reply = MDV_INFO_DATASET_TIMES_REPLY;
  memcpy(buf_ptr, &reply, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  // num_datasets
  si32 num_datasets_si32 = num_datasets;
  memcpy(buf_ptr, &num_datasets_si32, sizeof(si32));
  BE_from_array_32(buf_ptr, sizeof(si32));
  buf_ptr += sizeof(si32);
  
  // data times
  for (int i = 0; i < num_datasets; i++)
  {
    memcpy(buf_ptr, &data_times[i], sizeof(MDV_dataset_time_t));
    MDV_dataset_time_to_BE((MDV_dataset_time_t *)buf_ptr);
    buf_ptr += sizeof(MDV_dataset_time_t);
  }
  
  // Send the reply to the client

  if (SKU_write_message(clientfd,
			reply,
			(char *)reply_buffer,
			reply_len) != 1)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _programName, routine_name);
      fprintf(stderr, "Error writing reply to client on port %ld\n",
	      _params->port);
    }
    
    ufree(reply_buffer);
    
    return(MDV_SERVER_WRITE_ERROR);
  }

  ufree(reply_buffer);
  
  return(MDV_SERVER_SUCCESS);
}


/*********************************************************************
 * _reapChildren() - Reap any outstanding child processes.
 */

void MdvServer::_reapChildren(void)
{
  while (waitpid((pid_t)-1,
		 (int *)NULL,
		 (int)(WNOHANG | WUNTRACED)) > 0)
  {
    _numChildren--;
  }
  
  return;
}
