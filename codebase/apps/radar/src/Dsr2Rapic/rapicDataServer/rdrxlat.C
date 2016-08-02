/*
 * rdrxlat.C
 * 
 * Implementation of rdr_xlat class
 * 
 */
 
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h> 
#include "rdrxlat.h"
#include "rdrutils.h"
#include "siteinfo.h"


bool rdr_xlat_testpatterntoggle = false;

 
rdr_xlat::rdr_xlat(int Src_stnid, int Dest_stnid) : scan_client()
{
    init(Src_stnid, Dest_stnid);
}

rdr_xlat::rdr_xlat(char *initstring) : scan_client()
{
    if (!initstring) 
    {
	fprintf(stderr, "rdr_xlat::rdr_xlat - FAILED,  initstring = 0\n");
	valid = false;
	return;
    }
    if (sscanf(initstring, "rdrxlat=%d %d", &source_stnid, &dest_stnid) == 2)
    {
	init(source_stnid, dest_stnid);
	if (strstr(initstring, "APPLYTESTPATTERN"))
	    ApplyTestPattern = true;
    }
    else
    {
	fprintf(stderr, "rdr_xlat::rdr_xlat - FAILED, bad initstring = %s\n", initstring);
	valid = false;
    }
    strcpy(threadName, "rdr_xlat");
}

void rdr_xlat::init(int Src_stnid, int Dest_stnid)
{
    lock = new spinlock("rdr_xlat->lock", 3000);	// 30 secs
    source_stnid = Src_stnid;
    dest_stnid = Dest_stnid;
    AcceptNewScans = FALSE;
    AcceptFinishedScans = TRUE;
    src_array = 0; 
    dest_array = 0;
    src_xdim = src_ydim = dest_xdim = dest_ydim = 0;
    xlat_scan = 0;
    valid = true;
    SrcScan = 0;
    ApplyTestPattern = false;
}

rdr_xlat::~rdr_xlat()
{
    fprintf(stderr,"rdr_xlat::~rdr_xlat: Stopping thread.\n");  //SD add 21/7/00
    stopThread();
    if (lock) {
	delete lock;
	lock = 0;
    }
    if (src_array) {
       delete[] src_array;
       src_array = 0;
    }
    if (dest_array) {
       delete[] dest_array;
       dest_array = 0;
    }
    if (xlat_scan) {
       if (xlat_scan->ShouldDelete(this, "rdr_xlat::~rdr_xlat"))
       delete xlat_scan;
       xlat_scan = 0;
    }
}

int rdr_xlat::FinishedDataAvail(rdr_scan *finishedscan)
{
    int result;
    
    if (lock) lock->get_lock();
    result = scan_client::FinishedDataAvail(finishedscan);
    if (lock) lock->rel_lock();
    return result;
}

void rdr_xlat::ProcessCheckedData(int maxscans)	// process checked scans list
{
rdr_scan_node   *tempscannode;	

    while (checkedscans) {
	tempscannode = checkedscans->next;
	if (checkedscans->scan && 
	    (checkedscans->scan->scan_type == CompPPI) &&
	    (checkedscans->scan->station == source_stnid))
	{
	    doXlat(checkedscans);   // perform translation
	}
	delete checkedscans;
	decScanNodeCount();
	checkedscans = tempscannode;
	}
}


/* assume the source is an array which is n_az x n_bins in size and we need
   to process the lot
*/
void rdr_xlat::relocate(unsigned char *src, unsigned char *dest, int n_az, 
		    int src_n_bins, int dest_n_bins,
		    float rng_res, float src_start_rng, float dest_start_rng, 
		    double dest_ofs_x, double dest_ofs_y)
{
    int i, j;    /* array location for current destination point of interest */
    double t, r; /* polar co-ordinates for destination */
    double dest_x, dest_y; /* cartesian co-ordinates for destination */
    double dest_t, dest_i_f;          /* polar angle for source data */
    int dest_radl_ofs;
    int dest_i, dest_j;      /* array location for the nearest dest point */

    double src_x, src_y;   /* cartesion co-ordinates for source */
    double src_t, src_i_f;          /* polar angle for source data */
    int src_radl_ofs;
    int src_i, src_j;      /* array location for the nearest source point */

    double cost, sint;

#ifdef RINGS
    {
        extern void dump(char *);
        int i, j;
        for(j=0; j<n_bins; j++) { /*fill columns first */
            for(i=0; i< n_az; i++) {
                src[i*n_bins + j] = 'a' + (j/3)%26;
            } 
        }
        dump(src);
    }
#endif /* RINGS */
#ifdef WEDGES
{
        extern void dump(char *);
        int i, j;
        for(i=0; i< n_az; i++) /*fill rows first */{
            for(j=0; j<n_bins; j++) { 
                src[i*n_bins + j] = 'a' + (i/3)%26;
            } 
        }
        dump(src);
    }
#endif /* WEDGES */

    /* traverse destination volume and get nearest src point */
    /* rows are the polar angle, columns are the radial distance */

    for(i=0; i<n_az; i++) {  /*traverse along the row (range) first */
	t = DEG2RAD * i;
	cost = cos(t);
	sint = sin(t);
	dest_radl_ofs = i*dest_n_bins;
	for(j=0; j<dest_n_bins; j++) {
            r = dest_start_rng + (rng_res * (j + 0.5));	// add 0.5 to get centre of range bin
            dest_x = r * cost;
            dest_y = r * sint;

            src_x = dest_x - dest_ofs_x;
            src_y = dest_y - dest_ofs_y;
            src_t = atan2(src_y, src_x);
            src_i_f = src_t / DEG2RAD;
	    if (src_i_f < -0.5) src_i_f += 360.0;
	    src_i = (int)(0.5 + src_i_f);
            src_j = (int)(0.5 + ((hypot(src_x, src_y) - src_start_rng) /
		rng_res));

            if((src_j < src_n_bins)&&(src_j >= 0)) { 
                dest[dest_radl_ofs + j] = src[src_i*src_n_bins + src_j];
            }
        }
    }
    
    /* traverse src array and xlat to dest, replace dest cell if src > dest */
    for(i=0; i<n_az; i++) {  /*traverse along the row (range) first */
	t = DEG2RAD * i;
	cost = cos(t);
	sint = sin(t);
	src_radl_ofs = i*src_n_bins;
	for(j=0; j<src_n_bins; j++) {
            r = src_start_rng + (rng_res * (j + 0.5));	// add 0.5 to get centre of range bin
            src_x = r * cost;
            src_y = r * sint;

            dest_x = src_x + dest_ofs_x;
            dest_y = src_y + dest_ofs_y;
            dest_t = atan2(dest_y, dest_x);
            dest_i_f = dest_t / DEG2RAD;
	    if (dest_i_f < -0.5) dest_i_f += 360.0;
	    dest_i = (int)(0.5 + dest_i_f);
	    dest_j = (int)(0.5 + ((hypot(dest_x, dest_y) - dest_start_rng) /
		rng_res));

            if((dest_j < dest_n_bins)&&(dest_j >= 0)&&	// only overwrite smaller value
		(dest[dest_i*dest_n_bins + dest_j] < src[src_radl_ofs + j])) { 
                dest[dest_i*dest_n_bins + dest_j] = src[src_radl_ofs + j];
            }
}
    }
}


void rdr_xlat::XlatArray()
{
    int az, rng;
    unsigned char *p = src_array;
    float km_n, km_e;

    if (!dest_array)
	return;
    if (ApplyTestPattern) {
	if (rdr_xlat_testpatterntoggle)
	    for (az = 0; az < src_ydim; az++)
		for (rng = 0;  rng < src_xdim; rng++)
		{
		    if (*p == 0)
			*p = (rng / 10) % 7;
		    p++;
		}
	else
	    for (az = 0; az < src_ydim; az++)
		for (rng = 0;  rng < src_xdim; rng++)
{
		    if (*p == 0)
			*p = (az / 10) % 7;
		    p++;
		}
	rdr_xlat_testpatterntoggle = !rdr_xlat_testpatterntoggle;
	}
    LatLongKmDiff(StnRec[dest_stnid].Lat(), StnRec[dest_stnid].Lng(),
	StnRec[source_stnid].Lat(), StnRec[source_stnid].Lng(),
	&km_n, &km_e);
    relocate(src_array, dest_array, src_ydim, 
	src_xdim, dest_xdim, 
	SrcScan->rng_res / 1000.0, SrcScan->start_rng / 1000.0, 0, 
	km_n, km_e);
}
 
   
void rdr_xlat::doXlat(rdr_scan_node *src_scan)
{
    if (!src_scan || !src_scan->scan)
	return;
    if (src_array) {
	delete[] src_array;
	src_array = 0;
	}
    src_xdim = src_ydim = 0;
    SrcScan = src_scan->scan;
    src_array = SrcScan->Create2DPolarArray(&src_xdim, &src_ydim);
    if (!src_array)
	return;
    if (dest_array && (src_xdim * src_ydim != dest_xdim * dest_ydim)) {	// wrong dest array size
	delete[] dest_array;
	dest_array = 0;
	dest_xdim = dest_ydim = 0;
}
    if (!dest_array) {
	dest_xdim = src_xdim + int(SrcScan->start_rng / SrcScan->rng_res);	// dest start rng is 0, add cells
	dest_ydim = src_ydim;
	dest_array = new unsigned char[dest_xdim * dest_ydim];
    }
    memset(dest_array, 0, dest_xdim * dest_ydim);
    XlatArray();
    if (!xlat_scan) 
	xlat_scan = new rdr_scan(this, "rdr_xlat::doXlat");
    xlat_scan->set_dflt(SrcScan);
    xlat_scan->station = dest_stnid;
    xlat_scan->start_rng = 0;
    xlat_scan->EncodeDataFrom2DPolarArray(dest_array, dest_xdim, dest_ydim);
    xlat_scan->data_source = PROD_XLAT;
    ScanMng->NewDataAvail(xlat_scan);
    ScanMng->FinishedDataAvail(xlat_scan);
    if (xlat_scan->ShouldDelete(this, "rdr_xlat::doXlat")) {
	delete xlat_scan;
	}
    xlat_scan = 0;
    if (src_array) {
	delete[] src_array;
	src_array = 0;
	}
    SrcScan = 0;
}


void rdr_xlat::workProc()	// perform any "work" that needs doing, called repeatedly by runloop()
{
  ThreadObj::workProc();  // perform base class operations
    CheckNewData(TRUE);
    ProcessCheckedData();
}


void rdr_xlat::threadInit()
{
#ifdef THREAD_SPROC
    nice(15);   //back off scheduler priority a bit (15 is actually a lot)
#endif    
#ifdef THREAD_PTHREAD
// haven't implemented sched priority in pthreads yet
// apparently Linux doesn't actually change pthread priority 
// at the moment (01/03/01) anyway
#endif    
}


