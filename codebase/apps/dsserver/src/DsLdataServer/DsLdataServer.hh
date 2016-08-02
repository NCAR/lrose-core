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
// DsLdataServer.hh
//
// DsLdataServer Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// May 2009
//
///////////////////////////////////////////////////////////////


#include <dsserver/DsProcessServer.hh>
#include <dsserver/DsLdataMsg.hh>
#include <didss/LdataInfo.hh>
#include <string>
#include <map>
#include "Params.hh"
using namespace std;

class DsLdataServer : public DsProcessServer {

public:
  
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              
  //   
  DsLdataServer(string executableName,
		string instanceName,                                         
		int port,
		int maxQuiescentSecs,                                  
		int maxClients,
		bool isDebug,
		bool isVerbose,
		bool noThreads,
                bool isSecure,
                bool isReadOnly,
		const Params &params);

  // destructor

  virtual ~DsLdataServer();
  
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
  
private:

  // Private methods with no bodies. DO NOT USE!

  DsLdataServer();
  DsLdataServer(const DsLdataServer & orig);
  DsLdataServer & operator = (const DsLdataServer & other);
  
  bool _useThreads;
  const Params &_params;
  LdataInfo _ldata;

  int _handleOpen(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetDisplacedDirPath(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetLdataFileName(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetUseXml(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetUseAscii(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetSaveLatestReadInfo(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetUseFmq(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetFmqNSlots(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleSetReadFmqFromStart(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleRead(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleWrite(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  int _handleClose(const DsLdataMsg &inMsg, DsLdataMsg &outMsg);
  
};

