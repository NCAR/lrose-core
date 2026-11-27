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
// DsFileServer.hh
//
// FileServerobject
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////


#ifndef _DsFileServer_HH
#define _DsFileServer_HH

#include <dsserver/DsServer.hh>
#include <string>
using namespace std;

class Params;
class DsFileIoMsg;
class DsFileIo;
class Socket;

class DsFileServer : public DsServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  //   
  DsFileServer(string executableName,
	       string instanceName,                                         
	       int port,
	       int maxQuiescentSecs,                                  
	       int maxClients,
	       bool forkClientHandlers,
	       Params *params);

  // destructor

  virtual ~DsFileServer();
  
protected:

  // Handle a client's request for data. The passed
  //   message is a decoded DsResourceMsg that is
  //   *not* a server command.
  //
  // Look at the URL and determine if a port is needed.
  // Spawn the necessary server.
  // 
  virtual int handleDataCommand(Socket * socket,
				const void * data, ssize_t dataSize);
    
  bool timeoutMethod();

private:

  // Private methods with no bodies. DO NOT USE!
  // 
  DsFileServer();
  DsFileServer(const DsFileServer & orig);
  DsFileServer & operator = (const DsFileServer & other);

  Params *_params;

  int _open(DsFileIoMsg *msg,
	    DsFileIo *fileIo,
	    Socket * socket);

  int _serve(DsFileIoMsg *msg,
	     DsFileIo *fileIo,
	     Socket * socket);
};

#endif
