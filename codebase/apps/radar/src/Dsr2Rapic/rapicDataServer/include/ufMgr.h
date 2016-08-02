/*copied from NexRadMgr.h and merged with ufMgr.h */

#ifndef _ufMgr_H_
#define _ufMgr_H_

#include "spinlock.h"
#include "rdrscan.h"
#include "cdata.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
//using namespace std;
#else
#include <list.h>
#include <vector.h>
#endif

#include "uf.h"
#include "ufStnHdlr.h"

#define RAPIC_DBZ_BYTES sizeof(unsigned char)
#define RAPIC_VEL_BYTES sizeof(unsigned char)
#define RAPIC_DBZ_SCALE 1 //2   //used to cram floats into 8 bits
#define RAPIC_DBZ_OFFSET 0 //32 //
#define RAPIC_VEL_SCALE 10 //2   //
#define RAPIC_VEL_OFFSET 0 //64 //use nyquist value in code

#define NO_GATES 200
#define UF_FILE 1
#define UF_SOCKET 2
#define UF_AVAIL 0
#define UF_LINELEN 2048
#define UF_MAX_STATIONS 10

struct uf_station_init 
{
  char 		path[256];
  int		station;
  e_scan_type	radarType;
  ufStnHdlr *ufstn;
};

class ufMgr : public scan_client {
    void         Init_uf();
    bool	 debug;
    struct uf_station_init stn_init[UF_MAX_STATIONS];
    int          stn_count;
    
    
    
public:
    ufMgr();
    virtual	    ~ufMgr();
    void	    StartUfStnHdlrs();
    bool	    ufFlag;
    bool	    LoadFromDB;	//true if OK to load from database
    virtual int	    NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
    virtual int	    NewDataAvail(CData *newscan); // new scan passed by ScanMng
    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); 
    // add finished scan to new scan list. usually called by other thread
    virtual int	    FinishedDataAvail(CData *newscan); 
    virtual void    SetDataMode(e_scan_client_mode newmode);	// try to set data mode
  
};

#endif // _ufMgr_H_
