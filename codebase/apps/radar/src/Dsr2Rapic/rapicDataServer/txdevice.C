/*
 * txdevice.c
 * 
 * Implementation of the 3drapic radar comms txdevice class
 * 
 */
#include "comms.h"
#include "rpcomms.h"
#include "log.h"
#include <ctype.h>
#include "rdrutils.h"
#include "siteinfo.h"
#include <string.h>
#include "rdrscan.h"
#define _BSD_SIGNALS
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include "soundmng.h"
//SD debug 18/7/00
#ifdef STDCPPHEADERS
#include <iostream>
using namespace std;
#else
#include <iostream.h>
#endif

#ifndef NO_XWIN_GUI
#include "RxDevAlertWindow.h"
#endif

RPScanFilter::RPScanFilter(char *filterstr) {
    next = prev = 0;
    LastSentString = new char[64];
    strcpy(LastSentString, "Undefined");
    LastSentStatus = new char[32];
    strcpy(LastSentStatus, "");
    useRawFilterStr = false;
    DecodeFilterStr(filterstr);
    }

RPScanFilter::~RPScanFilter()
{
  if (LastSentString)
    delete[] LastSentString;
  if (LastSentStatus)
    delete[] LastSentStatus;
}

/*
// Allow partial filter strings. All fields default to allow all
// e.g. RPFILTER:2	would allow all Melb scans to pass
bool RPScanFilter::DecodeFilterStr(char *decodestr) {
char *filterstr;
char sscanfstr[64];

    station = -1;
    scan_type = -1;
    format = -1;
    data_source = -1;
    data_type = -1;
    valid = FALSE;
    if (!decodestr) return FALSE;
    if (filterstr = strstr(decodestr, RPFilterString)) {
	strcpy(sscanfstr, RPFilterString);
	strcat(sscanfstr, "%d:%d:%d:%d:%d");
	if (sscanf(decodestr, sscanfstr, &station, &scan_type, 
	    &format, &data_source, &data_type) > 0) valid =  TRUE;
	}
    return valid;
    }
*/

// Allow partial filter strings. All fields default to allow all
// e.g. RPFILTER:2	would allow all Melb scans to pass
bool RPScanFilter::DecodeFilterStr(char *decodestr) {
char *filterstr;
char *lasts;		 // used by strtok_r
char **plasts;		 // used by strtok_r
char *nexttok;		 // returned token pointer
char sep_tok[8] = " :";  // seperator tokens
char localstr[128];
char *ch;

    station = -1;
    scan_type = -1;
    format = -1;
    data_source = -1;
    data_type = -1;
    valid = FALSE;
    if (!decodestr) return FALSE;
    strncpy(localstr, decodestr, 128);

    // Check for raw RPFILTER string i.i. *RPFILTER:.......
    filterstr = strstr(localstr, rawRPFilterString); 
    useRawFilterStr = filterstr != NULL;
    if (useRawFilterStr)
      {
	useRawFilterStr = true;
	rawRPFilterStr = filterstr+1; // skip the *
	if (rawRPFilterStr[rawRPFilterStr.size()-1] == '\\') // remove trailing '/'
	  rawRPFilterStr.erase(rawRPFilterStr.size()-1);

      }
    else
      filterstr = strstr(localstr, RPFilterString);
    if (filterstr) {
      if (!useRawFilterStr)
	rawRPFilterStr = filterstr;  // keep a copy of raw RPFILTER: string
      lasts = NULL;
      plasts = &lasts;
      nexttok = strtok_r(filterstr, sep_tok, plasts);  // skip RPFILTER: 
      if (nexttok &&
	  (nexttok = strtok_r(NULL, sep_tok, plasts))) {
	ch = nexttok;
	station = decode_stnstr(nexttok);
	if (station != 0) valid = TRUE;
	else return FALSE;
      }
      if (nexttok && 
	  (nexttok = strtok_r(NULL, sep_tok, plasts)))
	scan_type = decode_scantypestr(nexttok);	
      if (nexttok && 
	  (nexttok = strtok_r(NULL, sep_tok, plasts)))
	format = decode_formatstr(nexttok);	
      if (nexttok && 
	  (nexttok = strtok_r(NULL, sep_tok, plasts)))
	data_source = decode_datasrcstr(nexttok);	
      if (nexttok && 
	  (nexttok = strtok_r(NULL, sep_tok, plasts)))
	data_type = decode_datatypestr(nexttok);	
    }
    else
      rawRPFilterStr.clear();
    return valid;
}

void  RPScanFilter::MakeFilterString(char *filterstring) {	// convert filter to filter string
    if (useRawFilterStr)
      strcpy(filterstring, rawRPFilterStr.c_str());
    else
      {
        if (data_type == e_vel)
	  sprintf(filterstring, "%s%1d:%1d:%1d:%1d:Refl,Vel", RPFilterString, 
	          station, scan_type, format, data_source);
        else
	  sprintf(filterstring, "%s%1d:%1d:%1d:%1d", RPFilterString, 
	         station, scan_type, format, data_source);
      }
    }

void  RPScanFilter::MakeTextFilterString(char *filterstring) {	// convert filter to filter string
    if (useRawFilterStr)
      strcpy(filterstring, rawRPFilterStr.c_str()+strlen(rawRPFilterString)-1);
    else {
      sprintf(filterstring, "%7.7s|%6.6s|%10.10s|%4.4s|%7.7s|%-19s %s", 
       	      stn_name(station), 
	      get_scan_type_text(scan_type), get_data_fmt_text(format), 
	      get_data_src_text(data_source), get_data_type_text((e_data_type)data_type), 
	      LastSentString, LastSentStatus);
      /*
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: stn_name()=[%s]\n", stn_name(station));
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: get_scan_type_text()=[%s]\n", get_scan_type_text(scan_type));
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: get_data_fmt_text()=[%s]\n", get_data_fmt_text(format));
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: get_data_src_text()=[%s]\n", get_data_src_text(data_source));
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: get_data_type_text()=[%s]\n", get_data_type_text((e_data_type)data_type));
      fprintf(stderr, "RPScanFilter::MakeTextFilterString: LastSentString=[%s]\n", LastSentString.c_str());
      */
      }
    }

bool RPScanFilter::MatchingFmt(rdr_scan *scan) {
  if (format < 0)                // match anything
    return true;
  if (format < e_df_max)         // match against e_data_fmt enum
    return (format == scan->data_fmt);
  else                           // match against number of levels
    return (format == scan->NumLevels);
}


bool RPScanFilter::MatchingScan(rdr_scan *scan) {
bool match;
    match =  (station < 0) ||
      (station == scan->station);
    if ((scan_type == PPI) || (scan_type == CompPPI))
      match &= ((scan->scan_type == PPI) || (scan->scan_type == CompPPI));
    else
      match &=  (scan_type < 0) ||
	(scan_type == scan->scan_type);
    match &= MatchingFmt(scan);
    match &= (data_source < 0) ||
      (data_source == scan->data_source);
    match &= (data_type < 0) ||
      (data_type == scan->data_type);
    return match;
    }
 
txdevice::txdevice(Comm *commhandler) : scan_client() {
    ClientType = SC_TXDEV;
    first = last = tx = 0;
    lastTxScanChangeTime = 0;
    txScanChangeTimeOut = 150;
    next = prev = 0;
    ListenMode= FALSE;
    ListenStr[0] = 0;	// will be initialised with "RPSrv_fd port" on 1st listen
    ID = -1;
    disconnect = reconnect = FALSE;
    clearq = FALSE;
    RdrStatRequest = QueryMode = SemiPerm = FALSE;
    QueryModeTimeout = 5;  // defualt to 5 secs
    lock = new spinlock("txdevice->lock", 500);	// 5 secs
    CommHandler = commhandler;
    MaxScanQSize = 16;
    ScanQSize = 0;	    // queue of filtered scans
    NewScanQSize = 0;	    // queue of new scans
    ScanQHardLimit = 200;   // hard limit of number of newscans to queue
    loadingRecentScans = false;  // typically only used for initial load of data from scanmng recent scans
    ExpBuffRd = new exp_buff_rd;
    TxBuffSize = 2048;
    TxBuff = new char[TxBuffSize];
    TxBuffIPPos = TxBuffOPPos = 0;
    RxBuffSize = 2048;
    RxBuff = new char[RxBuffSize];
    RxBuffPos = 0;
    new_connstr[0] = 0;
    ln_sz = 0;
    AutoReconnect = TRUE;
    ReconnectRetryTime = 0;
    ReconnPeriod = 10;
    ReconnDelay = 2;
    CallingMode = FALSE;
    TxFinishedScans = TRUE;
    AcceptDuplicates = FALSE;	// as a scan client, don't give me duplicate scans
    TempFilters = PermFilters = 0;
    FilterCount = 0;
    FiltersChanged = TRUE;
    Expect_Ack = FALSE;
    SendTxStnSet = FALSE;
    SendRefTime = FALSE;
    Description[0] = 0;
    Ack_Match = UNDEF;
    Ack_FailCount = 0;
    Ack_Timeout = 0;
    AckFailTimeout = 30;	// max time to wait for scan ack
    strcpy(Description, "");    // default is incoming request handler
    EnableComms = FALSE;    // intially disable comms processing
    RPStatus = DISABLED;
    ConnectFailTime = 0; // time connection failed, reset to 0 on successful connect
    lastConnectTime = 0;
    StatusChanged = FilterListStatusChanged = TRUE;
    transientDevice = false;
    transientTimeout = 0;
#ifndef NO_XWIN_GUI
    TxDevStatusWid = 0;
    AlertWid = 0;
#endif
    loopDelay = 0.2;
    Re_AlertTime = 0;
    Re_AlertPeriod = 15 * 60;	// default to 15 minutes
    AlertFirstPosted = 0;	    
    SilenceReAlert = SilenceFutureAlert = 
      SuppressAlerts = SuppressReAlert = false;
    last_alertstatus = this_alertstatus = AL_OK;
    AlertAckFailFlag = 0;
    strcpy(LastAlertStr, "");
    strcpy(ConnFailSound, "");
    strcpy(AckFailSound, "");
    silent = false;
    debuglevel = 0;
    strcpy(threadName, "txdevice");
    lastScanTxTime = 0;
    strcpy(lastScanTxString, "");
    }
    
txdevice::~txdevice() {
  // int waittime = int(DLY_TCK * 30);  //was 10,  SD 18/7/00
    if (next) next->prev = prev;
    if (prev) prev->next = next;
    fprintf(stderr, "txdevice %d: ~txdevice stopping thread\n", ID);
    stopThread();
    do_clearscanq();
    ClearPermFilters();
    ClearTempFilters();
    if (CommMngr && (CommHandler->ConnState == LISTENING))
	CommMngr->DecRPSrvListenQ();		// Dec listen queue size
    delete CommHandler;
    CommHandler = 0;
     delete lock;
    lock = 0;
    delete ExpBuffRd;
    ExpBuffRd = 0;
    delete[] TxBuff;
    TxBuff = 0;
    delete[] RxBuff;
    RxBuff = 0;
#ifndef NO_XWIN_GUI
    if (TxDevStatusWid)
	CloseTxDevStatusWid();
    if (AlertWid) 
      {
	delete (RxDevAlertWindow *)AlertWid;
	AlertWid = NULL;
      }
#endif
    }
    
void txdevice::Enable()
{
    EnableComms = true;
    do_newrpstatus(RDRIDLE);
}

void txdevice::Disable()
{
    EnableComms = false;
    do_newrpstatus(DISABLED);
    Re_AlertTime = 0;
    this_alertstatus = AL_OK;
#ifndef NO_XWIN_GUI
    UnPostTxDevAlert();
#endif
}

void txdevice::Reset()
{
    TxFinishedScans = TRUE;
    TxLatestScan = FALSE;
    AcceptDuplicates = FALSE;	// as a scan client, don't give me duplicate scans
    AllowReplayMode = FALSE;
    if ((sendRecentDataMins > 10) && ScanMng && ScanMng->keepScansInCache)
      {
	sendRecentDataMins = 10;
	do_clearscanq();
	loadingRecentScans = true;
	ScanMng->getRecentScans(sendRecentDataMins, 0, this);  // get recent scans from scanDataCache
	CheckNewScans();	// process newscans queue, ignore MaxScanQSsize
	loadingRecentScans = false;
      }
    sendRecentDataMins = 10;
    sendRecentDataFrom = 0;
    ResetPartialTx();
    if (!SemiPerm) tx = 0;  // if semiperm, retain tx
    if (TempFilters) ClearTempFilters();
    Re_AlertTime = 0;
#ifndef NO_XWIN_GUI
    UnPostTxDevAlert();
#endif
}

bool txdevice::isEnabled()
{
    return EnableComms;
}
    
void txdevice::CheckNewConnection()
{
  int WaitForQuery;

  SetAlertStatus();
  do_newrpstatus(RDRCONNECTED);
  ln_sz = 0;
  RdrStatTimeOut = time(0) + RDRSTATTMOUT;
  RdrStatMssgTime = time(0) + RDRSTATMSSGTM;
  TxFinishedScans = TRUE;
  TxLatestScan = FALSE;
  RdrStatRequest = TRUE;
  WaitForQuery = QueryModeTimeout * 2;	// wait for query string ( default 5 secs)
  QueryMode = SemiPerm = FALSE;
  while (!QueryMode && WaitForQuery) {    // will wait until query detected or QueryModeTimeout
    CheckDataRx();			    // this should allow plenty of time for filters
    if (!QueryMode) {		    // in semiperm mode to be rx'd
      sec_delay(0.5);		    // if not all filters are rx'd and TxLatestScan has been
      //fprintf(stderr, "q");;	    // received, "latest" scans for subsequent filters will be
    }				    // purged, but subsequent matching scans will be OK
    WaitForQuery--;
  }
  if (!QueryMode && !SemiPerm) 
    fprintf(stderr, "txdevice::CheckNewConnection #%d - No Query detected in %d secs.\n", ID, QueryModeTimeout);
  if (TxLatestScan)	// remove old scans from queue
    do_clearscanq();
  if (sendRecentDataMins && SemiPerm && ScanMng && ScanMng->keepScansInCache)
    {
      fprintf(stdout, "txdevice%d CheckNewConnection - Loading up to %d mins of recent data from cache\n", 
	      ID, sendRecentDataMins);
      if (sendRecentDataFrom)
	{
	  char timestr[128];
	  fprintf(stdout, "txdevice%d CheckNewConnection - **Loading data from %s onwards only\n", 
		  ID, ShortTimeString(sendRecentDataFrom, timestr));
	}
      do_clearscanq();
      loadingRecentScans = true;
      int minsloaded = ScanMng->getRecentScans(sendRecentDataMins,
					       sendRecentDataFrom, this);  // get recent scans from scanDataCache
      fprintf(stdout, "txdevice%d CheckNewConnection - Loaded %d mins of recent data from cache - "
	      " Checking %d new scans\n", ID, minsloaded, NewScanQSize);
      CheckNewScans();	// process newscans queue, ignore MaxScanQSsize
      loadingRecentScans = false;
      fprintf(stdout, "txdevice%d CheckNewConnection - Check complete, Sending %d scans to remote client\n", 
	      ID, ScanQSize);
    }
}

void txdevice::CheckComms() {
    time_t timenow;
char	tempstr[128];

    if (!EnableComms) {
	if (RPStatus != DISABLED) 
	{
	    Reset();
	    if (CommHandler->isConnected()) {
		CommHandler->Disconnect();
		}
	    do_newrpstatus(DISABLED);
	}
	return;
	}
    if ((strlen(Description) == 0) && (ID >= 0))
	sprintf(Description, "txdevice id=%d", ID);
    timenow = time(0);
    CheckNewScans();	// process newscans queue
    if (clearq) {
	if (lock) lock->get_lock();
	do_clearscanq();
	clearq = FALSE;
	if (lock) lock->rel_lock();
	}
    if (disconnect) {
	Reset();
	CommHandler->Disconnect();
	do_newrpstatus(RDRIDLE);
	RPStatus = COMMCLEAR;
	if (!AutoReconnect)
	    CommHandler->ConnReqCount = 0;
	RdrStatRequest = QueryMode = SemiPerm = FALSE;
	disconnect = FALSE;
	ln_sz = 0;
	}
    if (reconnect) {
	Reset();
	CommHandler->Disconnect();
	sec_delay(float(ReconnDelay));
	CommHandler->Reconnect();
	if (CommHandler->isConnected()) {
	    CheckNewConnection();
	    }
	else {
	    do_newrpstatus(RECONNFAILED);
	    SetAlertStatus(AL_RECONN_FAILED, "Reconnect attempt failed");
	    }
	reconnect = FALSE;
	ln_sz = 0;
	}
    if (new_connstr[0]) {
	do_newrpstatus(RDRCONNECTING);
	if (lock) lock->get_lock();
	strncpy(tempstr, new_connstr, 128);
	if (lock) lock->rel_lock();
	CommHandler->Connect(tempstr);
	if (CommHandler->isConnected()) {
	    CheckNewConnection();
	    }
	else {
	    do_newrpstatus(CONNFAILED);
	    SetAlertStatus(AL_CONN_FAILED, "Connection failed");
	    }
	ln_sz = 0;
	RdrStatRequest = QueryMode = SemiPerm = FALSE;
	RdrStatTimeOut = time(0) + RDRSTATTMOUT;
	RdrStatMssgTime = time(0) + RDRSTATMSSGTM;
	new_connstr[0] = 0;
	}
    if (CommHandler->ConnState == LISTENING) {
	CommHandler->CheckListen();
	RdrStatRequest = QueryMode = SemiPerm = FALSE;
	if (CommHandler->isConnected()) {
	    if (CommMngr)
		CommMngr->DecRPSrvListenQ();		// Dec listen queue size
	    CheckNewConnection();
	    }
	}
    if ((CommHandler->isConnected()) &&	    // check for status timeout
	(time(0) > RdrStatTimeOut) && !CallingMode) {   // DISABLE STATUS TIMEOUT WHEN CALLING OUT
	if (SemiPerm)
	    SetAlertStatus(AL_STAT_FAILED, "Handshake with remote failed, disconnecting");
	Reset();
	do_newrpstatus(RDRSTATUSFAIL);
	ListenMode = TRUE;
	sprintf(LogStr,"TxDevice%d %s: Remote(%s) Status Failed, RESETTING LISTEN\n", ID, Description, ConnectedToString());
	RapicLog(LogStr, LOG_ERR);
	}
    if (ListenMode) {
	Reset();
	CommHandler->Disconnect();
	RPStatus = COMMCLEAR;
	CommHandler->ConnReqCount = 0;
	RdrStatRequest = QueryMode = FALSE;
	if (CommMngr && (CommHandler->ConnState != LISTENING))
	  {
	    CommHandler->Listen(ListenStr);
	    if (CommHandler->ConnState == LISTENING) // if listen ok
	      CommMngr->IncRPSrvListenQ();		// Inc listen queue size
	  }
	SetAlertStatus();
	do_newrpstatus(RDRLISTENING);
	ListenMode = FALSE;
	}
    if (CommHandler->isConnected())
	CheckDataRx();
    if (!ListenMode && !disconnect &&
	(CommHandler->ConnState != LISTENING)) // no point checking tx if still listening
	CheckDataTx();
    if (!CallingMode &&
	!CommHandler->isConnected() &&
	(CommHandler->ConnState != LISTENING))
    {
      // sprintf(LogStr,"TxDevice%d %s: NOT CALLING, NOT CONNECTED AND NOT LISTENING!!,  Setting listen mode\n", ID, Description);
      // RapicLog(LogStr, LOG_ERR);
	ListenMode = true;
    }
	
    //do_checkscanq();  //rem SD 19/7/00
    }
    
void txdevice::workProc() {    
  ThreadObj::workProc();  // perform base class operations

	CheckComms();
}

void txdevice::threadExit()
{
    if (CommHandler->isConnected()) {
	CommHandler->Disconnect();
	}
    fprintf(stderr, "txdevice::threadExit #%d pid=%d, EXITING\n", ID, int(thread_id));
}
    
extern bool signals_handled;
void txdevice::setStopThreadFlag() {
    if (lock) lock->get_lock();
    disconnect = true;
    if (CommHandler)
	CommHandler->terminate = TRUE;
    ThreadObj::setStopThreadFlag();
    if ((thread_id > 0) && signals_handled)
	sendKill(thread_id, SIGHUP);	// try to terminate calls in prog
    if (lock) lock->rel_lock();
    }

bool txdevice::Full(int passedSize) {
    return FALSE;
    }
    
void txdevice::do_clearscanq() {
  bool tx_in_scanq = false;
  rdr_scan_node *temp;
  
  while (first) {
    temp = first->next;
    if (first == tx)
      {
	tx_in_scanq = true;
	fprintf(stderr,"txdevice::do_clearscanq - FOUND tx in scan queue\n");
      }
    delete first;
    decScanNodeCount();
    first = temp;
  }
  if (tx && !tx_in_scanq) {
    delete tx;
    decScanNodeCount();
    ExpBuffRd->open(0);
    lastTxScanChangeTime = 0;
  }
  first = last = tx = 0;
  ScanQSize = 0;
}

void txdevice::do_checkscanq() {
  rdr_scan_node *temp = first;
    while (temp) {
	if (temp == (rdr_scan_node *)0x1)
	{
		fprintf(stderr, "txdevice::do_checkscanq found rdr_scan_node ref to 0x1\n");
		return;
	}	
	if (temp == tx)
	{
		fprintf(stderr, "txdevice::do_checkscanq found tx on scan queue!!!!!\n");
		return;
	}	

	//fprintf(stderr, "txdevice::do_checkscanq: ptr is"); //SD add 19/7/00
	//cerr<<"node addr "<<temp<<'\n';
	
	temp = temp->next;
	}
    }

void txdevice::ClearScanQ() {
    if (lock) lock->get_lock();
    if (thread_id == 0) {
	do_clearscanq();
	}
    else clearq = TRUE;
    if (lock) lock->rel_lock();
    }
    
void txdevice::Set_NewConnect(char *connstr) {
    if (lock) lock->get_lock();
    if (thread_id == 0) {
	Reset();
	CommHandler->Connect(connstr);	// do now if not a thread
	}
    else {
	if (connstr) strncpy(new_connstr,connstr,128);
	else {
	    disconnect = TRUE;
	    CommHandler->lastconnstr[0] = 0;
	    }
	}
    if (lock) lock->rel_lock();
    }
    
void txdevice::Listen(char *str) {
    if (lock) lock->get_lock();
    if ((thread_id == 0) || !EnableComms) {
	Reset();
	CommHandler->Disconnect();	// do now if not sproc'd
	if (!AutoReconnect)
	    CommHandler->ConnReqCount = 0;
	if (CommMngr && (CommHandler->ConnState != LISTENING))
	    CommMngr->IncRPSrvListenQ();		// Inc listen queue size
	if (str) strncpy(ListenStr,str,64);
	CommHandler->Listen(str);
	if (CommMngr && (CommHandler->ConnState != LISTENING))	// if not listening
	    CommMngr->DecRPSrvListenQ();		// Dec listen queue size
	do_newrpstatus(RDRLISTENING);
	}
    else {			// else set variables and let sproc do it
	ListenMode = TRUE;
	if (str) strncpy(ListenStr,str,64);
	}
    if (lock) lock->rel_lock();
    }
    
void txdevice::Disconnect() {
    if (lock) lock->get_lock();
    if (thread_id == 0) {
	Reset();
	CommHandler->Disconnect();	// do now if not sproc'd
	if (!AutoReconnect)
	    CommHandler->ConnReqCount = 0;
	}
    else disconnect = TRUE;
    if (lock) lock->rel_lock();
    }
    
void txdevice::ClearPermFilters() {
RPScanFilter	*NextFilter;
    while (PermFilters) {
	NextFilter = PermFilters->next;
	delete PermFilters;
	PermFilters = NextFilter;
	FilterCount--;
	}	
    FiltersChanged = TRUE;
    }
    
void txdevice::ClearTempFilters() {
RPScanFilter	*NextFilter;
    while (TempFilters) {
	NextFilter = TempFilters->next;
	delete TempFilters;
	TempFilters = NextFilter;
	FilterCount--;
	}	
    fprintf(stderr, "txdevice::ClearTempFilters called\n");
    FiltersChanged = TRUE;
    }
    
void txdevice::AddFilter(RPScanFilter *addfilter, RPScanFilter **DestList) {
char tempstr[128];
    if (!addfilter || !addfilter->valid) return;
    if (lock) lock->get_lock();
    if (!DestList) DestList = &PermFilters;
    if (FilterCount >= RPTXDEVICEFILTERMAX) {
	sprintf(LogStr,"TxDevice%d %s: AddFilter - Perm Scan Filters list full\n", 
	    ID, Description);
	RapicLog(LogStr, LOG_ERR);
	return;
	}
    addfilter->next = *DestList;
    addfilter->prev = 0;
    if (*DestList) (*DestList)->prev = addfilter;
    (*DestList) = addfilter;
    FilterCount++;
    if (lock) lock->rel_lock();
    addfilter->MakeFilterString(tempstr);
    fprintf(stderr,"txdevice %d: AddFilter - Filter added=%s\n",ID, tempstr);
    FiltersChanged = TRUE;
    }
    
void txdevice::AddFilter(char *filterstr, RPScanFilter **DestList) {
RPScanFilter *addfilter;
    addfilter = new RPScanFilter(filterstr);
    if (addfilter->valid) AddFilter(addfilter, DestList);
    else delete addfilter;
    }

/*
 * return TRUE if passed by filter
 */
 
bool txdevice::FilterScan(rdr_scan *scan, RPScanFilter *matchfilter, RPScanFilter **FilterList) {
    bool match = FALSE;
    char tempstr[128]; 
    if (!FilterList) FilterList = &PermFilters;
    if (!*FilterList) {
	matchfilter = 0;
	return TRUE;	    // if no filters, return true
	}
    else {
	matchfilter = *FilterList;
	while (!match && (matchfilter != 0))	{
	    match = matchfilter->MatchingScan(scan);
	    if (match) {
		sprintf(tempstr, "%02d:%02d %02d/%02d/%04d", 
		    scan->hour, scan->min, 
		    scan->day, scan->month, scan->year);
		strncpy(matchfilter->LastSentString, tempstr, 63);
		strncpy(matchfilter->LastSentStatus, "", 31);
		FilterListStatusChanged = TRUE;
		}
	    else matchfilter = matchfilter->next;
	    }
	}
    return match;
    }
    
void txdevice::DelFaulty() {	// remove any faulty scans from queue
  rdr_scan_node *nextnode = 0, *tempnode;
    
  tempnode = first;
  while (tempnode) {
    nextnode = tempnode->next;
    if (tempnode != tx) {		// cannot delete tx node    
      if (tempnode->scan->Faulty()) {
	sprintf(LogStr, "txdevice #%d: %s DelFaulty DELETING FAULTY SCAN\n", 
		ID, Description); 
	RapicLog(LogStr, LOG_INFO);
	if (tempnode == last)
	  last = last->prev;
	if (tempnode == first)
	  first = first->next;
	delete tempnode;
	decScanNodeCount();
	ScanQSize--;
      }
    }
    tempnode = nextnode;
  }
}
    
bool txdevice::DelOldest() {
  rdr_scan_node *oldnode = 0, *tempnode;
  time_t oldtime = 0;
    
  tempnode = first;
  while (tempnode) {
    if (tempnode != tx) {		// cannot delete tx node    
      if (!oldtime) {
	oldtime = tempnode->scan->scan_time_t;	// first time
	oldnode = tempnode;
      }
      else if (tempnode->scan->scan_time_t < oldtime) {
	oldtime = tempnode->scan->scan_time_t;
	oldnode = tempnode;	    
      }
    }
    tempnode = tempnode->next;
  }
  if (oldnode) {
    if (oldnode == last)
      last = last->prev;
    if (oldnode == first)
      first = first->next;
    delete oldnode;
    decScanNodeCount();
    ScanQSize--;
    return true;
  }
  else return false;
}
    
bool txdevice::DelOldestMatching(rdr_scan *scan) {
  rdr_scan_node *oldnode = 0, *tempnode;
  time_t oldtime = 0;
  bool match;
    
  tempnode = first;
  while (tempnode) {
    if ((tempnode->scan != scan) &&	// don't delete this scan
	(tempnode != tx)) {		// cannot delete tx node    
      match = (tempnode->scan->station == scan->station) &&
	(tempnode->scan->scan_type == scan->scan_type) &&
	(tempnode->scan->data_fmt == scan->data_fmt);
      if (match) {
	if (!oldtime) {
	  oldtime = tempnode->scan->scan_time_t;	// first time
	  oldnode = tempnode;
	}
	else if (tempnode->scan->scan_time_t < oldtime) {
	  oldtime = tempnode->scan->scan_time_t;
	  oldnode = tempnode;	    
	}
      }
    }
    tempnode = tempnode->next;
  }
  if (oldnode) 
    {
      if (oldnode == last)
	last = last->prev;
      if (oldnode == first)
	first = first->next;
      delete oldnode;
      decScanNodeCount();
      ScanQSize--;
      return TRUE;
    }
  else return false;
}
    
int txdevice::NewDataAvail(rdr_scan *new_scan) {
  int result = 0;
  if (lock) lock->get_lock();
  if (loadingRecentScans || (NewScanQSize < ScanQHardLimit)) {
    if ((result = scan_client::NewDataAvail(new_scan)))
      {
	NewScanQSize++;
      }
  }
  else {
    sprintf(LogStr, "txdevice::NewDataAvail #%d: %s ERROR: "
	    "ScanQHardLimit EXCEEDED, NEW SCAN NOT ADDED\n", 
	    ID, Description); 
    RapicLog(LogStr, LOG_ERR);
    result = 0;
  }
  if (lock) lock->rel_lock();
  return result;
}
    
void txdevice::CheckNewScans() {
  rdr_scan_node *tempnode, *nextnode;
  bool	flag = TRUE;
  bool	PassFilter = false;
  RPScanFilter *matchfilter = 0;
  char tempstr[256];
  if (lock) lock->get_lock();
  tempnode = newscans;
  while (tempnode) {
    nextnode = tempnode->next;
    if (tempnode->scan &&
	(tempnode->scan->HeaderValid() || tempnode->scan->Finished())) {
      if (tempnode == newscans) {	    // removing head from list
	newscans = tempnode->next;  // update head pointer
      }
      /*
       * If TempFilters are defined,  filter newscans against them
       * Otherwise filter against permanent filters
       */
      if (tempnode->scan->HeaderValid()) { 
	PassFilter = 0;
	matchfilter = 0;
	if (TempFilters)	// check against temp (rx end) filters 
	  PassFilter = FilterScan(tempnode->scan, matchfilter, &TempFilters);
	else		// check against perm filters
	  PassFilter = FilterScan(tempnode->scan, matchfilter); 
	if (PassFilter) {
	  // remove from newscans list
	  if (tempnode->next) tempnode->next->prev = tempnode->prev;
	  if (tempnode->prev) tempnode->prev->next = tempnode->next;
	  // add to txdevice list
	  tempnode->next = first;	// list of scans to send
	  tempnode->prev = 0;	// first in  list
	  if (first) first->prev = tempnode;
	  if (!last) last = tempnode;
	  first = tempnode;
	  ScanQSize++;
	  if (matchfilter) 
	    strncpy(matchfilter->LastSentStatus, "Queued", 31);
	  FilterListStatusChanged = TRUE;
	  //	  DelFaulty();	    // delete any faulty scans
	  // limit queue size to MaxScanQSize or max of one of each product
	  flag = TRUE;
	  while (!loadingRecentScans &&     // if loadingRecentScans set, don't worry about qsizes
		 flag && 
		 (ScanQSize >= MaxScanQSize)) {
	    flag = DelOldestMatching(tempnode->scan);	// false if unable to delete anything
	    if (flag && SemiPerm && 
		(CommHandler->isConnected())){
	      //	      sprintf(LogStr, "txdevice #%d: %s CheckNewScans SEMIPERM TXDEVICE QUEUE OVERFLOW: DELETING OLDEST MATCHING **DATA LOST**\n", 
	      //		      ID, Description); 
	      //	      RapicLog(LogStr, LOG_ERR);
	    }
	    // only delete oldest matching scan
	  }
	}
	else {	// valid but doesn't match filters, delete		
// 	  fprintf(stderr, "txdevice %d::CheckNewScans: Scan doesn't match any filters,"
// 		  "  removing %s\n", ID, tempnode->scan->ScanString2(tempstr));
	  
	  delete tempnode;    // delete will link next/prev past this
	  decScanNodeCount();
	}
      }
      else {	// finished but not valid, delete		
	fprintf(stderr, "txdevice %d::CheckNewScans: Scan not valid,"
		"  removing %s\n", ID, tempnode->scan->ScanString2(tempstr));
	delete tempnode;    // delete will link next/prev past this
	decScanNodeCount();
      }
      NewScanQSize--;
    }
    tempnode = nextnode;
  }
  if (ScanQSize != first->ListRootCount())
    {
      fprintf(stderr, "txdevice%d CheckNewScans - ScanQSize mismatch, resetting from %d to %d\n",
	      ID, ScanQSize, first->ListRootCount());
      ScanQSize = first->ListRootCount();
    }
  if (lock) lock->rel_lock();
}

void txdevice::CheckDataRx() {
    // int numrd = 0;
    char	c;
    //	bool	debug = false;
#define INSTRMAX1 2048
    int		instrmax = INSTRMAX1;
    char	instr[INSTRMAX1];
    int		instrlen,in_pos;
    int		total_rd = 0;
    
    if (CommHandler->commfd < 0) return;
    while ((instrlen = CommHandler->Read(instr,instrmax)) > 0) {
	in_pos = 0;
	total_rd += instrlen;
	while (in_pos < instrlen) {
	    c = instr[in_pos];
	    switch (c) {									// replace all line term with null
		case CR:
		case LF:
		case CTRLZ:
		case 0:
		case EOT:
		case '#':
		    // lastlntime = time(0);	// time line rx'd
		    NewLine();
		    break;
		default:	// don't add leading white space
		    if (!((ln_sz == 0) && isspace(c))) {
			ln_buff[ln_sz] = c;
			ln_sz++;
			if (ln_sz == 1023) NewLine();
			}
		    break;
		}
	    in_pos++;
	    }
	}
    if (!CommHandler->isConnected()) {	// connection lost, reset tx
	sprintf(instr, "%s conn. lost while reading data", ConnectedToString());
	if (AutoReconnect)
	    SetAlertStatus(AL_CONN_LOST, instr);
	do_newrpstatus(RDRCONNLOST);
	if (!ConnectFailTime) ConnectFailTime = time(0);
	ResetPartialTx();
	if (!SemiPerm) tx = 0;  // if semiperm, retain tx
	}
    }
    
void txdevice::NewLine() {
  //	char	c;
  char *tempch = 0;

    
  if ((ln_sz == 0) || (ln_buff[0] == '\0')) return;
  ln_buff[ln_sz] = 0;			// pass null terminated strings
  ln_sz++;
  if (debuglevel > 0) {
    sprintf(LogStr,"TxDevice%d %s: NewLine - %s\n", ID, Description, ln_buff);
    RapicLog(LogStr, DfltLogLevel);
  }
  if (strstr(ln_buff,"RDRSTAT")) {
    if (debuglevel > 0) {
      sprintf(LogStr,"TxDevice%d %s: NewLine - RDRSTAT received\n", ID, Description);
      RapicLog(LogStr, DfltLogLevel);
    }
    RdrStatRequest = TRUE;
    RdrStatTimeOut = time(0) + RDRSTATTMOUT;
    ln_sz = 0;
    ln_buff[0] = 0;
    return;
  }
  if ((tempch = strstr(ln_buff,RPScanAckString)) && tx)
    CheckAckString(tempch,tx->scan);
  if (strstr(ln_buff,RPQueryString)) {
    if (strstr(ln_buff,RPQuerySemiPermStr)) {
      fprintf(stderr,"txdevice %d: NewLine - SemiPermanent connection detected\n",ID);
      SemiPerm = TRUE;
    }
    else {
      if (CommReq.DecodeQueryString(ln_buff)) {
	QueryMode = TRUE;
	sprintf(LogStr,"TxDevice%d %s: NewLine - Query Detected: %s\n", 
		ID, Description, ln_buff);
	RapicLog(LogStr, DfltLogLevel);
      }
      else fprintf(stderr, "TxDevice %d NewLine - Bad query detected: %s\n", ID, ln_buff);
    }
  }
  if (strstr(ln_buff,RPQuery_ScanByScanStr)) {
    fprintf(stderr,"txdevice %d: NewLine - Send Finished Scans = false\n",ID);
    TxFinishedScans = false;
  }
  if (strstr(ln_buff,RPQueryLatestScanStr)) {
    fprintf(stderr,"txdevice %d: NewLine - Send Latest Scan = TRUE\n",ID);
    TxLatestScan = TRUE;
  }
  if (strstr(ln_buff,RPAllowReplayScanStr)) {
    fprintf(stderr,"txdevice %d: NewLine - AllowReplayMode = TRUE\n",ID);
    AllowReplayMode = TRUE;
  }
  if ((tempch = strstr(ln_buff,RPSendRecentDataMins))) {
    int mins = 0;
    char scanstr[64];
    strcpy(scanstr, RPSendRecentDataMins);
    strcat(scanstr, "%d");
    if (sscanf(tempch, scanstr, &mins) == 1)
      {
	fprintf(stderr,"txdevice %d: NewLine - RPSendRecentDataMins = %d\n",ID, mins);
	sendRecentDataMins = mins;
      }
  }
  if ((tempch = strstr(ln_buff,RPSendRecentDataFrom))) {
    int yr, mon, day, hr, min, sec;
    char scanstr[64];
    strcpy(scanstr, RPSendRecentDataFrom);
    strcat(scanstr, "%4d%2d%2d%2d%2d%2d");

    if (sscanf(tempch,scanstr,
	       &yr, &mon, &day, &hr, &min, &sec) == 6) {
      {
	 sendRecentDataFrom = DateTime2UnixTime(yr, mon, day, hr, min, sec);
	fprintf(stderr,"txdevice %d: NewLine - %s\n",ID, tempch);
      }
    }
  }
  if (strstr(ln_buff,RPFilterString)) {
    AddFilter(ln_buff, &TempFilters); // add rx supplied filter
    fprintf(stderr,"TxDevice%d %s: CheckDataRx - Filter string detected - %s\n",ID, Description, ln_buff);
  }
  ln_sz = 0;
  ln_buff[0] = 0;
}
    
    // TxBuffOPPos - pos in TxBuff of output. ie data written to comms
    // TxBuffIPPos - pos in TxBuff of input. ie data from txdevice scan
    // write buffer to commhandler
    // return TRUE if buffer emptied, false if not
    // tmout in 0.01 secs, if > 0 retry until timeout expires
bool txdevice::WriteTxBuff(int tmout) {
  int	temp;
  bool done = TxBuffOPPos >= TxBuffIPPos;	// done if all data sent
  done |= !CommHandler->isConnected();        // or connection broken
  while (!done && !disconnect) {
    temp = CommHandler->Write(TxBuff,TxBuffIPPos - TxBuffOPPos,TxBuffOPPos);
    if (temp > 0) {
      RdrStatTimeOut = time(0) + RDRSTATTMOUT;// reset timeout if data tx'd
      TxBuffOPPos += temp;
    }
    else if (tmout > 0) {	    // if timeout 
      sec_delay(0.5);
      tmout -= 50;
    }
    done = (tmout <= 0) || 
      (!CommHandler->isConnected()) ||
      (TxBuffOPPos >= TxBuffIPPos);
  }
  if ((TxBuffOPPos) >= TxBuffIPPos) {		// buffer write complete
    TxBuffOPPos = TxBuffIPPos = 0;		// reset buffer
    return TRUE;
  }
  else return false;
}
    
bool txdevice::ScanMatchesReq(rdr_scan *scan, RPCommReq *thisReq)
{
  bool match = false;

  if (!scan) return false;
  if (tx->scan->station != CommReq.stn_id)
    return false;
  switch (thisReq->scan_type) {
  case RPT_DFLT_PPI1 :
  case RPT_DFLT_PPI2 :
    if ((scan->scan_type == PPI) ||
	(scan->scan_type == CompPPI))
      match = true;
    break;
  case RPT_USR_PPI :
  case RPT_USR_RHI :
    break;
  case RPT_VOL :
    if (scan->scan_type == VOL)
      match = true;
    break;
  }
  return match;
}

bool txdevice::GetQueryScan() { 
  bool scanfound = false;
    
  //    if (!QueryMode) return scanfound;
  tx = first;
  while (tx && !scanfound) {
    if (QueryMode) {
      scanfound = 
	//		!tx->scan->Faulty() &&
	ScanMatchesReq(tx->scan, &CommReq);
    }
    else {
      scanfound = (tx->scan && tx->scan->station);	// want scan with stn defined
    }
    if (TxFinishedScans && !tx->scan->Finished())
      scanfound = false;
    if (!scanfound) tx = tx->next;
  }
  if (!scanfound) scanfound = GetScanMngScan();
  if (!scanfound) scanfound = GetRealTimeSeqScan();
  if (!scanfound) scanfound = GetLatestDBScan();
  if (!scanfound)
    fprintf(stderr,"txdevice %d: GetQueryScan Failed on Stn=%d Scan_type=%d\n",
	    ID, CommReq.stn_id, CommReq.scan_type);
  return scanfound;
}
    
bool txdevice::GetScanMngScan() { 
    bool scanfound = false;
    
    return scanfound;
    }

//extern DisplMng *DisplMngr;
    
bool txdevice::GetRealTimeSeqScan() { 
    bool scanfound = false;
    // rdr_img *tempimg;
    
    // scan clients should be able to be queried for a 
    // reference to a desired scan
    // scanmng could then traverse clients to find anyone who has it
/*
    if (DisplMngr && DisplMngr->rdrseq && 
	DisplMngr->rdrseq->RealTime) {
*/
	/* traverse seq images and look for newest matching scan 
	REMEMBER: DIFFERENT THREAD,  MAIN THREAD COULD DELETE IMAGE 
	PUT SYNCHRONISED GETNEXTIMG(rdrimg *thisimg) INTO rdrseq
	SYNCHRONISE ALL IMAGE DELETE POINTS IN RDRSEQ
	Call IncUserCount on image while TRAVERSING IMAGE FOR SCAN, 
	Use ShouldDelete..delete after finished
	*/
/*
	}	
*/
    return scanfound;
    }
    
bool txdevice::GetLatestDBScan() { 
    bool scanfound = false;
    
    return scanfound;
    }
    
AckStatus txdevice::CheckAckString(char *AckString, rdr_scan *ackscan) {
int args, stnid, day, month, year, hour, min, juldayyear;
char prodstr[64];
char CheckStr[128];
int radlcount = 0;

    Ack_Match = UNDEF;
    strcpy(CheckStr,"");
    strcpy(prodstr,"");
    if ((AckString && !ackscan)) {
	Ack_Match = ACKMISMATCH;
	return Ack_Match;
	}
    if (strstr(AckString, "FAILED, BAD SCAN"))
	Ack_Match = NACKED;
    else if ((args = sscanf(AckString, "RDRSCAN_ACK: %d %d %d.%d %s", 
		&stnid, &juldayyear, &hour, &min, CheckStr)) >= 4) {
	rpdate2ymd(juldayyear,year,month,day);
	if ((stnid == ackscan->station) &&
	    (day == ackscan->day) &&
	    (month == ackscan->month) &&
	    (year == ackscan->year) &&
	    (hour == ackscan->hour) &&
	    (min == ackscan->min)) {
	    Ack_Match = ACKOK; 
	    }
	else Ack_Match = ACKMISMATCH;
	if (args > 5) {
		if (strstr(CheckStr, "RADLCOUNT:")) {
			sscanf(CheckStr,"RADLCOUNT:%d",&radlcount);
			if (radlcount != ackscan->num_radls) { 
				Ack_Match = ACKMISMATCH;
				fprintf(stderr, "txdevice%d::CheckAckString - Radial count mismatch, Ack count = %d, Scan count = %d\n", ID, radlcount, ackscan->num_radls);
				}
			}
		}	
	
	}
    else Ack_Match = ACKMISMATCH;   // bad ack, call it a mismatch
    if (Ack_Match != ACKOK)
	fprintf(stderr, "txdevice%d::CheckAckString - %s Matched=%d\n", ID,
		AckString, Ack_Match);
    return Ack_Match;
    }
    
bool	txdevice::CheckScanAck() {
    
    if (!tx || !Expect_Ack) return TRUE;
    Ack_Timeout = time(0) + AckFailTimeout;
    Ack_Match = UNDEF;
    while ((Ack_Timeout >= time(0)) && (Ack_Match == UNDEF)
	&& !stopFlag) {	// bail out if StopSProc is set
	CheckDataRx();
	if ((Ack_Match == UNDEF) && (!stopFlag))
	    sec_delay(0.5);
	}
    if (Ack_Match == UNDEF)
	Ack_Match = ACKTMOUT;
    if (Ack_Match == ACKOK)
	Ack_FailCount = 0;
    else if (Ack_Match != ACKTMOUT) Ack_FailCount++;
    return (Ack_Match == ACKOK);
    }


bool txdevice::GetFilterTxScan()
{
  rdr_scan_node   *temp;
  bool PassFilter = false;
  bool scanfound = false;
  RPScanFilter    *matchfilter = 0;
  //  char tempstr[256];
    tx = first;
    while (!scanfound && tx) {	    // look for scan to transmit
	PassFilter = false;
	matchfilter = 0;
	scanfound = (tx->scan && tx->scan->station);   // want scan with stn defined
	if (TxFinishedScans ||              // if txfinished, of not ONE filter
	    (FilterCount != 1))             // ignore not finished scans
	     	                    // Only allow ScanByScan behaviour for exactly ONE filter
	    scanfound &= tx->scan->Finished(); 
	if (scanfound) {	    // check for match against any filters
	    if (TempFilters)	    // check against temp (rx end) filters 
		scanfound &= (PassFilter = FilterScan(tx->scan, matchfilter, &TempFilters));
	    else		    // check against perm filters
		scanfound &= (PassFilter = FilterScan(tx->scan, matchfilter)); 
	    if (!scanfound) {	    // scan doesn't match filters, remove it
		temp = tx->next;    // try next
// 		fprintf(stderr, "txdevice %d::CheckDataTx: Scan doesn't match temp filters,"
// 			"  removing %s\n", ID, tx->scan->ScanString2(tempstr));
		RemoveTxScan();	    // remove bad match
		tx = temp;
		}
	    }
	else {		// this tx not ready yet, try next
	    tx = tx->next;
	    }
	}
    if (scanfound && PassFilter && matchfilter) {
	strncpy(matchfilter->LastSentStatus, "Sending", 31);
	FilterListStatusChanged = TRUE;
	}
    return scanfound;
}

void txdevice::RemoveTxScan() {
    if (tx == first)
	first = first->next;
    if (tx == last)
	last = last->prev;
    ScanQSize--;
    delete tx;
    decScanNodeCount();
    // move on to next
    tx = 0;
    ExpBuffRd->open(0);
    lastTxScanChangeTime = 0;
    }
    
void txdevice::CheckDataTx() {
  bool	    done = false, 
    scanfound = false;
  bool	    stillwriting = false,  TxScanComplete;
  int		    wrtsize = 0,rdsize = 0;
  int		    max_laps = 10;
  time_t	    timenow = time(0), timeout = 0;
  bool	    ReconnectAttempt = 0;
  char	    tempstr[256];
  char	    tempstr1[256];
  char	    tempstr2[256];
  RPScanFilter    *matchfilter = 0;
  bool	    PassFilter = false;

  if (CallingMode) SemiPerm = TRUE;	// AutoReconnect and CallingMode will be SemiPerm
  if (!CommHandler->isConnected()) {
    Reset();
    if (AutoReconnect && (timenow >= ReconnectRetryTime)) {
      if (ReconnectRetryTime > 0)	{ // has set timeout at least once
	if (!ConnectFailTime) ConnectFailTime = timenow;
	do_newrpstatus(RDRCONNECTING);
	SetAlertStatus(AL_CONN_LOST, "Connection lost,  attempting reconnect");
      }
      else {		    // initial failure only
	if (CommHandler->ConnDir != INCOMING)	// don't warn on listening pool txdevice
	  sprintf(LogStr,"TxDevice%d %s: NO CONNECTION - ATTEMPTING RECONNECT\n", ID, Description);
	RapicLog(LogStr, LOG_WARNING);
      }
      if (CommHandler) {
	Reset();
	CommHandler->Disconnect();
	sec_delay(float(ReconnDelay));
	if (CommMngr && (CommHandler->ConnState != LISTENING))
	  CommMngr->IncRPSrvListenQ();		// Inc listen queue size
	CommHandler->Reconnect();
      }
      if ((!CommHandler->isConnected()) &&
	  (!ReconnectRetryTime)) {	// first reconnect failure
	sprintf(LogStr,"TxDevice%d %s: FIRST RECONNECT ATTEMPT FAILED\n", ID, Description);
	RapicLog(LogStr, LOG_ERR);
      }	
      ReconnectRetryTime = timenow + ReconnPeriod;
      ReconnectAttempt = 1;
    }
    if (CommHandler->isConnected()) {
      if (ReconnectRetryTime) {
	sprintf(LogStr,"TxDevice%d %s: CONNECTION RE-ESTABLISHED(%s)\n", ID, Description, ConnectedToString());
	RapicLog(LogStr, LOG_WARNING);
      }
      ReconnectRetryTime = 0;
      ConnectFailTime = 0;
      CheckNewConnection();
    }
    else {
      if (ReconnectAttempt && (CommHandler->ConnReqCount > 0)) { // "REAL" reconnect fail
	if (SemiPerm)
	  SetAlertStatus(AL_RECONN_FAILED, "Reconnect attempt failed");
	do_newrpstatus(RECONNFAILED);
	if (!ConnectFailTime) ConnectFailTime = timenow;
      }
      Reset();
      return;
    }
  }
  if (TxBuffIPPos > 0) stillwriting = !WriteTxBuff();
  else {
    stillwriting = false;
  }
  if (!stillwriting && (CommHandler->isConnected())) {
    if (!tx && first) {
      if (!SemiPerm) {	    // transient connect, try to satisfy query, if defined
	if (GetQueryScan()) { // if query, try to match it, else get first reasonable scan
	  //		    fprintf(stderr, "txdevice %d Getting scan...%s Query satisfied\n", ID, RPQueryString);
	  ;
	}
	else {		    // 
	  //		    fprintf(stderr, "txdevice %d Getting scan...Unable to satisfy query\n", ID); 
	  do_newrpstatus(REQFAILED);
	  CommReq.MakeQueryString(tempstr1);
	  sprintf(TxBuff, "RPQUERYFAILED: %s\n", tempstr1);
	  TxBuffIPPos = strlen(TxBuff);
	  WriteTxBuff(100);	// allow up to 1 sec to write
	  CommHandler->FlushIO();
	  sec_delay(2);
	  if (!CallingMode) ListenMode = TRUE;
	  sprintf(LogStr,"TxDevice%d %s: RPQUERYFAILED: %s\n", ID, Description, tempstr1);
	  RapicLog(LogStr, LOG_ERR);
	}
      }
      else { // semi-perm connection, get tx scan matching any filters
	scanfound = GetFilterTxScan();
	if (scanfound) 
	  //		    fprintf(stderr, "txdevice::CheckDataTx % d - Getting scan...Sending default\n", ID);
	  ;
      }
      if (tx) {	
	fprintf(stderr,"txdevice %d::CheckDataTx: Sending scan: %s to %s",
		ID, tx->scan->ScanString2(tempstr), ConnectedToString()); 
	if (ScanQSize > 1) 
	  fprintf(stderr, " - qsize=%d\n", ScanQSize-1);
	else
	  fprintf(stderr, "\n");
	ExpBuffRd->open(tx->scan->GetExpBuff());
	lastTxScanChangeTime = time(0);
	ResetPartialTx();
	if (SendRefTime && !Expect_Ack) { // send ref time before next scan
	  RefTimeStr(TxBuff);
	  TxBuffIPPos = strlen(TxBuff);
	  //		    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	  WriteTxBuff(100);	// allow up to 1 sec to write
	}
	if (SendTxStnSet && !Expect_Ack) { // send tx stn set over new connection
	  TxStnSetStr(TxBuff);
	  //		    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	  TxBuffIPPos = strlen(TxBuff);
	  WriteTxBuff(100);	// allow up to 1 sec to write
	}
	lastScanTxTime = time(0);
	tx->scan->ScanString2(lastScanTxString);
      }
      else 
	{
	  ExpBuffRd->open(0);
	  lastTxScanChangeTime = 0;
	}
    }
    TxScanComplete = !tx || !tx->scan->scanSetSize();
    if  (!tx && RdrStatRequest && !TxBuffIPPos) {	// if no data to tx, reply to status request
      // **	    if (TxScanComplete) {
      sprintf(TxBuff, "MSSG: 30 Status information following - "
	      "3D-Rapic TxDevice %s\nEND STATUS\n",
	      versionStr());
      TxBuffIPPos = strlen(TxBuff);
      WriteTxBuff(100);
      CommHandler->FlushIO();
      RdrStatRequest = false;
      // **		}
    }
    if ((RdrStatMssgTime <= time(0)) && !TxBuffIPPos && !CallingMode) {
      // **	    if (TxScanComplete) {
      sprintf(TxBuff,"MSSG: 17 Disconnection will occur in %d seconds\n",int(RdrStatTimeOut-time(0)));
      time_t	    SaveRdrStatTimeOut = RdrStatTimeOut;
      TxBuffIPPos = strlen(TxBuff);
      WriteTxBuff(100);
      CommHandler->FlushIO();
      RdrStatMssgTime = time(0) + RDRSTATMSSGTM;
      RdrStatTimeOut = SaveRdrStatTimeOut;
      // **		}
      return;
    }
    if (!ExpBuffRd) {
      return;
    }

    //	if (!ExpBuffRd->ReadNodeValid()) ExpBuffRd->set_read_ofs(0);
    // commented out 13/5/2004 pjp - ray had problem with this in rapic tx

    //		if (TxBuffIPPos) done = !WriteTxBuff();	// if buffer present and can not be written, exit
  }
  else done= TRUE;	// if still writing TxBuff don't try to get more
  if (!tx || !tx->scan->scanSetSize()) done = TRUE;
  while (!done && !disconnect) {
    if ((wrtsize = (tx->scan->scanSetSize() - ExpBuffRd->rd_ofs_total))){
      lastTxScanChangeTime = time(0);
      while (!done && !disconnect) {
	rdsize = ExpBuffRd->read_blk(TxBuff,TxBuffSize - TxBuffIPPos,TRUE);
	if (rdsize > 0) 
	  TxBuffIPPos += rdsize;
	done = !TxBuffIPPos || !WriteTxBuff(); 
	// if !TxBuffIPPos, TxBuff is empty and no data was read into it
	// if !WriteTxBuff, buffer not drained, stop adding tx data
      }
      RdrStatMssgTime = time(0) + RDRSTATMSSGTM;  // prevent disconnection warning when sending data
    }
    bool txScanTimedOut = lastTxScanChangeTime &&
      (time(0) - lastTxScanChangeTime) > txScanChangeTimeOut;
    if (!(wrtsize = (tx->scan->scanSetSize() - ExpBuffRd->rd_ofs_total)) ||
	txScanTimedOut) {
      if (tx->scan->Finished() || txScanTimedOut) {	// if no more data and scan finished
	timeout = time(0) + AckFailTimeout;	// time limit to write buffer
	WriteTxBuff(AckFailTimeout * 100);  // allow AckFailTimeout secs to send
	if (txScanTimedOut) {
	  sprintf(LogStr,"TxDevice%d %s: Timed out on tx scan not updating (%dsecs) [%s] to %s\n",
		  ID, Description, int(txScanChangeTimeOut), tx->scan->ScanString2(tempstr),
		  ConnectedToString());
	  RapicLog(LogStr, LOG_ERR);
	}		
	if (!WriteTxBuff()) {
	  sprintf(LogStr,"TxDevice%d %s: Timed out (%dsecs) trying to send scan [%s] to %s\n",
		  ID, Description, AckFailTimeout, tx->scan->ScanString2(tempstr),
		  ConnectedToString());
	  RapicLog(LogStr, LOG_ERR);
	}		
	TxBuff[0] = EOT;    // send EOT char
	TxBuffIPPos = 1;
	WriteTxBuff(50);
	PassFilter = false;
	matchfilter = 0;
	if (TempFilters) 	// check against temp (rx end) filters 
	  PassFilter = FilterScan(tx->scan, matchfilter, &TempFilters);
	else			// check against perm filters
	  PassFilter = FilterScan(tx->scan, matchfilter); 
	if (PassFilter && matchfilter) {
	  strncpy(matchfilter->LastSentStatus, "Sent", 31);
	  FilterListStatusChanged = true;
	}
	if (SemiPerm) {
	  if (CheckScanAck()) {   // if Expect_Ack not set, CheckScanAck always succeeds
	    // if semi-perm, and scan acknowledge satisfied, remove this scan
	    SetAlertStatus(AL_CLEAR_ACKFAIL, "Scan acknowledged OK"); // clear ack alert
	    if (Expect_Ack) {
	      fprintf(stderr,"txdevice %d: CheckDataTx - Scan Acknowledge OK\n",ID);
	    }
	    RemoveTxScan();
	    do_newrpstatus(RDRCONNECTED);
	    if (SendRefTime & Expect_Ack) {	// send ref time after ack, should get straight through comms
	      RefTimeStr(TxBuff);
	      TxBuffIPPos = strlen(TxBuff);
	      //			    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	      WriteTxBuff(100);	// allow up to 1 sec to write
	    }
	    if (SendTxStnSet & Expect_Ack) {	// send tx stn set over new connection
	      TxStnSetStr(TxBuff);
	      //			    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	      TxBuffIPPos = strlen(TxBuff);
	      WriteTxBuff(100);	// allow up to 1 sec to write
	    }
	  }
	  else {	
	    // acknowledge is enabled and failed; reset, reconnect and try again
	    switch (Ack_Match) {
	    case ACKMISMATCH: 
	      SetAlertStatus(AL_ACK_FAIL, "Remote acknowledge match failed,  reconnecting");
	      strcpy(tempstr1,"Scan Acknowledge match failed");
	      break;
	    case ACKTMOUT:
	      SetAlertStatus(AL_ACK_FAIL, "Remote acknowledge timed out,  reconnecting");
	      strcpy(tempstr1,"Scan Acknowledge timed out");
	      break;
	    case NACKED:
	      SetAlertStatus(AL_ACK_FAIL, "Remote NEGATIVE acknowledge received,  reconnecting");
	      strcpy(tempstr1,"Scan NEGATIVE Acknowledge received");
	      break;
	    default:
	      break;
	    }
	    do_newrpstatus(ACKFAIL);
	    if (tx) strncpy(tempstr2, tx->scan->ScanString2(tempstr), 256);
	    else strcpy(tempstr2, "Undefined");
	    sprintf(LogStr,"TxDevice%d %s: %s (%s) Scan = %s\n",
		    ID, Description, tempstr1, ConnectedToString(), tempstr2);
	    RapicLog(LogStr, LOG_ERR);
	    if (Ack_FailCount >= 3) {
	      if (tx) strncpy(tempstr2, tx->scan->ScanString2(tempstr), 256);
	      else strcpy(tempstr2, "Undefined");
	      sprintf(LogStr,"TxDevice%d %s: RECURRENT Scan Acknowledge failure (%s), Removing scan: %s\n",
		      ID, Description, ConnectedToString(), tempstr2);
	      RapicLog(LogStr, LOG_ERR);
	      RemoveTxScan();
	      Ack_FailCount = 0;  // reset counter
	    }
	    else    
	      ResetPartialTx();
	    Reset();
	    if (CommHandler) {
	      CommHandler->Disconnect();
	      sec_delay(float(ReconnDelay));
	      CommHandler->Reconnect();
	    }
	    if (!CommHandler->isConnected()) {
	      SetAlertStatus(AL_RECONN_FAILED, "Reconnect failed");
	      do_newrpstatus(RECONNFAILED);
	      sprintf(LogStr,"TxDevice%d %s: SCAN ACK FAIL RECONNECT ATTEMPT FAILED\n", ID, Description);
	      RapicLog(LogStr, LOG_ERR);
	    }
	    else {
	      sprintf(LogStr,"TxDevice%d %s: CONNECTION RE-ESTABLISHED(%s)\n", ID, Description, ConnectedToString());
	      RapicLog(LogStr, LOG_WARNING);
	      CheckNewConnection();
	    }
	    ReconnectRetryTime = timenow + ReconnPeriod;
	  }
	}
	else if (!NextQueryScan(tx)) {	// when multi request implemented
	  sprintf(LogStr,"TxDevice%d CheckDataTx - Send complete, Disconnecting\n",ID);
	  RapicLog(LogStr, DfltLogLevel);
	  do_newrpstatus(REQCOMPLETE);
	  Reset();
	  if (CommHandler) CommHandler->Disconnect();				// query mode - get next scan for query
	  do_newrpstatus(RDRIDLE);
	  tx = 0;
	}
      }
      done = true;
    }
    max_laps--;
    done |= !max_laps;		// limit to 10 times around while loop per check
  }
  if (!CommHandler->isConnected()) {	    // connection lost while writing, reset tx
    Reset();
    do_newrpstatus(RDRCONNLOST);
  }
}

rdr_scan_node *txdevice::NextQueryScan(rdr_scan_node* thisscan) {
  if (thisscan);  // do nothing at the moment
  return 0;
}
    

void txdevice::ResetPartialTx() {	// if tx terminated, reset it    ExpBuffRd->set_read_ofs(0);
    if (!tx) return;
//    tx = 0;				// let calling choose whether to clear tx
    ExpBuffRd->set_read_ofs(0);
    lastTxScanChangeTime = 0;
    TxBuffIPPos = TxBuffOPPos = 0;
    }
    
RPCommStatus txdevice::GetStatus() {
    return RPStatus;
    }
    
void txdevice::do_newrpstatus(RPCommStatus newstatus) {
    
    if (!EnableComms) 
	newstatus = DISABLED;	// disabled overrides everything

int localstatchanged = newstatus != RPStatus;

    if (localstatchanged) {
	RPStatus = newstatus;
	StatusChanged = true;
	if (CommMngr) CommMngr->stateChanged();
	}
    if (localstatchanged && 
	(CommHandler->isConnected()) && 
	(newstatus == RDRCONNECTED)) {
        if (newstatus == RDRCONNECTED)
          lastConnectTime = time(0);
	if (SendRefTime) {	// send ref time over new connection
	    RefTimeStr(TxBuff);
	    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	    TxBuffIPPos = strlen(TxBuff);
	    WriteTxBuff(100);	// allow up to 1 sec to write
	    }
	if (SendTxStnSet) {	// send tx stn set over new connection
	    TxStnSetStr(TxBuff);
	    fprintf(stderr,"TxDevice%d %s: SENDING %s\n", ID, Description, TxBuff);
	    TxBuffIPPos = strlen(TxBuff);
	    WriteTxBuff(100);	// allow up to 1 sec to write
	    }
	}    
    }
    
char *txdevice::ConnectedToString() {
    return CommHandler->connectedtostr;
    }

char *txdevice::DescriptionStr() {
    return Description;
    }

void txdevice::SetAlertStatus(e_alert_status newalert, char *alertstr) {
    this_alertstatus = newalert;
    if (newalert == AL_ACK_FAIL)
	AlertAckFailFlag = 1;
    else if (newalert == AL_CLEAR_ACKFAIL) {
	AlertAckFailFlag = 0;
	this_alertstatus = AL_OK;
	}
	if (AlertAckFailFlag)			// while ack fail status force all alert status to ack fail
		this_alertstatus = AL_ACK_FAIL;
    if (alertstr) {
	if (lock) lock->get_lock();
	strncpy(LastAlertStr, alertstr, 128);
	if (lock) lock->rel_lock();
	}
    }

void txdevice::setSuppressAlerts(bool state)
{
char LogStr[256];
  SuppressAlerts = state;
  sprintf(LogStr,"TxDevice%d %s: ", ID, Description);
  if (!state)
    {
      strcat(LogStr," ALERTS ENABLED BY USER INTERFACE\n");
      Re_AlertTime = time(0);  // force realert if in alert state
      CheckAlertStatus();
    }
  else
    {
      strcat(LogStr," ALERTS SUPPRESSED BY USER INTERFACE\n");
#ifndef NO_XWIN_GUI
      UnPostTxDevAlert();
#endif
    }
  RapicLog(LogStr, LOG_WARNING);
}

void txdevice::setSuppressReAlert(bool state)
{
char LogStr[256];
  SuppressReAlert = state;
  sprintf(LogStr,"TxDevice%d %s: ", ID, Description);
  if (!state)
    {
      strcat(LogStr," REALERTS ENABLED BY USER INTERFACE\n");
      Re_AlertTime = time(0);  // force realert if in alert state
      CheckAlertStatus();
    }
  else
    {
      strcat(LogStr," REALERTS SUPPRESSED BY USER INTERFACE\n");
#ifndef NO_XWIN_GUI
      UnPostTxDevAlert();
#endif
    }
  RapicLog(LogStr, LOG_WARNING);
}

void txdevice::CheckAlertStatus() {
/*  Assume this is called by main thread ONLY
    if (GETPID() != sproc_ppid) {
	fprintf(stderr, "txdevice::CheckAlertStatus - ERROR: Called by child thread\n");
	return;
	}
*/
time_t thistime = time(0);
int AlertSilenced = 0;

    if (!SemiPerm)
      SuppressAlerts = true;
    AlertSilenced = SilenceReAlert || SilenceFutureAlert || SilenceAllAlerts
	|| SuppressAlerts;
    if (last_alertstatus != this_alertstatus) {
	/*
	 * Alert status changed
	 */
      SuppressReAlert = false;  // always clear suppressrealert state on alert state change
	if (last_alertstatus == AL_OK) {
	    /*
	     * status changed from OK to an alert status
	     */
	    if (!AlertFirstPosted) AlertFirstPosted = thistime;
	    if (lock) lock->get_lock();
#ifndef NO_XWIN_GUI
	    if (!(SuppressAllAlerts || SuppressAlerts))	PostTxDevAlert();
#endif
	    if (lock) lock->rel_lock();
	    SetReAlertTime();
	    if (!AlertSilenced) {
		  switch (this_alertstatus) {
		  case AL_CONN_FAILED:
		  case AL_CONN_LOST:
		  case AL_RECONN_FAILED:
		  case AL_STAT_FAILED:
#ifdef USE_SOUND
			if (strlen(ConnFailSound))
			  if (SoundManager && !silent) SoundManager->AddSound(ConnFailSound);
#endif
			break;
		  case AL_ACK_FAIL:
#ifdef USE_SOUND
			if (strlen(AckFailSound))
			  if (SoundManager && !silent) SoundManager->AddSound(AckFailSound);
#endif
			break;
		  default:
			break;
		  }
		}
	    }
	else if (this_alertstatus == AL_OK)	{ // clear alert
	    /*
	     * status changed from alert to OK status
	     */
#ifndef NO_XWIN_GUI
	    UnPostTxDevAlert();
#endif
	    Re_AlertTime = 0;
	    AlertFirstPosted = 0;
	    SilenceReAlert = false;
#ifdef USE_SOUND
	    if (!AlertSilenced && strlen(AlertClearedSound))
		if (SoundManager && !silent) SoundManager->AddSound(AlertClearedSound);
#endif
	    }
	last_alertstatus = this_alertstatus;
	}
    else if (this_alertstatus != AL_OK) {	
	/*
	 *  no change, but still alert state
	 *  Consider it is staill an alert state if AlertAckFailFlag
	 *  even if AL_OK status
	 */
	if (Re_AlertTime && Re_AlertPeriod && 
	    (thistime >= Re_AlertTime) && EnableComms) {		
	    if (lock) lock->get_lock();
#ifndef NO_XWIN_GUI
	    if (!(SuppressAllAlerts || SuppressAlerts || SuppressReAlert)) 
	      ReAlert();	// see if time to re-alert
#endif
	    if (lock) lock->rel_lock();
	    SetReAlertTime();
	    if (!AlertSilenced) {
		  switch (this_alertstatus) {
		  case AL_CONN_FAILED:
		  case AL_CONN_LOST:
		  case AL_RECONN_FAILED:
		  case AL_STAT_FAILED:
#ifdef USE_SOUND
			if (strlen(ConnFailSound))
			  if (SoundManager && !silent) SoundManager->AddSound(ConnFailSound);
#endif
			break;
		  case AL_ACK_FAIL:
#ifdef USE_SOUND
			if (strlen(AckFailSound))
			  if (SoundManager && !silent) SoundManager->AddSound(AckFailSound);
#endif
			break;
		  default:
			break;
		  }
		}
	    }
	}
    }

void txdevice::SetReAlertTime() {
    if (Re_AlertPeriod && EnableComms) Re_AlertTime = time(0) + Re_AlertPeriod;
    else Re_AlertTime = 0;
    }

// strbuff should be > 36 chars
void txdevice::RefTimeStr(char *strbuff) {
tm temptm;
time_t Time;
    if (!strbuff) return;
    Time = time(0);
    gmtime_r(&Time, &temptm);
    sprintf(strbuff, "RPREFTIME: %02d/%02d/%04d %02d:%02d:%02d",
	temptm.tm_mday, temptm.tm_mon+1, temptm.tm_year+1900, 
	temptm.tm_hour, temptm.tm_min, temptm.tm_sec);
    strcat(strbuff, "\r");
    strcat(strbuff, "\004");	// EOT
    }

// strbuff should be large enough for all filter stns e.g. 512 chars
void txdevice::TxStnSetStr(char *strbuff) {
char tempstr[8];
RPScanFilter *thisfilter;
int FiltersPresent = PermFilters || TempFilters;
int OpenFilter = 0;	// true if any open filters found

    if (!strbuff) return;
    strcpy(strbuff, "RPTXSTNSET");
    thisfilter = PermFilters;
    while (thisfilter && (thisfilter->station > 0)) {
	sprintf(tempstr, ":%d", thisfilter->station);
	strcat(strbuff, tempstr);
	thisfilter = thisfilter->next;
	}
    OpenFilter = thisfilter && (thisfilter->station <= 0);   
    thisfilter = TempFilters;
    while (thisfilter && (thisfilter->station > 0)) {
	sprintf(tempstr, ":%d", thisfilter->station);
	strcat(strbuff, tempstr);
	thisfilter = thisfilter->next;
	}
    OpenFilter |= thisfilter && (thisfilter->station <= 0);
    if (!FiltersPresent || OpenFilter)
	strcpy(strbuff, "RPTXSTNSET:ANY");
    strcat(strbuff, "\r");
    strcat(strbuff, "\004");	// EOT
    }

void txdevice::PrintScanUsage(FILE *file, bool verbose) {
int newcount = 0, newsize = 0, 
    checkedcount = 0, checkedsize = 0, 
    txcount = 0, txsize = 0;
    
    if (!file) file = stderr;
    if (lock) lock->get_lock();
    if (verbose && newscans) {
	newcount = newscans->ListCount();
	newsize = newscans->ListSize();
	}
    if (verbose && checkedscans) {
	checkedcount = checkedscans->ListCount();
	checkedsize = checkedscans->ListSize();
	}
    if (first) {
	txcount = first->ListRootCount();
	txsize = first->ListSize();
	}
    if (verbose)
	fprintf(file, "txdevice%d PrintScanUsage, NewScans: %d:%1.3fMB,  CompleteScans %d:%1.3fMB\n"
			"    TxQueueScans: %d:%1.3fMB, scan_node_count=%d", 
		ID, newcount, newsize/1000000.0, 
		checkedcount, checkedsize/1000000.0, 
		txcount, txsize/1000000.0,
		getScanNodeCount());
    else
	fprintf(file, "txdevice::PrintScanUsage, TxQueueScans: %d:%1.3fMB, scan_node_count=%d", 
		txcount, txsize/1000000.0, getScanNodeCount());
    if (RPStatus == RDRCONNECTED)
      {
	fprintf(file, "  Connected to %s\n", ConnectedToString());
      }
    else 
      fprintf(file, "  Not Connected\n");
    if (lock) lock->rel_lock();
    }

bool txdevice::ScanSourceOK(rdr_scan *srcscan)
{
  bool OK = false;

  if (DataMode == REPLAYMODE)
    switch (srcscan->data_source)
      {
      case REPLAY:
	OK = true;
	break;
      default:
	OK = false;
	break;
      }
  if (DataMode == REALTIMEMODE)
    switch (srcscan->data_source)
      {
	//	    case DBRELOADREALTIME:
      case COMM:
      case COMMREQ:
      case RADARCONVERT:
	//	    case PROD_ACCUM:
      case PROD_XLAT:
	OK = true;
	break;
      default:
	OK = false;
	break;
      }
  if (loadingRecentScans) // if reload recent scans, allow DBRELOADREALTIME source
    OK |= srcscan->data_source == DBRELOADREALTIME;
    ;
  return OK;
}

#ifndef NO_XWIN_GUI
void txdevice::UnPostTxDevAlert() {
  RxDevAlertWindow *thiswin = (RxDevAlertWindow *)AlertWid;
    if (AlertWid) 
      thiswin->hide();
    }
    
void txdevice::PostTxDevAlert() {
  if (SuppressAlerts || !isEnabled())
    return;
  ReAlert();
}

void txdevice::ReAlert() {
  RxDevAlertWindow *thiswin;
  if (!AlertWid)
    AlertWid = (void *) new RxDevAlertWindow("Transmit Device Alert");
  if (AlertWid && isEnabled() && !SuppressAlerts) {
    thiswin = (RxDevAlertWindow *)AlertWid;
    thiswin->upDate(this);
    if (!thiswin->iconic()) thiswin->hide(); // to force "un-iconify"
    if (!thiswin->visible()) thiswin->show();
    thiswin->raise();
  }
}
#endif    

bool txdevice::writeAlertToFile(FILE *commstatfile)
{
  bool alertActive = (this_alertstatus != AL_OK);
  char tempstr[256];
  
  if (!alertActive || !isEnabled())
    return false;
  else
    {
      fprintf(commstatfile, "**ALERT ON TxDevice %d - %s\n",
	      ID, Description);
      fprintf(commstatfile, "  Alert State=%s\n", LastAlertStr);
      fprintf(commstatfile,"  Alert first posted time - %s\n", 
	      TimeString(AlertFirstPosted, tempstr, true, true));
    }
  return true;
}

void txdevice::writeStatusToFile(FILE *commstatfile)
{
  char tempstr[256];

  fprintf(commstatfile, "TxDevice %d - %s",
	  ID, Description);
  
  fprintf(commstatfile, "  Status=%s threadid=%ld\n", RPCommStatusString[RPStatus], long(thread_id));
  if (RPStatus == RDRCONNECTED)
    {
      fprintf(commstatfile, "  Connected to %s - Established %s\n", 
	      ConnectedToString(),
	      TimeString(lastConnectTime, tempstr, true, true));
    }
  else if (this_alertstatus != AL_OK)
    {
      fprintf(commstatfile, "  **ALERT STATE=%s\n", LastAlertStr);
      fprintf(commstatfile,"  Alert first posted time - %s\n", 
	      TimeString(AlertFirstPosted, tempstr, true, true));
    }
  else if (strlen(ConnectedToString()))
    fprintf(commstatfile, "  Last Connected to %s - Established %s\n", 
	    ConnectedToString(),
	    TimeString(lastConnectTime, tempstr, true, true));
  fprintf(commstatfile, "  SemiPerm=%d\n", SemiPerm);
  fprintf(commstatfile, "  sendRecentDataMins=%d\n", sendRecentDataMins);
  fprintf(commstatfile, "  QueryMode=%d\n", QueryMode);
  if (transientDevice)
    fprintf(commstatfile, "  Transient device\n");
  if (transientTimeout)
    fprintf(commstatfile, "  Transient device timeout=%s\n", 
	    TimeString(transientTimeout, tempstr, true, true));
  fprintf(commstatfile, "  Expect_Ack=%d\n",  Expect_Ack);
  fprintf(commstatfile, "  SendRefTime=%d\n",  SendRefTime);
  fprintf(commstatfile, "  SendTxStnSet=%d\n", SendTxStnSet);
  if (FilterCount)
    {
      fprintf(commstatfile, "  Scan filter count=%d\n", FilterCount);
      fprintf(commstatfile, "  Permanent Filters\n");
      RPScanFilter *scanfilter = PermFilters;
      while (scanfilter)
	{
	  scanfilter->MakeTextFilterString(tempstr);
	  fprintf(commstatfile, "    %s\n", tempstr);
	  scanfilter = scanfilter->next;
	}
      fprintf(commstatfile, "  Temporary Filters\n");
      scanfilter = TempFilters;
      while (scanfilter)
	{
	  scanfilter->MakeTextFilterString(tempstr);
	  fprintf(commstatfile, "    %s\n", tempstr);
	  scanfilter = scanfilter->next;
	}
    }
  fprintf(commstatfile, "  TxFinishedScans=%d\n", TxFinishedScans);
  fprintf(commstatfile, "  TxLatestScan=%d\n", TxLatestScan);
  fprintf(commstatfile, "  AutoReconnect=%d\n", AutoReconnect);
  if (CallingMode)
    fprintf(commstatfile, "  CallingMode is set\n");
  if (lastScanTxTime)
    {
      fprintf(commstatfile, "  Last Scan Sent Time - %s\n", 
	      TimeString(lastScanTxTime, tempstr, true, true));
      fprintf(commstatfile, "  Last Scan Description   - %s\n", lastScanTxString);
    }
  PrintScanUsage(commstatfile, true);
  workProcExecTimer.dumpStatus(commstatfile, "  Exec Timer - ");
  workProcLoopTimer.dumpStatus(commstatfile, "  Loop Timer - ");
  fprintf(commstatfile, "\n");
}
