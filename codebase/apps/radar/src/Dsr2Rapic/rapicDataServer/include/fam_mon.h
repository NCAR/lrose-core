/*

  fam_mon.h
  
  Fam provides event driven file monitoring functionality
  The classes here are provided to make use of the fam infra-structure
  more convenient.

*/

#ifndef __FAM_MON__H
#define __FAM_MON__H

#include <fam.h>
#include <vector>
#include <string>
#include <bitset>
#include <map>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "rpEventTimer.h"
#ifndef COMPILE_FAM_MON_MAIN
#include "fileReader.h"
#endif

extern const char *famevent[];

typedef std::bitset<32> _event_mask;

const char *fam_event_name(int code);
int fam_event_by_name(char *eventstr);
int fam_event_bit(int code);
void fam_decodeEventsStr(char *eventsstr, _event_mask &event_mask);
void fam_encodeEventsStr(std::string &eventmaskstr, _event_mask &event_mask);


class fileEventObj {
 public:
  std::string eventFilename;
  time_t eventTime;
  time_t doEventTime;
  time_t doEventDelay;
  bool eventMapped;
  bool eventDone;
  virtual void resetDoEventTime(time_t timenow = 0) // set timeout to do event
    {
      if (!timenow) timenow = time(0);
      doEventTime = timenow + doEventDelay;
      eventDone = false;
    }
  virtual bool checkDoEventTime(time_t timenow)
    {
      bool doeventtime = false;
      if (!eventDone && doEventTime)
	{
	  if (!timenow) timenow = time(0);
	  doeventtime = timenow >= doEventTime;
	}
      return doeventtime;
    } 
  fileEventObj()
    {
      eventTime = doEventTime = 0;
      doEventDelay = -1;
      eventDone = true;
      eventMapped = false;
    };
  virtual ~fileEventObj() {};
};

class famEventObj : public fileEventObj {
 public:
  FAMCodes eventCode;
  famEventObj() : fileEventObj()
    {
      eventCode = FAMCodes(0);
    };
};

enum fileReqType {FRO_FAM, FRO_URL, FRO_LTNING, FRO_UNDEFINED};

class fileReqObj
{
 public:
  fileReqType froType;
  int fro_id;
  bool isDir;
  bool checkIsDir;
  std::string reqFName;  // request file name
  std::string realFName; // real file name
  std::string eventExecStr;
  int doEventDelay;
  time_t doEventTime;
  bool   _eventDone;
  time_t lastEventTime;
  time_t timeout;
  time_t nextTimeoutTime;
  time_t firstTimeoutTime;
  bool isTimedOut;
  // firstTimeoutExecStr used for first timeout
  // if defined, reTimeoutExecStr used for first timeout
  //   else firstTimeoutExecStr is used
  std::string firstTimeoutExecStr;
  std::string reTimeoutExecStr;
  std::string matchString;
  std::string excludeString;
  vector<std::string> excludeStrings;
  bool containsExcludeStr(char *bname);
  std::string userString;
  int debug;
  virtual void init(char *initstr);
#ifndef COMPILE_FAM_MON_MAIN
  fileReader *fReader;
  virtual void addReader(char *initstr);
  fileReaderRapic* addRapicReader(char *initstr);
  fileReaderNexrad* addNexradReader(char *initstr);
  fileReaderObsAXF* addObsAXFReader(char *initstr);
  fileReaderLightningAXF* addLightningAXFReader(char *initstr);
#endif
  fileReqObj(char *initstr = NULL, int id = 0)
    {
      froType = FRO_UNDEFINED;
      fro_id = id;
      isDir = false;
      checkIsDir = false;
      doEventDelay = -1;
      doEventTime = 0;
      _eventDone = false;
      lastEventTime = 0;
      timeout = nextTimeoutTime = firstTimeoutTime = 0;
      debug = 0;
#ifndef COMPILE_FAM_MON_MAIN
      fReader = NULL;
#endif
      init(initstr);
    };
  virtual ~fileReqObj()
    {
#ifndef COMPILE_FAM_MON_MAIN
      if (fReader)
	delete fReader;
#endif
    };
  void resetTimeout(time_t timenow = 0);
  virtual bool timedOut(time_t timenow = 0, bool runit = false); // test for timed out, run timeout command if runit is set
  virtual bool confirmTimeout(time_t timenow = 0); 
  // return true if file timeout confirmed 
  virtual void doTimeout(); // do the timeout operations
  virtual void doFirstTimeout(); // do the first timeout operations
  virtual void doReTimeout(); // do the re-timeout operations
  virtual void execFirstTimeout(); // run the first timeout exec
  virtual void execReTimeout(); // run the re-timeout exec
  virtual void resetDoEventTime(time_t timenow = 0);  // set timeout to do event
  virtual bool checkDoEventTime(time_t timenow = 0, bool runit = true);  
  virtual void doEvent(); // do the event operations  
  virtual void eventDone();
  virtual void doFileReader(); // do the fileReader operations 
  virtual void execEvent(); // run the event exec 
  virtual void setTimeout(time_t newtmout);
  virtual void setFName(char *newfname);   
  virtual bool check(time_t timenow = 0);
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
};

/*
  urlReqObj class - not fam based at all but plug into 
  FAMReqObj infrastructure
  Poll url header for changes and read new file when avail
  Reading could be to intermediate file for parsing, or parser
  could could be wired into curl WriteFunction
*/

class urlReqObj : public fileReqObj
{
 public:
  time_t nextPollTime;
  time_t pollPeriod;
  time_t lastFileTime;
  double lastFileSize;
  long   transferTimeout;
  CURL *curlHandle;
  CURLM *curlMultiRef;
  bool inCurlMulti;    // set if currently in curl_multi stack
  char *curlErrorStr;
  rpEventTimer transferTimes;
  urlReqObj(char *initstr = NULL, int id = 0)
    {
      froType = FRO_URL;
      nextPollTime = lastFileTime = 0;
      lastFileSize = 0;
      pollPeriod = 15;
      curlHandle = NULL;
      curlMultiRef = NULL;
      inCurlMulti = false;
      transferTimeout = 60;
      curlErrorStr = new char[CURL_ERROR_SIZE];
      strcpy(curlErrorStr, "");
      headerWrittenSize = dataWrittenSize = 0;
      headerWriteEvents = dataWriteEvents = 0;
      transferTime = 0;
      headerFile = dataFile = NULL;
      doEventDelay = 5;
/*       showProgress = false; */
      init(initstr);
      fro_id = id;
    };
  virtual ~urlReqObj();
  virtual bool urlChanged();  // return true if file at URL is newer
  virtual bool check(time_t timenow = 0); // return true if url date changed
  virtual void setTimeCondition(time_t timevalue);
  virtual void setCurlHandle(char *urlfname = NULL);
  virtual void setCurlURL(char *urlfname = NULL);
  virtual void setHeaderMode();
  virtual void setDataMode();
  virtual void init(char *initstr);
  // write header and write data called when data from url rx'd
  static size_t writeHeader(void *buffer, size_t size, 
			    size_t nmemb, void *userp);
  static size_t writeData(void *buffer, size_t size, 
			  size_t nmemb, void *userp);
  virtual void closeHeaderFile(char *str);
  virtual void closeDataFile(char *str);  
  virtual char* writeFileName(char *strbuff);
  std::string headerFileName, dataFileName;
  virtual void doEvent(); // do the event operations  
  virtual void doFileReader(); // do the fileReader operations 
  virtual void execEvent(); // run the event exec 
  size_t headerWrittenSize, dataWrittenSize;
  int   headerWriteEvents, dataWriteEvents;
  double transferTime, nameLookupTime, connectTime, 
    preTransferTime, startTransferTime, reDirectTime, 
    downloadSpeed, downloadSize;
/*   bool showProgress; */
  FILE *headerFile, *dataFile;
  void printStats(FILE *file, int debuglevel = 0);
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
};

class FAMReqObj : public fileReqObj 
{
public:
  FAMRequest fr;
  FAMConnection *fc;
  _event_mask event_mask;
  void setEventMask(FAMCodes famcode, bool state = true);
  void setEventExists(bool state = true)
    {
      setEventMask(FAMExists, state);
    };
  void setEventChanged(bool state = true)
    {
      setEventMask(FAMChanged, state);
    };
  void setEventCreated(bool state = true)
    {
      setEventMask(FAMCreated, state);
    };
  void setEventDeleted(bool state = true)
    {
      setEventMask(FAMDeleted, state);
    };
  void setEventMoved(bool state = true)
    {
      setEventMask(FAMMoved, state);
    };
  bool monitor(FAMConnection *_fc = NULL);
  // if monitor had failed, try again, see if file/dir exists now
  bool retryMonitor(FAMConnection *_fc = NULL);
  void stopMonitor();

// mechanism to allow restart of directory monitor to "clear it up" in gam_server
// gam_server seems to have a bug which keeps looking for files that have been
// deleted from the monitored directory and using huge amounts of CPU
  bool restartMonitor();
  time_t restartDirMonitorPeriod,  
    restartDirMonitorTime;         

  bool monitorActive;
  bool monitorFailed;
  bool MonitorFailed() 
  {
    return monitorActive && monitorFailed;
  };
  // if useEventMap is used, a map of unique events is kept to allow 
  // multiple delayed events to be kept
  // if not defined only the last event is kept, this will cause 
  // multiple file creates within the doEventDelay to be lost
  bool useEventMap;
  std::map<std::string, famEventObj> eventMap;
  bool mapNewEvent(famEventObj &newevent, time_t timenow = 0);
  famEventObj lastEvent;
/*   std::string lastEventFilename; */
/*   FAMCodes lastEventCode; */
/*   time_t eventExecTime; */
  FAMReqObj(char *initstr = 0, int id = 0) : 
    fileReqObj() // don't pass initstr, prevent double init
    {
      froType = FRO_FAM;  // default is FRO_FAM
      fro_id = id;
      fc = NULL;
      isDir = false;
      checkIsDir = true;
      timeout = nextTimeoutTime = firstTimeoutTime = 0;
      isTimedOut = false;
      doEventDelay = -1;
      useEventMap = false;
      event_mask.reset();
      event_mask[FAMChanged] = event_mask[FAMCreated] = true;
      monitorActive = monitorFailed = false;
      restartDirMonitorPeriod =
	restartDirMonitorTime = 0;
      if (initstr)
	init(initstr);
    };
  virtual void init(char *initstr);
  virtual bool timedOut(time_t timenow = 0, bool runit = false); // test for timed out, run timeout command if runit is set
  virtual bool check(time_t timenow = 0); // return true if  changed
  void resetDoEventTime(famEventObj *eventobj = NULL,
			time_t timenow = 0);  // set timeout to exec event
  // return true if it is time to exec event, run it if runit set
  bool checkDoEventTime(time_t timenow = 0, bool runit = true);  
  bool checkDoEventTime(famEventObj *eventobj = NULL,
		       time_t timenow = 0, bool runit = true);  
  virtual void doEvent(famEventObj *eventobj = NULL); // do the event operations
  virtual void eventDone(famEventObj *eventobj = NULL);
  virtual void doFileReader(famEventObj *eventobj = NULL); // do the fileReader operations 
  virtual void execEvent(famEventObj *eventobj = NULL); // run the event exec 
  virtual void processFAMEvent(FAMEvent *fe, // process the event passed
			       bool timestamp, bool localtime, 
			       bool timestamp_timet); 
  // if fname is a dir will detect and monitor accordingly
  virtual void setFName(char *newfname); 
  virtual void setDirName(char *newdirname);
  virtual ~FAMReqObj()
  {
  };
  virtual void dumpStatus(FILE *dumpfile, char *title = NULL);
};
 
typedef std::vector<FAMReqObj*>::iterator frovec_iter_type;

class FAMObject {
public:
  FAMConnection fc;
  std::vector<FAMReqObj*> fro;   // fam FRO objects
  std::vector<FAMReqObj*>::iterator fro_iter;
  int froCount;
  bool famOpenFailed;
  bool famIsOpen;
  std::string progName;
  std::string iniFname;
  std::string defaultEventExecStr;    // execString to use with FAMReq if no other exec specified
  time_t defaultDoEventDelay;
  bool defaultUseEventMap;
  std::string defaultFirstTmOutExecStr; // firstTmoutexecString to use with FAMReq if no other exec specified
  std::string defaultReTmOutExecStr;    // reTmoutexecString to use with FAMReq if no other exec specified
  time_t defaultTimeout;        // 
  time_t retryFailedMonitorTime;
  time_t retryFailedMonitorPeriod;
  int  retryFailedMonitorCount;
  std::string defaultMatchString;
  std::string defaultExcludeString;
  _event_mask defaultEventMask;
  bool noExistsEvents;// gamin 0.0.23 implementation allows FAMNoExists to stop exists events
                            // See http://www.gnome.org/~veillard/gamin/differences.html
  bool timestamp;
  bool localtime;
  bool timestamp_timet;
  int  debug;
  bool restartFAMOnError;
  int  Argc;
  char **Argv;
  FAMObject(int argc, char *argv[]);
  FAMObject(char *progname = NULL, char *initfname = 0);
  void setDefaults(int argc = 0, char *argv[] = NULL);
  virtual ~FAMObject();
  virtual void setMonDefaults(char *monstr, FAMReqObj *newfro);  
  virtual void setMonDefaults(char *monstr, fileReqObj *newfro);  
  virtual void newFAMReqEntry(char *monstr);  
  virtual void newURLReqEntry(char *monstr) {};  
  virtual void newLtningURLReqEntry(char *monstr) {};  
  virtual void init(int argc, char *argv[]);
  virtual void init(char *initfname = 0);
  virtual bool open(char *progname = 0);
  virtual bool add_fro(FAMReqObj *monfro);   // add fro to vector
                                             //   return false if not added
  virtual bool add_fro(urlReqObj *monfro);   // FAMObject doesn't support URL
                                             // need child class to implement
  virtual void remove_fro(FAMReqObj *monfro);// remove fro from vector
  virtual int  fro_exists(FAMReqObj *monfro);// check if fro in vector, return location
  virtual void close();
  virtual void close_all_fro();
  virtual bool monitor(FAMReqObj *monfro);
  virtual void cancelMonitor(FAMReqObj *monfro);
  virtual void check(); // check for events, then check the FAMReqObjs for any timeout or event actions
  virtual void checkFROs(); // check the FAMReqObjs for any timeout or event actions
  virtual void retryFailedMonitors();
  int  eventsPending();
  void processEvents();

  void setNoExistsEvents(bool state = true);
  // gamin 0.0.23 implementation allows FAMNoExists to stop exists events
  // See http://www.gnome.org/~veillard/gamin/differences.html
  // Current implementation does not support turning ExistsEvents back on

  std::string dumpStatusFName;
  time_t dumpStatusTime;
  time_t dumpStatusPeriod;
  virtual void dumpStatus(char *fname = NULL);
  virtual void dumpStatus(FILE *dumpfile);
  virtual void dumpStatus(FILE *dumpfile, char *title);
};

// Time String is hh:mm:ss dd/mm/yyyy
char *famShortTimeString(time_t time, char *outstr, 
			 bool force_gmtime = false,
			 bool localtime = false);
// Date String is dd/mm/yyyy hh:mm:ss
char *famShortDateString(time_t time, char *outstr, 
		      bool force_gmtime = false, 
		      bool localtime = false);

#ifndef COMPILE_FAM_MON_MAIN

/*
  Inherit the functionality of FAMObject - 
  Adds support for urlReqObj
  Adds fileReader funct
*/

typedef list <fileReqObj *> fileReqObjsList;
typedef list <fileReqObj *>::iterator fileReqObjsIter;

typedef list <urlReqObj *> urlReqObjsList;
typedef list <urlReqObj *>::iterator urlReqObjsIter;

class fileReaderManager : public FAMObject {
 public:
  fileReaderManager(char *progName = NULL, char *inifile = NULL);
  virtual ~fileReaderManager();
  void init(char *inifile = NULL);
  fileReaderType getReaderType(char *monstr);
  void setMonDefaults(char *monstr, FAMReqObj *newfro);
  void setMonDefaults(char *monstr, fileReqObj *newfro);
  urlReqObjsList urlReqObjs;
  CURLM   *curlMultiHandle;
  int inMultiCount;
  std::string tempURLDir;
  virtual void addCurlMulti(urlReqObj *uro);
  virtual void removeCurlMulti(urlReqObj *uro);
  virtual void removeCurlMulti(CURL *curlhandle);
  virtual urlReqObj* findUrlReqObj(CURL *curlhandle);
  virtual void newURLReqEntry(char *monstr);  
  virtual void newLtningURLReqEntry(char *monstr);  
  virtual void checkCurlMulti();
  virtual bool add_fro(urlReqObj *monfro);   
  virtual void remove_fro(urlReqObjsIter &iter, // remove fro from list
			  bool do_erase = true); // if true erase from vector
  virtual void remove_fro(urlReqObj *monfro, // remove fro from vector
			  bool do_erase);    // if true, erase from vector
  // check if fro in vector return iter  
  virtual urlReqObjsIter fro_exists(urlReqObj *monfro);
  virtual void checkURLs(); // check urlReqObjs and then FAMObject 
  virtual void check(); // check urlReqObjs and then FAMObject 
  virtual void clearURLs();  // remove all URLs
  virtual void dumpStatus(FILE *dumpfile);
  virtual void dumpStatus(FILE *dumpfile, char *title);
};

class fileReaderMngThread : public ThreadObj {
 public:
  fileReaderMngThread(char *progname, char *faminifile = NULL);
  virtual ~fileReaderMngThread();
  fileReaderManager *fileReaderMng;
  bool doneInit;
  virtual void workProc();
  virtual void threadInit(); // perform any init tasks before runloop starts
  virtual void threadExit(); // allow thread stopped tidy up
};

extern fileReaderMngThread *globalFileReaderMng;
fileReaderMngThread* getGlobalFileReaderMng();
fileReaderMngThread* initGlobalFileReaderMng(char *progName, 
					     char *inifile = NULL);
void closeGlobalFileReaderMng();

#endif

#endif
