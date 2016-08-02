/*
  CheckRapicStatus() issues a RDRSTAT: request to a Rapic Tx
  It is up to the routine which reads the radar data to look for the
  END RADAR STATUS string and issue a RapicSTatusAck() before the timeout
  otherwise a reconnection will be initiated if autoreconnect is TRUE
*/
    
#include "comms.h"
#include "rpcomms.h"
#include "log.h"
#include "siteinfo.h"
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "rdrscan.h"
#include "rdrutils.h"
#include <signal.h>
#include <sys/types.h>
#ifdef USE_SOUND
#include "soundmng.h"
#endif

#ifndef NO_XWIN_GUI
#include "RxDevAlertWindow.h"
#include <Vk/VkQuestionDialog.h>
#endif

map <int, stnScanStats> stnScanStatsMap;

extern bool signals_handled;

int DevTmOut = 600;		// 10 min device avail time out
extern int   ConnRank[];
    
const char RPQueryString[] = "RPQUERY:";    // RPQUERY: stn scantype angle count qtype qtime
const char RPQuerySemiPermStr[] = "SEMIPERMANENT CONNECTION - SEND ALL DATA";
const char RPFilterString[] = "RPFILTER:";	// RPFILTER:stnid:scantype:datafmt:srcstr
const char rawRPFilterString[] = "*RPFILTER:";	// raw RPFILTER, use literal string when sending
/* RPFilterString enum definitions (refer rdrscan.c)
   enum	e_scan_type {PPI, RHI, CompPPI,IMAGE,VOL,RHISet,MERGE, SCANERROR};
   enum	e_data_type {refl,vel,spectw};
   enum	e_data_fmt {RLE_6L_ASC,RLE_16L_ASC};
   enum	e_data_source {COMM, DB, COMMREQ, PROD_ACCUM};	// source class of data
   e.g. RPFILTER:-1:0:0:0	    would specify any stn,  refl only, RLE 6lvl only, COMM source only
*/
const char RPScanAckString[] = "RDRSCAN_ACK:";	// RDRSCAN_ACK: stnid dd/mm/yy hh:mm prod [rxsize CRC]
    
const char RPQuery_ScanByScanStr[] = "TXCOMPLETESCANS=0";
const char RPQueryLatestScanStr[] = "TXLATESTSCANONLY"; // on connection send latest matching scan only, not backlog
const char RPExpectAckStr[] = "EXPECTACK=1";
const char RPAllowReplayScanStr[] = "ALLOWREPLAY=1";
const char RPSendRecentDataMins[] = "SendRecentDataMins=";
const char RPSendRecentDataFrom[] = "SendRecentDataFrom=";
    
char *RPPortTypeString[] = {
  "Std1","Std2","Vol","Srv"};
char *RPDataProtString[] = {
  "Rapic"};
char *RPScanTypeString[] = {
  "Comp1","Comp2","UsrPPI","UsrRHI","VOL"};
char *RPScanTypeStringReadable[] = {
  "BaseScan","BaseScan2","User Def PPI","User Def RHI","Volumetric"};
char *RPQueryTypeString[] = {
  "Latest","ToTime","FromTime","CentreTime"};
char *RPCommStatusString[] = {
  "IDLE      ",	    // (Rx/Tx) Device idle, waiting for request or listening 
  "CONNECTING",	    // (Rx/Tx) Device in the process of establishing a connection 
  "CONNECTED ", 	    // (Rx/Tx) Device successfully connected
  "STATUSFAIL",	    // (Rx)    Remote end failed handshake protocol
  "REQCOMPLET",	    // (Rx/Tx) Device completed sending/receiving requested scan 
  "REQ FAILED",	    // (Rx/Tx) Failed to satisfy request	 	 
  "CLEAR COMM",	    // (Rx/Tx) Device disconnecting
  "COMMLOCKED",	    // N/A
  "CONNFAILED",	    // (Rx/Tx) Device failed to establish connection 
  "CONNLOST  ",	    // (Rx/Tx) Device lost an established connection
  "RECONNFAIL",	    // (Rx/Tx) Device failed to re-established lost connection,   
  "DISABLED",	    // (Rx/Tx) Device disabled
  "ACK FAILED",	    // (Tx)    Remote end failed to acknowledge receipt of scan
  "LISTENING"};	    // (Rx/Tx) Device in the process of waiting for incoming connection
char *RPCommSrvTypeString[] = {
  "None", "Req.", "Perm"};
    
ConnModes GetConnMode(char *connstr) {
  int connmode = 0;
  bool match = FALSE;
  while (!match && (connmode <= CM_SOCKX25)) {
    match = strstr(ConnModeString[connmode], connstr) != NULL;
    if (!match) connmode++;
  }
  if (match) return ConnModes(connmode);
  else return CM_GENERIC;
}
    
RP_PORT_TYPE GetRPPortType(char *portstr) {
  int porttype = 0;
  bool match = FALSE;
  while (!match && (porttype <= RPO_SRV)) {
    match = strstr(portstr,RPPortTypeString[porttype]) != NULL;
    if (!match) porttype++;
  }
  if (match) return RP_PORT_TYPE(porttype);
  else return RPO_STD1;
}
    
    
DATA_PROTOCOL GetDataProtocol(char *fmtstring) {
  int datafmt = 0;
  bool match = FALSE;
  while (!match && (datafmt <= DF_RAPIC)) {
    match = strstr(fmtstring,RPDataProtString[datafmt]) != NULL;
    if (!match) datafmt++;
  }
  if (match) return DATA_PROTOCOL(datafmt);
  else return DF_RAPIC;
}
    
    
RP_SCAN_TYPE GetRPScanType(char *scanstr) {
  int scantype = 0;
  bool match = FALSE;
  while (!match && (scantype <= RPT_VOL)) {
    match = strstr(scanstr,RPScanTypeString[scantype]) != NULL;
    if (!match) scantype++;
  }
  if (match) return RP_SCAN_TYPE(scantype);
  else return RPT_DFLT_PPI1;
}
	    
RP_QUERY_TYPE GetRPQueryType(char *querystr) {
  int querytype = 0;
  bool match = FALSE;
  while (!match && (querytype <= RPQ_CENTRETIME)) {
    match = strstr(querystr,RPQueryTypeString[querytype]) != NULL;
    if (!match) querytype++;
  }
  if (match) return RP_QUERY_TYPE(querytype);
  else return RPQ_LATEST;
}
    
    
RPCommReq::RPCommReq() {
  next = prev = 0;
  ConnList = ThisConn = 0;
  ThisConnRank = 0;
  AttemptInProg = ReqCompleted = FALSE;
  rxdev = 0;
  init();
}
    
void RPCommReq::init() {
  stn_id = 0;
  scan_type = RPT_DFLT_PPI1;
  req_angle = -9999;
  req_count = 1;
  QueryTime = TimePutOnQ = priority = 0;
  QueryType = RPQ_LATEST;
  bfDataTypes = bf_refl;
  strcpy(rpqueryStr, "");
}
	    
RPCommReq::~RPCommReq() {
  ClearConnList();
  if (next) next->prev = prev;
  if (prev) prev->next = next;
}
    
void RPCommReq::ClearConnList() {
  RPReqConnPnt *ctemp = ConnList;
  while (ConnList) {
    ctemp = ConnList->next;
    delete ConnList;
    ConnList = ctemp;
  }	
}

void RPCommReq::MakeQueryString(char *querystring) {
  if (strlen(rpqueryStr))  // if an rpqueryStr was specified, use it
    {
      strcpy(querystring, rpqueryStr);
      return;
    }
  else
    {
      if (scan_type == RPT_VOL) {
	sprintf(querystring,"%s %d %s %3.1f %d %s %d Refl,Vel\n",
		RPQueryString,stn_id,RPScanTypeString[scan_type],float(req_angle/10.0),
		//		    req_count,RPQueryTypeString[QueryType],QueryTime, DataTypesString);
		req_count,RPQueryTypeString[QueryType],int(QueryTime));
      }
      else {
	sprintf(querystring,"%s %d %s %3.1f %d %s %d\n",
		RPQueryString,stn_id,RPScanTypeString[scan_type],float(req_angle/10.0),
		//		    req_count,RPQueryTypeString[QueryType],QueryTime, DataTypesString);
		req_count,RPQueryTypeString[QueryType],int(QueryTime)); 
	    
      }
    }
}
/*
  char* RPCommReq::DataTypesString() {
	
  return datatypesstring;
  }
*/

bool RPCommReq::DecodeRequestString(char *REQSTR) {
  int int2;
  float float1;
  char stnstr[21], str1[21],str2[21], str3[16];
  int time1;
  int	    args = 0;
	    
  init();
  if ((args = sscanf(REQSTR,"request=%20s %20s %f %d %20s %d %s",
		     stnstr,str1,&float1,&int2,str2,&time1, str3)) < 1) return FALSE;
	    
  stn_id = decode_stnstr(stnstr);
  if (args >= 2) 
    scan_type = GetRPScanType(str1);
  if (args >= 3) 
    req_angle = int(float1*10);
  if (args >= 4) 
    req_count = int2;
  if (args >= 5) 
    QueryType = GetRPQueryType(str2);
  if (args >= 6) 
    QueryTime = time_t(time1);
  /*
    if (args > 6) 
    decodedatatypesstring(str3);
  */
  return TRUE;
}
    
bool RPCommReq::DecodeQueryString(char *REQSTR) {
  int int2;
  float float1;
  char stnstr[21], str1[21],str2[21], str3[16];
  int time1;
  int	    args = 0;
	    
  init();
  if ((args = sscanf(REQSTR,"RPQUERY: %20s %20s %f %d %20s %d %s",
		     stnstr,str1,&float1,&int2,str2,&time1, str3)) < 1) return FALSE;
  stn_id = decode_stnstr(stnstr);
  if (args >= 2) 
    scan_type = GetRPScanType(str1);
  if (args >= 3) 
    req_angle = int(float1*10);
  if (args >= 4) 
    req_count = int2;
  if (args >= 5) 
    QueryType = GetRPQueryType(str2);
  if (args >= 6) 
    QueryTime = time_t(time1);
  /*
    scan_type = GetRPScanType(str1);
    req_angle = int(float1*10);
    req_count = int2;
    QueryType = GetRPQueryType(str2);
    QueryTime = time1;
  */
  /*
    if (args > 6) 
    decodedatatypesstring(str3);
  */
  return TRUE;
}
    
void RPCommReq::String(char *ReqStr) {
  sprintf(ReqStr,"%12s TYP-%s ANG-%1.1f NO-%d QTYP-%s QTM-%d TMONQ-%d PR-%d",
	  StnRec[stn_id].Name,RPScanTypeString[scan_type],
	  req_angle/10., req_count,
	  RPQueryTypeString[QueryType],int(QueryTime),int(TimePutOnQ),priority);
}
    
bool RPCommReq::IsSame(RPCommReq *testreq) {
  if ((stn_id != testreq->stn_id) ||
      (scan_type != testreq->scan_type) ||
      (req_angle != testreq->req_angle) ||
      (req_count != testreq->req_count) ||
      (QueryType != testreq->QueryType) ||
      (QueryTime != testreq->QueryTime))
    return FALSE;
  else return TRUE;
}
    
void RPCommReq::AddConn(RPConnection *newconn) {
  RPReqConnPnt *temp = ConnList, *newpnt;
  int	connrank = newconn->conn_rank;
  newpnt = new RPReqConnPnt(newconn);
  if (!ConnList) {		    // first in list, add it & return
    ConnList = newpnt;
    return;
  }
  while (temp && (connrank < temp->Rank())) {
    if (!temp->next) {	    // temp is last, append newconn after this & return
      temp->next = newpnt;
      newpnt->prev = temp;
      return;
    }
    temp = temp->next;
  }
  newpnt->next = temp;
  newpnt->prev = temp->prev;
  temp->prev = newpnt;
  if (newpnt->prev)      	       	// insert
    newpnt->prev->next = newpnt;
  else				// if no prev must be first, set ConnList pointer
    ConnList = newpnt;
}
    
void RPCommReq::DelReqConn(RPReqConnPnt *delreqconn) {
  int	prevbaserank = ConnList->Conn->conn_rank;
  if (delreqconn == ConnList) {
    ConnList = ConnList->next;
    if (ConnList && 
	(ConnList->Conn->conn_rank != prevbaserank))   // top tier deleted
      ResetAllDevAvailTmOut();	// reset all tier devavail tmouts, only now trying to serve
  }
  if (delreqconn == ThisConn) { //
    ThisConn = ConnList;
    if (ThisConn) ThisConnRank = ThisConn->Rank();
    else ThisConnRank = 0;
  }
  delete delreqconn;
}
    
void RPCommReq::ResetAllDevAvailTmOut() {
  time_t now = time(0);
  RPReqConnPnt *temp = ConnList;
  while (temp) {
    temp->DevAvailTimeOut = now + DevTmOut;
    temp = temp->next;
  }
}
    
// try to find a connection that suits this rxdevice, and is in the
// top rank tier
RPReqConnPnt* RPCommReq::GetConnForRxDev(rxdevice *rxDev) {
  RPReqConnPnt *temp = ConnList, *BestConn = 0, *DelConn;
  int BestRank = 0;
  ConnModes rxdevmode = rxDev->CommHandler->ConnMode;
  char LogStr[256];
	
  if (!rxDev) return 0;
  if (AttemptInProg) {
    if ((rxDev->CommReq == this) || // check handler reference matches this
	(rxDev->newcommreq == this))// this req not yet actioned by thread 
      return 0;		    // do not respond if already trying, and valid handler
    else {			    // ??? Something wrong
      if (rxdev->CommReq)
	sprintf(LogStr, 
		"RPCommReq::GetConnForRxDev ERROR, AttemptInProg set but rxdev->CommReq != this\n"
		"rxdev Status=%s, rxdev->CommReq->stn_id=%d, rxdev->newcommreq=%p\n"
		"CLEARING ATTEMPT IN PROG, AND SETTING RXDEV TO 0\n", 
		RPCommStatusString[rxdev->RPStatus], int(rxdev->CommReq->stn_id), rxdev->newcommreq
		);
      else 
	sprintf(LogStr, 
		"RPCommReq::GetConnForRxDev ERROR, AttemptInProg set but rxdev->CommReq == 0\n"
		"rxdev Status=%s, rxdev->newcommreq=%p\n",   
		RPCommStatusString[rxdev->RPStatus], rxdev->newcommreq
		);
      RapicLog(LogStr, DfltLogLevel);
      AttemptInProg = FALSE;
      rxdev = 0;
      //		return 0;		    // request already being handled 
    }
  }
  temp = ConnList;
  if (temp) ThisConnRank = temp->Conn->conn_rank;		    // get base rank tier
  while (temp && (temp->Conn->conn_rank >= ThisConnRank)) {   // while same rank tier
    DelConn = 0;
    if (!temp->RetriesLeft || (time(0) > temp->DevAvailTimeOut))    // rule out if no
      DelConn = temp;					    // retires left or not avail yet
    else {
      if ((temp->Conn->conn_mode == rxdevmode) && 
	  (time(0) >= temp->RetryTime)) {	
	// if conn matches handler
	if (temp->Rank() > BestRank) {		    // if better rank than previous (more retries left)
	  BestRank = temp->Rank();		    // record new best rank
	  BestConn = temp;			    // use this as best
	}
      }
    }
    temp = temp->next;					    // try next
    if (DelConn) 
      DelReqConn(DelConn);			    // remove timed out or no retries left conn
  }
  ThisConn = BestConn;
  ThisConnRank = BestRank;
  return BestConn;				                                // return conn, 0 if no good
}
    
/* String returns a string which is suitable for lists etc. */
void RPCommReq::ShortString(char *REQSTR) {
  char temp[32];	
  sprintf(REQSTR,"%s %s %4.1fdeg", StnRec[stn_id].Name, 
	  RPScanTypeString[scan_type], req_angle/10.);
  if (AttemptInProg) {
    if (rxdev) sprintf(temp, "* %d", rxdev->ID);
    else sprintf(temp, "* ERROR - NO rxdevice");
    strcat(REQSTR, temp);
  }
}
    
/* DecodeString decodes the string manufactured by String */
bool RPCommReq::DecodeShortString(char *REQSTR) {
  char	str1[21], str2[21];
  float	float1;
    
  str1[0] = str2[0] = 0;
	    
  if (sscanf(REQSTR,"%s %s %f", 
	     str1, str2, &float1) == 3) {
    if ((stn_id = get_stn_id(str1)) < 0) {
      stn_id = 0;
      return FALSE;
    }
    scan_type = GetRPScanType(str1);
    req_angle = int(float1 * 10);
    return TRUE;
  }
  return FALSE;
}
    
bool RPCommReq::e_scan_type_is_same(e_scan_type scantype) {
  bool match = FALSE;
  switch (scantype) {
  case PPI:	    // fairly liberal matching on PPIs at the moment 
  case CompPPI:
    match = (scan_type == RPT_DFLT_PPI1) ||
      (scan_type == RPT_DFLT_PPI2) ||
      (scan_type == RPT_USR_PPI);
    break;
  case RHI:
    match = (scan_type == RPT_USR_RHI);
    break;
  case VOL:   
    match = (scan_type == RPT_VOL);
    break;
  default:
    break;
  }
  return match;
}
    
    
bool RPCommReq::RdrScanMatch(rdr_scan *matchscan) {
  bool match = FALSE;
    
  match = (stn_id == matchscan->station) && 
    e_scan_type_is_same(matchscan->scan_type);
  if (scan_type == RPT_USR_RHI)
    match &= (req_angle == matchscan->set_angle);
  return match;
}
    
    
bool RPCommReq::ShortIsSame(RPCommReq *testreq) {
  if ((stn_id != testreq->stn_id) ||
      (scan_type != testreq->scan_type) ||
      (req_angle != testreq->req_angle))
    return FALSE;
  else return TRUE;
}
    
    
RPConnection::RPConnection() {
  memset(this, 0, sizeof(RPConnection));
  ConnAttempts = ConnSuccess = ConnFails = 0;
}
    
RPConnection::~RPConnection() {
  if (next) next->prev = prev;
  if (prev) prev->next = next;
}
    
bool RPConnection::DecodeConnStr(char *CONNSTR) {
  int int1,int2,int3;
  char *temp;
  char stnstr[32], str1[21],str2[21],str3[21],str4[32],str5[32];
  str1[0] = str2[0] = str3[0] = str4[0] = str5[0] = 0;
	
  if (sscanf(CONNSTR,"connection=%32s %d %d %20s %20s %20s %32s %32s",
	     stnstr,&int2,&int3,str1,str2,str3,str4,str5) < 7) return FALSE;
  stn_id = decode_stnstr(stnstr);
  Retries = int2;
  tmout = int3;
  conn_mode = Str2ConnMode(str3);
  conn_rank = ConnRank[conn_mode];
  port_type = GetRPPortType(str2);
  data_protocol = GetDataProtocol(str1);
  strncpy(conn_string,str4,64);
  if (str5[0]) {
    strncat(conn_string," ",64);
    strncat(conn_string,str5,64);
  }
  if ((temp = strstr(CONNSTR, "timeout="))) {
    if (sscanf(temp," timeout=%d",&int1) == 1) {
      if ((int1 > 0) && (int1 < 900))
	conn_tmout = int1;
    }
  }
  return TRUE;
}
    
void RPConnection::String(char *ConnStr) {
  char percentstr[16];
  if (ConnAttempts)
    sprintf(percentstr, "%02d%%", ((ConnAttempts-ConnFails)*100)/ConnAttempts);
  else
    strcpy(percentstr, "00%");
  sprintf(ConnStr,"%12s MD-%s PT-%s FMT-%s CSTR-%s RK-%d RETRY-%d TO-%d CONNTMOUT-%d RDYTM-%d INUSE-%d CONNECTS-%d of %d (%s)",
	  StnRec[stn_id].Name,ConnModeString[conn_mode],RPPortTypeString[port_type],
	  RPDataProtString[data_protocol],conn_string,conn_rank,Retries,int(tmout),int(ready_tm),
	  InUse, int(conn_tmout), 
	  ConnSuccess, ConnAttempts, percentstr);
  if (ConnAttempts && (ConnFails == ConnAttempts))
    strcat(ConnStr, "**ALWAYS FAILED**");
}
    
void RPConnection::printState(FILE *outfile) {
  char tempstr[512];
  String(tempstr);
  if (outfile)
    fprintf(outfile, "%s\n", tempstr);
}
    
RPStnConnPnt::RPStnConnPnt(RPConnection *RPConn) {
  next = prev = 0;
  if (!RPConn) return;
  FirstStnConn = RPConn;
  StnID = RPConn->stn_id;
  EntryCount = 1;
}
    
RPStnConnPnt::~RPStnConnPnt() {
  if (next) next->prev = prev;
  if (prev) prev->next = next;
}
    
RPReqConnPnt::RPReqConnPnt(RPConnection *newconn) {
  time_t now = time(0);
  next = prev = 0;
  Conn = newconn;
  RetriesLeft = newconn->Retries;
  DevAvailTimeOut = now + DevTmOut;	// timeout to cancel this conn if no dev
  RetryTime = now;
}
    
RPReqConnPnt::~RPReqConnPnt() {
  if (next) next->prev = prev;
  if (prev) prev->next = next;
}
    
int RPReqConnPnt::Rank() {
  if (Conn->InUse || (Conn->ready_tm > time(0))) return 0;				// while InUse true return 0 rank
  return Conn->conn_rank + RetriesLeft;	// else return base rank + retries left
}
    
RPSchedEntry::RPSchedEntry() {
  memset(this, 0, sizeof(RPSchedEntry));
  GroupID = 0;
}
    
RPSchedEntry::~RPSchedEntry() {
  if (next) next->prev = prev;
  if (prev) prev->next = next;
}
    
bool RPSchedEntry::DecodeSchedStr(char *SCHEDSTR) {
  int int1=0,int2=0,int4=0,int5=-1,int6=-1;
  char str1[21],str2[21],str3[21],str4[32],stnstr[32];
  str1[0] = str2[0] = str3[0] = str4[0] = stnstr[0] = 0;
  int args = 0;
  char *BackupStr, *schedstr = SCHEDSTR, *GroupIDStr;
	    
  if ((schedstr = strstr(SCHEDSTR, "fbsched=")))
    args = sscanf(schedstr,"fbsched=%s %d %20s %d %20s %d %d",
		  stnstr,&int2,str2,&int4,str1,&int5,&int6);
  else if ((schedstr = strstr(SCHEDSTR, "schedule=")))
    args = sscanf(schedstr,"schedule=%s %d %20s %d %20s %d %d",
		  stnstr,&int2,str2,&int4,str1,&int5,&int6);
  if (args < 5) return FALSE;
  CommReq.stn_id = decode_stnstr(stnstr);
  minperiod = time_t(int2*60);	// convert minutes to time_t
  offset = time_t(int4*60);			// convert minutes to time_t
  CommReq.scan_type = GetRPScanType(str1);
  if (args == 7) {
    starttime = int5;
    stoptime = int6;
  }
  int1 = 0;
  args = sscanf(str2, "%d/%d", &int1, &int2);
  period = time_t(int1*60);			// convert minutes to time_t
  if (args == 1) fallbackperiod = period;
  else if (args > 1) fallbackperiod = time_t(int2*60);
  schedtime = 0;
  if ((BackupStr = strstr(SCHEDSTR, "backup=")))
    if (sscanf(BackupStr, "backup=%d", &int1) == 1) {
      backupmodetime = int1 * 60;
      backupmodetimeout = time(0) + backupmodetime;
    }
  if ((GroupIDStr = strstr(SCHEDSTR, "groupid=")))
    if (sscanf(GroupIDStr, "groupid=%d", &int1) == 1) {
      GroupID = int1;
    }
  return TRUE;
}
    
void RPSchedEntry::ResetBackupMode() {
  backupmodetimeout = time(0) + backupmodetime;
}
    
bool RPSchedEntry::ScanMatch(rdr_scan *matchscan) {
  return (CommReq.RdrScanMatch(matchscan));
}
    
void RPSchedEntry::EncodeSchedStr(char *SCHEDSTR) {
  char str1[21], str2[21];
  str1[0] = str2[0] = 0;
	    
  if ((fallbackperiod > period) || (fallbackperiod == 0)) // 0 means no sched on fallback
    sprintf(str1, "%d/%d", int(period) / 60, int(fallbackperiod) / 60);
  else
    sprintf(str1, "%d", int(period) / 60);
  if ((starttime > 0) && (stoptime > 0))
    sprintf(str2, "%d %d", int(starttime), int(stoptime));
  sprintf(SCHEDSTR,"schedule=%d\t%d\t%s\t%d\t%s\t%s",
	  CommReq.stn_id, int(minperiod / 60), str1, int(offset / 60), 
	  RPScanTypeString[CommReq.scan_type], 
	  str2);
  if (backupmodetime) {
    sprintf(str1, " backup=%d", int(backupmodetime/60));
    strcat(SCHEDSTR, str1);
  }
  if (GroupID) {
    sprintf(str1, " groupid=%d", GroupID);
    strcat(SCHEDSTR, str1);
  }
  strcat(SCHEDSTR,"\n");
}
    
/* String returns a string which is suitable for lists etc. */
void RPSchedEntry::String(char *SCHEDSTR) {
	    
  if ((fallbackperiod > period) || (fallbackperiod == 0)) // 0 means no sched on fallback
    sprintf(SCHEDSTR,"%s %s %d/%dmin (+%d)", StnRec[CommReq.stn_id].Name, 
	    RPScanTypeString[CommReq.scan_type], int(period / 60), int(fallbackperiod / 60), 
	    int(offset / 60));
  else
    sprintf(SCHEDSTR,"%s %s %dmin (+%d)", StnRec[CommReq.stn_id].Name, 
	    RPScanTypeString[CommReq.scan_type], int(period / 60), int(offset / 60));
  if (fallbackmode) strcat(SCHEDSTR, " Fallback");
}
    
/* String returns a string which is suitable for lists etc. */
void RPSchedEntry::printState(FILE *outfile) {
	    
  if (outfile)
    fprintf(outfile,"%7s %s %d/%dmin (+%d) schedtime=%d minper=%d start=%d stop=%d \n"
	    "bkmdtm=%d bkmdtmout=%d reqinprog=%d reqcomplete=%d\n", 
	    StnRec[CommReq.stn_id].Name, 
	    RPScanTypeString[CommReq.scan_type], int(period / 60), int(fallbackperiod / 60), 
	    int(offset / 60), 
	    int(schedtime), int(minperiod), int(starttime), int(stoptime), 
	    int(backupmodetime), int(backupmodetimeout), 
	    CommReq.AttemptInProg, CommReq.ReqCompleted);
}
    
/* DecodeString decodes the string manufactured by String */
void RPSchedEntry::DecodeString(char *SCHEDSTR) {
  char	stnstr[32], str2[21], str3[21];
  int	int1, int2, int3, args;
    
  stnstr[0] = str2[0] = str3[0] = 0;
	    
  period = offset = 0;
  if (sscanf(SCHEDSTR,"%s %s %s (+%d)", 
	     stnstr, str2, str3, &int2) == 4) {
    if ((CommReq.stn_id = decode_stnstr(stnstr)) < 0) {
      CommReq.stn_id = 0;
      return;
    }
    CommReq.scan_type = GetRPScanType(str2);
    args = sscanf(str3, "%d/%d", &int1, &int3);
    period = time_t(int1*60);			// convert minutes to time_t
    if (args == 1) fallbackperiod = period;
    else if (args > 1) fallbackperiod = time_t(int2*60);
    offset = time_t(int2*60);			// convert minutes to time_t
  }
}
    
    
#define ONEHOUR 3600
    
void RPSchedEntry::CalcSchedTime(int FallBackMode) {
  time_t localperiod;
  time_t timenow = time(0);
  time_t localtime = timenow - offset;
  bool	localfb = FallBackMode || fallbackmode;	// global or per entry
    
  if (!localfb) localperiod = period;
  else localperiod = fallbackperiod;
  if (!localperiod || disabled ||
      (backupmodetime && (timenow < backupmodetimeout)))
    schedtime = 0;	    // schedtime of 0 will never trigger
  else
    schedtime = localtime - (localtime % localperiod) + localperiod + offset;
}
    
bool RPSchedEntry::IsSame(RPSchedEntry *compare) {
  if (!compare) return FALSE;
  return ((compare->CommReq.stn_id == CommReq.stn_id) &&
	  (compare->CommReq.scan_type == CommReq.scan_type) &&
	  (compare->CommReq.req_angle == CommReq.req_angle));
}
    
FallbackComms::FallbackComms(rxdevice *parentRxDev, int stn) {
  next = 0;
  ParentRxDev = parentRxDev;
  Stn = stn;
  FBState = FBDetected = FALSE;
  FBSchedEntryList = 0;
  FBRxDev = 0;
  SuppressMainSchedMode = FALSE;
  MainSchedEnabled = TRUE;
  SuppressOtherRxDevMode = false;
  OtherRxDevEnabled = true;
}
	
FallbackComms::~FallbackComms() {
  RPSchedEntry *nextsched;
  while (FBSchedEntryList) {
    nextsched = FBSchedEntryList->next;
    delete FBSchedEntryList;
    FBSchedEntryList = nextsched;
  }
  if (FBRxDev && (FBRxDev->ListStatus == HLS_NOLIST)) 
    delete FBRxDev;	 
  // FBRxDev will be deleted by CommMng
}
	
void FallbackComms::AddSchedEntry(RPSchedEntry *SchedEntry) {
  SchedEntry->next = FBSchedEntryList;
  FBSchedEntryList = SchedEntry;
}
	
// return TRUE if status changed
bool FallbackComms::CheckFallback(RPCommMng *CommMng) {
  RPSchedEntry    *SchedEntry;
  bool status_changed = FALSE;
  if (FBState != FBDetected) {	    // chenge in status
    if (FBDetected) {	    // changed to fallback mode
      fprintf(stderr, "FallbackComms::CheckFallback - FALLBACK MODE DETECTED\n");
      if ((SchedEntry = FBSchedEntryList))
	fprintf(stderr, "FallbackComms::CheckFallback - ADDING FALLBACK SCHEDULE ENTRIES\n");
      while (SchedEntry) {
	if (CommMng) CommMng->AddSched(SchedEntry);
	SchedEntry = SchedEntry->next;
      }
      if (FBRxDev) {
	FBRxDev->setInUse(true);
	fprintf(stderr, "FallbackComms::CheckFallback - FBRxDev ENABLED\n");
      }
      if (Stn) {			// if stn defined and FBState
	ParentRxDev->setInUse(false);	// comms unavailable, disable until comms available again
	fprintf(stderr, "FallbackComms::CheckFallback - Station specific fallback detected,  disabling Parent RxDev\n");
      }
    }
    else {			    // fallback mode finished
      fprintf(stderr, "FallbackComms::CheckFallback - FALLBACK MODE FINISHED\n");
      if ((SchedEntry = FBSchedEntryList))
	fprintf(stderr, "FallbackComms::CheckFallback - REMOVING FALLBACK SCHEDULE ENTRIES\n");
      while (SchedEntry) {
	if (CommMng) CommMng->DeleteSchedEntry(SchedEntry);
	SchedEntry = SchedEntry->next;
      }
      if (FBRxDev) {
	FBRxDev->setInUse(false);
	FBRxDev->Set_Disconnect();
	fprintf(stderr, "FallbackComms::CheckFallback - FBRxDev DISABLED\n");
      }
      if (Stn) {			// if stn defined and not FBState
	ParentRxDev->setInUse(true);	// comms now available, enable
	fprintf(stderr, "FallbackComms::CheckFallback - Station specific fallback finished,  enabling Parent RxDev\n");
      }
    }
    status_changed = TRUE;
  }		    
  if (SuppressMainSchedMode) {    // ensure correct state of schedenable
    if (FBDetected && (!MainSchedEnabled)) {// fallback and sched still disabled, enable it
      if (CommMng) CommMng->SetSchedEnable(TRUE);	// enable main sched
      MainSchedEnabled = TRUE;
    }
    if ((!FBDetected) && MainSchedEnabled) { // no longer in fb,  
      if (CommMng) CommMng->SetSchedEnable(FALSE);	 // disable sched
      MainSchedEnabled = FALSE;	 
    }
  }
  if (SuppressOtherRxDevMode) {    // ensure correct state of other rxdevices
    if (FBDetected && (!OtherRxDevEnabled)) {// fallback and other rxdevices still disabled, enable them
      if (CommMng) CommMng->SetOtherRxDevEnable(TRUE);	// enable other rxdevices
      OtherRxDevEnabled = TRUE;
    }
    if ((!FBDetected) && OtherRxDevEnabled) { // no longer in fb,
      if (CommMng) CommMng->SetOtherRxDevEnable(FALSE);	 // disable other rxdevices
      OtherRxDevEnabled = FALSE;	 
    }
  }
  FBState = FBDetected;
  return status_changed;
}
    
/*
 * Called by main thread
 */
// rxdevice::rxdevice(Comm *CommHndl, RPCommMng *comm_manager) : threadObj() {
rxdevice::rxdevice(Comm *CommHndl) : ThreadObj() {
  strcpy(StatStr,"RDRSTAT:");
  faultno = 0;
  ChStTime = time(0);
  CommHandler = CommHndl;
//   CommManager = comm_manager;
  CommReq = 0;
  SendQuery = FALSE;
  linebuff = new rdr_scan_linebuff();
  ID = -1;
  hb_size = 0;
  headerStartTime = 0;
  total_rx = 0;
  new_scan = 0;
  AutoReconnect = FALSE;
  ServiceType = RPS_REQUEST;
  GetFinishedScans = TRUE;    // ask for completed scans from tx device
  GetLatestScan = FALSE;    // ask for completed scans from tx device
  AllowReplayScans = FALSE;   // by default don't get replay mode scans from tx device
  sendRecentDataMins = 0;
  sendRecentDataFrom = 0;
  //	wrtfile = -1;
  //	wrtfilesize = 0;
  //	instrmax = 512;
  //	instrlen = 0;
  next = prev = adminnext = adminprev = 0;
  new_connstr[0] = 0;
  newcommreq = 0;
  strcpy(RadarName, "Undef  ");
  disconnect = reconnect = FALSE;
  clear = FALSE;
  listenmode = FALSE;
  ListenStr[0] = 0;
  newctlval = FALSE;
  lock = new spinlock("rxdevice->lock", 500);	// 5 secs
  RPStatus = RDRIDLE;
  StatusChanged = TRUE;
  DisableConnectConfirm = false;
  PrimaryServerConnection = false;
  FBComms = 0;
#ifndef NO_XWIN_GUI
  CommHndlWid = 0;
  RxDevStatusWid = 0;
  AlertWid = 0;
  enabledTimeoutQDialog = 0;
#endif
  NewCommHndlWidStFlag = FALSE;
  SecsToNextVol = -1;
  RadlTimeout = 0;
  NextScanTimeout = 0;
  scantimeout = 300;	    // timeout for next scan to be received
  NextScanSetTimeout = 0;
  scansettimeout = 0;	    // timeout for next scan set to be received, if != 0and timeout expires, force a reconnect, SEMIPERM ONLY
  LastDataRx = 0;
  lastScanCompletedTime = 0;
  strcpy(lastScanString, "Undefined");
  ConnectFailTime = 0;
  ReconnectRetryTime = 0;
  ReconnPeriod = 10;	    // default to reconnect attempt every 10 secs.
  ReconnDelay = 2;	    //default to disconnect/reconnect delay of 2 secs
  dflt_timeout = 40;	    // default to 30secs for conn and data rx timeout
  timeout = dflt_timeout;	    // value set before each connection
  StChkRate = 30;
  StAckTime = StChkRate + 10;
  StatusReq = FALSE;
  StatusInfo = FALSE;
  RdrStCtl = new RdrStCtlStruct;
  ListStatus = HLS_NOLIST;
  notInUse = TRUE;	    // disabled until put on INUSE list
  strcpy(Description, "");// default is request handler
  strcpy(InitString, ""); // no init string
  strcpy(connectedtostring, "Undefined");
  ScanFilters = 0;
  FilterCount = 0;
  EnableComms = FALSE;    // intitally disable comms processing
  DisabledOnStartup = FALSE;    // by default commmng will enable at startup
  enabledTimeoutPeriod = 0;
  enabledTimeout = 0;
  loopDelay = 0.5;
  Re_AlertPeriod = 15 * 60;
  Re_AlertTime = 0;
  AlertFirstPosted = 0;	    
  SilenceReAlert = SilenceFutureAlert = 
    SuppressAlerts = SuppressReAlert = false;
  last_alertstatus = this_alertstatus = AL_OK;
  strcpy(LastAlertStr, "");
  strcpy(ConnFailSound, "");
  strcpy(RadarStatusFileName, "");
  strcpy(AlertClearedSound, "");
  strcpy(LastRadarStatusString, "UNDEFINED");
  silent = false;
  GroupID = 0;
  FBGroup = 0;
  FBGroupState = false;
  debuglevel = 0;
  strcpy(threadName, "rxdevice");
  lastConnectTime = 0;
};
    
/*
 * Called by main thread
 */
rxdevice::~rxdevice() {
  //    int	waittime = 20;	    //was 10, SD 21/7/00
  //  bool dotprint = FALSE;  //SD add 21/7/00
    
  if (new_scan) {
    new_scan->data_finished();
    if (new_scan->ShouldDelete(this, "rxdevice::~rxdevice")) delete new_scan;
    new_scan = 0;
  }
  fprintf(stderr, "rxdevice %d: ~rxdevice stopping thread\n", ID);
  stopThread();
    
  /*
    THIS HAS MORE SOPHISTICATION THAN STOPTHREAD, 
    MAY NEED TO RE-INVOKE LATER IF STOPTHREAD PROVES INADEQUATE
 
    while ((thread_id > 0) && (waittime > 0)) {  //SD added waittime 19/7/00
    sec_delay(0.01);	// wait for thread to exit
    waittime--;
    //if (waittime <= 0) thread_id = 0;   //SD rem 19/7/00
    if ((waittime %100) == 0) {
    fprintf(stderr, ".");
    dotprint = TRUE;
    #ifdef sgi
    if (thread_id > 0) {
    if (sendKill(thread_id, 0) < 0) {
    sprintf(LogStr, 
    "rxdevice %d: ~rxdevice() thread (pid=%d) has apparently terminated\n", 
    ID, thread_id);
    RapicLog(LogStr, DfltLogLevel);
    thread_id = 0;
    }
    }
    #endif
    }
    }
    if (dotprint) fprintf(stderr, "\n");
    if (waittime <= 0) {
    sprintf(LogStr, 
    "rxdevice %d: ~rxdevice() ERROR Timeout closing thread, ConnState = %s,  KILLING process %d, ID = %d, desc = %s\n", 
    ID, ConnStateStr[CommHandler->ConnState], thread_id,ID,DescriptionStr());
    RapicLog(LogStr, LOG_ERR);
    #ifdef THREAD_SPROC
    if (sendKill(thread_id, SIGKILL) < 0) {
    fprintf(stderr, 
    "rxdevice %d: ~rxdevice() thread (pid=%d) ATTEMPT TO KILL FAILED: %s\n", 
    ID, thread_id, strerror);
    thread_id = 0;
    }
    #endif
    }
  */

  delete lock;
  lock = 0;
  FallbackComms *fbcommsnext;
  while (FBComms) {
    fbcommsnext = FBComms->next;
    delete FBComms;
    FBComms = fbcommsnext;
  }
#ifndef NO_XWIN_GUI
  if (CommHndlWid) CloseWid(CommHndlWid);
  if (RxDevStatusWid) CloseRxDevStatusWid();
  if (CommHandler) delete CommHandler;
  if (AlertWid) 
    {
      delete (RxDevAlertWindow *)AlertWid;
      AlertWid = NULL;
    }
  if (enabledTimeoutQDialog)
    {
      theQuestionDialog->unpost(enabledTimeoutQDialog);
      enabledTimeoutQDialog = 0;
    }
#endif
  if (RdrStCtl) delete RdrStCtl;
  if (linebuff) delete linebuff;
  ClearFilters();
}
    
void rxdevice::setInUse(bool flag)
{
  notInUse = !flag;
}

bool rxdevice::isInUse()
{
  return !notInUse;
}

void rxdevice::Enable(bool state)
{
  char tempstr[64];

  if (state != EnableComms)
    {
      sprintf(LogStr,"RxDevice%d %s: (%s) EnableComms CHANGED from %d to %d\n", 
	      ID, Description, RadarName, EnableComms, state);
      RapicLog(LogStr, LOG_WARNING);	
      EnableComms = state;
    }
  if (EnableComms && DisabledOnStartup && enabledTimeoutPeriod)
    {
      enabledTimeout = time(0) + enabledTimeoutPeriod;
#ifdef SUN
      sprintf(LogStr,"RxDevice%d %s: (%s) enabledTimeout set to %s\n", 
	      ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr, 64));
#else
      sprintf(LogStr,"RxDevice%d %s: (%s) enabledTimeout set to %s\n", 
	      ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr));
#endif
      RapicLog(LogStr, LOG_WARNING);	
    }
  if (!EnableComms && enabledTimeout)
    enabledTimeout = 0;    
#ifndef NO_XWIN_GUI
  if (!EnableComms && AlertWid)
    UnPostRxDevAlert();
#endif
  timeout = dflt_timeout;	    // value set before each connection
}

void rxdevice::Disable()
{
  Enable(false);
}

bool rxdevice::isEnabled()
{
  return EnableComms;
}

void rxdevice::Set_AutoReconnect(bool flag)
{
  AutoReconnect = flag;
}


#ifndef NO_XWIN_GUI
void rxdevice::enabledTimeoutDisconnectOKCallback(Widget w, XtPointer clientData, XtPointer callData)
{
  rxdevice *rxdev = (rxdevice *) clientData;

  rxdev->enabledTimeoutDisconnectOK();
}

void rxdevice::enabledTimeoutDisconnectOK()
{
  sprintf(LogStr,
	  "RxDevice%d %s: CONNECTION TIMEOUT EXPIRED (%dhrs) -  Disconnection from %s CONFIRMED BY OPERATOR\n", 
	  ID, Description, int(enabledTimeoutPeriod/3600), 
	  ConnectedToString());
  RapicLog(LogStr, LOG_ERR);
  Enable(false);
  enabledTimeoutQDialog = 0;
  //  SetAlertStatus(AL_CONN_TIMEDOUT, "Manual Connection TimedOut");
}

void rxdevice::enabledTimeoutDisconnectCancelCallback(Widget w, XtPointer clientData, XtPointer callData)
{
  rxdevice *rxdev = (rxdevice *) clientData;

  rxdev->enabledTimeoutDisconnectCancel();
}

void rxdevice::enabledTimeoutDisconnectCancel()
{
  char tempstr[128];

  enabledTimeout = time(0) + enabledTimeoutPeriod;
  sprintf(LogStr,
	  "RxDevice%d %s: CONNECTION TIMEOUT EXPIRED (%dhrs) - Disconnection from %s CANCELLED  BY OPERATOR\n", 
	  ID, Description, int(enabledTimeoutPeriod/3600), 
	  ConnectedToString());
  RapicLog(LogStr, LOG_ERR);
#ifdef SUN
  sprintf(LogStr,"RxDevice%d %s: (%s) Extending enabledTimeout to %s\n", 
	  ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr, 64));
#else
  sprintf(LogStr,"RxDevice%d %s: (%s) Extending enabledTimeout to %s\n", 
	  ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr));
#endif
  RapicLog(LogStr, LOG_ERR);
  enabledTimeoutQDialog = 0;
}  
#endif

    
/*
  This is polled by commmng which should run in the main process(thread)
  This should make it safe to post XWindows dialogs from here
*/
void rxdevice::CheckComms() {

  char	tempstr[512];
    
  if (!EnableComms) {
    if (CommHandler->isConnected()) {
      do_disconnect();
    }
    if (RPStatus != DISABLED) do_newrpstatus(DISABLED);
    return;
  }
  if ((strlen(Description) == 0) && (ID >= 0))
    sprintf(Description, "rxdevice id=%d", ID);
  if (CommHandler->isConnected()) {
    ReadRP();
  }
  if (notInUse && (CommHandler->isConnected()))
    disconnect = TRUE;
  if (disconnect) {
    do_disconnect();
    do_newrpstatus(RDRIDLE);
    disconnect = FALSE;
    linebuff->reset();
  }
  if (reconnect) {
    CommHandler->Disconnect();
    sec_delay(float(ReconnDelay));
    if (!AutoReconnect) {
      CommHandler->Reconnect();
      if (CommHandler->isConnected()) {
	do_newrpstatus(RDRCONNECTED);
	SetAlertStatus();
      }
      else {
	do_newrpstatus(RECONNFAILED);
	SetAlertStatus(AL_RECONN_FAILED, "Reconnect attempt failed");
      }
    }
    else {	    // let rxdevice loop re-establish connection
      do_newrpstatus(RDRIDLE);
    }
    reconnect = FALSE;
    linebuff->reset();
  }
  if (newctlval) {
#ifndef NO_XWIN_GUI
    do_newcommhndlwidctl();
#endif
    newctlval = FALSE;
  }
  if (clear) {
    do_clear();
    clear = FALSE;
  }
  if (!notInUse) {
    if (listenmode) {
      CommHandler->ConnReqCount = 0;
      CommHandler->Listen(ListenStr);
      do_newrpstatus(RDRLISTENING);
      listenmode = FALSE;
    }
    if (CommHandler->ConnState == LISTENING)
      {
	CommHandler->CheckListen();
	if (CommHandler->isConnected()) {
	  if (AutoReconnect) {
	    makesemipermquery(tempstr);
	    fprintf(stderr,"RxDevice %d CheckComms - Sending semi-perm query - %s\n", 
		    ID, tempstr);
	    CommHandler->Write(tempstr);	
	  }
	  if (ScanFilters) SendFilters(); // send any filters to tx end
	  do_newrpstatus(RDRCONNECTED);
	}
      }		
    if (CommReq || AutoReconnect)
      CheckRapicStatus();
    if (new_connstr[0]) {
      if (lock) lock->get_lock();    // parent may still be setting string, wait for unlock
      strncpy(tempstr, new_connstr, 128);
      if (lock) lock->rel_lock();    // parent may still be setting string, wait for unlock
      do_newconnect(tempstr);
      if (CommHandler->isConnected()) {
	do_newrpstatus(RDRCONNECTED);
      }
      new_connstr[0] = 0;
    }
    if (newcommreq) {
      do_newconnect(newcommreq);
      if (CommHandler->isConnected()) {
	do_newrpstatus(RDRCONNECTED);
      }
      newcommreq = 0;
    }
  }
  NewCommHndlWidSt();	// do not force update, updates periodically
}
	    
/* SProcLoop (child thread) should only call the do_.... functions
 * The parent thread MUST NOT CALL THESE DIRECT,  and should use the 
 * Set_... calls to set variables for the SProcLoop to handle
 */
    
void rxdevice::workProc() {
  ThreadObj::workProc();  // perform base class operations
  CheckComms();
}
    
void rxdevice::threadExit()
{
  if (CommHandler->isConnected()) do_disconnect();
  fprintf(stderr, "rxdevice::threadExit #%d threadid=%d, EXITING\n", ID, int(thread_id));
}

void rxdevice::setStopThreadFlag() {
  if (lock) lock->get_lock();
  disconnect = true;	// disconnect flag interrupts comms reads for fast terminate
  new_connstr[0] = 0;	// clear any other flags to prevent last moment actions
  clear = FALSE;
  newcommreq = 0;
  listenmode = FALSE;
  newctlval = FALSE;
  AutoReconnect = FALSE;
  if (CommHandler)
    CommHandler->terminate = TRUE;
  ThreadObj::setStopThreadFlag();
  if ((thread_id > 0) && signals_handled)
    sendKill(thread_id, SIGHUP);	// try to terminate calls in prog
  if (lock) lock->rel_lock();
}

/*
 * Child thread..Sets a flag to tell the parent thread to update
 * the RPstatus GUI info
 */
void rxdevice::do_newrpstatus(RPCommStatus newstatus) {
  if (!EnableComms) 
    newstatus = DISABLED;	// disabled overrides everything
  if (newstatus != RPStatus) {
    RPStatus = newstatus;
    StatusChanged = TRUE;
    if (CommMngr) CommMngr->stateChanged();
    if (newstatus == RDRCONNECTED)
      {
	lastConnectTime = time(0);
	char tempstr[128];
	sprintf(tempstr, "RxDev%d %s", ID, ConnectedToString());
	linebuff->setLogMssg(tempstr);
      }
  }
}
    
void rxdevice::do_newconnect(char *connstr) {
  char tempstr[512];

  //	doclear();
  do_newrpstatus(RDRCONNECTING);
  timeout = dflt_timeout;  // value set before each connection
  RadlTimeout = 0;
  if (CommReq) {
    sprintf(LogStr, "******* RxDevice %d: donewconnect CLEARING COMMREQ,  SHOULD NEVER BE CALLED\n", ID);
    RapicLog(LogStr, LOG_DEBUG);
    CommReq->AttemptInProg = FALSE;
    CommReq->rxdev = 0;
    CommReq = 0;
  }
  NewCommHndlWidSt(TRUE);
  //    if (lock) lock->rel_lock();		// unlock while connecting		
  sprintf(LogStr, "RxDevice %d: Connecting to %s\n",ID, connstr);
  RapicLog(LogStr, DfltLogLevel);
  CommHandler->Connect(connstr);
  //    if (lock) lock->get_lock();		// lock again after connect
  if (CommHandler->isConnected()) {
    if (AutoReconnect) {
      makesemipermquery(tempstr);
      fprintf(stderr,"RxDevice %d do_newconnect - Sending semi-perm query - %s\n", 
	      ID, tempstr);
      CommHandler->Write(tempstr);	
      if (ScanFilters) SendFilters(); // send any filters to tx end
      StatusReq = FALSE;
      ChStTime = time(0);
      StAckTmOut = ChStTime + StAckTime;
    }
    else RadlTimeout = time(0) + timeout;	// arm radial timeout mechanism
  }
  NewCommHndlWidSt(TRUE);
}

void rxdevice::Set_NewConnect(char *connstr) {
  if (lock) lock->get_lock();
  if (new_connstr[0]) {
    sprintf(LogStr, "RxDevice %d: Set_NewConnect ERROR Previous new_connstr(%s) not cleared\n",ID, new_connstr);
    RapicLog(LogStr, LOG_INFO);
  }
  if (connstr) strncpy(new_connstr,connstr,128);
  else {
    disconnect = TRUE;
    CommHandler->lastconnstr[0] = 0;
  }
  if (lock) lock->rel_lock();
}
    
/*
 * Child thread call
 */
void rxdevice::makesemipermquery(char *tempstr) {
  char tempstr1[128];
  sprintf(tempstr,"%s %s ",
	  RPQueryString,RPQuerySemiPermStr);
  if (!GetFinishedScans)
    {
      strcat(tempstr, " ");
      strcat(tempstr, RPQuery_ScanByScanStr);
    }
  if (GetLatestScan)
    {
      strcat(tempstr, " ");
      strcat(tempstr, RPQueryLatestScanStr);
    }
  if (AllowReplayScans)
    {
      strcat(tempstr, " ");
      strcat(tempstr, RPAllowReplayScanStr);
    }
  if (sendRecentDataMins > 0)
    {
      sprintf(tempstr1, " %s%d", RPSendRecentDataMins, int(sendRecentDataMins));
      strcat(tempstr, tempstr1);
      if (ScanMng)
	{
	  if (!sendRecentDataFrom) // only use startupLatestTime at startupfirst time
	    sendRecentDataFrom = ScanMng->startupLatestTime - 600; // get an extra 10 minutes
	  else
	    sendRecentDataFrom = ScanMng->mostRecentScanTime() - 600; // get an extra 10 minutes
	}
      if (sendRecentDataFrom)
	{
	  int yr, mon, day, hr, min, sec;
	  UnixTime2DateTime(sendRecentDataFrom,
			    yr, mon, day, hr, min, sec);
	  sprintf(tempstr1, " %s%04d%02d%02d%02d%02d%02d", RPSendRecentDataFrom, 
		  yr, mon, day, hr, min, sec);
	  strcat(tempstr, tempstr1);
	}
    }
  //  fprintf(stderr,"rxdevice::makesemipermquery - string = %s\n",tempstr);
  strcat(tempstr, "\n");
}
	
/*
 * Child thread call
 */
void rxdevice::do_newconnect(RPCommReq *NewReq) {
  char tempstr[512];
  int	 conn_tmout = 0;

  if (!NewReq) return;
  do_newrpstatus(RDRCONNECTING);
  timeout = dflt_timeout;		    // value set before each connection
  RadlTimeout = 0;
  SourceType = RPO_VOL;
  if (CommReq) {
    sprintf(LogStr, "******* RxDevice %d: donewconnect CLEARING COMMREQ TO %s,"
	    "  SHOULD NEVER BE CALLED\n", ID, stn_name(NewReq->stn_id));
    RapicLog(LogStr, LOG_ERR);
    CommReq->AttemptInProg = FALSE;
    CommReq->rxdev = 0;
    CommReq = 0;
  }
  CommReq = NewReq;
  if (!CommReq->ThisConn)
    if (!CommReq->GetConnForRxDev(this)) {
      sprintf(LogStr, 
	      "RxDevice %d:  donewconnect PASSED CommReq to %s it cannot service!!\n",
	      ID, stn_name(NewReq->stn_id));
      RapicLog(LogStr, LOG_DEBUG);
      do_newrpstatus(REQFAILED);
      return;
    }
  if (CommReq->ThisConn->Conn->conn_tmout) {
    timeout = CommReq->ThisConn->Conn->conn_tmout;// value set before each connection
    conn_tmout = timeout;
  }
  CommReq->AttemptInProg = TRUE;
  CommReq->ReqCompleted = FALSE;
  CommReq->rxdev = this;
  CommReq->ThisConn->RetriesLeft--;
  CommReq->ThisConn->RetryTime = time(0) + 20;	// retry in 10 seconds
  CommReq->ThisConn->DevAvailTimeOut = time(0) +  DevTmOut;
  CommReq->ThisConn->Conn->InUse = TRUE;
  CommReq->ThisConn->Conn->ConnAttempts++;
  CommReq->ThisConn->Conn->ready_tm = time(0) + CommReq->ThisConn->Conn->tmout;
  strncpy(RadarName, StnRec[CommReq->stn_id].Name, 64); 
  NewCommHndlWidSt(TRUE);
  if (!CommReq->ThisConn->Conn->conn_string[0]) {
    disconnect = TRUE;
    CommHandler->lastconnstr[0] = 0;
  }
  else {
    sprintf(LogStr, "RxDevice %d: donewconnect - Connect request to %s\n", ID, stn_name(CommReq->stn_id));
    RapicLog(LogStr, DfltLogLevel);
    CommHandler->Connect(CommReq->ThisConn->Conn->conn_string, conn_tmout);	// do now if not sproc'd
    if ((CommHandler->isConnected()) && AutoReconnect) {
      makesemipermquery(tempstr);
      fprintf(stderr,"RxDevice %d do_newconnect - Sending semi-perm query - %s\n",
	      ID, tempstr);
      CommHandler->Write(tempstr);	
      if (ScanFilters) 
	SendFilters(); // send any filters to tx end
      StatusReq = FALSE;
      ChStTime = time(0);
      StAckTmOut = ChStTime + StAckTime;
      RadlTimeout = 0;	// no initial radial timeout for semi-perm
      NextScanTimeout = 0;
      if (scansettimeout) {
	NextScanSetTimeout = time(0) + scansettimeout;
#ifdef SUN
	// fprintf(stderr, "RxDevice %d: donewconnect - Setting NextScanSetTimeout to %s\n", 
	//	ID, ctime_r(&NextScanSetTimeout, tempstr, 64));
#else
	// fprintf(stderr, "RxDevice %d: donewconnect - Setting NextScanSetTimeout to %s\n", 
	//	ID, ctime_r(&NextScanSetTimeout, tempstr));
#endif
      }
    }
    if ((CommHandler->isConnected()) && CommReq) {
      ConnectFailTime = 0;
      RadlTimeout = time(0) + timeout;	
      NextScanTimeout = 0;    // next scan timeout mechanism not activated until first scan complete
      CommReq->MakeQueryString(tempstr);
      CommHandler->Write(tempstr);
      fprintf(stderr,"RxDevice %d: (%s) Sending - %s\n", ID, stn_name(NewReq->stn_id), tempstr);
      if (!AutoReconnect) {
	StatusReq = FALSE;
	ChStTime = time(0);
	StAckTmOut = ChStTime + StAckTime;
      }
    }
    NewCommHndlWidSt(TRUE);
  }
}

    
void rxdevice::Set_NewConnect(RPCommReq *NewReq) {
  if (!NewReq) return;
  if (lock) lock->get_lock();
  if (newcommreq) {
    sprintf(LogStr, "RxDevice %d: Set_NewConnect ERROR Previous newcommreq(%s) not cleared\n",ID, StnRec[newcommreq->stn_id].Name);
    RapicLog(LogStr, LOG_DEBUG);
  }
  NewReq->AttemptInProg = TRUE;
  NewReq->ReqCompleted = FALSE;
  NewReq->rxdev = this;
  if (thread_id == 0) {
    do_newconnect(NewReq);
  }
  else newcommreq = NewReq;
  if (lock) lock->rel_lock();
}

void rxdevice::do_clear() {
  disconnect = listenmode = SendQuery = StatusReq = 
    StatusInfo = AutoReconnect = FALSE;
  strncpy(RadarName, "Undef  ", 64);
  strcpy(StatStr,"RDRSTAT:");
  ListenStr[0] = 0;
  timeout = dflt_timeout;
  SecsToNextVol = -1;
  faultno = 0;
  ChStTime = time(0);
  /*
    if (CommReq) {
    CommReq->AttemptInProg = FALSE;
    CommReq->rxdev = 0;
    if (CommReq->ThisConn && CommReq->ThisConn->Conn)
    CommReq->ThisConn->Conn->InUse = FALSE;
    }
    CommReq = 0;
  */
  linebuff->reset();
  hb_size = 0;
  //	total_rx = 0;
  if (new_scan) {
    if (debuglevel > 0)
      cerr << "rxdevice::do_clear forcing finished on new_scan" << endl;
    new_scan->data_finished();
  }
  //    if (new_scan && new_scan->HasOSLock())
  //	fprintf(stderr,"RxDevice %d: (%s) ****do_clear ABOUT TO REMOVE SCAN STILL HOLDING OS LOCK****\n", ID, RadarName);
  if (new_scan && new_scan->ShouldDelete(this, "rxdevice::do_clear")) delete new_scan;
  new_scan = 0;
  RadlTimeout = 0;
  NextScanTimeout = 0;
  NextScanSetTimeout = 0;
  LastDataRx = 0;
  NewCommHndlWidSt(TRUE);
  do_newrpstatus(RDRIDLE);
}

void rxdevice::Set_Clear() {
  if (clear)
    {
      /*
	sprintf(LogStr,"RxDevice%d %s: Set_Clear CLEAR FLAG ALREADY SET, while calling %s\n", 
	ID, Description, RadarName);
	RapicLog(LogStr, LOG_ERR);
      */
      return;
    }
  if (RPStatus == RDRIDLE) 
    {
      /*
	sprintf(LogStr,"RxDevice%d %s: Set_Clear called when state=RDRIDLE, while calling %s\n", 
	ID, Description, RadarName);
	RapicLog(LogStr, LOG_ERR);
      */
      //	    return;    // already cleared **DO AGAIN JUST  IN CASE
    }
  if (lock) lock->get_lock();
  do_newrpstatus(COMMCLEAR);
  if (thread_id == 0) {
    do_clear();
  }
  else clear = TRUE;
  if (lock) lock->rel_lock();
}


void rxdevice::do_disconnect() {
  //    if (lock) lock->rel_lock();		// unlock while disconnecting					
  CommHandler->Disconnect();
  //    if (lock) lock->get_lock();		// relock after disconnecting					
  if (!AutoReconnect)
    CommHandler->ConnReqCount = 0;
  if (new_scan) {
    if (debuglevel > 0)
      cerr << "rxdevice::do_disconnect forcing finished on new_scan" << endl;
    if (new_scan) new_scan->data_finished();
  }
  if (new_scan && new_scan->ShouldDelete(this, "rxdevice::do_disconnect")) delete new_scan;
  new_scan = 0;
  NewCommHndlWidSt(TRUE);
}

void rxdevice::Set_Disconnect() {
  if (lock) lock->get_lock();
  if (thread_id == 0) {
    do_disconnect();
  }
  else disconnect = TRUE;
  if (lock) lock->rel_lock();
}

void rxdevice::Set_Listen(char *str) {
  if (lock) lock->get_lock();
  if (thread_id == 0) {
    CommHandler->ConnReqCount = 0;
    CommHandler->Listen(str);
    do_newrpstatus(RDRLISTENING);
    NewCommHndlWidSt(TRUE);
  }
  else {			// else set variables and let sproc do it
    if (str) strncpy(ListenStr,str,32);
    else ListenStr[0] = 0;
    listenmode = TRUE;
  }
  if (lock) lock->rel_lock();
}

RPCommStatus rxdevice::GetStatus() {
  return RPStatus;
  /*
    if (lock && (!lock->get_lock(1,TRUE))) return COMMLOCKED;
    else {
    if (lock) lock->rel_lock();
    return RPStatus;
    }
  */
}

/*
  The widget will be opened by commmng. ie main thread
  Status Updates to the widget will be made by the main thread when
  the NewCommHndlWidStFlag is set by the comms thread.
  Control changes will come from the widget(main) thread and set a flag 
  which the comms thread will act on 
*/ 

#ifndef NO_XWIN_GUI
void rxdevice::OpenCommHndlWid(int StatusOnly) {
  if (lock) lock->get_lock();
  if (!RdrStCtl) RdrStCtl = new RdrStCtlStruct;
  UpdateStatusWidTime = 0;
  RdrStCtl->TxOnState = 1;
  RdrStCtl->ServoOnState = 1;
  RdrStCtl->VolState = 1;
  RdrStCtl->RngResState = 1000;
  RdrStCtl->DemandPPIElev = 0;
  RdrStCtl->DemandRHIAzim = 0;
  RdrStCtl->AzScanRate = 30;
  RdrStCtl->NewTxOnState = RdrStCtl->NewServoOnState =
    RdrStCtl->NewRngRes = RdrStCtl->NewVolState = 
    RdrStCtl->PPIDemanded = RdrStCtl->RHIDemanded = 
    RdrStCtl->AzScanRateDemanded = -1;
  /*
    if (ServiceType == RPS_REQUEST)
    strncpy(RadarName, "IDLE", 64);
  */
  if (CommHndlWid) RaiseParentWid(CommHndlWid);
  else {
    RdrStCtl->StatusOnly = StatusOnly;
    RdrStCtl->WidCreated = 0;
    CommHndlWid = OpenRdrStCtl(this, RdrStCtl);
  }
  if (CommHndlWid) {
    NewCommHndlWidSt(TRUE);
  }
  if (lock) lock->rel_lock();
}
#endif

/*
 * main thread call
 */

void rxdevice::SetRdrStCtl() {
  time_t	    timenow = time(0);
  time_t	    timediff = 0;
  bool	    semiperm = AutoReconnect || 
    (CommReq && (CommReq->scan_type != RPT_DFLT_PPI1));

  if (!RdrStCtl) return; 
  sprintf(RdrStCtl->CommHndlStr, "#%2d %s %s pid=%d", 
	  ID, RPCommSrvTypeString[ServiceType], 
	  ConnModeString[CommHandler->ConnMode], int(thread_id));
  strncpy(RdrStCtl->RdrNameStr, RadarName, 64);
  if (SourceType == RPO_SRV) {
    strncat(RdrStCtl->RdrNameStr, " via Split: ", 64);
    strncat(RdrStCtl->RdrNameStr, CommHandler->lastconnstr, 64);
  }
  if (CommHandler) {
    if (CommHandler->isConnected()) {
      if (semiperm) {
	if (!StatusReq)
	  sprintf(RdrStCtl->ConnStatusStr,"%.12s to %.52s", 
		  ConnStateStr[CommHandler->ConnState], CommHandler->lastconnstr);
	else 
	  sprintf(RdrStCtl->ConnStatusStr, "Check Status (%2d secs left)", 
		  int(StAckTmOut - timenow));
	if (SecsToNextVol > 0) 
	  sprintf(RdrStCtl->TimeToNextStr, "%d", int(SecsToNextVol));
	else if (SecsToNextVol < 0)
	  strcpy(RdrStCtl->TimeToNextStr, "Not available");
      }
      else {
	sprintf(RdrStCtl->ConnStatusStr,"%.12s to %.52s", 
		ConnStateStr[CommHandler->ConnState], CommHandler->lastconnstr);
	if (!LastDataRx && RadlTimeout) {  // waiting for first sign of data
	  sprintf(RdrStCtl->RdrStatusStr, "Waiting for data (%2d secs left)", 
		  int(RadlTimeout - timenow));
	}
	else
	  sprintf(RdrStCtl->RdrStatusStr, "Radial timeout in %2d secs", 
		  int(RadlTimeout - timenow));
      }
      ConnectFailTime = 0;
    }
    else {
      strncpy(RdrStCtl->ConnStatusStr, ConnStateStr[CommHandler->ConnState], 16);
      if (AutoReconnect) { 
	if (ConnectFailTime)
	  timediff = timenow - ConnectFailTime;
	sprintf(RdrStCtl->TimeToNextStr,  "%02d:%02d:%02d since last connect", 
		int(timediff / 3600),  int((timediff % 3600) / 60), int(timediff % 60));
      }
      else RdrStCtl->TimeToNextStr[0] = 0;       
      strncpy(RdrStCtl->RdrStatusStr, RPCommStatusString[RPStatus], 63); 
    }
  }
  else strcpy(RdrStCtl->TimeToNextStr, "Not available");
}

/*
 * main thread call
 */

void rxdevice::NewCommHndlWidSt(bool force) {
#ifndef NO_XWIN_GUI
  if (!CommHndlWid) return;
  if (force || (time(0) > UpdateStatusWidTime)) {
    SetRdrStCtl();
    NewCommHndlWidStFlag = TRUE;    // flag to tell commmng to update widget
    UpdateStatusWidTime = time(0) + 5;
  }
#endif
}
    
#ifndef NO_XWIN_GUI
void rxdevice::CloseCommHndlWid() {
  if (CommHndlWid) CloseWid(CommHndlWid);
  CommHndlWid = 0;
}

void rxdevice::NewCommHndlWidCtl() {
  if (lock) lock->get_lock();
  newctlval = TRUE;
  if (lock) lock->rel_lock();
}

void rxdevice::do_newcommhndlwidctl() {
  if (!RdrStCtl) return; 
  if (RdrStCtl->NewTxOnState >= 0) {
    CmdTxOnState(RdrStCtl->NewTxOnState);
    RdrStCtl->NewTxOnState = -1;
  }
  if (RdrStCtl->NewServoOnState >= 0) {
    CmdServoOnState(RdrStCtl->NewServoOnState);
    RdrStCtl->NewServoOnState = -1;
  }
  if (RdrStCtl->NewVolState >= 0) {
    CmdVolState(RdrStCtl->NewVolState);
    RdrStCtl->NewVolState = -1;
  }
  if (RdrStCtl->NewRngRes >= 0) {
    CmdRngResVal(RdrStCtl->NewRngRes);
    RdrStCtl->NewRngRes = -1;
  }
  if (RdrStCtl->PPIDemanded >= 0) {
    DemandPPI(RdrStCtl->DemandPPIElev);
    RdrStCtl->PPIDemanded = -1;
  }
  if (RdrStCtl->RHIDemanded >= 0) {
    DemandRHI(RdrStCtl->DemandRHIAzim);
    RdrStCtl->RHIDemanded = -1;
  }
}

void rxdevice::CommHndlWidClosed() {
  if (lock) lock->get_lock();
  CommHndlWid = 0;
  NewCommHndlWidStFlag = FALSE;    // flag to tell commmng to update widget
  if (lock) lock->rel_lock();
}
#endif

void rxdevice::CmdTxOnState(int newstate) {
  char tempstr[48];
  if (newstate) sprintf(tempstr, "COMMAND: 6 /RADIATE\r");
  else sprintf(tempstr, "COMMAND: 6 /STBY\r");
  printf("rxdevice::CmdTxOnState - Command sent = %s\n", tempstr);
  SendCommand(tempstr);            
}
    
void rxdevice::CmdServoOnState(int newstate) {
  char tempstr[48];
  if (newstate) sprintf(tempstr, "COMMAND: 6 /SERVOON\r");
  else sprintf(tempstr, "COMMAND: 6 /SERVOOFF\r");
  printf("rxdevice::CmdServoOnState - Command sent = %s\n", tempstr);
  SendCommand(tempstr);            
}

void rxdevice::CmdVolState(int newstate) {
  char tempstr[48];
  if (newstate) sprintf(tempstr, "COMMAND: 6 /VN\r");
  else sprintf(tempstr, "COMMAND: 6 /VI\r");
  printf("rxdevice::CmdVolState - Command sent = %s\n", tempstr);
  SendCommand(tempstr);            
}

void rxdevice::CmdRngResVal(int newstate) {
  char tempstr[48];
  sprintf(tempstr, "COMMAND: 6 /VRR%d\r", newstate);
  printf("rxdevice::CmdRngResVal - Command sent = %s\n", tempstr);
  SendCommand(tempstr);            
}

void rxdevice::DemandPPI(rdr_angle demandel) {
  char tempstr[48];
  sprintf(tempstr, "COMMAND: 1 /E1%1.1f\r", demandel/10.);
  printf("rxdevice::DemandPPI - Command sent = %s\n", tempstr);
  SendCommand(tempstr);            
}

void rxdevice::DemandRHI(rdr_angle demandaz) {
  char tempstr[48];
  sprintf(tempstr, "COMMAND: 2 /AZ%1.1f\r", demandaz/10.);
  printf("rxdevice::DemandRHI - Command sent = %s\n", tempstr);
  SendCommand(tempstr);        
}

void rxdevice::ClearFilters() {
  RPScanFilter	*NextFilter;
  while (ScanFilters) {
    NextFilter = ScanFilters->next;
    delete ScanFilters;
    ScanFilters = NextFilter;
    FilterCount--;
  }	
}
    
void rxdevice::AddFilter(RPScanFilter *addfilter) {
  if (!addfilter || !addfilter->valid) return;
  if (lock) lock->get_lock();
  if (FilterCount >= RPTXDEVICEFILTERMAX) {
    sprintf(LogStr,"RxDevice%d %s: AddFilter - Scan Filters list full\n", 
	    ID, Description);
    RapicLog(LogStr, LOG_ERR);
    return;
  }
  addfilter->next = ScanFilters;
  addfilter->prev = 0;
  if (ScanFilters) ScanFilters->prev = addfilter;
  ScanFilters = addfilter;
  FilterCount++;
  if (lock) lock->rel_lock();
}
    
void rxdevice::AddFilter(char *filterstr) {
  RPScanFilter *addfilter;
  addfilter = new RPScanFilter(filterstr);
  if (addfilter->valid) AddFilter(addfilter);
  else delete addfilter;
}

void rxdevice::SendFilters() {
  RPScanFilter	*ThisFilter = ScanFilters;
  char filterstr[128];
  int retries, retrycount = 30;
  int temp = 0;
  while (ThisFilter && (temp >= 0) && CommHandler->isConnected()) {
    ThisFilter->MakeFilterString(filterstr);
    strcat(filterstr, "\n");
    temp = CommHandler->WriteBuffer(filterstr);    
    if (temp < 0) return;
    retries = retrycount;
    while (retries && (temp == 0)) {	    // if WriteBuffer didn't complete
      sec_delay(0.5);   // wait before trying write again
      temp = CommHandler->WriteBuffer(); // try to finish send
      if (temp < 0) return;
      retries--;
    }
    fprintf(stderr, "RxDevice %d SendFilters - Sent %s\n", ID, filterstr);
    if (retries) ThisFilter = ThisFilter->next; // succeeded, do next
    else ThisFilter = 0;			    // failed, give up
  }	
}

void rxdevice::SendCommand(char *command) {
  if (!CommHandler->isConnected())
    return;
  int
    temp = CommHandler->Write(command);    
  if (temp < 0) {
    sprintf(LogStr, "rxdevice::SendCommand ERROR Sending command: %s to %s\n", command, ConnectedToString());
    RapicLog(LogStr, DfltLogLevel);
  }
}

void rxdevice::CheckRapicStatus() {
  time_t  timenow;
  char tempstr[512];
  char tempstr2[64];
  bool statustimedout = 0, radltimedout = 0, 
    scantimedout = 0, scansettimedout = 0, datatimedout = 0;
  bool forcedisconnect = FALSE;
  bool semiperm = FALSE;
  FallbackComms	*fbcomms;
    
  if (!CommHandler || clear || disconnect ||	// clearing call
      (CommReq && ((RPStatus == REQFAILED) || (RPStatus == REQCOMPLETE) ||
		   (RPStatus == RDRIDLE))))
    return;
  bool 
    Connected = CommHandler->isConnected();
  //   if (CommHandler->commfd < 0) return;	// Reconnect will open port anyway
  if (Connected)
    ConnectFailTime = 0;
  timenow = time(0);
  statustimedout = StatusReq && (timenow > StAckTmOut); 
  radltimedout =  RadlTimeout && (timenow > RadlTimeout);
  scantimedout = NextScanTimeout && (timenow > NextScanTimeout);
  scansettimedout = NextScanSetTimeout && (timenow > NextScanSetTimeout);
  datatimedout = Connected && LastDataRx && ((timenow - LastDataRx) > timeout);
  if (!statustimedout && datatimedout) {	// additional timeout mechanism
    if (CommReq) strncpy(tempstr, StnRec[CommReq->stn_id].Name, 255);
    else strncpy(tempstr, "Undefined", 255);
    sprintf(LogStr,"RxDevice%d %s: NO DATA TIMEOUT FROM %s - Disconnecting\n", 
	    ID, Description, ConnectedToString());
    RapicLog(LogStr, LOG_WARNING);
  }
  if (statustimedout || radltimedout || 
      scantimedout || scansettimedout || datatimedout) {   // close off current scan
    char tempstr3[64] = "";
    char tempstr5[64] = "";
    if (statustimedout) {
      strcat(tempstr3, "STATUS ");
      sprintf(tempstr5, "%s", ShortTimeString(StAckTmOut,tempstr));
    }
    if (radltimedout) {
      strcat(tempstr3, "RADL ");
      sprintf(tempstr5, "%s", ShortTimeString(RadlTimeout,tempstr));
    }
    if (scantimedout) {
      strcat(tempstr3, "SCAN ");
      sprintf(tempstr5, "%s", ShortTimeString(NextScanTimeout,tempstr));
    }
    if (scansettimedout) {
      strcat(tempstr3, "SCANSET ");
      sprintf(tempstr5, "%s", ShortTimeString(StAckTmOut,tempstr));
    }
    if (datatimedout) {
      strcat(tempstr3, "DATA ");
      sprintf(tempstr5, "%d", int(timeout));
    }
    char tempstr4[128] = "Undefined";
    if (new_scan)
      new_scan->ScanString2(tempstr4);
    sprintf(LogStr,"RxDevice%d %s: %s TIMEOUT FROM %s at %s, LastData at %s, "
	    "Scan=%s Timeout=%s\n", 
	    ID, Description, tempstr3, ConnectedToString(), 
	    ShortTimeString(timenow, tempstr), 
	    ShortTimeString(LastDataRx, tempstr2), tempstr4, tempstr5);
    RapicLog(LogStr, LOG_WARNING);
    if (new_scan) 
      {
	if (debuglevel > 0)
	  cerr << "rxdevice::NewLine - CheckRapicStatus "
	       << tempstr3 << "TIMEOUT - finishing new_scan"
	     << endl;
	new_scan->data_finished();
	if (new_scan->lastScan())
	  {
	    if (debuglevel > 0)
	      {
 		new_scan->lastScan()->setRxTimeStart(headerStartTime, false,
 						     "CheckRapicStatus timeout");
		new_scan->lastScan()->setRxTimeEnd(time(0), false,
						   "CheckRapicStatus timeout");
	      }
	    else
	      {
 		new_scan->lastScan()->setRxTimeStart(headerStartTime);
		new_scan->lastScan()->setRxTimeEnd(time(0));
	      }
	  }
      }
    if (new_scan && 
	new_scan->ShouldDelete(this, "rxdevice::CheckRapicStatus")) 
      delete new_scan;
    new_scan = 0;
    headerStartTime = 0;
    RadlTimeout = NextScanTimeout = NextScanSetTimeout = 0;
    LastDataRx = 0;
    StatusReq = FALSE;	// force new statusreq
    ChStTime = timenow;
    StAckTmOut = timenow + StAckTime;
  }
  /*
   * Do not disconnect Semi-permanent on radl or data timeout,  only on status failure
   * DO disconnect in all other cases
   */
  semiperm = AutoReconnect || 
    (CommReq && (CommReq->scan_type != RPT_DFLT_PPI1));
  forcedisconnect = statustimedout ||	    // disconnect on ALL status fail
    scansettimedout ||
    ((radltimedout || datatimedout) && !semiperm); // only disconnect on non-SemiPErm data timeouts
  forcedisconnect |= (CommReq && scantimedout);   // disconnect if some scans rx'd then scans timed out	
  if (forcedisconnect || !Connected) {
    if (forcedisconnect) {
      CommHandler->ConnState = CONNLOST;
      do_newrpstatus(RDRCONNLOST);
    }
    if (ConnectFailTime == 0) ConnectFailTime = timenow;
    if (AutoReconnect && (timenow >= ReconnectRetryTime) &&
	(CommHandler->ConnReqCount > 0)) {	    // past initial startup, connection has been attempted
      if (statustimedout) {
	StatusChanged = TRUE;
	if (CommMngr) CommMngr->stateChanged();
	SetAlertStatus(AL_STAT_FAILED, "Handshake with remote failed, attempting reconnect");
	sprintf(LogStr,"RxDevice%d %s: RADAR STATUS FROM %s FAILED - ATTEMPTING RECONNECT\n", ID, Description, ConnectedToString());
	RapicLog(LogStr, LOG_WARNING);
      }
      else  if (!Connected) {
	StatusChanged = TRUE;
	if (CommMngr) CommMngr->stateChanged();
	if (!ReconnectRetryTime) {	    // first reconnect attempt
	  SetAlertStatus(AL_CONN_FAILED, "Connection failed, attempting reconnect");
	  sprintf(LogStr,"RxDevice%d %s: NO CONNECTION TO %s - ATTEMPTING RECONNECT\n", ID, Description, ConnectedToString());
	  RapicLog(LogStr, LOG_WARNING);
	  ReconnectRetryTime = timenow + ReconnPeriod;
	}
      }
      sec_delay(float(ReconnDelay));
      CommHandler->Reconnect();
      StatusChanged = TRUE;
      if (CommMngr) CommMngr->stateChanged();
      timenow = time(0);	// get new timenow as Reconnect can take significant time
      Connected = CommHandler->isConnected();
      if (Connected && semiperm && !DisableConnectConfirm)
	if (!ConfirmConnection(15))	// test for valid ack from other end
	  {				// this will prevent 3drapic servers with no free splits
	    disconnect = TRUE;
	    CommHandler->ConnState = CONNLOST;
	    sprintf(LogStr,"RxDevice%d %s: CONNECTION CONFIRMATION TO %s FAILED\n", ID, Description, ConnectedToString());
	    RapicLog(LogStr, LOG_WARNING);
	    do_newrpstatus(RDRCONNLOST);
	    timenow = time(0);	// get new timenow as Reconnect can take significant time
	    ReconnectRetryTime = timenow + ReconnPeriod;
	  }
	else
	  {	
	    sprintf(LogStr,"RxDevice%d %s: CONNECTION CONFIRMATION TO %s SUCCEEDED\n", ID, Description, ConnectedToString());
	    RapicLog(LogStr, LOG_NOTICE);
	  }
      Connected = CommHandler->isConnected();
      timenow = time(0);	// get new timenow as Reconnect can take significant time
      if (!Connected) {
	do_newrpstatus(REQFAILED);
	fbcomms = FBComms;
	while (fbcomms) {
	  if (fbcomms->Stn == 0) fbcomms->FBDetected = TRUE;
// 	  fbcomms->CheckFallback(CommManager);    
	  fbcomms->CheckFallback(CommMngr);    
	  fbcomms = fbcomms->next;	
	  // checkfallback will enable/disable fallback schedule/handler as appropriate
	}
	if (FBGroup && !FBGroupState) {
// 	  CommManager->SetGroupEnable(FBGroup, true);	
	  if (CommMngr) CommMngr->SetGroupEnable(FBGroup, true);	
	  FBGroupState = true;
	  sprintf(LogStr,"RxDevice%d %s: CONNECTION FAILURE, ENABLING GROUP # %d\n", ID, Description, FBGroup);
	  RapicLog(LogStr, LOG_ERR);
	}
	if (CommHandler->ConnReqCount > 0) { // "REAL" reconnect fail
	  SetAlertStatus(AL_RECONN_FAILED, "Reconnect attempt failed");
	  if (!ReconnectRetryTime) {	    // first reconnect attempt
	    sprintf(LogStr,"RxDevice%d %s: RECONNECT ATTEMPT TO %s FAILED\n", ID, Description, ConnectedToString());
	    RapicLog(LogStr, LOG_ERR);
	  }
	}
	ReconnectRetryTime = timenow + ReconnPeriod;
      }
      else {
	SetAlertStatus();
	ChStTime = timenow;
	StAckTmOut = timenow + StAckTime;
	do_newrpstatus(RDRCONNECTED);
	if (ReconnectRetryTime) {
	  sprintf(LogStr,"RxDevice%d %s: CONNECTION TO %s RE-ESTABLISHED\n", ID, Description, ConnectedToString());
	  RapicLog(LogStr, LOG_WARNING);
	}
	if (scansettimeout) {
	  NextScanSetTimeout = time(0) + scansettimeout;
#ifdef SUN
	  fprintf(stderr, "RxDevice %d: CheckRapicStatus - New Connection, setting NextScanSetTimeout to %s\n", 
		  ID, ctime_r(&NextScanSetTimeout, tempstr, 64));
#else
	  fprintf(stderr, "RxDevice %d: CheckRapicStatus - New Connection, setting NextScanSetTimeout to %s\n", 
		  ID, ctime_r(&NextScanSetTimeout, tempstr));
#endif
	}
	ReconnectRetryTime = 0;
	fbcomms = FBComms;
	while (fbcomms) {
	  if (fbcomms->Stn == 0) fbcomms->FBDetected = FALSE;
// 	  fbcomms->CheckFallback(CommManager);
	  fbcomms->CheckFallback(CommMngr);
	  fbcomms = fbcomms->next;
	  // checkfallback will enable/disable fallback schedule/handler as appropriate
	}
	if (FBGroup && FBGroupState) {
// 	  CommManager->SetGroupEnable(FBGroup, false);	
	  if (CommMngr) CommMngr->SetGroupEnable(FBGroup, false);	
	  FBGroupState = false;
	  sprintf(LogStr,"RxDevice%d %s: CONNECTION RE-ESTABLISHED, DISABLING GROUP # %d\n", ID, Description, FBGroup);
	  RapicLog(LogStr, LOG_ERR);
	}
      }
      Connected = CommHandler->isConnected();
    }
    if (!AutoReconnect) {
      if (CommReq) {
	if (!Connected)
	  sprintf(LogStr,"RxDevice%d %s: (%s %s) CONNECTION TO %s LOST - Disconnecting\n", 
		  ID, Description, StnRec[CommReq->stn_id].Name, 
		  RPScanTypeStringReadable[CommReq->scan_type], ConnectedToString());
	else if (statustimedout)
	  sprintf(LogStr,"RxDevice%d %s: (%s %s) Status request to %s timed out - Disconnecting\n", 
		  ID, Description, StnRec[CommReq->stn_id].Name, 
		  RPScanTypeStringReadable[CommReq->scan_type], ConnectedToString());
	else if (radltimedout || datatimedout)
	  sprintf(LogStr,"RxDevice%d %s: (%s %s) Data input from %s timed out - Disconnecting\n", 
		  ID, Description, StnRec[CommReq->stn_id].Name, 
		  RPScanTypeStringReadable[CommReq->scan_type], ConnectedToString());
	RapicLog(LogStr, DfltLogLevel);
      }
      else {
	if (!Connected)
	  sprintf(LogStr,"RxDevice%d %s: (%s) CONNECTION LOST - Disconnecting\n", 
		  ID, Description, ConnectedToString());
	else if (statustimedout)
	  sprintf(LogStr,"RxDevice%d %s: (%s) Status request timed out - Disconnecting\n", 
		  ID, Description, ConnectedToString());
	else if (radltimedout || datatimedout)
	  sprintf(LogStr,"RxDevice%d %s: (%s) Data input timed out - Disconnecting\n", 
		  ID, Description, ConnectedToString());
	RapicLog(LogStr, DfltLogLevel);
      }
      disconnect = TRUE;
      do_newrpstatus(REQFAILED);
    }
    Connected = CommHandler->isConnected();
    if (!disconnect && Connected) {    // can only have re-established via auto-reconnect
      if (ReconnectRetryTime) {
	sprintf(LogStr,"RxDevice%d %s: CONNECTION TO %s RE-ESTABLISHED\n", ID, Description, ConnectedToString());
	RapicLog(LogStr, LOG_WARNING);
	ReconnectRetryTime = 0;
      }
      ConnectFailTime = 0;
      SetAlertStatus();
      if (AutoReconnect) {
	makesemipermquery(tempstr);
	fprintf(stderr,"RxDevice %d CheckRapicStatus - Sending semi-perm query - %s\n", 
		ID, tempstr);
	CommHandler->Write(tempstr);	
      }
      else if (CommReq) {
	CommReq->MakeQueryString(tempstr);
	fprintf(stderr,"RxDevice %d CheckRapicStatus - Sending Query - %s\n", 
		ID, tempstr);
	CommHandler->Write(tempstr);
      }
      if (ScanFilters && DisableConnectConfirm) // if DisableConnectConfirm not defined filters will already have been sent
	SendFilters(); // send any filters to tx end
      //	    fprintf(stderr,"rxdevice::do_newconnect - Sending Query - %s\n",tempstr);
      StatusReq = FALSE;
      ChStTime = time(0);
      StAckTmOut = ChStTime + StAckTime;
      RadlTimeout = 0;	// no initial radial timeout for semi-perm
      NextScanTimeout = 0;
      if (scansettimeout) {
	NextScanSetTimeout = time(0) + scansettimeout ;
#ifdef SUN 
	fprintf(stderr, "RxDevice %d: CheckRapicStatus - Setting NextScanSetTimeout to %s\n",
		ID, ctime_r(&NextScanSetTimeout, tempstr, 64));
#else
	fprintf(stderr, "RxDevice %d: CheckRapicStatus - Setting NextScanSetTimeout to %s\n",
		ID, ctime_r(&NextScanSetTimeout, tempstr));
#endif
      }
    }
  }
  if (!disconnect && Connected) {
    ConnectFailTime = 0;
    SetAlertStatus();
    if (semiperm) {
      if (time(0) >= ChStTime) {
	SendStatReqStr();
	ChStTime = timenow + StChkRate;     // Check Rapic Status every 30 secs
      }
    }
  }
};
	
/*
 * check for successful status ack
 */
bool rxdevice::ConfirmConnection(int timeoutsecs)
{
  char tempstr[512];

  time_t timenow = time(0);
  time_t Timeout = timenow + timeoutsecs;
  time_t TimeLeft = Timeout - timenow;
   
  if (!CommHandler->isConnected()) return false;
  makesemipermquery(tempstr);
  fprintf(stderr,"RxDevice %d ConfirmConnection - Sending semi-perm query - %s\n", 
	  ID, tempstr);
  CommHandler->Write(tempstr);	
  if (ScanFilters) SendFilters(); // send any filters to tx end
  SendStatReqStr();
  while (StatusReq && (TimeLeft > 0) && 
	 !disconnect && 
	 CommHandler->isConnected())
    {
      ReadRP();
      timenow = time(0);
      TimeLeft = Timeout - timenow;
      sec_delay(0.01);
    }
  return (!StatusReq && !disconnect);	// ok if statusreq cleared, & disconnect is false
  // disconnect will be true if no data splits free from server txdevice
}

void rxdevice::SendStatReqStr()
{
  // DO NOT SEND RDRSTAT: TO RapicTx Standard port request
  if (!CommHandler->isConnected()) return;
  if (!(CommReq && (CommReq->scan_type == RPT_DFLT_PPI1))) {
    strcpy(outstr,"\r");
    strcat(outstr,StatStr);
    strcat(outstr,"\r");
    CommHandler->Write(outstr,strlen(outstr));
    if (debuglevel > 1)
      fprintf(stdout, "RxDevice %d SendStatReqStr - Sent %s\n",
	      ID, StatStr);
  }
  if (!StatusReq) {                   // if status ack timer not started
    StatusReq = TRUE;                 // set flag and start it
    StAckTmOut = time(0) + StAckTime;
    NewCommHndlWidSt(TRUE);
  }
}
	
void rxdevice::RapicStatusAck() {
  do_newrpstatus(RDRCONNECTED);
};
	
/*
  Reset the status check and status ack time outs.
  For use when radar data is being recieved, ie no problem with status
*/
void rxdevice::ChkStatusReset() {
  time_t  timenow = time(0);
  StatusReq = FALSE;			// req ack'd, clear
  ChStTime = timenow + StChkRate;// Check Rapic Status again in 30 secs
  StAckTmOut = ChStTime + StAckTime;	// redundant anyway because StatusReq is FALSE
};
	
void rxdevice::NewScanStarted(rdr_scan *NewScan) 
{
  char tempstr[256];

  if (ScanMng)
    {
      if (!NewScan)
	NewScan = new_scan;
      ScanMng->NewDataAvail(new_scan);
    }
  if (scansettimeout) 
    {
      NextScanSetTimeout = time(0) + scansettimeout;
      if (debuglevel)
#ifdef SUN
	fprintf(stderr, "RxDevice %d: NewScanStarted - Resetting NextScanSetTimeout to %s\n", 
		ID, ctime_r(&NextScanSetTimeout, tempstr, 64));
#else
      fprintf(stderr, "RxDevice %d: NewScanStarted - Resetting NextScanSetTimeout to %s\n", 
	      ID, ctime_r(&NextScanSetTimeout, tempstr));
#endif
    }
}

void rxdevice::NewScanFinished(rdr_scan *NewScan) 
{
  if (ScanMng)
    {
      if (!NewScan)
	NewScan = new_scan;
      ScanMng->FinishedDataAvail(new_scan);
    }
  if (NewScan)
    {
      lastScanCompletedTime = time(0);
      NewScan->ScanString(lastScanString);
    }
  currentRadarFaults.setFaulty(NewScan->station,
			       NewScan->get_fault_no(),
			       (char *)NewScan->get_fault_cstr(),
			       NewScan->scan_time_t);
}

void rxdevice::ReadRP() {
  char	c;
  //	bool	debug = FALSE;
  int		instrmax = RXDEV_STRMAX;
  char	instr[RXDEV_STRMAX+1];
  int		instrlen,in_pos;
  int		total_rd = 0;
  bool newline_ok = false;

  if (!CommHandler || !CommHandler->isConnected())
    return;
  while (!disconnect && ((instrlen = CommHandler->Read(instr,instrmax)) > 0)) {
    in_pos = 0;
    instr[instrlen] = 0;
    total_rd += instrlen;
    while (!disconnect && (in_pos < instrlen)) {
      c = instr[in_pos];
      linebuff->addchar_parsed(c);
      if (linebuff->IsEOL())
	{
	  if (!(newline_ok = NewLine())) // parsing may fail due to scan mismatch etc.
	    newline_ok = NewLine();      // try parsing again
	  linebuff->reset();             // ensure linebuff cleared
	}
      in_pos++;
    }
    sec_delay(0.001);
  }
  if (!CommHandler->isConnected()) {	// connection lost
    if (AutoReconnect)
      SetAlertStatus(AL_CONN_LOST, "Connection lost while reading data");
    do_newrpstatus(RDRCONNLOST);
    if (!ConnectFailTime) ConnectFailTime = time(0);
  }
}
	
	
bool rxdevice::NewLine() {
  //	char	c;
  int	mssgno;
  char	mssgstr[128];
  char	faultstr[128] = "";
  char	statusstr[128];
  char	str[128];
  //    bool	dummy = TRUE;
  bool	mssg = FALSE;
  bool	data = FALSE;
  bool endscan = FALSE;
  bool endscanset = FALSE;
  bool line_mismatch = false;   // true if new line doesn't go with this scan
  //  float	tempf;
  int	temptime;
  int	tempint;
  char*	tempstr;
  bool	semiperm = AutoReconnect || 
    (CommReq && (CommReq->scan_type != RPT_DFLT_PPI1));
  char	*lbuff = linebuff->line_buff;
  int		lsize = linebuff->lb_size;

  /*
    int	wrtsz;
    off_t	wrtofs;
  */
    
  if ((lsize == 0) || (lbuff[0] == '\0')) 
    return true;
  linebuff->ensureTerminated();
  if (!linebuff->IsRadl() && (debuglevel > 1)) {
     fprintf(stderr,"RxDevice %d: (%s) NewLine - %s\n", ID, RadarName, lbuff);
  }
  if (!new_scan) {
    new_scan = new rdr_scan(this, "rxdevice::NewLine");
    new_scan->data_source = COMM;
    if (debuglevel > 0)
      cerr << "rxdevice::NewLine creating new_scan" << endl;
//       new_scan->lastScan()->setRxTimeStart(time(0), false, "rxdevice::NewLine - newscan created");
//     else
//       new_scan->lastScan()->setRxTimeStart(time(0));
    /*
     * at this stage,  don't distinguish btwn COMM and COMMREQ
     */
    //	if (CommReq) new_scan->data_source = COMMREQ;
    //	else new_scan->data_source = COMM;
  }
  if (new_scan) {
    endscan = new_scan->end_img_str(lbuff);
    endscanset = new_scan->end_scanset_str(lbuff);
  }
  if (linebuff->IsRadl()) {
    if (debuglevel > 2)  
      fprintf(stderr,"RxDevice %d: (%s) NewLine - %s\n", ID, RadarName, lbuff);
    else if (debuglevel > 0) 
      fprintf(stderr,"r");
    data = TRUE;
    LastDataRx = time(0);
    RadlTimeout = LastDataRx + timeout;	
    // allow timeout secs for next radl
    // if the scanset is not complete within timeout secs of last radl
    // assume volume has stalled and give up on it
    //	tempf = -100;
    //	if ((sscanf(linebuff->line_buff,"%*c%f",&tempf)== 1) &&
    //	    ((tempf > -10) && (tempf <=360)))
    ChkStatusReset();
    if (hb_size > 0) {  // if header buffer, write it to new_scan
      if (debuglevel > 0) printf("ADDING HEADER - Size=%d of %d\n", hb_size, RXDEV_STRMAX);
      if (new_scan)
	{
	  new_scan->add_line(head_buff,hb_size, true);	// add header line
 	  new_scan->lastScan()->setRxTimeStart(time(0), true);// overwrite created time
	}
      if (RdrStCtl)
	strcpy(RdrStCtl->RdrStatusStr, "Receiving Data");
      NewCommHndlWidSt(TRUE);
      head_buff[0] = 0;
      hb_size = 0;

      if (faultno && new_scan) 
	{		// fault condition, mark scan
	  new_scan->fault_no(faultno, faultstr);
	  faultno = 0;
	}

      // if (new_scan && !new_scan->Faulty() && !new_scan->ShownToScanClients) {
      if (new_scan && !new_scan->ShownToScanClients) { // allow faulty to be
	if (debuglevel > 0)                            // shown to clients
	  fprintf(stderr,"RxDevice %d: (%s) Showing to scan clients through NewDataAvail\n", ID, RadarName);
	NewScanStarted(new_scan);
      }
    }
    if (new_scan)
      {
	new_scan->add_line(linebuff);	// add radial data
      }
  }
  else if (sscanf(lbuff,"MSSG: %d%s",&mssgno,mssgstr) >= 1) {
    if (RdrStCtl) {
      if (mssgstr[0] && (tempstr = strstr(lbuff, mssgstr))) // strip off leading MSSG: & #
	strncpy(RdrStCtl->RdrStatusStr, tempstr, 60);
      else
	strncpy(RdrStCtl->RdrStatusStr, lbuff, 60);
    }
    NewCommHndlWidSt(TRUE);
    mssg = TRUE;
    LastDataRx = time(0);
    if (mssgno != 17) ChkStatusReset();
    switch (mssgno) {
    case 5:	    // list of messages to log
    case 6:	    // as DfltLogLevel only
    case 11:
    case 24:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, DfltLogLevel);
      if ((mssgno == 24) && RdrStCtl)
	sprintf(RdrStCtl->TimeToNextStr, "Starting Volumetric scan"); 
      break;
    case 9:	// cannot get a server connection, give up
      disconnect = TRUE;
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, DfltLogLevel);
      if (AutoReconnect) {
	sprintf(LogStr,"RxDevice%d %s:  (%s) Waiting 15secs before next reconnect attempt\n",
		ID, Description, RadarName);
	RapicLog(LogStr, DfltLogLevel);
	ReconnectRetryTime = time(0) + ReconnPeriod;
      }
      do_newrpstatus(REQFAILED);
      break;
    case 43:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      break;
    case 10:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, DfltLogLevel);
      if (CommReq) {
	sprintf(LogStr,"RxDevice%d %s: (%s) RADAR IN CALIBRATION MODE - TERMINATING REQUEST\n", 
		ID, Description, RadarName);
	RapicLog(LogStr, DfltLogLevel);
	faultno = 1;    // use radar in wind find fault behaviour
	strcpy(faultstr, "RADAR IN CALIBRATION MODE");
	if (new_scan)
	  {
	    if (debuglevel > 0)
	      cerr << "rxdevice::NewLine - Radar in Cal Mode - finishing new_scan "
		   << " and setting endscan" << endl;
	    new_scan->data_finished();
	    endscan = true;
	    new_scan->fault_no(faultno, faultstr);
	  }
	if (new_scan && new_scan->ShouldDelete(this, "rxdevice::NewLine1")) 
	  delete new_scan;
	new_scan = 0;
	disconnect = TRUE;
	do_newrpstatus(REQFAILED);
      }
      break;
    case 30:
      StatusInfo = TRUE;
      if (strstr(lbuff, "Comms Split"))
	SourceType = RPO_SRV;
      if (strstr(lbuff,"END STATUS")) {
	LastDataRx = time(0);
	RapicStatusAck();
	StatusInfo = FALSE;
      }
      break;
    case 23:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, DfltLogLevel);
      if (sscanf(mssgstr, "%d", &temptime)) {
	if (CommReq && (SecsToNextVol >= 0) && 
	    (temptime > SecsToNextVol)) {	// next vol time wrapped arround, must have failed
	  sprintf(LogStr, "RxDevice %d: (%s) Volumetric CommReq Failed - Timed Out\n", ID, RadarName);
	  RapicLog(LogStr, DfltLogLevel);
	  if (new_scan) {
	    if (debuglevel > 0)
	      cerr << "rxdevice::NewLine - Comms failed - finishing new_scan "
		   << "and setting endscan" << endl;
	    new_scan->data_finished();
	    endscan = true;
// 	    new_scan->fault_no(faultno, faultstr);
	    if (new_scan->ShouldDelete(this, "rxdevice::NewLine2")) 
	      delete new_scan;
	  }
	  new_scan = 0;
	  faultno = 0;
	  hb_size = 0;		// reset header, just in case of empty scan (No Image Data)
	  head_buff[hb_size] = 0;
	  if (CommReq) {
	    disconnect = TRUE;
	    do_newrpstatus(REQFAILED);
	    /*
	      strncpy(RadarName, "IDLE", 64);
	      CommReq->AttemptInProg = FALSE;
	      CommReq->rxdev = 0;
	    */
	  }
	}
	SecsToNextVol = temptime;
      }
      break;
    case 44:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (RdrStCtl) RdrStCtl->TxOnState = 0;
      break;
    case 45:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (RdrStCtl) RdrStCtl->TxOnState = 1;
      break;
    case 46:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (RdrStCtl) RdrStCtl->ServoOnState = 0;
      break;
    case 47:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      RdrStCtl->ServoOnState = 1;
      break;
    case 32:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (RdrStCtl) RdrStCtl->VolState = 0;
      break;
    case 31:
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (RdrStCtl) RdrStCtl->VolState = 1;
      break;
    case 48: 
      sprintf(LogStr,"RxDevice%d %s: (%s) %s\n", ID, Description, RadarName, lbuff);
      RapicLog(LogStr, LOG_NOTICE);
      if (sscanf(mssgstr, "%d", &tempint) && RdrStCtl)
	RdrStCtl->RngResState = tempint;
      break;
    }
  }
  else if (sscanf(lbuff,"EXTSTS: %128s", statusstr) == 1) {
    WriteRadarStatusFile(statusstr);
  }
  else if (strstr(lbuff,"RDRSTAT")) {
    if (debuglevel > 0) {
      sprintf(LogStr,"RxDevice%d %s: (%s) RDRSTAT received\n", ID, Description, RadarName);
      RapicLog(LogStr, DfltLogLevel);
    }
    mssg = TRUE;
    LastDataRx = time(0);
  }
  else if (strstr(lbuff,"END STATUS")) {
    mssg = TRUE;
    LastDataRx = time(0);
    RapicStatusAck();
    StatusInfo = FALSE;
  }
  else if (strstr(lbuff,"PRODUCT:"))
    {
      if (strstr(lbuff,"ERROR")) {
	if (faultno == 0) 
	  {
	    faultno = 1;
	    strcpy(faultstr, "ERROR PRODUCT");
	    if (new_scan)
	      new_scan->fault_no(faultno, faultstr);
	  }
	ChkStatusReset();
      }
      else if ((tempstr = strstr(lbuff, "VOLUMETRIC")))
	if (sscanf(tempstr, "VOLUMETRIC %s", str) == 1)
	  {
	    if (new_scan &&                   // if volumetric label doesn't match terminate
		new_scan->scanSetCount() &&   // this volume but don't clear header. Allow new header to start new volume
		(new_scan->scan_type == VOL))
	      {
		if (strcmp(new_scan->volumeLabel(), str)) // if not same volume label terminate this volume
		  {
		    sprintf(LogStr, "RxDevice %d: (%s) Volumetric label %s mismatch, %s\nTerminating volume\n", ID, RadarName,new_scan->volumeLabel(), str);
		    RapicLog(LogStr, LOG_ERR);
		    if (new_scan) {
		      if (debuglevel > 0)
			cerr << "rxdevice::NewLine - Vol label mismatch"
			     << " - finishing new_scan and setting endscan"
			     << endl;
		      new_scan->data_finished();
		      endscan = true;
// 		      new_scan->fault_no(faultno, faultstr);
		      //			    if (new_scan && new_scan->HasOSLock())
		      //				fprintf(stderr,"RxDevice %d: (%s) ****NewLine case 23: ABOUT TO REMOVE SCAN STILL HOLDING OS LOCK****\n", ID, RadarName);
		    }
		    line_mismatch = true;
		  }
	      }
	  }    
	else if ((tempstr = strstr(lbuff, "PPI")))
	  {
	    if (new_scan &&                   // if current scan is VOL, but new header is not
		new_scan->scanSetCount() &&   // close this volume but don't clear header. Allow new header to start new PPI
		(new_scan->scan_type == VOL))
	      {
		sprintf(LogStr, "RxDevice %d: (%s) Attempting to add PPI to VOLUME, terminating volume\n", ID, RadarName);
		RapicLog(LogStr, LOG_ERR);
		if (new_scan) {
		  if (debuglevel > 0)
		    cerr << "rxdevice::NewLine - Attempting to add PPI to VOLUME"
			 << " - finishing new_scan and setting endscan"
			 << endl;
		  new_scan->data_finished();
		  endscan = true;
// 		  new_scan->fault_no(faultno, faultstr);
		  //			    if (new_scan && new_scan->HasOSLock())
		  //				fprintf(stderr,"RxDevice %d: (%s) ****NewLine case 23: ABOUT TO REMOVE SCAN STILL HOLDING OS LOCK****\n", ID, RadarName);
		}
		line_mismatch = true;
	      }
	  }
    }
  else if (sscanf(lbuff,"FAULT: %d %n",&faultno,&tempint) >= 1) {
    sprintf(LogStr,"RxDevice%d %s: (%s) - %s\n", ID, Description, RadarName, lbuff);
    RapicLog(LogStr, LOG_ERR);
    if (tempint < lsize)
	strncpy(faultstr, lbuff+tempint, 128);
    if (RdrStCtl) {
      strcpy(RdrStCtl->RdrStatusStr, "FAULT: ");
      strncat(RdrStCtl->RdrStatusStr, faultstr, 50);
      if (new_scan)
	new_scan->fault_no(faultno, faultstr);
    }
    ChkStatusReset();
    sprintf(str,"Radar Fault - %s %s", RadarName, faultstr);
    if (semiperm)
      SetAlertStatus(AL_RADAR_FAULT, str);
  }
  else if (sscanf(lbuff,"NAME: %s", str) == 1) {
    if (strcmp(RadarName, str)) {  // not the same
      strncpy(RadarName, str, 64);
      NewCommHndlWidSt();
    }
  }
  else if (sscanf(lbuff,"RPQUERY: Query data not %s", str) == 1) {
    if (!semiperm) disconnect = TRUE;
    sprintf(LogStr,"RxDevice%d %s: %s\n", ID, Description, lbuff);
    RapicLog(LogStr, DfltLogLevel);
  }
  if (endscan) {	// end image message to terminate
    NextScanTimeout = time(0) + scantimeout;
    if (RdrStCtl)
      strcpy(RdrStCtl->RdrStatusStr, "Scan Complete");
    NewCommHndlWidSt(TRUE);
    LastDataRx = time(0);	// previous scan set
    ChkStatusReset();
    if (!line_mismatch && (hb_size > 0)) 
      {  // scan without any radials will write header here
	if (debuglevel > 0) printf("ADDING HEADER\n");
	if (new_scan)
	  {
	    new_scan->add_line(head_buff,hb_size, true);	// add header line
 	    new_scan->lastScan()->setRxTimeStart(time(0), true,
						 "rxdevice::NewLine - Header added");// overwrite created time
	  }
	head_buff[0] = 0;
	hb_size = 0;
	if (faultno && new_scan) 
	  {		// fault condition, mark scan
	    new_scan->fault_no(faultno, faultstr);
	    faultno = 0;
	  }
	//      if (new_scan && !new_scan->Faulty() && 
	if (new_scan &&       // allow faulty to be shown to clients
	    !new_scan->ShownToScanClients && new_scan->HeaderValid()) 
	  {
	    if (debuglevel > 0) 
	      fprintf(stderr,"RxDevice %d: (%s) Showing to scan clients through NewDataAvail\n", ID, RadarName);
	    NewScanStarted(new_scan);
	  }
      }
    if (new_scan) {
      if (!line_mismatch) // don't add line if header mismatch
	{
	  // adding this line should cause new scan instance to be appended 
	  // to new_scan
	  new_scan->add_line(linebuff);
	}
      //      if (new_scan && !new_scan->Faulty() && 
      if (new_scan &&   // allow faulty to be shown to clients
	  !new_scan->ShownToScanClients && new_scan->HeaderValid() && ScanMng)
	NewScanStarted(new_scan);
	  //	ScanMng->NewDataAvail(new_scan);
      if (new_scan && new_scan->lastScan())
	{
	  if (debuglevel > 0)
	    {
 	      new_scan->lastScan()->setRxTimeStart(headerStartTime, false,
						     "rxdevice::NewLine - endscan");
	      new_scan->lastScan()->setRxTimeEnd(time(0), 
						 "rxdevice::NewLine - endscan");
	    }
	  else
	    {
 	      new_scan->lastScan()->setRxTimeStart(headerStartTime);
	      new_scan->lastScan()->setRxTimeEnd(time(0));
	    }
	}
      if (new_scan->scanSetCount()) {
	char tempstr[128];
	if (new_scan->scan_type == VOL) {
	  if (new_scan->lastScan() && !(CommMngr && CommMngr->commsConsoleQuiet))
	    {
	      printf("RxDevice %d: %s Volumetric pass %d of %d. Angle = %1.1f ",
		     ID,
		     RadarName, 
		     new_scan->lastScan()->vol_scan_no,
		     new_scan->vol_scans, 
		     new_scan->lastScan()->set_angle/10.);
	      printf("Data size = %d\n ScanTm=%s StartDelay=%dsecs EndDelay=%dsecs",
		     new_scan->scanSetSize(), 
		     ShortTimeString(new_scan->lastScan()->scan_time_t, tempstr),
		     int(new_scan->lastScan()->RxTimeStart() - 
			 new_scan->lastScan()->scan_time_t),
		     int(new_scan->lastScan()->RxTimeEnd() - 
			 new_scan->lastScan()->scan_time_t));
	      printf(" Bytes/sec=%d\n",
		     int(new_scan->lastScan()->thisScanSize() / 
			 new_scan->lastScan()->rxTimeSecs()));
	    }
	  if (RdrStCtl)
	    sprintf(RdrStCtl->TimeToNextStr, "Pass %d of %d. Angle = %1.1f", 
		    new_scan->lastScan()->vol_scan_no,
		    new_scan->vol_scans, 
		    new_scan->lastScan()->set_angle/10.);
	}
	else {
	  if (!(CommMngr && CommMngr->commsConsoleQuiet))
	    {
	      if (new_scan->Complete())
		{
		  char tempstr[128];
		  printf("RxDevice %d: %s Scan complete=%s "
			 "Scan set size = %d\n",
			 ID,
			 RadarName, new_scan->ScanString2(tempstr),
			 new_scan->scanSetSize());
		  sprintf(RdrStCtl->TimeToNextStr, "Scan Complete. Angle=%1.1f", 
			  new_scan->lastScan()->set_angle/10.);
		}
	      else
		{
		  char tempstr[128];
		  printf("RxDevice %d: %s Scan INCOMPLETE=%s "
			 "Scan set size = %d\n",
			 ID,
			 RadarName, new_scan->ScanString2(tempstr),
			 new_scan->scanSetSize());
		  sprintf(RdrStCtl->TimeToNextStr, "Scan Incomplete. Angle=%1.1f", 
			  new_scan->lastScan()->set_angle/10.);
		}
	    }
	}
	SecsToNextVol = 0;
      }
      else {
	if (new_scan && new_scan->HeaderValid())	  
	  sprintf(LogStr, "RxDevice %d: (%s) END OF SCAN DETECTED, NO SCANS COMPLETE!!\n", ID, new_scan->ScanString());
	else
	  sprintf(LogStr, "RxDevice %d: (%s) END OF SCAN DETECTED, NO HEADER & NO SCANS COMPLETE!!\n", ID, RadarName);
	RapicLog(LogStr, DfltLogLevel);
	if (RdrStCtl)
	  sprintf(RdrStCtl->TimeToNextStr, 
		  "(%s) ERROR,  END OF SCAN DETECTED, NO SCANS COMPLETE", 
		  RadarName);
      }
      if (scansettimeout)
	NextScanSetTimeout = time(0) + scansettimeout;
      endscanset |= new_scan->Complete() || new_scan->Finished();
      if (!endscanset && new_scan->Faulty()) {	 // decide whether to stop on fault
	if (semiperm && 
	    ((new_scan->get_fault_no() == 2) || // SEMIPERM, img data not avail, wait
	     (new_scan->get_fault_no() == 1))) { // SEMIPERM, radar in wind find, wait
	  //	  faultno = 0;
	  //	  hb_size = 0;		// reset fault header
	  //	  head_buff[hb_size] = 0;
	  /* need to test this behaviour
	     if (scansettimeout) {
	     NextScanSetTimeout = time(0) + scansettimeout;
	     fprintf(stderr, "RxDevice %d: NewLine - No Data Available or Radar in Wind-Find\n"
	     "Setting NextScanSetTimeout to %s\n", 
	     ID, ctime_r(&NextScanSetTimeout, str));
	     }
	  */
	}
	//	else endscanset = TRUE;  // don't terminate scanset on fault
      }
    }
    else {
      if (semiperm && new_scan && 
	  ((new_scan->get_fault_no() == 2) || // SEMIPERM, img data not avail, wait
	   (new_scan->get_fault_no() == 1))) {	// img data not avail, wait
	faultno = 0;
	hb_size = 0;		// reset fault header
	head_buff[hb_size] = 0;
      }
      else endscanset = TRUE;
    }
  }
  if (endscanset) {
    if (RdrStCtl)
	strcpy(RdrStCtl->RdrStatusStr, "Scan Set Complete");
      NewCommHndlWidSt(TRUE);
      RadlTimeout = 0;	// cancel timeout mechanism
      NextScanTimeout = 0;
      if (scansettimeout)
	NextScanSetTimeout = time(0) + scansettimeout;
      if (new_scan) {
	if (new_scan->scan_type == VOL) 
	  {
	    if (!(CommMngr && CommMngr->commsConsoleQuiet))
	      {
	      if (new_scan->Complete())
		{
		  char tempstr[128];
		  sprintf(LogStr,"RxDevice%d %s: Scan Set Complete=%s\n", 
			  ID, Description, new_scan->ScanString2(tempstr));
		}
	      else
		{
		  char tempstr[128];
		  sprintf(LogStr,"RxDevice%d %s: Scan Set InComplete=%s\n", 
			  ID, Description, new_scan->ScanString2(tempstr));
		}
	      }
	  }
	else 
	  {
	    if (!(CommMngr && CommMngr->commsConsoleQuiet))
	      {
		char tempstr[128];
		sprintf(LogStr,"RxDevice%d %s: Scan Complete=%s\n", 
			ID, Description, new_scan->ScanString2(tempstr));
	      }
	  }
	if (!(CommMngr && CommMngr->commsConsoleQuiet))
	  RapicLog(LogStr, DfltLogLevel);
	if (debuglevel > 0)
	  new_scan->lastScan()->setRxTimeSetEnd(time(0),
						"rxdevice::NewLine - endscanset");
	else
	  new_scan->lastScan()->setRxTimeSetEnd(time(0));
      }
      if (new_scan && new_scan->scanSetCount() &&
	  (new_scan->scan_type == VOL) && 
	  (new_scan->lastScan()->vol_scan_no < new_scan->vol_scans)) {
	sprintf(LogStr, "RxDevice%d %s: (%s) - SCAN SET INCOMPLETE: %d of %d scans\n"
		"%s\n", 
		ID, Description, stn_name(new_scan->station), 
		new_scan->lastScan()->vol_scan_no, new_scan->vol_scans,
		new_scan->ScanString());
	RapicLog(LogStr, LOG_ERR);
	if (debuglevel > 0)
	  cerr << "rxdevice::NewLine - endscanset detected "
	       << " - closing incomplete new_scan. Scan "
	       << new_scan->lastScan()->vol_scan_no << " of "
	       << new_scan->vol_scans
	       << endl;
	new_scan->data_finished();
	faultno = 20;
	sprintf(faultstr, "SCAN SET INCOMPLETE: %d of %d scans",
		new_scan->lastScan()->vol_scan_no, new_scan->vol_scans);
	new_scan->fault_no(faultno, faultstr);
      }
      if (CommReq) {
	disconnect = TRUE;
	do_newrpstatus(REQCOMPLETE);
	if (CommReq->ThisConn)
	  CommReq->ThisConn->Conn->ConnSuccess++;
	if (new_scan) {
	  if (!new_scan->Faulty() ||	// if scan rx'd OK
	      ((new_scan->get_fault_no() == 6) ||	// or Radar turned off, no scan will be avail
	       (new_scan->get_fault_no() == 1)))	// or Radar in Windfind
	    CommReq->ReqCompleted = TRUE; // request is done
	}
	/*
	  strncpy(RadarName, "IDLE", 64);
	*/
      }
      if (new_scan) {
	if (debuglevel > 0)
	  cerr << "rxdevice::NewLine - endscanset detected "
	       << " - finishing new_scan"
	       << endl;
	new_scan->data_finished();
	//	if (!new_scan->Faulty()) {   // show faulty scans to clients
	//	  if (debuglevel > 0) 
	    fprintf(stderr,"RxDevice %d: (%s) Showing to scan clients through FinishedDataAvail\n", ID, RadarName);
	  NewScanFinished(new_scan);
	  //	}
	  //	else 
	  //	  fprintf(stderr,"RxDevice %d: (%s) Scan Faulty, not showing to FinishedDataAvail\n", ID, RadarName);
      }
      else 
	fprintf(stderr,"RxDevice %d: (%s) NewScan == 0!!!!!\n", ID, RadarName);
      if (RdrStCtl && new_scan) {
	if (new_scan->scan_type == VOL) 
	  sprintf(RdrStCtl->TimeToNextStr, "Volume scans complete=%d of %d", 
		  new_scan->vol_scan_no, new_scan->vol_scans);
	else
	  sprintf(RdrStCtl->TimeToNextStr, "Scan complete");
      }
      //	    if (new_scan && new_scan->HasOSLock())
      //		fprintf(stderr,"RxDevice %d: (%s) ****NewLine endscanset: ABOUT TO REMOVE SCAN STILL HOLDING OS LOCK****\n", ID, RadarName);
      if (debuglevel > 0)
	cerr << "rxdevice::NewLine Dereferencing new_scan" << endl;
      if (new_scan && new_scan->ShouldDelete(this, "rxdevice::NewLine3")) 
	{
	  delete new_scan;
	}
      new_scan = 0;
      faultno = 0;
      if (!line_mismatch) {  // we definitely want to keep header on mismatch 
	hb_size = 0;		// reset header, just in case of empty scan (No Image Data)
	head_buff[hb_size] = 0;
      }
      SecsToNextVol = 0;
      NewCommHndlWidSt(TRUE);
      if (!line_mismatch && semiperm)   // send rdrstat req straight away
	SendStatReqStr();
    }
  if (!data && !StatusInfo && !mssg && !endscanset &&
      !(endscan && !line_mismatch)) {	// make all else header info
    if ((hb_size + lsize) >= RXDEV_STRMAX) {
      lsize = RXDEV_STRMAX-hb_size;
      sprintf(LogStr,"RxDevice%d %s: HEADER OVERFLOW ERROR\n", ID, Description);
      RapicLog(LogStr, LOG_ERR);
      memcpy(&head_buff[hb_size],lbuff,lsize);
      hb_size += lsize;
      if (new_scan) 
	{
	  new_scan->add_line(head_buff,hb_size, true);
 	  new_scan->lastScan()->setRxTimeStart(time(0));
	}
      hb_size = 0;
      head_buff[hb_size] = 0;
      if (debuglevel > 0) printf("%s\n",lbuff);
    }
    else { 
      //	    LastDataRx = time(0);   // may not be true header, don't class as data yet
      if (hb_size == 0)
	{
	  headerStartTime = time(0);
	  if (debuglevel > 0)
	    {
	      char tempstr[128];
	      fprintf(stdout, "rxdevice::NewLine - Starting header at %s Header Line - %s\n",
		      ShortTimeString(headerStartTime, tempstr), lbuff);
	    }
	}
      memcpy(&head_buff[hb_size],lbuff,lsize); 
      hb_size += lsize;
      head_buff[hb_size] = 0;
      if (debuglevel > 2) printf("rxdevice::NewLine - Header Line - %s\n",lbuff);
    }
  }
  //	if (StatusInfo || mssg) printf("%s\n",lbuff);
  //    if (mssg) printf("%s\n",lbuff);
  total_rx += lsize;
  if (!line_mismatch)
    linebuff->reset();
  return true;
}
	
char *rxdevice::ConnectedToString() {
  return CommHandler->connectedtostr;
}

char *rxdevice::DescriptionStr() {
  return Description;
}

void rxdevice::SetAlertStatus(e_alert_status newalert, char *alertstr) {
  this_alertstatus = newalert;
  if (alertstr) {
    if (lock) lock->get_lock();
    strncpy(LastAlertStr, alertstr, 128);
    if (lock) lock->rel_lock();
  }
}

void rxdevice::setSuppressAlerts(bool state)
{
char LogStr[256];
  SuppressAlerts = state;
  sprintf(LogStr,"RxDevice%d %s: ", ID, Description);
  if (!state)
    {
      strcat(LogStr," ALERTS ENABLED BY USER INTERFACE\n");
      setSuppressReAlert(false);
      Re_AlertTime = time(0);  // force realert if in alert state
      CheckAlertStatus();
    }
  else
    {
      strcat(LogStr," ALERTS SUPPRESSED BY USER INTERFACE\n");
#ifndef NO_XWIN_GUI
      UnPostRxDevAlert();
#endif
    }
  RapicLog(LogStr, LOG_WARNING);
}

void rxdevice::setSuppressReAlert(bool state)
{
char LogStr[256];
  SuppressReAlert = state;
  sprintf(LogStr,"RxDevice%d %s: ", ID, Description);
  if (!state)
    {
      strcat(LogStr," REALERT ENABLED BY USER INTERFACE\n");
      Re_AlertTime = time(0);  // force realert if in alert state
      CheckAlertStatus();
    }
  else
    {
      strcat(LogStr," REALERT SUPPRESSED BY USER INTERFACE\n");
#ifndef NO_XWIN_GUI
      UnPostRxDevAlert();
#endif
    }
  RapicLog(LogStr, LOG_WARNING);
}

void rxdevice::CheckAlertStatus() {
  time_t thistime = time(0);
  int AlertSilenced = 0;

#ifndef NO_XWIN_GUI
  // manage enabled timeout for disabled_on_startup connections
  // If timeout expires prompt operator for disconnect or extend connect


  if (EnableComms && DisabledOnStartup && enabledTimeout &&
      (time(0) > enabledTimeout))
    {
      if (!enabledTimeoutQDialog)
	{
	  sprintf(enabledTimeoutDialogString, 
		  "Manual Connection %s - timed out.\nPress OK to DISCONNECT\nPress CANCEL to EXTEND CONNECTION",
		  Description);
	  enabledTimeoutQDialog = theQuestionDialog->post(enabledTimeoutDialogString, 
							  &rxdevice::enabledTimeoutDisconnectOKCallback,
							  &rxdevice::enabledTimeoutDisconnectCancelCallback,
							  (XtPointer) this);
	}
    }
#endif

  AlertSilenced = SilenceReAlert || SilenceFutureAlert || 
    SilenceAllAlerts || SuppressAllAlerts || SuppressAlerts;
  if (last_alertstatus != this_alertstatus) {
	/*
	 * Alert status changed
	 */
    SuppressReAlert = false;  // always clear suppressrealert state on alert state change
    if (this_alertstatus == AL_OK) {	// clear alert
#ifndef NO_XWIN_GUI
      UnPostRxDevAlert();
#endif
      Re_AlertTime = 0;
      AlertFirstPosted = 0;
      SilenceReAlert = 0;
#ifdef USE_SOUND
      if (!AlertSilenced && strlen(AlertClearedSound))
	if (SoundManager && !silent) SoundManager->AddSound(AlertClearedSound);
#endif
      //		playaiff(AlertClearedSound);
    }
    else {			    // new alert state
      if (!AlertFirstPosted) AlertFirstPosted = thistime;
      if (lock) lock->get_lock();
#ifndef NO_XWIN_GUI
      if (!(SuppressAllAlerts || SuppressAlerts)) 
	PostRxDevAlert();
#endif
      if (lock) lock->rel_lock();
      SetReAlertTime();
      if (!AlertSilenced) {
	switch (this_alertstatus) {
	case AL_CONN_FAILED:
	case AL_CONN_LOST:
	case AL_RECONN_FAILED:
	case AL_STAT_FAILED:
	  if (strlen(ConnFailSound))
#ifdef USE_SOUND
	    if (SoundManager && !silent) SoundManager->AddSound(ConnFailSound);
#endif
	  //			    playaiff(ConnFailSound);
	  break;
	default:
	  break;
	}
      }
    }
    last_alertstatus = this_alertstatus;
  }
  else if (this_alertstatus != AL_OK) {	// no change, but alert state
    if (Re_AlertTime && (thistime >= Re_AlertTime)) {		
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
	  if (strlen(ConnFailSound))
#ifdef USE_SOUND
	    if (SoundManager && !silent) SoundManager->AddSound(ConnFailSound);
#endif
	  //			    playaiff(ConnFailSound);
	  break;
	default:
	  break;
	}
      }
    }	    
  }
}

void rxdevice::SetReAlertTime() {
  if (Re_AlertPeriod) Re_AlertTime = time(0) + Re_AlertPeriod;
  else Re_AlertTime = 0;
}

void rxdevice::WriteRadarStatusFile(char *statusstring) {
  FILE *RadarStatusFile = 0;
  char tempfilename[256];
  tm	 ts;
  time_t timenow = time(0);

  if (!statusstring || (strlen(RadarStatusFileName) == 0))
    return;
  if (strncmp(statusstring, LastRadarStatusString, 128) == 0)
    return;
  RadarStatusFile = fopen(RadarStatusFileName, "w");
  if (!RadarStatusFile)
    {
      sprintf(LogStr,"RxDevice%d %s: (%s) STATUS CHANGED from %s to %s "
	      "***ERROR - UNABLE TO WRITE RADARSTATUSFILE - %s***\n", 
	      ID, Description, RadarName, LastRadarStatusString, statusstring, 
	      RadarStatusFileName);
      RapicLog(LogStr, LOG_ERR);	
      fprintf(stderr, "rxdevice::WriteRadarStatusFile - Status changed but unable to open file %s\n", 
	      RadarStatusFileName);
      return;	
    }
  else
    {
      fprintf(RadarStatusFile, "New Status = %s\n", statusstring);
      fprintf(RadarStatusFile, "PrevStatus = %s\n", LastRadarStatusString);
      gmtime_r(&timenow, &ts);
      fprintf(RadarStatusFile, "Change Time= %02d/%02d/%04d %02d:%02d UTC\n", 
	      ts.tm_mday, ts.tm_mon, ts.tm_year+1900, ts.tm_hour, ts.tm_min);
      fclose(RadarStatusFile);
      strcpy(tempfilename, RadarStatusFileName);
      strcat(tempfilename, ".flg");
      symlink(RadarStatusFileName, tempfilename);
      sprintf(LogStr,"RxDevice%d %s: (%s) STATUS CHANGED from %s to %s\n", 
	      ID, Description, RadarName, LastRadarStatusString, statusstring);
      RapicLog(LogStr, LOG_NOTICE);	
    }
  strncpy(LastRadarStatusString, statusstring, 128);
}

void rxdevice::SetInitString(char *initstr)
{
  if (initstr)
    strncpy(InitString, initstr, 128);
  if (CommHandler)
    CommHandler->SetInitString(InitString);
}

void rxdevice::ForceReconnect()
{
  char tempstr[64]; 

  Enable(); // ensure device is enabled
  if (EnableComms && DisabledOnStartup && enabledTimeoutPeriod) // manage enable timeout for disabledonstartup devices
    {
      enabledTimeout = time(0) + enabledTimeoutPeriod;
#ifdef SUN
      sprintf(LogStr,"RxDevice%d %s: (%s) enabledTimeout set to %s\n", 
	      ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr, 64));
#else
      sprintf(LogStr,"RxDevice%d %s: (%s) enabledTimeout set to %s\n", 
	      ID, Description, RadarName, ctime_r(&enabledTimeout, tempstr));
#endif
      RapicLog(LogStr, LOG_WARNING);	
    }
  if (!EnableComms && enabledTimeout)
    enabledTimeout = 0;
  reconnect = true;
}


#ifndef NO_XWIN_GUI
void rxdevice::UnPostRxDevAlert() {
  RxDevAlertWindow *thiswin = (RxDevAlertWindow *)AlertWid;
  if (AlertWid) 
    thiswin->hide();
}
    
void rxdevice::PostRxDevAlert() {
  if (SuppressAlerts || !isEnabled())
    return;
  ReAlert();
}

void rxdevice::ReAlert() {
  RxDevAlertWindow *thiswin;
  if (!AlertWid)
    AlertWid = (void *) new RxDevAlertWindow("Receive Device Alert");
  if (AlertWid && isEnabled() && !SuppressAlerts) {
    thiswin = (RxDevAlertWindow *)AlertWid;
    thiswin->upDate(this);
    if (!thiswin->visible()) thiswin->show();
    thiswin->raise();
  }
}
    
#endif

bool rxdevice::writeAlertToFile(FILE *commstatfile)
{
  bool alertActive = (this_alertstatus != AL_OK);
  char tempstr[256] = "";
  
  if (!alertActive || !isEnabled())
    return false;
  else
    {
      fprintf(commstatfile, "**ALERT ON RxDevice %d - %s (%s)\n",
	      ID, Description, RadarName);
      fprintf(commstatfile, "  Alert State=%s\n", LastAlertStr);
      fprintf(commstatfile,"  Alert first posted time - %s\n", 
	      TimeString(AlertFirstPosted, tempstr, true, true));
    }
  return true;
}

void rxdevice::writeStatusToFile(FILE *commstatfile)
{
  char tempstr[256] = "";
  bool alertActive = (this_alertstatus != AL_OK);
  
  if (alertActive)
    strcpy(tempstr, "*");
  fprintf(commstatfile, "%sRxDevice %d - %s (%s) threadid=%ld\n",
	  tempstr, ID, Description, RadarName, long(thread_id));
  
  fprintf(commstatfile, "  Status=%s\n", RPCommStatusString[RPStatus]);
  if (RPStatus == RDRCONNECTED)
    fprintf(commstatfile, "   Connected to %s - Established %s\n", 
	    ConnectedToString(),
	    TimeString(lastConnectTime, tempstr, true, true));
  else if (this_alertstatus != AL_OK)
    {
      fprintf(commstatfile, "  **ALERT STATE=%s\n", LastAlertStr);
      fprintf(commstatfile,"  Alert first posted time - %s\n", 
	      TimeString(AlertFirstPosted, tempstr, true, true));
    }
  if (ConnectFailTime)
    {
      fprintf(commstatfile,"  Connection fail time - %s\n", 
	      TimeString(ConnectFailTime, tempstr, true, true));
    }
  if (FBGroup)
    {
      fprintf(commstatfile, "  Fallback Group=%d\n", FBGroup);
    }
  if (GroupID)
    {
      fprintf(commstatfile, "  GroupID=%d", GroupID);
      if (EnableComms)
	fprintf(commstatfile, " - ACTIVATED BY FALLBACK");
      fprintf(commstatfile, "\n");
    }
  if (ScanFilters)
    {
      fprintf(commstatfile, "  Scan filter count=%d\n", FilterCount);
      RPScanFilter *scanfilter = ScanFilters;
      while (scanfilter)
	{
	  scanfilter->MakeTextFilterString(tempstr);
	  fprintf(commstatfile, "    %s\n", tempstr);
	  scanfilter = scanfilter->next;
	}
    }
  fprintf(commstatfile, "  GetFinishedScans=%d ", GetFinishedScans);
  fprintf(commstatfile, "  AllowReplayScans=%d\n", AllowReplayScans);
  fprintf(commstatfile, "  sendRecentDataMins=%d ", sendRecentDataMins);
  fprintf(commstatfile, "  AutoReconnect=%d\n", AutoReconnect);
  fprintf(commstatfile, "  timeout=%d dflt_timeout=%d scantimeout=%d"
	  " scansettimeout=%d StChkRate=%d\n", 
	  int(timeout), int(dflt_timeout), int(scantimeout), 
	  int(scantimeout), int(StChkRate));
  if (DisabledOnStartup)
    {
      fprintf(commstatfile, "  DisabledOnStartup\n");
      if (enabledTimeout)
	fprintf(commstatfile,"  enabledTimeout set to %s\n", 
		TimeString(enabledTimeout, tempstr, true, true));
    }
  if (lastScanCompletedTime)
    {
      fprintf(commstatfile, "  Last Scan Finished Time - %s\n", 
	      TimeString(lastScanCompletedTime, tempstr, true, true));
      fprintf(commstatfile, "  Last Scan Description   - %s\n", lastScanString);
    }
  fprintf(commstatfile, "  Last Data Rx'd Time - %s\n", 
	  TimeString(LastDataRx, tempstr, true, true));
  fprintf(commstatfile, "  Total Data Rx'd=%1.1fMB\n", float(total_rx/1000000.));
  workProcExecTimer.dumpStatus(commstatfile, "  Exec Timer - ");
  workProcLoopTimer.dumpStatus(commstatfile, "  Loop Timer - ");
  fprintf(commstatfile, "\n");
}

