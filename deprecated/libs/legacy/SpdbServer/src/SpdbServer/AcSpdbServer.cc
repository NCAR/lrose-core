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
 * AcSpdbServer.cc: AcSpdbServer object code.  This object serves out
 *                  data from an aircraft tracks SPDB database.  This
 *                  class is built on top of the SpdbServer base class.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1997
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
#include <rapformats/ac_data.h>
#include <rapformats/ac_posn.h>
#include <toolsa/KeyedList.h>
#include <symprod/spdb_products.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include <SpdbServer/AcSpdbServer.h>
using namespace std;


/*
 * Local typedefs
 */

typedef struct
{
  char callsign[AC_POSN_N_CALLSIGN];
} callsign_t;


/*
 * Global variables
 */


/*********************************************************************
 * Constructor
 */

AcSpdbServer::AcSpdbServer(const int port,
			   const char *database_product_label,
			   const int database_product_id,
			   const char *database_dir,
			   const char *prog_name,
			   const char *servmap_type,
			   const char *servmap_subtype,
			   const char *servmap_instance,
			   const int max_children,
			   AcSpdbServerTransform input_transform,
			   const si32 input_product_id,
			   AcSpdbServerTransform output_transform,
			   const si32 output_product_id,
			   const int before_secs,
			   const int after_secs,
			   const int wait_msecs,
			   const int realtime_avail_flag,
			   SpdbServerDebugLevel debug_level) :
SpdbServer(port,
	   database_product_label, database_product_id, database_dir,
	   prog_name,
	   servmap_type, servmap_subtype, servmap_instance,
	   max_children,
	   NULL, input_product_id,
	   NULL, output_product_id,
	   wait_msecs, realtime_avail_flag, debug_level)
{
  static char *routine_name = "Constructor";
  
  if (debug_level >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
    
  // Save the transform routines

  _acInputTransform = input_transform;
  _acOutputTransform = output_transform;
  
  // Store the product ids

  if (_acInputTransform == NULL)
    _inputProductId = database_product_id;
  else
    _inputProductId = input_product_id;

  if (_acOutputTransform == NULL)
    _outputProductId = database_product_id;
  else
    _outputProductId = output_product_id;
  
  // Save the time range for the tracks

  _beforeSecs = before_secs;
  _afterSecs = after_secs;
  
}


/*********************************************************************
 * Destructor
 */

AcSpdbServer::~AcSpdbServer(void)
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

}


/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * _processGetDataRequest() - process an SPDB_GET_DATA request from
 *                            a client.
 */

SpdbServerStatus
AcSpdbServer::_processGetDataRequest(int product_id,
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
  
  if (product_id != _outputProductId)
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
  si32 nchunks_transformed;
  spdb_chunk_ref_t *transformed_hdrs;
  
  if ((transformed_data = _transformOutputData(request_time,
					       data_type,
					       nchunks,
					       chunk_hdrs,
					       chunk_data,
					       &nchunks_transformed,
					       &transformed_hdrs))
      == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 nchunks_transformed,
						 transformed_hdrs,
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
AcSpdbServer::_processGetDataClosestRequest(const int request,
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
  
  if (product_id != _outputProductId)
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
	      "ERROR - %s::%s - No data from SPDB_fetch_closest - time_margin = %d, request_time = %s",
	      _className(), routine_name, time_margin, utimstr(request_time));
    }
    
    _sendSimpleReply(clientfd, SPDB_NO_DATA);
    
    return(SPDB_SERVER_SUCCESS);
  }
  
  // Transform the data, if necessary

  void *transformed_data;
  si32 nchunks_transformed;
  spdb_chunk_ref_t *transformed_hdrs;
  
  if ((transformed_data = _transformOutputData(request_time,
					       data_type,
					       nchunks,
					       chunk_hdrs,
					       chunk_data,
					       &nchunks_transformed,
					       &transformed_hdrs))
      == NULL)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
      fprintf(stderr,
	      "ERROR - %s::%s - Transformation failed.\n",
	      _className(), routine_name);
    
    return(SPDB_SERVER_DATA_ERROR);
  }
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 nchunks_transformed,
						 transformed_hdrs,
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
AcSpdbServer::_processGetDataIntervalRequest(int product_id,
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
  
  if (product_id != _outputProductId)
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
  si32 nchunks_transformed;
  spdb_chunk_ref_t *transformed_hdrs;
  
  if ((transformed_data = _transformOutputData((start_time + end_time) / 2,
					       data_type,
					       nchunks,
					       chunk_hdrs,
					       chunk_data,
					       &nchunks_transformed,
					       &transformed_hdrs))
      == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 nchunks_transformed,
						 transformed_hdrs,
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
AcSpdbServer::_processGetDataValidRequest(int product_id,
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
  
  if (product_id != _outputProductId)
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
      fprintf(stderr, "Error extracting data for search time %s\n",
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
  si32 nchunks_transformed;
  spdb_chunk_ref_t *transformed_hdrs;
  
  if ((transformed_data = _transformOutputData(search_time,
					       data_type,
					       nchunks,
					       chunk_hdrs,
					       chunk_data,
					       &nchunks_transformed,
					       &transformed_hdrs)) == NULL)
    return(SPDB_SERVER_DATA_ERROR);
  
  // Send the reply to the client

  SpdbServerStatus reply_status = _sendDataReply(clientfd,
						 nchunks_transformed,
						 transformed_hdrs,
						 transformed_data);
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Normal exit\n",
	    _className(), routine_name);
    
  return(reply_status);
}


/*********************************************************************
 * _transformOutputData() - apply the output transformation to the
 *                          aircraft track data, if there is one
 */

void *AcSpdbServer::_transformOutputData(time_t request_time,
					 int data_type,
					 si32 nchunks,
					 spdb_chunk_ref_t *chunk_hdrs,
					 void *chunk_data,
					 si32 *nchunks_ret,
					 spdb_chunk_ref_t **chunk_hdrs_ret)
{
  static char *routine_name = "_transformOutputData";
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ROUTINES)
    fprintf(stderr,
	    "%s::%s: Entering\n",
	    _className(), routine_name);
  
  static spdb_chunk_ref_t *chunk_hdrs_ret_buf = NULL;
  static int chunk_hdrs_ret_alloc = 0;
  
  si32 nchunks_full;
  spdb_chunk_ref_t *chunk_hdrs_full;
  void *chunk_data_full;
  
  si32 nchunks_before;
  static MEMbuf *chunk_hdrs_before = NULL;
  static MEMbuf *chunk_data_before = NULL;
  
  si32 nchunks_after;
  static MEMbuf *chunk_hdrs_after = NULL;
  static MEMbuf *chunk_data_after = NULL;
  
  int chunk;
  int output_chunk;
  
  void *transformed_data;
  
  // Loop through the chunk headers and make a list of chunk headers
  // that includes only a single chunk header for each aircraft in
  // the data.  The chunk header that is kept will be used as the
  // current aircraft position.

  KeyedList<callsign_t, spdb_chunk_ref_t> curr_ac;
  spdb_chunk_ref_t *curr_ac_rec;
  callsign_t key_node;
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
    fprintf(stderr, "\n\nProcessing %d chunks\n", nchunks);
  
  for (chunk = 0; chunk < nchunks; chunk++)
  {
    ac_posn_t ac_posn;
    ac_data_t ac_data;
    
    // Copy the chunk data

    if (_databaseProductId == SPDB_AC_DATA_ID)
    {
      memcpy(&ac_data, (char *) chunk_data + chunk_hdrs[chunk].offset,
	     sizeof(ac_data_t));
      ac_data_from_BE(&ac_data);
      
      ac_posn.lat = ac_data.lat;
      ac_posn.lon = ac_data.lon;
      ac_posn.alt = ac_data.alt;
      STRcopy(ac_posn.callsign, ac_data.callsign, AC_POSN_N_CALLSIGN);
    }
    else
    {
      memcpy(&ac_posn, (char *) chunk_data + chunk_hdrs[chunk].offset,
	     sizeof(ac_posn_t));
      BE_to_ac_posn(&ac_posn);
    }
    
    STRcopy(key_node.callsign, ac_posn.callsign, AC_POSN_N_CALLSIGN);
    curr_ac_rec = curr_ac.find(key_node);
    
    if (curr_ac_rec == NULL)
    {
      // Callsign not found in list, add aircraft

      curr_ac.add(key_node, chunk_hdrs[chunk]);
    }
    else if (abs((double) request_time - (double) curr_ac_rec->valid_time) >
	     abs((double) request_time - (double) chunk_hdrs[chunk].valid_time))
    {
      // This chunk closer to requested time, make this chunk
      // the current one for this aircraft.
      
      *curr_ac_rec = chunk_hdrs[chunk];
    }
    
  } /* endfor - chunk */
  
  // Now get all of the data for all of the tracks.  Since the
  // chunks are returned in chronological order, the first chunk
  // contains the earliest time in the initial list and the last
  // chunk contains the latest time.  We'll use these times to
  // get the data for the full tracks, then we'll throw away tracks
  // outside of the time range based on the "current position" time
  // as we compile the track for each aircraft.

  time_t first_chunk_time = chunk_hdrs[0].valid_time - _beforeSecs;
  time_t last_chunk_time = chunk_hdrs[nchunks-1].valid_time + _afterSecs;
  
  if (SPDB_fetch_interval(&_spdbHandle,
			  data_type,
			  first_chunk_time,
			  last_chunk_time,
			  &nchunks_full,
			  &chunk_hdrs_full,
			  &chunk_data_full) != 0)
  {
    if (_debugLevel >= SPDB_SERVER_DEBUG_ERRORS)
    {
      fprintf(stderr, "ERROR - %s::%s\n",
	      _className(), routine_name);
      fprintf(stderr, "Error extracting data for full time interval (%s - %s)\n",
	      utimstr(first_chunk_time),
	      utimstr(last_chunk_time));
    }
    
    nchunks_full = 0;
    chunk_hdrs_full = NULL;
    chunk_data_full = NULL;

  }
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
    fprintf(stderr, "   Got %d chunks in full data list\n", nchunks_full);
  
  // Swap the chunks to native format

  ac_posn_t *ac_posn_full = (ac_posn_t *)NULL;
  ac_data_t *ac_data_full = (ac_data_t *)NULL;
  
  if (_databaseProductId == SPDB_AC_DATA_ID)
  {
    ac_data_full = (ac_data_t *)chunk_data_full;
    
    for (chunk = 0; chunk < nchunks_full; chunk++)
    {
      ac_data_t *ac_data =
	(ac_data_t *)((char *) chunk_data_full + chunk_hdrs_full[chunk].offset);
      
      ac_data_from_BE(ac_data);
    }
  }
  else
  {
    ac_posn_full = (ac_posn_t *)chunk_data_full;

    for (chunk = 0; chunk < nchunks_full; chunk++)
      BE_to_ac_posn(&ac_posn_full[chunk]);
  }
  
  // Allocate space for the returned chunk headers

  *nchunks_ret = curr_ac.size();
  
  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
    fprintf(stderr, "   Processing %d different aircraft\n", *nchunks_ret);
  
  if (chunk_hdrs_ret_alloc < *nchunks_ret)
  {
    chunk_hdrs_ret_alloc = *nchunks_ret;
    
    if (chunk_hdrs_ret_buf == NULL)
      chunk_hdrs_ret_buf =
	(spdb_chunk_ref_t *)umalloc(chunk_hdrs_ret_alloc *
				    sizeof(spdb_chunk_ref_t));
    else
      chunk_hdrs_ret_buf =
	(spdb_chunk_ref_t *)urealloc(chunk_hdrs_ret_buf,
				     chunk_hdrs_ret_alloc *
				     sizeof(spdb_chunk_ref_t));
  }
  
  *chunk_hdrs_ret = chunk_hdrs_ret_buf;

  // Process each aircraft in the list

  output_chunk = 0;
  
  for (curr_ac_rec = curr_ac.first(&key_node);
       curr_ac_rec != NULL;
       curr_ac_rec = curr_ac.next(&key_node))
  {
    int current_chunk_num = -1;
    
    ac_posn_t current_chunk;
    
    // Initialize the chunk buffers

    if (chunk_hdrs_before == NULL)
    {
      chunk_hdrs_before = MEMbufCreate();
      chunk_data_before = MEMbufCreate();
      
      chunk_hdrs_after = MEMbufCreate();
      chunk_data_after = MEMbufCreate();
    }
    else
    {
      MEMbufReset(chunk_hdrs_before);
      MEMbufReset(chunk_data_before);
      
      MEMbufReset(chunk_hdrs_after);
      MEMbufReset(chunk_data_after);
    }
    
    nchunks_before = 0;
    nchunks_after = 0;
    
    // Pull the relevant chunks out of the full list

    for (chunk = 0; chunk < nchunks_full; chunk++)
    {
      char *current_callsign;
      ac_data_t *ac_data;
      
      if (_databaseProductId == SPDB_AC_DATA_ID)
      {
	ac_data =
	  (ac_data_t *)((char *) chunk_data_full + chunk_hdrs_full[chunk].offset);
	
	current_callsign = ac_data->callsign;
      }
      else
	current_callsign = ac_posn_full[chunk].callsign;
      
      if (STRequal_exact(key_node.callsign,
			 current_callsign))
      {
	if (chunk_hdrs_full[chunk].valid_time <
	    curr_ac_rec->valid_time - _beforeSecs)
	{
	  // This chunk is too old to include in the track -- skip it
	}
	else if (chunk_hdrs_full[chunk].valid_time >
		 curr_ac_rec->valid_time + _afterSecs)
	{
	  // This chunk is too young to include in the track -- skip it
	}
	else if (chunk_hdrs_full[chunk].valid_time <
		 curr_ac_rec->valid_time)
	{
	  // This chunk goes in the before list

	  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
	    fprintf(stderr, "Adding before position for callsign <%s>\n",
		    current_callsign);
	  
	  ac_posn_t ac_before;
	  
	  if (_databaseProductId == SPDB_AC_DATA_ID)
	  {
	    ac_before.lat = ac_data->lat;
	    ac_before.lon = ac_data->lon;
	    ac_before.alt = ac_data->alt;
	    STRcopy(ac_before.callsign, ac_data->callsign,
		    AC_POSN_N_CALLSIGN);
	  }
	  else
	    ac_before = ac_posn_full[chunk];
	  
	  MEMbufAdd(chunk_hdrs_before,
		    &chunk_hdrs_full[chunk],
		    sizeof(spdb_chunk_ref_t));
	  
	  MEMbufAdd(chunk_data_before,
		    &ac_before,
		    sizeof(ac_posn_t));
	  
	  nchunks_before++;
	}
	else if (chunk_hdrs_full[chunk].valid_time >
		 curr_ac_rec->valid_time)
	{
	  // This chunk goes in the after list

	  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
	    fprintf(stderr, "Adding after position for callsign <%s>\n",
		    current_callsign);
	  
	  ac_posn_t ac_after;
	  
	  if (_databaseProductId == SPDB_AC_DATA_ID)
	  {
	    ac_after.lat = ac_data->lat;
	    ac_after.lon = ac_data->lon;
	    ac_after.alt = ac_data->alt;
	    STRcopy(ac_after.callsign, ac_data->callsign,
		    AC_POSN_N_CALLSIGN);
	  }
	  else
	    ac_after = ac_posn_full[chunk];
	  
	  MEMbufAdd(chunk_hdrs_after,
		    &chunk_hdrs_full[chunk],
		    sizeof(spdb_chunk_ref_t));
	  
	  MEMbufAdd(chunk_data_after,
		    &ac_after,
		    sizeof(ac_posn_t));
	  
	  nchunks_after++;
	}
	else
	{
	  // This is the current chunk -- keep track of it for the
	  // transformation.

	  if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
	    fprintf(stderr, "Adding current position for callsign <%s>\n",
		    current_callsign);
	  
	  current_chunk_num = chunk;
	  
	  if (_databaseProductId == SPDB_AC_DATA_ID)
	  {
	    current_chunk.lat = ac_data->lat;
	    current_chunk.lon = ac_data->lon;
	    current_chunk.alt = ac_data->alt;
	    STRcopy(current_chunk.callsign, ac_data->callsign,
		    AC_POSN_N_CALLSIGN);
	  }
	  else
	    current_chunk = ac_posn_full[chunk];
	}
	
      }
      
    } /* endfor - chunk */
    
    // Swap the data to back to BE format before adding it to the buffer
    // since we always send data in BE.
      
    ac_posn_t *ac_list;
    int ac_list_len;
    
    ac_list = (ac_posn_t *)MEMbufPtr(chunk_data_before);
    ac_list_len = MEMbufLen(chunk_data_before);
      
    for (int posn = 0; posn < ac_list_len / (int)sizeof(ac_posn_t); posn++)
      BE_from_ac_posn(&ac_list[posn]);
    
    BE_from_ac_posn(&current_chunk);
      
    ac_list = (ac_posn_t *)MEMbufPtr(chunk_data_after);
    ac_list_len = MEMbufLen(chunk_data_after);
      
    for (int posn = 0; posn < ac_list_len / (int)sizeof(ac_posn_t); posn++)
      BE_from_ac_posn(&ac_list[posn]);
      
    if (_acOutputTransform == NULL)
    {
      // no transform - sending output as it came in

      MEMbufAdd(_transformBuf,
		MEMbufPtr(chunk_data_before),
		MEMbufLen(chunk_data_before));
      
      MEMbufAdd(_transformBuf,
		&current_chunk,
		sizeof(ac_posn_t));
      
      MEMbufAdd(_transformBuf,
		MEMbufPtr(chunk_data_after),
		MEMbufLen(chunk_data_after));
      
      // Update the chunk header.  All of the positions for the
      // single aircraft are included in the chunk.

      chunk_hdrs_ret_buf[output_chunk] = chunk_hdrs_full[current_chunk_num];
      chunk_hdrs_ret_buf[output_chunk].len =
	MEMbufLen(chunk_data_before) + sizeof(ac_posn_t) +
	  MEMbufLen(chunk_data_after);

      if (output_chunk == 0)
	chunk_hdrs_ret_buf[output_chunk].offset = 0;
      else
	chunk_hdrs_ret_buf[output_chunk].offset =
	  chunk_hdrs_ret_buf[output_chunk-1].offset +
	    chunk_hdrs_ret_buf[output_chunk-1].len;
      
      output_chunk++;
    }
    else
    {

      // sending transformed data

      int transformed_len;
      void *transformed_chunk;

      if (_debugLevel >= SPDB_SERVER_DEBUG_ALL)
	fprintf(stderr, "   Transforming data: %d before, %d after\n",
		nchunks_before, nchunks_after);
      
      if ((transformed_chunk =
	   _acOutputTransform(&chunk_hdrs_full[current_chunk_num],
			      nchunks_before,
			      nchunks_after,
			      MEMbufPtr(chunk_data_before),
			      &current_chunk,
			      MEMbufPtr(chunk_data_after),
			      &transformed_len)) != NULL)
      {
	MEMbufAdd(_transformBuf, transformed_chunk, transformed_len);
      
	// Update the chunk header

	chunk_hdrs_ret_buf[output_chunk] = chunk_hdrs_full[current_chunk_num];
	chunk_hdrs_ret_buf[output_chunk].len = transformed_len;
      
	if (output_chunk == 0)
	  chunk_hdrs_ret_buf[output_chunk].offset = 0;
	else
	  chunk_hdrs_ret_buf[output_chunk].offset =
	    chunk_hdrs_ret_buf[output_chunk-1].offset +
	    chunk_hdrs_ret_buf[output_chunk-1].len;
      
	output_chunk++;
      }
    } /* endelse - transform data */

  } /* endfor - curr_ac_rec */
  
  transformed_data = MEMbufPtr(_transformBuf);
  *nchunks_ret = output_chunk;
  
  return(transformed_data);
  
}
