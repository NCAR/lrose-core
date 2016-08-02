#ifndef	__VIL_H
#define __VIL_H

/* 
    vil.h
        
*/

#include "rdr.h"
#include "rdrutils.h"
#include "rdrscan.h"

#define VMAXELEVCOUNT 30

class VILUtils : public scanProductCreator {
    float	    Elevs[VMAXELEVCOUNT];	/* elevs in ascending order */
    float	    SinEl[VMAXELEVCOUNT];	/* sin values for elevs */
    float	    CosEl[VMAXELEVCOUNT];	/* cos values for elevs */
    rdr_scan	    *Scans[VMAXELEVCOUNT];	/* pointers to vol scans */
    rdr_scan	    *rootScan;
    int		    ElevCount;			/* number of elevs */
    bool	    ScanSetDefined;
    LevelTable	    *LevelTbl;
    float	    hailLimit;	// dbz hail limit
    float	    maxVIL;	// max VIL level
    int		    threshLevels;   // number of levels for thresholding
public:
    int		    get_vil_radl_angl(s_radl* VILradl, rdr_angle Az);// returns -1 if failed
    // get and unpack radial at given angle. If not found get next
    int	get_test_radl_angl(s_radl* VILradl, rdr_angle Az);
    
    void	    ScaleVILRadl(s_radl *vilradl, 
	    int numlevels = 0, 
	    float maxval = 0);
    VILUtils();
    virtual ~VILUtils();
    void LoadScanSet(rdr_scan *rootvolscan, float dBZHailLimit = 0);	 /* initialise with given scan set */
    float dBZtoZ47(float dBZ);
    float VIL_M_Z47(float dh, float Z1,  float Z2);
    float VIL_M_dBZ(float dh, float dBZ1,  float dBZ2);

    // creates a scan product from the  passed src_scan
    // if scansetroot is passed the product is added to that scanset
    virtual rdr_scan* createScanProduct(rdr_scan *src_scan, rdr_scan *scansetroot = NULL);

    void setVILRdrScanParams(float maxval = 0, 
	    int numlevels = 0, 
	    float dBZHailLimit = 0);	
    rdr_scan* makeVILRdrScan(rdr_scan *srcscan,
	    float maxval = 0, 
	    int numlevels = 0, 
		float dBZHailLimit = 0);	// build a rdr_scan based on current settings
};

#endif	/* __VIL_H */
