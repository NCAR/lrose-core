/*
 * rdrInvent.C
 * 
 * Implementation of rdrInvent class
 *    adapted from rdrfilter.C  SD 17/2/05
 * 
 */
 
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "rdrInvent.h"
#include "rdrutils.h"
#include "siteinfo.h"
#include <stdlib.h>
static char const cvsid[] = "$Id: rdrInvent.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";


rdrInvent::rdrInvent(int fd)
{
    //char *tempstr;

    debug = 1;
  
    src_array = 0; 
    dest_array = 0;
    src_xdim = src_ydim = dest_xdim = dest_ydim = 0;
    //valid = true;
    newRange = 1000;  //no of gates, set to 1000 for S1
    s1fd = fd;
    scancount = 1;  //yes, starts from 1 not 0
    
    
}

rdrInvent::~rdrInvent()
{
    fprintf(stderr,"rdrInvent::~rdrInvent\n");  
    if (src_array) {
       delete[] src_array;
       src_array = 0;
    }
    if (dest_array) {
       delete[] dest_array;
       dest_array = 0;
    }
}

void rdrInvent::expandArray()
{
    int  az,  rng;
    //    unsigned char *p = src_array;
    // float km_n, km_e;
    if (!dest_array)
      return;
    for (az = 0; az < src_ydim; az++)      
    {
      for (rng = 0;  rng < src_xdim; rng++) 
	{
	  dest_array[az*dest_xdim+rng] = src_array[az*src_xdim+rng];
	}
      
      for (;  rng < dest_xdim; rng++) 
	{
	  dest_array[az*dest_xdim+rng] = src_array[az*src_xdim+2*src_xdim - rng-1];  //reflection from edge
	  //dest_array[az*dest_xdim+rng] = src_array[az*src_xdim+rng - src_xdim];   //again from the center
	  //this probably wont work,  need to go down to actual value, not the code
	  //repeat difference!
	  //dest_array[az*dest_xdim+rng] = src_array[az*src_xdim+2*src_xdim - dest_xdim + rng]; 
	  //dest_array[az*dest_xdim+rng] = 0;  
	}
      //should a double zero be written at the end??
      /*
      for (rng = dest_xdim-4;  rng < dest_xdim; rng++) 
	{
	  dest_array[az*dest_xdim+rng] = 0;
	}
      */
    }
}
    

void rdrInvent::reverseVelocity(rdr_scan *scan)
{
  //reverse velocity for early Mt Stapylton data SD 20/3/2006
    int  az,  rng;
    float vel;
    if (scan->NumLevels == 0) printf("ERROR: Numlevels zero!!!\n");
    
    if (!dest_array)
      return;
    for (az = 0; az < src_ydim; az++)      
    {
      for (rng = 0;  rng < src_xdim; rng++) 
	{
	  //get real velocity value, reverse it, encode it back
	  //  if special value, leave as is.
	  if (src_array[az*src_xdim+rng] > scan->NumLevels -1  ||
	      src_array[az*src_xdim+rng] == 0)
	    dest_array[az*dest_xdim+rng] = src_array[az*src_xdim+rng];
	  else {
	    vel = (src_array[az*src_xdim+rng] - scan->NumLevels/2.0)*scan->nyquist/(scan->NumLevels/2.0 -1.0);
	    dest_array[az*dest_xdim+rng] = (int)((-1.0*vel*(scan->NumLevels/2-1))/scan->nyquist + scan->NumLevels/2);
	  }
	  
	}
    }
}
    
void rdrInvent::createScanProduct(rdr_scan *srcscan)
  //vol_scan_no ,vol_scans
  //invent an S1 scan with extra moment uncorrrefl duplicated from refl,  and extra bins
  //being reflections of orig outer edge
{
  //switch 3 moments or not
  //bool addMoment = true;
  bool addMoment = false;
  char headerStr[100];
  int headerStrLen = 0;
  int scan_ind = 0;
  int scan_ind_out = 1;
  rdr_scan *temp_scan = 0; //for elevation loop
  
  
  rdr_scan *invented_scan = 0;  
 
  if (!srcscan)
    return ;
    
    if (src_array) {
	delete[] src_array;
	src_array = 0;
    }
    
    src_xdim =  src_ydim = 0;
    
    //    int first = 0;
  
//    fprintf(stderr, "rdrInvent::MakeFilteredScan starting for %s\n", srcscan->ScanString());
    src_array = srcscan->Create2DPolarArray(&src_xdim, &src_ydim);
    //fprintf(stderr, "rdrInvent:createScanProduct: src_xdim, %d. src_ydim %d\n",src_xdim, src_ydim);

    if (!src_array)
      return ;
    /*
    if (dest_array && (src_xdim * src_ydim != dest_xdim * dest_ydim)) {	
      // wrong dest array size
      delete[] dest_array;
      dest_array = 0;
      dest_xdim = dest_ydim = 0;
    }
    */
    if (!dest_array) {
      //dest_xdim = src_xdim+400;   //new output size,  parametize this!
      //dest_xdim = newRange;   //new output size,  parametize this!
      dest_xdim = src_xdim;   //new output size,  parametize this!
      dest_ydim = src_ydim;
      dest_array = new unsigned char[dest_xdim * dest_ydim];
    }
    memset(dest_array, 0, dest_xdim * dest_ydim);
    
    //expandArray used to generate simulated S1 radar data,  2005.
    //expandArray();
    //reverse velocity, size same   SD 20/3/2006
    if (srcscan->data_type == e_vel) reverseVelocity(srcscan);
    else memcpy(dest_array,src_array,dest_xdim * dest_ydim);
    
    
    //fprintf(stderr, "rdrInvent:createScanProduct: srcscan max_rng %d\n",srcscan->max_rng );
    invented_scan = new rdr_scan(this, "rdrInvent::createScanProduct");
    invented_scan->set_dflt(srcscan);
    //invented_scan->max_rng = (srcscan->start_rng + newRange*srcscan->rng_res)/1000;

    //readjust scan counts for new moment
    //assume there is velocity,  but that we dont know number of elevations
    if (addMoment)
      invented_scan->vol_scans = invented_scan->vol_scans*3/2; //it should be even to start with if vel present
    invented_scan->vol_scan_no = scancount++;
    
    //write out /IMAGE headers, not used by rapic, but by rapic2keyvals.pl
    if (srcscan->isRootScan())
      {
	headerStrLen = sprintf(headerStr,"\n/IMAGE: %4d %02d%02d%02d%02d%02d\n/RXTIME:\n/IMAGESCANS: %d\n/IMAGESIZE:\n",
	       srcscan->station,
	       srcscan->year%100,
	       srcscan->month,
	       srcscan->day,
	       srcscan->hour,
	       srcscan->min,
	       invented_scan->vol_scans);
	if (!write(s1fd,headerStr,headerStrLen)) 
	  fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing /IMAGE: line\n");
	//write out elevations
	scan_ind = 0;
	scan_ind_out = 1;
	
	while ((temp_scan = srcscan->gotoScan(scan_ind++))) {
	  //elev in set_angle
	  headerStrLen = sprintf(headerStr,"/SCAN %4d: %4d %02d%02d%02d%02d%02d %2d %5.1f %2d %2d\n",
	       scan_ind_out++,	 
	       temp_scan->station,
	       temp_scan->year%100,
	       temp_scan->month,
	       temp_scan->day,
	       temp_scan->hour,
	       temp_scan->min,
	       temp_scan->scan_type,
	       temp_scan->set_angle/10.,
	       temp_scan->data_type,
	       temp_scan->data_fmt );
	  if (!write(s1fd,headerStr,headerStrLen)) 
	    fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing /SCAN: line\n");
	  if (addMoment && temp_scan->data_type == e_refl) {
	    headerStrLen = sprintf(headerStr,"/SCAN %4d: %4d %02d%02d%02d%02d%02d %2d %5.1f %2d %2d\n",
	       scan_ind_out++,	 
	       temp_scan->station,
	       temp_scan->year%100,
	       temp_scan->month,
	       temp_scan->day,
	       temp_scan->hour,
	       temp_scan->min,
	       temp_scan->scan_type,
	       temp_scan->set_angle/10.,
	       e_rawrefl,
	       temp_scan->data_fmt );
	    if (!write(s1fd,headerStr,headerStrLen)) 
	      fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing /SCAN: line\n");
	  }
	}
	//finish with end header /IMAGEHEADER END:
	if (!write(s1fd,"/IMAGEHEADER END:\n\0",18)) 
	  fprintf(stderr, "rdrInvent.createScanProduct: ERROR writing : /IMAGEHEADER END: line\n");
      }
    //end if rootscan 

    invented_scan->WriteDataFrom2DPolarArray(s1fd, dest_array, dest_xdim, dest_ydim); //0 was srcscan!
    
    if (addMoment && srcscan->data_type == e_refl) {
      //duplicate this as emulated uncorrected refl
      invented_scan->data_type = e_rawrefl;
      invented_scan->vol_scan_no = scancount++;
      invented_scan->WriteDataFrom2DPolarArray(s1fd, dest_array, dest_xdim, dest_ydim);
    }
    
    if (invented_scan->ShouldDelete(this, "rdrInvent::createScanProduct"))
      delete invented_scan;
    
// **debug code**    invented_scan = scansetroot->EncodeDataFrom2DPolarArray(src_array, dest_xdim, dest_ydim, srcscan);
//    fprintf(stderr, "rdrInvent::MakeFilteredScan finished for %s\n", srcscan->ScanString());
    if (src_array) {
	delete[] src_array;
	src_array = 0;
    }
    if (dest_array) {
      delete[] dest_array;
      dest_array = 0;
      }
    return ;
}	


