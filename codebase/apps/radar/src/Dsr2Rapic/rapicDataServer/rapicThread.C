/*
  rapicThread.C

  Implementation of rapicThread class
  Creates instances of Comms, DB and ScanMng clsses
  and calls their respective check() methods periodically
*/

#include "rapicThread.h"

rapicThread::rapicThread(float lp_dly,
			 char *comminifile,
			 char *dbinifile) : ThreadObj(lp_dly)
{
  NextCommPollTime = 0;
  timenow = time(0);
  CommPollPeriod = 1;
  GetSiteInfo(DISABLE_COVERAGE_LOAD);
  FreeListMng = new free_list_mng;
  ScanMng = new scan_mng;
  string dbIniFile = "rpdb.ini";
  if (dbinifile)
    dbIniFile = dbinifile;
  if (FileExists(dbIniFile.c_str()))
  {
    if (InitISAM(10,30,12))
      fprintf(stderr,"CTree InitISAM ERROR");
    else {
      DBMngr = new DBMng(true,dbinifile );  // don't allow db purge if cache copy mode
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
  }
  string commIniFile = "rpcomm.ini";
  if (comminifile)
    commIniFile = comminifile;
  if (FileExists(commIniFile.c_str()))
  CommMngr = new RPCommMng((char *)commIniFile.c_str());
}

rapicThread::~rapicThread()
{
  if (CommMngr) {
    delete CommMngr;
    CommMngr = 0;
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
}

void rapicThread::workProc()
{
  time_t timenow = time(0);
  if (CommMngr) {
    if (timenow >= NextCommPollTime) {
      CommMngr->CheckComms();
      NextCommPollTime = timenow + CommPollPeriod;
    }
  }
  if (DBMngr && (timenow > DBMngr->DBCheckTime)) {
    if (DBMngr->CheckScans())   // false if scan check not yet complete
      DBMngr->DBCheckTime = timenow + DBMngr->DBCheckPeriod;
  }
  if (ScanMng && (timenow > ScanMng->CheckTime))
    ScanMng->Check();
}

  
