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
 * AcSpdbServer.h : header file for AcSpdbServer object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 1997
 *
 * Nancy Rehak
 *
 ************************************************************************/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/03 19:19:08 $
 *   $Id: AcSpdbServer.h,v 1.4 2016/03/03 19:19:08 dixon Exp $
 *   $Revision: 1.4 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

#ifndef AcSpdbServer_H
#define AcSpdbServer_H

/*
 **************************** includes ***********************************
 */

#include <stdio.h>
#include <sys/time.h>

#include <symprod/spdb.h>
#include <symprod/spdb_client.h>

#include <toolsa/membuf.h>

#include <SpdbServer/SpdbServer.h>

/*
 ******************************** defines ********************************
 */

/*
 ******************************** types **********************************
 */

//
// Define the interface to the transform routines.  The transform routines
// should transform the input_data, of length input_len, with the given chunk
// header information into the returned data, of length output_len.  This
// routine should allocate new space for the transformed data which will be
// freed by the calling routine (it's freed using "ufree", so allocations
// should be done using "umalloc").  Note that the input_data and the returned
// data should be in big-endian format.  The SpdbServer object does not know
// anything about byte-swapping chunk data.
//

typedef void *(*AcSpdbServerTransform)(spdb_chunk_ref_t *chunk_hdr_curr,
                                       int nchunks_before,
                                       int nchunks_after,
                                       void *chunk_data_before,
                                       void *chunk_data_curr,
                                       void *chunk_data_after,
				       int *output_len);


/*
 ************************* class definitions *****************************
 */

class AcSpdbServer : public SpdbServer
{
  public :
    
    // Constructor

    AcSpdbServer(const int port,
		 const char *database_product_label,
		 const int database_product_id,
		 const char *database_dir,
		 const char *prog_name,
		 const char *servmap_type,
		 const char *servmap_subtype,
		 const char *servmap_instance,
		 const int max_children = 64,
		 AcSpdbServerTransform input_transform = NULL,
		 const si32 input_product_id = -1,
		 AcSpdbServerTransform output_transform = NULL,
		 const si32 output_product_id = -1,
		 const int before_secs = 0,
		 const int after_secs = 0,
		 const int wait_msecs = 100,
		 const int realtime_avail_flag = TRUE,
		 SpdbServerDebugLevel debug_level = SPDB_SERVER_DEBUG_ERRORS);

    // Destructor

    virtual ~AcSpdbServer(void);
  
  protected :

    AcSpdbServerTransform _acInputTransform;
    AcSpdbServerTransform _acOutputTransform;

    int _beforeSecs;
    int _afterSecs;
  
    // Private member functions

    SpdbServerStatus _processGetDataRequest(int product_id,
                                            char *request_info,
					    int request_info_len,
					    int clientfd);
  
    SpdbServerStatus _processGetDataClosestRequest(const int request,
                                                   int product_id,
                                                   char *request_info,
						   int request_info_len,
						   int clientfd);
  
    SpdbServerStatus _processGetDataIntervalRequest(int product_id,
                                                    char *request_info,
						    int request_info_len,
						    int clientfd);
  
    SpdbServerStatus _processGetDataValidRequest(int product_id,
                                                 char *request_info,
						 int request_info_len,
						 int clientfd);
  
    void *_transformOutputData(time_t request_time,
			       int data_type,
			       si32 nchunks,
			       spdb_chunk_ref_t *chunk_hdrs,
			       void *chunk_data,
			       si32 *nchunks_ret,
			       spdb_chunk_ref_t **chunk_hdrs_ret);
  
  
    virtual const char *const _className(void)
    {
      return("AcSpdbServer");
    }
  
};


#endif
