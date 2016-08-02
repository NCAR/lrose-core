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
////////////////////////////////////////////////////////////////
// IpCache.cc
//
// Ip Name and Address caching
//
// Mike Dixon, RAL, UCAR, Boulder, CO, 80305
// June 2005
//
// Based on code by Phil Purdam,
// Bureau of Meteorology Research Center (BMRC), Australia
//
////////////////////////////////////////////////////////////////

#include <toolsa/IpCache.hh>
#include <cstdio>
#include <cstring>
using namespace std;

// static initializers

int IpCache::_requests = 0;
int IpCache::_lookups = 0;
int IpCache::_fails = 0;
bool IpCache::_debug = false;

// address by name cache

IpCache::ByNameMap IpCache::_cache;

// mutex for locking the cache

pthread_mutex_t IpCache::_mutex = PTHREAD_MUTEX_INITIALIZER;

//////////////////////////////////////
// get address from name
//
// Loads ip_addr_str string.
// If rethostaddr is not NULL, loads it up with the result.
//
// Returns 0 on success, -1 on failure

int IpCache::getAddrByName(const char *name,
			   std::string& in_addr_str,
			   in_addr *rethostaddr) {

#ifdef __linux
  
  // lock the cache
  
  pthread_mutex_lock(&_mutex);
  
  // look for cached entry
  
  _requests++;
  ByNameMap::iterator iter = _cache.find(name);
  
  if (iter != _cache.end()) {
    // entry exists
    if (IpCache::_debug) {
      ByName &entry = iter->second;
      cerr << "IpCache::getAddrByName found in cache: "
	   << entry.hostname << " (" << entry.inAddrString << ")" << endl;
    }
  } else {
    // add an entry
    ByName entry(name);
    _cache.insert(ByNameMap::value_type(name, entry));
    iter = _cache.find(name);
    if (IpCache::_debug) {
      cerr << "IpCache::getAddrByName - inserting new name into cache - "
	   << name << endl;
    }
  }
    
  ByName &entry = iter->second;
  
  if (entry.needRefresh()) {
    _lookups++;
    entry.resolve();
  }
  
  int result = -1;
  if (entry.resolvedOK()) {
    // success
    if (rethostaddr) {
      memcpy(rethostaddr, &(entry.inaddr), sizeof(in_addr));
    }
    in_addr_str = entry.inAddrString;
    if (IpCache::_debug) {
      cerr << " Resolved OK (" << entry.inAddrString << ")" << endl;
    }
    result = 0;
  } else {
    // failure
    if (rethostaddr) {
      memset(rethostaddr, 0, sizeof(in_addr));
    }
    in_addr_str = "";
    _fails++;
    if (IpCache::_debug) {
      cerr << " Resolve failed" << endl;
    }
    result = -1;
  }
  
  // unlock the cache
  
  pthread_mutex_unlock(&_mutex);
    
  return result;

#else

  return -1;

#endif

}

#ifdef __linux

//////////////////////////////
// dump cache status to file

void IpCache::dumpStatus(ostream &out) {
  out << "--->> IpCache status" << endl;
  out << "  AddressByName cache size: " << _cache.size() << endl;
  out << "                nRequests: " << _requests << endl;
  out << "                nLookups: " << _lookups << endl;
  out << "                nFails: " << _fails << endl;
}

/////////////////////////////////////////////////
// ByName helper class

IpCache::ByName::ByName(const std::string &name) :
  hostname(name),
  _lastResolvedTime(0),
  _nextResolveTime(0),
  _refreshPeriod(3600),
  _failedRefreshPeriod(300),
  _lookups(0),
  _lookupFails(0)
{
}

// check if need refresh
// returns true if resolve needed

bool IpCache::ByName::needRefresh() {
  if (time(NULL) > _nextResolveTime) {
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////
// resolve using name
// returns 0 on success, -1 on failure

int IpCache::ByName::resolve() {  
  
  addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_CANONNAME;
  
  _lastResolvedTime = 0;
  _nextResolveTime = 0;
  
  int result = 0;
  _lookups++;
  
  addrinfo *addrInfo;
  if ((result = getaddrinfo(hostname.c_str(), NULL, &hints, &addrInfo)) != 0) {
    
    if (IpCache::_debug) {
      cerr << "nameByAddr::resolve -  getaddrinfo failed for "
	   << hostname << " - " << gai_strerror(result) << endl;
    }
    
    _lastResolvedTime = 0;
    inAddrString = "";
    _nextResolveTime = time(NULL) + _failedRefreshPeriod;
    _lookupFails++;
    // freeaddrinfo(addrInfo);
    return -1;
    
  } else {
    
    _lastResolvedTime = time(NULL);
    _nextResolveTime = _lastResolvedTime + _refreshPeriod;
    
    if (IpCache::_debug) {
      cerr << "ByName::resolve succeeded for " << hostname << endl;
    }
    
    int count = 1;
    addrinfo *this_ai = addrInfo;
    while (this_ai) {
      
      if (count == 1) {
	// use first address for this instance
	memcpy(&inaddr, &(((sockaddr_in *)(this_ai->ai_addr))->sin_addr),
	       sizeof(in_addr));
	unsigned char *ucharptr = (unsigned char *)&inaddr;
	char addrstr[128];
	sprintf(addrstr, "%d.%d.%d.%d",
		ucharptr[0], ucharptr[1], ucharptr[2], ucharptr[3]);
	inAddrString = addrstr;
      } // if (count == 1)
      
      if (IpCache::_debug) {
	in_addr inAddr;
	memcpy(&inAddr, &(((sockaddr_in *)(this_ai->ai_addr))->sin_addr),
	       sizeof(in_addr));
	unsigned char *ucharptr = (unsigned char *)&inAddr;
	fprintf(stderr, "ia%d addlen=%d, name=%s %d.%d.%d.%d\n",
		count, this_ai->ai_addrlen, this_ai->ai_canonname, 
		ucharptr[0], ucharptr[1], ucharptr[2], ucharptr[3]);
      }

      this_ai = this_ai->ai_next;
      count++;

    } // while

  } // if ((result ...

  freeaddrinfo(addrInfo);
  return 0;
    
  
}

#endif
