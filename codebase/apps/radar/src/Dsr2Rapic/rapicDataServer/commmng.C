/*

commmng.c

Implementation of the RPCommMng class

*/

#include "rpcomms.h"
#include "siteinfo.h"
#include <sys/types.h>
#include <ctype.h>
#include "rdrutils.h"
#include <sys/stat.h>
#include <stdlib.h>
#include "log.h"
#include <string.h>

#ifndef NO_XWIN_GUI
#include "RxDevAlertWindow.h"
#include "CommMngWindowClass.h"
#endif

stnScanStatsMap* StnScanStats = new stnScanStatsMap;

RPCommMng *CommMngr = 0;
int	    RPSrv_fd = -1;
u_short	    RPSrv_port = 0;
time_t      RPSrv_port_warn_time = 0;
time_t      RPSrv_port_warn_period = 300;  // warn about bind failure every 5 minutes

u_short RapicServerPort = RADAR_SERVER_DFLT;

char	commfname[] = "rpcomm.ini";
char	newcommflagname[] = "rpcomm.ini.reload";// flag file to reload new config
char	addcommflagname[] = "rpcomm.ini.add";	// flag file to add to config
char	dumpschedflagname[] = "dumpsched";	// flag to dump sched to stderr
char	dumpconnflagname[] = "dumpconn";	// flag to dump sched to stderr
char    commstatusfilename[] = "rpcomm.status"; // write status to this file periodically
char	DFLT_TxAlertClearedSound[] = "alertcleared.aifc";
char	DFLT_TxConnFailSound[] = "txconnfail.aifc";
char	DFLT_TxAckFailSound[] = "txackfail.aifc"; 	
char	DFLT_RxAlertClearedSound[] = "alertcleared.aifc";
char	DFLT_RxConnFailSound[] = "rxconnfail.aifc";
char	RPCommSaveConfigHeader[] = "#3DRAPIC GENERATED RPCOMM.INI"; 
char	LinkStatusFName[] = "/tmp/rpcomm.status";
time_t	LinkStatusFTime = 0;
int	ConnRank[CM_SOCKX25+1] = {0,300,200,100,500,400};
bool	SilenceAllAlerts = false;
bool	SuppressAllAlerts = false;
long long	TotalCommsRx = 0;

char *AlertStrings[] = {
  "OK", "Connect Failed", "Connection Lost", "Reconnect Failed", 
  "Scan Acknowledge Failed", "Remote Status Failed", "Scan Ack Failure Cleared",
  "Radar Fault Detected", "Manual Connect Timeout"
};

char ClearAcceptMssg[] = "MSSG: 9 Sorry,  no 3D-Rapic data splits free...BYE\n";

/*
  CM_GENERIC - place holder only,                                  rank 0
  CM_SERIAL  - normally leased line, no cost but limited resource  rank 300
  CM_X28		 - little cost but limited resource                    rank 200
  CM_HAYES	 - normally high cost and limited resource             rank 100
  CM_SOCKET	 - normally low cost, can have multiple RxDevs			 rank 500
  CM_SOCKX25 - as for CM_SOCKET, protocol xlat resources limited	 rank 400
*/

void RPCommMng::OpenRPSrvSocket(u_short srv_port) {
  if (disableListen || 
      (!TxDevHead && !createTxDevOnDemand))   // No need to listen for incoming connections
    return;
  if (!RPSrv_lock) RPSrv_lock = new spinlock("RPCommMng->RPSrv_lock", 200); // 2 secs
  RPSrv_lock->get_lock();
  if (RPSrv_fd >= 0) {
    RPSrv_lock->rel_lock();
    return;
  }
  RPSrv_fd = socket(PF_INET,SOCK_STREAM,0);
  if (RPSrv_fd < 0) {
    perror("RPCommMng::OpenRPSrvSocket NO SOCKET");
    RPSrv_lock->rel_lock();
    return;
  }
  //	fprintf(stderr,"Socket::Open - commfd = %d\n",commfd);
  /*
   * Typically O_NONBLOCK type writes will return -1 and errno = EAGAIN
   * if unable to write data
   * O_NDELAY will return 0 if unable to write data
   */
#ifdef sgi
  if (fcntl(RPSrv_fd,F_SETFL,O_NDELAY) < 0)
    perror("RPCommMng::OpenRPSrvSocket fcntl failed: ");
#else
  if (fcntl(RPSrv_fd,F_SETFL,O_NONBLOCK) < 0)
    perror("RPCommMng::OpenRPSrvSocket fcntl failed: ");
#endif
  RPSrv_sin.sin_family = AF_INET;
  RPSrv_sin.sin_addr.s_addr = htonl(INADDR_ANY);
  if (srv_port) RPSrv_sin.sin_port = htons(srv_port);	
  else RPSrv_sin.sin_port = htons(RapicServerPort);
  RPSrv_port = ntohs(RPSrv_sin.sin_port);
  int  val = 1;
  int size = sizeof(val);
#ifndef sun
  setsockopt(RPSrv_fd, SOL_SOCKET,SO_REUSEADDR, &val, size);
#else
  setsockopt(RPSrv_fd,SOL_SOCKET, SO_REUSEADDR, (char *)&val, size);
#endif
  if (bind(RPSrv_fd, (sockaddr *)&RPSrv_sin, sizeof(RPSrv_sin)) < 0) 
    {
      if (time(0) > RPSrv_port_warn_time)
	{
	  perror("RPCommMng::OpenRPSrvSocket - bind failed");	
	  RPSrv_port_warn_time = time(0) + RPSrv_port_warn_period;
	}
      close(RPSrv_fd);
      RPSrv_listenq = 0;
      RPSrv_fd = -1;
      RPSrv_lock->rel_lock();
      RPSrv_bind_failed = true;
      return;
    }
  else
    {
      fprintf(stderr, "RPCommMng::OpenRPSrvSocket - RPSrv_fd opened OK - fd = %d\n", RPSrv_fd);
      RPSrv_bind_failed = false;
    }
  if (listen(RPSrv_fd, RPSrv_listenq+4) < 0) {    // allow extra incoming calls to be queued
    perror("RPCommMng::OpenRPSrvSocket - listen failed");	
    close(RPSrv_fd);
    RPSrv_listenq = 0;
    RPSrv_fd = -1;
    RPSrv_bind_failed = true;
    RPSrv_lock->rel_lock();
    return;
  }
  RPSrv_port = ntohs(RPSrv_sin.sin_port);
  RPSrv_lock->rel_lock();
}

void RPCommMng::SetRPSrvListenQ(int listenq_size) {
  if (RPSrv_lock) RPSrv_lock->get_lock();
  RPSrv_listenq = listenq_size;
  if (RPSrv_fd < 0) {
    if (RPSrv_lock) RPSrv_lock->rel_lock();
    OpenRPSrvSocket();
    if (RPSrv_lock) RPSrv_lock->get_lock();
  }
  else {
    if (listen(RPSrv_fd, RPSrv_listenq+4) < 0) {// allow extra incoming calls to be queued
      perror("RPCommMng::IncRPSrvListenQ - listen failed");	
    }
    else {
      //	    fprintf(stderr,"Free splitter count (incremented) = %d\n",RPSrv_listenq);
    }
  }
  if (RPSrv_lock) RPSrv_lock->rel_lock();
}
void RPCommMng::IncRPSrvListenQ() {
  if (RPSrv_lock) RPSrv_lock->get_lock();
  RPSrv_listenq += 1;
  if (RPSrv_fd < 0) {
    if (RPSrv_lock) RPSrv_lock->rel_lock();
    OpenRPSrvSocket();
    if (RPSrv_lock) RPSrv_lock->get_lock();
  }
  else {
    if (listen(RPSrv_fd, RPSrv_listenq+4) < 0) {// allow extra incoming calls to be queued
      perror("RPCommMng::IncRPSrvListenQ - listen failed");	
    }
    else {
      //	    fprintf(stderr,"Free splitter count (incremented) = %d\n",RPSrv_listenq);
    }
  }
  if (RPSrv_lock) RPSrv_lock->rel_lock();
}
		
void RPCommMng::DecRPSrvListenQ() {
  if (RPSrv_lock) RPSrv_lock->get_lock();
  if (RPSrv_listenq > 0) RPSrv_listenq -= 1;
  else fprintf(stderr, "RPCommMng::DecRPSrvListenQ ERROR - attempt to decrement below 0\n");
  if (RPSrv_fd < 0) {
    if (RPSrv_lock) RPSrv_lock->rel_lock();
    OpenRPSrvSocket();
    if (RPSrv_lock) RPSrv_lock->get_lock();
  }
  else {
    if (listen(RPSrv_fd, RPSrv_listenq+4) < 0) {// allow extra incoming calls to be queued
      perror("RPCommMng::DecRPSrvListenQ - listen failed");	
    }
    else {
      //	    fprintf(stderr,"RPCommMng::Free splitter count (decremented) = %d\n",RPSrv_listenq);
    }
  }
  if (RPSrv_lock) RPSrv_lock->rel_lock();
}
		
int RPCommMng::GetRPSrvListenQ() {
  return RPSrv_listenq;
}

void RPCommMng::CheckRPSrvSocket() {
  int tempfd; 
  bool stateChanged = false;

#if (defined sgi) || (defined sun)
  int len;
#else
  socklen_t len;
#endif

  int maxloop = 4;
  sockaddr_in	tempsin;
  bool done = FALSE;
  char	listenstr[128];
  txdevice *new_txdevice = 0;
  char    hostname[128];

  if (RPSrv_fd < 0)
    OpenRPSrvSocket();
  if ((RPSrv_fd < 0) || RPSrv_listenq) return;
  if (RPSrv_lock) RPSrv_lock->get_lock();
  while (maxloop-- && !done)
    if (!RPSrv_listenq) {	    // no listening servers, pick up call
      len = sizeof (tempsin);
      tempfd = accept(RPSrv_fd, (sockaddr *)&tempsin, &len);
      if (tempfd >= 0) {
	if (createTxDevOnDemand)
	  {
	    sprintf(listenstr, "%d %d", RPSrv_fd, RPSrv_port);
	    new_txdevice = NewTxDevice(CM_SOCKET,listenstr);	// create txdevice
	    new_txdevice->transientDevice = true;		// mark it as transient
	    new_txdevice->transientTimeout = time(0) + 600;	// allow 10 minutes before unconnected timeout
	    sprintf(new_txdevice->Description, "Auto-Created TxDev ID=%d", new_txdevice->ID);
	    sprintf(listenstr, "%d %d", RPSrv_fd, RPSrv_port);
	    if (RPSrv_lock) RPSrv_lock->rel_lock();
	    new_txdevice->Listen(listenstr);
	    ((rpSocket *)(new_txdevice->CommHandler))->sin = tempsin;	// set commhandler's sin
	    new_txdevice->CommHandler->CheckListen(tempfd);	// pass new fd to new txdevice
	    new_txdevice->CheckNewConnection();	// pass new fd to new txdevice
	    new_txdevice->Enable();
	    sprintf(LogStr, "RPCommMng::CheckRPSrvSocket - Created new txdevice:%d to handle incoming request\n", new_txdevice->ID);
	    RapicLog(LogStr, LOG_WARNING);
	    if (RPSrv_lock) RPSrv_lock->get_lock();
	    stateChanged = true;
	  }
	else
	  {
	    if (!GetNameByAddr(&tempsin.sin_addr, hostname, 128))
	      strncpy(hostname, inet_ntoa(tempsin.sin_addr), 128);
	    send(tempfd,ClearAcceptMssg,strlen(ClearAcceptMssg),0);
	    fprintf(stderr,"RPCommMng::CheckRPSrvSocket - Call from host %s accepted/closed." 
		    "No Free Tx Devices\n", 
		    hostname);
	    if (shutdown(tempfd,2) < 0) 
	      perror("RPCommMng::CheckRPSrvSocket socket shutdown failed: ");
	    if (close(tempfd) < 0)
	      perror("RPCommMng::CheckRPSrvSocket socket close failed: ");
	    txdevConnectRefusedCount++;
	  }
      }
    }
#ifndef NO_XWIN_GUI
  if (stateChanged)
    UpdateCommMngWid(true);  // force full list update
#endif
  if (RPSrv_lock) RPSrv_lock->rel_lock();
}    

RPCommMng::RPCommMng(char *inifile) : scan_client() {
  char *EnvString;
  IsOpen = FALSE;
  ClientType = SC_COMMMNG;
  AcceptNewScans = FALSE;	    // don't accept new scans
  AcceptFinishedScans = TRUE; // do accept finished scans
  RxDevHead = RxDevTail = 0;
  RxDevInUse = 0;
  RxDevFree = 0;
  TxDevHead = TxDevTail = 0;
  SchedEntries = ThisSchedEntry = 0;
  ReqPending = 0;
  ConnQueue = 0;
  ConnStnPnt = 0;
  ThisConnStnPnt = 0;
  NextRxID = NextTxID = 1;
  lock = new spinlock("RPCommMng->lock", 100);
  //	lock = 0;
#ifndef NO_XWIN_GUI
  CommMngWid = SchedEditWid = ReqEditWid= 0;
  newCommMngWid = 0;
#endif
  TxDeviceQSize = 16;
  strncpy(FallBackFName, "/tmp/rpcomm.fallback", 64);
  strcpy(ThisConfigName, commfname);
  CheckFallBackMode = FALSE;
  FallBackMode = FALSE;
  SchedEnabled = TRUE;
#ifndef SINGLE_THREAD_COMMS
  SingleThreadComms = FALSE;
#else
  SingleThreadComms = TRUE;
#endif
  if ((EnvString = getenv("RPSINGLETHREADCOMMS"))) {
    fprintf(stderr, "RPSINGLETHREADCOMMS environment variable detected\n"
	    "Communications connections will cause pauses in the operation of the display\n");
    SingleThreadComms = TRUE;
  }
  RxLoopDelay = 0.5;	// 
  TxLoopDelay = 0.5;
  TxReconnPeriod = RxReconnPeriod = 10;
  TxReconnDelay = RxReconnDelay = 2;
  TxRe_AlertPeriod = 15 * 60;
  TxAckFailTimeout = 30;
  TxSendRefTime = TxSendTxStnSet = FALSE;
  strncpy(TxAlertClearedSound, DFLT_TxAlertClearedSound, 128);
  strncpy(TxConnFailSound, DFLT_TxConnFailSound, 128);
  strncpy(TxAckFailSound, DFLT_TxAckFailSound, 128);
  RxRe_AlertPeriod= 15 * 60;
  strncpy(RxAlertClearedSound, DFLT_RxAlertClearedSound, 128);
  strncpy(RxConnFailSound, DFLT_RxConnFailSound, 128);
  CheckFlagFilesTime = 0;	// periodically check for newcommflagname
  CheckFlagFilesPeriod = 15;	// every 15 secs    
  writeStatusFileTime = 0;	// periodically write status file
  writeStatusFilePeriod = 120;	// every 120 secs    
  statusFileName = NULL;
  Rdr_Xlat = 0;
  RPSrv_fd = -1;
  RPSrv_listenq = 0;
  RPSrv_port = 0;
  RPSrv_lock = 0;
  RPSrv_bind_failed = false;
  createTxDevOnDemand = false;
  disableListen = false;
  RxCheckRadialsForMSSG = false;
  Open(inifile);
  if (ScanMng) ScanMng->AddClient(this);
  state_changed = true;
  commsConsoleQuiet = false;
  txdevConnectRefusedCount = 0;
}

RPCommMng::~RPCommMng() {
  if (lock) delete(lock);
  if (RPSrv_lock) delete(RPSrv_lock);
  if (statusFileName)
    free(statusFileName);
  Close();
}

bool RPCommMng::Full(int passedSize) {
  return FALSE;
}

int RPCommMng::NewDataAvail(rdr_scan *new_scan) {
  if (new_scan);	// not interested in new scans, wait for finished
  return 0;
}

int RPCommMng::FinishedDataAvail(rdr_scan *finished_scan) {
  int	lock_ok = 0, result = 0;
    
  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"RPCommMng::FinishedDataAvail - get_lock FAILED\n");	
  if ((finished_scan->data_source == COMM) ||   // not interested in DB scans
      (finished_scan->data_source == COMMREQ))
    result = scan_client::FinishedDataAvail(finished_scan);
  if (lock_ok) lock->rel_lock();
  return result;
}

/*
  This client should only see FINISHED scans. Simply check and remove
*/

void  RPCommMng::CheckNewScans() {
  rdr_scan_node   *tempscannode;	
  RPSchedEntry	*thissched = SchedEntries;
  int		lock_ok = 0;
  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"RPCommMng::CheckNewScans - get_lock FAILED\n");	
  while (newscans) 
    {
      thissched = SchedEntries;
      while (thissched) 
	{
	  /*
	   * if a scan matches a backup mode sched entry
	   * and the scan is not a result of a comms request
	   * rearm the sched backupmode timeout
	   */
	  if (thissched->backupmodetime && 
	      thissched->ScanMatch(newscans->scan) &&
	      newscans->scan->data_source == COMM) {// scan not the result of commreq    
	    fprintf(stderr,"RPCommMng::CheckNewScans() - Resetting sched entry backup mode. Stn=%s Product=%s\n", 
		    StnRec[newscans->scan->station].Name, 
		    scan_type_text[newscans->scan->scan_type]);
	    thissched->ResetBackupMode();	// rearm sched fallback timeout
	  }
	  thissched = thissched->next;
	}
      tempscannode = newscans->next;
      delete newscans;
      decScanNodeCount();
      newscans = tempscannode;
    }
  if (lock_ok) 
    lock->rel_lock();
}

rxdevice *RPCommMng::ParseRxDeviceString(char *devstr, FILE *rpcomm_ini) {
  rxdevice *new_RxDev=0;
  char 	*connstring, *temp2, *lastchar;
  char	device[128],port[128],connstr[128],connstr2[128], 
    getstring[256], tempstring[128];
  ConnModes connmode;
  int		tempint;
  float	tempfloat;
  FallbackComms *fbcomms = 0;
  RPSchedEntry *newsched=0;
  bool	fbdevice = (strstr(devstr, "fbdev=") != 0);
  bool	rxdevice = (strstr(devstr, "rxdevice=") != 0) ||
    (strstr(devstr, "device=") != 0);

  device[0] = port[0] = connstr[0] = connstr2[0] = 0;
  strncpy(getstring, devstr, 128);	// temporary copy
  connstring = strstr(getstring,"connstr=");
  if (connstring) {
    temp2 = connstring-1;	// char before connstr=, must be white space
    *temp2 = 0;		// insert null before connstr= for following sscanf
  }
  if (!(fbdevice || rxdevice))
    return new_RxDev;
  if (sscanf(devstr," rxdevice=%s %s",device,port) < 1) {
    if (sscanf(devstr," device=%s %s",device,port) < 1) {
      if (sscanf(devstr," fbdev=%s %s",device,port) < 1) {
	return new_RxDev;	    // no device string read
      }
    }
  }
  sprintf(LogStr,"RPCommMng::ParseRxDeviceString device=%s port=%s\n",device,port);
  if ((connmode = Str2ConnMode(device)) != CM_GENERIC) {
    new_RxDev = NewRxDev(connmode,port);
  }
  if (!new_RxDev) {
    fprintf(stderr, "RPCommMng::ParseRxDeviceString - ERROR,  Unable to open new device\n");
    return 0;
  }
  if (connstring) {
    if (sscanf(connstring," connstr=%s %s",connstr,connstr2) >= 1) {
      if (strlen(connstr2) && !isdigit(connstr2[0]))
	connstr2[0] = 0;	    // socket port MUST BE A NUMBER
      sprintf(LogStr,"RPCommMng::ParseRxDeviceString connstr=%s connstr2=%s\n",connstr,connstr2);
      strcat(connstr," ");
      strcat(connstr,connstr2);
      if (new_RxDev && connstr[0]) {
	new_RxDev->ServiceType = RPS_SEMI_PERM;
	new_RxDev->AutoReconnect = TRUE;
	new_RxDev->Set_NewConnect(connstr);
	if (!fbdevice)			// dedicated (semiperm) rxdevice
	  UseRxDev(new_RxDev);	// put on RxDevInUse list
      }
    }
    if (strstr(connstring,"txcomplete=0")) {
      new_RxDev->GetFinishedScans = FALSE;
    }
    if (strstr(connstring,"scanbyscan")) {
      new_RxDev->GetFinishedScans = FALSE;
    }
    if (strstr(connstring,"txcompleted")) {
      new_RxDev->GetFinishedScans = TRUE;
    }
    if (strstr(connstring,"txlatestonly")) {
      new_RxDev->GetLatestScan = TRUE;
    }
    if (strstr(connstring,"allowreplay")) {
      new_RxDev->AllowReplayScans = TRUE;
    }
    if ((temp2 = strstr(connstring,"sendrecentdatamins="))) {
      if (sscanf(temp2,"sendrecentdatamins=%d",&tempint) == 1) {
	{
	  if (tempint > 360)
	    tempint = 360;
	  if ((tempint > 0) && (tempint <= 360))
	  new_RxDev->sendRecentDataMins = tempint;
	}
      }
    }
    if ((temp2 = strstr(connstring,"timeout="))) {
      if (sscanf(temp2," timeout=%d",&tempint) == 1) {
	if ((tempint > 0) && (tempint < 900))
	  new_RxDev->dflt_timeout = tempint;
      }
    }
    if ((temp2 = strstr(connstring,"statuscheckrate="))) {
      if (sscanf(temp2," statuscheckrate=%d",&tempint) == 1) {
	if ((tempint > 0) && (tempint < 900))
	  new_RxDev->StChkRate = tempint;
      }
    }
    if ((temp2 = strstr(connstring,"statusacktimeout="))) {
      if (sscanf(temp2," statusacktimeout=%d",&tempint) == 1) {
	if ((tempint > 0) && (tempint < 900))
	  new_RxDev->StAckTime = tempint;
      }
    }
  }
  else {
    if (!fbdevice)				// if not dedicated rxdevice
      FreeRxDev(new_RxDev);		// put on free RxDev list
  }
  RapicLog(LogStr, DfltLogLevel);
  strncpy(getstring, devstr, 256);		// check continuation lines
  lastchar = getstring;
  if (strlen(getstring))
    lastchar = getstring + strlen(getstring) - 1;	// last character
  while ((lastchar > getstring) && isspace(*lastchar))
    lastchar--;				// strip trailing whitespace
  while (strlen(getstring) && 
	 (*lastchar == '\\')) {			// is last non-white is \, continuation
    if (rpcomm_ini && fgets(getstring,256,rpcomm_ini)) {
      if (getstring[0] != '#') {
	if ((temp2 = strstr(getstring,"fbstn="))) {
	  if (sscanf(temp2, "fbstn=%d", &tempint) == 1) {
	    fbcomms = new FallbackComms(new_RxDev, tempint);
	    fbcomms->next = new_RxDev->FBComms;
	    new_RxDev->FBComms = fbcomms;
	  }
	}
	if ((temp2 = strstr(getstring,"fbsched="))) {
	  newsched = new  RPSchedEntry;
	  if (newsched->DecodeSchedStr(temp2)) {
	    if (newsched->period != newsched->fallbackperiod)
	      CheckFallBackMode = TRUE;
	    if (!new_RxDev->FBComms) { 
	      fbcomms = new FallbackComms(new_RxDev);
	      fbcomms->next = new_RxDev->FBComms;
	      new_RxDev->FBComms = fbcomms;
	    }
	    if (new_RxDev->FBComms && new_RxDev->FBComms->FBSchedEntryList)
	      new_RxDev->FBComms->FBSchedEntryList->prev = newsched;
	    newsched->prev = 0;
	    newsched->next = new_RxDev->FBComms->FBSchedEntryList;
	    new_RxDev->FBComms->FBSchedEntryList = newsched;
	    GetFallBackMode();
	    //		    newsched->CalcSchedTime(FallBackMode);
	  }
	  else {
	    delete newsched;
	    newsched = 0;
	  }
	}
	// NOT YET TESTED, SO COMMENTED OUT
	if ((temp2 = strstr(getstring,"fbdev="))) {
	  if (temp2[strlen(temp2)-1] == '\\') 
	    temp2[strlen(temp2)-1] = ' ';   // do not allow continuation line to pass, things could get too recursive
	  if (!new_RxDev->FBComms) { 
	    fbcomms = new FallbackComms(new_RxDev);
	    fbcomms->next = new_RxDev->FBComms;
	    new_RxDev->FBComms = fbcomms;
	  }
	  if (new_RxDev->FBComms && !new_RxDev->FBComms->FBRxDev) {
	    new_RxDev->FBComms->FBRxDev = ParseRxDeviceString(temp2);
	    new_RxDev->FBComms->FBRxDev->setInUse(false);
	  }
	}
	//
	if ((temp2 = strstr(getstring,"fbtomainsched"))) {
	  if (!new_RxDev->FBComms) { 
	    fbcomms = new FallbackComms(new_RxDev);
	    fbcomms->next = new_RxDev->FBComms;
	    new_RxDev->FBComms = fbcomms;
	  }
	  if (new_RxDev->FBComms) {
	    new_RxDev->FBComms->SuppressMainSchedMode = TRUE;
	  }
	}
	if ((temp2 = strstr(getstring,"fbtocommserver"))) {
	  if (!new_RxDev->FBComms) { 
	    fbcomms = new FallbackComms(new_RxDev);
	    fbcomms->next = new_RxDev->FBComms;
	    new_RxDev->FBComms = fbcomms;
	  }
	  if (new_RxDev->FBComms) {
	    new_RxDev->PrimaryServerConnection = true;
	    new_RxDev->FBComms->SuppressMainSchedMode = TRUE;
	    new_RxDev->FBComms->SuppressOtherRxDevMode = TRUE;
	  }
	}
	if ((temp2 = strstr(getstring,"description="))) {
	  temp2 += strlen("description=");
	  StripTrailingWhite(temp2);
	  strncpy(new_RxDev->Description, temp2, 
		  sizeof(new_RxDev->Description));
	  if (new_RxDev->Description[strlen(new_RxDev->Description)-1] == '\\')
	    new_RxDev->Description[strlen(new_RxDev->Description)-1] = 0;
	}
	temp2 = strstr(getstring,rawRPFilterString);
	if (!temp2)  // look for rawRPFilterString first, then non-raw
	  temp2 = strstr(getstring,RPFilterString);
	if (temp2) 
	  new_RxDev->AddFilter(temp2); 		
	if ((temp2 = strstr(getstring,"loopdelay="))) {
	  if (sscanf(temp2, "loopdelay=%f", &tempfloat) == 1) {
	    new_RxDev->loopDelay = tempfloat;
	    printf("RPCommMng::ParseRxDeviceString Setting loopDelay to %1.1fsecs\n", tempfloat);
	  } 
	}
	if (strstr(getstring,"txcomplete=0")) {
	  new_RxDev->GetFinishedScans = FALSE;
	}
	if (strstr(getstring,"scanbyscan")) {
	  new_RxDev->GetFinishedScans = FALSE;
	}
	if (strstr(getstring,"txcompleted")) {
	  new_RxDev->GetFinishedScans = TRUE;
	}
	if (strstr(getstring,"txlatestonly")) {
	  new_RxDev->GetLatestScan = TRUE;
	}
	if ((temp2 = strstr(getstring,"connfail_sound="))) {
	  if (sscanf(temp2,"connfail_sound=%s",tempstring) == 1) {
	    strncpy(new_RxDev->ConnFailSound, tempstring, 128);
	  }
	}
	if ((temp2 = strstr(getstring,"reconnperiod="))) {
	  if (sscanf(temp2,"reconnperiod=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_reconnperiod set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_RxDev->ReconnPeriod = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"reconndelay="))) {
	  if (sscanf(temp2,"reconndelay=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_reconndelay set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_RxDev->ReconnDelay = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"initstring="))) {
	  if (sscanf(temp2,"initstring=%s",tempstring) == 1) {
	    sprintf(LogStr,"RPCommMng::LoadConfig initstring found - %s\n",tempstring);
	    RapicLog(LogStr, DfltLogLevel);
	    new_RxDev->SetInitString(tempstring);
	  }
	}
	if ((temp2 = strstr(getstring,"radarstatusfile="))) {
	  if (sscanf(temp2,"radarstatusfile=%s",tempstring) == 1) {
	    strncpy(new_RxDev->RadarStatusFileName, tempstring, 128);
	    sprintf(LogStr,"RPCommMng::LoadConfig - radarstatusfile found - %s\n",
		    new_RxDev->RadarStatusFileName);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if (strstr(getstring,"suppress_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseRxDeviceString - Alerts suppressed\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_RxDev->SuppressAlerts = true;
	}
	if (strstr(getstring,"enable_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseRxDeviceString - Alerts enabled\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_RxDev->SuppressAlerts = false;
	}
	if (strstr(getstring,"disable_connect_confirm")) {
	  sprintf(LogStr,"RPCommMng::ParseRxDeviceString - Connection confirmation disabled\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_RxDev->DisableConnectConfirm = true;
	}
	if (strstr(getstring,"silence_all_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseRxDeviceString - Silence All Alerts set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_RxDev->silent = true;
	}
	if ((temp2 = strstr(getstring,"scansettimeout="))) {
	  if (sscanf(temp2, " scansettimeout=%d", &tempint) == 1) {
	    new_RxDev->scansettimeout = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - scansettimeout of %d invoked\n", tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if (strstr(getstring, "disabled_on_startup")) {
	  new_RxDev->DisabledOnStartup = TRUE;
	  sprintf(LogStr,"RPCommMng::ParseRxDeviceString - disabled_on_startup detected,  rxdevice disabled until manually enabled\n");
	  RapicLog(LogStr, DfltLogLevel);
	}
	if ((temp2 = strstr(getstring,"enabled_timeout="))) {
	  if (sscanf(temp2, "enabled_timeout=%f", &tempfloat) == 1) {
	    new_RxDev->enabledTimeoutPeriod = time_t(tempfloat * 60 * 60);
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - enabledTimeoutPeriod=%1.1f hours\n", tempfloat);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	else if ((temp2 = strstr(getstring,"timeout="))) {
	  if (sscanf(temp2,"timeout=%d",&tempint) == 1) {
	    if ((tempint > 0) && (tempint < 900))
	      new_RxDev->dflt_timeout = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - timeout=%d\n", tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if ((temp2 = strstr(getstring,"debug="))) {
	  if (sscanf(temp2, "debug=%d", &tempint) == 1) {
	    new_RxDev->debuglevel = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - debuglevel=%d mode enabled\n", tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if ((temp2 = strstr(connstring,"statuscheckrate="))) {
	  if (sscanf(temp2," statuscheckrate=%d",&tempint) == 1) {
	    if ((tempint > 0) && (tempint < 900))
	      new_RxDev->StChkRate = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - statuscheckrate=%d\n", 
		    tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if ((temp2 = strstr(connstring,"statusacktimeout="))) {
	  if (sscanf(temp2," statusacktimeout=%d",&tempint) == 1) {
	    if ((tempint > 0) && (tempint < 900))
	      new_RxDev->StAckTime = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - statusacktimeout=%d\n", 
		    tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
	if ((temp2 = strstr(getstring,"groupid="))) {
	  if (sscanf(temp2, "groupid=%d", &tempint) == 1) {
	    if (tempint > 0) {
	      new_RxDev->GroupID = tempint;
	      new_RxDev->DisabledOnStartup = TRUE; // 
	      sprintf(LogStr,"RPCommMng::ParseRxDeviceString - groupid=%d\n", tempint);
	      RapicLog(LogStr, DfltLogLevel);
	    }
	  }
	}
	if ((temp2 = strstr(getstring,"fbgroup="))) {
	  if (sscanf(temp2,"fbgroup=%d",&tempint) == 1) {
	    if ((tempint > 0))
	      new_RxDev->FBGroup = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"sendrecentdatamins="))) {
	  if (sscanf(temp2,"sendrecentdatamins=%d",&tempint) == 1) {
	    {
	      if (tempint > 360)
		tempint = 360;
	      if ((tempint > 0) && (tempint <= 360))
		new_RxDev->sendRecentDataMins = tempint;
	    }
	  }
	}
      }
    }
    else {
      getstring[0] = 0;
    }
    lastchar = getstring;
    if (strlen(getstring))
      lastchar = getstring + strlen(getstring) - 1;	// last character
    while ((lastchar > getstring) && isspace(*lastchar))
      lastchar--;				// strip trailing whitespace
  }
  //    new_RxDev->EnableComms = TRUE;
  return new_RxDev;
}

void RPCommMng::ParseTxDeviceString(char *txdevicestr, FILE *rpcomm_ini) {
  char	device[128], port[128], connstr[128],connstr2[128], 
    getstring[256], tempfname[128], listenstr[128];
  char 	*connstring, *filterstr, *delaystr, *temp2, *lastchar;
  float	tempfloat;
  int		tempint;
  ConnModes	connmode;
  txdevice	*new_txdevice=0;
  bool	txdevice = strstr(txdevicestr,"datasplit=") ||
    strstr(txdevicestr,"txdevice=");

  if (!txdevicestr || (strlen(txdevicestr) == 0)) return;
  device[0] = port[0] = connstr[0] = connstr2[0] = 0;
  strncpy(getstring, txdevicestr, 256);
  connstring = strstr(txdevicestr,"connstr=");
  filterstr = strstr(getstring,RPFilterString);
  if (connstring) {
    temp2 = connstring-1;	// char before connstr=, must be white space
    *temp2 = 0;	// insert null before connstr= for following sscanf
  }
  if (!txdevice) return;
  if (strstr(txdevicestr,"datasplit="))
    sscanf(txdevicestr," datasplit=%s %s",device,port);
  else if (strstr(txdevicestr,"txdevice="))
    sscanf(txdevicestr," txdevice=%s %s",device,port);
  sprintf(LogStr,"RPCommMng::ParseTxDeviceString txdevice=%s port=%s\n",device,port);
  RapicLog(LogStr, DfltLogLevel);
  if ((connmode = Str2ConnMode(device)) != CM_GENERIC)
    new_txdevice = NewTxDevice(connmode,port);
  if (!new_txdevice) return;
  if (new_txdevice && connstring) {	// only add filters to calling device
    if (filterstr) new_txdevice->AddFilter(filterstr); // add perm filter
    if (sscanf(connstring," connstr=%s %s",connstr,connstr2) >= 1) {
      if (strlen(connstr2) && !isdigit(connstr2[0]))
	connstr2[0] = 0;	    // socket port MUST BE A NUMBER
      sprintf(LogStr,"RPCommMng::ParseTxDeviceString connstr=%s connstr2=%s\n",connstr,connstr2);
      RapicLog(LogStr, DfltLogLevel);
      strcat(connstr," ");
      strcat(connstr,connstr2);
      if (new_txdevice && connstr[0]) {
	new_txdevice->AutoReconnect = TRUE;
	new_txdevice->CallingMode = TRUE;	// txdevice calls out
	new_txdevice->Set_NewConnect(connstr);
      }
    }
  }
  else if (new_txdevice) 
    {
      if (connmode == CM_SOCKET)
	{
	  if (RPSrv_fd < 0)
	    OpenRPSrvSocket();
	  sprintf(listenstr, "%d %d", RPSrv_fd, RPSrv_port);
	}
      else
	strcpy(listenstr, "");
      new_txdevice->Listen(listenstr);
    }
  if (strstr(getstring, "expect_ack"))
    new_txdevice->Expect_Ack = TRUE;
  lastchar = getstring;
  if (strlen(getstring))		    // check continuation lines
    lastchar = getstring + strlen(getstring) - 1;	// last character
  while ((lastchar > getstring) && isspace(*lastchar))
    lastchar--;				// strip trailing whitespace
  while (strlen(getstring) && 
	 (*lastchar == '\\')) {
    if (rpcomm_ini && fgets(getstring,256,rpcomm_ini)) {
      lastchar = getstring;
      if (strlen(getstring))
	lastchar = getstring + strlen(getstring) - 1;	// last character
      while ((lastchar > getstring) && isspace(*lastchar)) {
	lastchar--;				// strip trailing whitespace
      }
      if (getstring[0] != '#') {			// ignore commented lines
	if (strstr(getstring, "expect_ack"))
	  new_txdevice->Expect_Ack = TRUE;
	filterstr = strstr(getstring,RPFilterString);
	if (filterstr) new_txdevice->AddFilter(filterstr); 		
	if ((filterstr = strstr(getstring,"description="))) {
	  filterstr += strlen("description=");
	  StripTrailingWhite(filterstr);
	  strncpy(new_txdevice->Description, filterstr, 
		  sizeof(new_txdevice->Description));
	  if (new_txdevice->Description[strlen(new_txdevice->Description)-1] == '\\')
	    new_txdevice->Description[strlen(new_txdevice->Description)-1] = 0;
	}
	if (strstr(getstring, "expect_ack"))
	  new_txdevice->Expect_Ack = TRUE;
	if ((delaystr = strstr(getstring,"loopdelay="))) {
	  if (sscanf(delaystr, "loopdelay=%f", &tempfloat) == 1) {
	    new_txdevice->loopDelay = tempfloat;
	    printf("RPCommMng::ParseTxDeviceString Setting loopDelay to %1.1fsecs\n", tempfloat);
	  } 
	}
	if ((temp2 = strstr(getstring,"connfail_sound="))) {
	  if (sscanf(temp2,"connfail_sound=%s",tempfname) == 1) {
	    strncpy(new_txdevice->ConnFailSound, tempfname, 128);
	    printf("RPCommMng::ParseTxDeviceString Setting ConnFailSound to %s\n", new_txdevice->ConnFailSound);
	  }
	}
	if ((temp2 = strstr(getstring,"ackfail_sound="))) {
	  if (sscanf(temp2,"ackfail_sound=%s",tempfname) == 1) {
	    strncpy(new_txdevice->AckFailSound, tempfname, 128);
	    printf("RPCommMng::ParseTxDeviceString Setting AckFailSound to %s\n", new_txdevice->AckFailSound);
	  }
	}
	if ((temp2 = strstr(getstring,"reconnperiod="))) {
	  if (sscanf(temp2,"reconnperiod=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::ParseTxDeviceString reconnperiod set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_txdevice->ReconnPeriod = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"ackfail_tmout="))) {
	  if (sscanf(temp2,"ackfail_tmout=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::ParseTxDeviceString ackfail_tmout set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_txdevice->AckFailTimeout = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"reconndelay="))) {
	  if (sscanf(temp2,"reconndelay=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::ParseTxDeviceString reconndelay set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_txdevice->ReconnDelay = tempint;
	  }
	}
	if (strstr(getstring, "sendtxstnset"))
	  new_txdevice->SendTxStnSet = TRUE;
	if (strstr(getstring, "sendreftime"))
	  new_txdevice->SendRefTime = TRUE;
	if (strstr(getstring,"suppress_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseTxDeviceString - Alerts suppressed\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_txdevice->SuppressAlerts = true;
	}
	if (strstr(getstring,"enable_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseTxDeviceString - Alerts enabled\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_txdevice->SuppressAlerts = false;
	}
	if (strstr(getstring,"silence_all_alerts")) {
	  sprintf(LogStr,"RPCommMng::ParseTxDeviceString - Silence All Alerts set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  new_txdevice->silent = true;
	}
	if ((temp2 = strstr(getstring,"querymodetimeout="))) {
	  if (sscanf(temp2,"querymodetimeout=%d",&tempint) == 1) {
	    sprintf(LogStr,"RPCommMng::ParseTxDeviceString querymodetimeout set to %d\n",tempint);
	    RapicLog(LogStr, DfltLogLevel);
	    new_txdevice->QueryModeTimeout = tempint;
	  }
	}
	if ((temp2 = strstr(getstring,"debug="))) {
	  if (sscanf(temp2, "debug=%d", &tempint) == 1) {
	    new_txdevice->debuglevel = tempint;
	    sprintf(LogStr,"RPCommMng::ParseRxDeviceString - debuglevel=%d mode enabled\n", tempint);
	    RapicLog(LogStr, DfltLogLevel);
	  }
	}
      }
    }
    else {
      getstring[0] = 0;
    }
  }
}

void RPCommMng::LoadConfig(char *configname) {
  char	device[128],port[128],connstr[128],connstr2[128],getstring[256];
  char 	txdevicestr[128];
  char	tempfname[128];
  int	tempint;
  float	tempf;
  FILE	*rpcomm_ini;
  RPSchedEntry *newsched=0;
  RPCommReq	*newreq=0;
  rxdevice	*temprxdevice;
  txdevice	*temptxdevice;

  if (!configname)		// configname not specified, use default
    configname = commfname;
  strcpy(ThisConfigName, configname);	// save name of this config
  strcpy(tempfname, configname);
  strcat(tempfname, ".sav");
  createTxDevOnDemand = false;
  disableListen = false;
  RxCheckRadialsForMSSG = false;
  rpcomm_ini = fopen(configname,"r");
  if (!rpcomm_ini) rpcomm_ini = fopen(tempfname,"r");
  if (rpcomm_ini) {
    while (fgets(getstring,256,rpcomm_ini)) {
      device[0] = port[0] = connstr[0] = connstr2[0] = txdevicestr[0] = 0;
      if (getstring[0] != '#') {
	if (strstr(getstring,"datasplit=") ||
	    strstr(getstring,"txdevice=")) {
	  ParseTxDeviceString(getstring, rpcomm_ini);
	}
	else if (strstr(getstring,"device=") || 
		 strstr(getstring,"rxdevice=")) {
	  ParseRxDeviceString(getstring, rpcomm_ini);
	}
	else if (strstr(getstring,"rxdevsat=")) {
	  //		    ParseRxDevSatString(getstring, rpcomm_ini);
	}
	else if (sscanf(getstring," datasplit_qsize %d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_qsize set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxDeviceQSize = tempint;
	}
	else if (sscanf(getstring," txdevice_qsize=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_qsize set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxDeviceQSize = tempint;
	}
	else if (sscanf(getstring," rxdevice_alertcleared_sound=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_alertcleared_sound set to %s\n",tempfname);
	  strncpy(RxAlertClearedSound, tempfname, 128);
	}
	else if (sscanf(getstring," txdevice_alertcleared_sound=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_alertcleared_sound set to %s\n",tempfname);
	  strncpy(TxAlertClearedSound, tempfname, 128);
	}
	else if (sscanf(getstring," rxdevice_connfail_sound=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_connfail_sound set to %s\n",tempfname);
	  strncpy(RxConnFailSound, tempfname, 128);
	}
	else if (sscanf(getstring," txdevice_connfail_sound=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_connfail_sound set to %s\n",tempfname);
	  strncpy(TxConnFailSound, tempfname, 128);
	}
	else if (sscanf(getstring," txdevice_ackfail_sound=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_ackfail_sound set to %s\n",tempfname);
	  strncpy(TxAckFailSound, tempfname, 128);
	}
	else if (sscanf(getstring," rxdevice_connfail_realerttime=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_connfail_realerttime set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  RxRe_AlertPeriod = tempint;
	}
	else if (sscanf(getstring," txdevice_connfail_realerttime=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_connfail_realerttime set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxRe_AlertPeriod = tempint;
	}
	else if (sscanf(getstring," rxdevice_loopdelay=%f",&tempf) == 1) {
	  RxLoopDelay = tempf;
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_loopdelay set to %1.2f\n",RxLoopDelay);
	  RapicLog(LogStr, DfltLogLevel);
	}
	else if (sscanf(getstring," rxdevice_reconnperiod=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_reconnperiod set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  RxReconnPeriod = tempint;
	}
	else if (sscanf(getstring," rxdevice_reconndelay=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rxdevice_reconndelay set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  RxReconnDelay = tempint;
	}
	else if (sscanf(getstring," txdevice_loopdelay=%f",&tempf) == 1) {
	  TxLoopDelay = tempf;
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_loopdelay set to %1.2f\n",TxLoopDelay);
	  RapicLog(LogStr, DfltLogLevel);
	}
	else if (sscanf(getstring," txdevice_reconnperiod=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_reconnperiod set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxReconnPeriod = tempint;
	}
	else if (sscanf(getstring," txdevice_reconndelay=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_reconndelay set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxReconnDelay = tempint;
	}
	else if (sscanf(getstring," txdevice_ackfail_tmout=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_ackfail_tmout set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  TxAckFailTimeout = tempint;
	}
	else if (strstr(getstring,"txdevice_sendtxstnset")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_sendpcstnset set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  TxSendTxStnSet = TRUE;
	}
	else if (strstr(getstring,"allow_nullradl_filtering")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig allow_nullradl_filtering set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  keepNullRadls = false;
	}
	else if (strstr(getstring,"txdevice_sendreftime")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig txdevice_sendreftime set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  TxSendRefTime = TRUE;
	}
	else if (strstr(getstring,"connection=")) {
	  AddConn(getstring);
	}
	else if (strstr(getstring,"silence_all_alerts")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - Silence All Alerts set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  SilenceAllAlerts = true;
	}
	else if (strstr(getstring,"suppress_all_alerts")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - Suppress All Alerts set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  SuppressAllAlerts = true;
	}
	else if (strstr(getstring,"request=")) {
	  newreq = new  RPCommReq;
	  if (newreq->DecodeRequestString(getstring)) {
	    AddReq(newreq);
	  }
	  else {
	    delete newreq;
	    newreq = 0;
	  }
	}
	else if (strstr(getstring,"schedule=")) {
	  newsched = new  RPSchedEntry;
	  if (newsched->DecodeSchedStr(getstring)) {
	    if (newsched->period != newsched->fallbackperiod)
	      CheckFallBackMode = TRUE;
	    AddSched(newsched);
	  }
	  else {
	    delete newsched;
	    newsched = 0;
	  }
	}
	else if (strstr(getstring,"rdrxlat=")) {
	  if (Rdr_Xlat)
	    {
	      fprintf(stderr, "RPCommMng::LoadConfig - Another rdrxlat defined,  deleting first\n");
	      if (ScanMng) 
		ScanMng->RemClient(Rdr_Xlat);
	      delete Rdr_Xlat;
	      Rdr_Xlat = 0;
	    }
	  Rdr_Xlat = new rdr_xlat(getstring);
	  if (!Rdr_Xlat->isValid())
	    {
	      fprintf(stderr, "RPCommMng::LoadConfig - Bad rdrxlat defined,  deleting it\n");
	      delete Rdr_Xlat;
	      Rdr_Xlat = 0;
	    }
	  if (Rdr_Xlat && ScanMng)
	    {
	      ScanMng->AddClient(Rdr_Xlat);
	      Rdr_Xlat->startThread();
	    }
	}
	else if (strstr(getstring,"create_txdev_on_demand")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - create_txdev_on_demand set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  createTxDevOnDemand = true;
	}
	else if (strstr(getstring,"disable_listen")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - disable_listen set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  disableListen = true;
	}
	else if (strstr(getstring,"rxdevice_check_radls_for_mssg")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - rxdevice_check_radls_for_mssg\n");
	  RapicLog(LogStr, DfltLogLevel);
	  checkRadialMSSG = true;
	}
	else if (sscanf(getstring,"statusFileName=%s",tempfname) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig statusFileName set to %s\n",tempfname);
	  statusFileName = strdup(tempfname);
	}
	else if (sscanf(getstring,"writeStatusFilePeriod=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig writeStatusFilePeriod set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  writeStatusFilePeriod = tempint;
	}
	else if (strstr(getstring,"noRp_SO_KEEPALIVE")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - noRp_SO_KEEPALIVE set\n");
	  RapicLog(LogStr, DfltLogLevel);
	  noRp_SO_KEEPALIVE = true;
	}
	else if (strstr(getstring,"noRp_SO_LINGER")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - noRp_SO_LINGER\n");
	  RapicLog(LogStr, DfltLogLevel);
	  noRp_SO_LINGER = true;
	}
	else if (strstr(getstring,"useIPNameCache")) {
	  sprintf(LogStr,"RPCommMng::LoadConfig - useIPNameCache\n");
	  RapicLog(LogStr, DfltLogLevel);
	  useIPNameResCache = true;
	}
	else if (sscanf(getstring," serveRecentDataMins=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig serveRecentDataMins set to %d minutes\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  if (ScanMng)
	    ScanMng->setRecentCachePeriod(tempint);
	}
 	else if (sscanf(getstring," rapicServerPort=%d",&tempint) == 1) {
	  sprintf(LogStr,"RPCommMng::LoadConfig rapicServerPort set to %d\n",tempint);
	  RapicLog(LogStr, DfltLogLevel);
	  RapicServerPort = u_short(tempint);
	}
     }
    }
    fclose(rpcomm_ini);
  }
  temprxdevice = RxDevHead;
  while (temprxdevice) {	// Start all rx devices
    if (!temprxdevice->DisabledOnStartup) {
      temprxdevice->Enable();
      temprxdevice->do_newrpstatus(RDRIDLE);
    }
    temprxdevice = temprxdevice->adminnext;
  }
  temptxdevice = TxDevHead;
  while (temptxdevice) {	// Start all tx devices
    temptxdevice->Enable();
    temptxdevice->do_newrpstatus(RDRIDLE);
    temptxdevice = temptxdevice->next;
  }
  SetGroupEnable(-1, false);  // initially disable all groups
}

void RPCommMng::Open(char *configname, int CloseExisting) {

  if (IsOpen && CloseExisting)
    Close();	// close existing comms before re-opening
  InitGetAddrByName();
  InitGetNameByAddr();
  NextRxID = NextTxID = 1;
  OpenRPSrvSocket();
  LoadConfig(configname);
  IsOpen = TRUE;
}

void RPCommMng::SetTxDevDefaults(txdevice *txdev) { // set commmng supplied defaults
  txdev->Re_AlertPeriod = TxRe_AlertPeriod;
  strncpy(txdev->AlertClearedSound, TxAlertClearedSound, 128);
  strncpy(txdev->ConnFailSound, TxConnFailSound, 128);
  strncpy(txdev->AckFailSound, TxAckFailSound, 128);
  txdev->setLoopDelay(TxLoopDelay);
  txdev->ReconnPeriod = TxReconnPeriod;
  txdev->ReconnDelay = TxReconnDelay;
  txdev->AckFailTimeout = TxAckFailTimeout;
  txdev->SendRefTime = TxSendRefTime;
  txdev->SendTxStnSet = TxSendTxStnSet;
  txdev->MaxScanQSize = TxDeviceQSize;
}

void RPCommMng::SetRxDevDefaults(rxdevice *rxdev) {
  rxdev->Re_AlertPeriod = RxRe_AlertPeriod;
  strncpy(rxdev->AlertClearedSound, RxAlertClearedSound, 128);
  strncpy(rxdev->ConnFailSound, RxConnFailSound, 128);
  rxdev->setLoopDelay(RxLoopDelay);
  rxdev->ReconnPeriod = RxReconnPeriod;
  rxdev->ReconnDelay = RxReconnDelay;
  rxdev->checkRadialsForMSSG = RxCheckRadialsForMSSG;
}

#define IS_SCHEDULE_ENTRY (sscanf(tempstr, "schedule= %d", &tempint)==1)

void RPCommMng::SaveConfig() {
  FILE	*rpcomm_ini = 0;
  FILE	*savefile = 0, *tempfile = 0;
  char	savefname[128], tempfname[256], tempstr[256];
  bool	original = FALSE;
  RPSchedEntry *tempSched = SchedEntries;
  int tempint = 0;

  strcpy(savefname, ThisConfigName);
  strcat(savefname, ".sav");
  rpcomm_ini = fopen(ThisConfigName,"r");	// check for original rpcomm.ini
  if (!rpcomm_ini) {			// rpcomm.ini not available
    savefile = fopen(savefname,"r");   // copy the saved original to rpcomm.ini
    if (savefile) {
      fclose(savefile);
      CopyFile(savefname, ThisConfigName);
    }
    rpcomm_ini = fopen(ThisConfigName,"r");
  }
  else {			// else check if orig, copy to .sav if true
    if (fgets(tempstr, 256, rpcomm_ini)) {
      original = !strstr(tempstr, RPCommSaveConfigHeader);
    }
    if (original) CopyFile(ThisConfigName, savefname); // copy original to saved original
    rewind(rpcomm_ini);
  }
  strcpy(tempfname, ThisConfigName);
  strcat(tempfname, ".tmp");
  tempfile = fopen(tempfname, "w+");
  if (!tempfile) {
    if (rpcomm_ini) fclose(rpcomm_ini);
    sprintf(LogStr, "RPCommMng::SaveConfig() FAILED. UNABLE TO WRITE %s\n", tempfname);
    RapicLog(LogStr, LOG_ERR);
    return;
  }
  fprintf(tempfile, "%s\n", RPCommSaveConfigHeader);	// 1st line is identifying header
  if (rpcomm_ini) {	    // copy all but the schedule entries tp temp
    if (fgets(tempstr, 256, rpcomm_ini)) {	// 1st string often header, ignore it
      if (!strstr(tempstr, RPCommSaveConfigHeader))   // if not,copy it
	if (!IS_SCHEDULE_ENTRY)		// also don't copy schedule line
	  fputs(tempstr, tempfile);
    }
    while (fgets(tempstr, 256, rpcomm_ini)) {
      if (!IS_SCHEDULE_ENTRY)		// don't copy schedule line
	fputs(tempstr, tempfile);
    }
    fclose(rpcomm_ini);
  }
  while (tempSched) {
    tempSched->EncodeSchedStr(tempstr);
    fputs(tempstr, tempfile);
    tempSched = tempSched->next;
  }
  fclose(tempfile);
  rename(tempfname, ThisConfigName);   // rename new rpcomm.ini over old
}

void RPCommMng::Close() {
  rxdevice	*temprxdevice;
  txdevice	*temptxdevice;
  RPSchedEntry	*tempSched;
  RPCommReq	*tempReq;
  RPConnection	*tempConn;
  RPStnConnPnt	*tempStnConn;

  temprxdevice = RxDevHead;
  while (temprxdevice) {	// signal all Free RxDev to stop
    temprxdevice->setStopThreadFlag();  
    temprxdevice = temprxdevice->adminnext;
  }
  while (RxDevHead) {
    temprxdevice = RxDevHead->adminnext;
    delete RxDevHead;
    RxDevHead = temprxdevice;
  }
  RxDevTail = 0;
  RxDevFree = 0;
  RxDevInUse = 0;
  temptxdevice = TxDevHead;
  while (temptxdevice) {	// signal all txdevices to stop
    temptxdevice->setStopThreadFlag();  
    temptxdevice = temptxdevice->next;
  }
  while (TxDevHead) {
    temptxdevice = TxDevHead->next;
    if (ScanMng) 
      ScanMng->RemClient(TxDevHead);
    //SD debug 18/7/00.  
    //printf("RPCommMng::Close() ABOUT to do_checkscanq id=%d\n",TxDevHead->ID);
    //TxDevHead->do_checkscanq();

    delete TxDevHead;
    TxDevHead = temptxdevice;
  }
  TxDevTail = 0;
  while (SchedEntries) {
    tempSched = SchedEntries->next;
    delete SchedEntries;
    SchedEntries = tempSched;
  }
  while (ReqPending) {
    tempReq = ReqPending->next;
    delete ReqPending;
    ReqPending = tempReq;
  }
  while (ConnQueue) {
    tempConn = ConnQueue->next;
    delete ConnQueue;
    ConnQueue = tempConn;
  }
  while (ConnStnPnt) {
    tempStnConn = ConnStnPnt->next;
    delete ConnStnPnt;
    ConnStnPnt = tempStnConn;
  }
  if (Rdr_Xlat) 
    {
      if (ScanMng) 
	ScanMng->RemClient(Rdr_Xlat);
      delete Rdr_Xlat;
      Rdr_Xlat = 0;
    }
#ifndef NO_XWIN_GUI
  CloseSchedEditWid();
  CloseReqEditWid();
  CloseCommMngWid();
  newCloseCommMngWid();
#endif
  IsOpen = FALSE;
}


void RPCommMng::CheckComms() {
  rxdevice	*RxDev;
  txdevice	*TxDev;
  bool CommMngGUIDelta = FALSE;

  CheckTxDeviceListenCount();
  CheckSched();
  CheckReq();
  if (!disableListen)
    CheckRPSrvSocket();
  if (SingleThreadComms) {
    RxDev = RxDevHead;
    while (RxDev) {	
      RxDev->CheckComms();  
      RxDev = RxDev->adminnext;
    }
    TxDev = TxDevHead;
    while (TxDev) {
      TxDev->CheckComms();
      TxDev = TxDev->next;
    }
  }
  RxDev = RxDevHead;
  TotalCommsRx = 0;
  while (RxDev) {	


    //    if (RxDev->threadTimedOut())
      
    if (RxDev->NewCommHndlWidStFlag) {
#ifndef NO_XWIN_GUI
      if (RxDev->CommHndlWid) NewRdrStVals(RxDev->CommHndlWid, RxDev->RdrStCtl);
#endif
      RxDev->NewCommHndlWidStFlag = FALSE;
    }
    if (RxDev->StatusChanged) {
#ifndef NO_XWIN_GUI
      if (RxDev->RxDevStatusWid) RxDev->UpDateRxDevWid();
#endif
      CommMngGUIDelta = TRUE;
    }
    RxDev->CheckAlertStatus();
    TotalCommsRx += RxDev->total_rx;
    RxDev = RxDev->adminnext;
  }
  TxDev = TxDevHead;
  while (TxDev) {
    if (TxDev->StatusChanged || TxDev->FilterListStatusChanged) {
#ifndef NO_XWIN_GUI
      if (TxDev->TxDevStatusWid) TxDev->UpDateTxDevWid();
#endif
      if (TxDev->StatusChanged)
	CommMngGUIDelta = TRUE;
    }
    TxDev->CheckAlertStatus();
    TxDev = TxDev->next;
  }
#ifndef NO_XWIN_GUI
  if (CommMngGUIDelta) UpdateCommMngWid();
#endif
  if (time(0) > CheckFlagFilesTime) {
    CheckFlagFiles();
    CheckFlagFilesTime = time(0) + CheckFlagFilesPeriod;
  }
  if (state_changed || (writeStatusFilePeriod && 
			(time(0) > writeStatusFileTime))) {
    writeStatusToFile(statusFileName);
    writeStatusFileTime = time(0) + writeStatusFilePeriod;
    state_changed = false;
  }
}	

void RPCommMng::CheckFlagFiles() {
  commsConsoleQuiet = FileExists("rpcomm.quiet");
  CheckNewConfigFile();   // check for config to replace existing
  CheckConfigAddFile();   // check for config to add to existing
  CheckDumpSchedFile();
  CheckDumpConnFile();
}

void RPCommMng::SetSchedEnable(bool state) {
  bool lock_ok = FALSE;
  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"RPCommMng::FinishedDataAvail - get_lock FAILED\n");	
  SchedEnabled = state;    
  if (lock_ok)
    lock->rel_lock();
}

void RPCommMng::SetOtherRxDevEnable(bool state) {
  rxdevice *temprxdev = RxDevHead;
  bool lock_ok = FALSE;
  if (lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"RPCommMng::SetOtherRxDevEnable - get_lock FAILED\n");	
  while (temprxdev)
    {
      if (!temprxdev->PrimaryServerConnection)
	{
	  if (state) temprxdev->Enable();
	  else temprxdev->Disable();
	}
      temprxdev = temprxdev->adminnext;
    }
  if (lock_ok)
    lock->rel_lock();
}

// If called with groupid, all non 0 groupid rxdevices and sched entries 
// will be set to state
void RPCommMng::SetGroupEnable(int groupid, bool state, bool nested)
{
  rxdevice *temprxdev = RxDevHead;
  RPSchedEntry	*thisschedentry;
  bool lock_ok = FALSE;
  if (!groupid)
    {
      fprintf(stderr,"RPCommMng::SetGroupEnable - ERROR CALLED WITH GROUPID=0\n");	
      return;	
    }
  if (!nested && lock && !(lock_ok = lock->get_lock()))
    fprintf(stderr,"RPCommMng::SetGroupEnable - get_lock FAILED\n");	
  while (temprxdev)
    {
      if (temprxdev->GroupID &&   // must be non zero
	  ((temprxdev->GroupID == groupid) || (groupid == -1)))
	{
	  if (state) temprxdev->Enable();
	  else 
	    {
	      temprxdev->Disable();
	      // disable "downstream" fbgroups
	      if (temprxdev->FBGroup && (temprxdev->FBGroup != groupid))
		SetGroupEnable(temprxdev->FBGroup, false, true);	// set nested call flag to avoid recursive locking
	    }
	}
      temprxdev = temprxdev->adminnext;
    }
  thisschedentry = SchedEntries;
  while (thisschedentry)
    {
      if (thisschedentry->GroupID && 
	  ((thisschedentry->GroupID == groupid) || (groupid == -1)))
	thisschedentry->disabled = !state;	
      thisschedentry = thisschedentry->next;
    }
  if (lock_ok)
    lock->rel_lock();
}

void RPCommMng::CheckDumpSchedFile() {
  FILE *dumpfile;
  if (FileExists(dumpschedflagname, MUSTBEWRITABLE, DELETEFLAGFILE)) {
    if ((dumpfile = fopen("dumpsched.txt","w"))) {
      DumpSchedState(dumpfile);
      fclose(dumpfile);
    }
    else
      DumpSchedState(stderr);
  }
}

void RPCommMng::CheckDumpConnFile() {
  FILE *dumpfile;
  if (FileExists(dumpconnflagname, MUSTBEWRITABLE, DELETEFLAGFILE)) {
    if ((dumpfile = fopen("dumpconn.txt","w"))) {
      DumpConnState(dumpfile);
      fclose(dumpfile);
    }
    else
      DumpConnState(stderr);
  }
}

void RPCommMng::CheckNewConfigFile() {
  FILE *CommFlagFile = 0;
  char NewConfigName[256] = "";
  char *pNewConfigName = 0;
  int  CloseExistingConfig = 1;
  int  args = 0;

  if (FileExists(newcommflagname, MUSTBEWRITABLE)) {
    if ((CommFlagFile = fopen(newcommflagname, "r"))) {	// check whether flag file contains config file name
      if (((args = fscanf(CommFlagFile, "%255s", 
			  NewConfigName)) == 1) && 
	  FileExists(NewConfigName)) {	    // check whether config file exists
	pNewConfigName = NewConfigName; // yes, use it for reload
      }
    }
    if (pNewConfigName)
      sprintf(LogStr, "RPCommMng - NEW CONFIG FLAG FILE DETECTED,  RELOADING COMMS CONFIG FROM %s\n", pNewConfigName);		
    else
      sprintf(LogStr, "RPCommMng - NEW CONFIG FLAG FILE DETECTED,  RELOADING COMMS CONFIG FROM %s\n", commfname);
    RapicLog(LogStr, LOG_WARNING);
    Open(pNewConfigName, CloseExistingConfig);
    FileExists(newcommflagname, MUSTBEWRITABLE, DELETEFLAGFILE); //remove file
  }
}

void RPCommMng::CheckConfigAddFile() {	// add the named config file to the existing config
  FILE *CommFlagFile = 0;
  char NewConfigName[256] = "";
  char *pNewConfigName = 0;
  int  CloseExistingConfig = 0;
  int  args = 0;

  if (FileExists(addcommflagname, MUSTBEWRITABLE)) {
    if ((CommFlagFile = fopen(addcommflagname, "r"))) {	// check whether flag file contains config file name
      if (((args = fscanf(CommFlagFile, "%255s", 
			  NewConfigName)) == 1) && 
	  FileExists(NewConfigName)) {	    // check whether config file exists
	pNewConfigName = NewConfigName; // yes, use it for reload
      }
    }
    if (pNewConfigName)
      sprintf(LogStr, "RPCommMng - ADD CONFIG FLAG FILE DETECTED,  ADDING COMMS CONFIG FROM %s\n", pNewConfigName);		
    else
      sprintf(LogStr, "RPCommMng - ERROR: ADD CONFIG FLAG FILE DETECTED BUT NO VALID FILE TO ADD\n");		
    RapicLog(LogStr, LOG_WARNING);
    if (pNewConfigName)	    // only add using new file name, not default name
      Open(pNewConfigName, CloseExistingConfig);   // don't close existing config
    FileExists(addcommflagname, MUSTBEWRITABLE, DELETEFLAGFILE); //remove file
  }
}

// GetStnConnPnt will return when RPStnConnPnt >= desired stn if match FALSE
// else if match TRUE will only return RPStnConnPnt->StnID == stn
// MUST CHECK STN BEFORE USING THIS
RPStnConnPnt* RPCommMng::GetStnConnPnt(int stn, bool match) {
  RPStnConnPnt *temp;
  temp = ConnStnPnt;
  while (temp && (stn < temp->StnID))
    temp = temp->next;
  if (match && temp && (temp->StnID != stn)) temp = 0;
  return temp;
}

RPConnection* RPCommMng::GetFirstStnConn(int stn) {
  RPStnConnPnt *temp = GetStnConnPnt(stn,TRUE);
  if (temp) return temp->FirstStnConn;
  else return 0;
}

void RPCommMng::AddConn(RPConnection *newconn) {
  //bool newstn = TRUE;
  RPStnConnPnt	*StnConnPnt,*temp;
  if (!newconn) return;
  StnConnPnt = GetStnConnPnt(newconn->stn_id,FALSE);
  if ((StnConnPnt && (StnConnPnt->StnID == newconn->stn_id))) {
    StnConnPnt->EntryCount++;	// existing stn conn list, ins this conn
    newconn->next = StnConnPnt->FirstStnConn;
    newconn->prev = StnConnPnt->FirstStnConn->prev;
    if (newconn->prev) newconn->prev->next = newconn;
    newconn->next->prev = newconn;
    if (ConnQueue == StnConnPnt->FirstStnConn)	// inserting before first entry
      ConnQueue = newconn;	// ConnQueue point to new head
    StnConnPnt->FirstStnConn = newconn;
    return;
  }
  else {			// no existing stn conn list, create new
    if (ConnQueue) ConnQueue->prev = newconn; // add newconn to start of queue
    newconn->prev = 0;
    newconn->next = ConnQueue;
    ConnQueue = newconn;
    temp = new RPStnConnPnt(newconn);
    if (!ConnStnPnt) {		// no existing entries, this will be first 
      ConnStnPnt = temp;
      return;
    }
    if (ConnStnPnt == StnConnPnt) {	// newconn will precede first in list
      temp->next = ConnStnPnt;
      ConnStnPnt->prev = temp;
      ConnStnPnt = temp;
      return;
    }
    if (!StnConnPnt) {		// past last stn in list
      StnConnPnt = ConnStnPnt;
      while (StnConnPnt->next)	// step to last stn
	StnConnPnt = StnConnPnt->next;
      temp->prev = StnConnPnt;
      StnConnPnt->next = temp;
      return;
    }
    // if we get to here  StnConnPnt points to next stn after this stn
    temp->next = StnConnPnt;    // insert into list
    temp->prev = StnConnPnt->prev;
    if (temp->prev) temp->prev->next = temp;
    StnConnPnt->prev = temp;
  }
}

void RPCommMng::AddConn(char *NewConnStr) {
  RPConnection *newconn=0;
  newconn = new  RPConnection;
  if (newconn->DecodeConnStr(NewConnStr)) {
    AddConn(newconn);
  }
  else {
    delete newconn;
    newconn = 0;
  }
}

int RPCommMng::GetConnStnItem(int itemno) {
  int entry = 1;
  RPStnConnPnt *thisentry = ConnStnPnt;
  while ((entry < itemno) && thisentry) {
    thisentry = thisentry->next;
    entry++;
  }
  if (thisentry) return thisentry->StnID;
  else return -1;
}

int RPCommMng::GetFirstConnStn() {
  ThisConnStnPnt = ConnStnPnt;
  if (ThisConnStnPnt) return ThisConnStnPnt->StnID;
  else return -1;
}

int RPCommMng::GetNextConnStn() {
  if (ThisConnStnPnt) ThisConnStnPnt = ThisConnStnPnt->next;
  if (ThisConnStnPnt) return ThisConnStnPnt->StnID;
  else return -1;
}

int RPCommMng::GetPrevConnStn() {
  if (ThisConnStnPnt) ThisConnStnPnt = ThisConnStnPnt->prev;
  if (ThisConnStnPnt) return ThisConnStnPnt->StnID;
  else return -1;
}

void  RPCommMng::GetConnScanTypes(int stnid, SCAN_TYPE_ARRAY typearray) {
  RPConnection *tempconn;
  RPStnConnPnt *tempstnpnt;
  for (int typ = RPT_DFLT_PPI1; typ <= RPT_VOL; typ++) 
    typearray[typ] = FALSE;
  if (!(tempstnpnt = GetStnConnPnt(stnid,TRUE))) return;
  tempconn = tempstnpnt->FirstStnConn;
  while (tempconn && (tempconn->stn_id == stnid)) {
    switch (tempconn->port_type) {
    case RPO_STD1:
      /*				typearray[RPT_USR_PPI] = 
					typearray[RPT_USR_RHI] = */
      typearray[RPT_DFLT_PPI1] = 
	TRUE;
      break;
    case RPO_STD2:
      /*				typearray[RPT_USR_PPI] = 
					typearray[RPT_USR_RHI] = */
      typearray[RPT_DFLT_PPI2] = 
	TRUE;
      break;
    case RPO_VOL:
      typearray[RPT_VOL] = TRUE;
      break;
    case RPO_SRV:
      typearray[RPT_DFLT_PPI1] = 
	typearray[RPT_VOL] = TRUE;
      break;
    }
    tempconn = tempconn->next;
  }
}

	
bool RPCommMng::ScanTypeAvailFromPort(RP_SCAN_TYPE scan_type,
				      RP_PORT_TYPE port_type) {
  switch (scan_type) {
  case RPT_DFLT_PPI1:
    if ((port_type == RPO_STD1) || (port_type == RPO_SRV)) return TRUE;
    break;
  case RPT_DFLT_PPI2:
    if ((port_type == RPO_STD2) || (port_type == RPO_SRV)) return TRUE;
    break;
  case RPT_USR_PPI:
  case RPT_USR_RHI:
    if ((port_type == RPO_STD1) || (port_type == RPO_STD2)) return TRUE;
    break;
  case RPT_VOL:
    if ((port_type == RPO_VOL) || (port_type == RPO_SRV)) return TRUE;
    break;
  }
  return FALSE;
}

// if this returns 0, there is no connection capable of satisfying the request
// if this returns 1, there is a connection available, but it is currently busy
int RPCommMng::GetBestReqConn(RPCommReq *Req,RPConnection *Conn) {
  int highestrank = 0,temprank = 0;
  RPConnection *highestconn = 0,*tempconn = GetFirstStnConn(Req->stn_id);
  while (tempconn) {
    if ((temprank = ReqToConnRank(Req,tempconn)) > highestrank) {
      highestrank = temprank;
      highestconn = tempconn;
    }
    tempconn = tempconn->next;
    if (tempconn && (tempconn->stn_id != Req->stn_id)) tempconn = 0;
  }
  Conn = highestconn;
  return highestrank;
}

bool RPCommMng::GetReqConnList(RPCommReq *Req) {
  RPConnection *tempconn = GetFirstStnConn(Req->stn_id);
  if (Req->ConnList) return TRUE;
  while (tempconn) {
    if (ReqToConnRank(Req,tempconn)) 
      Req->AddConn(tempconn);
    tempconn = tempconn->next;
    if (tempconn && (tempconn->stn_id != Req->stn_id)) 
      tempconn = 0;
  }
  return Req->ConnList != 0;
}
	
bool RPCommMng::NoRxDevAvail(RPConnection *Conn) {
  rxdevice *temp = RxDevFree;
  bool RxDevfound = temp && (Conn->conn_mode == temp->CommHandler->ConnMode);
  while (temp && !RxDevfound) {	    // check free RxDev for conn_mode
    temp = temp->next;
    RxDevfound = temp && (Conn->conn_mode == temp->CommHandler->ConnMode);
  }
  if (RxDevfound) return FALSE;
  temp = RxDevInUse;		    // check free RxDev for conn_mode AND RPS_REQUEST service
  RxDevfound = temp && (Conn->conn_mode == temp->CommHandler->ConnMode)
    && (temp->ServiceType == RPS_REQUEST);
  while (temp && !RxDevfound) {
    temp = temp->next;
    RxDevfound = temp && (Conn->conn_mode == temp->CommHandler->ConnMode)
      && (temp->ServiceType == RPS_REQUEST);
  }
  return !RxDevfound;
}

int RPCommMng::ReqToConnRank(RPCommReq *Req,RPConnection *Conn)	{
  if (!Conn || !Req) return 0;									// something not defined, no good
  if (Conn->stn_id != Req->stn_id) return 0;		// stn_id mismatch, no good
  if (time(0) < Conn->ready_tm) return 0;			// conn not ready yet, no good
  if (!ScanTypeAvailFromPort(Req->scan_type,Conn->port_type)) // can't use conn, no good
    return 0;
  if (NoRxDevAvail(Conn))		// no RxDev suitable
    return 0;
  if (Conn->InUse) return 1;			// not avail now, may be later, low rank but > 0
  return Conn->conn_rank + Conn->Retries;
}

bool RPCommMng::IsDupReq(RPCommReq *Req) {
  RPCommReq *temp = ReqPending;
  while (temp && !temp->IsSame(Req))
    temp = temp->next;
  return temp != 0;		// if temp != 0 a match was found
}

void RPCommMng::RemReq(RPCommReq *Req) {
  RPCommReq *temp = ReqPending;
  while (temp && (temp != Req))
    temp = temp->next;
  if (temp == Req) {
    if (Req == ReqPending) ReqPending = Req->next;
    delete Req;
    Req = 0;
#ifndef NO_XWIN_GUI
    ReqEditUpdateList(ReqEditWid);
#endif
  }
  return;		// if temp != 0 a match was found
}

void RPCommMng::AddReq(RPCommReq *Req) {
  char tmp[128];
  if (IsDupReq(Req)) {
    sprintf(LogStr, "RPCommMng::AddReq - Duplicate request not added\n");
    RapicLog(LogStr, DfltLogLevel);
    delete Req;
    return;
  }
  if (!GetReqConnList(Req)) {
    Req->String(tmp);
    sprintf(LogStr, "RPCommMng::AddReq - No RxDev suitable for this request-%s\n", tmp);
    RapicLog(LogStr, LOG_ERR);
    delete Req;
    return;
  }
  if (ReqPending) ReqPending->prev = Req;
  Req->next = ReqPending;
  Req->prev = 0;
  ReqPending = Req;
  Req->TimePutOnQ = time(0);
  Req->TimeLastAttempt = time(0);
  char tempstr[256];
  time_t now = time(0);
  Req->String(tempstr);
  sprintf(LogStr, "RPCommMng::AddReq - Request added: %s%s\n", 
	  ctime(&now), tempstr);
  RapicLog(LogStr, DfltLogLevel);
#ifndef NO_XWIN_GUI
  ReqEditUpdateList(ReqEditWid);
#endif
}

void RPCommMng::AddReq(int stn, RP_SCAN_TYPE ScanType, rdr_angle ReqAngle,
		       int ReqCount, RP_QUERY_TYPE QueryType, time_t QueryTime) {
  RPCommReq *newreq;
  if (!stn) return;
  newreq = new RPCommReq;
  newreq->stn_id = stn;
  newreq->scan_type = ScanType;
  newreq->req_angle = ReqAngle;
  newreq->req_count = ReqCount;
  newreq->QueryType = QueryType;
  newreq->QueryTime = QueryTime;
  newreq->priority = 50;
  AddReq(newreq);
}

void RPCommMng::AddSched(int stn, RP_SCAN_TYPE ScanType, rdr_angle ReqAngle,
			 int period, int offset) {
  RPSchedEntry *newsched, *tmpsched, *nextsched;
  RPCommReq *newreq;
  char tmp[128];
    
  if (!stn) return;
  newsched = new  RPSchedEntry;
  newreq = &newsched->CommReq;
  newreq->stn_id = stn;
  newreq->scan_type = ScanType;
  newreq->req_angle = ReqAngle;
  newreq->req_count = 1;
  newreq->QueryType = RPQ_LATEST;
  newreq->QueryTime = 0;
  newreq->priority = 50;
  newsched->period = period;
  newsched->offset = offset;
  newsched->minperiod = period;
  tmpsched = SchedEntries;
  while (tmpsched) {	    // check for duplicate, remove if present
    if ((tmpsched->CommReq.stn_id == stn) &&
	(tmpsched->CommReq.scan_type == ScanType) &&
	(tmpsched->CommReq.req_angle == ReqAngle)) {
      sprintf(LogStr, "RPCommMng::AddSched - NEW SCHED ENTRY OVERWRITING DUPLICATE\n");
      RapicLog(LogStr, LOG_ERR);
      if (tmpsched == SchedEntries)  // delete duplicate sched
	SchedEntries = SchedEntries->next;
      nextsched = tmpsched->next;    
      delete tmpsched;		    
      tmpsched = nextsched;	    // check next
    }
    else tmpsched = tmpsched->next;   // check next
  }
  if (GetReqConnList(&newsched->CommReq)) {	// check that req can be serviced before scheduling
    newsched->CommReq.ClearConnList();	// remove ConnList
    if (SchedEntries) SchedEntries->prev = newsched;
    newsched->prev = 0;
    newsched->next = SchedEntries;
    SchedEntries = newsched;
    GetFallBackMode();
    newsched->schedtime = time(0);	// call immediately
    //    newsched->CalcSchedTime(FallBackMode);
#ifndef NO_XWIN_GUI
    SchedEditUpdateList(SchedEditWid);
#endif
  }
  else { 
    newsched->CommReq.String(tmp);
    sprintf(LogStr, "RPCommMng::AddSched - No RxDev suitable for this request-%s\n", tmp);
    RapicLog(LogStr, LOG_ERR);
    delete newsched;
  }
}

/*
 * Do NOT place the passed NewSched on the list. Make a copy to put on 
 * 
 */
void RPCommMng::AddSched(RPSchedEntry *NewSched) {
  RPSchedEntry *tmpsched, *nextsched, *newsched;
  //    RPCommReq *newreq, *NewReq;
  char tmp[128];

  if (!(NewSched && NewSched->CommReq.stn_id)) return;
  newsched = new  RPSchedEntry;
  *newsched = *NewSched;		    // copy members
  newsched->CommReq = NewSched->CommReq;  // copy commreq members
  /*
    newreq = &newsched->CommReq;
    NewReq = &NewSched->CommReq;
    newreq->stn_id = NewReq->stn;
    newreq->scan_type = NewReq->ScanType;
    newreq->req_angle = NewReq->ReqAngle;
    newreq->req_count = NewReq->req_count;
    newreq->QueryType = NewReq->QueryType;
    newreq->QueryTime = NewReq->QueryTime;
    newreq->priority = NewReq->priority;
    newsched->period = NewSched->period;
    newsched->offset = NewSched->offset;
    newsched->minperiod = NewSched->period;
  */
  tmpsched = SchedEntries;
  while (tmpsched) {	    // check for duplicate, remove if present
    if ((tmpsched->CommReq.stn_id == newsched->CommReq.stn_id) &&
	(tmpsched->CommReq.scan_type == newsched->CommReq.scan_type) &&
	(tmpsched->CommReq.req_angle == newsched->CommReq.req_angle)) {
      sprintf(LogStr, "RPCommMng::AddSched - NEW SCHED ENTRY OVERWRITING DUPLICATE\n");
      RapicLog(LogStr, LOG_ERR);
      if (tmpsched == SchedEntries)  // delete duplicate sched
	SchedEntries = SchedEntries->next;
      nextsched = tmpsched->next;    
      delete tmpsched;		    
      tmpsched = nextsched;	    // check next
    }
    else tmpsched = tmpsched->next;   // check next
  }
  if (GetReqConnList(&newsched->CommReq)) {	// check that req can be serviced before scheduling
    newsched->CommReq.ClearConnList();	// remove ConnList
    if (SchedEntries) SchedEntries->prev = newsched;
    newsched->prev = 0;
    newsched->next = SchedEntries;
    SchedEntries = newsched;
    GetFallBackMode();
    newsched->schedtime = time(0);	// call immediately
    //    newsched->CalcSchedTime(FallBackMode);
#ifndef NO_XWIN_GUI
    SchedEditUpdateList(SchedEditWid);
#endif
  }
  else { 
    newsched->CommReq.String(tmp);
    sprintf(LogStr, "RPCommMng::AddSched - No RxDev suitable for this request-%s\n", tmp);
    RapicLog(LogStr, LOG_ERR);
    delete newsched;
  }
}

/* locate sched entry with same stn,type,angle request */
RPSchedEntry* RPCommMng::GetSchedEntry(char *str) {
  RPSchedEntry localentry, *tmpsched;
    
  localentry.DecodeString(str);  // convert string to sched entry
  tmpsched = SchedEntries;  // search list for match
  while (tmpsched &&	    // fall through on match or no more
	 (!localentry.IsSame(tmpsched)))
    tmpsched = tmpsched->next;
  return tmpsched;	    // returns 0 if not found
}
    
/* locate sched entry matching request */
RPSchedEntry* RPCommMng::GetSchedEntry(RPSchedEntry *schedentry) {
  RPSchedEntry localentry, *tmpsched;
    
  tmpsched = SchedEntries;  // search list for match
  while (tmpsched &&	    // fall through on match or no more
	 (!schedentry->IsSame(tmpsched)))
    tmpsched = tmpsched->next;
  return tmpsched;	    // returns 0 if not found
}
    
/* locate sched entry number n in the linked list */
RPSchedEntry* RPCommMng::GetSchedEntryNum(int n) {
  RPSchedEntry localentry, *tmpsched;
    
    
  tmpsched = SchedEntries;  // search list for match
  while (tmpsched && n) {   // fall through n=0 or no more
    tmpsched = tmpsched->next;
    n--;
  }
  return tmpsched;	    // returns 0 if not found
}
    
int RPCommMng::DeleteSchedEntry(char *str) {
  RPSchedEntry *tmpsched;
    
  if ((tmpsched = GetSchedEntry(str))) {    // GetSchedEntry sets tempschedentry
					    // delete duplicate sched
    if (tmpsched == SchedEntries) {    // if first, get new first
      SchedEntries = SchedEntries->next;
    }
    if (tmpsched == ThisSchedEntry)
      ThisSchedEntry = SchedEntries;
    delete tmpsched;
    return 0;
  }
  else return -1;
}    
    
int RPCommMng::DeleteSchedEntry(RPSchedEntry *schedentry) {
  RPSchedEntry *tmpsched;
    
  if ((tmpsched = GetSchedEntry(schedentry))) {    // GetSchedEntry sets tempschedentry
    // delete duplicate sched
    if (tmpsched == SchedEntries) {    // if first, get new first
      SchedEntries = SchedEntries->next;
    }
    if (tmpsched == ThisSchedEntry)
      ThisSchedEntry = SchedEntries;
    delete tmpsched;
    return 0;
  }
  else return -1;
}    
    
int RPCommMng::DeleteSchedEntryNum(int n) {
  RPSchedEntry *tmpsched;
    
  if ((tmpsched = GetSchedEntryNum(n))) {    // GetSchedEntry sets tempschedentry
    // delete duplicate sched
    if (tmpsched == SchedEntries) {    // if first, get new first
      SchedEntries = SchedEntries->next;
    }
    if (tmpsched == ThisSchedEntry)
      ThisSchedEntry = SchedEntries;
    delete tmpsched;
    return 0;
  }
  else return -1;
}    
    
int RPCommMng::ModifySchedEntry(char *str, int period, int offset) {
  RPSchedEntry *tmpsched;

  if ((tmpsched = GetSchedEntry(str))) {    // GetSchedEntry sets tempschedentry
    tmpsched->period = period;
    tmpsched->offset = offset;
    return 0;
  }
  else return -1;
}

int RPCommMng::ModifySchedEntryNum(int n, int period, int offset) {
  RPSchedEntry *tmpsched;

  if ((tmpsched = GetSchedEntryNum(n))) {    // GetSchedEntry sets tempschedentry
    tmpsched->period = period;
    tmpsched->offset = offset;
    return 0;
  }
  else return -1;
}

int  RPCommMng::FirstSchedString(char *str) {

  ThisSchedEntry = SchedEntries;
  if (ThisSchedEntry) {
    ThisSchedEntry->String(str);
    return 0;
  }
  else return -1;
}

int  RPCommMng::NextSchedString(char *str) {

  ThisSchedEntry = ThisSchedEntry->next;
  if (ThisSchedEntry) {
    ThisSchedEntry->String(str);
    return 0;
  }
  else return -1;
}

/* locate req entry with same stn,type,angle request */
RPCommReq* RPCommMng::GetReqEntry(char *str) {
  RPCommReq localentry;
    
  localentry.DecodeShortString(str);  // convert string to sched entry
  tempreqentry = ReqPending;  // search list for match
  while (tempreqentry &&	    // fall through on match or no more
	 (!localentry.ShortIsSame(tempreqentry)))
    tempreqentry = tempreqentry->next;
  return tempreqentry;	    // returns 0 if not found
}
    
int RPCommMng::DeleteReqEntry(char *str) {
  RPCommReq *tmpreq;
    
  if ((tmpreq = GetReqEntry(str))) {    // GetReqEntry sets tempreqentry
    if (tmpreq->AttemptInProg) return -1;
    if (tempreqentry == ReqPending)  // delete duplicate req
      ReqPending = ReqPending->next;
    tempreqentry = tempreqentry->next;
    delete tmpreq;
    return 0;
  }
  else return -1;
}    
    
int  RPCommMng::FirstReqString(char *str) {
  tempreqentry = ReqPending;
  if (tempreqentry) {
    tempreqentry->ShortString(str);
    return 0;
  }
  else return -1;
}

int  RPCommMng::NextReqString(char *str) {
  tempreqentry = tempreqentry->next;
  if (tempreqentry) {
    tempreqentry->ShortString(str);
    return 0;
  }
  else return -1;
}

RPCommReq* RPCommMng::GetBestReqForRxDev(rxdevice *RxDev) {
  time_t oldest = 0, now = time(0), elapsed = 0;
  RPCommReq *temp = ReqPending, *oldestreq = 0;
    
  while (temp) {
    if (!temp->AttemptInProg) {
      if (temp->GetConnForRxDev(RxDev)) {
	//	    elapsed = now - temp->TimePutOnQ;
	elapsed = now - temp->TimeLastAttempt;  // use time since last attempt
	if (elapsed > oldest) {
	  oldestreq = temp;
	  oldest = elapsed;
	}
      }
    }
    temp = temp->next;
  }
  return oldestreq;
}

int dbgint = 0;

void RPCommMng::CheckReq() {
  rxdevice *RxDev, *nextRxDev;
  RPCommReq *best_req;
  bool ReqListDelta = FALSE;
    
  RxDev = RxDevInUse;
  while (RxDev) {                 // look for finished RxDev
    nextRxDev = RxDev->next;    // remove them from InUse list
    if ((RxDev->ServiceType == RPS_REQUEST)) {
      if (RxDev->GetStatus() == REQCOMPLETE) {
	RxDev->Set_Clear(); // don't free RxDev yet, initiate clear
	// RxDev will become idle then be freed
      }
      else if (RxDev->GetStatus() == REQFAILED) {
	if (RxDev->CommReq) {
	  ReqListDelta = TRUE;
	  RxDev->CommReq->TimeLastAttempt = time(0);
	}
	if (RxDev->CommReq->ThisConn)
	  RxDev->CommReq->ThisConn->Conn->ConnFails++;
	RxDev->Set_Clear();   // don't free RxDev yet, initiate clear
	// RxDev will become idle then be freed
      }
      else if (RxDev->GetStatus() == RDRIDLE) {	// remove dead RxDev
	if (RxDev->CommReq) {
	  if (RxDev->CommReq->ThisConn)
	    RxDev->CommReq->ThisConn->Conn->InUse = FALSE;
	  RxDev->CommReq->AttemptInProg = FALSE;
	  RxDev->CommReq->rxdev = 0;
	  if (RxDev->CommReq->ReqCompleted)
	    RemReq(RxDev->CommReq);
	  else
	    RxDev->CommReq->TimeLastAttempt = time(0);
	  RxDev->CommReq = 0;
	  ReqListDelta = TRUE;
	}
	FreeRxDev(RxDev);
      }
    }
    RxDev = nextRxDev;
  }
  RxDev = RxDevFree;		// check RxDevfree for something that
  while (RxDev) {		// may be used to satisfy a CommReq
    nextRxDev = RxDev->next;
    if (((RxDev->GetStatus() != RDRIDLE) &&
	 (RxDev->GetStatus() != COMMCLEAR)) ||
	RxDev->CommReq ||
	RxDev->newcommreq)	// shouldn't be on free list
      {
	sprintf(LogStr,"RPCommMng::CheckReq found RxDevice%d %s: on RxDevFree list. State=%s\n", 
		RxDev->ID, RxDev->Description, RPCommStatusString[RxDev->GetStatus()]);
	RapicLog(LogStr, LOG_ERR);
	UseRxDev(RxDev);		// put back on in use list
      }
    else if (ReqPending &&
	     (RxDev->GetStatus() == RDRIDLE)) {		// find most deserving request for this handler
      if ((best_req = GetBestReqForRxDev(RxDev))) {
	RxDev->AutoReconnect = FALSE;
	UseRxDev(RxDev);
	RxDev->Set_NewConnect(best_req);
	ReqListDelta = TRUE;
#ifndef NO_XWIN_GUI
	UpdateCommMngWid();
#endif
      }
    }
    RxDev = nextRxDev;
  }
  RPCommReq 
    *next_req,			// remove any dead requests
    *this_req = ReqPending;
  while (this_req) {
    next_req = this_req->next;
    if (this_req->AttemptInProg && (this_req->rxdev == NULL))
      {
	sprintf(LogStr,"RPCommMng::CheckReq - Req AttemptInProg set but no rxdev Req - Stn=%s Type=%s\n",
		StnRec[this_req->stn_id].Name,RPScanTypeStringReadable[this_req->scan_type]);
	RapicLog(LogStr, LOG_ERR);
	this_req->AttemptInProg = FALSE;	    
      }
    if (this_req->rxdev &&    // check rxdev is still servicing req
	!((this_req->rxdev->CommReq == this_req) ||	//either CommReq or newcommreq should match this_req
	  (this_req->rxdev->newcommreq == this_req))) {
      fprintf(stderr, "RPCommMng::CheckReq Orphaned CommReq Found."
	      " Offending rxdevice ID = %2d\n", 
	      this_req->rxdev->ID);
      this_req->AttemptInProg = FALSE;    // if not, free up this request
      this_req->rxdev = NULL;
      if (this_req->ReqCompleted)
	RemReq(RxDev->CommReq);
      ReqListDelta = TRUE;
    }
    if (this_req && !this_req->ConnList) {
      sprintf(LogStr,"RPCommMng::CheckReq - Removing Req - Stn=%s Type=%s\n",
	      StnRec[this_req->stn_id].Name,RPScanTypeStringReadable[this_req->scan_type]);
      RapicLog(LogStr, LOG_ERR);
      if (this_req->rxdev) {
	fprintf(stderr, "RPCommMng::CheckReq About to remove request which still refers to rxdevice #%2d\n", 
		this_req->rxdev->ID);
      }
      this_req->AttemptInProg = FALSE;    // if not, free up this request
      this_req->rxdev = NULL;
      RemReq(this_req);
      ReqListDelta = FALSE;	// remreq will update list
    }
    this_req = next_req;
  }
  if (ReqListDelta) {
#ifndef NO_XWIN_GUI
    ReqEditUpdateList(ReqEditWid);

#endif
  }
}
	
void RPCommMng::FallBackDetected() {
}

void RPCommMng::FallForwardDetected() {
}

void RPCommMng::SetPerStnFallback(int stnid) {
  RPSchedEntry	*thissched = SchedEntries;
  rxdevice	*thisRxDev;
  FallbackComms	*fbcomms;

  while (thissched) {
    if (thissched->CommReq.stn_id == stnid) {
      thissched->fallbackmode = TRUE;  // 
    }
    thissched = thissched->next;
  }
  thisRxDev = RxDevHead;
  while (thisRxDev) {   
    fbcomms = thisRxDev->FBComms;
    while (fbcomms) {
      if ((stnid = fbcomms->Stn)) fbcomms->FBDetected = TRUE;
      fbcomms = fbcomms->next;
    }
    thisRxDev = thisRxDev->adminnext;
  }
}

time_t LastCiscoCheckTime = 0;

void RPCommMng::InitFBDetRxDev() {
  rxdevice	*thisRxDev;
  FallbackComms	*fbcomms;
  thisRxDev = RxDevHead;
  while (thisRxDev) {   
    fbcomms = thisRxDev->FBComms;
    while (fbcomms) {
      fbcomms->FBDetected = FALSE;
      fbcomms = fbcomms->next;
    }
    thisRxDev = thisRxDev->adminnext;
  }
}

void RPCommMng::CheckFBDetRxDev() {
  rxdevice	*thisRxDev;
  FallbackComms	*fbcomms;
  thisRxDev = RxDevHead;
  while (thisRxDev) {   
    fbcomms = thisRxDev->FBComms;
    while (fbcomms) {
      if (fbcomms->CheckFallback(this))
	// checkfallback will enable/disable fallback schedule/RxDev as appropriate
	fbcomms = fbcomms->next;
    }
    thisRxDev = thisRxDev->adminnext;
  }
}

void RPCommMng::CheckNetworkStatus() {
  FILE *ciscofile;
  char tempstr[256];
  int stnid = 0;
  RPSchedEntry *thissched = SchedEntries;
  struct stat localstat;
    
    
  if (stat("cisco_status.warn", &localstat) < 0) return;
  if (localstat.st_mtime == LastCiscoCheckTime) return;
  sec_delay(0.5);	    // try to avoid reading while file being written
  LastCiscoCheckTime = localstat.st_mtime;
  printf("RPCommMng::CheckCiscoStatus - cisco_status.warn status changed\n");
  while (thissched) {	    // save current state, and reset to no fallback
    thissched->prevfallback = thissched->fallbackmode;
    thissched->fallbackmode = FALSE;    // all default to no fallback
    thissched = thissched->next;
  }
  InitFBDetRxDev();
  if ((ciscofile = fopen("cisco_status.warn", "r"))) {
    while (fgets(tempstr, 256, ciscofile)) {
      if (sscanf(tempstr, "Fallback mode invoked for stnid=%d", &stnid) == 1) {
	SetPerStnFallback(stnid);
	printf("RPCommMng::CheckCiscoStatus - Fallback mode detected for stnid=%d\n", stnid);
      }
    }
  }
  CheckFBDetRxDev();
  thissched = SchedEntries;
  while (thissched) {		// look for fallback state change
    if (thissched->prevfallback != thissched->fallbackmode) {
      thissched->CalcSchedTime(FallBackMode);
      if (thissched->fallbackmode) {
	if (thissched->fallbackperiod)
	  fprintf(stderr, "Sched entry FALLBACK - %s %s New period=%d\n", 
		  StnRec[thissched->CommReq.stn_id].Name, 
		  RPScanTypeStringReadable[thissched->CommReq.scan_type], 
		  int(thissched->fallbackperiod/60));
	else
	  fprintf(stderr, "Sched entry FALLBACK - %s %s ENTRY DISABLED\n", 
		  StnRec[thissched->CommReq.stn_id].Name, 
		  RPScanTypeStringReadable[thissched->CommReq.scan_type]);
      }
      else
	fprintf(stderr, "Sched entry FALLFORWARD - %s %s New period=%d\n",
		StnRec[thissched->CommReq.stn_id].Name, 
		RPScanTypeStringReadable[thissched->CommReq.scan_type], 
		int(thissched->period/60));
    }
    thissched = thissched->next;
  }
}

void RPCommMng::GetFallBackMode() { // sets the GLOBAL (affects ALL stns) FallBackMode
  int fd, numrd;
  char tempstr[128];
  struct stat localstat;
  if (CheckFallBackMode) {
    fd = open(FallBackFName, O_RDONLY);
    if (fd >= 0) {
      if (!FallBackMode)	// new fallback occurred
	FallBackDetected();
      FallBackMode = TRUE;
      close(fd);
    }
    else {
      if (FallBackMode) // new fall forward detected
	FallForwardDetected();
      FallBackMode = FALSE;
    }
  }
  else FallBackMode = FALSE;
  fd = open(LinkStatusFName, O_RDONLY);
  if (fd >= 0) {
    fstat(fd, &localstat);   // only update status when file changed
    if (localstat.st_mtime != LinkStatusFTime) {
      sprintf(LogStr, "RPCommMng::GetFallBackMode() - %s CHANGED\n", LinkStatusFName);
      RapicLog(LogStr, LOG_ERR);
      LinkStatusFTime = localstat.st_mtime;
      numrd = read(fd, tempstr, 127);
      if (numrd > 0)
	tempstr[numrd-1] = 0;	// CRUDE, write over last \n
#ifndef NO_XWIN_GUI
      if (CommMngWid && (numrd > 0))
	CommMngSetLinkStatus(CommMngWid, tempstr);
#endif
    }
    close(fd);
  }
#ifndef NO_XWIN_GUI
  else 
    if (CommMngWid) 
      CommMngSetLinkStatus(CommMngWid, "Status Unavailable");
#endif
  CheckNetworkStatus();
}

void RPCommMng::CheckSched() {
  RPSchedEntry	*thissched = SchedEntries;
  time_t  timenow = time(0);
  RPCommReq *NewReq;

  CheckNewScans();    // check scans received, for backup mode schedule entries
 
  if (!SchedEnabled)
    return;

  GetFallBackMode();
  while (thissched) {
    if (thissched->schedtime &&	//schedtime of 0 will never trigger
	(thissched->schedtime <= timenow)) {
      NewReq = new RPCommReq;
      *NewReq = thissched->CommReq;
      thissched->CalcSchedTime(FallBackMode);
      AddReq(NewReq);
    }
    thissched = thissched->next;
  }
}


rxdevice *RPCommMng::NewRxDev(ConnModes ConnMd, char *port) {
    
  rxdevice *newh = 0;
  Comm *comm = 0;
    
  switch (ConnMd) {
  case CM_SERIAL:
    comm = new Serial();
    break;
  case CM_X28:
    comm = new X28();
    break;
  case CM_HAYES:
    comm = new Hayes();
    break;
  case CM_SOCKX25:
    comm = new SockX25();
    break;
  case CM_SOCKET:
    comm = new rpSocket();
    break;
  default:
    sprintf(LogStr,"RPCommMng::NewRxDev - Unsupported device type = %d\n",ConnMd);
    RapicLog(LogStr, LOG_ERR);
  }
  if (comm) {
    comm->Init();
    if (port) comm->Open(port);
    else comm->Open();
    newh = new rxdevice(comm);
    if (ConnMd == CM_SERIAL)    // opening actually connects
      comm->Disconnect();	    // disconnect it
    else if (ConnMd == CM_HAYES)
      comm->Close();
  }
  if (newh) {
    newh->ID = NextRxID;
    NextRxID++;
    SetRxDevDefaults(newh);
    if (!SingleThreadComms) {
      if (newh->startThread()) {
	newh->adminprev = RxDevTail;
	newh->adminnext = 0;
	if (RxDevTail) 
	  RxDevTail->adminnext = newh;
	RxDevTail = newh;
	if (!RxDevHead)	    // first in list 
	  RxDevHead = newh; 
      }
      else  {	
	delete newh;
	newh = 0;
      }
    }
  }
    
  return newh;
}

void RPCommMng::UseRxDev(rxdevice *useRxDev) {
  if (!useRxDev) return;
  if (useRxDev == RxDevFree) RxDevFree = useRxDev->next; // remove from RxDevFree list
  if (useRxDev->next) useRxDev->next->prev = useRxDev->prev;
  if (useRxDev->prev) useRxDev->prev->next = useRxDev->next;
  useRxDev->prev = 0;
  useRxDev->next = RxDevInUse;       // add RxDev to in use list
  if (RxDevInUse) RxDevInUse->prev = useRxDev;
  RxDevInUse = useRxDev;
  useRxDev->ListStatus = HLS_INUSE;
  useRxDev->setInUse(true);
}

void RPCommMng::FreeRxDev(rxdevice *freeRxDev) {
  if (!freeRxDev) return;
  if (freeRxDev == RxDevInUse) RxDevInUse = freeRxDev->next; // remove from RxDevInUse list
  if (freeRxDev->next) freeRxDev->next->prev = freeRxDev->prev;
  if (freeRxDev->prev) freeRxDev->prev->next = freeRxDev->next;
  freeRxDev->prev = 0;
  freeRxDev->next = RxDevFree;                     // add RxDev to free list
  if (RxDevFree) RxDevFree->prev = freeRxDev;
  RxDevFree = freeRxDev;
  freeRxDev->Set_Clear();
  freeRxDev->ListStatus = HLS_FREE;
  freeRxDev->setInUse(false);	// disable RxDev until in use again
}


txdevice* RPCommMng::NewTxDevice(ConnModes ConnMd, char *port) {
  txdevice *new_txdevice = 0;
  Comm *comm = 0;

  switch (ConnMd) {
  case CM_SERIAL:
    comm = new Serial();
    break;
  case CM_X28:
    comm = new X28();
    break;
  case CM_SOCKX25:
    comm = new SockX25();
    break;
  case CM_SOCKET:
    comm = new rpSocket();
    break;
  default:
    sprintf(LogStr,"RPCommMng::NewTxDevice - Unsupported device type = %d\n",ConnMd);
    RapicLog(LogStr, LOG_ERR);
  }
  if (comm) {
    comm->Init();
    if (port) comm->Open(port);
    else comm->Open();
    new_txdevice = new txdevice(comm);
  }
  if (new_txdevice) {
    new_txdevice->ID = NextTxID;
    NextTxID++;
    SetTxDevDefaults(new_txdevice);
    if (!SingleThreadComms) {
      if (new_txdevice->startThread()) {
	new_txdevice->prev = TxDevTail;
	new_txdevice->next = 0;
	if (TxDevTail) 
	  TxDevTail->next = new_txdevice;
	if (!TxDevHead)
	  TxDevHead = new_txdevice;
	TxDevTail = new_txdevice;
	if (ScanMng) ScanMng->AddClient(new_txdevice);
      }
      else {
	delete new_txdevice;
	new_txdevice = 0;
      }
    }
  }
  return new_txdevice;
}

int RPCommMng::TxDevicesListening() {
  int temp = 0;
  txdevice *temptxdevice = TxDevHead;
  while (temptxdevice && temptxdevice->CommHandler) {
    if (temptxdevice->CommHandler->ConnState == IDLE) ;
    if (temptxdevice->CommHandler->ConnState == LISTENING) temp++;
    temptxdevice = temptxdevice->next;
  }
  return temp;
}

void RPCommMng::CheckTxDeviceListenCount() {
  int temp = TxDevicesListening();
  if (temp != RPSrv_listenq) {
    fprintf(stderr, "RPCommMng::CheckTxDeviceListenCount() - NOTICE: TxDevicesListening <> RPSrv_listenq\n"
	    "TxDevicesListening=%d, RPSrv_listenq=%d resetting to %d\n", 
	    temp, RPSrv_listenq, temp);
  }
  SetRPSrvListenQ(temp);
}

#ifndef NO_XWIN_GUI
void RPCommMng::OpenCommMngWid() {
  if (!CommMngWid) {
    LinkStatusFTime = 0;
    if ((CommMngWid = ::OpenCommMngWid((void *)this)))
      UpdateCommMngWid(true);
  }
  else RaiseParentWid(CommMngWid);
}

void RPCommMng::CloseCommMngWid() {
  if (CommMngWid) {
    CloseWid(CommMngWid);
    CommMngWid = 0;
  }
}

void RPCommMng::CommMngWidClosed() {
  CommMngWid = 0;
}

void RPCommMng::UpdateCommMngWid(bool force) {
  rxdevice *tempRxDev;
  txdevice *temptxdevice = 0;
  char tempstr[64];

  if (newCommMngWid) newUpdateCommMngWid(force);
  if (!CommMngWid) return;
  CommMngWidClearList(CommMngWid);
  tempRxDev = RxDevHead;
  while (tempRxDev) {
    sprintf(tempstr, "%2drx %-10s %4s %7s %7s %d", 
	    tempRxDev->ID, 
	    RPCommStatusString[tempRxDev->GetStatus()], 
	    RPCommSrvTypeString[tempRxDev->ServiceType], 
	    tempRxDev->RadarName, 
	    ConnModeString[tempRxDev->CommHandler->ConnMode], 
	    int(tempRxDev->thread_id));
    CommMngWidAddToList(CommMngWid, tempstr);
    tempRxDev = tempRxDev->adminnext;
  }   
  temptxdevice = TxDevHead;
  while (temptxdevice) {
    sprintf(tempstr, "%2dtx %-10s %7s %s %d", 
	    temptxdevice->ID, 
	    ConnStateStr[temptxdevice->CommHandler->ConnState],  
	    ConnModeString[temptxdevice->CommHandler->ConnMode], 
	    temptxdevice->Description, 
	    int(temptxdevice->thread_id));
    CommMngWidAddToList(CommMngWid, tempstr);
    temptxdevice = temptxdevice->next;
  }
}

void RPCommMng::OpenSchedEditWid() {
  if (!SchedEditWid)
    SchedEditWid = ::OpenSchedEditWid((void *)this);
  else RaiseParentWid(SchedEditWid);
}

void RPCommMng::CloseSchedEditWid() {
  if (SchedEditWid) {
    CloseWid(SchedEditWid);
    SchedEditWid = 0;
  }
}

void RPCommMng::SchedEditWidClosed() {
  SchedEditWid = 0;
}

void RPCommMng::OpenReqEditWid() {
  if (!ReqEditWid)
    ReqEditWid = ::OpenReqEditWid((void *)this);
  else RaiseParentWid(ReqEditWid);
}

void RPCommMng::CloseReqEditWid() {
  if (ReqEditWid) {
    CloseWid(ReqEditWid);
    ReqEditWid = 0;
  }
}

void RPCommMng::ReqEditWidClosed() {
  ReqEditWid = 0;
}
#endif

void RPCommMng::OpenRxDevID(int id, int StatusOnly) {
  rxdevice *tempRxDev;

  tempRxDev = RxDevHead;
  while (tempRxDev) {
    if (tempRxDev->ID == id) {
#ifndef NO_XWIN_GUI
      tempRxDev->OpenCommHndlWid(StatusOnly);
#endif
      tempRxDev = 0;
    }
    if (tempRxDev) tempRxDev = tempRxDev->adminnext;
  }   
}

void RPCommMng::newOpenRxDevID(int id) {
  rxdevice *tempRxDev;

  tempRxDev = RxDevHead;
  while (tempRxDev) {
    if (tempRxDev->ID == id) {
#ifndef NO_XWIN_GUI
      tempRxDev->OpenRxDevStatusWid();
      if (tempRxDev->AlertWid) tempRxDev->ReAlert();
#endif
      tempRxDev = 0;
    }
    if (tempRxDev) tempRxDev = tempRxDev->adminnext;
  }   
}

void RPCommMng::newOpenTxDevID(int id) {
  txdevice *tempTxDev;

  tempTxDev = TxDevHead;
  while (tempTxDev) {
    if (tempTxDev->ID == id) {
#ifndef NO_XWIN_GUI
      tempTxDev->OpenTxDevStatusWid();
      if (tempTxDev->AlertWid) tempTxDev->ReAlert();
#endif
      tempTxDev = 0;
    }
    if (tempTxDev) tempTxDev = tempTxDev->next;
  }   
}

void RPCommMng::DumpSchedState(FILE *dumpfile) {
  RPSchedEntry *thissched = SchedEntries;

  if (!dumpfile)
    dumpfile = stderr;
  while (thissched) { 
    thissched->printState(dumpfile);
    thissched = thissched->next;
  }
}

void RPCommMng::DumpConnState(FILE *dumpfile) {
  RPConnection *thisconn = ConnQueue;

  if (!dumpfile)
    dumpfile = stderr;
  while (thisconn) { 
    thisconn->printState(dumpfile);
    thisconn = thisconn->next;
  }
}

void RPCommMng::writeStatusToFile(char *fname)
{
  FILE *statfile = NULL;
  if (!fname) fname = commstatusfilename;
  char tempfname[128];
  strncpy(tempfname, fname, 120);
  strcat(tempfname, ".temp");
  if ((statfile = fopen(fname, "w")))
    {
      writeStatusToFile(statfile);
      fclose(statfile);
      rename(tempfname, fname);
    }      
}

void RPCommMng::writeStatusToFile(FILE *commstatfile)
{
  rxdevice	*RxDev;
  txdevice	*TxDev;
  
  char tempstr[256];
  time_t timenow = time(0);
  int tempint = RPSrv_port;

  if (!commstatfile) return;
  fprintf(commstatfile, "Rapic Communications Status Report - %s\n", TimeString(timenow, tempstr, true));
  fprintf(commstatfile, "Schedule Enabled = %d\n", SchedEnabled);
  fprintf(commstatfile, "createTxDevOnDemand=%d\n", createTxDevOnDemand);
  fprintf(commstatfile, "createTxDevOnDemand=%d\n", createTxDevOnDemand);
  fprintf(commstatfile, "useIPNameResCache=%d\n", useIPNameResCache);
  if (useIPNameResCache)
    IPNameResCache.dumpStatus(commstatfile);
  fprintf(commstatfile, "Available TxDevices=%d\n", GetRPSrvListenQ());
  if (txdevConnectRefusedCount)
    fprintf(commstatfile, "Txdevice service requests refused=%d\n", txdevConnectRefusedCount);   
  fprintf(commstatfile, "Rapic Serving on port %d\n", tempint);
  if (RPSrv_bind_failed)
    fprintf(commstatfile,"**** BIND TO PORT %d FAILED, UNABLE TO LISTEN FOR INCOMING CONNECTIONS ****\n",
	    tempint);
  fprintf(commstatfile,"Total Comms data received = %1.1fMB\n",float(TotalCommsRx)/1000000.0);
  fprintf(commstatfile, "\n---------Rx Device Alerts---\n");
  RxDev = RxDevHead;
  while (RxDev) {	
    RxDev->writeAlertToFile(commstatfile);
    RxDev = RxDev->adminnext;
  }
  fprintf(commstatfile, "\n---------Tx Devices Alerts---\n");
  TxDev = TxDevHead;
  while (TxDev) {
    TxDev->writeAlertToFile(commstatfile);
    TxDev = TxDev->next;
  }
  fprintf(commstatfile, "\n---------Rx Devices----------\n");
  RxDev = RxDevHead;
  while (RxDev) {	
    RxDev->writeStatusToFile(commstatfile);
    RxDev = RxDev->adminnext;
  }
  fprintf(commstatfile, "\n---------Tx Devices----------\n");
  TxDev = TxDevHead;
  while (TxDev) {
    TxDev->writeStatusToFile(commstatfile);
    TxDev = TxDev->next;
  }
}

#ifndef NO_XWIN_GUI
void RPCommMng::ShowAllAlerts() {
  rxdevice	*RxDev;
  txdevice	*TxDev;

  RxDev = RxDevHead;
  while (RxDev) {	
    if (RxDev->AlertWid) RxDev->ReAlert();
    RxDev = RxDev->adminnext;
  }
  TxDev = TxDevHead;
  while (TxDev) {
    if (TxDev->AlertWid) TxDev->ReAlert();
    TxDev = TxDev->next;
  }
}

void RPCommMng::AcknowledgeAllAlerts() {
  rxdevice	*RxDev;
  txdevice	*TxDev;
  char LogStr[256];

  RxDev = RxDevHead;
  while (RxDev) {	
    if (RxDev->AlertWid)
      ((RxDevAlertWindow *)RxDev->AlertWid)->hide();
    RxDev = RxDev->adminnext;
  }
  TxDev = TxDevHead;
  while (TxDev) {
    if (TxDev->AlertWid)
      ((RxDevAlertWindow *)TxDev->AlertWid)->hide();
    TxDev = TxDev->next;
  }
  sprintf(LogStr,"ALL ALERTS ACKNOWLEDGED BY USER INTERFACE\n");
  RapicLog(LogStr, LOG_WARNING);
}

void RPCommMng::newOpenCommMngWid() {
  if (!newCommMngWid) {
    newCommMngWid = (void *) new CommMngWindowClass("Rapic Comms Manager");	    
    ((CommMngWindowClass *)newCommMngWid)->show();
    newUpdateCommMngWid();
  }
  else {
    ((CommMngWindowClass *)newCommMngWid)->raise();
  }
}
    	
void RPCommMng::newCloseCommMngWid() {
  if (!newCommMngWid) return;
  delete (CommMngWindowClass *)newCommMngWid;
  newCommMngWid = 0;
}

void RPCommMng::newUpdateCommMngWid(bool force) {
  if (!newCommMngWid) return;
  ((CommMngWindowClass *)newCommMngWid)->upDate(this, force);
}

#endif

stnScanStats* stnScanStatsMap::getScanStats(int rdr, bool createflag)
{
  map <int, stnScanStats*>::iterator iter;
  iter = scanStatsMap.find(rdr);
  if (iter != scanStatsMap.end())
    return iter->second;
  else
    {
      if (createflag)
	return createScanStats(rdr);
      else
	return NULL;
    }
}

stnScanStats* stnScanStatsMap::createScanStats(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (!stats)
    {
      stats = new stnScanStats();
      scanStatsMap[rdr] = stats;
    }
  return stats;
}
  

void stnScanStatsMap::incNewScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->newScans++;
  else
    fprintf(stderr, "stnScanStatsMap::incNewScans - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

void stnScanStatsMap::incDupScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->dupScans++;
  else
    fprintf(stderr, "stnScanStatsMap::incDupScans - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

void stnScanStatsMap::incIncompleteScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->incompleteScans++;
  else
    fprintf(stderr, "stnScanStatsMap::incIncompleteScans - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

void stnScanStatsMap::incFaultScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->faultScans++;
  else
    fprintf(stderr, "stnScanStatsMap::incFaultScans - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

void stnScanStatsMap::setLastScanTime(int rdr, time_t tm)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->lastScanTime = tm;
  else
    fprintf(stderr, "stnScanStatsMap::setLastScanTime - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

void stnScanStatsMap::setLastScanRxTime(int rdr, time_t tm)
{
  stnScanStats *stats = getScanStats(rdr, true);
  if (stats)
    stats->lastScanRxTime = tm;
  else
    fprintf(stderr, "stnScanStatsMap::setLastScanRxTime - Warning - Failed to create stnScanStats for rdrid=%d\n",
	   rdr);
}

int  stnScanStatsMap::getNewScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->newScans;
  else return 0;
}

int  stnScanStatsMap::getDupScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->dupScans;
  else return 0;
}

int  stnScanStatsMap::getIncompleteScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->incompleteScans;
  else return 0;
}

int  stnScanStatsMap::getFaultScans(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->faultScans;
  else return 0;
}

time_t stnScanStatsMap::getLastScanTime(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->lastScanTime;
  else return 0;
}
  
time_t stnScanStatsMap::getLastScanRxTime(int rdr)
{
  stnScanStats *stats = getScanStats(rdr);
  if (stats)
    return stats->lastScanRxTime;
  else return 0;
}
