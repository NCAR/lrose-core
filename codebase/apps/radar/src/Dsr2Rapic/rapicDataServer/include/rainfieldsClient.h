#ifndef __RAINFIELDSCLIENT_H__
#define __RAINFIELDSCLIENT_H__

#include <algorithm>
#include <iostream>
#include <deque>

#include "rainlib/types.hh"
#include "rainlib/Exceptions.hh"
#include "rainlib/Date.hh"
#include "MyRainfields.hh"
#include "siteinfo.h"
#include "rainfieldImg.h"
#include "spinlock.h"
#include "threadobj.h"
#include "rpEventTimer.h"
#include "comms.h"
#include "CImage.hh"

extern int defaultRfServerPort;
extern const char defaultRfInitFileName[];
extern const char defaultRfInitFileName2[];
extern FILE *rainfieldsLogFile;  // by default log output to stderr
       // if useRainfieldsLogFile in rainfieldsClient.ini - use file rainfieldsLogFile
extern bool rfBaseProdRainRate; // if true convert & display baseProd accum(mm) as rainrate(mm/hr)

void rfReportError(Rainfields& server, int server_result = 0);

class rfProjectionCache
{
public:
    rfProjectionCache();
    rfProjectionCache(Rainfields& server);
    ~rfProjectionCache();
    void setServer(Rainfields& server);
    /** This locates a projection object given its unique id.

	@param	id  	    The unique id.
	@return 0 if the object could not be found anywhere.
	@throw	Exception   These are server exceptions.
    */
    Ref<Projection>
    locateProjection(ID id)
	throw (::Exception);
protected:

    typedef Ref<Projection> Data;
    typedef map<ID, Data>   ProjnMap;

    Rainfields    *server_;
    ProjnMap	  projnMap_;
    
    spinlock      *lock;

};

class rfRequest
{
  string   dateL;
  char     *dateL_cstr;
  string   dateU;
  char     *dateU_cstr;
  ulong    interval;
  ulong    projnId;
  ulong    statnId;
  rf1__ImageDataType dataType;
  bool	   errorType;
  bool	   isCalib;
  ulong    resln;
  ulong    cappi;
  bool     clutter;
  bool     occultation;
  bool     attenuation;
  bool     hail;
  string   tag;
  char     *tag_cstr;
  string   user;
  char     *user_cstr;
  rf1__ImageKey	key;
 public:
  void     reset();  // reset the key back to defaults, i.e. ALL key pointers to NULL
  void     set_genDateL(time_t time);
  void     set_genDateU(time_t time);
  void     set_forDateL(time_t time);
  void     set_forDateU(time_t time);
  void     set_crDateL(time_t time);
  void     set_crDateU(time_t time);
  void     set_interval(ulong val);
  void     set_projnId(ulong val);
  void     set_statnId(ulong val);
  void     set_dataType(rf1__ImageDataType dt);
  void     set_errorType(bool errtype);
  void     set_isCalib(bool iscalib);
  void     set_resln(ulong val);
  void     set_cappi(ulong val);
  void     set_clutter(bool cltr);
  void     set_occultation(bool occul);
  void     set_attenuation(bool atten);
  void     set_hail(bool hl);
  void     set_tag(string tg);
  void     set_user(string usr);
  rf1__ImageKey* getkey() { return &key; };
};
  
class rfReqStatus
{
 public:
  rpEventTimer reqTimer;     // time req events
  bool    lastReqFailed;       // true if last req failed
  string  lastReqFailDesc;   // if failed, description of failure
  timeval lastReqSuccessTime;// time last successful req made
  time_t  lastPollTime;      // time of latest rainfall update 
  bool    lastPollSkipped;   // time of latest rainfall update
  time_t  lastPollChange;    // time latest rainfall update detected 
  time_t  lastPollInterval;  // interval of latest rainfall poll
  time_t  lastImageTime;     // gen time of latest rainfall image found 
  time_t  lastCrImageTime;   // cr time of latest rainfall image found
  time_t  baseImageInterval; // base interval given by latest base(no tag) rainfall image
  time_t  timeToNext;        // expected time to next product
  time_t  prevImageTime;     // time of latest rainfall image found 
  time_t  lastImgInterval;   // last interval of actual measured rf update 
  time_t  minImgInterval;    // min interval of rainfall update 
  time_t  maxImgInterval;    // max interval of rainfall update 
  //  int     reqCount;
  int     missedCount;       // count of missed products
  int     prodFoundCount;
  int     failCount;
  int     imgCount;
  int     imgMissedCount;     
  rfReqStatus()
    {
      timeval_set(lastReqSuccessTime);
      lastPollTime = lastPollChange = lastPollInterval = 
	timeToNext = 
	lastImageTime = lastCrImageTime = prevImageTime = 
	baseImageInterval = timeToNext = 
	lastImgInterval = minImgInterval = maxImgInterval = 0;
      lastReqFailed = false;
      prodFoundCount = failCount = missedCount = 0;
      lastPollSkipped = false;
      imgCount = imgMissedCount = 0;
      reqTimer.setLabel("rfReqStatus - reqTimer");
    };
  void dumpStatus(int stnid, FILE *statusfile = NULL);
  void dumpPollStatus(int stnid, FILE *statusfile = NULL, bool no_stnid = false);
  time_t timeToNextCr(time_t timenow = 0);  // seconds til next crTime - 0 if not defined yet
  int missedCountCr(time_t timenow = 0);  // number of producst missed based on last time and interval
  void startReq(char *_str = NULL);
  void endReq(bool prodfound, 
	      bool failed = false,
	      const char* failstr = NULL);
};

enum rfProdType { rfRainRate, rfAccum, rfAccumSince, rfAccumFcst };

class rainfallProduct
{
 public:
  string rainfallProdKey;
  rfProdType RFprodType;  // type of rainfall product
  int interval;      // period (mins) for accum & accumfcst, 
                     // mins since start of day (usually local time??) 
                     // for accumsince
  rainfallProduct();
  rainfallProduct(int _interval, rfProdType _prodType = rfAccum);
  string getRainfallProdKey(int _interval, rfProdType _prodType);
};

class rainfieldProdReq
{
 public:
  int stnid;
  time_t gentime;
  rfProdType prodtype;
  time_t interval;
  bool realtime;
  rainfieldProdReq(int _stnid, time_t _gentime, 
		   rfProdType _prodtype, time_t _interval,
		   bool _realtime) :
    stnid(_stnid), gentime(_gentime), 
    prodtype(_prodtype), interval(_interval), 
    realtime(_realtime)
    {
    };
  rainfieldProdReq() :
    stnid(0), gentime(0), 
    prodtype(rfAccum), interval(3600), 
    realtime(true)
    {
    };
};

// request specified products from server
class rainfieldImgReq
{
 public:
  int stnid;
  time_t starttime;
  time_t endtime;
  rf1__ImageDataType dataType;
  time_t interval;
  e_rf_data_mode data_mode;
  
  rainfieldImgReq(int _stnid, time_t _time1, time_t _time2,
		  bool realtime = true,
		  rf1__ImageDataType _dataType = Rainfall,
		  time_t _interval = 600) :
    
    stnid(_stnid),
    starttime(_time1),
    endtime(_time2),
    dataType(_dataType),
    interval(_interval)
    {
      if (realtime)
	data_mode = RF_REALTIME;
      else
	data_mode = RF_DB;
    };
  rainfieldImgReq() :
    stnid(0),
    starttime(0),
    endtime(0),    
    dataType(Rainfall),
    interval(interval)
    {      
      data_mode = RF_REALTIME;
    };
};

extern bool useGlobalRFLoadImgLock;
extern spinlock *globalRFLoadImgLock;
inline bool getGlobalRFLoadImgLock()
{
  if (useGlobalRFLoadImgLock && !globalRFLoadImgLock)
    globalRFLoadImgLock = new spinlock("globalRFLoadImgLock", float(10.0));
  if (useGlobalRFLoadImgLock && globalRFLoadImgLock)
    return globalRFLoadImgLock->get_lock();
  else 
    return true;  // if not using lock always return true
};

inline void relGlobalRFLoadImgLock()
{
  if (useGlobalRFLoadImgLock && globalRFLoadImgLock)
    globalRFLoadImgLock->rel_lock();
};

class rfServer : public ThreadObj
{
 public:
  
  /*  timeval lastReqTime;         // time last req made
  float   lastReqRespondTime;  // time server took to respond
  int     totalReqs;
  */
  rpEventTimer reqTimer;
  rpEventTimer getStnsTimer;
  bool    lastReqFailed;       // true if last req failed
  string  lastReqFailDesc;     // if failed, description of failure
  timeval lastReqSuccessTime;  // time last successful req made
  int     prodFoundCount;
  int     failCount;
  int     maxSeqLoadMins;      // limit max time depth for loads(mins)
  stnSet  stnsAvail;           // list of available stations
  rfRequest rfReq;
  map<int, rfReqStatus> reqStatus; // map by stnid
  time_t  lastPollTime;
  time_t  lastPollChange;
  time_t  lastImageTime;
  void    setTimeouts(int timeoutsecs)
    {
      recv_timeout = timeoutsecs;
      send_timeout = timeoutsecs;
      connect_timeout = timeoutsecs;
      accept_timeout = timeoutsecs;
    } 
  void    setRecvTimeout(int timeoutsecs) 
    { recv_timeout = timeoutsecs; };
  void    setSendTimeout(int timeoutsecs)
    { send_timeout = timeoutsecs; };
  void    setConnectTimeout(int timeoutsecs)
    { connect_timeout = timeoutsecs; };
  void    setAcceptTimeout(int timeoutsecs)
    { accept_timeout = timeoutsecs; };
  int     recv_timeout;
  int     send_timeout;
  int     connect_timeout;
  int     accept_timeout;
  int     port;
  float   pxlScaleFactor;  // default products from rainfields server are in 0.1mm units
  Rainfields server;
  bool    serverAvail;
  string  serverName;
  string  resolvedServerName;
  bool    usesRainRates; // if true, all accums in rainrate(mm/hr), base-prod in mm
  rfProjectionCache projCache; // per server projection cache
  rfServer(string host, int _port = 0);
  rfServer(char *initstr);
  ~rfServer();
  void init();
  void init(char *initstr);
  void setEndPoint(string host, int _port = 0);
 
  int  getStnCount(int stn, time_t time1 = 0, time_t time2 = 0, rf1__ImageDataType dataType = Rainfall);
  time_t checkTime;
  int checkPeriod;
  time_t pollTime;
  int pollPeriod;
  void workProc();
  void checkServer();  // periodically check server for stn lists etc.
  bool pollServer(stnSet *pollstns = NULL,
		  stnSetRefCount *rainrateloadstns = NULL);   
  bool pollServer(int stnid, bool loadrainrate = true);   // poll servers for new products for defined stn
  void getStnsAvail(time_t time1 = 0, time_t time2 = 0);
  void dumpStatus(FILE *statusfile = NULL, bool detailed = true);
  void dumpPollStatus(FILE *statusfile = NULL, bool detailed = true);
  // this fetches a complete rf1__Image including data for the given product id
  // rfimg is modified to point to the new instance
  bool getImgByID(unsigned long id, int stnid, rf1__Image* &rfimg);
  bool getImgsBtwn(int stnid, time_t time1, time_t time2, 
		   rf1__ImageDataType dataType,
		   vector<CImage>& cimages);
  // This returns meta-data only in result, i.e. not actual rlz data
  bool getImgsBtwn(int stnid, time_t time1, time_t time2, 
		   rf1__ImageDataType dataType,
		   rf1__ImageResult& result);

  // This will create rainfieldImg instances for each resultant image
  // return number of images loaded and passed to scanmng
  bool loadRfImg(int stnid, rf1__Image& rfimg, bool realtime = true);
  int loadImgsResult(int stnid, rf1__ImageResult& result, bool realtime = true);
  int loadImgsBtwn(int stnid, time_t time1, time_t time2, 
		   bool realtime = true,
		   rf1__ImageDataType dataType = Rainfall,
		   time_t interval = 600);

  bool getImgClosest(int stnid, time_t imgtime, time_t window, 
		     rf1__ImageDataType dataType,
		     Ref<rainfieldImg>& rfimg);

  bool getAccumImg(int stnid,
		   time_t gentime,         // usually genDate from baserfimg
		   time_t accumperiod, 
		   bool loadimg = true,    // if loadimg, load image & pass to scanmng
		   bool realtime = true,
		   bool uselock = true);  // this may be called when lock already locked

  bool getAccumImg(rainfieldImg &baserfimg,
		   time_t accumperiod, 
		   bool loadimg = true,    // if loadimg, load image & pass to scanmng
		   bool realtime = true,
		   bool uselock = true);  // this may be called when lock already locked
  void checkAddProdReq(rainfieldImg &baserfimg,
		       bool realtime);    // check whether to make prods for img
  void checkProdReqs();                   // check the prod req queue
  bool handleProdReq(rainfieldProdReq& prodreq); // make product create request
  deque<rainfieldProdReq> prodReq; // queue of product requests
  stnSet accumProdStns;
  
  bool rainrateLoadStn(int stn, stnSetRefCount *rainrateloadstns = NULL);
  bool rainrateLoadStnCount(stnSetRefCount *rainrateloadstns = NULL);

  // if pollstns not defined poll all known
  // load new realtime images as rainfieldImg and show to ScanMng
  bool pollRealTime(stnSet *pollstns = NULL,
		    stnSetRefCount *rainrateloadstns = NULL); 
 
  time_t lastPollChanged(int stnid); // return last rainfall product poll change time
  time_t lastCrTime(int stnid); // return last rainfall product Cr Time, end of vol
  time_t lastGenTime(int stnid); // return last rainfall product Gen Time, start of vol
  void startReq(char *_str = NULL);
  void endReq(bool prodfound, 
	      bool failed = false, 
	      const char* failstr = NULL);
  spinlock *lock;
  bool getLock(char *lockstr);
  void relLock();
  bool deleted;
  string rfServerAuthUser;
  string rfServerAuthPwd;
};


class rainfieldsMngr : public ThreadObj
{
  time_t checkServersTime;
  int checkServersPeriod;
  time_t pollTime;
  time_t  lastPollChange;
  int pollPeriod;
  time_t checkReqTime;
  int checkReqPeriod;
  int maxSeqLoadMins;                // limit max time depth for loads(mins)
  rpEventTimer pollServerTimer;      // time per server poll calls
  rpEventTimer getStnsServerTimer;   // time per server getStns calls
  rpEventTimer pollTimer;            // time overall poll calls
  rpEventTimer getStnsTimer;         // time overall getStns calls
  spinlock *lock;                    // main lock
  spinlock *quicklock;               // lock for quick actions
  bool initdone;
  string initfilename;
  bool singleThreaded;  // if singleThreaded use polling for all servers
                        // otherwise rfServer entries will run their own thread
  stnSetRefCount rainRateLoadStns;
  deque<rainfieldImgReq> loadReq; // queue of load requests
  map<string, rfServer*> servers; // map of servers, by servername
  time_t purgeUnrefStnDelay;
  map<int, time_t> purgeUnrefStnTimes; // map of times to purge deref'd 
                                        // stn's images

  stnSetRefCount* getRainRateLoadStns()
    {
      return &rainRateLoadStns;
    }

  
 public:
  void addServer(string servername, int port = 0);  // add a server to the map
  void addServer(char *initstr);  // add a server to the map
  void removeServer(string servername);  // remove server from the map
  void removeAllServers();  // remove server from the map
  void startServer(string servername);  // remove server from the map
  void startAllServers();  // remove server from the map
  void stopServer(string servername);  // remove server from the map
  void stopAllServers();  // remove server from the map
  void close();
  void init(string rfinitfilename);
  /*
  bool getRainfallImgsBtwn(int stnid, time_t time1, time_t time2, 
		      Ref<rainfieldImg>& rfimg);
  */
  bool getRainfallImgClosest(int stnid, time_t imgtime, time_t window, 
		      Ref<rainfieldImg>& rfimg);
  bool getAccumImg(int stnid, time_t imgtime, time_t window, 
		      Ref<rainfieldImg>& rfimg);

  // This will create rainfieldImg instances for each resultant image
  // return number of images loaded and passed to scanmng
  int loadImgsBtwn(int stnid, time_t time1, time_t time2, 
		   bool realtime = true,
		   rf1__ImageDataType dataType = Rainfall,
		   time_t interval = 600);
  int loadImgs(rainfieldImgReq req);

  // the following place a request to load rainfield images
  void reqLoadImgsBtwn(int stnid, time_t time1, time_t time2,
		       bool realtime = true,
		       rf1__ImageDataType dataType = Rainfall,
		       time_t interval = 600);
  void reqLoadImgs(rainfieldImgReq req);
  void checkLoadReq();

  void workProc();
  void checkServers();  // periodically check servers for stn lists etc.
  // poll servers for new products for defined pollStns
  // return true if something did update
  void dumpPollStatus(FILE *statusfile = NULL);
  bool pollServers(stnSet *pollStns = NULL,
		   stnSetRefCount *rainrateloadstns = NULL);   
  bool pollServers(int stnid, bool loadrainrate = true);   // poll servers for new products for defined stn

  stnSet  pollStns;     // stns to be polled for new products
  bool addRainRateLoadStn(int stnid);   // return true if it is a new stn

  map<string, rainfallProduct> rfProdOptions; 
  // rainfield product requests to allow
  void addRfProdOption(int _interval, rfProdType _prodType = rfAccum);
  void checkStnPurgeTimes();  // check for deref'd stns to purge
  // schedule removal of stn, not actually performed for purgeUnrefStnDelay
  // seconds to allow for 
  bool removeRainRateLoadStnRef(int stnid, time_t delay = -1); 
  void doRemoveRainRateLoadStn(int stnid);// return true if removed last ref
  stnSet  stnsAvail;    // list of available stations
  bool stnAvail(int stnid);
  bool stnRainRateLoad(int stnid); //return true if stn is ref'd, or 
                                   // still scheduled for purging 
  int  stnRainRateLoadCount(); // return count of stns to load rain rates

  void newSeqLoaded(); // if a new seqence has been loaded, fetch relevant data

  rainfieldsMngr();
  rainfieldsMngr(string rfinitfilename);
  ~rainfieldsMngr();
};  

extern rainfieldsMngr *rainfieldsMgr;
  
#endif
