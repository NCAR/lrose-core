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
////////////////////////////////////////////////////////////////////////////////
//
// DsFmqServer.hh
// DsFmqServer class
//
// Mike Dixon, RAL, NCAR, Boulder, CO, USA
// Jan 2009
//
////////////////////////////////////////////////////////////////////////////////
#ifndef _DS_FMQ_SERVER_INC_
#define _DS_FMQ_SERVER_INC_

#include <dsserver/DsProcessServer.hh>
#include <Fmq/Fmq.hh>
#include <Fmq/DsFmqMsg.hh>
#include "Params.hh"
using namespace std;

class DsFmqServer : public DsProcessServer
{
public:
  DsFmqServer(const string& progName,
	      const Params &params);
  
  virtual ~DsFmqServer() {};
  
protected:
  
  string _progName;
  const Params &_params;
  
  virtual int handleDataCommand(Socket* socket, 
				const void* data, 
				ssize_t dataSize);

  // local Fmq object, for file operations

  Fmq _fmq;

  // local copy of socket

  Socket *_socket;

  // local set methods

  int _setCompressionMethod(DsFmqMsg &msg);
  int _setBlockingWrite(DsFmqMsg &msg);
  int _setSingleWriter(DsFmqMsg &msg);
  int _setRegisterWithDmap(DsFmqMsg &msg);
  int _performInit(DsFmqMsg &msg);
  int _performSeek(DsFmqMsg &msg);
  int _performSeekToId(DsFmqMsg &msg);
  int _performRead(DsFmqMsg &msg);
  int _performWrite(DsFmqMsg &msg);
  int _performClose(DsFmqMsg &msg);
  
  // communications

  int _sendReply(int msgType, int status,
		 const string &errorStr);

};

#endif
