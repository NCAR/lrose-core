#ifndef __DB_H
#define __DB_H

#include <stdio.h>
#include <sys/types.h>
#include "rdr.h"
#include "rdrscan.h"
#include "siteinfo.h"
#include "ctstdr.h"
#include "ctoptn.h"
#include "ctaerr.h"
#include "ctdecl.h"
#include "cterrc.h"
#include "ctifil.h"
#include <X11/Intrinsic.h>

#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#else
#include <list.h>
#include <vector.h>
#endif

#define _SGI_MP_SOURCE

#ifdef __cplusplus
extern "C" {
#endif
extern char rapic_data_path[];
#ifdef __cplusplus
	}
#endif

/*
	imgrec is a structure for representing image/scan records in
	the CTREE ISAM databases
	The CTREE ISAM database is used to maintain an index into
	the actual image data which is stored in separate files.
	It is envisiged that images (ie sets of scans) will be stored,
	but individual scans are also supported.
*/

class rdr_seq;

enum idxtype {NOIDX,STN,DTTM};
enum DBSEQMODE {REALTIME, LOADTO, LOADFROM, LOADCENTRE};
enum rpisam_lockmode {RP_LOCK_READ,  RP_LOCK_WRITE};

// the following are in seconds
// the following default values may be overriden inn rpdb.ini by
// maxLoadRealTimeHours=24
// maxLoadRealTimeGap=2
// maxLoadDBGap=2
extern int maxLoadRealTimeSecs;  // limit the time depth to load
extern int maxLoadRealTimeGap;   // stop loading realtime sequence if the gap btwn imgs exceeds this
extern int maxLoadDBGap;         // stop loading database sequence if the gap btwn imgs exceeds this

class scan_rec {				// BYTES/POS
	COUNT			del_flag;	// 2/0		CTREE PLUS DELETE FLAG
public:
	short			stnid;		// lsb 8bits stnid, msb 8bits countryid	// 2/2		SCAN SET station id
	time_t			datetime;	// 4/4		primary date/time time_t format
	short			scan_type;	// 2/8		scan type PPI/RHI SCAN SET etc
	rdr_angle		setangle;	// 2/10		set angle of single scans or no. of scans in image
	short			data_type;	// 2/12		refl. vel. etc.
	short			data_fmt;	// 2/14		data format (Rapic 6lvl, 16lvl etc.)
	char			filename[16];	// 16/16	filename of radar data file
	// date/time of 1st img
	// ie yymmddhhmm
	// assumes path specified in rpdbpath
        // The associated isam files uses same name + .dat & .idx
	unsigned long		offset;		// 4/32		offset of this img within file
	int			size;		// 4/36		size of this image
	char			extrabytes[12];	// 12/40	leave rest for later use
public:
	scan_rec(rdr_scan *scan = 0);
/*
	void SetImgRec(rdr_img *img);			// put img values into scan_rec
	void GetImgRec(rdr_img *img);			// get img values from scan_rec
*/

	void SetScanRec(rdr_scan *scan);	// put scan values into scan_rec
	void GetScanRec(rdr_scan *scan);	// get scan values from scan_rec
	void PrintRec(FILE *file=stdout);	// a copy of record details
	void PrintRecShort(FILE *file=stdout, 
			   bool sizefirst = false);// print a short copy of record details
	void RecString(char *retstr);		// return record details string
	void StnDateTimeString(char *retstr);	// return date/time string
	void CopyRec(scan_rec *destrec);	// copy this rec to dest.
	e_scan_type scantype() { return e_scan_type(scan_type); };
	};

#define KEYSIZE sizeof(time_t)+(4*sizeof(short))+sizeof(rdr_angle)
typedef char rp_key[KEYSIZE];

/* 
	datafile_header is the first "record" of datafile.
	Anything appending to this file MUST modify filesize.
	This allows multiple processes to know where to append data.
	This record MUST be locked prior to modification.
*/
struct datafile_header {
	char			headerid[32];
	off_t			filesize;
	time_t		lastupdated;
	char			reserved[1024];
	};

struct rp_isam_state {
	rp_isam_state	*next, *prev;	// stack pointers
	COUNT		CurrentIdx;	// currently selected read index
	scan_rec	CurrentRec;	// current scan_rec
	rp_key		StnKey,DttmKey;
	rp_isam_state();
	~rp_isam_state();
	void		SetState(COUNT currentidx, 
				scan_rec *currentrec, 
				rp_key stnkey, 
				rp_key dttmkey);
	void		GetState(COUNT *currentidx, 
				scan_rec *currentrec, 
				rp_key stnkey, 
				rp_key dttmkey);
	};

class rp_copy_req
{
 public:
  string destPath;
  string destFName;
  bool   destDateFName;
  int    Stn;
  stnSet stn_set;
  e_scan_type scan_type;
  time_t start_time, end_time;
  int    max_dbsize;
  int    spacing;
  void   init();
  void   setByArgs(int argc, char **argv);
  rp_copy_req(string destpath, string destfname, int stn = 0, 
		    e_scan_type scantype = e_st_max, 
		    time_t starttime = 0, time_t endtime = 0, 
		    int maxdbsize = 0);
  rp_copy_req(int argc, char **argv);
  rp_copy_req();
};

#define BUILDNEWISAM TRUE
#define ADDISAMONLY TRUE

// The composite database system files and name
class  rp_isam {								
public:
	IFIL		rp_ifil;	// CTREE ISAM IFIL
	COUNT		dfil,stnidx,dttmidx;// CTREE ISAM file no.s
	COUNT		CurrentIdx;	// currently selected read index
	scan_rec	CurrentRec,	// current scan_rec
			temprec;	// temp rec variable
	rp_key		stnkey,dttmkey;
	stnSet          stn_set;        // set of stns with recs in this db
	rp_isam		*srcdb;		// pointer to current src db (dir services and copy/load)
	rp_isam		*destdb;	// dest db, used for copying
	bool		useProgressDialog;
	int             lastProgressPercent;            
	string          progressTitle;
	void            startProgress(char *progresstitle);
	// updateProgress returns true if ProgressDialogInterrupted()
	bool            updateProgress(int percent, 
				       time_t scantime,
				       int loadedcount,
				       long loadedsize);
	void            closeProgress();
	int		datafd;		// data file descr.
	off_t		datasize;	// data file size
	char		pname[256];	// database path
	char		fname[256];	// database short name
	char		fullname[512];	// database full name PATH + (yymmddhhmm)
	char		fullname_isam[512]; // isam full name PATH + (yymmddhhmm) + [.i386] if reqd
	char		label_from_to[256]; // database label, first to last rec
	bool		USELOCKS;	// if false will not use file locks
	int		dfilelocktmout;
	bool		ReadOnly;	// if true, read only mode database, no writes allowed, use READREC locks
	bool		showDBScansToScanMng;
	rdr_scan 	*tempscan;	// GP radar scan pointer
#ifndef NO_XWIN_GUI
        rdr_seq         *Viewer;        // scan viewer resource
#endif
	rp_isam_state	*StateStack, 	// db state stack, entries are reused 
			*StateStackTop;	// pointer to the top of stack
	time_t		first_tm, last_tm;  // times of first and last records
	rp_isam(bool rdonly = false);
	virtual ~rp_isam();
	virtual scan_rec* GetCurrentRec();
	virtual void setIDXMode(idxtype IdxType);
	virtual void addIDXSuffix(char *dbname);
	virtual void addDATSuffix(char *dbname);
	virtual bool Open(const char *pathnm = 0, const char *filenm = 0, 
			  bool RdOnly = false, bool AllowRdOnlyRebuild = false);
			// Open the db given by name
			// return TRUE if successful
	virtual bool OpenIsam(const char *pathnm = 0, const char *filenm = 0, bool RdOnly = false);	// Open ISAM given by name
	virtual bool isOpen() { return dfil != -1; };	// return true if is open
	virtual bool Create(const char *pathnm = 0, const char *filenm = 0);
			// Create the db given by name
			// return TRUE if successful
	virtual bool CreateIsam(const char *pathnm = 0, const char *filenm = 0); 
			// Create ISAM given by name

	virtual bool IsOpen();	// return state of db
	virtual	LONG	RecCount();	// count of recs in ISAM
	void		Close();
	int		lockdfile(short locktype = F_WRLCK);// lock/unlock data fl, <> 0 if fail
	int		unlockdfile();// lock/unlock data fl, <> 0 if fail
	int		lockdfileretry(int retrytenths = -1,  short locktype = F_WRLCK);//lock data file, <> 0 if fail
			// retrytenths = 10ths of secs to retry
	virtual int	lock_rp_isam(rpisam_lockmode lockmode = RP_LOCK_WRITE);	// lock rp_isam, <> 0 if fail
	virtual int	unlock_rp_isam();	// lock rp_isam, <> 0 if fail
	bool		WrtHeader();
	bool		RdHeader(datafile_header *header = 0);
	virtual		bool AddRec(scan_rec *Rec);
// add scan to database, if IsamOnly, don't add to data file
	virtual bool	AddScan1(rdr_scan *addscan, bool IsamOnly=FALSE,
				char *fn=0, int ofs=-1, int sz=-1);
	virtual bool	AddScan(rdr_scan *addscan);
	virtual bool	AddScanIsam(rdr_scan *addscan,
				char *fn, int ofs, int sz);
	virtual bool	GetScanSet(rdr_scan *getscan);	// get current scan set
	virtual bool	FirstRec(idxtype IdxType);	// set pos to first in sel ISAM - NO IDX TYPE USES CURRENT IDX, IDX TYPE SETS CURRENT IDX
	virtual bool	LastRec(idxtype IdxType);		// set pos to last in sel ISAM - NO IDX TYPE USES CURRENT IDX, IDX TYPE SETS CURRENT IDX
	virtual time_t	FirstRecTime();	// set pos to first in DTTM, return time 
	virtual time_t	LastRecTime();  // set pos to last in DTTM, return time 
	virtual void	getStnSet(stnSet* stnset = NULL);  // get set of stns in this db, return pointer to stn_set
	virtual bool	NextStn();	// next stn in STN ISAM - FAILS IF CURRENT IDX NOT STN
	virtual bool	PrevStn();	// prev stn in STN ISAM - FAILS IF CURRENT IDX NOT DTTM
	virtual bool	NextDay();	// next Day in DTTM ISAM (ie next DAY)- USES CURRENT IDX
	virtual bool	PrevDay();	// prev Day in DTTM ISAM (ie prev DAY)- USES CURRENT IDX
	virtual bool	NextRec(bool quiet = false);	// step to next rec - USES CURRENT IDX
	virtual bool	PrevRec(bool quiet = false);	// step to prev rec - USES CURRENT IDX
	virtual bool	NextScan(rdr_scan *nextscan);	
			// step to next scan (skips scans) - USES CURRENT IDX
	virtual bool	PrevScan(rdr_scan *prevscan);	
			// step to prev scan (skips scans) - USES CURRENT IDX
	virtual bool	SearchDtTm(time_t tm, bool quiet = false);	
			// search for GTE than tm SETS DTTM AS CURRENT IDX
	virtual bool	SearchStn(short stn, time_t tm = -1);
			// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual bool	FirstStnRec(short stn);
			// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual bool	LastStnRec(short stn);// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual void	RecAtPercent(COUNT percent, scan_rec *retrec, idxtype IdxType);
	virtual void	Dump100Percent(); // diagnostic, print 100 recs
	virtual void	MakeScanStnKey(rdr_scan *keyscan);
	virtual void	MakeScanDttmKey(rdr_scan *keyscan);
	virtual void	MakeStnKey(short StnId, time_t tm);
	virtual void	MakeDttmKey(time_t dttm);
	virtual void    MakeRecStnKey(scan_rec *keyrec);
	virtual void    MakeRecDttmKey(scan_rec *keyrec);
	virtual bool	RecPresent(scan_rec *presentrec);
	virtual bool	ScanPresent(rdr_scan *presentscan);
	// ReadImg can read ascii rapic data from any file into a rdr_img
	// Returns - -1=EOF, 0=no image, 1=image read OK
	virtual int	ReadScanSet(int fd, unsigned long &offset, 
				    unsigned long &endoffset,
	    int &size, rdr_scan *readscan, int raw);
	virtual bool	Rebuild(bool ForceNewISAM = FALSE);	// generate new ISAM from data file
	virtual void	ReadFile(int fd, char *fn, bool AddIsamOnly = FALSE);
			// read radar data file into this
// typically use AddIsamOnly to rebuild from data file

#ifndef NO_XWIN_GUI
	virtual void	PreviewScan(rdr_seq *viewer = 0, bool flag = FALSE);	
			// use rdrseq to display current. FALSE to clear view mode
#endif
	virtual bool	OpenSrcDB(char *srcname, 
				  bool RdOnly = false, bool AllowRdOnlyRebuild = false);
	virtual void	CloseSrcDB();
	virtual bool	OpenDestDB(char *destname);
	virtual bool	CreateDestDB(char *destname);
	virtual void	CloseDestDB();
	
	// if copy completed, return true
	// if maxdbsize specified and destdb size excceds it, return false (copy not complete)
	// copyrecs can then be called with resumecopy set to continue where left off
	virtual bool 	CopyRecs(long &recscopied, int stn  = 0, time_t tm1 = 0, time_t tm2 = 0, 
				 int spacing = 1, e_scan_type scantype = e_st_max,
				 long long maxdbsize = 0, bool resumecopy = false, bool quiet = false);
	virtual bool 	CopyRecs(long &recscopied, stnSet &copystns, time_t tm1 = 0, time_t tm2 = 0, 
				 int spacing = 1,  e_scan_type scantype = e_st_max,
				 long long maxdbsize = 0, bool resumecopy = false, bool quiet = false);
	virtual bool 	CopyRecs(long &recscopied, rp_copy_req& copy_req, bool resumecopy = false, bool quiet = false);	
	virtual void 	dumpRecStats(long &recsmatching, rp_copy_req& copy_req, FILE *dumpfile, 
				     bool sizefirst = false);
	virtual bool    CopyCurrent(rp_isam *srcisam, bool quiet = false);	// copy current rec from srcisam

	/*
	 * The following load fns load a sequence to the rdr_seq class
	 */
#define NULL_CLIENT (scan_client*) 0
	virtual void	passScanToClient(rdr_scan *newscan, 
			    scan_client *ScanClient, 
			    scan_client  *ScanClient2 = NULL_CLIENT);
	virtual void	LoadSeq(scan_client *ScanClient, DBSEQMODE mode = REALTIME, int num = -1, int stn = 0, time_t tm = 0, int spacing = 1);				
	virtual void	LoadTo(scan_client *ScanClient, bool ReloadRealtime, int num = -1,
			       int stn = 0, time_t tm = 0, int spacing = 1, 
			       scan_client  *ScanClient2 = NULL_CLIENT);	
	virtual void	LoadFrom(scan_client *ScanClient, int num = -1, 
				int stn = 0, time_t tm = 0, int spacing = 1,
				scan_client *ScanClient2 = NULL_CLIENT );
	virtual void	LoadCentre(scan_client *ScanClient, int num,
				   int stn = 0, time_t tm = 0, int spacing = 1,
				   scan_client *ScanClient2 = NULL_CLIENT);

	virtual int	GetCurrentRecFd();	// return fd corresponding to CurrentRec
	virtual int	AppendData(int srcfd, unsigned long &destoffs, unsigned int size); 
			// append size bytes of data from srcfd at ofs to datafd
	virtual void	SetDBReadOnlyMode(bool SetDBReadOnly);	
	virtual bool	GetDBReadOnlyMode();	
	virtual void	PushState();	// save current db state
	virtual void	PopState();	// restore last pushed db state)
	virtual char	*LabelFromTo() {return label_from_to; };	
	};

#define rpdb_files "rpdb_files"
// defines names of current working and archive files
/* text file - format
working=fname
archive=fname
*/
// if not present or empty or file errors, create new db's

extern char rpdbinifile1[];
extern char rpdbinifile2[];
// defines path for database system. Allows remote database
/* text file - format
pathname=fname		#optional field. May set remote fileserver. Current path if null or not present
*/

#define MrgIsamName "mrgisam"

class rp_db : public rp_isam {
public:
	rp_db(char *pathname = NULL, bool rdonly = false, bool allowdbpurge = true);
	virtual ~rp_db();
	void		OpenDB(char *pathname = NULL, bool rdonly = false, bool allowdbpurge = true); 
			// check for current db's
			// and open them if possible
	void 		Close();
	rp_db		*next,*prev;	// pointers for linked list of db's
	rp_isam		*workdb;    	// working radar data base
	rp_isam		*archdb;	// archive radar data base
	int		maxdatasz;	// max radar data file size before switch
	bool		DeleteOldDatabases;
	int		DBPurgeDepth;
	char		ArchiveDevice[256];
	char		SwitchDBFlagName[256];
	int		mrgDupCount;
	// before call (Could also include stnid)
	virtual scan_rec* GetCurrentRec();
	// add img to working databases, if max sz, cause switch
	// if destdb defined and != workdb, ignore size/switch	
/*
	virtual bool	AddImgDest(rdr_img *addimg,rp_isam *destdb = 0);	
	virtual bool	AddImg(rdr_img *addimg);	
	virtual bool	GetImg(rdr_img *getimg);	// read current image from db
*/
	virtual bool	AddScanDest(rdr_scan *addscan,rp_isam *destdb = 0);	
	virtual bool	AddScan(rdr_scan *addscan);	// add scan to working databases, if max sz, cause switch
	virtual bool	GetScanSet(rdr_scan *getscan);	// read current scan set from db
	virtual bool	FNameMatch(char *fname1, char *fname2);
	virtual void	MakeDBName(rdr_scan *scan, char *name);
	virtual void	MakeDBName(scan_rec *rec, char *name);
	virtual void 	SaveDBNames();			// write new "rpdb_files" file
	virtual bool	SwitchDBFlag();
	virtual void	SwitchDB(rdr_scan *scan);	// force switch of isams/files, ie work to arch, new work
	virtual void	SwitchDB(scan_rec *rec);	// force switch of isams/files, ie work to arch, new work
  			// Before any working data is added, archisam copied to bothisam
	bool		ArchWritten;				// flag to warn of archive write failure!!
	virtual void 	FlagArchSave(char *name);
	virtual void	WriteArch(char *archdev);	// write the archive files to the archdev

	// if "count">0 purge "count" oldest databases
	// else if DBPurgeDepth > -1 purge databases back to depth
	virtual void	PurgeOldDBs(int count = -1);	
	virtual void	DeleteDB(char *delname, char *pname = 0);// delete database
	// usually use mt to go to end of data on tape and tar to write files
  // MANAGEMENT WILL REQUIRE SOME THOUGHT. IE TAPE SPACE STRATEGIES. WARN OF IMPENDING FILLING

/*  Could rebuild both working and arch here, but may not be necessary
	virtual void	ReadFile(int fd, char *fn, bool AddIsamOnly = FALSE);
			// read radar data file into this
*/
// typically use AddIsamOnly to rebuild from data file
	virtual bool	CreateMrgIsam();	// regenerate fresh merged working/archive isam
	virtual bool	RepairMrgIsam();	// repair component databases and regenerate fresh merged working/archive isam
	virtual bool	FirstRec(idxtype IdxType);	// set pos to first in sel ISAM - NO IDX TYPE USES CURRENT IDX, IDX TYPE SETS CURRENT IDX
	virtual bool	LastRec(idxtype IdxType);		// set pos to last in sel ISAM - NO IDX TYPE USES CURRENT IDX, IDX TYPE SETS CURRENT IDX
	virtual bool	NextStn();	// next stn in STN ISAM - FAILS IF CURRENT IDX NOT STN
	virtual bool	PrevStn();	// prev stn in STN ISAM - FAILS IF CURRENT IDX NOT DTTM
	virtual bool	NextDay();	// next Day in DTTM ISAM (ie next DAY)- USES CURRENT IDX
	virtual bool	PrevDay();	// prev Day in DTTM ISAM (ie prev DAY)- USES CURRENT IDX
	virtual bool	NextRec(bool quiet = false);	// step to next rec - USES CURRENT IDX
	virtual bool	PrevRec(bool quiet = false);	// step to prev rec - USES CURRENT IDX
	virtual bool	NextScan(rdr_scan *nextscan);	// step to next img (skips scans) - USES CURRENT IDX
	virtual bool	PrevScan(rdr_scan *prevscan);	// step to prev img (skips scans) - USES CURRENT IDX
	virtual bool	SearchDtTm(time_t tm, bool quiet = false);	// search for GTE than tm SETS DTTM AS CURRENT IDX
	virtual bool	SearchStn(short stn, time_t tm = -1);// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual bool	FirstStnRec(short stn);// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual bool	LastStnRec(short stn);// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	/*
	 * The folowing load fns return a scan as specified
	 */
		
	virtual rdr_scan* LoadStn(short stn, time_t tm = -1);// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
	virtual rdr_scan* LoadStnNext(short stn);	// search for GTE than stn (date OPTIONAL - DFLT = -1) SET STN AS CURRENT IDX
#ifndef NO_XWIN_GUI
	virtual void	PreviewScan(rdr_seq *viewer = 0, bool flag = FALSE);	// use rdrseq to display 
#endif
	// Using rp_isam CopyRecs -	virtual void 	CopyRecs(int stn  = 0, time_t tm1 = 0, time_t tm2 = 0, int spacing = 1);
	virtual bool    CopyCurrent(rp_isam *srcisam);		// copy current rec from srcisam
	virtual int	GetCurrentRecFd();	// return fd corresponding to CurrentRec
	virtual bool	RecPresent(scan_rec *presentrec);
	virtual bool	ScanPresent(rdr_scan *presentscan);
 	virtual void	SetDBReadOnlyMode(bool SetDBReadOnly);	
 	virtual bool	GetDBReadOnlyMode();	
	virtual void	PushState();	// save current db state
	virtual void	PopState();	// restore last pushed db state)
};

class RapicFileEntry {
public:
	char		FilePath[256];
	char            DatePath[64];            
	char		FileSuffix[64];
	int		FileStn;	// 0=no vol file output, > 0=specified stn, -1=all stns
	bool		FileNameUseStnID;
	bool		FileNameUseScanType;
	bool            useDatePathHierarchy;  // if true construct date based path hierarchy, .../2002/07/23/ 
	bool            useStnDatePathHierarchy;  // if true construct date based path hierarchy, .../2002/07/23/ 
	bool		VolumeOnly;
	bool		CompScanOnly;
	bool            writeReflOnly;  // suppress non-refl output
	bool		verbose;
	bool		CreateLatestDataInfo;
	bool		CreateFileList;
	mode_t		permissions;
	uid_t		owner;
	gid_t		group;
	bool		scanMatch(rdr_scan *addscan);
	bool		VolFileName(char *namestr, rdr_scan *scan, bool nopath = false, mode_t perms = 0755);
	RapicFileEntry(char *initstr = 0);
	void		init(char *initstr = 0);
	void		CreateLatestDataInfoFile(rdr_scan *addscan);
	void		UpdateFileList(rdr_scan *addscan);
	void		WriteRapicFile(rdr_scan *addscan = 0);
	char            *datePath(rdr_scan *addscan);
	char            *stnDatePath(rdr_scan *addscan);
};

/* 
   basic database information for rpdb_cache class, start/end times and available stns
   There can be thousands of these, so don't generally keep an rp_isam reference with each one.
*/

class rpdb_cachedata
{
 public:
  time_t start_time, end_time;  // times of first and last records
  string db_name;
  stnSet stn_set;
  long rec_count;
  long long datasize;
  void getCacheData(string dbname, rp_isam *rpdb);
  void dumpData(FILE *file = NULL, bool printStnNames = false);
  rpdb_cachedata();
  rpdb_cachedata(string dbname, rp_isam *rpdb);
  rpdb_cachedata(string dbname, time_t starttime, time_t endtime, stnSet stnset);
};

class rpdb_cache
{
  multimap<time_t, string> startTimes;  // keep map of starttime vs db name for time period searches
  multimap<time_t, string> endTimes;    // keep map of endtime vs db name for time period searches
  map <string, rpdb_cachedata> cachedData;
  string path_name;
  bool allowRebuild;
 public:
  time_t cacheStartTime, cacheEndTime;
  long totalRecs; // sum of records of all referred dbs
  long long totalDBSize;
  stnSet stn_set;
  void copyDB(string dbprefix, int stn = 0, e_scan_type scantype = e_st_max, 
	      time_t starttime = 0, time_t endtime = 0, int maxdbsixe_mb = 0);
  void copyDB(rp_copy_req& copyreq);
  void dumpStats(rp_copy_req& copyreq, char *dump_fname);
  void addToCache(string pathname, string dbname, rp_isam *rpisam);
  void loadDbList(string pathname, string listname);   // load databases in list file
  void dumpCacheContents(char *filenm = NULL);
  void close();
  void open(string pathname, string listname, bool allowrebuild = false);
  void open(char *initstr);  // initialise with 
  bool open_rp_isam(rp_isam *rpisam, 
		    const char *pathname, 
		    const char *dbname);
  rpdb_cache(string pathname, string listname, bool allowrebuild = false);
  rpdb_cache(char *initstr);  // initialise with 
  ~rpdb_cache();
};

/*
	Class to allow receiving system to write to multiple databases
	FirstDB will always be the Read/Write database for 3D-Rapic use
*/

class DBMng : public scan_client {
	bool DBReadOnly;		// passed by ScanMng. Check for completion
public:
	rp_db   *FirstDB;
	rp_db	*LastDB;
	rp_db   *InitLoadDB;
	time_t	DBCheckTime;
	time_t	DBCheckPeriod;
	rdr_scan_node *IncompleteScans;	// linked list of new incomplete scans
					// add to db when complete, and remove 
					// from this list
	int		maxdatasz;	// max radar data file size before switch
	char		ArchiveDevice[256];
	bool		filterBadDate;	// if true filter real-time data with date/time values outside window (nominally 6 hours)
	bool		noRapicDB;	// if true, don't create rapic dbs
	time_t		badDateTimeWindow;
	vector<RapicFileEntry> rapicFileEntries;
	rpdb_cache      *rpdbCache;
	void            openRpdbCache(char *cacheinitstr);
	void            openRpdbCache(string cachepath, string cachelist, bool allowrebuild = false);           
	void		CheckRapicFileWrite(rdr_scan *addscan = 0);
	void		AddDB(rp_db *newdb);
	DBMng(bool allowdbpurge = true,
	      char *inifilename = NULL);
	~DBMng();
//	bool		AddImg(rdr_img *addimg);	// add img to all databases
	
	bool		AddScan(rdr_scan *addscan);	// add scan to all databases
	virtual int	NewDataAvail(rdr_scan *newscan);	// new scan passed by ScanMng
	bool	ScanSourceOK(rdr_scan *srcscan);
	virtual bool Full(int passedSize = -1);
	virtual bool MergeScans();	// true if scan client is merging scans flag
	virtual bool IsDuplicate(rdr_scan *dupscan, 	// return true if known ie already in seq or database etc
	    bool FinishedOnly = FALSE);	// return true if known ie already in seq or database etc
	bool		CheckScans();		// check scans on incomplete list, add completed scans
	virtual void	SetDBReadOnlyMode(bool SetDBReadOnly);	
	virtual bool	GetDBReadOnlyMode();	
	virtual void	PrintScanUsage(FILE *file, bool verbose = false);
	// use InitLoadDB if declared, otherwise use normal working db
	virtual void    loadInitialSeq(scan_client *ScanClient, int num = -1, bool closeAfterLoad = false);
	virtual void    closeInitLoadDB();
	};

extern DBMng *DBMngr;
extern Widget DBOpsW;
extern rpdb_cache *rpdbCache;

#endif // __DB_H
