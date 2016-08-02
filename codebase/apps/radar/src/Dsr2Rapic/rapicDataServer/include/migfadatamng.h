#ifndef _MIGFADATAMNG_H_
#define _MIGFADATAMNG_H_

#include "spinlock.h"
#include "rdrscan.h"
#include "cdata.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#else
#include <list.h>
#include <vector.h>
#endif
#include <ringbuf.h>	//Migfa include collection, ie /radar1/sandy/obmigfa-2-0/software/share/inc
#include <tdwr_defs.h>  //Migfa include collection 

#define RAPIC_DBZ_BYTES sizeof(unsigned char)
#define RAPIC_VEL_BYTES sizeof(unsigned char)
#define RAPIC_DBZ_SCALE 2 //2   //used to cram floats into 8 bits
#define RAPIC_DBZ_OFFSET 64 //32 //
#define RAPIC_VEL_SCALE 5 //2   //
#define RAPIC_VEL_OFFSET 1 //64 //use nyquist value in code

#define NO_GATES 200

class MigfaDataMng : public scan_client {
    THREADID_T	 Thread_pid,Thread_ppid;
    int		 StopThreadFlag;
    int		 VolumeNo;  //count of volumes (scans in migfese)
    time_t	 last_scan_time;
    static void	 ThreadEntry(void *thismigfadatamng);
    spinlock	*lock;
    spinlock	*migfaDataListLock;
    void	 runLoop();
    void	 readInitFile(char *initfilename = 0);
    RingBuf     *ring;
    int          nGates, maxGates;
    int          shm_key,shm_mbytes, shm_mcount, ring_addr;
    void         Init_migfa();
    int	         migfaWriteOutputRing(caddr_t tdwr_radial, int radial_bytes );
    int	         rapicToRadial(bool last_radial, rdr_scan *reflscan, s_radl
			       *refl_radl, rdr_scan *velscan, s_radl *vel_radl,
			       caddr_t *migfa_radial );
    rdr_scan_node    *newscans_last, *checkedscans_last;
    
public:
    MigfaDataMng();
    virtual	    ~MigfaDataMng();
    void	    StartThread();
    void	    StopThread();
    virtual void    GetListLock();  // allow MigfaDataNode List locking
    virtual void    RelListLock();  // MUST BE USED RESPONSIBLY!!!
    int		    migfaDataNodeCount;
    int		    migfaDataNodePos;	
    bool	    migfaFlag;
    int             station;
    
    

    rdr_scan_node   *MigfaDataNodes;	// linked list of rdr nodes
    rdr_scan_node   *ThisMigfaDataNode;	// linked list of incomplete rdr nodes



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

extern MigfaDataMng *MigfaDataManager;

#endif _MIGFADATAMNG_H_
