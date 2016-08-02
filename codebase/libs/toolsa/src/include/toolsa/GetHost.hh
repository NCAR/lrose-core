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
// GetHost.hh
//
// GetHost class
//
// This is a thread-safe class which uses gethostbyname_r().
//
////////////////////////////////////////////////////////////////

#ifndef GetHost_HH
#define GetHost_HH


#include <string>

#include <iostream>
#include <sys/utsname.h>
#ifdef NO_DATA
#undef NO_DATA
#endif
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
using namespace std;

///////////////////////////////////////////////////////////////
// class definition

class GetHost

{

public:
  
  typedef struct hostent hostent_t;
  
  ///////////////
  // constructor
  
  GetHost();

  /////////////
  // destructor
  
  virtual ~GetHost();

  /////////////////////////////////////////////////////////////////
  // getByName()
  //
  // Returns pointer to hostent on success, NULL on failure.
  //
  // On error, use getErrorStr() to get error string.

  const struct hostent *getByName(string hostname = "");
  
  ////////////////////////////////////////////
  // print()
  //
  // Print the current host info
  
  void print(ostream &out);
  
  ////////////////////////////////////////////
  // printHostent()
  //
  // Print a hostent struct
  
  static void printHostent(const struct hostent &hh, ostream &out);
  
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
  // this heuristcially, by comparing various strings etc.
  //
  // Thread safe
  //
  // Returns true or false
  
  bool hostIsLocal(const string &remote_hostname);

  ///////////////////////////////////////////////////////////
  // hostIsLocalStrict()
  // 
  // Performs a strict check on whether the hostname given is the
  // local host. It does this by comparing the IP addresses.
  //
  // Caution: not thread-safe, uses gethostbyname()
  //
  // Returns true or false
  
  bool hostIsLocalStrict(const string &hostname);

  ///////////////////////////////////////////////////////////
  // hostIsLocal2()
  // 
  // Firsts checks using hostIsLocal(). If this fails, checks
  // using hostIsLocalStrict().
  //
  // Caution: not thread-safe, uses gethostbyname()
  //
  // Returns true or false

  bool hostIsLocal2(const string &hostname);

  // access to data members
  
  const hostent_t &getHostent() { return (_hostent); }
  const string &getName() const { return (_name); }
  const string &getIpAddr() const { return (_ipAddr); }
  int getAddrType() const { return (_addrType); }
  int getAddrLen() const { return (_addrLen); }
  const char *getAddr() const { return (_addr); }
  const string &getErrorStr() const { return (_errorStr); }
  
protected:

  string _errorStr;

  // mutex to protect in case underlying functions are not
  // thread-safe

  static pthread_mutex_t _mutex;
  
  // hostent and buffer for re-entrant call

  static const int _hostBufSize = 8192;
  hostent_t _hostent;
  char _hostBuf[_hostBufSize];
  
  string _name;
  string _ipAddr;
  int _addrType;
  int _addrLen;

  static const int _maxAddrLen = 32;
  char _addr[_maxAddrLen];

  string _getUname();
  string _getLocalHostByName();
  string _loadIpAddr(const struct hostent &hh);
 
private:

};

#endif


