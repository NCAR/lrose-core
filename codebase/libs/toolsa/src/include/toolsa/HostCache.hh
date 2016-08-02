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
// HostCache.hh
//
// HostCache class
//
// This is a thread-safe class which wraps HostCachebyname().
// It caches the hostname results for efficiency.
//
// This is buggy, so do not use yet.
//
////////////////////////////////////////////////////////////////

#ifndef HostCache_HH
#define HostCache_HH


#include <string>
#include <map>
#include <pthread.h>
#include <iostream>
#include <sys/utsname.h>
#ifdef NO_DATA
#undef NO_DATA
#endif
#include <netdb.h>
#include <sys/socket.h>
using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class HostCache

{

public:
  
  typedef struct hostent hostent_t;
  
  ///////////////
  // constructor
  
  HostCache();

  /////////////
  // destructor
  
  virtual ~HostCache();

  /////////////////////////////////////////////////
  // getByName()
  //
  // Returns 0 on success, -1 on failure.
  //
  // If successful, use HostCacheEnt() to get pointer to host entry.
  //
  // On error, use getErrorStr() to get error string.
  
  int getByName(const string hostname);
  
  ////////////////////////////////////////////
  // print()
  //
  // Print the current host info
  
  void print(ostream &out);
  
  ////////////////////////////////////////////
  // printCache()
  //
  // Print the cache
  
  void printCache(ostream &out);
  
 ////////////////////////////////////////////
  // printHostent()
  //
  // Print a hostent struct
  
  static void printHostent(const struct hostent *hh, ostream &out);
  
  //////////////////////////////////////*
  // localHostName()
  //
  // Short host name - no internet detail.
  
  string localHostName();

  ///////////////////////////////////////////////////
  // localHostNameFull()
  //
  // Fully qualified internet host name.
  
  string localHostNameFull();

  //////////////////////////////////////////////////
  // localIpAddr()
  // 
  // Returns IP address of local host.
  //
  
  string localIpAddr();

  //////////////////////////////////////////////////
  // remoteIpAddr()
  // 
  // Returns IP address of remote host.
  //
  
  string remoteIpAddr(const string &remote_hostname);
  
  ///////////////////////////////////////////////////////////
  // hostIsLocal()
  // 
  // Checks if the hostname given is the local host. It does
  // this by comparing the IP addresses.
  //
  // Returns true or false
  
  bool hostIsLocal(const string &remote_hostname);

  // access to data members
  
  const string &getErrorStr() const { return (_errorStr); }
  const string &getName() const { return (_info.name); }
  const string &getIpAddr() const { return (_info.ipAddr); }
  const int getAddrType() const { return (_info.addrType); }
  const int getAddrLen() const { return (_info.addrLen); }
  const unsigned char *getAddr() { return (_info.addr); }

protected:

  static const int maxAddrLen = 32;

  class hostInfo {
  public:
    string name;
    string ipAddr;
    int addrType;
    int addrLen;
    unsigned char addr[maxAddrLen];
  };
  
  // map for caching host info
  
  typedef pair<string, hostInfo > InfoPair;
  typedef map<string, hostInfo, less<string> > InfoMap;
  typedef InfoMap::iterator InfoIter;
  
  static InfoMap _cache;
  hostInfo _info;
  
  string _errorStr;
  string _ipaddr;
  
  // static mutex - locks out hostent struct in gethostbyname to
  // make it thread-safe
  
  static pthread_mutex_t _mutex;
  
  string _getUname();
  string _getLocalHostByName();
  string _loadIpAddr(const struct hostent *hh);
  void _printInfo(const hostInfo &info, ostream &out);
 
private:

};

#endif


