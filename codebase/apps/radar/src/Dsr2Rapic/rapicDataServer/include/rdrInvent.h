#ifndef	__RDRINVENT_H
#define __RDRINVENT_H

/*
 * rdrInvent.h	
 *  
 *	Class definition for scan filter which takes radar data 
 *	and median filters it into a new rdr_scan
 */


#include "rdrscan.h"

class rdrInvent : public scanProductCreator {
    int debug;
  
    unsigned char *src_array, *dest_array;
    int		src_xdim, src_ydim, dest_xdim, dest_ydim;
    
    //bool	valid;
    int		scan_vol_no;	    //scan no. in this volume
    int		minNeighbors;	    // set min neighbors for filter
    int		newRange;
    int		s1fd; //output file handle
    int		scancount;
    int		dummy;
    

public:
    rdrInvent(int fd);
    ~rdrInvent();

    //rdr_scan* 	MakeInventedScan(rdr_scan *srcscan, rdr_scan *inventedscanset = NULL);
    void expandArray();
    void reverseVelocity(rdr_scan *scan);
    
    //unsigned char filterPt(unsigned char src_array[], int i, int j, int prev_j, int next_j);
    // void	fetchPts9(unsigned char src_array[], int i, int j, int prev_j, int next_j, int PtArr[]);
    // int 	mode(int sumArr[],int sumInd);
    //int 	median(int arr[],int n);
    // creates a scan product from the  passed src_scan
    // if scansetroot is passed the product is added to that scanset
    virtual void createScanProduct(rdr_scan *src_scan);
};   
        
#endif
