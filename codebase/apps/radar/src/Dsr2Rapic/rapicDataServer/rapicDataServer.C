#define IRIX6
#include "freelist.h"
#include "rpcomms.h"
#include "nexrad_port_types.h"
#include "nexrad_structs.h"
#include "nexrad.h"
#include "ufMgr.h"
#include "NexRadMgr.h"
#include "fam_mon.h"
#include "replay.h"
#include "siteinfo.h"
#include "rpdb.h"
#include "rdr.h"
#include <signal.h>

bool quitRapicConvert = false;
extern RPCommMng *CommMngr;
extern scan_mng *ScanMng;
extern NexRadMgr *NexRadManager;
extern ufMgr *ufManager;

char Title[256];
pid_t mainpid = 0;
time_t	NextCommPollTime = 0;
time_t	timenow = time(0);
time_t 	CommPollPeriod = 1;
time_t          ReplayCheckTime = 0;
time_t          ReplayCheckPeriod = 30;
char 	arenaname[128] = "rapicToNexrad.arena";
time_t		MemoryStatusDumpTime = 0;
time_t		MemoryStatusDumpPeriod = 3600;
bool		reqMemCheck = false;
time_t RapicStartedTime;

bool   rpdbCacheCopyMode = false;
bool   rpdbCacheStatsMode = false;
rp_copy_req *cacheCopyReq = NULL;

#ifdef USE_SIGACTION
void HandleSignal(int signo, siginfo_t *siginfo, void *ptr)
#else
void HandleSignal(int signo, int code, sigcontext *sc )
#endif
{
  char tempstr[128];
  
  if (getpid() != mainpid)
    return;
#ifdef USE_SIGACTION
//   if (siginfo)
//     cout << "SIGNAL RECEIVED=" << siginfo->si_signo
// 	 << " Errno=" << siginfo->si_errno
// 	 << " Code=" << siginfo->si_code
// 	 << " Calling pid=" << siginfo->si_pid
// 	 << "\n utime=" << siginfo->si_utime
// 	 << " stime=" << siginfo->si_stime << endl;
#endif
  sprintf(tempstr,"SIGNAL RECEIVED = %d\n",signo);
  RapicLog(tempstr, LOG_WARNING);
  fprintf(stderr,"SIGNAL RECEIVED = %d\n",signo);
  switch (signo) 
    {
    case SIGHUP:
    case SIGINT:
    case SIGTERM:
    case SIGABRT:
    case SIGUSR1:
      sprintf(tempstr,"*****RAPIC QUIT SIGNAL RECEIVED - %d*****", signo);
      RapicLog(tempstr, LOG_ERR);
      fprintf(stdout,"%s\n", tempstr);
      quitRapicConvert = true;
      break;
    }	
}

bool check_rpdbCacheCopyMode( int argc, char **argv )
{
  if (argc < 2) return false;
  int cache_path_arg = argFound(argc, argv, "cache_path");
  int cache_list_arg = argFound(argc, argv, "cachedb_list");
  int allowrebuild = argFound(argc, argv, "allowRebuild");
  if ((cache_path_arg > 0) && (cache_list_arg > 0) && DBMngr)
    DBMngr->openRpdbCache(argv[cache_path_arg], argv[cache_list_arg], (allowrebuild > 0));
  if (argFound(argc, argv, "dump_stats"))
    rpdbCacheStatsMode = true;
  if (rpdbCacheStatsMode || DBMngr->rpdbCache)
    {
      rpdbCacheCopyMode = true;
      if (!cacheCopyReq) cacheCopyReq = new rp_copy_req(argc, argv);
      return true;
    }
  else
    return false;
}
      
bool appClosing()
{
  return quitRapicConvert;
}

int main ( int argc, char **argv )
{
  //FILE *testarena;
  char tempstr[256];

  setDefaultLogName("rapicDS.log");
  RapicStartedTime = time(0);
  mainpid = getpid();
  sprintf(arenaname,"%s.arena",argv[0]);
  sprintf(Title,"%s %s",argv[0], versionStr());
  sprintf(tempstr,"%s - Starting as pid %d\n",Title, mainpid);
  RapicLog(tempstr, LOG_WARNING);
  init_signals();

  if (FileExists("rapicquitflag", true, true)) {
    sprintf(tempstr,"%s - Warning - rapicquitflag detected on startup, deleting",Title);
    RapicLog(tempstr, LOG_WARNING);
  }

#ifdef THREAD_SPROC
  usptr_t *arena = 0;
  if (testarena = fopen(arenaname,"r")) {
    fprintf(stderr,"WARNING - IRIX ARENA ALREADY EXISTS - %s\n",arenaname);
    //		fprintf(stderr,"FATAL ERROR - 3D Rapic CANNOT RUN");
    //		exit(-1);
  } 
  else fclose(testarena);
  int ret;
  if (ret = usconfig(CONF_INITSIZE,256000))
    { perror("usconfig(CONF_INITSIZE)"); };
  if (ret = usconfig(CONF_LOCKTYPE,US_DEBUGPLUS))
    { perror("usconfig(CONF_LOCKTYPE)"); };
  //	usconfig(CONF_LOCKTYPE,US_DEBUG);
  //	usconfig(CONF_LOCKTYPE,US_NODEBUG);
  if (ret = usconfig(CONF_ARENATYPE,US_SHAREDONLY))
    { perror("usconfig(CONF_ARENATYPE)"); };
  if (ret = usconfig(CONF_INITUSERS,150))
    { perror("usconfig(CONF_INITUSERS)"); };
  //	usconfig(CONF_STHREADIOOFF);
  if (!(arena = usinit(arenaname))) {
    fprintf(stderr,"usinit(%s) - FAILED ", arenaname);
    perror(0);
    fprintf(stderr,"FATAL ERROR - %s CANNOT RUN", argv[0]);
    exit(-1);
  } 
#endif

  GetSiteInfo(DISABLE_COVERAGE_LOAD);
  FreeListMng = new free_list_mng;
  ScanMng = new scan_mng;
  rpdbCacheCopyMode = argFound(argc, argv, "cache_path");
  if (InitISAM(10,30,12))
    fprintf(stderr,"CTree InitISAM ERROR");
  else {
    DBMngr = new DBMng(!rpdbCacheCopyMode);  // don't allow db purge if cache copy mode
    if (DBMngr) 
      {
	if (ScanMng) 
	  {
	    ScanMng->AddClient(DBMngr);
	    if ((ScanMng->recentCachePeriod > 0) && ScanMng->keepScansInCache)
	      {
		fprintf(stdout, "Loading last %dmins of data from database into recent scan cache\n",
			ScanMng->recentCachePeriod);
		DBMngr->loadInitialSeq(NULL, 1, true);
		ScanMng->startupLatestTime = ScanMng->mostRecentScanTime();  //latest time from db on startup
		ScanMng->PrintScanUsage(stdout, true);
	      }
	  }
      }
    }  
  CommMngr = new RPCommMng;
  if (FileExists(defaultreplayinifile))
    {
      if (ReplayMngr)
        {
          delete ReplayMngr;
          ReplayMngr = 0;
        }
      ReplayMngr = new replay(defaultreplayinifile);
    }
  if (FileExists("nexrad.ini"))
    {
      NexRadManager = new NexRadMgr();
      if (NexRadManager)
	{
	  NexRadManager->StartNexRadStnHdlrs();
	}
    }
  if (FileExists("uf.ini"))
    {
      ufManager = new ufMgr();
      if (ufManager)
	{
	  ufManager->StartUfStnHdlrs();
	}
    }
  if (FileExists("fileReaders.ini"))
    initGlobalFileReaderMng(argv[0], "fileReaders.ini");
  check_rpdbCacheCopyMode(argc, argv);
  if (DBMngr && DBMngr->rpdbCache && cacheCopyReq)
    {
      if (rpdbCacheStatsMode)
	{
	  DBMngr->rpdbCache->dumpCacheContents("rpdb_cache_contents.txt");
	  DBMngr->rpdbCache->dumpStats(*cacheCopyReq, "cache_stats.txt");
	}
      else if (rpdbCacheCopyMode)
	{
	  DBMngr->rpdbCache->dumpCacheContents("rpdb_cache_contents.txt");
	  DBMngr->rpdbCache->copyDB(*cacheCopyReq);
	}
    }
  while (!quitRapicConvert && !rpdbCacheCopyMode)
    {
      timenow = time(0);
      if (CommMngr) {
	if (time(0) >= NextCommPollTime) {
	  CommMngr->CheckComms();
	  NextCommPollTime = timenow + CommPollPeriod;
	}
      }
      if (DBMngr && (timenow > DBMngr->DBCheckTime)) {
	if (DBMngr->CheckScans())   // false if scan check not yet complete
	  DBMngr->DBCheckTime = timenow + DBMngr->DBCheckPeriod;
      }
      if (ReplayMngr && (timenow > ReplayCheckTime)) {
	ReplayMngr->check_replay();
	ReplayCheckTime = timenow + time_t(float(ReplayCheckPeriod)/ReplayMngr->getTimeScale());
      }
      if (ScanMng && (timenow > ScanMng->CheckTime))
	ScanMng->Check();
      if (reqMemCheck ||
	  (MemoryStatusDumpTime <= timenow) || 
	  FileExists("memdump.flag", 1, 1)) {
	AppendMemoryStatus();
	MemoryStatusDumpTime = time(0) + MemoryStatusDumpPeriod;
	reqMemCheck = false;
      }
      sec_delay(0.5);
      if (FileExists("rapicquitflag", true, false))
	{
	  quitRapicConvert = true;
	  fprintf(stdout,"%s - Detected rapicquitflag - Shutting down - pid %d",Title, mainpid);
	}
      
    }
  fprintf(stdout,"%s - Shutting down - pid %d",Title, mainpid);
  if (CommMngr) {
    delete CommMngr;
    CommMngr = 0;
  }
  if (ufManager)
    {
      delete ufManager;
      ufManager = 0;
    }
  if (NexRadManager)
    {
      delete NexRadManager;
      NexRadManager = 0;
    }
  if (DBMngr)
    {
      delete DBMngr;
      DBMngr = 0;
    }
  if (ScanMng) {
    delete ScanMng;
    ScanMng = 0;
  }
  if (ReplayMngr)
    {
      delete ReplayMngr;
      ReplayMngr = 0;
    }
  closeGlobalFileReaderMng();
  if (FileExists("rapicquitflag", true, true))
    {
      fprintf(stdout,"%s - Shut down complete - pid %d - Removing rapicquitflag",Title, mainpid);
    }
      
  return 0;
}

extern int total_scans;
#ifdef HAVE_MALLINFO
extern struct mallinfo	mi;
#endif
extern int    TotalCommsRx;
extern int RpIsamStateCount;
time_t lastDumpTime = 0;
long long last_spinlock_lock_ops = 0;
long long last_spinlock_lock_attempt_ops = 0;
long long last_spinlock_lock_fail_count = 0;
long long last_spinlock_rel_ops = 0;

void DumpMemoryStatus(FILE *file, bool closing) {
  time_t Time = time(0);
  int	total_scan_size = total_scans * sizeof(rdr_scan);
  float	runhours = (Time - RapicStartedTime) / 3600.0;

  Check_mallinfo();
  if (!file) file = stderr;
  fprintf(file,"%s - running as pid %d\n", Title, mainpid);
  fprintf(file,"Memory Usage as at %s", ctime(&Time));
  if (runhours < 48)
    fprintf(file,"Rapic has been running for %1.1f hours", runhours);
  else
    fprintf(file,"Rapic has been running for %1.1f days", runhours/24.0);
  if (closing)
    fprintf(file, " - %s Closing down\n", Title);
  else
    fprintf(file, "\n");
  Print_mallinfo(file);
  fprintf(file,"Total Scan Count=%d Total Scan Size=%1.1fMB Total ExpBuff Size=%1.1fMB\n",
	  total_scans, total_scan_size/1000000.0, TotalExpBuffNodeAlloc/1000000.0);
#ifdef HAVE_MALLINFO
  fprintf(file,"Non RdrScan/ExpBuff Mem=%1.1fMB\n",
	  (mi.uordblks + mi.usmblks - TotalExpBuffNodeAlloc - total_scan_size)/1000000.0);
#endif
  fprintf(file,"Total Comms data received = %1.1fMB\n",TotalCommsRx/1000000.0);
  if (ScanMng) 
    ScanMng->PrintScanUsage(file);
  //    fprintf(file,"TotalExpBuffNodeAlloc=%d\n",TotalExpBuffNodeAlloc);
  fprintf(file,"RpIsamStateCount = %d\n", RpIsamStateCount);
  int dtime = Time-lastDumpTime;
  if (lastDumpTime && dtime)
    {
      fprintf(file, "Lock Stats -  Locks/sec=%6.1f Attempts/sec=%6.1f Rel/sec=%6.1f Fails=%1.0f Fails/hr=%6.3f\n",
	      float(spinlock_lock_ops/(dtime)),
	      float(spinlock_lock_wait_ops/(dtime)),
	      float(spinlock_rel_ops/(dtime)),
	      float(spinlock_lock_fail_count),
	      float((spinlock_lock_fail_count-last_spinlock_lock_fail_count)/(dtime))*60*60);
      spinlock_lock_ops = 0;
      spinlock_lock_wait_ops = 0;  
      last_spinlock_lock_fail_count = spinlock_lock_fail_count;
      spinlock_rel_ops = 0;
    }
  printVmSize(file);
  lastDumpTime = Time;
  if (closing)
    fprintf(file, "%s Closed at %s\n\n", Title, ctime(&Time));
  fprintf(file,"--------------------------------------------------------\n\n");
  /*
    if (FreeList)
    fprintf(stderr,"freelist nodes=%d\n",FreeList->freecount);
  */
}
    
