#ifndef __RPCOMMS_H
#define __RPCOMMS_H
/*
	RapicComm adds Rapic specific services to the Comm class
*/

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "rdrscan.h"
#include "rpdb.h"
#include "comms.h"
#include "uiftovrc.h"
#include "rdrxlat.h"

#ifndef NO_XWIN_GUI
#include <X11/Intrinsic.h>
#endif


class rxdevice;	// forward reference
class RPCommMng;// forward reference

extern const char RPFilterString[];	// RPFILTER:stnid:scantype:datafmt:srcstr
extern const char RPQueryString[];	// RPQUERY: stn scantype angle count qtype qtime
extern const char RPQuerySemiPermStr[];
extern const char RPQuery_ScanByScanStr[];
extern const char RPQueryLatestScanStr[];
extern const char RPAllowReplayScanStr[];	// 
extern const char RPSendRecentDataMins[];	// 
extern const char RPSendRecentDataFrom[];	// 
extern const char RPExpectAckStr[];	// rx end can send this to tx end to indicate rx end will ack scan rx
extern const char RPScanAckString[];	// RDRSCAN_ACK: stnid dd/mm/yy hh:mm [prod] [rxsize] [CRC]

extern char *RPPortTypeString[];
extern char *RPDataFmtString[];
extern char *RPScanTypeString[];
extern char *RPScanTypeStringReadable[];
extern char *RPQueryTypeString[];
extern char *RPCommStatusString[];
extern char *RPCommSrvTypeString[];
extern char *AlertStrings[];
extern char LinkStatusFName[];
extern bool SilenceAllAlerts;	// all alerts silenced
extern bool SuppressAllAlerts;	// all alerts silenced and display suppressed (log entries still generated)

/*
	stn = stn id (0 for all)
	prod = PPI1 (PPI) PPI2 USRPPI USRRHI VOL (ALL for all products)
*/

/*
	stn = stn id
	scantype = PPI1 (PPI) PPI2 USRPPI USRRHI VOL
	angle (floating point) ignored if prod is not USRPPI or USRRHI
	count = no of scans requested
	qtype = LATEST, TOTIME, FROMTIME, CENTRETIME
	qtime = Unix style time_t time for scans (seconds since jan 1 1970)

	Other scan paramaters:
		rng res 	- rng res. in metres
		vid res		- video res in levels
		vid type	- video type: refl, vel, spectwidth

*/

// if these enumerations are expanded, the Get.... functions in
// rpcomms.c will need to have the new strings added, and the upper
// limit enumeration modified
enum RPCommStatus {RDRIDLE, RDRCONNECTING, RDRCONNECTED, RDRSTATUSFAIL, 
    REQCOMPLETE, REQFAILED, COMMCLEAR, COMMLOCKED, CONNFAILED, RDRCONNLOST, 
    RECONNFAILED, DISABLED, ACKFAIL, RDRLISTENING};
enum RP_SVC_TYPE {RPS_NONE, RPS_REQUEST, RPS_SEMI_PERM};
enum RP_PORT_TYPE {RPO_STD1, RPO_STD2, RPO_VOL, RPO_SRV};
enum RP_SCAN_TYPE   {RPT_DFLT_PPI1, RPT_DFLT_PPI2, RPT_USR_PPI, 
		    RPT_USR_RHI, RPT_VOL};
typedef bool SCAN_TYPE_ARRAY[RPT_VOL+1];

enum e_alert_status {AL_OK, AL_CONN_FAILED, AL_CONN_LOST, 
    AL_RECONN_FAILED, AL_ACK_FAIL, AL_STAT_FAILED, AL_CLEAR_ACKFAIL, 
    AL_RADAR_FAULT, AL_CONN_TIMEDOUT };
// ***IF NEW e_alert_status ADDED - NEED TO CORRESPONDING STRING TO 
// commmng.C:char *AlertStrings[]

enum RP_QUERY_TYPE {RPQ_LATEST, RPQ_TOTIME, RPQ_FROMTIME, RPQ_CENTRETIME};
enum DATA_PROTOCOL {DF_RAPIC};

enum AckStatus {UNDEF, ACKMISMATCH, NACKED, ACKTMOUT, ACKOK};

/*

	PORT NAME		DESCRIPTION
	RPO_STD1		Standard product #1 port on Rapic Tx
	RPO_STD2		Standard product #2 port on Rapic Tx
	RPO_VOL			Volumetric port on Rapic Tx
	RPO_SRV			Data TxDevice on 3D-Rapic - may be queried
	if RPO_SRV queried, it won't remove that scan from server queue

	PRODUCT TYPES

	ENUM NAME  DESCRIPTION		AVAILABLE FROM
	__________________________________________________________
	DFLT_PPI1  default scan		RPO_STD1, query to RPO_SRV
	DFLT_PPI2  default scan		RPO_STD2, query to RPO_SRV
	USR_PPI    user defined ppi	RPO_STD1, RPO_STD2
	USR_RHI    user defined rhi	RPO_STD1, RPO_STD2
	SESSION	   session (not vol)	RPO_STD1, RPO_STD2
	VOL	   full vol(inc session) RPO_VOL
*/

/*
	The RPConnection class provides a linked list of connection
	data to access radars.
*/

class RPConnection {
friend class RPCommMng;
	RPConnection *next,*prev;
public:
	int		stn_id;
	ConnModes 	conn_mode;
	RP_PORT_TYPE	port_type;
	DATA_PROTOCOL	data_protocol;
	char		conn_string[64];
	int		conn_rank;
	int		Retries;
	time_t		tmout;		// min time btwn uses of this connection
	time_t		ready_tm;	// time this will be available for use again
	time_t		conn_tmout;	// connection timeout (secs) for this connection
	// if conn_tmout=0, use default for comms handler
	int		ConnAttempts;	// number of times this connection used
	int		ConnSuccess;	// number of times this conn succeeded
	int		ConnFails;	// number of times this conn failed
	RPConnection();
	~RPConnection();
	bool		InUse;
	bool 		DecodeConnStr(char *CONNSTR);	// load string into connection
	void String(char *ConnStr);	// text string of this
	void printState(FILE *outfile);
	};

class RPStnConnPnt {		// pointer to first RPConnection entry for stn
friend class RPCommMng;
	RPStnConnPnt *next,*prev;
	RPConnection *FirstStnConn;
public:
	int						StnID;
	int						EntryCount;
	RPStnConnPnt(RPConnection *RPConn);
	~RPStnConnPnt();
	};

class RPReqConnPnt {		// used for request connection list
public:
	RPReqConnPnt *next,*prev;
	RPConnection *Conn;
	int Rank();						// ConnRank + retries left
	int RetriesLeft;
	time_t	RetryTime;		// time that this may be retried
	time_t	DevAvailTimeOut;	// timeout for device available, remove this if time out
	RPReqConnPnt(RPConnection *NewConn);
	~RPReqConnPnt();
	};

/*
	The RPCommReq class provides a linked list of requests for
	data from a given radar site.
*/

class RPCommReq {
friend class RPCommMng;
friend class rxdevice;
friend class RPSchedEntry;
private:
	RPCommReq	*next,*prev;
	RPReqConnPnt 	*ConnList,  // list of possible conns for this request
			*ThisConn;	// currently selected conn
	int		ThisConnRank;
	bool		AttemptInProg, ReqCompleted;
	rxdevice	*rxdev;		// if in prog, point to handler
	char            rpqueryStr[128];// if specified use this string for RPQUERY:
public:
	int		stn_id;
	RP_SCAN_TYPE	scan_type;
	bool		RdrScanMatch(rdr_scan *matchscan);  // true if rdr_scan matches this req
	bool		e_scan_type_is_same(e_scan_type scantype);// true if scantype matches this req
	rdr_angle	req_angle;
	int		req_count;		// no of scans requested
	RP_QUERY_TYPE	QueryType; 	// RPQ_LATEST, RPQ_TOTIME, RPQ_FROMTIME, RPQ_CETRETIME
	time_t		QueryTime;		// time for query request
	time_t		TimePutOnQ;		// time this request put on queue, could be used to favor old requests
	time_t		TimeLastAttempt;		// time this request was last attempted
	int		priority;
	bf_data_type	bfDataTypes;		// define the data types required
	char		datatypesstring[16];	//
	char		*DataTypesString();	//
	RPCommReq();
	~RPCommReq();
	void		init();
	bool		DecodeQueryString(char *REQSTR);	// load RPQUERY: string into request
	bool		DecodeRequestString(char *REQSTR);	// load request= string into request
	void		MakeQueryString(char *querystring);	// convert request to query string

	void		String(char *ReqStr);	// text string of this
	bool		DecodeString(char *REQSTR);
	bool		IsSame(RPCommReq *testreq);				// return if testreq same as this

	void		ShortString(char *ReqStr);	// text string of this
	bool		DecodeShortString(char *REQSTR);
	bool		ShortIsSame(RPCommReq *testreq);				// return if testreq same as this

	void		AddConn(RPConnection *newconn);			// insert new conn into conn list
	void		DelReqConn(RPReqConnPnt *delreqconn);	// delete given reqconn from list
	void		ResetAllDevAvailTmOut();
	// return best conn for handler, if any
	RPReqConnPnt*	GetConnForRxDev(rxdevice *rxDev);
	void		ClearConnList();
	};

class RPSchedEntry {
friend class RPCommMng;
friend class FallbackComms;
	RPSchedEntry *next,*prev;
	RPCommReq CommReq;
	time_t	schedtime, period, offset, minperiod, fallbackperiod, 
		starttime, stoptime, 
		backupmodetime,		// if != 0 specifies timeout in minutes.
		backupmodetimeout;	// if matching scan not rx'd in time, activate this sched entry
	bool DecodeSchedStr(char *schedstring);
	void CalcSchedTime(int FallBackMode = 0);
	void EncodeSchedStr(char *SchedStr); // rpcomm.ini text string of this
	void String(char *SCHEDSTR);	// readable text string of this
	void DecodeString(char *SCHEDSTR); // convert readable string back
	void printState(FILE *outfile);
	bool IsSame(RPSchedEntry *compare);
	bool	fallbackmode, prevfallback;	// per stn fallback flag
	bool	disabled;		// if disables, schedtime always = 0
	int	GroupID;
	RPSchedEntry();
	~RPSchedEntry();
	void ResetBackupMode();
	bool ScanMatch(rdr_scan *matchscan);
public:
	time_t Period() {return period;};
	time_t Offset() {return offset;};
	};

#define RADAR_COMMTXDEVICE_PORT	(IPPORT_USERRESERVED + 101)
#define RADAR_DBSRV_PORT	(IPPORT_USERRESERVED + 102)
#define RADAR_PORT_DFLT 23

/*
	The rxdevice class handles connection to a radar, and subsequent
	data input (&output for interactive) from the radar.
	For SEMI-PERMANENT connections, reconnection will be automatically
	initiated if status request are not acknowledged by the radar.
	Radar images are inserted in the display manager's sequence, if
	appropriate, and are added to the database.
	A rxdevice is created for each "physical" comms. channel
	assigned to 3D-Rapic, e.g. each socket, X28 port, Hayes port,
	SockX25 port etc.

	The FallbackSched contains a list of sched entries to add to sched
	if the defined station is detected as being in fallback mode.
	THis facility is initially for use by MMS to reduce cost when dial backup
	is invoked.
	If FallbackStn == 0 and rxdevice is semi-perm,  invoke sched entries
	if connection fails. Particularly useful when usually rx'ing scans
	from a 3drapic server,  but connection fails.
	
	A FBToCommServer mode rxdevice mode exists which allows one rxdevice
	connection to a "primary" server to control the schedule and perm rxdevice
	connections on this machine.
	If the "primary" server connection is OK, this machine's schedule and
	semi-perm rxdevices will be held disabled.
	If the "primary" server connection fails, this machine's schedule and 
	semi-perm rxdevices will be enabled until the primary server connection 
	is re-established.
	
	The FBGroup option allows one rxdevice to hold a "group" of
	schedule entries of the nominated group number disabled while
	the connection is maintained.
	THe group will be enabled if the rxdevice's connection fails, and
	disabled again when the connection is re-established.
	This mechanism allows for unlimited nesting of fallback devices

*/

class FallbackComms {
friend class RPCommMng;
friend class rxdevice;
    FallbackComms   *next;		// single linked list
    rxdevice	    *ParentRxDev;	// rxdevice that "owns" this
    int		    Stn;	        // if fallback indicated on this stn, discontinue semi-perm until fall-forward
    bool	    FBState, FBDetected ;// if fallback, discontinue semi-perm, use SchedEntry
    bool	    SuppressMainSchedMode; // in this mode, main sched is suppressed
    bool	    MainSchedEnabled;	// current state of the sched disable
    bool	    SuppressOtherRxDevMode; // in this mode all other perm-mode rxdevices are disabled while
					// not in fallback mode
    bool	    OtherRxDevEnabled;	// current state of rx device disables
    // while the ParentRxDev connection is connected, 
    // If the perm connection fails, then main sched will be enabled
    RPSchedEntry    *FBSchedEntryList;	// these entries are in schedule list, but disabled until fallback invoked
    rxdevice	    *FBRxDev;		// rxdevice to open if fallback invoked
    FallbackComms(rxdevice *parentrxdev, int stn = 0);
    ~FallbackComms();
    void AddSchedEntry(RPSchedEntry *SchedEntry); // attach this entry to list. Caller shouldn't delete
    bool CheckFallback(RPCommMng *CommMng = 0); 
    // if mode changed, activate sched/rxdevice through commmng if fallback
    // else deactivate
    };

extern const char RPFilterString[];  // format RPFILTER:stnid:scantype:datafmt:srcstr
extern const char rawRPFilterString[];  // format RPFILTER:stnid:scantype:datafmt:srcstr
/* RPFilterString enum definitions (refer rdrscan.c)
enum	e_scan_type {PPI, RHI, CompPPI,IMAGE,VOL,RHISet,MERGE, SCANERROR};
enum	e_data_type {refl,vel,spectw};
enum	e_data_fmt {RLE_6L_ASC,RLE_16L_ASC,RAW_8BIT,RLE_8BIT};
enum	e_data_source {COMM,DB, COMMREQ, PROD_ACCUM};	// source class of data
enum	e_data_type {refl,vel,spectw};
e.g. RPFILTER:-1:0:0:0	    would specify any stn,  refl only, RLE 6lvl only, COMM source only
*/
struct RPScanFilter {
    RPScanFilter *next, *prev;	// for filter list
    int		station;	// if station < 0 match on all stations
    int 	scan_type;	// (e_scan_type)if type < 0 match on all types

  /* 
     NOTE: format has a dual usage
     Where format is < e_df_max it will be treated explicitly as an e_data_fmt enumerator
     Where format is > e_df_max it will be interpreted as specifying the number of levels required
  */
    int 	format;		// (e_data_fmt)if type < 0 match on all formats


    int 	data_source;	// (e_data_source)if data_source < 0 match on all sources
    int		data_type;	// (e_data_type)
    bool	valid;
    char        *LastSentString;
    char        *LastSentStatus;
/*     string	LastSentString; // last product sent description */
/*     string	LastSentStatus; // last product sent status description */
    string      rawRPFilterStr; // actual RPFILTER: string before decoding 
    bool        useRawFilterStr;     // if true use rawRPFilterStr instead of
                // making the filter string from the fields
    RPScanFilter(char *filterstr = 0);
    ~RPScanFilter();
    bool DecodeFilterStr(char *filterstr = 0); // attempt to decode string
    void  MakeFilterString(char *filterstring);	// convert filter to filter string
    void  MakeTextFilterString(char *filterstring);// convert filter to readable filter string
    bool MatchingFmt(rdr_scan *scan);
    bool MatchingScan(rdr_scan *scan);	// true if scan matches filter
    };

struct stnScanStats {
  int    newScans, dupScans, incompleteScans, faultScans;
  time_t lastScanTime, lastScanRxTime;
  stnScanStats() 
  { 
    newScans = dupScans = incompleteScans = faultScans = 0;
    lastScanTime = lastScanRxTime = 0;
  };
};

class stnScanStatsMap {
 public:
  map <int, stnScanStats*> scanStatsMap;
  stnScanStats* getScanStats(int rdr, bool createflag = false);
  stnScanStats* createScanStats(int rdr);
  void incNewScans(int rdr);
  void incDupScans(int rdr);
  void incIncompleteScans(int rdr);
  void incFaultScans(int rdr);
  void setLastScanTime(int rdr, time_t tm);
  void setLastScanRxTime(int rdr, time_t tm);
  int  getNewScans(int rdr);
  int  getDupScans(int rdr);
  int  getIncompleteScans(int rdr);
  int  getFaultScans(int rdr);
  time_t getLastScanTime(int rdr);
  time_t getLastScanRxTime(int rdr);
};

extern stnScanStatsMap* StnScanStats;

#define RPRXDEVICEFILTERMAX 100
#define RXDEV_STRMAX 4096	

enum HandlerListStatus {HLS_NOLIST, HLS_FREE, HLS_INUSE};

class rxdevice : public ThreadObj {
friend class RPCommReq;
friend class RPCommMng;
#ifndef NO_XWIN_GUI
friend class CommRxDevMngForm;
friend class RxCommStatusWindow;
friend class RxCommStatusForm;
friend class RxDevAlertWindow;
#endif
friend class FallbackComms;
protected:
	Comm  		*CommHandler;	    // Communications handler
	char 		new_connstr[128];	// pass new connection string
	char		LogStr[256];
	char		connectedtostring[128];
	char		InitString[128];    // string to initialise device. e.g. Hayes init
	bool		disconnect, reconnect;
	bool		clear;
	bool		listenmode;
	bool		newctlval;
	bool		notInUse;		// if true, do not connect or listen
	bool		EnableComms;	// if FALSE, stops comms processing
	bool		DisabledOnStartup;	// if TRUE, will start up disabled. Must be enabled manually through GUI
	time_t		enabledTimeoutPeriod;	// number of seconds to leave DisabledOnStartup connection open
	time_t		enabledTimeout;	// timeout
#ifndef NO_XWIN_GUI 
	Widget          enabledTimeoutQDialog;
	char            enabledTimeoutDialogString[256];
	static          void enabledTimeoutDisconnectOKCallback(Widget w, XtPointer clientData, XtPointer callData);
        void            enabledTimeoutDisconnectOK();
	static          void enabledTimeoutDisconnectCancelCallback(Widget w, XtPointer clientData, XtPointer callData);
        void            enabledTimeoutDisconnectCancel();
#endif
	bool		DisableConnectConfirm;	// disable status ack confirmation requirement
	bool		silent;		// disable sounds
	bool		PrimaryServerConnection; // if true this is a connection to a primary comms server
	int		debuglevel;
	// 
	
	HandlerListStatus   ListStatus; // which handler linked list this is on
	char		ListenStr[64];
	char		RadarName[64];
	char		Description[128]; // description of this rxdevice's function
	time_t          lastConnectTime;  // time last connection successfully established
	RPCommReq	*CommReq, *newcommreq;
	// if CommReq defined, send query on connection (if appropriate)
	// then disconnect on completion, else camp on line
	bool		SendQuery;	// true if appropriate to send query string
	
	char		RadarStatusFileName[128];	// if defined and the EXTSTS: string is
				// rxd from the radar, the string will be written to the named file
	char		LastRadarStatusString[128];		// last string written to file
	void		WriteRadarStatusFile(char *statusstring);
	bool            checkRadialsForMSSG;  // if true check radials for embedded MSSG: 30 string
	void            checkLineBuffRadialForMSSG();
#ifndef NO_XWIN_GUI
	void		*CommHndlWid;
	void		*RxDevStatusWid;
	void		*AlertWid;	    // alert Widget
#endif
	time_t		Re_AlertTime;	    // time to re-alert
	time_t		Re_AlertPeriod;	    // time between re-alerts
	time_t		AlertFirstPosted;   // time to re-alert
	bool		SilenceReAlert, SilenceFutureAlert, 
	  SuppressReAlert, SuppressAlerts;
	void            setSuppressAlerts(bool state);
	void            setSuppressReAlert(bool state);
	char		AlertClearedSound[128]; // aifc sound file. use commmng name if not defined
	char		ConnFailSound[128]; // aifc sound file. use commmng name if not defined
	e_alert_status	last_alertstatus, this_alertstatus;
	char		LastAlertStr[128];
	// SetConnAlert used by child thread to set
	// alert status flags for main thread to pick up via CheckAlertStatus
	// LastAlertStr changes must be thread safe
	// SetAlertStatus() (no params) will clear alert
	void		SetAlertStatus(e_alert_status = AL_OK, char *alertstr = 0);
	// CheckAlertStatus, MUST BE CALLED BY MAIN THREAD ONLY
	// call PostAlert etc which operate on Widgets directly
	void		CheckAlertStatus(); 
	void		SetReAlertTime();
	void		PostRxDevAlert();
	void		ReAlert();
	void		UnPostRxDevAlert();
	RdrStCtlStruct	*RdrStCtl;
	void		SetRdrStCtl();
	FallbackComms	*FBComms;  // see above					
	int		FBGroup;    // groupid to fall back too if connect fails
	bool		FBGroupState;    // current state of fbgroup
/* 	RPCommMng	*CommManager; */
	/* Scan filters on the rxdevice are configured via rpcomm.ini
	 * to be passed to a txdevice upon connection.
	 * If no filters are defined the txdevice will send all scans it
	 * receives.
	 */
	RPScanFilter	*ScanFilters;	    // list of scan filters
	int		FilterCount;
	void		AddFilter(RPScanFilter *addfilter);
	void		AddFilter(char *filterstr);
	void		ClearFilters();
	void		SendFilters();
	virtual void	NewScanStarted(rdr_scan *NewScan = 0);
	virtual void	NewScanFinished(rdr_scan *NewScan = 0);
	bool		ConfirmConnection(int timeoutsecs = 20);
	void		SendStatReqStr();
public:
	rxdevice(Comm *CommHndl = 0);
	~rxdevice();			// NOTE: DOES NOT RELINK PAST this in destructor
	rxdevice	*next,*prev,	// links for RxDevInUse/RxDevFree list	
			*adminnext, *adminprev;// links for admin use
	int		ID;		// RxDev id
	int		GroupID;
	RP_SVC_TYPE	ServiceType;
	RP_PORT_TYPE	SourceType;	// data source type
	bool		GetFinishedScans;   // ask for completed scans only from txdevice
	bool		AllowReplayScans;   // allow replay mode scans from txdevice
	int		sendRecentDataMins; // ask for recent data (mins) from txdevice
	time_t		sendRecentDataFrom; // ask for recent data from time from txdevice
	bool		GetLatestScan;   // ask for latest scans only from txdevice, not backlog
	RPCommStatus	GetStatus();
	// indicate to commmng that new status values need to be written to widget
	bool		NewCommHndlWidStFlag;	
	void		CheckRapicStatus();   // request Rapic status
	void		ReadRP();
	void		Set_Disconnect();
	void		Set_Clear();
	void		Enable(bool state = true);
	void		Disable();
	void		ForceReconnect();
	bool		isEnabled();
	void		setInUse(bool flag = true);
	bool		isInUse();
	void		do_newconnect(char *connstr);			// simple port connection
	void		Set_NewConnect(char *connstr);			// simple port connection
	void		do_newconnect(RPCommReq *NewReq);	// Comm request connection
	void		Set_NewConnect(RPCommReq *NewReq);	// Comm request connection
	void		Set_Listen(char *portstr = 0);
	void		Set_AutoReconnect(bool flag = true);
	void		CheckComms();
	virtual void	workProc();
	virtual void	threadExit(); // allow thread stopped tidy up
	virtual void	setStopThreadFlag();
#ifndef NO_XWIN_GUI
	void		OpenCommHndlWid(int StatusOnly = 1); // open comms handler widget
	void		CloseCommHndlWid();	// close widget
	void		CommHndlWidClosed();	// close widget
	void		NewCommHndlWidCtl();	// read new control values
	void		do_newcommhndlwidctl();	// notify thread of new wid ctl values
	void		UpDateRxDevWid(bool force = FALSE);	// act on new wid ctl values
	void		OpenRxDevStatusWid();	// open comms handler widget
	void		CloseRxDevStatusWid();	// close widget
	void		RxDevStatusWidClosed();	// close widget
#endif
	void		NewCommHndlWidSt(bool force = FALSE);	// act on new wid ctl values
	void		CmdTxOnState(int newstate);
	void		CmdServoOnState(int newstate);
	void		CmdVolState(int newstate);
	void		CmdRngResVal(int newstate);
	void		DemandPPI(rdr_angle demandel);
	void		DemandRHI(rdr_angle demandaz);
	void		SendCommand(char *command);
	char		*ConnectedToString();
	char		*DescriptionStr();
	void		SetInitString(char *intstr);
	bool            writeAlertToFile(FILE *commstatfile);
	void            writeStatusToFile(FILE *commstatfile);
protected:
	void		RapicStatusAck();     // END STATUS message received
	void		ChkStatusReset();     // reset timeouts, used when rxing data
	bool		NewLine();            // return false if parsing failed, needs to re-parse
	void 		do_disconnect();
	void 		do_clear();
	bool		StatusReq;
	bool		StatusInfo;
	bool		AutoReconnect;// switch for reconnect on status fail
	bool		StatusChanged;
	RPCommStatus	RPStatus;
	void		do_newrpstatus(RPCommStatus newstatus);
	void		makesemipermquery(char *tempstr);
	time_t		ChStTime;     // Time for next Rapic status check
	time_t		StChkRate;    // time between checks
	time_t		StAckTmOut;   // time of status acknowledge time out
	time_t		StAckTime;    // time to allow before timeout
	time_t		LastDataRx, RadlTimeout, lastScanCompletedTime, 
			NextScanTimeout, NextScanSetTimeout;	// time last data or status received
	time_t		SecsToNextVol;	// seconds to next volume scan
	time_t		UpdateStatusWidTime;
	time_t		ConnectFailTime; // time connection failed, reset to 0 on successful connect
	time_t		ReconnectRetryTime;	// time to try autoreconnect again
	time_t		ReconnPeriod;	// period btwn reconnect retry attempts
	time_t		ReconnDelay;	// period btwn disconnect/reconnect
	time_t		timeout, scantimeout, scansettimeout;      // per connection connect and rx data timeout, may be specified by connection
	time_t		dflt_timeout;	// default connect and rx data timeout
	char		StatStr[64];	// string to prompt radar for status
	char            lastScanString[128];
	rdr_scan_linebuff *linebuff;	// buffer to assemble Rapic data lines
	long long		total_rx;			// total data recieved
	char		head_buff[RXDEV_STRMAX];
	int		hb_size;
	time_t          headerStartTime;
	char		outstr[128];
	int		faultno;
	bool		error;
	rdr_scan	*new_scan;
	spinlock	*lock;
	};


#define RDRSTATTMOUT 60
#define RDRSTATMSSGTM 10
#define RDRSCANACKTMOUT 30
#define RPTXDEVICEFILTERMAX 200

class txdevice : public scan_client {
#ifndef NO_XWIN_GUI
friend class CommTxDevMngForm;
friend class TxCommStatusForm;
friend class RxDevAlertWindow;
#endif
friend class RPCommMng;
	bool		ListenMode;
	char		ListenStr[64];
	char 		new_connstr[128]; // pass new connection string
	char		LogStr[256];
	char		Description[128]; // description of this txdevice's function
	bool		disconnect, reconnect, clearq;
	bool		EnableComms; // if FALSE, stops comms processing
	int		debuglevel;
	/*
	 * Perm scan filters are set by the rpcomm.ini config.
	 * Typically where the txdevice is connecting to a known
	 * destination to send specified data
	 * TempScan Filter strings are passed by the receiving end.
	 * Typically the txdevice is available for incoming connections
	 * and has no Perm scan filters.
	 * Temp scan filters should be cleared on loss of connection.
	 */
	int 		FilterCount;	// count of filter entries
	RPScanFilter	*PermFilters;	// tx list of scan filters
	RPScanFilter	*TempFilters;	// list of receive end specified scan filters
	bool		FiltersChanged;	// if new temp filter settings
	// If NOT QueryMode, assume SemiPerm connection type:
	// the tx node is removed from the front of the queue,
	// new scans are added to the FRONT and will be transmitted
	// BEFORE older scans.
	// This allows auto culling of oldest from the end of the queue
	// If Query mode, assume a transient connection:
	// Take appropriate scans from nearest front of queue, but do not remove.
	// 
	rdr_scan_node	*first,*last,*tx;
        time_t          lastTxScanChangeTime;
        time_t          txScanChangeTimeOut;
	exp_buff_rd	*ExpBuffRd;
	int		ScanQSize, NewScanQSize, MaxScanQSize, // scan queue can grow beyond max. with different products
			ScanQHardLimit;	  // newDataAvail will not add scans beyond hard limit  
	int 		TxBuffSize;
	char		*TxBuff;
	int		TxBuffIPPos,TxBuffOPPos;
	char		ln_buff[1024];	// left justified rx string
	int		ln_sz;
	int 		RxBuffSize;
	char		*RxBuff;
	int		RxBuffPos;
	bool		RdrStatRequest;
	time_t 		RdrStatTimeOut;
	time_t 		RdrStatMssgTime;
	time_t		ReconnectRetryTime;
	time_t		ReconnPeriod;	// period btwn reconnect retry attempts
	time_t		ReconnDelay;	// period btwn reconnect retry attempts
	int		AckFailTimeout;	// max time to wait for scan ack
	int		QueryModeTimeout; // Time to wait for query - units of 0.5secs
	bool		QueryMode;
	bool		SemiPerm;	// true if connected is semi-perm
	bool		transientDevice;// this txdevice is transient, commmng will delete after an idle period
	time_t		transientTimeout; // commmng will delete this device if the timeout is exceeded
	RPCommReq	CommReq;	// incoming radar scan request
	bool		Expect_Ack;	// if true expect other end to acknowledge scan
	bool		SendRefTime;	// if true send a reference time string before each scan
	bool		SendTxStnSet;	// if true send stnset to be tx'd
	AckStatus	Ack_Match;	// true if RDRSCAN_ACK string matches tx'd scan
	time_t		Ack_Timeout;	// time
	int		Ack_FailCount;  // if ack fails recur (3 times), it is most likely a bad tx scan, remove it
	bool		WriteTxBuff(int tmout = 0); // tmout in 1/100ths sec
	rdr_scan_node	*NextQueryScan(rdr_scan_node* thisscan);
	bool		ScanMatchesReq(rdr_scan *scan, RPCommReq *thisReq);
	bool		GetQueryScan();
	bool		GetScanMngScan();
	bool		GetRealTimeSeqScan();
	bool		GetLatestDBScan();
	bool		StatusChanged, FilterListStatusChanged;
	bool		silent;		// disable sounds
	RPCommStatus	RPStatus;
	void		do_newrpstatus(RPCommStatus newstatus);
	time_t		ConnectFailTime; // time connection failed, reset to 0 on successful connect
	time_t          lastConnectTime;  // time last connection successfully established
public:
	int		ID;		// rxdev numeric id
	Comm		*CommHandler;
	txdevice(Comm *commhandler);
	~txdevice();
	txdevice	*next,*prev;	// NOTE: DOES NOT RELINK PAST this in destructor
	bool		AutoReconnect;
	bool		TxFinishedScans;// if true wait until scan is "finished" before sending
	bool		TxLatestScan;	// if true only send latest matching scan on incoming connection 
	int		sendRecentDataMins; // remote asking for recent data (mins) from txdevice
	time_t		sendRecentDataFrom; // remote asking for recent data from time from txdevice
	bool            loadingRecentScans;
	bool		CallingMode;	    // true if this txdevice originates call
	virtual void	setStopThreadFlag();
	virtual void	workProc();
	virtual void	threadExit(); // allow thread stopped tidy up
	void		CheckComms();
	void		CheckNewConnection();
	void		Enable();
	void		Disable();
	bool		isEnabled();
	void		Set_NewConnect(char *connstr);
	void		Disconnect();
	void		Reconnect();
	void		Listen(char *portstr = 0);
	void		Reset();    // reset state to default 
	RPCommStatus	GetStatus();
//	void		WriteBuffer();
	void		CheckDataTx();
	void		CheckDataRx();
	void		NewLine();
	void		AddFilter(RPScanFilter *addfilter, RPScanFilter **DestList = 0);
	void		AddFilter(char *filterstr, RPScanFilter **DestList = 0);
	void		ClearPermFilters();
	void		ClearTempFilters();
	bool		GetFilterTxScan();
	bool		FilterScan(rdr_scan *scan, RPScanFilter *matchfilter, RPScanFilter **FilterList = 0);
	bool		CheckScanAck();
	AckStatus	CheckAckString(char *AckString, rdr_scan *ackscan);
	void		DelFaulty();	// delete any faulty scans
	bool		DelOldestMatching(rdr_scan *scan);	// delete the oldest matching (not tx) scan on queue
	bool		DelOldest();	// delete the oldest (not tx) scan on queue
	virtual int	NewDataAvail(rdr_scan *new_scan); // new scans place on queue by other threads
	virtual void	CheckNewScans();// process newscans queue in txdevice thread
	virtual bool Full(int passedSize = -1);
	void		do_clearscanq();
	void		do_checkscanq();
	void		RemoveTxScan();
	void		ClearScanQ();
	void		ResetPartialTx();   // tx scan not fully sent, reset
	char		*ConnectedToString();
	char		*DescriptionStr();
#ifndef NO_XWIN_GUI
	void		*TxDevStatusWid;
	void		*AlertWid;	    // alert Widget
#endif
	time_t		Re_AlertTime;   // time to re-alert
	time_t		Re_AlertPeriod; // time between re-alerts
	time_t		AlertFirstPosted;   // time to re-alert
	bool		SilenceReAlert, SilenceFutureAlert, 
	  SuppressReAlert, SuppressAlerts;
	void            setSuppressAlerts(bool state);
	void            setSuppressReAlert(bool state);
	char		AlertClearedSound[128]; // aifc sound file. use commmng name if not defined
	char		ConnFailSound[128]; // aifc sound file. use commmng name if not defined
	char		AckFailSound[128];
	e_alert_status	last_alertstatus, this_alertstatus;
	int		AlertAckFailFlag;
	char		LastAlertStr[128];
	// SetConnAlert used by child thread to set
	// alert status flags for main thread to pick up via CheckAlertStatus
	// LastAlertStr changes must be thread safe
	// SetAlertStatus() (no params) will clear alert
	void		SetAlertStatus(e_alert_status = AL_OK, char *alertstr = 0);
	// CheckAlertStatus, MUST BE CALLED BY MAIN THREAD ONLY
	// call PostAlert etc which operate on Widgets directly
	void		CheckAlertStatus(); 
	void		SetReAlertTime();
	void		PostTxDevAlert();
	void		ReAlert();
	void		UnPostTxDevAlert();
#ifndef NO_XWIN_GUI
	void		UpDateTxDevWid(bool force = FALSE);	// act on new wid ctl values
	void		OpenTxDevStatusWid();	// open comms handler widget
	void		CloseTxDevStatusWid();	// close widget
	void		TxDevStatusWidClosed();	// close widget
#endif
	void		RefTimeStr(char *strbuff);
	void		TxStnSetStr(char *strbuff);
	virtual void	PrintScanUsage(FILE *file, bool verbose = false);
	virtual bool    ScanSourceOK(rdr_scan *srcscan);	//returns true if scan is from an appropriate source
	bool            writeAlertToFile(FILE *commstatfile);
	void            writeStatusToFile(FILE *commstatfile);
	time_t          lastScanTxTime;
	char            lastScanTxString[128];
    };
    
#define RADAR_PORT_DFLT 23
#define RADAR_SERVER_DFLT 15555
#define RAPIC_CONVERTER_DFLT 15556

extern u_short RapicServerPort;
extern u_short DefaultListenPort;
extern u_short DefaultConnectPort;

class RPCommMng : public scan_client {
#ifndef NO_XWIN_GUI
friend class CommTxDevMngForm;
friend class CommRxDevMngForm;
#endif
	rxdevice	*RxDevInUse;
	rxdevice	*RxDevFree;
	rxdevice	*RxDevHead, *RxDevTail;// allows add rxdev to tail of list
	txdevice 	*TxDevHead, *TxDevTail;// allows add rxdev to tail of list
	RPSchedEntry	*SchedEntries, *ThisSchedEntry;
	RPCommReq	*ReqPending, *tempreqentry;
	RPConnection 	*ConnQueue;		// available connections, kept in stn order
	RPStnConnPnt	*ConnStnPnt,*ThisConnStnPnt;	// list of stn conn pointers, in stn order
	spinlock	*lock;				// spin lock for multi thread env.
	int		TxDeviceQSize;		// default tx queue for txdevice
	int		NextRxID,NextTxID;
	rdr_xlat	*Rdr_Xlat;		// allow one rdr_xlat object

	/*
	 * SchedDisabled can be used by rxdevice to disable schedule
	 * whilst successfully connected to and receiving data from a 
	 * another 3drapic server.
	 * Scheduler can be enabled if primary server fails, ie this is
	 * a fallback or "hot spare" machine 
	 */
	bool		SchedEnabled;		// flag to enable/disable schedule operation
	bool		createTxDevOnDemand;	// if true, txdevices will be created if there aren't
						// any free to handle incoming request
	bool            disableListen;          // if set, don't open server socket for listen
	char		FallBackFName[64];
	char		LogStr[256];
	char		ThisConfigName[256];

	void		CheckTxDeviceListenCount();
	
	bool		IsOpen;			// true if commmng is currently "open"
	bool		CheckFallBackMode;	// true if any sched entries define separate fallback period
	bool		FallBackMode;
	void		GetFallBackMode();
	void		FallBackDetected();
	void		FallForwardDetected();
	void		SetPerStnFallback(int stnid);   // mark any sched entries for this stn as fallbackmode
	void		CheckNetworkStatus();
	void		InitFBDetRxDev();	// reset all rxdev's FBComms FBDetected flag
	void		CheckFBDetRxDev();	// Check all rxdev's FBComms FBDetected flag
	
	// set the enabled state of the group 
	void		CheckFlagFiles();	// check for comms flag files
	void		CheckNewConfigFile();
	void		CheckConfigAddFile();
	void		CheckNewReqFile();
	void		CheckNewSchedFile();
	void		CheckDumpSchedFile();
	void		CheckDumpConnFile();
	
	bool ScanTypeAvailFromPort(RP_SCAN_TYPE scan_type,
			RP_PORT_TYPE port_type);
	// return true if scan type may be accessed via port

	int 		ReqToConnRank(RPCommReq *Req,RPConnection *Conn);
	// return ranking > 0  if connection suitable for req
	// ranking based on conn type preference and retries

	// Get the highest ranking connection for the given request
	int		GetBestReqConn(RPCommReq *Req,RPConnection *Conn);

	// assemble the connection list for the given request
	bool		GetReqConnList(RPCommReq *Req);
	bool		NoRxDevAvail(RPConnection *Conn);

	// get the highest ranking (oldest) request for the given handler
	RPCommReq*	GetBestReqForRxDev(rxdevice *RxDev);
	virtual int	NewDataAvail(rdr_scan *new_scan); // IGNORE NEW, WAIT FOR FINISHED, DO NOTHING
	virtual int	FinishedDataAvail(rdr_scan *finished_scan); // new scans place on queue by other threads
	virtual bool Full(int passedSize = -1);		// client "full indication
	virtual void    CheckNewScans();	// check new scans list
    
public:
	bool		SingleThreadComms;
	bool            commsConsoleQuiet;
#ifndef NO_XWIN_GUI
	void		*CommMngWid, *newCommMngWid;
	void		*SchedEditWid;
	void		*ReqEditWid;
#endif
	time_t		TxRe_AlertPeriod;	// default time between re-alerts
	char		TxAlertClearedSound[128];// default rx aifc sound file. use commmng name if not defined
	char		TxConnFailSound[128];	// default tx aifc sound file. use commmng name if not defined
	char		TxAckFailSound[128];
	time_t		RxRe_AlertPeriod;	// default time between re-alerts
	char		RxAlertClearedSound[128];// default rx aifc sound file. use commmng name if not defined
	char		RxConnFailSound[128];	// default tx aifc sound file. use commmng name if not defined
	float		RxLoopDelay;		// default rxdevice loop delay
	time_t		RxReconnPeriod;		// default rxdevice reconnect retry time
	time_t		RxReconnDelay;		// delay btwn disconnect/reconnect
	float		TxLoopDelay;		// default txdevice loop delay
	time_t		TxReconnPeriod;		// default txdevice reconnect retry time
	time_t		TxReconnDelay;		// delay btwn disconnect/reconnect
	int		TxAckFailTimeout;	// max time to wait for scan ack
	bool		TxSendRefTime;	// if true send a reference time string before each scan
	bool		TxSendTxStnSet;	// if true send stnset to be tx'd
	bool            RxCheckRadialsForMSSG;  // if true all rxdevices check radials for embedded MSSG: 30 string
	time_t	        CheckFlagFilesTime;	// periodically check for newcommflagname
	time_t	        CheckFlagFilesPeriod;	// every 15 secs    
	time_t	        writeStatusFileTime;	// periodically check for newcommflagname
	time_t	        writeStatusFilePeriod;	// every ?? secs    
	char            *statusFileName;
	void		SetTxDevDefaults(txdevice *txdev);  // set commmng supplied defaults
	void		SetRxDevDefaults(rxdevice *rxdev);
	RPCommMng(char *inifile = 0);
	~RPCommMng();
	void		Open(char *configname = 0, int CloseExisting = 1);
	void		Close();
	void		LoadConfig(char *configname = 0);	// read in rpcomm.ini file
	void		SaveConfig();	// save comms config rpcomm.ini, particularly schedule alterations
	void		CheckReq();	// check request queue, try to service requests
	void		CheckSched();	// check scheduler, add req if due
	void		CheckComms();	// if single thread, poll main loop of RxDev and txdevices
	bool		IsDupReq(RPCommReq *Req);	// true if same request already on queue
	void		AddReq(RPCommReq *Req);
	void		AddReq(int stn, RP_SCAN_TYPE ScanType=RPT_DFLT_PPI1, rdr_angle ReqAngle=0,
					int ReqCount=1, RP_QUERY_TYPE QueryType=RPQ_LATEST, time_t QueryTime=0);
	void		RemReq(RPCommReq *Req);
	void		AddSched(int stn, RP_SCAN_TYPE ScanType, rdr_angle ReqAngle,
			    int period, int offset);
	void		AddSched(RPSchedEntry *schedentry);
	void		SetSchedEnable(bool state = TRUE);
	void		SetOtherRxDevEnable(bool state = TRUE);
	void		SetGroupEnable(int groupid, bool state, bool nested = false);  

	RPSchedEntry	*GetSchedEntry(char *str);
	RPSchedEntry	*GetSchedEntry(RPSchedEntry *schedentry);
	RPSchedEntry	*GetSchedEntryNum(int n);
	int		DeleteSchedEntry(char *str);
	int		DeleteSchedEntry(RPSchedEntry *schedentry);
	int		ModifySchedEntry(char *str, int period, int offset);
	int		DeleteSchedEntryNum(int n);
	int		ModifySchedEntryNum(int  n, int period, int offset);
	int		FirstSchedString(char *str);
	int		NextSchedString(char *str);
	void		DumpSchedState(FILE *dumpfile = 0);
	void		DumpConnState(FILE *dumpfile = 0);

	RPCommReq	*GetReqEntry(char *str);
	int		DeleteReqEntry(char *str);
	int		FirstReqString(char *str);
	int		NextReqString(char *str);

// GetStnConnPnt will return when RPStnConnPnt >= desired stn
// MUST CHECK STN BEFORE USING THIS
	RPStnConnPnt*	GetStnConnPnt(int stn, bool match = TRUE);
	RPConnection*	GetFirstStnConn(int stn);
	void		AddConn(char *NewConnStr);
	void		AddConn(RPConnection *NewConn);
	int		GetFirstConnStn();
	int		GetNextConnStn();
	int		GetPrevConnStn();
	int		GetConnStnItem(int itemno);
	void		GetConnScanTypes(int stnid, SCAN_TYPE_ARRAY typearray);
	rxdevice	*ParseRxDeviceString(char *devstr, FILE *rpcomm_ini = 0);
	void		ParseTxDeviceString(char *txdevicestr, FILE *rpcomm_ini = 0);
	rxdevice	*NewRxDev(ConnModes ConnMd, char *port = 0);
	void 		UseRxDev(rxdevice *useRxDev);		// remove from free list, add to in use list
	void		FreeRxDev(rxdevice *freeRxDev);	// remove from in use list, add to free list
	txdevice	*NewTxDevice(ConnModes ConnMd, char *port = 0);
	int		TxDevicesListening();	// return number of txdevices in LISTEN mode

#ifndef NO_XWIN_GUI
	void		OpenCommMngWid();	// DO NOT FILL LIST UNTIL WIDGET CREATE CALLS BACK
	void		CloseCommMngWid();	// DO NOT FILL LIST UNTIL WIDGET CREATE CALLS BACK
	void		UpdateCommMngWid(bool force = false);	// typically called by widget create, or RxDev state change
	void		CommMngWidClosed();
	void		newOpenCommMngWid();	// DO NOT FILL LIST UNTIL WIDGET CREATE CALLS BACK
	void		newCloseCommMngWid();   // DO NOT FILL LIST UNTIL WIDGET CREATE CALLS BACK
	void		newUpdateCommMngWid(bool force = false);	// typically called by widget create, or RxDev state change
	void		newCommMngWidClosed();
	void		AcknowledgeAllAlerts();	// hide all alerts
	void		ShowAllAlerts();	// show all alerts

	void		OpenSchedEditWid();  
	void		CloseSchedEditWid(); 
	void		SchedEditWidClosed();

	void		OpenReqEditWid();  
	void		CloseReqEditWid(); 
	void		ReqEditWidClosed();
#endif

	void		OpenRxDevID(int id, int StatusOnly = 1);
	void		newOpenRxDevID(int id);
	void		newOpenTxDevID(int id);

// 3d-rapic radar server internet port
// If defined (>=0) it will be bound to the standard
// rp_srv internet port
// once defined it should not be re-bound
// default srv_port = RADAR_SERVER_DFLT
// default listen q = 1;
	void	    OpenRPSrvSocket(u_short srv_port = 0);
	void	    SetRPSrvListenQ(int listenq_size);
	void	    IncRPSrvListenQ();
	void	    DecRPSrvListenQ();
	int	    GetRPSrvListenQ();	    // return queue size
	void	    CheckRPSrvSocket();    // check for incoming calls when no listening devices available
	// RPSrv_fd provides an incoming internet socket binding point
	// Also starts a listen with ListenQ queue size
	int	    RPSrv_listenq;
	spinlock    *RPSrv_lock;
	sockaddr_in RPSrv_sin;
	bool        RPSrv_bind_failed;
	//	virtual void    PrintScanUsage(FILE *file = 0, bool verbose = false);	// print information on scans used, memory consumed etc.

	void         writeStatusToFile(char *fname = 0);
	void         writeStatusToFile(FILE *commstatfile);
	bool         state_changed;
	void         stateChanged() { state_changed=true; };
	int          txdevConnectRefusedCount;
	};

extern int	    RPSrv_fd;
extern u_short	    RPSrv_port;
extern RPCommMng    *CommMngr;

#endif /*__RPCOMMS_H */


