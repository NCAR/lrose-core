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
// IpCache.hh
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

#ifndef	__IPNAMECACHE_HH
#define __IPNAMECACHE_HH

#include <string>
#include <map>
#include <iostream>
#include <netdb.h>
#if defined(CYGWIN)
#include <netinet/in.h>
#endif
#include <pthread.h>

using namespace std;

class IpCache
{
  
  // helper classes

  class ByName;

public:
  
  // resolve address from name
  // returns 0 on success, -1 on failure
  
  static int getAddrByName(const char *name,
			   std::string& in_addr_str,
			   in_addr *rethostaddr = NULL);  

  // debugging
  
  static bool _debug;
  static void setDebug(bool state) { _debug = state; }
  static int getRequestCount() { return _requests; };
  static int getLookupCount() { return _lookups; };
  static int getRefreshCount() { return _fails; };
  static void dumpStatus(ostream &out = cerr);

protected:
  
  static int _requests, _lookups, _fails;

  typedef std::map<std::string, ByName> ByNameMap;
  static ByNameMap _cache;  // address by name cache

  // mutex to protect cache and make thread-safe
  
  static pthread_mutex_t _mutex;

private:

  // private constructor and destructor - do not use

  IpCache();
  IpCache(const IpCache &);
  ~IpCache();
  
  // helper (inner) classes

  class ByName {

  public:
    
    ByName(const string &name);
	~ByName() { };

    int resolve();
    bool needRefresh();
    bool resolvedOK() { return _lastResolvedTime != 0; };
    
    in_addr inaddr;
    std::string inAddrString;  // numeric address string in nn.nn.nn.nn form
    std::string hostname;
    
  private:
    time_t _lastResolvedTime;   // use as valid flag, if time=0 not valid
    time_t _nextResolveTime;
    int _refreshPeriod, _failedRefreshPeriod;
    int _lookups, _lookupFails;

  };

};
  
#endif
