#ifndef	__IPNAMERESCACHE_H
#define __IPNAMERESCACHE_H

#include <string>
#include <map>
typedef unsigned char uchar;
#include <netdb.h>
#include "spinlock.h"

class ipAddrByName
{
 public:
  bool debug;
  std::string hostname;
  std::string inAddrString;  // numeric address string in nn.nn.nn.nn form
  addrinfo    *_addrinfo;
  in_addr     inaddr;
  time_t      lastResolvedTime,   // use as valid flag, if time=0 not valid
    nextResolveTime;
  int         refreshPeriod, failedRefreshPeriod, 
    lookups, lookupFails;
  bool        resolveName(std::string hname);
  bool        resolveName();
  bool        resolvedOK() { return lastResolvedTime != 0; };
  void        clear();
  bool        check(); // return true if refresh occurred
  ipAddrByName();
  ipAddrByName(std::string hname);
  ~ipAddrByName();
};

class ipNameByAddr
{
 public:
  std::string addrstring;
  addrinfo    *_addrinfo;
  in_addr     inaddr;
  time_t      lastResolvedTime,   // use as valid flag, if time=0 not valid
    nextResolveTime;
  int         refreshPeriod, lookups, lookupFails;
  bool        resolveAddr(std::string aname);
  bool        resolveAddr();
  void        clear();
  void        check();
  ipNameByAddr();
  ipNameByAddr(std::string hname);
  ~ipNameByAddr();
};

extern bool useIPNameResCache;


class ipNameResCache
{
  spinlock *lock;
  int  abnRequests, abnLookups, abnFails;
 public:
  std::map<std::string, ipAddrByName> abn_cache;  // address by name cache
  bool getAddrByName(const char *name, char *in_addr_str, in_addr *rethostaddr);  
  bool getAddrByName(const char *name, in_addr *rethostaddr);  
  bool getAddrByName(const char *name, char *in_addr_str);  
  bool getAddrByName(const char *name, std::string& in_addr_str);  
  std::map<std::string, ipNameByAddr> nba_cache;  // name by address cache
  bool getNameByAddr(const void *addr, char *retname, int maxchars);
  ipNameResCache();
  ~ipNameResCache();
  bool debug;
  int  getAbnRequestCount() { return abnRequests; };
  int  getAbnLookupCount() { return abnLookups; };
  int  getAbnRefreshCount() { return abnFails; };
  void dumpStatus(FILE *file);
};
  
extern ipNameResCache IPNameResCache;

#endif
