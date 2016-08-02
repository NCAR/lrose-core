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
// DmapServer.cc
//
// DataMapper Server object
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 1999
//
///////////////////////////////////////////////////////////////

#include <cerrno>
#include <toolsa/TaStr.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/TaArray.hh>
#include <dsserver/DmapAccess.hh>
#include <dsserver/DmapMessage.hh>
#include <sys/wait.h>
#include "DmapServer.hh"
using namespace std;

//////////////////////////////////////////
// constructor
//
// Inherits from DsServer

DmapServer::DmapServer(string executableName,
		       string instanceName,
		       int port,
		       int maxQuiescentSecs,
		       int maxClients,
		       bool isDebug,
		       bool isVerbose,
		       bool noThreads,
		       const Params &params,
		       const vector<string> &dataTypes)

  : DsThreadedServer(executableName,
		     instanceName,
		     port,
		     maxQuiescentSecs,
		     maxClients,
		     isDebug,
		     isVerbose),

    _useThreads(!noThreads),
    _params(params),
    _store(params, dataTypes)
  
{

  setNoThreadDebug(noThreads);
  if (_useThreads) {
    pthread_mutex_init(&_storeMutex, NULL);
  }
  _lastSaveTime = time(NULL);
  _lastPurgeTime = time(NULL);

}

//////////////
// destructor

DmapServer::~DmapServer()

{

}

///////////////////////////
// Handle incoming requests

int DmapServer::handleDataCommand(Socket *socket,
				  const void *data,
				  ssize_t dataSize)

{

  if (_isVerbose) {
    cerr << "Entering DmapServer::handleDataCommand()." << endl;
  }
  
  DmapMessage msg;
  if (msg.disassemble((void *) data, dataSize)) {
    cerr << "ERROR - DmapServer::handleDataCommand" << endl;
    cerr << "  Invalid DsMapMsg message" << endl;
    return -1;
  }

  if (_isVerbose) {
    cerr << "------------------------------------" << endl;
    msg.print(cerr);
  }

  // pointer for reply

  void *replyBuf;

  if ((msg.isReqSelectedSetsInfo() || msg.isReqAllSetsInfo()) &&
      msg.getRelayHostList().size() > 0) {

    ////////////// Relaying request to next host //////////////
    // special case for request relaying
    // This is indicated if the relayHostList is not empty

    replyBuf = _relayRequest(msg);

  } else if (msg.isRegLatestDataInfo()) {
    
    ////////////// registering latest data //////////////

    if (_isVerbose) {
      cerr << "  registering latest data info ..." << endl;
    }

    // register latest data info

    _lockStore();
    _store.regLatestDataInfo(msg.getInfo(0));
    _unlockStore();
    
    // assemble reply

    replyBuf = msg.assembleReplyRegStatus();
  
  } else if (msg.isRegStatusInfo()) {
    
    ////////////// registering latest data //////////////

    if (_isVerbose) {
      cerr << "  registering status info ..." << endl;
    }

    // register status

    _lockStore();
    _store.regStatusInfo(msg.getInfo(0));
    _unlockStore();

    // assemble reply

    replyBuf = msg.assembleReplyRegStatus();

  } else if (msg.isRegDataSetInfo()) {
    
    ////////////// registering data set info //////////////

    if (_isVerbose) {
      cerr << "  registering data set info ..." << endl;
    }

    // register data set

    _lockStore();
    _store.regDataSetInfo(msg.getInfo(0));
    _unlockStore();

    // assemble reply

    replyBuf = msg.assembleReplyRegStatus();

  } else if (msg.isRegFullInfo()) {
    
    ////////////// registering data set info //////////////

    if (_isVerbose) {
      cerr << "  registering full info ..." << endl;
    }

    // register

    _lockStore();
    _store.regFullInfo(msg.getInfo());
    _unlockStore();

    // assemble reply

    replyBuf = msg.assembleReplyRegStatus();

  } else if (msg.isDeleteInfo()) {
    
    ////////////// deleting //////////////

    if (_isVerbose) {
      cerr << "  deleting info ..." << endl;
    }

    // perform delete

    _lockStore();
    _store.deleteInfo(msg.getInfo(0));
    _unlockStore();

    // assemble reply

    replyBuf = msg.assembleReplyRegStatus();

  } else if (msg.isReqSelectedSetsInfo()) {
    
    ////////////// requesting selected info //////////////

    if (_isVerbose) {
      cerr << "  requesting selected info ..." << endl;
    }
    
    _lockStore();

    time_t now = time(NULL);
    infoSet_t selectedSet;
    const DMAP_info_t &msgInfo = msg.getInfo(0);
    _store.loadSelected(msgInfo.datatype, msgInfo.dir, selectedSet);
    TaArray<DMAP_info_t> selectedArray_;
    DMAP_info_t *selectedArray = selectedArray_.alloc(selectedSet.size());
    infoSet_t::iterator ii;
    int n = 0;
    for (ii = selectedSet.begin(); ii != selectedSet.end(); ii++, n++) {
      selectedArray[n] = ii->second;
      selectedArray[n].check_time = (ti32) now;
    } // ii
    
    replyBuf = msg.assembleReplyWithInfo(selectedSet.size(),
					 selectedArray);
    
    _unlockStore();

  } else if (msg.isReqAllSetsInfo()) {
    
    ////////////// requesting all info //////////////

    if (_isVerbose) {
      cerr << "  requesting all info ..." << endl;
    }

    _lockStore();

    time_t now = time(NULL);
    const infoSet_t &infoSet = _store.infoSet();
    TaArray<DMAP_info_t> infoArray_;
    DMAP_info_t *infoArray = infoArray_.alloc(infoSet.size());
    infoSet_t::const_iterator ii;
    int n = 0;
    for (ii = infoSet.begin(); ii != infoSet.end(); ii++, n++) {
      infoArray[n] = ii->second;
      infoArray[n].check_time = (ti32) now;
    } // ii
    replyBuf = msg.assembleReplyWithInfo(infoSet.size(), infoArray);

    _unlockStore();
    
  } else {
    
    replyBuf = msg.assembleReplyRegStatus
      (TRUE, "Unkown message type.");

    if (isDebug()) {
      cerr << "ERROR - DataMapper - unknown message type" << endl;
    }
    
  }
  
  // send reply

  int replyBuflen = msg.lengthAssembled();

  if (socket->writeMessage(DS_MESSAGE_TYPE_DMAP,
			   replyBuf, replyBuflen, 1000)) {
    cerr << "ERROR - COMM -DmapServer::handleDataCommand" << endl;
    cerr << "Writing reply" << endl;
    return -1;
  }

  // done

  if (_isVerbose) {
    cerr << "Leaving DmapServer::handleDataCommand()." << endl;
  }
  
  return 0;

}

//////////////////////////////////////////////////////////////////
// relay request, relay request to next host
//
// Uses this DataMapper as a relay to pass on the request and
// the return the reply.
// The status of this DataMapper does not change.
//
// Returns pointer to the reply buffer
      
void *DmapServer::_relayRequest(DmapMessage &msg)

{

  void *replyBuf = NULL;

  if (msg.isReqSelectedSetsInfo()) {
    
    const DMAP_info_t &msgInfo = msg.getInfo(0);
    DmapAccess access;
    if (access.reqSelectedInfo(msgInfo.datatype,
			       msgInfo.dir,
			       msg.getRelayHostList().c_str())) {

      string errStr =
	"ERROR - COMM -DataMapper::DmapServer::handleDataCommand()\n";
      errStr += "  ReqSelectedSetsInfo request\n";
      errStr += "  Cannot relay request to relay host: ";
      errStr += msg.getRelayHostList();
      cerr << errStr << endl;
      replyBuf = msg.assembleReplyRegStatus(TRUE, errStr.c_str());

    } else {

      TaArray<DMAP_info_t> info_;
      DMAP_info_t *info = info_.alloc(access.getNInfo());
      for (int i = 0; i < access.getNInfo(); i++) {
	info[i] = access.getInfo(i);
      }
      replyBuf = msg.assembleReplyWithInfo(access.getNInfo(), info);
      
    } // if (access....

  } else if (msg.isReqAllSetsInfo()) {
    
    DmapAccess access;
    if (access.reqAllInfo(msg.getRelayHostList().c_str())) {
      
      string errStr =
	"ERROR - COMM -DataMapper::DmapServer::handleDataCommand()\n";
      errStr += "  ReqAllSetsInfo request\n";
      errStr += "  Cannot relay request to relay host: ";
      errStr += msg.getRelayHostList();
      cerr << errStr << endl;
      replyBuf = msg.assembleReplyRegStatus(TRUE, errStr.c_str());
      
    } else {

      TaArray<DMAP_info_t> info_;
      DMAP_info_t *info = info_.alloc(access.getNInfo());
      for (int i = 0; i < access.getNInfo(); i++) {
	info[i] = access.getInfo(i);
      }
      replyBuf = msg.assembleReplyWithInfo(access.getNInfo(), info);
      
    } // if (access....

  }

  return replyBuf;

}

/////////////////////////////////
// lock the store mutex

void DmapServer::_lockStore()
{
  if (_useThreads) {
    pthread_mutex_lock(&_storeMutex);
    if (_isVerbose) {
      cerr << "--------> locking store" << endl;
    }
  }
}

/////////////////////////////////
// unlock the store mutex

void DmapServer::_unlockStore()
{
  if (_useThreads) {
    if (_isVerbose) {
      cerr << "--------> unlocking store" << endl;
    }
    pthread_mutex_unlock(&_storeMutex);
  }
}

/////////////////////////////////////////////////////
// override the timeoutMethod
//
// This gets called when the server times out waiting
// for a client.

bool DmapServer::timeoutMethod()
{

  // purge every now and again

  if (_params.purge_old_entries) {
    time_t now = time(NULL);
    if (now - _lastPurgeTime > 5) {
      _lockStore();
      _store.purge();
      _unlockStore();
      _lastPurgeTime = now;
    }
  }

  // call base classs method

  return DsThreadedServer::timeoutMethod();

}

/////////////////////////////////////////////////////
// override the postHandlerMethod
//
// This gets called after a message has been handled.

bool DmapServer::postHandlerMethod()
{

  // save state every now and again
  
  if (_params.save_state) {
    time_t now = time(NULL);
    if (now - _lastSaveTime > _params.save_state_secs) {
      _lockStore();
      _store.saveState();
      _unlockStore();
      _lastSaveTime = now;
    }
  }

  // call base classs method

  return DsThreadedServer::postHandlerMethod();
  
}


