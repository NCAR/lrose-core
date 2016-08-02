#ifndef	__RDRSCAN_H
#define __RDRSCAN_H

#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#include <string>
#include <map>
using namespace std;
#else
#include <list.h>
#include <vector.h>
#include <string.h>
#include <map.h>
#include <multimap.h>
#endif
#ifdef sgi
#include <sys/bsd_types.h>
#endif
#include <ctype.h>
#include "rdr.h"
#include "expbuff.h"
#include "cdata.h"
#include "cdatanode.h"
#include "histogram.h"
#include "threadobj.h"
#include "rdrtypes.h"
#include "levelTable.h"
#include <math.h>
#ifdef USE_RAINFIELDS_CLIENT
#include "rainfieldImg.h"
#endif
#include "siteinfo.h"
// #include "rdrfilter.h"

extern bool checkRadialMSSG;
extern bool useScanUserList;
extern bool writeDetailedScanReport;
extern bool keepNullRadls;
/*
 * In realtime mode,  clients will typically read
 * COMM,  COMMREQ,  PROD_ACCUM and PROD_XLAT data
 * In DBREVIEWMODE mode the data source will be DB,  generally loaded
 * in bursts of many data scans.
 * In REPLAY mode the data source will be REPLAY, generally loaded in
 * a "simulated real time" manner,  i.e. with realistic time
 * between scans.
 */
enum e_scan_client_mode {
  REALTIMEMODE,	// data typically sourced from real-time comms 
  DBREVIEWMODE,	// "archive review" mode, data from DB
  REPLAYMODE};

/*
 * COMM - scan from perm connection,  ie NOT through comm req
 * DB - scan from database
 * COMMREQ - scan from comm req
 * If a source is added the corresponding string MUST be added to 
 * data_src_text in rdrscan.C
 * May need to add to scanclient's ScanSourceOK method
 */
enum	e_data_source {
  COMM, 
  DB, 
  DBRELOADREALTIME,
  COMMREQ, 
  PROD_ACCUM, 
  PROD_XLAT,
  REPLAY, 
  PROD_FILTDATA, 
  PROD_CAPPI, 
  PROD_VIL, 
  PROD_TOPS,
  RADARCONVERT,
  e_ds_max	// insert new enumerations before this
}; 
// source class of data

extern char *data_src_text[];
int get_data_src(char *srcstr);
int decode_datasrcstr(char *srcstr); // decodes either numeric or text
char *get_data_src_text(int datasrc);

enum	data_mode {INDEX, FLOATVALUE, BYTEVALUE}; // s_radl data type

enum	e_unpackedradltype {SIMPLE, MULTIMOMENT};

enum	SCANCLIENTTYPE {SC_UNDEFINED, SC_SEQ, SC_DB, SC_TXDEV, 
			SC_COMMMNG, SC_RAINACC, SC_SATDATAMNG, SC_NEXRADMNG, 
			SC_NEXRADSTNHDLR, SC_UFMNG, SC_UFSTNHDLR, SC_TITAN};
/***** WHEN A NEW TYPE IS ADDED A NEW STRING MUST BE *****/
/***** ADDED TO scanclienttype_strings in scanmng.C  *****/
/***** OR IT WILL CRASH *******/

extern char *scanclienttype_strings[];



struct	s_comp_params {
  short	scan_count;	    // number of scans in composite ppi
  rdr_angle 	elev_10th_arr[10];  // elev of scans (ELEV in 10ths of deg)
  short	rng_arr[10];	    // elev switch ranges (km)	
};

struct 	s_rdrparams {
  char	name[17];	
  short	wavelength;	// 10ths of centimetres
  rdr_angle	az_beamwidth,
    el_beamwidth;	// 10ths of degrees
  short	spare[32];
};

struct RadlComponent {	    // template to drop over MULTIMOMENT radial storage
  e_data_type  moment;	    // e_data_type moment
  short	length;	    // length of this moment component, including this header
  short	nextofs;    // offset of next moment (0 for last), relative to start of concat buffer
  char	data[1];    // data array
  RadlComponent *NextMoment(); // drop over next moment, return null if no more
};

struct UnpackedRadl {	    // simple(refl only) or multi-moment radial(multiple moments concatenated using RadlMoment struct)
  int		start_ofs;  // -1 signifies no radial
  short	radl_length;
};

extern int	RADLBUFFSIZE;

enum	e_rngres_reduce_mode {RRR_MAX, RRR_MIN, RRR_MED, RRR_AVG, RRR_PWR}; // range resolution reduction method
enum	e_rngres_increase_mode {RRI_DUP, RRI_INTERP}; // range resolution increase method


struct	s_radl {		// unpacked radial array
  rdr_angle	az,az1,az2;	// 10ths of deg, az - centre, az1/az2 width to draw
  float         az_f()
  {
    return float(az)/10.0;
  };
  inline float	azres()	        // float degrees az res
  {
    return fabs(az2/10.0 - az1/10.0);
  };
  rdr_angle	az_hr;		// hi-res az, other values are rounded to angle res.
  rdr_angle	el,el1,el2;	// 10ths of deg, el - centre, el1/el2 height to draw
  float         el_f()
  {
    return float(el)/10.0;
  };
  float	        startrng;	// metres, range of 1st data, default 4000
  inline float	startRngKM()	// km range of 1st data, default 4
  {
    return startrng/1000.0;
  };
  float	        rngres;		// metres, rng res
  inline float	rngResKM()	// km rng res
  {
    return rngres/1000.0;
  };
  int     	data_size;	// max used size of data in buffer
  int		buffsize;	// buffer size
  float		undefinedrng;	// rng to which data is undefined, 
  // primarily for CAPPI

  short	        numlevels;
  data_mode	mode;		// INDEX or VALUE mode
  e_data_type   data_type;	// moment type of data
  bf_data_type  bfdata_type;	// moment type of data
  e_unpackedradltype radltype;  // data storage method   
  e_scan_type	scan_type;	// type of scan(PPI,RHI,CompPPI,VOL,RHISet)
  uchar         *data;    	// uncompressed radial data (0.5dBZ units for refl)
  float	        *Values;	// optional value array (floating point Values)

  LevelTable	*LvlTbl;	// pointer to corresponding level table (0.5dBZ units of refl)
  // The LvlTbl is assumed to be created and disposed of by something else
  // THIS IS A REF ONLY AND NOT DELETED IN DESTRUCTOR

  void resize(int newsize, bool copy = false);
  
  s_radl(int BuffSize = 0);
  ~s_radl();
  void 	Clear();	// zero out this radial
  int	IndexAtRange(float rng, float *idxrng = 0);// return the array index at the given km rng
  float	RangeAtIndex(int idx);// return the range (km) at the idx value
  uchar	DataAtRange(float rng);	    // return the data index at the given km rng
  float	ValueAtRange(float rng);    // return the Value at the given km rng
  float	ValueAtIndex(int idx);    // return the Value at the given cell index
  int	CellsBtwnRngs(float rng1, float rng2);    // return the number of cells between the rngs
  int	GetCellsBtwnRngs(float rng1, float rng2, unsigned char *buff, int maxcount);    // return the number of cells between the rngs
  int	GetCellsBtwnRngs(float rng1, float rng2, vector<uchar> &buff, int buffpos, int maxcount);    // return the number of cells between the rngs
  void	TanPlaneRadl(float *cosel = 0);	// convert to tangent plane radial (0deg)
  int	PadRadl(int padrng); // pad radial to given range
  // simply threshold float *floatarray values into char *data array
  // other parameters of s_radl need to be set, e.g. rng res etc. 
  // if floatarray set to 0, try to use Values
  // This allows Values to be set to float array for thresholding
  void	ThresholdFloat(float *floatarray = 0, int size = 0, LevelTable *thresh_table = 0);
  void	ThresholdFloat(ushort *ushortarray, int size = 0, LevelTable *thresh_table = 0);
  // convert indexes back to Float values
  void	indexToFloat(float *floatarray = 0, int size = 0, LevelTable *thresh_table = 0);
  int   indexVal(float val)  // return index value for float value	
  {
    if (LvlTbl) return LvlTbl->threshold(val);
    else  return 0;
  };
  // convert radial data array to Rapic format data string
  int	Encode16lvlAz(uchar *outstring, int maxopsize);
  int	Encode16lvlEl(uchar *outstring, int maxopsize);
  int	Encode6lvlAz(uchar *outstring, int maxopsize);
  int	Encode6lvlEl(uchar *outstring, int maxopsize);

  int	EncodeBin(uchar *outstring, int maxopsize);
  int	EncodeAz(uchar *outstring, int maxopsize);
  int	EncodeEl(uchar *outstring, int maxopsize);
  void	RngRes2000to1000(); // convert 2000m res radial to 1000m res
  bool  RngResCanConvert(float op_res, float rng_res = 0); // retrn true if able to convert to op_res
  bool  RngResConvert(float op_res, 
		      e_rngres_reduce_mode reduce_mode,          // convert to op_res using reduce/increase mode
		      e_rngres_increase_mode increase_mode);     // return false if op_res not eact multiple of current res
  void  RngResReduce(int reduce_factor, e_rngres_reduce_mode mode); // reduce range resolution by given factor
  void  RngResReduceMax(int reduce_factor); // reduce range resolution by given factor using max val
  void  RngResReduceMin(int reduce_factor); // reduce range resolution by given factor using min val 
  void  RngResReduceMed(int reduce_factor); // reduce range resolution by given factor using median val 
  void  RngResReduceAvg(int reduce_factor); // reduce range resolution by given factor using average
  void  RngResReducePwrAvg(int reduce_factor); // reduce range resolution by given factor using average of refl power
  void  RngResIncrease(int increase_factor, e_rngres_increase_mode mode); // increase range resolution by given factor
  void  RngResIncreaseDup(int increase_factor); // increase range resolution by given factor by duplication
  void  RngResIncreaseInterp(int increase_factor); // increase range resolution by given factor by interpolation
  void	TruncateData();	    // set data_size to last nn-zero data
  void	AddThisToHistogram(histogramClass *hist = 0);

  // return index of max val (-1 if no data)
  // optionally return maxidx, maxrng, maxval, maxfval
  int		MaxValIdx(int *maxidx = 0, float *maxrng = 0, 
			  int *maxval = 0, float *maxfval = 0);
  // as above, but traverses floating point Values
  int		MaxFValIdx(int *maxidx = 0, float *maxrng = 0, 
			   int *maxval = 0, float *maxfval = 0); 
};

struct 	s_radl_node {			// radial node for linked lists
  s_radl	*RadlData;
  s_radl_node	*Next,*Prev;
  s_radl_node(int BuffSize = 0);
  ~s_radl_node();
  void resize(int newsize)
  {
    if (RadlData)
      RadlData->resize(newsize);
  };
};

class radl_pnt {
 public:
  radl_pnt	*next,*prev;	// for linked list in free list
  e_unpackedradltype	radltype;// data storage method
  int		numradials;
  int		*PntTbl;	// table of radial start offsets
  void 	        *FreeList;
  radl_pnt(int NumRadials, void *freelist = 0);
  // if FreeList undefine, try to use global FreeListMng->RadlPntFreeList
  ~radl_pnt();
  int           getSize(); // return size of this object
};

class radl_pnt_freelist {
  spinlock	*lock;
  radl_pnt	*FreeList;
  int           freeCount;
  long long     freeSize;
 public:
  radl_pnt	*GetRadlPnt(int NumRadials);
  void	StoreRadlPnt(radl_pnt *StorePnt);
  radl_pnt_freelist();
  ~radl_pnt_freelist();
  int    getCount() { return freeCount; };
  long long getSize() { return freeSize; };
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

  Traditional volumetric scan sets generated by the rapic transmitter
  know how many scans they are doig so can use a PASS: x of y message
  Some research radars may not know ahead of time how many tilts are to be
  performed and so rdrscan may support a PASS: x style also
  A PASS: x of 0 style may also be used which will offer reasonable backwards
  compatability with older 3drapic systems, 
  In this case the data must also send an
  END SCAN SET message to close the set

*/

// const	int	LBMAX=2048;	// max line_buff size
class rdr_scan_linebuff {
 public:	
  int	new_scan_ofs;	// start of next scan
  bool	isBinRadl;
  bool	isRadl;
  bool	isComment;
  bool	EOL;
  bool	termCh1, termCh2;   // used for binRadl EOL detection
  bool	Overflow;
  int	lb_size;		// current size
  char	*line_buff;// line of data buff
  int	lb_max;
  char	*lb_pnt;    // pointer to next vacant pos in buffer
  char  logmssg[128];   // string to include in log messages
  rdr_scan_linebuff(int buffsz = 2048);
  ~rdr_scan_linebuff();
  inline bool addchar(char c)	// simply append char to linebuff
    {				// ****NO EOL TESTING PERFORMED
      if (IsFull()) return false;
      if (lb_size == 0)   // first char of line
	{
	  if (isspace(c) || iscntrl(c))	// don't add leading white space
	    //		if (!isalnum(c))// first must be alphanumeric
	    return true;
	  switch (c) {
	  case '@' : isBinRadl = true; isRadl = true;
	    break;
	  case '%' : isRadl = true;
	    break;
	  case '/' : isComment = true;
	    break;
	  }
	}
      addc(c);
      return true;
    };
  bool shouldDoMssgCheck();
  bool addchar_parsed(char c);

  inline void addc(char c) 
    { 
      *lb_pnt = c;
      lb_pnt++;
      lb_size++;
    };
	
  void reset(char *_logmssg = NULL);
  void setLogMssg(char *_logmssg);
  inline bool	IsBinRadl()
    {
      return isBinRadl = ((lb_size > 0) && (line_buff[0] == '@'));
    };
  inline bool	IsRadl()
    {
      return isRadl = ((lb_size > 0) && 
		       ((line_buff[0] == '@') || (line_buff[0] == '%')));
    }
  inline bool	IsComment()
    {
      return isComment = ((lb_size > 0) && (line_buff[0] == '/'));
    }
  inline bool	IsEOL() { return EOL; };
  inline bool	IsFull(int tailspace = 0) { return lb_size >= lb_max - tailspace; }; // buffer is full
  void		ensureTerminated(); // ensure line is null terminated
};

#ifdef TRACKCLIENTS_OLD
class ScanUserNode {
 public:
  ScanUserNode(void *userpnt, char *str);
  ~ScanUserNode();
  void *UserPnt;
  string Str;
};
#endif

class rdr_scan_node;	// forward declaration
class scanProductCreator;


enum RDRSCAN_DELSTATE {NO_DEL, DELETING, DELETED};
time_t rdrScanKeyTimet(const char *scankey);

// az dim for full sweep should be numradls + 1 to 
// repeat start radial at end

class rdr_scan_array {
 public:
  uchar *data;
  int rngDim, azDim, tiltDim;
  float rngRes,   // km
    azRes;        // deg 
  float startRng, // km
    endRng;       // km
  float startAz, // deg
    endAz;       // deg
  bool fullCircle;
  vector<float> elTiltTable;
  e_data_type dataType;
  rdr_scan_array(int _rngdim, int _azdim, int tiltdim = 1);
  ~rdr_scan_array();
  void setDims(int _rngdim, int _azdim, int tiltdim = 1);
  void clearData();
};

class rdr_scan {
  friend class rdrfilter;

  exp_buff*	data_buff;	    // data buffer
  exp_buff_rd	dbuff_rd;	    // data buffer reader
  int		header_start;	    // start of header in buffer
  int		data_start;	    // start of radar data in buffer
  bool	        data_valid;	    // true if data reasonable
  bool	        header_valid;	    // true if header complete, not necessarily data
  bool	        scan_complete;	    // true if complete, no more changes
  int		faultno;
  string        faultstr;
  // if scan_complete and not data_valid, scan is faulty
  int		nullcount;
  // The following allow sets of scans. Single scan is a scan set of 1
  char	        setlabel[32];	    // date/time stamp for this scan set.
  // all set scans should have same
  char	        scanStr[128];	    // short description of this scan
  char	        scanKey[64];	    // ascii key for this scan
  rdr_scan	*nextinset,*previnset;	// scan set links
  rdr_scan 	*rootscan;	    // root scan of this set
  rdr_scan	*lastscan;	    // last scan in this set 


  //  rdr_scan 	*thisscan;	    // current scan in scan set
  //  int		thisscanno;	    // pos. of thisscan in set (0 = root)
 private:
  //  rdr_scan* 	reset_scan();	
  // set thisscan to first in list (thiscan for single scan e.g. PPI RHI)
  //  rdr_scan* 	this_scan();	    
  // set thisscan to first in list (thiscan for single scan e.g. PPI RHI)
  //  rdr_scan* 	next_scan();	    // return next scan in set. Sets thisscan. 
  // Will always set thisscan NULL for single scans (PPI & RHI)
  // rdr_scan* 	prev_scan();	    // return prev scan in set. Sets thisscan. 
  // Will always set thisscan NULL for single scans (PPI & RHI)
  //  rdr_scan* goto_scan(int n);	    
  // return scan "n" in set. Sets thisscan. Will always set thisscan NULL 
  // for n > num_scans
  //  rdr_scan *setRootScan(); // return pointer to root scan - sets thisscan
  //  rdr_scan *setNextInSet();// return pointer to next in set - sets thisscan
  //  rdr_scan *nextInSet();	
  // return pointer to next in set - doesn't set thisscan
  //  rdr_scan *setLastScan(); // return pointer to last in set - sets thisscan

  void	*scanCreator;	    // pointer to whatever created this scan instance
 public:
  string scanCreatorString; // name of who created this scan instance
  string creatorFieldString;// string from the scan's CREATOR: field
  float  creatorFieldVersion;// version number from scan's VERS: field
  rdr_scan* gotoScan(int n); // step to scan n - 0=rootscan
  void	check_data();
  bool dataValid() { return  data_valid; };
  bool scanComplete() { return scan_complete; };
  void setScanComplete(bool state = true) { scan_complete = state; };

 private:

  list<rdr_scan*> productScans; // linked list of derived products, cappi vil etc
  spinlock	*productScanLock;
    
  RDRSCAN_DELSTATE delstate;	    // track delete state, use for debugging
  map<void*, string> ScanUserList;	// keep track of users referring to this scan
  void	AddUserToList(void *user, char *str);
  bool	RemoveUserFromList(void *user, char *str);
  bool	FindUserNode(void *user, bool remove = false);
  void	ClearUserList(bool report);
 public:
  void	DumpUserList(FILE *outfile = NULL, char *prefix = NULL);
 private:
    
  rdr_scan_linebuff *linebuff;
    
  int		lockwaitmax;	    // max no of CLKTICKS
  spinlock	*lock;
  bool	get_lock();
  void	rel_lock();
  void	del_lock();
    
  radl_pnt *RadlPointers;	    // radial start pointers
  void	SetRadlPnt();	    // set the radl pointers for this scan
  bool	UseRadlPnt;

  int		UserCount;
        
  int	get_date(char* LineIn,int StartPos);
  int	get_time(char* LineIn,int StartPos);
  int	get_timestamp(char* LineIn,int StartPos);
  int	get_datetime(char* LineIn,int StartPos, time_t *outtime);
  void	get_threshtbl(char *LineIn, float *threshtbl, int &numthresh);

  bool	load_new_scan(); // create new scan in this set, from 
                         // return false if scan invalid or child doesn't match
  bool	load_scan(int scan_ofs);

  bool	debug;		// if true output debug info
  int		data_size;	// size of data buffer
  short	        num_scans;	
  // count of scans in this scan set or the volume set this prod scan 
  // was derived from
  // if this is a prod scan and num_scans is set to -1, the num_scans won't be tested
  // for product regeneration on volume set changes (e.g. filter is single scan based)
  time_t        headerAddedTime;// time last header added to root scan
  time_t        rxTimeStart,    // time receive of scan started
    rxTimeEnd,                  // time receive of this scan finished
    rxTimeSetEnd;                 // time receive of scan set finished

 public:
  e_data_source data_source;	// data source class - COMM,DB etc.
  int		station;	// source radar station, lsbyte=stnid, msbyte=countryid
  int		year,month,day,hour,min,sec;	// date/time of scan
  time_t	scan_time_t;	// unix time_t date/time of scan capture
  time_t	FirstTm,LastTm;	// date/time of VOLUME'S 1st scan,last scan
  time_t	StartTm,EndTm;	// start/end date/time of product e.g. rain_accum
  time_t        Period() { return EndTm - StartTm; };
  time_t 	LastScanTm;	    // time of last scan, clients may use to detect scan set change
  void setRxTimeStart(time_t rxstart, bool force = false, char *debugstr = NULL);
  void setRxTimeEnd(time_t rxend, bool force = false, char *debugstr = NULL);
  void setRxTimeSetEnd(time_t rxsetend, bool force = false, char *debugstr = NULL);
  time_t RxTimeStart() { return rxTimeStart; };
  time_t RxTimeEnd() { return rxTimeEnd; };
  time_t RxTimeSetEnd() { return rxTimeSetEnd; };
  int    rxTimeSecs();          // secs btwn end&start - return at least 1
  s_rdrparams	rdr_params;	// radar parameters
  e_scan_type	scan_type;	// type of scan(PPI,RHI,CompPPI,VOL,RHISet)
  e_data_type   data_type;	// refl. vel. etc.
  bf_data_type  bfdata_type;// multi-moment bitfield
  e_unpackedradltype radltype;// method of storage in radials
  //    char	moments;	// bitfield of data_types present
  e_data_fmt	data_fmt;	// data format (Rapic 6lvl, 16lvl etc.)
  rdr_angle     angle_res;	// radial width(10ths 0f deg)
  float         angle_res_f()
    {
      return float(angle_res)/10.0;
    };
  //    LevelTable	*LvlTbl;	// Level threshold table, only present in root scan
  LevelTable	*_LvlTbls[e_dt_max];   
  LevelTable	*LvlTbls(e_data_type lt_type)
    {
      return rootScan()->_LvlTbls[lt_type];
    };
  // array of pointers to Level threshold tables for each data type
  // only present in root scan
  bool          clearAirMode;   // if true, level 1 will be a special clear air value

  int		NumLevels;	// number of threshold levels
  float	        nyquist;	// vel scans store nyquist value
  float         frequency;      // frequency of radar in MHz
  float	        rng_res;	// metres
  inline float	rngResKM()	// km rng res
  {
    return rng_res/1000.0;
  };
  float	        start_rng;	// metres start of radial data
  inline float	startRngKM()	// km range of 1st data, default 4
  {
    return start_rng/1000.0;
  };
  float         undefined_rng;  // metres usually start_rng, CAPPI will be different
  float	        max_rng;	// km max range
  inline float	endRngKM()	// km range of last data, default 4
  {
    return max_rng;
  };  
  short     	num_radls;	// number of non-zero radials
  short     	num_bad_radls;	// number of bad radials
  short	        radl_pos;	// pos of this radial(count)
  int           prf;            // pulse repetition frequency
  short	        vol_scans,      // scans in vol, if 0 scans is unknown
    vol_scan_no;	        // this scan no 
  short	        vol_tilts,vol_tilt_no;	// tilts in vol./this tilt no
  short	        completed_tilts;// count of tilts (may be multiple moment scans per tilt) in this scan set
  rdr_angle	previous_tilt_angle;// use this to detect start of new. see completed tilts 
  int   completedTilts();
  int   thisTilt(rdr_scan *tiltscan = NULL); // if no tiltscan, use this
  int   completedScans();

  // -1=no volume id defined, default behaviour
  int   volume_id;      // radar volume id, for multi volume radars

  int   dataSize() { return data_size; };
  int   rngBins() 
    { 
      int bins = int(((max_rng * 1000) - start_rng) / rng_res);
      if (bins > RADLBUFFSIZE)
	RADLBUFFSIZE = bins;   // expand default radl size to new max
      return bins;  // maximum number of radial bins
    };
  int   volMaxRngBins(e_data_type field = e_dt_max) 
    { 
      int maxbins = 0;
      rdr_scan *pscan = rootscan;
      while (pscan)
	{
	  if (pscan->rngBins() > maxbins)
	    maxbins = rngBins();
	  pscan = pscan->nextinset;
	}
      return maxbins;  // maximum number of radial bins
    };
  rdr_angle	set_angle;	// constant angle(el for PPI) (cappi ht in metres)
  inline float	setAngle()	// return floating pt angle degrees
  {
    return set_angle / 10.0;
  };  
  bool sectorScan;
  // for sector scans, start/end angles
  // start is "left" az(most CCW), end is "right" az (most CW)
  // RHISet will usually be sector in elev, rely on scan set for az sector
  float sectorStartAngle, 
    sectorEndAngle;         
  bool sectorAngleIncreasing;
  s_comp_params   comp_params;// composite parameters

  bool	ShownToScanClients; // true if scan manager has shown this scan to clients
  bool	ScanSetComplete;    // true if all scans present
  bool	ScanFinished;	    // true if no further additions
  bool	StoredInDB;	    // true if it has been saved to the database
  bool	DBDuplicate;	    // true if db found it was a duplicate
  bool	ScanMngDuplicate;   // true if scanmng found it was a duplicate

  rdr_scan(void *creator, char *str, 
	   rdr_scan *rootscan = 0, bool appendtoroot = false);	
  // root scan is only defined when root scan is generating new child scans
  // DATA CAN ONLY BE ADDED THROUGH ROOT SCAN
  // if linktoroot is set append this scan to the end of the rootscan's list
  ~rdr_scan();				// destructor
  void	IncUserCount(void *user, char *str);
  bool	ShouldDelete(void *user, char *str);
  int	GetUserCount();

  void	set_dflt();
  void	set_dflt(rdr_scan *basis_scan);
  rdr_scan *rootScan();	// return pointer to root scan - doesn't set thisscan
  bool	isRootScan()	// return true if this is the root scan
    {
      return (this == rootscan);
    };
  rdr_scan *NextScan(rdr_scan *scan);  
  // return pointer to next after given scan - doesn't set thisscan
  rdr_scan *lastScan();	// return pointer to last in set - doesn't set thisscan
  void	add_data(char* inbuff, int dcount);// read radar data, filter,
  void 	add_line(rdr_scan_linebuff *lbuff, 
		 bool check = false, 
		 bool allowchildadd = false);	// add filtered line of data
  void 	add_line(char *ln = 0, int lnsz = -1, 
		 bool check = false, 
		 bool allowchildadd = false);// add filtered line of data
  int		read_line(char *linebuff, int maxchars, exp_buff_rd *dbuffrd);   // read a line of data from the data_buff

  bool	end_img_str(char* instr); // END RADAR IMAGE OR END INCOMPLETE VOLUME
  bool	end_img_str(rdr_scan_linebuff *lbuff); // END RADAR IMAGE OR END INCOMPLETE VOLUME
  bool	end_scanset_str(char* instr); // END RADAR IMAGE OR END INCOMPLETE VOLUME
  bool	end_scanset_str(rdr_scan_linebuff *lbuff); // END RADAR IMAGE OR END INCOMPLETE VOLUME
  void	data_finished();	// SET STATUS no more data for this scan
  void	fault_no(int fault, char *_faultstr = NULL);	// set fault variable
  int	get_fault_no();	// get fault variable
  string& get_fault_str()
    {
      return faultstr;	// get fault variable
    };
  const char* get_fault_cstr()
    {
      return faultstr.c_str();	// get fault variable
    };
  bool	Complete();	// true if all scans present
  bool	thisScanComplete()	// true if this scan is complete
    { return scan_complete; };
  bool	Finished();	// GET STATUS true if no further additions
  bool	HeaderValid();	// true if header data has been set
  bool	CheckForValidHeader();	// true if adequate valid header data present
  bool 	Faulty();	// true if faulty or complete and not valid
  // scan clients should look for this and delete if true
  bool	HasOSLock();	    // diagnostic utility, true if OS lock present
  int	StnID();	// lsbyte of "station" is stnid
  int	CountryID();	// msbyte of "station" is countryid
  int   CountryCodeToID(int _CountryCode); // return internal id for country code
  int   CountryIDToCode(int _CountryID);   // return country code for internal id 

    
  time_t ScanTime();		    // return "official" time of scan
  time_t FirstTime();		    // return time of earliest scan in set
  time_t LastTime();		    // return time of latest scan in set	
    
  rdr_scan* getTilt(int tiltnum, e_data_type datatype);	
  // return pointer to matching type at tilt
    
  exp_buff*	DataBuff() { 
    if (rootscan) 
      return rootscan->data_buff;
    else
      return data_buff; 
  }; // data buffer

  /*
   * The following calls MUST be passed 
   * exp_buff_rd *dbuffrd, short *radlpos
   * instances if being called from other than the main thread
   * This is required to store the state information required
   * for these stateful operations
   * If being called from the main thread NULL pointers can be passed
   * in which case the dbuff_rd and radl_pos objects will be used
   */
  void	reset_radl(exp_buff_rd *dbuffrd, short *radlpos); // point to first radial
  int	get_next_radl(s_radl *NextRadl, exp_buff_rd *dbuffrd, short *radlpos, 
		      e_data_type datatype=e_refl);    // get and unpack next defined radial
  int	get_radl_angl(s_radl *Radl, rdr_angle Angl, exp_buff_rd *dbuffrd,
		       short *radlpos, e_data_type datatype=e_refl);// returns -1 if failed
  // get and unpack radial at given angle.
  int	get_radl_no(s_radl *radl, int radlno, exp_buff_rd *dbuffrd,
			    short *radlpos, e_data_type datatype=e_refl);// get radial n, returns -1 if failed, 
  /* e.g. if angle_res = 0.5(5) radial 10 = radial at 5degrees */
  // return false if no echo at angl/rng
  bool	get_data_angl_rng(uchar &retval, rdr_angle Angl, 
			  float rng, exp_buff_rd *dbuffrd, 
			  short *radlpos, e_data_type datatype=e_refl, 
			  bool tanplanerng = false);

  // return false if no echo at angl/rng
  bool  get_val_angl_rng(float &retval, int &idx, float az, 
			 float rng, exp_buff_rd *dbuffrd, 
			 short *radlpos, e_data_type datatype=e_refl, 
			 bool tanplanerng = false);
  bool  get_val_angl_rng(float &retval, float az, float rng, 
			 exp_buff_rd *dbuffrd, 
			 short *radlpos, e_data_type datatype=e_refl, 
			 bool tanplanerng = false);
  

  int   write_error;   // errno status of last write operation - 0 = no error
  // write scan to given file at current pos
  // if dataonly defined, don't write header block (assume already written)
  int	write_scan(char *fname, 
		   bool reflonly = false);
  // write scan to given file at current pos
  // if dataonly defined, don't write header block (assume already written)
  int	write_scan(int fd, 
		   bool reflonly = false);
  int	append_scan(int fd,off_t *startofs,int *size,
		    bool writeReflOnly = false);  // append img to given file
  int	write_scan_set(char *fname, 
		       bool writeReflOnly = false);// write scan set to given file
  int	write_scan_set(int fd,off_t *startofs,int *size,
		       bool writeReflOnly = false);// write scan set to given file at current pos
  int	append_scan_set(int fd,off_t *startofs,int *size,
		       bool writeReflOnly = false);	// append img to given file
  exp_buff *GetExpBuff() {return data_buff;};

  bool  readRapicFile(char *rapicfname);
  bool  readRapicFile(int rapicfd, unsigned long &offset, 
		      unsigned long &endoffset, int &sz);
  void  AddScanLine(char *ln_buff, int lb_size);
    
  bool	ScanSame(rdr_scan *samescan);
  char	*ScanString(char *scanstring = 0);
  char	*ScanString2(char *scanstring); // caller must provide string buffer
  char	*ScanKey(char *scanstring = 0);
  char  *volumeLabel() { return setlabel; };
  // the char *buffer form writes the header to the supplied buffer
  // the exp_buff form writes to a temp buffer then copies it to the exp_buff
  void	Write16lvlHeader(exp_buff *outbuff = 0);	// write this scan's 16lvl ASCII header
  void	Write16lvlHeader(char *buffer, int &hdrsize, int maxsize);		// write this scan's 16lvl ASCII header
  rdr_scan*	check_scan_type(e_scan_type scan_type, e_data_type data_type, 
				rdr_angle elev, rdr_angle elev_tol); 

  rdr_scan_array *scanArray;
  rdr_scan_array *_volScanArray;
  rdr_scan_array *volScanArray()
    {
      return rootscan->_volScanArray;
    };
  
  // Creates and returns a pointer to a 2D/3D array of this scan
  // If rngdim is specified and != 0, it will be used for O/P array dimension
  // rngdim & azdim values are set by this method
  // IT IS THE RESPONSIBILITY OF THE CALLER TO DELETE THE ARRAY WHEN FINISHED 
  // If repeatfirstradl is set, the first radial will be repeated at the end
  // of the array and 
  unsigned char* Create2DPolarArray(int *rngdim = 0, int *azdim = 0,
				    uchar *destbuffer = NULL);

  rdr_scan_array *CreateScanArray();
  void deAllocScanArray();

  // create a rdr_scan_array to the rdr_scan->rootscan::volScanArray reference
  // and returns a pointer to it
  // volScanArray will be deleted by the rdr_scan dtor, but should usually
  // be deallocated by the caller of this method after use
  rdr_scan_array* CreateVolScanArray(e_data_type datafield);
  void allocVolScanArray(int _rngdim, int _azdim, int _tiltdim)
  {
    deAllocVolScanArray();
    rootscan->_volScanArray = new rdr_scan_array(_rngdim, _azdim, _tiltdim);
  };
  void deAllocVolScanArray()
  {
    if (rootscan->_volScanArray)
      delete rootscan->_volScanArray;
    rootscan->_volScanArray = NULL;
  };

  // Populates the data_buff with header and radial data
  // The 2DArray is rng/az indexes
  // Assumes this rdr_scan's parameters have already been initialised
  rdr_scan *EncodeDataFrom2DPolarArray(unsigned char *dest_array, int dest_xdim, int dest_ydim, 
				       rdr_scan *srcscan = 0);	// use optional srcscan for header values
  //SD added 24/2/5
  void  WriteDataFrom2DPolarArray(int fd, unsigned char *dest_array, int dest_xdim, int dest_ydim);

  void	setCreator(void *creator);
  void*	getCreator();

  void	addProductScan(rdr_scan* newProdScan);	// add product scan to this rdr_scan
  bool	delAllProductScans(void *creator, char *str);  // del all product scans - return true if something was deleted
  bool	delProductScansByCreator(void *creator, char *str);// del all product scans from this creator - return true if something was deleted
  // this method will look for a scan by the defined creator
  // but will not create one if it doesn't exist
  rdr_scan*	getProductScanByCreator(void *creator);// return first product scans from this creator
  rdr_scan_node* getProductScanNodeByCreator(void *creator);// return first product scans from this creator
  // this method will look for a scan product of the scanProductCreator
  // if it doesn't exist, it will be created and added to the productScans list
  rdr_scan*	getProductScanByScanCreator(scanProductCreator *creator);// return first product scans from this creator

  void setScanCount(int scancount = 0) { num_scans = scancount; };
  void setDataSize(int datasize = 0) { data_size = datasize; };

  int	scanSetCount();	   // return number of scans in this scan set
  int	scanSetSize();	   // return total size of this scan set
  int	thisScanSize();	   // return size of this scan in set
  int	productScanCount();// return number of scans in product list
  int	productScansSize();// return total size of product scans in list
};

class rdrscan_freelist {
  rdr_scan *FreeList;
 public:
  rdr_scan *GetRdrScan(); // try to allocate from freelist, else use new
  void    StoreRdrScan(rdr_scan *StorePnt);	// add to free list
  int     freeCount();	// number of scans in list
  rdrscan_freelist();
  ~rdrscan_freelist();
};

class rdr_scan_node {
 public:
  rdr_scan_node *next,*prev;
  rdr_scan *scan;
  rdr_scan *thisscan;
  void *userpnt;
  char userstr[64];
  rdr_scan_node(void *user, char *str, rdr_scan *ThisScan=0);	// attach to given scan
  ~rdr_scan_node();			
  void remove_from_list();		// remove from linked list, move prev/next links
  void detachscan();			// detach from current scan
  void attachscan(void *user, char *str, rdr_scan *ThisScan);// attach to given scan
  int this_scan_no;
  int	scansinset;
  bool	GPFlag;		// general pupose flag
  rdr_scan *goto_scan(int n);
  rdr_scan *reset_scan();
  rdr_scan *next_scan();
  rdr_scan *prev_scan();
  rdr_scan *this_scan();

  int	    ListCount();	   // return number of all scans (incl children) in linked list from here onwards
  int	    ListRootCount();	   // return number of root scans in linked list from here onwards
  int	    ListSize();	           // return total size of linked list from here onwards
  int	    ListSizePPI();	           // return total size of linked list from here onwards
  int	    ListSizeVol();	           // return total size of linked list from here onwards
  int	    scanSetCount();        // return number of scans in this node's scan
  int	    scanSetSize();	   // return total size of this node's scan

  int	    productListCount();	   // return number of product scans in linked list from here onwards
  int	    productListSize();	   // return total size of product scan in linked list from here onwards
  int	    productScanCount(); // return number of product scans in this node's scan
  int	    productScansSize();  // return total size of product scans in the node's scan
};

class scanProductCreator {
 public:
  // creates a scan product from the  passed src_scan
  // if scansetroot is passed the product is added to that scanset
  virtual rdr_scan* createScanProduct(rdr_scan *src_scan, rdr_scan *scansetroot = NULL);
  scanProductCreator();
  virtual ~scanProductCreator();
};

class rdrscannode_freelist {
  rdr_scan_node *FreeList;
 public:
  rdr_scan_node *GetRdrScanNode();
  void    StoreRdrScanNode(rdr_scan_node *StorePnt);
  rdrscannode_freelist();
  ~rdrscannode_freelist();
};


/*
  rdr_img reads data from the comms or database until end of scan is
  encountered. it then creates a rdr_scan for that data and adds it 
  to a linked list of scans
*/

class rdr_img {
  friend	class	rdr_seq;
  friend	class	DisplWin;
 public:
  bool	        PrimVolComplete;	// true if primary volume scans complete
  time_t	img_time_t,		// rounded off image time mod nearest time span 
    img_time_span,	// period of time covered by this image
    FirstTm,LastTm;	// date/time of 1st scan,last scan
  int		    year,month,day,hour,min,sec;// date/time of image
  short	    num_scans;		// number of root scans making up image
  bool	    InSeq;		// true if currently in sequence
  int       seqpos;             // pos of this img in seq, n-0, 0 is last, 
  int	    UserCount;
  void	    *Creator;
  char	    CreatorStr[64];
  int       lastImgSz, lastRFImgSz;
  stnSetRefCount stnVolCounts;  // keep track of number of volume scans per stn
  // to support multi volume_id handling, also 5 minute vols in 10 min sequence
 private:
  rdr_scan_node   *first_scan;		// first scan in linked list
  rdr_scan_node   *this_scan;		// current scan
  rdr_scan_node   *next_scan;		// next one
  rdr_scan_node   *last_scan;		// last in list
  rdr_img         *next_img;		// linked list for sequence
  rdr_img	  *prev_img;
  void 		  check_scans();	// remove invalid scans from list
  bool		  bDataChanged;	// set flag when scan added, rdrseq may use

  CDataNode	  *sat_cdata;		// pointer to satellite data
  CDataNode	  *first_cdata;	// first scan in linked list
  CDataNode	  *this_cdata;	// current scan
  CDataNode	  *next_cdata;	// next one
  CDataNode	  *last_cdata;	// last in list
  void 		  check_cdata();	// vector invalid scans out of list

 public:
#ifdef USE_RAINFIELDS_CLIENT
// rainfield images, mapped by product key
  map<string, Ref<rainfieldImg> >  rf_Images; 
  bool              add_data(Ref<rainfieldImg>& newrfimg); // add RF image
  rainfieldImg*     getRFImg(int stnid, void* creator); // get RF image by stnid and creator
  rainfieldImg*     getRFImg(int stnid, int accumtime); // get RF image by stnid and accum time
  int               RemoveRFImgs(int stn); // remove all rainfield images for this stn
#endif
  int               getRFImgSz();
  int               getRFImgCount();
  int		    img_size();		// return size of data buffer
  int		    img_size_ppi();	// return size of data buffer
  int		    img_size_vol();	// return size of data buffer
  rdr_img(rdr_scan *initscan, time_t timespan = 600, 
	  void *creator = NULL, char *str = NULL);
  rdr_img(CData *initdata, time_t timespan = 600, 
	  void *creator = NULL, char *str = NULL);
  rdr_img(time_t inittime = 0, time_t timespan = 600, 
	  void *creator = NULL, char *str = NULL);
  void init(time_t inittime = 0, time_t interval = 600, 
	    void *creator = NULL, char *str = NULL);
  ~rdr_img();
  /*
   *	add new scan to list,  REJECT IF NOT IN TIME WINDOW +/- (timespan/2)
   */
  bool	            add_data(rdr_scan *addscan);
  bool	            add_data(CData *addcdata);
  rdr_scan*	    FirstScan();	// return pointer to first scan in list
  rdr_scan*	    SetFirstScan();	// set this_scan pointer to first in list
  rdr_scan*	    NextScan();
  rdr_scan*	    NextRootScan();
  rdr_scan*	    ThisScan();
  rdr_scan*	    ScanTypeMatch(rdr_scan *matchscan);	// true if same stn/type scan exists in image
  rdr_img*	    PrevImg() { return prev_img; };
  rdr_img*	    NextImg() { return next_img; };
  CData*	    FirstCData();	// return pointer to first cdata in list
  CDataNode*	    FirstCDataNode();	// return pointer to first cdatanode in list
  CData*	    FirstSatCData();	// return pointer to first sat_cdata in list
  CDataNode*	    FirstSatCDataNode();// return pointer to first sat_cdatanode in list
  bool	            VolComplete(int stn);// true if stn volume scans complete
  void	            IncUserCount();
  int		    GetUserCount();
  bool	            ShouldDelete();
  bool	            WithinTimeSpan(time_t time);    // return true if time belongs in this image
  int		    ListCount();	// return number of scans in img linked list
  int		    ListRootCount();	// return number of scans in img linked list
  int		    ListSize();		// return total size of img linked list
  int		    ListSizePPI();	// return total size of CompPPIs list
  int		    ListSizeVol();	// return total size of VOLs list
  int		    ListSizeRF();	// return total size of Rainfields list
  int	            productListCount();	   // return number of product scans in linked list from here onwards
  int	            productListSize();	   // return total size of product scan in linked list from here onwards
  bool	            DataChanged();
  void	            SetDataChanged(bool state = TRUE);
  time_t	    imgTime() { return img_time_t; };
  time_t	    imgTimeSpan() { return img_time_span; };
  void	            setImgTime(time_t inittime, time_t interval = 0);
  void	            delProductScansByCreator(void *creator, char *str);// del all product scans from this creator
};

#define MOVETOCHECKED TRUE
#define DONTMOVETOCHECKED FALSE
#define REJECTFAULTY TRUE
#define DONTREJECTFAULTY FALSE
#define FINISHEDONLY TRUE
#define UNFINISHED FALSE

/*
  scanEventClient - simple base class to receive new and finished scan events from the scanMng
*/

class scanEventClient : public ThreadObj {
 public:
  bool	          AcceptDuplicates;	// if FALSE, scanmng won't pass duplicate scan to client
  bool	          AcceptNewScans,	// if true use scans passed by NewScanAvail
                  AcceptFinishedScans;// if true use scans passed by FinishedScanAvail
  scanEventClient() : ThreadObj()
    {
      AcceptDuplicates = false;
      AcceptNewScans = false;
      AcceptFinishedScans = true;
    };
  virtual ~scanEventClient() {};
  virtual int	  NewDataAvail(rdr_scan *newscan)
    { return 1;}; //return 1 if scan added, else 1
  virtual int	  FinishedDataAvail(rdr_scan *finishedscan)
    { return 1; }; // add finished scan to new sca list. usually called by other thread
};

class scan_client : public ThreadObj {
 private:
  
  int		  _scan_node_count;	// scan node accounting check
  time_t 	  rdrimg_tmwin;	        // time window to qualify for same image
 public:
  scan_client	  *nextclient,*prevclient;
  scan_client();
  virtual ~scan_client();
  spinlock 	  *lock;
  bool	          RealTime;		// if FALSE, scan_mng will not add scans
  bool	          AcceptDuplicates;	// if FALSE, scanmng won't pass duplicate scan to client
  virtual bool    AcceptDuplicate(rdr_scan *dupscan) // if FALSE, scanmng won't pass duplicate scan to client
    { return AcceptDuplicates; }; // default action is to use AcceptDuplicates value
                                  // child classes could base decision on dupscan properties
  bool	          AcceptNewScans,	// if true use scans passed by NewScanAvail
                  AcceptFinishedScans;// if true use scans passed by FinishedScanAvail
  bool	          AllowReplayMode;	// if not true, cannot set REPLAY mode
  bool	          AllowDBReviewMode;	// if not true, cannot set ARCHIVE mode
  e_scan_client_mode  DataMode;	// accept REPLAY sourced data
  virtual void    SetDataMode(e_scan_client_mode newmode);	// try to set data mode
  virtual e_scan_client_mode GetDataMode() { return DataMode; };	// get data mode
  virtual void    DataModeChanged();	// try to set data mode
  virtual bool    ScanSourceOK(rdr_scan *srcscan);	//returns true if scan is from an appropriate source
  SCANCLIENTTYPE  ClientType;	 // type of client
  virtual void    setRdrImgTmWin(time_t newtmwin)
    { rdrimg_tmwin = newtmwin; };  // set time window to qualify for same image
  time_t          rdrImgTmWin() { return rdrimg_tmwin; };
  bool	          ignoreFutureData;	// if true, ignore scans that have "future" time stamps
  time_t	  futureDataTmWin;	// time window for "future" data
  virtual bool    MergeScans();	// true if scan client is merging scans flag

  /* most clients should use either NewScanAvail OR CompleteScanAvail
   * If both are used,  the client must have duplicate scan handling 
   */
  rdr_scan_node   *newscans, *finishedscans, *checkedscans;		// new scans, usually added by other threads 
  virtual void    incScanNodeCount();
  virtual bool    decScanNodeCount();
  int             getScanNodeCount() { return _scan_node_count; };
  CDataNode	  *newcdata, *checkedcdata;		// new scans, usually added by other threads 
  int		  cdatanode_count;	// scan node accounting check
  // add scan to new scan list. Usually called by other thread via serialised scan_mng
  virtual int	  NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
#ifdef USE_RAINFIELDS_CLIENT
  virtual bool    ScanSourceOK(Ref<rainfieldImg>& rfimg);	//returns true if scan is from an appropriate source
  bool            acceptRFImages;// NOTE: DO NOT ACCEPT RF IMAGES BY DEFAULT
  map<string, Ref<rainfieldImg> >  new_rf_Images; // temp storage for new rfimgs
  virtual int	  NewDataAvail(Ref<rainfieldImg>& rfimg); //return 1 if scan added, else 1
  virtual void    ProcessNewRFImages(); // process the new_rf_Images
  virtual void    PurgeNewRFImages(); // purge the new_rf_Images
#endif
  virtual int	  NewDataAvail(CData *newcdata); // new scan passed by ScanMng
  virtual int	  FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
  virtual int	  FinishedDataAvail(CData *newcdata); // add finished scan to new sca list. usually called by other thread
  // quickly check new scans list, add scans to keep to checkedscans, discard others
  virtual void    CheckNewData(bool MoveToChecked = FALSE, 
			       bool RejectFaulty = TRUE); // keep this quick, uses lock
  virtual void    PurgeNewData();
  /*
   * ProcessCheckedData doesn't need to be locked,  can perform 
   * time consuming operations
   */				
  virtual void    ProcessCheckedData(int maxscans = -1);// process checked scans list
  virtual void    PurgeCheckedData();
  virtual bool    Full(int passedSize = -1, bool forcecheck = true);		// client "full indication
  virtual int	  percentFull(int passedSize = -1);		// client percentage full indication
  virtual int	  NumImgs();		// client image count
  virtual bool    IsDuplicate(rdr_scan *dupscan, 	// return true if known ie already in seq or database etc
			      bool FinishedOnly = FALSE);  // if true, ignore unfinished scnas
  virtual bool    IsDuplicate(CData *dupdata, 	// return true if known ie already in seq or database etc
			      bool FinishedOnly = FALSE);  // if true, ignore unfinished scnas
  virtual void    PrintScanUsage(FILE *file = 0, bool verbose = false);	// print information on scans used, memory consumed etc.

};

class scanclient_listnode {
  scan_client	*ScanClient;
  scanclient_listnode *next, *prev;
};

enum RSC_RESULT {
    NOT_IN_CACHE,       // scan with same key not in cache
    IN_CACHE_DUPL,      // scan with same key in cache, not same rdr_scan*, i.e. another instance
    IN_CACHE_SAME_INST  // scan with same key in cache, has same rdr_scan*, i.e. same instance
    };


// registry of ALL rdr_scans resident in memory
class scan_registry
{
 public:
  spinlock	*lock;
  map<rdr_scan*, string> new_rdrscan_registry;  // map of pointers to new radar scans, may not have headers yet
  multimap<string, rdr_scan*> rdrscan_registry; // move new scans to this multimap when they have valid headers for a key
  void add_new_scan(rdr_scan *remscan, string scanstr);
  bool remove_scan(rdr_scan *addscan);
  void clear(bool deletescans = false);
  int total_scan_count(bool newscanflag = true, bool validscanflag = true);
  long total_scan_size(bool newscanflag = true, bool validscanflag = true);
  void UsageReport(FILE *file = stdout);
  void DetailedReport(char *fname = NULL);
  void check();
  scan_registry();
  ~scan_registry();
};

typedef map<string, rdr_scan*> rdrStrScanMap;
class scan_mng {
 public:
  scan_client *ClientList;
  list<scanEventClient*> scanEventClients;
  scan_mng();
  virtual ~scan_mng();
  spinlock *lock;

  // recent scan cache can exceed scan_registry depth, rdr_scan is not necessarily still in memory
  spinlock *scanCacheLock;
  bool useRecentScansCache;
  bool keepScansInCache;     // if set, increment usercount of scan added to cache, i.e. ensure  instance not deleted
  double lastCacheSize;    // 
  time_t firstCacheTime, lastCacheTime;
  time_t startupLatestTime; // latest time loaded from db on startup
  double cachedScansSize();
  rdrStrScanMap recentScans;   // cache of recent scans
  // keep the rdr_scan* to check for same instance in cache only
  // *** If keepScansInCache not set, don't use the rdr_scan* returned, 
  // as the recentCache will not increment user count, so it may have been deleted
  bool addRecentScanOK(rdr_scan *newscan); // returns false if already in cache, i.e. duplicate
  RSC_RESULT scanKeyInRecentScans(rdr_scan *newscan); // returns scan key in cache status, 
  RSC_RESULT scanInRecentScans(rdr_scan *newscan); // returns scan instance in cache status
  // i.e. NOT_IN_CACHE, IN_CACHE_DUPL or IN_CACHE_SAME_INST 
  void setRecentCachePeriod(int newcacheperiod)
    {
      keepScansInCache = true;
      recentCachePeriod = newcacheperiod; // set mins of rdr_scan keys to keep in recentScans cache
      purgeRecentScans();
    }
  int recentCachePeriod;       // mins of rdr_scan keys to keep in recentScans cache
  void purgeRecentScans(time_t purge_before_time = 0); // purge all recent scan before this time
  void clearRecentScans(); // clear all recent scan cache
  // if time not defined use current time - recentCachePeriod
/*   int  getRecentScans(int stnid, e_scan_type scan_type,  */
/* 		      int minutes, rdrStrScanMap &results); */
/*   int  getRecentScans(stnSet stns, e_scan_type scan_type,  */
/* 		      int minutes, rdrStrScanMap &results); */
  // return number of minutes actually loaded
  int  getRecentScans(int minutes, time_t getfrom, 
		       scan_client *client, bool reverseorder = false);
  time_t mostRecentScanTime(); // return time of the most recent scan
  int   cachePercentFull();  // percent full based on last-first time and recentCachePeriod
  time_t CheckTime, CheckPeriod;
  void Check();
  e_scan_client_mode currentDataMode;
  void NewDataAvail(rdr_scan *new_scan);	// simply add new scan to newscans list
#ifdef USE_RAINFIELDS_CLIENT
  void NewDataAvail(Ref<rainfieldImg>& rfimg); //return 1 if scan added, else 1
#endif
  void NewDataAvail(CData *new_data);		// simply add new scan to newscans list
  void FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
  void FinishedDataAvail(CData *finisheddata);// add finished scan to new sca list. usually called by other thread
  void AddClient(scan_client *new_client);
  void RemClient(scan_client *rem_client);
  void AddEventClient(scanEventClient *new_client);
  void RemEventClient(scanEventClient *rem_client);
  bool IsDuplicate(rdr_scan *new_scan,	// see if any scan clients already have this scan
		   bool FinishedOnly = FALSE);	// if true, ignore unfinished scnas
  bool IsDuplicate(CData *newdata,	// see if any scan clients already have this scan
		   bool FinishedOnly = FALSE);	// if true, ignore unfinished scnas
  bool	CheckDuplicates;	// if true try to remove duplicate scans
  bool  reportDuplicates;
  void    SetDataMode(e_scan_client_mode newmode);  // will traverse clients and turn on ReplayMode
  e_scan_client_mode GetDataMode() { return currentDataMode; };  // get last set data mode
  // scan clients must have both AllowReplayData AND Replay mode true to accept replay data
  void get_lock();
  void rel_lock();
  virtual void    PrintScanUsage(FILE *file = 0, bool verbose = false);	// print information on scans used, memory consumed etc.
  void NewSeqLoaded();
  bool appClosing;
};

char *scanTypeText(rdr_scan *scan);
	
extern scan_mng *ScanMng;
extern scan_registry *ScanRegistry;

#endif	/* __RDRSCAN_H */
