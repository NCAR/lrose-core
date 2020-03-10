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

#ifndef DS_CLIENT_HH
#define DS_CLIENT_HH

////////////////////////////////////////////////////////////////////
// DsClient.hh
//
// Object for clients to use to contact servers
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2002
//
///////////////////////////////////////////////////////////////

#include <didss/DsURL.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

class DsClient {

public:

  // constructor

  DsClient();

  // destructor

  virtual ~DsClient();
  
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
  
  int communicateAutoFwd(const DsURL &url, int msgType,
			 const void *msgBuf, ssize_t msgLen);
  
  // get data after successful comm call

  const void *getReplyBuf() { return _sock.getData(); }
  ssize_t getReplyLen() { return _sock.getNumBytes(); }

  // Request the DsServerMgr to start the server for this URL
  //
  // Returns 0 on success, -1 on failure.
  
  int requestMgrStartServer(const DsURL &url);

  // free up data

  void freeData();

  // set debugging

  void setDebug(bool state) { _debug = state; }

  // merge debugging with error string instead of writing to cerr

  void setMergeDebugWithErrStr(bool state) { _mergeDebugWithErrStr = state; }

  // set timeout for socket open calls, in msecs
  // default is blocking open

  void setOpenTimeoutMsecs(int msecs) { _openTimeoutMsecs = msecs; }

  // clear/set/get the Error String.
  // This has contents when an error is returned.
  
  void clearErrStr() const { _errStr = ""; }
  void setErrStr(const string &str) const { _errStr = str; }
  string getErrStr() const { return _errStr; }

protected:
  
  bool _debug;
  bool _mergeDebugWithErrStr;
  ThreadSocket _sock;
  mutable string _errStr;
  int _openTimeoutMsecs;
  
  void _closeSocket();

  int _communicateNoFwd(const DsURL &url, int msgType,
			const void *msgBuf, ssize_t msgLen,
			int commTimeoutMsecs);
  
  int _communicateFwd(const DsURL &url, int msgType,
		      const void *msgBuf, ssize_t msgLen,
		      int commTimeoutMsecs, bool &tunnelFailed);

  void _writeDebug(const string &label, const string &msg = "");
  void _writeDebug(const string &label, int ival);
  void _writeDebug(const string &label, long lval);
  
private:

};

#endif

