#ifndef	__RDRSEQ_H
#define __RDRSEQ_H

#include "displ.h"
#include "ogldispl.h"
#include "gldtawin.h"
#include "siteinfo.h"
#include "rdrscan.h"
#include <sys/types.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rain_acc.h"

enum LATESTMODE {LM_STATIC, LM_COMPLETE, LM_SCANBYSCAN};
enum SEQCTL {STOP, START, STEPFWD, STEPBWD,
	     FIRST, LAST, POS, TOGGLESTARTSTOP};

enum e_winGroupMode {wgm_icon, wgm_hide};
extern vector<string> WinGroupNames;
extern e_winGroupMode winGroupMode;
extern bool rdrseqWindowClosed;

// any added parameters MUST SENSIBLY DEFAULT TO ZERO
// when adding parameters spare must be decremented to allow for 
// the space used by the new variable
// keep in mind actual ALIGNED memory use for the variable
// THESE FILES WILL BE BYTE ORDERING DEPENDANT, NOT PORTABLE

class rdrseq_params {
friend class rdr_seq;
	int	    seq_delay;      // time between images (secs)
	int	    seq_hold;       // time to hold end of seq (secs)
	int	    seq_depth;      // depth of sequence to display (-1 = unlimited)
	int	    seqMem;         // memory allowance for seq
	int	    WinCount;
	unsigned char DisplLinks;     // if true, link all display windows pan/zoom
	WinXYWH	    ImgDataW;	    // pos/size of img data window
	WinXYWH	    ReflPalW;	    // pos/size of img data window
	WinXYWH	    HghtPalW;	    // pos/size of img data window
	LATESTMODE  LatestMode;	    // sets latest mode
	int 	    param_size;	    // no of bytes used in this = sizeof(seq_params) - sizeof(seq_params.spare)
	int 	    VersionNo;	    // version no. of 3D-Rapic when saved
	int	    ShowLatest;	    // show latest status
	int	    ignoreFutureData;// if true, ignore "future" data, i.e. timestamped in the future
	int	    futureDataTmWin;// window to allow for future data, seconds
	int         rdrimg_tmwin;   // 
	unsigned char params1[4];   // [0]=UNITS enum
	char	    spare[32];	    // spare space
	void        writeASCII(FILE *outfile = stderr);
	bool        readASCII(FILE *outfile);
	};

class rdr_seq : public scan_client {
	rdr_img*	first_seq;	// first (earliest) in seq
	rdr_img*	this_seq;	// currently displayed img
	rdr_img*	last_seq;	// last (latest) in seq
	rdr_scan_node	*embryos;	// very early scans, root scan not finished
	CDataNode	*cdata_embryos;	// very early scans, root scan not finished
	timeval		nexttime;	// time to update image
	DisplWin	*FirstWin,*TempWin,*LastWin;	// list of display windows
	DisplWin	*FocusWin;	// Window with current focus
	rdr_img		*viewimg;	// if != 0 view this only, ie no seq

	CDataNode	*thisSatDataNode;   // pointer to current sat data frame node
					// DO NOT DELETE THIS
					// set by ViewImg

	map<time_t, rdr_img*> imgmap;   // map of rdr_imgs
	rdr_img		*savethis;	// save pos. in seq. prior to viewimg
	GlImgData	*ImgData;	// clock, etc. for this img
	bool		seq_stopped;	// stops seq
	bool		ShowLatest;	// true if seq. latest clicked
	time_t		revertToLatestTime;
	time_t		revertToLatestDelay;
	bool		dualHead;	// true if dual head display
	int	        currentScreen;	// set screen for subsequent window opens
	void		step_fwd();	// step to next
	void		step_bwd();	// step to prev
	int 		draw_latest(int mode = 0,bool verbose=FALSE); // draw last image(1 for )
	void 		goto_latest();
	int 		draw_oldest(int mode = 0,bool verbose=FALSE);// draw last image(1 for )
	void 		goto_oldest();
	void		SetPos(int pos);
	void		CheckScans();	// check embryo scans
	bool		ImgChanged(rdr_img *img);	// check incomplete img for change
	void		del_seqimg(rdr_img *delimg);	// notify windows of impending img del
	bool		InsertScan(rdr_scan *insertscan);	// insert scan into existing image, if
					// suitable, else open new image
	rdr_img		*ImgTimeMatch(rdr_scan *insertscan); 
	rdr_img		*ImgTimeMatch(time_t matchtime); 
	// return image with matching time (within window) , else return 0
	// if rdrimg_timewin = -1 return nearest image
public:
	float		seqdelay;		// time between images (secs)
	float		seqhold;		// time to hold end of seq
	char		this_text[128];		// stn/date/time of this img, text string
	long		IPFocus;
	LATESTMODE	LatestMode; // sets the latest img behaviour (e.g auto)
	int		seq_count;		// no. of images in seq.
	int 		seq_depth;		// depth of sequence to display (-1 = unlimited)
	virtual void    SetSeqDepth(int newdepth);
	int		seq_pos;		// position in sequence, 0=latest
	int		rf_seq_mem;		// current rainfields size
	int		rf_seq_count;		// current rainfields size
 	int		currentSeqMem;		// current size of sequence
	virtual void    setSeqMem(int newmaxmem);
	int 		seqMem;		        // currently selected seq memory allowance
	int             seqMemLimit;            // max seq mem allowance
	short		MPosX,MPosY;		// mouse cursor position
	bool		DisplLinks;		// if true, link all same stn display windows pan/zoom
	bool		DisplLinkNeighbors;	// if true, link all neighboring radar display windows pan/zoom
	float           DisplLinkNeighborRng;   // max rng for link neighbors
	int             currentWinGroup;        // 0=show all groups, 
	                                        // winGroup=0(default) windows always shown
	void            showWinGroup(int selwingroup = -1);
	int             winGroupCount(int wingroup);  // return number of windows in group
	void            setDisplNeighborLinkMode(bool linkneighbors, float linkrng = 0)
	  {
	    DisplLinkNeighbors = linkneighbors;
	    if (linkrng)
	      DisplLinkNeighborRng = linkrng;
	  };
	UNITS		Units;			// Metric/Imperial switch
	rdr_seq(int seq_mem_size = 0);
	virtual ~rdr_seq();
	void		AddWin(char* WName,WinType WType, DisplWinParams *params=NULL);
	void		AddWin(char* WName,WinType WType, int initstn);
	void		AddWin(char* WName,WinType WType,
			       int initstn, DisplWinParams *params);
	DisplWin	*firstWin() { return FirstWin; };
	DisplWin	*lastWin() { return LastWin; };
	DisplWin	*focusWin() { return FocusWin; };
	void		CheckWindows(rdr_img *Img = 0);
	void		CloseAllWins();
	void		CheckForClosedWindows(bool forcecheck = false);	// look for callback style windows closed by user
	virtual void    setRdrImgTmWin(time_t newtmwin);  // time window to qualify for same image
	void            newRdrImgTmWin();       // time period changed - restructure rdr_scans into new sequence
	bool		seq_full(bool forcecheck = true); // seq full, new imgs will overwrite
	int		seq_size();			// total data size of sequence
	int 		check_seq(bool);		// time to step?
	void		CheckPos();			// set seq_pos absolutely
	void		SeqCtl(SEQCTL ctl, int pos=0);
	bool		SeqStopped();
	bool            inSeqLatestMode()
	  { return ShowLatest; };
	bool            imgIsLatest(rdr_img *_img)
	  { return _img == last_seq; };
	bool		ToggleStartStopSeq();
	void		add_img(rdr_img*);	// add new to sequence
	virtual void    DataModeChanged();  // try to set data mode
	virtual int  	NewDataAvail(rdr_scan *newscan); // new scan passed by ScanMng
#ifdef USE_RAINFIELDS_CLIENT
	virtual int	NewDataAvail(Ref<rainfieldImg>& rfimg); //return 1 if scan added, else 1
	virtual void    ProcessNewRFImages(); // process the new_rf_Images
	bool		InsertRFImg(Ref<rainfieldImg>& rfimg);   // insert rfimg into existing image if
					      // suitable, else open new image
	void            RemoveRFImgs(int stn); // remove all rainfield images for this stn
#endif
	virtual int  	NewDataAvail(CData *newdata); // new scan passed by ScanMng
	virtual int  	FinishedDataAvail(rdr_scan *newscan); // new scan passed by ScanMng
//	virtual int  	FinishedDataAvail(CData *newdata); // new scan passed by ScanMng
	virtual void	ImageScanFinished(rdr_img *img);
	virtual void	CheckImageRender(CData *pCData);
	virtual void    CheckNewData(bool MoveToChecked = FALSE, 
				    bool RejectFaulty = FALSE); // keep this quick, uses lock
	virtual void    ProcessCheckedData(int maxscans = -1);// process checked scans list
	
	virtual int	percentFull(int passedSize = -1);		// client percentage full indication
	virtual bool    Full(int passedSize = -1, bool forcecheck = true);
	virtual bool    MergeScans();	// true if scan client is merging scans flag
	virtual int	NumImgs();	// client image count
	rdr_img*	this_img();
	void		del_this();			// remove this_img from seq
	void		del_old();			// remove first_img from seq
	void		clear_seq();		// remove all from seq
	int 		draw_this(int mode = 0,bool verbose=FALSE,
				  rdr_img *forceimg = 0);		// draw current image(1 for FAST)
	rdr_img	        *ThisSeq();

	time_t          firstSeqTime();
	time_t          lastSeqTime();
	time_t          thisSeqTime();

	void		InitUsrIFace();
	void		HndlKbd(unsigned short);				// check keyboard for seq ctl
	void		CheckUsrIFace();
	void		SetGLFocusWin(long);		// set FocusWin to InputFocus handle
	void		SetFocusWin(DisplWin *newFocusWin);		// set FocusWin to InputFocus handle
	void		OpenImgData();
	void		UpDateImgData(rdr_img *img = 0);
	
/*  USE GLOBAL VALUES. IF MULTI RDRSEQ ON DIFFERENT DISPLAYS, NEED TO
 * USE FOLLOWING FOR COLOR MAPS FOR EACH DISPLAY
	int		cmapbits;
    	int		zmapsize, zmapbase,
			htmapsize, htmapbase, 
			vmapbase, vmapsize, 
			vilmapbase, vilmapsize;
*/
	ReflPal		*ZPal;	  // reflectivity color palette
	HeightPal	*HghtPal; // height palette data
	VILPal		*VILPAL;
	VelocPal	*VelocPAL;
	AccumPal	*AccumPAL;
	
	void		SetAllDisplViewPt(DisplWin *ViewWin, bool AllWins = false);
	void		ViewScan(rdr_scan *VIEWSCAN = 0, 
				 bool AddToImg = FALSE, 
				 bool DrawIt = TRUE);
			// temp. scan viewer (stops seq)
			// pass with viewscan = 0 to disable
			// if AddToImg, add scan to viewimg
	void		ViewImg(CDataNode *viewcdatanode = 0);
	void		ViewImg(rdr_img *VIEWIMG = 0);
			// temp. img viewer (stops seq)
			// pass with viewscan = 0 to disable
	void		DrawViewImg();	// draw viewimg. Use after multi scan img assembled
	int		NumberWins();	// go through list of Wins & number them
	DisplWin        *GetWinNum(int n);	// return the n'th DisplWin
	void 		PutState(char *fname=0);	// save state to disk
	bool		GetState(char *fname=0);	// get state from disk
	void		NewUnits(UNITS NewUnits = UNITS_UNDEF);
	bool		MergePresent();	//return true if there is a merge window in the list
	//	accum_mngr	*AccumMng;
	//	void		SetAccumMng(accum_mngr *accmng);
	virtual void	PrintScanUsage(FILE *file, bool verbose = false);
	bool		IsDualHead();
	void		setScreen(long newscreen = 0);
	int		getScreen();
	void		CheckImageRender(); // check whether any new images need to be
			// rendered to GIF/JPEG
	// remove all product scans from the defined creator from the seq
	void		delProductScansByCreator(void *creator, char *str);// del all product scans from this creator
	void            newLatLongCursorPos(LatLongHt *CursorLatLong);
	void            syncCursorData(CursorDataStruct *cursordata,
				       DisplWin *callingwin);
	// get distance between 2 window centres
	float           windowsDistance(DisplWin *win1, 
					DisplWin *win2);
	DisplWin*       nearestWin(DisplWin *win1,
				   WinType wintype = WNOWIN);
};

extern rdr_seq *RdrSeq;

#endif	/* __RDRSEQ_H */
