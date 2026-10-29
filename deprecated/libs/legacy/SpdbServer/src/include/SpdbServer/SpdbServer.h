/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/* ** Copyright UCAR (c) 1990 - 2016                                         */
/* ** University Corporation for Atmospheric Research (UCAR)                 */
/* ** National Center for Atmospheric Research (NCAR)                        */
/* ** Boulder, Colorado, USA                                                 */
/* ** BSD licence applies - redistribution and use in source and binary      */
/* ** forms, with or without modification, are permitted provided that       */
/* ** the following conditions are met:                                      */
/* ** 1) If the software is modified to produce derivative works,            */
/* ** such modified software should be clearly marked, so as not             */
/* ** to confuse it with the version available from UCAR.                    */
/* ** 2) Redistributions of source code must retain the above copyright      */
/* ** notice, this list of conditions and the following disclaimer.          */
/* ** 3) Redistributions in binary form must reproduce the above copyright   */
/* ** notice, this list of conditions and the following disclaimer in the    */
/* ** documentation and/or other materials provided with the distribution.   */
/* ** 4) Neither the name of UCAR nor the names of its contributors,         */
/* ** if any, may be used to endorse or promote products derived from        */
/* ** this software without specific prior written permission.               */
/* ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  */
/* ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      */
/* ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    */
/* *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* */
/************************************************************************
 * SpdbServer.h : header file for SpdbServer object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 1996
 *
 * Nancy Rehak
 *
 ************************************************************************/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:19:08 $
 *   $Id: SpdbServer.h,v 1.14 2016/03/03 19:19:08 dixon Exp $
 *   $Revision: 1.14 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

#ifndef SpdbServer_H
#define SpdbServer_H

/*
 **************************** includes ***********************************
 */

#include <stdio.h>

#include <symprod/spdb.h>
#include <symprod/spdb_client.h>

#include <toolsa/membuf.h>

/*
 ******************************** defines ********************************
 */

// Number of seconds between calls to SMU_auto_last_data calls.  These
// calls are needed for servers that are serving out data from a database
// that is being fed from another process (for example, servers serving
// out data to displays in SYMPROD format).  We don't want to do these
// calls too often because they cause database reads to get the data
// times to be sent to the server mapper.

#define SPDB_SERVER_DATA_TIME_UPDATE     15

/*
 ******************************** types **********************************
 */

typedef enum
{
  SPDB_SERVER_DEBUG_OFF,
  SPDB_SERVER_DEBUG_ERRORS,
  SPDB_SERVER_DEBUG_MSGS,
  SPDB_SERVER_DEBUG_ROUTINES,
  SPDB_SERVER_DEBUG_ALL
} SpdbServerDebugLevel;

typedef enum
{
  SPDB_SERVER_SUCCESS,

  SPDB_SERVER_INVALID_REQUEST,
  SPDB_SERVER_WRITE_ERROR,
  SPDB_SERVER_DATA_ERROR,
  SPDB_SERVER_MALLOC_ERROR,
  SPDB_SERVER_REQUEST_ERROR,

  SPDB_SERVER_FAILURE
} SpdbServerStatus;

//
// Define the interface to the transform routines.  The transform routines
// should transform the input_data, of length input_len, with the given chunk
// header information into the returned data, of length output_len.  This
// routine should return a pointer to a static buffer that is maintained
// locally.  This pointer is NOT freed by the calling routine.  Note that
// the input_data and the returned data should be in big-endian format.
// The SpdbServer object does not know anything about byte-swapping chunk data.
//

typedef void *(*SpdbServerTransform)(spdb_chunk_ref_t *chunk_hdr,
				     void *input_data,
				     int input_len,
				     int *output_len);


/*
 ************************* class definitions *****************************
 */

class SpdbServer
{
  public :
    
    // Constructor

    SpdbServer(const int port,
	       const char *database_product_label,
	       const int database_product_id,
	       const char *database_dir,
	       const char *prog_name,
	       const char *servmap_type,
	       const char *servmap_subtype,
	       const char *servmap_instance,
	       const int max_children = 64,
	       SpdbServerTransform input_transform = NULL,
	       const si32 input_product_id = -1,
	       SpdbServerTransform output_transform = NULL,
	       const si32 output_product_id = -1,
	       const int wait_msecs = 100,
	       const int realtime_avail_flag = TRUE,
	       SpdbServerDebugLevel debug_level = SPDB_SERVER_DEBUG_ERRORS,
	       const int latest_only_flag = FALSE);

    // Destructor

    virtual ~SpdbServer(void);
  
    // Main operational loop.  This routine just does all of the work
    // of the server and never returns to the caller.

    void operate(void);
  
    // Close all open sockets when a SIGPIPE signal is received.  The
    // application should call this routine in it's SIGPIPE handler.

    void handleSigpipe(void);

  protected :

    int _port;
    int _serverfd;

    int _clientfd;
  
    char *_programName;
    char *_servmapType; 
    char *_servmapSubtype;
    char *_servmapInstance;
    int _waitMsecs;

    int _numChildren;
    int _maxChildren;

    SpdbServerTransform _inputTransform;
    SpdbServerTransform _outputTransform;
  
    int _realtimeAvailFlag;
    int _latestOnlyFlag;

    char *_databaseProductLabel;
    si32 _databaseProductId;
    char *_databaseDirName;
    spdb_handle_t _spdbHandle;
  
    si32 _inputProductId;
    si32 _outputProductId;

    SpdbServerDebugLevel _debugLevel;
  
    MEMbuf *_transformBuf;
    MEMbuf *_transformHdrs;
    char _errorMsg[1024];
  
    virtual const char *const _className(void)
    {
      return("SpdbServer");
    }
  
    void _handleClient();

    SpdbServerStatus _processRequest(const int request,
                                     const int product_id,
				     char *request_info,
				     int request_info_len,
				     int clientfd);
  
    virtual SpdbServerStatus _processGetDataRequest(int product_id,
                                                    char *request_info,
					            int request_info_len,
					            int clientfd);
  
    virtual SpdbServerStatus _processGetDataClosestRequest(const int request,
							   int product_id,
                                                           char *request_info,
						           int request_info_len,
						           int clientfd);
  
    virtual SpdbServerStatus _processGetDataIntervalRequest(int product_id,
                                                            char *request_info,
			                                    int request_info_len,
						            int clientfd);
  
    virtual SpdbServerStatus _processGetDataValidRequest(int product_id,
                                                         char *request_info,
						         int request_info_len,
						         int clientfd);
  
    virtual void *_transformOutputData(si32 nchunks,
                                       spdb_chunk_ref_t *chunk_hdrs,
                                       void *chunk_data,
                                       si32 *trans_nchunks,
                                       spdb_chunk_ref_t **trans_chunk_hdrs);

    SpdbServerStatus _processPutDataRequest(int product_id,
                                            char *request_info,
					    int request_info_len,
					    const int request_type,
					    int clientfd);
  
    SpdbServerStatus _sendSimpleReply(const int clientfd,
				      const spdb_reply_t reply);
  
    SpdbServerStatus _sendDataReply(int clientfd,
				    si32 nchunks,
				    spdb_chunk_ref_t *chunk_hdrs,
				    void *chunk_data);
  
    void _reapChildren(void);
};


#endif
