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
// DmapServer.hh
//
// DataMapper Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////


#include <dsserver/DsThreadedServer.hh>
#include "Params.hh"
#include "InfoStore.hh"
using namespace std;

class DmapServer : public DsThreadedServer {

public:
    
  // Constructor:                       
  //   o Registers with procmap
  //   o Opens socket on specified port              

  DmapServer(string executableName,
	     string instanceName,                                         
	     int port,
	     int maxQuiescentSecs,                                  
	     int maxClients,
	     bool isDebug,
	     bool isVerbose,
	     bool noThreads,
	     const Params &params,
	     const vector<string> &dataTypes);

  // destructor

  virtual ~DmapServer();
  
protected:

  // Handle a client's request for data. The passed
  //   message is a decoded DsResourceMsg that is
  //   *not* a server command.
  //
  // Look at the URL and determine if a port is needed.
  // Spawn the necessary server.

  virtual int handleDataCommand(Socket *socket,
				const void *data,
				ssize_t dataSize);
  
  // override the timeout and post handler methods from parent class

  virtual bool timeoutMethod();
  virtual bool postHandlerMethod();

private:

  bool _useThreads;
  const Params &_params;
  time_t _lastSaveTime;
  time_t _lastPurgeTime;

  // info store

  InfoStore _store;
  
  // Thread mutex to mutually protect processing of the store

  pthread_mutex_t _storeMutex;
  void _lockStore();
  void _unlockStore();

  // functions

  void *_relayRequest(DmapMessage &msg);

  // Private methods with no bodies. DO NOT USE!

  DmapServer();
  DmapServer(const DmapServer & orig);
  DmapServer & operator = (const DmapServer & other);

};

