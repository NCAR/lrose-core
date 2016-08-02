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
/////////////////////////////////////////////////////////////
// MdvServer.hh
//
// MdvServer object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// August 1998
//
///////////////////////////////////////////////////////////////

#ifndef MdvServer_HH
#define MdvServer_HH

#include <cstdio>

#include <toolsa/os_config.h>
#include <didss/DsInputPath.hh>
#include <mdv/mdv_client.h>
#include <mdv/mdv_handle.h>
#include <tdrp/tdrp.h>

#include "Args.hh"
#include "Params.hh"
using namespace std;


/*
 ******************************** defines ********************************
 */

// Number of seconds between calls to SMU_auto_last_data calls.  These
// calls are needed for servers that are serving out data from a database
// that is being fed from another process .

#define MDV_SERVER_DATA_TIME_UPDATE     15

// Number of milli-seconds to wait for a client connect before moving on
// to other duties.

#define MDV_SERVER_WAIT_MSECS         1000

/*
 ******************************** types **********************************
 */

typedef enum
{
  MDV_SERVER_SUCCESS,

  MDV_SERVER_INVALID_REQUEST,
  MDV_SERVER_WRITE_ERROR,
  MDV_SERVER_DATA_ERROR,
  MDV_SERVER_MALLOC_ERROR,
  MDV_SERVER_REQUEST_ERROR,

  MDV_SERVER_FAILURE
} MdvServerStatus;

/*
 ************************* class definitions *****************************
 */

class MdvServer
{
  
public:

  // constructor

  MdvServer(int argc, char **argv);

  // destructor
  
  ~MdvServer();

  // run 

  void run();

  // data members

  int okay;
  int done;

protected:
  
private:

  char *_programName;
  Args *_args;

  char *_dataDirectory;
  
  Params *_params;
  char *_paramsPath;
  
  int _serverfd;
  int _clientfd;
  
  int _numChildren;
  int _maxChildren;
  
  DsInputPath *_inputPath;
  
  // Private methods

  void _handleClient();
  
  MdvServerStatus _processRequest(const int request,
				  char *request_info,
				  int request_info_len,
				  int clientfd);
  
  MdvServerStatus _processGetRequest(const int request,
				     char *request_info,
				     int request_info_len,
				     int clientfd);
  
  MdvServerStatus _processInfoDatasetTimesRequest(MDV_dataset_time_request_t *request_info,
						  int request_info_len,
						  int clientfd);
  
  MdvServerStatus _sendSimpleReply(int clientfd,
				   int reply);
  
  MdvServerStatus _sendDataReply(int clientfd,
				 MDV_handle_t *mdv_handle);
  
  MdvServerStatus _sendDatasetTimesReply(int clientfd,
					 MDV_dataset_time_t *data_times,
					 int num_datasets);
  
  void _reapChildren(void);
  
};

#endif
