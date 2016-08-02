#ifndef	__MDV_H
#define __MDV_H

/* 
    cappi.h
        
*/

#include "rdr.h"
#include "rdrutils.h"
#include "rdrscan.h"

#define CMAXELEVCOUNT 30

struct MDVRngPair {
	float StartRng, EndRng;		/* end rng redundant */
};

enum MDVmode { MDVNearest,  MDVInterp };

float IntersectRng(float el,  float ht);

class MDVElevSet {
protected:
    int		    debug;
    s_radl 	    *TempRadl;
    int 	    maxrng,minrng;
    float	    Elevs[CMAXELEVCOUNT];	/* elevs in ascending order */
    float	    SinEl[CMAXELEVCOUNT];	/* sin values for elevs */
    float	    CosEl[CMAXELEVCOUNT];	/* cos values for elevs */
    rdr_scan	    *Scans[CMAXELEVCOUNT];	/* pointers to vol scans */
    exp_buff_rd	    *dbuffrd;			// rdrscan buffer state object
    short	    radlpos;			// rdrscan buffer state object
    int		    ElevCount;			/* number of elevs */
    MDVmode	    mode;
    bool	    RngSetDefined;
    float	    RngSetHeight;		/* floating pt. km height */
    float	    MDVHeight;		/* desired MDV height */
    float	    RngSetStartRng;
    MDVRngPair      RngSet[CMAXELEVCOUNT];	/* Rng where index is closest */
    MDVRngPair      ElevSet[CMAXELEVCOUNT];
//    s_radl	    thisscanradl, prevscanradl;
    void	    CalcNearestSet();		/* calc nearest rng set, height in kms*/
    void	    CalcInterpSet();		/* calc interp set, height in kms*/
    void	    CalcRngSet();		/* calc appropriate rng set for mode */
public:
    MDVElevSet();
    ~MDVElevSet();
    void	    SetMode(MDVmode newmode);
    void	    SetHeight(float newheight);	/* new height in km */
    float	    GetHeight();
    MDVmode	    GetMode();
    int		    get_radl_angl_nearest(s_radl* MDVradl, rdr_angle Az);// returns -1 if failed
    int		    get_radl_angl_interp(s_radl* MDVradl, rdr_angle Az);// returns -1 if failed
    int		    get_radl_angl(s_radl* MDVradl, rdr_angle Az);// returns -1 if failed
    // get and unpack radial at given angle. If not found get next
    int  get_radl_size();
    void            LoadScan(rdr_scan *volscan);
    void            ResetScan();
    void	    LoadScanSet(rdr_scan *rootvolscan);	 /* initialise with given scan set */
    e_data_type	    data_type;			/* data type for display */
};

#endif	/* __MDV_H */
