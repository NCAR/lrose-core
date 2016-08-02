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
// HostCache.cc
//
// HostCache class
//
// This is a thread-safe class which wraps gethostbyname().
// It caches the hostname results for efficiency.
//
// This is buggy, so do not use yet.
//
////////////////////////////////////////////////////////////////


#include <toolsa/umisc.h>
#include <toolsa/HostCache.hh>
#include <cstdio>
#include <utility>
using namespace std;

pthread_mutex_t HostCache::_mutex = PTHREAD_MUTEX_INITIALIZER;
HostCache::InfoMap HostCache::_cache;

////////////////////////////////////////////////////////////
// Constructor

HostCache::HostCache()
  
{
}

////////////////////////////////////////////////////////////
// destructor

HostCache::~HostCache()

{
}


/////////////////////////////////////////////////////////////////
// getByName()
//
// Returns 0 on success, -1 on failure.
//
// If successful, use getHostent() to get pointer to host entry.
//
// On error, use getErrorStr() to get error string.
///

int HostCache::getByName(const string hostname)
  
{

  int iret = 0;

  // lock to protect hostent and cache
  
  pthread_mutex_lock(&_mutex);
  cerr << "======= start of HostCache::getByName() =======" << endl;

  // check if the entry is already cached

  InfoIter ii = _cache.find(hostname);
  bool cached = false;
  if (ii != _cache.end()) {
    cached = true;
  }
  
  if (cached) {

    // copy cached value to local
    
    _info = (*ii).second;

  } else {
  
    // not cached, get the entry
    
    hostent_t *hh = gethostbyname(hostname.c_str());

    if (hh == NULL) {

      _errorStr = "gethostbyname failed\n";
      switch (h_errno) {
      case HOST_NOT_FOUND:
	_errorStr += "  HOST_NOT_FOUND\n";
	break;
      case TRY_AGAIN:
	_errorStr += "  TRY_AGAIN\n";
	break;
      case NO_RECOVERY:
	_errorStr += "  NO_RECOVERY\n";
	break;
      case NO_DATA:
	_errorStr += "  NO_DATA\n";
	break;
      }
      iret = -1;

    } else {
    
      // copy to local memory
      
      _info.name = hh->h_name;
      memset(_info.addr, 0, maxAddrLen);
      _info.addrLen = MIN(maxAddrLen, hh->h_length);
      if (hh->h_addr_list != NULL && hh->h_addr_list[0] != NULL) {
	_info.ipAddr = _loadIpAddr(hh);
	memcpy(_info.addr, hh->h_addr_list[0], _info.addrLen);
      } else {
	_info.ipAddr = "No_ipaddr_available";
      }
      _info.addrType = hh->h_addrtype;
      
      // store in cache
      
      InfoPair pr;
      pr.first = hostname;
      pr.second = _info;
      _cache.insert(pr);
      
    }
    
  } // if (cached)

  // unlock
  
  cerr << "******* end  of HostCache::getByName() *******" << endl;
  cerr << "iret: " << iret << endl;

  pthread_mutex_unlock(&_mutex);
  
  printCache(cerr);
  
  return iret;

}

////////////////////////////////////////////
// print()
//
// Print the current host info
///

void HostCache::print(ostream &out)
     
{
  
  out << "====== HOST INFO ======" << endl;
  _printInfo(_info, out);

}

////////////////////////////////////////////
// printCache()
//
// Print the cache
///

void HostCache::printCache(ostream &out)
  
{

  // lock cache
  
  pthread_mutex_lock(&_mutex);

  cerr << ">>>>>>> start of HostCache::printCache() >>>>>>>" << endl;

  if (_cache.empty()) {
    out << "No entries in cache." << endl;
  }

  out << ">>>>> HostCache cache <<<<<" << endl;
  out << "  Nentries: " << _cache.size() << endl;
  out << endl;

  int i = 0;
  InfoIter ii;
  for (ii = _cache.begin();
       ii != _cache.end(); ii++, i++) {
    out << "  Entry #" << i;
    out << ": " << (*ii).first << endl;
    _printInfo((*ii).second, out);
  }
  
  // unlock cache
  
  cerr << "<<<<<<< end  of HostCache::printCache() <<<<<<<" << endl;
  pthread_mutex_unlock(&_mutex);
  
}

////////////////////////////////////////////
// printHostent()
//
// Print a hostent struct

void HostCache::printHostent(const struct hostent *hh, ostream &out)
  
{
  
  out << "    h_name: " << hh->h_name << endl;

  char **aliases = hh->h_aliases;
  while (*aliases != NULL) {
    out << "    alias: " << *aliases << endl;
    aliases++;
  }
   
  if (hh->h_addrtype == AF_INET) {
    out << "    addrtype: AF_INET" << endl;
#ifdef AF_INET6
  } else if (hh->h_addrtype == AF_INET6) {
    out << "    addrtype: AF_INET6" << endl;
#endif
  } else {
    out << "    addrtype: UNKNOWN" << endl;
  }
  
  out << "    h_length: " << hh->h_length << endl;
  
  char ipaddr[256];
  char **addr_list = hh->h_addr_list;
  while (*addr_list != NULL) {
    char *strp = ipaddr;
    for (int i = 0; i < hh->h_length; i++) {
      sprintf (strp, "%d", (unsigned char) (*addr_list)[i]);
      strp = ipaddr + strlen(ipaddr);
      if (i < hh->h_length - 1) {
        *strp = '.';
        strp++;
      }
    }
    out << "    ipaddr: " << ipaddr << endl;
    addr_list++;
  }
  
}

////////////////////////////////////////////
// _printInfo()
//
// Print an info object

void HostCache::_printInfo(const hostInfo &info, ostream &out)
  
{
  out << "    name: " << info.name << endl;
  out << "    ipAddr: " << info.ipAddr << endl;
  out << "    addrLen: " << info.addrLen << endl;
  if (info.addrType == AF_INET) {
    out << "    addrtype: AF_INET" << endl;
#ifdef AF_INET6
  } else if (info.addrType == AF_INET6) {
    out << "    addrtype: AF_INET6" << endl;
#endif
  } else {
    out << "    addrType: " << info.addrType << " (UNKNOWN)" << endl;
  }
}

//////////////////////////////////////*
// localHostName()
//
// Short host name - no internet detail.

string HostCache::localHostName()

{
  
#if defined(DECOSF1)
  return (_getLocalHostByName());
#else
  return (_getUname());
#endif

}

///////////////////////////////////////////////////
// localHostNameFull()
//
// Fully qualified internet host name.

string HostCache::localHostNameFull()

{
  
#if defined(DECOSF1)
  return (_getUname());
#else
  return (_getLocalHostByName());
#endif

}

//////////////////////////////////////////////////
// localIpAddr()
// 
// Returns IP address of local host.

string HostCache::localIpAddr()

{
  
  struct utsname u;

  if (uname(&u) < 0) {
    return ("Unknown");
  }
  
  if (getByName(u.nodename)) {
    return ("Unknown");
  }

  return (_info.ipAddr);

}

//////////////////////////////////////////////////
// remoteIpAddr()
// 
// Returns IP address of remote host.
//

string HostCache::remoteIpAddr(const string &remote_hostname)
  
{
  
  if (getByName(remote_hostname)) {
    return ("Unknown");
  }

  return (_info.ipAddr);

}

///////////////////////////////////////////////////////////
// hostIsLocal()
// 
// Checks if the hostname given is the local host. It does
// this by comparing the IP addresses.
//
// Returns true or false

bool HostCache::hostIsLocal(const string &remote_hostname)

{

  struct utsname u;
  if (uname(&u) < 0) {
    return (false);
  }
  if (getByName(u.nodename)) {
    return (false);
  }
  hostInfo local = _info;
  if (getByName(remote_hostname)) {
    return (false);
  }
  
  if (_info.ipAddr == local.ipAddr) {
    return true;
  } else {
    return false;
  }

//   int length = MIN(local.addrLen, _info.addrLen);
//   if (memcmp(local.entryStruct.h_addr_list[0],
// 	     _info.entryStruct.h_addr_list[0],
// 	     length)) {
//     return (false);
//   } else {
//     return (true);
//   }

}

////////////////////////////
// get name from uname call
///

string HostCache::_getUname()

{
  struct utsname uu;
  if (uname(&uu) < 0) {
    return ("Unknown");
  }
  return (uu.nodename);
}

///////////////////////////////////
// get name from gethostbyname call
///

string HostCache::_getLocalHostByName()

{

  struct utsname uu;

  if (uname(&uu) < 0) {
    return ("Unknown");
  }

  if (getByName(uu.nodename)) {
    return ("Unknown");
  }

  return (_info.name);

}

///////////////////////////////////////
// load up ipaddr string from addr_list

string HostCache::_loadIpAddr(const struct hostent *hh)
     
{
  
  string ipAddr;
  for (int i = 0; i < hh->h_length; i++) {
    char tmpStr[32];
    sprintf (tmpStr, "%d", (unsigned char) hh->h_addr_list[0][i]);
    ipAddr += tmpStr;
    if (i < hh->h_length - 1) {
      ipAddr += ".";
    }
  }
  return ipAddr;

}



