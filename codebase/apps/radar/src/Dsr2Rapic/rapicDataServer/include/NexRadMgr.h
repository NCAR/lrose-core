#ifndef _NexRadMgrH_
#define _NexRadMgr_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "spinlock.h"
#include "rdrscan.h"
#include "cdata.h"
#include "spinlock.h"
//SD 14/2/5
#include "rpdb.h"

#include "rdrscan.h"
#include "cdata.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#else
#include <list.h>
#include <vector.h>
#endif
#include "nexrad_port_types.h"
#include "nexrad_structs.h"
#include "nexrad.h"

#define RAPIC_DBZ_BYTES sizeof(unsigned char)
#define RAPIC_VEL_BYTES sizeof(unsigned char)
#define RAPIC_DBZ_SCALE 1 //2   //used to cram floats into 8 bits
#define RAPIC_DBZ_OFFSET 0 //32 //
#define RAPIC_VEL_SCALE 10 //2   //
#define RAPIC_VEL_OFFSET 0 //64 //use nyquist value in code

#define MAXOUTPUT 20
#define NEXRAD_START_PORT 11335  //local port starting number
#define NEXRAD_CHECKWRITE_LIMIT 300
#define NEX_VS_THRESH 20
#define NEX_VS_FILENAME "nexrad_vsno.dat"
#define NEX_LINELEN 2048
#define NEX_MAX_STATIONS 100
#define NEX_MAX_PATTS 10


struct scan_pattern //SD add 22/8/00
{
  int scan_count;
  int pattern;
};


class NexRadStnHdlr : public scan_client {
    spinlock	*lock;
    spinlock	*NexRadDataListLock;
    void	 workProc();
    void	threadInit();
    void	threadExit();
    NexRadOutpDesc *outputdesc[MAXOUTPUT];
    int		 outputcount;
    
    int	         rapicToRadial(bool last_radial, rdr_scan *reflscan, s_radl
			       *refl_radl, rdr_scan *velscan, s_radl *vel_radl,
			       caddr_t *NexRad_radial );
    rdr_scan_node    *newscans_last, *checkedscans_last;
    int          computeAndWriteVcp(int vol_id, rdr_scan* tempscan);
    int          update_vol_scan_no();
    bool	 debug;
    bool      filter_Vel;
    bool      filter_Refl;
    bool      filter_SpWdth;
    e_scan_type  radarType;
    char         radarTypeStr[20];
    char	 path[255];
    char	 vcp_locn[255];
    char	 udp_address[100];
    int	         udp_port;
    //first of block of local ports
    int          start_local_port; 
    int          local_port; 
    int refl_rngres_reduce_factor;
    int vel_rngres_reduce_factor;
    //int		 pattern;  SD alt 22/8/00
    scan_pattern* scan_patt_arr;
    int          scan_patt_count;
    
    int          packet_size;
    int          delay_pac;
    int          min_time_per_tilt;
    int          seq_num;
    ui32         fr_seq;
    int 	 GetScanPattern(rdr_scan* scan);
    bool         noTempFile;
    int               s1fd;   //rdrInvent.C output file handle SD
			      //14/2/5
    int vcpNumPerVolumeId[4]; //array giving default VCP	 for each
			     //VOLUMEID, max of 4
    
public:
    int		    VolumeNo;  //count of volumes (scans in migfese)
    NexRadStnHdlr();
    NexRadStnHdlr( int station,
		    char *path,
		    NexRadOutpDesc **outputdesc,
		    int outputcount,
		    e_scan_type radarType,
		    int packet_size,
		    //int pattern,  SD alt 22/8/00
		    scan_pattern* scan_patt_arr,
		    int scan_patt_count,
		    char *vcp_locn,
		    int delay,
		    int mintimepertilt, 
		    int start_local_port,
		    bool debug,
		    bool LoadFromDB,
		    bool NexRadFlag,
		    bool writeSimRapic,
		    bool completeVol,
		    bool ignoreFutureData,
		    bool noTempFile, 
		    bool filter_Vel,
		    bool filter_Refl,
		    bool filter_SpWdth,
		    int refl_Rngres_reduce_factor,
		    int vel_Rngres_reduce_factor);
    
    virtual	    ~NexRadStnHdlr();
    void	    StartThread();
    void	    StopThread();
    virtual void    GetListLock();  // allow NexRadDataNode List locking
    virtual void    RelListLock();  // MUST BE USED RESPONSIBLY!!!
    int		    NexRadDataNodeCount;
    int		    NexRadDataNodePos;	
    bool	    NexRadFlag;
    bool	    writeSimRapic;  //call rdrInvent
    bool	    completeVol;  //call rdrInvent
    bool	    LoadFromDB;	//true if OK to load from database
    int             station;
    char 	    stationName[100];
    float	    latit,longit,alt;
    

    rdr_scan_node   *NexRadDataNodes;	// linked list of rdr nodes
    rdr_scan_node   *ThisNexRadDataNode;	// linked list of incomplete rdr nodes



    virtual int	    NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
    virtual int	    NewDataAvail(CData *newscan); // new scan passed by ScanMng
    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual int	    FinishedDataAvail(CData *newscan); // add finished scan to new sca list. usually called by other thread
// quickly check new scans list, add scans to keep to checkedscans, discard others
    virtual void    CheckNewScans(bool MoveToChecked = FALSE, 
		    bool RejectFaulty = TRUE); // keep this quick, uses lock
/*
 * ProcessCheckedScans doesn't need to be locked,  can perform 
 * time consuming operations
 */				
    virtual void    ProcessCheckedScans(int maxscans = -1);// process checked scans list
    
};

struct station_init 
{
  char 		path[256];
  char          vcp_locn[256];
  NexRadOutpDesc *outputdesc[MAXOUTPUT];
  int	        outputcount;
  int		station;
  e_scan_type	radarType;
  int 		packet_size;
  //int 		pattern;  SD alt 22/8/00
  scan_pattern* scan_patt_arr;
  int           scan_patt_count;
  int		delay;
  int		min_time_per_tilt;
  int		start_local_port;
  int           refl_rngres_reduce_factor;
  int           vel_rngres_reduce_factor;
  NexRadStnHdlr *nexradstn;
  bool       filter_Vel;
  bool       filter_Refl;
  bool       filter_SpWdth;
  bool       noTempFile;
};

class NexRadMgr : public scan_client {
    void         Init_NexRad();
    bool	 debug;
    struct station_init stn_init[NEX_MAX_STATIONS];
    int          stn_count;
    
    
    
public:
    NexRadMgr();
    virtual	    ~NexRadMgr();
    void	    StartNexRadStnHdlrs();
    bool	    NexRadFlag;
    bool	    writeSimRapic;
    bool	    completeVol;
    bool	    LoadFromDB;	//true if OK to load from database
    virtual int	    NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
    virtual int	    NewDataAvail(CData *newscan); // new scan passed by ScanMng
    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); 
    // add finished scan to new scan list. usually called by other thread
    virtual int	    FinishedDataAvail(CData *newscan); 
    virtual void    SetDataMode(e_scan_client_mode newmode);	// try to set data mode
  
};

#endif // _NexRadMgr_H_
