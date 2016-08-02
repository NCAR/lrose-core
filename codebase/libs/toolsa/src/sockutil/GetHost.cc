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
// GetHost.cc
//
// GetHost class
//
// This is a thread-safe class which wraps gethostbyname().
// It caches the hostname results for efficiency.
//
////////////////////////////////////////////////////////////////


#include <toolsa/GetHost.hh>
#include <toolsa/toolsa_macros.h>
#include <cstdio>
#include <cstring>
using namespace std;

pthread_mutex_t GetHost::_mutex = PTHREAD_MUTEX_INITIALIZER;

////////////////////////////////////////////////////////////
// Constructor

GetHost::GetHost()
{
  // construct for local host
  getByName();
}

////////////////////////////////////////////////////////////
// destructor

GetHost::~GetHost()

{
}


/////////////////////////////////////////////////////////////////
// getByName()
//
// Returns pointer to hostent on success, NULL on failure.
//
// On error, use getErrorStr() to get error string.

const struct hostent *GetHost::getByName(string hostname /* = "" */)
     
{

  // first get the host if not specified

  if (hostname.size() == 0) {
    struct utsname u;
    if (uname(&u) < 0) {
      hostname = "localhost";
    } else {
      hostname = u.nodename;
    }
  }

#if !defined(SUNOS5) && !defined (SUNOS5_INTEL) && !defined(IRIX6) && !defined(__linux)

  // generic OS, use non-recursive method

  return gethostbyname(hostname.c_str());

#else // non-generic

  pthread_mutex_lock(&_mutex);

  // get the host entry

  hostent_t *hh;

#if defined(SUNOS5) || defined (SUNOS5_INTEL) || defined(IRIX6)

  int h_errnop;
  hh = gethostbyname_r(hostname.c_str(),
		       &_hostent,
		       _hostBuf,
		       _hostBufSize,
		       &h_errnop);
  
#elif defined(__linux)

  int h_errnop;
  if (gethostbyname_r(hostname.c_str(),
		      &_hostent,
		      _hostBuf,
		      _hostBufSize,
		      &hh,
		      &h_errnop)) {
    hh = NULL;
  }

#endif

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
    pthread_mutex_unlock(&_mutex);
    return NULL;
  }

  _name = hh->h_name;
  _ipAddr = _loadIpAddr(*hh);
  _addrType = hh->h_addrtype;
  _addrLen = hh->h_length;
  if (_addrLen > _maxAddrLen) {
    _addrLen = _maxAddrLen;
  }
  memcpy(_addr, hh->h_addr_list[0], _addrLen);

  pthread_mutex_unlock(&_mutex);
  return hh;

#endif // generic

}

////////////////////////////////////////////
// print()
//
// Print the current host info
///

void GetHost::print(ostream &out)
     
{
  
  out << "====== HOST ENTRY ======" << endl;
  printHostent(_hostent, out);

}

////////////////////////////////////////////
// printHostent()
//
// Print a hostent struct

void GetHost::printHostent(const struct hostent &hh, ostream &out)
  
{
  
  out << "    h_name: " << hh.h_name << endl;

  char **aliases = hh.h_aliases;
  while (*aliases != NULL) {
    out << "    alias: " << *aliases << endl;
    aliases++;
  }
   
  if (hh.h_addrtype == AF_INET) {
    out << "    addrtype: AF_INET" << endl;
#ifdef AF_INET6
  } else if (hh.h_addrtype == AF_INET6) {
    out << "    addrtype: AF_INET6" << endl;
#endif
  } else {
    out << "    addrtype: UNKNOWN" << endl;
  }
  
  out << "    h_length: " << hh.h_length << endl;
  
  char ipaddr[256];
  char **addr_list = hh.h_addr_list;
  while (*addr_list != NULL) {
    char *strp = ipaddr;
    for (int i = 0; i < hh.h_length; i++) {
      sprintf (strp, "%d", (unsigned char) (*addr_list)[i]);
      strp = ipaddr + strlen(ipaddr);
      if (i < hh.h_length - 1) {
        *strp = '.';
        strp++;
      }
    }
    out << "    ipaddr: " << ipaddr << endl;
    addr_list++;
  }
  
}

//////////////////////////////////////*
// localHostName()
//
// Short host name - no internet detail.

string GetHost::localHostName()

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

string GetHost::localHostNameFull()

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

string GetHost::localIpAddr()

{
  
  struct utsname u;

  if (uname(&u) < 0) {
    return ("localIpAddr_Unknown");
  }
  
  if (getByName(u.nodename) == NULL) {
    return ("localIpAddr_Unknown");
  }

  return (_ipAddr);

}

//////////////////////////////////////////////////
// remoteIpAddr()
// 
// Returns IP address of remote host.
//

string GetHost::remoteIpAddr(const string &remote_hostname)
  
{
  
  if (getByName(remote_hostname) == NULL) {
    return ("remoteIpAddr_Unknown");
  }

  return (_ipAddr);

}

///////////////////////////////////////////////////////////
// hostIsLocal()
// 
// Checks if the hostname given is the local host. It does
// this heuristcially, by comparing various strings etc.
//
// Returns true or false

bool GetHost::hostIsLocal(const string &hostname)

{

  // check for local

  if (hostname == "localhost" ||
      hostname == "" ||
      hostname == "127.0.0.1") {
    return true;
  }

  // check for exact match

  string ourName = localHostName();
  if (hostname == ourName) {
    return true;
  }

  // if no periods in names, and they do not match exactly,
  // this is not local

  if (ourName.find('.') == string::npos &&
      hostname.find('.') == string::npos) {
    return false;
  }

  // try stripping periods off the longest name, to see if we
  // can match that way

  string shorter, longer;
  if (ourName.size() > hostname.size()) {
    shorter = hostname;
    longer = ourName;
  } else {
    shorter = ourName;
    longer = hostname;
  }
  size_t periodPos = longer.find('.');
  while (periodPos != string::npos) {
    string tmp = longer.substr(0, periodPos);
    longer = tmp;
    if (longer == shorter) {
      return true;
    }
    periodPos = longer.find('.');
  } // while

  // only go on for IP addresses

  int a1, a2, a3, a4;
  if (sscanf(hostname.c_str(), "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4) {
    return false;
  }

  struct utsname u;
  if (uname(&u) < 0) {
    return (false);
  }
  if (getByName(u.nodename) == NULL) {
    return (false);
  }
  char addr[_maxAddrLen];
  int addrLen = _addrLen;
  memcpy(addr, _addr, addrLen);

  if (getByName(hostname) == NULL) {
    return (false);
  }
  
  int len = MIN(addrLen, _addrLen);
  if (memcmp(addr, _addr, len)) {
    return (false);
  } else {
    return (true);
  }
  
}

///////////////////////////////////////////////////////////
// hostIsLocalStrict()
// 
// Performs a strict check on whether the hostname given is the
// local host. It does this by comparing the IP addresses.
//
// Returns true or false

bool GetHost::hostIsLocalStrict(const string &hostname)

{

  if (localIpAddr() == remoteIpAddr(hostname)) {
    return true;
  } else {
    return false;
  }

}

///////////////////////////////////////////////////////////
// hostIsLocal2()
// 
// Firsts checks using hostIsLocal(). If this fails, checks
// using hostIsLocalStrict().
//
// Returns true or false

bool GetHost::hostIsLocal2(const string &hostname)

{

  if (hostIsLocal(hostname)) {
    return true;
  }
  return hostIsLocalStrict(hostname);

}

////////////////////////////
// get name from uname call
///

string GetHost::_getUname()

{
  struct utsname uu;
  if (uname(&uu) < 0) {
    return ("getUname_Unknown");
  }
  return (uu.nodename);
}

///////////////////////////////////
// get name from gethostbyname call
///

string GetHost::_getLocalHostByName()

{

  struct utsname uu;

  if (uname(&uu) < 0) {
    return ("getLocalHostByName_Unknown");
  }

  if (getByName(uu.nodename) == NULL) {
    return ("getLocalHostByName_Unknown");
  }

  return (_name);

}

///////////////////////////////////////
// load up ipaddr string from addr_list

string GetHost::_loadIpAddr(const struct hostent &hh)
     
{
  
  string ipAddr;
  for (int i = 0; i < hh.h_length; i++) {
    char tmpStr[32];
    sprintf (tmpStr, "%d", (unsigned char) hh.h_addr_list[0][i]);
    ipAddr += tmpStr;
    if (i < hh.h_length - 1) {
      ipAddr += ".";
    }
  }
  return ipAddr;

}



