#ifndef	__RDRXLAT_H
#define __RDRXLAT_H

/*
 * rdrxlat.h	
 *  
 *	Class definition for scan client which takes radar data from
 *	one radar site and "translates" it to another origin
 *	Data translation is done in the polar domain
 */

#include "rdrscan.h"
#include <math.h>

#define  AZINC  30 /* degrees of azimuth increment (360/n_az ?) */
#define  BINLEN 1 /* metres */
#define  PI  3.14159265358979323846  /* pi *//* copy from math.h */
#define  DEG2RAD (PI/180.0)


class rdr_xlat : public scan_client {

    int		source_stnid;
    int		dest_stnid;
    unsigned char   *src_array, *dest_array;
    int		src_xdim, src_ydim, dest_xdim, dest_ydim;
    rdr_scan	*SrcScan, *xlat_scan; 
    bool	valid;
    bool	ApplyTestPattern;

    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual void    ProcessCheckedData(int maxscans = -1);// process checked scans list

    
    void	doXlat(rdr_scan_node *src_scan);    // xlat array into new scan and pass it to scanmng
    void	relocate(unsigned char *src, unsigned char *dest, int n_az, 
		    int src_n_bins, int dest_n_bins,
		    float rng_res, float src_start_rng, float dest_start_rng, 
		    double dest_ofs_x, double dest_ofs_y);
    void	XlatArray();	// actually do translation from src_array to dest_array
    void	init(int src_stnid, int dest_stnid);    
public:
    rdr_xlat(int src_stnid, int dest_stnid);
    rdr_xlat(char *initstring);	// text init string "xlatdevice=src_stnid dest_stnid"
    ~rdr_xlat();
    void	threadInit();
    void	workProc();		// perform any "work" that needs doing, called repeatedly by runloop()
    bool	isValid() {return valid;};	// return if stn ids are valid
};   
        
#endif
