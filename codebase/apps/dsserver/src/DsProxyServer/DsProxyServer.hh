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
// DsProxyServer.hh
//
// Proxy Server Object
//
// Paddy McCarthy, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1999
//
///////////////////////////////////////////////////////////////

#ifndef DsProxyServerINCLUDED
#define DsProxyServerINCLUDED

#include <dsserver/DsProcessServer.hh>
#include <string>
using namespace std;

class DsURL;

class DsProxyServer : public DsProcessServer {

public:

  DsProxyServer(string executableName,
                const Params &params);
  
  virtual ~DsProxyServer();
  
protected:

  virtual int handleDataCommand(Socket * socket,
                                const void * data, ssize_t dataSize);
  
private:
  
  // Private methods with no bodies. DO NOT USE!

  DsProxyServer();
  DsProxyServer(const DsProxyServer & orig);
  DsProxyServer & operator = (const DsProxyServer & other);
  
  const Params &_params;

  void _sendErrorReply(Socket *clientSock,
                       const string &errMsg,
                       DsServerMsg::msgErr errType);

  string _getUrlStr(const DsServerMsg &msg);

};

#endif
