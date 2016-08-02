#ifndef __DISPL_MNG_H
#define __DISPL_MNG_H

#include "rdrseq.h"
#include "siteinfo.h"
class DisplMng {
public:
	pid_t		pid;
	rdr_seq		*rdrseq;
	DisplMng(int	seqsetsize = 0);
	~DisplMng();
	void            init(char *fname = NULL);
	float defaultMindBZ;
	float defaultMindBZTops;
	int   maxSeqMem;         // max seqmem size in MB
	int   initialSeqMem;     // max seqmem size in MB
	int   initialSeqDepth;   // max seqmem size in MB
	int   _newWinStn;         // max seqmem size in MB
	bool  drawInterpolated;
	bool  mapInForeground;
	bool  noWinLink;
	int   seqTimeStep;
	bool  useLastScanDList;
	bool  defaultOutputPNG;
	
	stnList stnPageList; // list of stn order
	int   SelPrevStn(int thisstn);
	int   SelNextStn(int thisstn);
	// select prev/next stn for all wins showing same stn 
	void   SelPrevStn_StnWin(DisplWin *thiswin);
	void   SelNextStn_StnWin(DisplWin *thiswin);
	// select prev/next stn for all wins in group 
	void   SelPrevStn_GroupWin(DisplWin *thiswin);
	void   SelNextStn_GroupWin(DisplWin *thiswin);
	int    newWinStn();
};

bool rapicUseVARender();
extern bool rapicUseVA;
extern bool rapicUseCg;
extern float minUseVADriver;
extern float minUseCgDriver;
extern bool allowDualVXSect;
extern int maxVolID;
extern DisplMng *DisplMngr;

#endif // __DISPL_MNG_H
