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
////////////////////////////////////////////////////////////////////
// PmuInfo.cc
//
// Provides access to PMU info in a thread-safe manner.
//
// Mike Dixon, RAP, NCAR, POBox 3000, Boulder, CO, 80307, USA
//
// April 2000
//
////////////////////////////////////////////////////////////////////


#include <toolsa/str.h>
#include <toolsa/PmuInfo.hh>
#include <toolsa/TaStr.hh>
#include <toolsa/ThreadSocket.hh>
using namespace std;

///////////////////////
// default constructor

PmuInfo::PmuInfo()

{
  _nProcs = 0;
  _upTime = 0;
  _replyTime = 0;
  _procmapPort = PMU_get_default_port();
}

/////////////
// destructor
  
PmuInfo::~PmuInfo()

{

}

////////////////////////////////////////////////////////////////
// Read the info, from procmap on hostname.
//
// All processes which match procname, up to its length, and
// instance, up to its length, are returned.
// So to get all processes, use empty strings.
// To get a single process, use full name and instance.
//
// Returns 0 on success, -1 on error
// Get error via getErrStr().

int PmuInfo::read(string hostname /* = "localhost"*/,
		  string procname /* = ""*/,
		  string instance /* = ""*/ )

{

  // prepare error string

  _errStr = "ERROR - PmuInfo::read\n";
  TaStr::AddStr(_errStr, "  hostname: ", hostname);
  TaStr::AddStr(_errStr, "  procname: ", procname);
  TaStr::AddStr(_errStr, "  instance: ", instance);

  // check to see if this request goes via a relay host.
  // Hosts in the list will be colon-delimited.

  if (hostname.find(":", 0) == string::npos) {

    return (_read(hostname, procname, instance));

  } else {

    if (_readRelay(hostname, procname, instance)) {
      return -1;
    }
    if (_replyTime < 0) {
      return -1;
    }

  }
  
  return 0;

}
  

////////////////////////////////////////////////////////////////
// Read the info, from procmap on hostname.
// No relay.

int PmuInfo::_read(const string &hostname,
		   const string &procname,
		   const string &instance)
  
{

  // open socket to procmap

  ThreadSocket sock;
  if (sock.open(hostname.c_str(), _procmapPort)) {
    _errStr += "  Cannot open socket\n";
    return -1;
  }
  
  // set up the request

  PROCMAP_request_t req;
  STRncopy(req.name, procname.c_str(), PROCMAP_NAME_MAX);
  STRncopy(req.instance, instance.c_str(), PROCMAP_INSTANCE_MAX);
  PMU_htonl_Request(&req);

  // write the request

  if (sock.writeMessage(PROCMAP_GET_INFO, &req, sizeof(req))) {
    _errStr += "  Cannot write request to procmap\n";
    sock.close();
    return -1;
  }

  // read the reply

  if (sock.readMessage()) {
    _errStr += "  Cannot read reply from procmap\n";
    sock.close();
    return -1;
  }

  // check size

  if (sock.getHeaderLength() < (int) sizeof(PROCMAP_reply_t)) {
    _errStr += "  Return buffer not long enough.\n";
    TaStr::AddInt(_errStr, "  Buffer size: ", sock.getHeaderLength());
    TaStr::AddInt(_errStr, "  Min length: ", sizeof(PROCMAP_reply_t));
    return -1;
  }

  PROCMAP_reply_t *reply = (PROCMAP_reply_t *) sock.getData();
  PMU_ntohl_Reply(reply);

  _upTime = reply->uptime;
  _replyTime = reply->reply_time;
  _nProcs = reply->n_procs;

  if (_nProcs > 0 && PROCMAP_SUCCESS != reply->return_code) {
    TaStr::AddInt(_errStr, "  Bad return code: ", reply->return_code);
    return -1;
  }
  
  // check size

  int infoLen = _nProcs * sizeof(PROCMAP_info_t);
  if (sock.getHeaderLength() != (int) (sizeof(PROCMAP_reply_t) + infoLen)) {
    _errStr += "  Return buffer incorrect length.\n";
    TaStr::AddInt(_errStr, "  Buffer size: ", sock.getHeaderLength());
    TaStr::AddInt(_errStr, "  Expected size: ",
		  sizeof(PROCMAP_reply_t) + infoLen);
    return -1;
  }

  // copy info into MemBuf, and swap
  
  _infoBuf.free();
  _infoBuf.add(reply + 1, infoLen);

  PROCMAP_info_t *info = (PROCMAP_info_t *) _infoBuf.getPtr();
  for (int i = 0; i < _nProcs; i++) {
    PMU_ntohl_Info(info + i);
  }

  return 0;

}
  

////////////////////////////////////////////////////////////////
// Read the info, from procmap at end of host list.
// Relay via the hosts in the list.

int PmuInfo::_readRelay(const string &hostlist,
			const string &procname,
			const string &instance)

{

  // get location of first colon in host list

  size_t colonPos = hostlist.find(":", 0);
  if (colonPos == string::npos) {
    return (_read(hostlist, procname, instance));
  }

  // compute host name to contact, and remaining_hosts

  string hostname, remainingList;
  hostname.assign(hostlist, 0, colonPos);
  remainingList.assign(hostlist, colonPos + 1, string::npos);

  // open socket to procmap

  ThreadSocket sock;
  if (sock.open(hostname.c_str(), _procmapPort)) {
    _errStr += "  Cannot open socket\n";
    return -1;
  }
  
  // set up the request

  PROCMAP_request_relay_t req;
  STRncopy(req.relay_hosts, remainingList.c_str(), PROCMAP_HOST_RELAY_MAX);
  STRncopy(req.name, procname.c_str(), PROCMAP_NAME_MAX);
  STRncopy(req.instance, instance.c_str(), PROCMAP_INSTANCE_MAX);

  // write the request

  if (sock.writeMessage(PROCMAP_GET_INFO_RELAY, &req, sizeof(req))) {
    _errStr += "  Cannot write request to procmap\n";
    sock.close();
    return -1;
  }

  // read the reply

  if (sock.readMessage()) {
    _errStr += "  Cannot read reply from procmap\n";
    sock.close();
    return -1;
  }

  // check size

  if (sock.getHeaderLength() < (int) sizeof(PROCMAP_reply_t)) {
    _errStr += "  Return buffer not long enough.\n";
    TaStr::AddInt(_errStr, "  Buffer size: ", sock.getHeaderLength());
    TaStr::AddInt(_errStr, "  Min length: ", sizeof(PROCMAP_reply_t));
    return -1;
  }

  PROCMAP_reply_t *reply = (PROCMAP_reply_t *) sock.getData();
  PMU_ntohl_Reply(reply);
  
  _upTime = reply->uptime;
  _replyTime = reply->reply_time;
  _nProcs = reply->n_procs;

  if (_nProcs > 0 && PROCMAP_SUCCESS != reply->return_code) {
    TaStr::AddInt(_errStr, "  Bad return code: ", reply->return_code);
    return -1;
  }
  
  // check size

  int infoLen = _nProcs * sizeof(PROCMAP_info_t);
  if (sock.getHeaderLength() != (int) (sizeof(PROCMAP_reply_t) + infoLen)) {
    _errStr += "  Return buffer incorrect length.\n";
    TaStr::AddInt(_errStr, "  Buffer size: ", sock.getHeaderLength());
    TaStr::AddInt(_errStr, "  Expected size: ",
		  sizeof(PROCMAP_reply_t) + infoLen);
    return -1;
  }

  // copy info into MemBuf, and swap
  
  _infoBuf.free();
  _infoBuf.add(reply + 1, infoLen);

  PROCMAP_info_t *info = (PROCMAP_info_t *) _infoBuf.getPtr();
  for (int i = 0; i < _nProcs; i++) {
    PMU_ntohl_Info(info + i);
  }

  return 0;

}
  

//////////////////////////////////////////////////////////////////////
// get array of process info structs

const PROCMAP_info_t *PmuInfo::getInfoArray() const

{
  return ((PROCMAP_info_t *) _infoBuf.getPtr());
}
  
