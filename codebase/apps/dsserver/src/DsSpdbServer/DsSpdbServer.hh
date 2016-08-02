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
//////////////////////////////////////////////////////////////////////////////
//
//  DsSpdbServer class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _DS_SPDB_SERVER_HH_
#define _DS_SPDB_SERVER_HH_

#include <string>
#include <dsserver/DsProcessServer.hh>
#include <Spdb/DsSpdbMsg.hh>
#include <Spdb/DsSpdb.hh>
#include <list>
using namespace std;

// Forward class declarations

class Sounding;
class Socket;
class Params;
class PutArgs;

class DsSpdbServer : public DsProcessServer
{

public:
  
  DsSpdbServer(const string& executableName,
               const Params *initialParams);

  ~DsSpdbServer();
  
protected:

  // Override handleDataCommand from DsProcessServer class.
  // Always return true, so that parent will not exit
  
  virtual int handleDataCommand(Socket *socket,
				const void *data, ssize_t dataSize);
  
  // Override base class on timeout and post handlers
  // always return true - i.e. never exit

  virtual bool timeoutMethod();
  virtual bool postHandlerMethod();

private:

  // initial params set in parent - do not modify in threads
  // local params are used in threads
  
  const Params *_initialParams;
  Params *_params;
  
  // keeping track of active puts in child processes
  
  list<PutArgs *> _putList;
  int _nPutChildren;
  
  // methods
  
  int _loadLocalParams(const string &paramFile, Params *params);
  
  int _handleGetSingle(const DsSpdbMsg &inMsg, 
                       Socket &socket,
                       DsSpdbMsg::info_t &getInfo,
                       bool sendReply,
                       DsSpdbMsg &replyMsg);
  
  int _handleGetWithFailover(const DsSpdbMsg &inMsg, 
                             Socket &socket);
  
  int _handleGetWithSearch(const DsSpdbMsg &inMsg, 
                           Socket &socket);
  
  void _selectGetData(const int n_chunks_in,
                      const Spdb::chunk_ref_t *chunk_refs_in,
                      const Spdb::aux_ref_t *aux_refs_in,
                      const void *chunk_data_in,
                      int &n_chunks_out,
                      MemBuf &refBufOut,
                      MemBuf &auxBufOut,
                      MemBuf &dataBufOut);
  
  int _handlePut(const DsSpdbMsg &inMsg, 
                 Socket &socket );
  
  int _putForward(const DsSpdbMsg &inMsg, 
                  string &errStr);
  
  int _forwardLocal(const DsURL &url,
                    const DsSpdbMsg &inMsg);

  int _writeMsgBufToFile(DsURL url,
                         const DsSpdbMsg &inMsg);

  int _queueForwardingToServer(const DsURL &url);
  
  void _performQueuedPuts(const DsSpdbMsg &inMsg);

  void _doPutInChild(PutArgs *putArgs,
                     DsSpdbMsg &putMsg);

  int _communicate(const DsSpdbMsg &msg, DsURL url,
                   DsSpdbMsg &replyMsg, string &errStr);
  
  void _purgeCompletedPuts();
  
  void _logRemoteForwarding(const string &label,
                            const string &modeStr,
                            const DsURL &url,
                            const PutArgs &putArgs,
                            const DsSpdbMsg &putMsg);
    
};

#endif
