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
//////////////////////////////////////////////////////////
// Procmap.cc: Proc mapper
//////////////////////////////////////////////////////////

#include "Procmap.hh"
#include <toolsa/str.h>
#include <toolsa/MemBuf.hh>
#include <toolsa/PmuInfo.hh>

#define LISTEN_MSECS 500
#define PURGE_SECS 5

threadArgs::threadArgs(int sockFd, bool debug,
		       InfoStore *store, time_t startTime) :
  _sockFd(sockFd),
  _debug(debug),
  _store(store),
  _startTime(startTime)
{
}

Procmap::Procmap (int argc, char **argv)
     
{

  // initialize members

  isOK = true;
  _port = PMU_get_default_port();
  _lastPurge = 0;
  _nThreads = 0;
  _startTime = time(NULL);
  _protoFd = -1;

  // set programe name

  _progName = "procmap";
  ucopyright((char *) _progName.c_str());

  // get command line args

  if (_args.parse(argc, argv, _progName)) {
    cerr << "ERROR: " << _progName << endl;
    cerr << "Problem with command line args" << endl;
    isOK = FALSE;
    return;
  }

  // run as daemon?

  if (_args.daemon) {
    _args.debug = false;
    udaemonize();
  }
  
  // set store object debugging

  if (_args.debug) {
    _store.setDebug();
    cerr << "Max n threads: " << _args.maxThreads << endl;
  }

}

//////////////
// destructor
//

Procmap::~Procmap ()

{
  if (_protoFd >= 0) {
    SKU_close(_protoFd);
  }
}

////////////////////////////////////////////
// run - communicate with clients and procs
//

int Procmap::Run()
{

  // open port on which to listen
  
  if ((_protoFd = SKU_open_server(_port)) < 0) {
    cerr << "ERROR - Procmap::Run()" << endl;
    cerr << "   SKU_open_server failed, port: " << _port << endl;
    return -1;
  }
  
  // loop forever, waiting for connections

  while (true) {

    // get next client
    // emerge after timeout or client connects

    int sockFd = SKU_get_client_timed(_protoFd, LISTEN_MSECS);
    
    if (sockFd >= 0) {
      
      // got a client
      // check that the number of threads is OK
      // if not, send failure reply
      
      if (_nThreads > _args.maxThreads) {
	cerr << "WARNING - Procmap::Run()" << endl;
	cerr << "  Too many threads, sending failure reply." << endl;
	_sendReply(sockFd, _args.debug, _startTime, PROCMAP_FAILURE);
	SKU_close(sockFd);
	continue;
      }
      
      // Create a thread to handle the request, then detach it.
      // The socket will be closed by the thread.
      // The args are freed by the thread.

      pthread_t thread;
      threadArgs *thrArgs = new threadArgs(sockFd, _args.debug,
					   &_store, _startTime);
      int iret;
      if ((iret = pthread_create(&thread, NULL, _handleIncoming,
				 (void *) thrArgs))) {
	cerr << "ERROR - " << _progName << ":Run()." << endl;
	cerr << "  Cannot create thread." << endl;
	cerr << "  " << strerror(iret) << endl;
	SKU_close(sockFd);
	continue;
      }
      pthread_detach(thread);
      
    } // if (sockFd >= 0)
    
    // time out anybody who has not registered lately
    
    time_t now = time(NULL);
    if (now - _lastPurge >= PURGE_SECS) {
      _store.Purge(now);
      _lastPurge = now;
    }

    // make sure we do not run away with things

    umsleep(10);
  
  } // while (true)

  return 0;

}

///////////////////////////////////////////////////////
// handle incoming message - this is a separate thread
//

void *Procmap::_handleIncoming(void *thread_args)

{

  // interpret args and then free

  threadArgs *thrArgs = (threadArgs *) thread_args;
  int sockFd = thrArgs->_sockFd;
  bool debug = thrArgs->_debug;
  InfoStore *store = thrArgs->_store;
  time_t startTime = thrArgs->_startTime;
  delete thrArgs;

  // read message from client
  
  SKU_header_t header;
  long nbytes_in;
  char *msg_in;
  if (SKU_read_message(sockFd, &header,
		       &msg_in, &nbytes_in, -1) == 1) {

    // successful read - process the message
    
    switch (header.id) {
      
    case PROCMAP_REGISTER:
    case PROCMAP_UNREGISTER:
      _handleRegister(sockFd, debug, store, startTime,
		      header, msg_in);
      break;
      
    case PROCMAP_GET_INFO:
      _handleGetInfo(sockFd, debug, store, startTime,
		     header, msg_in);
      break;
    
    case PROCMAP_GET_INFO_RELAY:
      _handleGetInfoRelay(sockFd, debug, store, startTime,
			  header, msg_in);
      break;
    
    default:
      if (debug) {
	cerr << "WARNING - Procmap::_handleIncoming()." << endl;
	cerr << "Incorrect data type in incoming message: "
	     << header.id << endl;
      }
      
    } // switch
    
  } // if (SKU_read_message ...

  if (debug) {
    store->print(cerr);
  }

  // wait 30 secs for EOF from client, so that we can do the 'passive'
  // close. This prevent procmap sockets from having the TIME_WAIT stats.

  SKU_read_select(sockFd, 30000);

  // close the socket

  SKU_close(sockFd);

  return NULL;
  
}

///////////////////////////
// handle register message
//

void Procmap::_handleRegister(int sockFd,
			      bool debug,
			      InfoStore *store,
			      time_t startTime,
			      const SKU_header_t &header,
			      void *msg_in)

{

  // check the length of the message
  
  if (header.len != sizeof(PROCMAP_info_t)) {
    if (debug) {
      cerr << "WARNING - Procmap::_handleRegister" << endl;
      cerr << "Reg request message length incorrect: "
	   <<  header.len << " bytes" << endl;
      cerr << "Should be: " << sizeof(PROCMAP_info_t) << " bytes" << endl;
    }
    _sendReply(sockFd, debug, startTime, PROCMAP_FAILURE);
    return;
  }
  
  // convert incoming info in host byte order
  
  PROCMAP_info_t info;
  memcpy(&info, msg_in, sizeof(info));
  PMU_ntohl_Info(&info);

  // update store

  switch (header.id) {
  case PROCMAP_REGISTER:
    store->RegisterProc(info);
    break;
  case PROCMAP_UNREGISTER:
    store->UnRegisterProc(info);
    break;
  } // switch
  
  // send reply to indicate success
  
  _sendReply(sockFd, debug, startTime, PROCMAP_SUCCESS);

}

//////////////////////////
// handle get info message
//

void Procmap::_handleGetInfo(int sockFd,
			     bool debug,
			     InfoStore *store,
			     time_t startTime,
			     const SKU_header_t header,
			     void *msg_in)
  
{
  
  // check the length of the message
  
  if (header.len != sizeof(PROCMAP_request_t)) {
    if (debug) {
      cerr << "WARNING - Procmap::_handleGetInfo" << endl;
      cerr << "Info request message length incorrect: "
	   <<  header.len << " bytes" << endl;
      cerr << "Should be: " << sizeof(PROCMAP_request_t) << " bytes" << endl;
    }
    _sendReply(sockFd, debug, startTime, PROCMAP_FAILURE);
    return;
  }
  
  // convert incoming request into host byte order
  
  PROCMAP_request_t req;
  memcpy(&req, msg_in, sizeof(req));
  PMU_ntohl_Request(&req);
  
  // get info from the store
  
  MemBuf infoBuf;
  store->GetInfo(req, infoBuf);
  int nProcs = infoBuf.getLen() / sizeof(PROCMAP_info_t);
  
  // swap info
  
  PROCMAP_info_t *info = (PROCMAP_info_t *) infoBuf.getPtr();
  for (int i = 0; i < nProcs; i++) {
    PMU_htonl_Info(&info[i]);
  }
  
  // fill out reply header
  
  PROCMAP_reply_t reply;
  memset(&reply, 0, sizeof(PROCMAP_reply_t));
  reply.uptime = time(NULL) - startTime;
  reply.n_procs = nProcs;
  reply.reply_time = time(NULL);
  if (nProcs > 0) {
    reply.return_code = PROCMAP_SUCCESS;
  } else {
    reply.return_code = PROCMAP_FAILURE;
  }
  PMU_htonl_Reply(&reply);
  
  // assemble reply and info into a single buffer

  MemBuf outBuf;
  outBuf.add(&reply, sizeof(reply));
  outBuf.add(infoBuf.getPtr(), infoBuf.getLen());
  
  // send buffer
  
  if (SKU_write_message(sockFd, 1,
			(char *) outBuf.getPtr(), outBuf.getLen()) != 1) {
    if (debug) {
      cerr << "WARNING - Procmap::_handleGetInfo" << endl;
      cerr << "Failure to send reply" << endl;
    }
  }
  
}

////////////////////////////////////////////////////////////
// handle get info message - need to relay from another host
//

void Procmap::_handleGetInfoRelay(int sockFd,
				  bool debug,
				  InfoStore *store,
				  time_t startTime,
				  const SKU_header_t header,
				  void *msg_in)
  
{
  
  // check the length of the message
  
  if (header.len != sizeof(PROCMAP_request_relay_t)) {
    if (debug) {
      cerr << "WARNING - Procmap::_handleGetInfoRelay" << endl;
      cerr << "Info request message length incorrect: "
	   <<  header.len << " bytes" << endl;
      cerr << "Should be: " << sizeof(PROCMAP_request_relay_t)
	   << " bytes" << endl;
    }
    _sendReply(sockFd, debug, startTime, PROCMAP_FAILURE);
    return;
  }

  // prepare reply

  MemBuf outBuf;
  PROCMAP_reply_t reply;
  memset(&reply, 0, sizeof(PROCMAP_reply_t));

  // copy in request message
  
  PROCMAP_request_relay_t relay_req;
  memcpy(&relay_req, msg_in, sizeof(relay_req));

  // if relay_hosts is zero length, use local

  if (strlen(relay_req.relay_hosts) == 0) {

    // get info from the store
    
    MemBuf infoBuf;
    PROCMAP_request_t req;
    STRncopy(req.name, relay_req.name, PROCMAP_NAME_MAX);
    STRncopy(req.instance, relay_req.instance, PROCMAP_INSTANCE_MAX);
    store->GetInfo(req, infoBuf);
    int nProcs = infoBuf.getLen() / sizeof(PROCMAP_info_t);
    
    // swap info
    
    PROCMAP_info_t *info = (PROCMAP_info_t *) infoBuf.getPtr();
    for (int i = 0; i < nProcs; i++) {
      PMU_htonl_Info(&info[i]);
    }
  
    // fill out reply header
    
    reply.uptime = time(NULL) - startTime;
    reply.n_procs = nProcs;
    reply.reply_time = time(NULL);
    if (nProcs > 0) {
      reply.return_code = PROCMAP_SUCCESS;
    } else {
      reply.return_code = PROCMAP_FAILURE;
    }
    PMU_htonl_Reply(&reply);
    
    // assemble reply and info into a single buffer
    
    outBuf.add(&reply, sizeof(reply));
    outBuf.add(infoBuf.getPtr(), infoBuf.getLen());

  } else {

    // get info from relay host
    
    PmuInfo pmuInfo;
    if (pmuInfo.read(relay_req.relay_hosts)) {

      reply.n_procs = 0;
      reply.return_code = PROCMAP_FAILURE;
      reply.reply_time = -1;
      PMU_htonl_Reply(&reply);
      outBuf.add(&reply, sizeof(reply));

    } else {

      reply.uptime = pmuInfo.getUpTime();
      reply.n_procs = pmuInfo.getNProcs();
      reply.reply_time = pmuInfo.getReplyTime();
      if (reply.n_procs > 0) {
	reply.return_code = PROCMAP_SUCCESS;
      } else {
	reply.return_code = PROCMAP_FAILURE;
      }
      PMU_htonl_Reply(&reply);
      outBuf.add(&reply, sizeof(reply));

      for (int i = 0; i < pmuInfo.getNProcs(); i++) {
	PROCMAP_info_t info = pmuInfo.getInfoArray()[i];
	PMU_htonl_Info(&info);
	outBuf.add(&info, sizeof(info));
      }

    }

  }

  // send buffer
  
  if (SKU_write_message(sockFd, 1,
			(char *) outBuf.getPtr(), outBuf.getLen()) != 1) {
    if (debug) {
      cerr << "WARNING - Procmap::_handleGetInfoRelay" << endl;
      cerr << "Failure to send reply" << endl;
    }
  }
  
}

///////////////////////////////////////////
// return reply
//

void Procmap::_sendReply(int sockFd, bool debug,
			 time_t startTime, int return_code)
  
{
  
  PROCMAP_reply_t reply;
  memset(&reply, 0, sizeof(PROCMAP_reply_t));
  reply.uptime = time(NULL) - startTime;
  reply.return_code = return_code;
  reply.reply_time = time(NULL);
  PMU_htonl_Reply(&reply);
  
  if (SKU_write_message(sockFd, 1, (char *) &reply, sizeof(reply)) != 1) {
    if (debug) {
      cerr << "WARNING - procmap:sendReply" << endl;
      cerr << "Failed to send reply" << endl;
    }
  }

}
