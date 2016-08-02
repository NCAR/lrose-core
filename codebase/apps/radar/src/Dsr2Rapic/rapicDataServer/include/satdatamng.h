#ifndef _SATDATAMNG_H_
#define _SATDATAMNG_H_

#include "spinlock.h"
#include "rdrscan.h"
#include "cdatasat.h"
#include "cdatasat_svissr.h"
#include "cdatasat_subset.h"
#ifdef STDCPPHEADERS
#include <list>
#include <vector>
#else
#include <list.h>
#include <vector.h>
#endif

enum SatDataType {svissr, hrpt};
enum SatFileType {raw, asda, cdata};

extern char satDisplayInitFile[];

class cdataSatBuffer {
public:
    int	size;
    unsigned char *buf;
    cdataSatBuffer(int buffsize = 0, unsigned char *buffer = 0) :
	size(buffsize), buf(buffer) {};
    void set(int buffsize, unsigned char *buffer) 
    {
	size = buffsize;
	buf = buffer;
    }
};

/*
 * Check for the presence of flag files which contain filenames
 * of new sat images.
 * 
 * e.g. newgms.flag
 * newgms contents
 * filename datatype filetype
 * datatype (svissr, .....)
 * filetype (raw, asda, cdata)
 * 
 */
class SatDataMng : public scan_client {
    int		    NextSubsetIdent;    // next subset ident no.
    spinlock	*lock;
    spinlock	*satDataListLock;
    spinlock	*subsetListLock;
    void	    workProc();
    void	    threadInit();
    void	    threadExit();
    void	    readInitFile(char *initfilename = 0);
    void	    readStateFile(char *newstatefile = 0);
    void	    writeStateFile(char *newstatefile = 0);
    void	    readFileList(char *newlistfname = 0);
    void	    writeFileList(char *newlistfname = 0);
    bool	    fileIsInList(char *filename, char *readlistfname = 0);
    int		    maxSvissrFrames;
    int		    maxSvissrFiles;
    double	    minFreeDisk;
    char	    SvissrDataDir[256];	// directory in which data should be stored
    char	    defaultJpegDir[256];
    char	    autoJpegDir[256];
    char	    newSvissrFlagFile[256];
    char	    svissrFileList[256];
    char	    lastSvissrFile[256];    // file name of last svissr file, use this for disk free calcs
    int		    seqDepth;
    int		    uifseqsize;				// track uif version of seq size
public:
    SatDataMng();
    virtual	    ~SatDataMng();
    virtual void    GetListLock();  // allow SatDataNode List locking
    virtual void    RelListLock();  // MUST BE USED RESPONSIBLY!!!
    virtual void    CheckNewFiles(char *newsatfile = 0, 
	bool		deleteAfterRead = true, 
	bool		createArchiveLink = true);		// look for satgms.flag file, if present load files named by satgms.flag
    CDataNode	    *ResampleSubsetNodes;   // linked list of subsets to be resampled nodes
    CDataNode	    *SatDataNodes;			// linked list of CDataSat nodes
    CDataNode	    *ViewSatDataNode;	    // temp "view" sat data node
    CDataNode	    *ThisSatDataNode;	    // linked list of incomplete CDataSat nodes
    CDataNode	    *StepFwdSatDataNode(CDataNode *basisnode = 0, bool RelLock = true);	    // linked list of incomplete CDataSat nodes
    CDataNode	    *StepBwdSatDataNode(CDataNode *basisnode = 0, bool RelLock = true);	    // linked list of incomplete CDataSat nodes
    CDataNode	    *SetPos(int &pos, bool RelLock = true);	    // linked list of incomplete CDataSat nodes
    CDataNode	    *FirstSatDataNode(bool RelLock = true);	    // linked list of incomplete CDataSat nodes
    CDataNode	    *LastSatDataNode(bool RelLock = true);	    // linked list of incomplete CDataSat nodes
    bool	    IsLastInSeq(CDataNode *basisnode = 0);
    bool	    IsValidNode(CDataNode *basisnode);
    void	    UIFSeqPos();			// update seq pos info.
    void	    UIFSeqSize();			// update seq pos info.
    int		    satDataNodeCount;
    int		    satDataNodePos;	
    bool	    purgeBadFiles;
    int		    NewSubsetIdent();		// return next sat subset ident
    void	    AddSubsetParams(SatSubsetParams *newsubset);
    void	    ChangeSubsetParams(SatSubsetParams *newsubset);	// new
    SatSubsetParams *GetSubsetParams(int subsetid);
    int		    DeleteSubsetParams(int subsetid);
    int		    RequestDeleteSubsetParams(int subsetid);
    void	    CheckSubsetParams();
    void	    delSubsetsByIdent(int subsetident);
    // set the subsetparams of each matching subset and 
    // set the resampling flag
    void	    setNeedsResamplingFlags(int subsetid); 
    void	    setNeedsResamplingFlags(SatSubsetParams *resampleSubsetParams); 
    void	    setSubsetNavGridInvalidFlags(int subsetid); 
    void	    setSubsetInvalidFlags(int subsetid); 
    void	    setAutoWebJpegMode(int subsetid, bool state); 
    void	    setAutoLocalJpegMode(int subsetid, bool state); 
    void	    setNeedsRenderFlags(int subsetid); 
	// the refnavsatprojgrid is kept by the ogldisplsat window
	// so can be relied upon not to be deleted until the subset is deleted
    void	    setSubsetRefNavSatGrids(int subsetid, 
			projGrid *refNavSatProjGrid, 
			CSatNavSvissrSm *refsvissrnavigation);
	
    void	    setMaxFrames(int maxframes);
    void	    setSeqDepth(int seqdepth);
    SatSubsetParams *SubsetParams;	    
    
    bool	    reuseFreeSubsetBuffers; // if false, simply delete and recreate buffers
    int		    maxFreeSubsetBuffers;   // normally one per SubsetParams entry
    vector<cdataSatBuffer> freeSubsetBuffers;
    void	    addSubsetBufferToFree(int buffsize, unsigned char *buffer);
    unsigned char*  getSubsetBufferFromFree(int buffsize);
    void	    ClearAllSubsetBuffers();

    /*
     * pass a subset to be resampled. It must contain a link to the parent
     * sat data set,  and have its subset params set appropriately
     * CheckSubsetToResample will traverse list,  resampling subsets
     * as reqd, clear the resampling flag then removing them from list.
     */
    void	    AddSubsetToResample(CDataSatSubset *subtoresample);
    void	    CheckSubsetToResample();
    /*
     * check subsets for passed checksat are up to date
     * If no sat passed, check full Subsets list
     */
    int				CheckSubsets(CDataSatSvissr *checksat, int maxresamples = -1);
    int				CheckSubsets(int maxresamples = -1);
	virtual void	CheckImageRender();
	virtual bool	CheckImageRender(CDataSatSvissr *checksat);
    virtual int	    NewDataAvail(rdr_scan *newscan); //return 1 if scan added, else 1
    virtual int	    NewDataAvail(CData *newcdata); // new scan passed by ScanMng
    virtual int	    FinishedDataAvail(rdr_scan *finishedscan); // add finished scan to new sca list. usually called by other thread
    virtual int	    FinishedDataAvail(CData *newcdata); // add finished scan to new sca list. usually called by other thread
// quickly check new scans list, add scans to keep to checkedscans, discard others
    virtual void    CheckNewData(bool MoveToChecked = FALSE, 
			bool RejectFaulty = TRUE); // keep this quick, uses lock
    virtual void    checkFileSpace(double minfreespace = 0);
    virtual void    checkLoopDepth(int loopdepth = 0);
    virtual int	    deleteOldestSvissrFile(int delCount = 1);
    virtual int	    deleteOldestSvissrFrame(int delCount = 1);
    virtual int	    FilesAvailCount();
    virtual void    MakeArchiveLink(char *newfilename);
    virtual void    DeleteArchiveLink(char *newfilename);
    virtual void    ViewGMSFile(char *filename);	// load and view GMS file
    virtual void    SeqImgDeleted(rdr_img *delimg);
    bool			SeqPosSizeChanged;
    virtual void    CheckSeqPos();	// check whether GUI seq pos/size needs update
/*
 * ProcessCheckedScans doesn't need to be locked,  can perform 
 * time consuming operations
 */				
    virtual void    ProcessCheckedData(int maxscans = -1);// process checked scans list
    virtual void    DumpStatus(FILE *dumpfile);	// dump staus to dumpfile
};

extern SatDataMng *SatDataManager;

#endif
