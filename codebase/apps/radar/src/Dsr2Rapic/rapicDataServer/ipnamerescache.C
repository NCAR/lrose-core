/*

  ipnamerescache.C
  
  Implements the ipAddrByName, ipNameByAddr & ipNameResCache classes

*/

#include "ipnamerescache.h"

ipNameResCache IPNameResCache;
bool useIPNameResCache = false;

ipAddrByName::ipAddrByName(std::string hname) :
  debug(false),
  hostname(hname),
  _addrinfo(NULL),
  lastResolvedTime(0),
  nextResolveTime(0),
  refreshPeriod(3600),
  failedRefreshPeriod(300),
  lookups(0),
  lookupFails(0)
{
  if (hostname.size())
    resolveName(hostname);
}

ipAddrByName::ipAddrByName() :
  debug(false),
  _addrinfo(NULL),
  lastResolvedTime(0),
  nextResolveTime(0),
  refreshPeriod(3600),
  failedRefreshPeriod(300),
  lookups(0),
  lookupFails(0)
{}

ipAddrByName::~ipAddrByName()
{
  clear();
}

void ipAddrByName::clear()
{
  if (_addrinfo != NULL)
    {
      freeaddrinfo(_addrinfo);
      _addrinfo = NULL;
    }
  lastResolvedTime = nextResolveTime = 0;
}

bool ipAddrByName::check()  // return whether lookup occurred, even if unsuccessful
{
  if ((time(0) > nextResolveTime) &&
      hostname.size())
    {
      resolveName();
      return true;    // true if lookup occurred
    }
  else
    return false;
}

bool ipAddrByName::resolveName()
{
  return resolveName("");
}

bool ipAddrByName::resolveName(std::string hname)
{  
  addrinfo aiHints, *this_ai;
  in_addr inAddr;
  uchar *ucharptr;
  char addrstr[128];

  memset(&aiHints, 0, sizeof(aiHints));
  aiHints.ai_family = AF_INET;
  aiHints.ai_socktype = SOCK_STREAM;
  aiHints.ai_protocol = IPPROTO_TCP;
  aiHints.ai_flags = AI_CANONNAME;

  if (hname.size())
    hostname = hname;
  if (_addrinfo != NULL)
    clear();
  int result = 0;
  lookups++;
  if ((result = getaddrinfo(hostname.c_str(), NULL, NULL, &_addrinfo)) != 0)
    {
      if (debug)
	fprintf(stderr, "nameByAddr::resolveName -  getaddrinfo failed for %s - %s\n", 
		hostname.c_str(), gai_strerror(result));
      lastResolvedTime = 0;
      inAddrString.clear();
      nextResolveTime = time(0) + failedRefreshPeriod;
      lookupFails++;
      return false;
    }
  else
    {
      lastResolvedTime = time(0);
      nextResolveTime = lastResolvedTime + refreshPeriod;
      this_ai = _addrinfo;
      if (debug)
	fprintf(stdout, "ipAddrByName::resolveName succeeded for %s\n",
		hostname.c_str());
      int count = 1;
      while (this_ai)
	{
	  if (count == 1)  // use first address for this instance
	    {
	      memcpy(&inaddr, &(((sockaddr_in *)(this_ai->ai_addr))->sin_addr), sizeof(in_addr));
	      ucharptr = (uchar *)&inaddr;
	      sprintf(addrstr, "%d.%d.%d.%d",
		      ucharptr[0], ucharptr[1], ucharptr[2], ucharptr[3]);
	      inAddrString = addrstr;
	    }      
	  memcpy(&inAddr, &(((sockaddr_in *)(this_ai->ai_addr))->sin_addr), sizeof(in_addr));
	  ucharptr = (uchar *)&inAddr;
	  if (debug)
	    {
	      fprintf(stdout, "ia%d addlen=%d, name=%s %d.%d.%d.%d\n",
		      count, this_ai->ai_addrlen, this_ai->ai_canonname, 
		      ucharptr[0], ucharptr[1], ucharptr[2], ucharptr[3]);
	    }
	  this_ai = this_ai->ai_next;
	  count++;
	}
      return true;
    }
}
    
ipNameByAddr::ipNameByAddr(std::string aname) :
  addrstring(aname),
  _addrinfo(NULL),
  lastResolvedTime(0),
  nextResolveTime(0),
  refreshPeriod(3600),
  lookups(0),
  lookupFails(0)
{
  if (addrstring.size())
    resolveAddr(addrstring);
}

ipNameByAddr::ipNameByAddr() :
  _addrinfo(NULL),
  lastResolvedTime(0),
  nextResolveTime(0),
  refreshPeriod(3600),
  lookups(0),
  lookupFails(0)
{}

ipNameByAddr::~ipNameByAddr()
{
  clear();
}

void ipNameByAddr::clear()
{
  if (_addrinfo != NULL)
    {
      freeaddrinfo(_addrinfo);
      _addrinfo = NULL;
    }
  lastResolvedTime = nextResolveTime = 0;
}

void ipNameByAddr::check()
{
  if ((time(0) > nextResolveTime) &&
      addrstring.size())
    {
      if (!resolveAddr())
	nextResolveTime = nextResolveTime + refreshPeriod/2;
    }
}

bool ipNameByAddr::resolveAddr()
{
  return resolveAddr("");
}

bool ipNameByAddr::resolveAddr(std::string aname)
{  
  if (aname.size())
    addrstring = aname;
  if (_addrinfo != NULL)
    clear();
  int result = 0;
  if ((result = getaddrinfo(addrstring.c_str(), NULL, NULL, &_addrinfo)) != 0)
    {
      fprintf(stderr, "nameByAddr::resolveAddr -  getaddrinfo failed for %s - %s\n", 
	      addrstring.c_str(), gai_strerror(result));
      lastResolvedTime = nextResolveTime = 0;
      return false;
    }
  else
    {
      lastResolvedTime = time(0);
      nextResolveTime = lastResolvedTime + refreshPeriod;
      return true;
    }      
}
    
ipNameResCache::ipNameResCache() :
  debug(false)
{
  lock = new spinlock("addrByNameCache", float(30.0));
}

ipNameResCache::~ipNameResCache()
{
  if (lock)
    delete lock;
}

bool ipNameResCache::getAddrByName(const char *name, in_addr *rethostaddr)
{
  return getAddrByName(name, NULL, rethostaddr);
}

bool ipNameResCache::getAddrByName(const char *name, char *in_addr_str)
{
  return getAddrByName(name, in_addr_str, NULL);
}

bool ipNameResCache::getAddrByName(const char *name, std::string& in_addr_str)
{
  char tempstr[64];
  bool result = getAddrByName(name, tempstr, NULL);
  if (result)
    in_addr_str = tempstr;
  else
    in_addr_str.clear();
  return result;
}  

bool ipNameResCache::getAddrByName(const char *name, char *in_addr_str, in_addr *rethostaddr)
{
  if (lock && !lock->get_lock()) // if can't get lock return false
    return false;
  bool result = false;
  std::map<std::string, ipAddrByName>::iterator iter;
  std::map<std::string, ipAddrByName>::iterator iterend = abn_cache.end();
  abnRequests++;
  iter = abn_cache.find(name);
  if (iter != iterend)   // found name in cache, use it
    {
      if (iter->second.check())   // check for cache refresh first, 
	{
	  abnLookups++;
	  if (!iter->second.resolvedOK())
	    abnFails++;
	}
      if (iter->second.resolvedOK()) // 
	{
	  if (rethostaddr)
	    memcpy(rethostaddr, &(iter->second.inaddr), sizeof(in_addr));
	  if (in_addr_str)
	    strcpy(in_addr_str, iter->second.inAddrString.c_str());
	  if (debug)
	    {
	      fprintf(stdout, "ipNameResCache::getAddrByName - %s (%s) found in cache\n", 
		      iter->second.hostname.c_str(), 
		      iter->second.inAddrString.c_str());
	    }
	  result = true;
	}
      else
	{
	  if (rethostaddr)
	    memset(rethostaddr, 0, sizeof(in_addr));
	  if (in_addr_str)
	    strcpy(in_addr_str, "");
	  result = false;
	}
    }
  else
    {
      abnLookups++;
      result = abn_cache[name].resolveName(name);
      if (result)
	{
	  if (rethostaddr)
	    memcpy(rethostaddr, &(abn_cache[name].inaddr), sizeof(in_addr));
	  if (in_addr_str)
	    strcpy(in_addr_str, abn_cache[name].inAddrString.c_str());
	}
      else
	{
	  if (rethostaddr)
	    memset(rethostaddr, 0, sizeof(in_addr));
	  if (in_addr_str)
	    strcpy(in_addr_str, "");
	  abnFails++;	
	}
      if (debug)
	{
	  fprintf(stdout, "ipNameResCache::getAddrByName - New name inserted into cache - %s\n",
		  name);
	  if (result)
	    fprintf(stdout, " Resolved OK (%s)\n", 
		    abn_cache[name].inAddrString.c_str());
	  else
	    fprintf(stdout, " Resolve Failed\n");
	}
    }
  if (lock)
    lock->rel_lock();
  return result;
}

void ipNameResCache::dumpStatus(FILE *file)
{
  if (!file)
    file = stdout;
  fprintf(file, "ipNameResCache status - AddressByName entries=%d\n"
	  "abnRequests=%d abnLookups=%d abnFails=%d\n",
	  int(abn_cache.size()), abnRequests, abnLookups, abnFails);
}

