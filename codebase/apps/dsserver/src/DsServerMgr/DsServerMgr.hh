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
//
// $Id: DsServerMgr.hh,v 1.23 2016/03/04 02:29:41 dixon Exp $
//

#ifndef DsServerMgrINCLUDED
#define DsServerMgrINCLUDED

#ifndef DsThreadServerINCLUDED
# include <dsserver/DsThreadedServer.hh>
#endif

#ifndef _DS_SERVER_MSG_INC_
# include <dsserver/DsServerMsg.hh>
#endif

#ifndef DSURLINCLUDED
# include <didss/DsURL.hh>
#endif

#include <set>
#include <string>
#include <pthread.h>
using namespace std;

/////////////////////////////////////////////////////////
//
// DsServerMgr -- Server for Managing Other DIDSS Servers
//
//                Paddy McCarthy - Jan 1999
//                Mike Dixon - Feb 2001
//
// Design Notes:
// -------------
//
// Derives from DsThreadedServer.
// See dsserver/DsThreadedServer.hh for details.
//
// The DsServerMgr is responsible for reading client requests (URLs), and
//   finding a server to provide data for the request. If a server is not
//   already running to meet a request, the DsServerMgr starts a new server
//   on a well-known port. Then it replies to the client with a 
//   completed URL, containing the port where the appropriate server is 
//   listening.
// 
// The DsServerMgr never exits because of quiescence. The maxQuiescentSecs
//   passed to the constructor is used for spawning child servers -- it 
//   determines how long they wait quietly before exiting.

class DsServerMgr : public DsThreadedServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens listening socket on specified port              

  DsServerMgr(string executableName,
	      string instanceName,                                         
	      int port,
	      int childMaxQuiescentSecs, // Used for Sub-Servers only.
	      int maxClients,
	      bool isDebug,                                           
	      bool isVerbose,
	      bool isSecure,
              bool mdvReadOnly,
              bool spdbReadOnly);

  // Destructor.

  virtual ~DsServerMgr();
    
protected:

  // is server secure

  
  // Quiescent time used for new Sub-Servers. The amount of time
  //   they are quiet before exiting. If set to -1, they never exit.
  // 

  int _childMaxQuiescentSecs;

  // timeouts

  int _pingTimeoutMsecs;
  int _commTimeoutMsecs;
  
  // read-only mode for DsMdvServer and DsSpdbServer

  bool _mdvReadOnly;
  bool _spdbReadOnly;

  // Set for keeping track of requests in progress.
  // This prevents a server from being started simultaneously by
  // separate requests.

  set<string> _pending;

  // Mutex for pending set

  pthread_mutex_t _pendingMutex;

  // Handle a client's request for data. The passed
  //   message is a decoded DsServerMsg that is
  //   *not* a server command.
  // 
  // Look at the URL and determine what datatype is being served.
  //   Spawn the necessary server.
  // 
  // Threads: Called by Worker thread.
  //
  // Returns  0 on success
  //         -1 on error. Note that this causes server to exit.
  // 
  virtual int handleDataCommand(Socket * socket,
				const void * data, ssize_t dataSize);
    
  // Handle a command from a client that requests information           
  //   directly from and about the server.
  // 
  // Handles the following commands:
  //   GET_NUM_SERVERS,    Returns integer.
  //   GET_SERVER_INFO,    Returns int and formatted string, list of servers.
  //   GET_FAILURE_INFO,   Returns int and formatted string, failure list.
  //   GET_DENIED_SERVICES Returns int and formatted string, executable list.
  //   SHUTDOWN            Returns an error string and doesn't exit.
  // 
  // Threads: Called by Worker thread.
  // 
  // Returns  0 on success
  //         -1 on error. Note that this causes server to exit.
  // 
  virtual int handleServerCommand(Socket * socket,
				  const void * data, ssize_t dataSize);

  // Perform any actions needed on timeout when waiting for clients.
  //   Checks the spawn queue, and spawns any needed servers.
  // 
  // Threads: Called by Boss thread.
  // 
  // Returns: true always, so that the server will continue to wait for
  //            clients.
  // 
  virtual bool timeoutMethod();

  // Perform any actions needed after handling a client before waiting again.
  //   Checks the spawn queue, and spawns any needed servers.
  // 
  // Threads: Called by Boss thread.
  // 
  // Returns: true always, so that the server will continue to wait for
  //            clients.
  // 
  virtual bool postHandlerMethod();

  // Perform any actions needed when the quiescence-checker exits.
  //   For this server, it is an error for the quiescence-checker to
  //   be running, much less exiting.
  // 
  // Threads: Called by boss thread.
  // 
  // Returns: false always, so that the server will not complete the exit.
  // 
  virtual bool exitMethod();

  // Reap children.
  // 
  // Threads: Called by Boss thread.
  // 
  void reapChildren();

private:
  
  int handleURL(Socket * socket, DsURL & url,
		DsServerMsg::msgErr &errCode,string & errStr);

  int pingServer(const string & executableName, int port);
  
  int waitForServer(const string & executableName, int port,
		    int wait_secs);

  int spawnServer(const string &executable_name, int port,
		  Socket *socket, string & errStr);

  void addPending(const string &executable_name);
  void removePending(const string &executable_name);
  bool isPending(const string &executable_name);

  int sendReply(Socket * socket,
		DsServerMsg::msgErr errCode, const string & errStr);

  int sendReply(Socket * socket, const DsURL & url,
		DsServerMsg::msgErr errCode, const string & errStr);

  // Private methods with no bodies. DO NOT USE!
  // 

  DsServerMgr();
  DsServerMgr(const DsServerMgr & orig);
  DsServerMgr & operator = (const DsServerMgr & other);

};

#endif
