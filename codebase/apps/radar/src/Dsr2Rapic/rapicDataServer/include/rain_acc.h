#ifndef	__ACCUM_H
#define __ACCUM_H

#include "rdrscan.h"
#include "cappi.h"
#include "AccumProdWindowClass.h"
#include "AccumMngWindowClass.h"

/*
 * Default accum_buff 360 radials @ 1000m res to 256km
 */

enum accum_scans_types {ACC_ANY, ACC_VOLONLY, ACC_COMPPPI_ONLY};
enum accum_mode {ACC_ONTHEHOUR, ACC_RUNNING};

#define ACC_DFLTCAPPIHT 1500	// default cappi height (metres)
#define ACC_CAPPIUSEDEFAULT -1	// use default cappi height

class AccumProdWindowClass;	// forward declaration
class AccumMngWindowClass;	// forward declaration
class rain_accum;

class accum_product{
    accum_scans_types scanTypes;	// default to any scans, may specify vol only
public:
    int		StnID;
    int		Period;
    accum_mode	Mode;	    // default to ACC_RUNNING
    int		CappiHt;    // cappi height to use in metres
    float	RainRateClip;	// clipping value for rains rates (mm/hr)
    accum_product *next, *prev;
    bool	enabled;
    rain_accum  *RainAccum;
    string _prodKey;
    string _fullProdString;
    string _shortProdString;
    accum_product(bool cachecappis = false, int stnid = 0, int period = 3600,  
	accum_scans_types scans_types = ACC_ANY, 
	accum_mode mode = ACC_RUNNING, 
	int cappiht = ACC_DFLTCAPPIHT, 
	float rainrateclip = 0);
    ~accum_product();
    int IsSame(int stnid, int period, 
	accum_scans_types scans_types = ACC_ANY, 
	accum_mode mode = ACC_RUNNING, 
	int cappiht = ACC_CAPPIUSEDEFAULT, 
	float rainrateclip = 0);
    bool LT(accum_product *otheraccumproduct); // return true if this < other
    bool GT(accum_product *otheraccumproduct); // return true if this > other
    bool EQ(accum_product *otheraccumproduct); // return true if this == other

    accum_scans_types ScanTypes() { return scanTypes; };
    void setScanTypes(accum_scans_types newtype) 
      { scanTypes = newtype; };
    int ScanMatch(rdr_scan *matchscan);
    void String(char *outstring);	    // readable string value
    void EncodeString(char *outstring);	    // return accum.ini string value
    bool DecodeString(char *instring);   // initialise this to string value
    AccumProdWindowClass *editor;
    void Edit();		// launch a GUI editor for this classes properties
    string makeProdKey();
    string prodKey();
    void makeProdStrings();
    string prodString(bool shortstring = false);
    const char* prodCharString(bool shortstring = false);
};


class accum_buff {
public:
    // allocate buffer
    accum_buff(int rngcells = 256,  int radls = 360, float rngres = 1000);    
    ~accum_buff();  // delete buffer
    void    accum_radl(s_radl *accum_radl, int secs, float &MaxVal, LevelTable *RTable = 0);
    void    ResetRadl(LevelTable *threshtable = 0);
    void    Clear();				//
    s_radl* ThisRadl(char *encodedradl, int maxopsize);	// if output defined, threshold and encode to output
    s_radl* StepRadl(char *encodedradl, int maxopsize);
    int	    RngCells, Radls;
    float   RngRes;
    int     buffsize;
    rdr_angle	angle_res;
    s_radl  *thisradl;
    int	    thisradlnum;
    float   *p_fradl;
    float   *buff;
    };

class accum_scan_data {
public:
    accum_scan_data *next, *prev;
    bool	Valid;
    int		StnID;
    time_t	scan_time_t;    
    e_scan_type	scan_type;
    e_data_fmt	data_fmt;
    int		accum_secs;
    int		CappiHt;
    float	RainRateClip;
    float	MaxVal;
    rdr_scan    *cappiScan;  // reference to cappi scan for this scan
    bool        usedFlag;      // flag to help manage unused accum_scan_data entries
    accum_scan_data(rdr_scan *accumscan = 0, int secs = 0, 
		int cappiht = 0, float rainrateclip = 0, 
		float maxval = 0, bool cacheCAPPIs = false);
    ~accum_scan_data();
    void Set(rdr_scan *accumscan, int secs, int cappiht, 
	     float rainrateclip, float maxval, bool cacheCAPPIs);
    void Clear(void *cappiscancreator);
    bool match(int stnid, time_t scantimet,
	       int cappiht, float rainrateclip);
    void WriteString(char *outstring = 0);   // write string to outstring
    void ReadString(char *instring);
    };

/*
 * Default rain_accum 360 radials @ 1000m res to 256km
 */

class rain_accum {
public:
    rain_accum	*next, *prev;
    accum_buff	*accum;	    // accumulator buffer   
    rdr_scan	*prev_scan, *next_scan;
    /*
     * Nodes added as needed. i.e. Valid flag set.
     * End of current list is next=0 or next->Valid == FALSE
     */
    accum_scan_data *ScanDataList, *ThisScanData;
    int         scanDataListSize;
    bool        cacheCAPPIs;
    int         cachedCAPPICount();
    int         cachedCAPPISize();
    time_t	StartTime, EndTime;
    float       rng_res;
    float	start_rng;
    float	max_rng;
    int		StnID;
//    LevelTable	*STDThreshTable, *ThreshTable, *RTable;
    LevelTable	*STDThreshTable, *RTable;
    float	MaxVal;
    bool	AutoScale;
    accum_scans_types scans_types;	// default to any scans, may specify vol only
    CAPPIElevSet *makeCAPPI;
    int		CappiHt;
    float	RainRateClip;
    void	ClearScanDataList();
    accum_scan_data *AddScanData(rdr_scan *accumscan, int secs, 
		int cappiht, float rainrateclip, float maxval);
    // add this scan to accum, return if OK, 0 if not added
    // cappiht in metres, if not vol, cappiht is ignored
    int		add_scan(rdr_scan *accum_scan, int accum_secs, 
		    int cappiht = ACC_CAPPIUSEDEFAULT, 
		    float RainRateClip = 0);
    int		get_next_scan_cached();
    int		get_next_scan();
    // fetch all scans required for the defined accumulation
    // initially take from db, may add check scans in memory(seq) later
    // return number of scans used
    int		make_accum(int stnid, time_t starttime, time_t endtime, 
		    int cappiht = ACC_CAPPIUSEDEFAULT, 
		    float rainrateclip = 0);
    int		make_accum(accum_product *acc_prod, rdr_scan *lastscan);
    // turn accum data into 32lvl encoded accumulation scan
    rdr_scan*	make_scan();    
    rain_accum(bool cachecappis, int rngcells = 256,  int radls = 360, float rngres = 1000);
    ~rain_accum();
    };
    
class accum_mngr : public scan_client {
public:
    accum_mngr();
    virtual ~accum_mngr();
    virtual void	workProc();
    virtual void	threadExit(); // allow thread stopped tidy up
    void	    ReadFile(char *fname = 0);	    // read the accum.ini file
    void	    WriteFile(char *fname = 0);	    // write the accum.ini file
    void	    Clear();			    // clear the accum_prods
    bool            cacheCAPPIs;
    rain_accum	    *RainAccum;
    accum_product   *AccumProd;		// list of accum products to generate
    accum_product   *NewProd;		// newly created product, added to AccumProd when edit OK hit
    int		    ProductCount;
    virtual bool AcceptDuplicate(rdr_scan *dupscan); // check whether we want this dupscan
    void            NewProdAvail();
    virtual int     NewDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual int     FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual void    CheckNewData(bool MoveToChecked = FALSE, 
				bool RejectFaulty = TRUE);	// check new scans list
    virtual void    ProcessCheckedData(int maxscans = -1);// process checked scans list
    bool	    AddAccumProduct(int stnid, 
			int period, 
			accum_scans_types types = ACC_ANY, 
			accum_mode mode = ACC_RUNNING, 
			int cappiht = ACC_CAPPIUSEDEFAULT, 
			float rainrateclip = 0);
    void	    EditAllProducts();
    void	    EditProduct(int n);	    // open the editor for product number n
    void	    EditNewProduct();	    // create NewProd with editor
    void	    Edit();
    
    AccumMngWindowClass	*editor;
    
    void	    RemoveAccumProduct(accum_product *thisaccumproduct); 
    void	    RemoveAccumProduct(int entryno); 
    void	    RemoveAccumProduct(int stnid, 
			int period, 
			accum_scans_types types = ACC_ANY, 
			accum_mode mode = ACC_RUNNING, 
			int cappiht = ACC_CAPPIUSEDEFAULT, 
			float rainrateclip = 0);

    accum_product   *firstAccumProd();
    accum_product   *lastAccumProd();

    // return first accum product matching stn
    accum_product   *FirstStnAccumProduct(int stnid); 
    // return next accum product from same stn after this, if none, return same
    accum_product   *NextStnAccumProduct(accum_product *thisaccumproduct);
    // insert accum prod into list, list is sorted in ascending order
    bool            insertAccumProd(accum_product *newprod);
    // return true if prod is in the prod list
    bool            accumProdInList(accum_product *prod);
    accum_product   *FindAccumProduct(accum_product *thisaccumproduct);
    accum_product   *FindAccumProduct(int stnid, 
			int period, 
			accum_scans_types types = ACC_ANY, 
			accum_mode mode = ACC_RUNNING, 
			int cappiht = ACC_CAPPIUSEDEFAULT, 
			float rainrateclip = 0);
    /*
     * Check if source matches defined products and return 
     * a linked list of accum scans if appropriate,  else return 0
     */
    rdr_scan_node*	MakeAccumsIfProduct(rdr_scan *srcscan);
    virtual void	PrintScanUsage(FILE *file, bool verbose = false);
    };

extern accum_mngr *AccumMng;

#endif	/* __ACCUM_H */
