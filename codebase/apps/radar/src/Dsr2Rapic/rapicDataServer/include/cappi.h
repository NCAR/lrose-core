#ifndef	__CAPPI_H
#define __CAPPI_H

/* 
    cappi.h
        
*/

#include "rdr.h"
#include "rdrutils.h"
#include "rdrscan.h"

#define CMAXELEVCOUNT 30

struct CAPPIRngPair {
  float StartRng, EndRng;		/* end rng redundant */
};

enum CAPPImode { CAPPINearest,  CAPPIInterp };

float IntersectRng(float el,  float ht);

class CAPPIElevSet : public scanProductCreator {
  float	    Elevs[CMAXELEVCOUNT+1];	/* elevs in ascending order */
  float	    SinEl[CMAXELEVCOUNT+1];	/* sin values for elevs */
  float	    CosEl[CMAXELEVCOUNT+1];	/* cos values for elevs */
  rdr_scan	    *Scans[CMAXELEVCOUNT];	/* pointers to vol scans */
  rdr_scan	    *rootScan;			/* root scan this is based on */
  int		    ElevCount;			/* number of elevs */
  int             max_bins;                   /* max number of bins in Scans */
  CAPPImode	    mode;
  bool	    RngSetDefined;
  float	    RngSetHeight;	     /* floating pt. km height */
  float	    CAPPIHeight;	     /* desired CAPPI height (km) */
  bool            CAPPIHtSet;        /* false until cappi height explicitly set */
  float	    RngSetStartRng;
  CAPPIRngPair    RngSet[CMAXELEVCOUNT+1];	/* Rng where index is closest */
  //    s_radl	    thisscanradl, prevscanradl;
  void	    CalcNearestSet();		/* calc nearest rng set, height in kms*/
  void	    CalcInterpSet();		/* calc interp set, height in kms*/
  void	    CalcRngSet();		/* calc appropriate rng set for mode */
 public:
  e_data_type	    data_type;			/* data type for display */
  void	    SetMode(CAPPImode newmode);
  void	    SetHeight(float newheight);	/* new height in km */
  float	    GetHeight();                /* get height in km */
  CAPPImode	    GetMode() ;
  bool            getHeightSet();
  /*
    void	    reset_radl();		// point to first radial
    int		    get_next_radl(s_radl* CAPPIradl,  float height);	
    // get and unpack next radial
  */

  int		    get_radl_angl_nearest(s_radl* CAPPIradl, rdr_angle Az);
  // returns -1 if failed

  int		    get_radl_angl_interp(s_radl* CAPPIradl, rdr_angle Az);
  // returns -1 if failed

  int		    get_radl_angl(s_radl* CAPPIradl, rdr_angle Az);
  // returns -1 if failed
  // get and unpack radial at given angle. If not found get next

  CAPPIElevSet();
  void	    LoadScanSet(rdr_scan *rootvolscan);	 /* initialise with given scan set */

  // build a rdr_scan based on current settings
  rdr_scan*	    makeCappiRdrScan(rdr_scan *srcscan = 0, 
				     float cappiht = -1.0);	
  virtual rdr_scan* createScanProduct(rdr_scan *src_scan, 
				      rdr_scan *scansetroot = NULL);
};

#endif	/* __CAPPI_H */
