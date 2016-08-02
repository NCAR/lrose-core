//  rapicWDSSclient.h

//  rapic wdss client class - child class of ThreadObj
//  Provides a remote Wdss client functionality
//  
//  Can either be run as a thread which will update itself
//  or can be polled by a managing object calling workProc

#ifndef __RAPIC_WDSS_CLIENT_H
#define __RAPIC_WDSS_CLIENT_H 

#include "threadobj.h"
#include "spinlock.h"
#include "wdss/DsWdss.hh"
#ifdef STDCPPHEADERS
#include <string>
using namespace std;
#else
#include <string>
#endif

class rapicWDSSClient : public ThreadObj
{
 public:
  rapicWDSSClient(char *initstr = 0);
  ~rapicWDSSClient();
  void init(char *initstr);  // 
  void setWdssClientParams();
  bool checkWdssServer();
  bool readWdssServer();
  bool wdssServerChanged();  // 
  bool wdssServerPollIsDue();
  virtual void workProc();
  bool getLock();
  void relLock();
  bool valid();
  DsWdss *getWdssClient() { return wdssClient; };
  int getStn() { return stn; };
  float getThresh() { return thresh; };
  time_t getLatestScanTime() { return latestScanTime; };
 protected:
 private:
  DsWdss *wdssClient;
  spinlock *lock;
  string serverURL;
  time_t latestScanTime;
  time_t lastServerCheck;
  bool debug;
  time_t interval_start, interval_end;
  FILE *debugFile;
  time_t wdssServerPollPeriod; // time between wdss Server polls
  time_t nextWdssServerPollTime;   // time next wdss Server poll is due
  int stn;
  float thresh;
};

#endif
