#ifndef _ufStnHdlr_H_
#define _ufStnHdlr_H_

#include "spinlock.h"
#include "rdrscan.h"
#include "cdata.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
using namespace std;
#else
#include <list.h>
#include <vector.h>
#endif

#include "uf.h"

#define RAPIC_DBZ_BYTES sizeof(unsigned char)
#define RAPIC_VEL_BYTES sizeof(unsigned char)
#define RAPIC_DBZ_SCALE 1 //2   //used to cram floats into 8 bits
#define RAPIC_DBZ_OFFSET 0 //32 //
#define RAPIC_VEL_SCALE 10 //2   //
#define RAPIC_VEL_OFFSET 0 //64 //use nyquist value in code

#define NO_GATES 200


class ufStnHdlr : public scan_client {
    int		 VolumeNo;  //count of volumes (scans in migfese)
    spinlock	*lock;
    spinlock	*ufDataListLock;
    void	 workProc();
    void	threadInit();
    void	 readInitFile(char *initfilename = 0);
    void         init_uf();
    int	         rapicToRadial(bool last_radial, rdr_scan *reflscan, s_radl
			       *refl_radl, rdr_scan *velscan, s_radl *vel_radl,
			       caddr_t *uf_radial );
    void 	dumpBuff();
    

    rdr_scan_node    *newscans_last, *checkedscans_last;
    bool	 debug;
    e_scan_type  radarType;
    char         radarTypeStr[20];
    char	 path[255];
    struct voldata ppivol;
    int          checkIniFileDelay; //number of secs between rereaduf.ini flag check
    time_t       checkIniFileTime;
    
    
public:
    //ufStnHdlr();
    ufStnHdlr(      int station,
		    char *path,
		    e_scan_type radarType,
		    bool debug,
		    bool ufFlag);
    
    virtual	    ~ufStnHdlr();
    virtual void    GetListLock();  // allow UfDataNode List locking
    virtual void    RelListLock();  // MUST BE USED RESPONSIBLY!!!
    int		    ufDataNodeCount;
    int		    ufDataNodePos;	
    bool	    ufFlag;
    int             station;
    char 	    stationName[100];
    float	    latit,longit,alt;

    rdr_scan_node   *UfDataNodes;	// linked list of rdr nodes
    rdr_scan_node   *ThisUfDataNode;	// linked list of incomplete rdr nodes

    //struct voldata  ppivol;  //intermediate data structure for uf write
    int             seqno;
    int	            imageno;
    

    virtual int	    NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); 
    // add finished scan to new sca list. usually called by other thread
    // quickly check new scans list, add scans to keep to checkedscans, discard others
    virtual void    CheckNewScans(bool MoveToChecked = FALSE, 
							bool RejectFaulty = TRUE); // keep this quick, uses lock
/*
 * ProcessCheckedScans doesn't need to be locked,  can perform 
 * time consuming operations
 */				
    virtual void    ProcessCheckedScans(int maxscans = -1);// process
    // checked scans list
    void 	addRapic2Scan(rdr_scan *rdrscan);
    void 	init_scan(rdr_scan *rdrscan);
    int 	init_ppivol(rdr_scan *rdrscan);
    uf	       *ufInst;


};

//extern ufStnHdlr *ufManager;

#endif // _ufStnHdlr_H_
