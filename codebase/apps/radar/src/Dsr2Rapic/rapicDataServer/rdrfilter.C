/*
 * rdrfilter.C
 * 
 * Implementation of rdrfilter class
 * 
 */
 
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "rdrfilter.h"
#include "rdrutils.h"
#include "siteinfo.h"
#include <stdlib.h>
static char const cvsid[] = "$Id: rdrfilter.C,v 1.2 2008/02/29 04:44:54 dixon Exp $";


rdrfilter::rdrfilter()
{
char *tempstr;

    debug = 1;
  
    src_array = 0; 
    dest_array = 0;
    src_xdim = src_ydim = dest_xdim = dest_ydim = 0;
    valid = true;
    if ((tempstr = getenv("FILTER_MIN_NEIGHBOR")))
    {
	if (sscanf(tempstr, "%d", &minNeighbors) != 1)
	    minNeighbors = 1;
    }
    else
	minNeighbors = 1;
}

rdrfilter::~rdrfilter()
{
    fprintf(stderr,"rdrfilter::~rdrfilter\n");  
    if (src_array) {
       delete[] src_array;
       src_array = 0;
    }
    if (dest_array) {
       delete[] dest_array;
       dest_array = 0;
    }
}

void rdrfilter::filterArray()
{
    int prev_az, az, next_az, 
      rng;
    //    unsigned char *p = src_array;
    // float km_n, km_e;
    bool dofilter;
    if (!dest_array)
      return;
    for (az = 0; az < src_ydim; az++)  
    {
      if (az == 0)
	prev_az = src_ydim - 1;
      else
	prev_az = az -1;
      if (az == src_ydim - 1)
	next_az = 0;
      else
	next_az = az + 1;
      for (rng = 0;  rng < src_xdim; rng++) //avoid boundaries
	{
	    if ((rng == 0) || (rng == src_xdim-1))
		dofilter = false;
	    else
		dofilter = true;
	  if (src_array[az*src_xdim+rng] && dofilter)
	    dest_array[az*src_xdim+rng] = filterPt(src_array, rng, az, prev_az, next_az);
	  else
	    dest_array[az*src_xdim+rng] = src_array[az*src_xdim+rng];
	}
/*
	src_array[az*src_xdim+1] = 80;
	src_array[az*src_xdim+100] = 100;
	src_array[az*src_xdim+200] = 200;
*/
    }
}
    
unsigned char rdrfilter::filterPt(unsigned char src_array[], int i, int j, int prev_j, int next_j)
{
  int ptArr[9];
  int ptArr2[10];
  int pt, count = 0;
  uchar result;
  //  bool resultok = false;
  //  bool debug = true;

    fetchPts9(src_array, i, j, prev_j, next_j, ptArr);
    for (pt = 0; pt < 9; pt++)
    {
	if (ptArr[pt]) // copy non-zero pts to ptArr2 **MUST START AT 1 FOR median fn!!!**
	    ptArr2[(count++ + 1)] = ptArr[pt];
    }
    if (count > minNeighbors)
      {
	result = (uchar)(median(ptArr2,count) & 0xff);
	/*
	if (debug) // check result value is in original array 
	  {
	    for (pt = 0; pt < 9; pt++)
	      if (ptArr[pt] == result)
		{
		  resultok = true;
		  break;
		}
	    if (!resultok) // check result value is in ptArr2 array 
	      {
		for (pt = 0; pt < count; pt++)
		  if (ptArr2[pt+1] == result)
		    {
		      resultok = true;
		      break;
		    }
	      }
	  }
	*/
	return result;
      }
    else
	return (uchar)ptArr[0];	// if <= minNeighbors non-zero pts, return centre
//	return 0;	// if < 3 non-zero pts, return 0
}
    
void rdrfilter::fetchPts9(unsigned char *src_array, int i, int j, int prev_j, int next_j, int PtArr[])
{
  unsigned char *centre = &(src_array[j*src_xdim+i]); // pointer to centre value
  unsigned char *prev_centre = &(src_array[prev_j*src_xdim+i]); // pointer to prev "line" (radial) centre value
  unsigned char *next_centre = &(src_array[next_j*src_xdim+i]); // pointer to next "line" (radial) centre value

  //if (j !=0 && j != src_ydim-1)
      {
	PtArr[0] = (int)(*(centre) & 0xff);  //centre
	PtArr[1] = (int)(*(next_centre) & 0xff);
	PtArr[2] = (int)(*(prev_centre) & 0xff);
	PtArr[3] = (int)(*(centre+1) & 0xff);
	PtArr[4] = (int)(*(centre-1) & 0xff);
	PtArr[5] = (int)(*(next_centre+1) & 0xff);
	PtArr[6] = (int)(*(prev_centre-1) & 0xff);
	PtArr[7] = (int)(*(next_centre-1) & 0xff);
	PtArr[8] = (int)(*(prev_centre+1) & 0xff);
      }
}

int rdrfilter::mode(int sumArr[],int sumInd)
  {
    //return the most commonly used member of the array,  assume all members are from bytes
    int hist[256];
    int max = 0, highest = 0, med = 0, i;
    
    for (i=0;i<sumInd;i++)
      {
	if (sumArr[i] < 256) hist[sumArr[i]]++;
	if (sumArr[i] > max) max = sumArr[i];
      }
    if (max > 255) max = 255;
    
    //find highest hist
    for (i=0;i<=max;i++)
      if (hist[i] >= highest) // >= so we get the last highest
	{
	  highest = hist[i];
	  med = i;
	}
    return med;
  }

int rdrfilter::median(int arr[],int n)
  {
#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

/*
Returns the kth smallest value in the array arr[1..n]. The input array 
will be rearranged
to have this value in location arr[k], with all smaller
elements moved to arr[1..k-1] (in arbitrary order) and all 
larger elements in arr[k+1..n] (also in arbitrary order).
*/

    if (n < 1)        // illegal value for n
      return 0;

    int k = (n / 2) + 1; // array values start at one, prev k=n/2 was incorrect
    
  int i,ir,j,l,mid;
  int a,temp;
  l=1;
  ir=n;
  for (;;) {
    if (ir <= l+1) { //Active partition contains 1 or 2 elements.
      if (ir == l+1 && arr[ir] < arr[l]) { //Case of 2 elements.
        SWAP(arr[l],arr[ir])
          }
      return arr[k];
    } else {
      mid=(l+ir) >> 1; 
      //Choose median of left, center, and right el-
      //ements as partitioning element a. 
      //Also rearrange so that arr[l] <= arr[l+1],arr[ir] >= arr[l+1].
      SWAP(arr[mid],arr[l+1])
        if (arr[l] > arr[ir]) {
          SWAP(arr[l],arr[ir])
            }
      if (arr[l+1] > arr[ir]) {
        SWAP(arr[l+1],arr[ir])
          }
      if (arr[l] > arr[l+1]) {
        SWAP(arr[l],arr[l+1])
          }
      i=l+1; //Initialize pointers for partitioning.
      j=ir;
      a=arr[l+1]; //Partitioning element.
      for (;;) { //Beginning of innermost loop.
        do i++; while (arr[i] < a); //Scan up to find element > a.
        do j--; while (arr[j] > a); //Scan down to find element < a.
        if (j < i) break; //Pointers crossed. Partitioning complete.
        SWAP(arr[i],arr[j])
          } //End of innermost loop.
      arr[l+1]=arr[j]; //Insert partitioning element.
      arr[j]=a;
      if (j >= k) ir=j-1; //Keep active the partition that contains the kth element.
      if (j <= k) l=i;
    }
  }
}

rdr_scan* rdrfilter::createScanProduct(rdr_scan *srcscan, rdr_scan *scansetroot)
{
  rdr_scan *filter_scan = 0;  

  if (!srcscan)
    return scansetroot;
    
  if (src_array) {
    delete[] src_array;
    src_array = 0;
  }
  src_xdim =  src_ydim = 0;
  //    int first = 0;
  
  //    fprintf(stderr, "rdrfilter::MakeFilteredScan starting for %s\n", srcscan->ScanString());
  src_array = srcscan->Create2DPolarArray(&src_xdim, &src_ydim);
  if (!src_array)
    return scansetroot;
  if (dest_array && (src_xdim * src_ydim != dest_xdim * dest_ydim)) {	
    // wrong dest array size
    delete[] dest_array;
    dest_array = 0;
    dest_xdim = dest_ydim = 0;
  }
  if (!dest_array) {
    dest_xdim = src_xdim;
    dest_ydim = src_ydim;
    dest_array = new unsigned char[dest_xdim * dest_ydim];
  }
  memset(dest_array, 0, dest_xdim * dest_ydim);
    
  filterArray();
    
  //embed new filter_scan into output linked list
    
  if (!scansetroot)
    {
      scansetroot = new rdr_scan(this, "rdrfilter::MakeFilteredScan");
      scansetroot->set_dflt(srcscan);
    }
  scansetroot->add_line("FILTERTYPE:MEDIAN"); // mark this scan as filtered
  filter_scan = scansetroot->EncodeDataFrom2DPolarArray(dest_array, dest_xdim, dest_ydim, srcscan);
  if (filter_scan)
    {
      filter_scan->setCreator(this);
      filter_scan->num_scans = -1;  // -1 will turn off product update on vol scan delta
      filter_scan->FirstTm = srcscan->FirstTime();
    }
  if (src_array) {
    delete[] src_array;
    src_array = 0;
  }
  if (dest_array) {
    delete[] dest_array;
    dest_array = 0;
  }
  return filter_scan;
}	

rdr_scan* rdrfilter::MakeFilteredScan(rdr_scan *srcscan, rdr_scan *filteredscanset)
{
  return createScanProduct(srcscan, filteredscanset);
}
