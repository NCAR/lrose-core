#ifndef	__RDRFILTER_H
#define __RDRFILTER_H

/*
 * rdrfilter.h	
 *  
 *	Class definition for scan filter which takes radar data 
 *	and median filters it into a new rdr_scan
 */


#include "rdrscan.h"

class rdrfilter : public scanProductCreator {
    int debug;
  
    unsigned char *src_array, *dest_array;
    int		src_xdim, src_ydim, dest_xdim, dest_ydim;
    
    bool	valid;
    int		scan_vol_no;	    //scan no. in this volume
    int		minNeighbors;	    // set min neighbors for filter

public:
    rdrfilter();
    ~rdrfilter();

    rdr_scan* 	MakeFilteredScan(rdr_scan *srcscan, rdr_scan *filteredscanset = NULL);
    void	filterArray();
    unsigned char filterPt(unsigned char src_array[], int i, int j, int prev_j, int next_j);
    void	fetchPts9(unsigned char src_array[], int i, int j, int prev_j, int next_j, int PtArr[]);
    int 	mode(int sumArr[],int sumInd);
    int 	median(int arr[],int n);
    // creates a scan product from the  passed src_scan
    // if scansetroot is passed the product is added to that scanset
    virtual rdr_scan* createScanProduct(rdr_scan *src_scan, rdr_scan *scansetroot = NULL);
};   
        
#endif
