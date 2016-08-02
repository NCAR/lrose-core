#ifndef _CRDRSCAN_H_
#define _CRDRSCAN_H_

#include "CData.h"
#include "rdr.h"

#ifndef STDNODES
#include "CDataNodeTemplate.h"
#endif

#ifdef WIN32
#include <sys\types.h>
//#define fabsf fabs
#include "io.h"
#endif

#ifdef RAPICTXMODS
void rpdate2ymd(int rpdate, int &year, int &month, int &day);
#endif

#ifdef USE_DEFRAG_FN_ORIDE
#include "defrag.h"
#endif

/*
	PPI - single PPI scan
	RHI - single RHI scan	
	CompPPI - single composite PPI scan
	IMAGE - Multiple radar/scan type image.
	VOL - closely (time) related scan set.
*/
typedef char *strarr[];
enum	e_scan_type {eMatchAllScanTypes=-1, PPI, RHI, CompPPI,IMAGE,VOL,RHISet,MERGE, SCANERROR, SectorScan, PointAerial};
extern char *scan_type_text[];
// the IMAGE type is used by the database to identify scans and images
/*
 * Following utilities can return -1 if "-1" or "ANY" string passed or 
 * no match 
 * It is up to the user of this utility to handle -1  
 */
e_scan_type decode_scantypestr(char *scanstr);  // decodes either numeric or text
e_scan_type get_scan_type(char *scanstr);
char *get_scan_type_text(e_scan_type scantype);
bool	IsValidScanType(e_scan_type type);

enum	e_data_type {eMatchAllDataTypes=-1, e_refl, e_vel, e_spectw, e_diffz, e_rawrefl, e_rainaccum, e_multimoment, e_undefined};
int	Enum2BF(e_data_type);

//enum	bf_data_type {/*bf_none=0, */bf_refl=1, bf_vel=2, bf_spectw=4, bf_diffz=8, bf_rawrefl=16, bf_rainaccum=32};
union BFmoments {
  USHORT BitField;
	struct {
		USHORT Refl : 1;
		USHORT Vel : 1;
		USHORT Spectw : 1;
		USHORT DiffZ : 1;
		USHORT RawRefl : 1;
		USHORT RainAccum : 1;
	};
	BFmoments() { 
		BitField = 0; 
	};
	bool HasMoment(e_data_type testtype) {
		if(Enum2BF(testtype) & BitField) return true;
		else return false;
	};
	void Flush() {
		BitField = 0;
	};
	void AddMoment(e_data_type newtype) {
		BitField |= Enum2BF(newtype);
	};
};

e_data_type NextMoment(e_data_type);

extern char *data_type_text[];
e_data_type get_data_type(char *dtastr);
e_data_type decode_datatypestr(char *dtastr);
char *get_data_type_text(e_data_type datatype);
bool	IsValidDataType(e_data_type type);

enum	e_data_fmt {eMatchAllDataFormats=-1, RLE_6L_ASC,RLE_16L_ASC, RAW8BIT, THRESHOLD_6,	THRESHOLD_16};
extern char *data_fmt_text[];
e_data_fmt get_format_type(char *fmtstr);
e_data_fmt decode_formatstr(char *fmtstr); // decodes either numeric or text
char *get_data_fmt_text(e_data_fmt datafmt);
bool	IsValidDataFormat(e_data_fmt format);

enum	enumUnfold	{ eUnfoldNone, eUnfold2to3, eUnfold3to4, eUnfold4to5 };

/*
 * COMM - scan from perm connection,  ie NOT through comm req
 * DB - scan from database
 * COMMREQ - scan from comm req
 */
enum	e_data_source {eMatchAllDataSources=-1, COMM, DB, COMMREQ, REAL_TIME}; // source class of data
extern char *data_src_text[];
e_data_source get_data_src(char *srcstr);
e_data_source decode_datasrcstr(char *srcstr); // decodes either numeric or text
char *get_data_src_text(e_data_source datasrc);
bool	IsValidDataSource(e_data_source source);

enum	data_mode {INDEX, FLOATVALUE, BYTEVALUE};	// s_radl data type

enum	e_unpackedradltype {SIMPLE, MULTIMOMENT, ASCIIDATA};

enum	SCANCLIENTTYPE {SC_UNDEFINED, SC_SEQ, SC_DB, SC_TXDEV, SC_COMMMNG, SC_CACHEDEV};

enum	SCANCLIENTCACHE {SC_NOCACHE, SC_DBCACHE, SC_SECONDARYCACHE, SC_PRIMARYCACHE };

enum  e_ExtensionType { INGEST_EXTENSION, OTHER_EXTENSION}; // rdr_scan extension type ID's

/*#include "rdlcomp.h"
#include "mmoment.h"*/


/*struct	s_comp_params {
  short	scan_count;	    // number of scans in composite ppi
  rdr_angle 	elev_10th_arr[10];  // elev of scans (ELEV in 10ths of deg)
  short	rng_arr[10];	    // elev switch ranges (km)	
	void operator=(s_comp_params &a);
};*/

struct s_volume_params {
	short volume_count;
	rdr_angle *angle;					// angles for each individual scan i.e. El for PPI/volumetric, Az for Batch RHI's
	short			*range;
	rdr_angle	start;					// bottom rhi angle - start sector scan angle
	rdr_angle stop;						// top rhi angle - stop sector scan angle
	s_volume_params(int count);
	s_volume_params();
	~s_volume_params();
	void add_tilt(rdr_angle angle, short range);
	void operator=(s_volume_params &a);
#define DEFRAGDECLARATION
#include "defrag.h"
};

/*struct s_rhi_params {
	rdr_angle	top;					// top rhi angle
	rdr_angle bottom;				// bottom rhi angle
	short	range;						// range in km
	char	direction;				// TRUE for UP, FALSE for down
	void operator=(s_rhi_params &a);
};*/

struct 	s_rdrparams {
    char	name[17];	
    short	wavelength;	// 10ths of centimetres - millimeters!!!!!
    rdr_angle	az_beamwidth,
		el_beamwidth;	// 10ths of degrees
    short	spare[32];
	void operator=(s_rdrparams &a);
};


/// NOTE RadlComponent is defined in mmoment.h
/*struct RadlComponent {	    // template to drop over MULTIMOMENT radial storage
    e_data_type  moment;	    // e_data_type moment
    short	length;	    // length of this moment component, including this header
    short	nextofs;    // offset of next moment (0 for last), relative to start of concat buffer
    char	data[1];    // data array
    RadlComponent *NextMoment(); // drop over next moment, return null if no more
    };*/

struct UnpackedRadl {	    // simple(refl only) or multi-moment radial(multiple moments concatenated using RadlMoment struct)
    int		start_ofs;  // -1 signifies no radial
    short	radl_length;
    };

struct LevelTable : public CLinkedList<LevelTable> {
	e_data_type			m_moment;			// moment this table represents
//	LevelTable		 *next, *prev;	// linked list of tables
	short						m_NumLevels;
	float					 *m_pfLevels;		// Level in signed floating point units
	unsigned char	 *m_pXlatTable;	// translation table to allow faster level lookup
public:
	//constuctors
	LevelTable(short NUMLEVELS, float *InitTbl);
	LevelTable(short NUMLEVELS, int *InitTbl);
	~LevelTable();
	// implementation
#define DEFRAGDECLARATION
#include "defrag.h"
	void SetThresholds(int nNum, int* pLevels);
	void SetThresholds(int nNum, float* pLevels);
	void ThresholdData(unsigned char *pIPbuffer, unsigned char *pOPbuffer, int length);
	void ThresholdData(short *pIPbuffer, unsigned char *pOPbuffer, int length);
	void ThresholdData(float *pIPbuffer, unsigned char *pOPbuffer, int length);
private:
	void BuildXLatTable();
};

const	int		RADLBUFFSIZE = 1024;
struct	s_radl {		// unpacked radial array
    rdr_angle	az,az1,az2;	// 10ths of deg, az - centre, az1/az2 width to draw
    rdr_angle	el,el1,el2;	// 10ths of deg, el - centre, el1/el2 height to draw
    int		startrng;	// metres, range of 1st data, normally 4000
    int		rngres;		// metres, rng res
    int     	data_size;	// max used size of data in buffer
    int		buffsize;	// buffer size
    int		undefinedrng;	// rng to which data is undefined, primarily for CAPPI
    data_mode	mode;		// INDEX or VALUE mode
    e_data_type data_type;	// simple moment type of data
//    bf_data_type bfdata_type;	// multimoment type of data
		BFmoments BFdata_type;
    e_unpackedradltype radltype;// data storage method   
		/***** NOTE DATA TYPE MUST BE UNSIGNED TO ALLOW CORRECT COMPARISM OF ABSOLUTE 8 bit RAW DATA ***/
    unsigned char    	*data;    	// uncompressed radial data (0.5dBZ units of refl)
    LevelTable	*LvlTbl;	// pointer to corresponding level table (0.5dBZ units of refl)
    float	*Values;	// optional value array (Actual (not 0.5units) dBZ Values)
    s_radl(int BuffSize = 0);
    ~s_radl();
    void 	Clear();	// zero out this radial
    int		IndexAtRange(float rng, float *idxrng = 0);// return the array index at the given km rng
    unsigned char	DataAtRange(float rng);	    // return the data index at the given km rng
//    float	ValueAtRange(float rng);    // return the Value at the given km rng
    void	TanPlaneRadl(float *cosel = 0);	// convert to tangent plane radial (0deg)
    int		PadRadl(int padrng); // pad radial to given range
    // simply threshold float *floatarray values into unsigned char *data array
    // other parameters of s_radl need to be set, e.g. rng res etc. 
    void	ThresholdData(LevelTable *thresh_table = 0);
    void	ThresholdData(float *array, int size, LevelTable *thresh_table = 0);
    void	ThresholdData(unsigned char *array, int size, LevelTable *thresh_table = 0);
    // convert 16 level char *data array to ASCII_16 radial
    void	Encode16lvlAz(char *outstring);
    void	Encode16lvlEl(char *outstring);
    void	Encode6lvlAz(char *outstring);
    void	Encode6lvlEl(char *outstring);
    void	RngRes2000to1000(); // convert 200m res radial to 100m res
    void	TruncateData();	    // set data_size to last nn-zero data
#define DEFRAGDECLARATION
#include "defrag.h"
};

struct 	s_radl_node {				// radial node for linked lists
		s_radl_node() {
			if(!DeFragList.IDString[0])
				strcpy(DeFragList.IDString, "s_radl_node");
		}
    s_radl	RadlData;
    s_radl_node	*Next,*Prev;
#define DEFRAGDECLARATION
#include "defrag.h"
};

class radl_pnt : public CLinkedList<radl_pnt> {
public:
//    radl_pnt	*next,*prev;	// for linked list in free list
    e_unpackedradltype	radltype;// data storage method
    int		numradials;
    int		*PntTbl;	// table of radial start offsets
    void 	*FreeList;
    radl_pnt(int NumRadials, void *freelist = 0);
    // if FreeList undefine, try to use global FreeListMng->RadlPntFreeList
    ~radl_pnt();
    };

class radl_pnt_freelist {
    radl_pnt	*FreeList;
public:
    radl_pnt	*GetRadlPnt(int NumRadials);
    void	StoreRadlPnt(radl_pnt *StorePnt);
    radl_pnt_freelist();
    ~radl_pnt_freelist();
    };

	/*
	rdr_scan is passed a buffer and start offset of its data
	The constructor traverses the data until EOBuff or an end
	of image is encountered. The data passed must therefore be
	a complete terminated radar scan.
	NOTE: RELIES ON CR TERMINATED LINES. # TERMINATED LINES SHOULD BE
	FILTERED RDR_IMG BEFORE ADDING DATA TO BUFFER
	Support is provided for sets of closely related scans (e.g. Volumes)
	Sets currently supported are VOL (set of PPI scans) and RHISet.
	It is suggested that the reset_scan and next_scan methods be used
	to access scans. In the single scan (PPI & RHI case, reset_scan
	returns this, and next_scan returns NULL.
	Linked lists of scans may be traversed by reset_scan, next_scan until
	NUL returned, then using _next->reset_scan
	Only the root rdr_scan will have an open exp_buff.
	Child scans will reference root's exp_buff throught their own	
	exp_buff_rd 
	New data can ONLY BE ADDED THROUGH THE ROOT SCAN
*/

const	int	LBMAX=1024;	// max line_buff size
struct rdr_scan_linebuff {
	int	new_scan_ofs;	// start of next scan
	int	lb_size;		// current size
	char	line_buff[LBMAX];// line of data buff
	};


class CDataRdr : public CData {
protected:
  exp_buff*		pDataBuff;				// data buffer
	CDataRdr		*pLastScan;					// last scan in this set 
  int					thisscanno;					// pos. of thisscan in set (0 = root)
  time_t 			LastScanTm;					// time of last scan, clients may use to detect scan set change
  int					faultno;
  char				setlabel[32];				// date/time stamp for this scan set.
	char				ScanStr[128];				// short description of this scan
  radl_pnt		*pRadlPointers;	    // radial start pointers
  bool				bUseRadlPnt;
  radl_pnt		*pXRefRadlNo;	      // cross reference radial number to actual angle
	short     	num_radls;						// number of non-zero radials
	short				num_scans;						// count of scans in this scan set
private:
	void					IncNumRadls();
protected:
	void				SetRootScan(CDataRdr* root);
	CDataRdr*		GetThisScan();    // access parent class ThisSet and typecast to a CDataRdr
	void				set_dflt();
	void				SetRadlPnt();	    // set the radl pointers for this scan
//  CDataRdr* 	reset_scan();	    // set thisscan to first in list (thiscan for single scan e.g. PPI RHI)
//  CDataRdr* 	this_scan();	    // return pThisScan if complete
//  CDataRdr* 	next_scan();	    // return next scan in set. Sets thisscan. Will always set thisscan NULL for single scans (PPI & RHI)
//  CDataRdr* 	prev_scan();	    // return next scan in set. Sets thisscan. Will always set thisscan NULL for single scans (PPI & RHI)
	virtual int		write_scan(int fd);
  virtual int		append_scan(int fd,off_t *startofs,int *size);  // append img to given file
	virtual void	append_scan(CDataRdr*); // needed to allow subclasses to append scans
	virtual void  DelLock();
public:
  e_data_source data_source;				// data source class - COMM,DB
  int					station;							// source radar station, lsbyte=stnid, msbyte=countryid
  int					year,month,day,hour,min,sec;	// date/time of scan
  time_t			scan_time_t;					// unix time_t date/time of scan capture
	s_rdrparams	rdr_params;						// radar parameters
	e_scan_type	scan_type;						// type of scan(PPI,RHI,CompPPI,VOL,RHISet)
	e_data_type data_type;			// e_refl. e_vel. etc.
//	bf_data_type    bfdata_type;			// multi-moment bitfield
	BFmoments		BFdata_type;
	e_unpackedradltype radltype;			// method of storage in radials
	e_data_fmt	data_fmt;							// data format (Rapic 6lvl, 16lvl etc.)
	rdr_angle   angle_res;						// radial width(10ths 0f deg)
	float				nyquist;							// vel scans store nyquist value
	short				rng_res;							// metres
	short				start_rng;						// metres start of radial data
	float				max_rng;							// km max range
	int					data_start;						// start position of this scan in data buffer
	short				vol_scans,vol_scan_no;	// scans in vol./this scan no.
	rdr_angle		set_angle;						// constant angle(el for PPI)

  bool				bShownToScanClients;	// true if scan manager has shown this scan to clients
	s_volume_params *pVolumeParams;		// scan volume parameters
  LevelTable	*pLvlTbl;	// Level threshold table, only present in root scan
public:
	// constructors
	CDataRdr(CDataRdr* rootscan = NULL);
						// rootscan is only defined when the RootScan is generating new child scans
						// DATA CAN ONLY BE ADDED THROUGH THE ROOT SCAN
	virtual		~CDataRdr();							// note virtualised destructor to allow correct destruction from base class
public:
	// public methods
#define DEFRAGDECLARATION
#include "defrag.h"
	virtual bool	Lock();								// protect data accesses if concurrently reading
	virtual void  Unlock();
	virtual void OpenReader(exp_buff_rd* pdbuff_rd = NULL); // open rootscan's data buffer
	virtual	void	ResetReader(exp_buff_rd* pdbuff_rd = NULL); // was reset_radl
//  CDataRdr*		goto_scan(int n);	    // return scan "n" in set. Sets thisscan. Will always set thisscan NULL for n > num_scans
//	CDataRdr*		RootScan();						// return pointer to root scan
//	CDataRdr*		NextInSet();					// return pointer to next in set
	CDataRdr*		NextScan(CDataRdr* scan = NULL);	// return pointer to next after given scan
//	CDataRdr*		LastScan();						// return pointer to last in set
  virtual void	add_data(char* inbuff, int dcount, e_unpackedradltype type = SIMPLE);	// add new data to data buffer
  virtual void	add_data(s_radl *inradl); // add new data to data buffer via a s_radl typ result of CAPPI generation
  virtual void	SetDataFinished(bool state = TRUE);	// SET STATUS no more data for this scan
	virtual void	SetDataValid(bool state = TRUE);
	virtual void  SetFaultNo(int fault);
	virtual int		GetFaultNo();
	virtual	bool	IsFaulty();							// true if faulty or complete and not valid
	bool					ShownToScanClients();
	virtual void	SetCurrentTime();		// set time parameters to current time (Rapic Tx typ.)
  virtual int		get_next_radl(s_radl *NextRadl, e_data_type datatype=e_refl, exp_buff_rd	*dbuff_rd = NULL);    // get and unpack next defined radial
  virtual int		get_radl_angl(s_radl *Radl, rdr_angle Angl, e_data_type datatype=e_refl, exp_buff_rd	*dbuff_rd = NULL);// returns -1 if failed
    // get and unpack radial at given angle.
  virtual int		get_radl_no(s_radl *radl, int radlno, e_data_type datatype=e_refl, exp_buff_rd	*dbuff_rd = NULL);// get radial n, returns -1 if failed, 
    /* e.g. if angle_res = 0.5(5) radial 10 = radial at 5degrees */
  virtual unsigned char	get_data_angl_rng(rdr_angle Angl, float rng, e_data_type datatype=e_refl, exp_buff_rd	*dbuff_rd = NULL);
    
  virtual int		write_scan_set(int fd,off_t *startofs,int *size);	// write scan set to given file at current pos
  virtual int		append_scan_set(int fd,off_t *startofs,int *size);	// append img to given file
  int						scan_set_size();
	bool					ScanSame(CDataRdr* samescan);
	char*					ScanString(char *scanstring = NULL);
	virtual CData* ResetCompleteDataSet();
	virtual	CData* ThisCompleteDataSet();
	virtual CData* NextCompleteDataSet();
	virtual CData* PrevCompleteDataSet();
	virtual CData* GotoCompleteDataSet(int n);
	virtual void	SetLastScan(CDataRdr *last);
	CDataRdr*			GetRootScan();		// access parent class RootSet and typecast to a CDataRdr
	CDataRdr*			GetLastScan();		// access last class entry and typecast to a CDataRdr
	int						GetNumRadls();		// thread safe method to access num_radls
	int						GetNumScans();
	void					IncNumScans();
	s_volume_params* GetVolumeParams();
	int						GetDataSize();
	void					AddRadlPoint(rdr_angle newangle);
	bool					HasDataType(e_data_type datatype);
};

class rdrscan_freelist {
	CDataRdr*	FreeList;
public:
	CDataRdr*	GetRdrScan(); // try to allocate from freelist, else use new
	void    StoreRdrScan(CDataRdr* StorePnt);	// add to free list
	rdrscan_freelist();
	~rdrscan_freelist();
 };

#ifdef STDNODES
class rdr_scan_node {
public:
	rdr_scan_node *next,*prev;
	CDataRdr*	scan;
	CDataRdr*	thisscan;
	rdr_scan_node(CDataRdr*	ThisScan=0);	// attach to given scan
	~rdr_scan_node();			
	void attachscan(CDataRdr*	ThisScan = 0);// attach to given scan
	void detachscan();			// detach from current scan
	int this_scan_no;
	int	scansinset;
	bool	GPFlag;		// general pupose flag
	virtual CDataRdr*	goto_scan(int n);
	virtual CDataRdr*	reset_scan();
	virtual CDataRdr*	next_scan();
	virtual CDataRdr*	prev_scan();
	virtual CDataRdr*	this_scan();
};
#endif

#include "Requests.h"

#ifdef STDNODES
class rdrscannode_freelist {
	rdr_scan_node *FreeList;
public:
	rdr_scan_node *GetRdrScanNode();
	void    StoreRdrScanNode(rdr_scan_node *StorePnt);
	rdrscannode_freelist();
	~rdrscannode_freelist();
};
#else
class rdrscannode_freelist {
	CDataNode<CDataRdr>* FreeList;
public:
	CDataNode<CDataRdr>* GetRdrScanNode();
	void    StoreRdrScanNode(CDataNode<CDataRdr>* StorePnt);
	rdrscannode_freelist();
	~rdrscannode_freelist();
};
#endif


/*
	rdr_img reads data from the comms or database until end of scan is
	encountered. it then creates a rdr_scan for that data and adds it 
	to a linked list of scans
*/

class rdr_img {
friend	class	rdr_seq;
friend	class	DisplWin;
public:
	bool	    PrimVolComplete;	// true if primary volume scans complete
	time_t	    FirstTm,LastTm;	// date/time of 1st scan,last scan
	int		    year,month,day,hour,min,sec;	// date/time of 1st scan
	short	    num_scans;		// number of root scans making up image
	bool	    InSeq;		// true if currently in sequence
	bool	    ScanAdded;		// set flag when scan added, rdrseq may use
	int		    UserCount;
private:
	CDataNode<CDataRdr>* first_scan;		// first scan in linked list
	CDataNode<CDataRdr>* this_scan;		// current scan
	CDataNode<CDataRdr>* next_scan;		// next one
	CDataNode<CDataRdr>* last_scan;		// last in list
	rdr_img*	    next_img;		// linked list for sequence
	rdr_img*	    prev_img;
	void 	    check_scans();	// vector invalid scans out of list
public:
	int		    img_size();		// return size of data buffer
	rdr_img(CDataRdr*	initscan = 0);
	~rdr_img();
	void	    add_scan(CDataRdr* addscan);// add new scan to list
	CDataRdr*	    FirstScan();	// return pointer to first scan in list
	CDataRdr*	    SetFirstScan();	// set this_scan pointer to first in list
	CDataRdr*	    NextScan();
	CDataRdr*	    NextRootScan();
	CDataRdr*	    ThisScan();
	CDataRdr*	    ScanTypeMatch(CDataRdr* matchscan);	// true if same stn/type scan exists in image
	bool	    VolComplete(int stn);// true if stn volume scans complete
//	void	    IncUserCount();
//	bool	    ShouldDelete();
};

/*
class scan_client {
public:
	scan_client	    *nextclient,*prevclient;
	scan_client();
	~scan_client();
	bool	    RealTime;		// if FALSE, scan_mng will not add scans
	bool	    AcceptDuplicates;	// if FALSE, scanmng won't pass duplicate scan to client
	bool	    AcceptNewScans,	// if true use scans passed by NewScanAvail
							AcceptFinishedScans;// if true use scans passed by FinishedScanAvail
	SCANCLIENTTYPE  ClientType;		// type of client
	SCANCLIENTCACHE CacheType;
#ifdef STDNODES
	rdr_scan_node   *newscans;		// new scans, usually added by other threads 
#else
	CDataNode<CDataRdr>* newscans;		// new scans, usually added by other threads 
#endif
	time_t 	    rdrimg_tmwin;	// time window to qualify for same image
	virtual bool MergeScans();	// true if scan client is merging scans flag
	// most clients should use either NewScanAvail OR CompleteScanAvail
	// If both are used,  the client must have duplicate scan handling 
	//
	virtual int    NewScanAvail(CDataRdr* newscan);    // add scan to new sca list. usually called by other thread
	//return 1 if scan added, else 1
	virtual int    FinishedScanAvail(CDataRdr* finishedscan); // add finished scan to new sca list. usually called by other thread
	//return 1 if scan added, else 1
	virtual void    CheckNewScans(bool MoveToChecked, bool RejectFaulty); // check new scans list
	virtual bool Full();		// client "full indication
	virtual int	    NumImgs();		// client image count
	virtual bool IsDuplicate(CDataRdr* dupscan);	// return true if known ie already in seq or database etc
	virtual int ScanQuery(scan_query *query);
};
*/
#define MOVETOCHECKED TRUE
#define DONTMOVETOCHECKED FALSE
#define REJECTFAULTY TRUE
#define DONTREJECTFAULTY FALSE
#define FINISHEDONLY TRUE
#define UNFINISHED FALSE


class scan_client : public CLinkedList<scan_client> {
public:
//	scan_client	    *nextclient,*prevclient;
	scan_client();
	virtual ~scan_client();
	spinlock 	Lock;
	int				ScanQSize;				// queue of filtered scans
	int				NewScanQSize;			// queue of new scans
	bool	    RealTime;		// if FALSE, scan_mng will not add scans
	bool	    AcceptDuplicates;	// if FALSE, scanmng won't pass duplicate scan to client
	bool	    AcceptNewScans;	// if true use scans passed by NewScanAvail
	bool			AcceptFinishedScans;// if true use scans passed by FinishedScanAvail
	SCANCLIENTTYPE  ClientType;		// type of client
	SCANCLIENTCACHE CacheType;
	CDataNode<CDataRdr>  *newscans, *checkedscans;		// new scans, usually added by other threads 
	CDataNode<CDataRdr> *lastcheckedscan;
	time_t 	    rdrimg_tmwin;	// time window to qualify for same image
	virtual bool	MergeScans();	// true if scan client is merging scans flag
	/* most clients should use either NewScanAvail OR CompleteScanAvail
	* If both are used,  the client must have duplicate scan handling 
	*/
	// add scan to new scan list. Usually called by other thread via serialised scan_mng
	virtual int    NewScanAvail(CDataRdr *newscan); //return 1 if scan added, else 1
	virtual int    NewScanAvail(CDataRdr *newscan, char *creater); //return 1 if scan added, else 1
	virtual int    FinishedScanAvail(CDataRdr *finishedscan); // add finished scan to new sca list. usually called by other thread
	virtual bool	 QueryResult(CDataNode<CDataRdr> *querynode = NULL);
	void					 PruneNode(CDataNode<CDataRdr> *node, char *deleter);
	void					 MoveToCheckedQueue(CDataNode<CDataRdr> *node);
	//return 1 if scan added, else 0
	// quickly check new scans list, add scans to keep to checkedscans, discard others
	virtual void    CheckNewScans(bool MoveToChecked = false, 
			bool RejectFaulty = true); // keep this quick, uses lock
	/*
	* ProcessCheckedScans doesn't need to be locked,  can perform 
	* time consuming operations
	*/				
	virtual void    ProcessCheckedScans(int maxscans = -1);// process checked scans list
	virtual bool		Full();		// client "full indication
	virtual int	    NumImgs();		// client image count
	virtual bool		IsDuplicate(CDataRdr *dupscan, 	// return true if known ie already in seq or database etc
											bool FinishedOnly = FALSE);  // if true, ignore unfinished scans
	virtual void    PrintScanUsage(FILE *file = 0, bool verbose = false);	// print information on scans used, memory consumed etc.
	virtual int			ScanQuery(CScanQuery *query);
};


class scan_mng {
public:
	scan_client *ClientList, *EndClientList;
	scan_mng();
	virtual ~scan_mng();
	spinlock *lock;
	void NewScanAvail(CDataRdr*	new_scan);	// simply add new scan to newscans list
	void FinishedScanAvail(CDataRdr*	finishedscan); // add finished scan to new sca list. usually called by other thread
	void AddClient(scan_client *new_client);
	void RemClient(scan_client *rem_client);
	bool IsDuplicate(CDataRdr*	new_scan, bool FinishedOnly = FALSE);// see if any scan clients already have this scan
	bool RemoveDuplicates;	    // if true try to remove duplicate scans
	void get_lock();
	void rel_lock();
	virtual CScanQuery* ScanQuery(scan_client *pOwner, CScanQuery *pQuery);
	virtual void  PrintScanUsage(FILE *file = 0);	// print information on scans used, memory consumed etc.
};
	
extern scan_mng *ScanMng;

#endif
