/*  rapicWDSSClientMng.h */

/*  This class is responsible for managing rapicWDSSClient instances */


/*
  Loads configuration from rapicWDSSClient.ini
  
  rapicWDSSClient.ini contains the URLs or file paths of the WDSS data
  Creates an instance of rapicWDSSClient for each URL
  
  Lines starting with # will be ignored

  stn=nn thresh=tdbz url=urlstring [pollperiod=pollsecs] [debug]


*/

#ifndef __RAPIC_WDSS_CLIENT_MNG_H
#define __RAPIC_WDSS_CLIENT_MNG_H 

#include "rapicWDSSClient.h"

typedef vector<rapicWDSSClient*>::iterator tCI;


class rapicWDSSClientMng
{
 public:
  rapicWDSSClientMng(char *initfname = 0);
  ~rapicWDSSClientMng();
  void init(char *initfname);
  void reload();
  string initFname;
  void addNewClient(char *newclientinitstr); // create a new rapicWDSSClient, init with str
  void addNewClient(rapicWDSSClient *newclient);
  void clearAllClients();
  void clearClient(rapicWDSSClient *clearclient);
  void startAllClients();
  void startClient(rapicWDSSClient *startclient);
  void stopAllClients(bool nonBlocking = true); // by default make non blocking call to stop clients
  void stopClient(rapicWDSSClient *stopclient, bool nonBlocking = true);
  void check();
  time_t checkTime() { return nextCheckTime; };
  tCI firstClient(int stn = 0, float thresh = 0);
  tCI nextClient(tCI thisiter, int stn = 0, float thresh = 0);
  tCI endClient() { return wdssClients.end(); };
 protected:
 private:
  time_t nextCheckTime;
  time_t checkPeriod;
  bool titanIntervalMode;

  vector<rapicWDSSClient*> wdssClients;
};

extern rapicWDSSClientMng *rapicWDSSClientMgr;

#endif
