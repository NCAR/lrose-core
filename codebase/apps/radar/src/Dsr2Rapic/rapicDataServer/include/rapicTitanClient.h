//  titanclient.h

//  rapic titan client class - child class of ThreadObj
//  Provides a remote Titan client functionality
//  
//  Can either be run as a thread which will update itself
//  or can be polled by a managing object calling workProc

#ifndef __RAPIC_TITAN_CLIENT_H
#define __RAPIC_TITAN_CLIENT_H 

// #include "rapicTitanClientArgs.h"
#ifdef STDCPPHEADERS
#include <string>
#include <vector>
#include <map>
using namespace std;
#else
#include <string>
#endif
#include "threadobj.h"
#include "spinlock.h"
#include "rpEventTimer.h"
#include "titan/DsTitan.hh"
#include "dsserver/DsLocator.hh"

extern spinlock *glbRapicTitanClientLock;
extern int rapicTitanClientCount;

/* stl map of pointers to titan cells for given time, indexed by cellid */
// not thread safe, assumes use from main rendering thread only
class titanCellMap 
{
 public:
  time_t cell_time;
  float lastlat, lastlong, lastdist;
  TitanTrackEntry *lastCell;
  map<int, TitanTrackEntry*> cellMap;
  titanCellMap() { clear(); };
  void addCell(TitanTrackEntry *newcell);
  void clear();
  void newTime(time_t newtime) { clear(); cell_time = newtime; };
  TitanTrackEntry *getNearestCell(float lat, float lng, 
				  float &retdist, 
				  float max_dist_to_nearest_cell = -1);
};

class titanStnURL {
 private:
  time_t _latestScanTime;
  time_t _lastServerCheck;
  time_t _nextScanExpectedTime;
  time_t _nextScanOverdueTime;
  bool   useResolvedUrl;
  string serverURL();
  string rawURL;
  string resolvedURL; // hostname replaced by resolved numeric
  string UrlPrefix;
  string UrlHostName;
  string UrlResolvedHostName;
  string UrlPath;
  bool resolveURL();   // resolve URL using cached name service
 public:
  titanStnURL(char *initstr = NULL);
  void set(char *initstr);
  void set(int _stn, float _thresh, char *_urlstring, bool area_flag = false);
  rpEventTimer reqTimer;
  rpEventTimer pingTimer;
  bool inUse;
  string getNamedServerURL();
  string getResolvedServerURL();
  bool URL_OK; 
  bool useInterval;   // true if url supports interval mode loads
  bool useCompression;   // true if url supports compression mode loads
  int stn;
  int area;      // for titan composites, use comp_area intead of stn
  float thresh;
  void setLatestScanTime(time_t scantime);
  void setLastServerCheck(time_t checktime);
  time_t latestScanTime() { return _latestScanTime; };
  time_t lastServerCheck() { return _lastServerCheck; };
  time_t dataStartTime();
  time_t dataEndTime();
  time_t nextScanExpectedTime() { return _nextScanExpectedTime; };
  time_t nextScanOverdueTime();
  rpEventTimer scanInterval;  // scan product interval event timer, track max/min/avg/last
  rpEventTimer prodDelay;     // product available delay, i.e. delay to get product from server
  bool useSmartPoll;          // smart polling will delay polling until a new product is due
  DsTitan *currentTitanClient, 
    *tempTitanClient;         // main titan client, maintains track lists
  DsTitan *titanCheckClient;    // client to check for update time of titan on server
  void dumpStatus(FILE *statusfile);
  void dumpSummary(FILE *statusfile);
  /*
  bool smartPollIsDue(time_t &nextpolltime,    // if poll is due, also update next poll time
		      time_t timenow = 0); // return if url should be polled
  time_t smartPollDueTime(time_t &nextpolltime,// if poll is due, also update next poll time
			  time_t timenow); // return time next product is due
  */
  time_t nextSmartPollTime(time_t timenow, 
			   time_t pollperiod);   // scan interval based polling interval
};

class rapicTitanClient : public ThreadObj
{
 public:
  rapicTitanClient(char *initstr = NULL);
  ~rapicTitanClient();
  void init(char *initstr);  // 
  void setTitanClientParams(DsTitan *titanclient);
  titanStnURL singleURL;
  map <string, titanStnURL*> stnURLMap;
  bool checkTitanServer();
  bool readTitanServer();
  bool titanServerChanged();  // 
  bool titanServerPollIsDue();
  void setTitanServerPollTime(time_t polltime = 0);
  time_t getLatestScanTime();
  time_t dataStartTime();
  time_t dataEndTime();
  virtual void workProc();
  rpEventTimer getGlbLockTimer;
  rpEventTimer holdGlbLockTimer;
  bool getGlbLock(const char *holder);
  float relGlbLock();       // return number of seconds lock was held
  float glbLockWarnTime;
  int getGlbLockFails;
  char* getGlbLockHolder();
  bool getLock();
  float relLock();
  bool valid();
  DsTitan *getTitanClient() { return titanClient(); };
  int getStn() { return singleURL.stn; };
  int getArea() { return singleURL.area; };
  float getThresh() { return singleURL.thresh; };
  void setRealTimeMode(bool state = true);
  bool getRealTimeMode() { return realTimeMode; };
  void setIntervalMode(bool state);
  bool useIntervalMode;
  void setDebug(bool state = true) { debug = state; };
  bool  getDebug() { return debug; };
  time_t intervalStartTime, intervalEndTime;
  bool realTimeMode;
  int clientID;
  string getNamedServerURL();
  string getResolvedServerURL();
  DsTitan *titanClient();
  // keep track of server response times
  timeval lastReqTime;         // time last req made
  float   lastReqRespondTime;  // time server took to respond
  bool    lastReqFailed;       // true if last req failed
  string  lastErrorString;
  timeval lastReqSuccessTime;  // time last successful req made
  void dumpStatus(FILE *statusfile);
  void dumpSummary(FILE *statusfile);
  void setUseGlbLock(bool state) { useGlbLock = state; };
 protected:
 private:
  spinlock *lock;
  bool debug;
  FILE *debugFile;
  time_t titanServerPollPeriod; // time between titan Server polls
  time_t titanServerPollPeriod_URL_Failed; // time between titan Server polls if URL read failed
  time_t nextTitanServerPollTime;   // time next titan Server poll is due
  //  rapicTitanClientArgs args;
  bool useGlbLock;    // use of global lock wil serialise all titanClient read operations
  // currently suspect titanclient is not totally thread safe, so serialise by default
};

#endif
