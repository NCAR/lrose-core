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

#ifndef DS_THREADED_CLIENT_HH
#define DS_THREADED_CLIENT_HH

////////////////////////////////////////////////////////////////////
// DsThreadedClient.hh
//
// Object for clients to use to contact servers, for threaded apps.
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////

#include <dsserver/DsClient.hh>
using namespace std;

class DsThreadedClient : public DsClient {

public:

  // constructor

  DsThreadedClient();

  // destructor

  virtual ~DsThreadedClient();
  
  // Communicate with server, automatically forwarding
  // through proxy or tunnel as required
  //
  // This assumes a one-shot message sequence,
  // i.e. a single request to the server followed
  // by a single reply.
  //
  // Forwarding via a proxy and/or tunnel is handled.
  //
  // After success, retrieve the returned message
  // using getReplyBuf() and getReplyLen().
  //
  // Returns 0 on success, -1 on failure.
  
  int communicateAutoFwd(DsURL &url,
			 int msgType,
			 const void *msgBuf,
			 ssize_t msgLen,
			 ssize_t &nbytesWriteExpected,
			 ssize_t &nbytesWriteDone,
			 ssize_t &nbytesReadExpected,
			 ssize_t &nbytesReadDone);
  
protected:
  
  int _communicateNoFwd(DsURL &url,
			int msgType,
			const void *msgBuf,
			ssize_t msgLen,
			ssize_t &nbytesWriteExpected,
			ssize_t &nbytesWriteDone,
			ssize_t &nbytesReadExpected,
			ssize_t &nbytesReadDone);

  int _communicateFwd(DsURL &url,
		      int msgType,
		      const void *msgBuf,
		      ssize_t msgLen,
		      ssize_t commTimeoutMsecs,
		      ssize_t &nbytesWriteExpected,
		      ssize_t &nbytesWriteDone,
		      ssize_t &nbytesReadExpected,
		      ssize_t &nbytesReadDone,
		      bool &tunnelFailed);
  
private:

};

#endif

