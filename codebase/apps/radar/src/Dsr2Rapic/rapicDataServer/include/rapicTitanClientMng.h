/*  rapicTitanClientMng.h */

/*  This class is responsible for managing rapicTitanClient instances */


/*
  Loads configuration from rapicTitanClient.ini
  
  rapicTitanClient.ini contains the URLs or file paths of the titan data
  Creates an instance of rapicTitanClient for each URL
  
  Lines starting with # will be ignored

  stn=nn thresh=tdbz url=urlstring [pollperiod=pollsecs] [debug]


*/

#ifndef __RAPIC_TITAN_CLIENT_MNG_H
#define __RAPIC_TITAN_CLIENT_MNG_H 

#include "rapicTitanClient.h"
#include <list>

typedef list<rapicTitanClient*>::iterator tCI;

bool toggleTitanIntervalMode();
bool setTitanRealTimeMode();

class rapicTitanClientMng
{
 public:
  rapicTitanClientMng(char *initfname = 0);
  ~rapicTitanClientMng();
  void init(const char *initfname);
  void reload();
  rapicTitanClient* addNewClient(char *newclientinitstr); // create a new rapicTitanClient, init with str
  void addNewClient(rapicTitanClient *newclient);
  void clearAllClients(bool quitting = false); // if quitting, force immediate glblock timeout
  void clearClient(rapicTitanClient *clearclient);
  void startAllClients();
  void startClient(rapicTitanClient *startclient);
  void stopAllClients(bool nonBlocking = true); // by default make non blocking call to stop clients
  void stopClient(rapicTitanClient *stopclient, bool nonBlocking = true);
  void checkAllClients();  // force all clients to do a check straight away
  void check();
  time_t checkTime() { return nextCheckTime; };
  tCI firstClient(int stn = 0, float thresh = 0, bool area_flag = false);
  tCI nextClient(tCI thisiter, int stn = 0, float thresh = 0, bool area_flag = false);
  tCI endClient() { return titanClients.end(); };
  rapicTitanClient* getLatestMatchingClient(int stn, float thresh, bool area_flag = false);
  bool getMatchingClients(list<rapicTitanClient*> &titanclients, int stn, float thresh = 0, bool area_flag = false);
  bool toggleTitanIntervalMode();  // returns new state of titanIntervalMode
  void setRealTimeMode(bool state = true);
  bool getRealTimeMode() { return realTimeMode; };
  string initFname;
 protected:
 private:
  time_t nextCheckTime;
  int checkPeriod;
  time_t nextStatusTime;
  int statusPeriod;
  bool useGlbLock; // rapicTitanClient defaults to using 
  bool titanIntervalMode;
  bool realTimeMode;

  list<rapicTitanClient*> titanClients;
  void dumpStatus(FILE *statusfile = NULL);
};

extern rapicTitanClientMng *rapicTitanClientMgr;

#endif
