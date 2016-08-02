#ifndef	__DISPL_H
#define __DISPL_H

// #include <bstring.h>
#include <string.h>
#include "rdrscan.h"
#include "cdata.h"
#include "uiftovrc.h"
#include "rdrfilter.h"
#include "stormRelClass.h"
#ifdef sgi
#include <fmclient.h>
#include <X11/Intrinsic.h>
#include <GL/gl.h>
//typedef Colorindex COLORINDEX;
typedef int COLORINDEX;
#endif
#ifdef aix
#include <gl.h>
typedef Int16 COLORINDEX;
#define fhypot hypot
#define fatan2 atan2
// the way the qstrip is used turns out to be identical to a tmesh call
#define bgnqstrip bgntmesh
#define endqstrip endtmesh
#endif
#ifdef linux
typedef int COLORINDEX;
#define fhypot hypot
#define fatan2 atan2
// the way the qstrip is used turns out to be identical to a tmesh call
#define bgnqstrip bgntmesh
#define endqstrip endtmesh
#define XMAXSCREEN 1279
#define YMAXSCREEN 1023
#endif

extern float winScaleStdSize;
extern float winScaleFactorMin;

struct WinXYWH {
  unsigned char Present;
  unsigned char Iconified;
  long x,y;
  long w,h;
  void set(long X, long Y, long W, long H)
  {
    x = X; y = Y; w = W; h = H;
  };
};

#include "graphutils.h"
#include "draw.h"
#include "projection.h"

#ifdef USE_TITAN_CLIENT
class drawTitan;
class SimpleTitanPropsWindow;
class TitanDataTable;
#endif
#ifdef USE_WDSS_CLIENT
class drawWDSS;
class SimpleWDSSPropsWindowClass;
#endif
class WdssDataTable;

enum WinType {WNOWIN,W3DPPIRHI,WPPIRHIPair,WPPIRHITriple,
	WPPI,WRHI,WVIL,WVIZ,WTops,WCZBlocks,WPPIRHI, WCAPPI, 
	WOGLSat};

extern char *WinTypeString[];

class DisplWin;

class DisplWinParams {			// Display Window save parameters
friend class DisplWin;
public:
	int 		WinNum;		// Win pos in seq list (1=first)
	WinType		Type;
	char		WinName[128];
	long		winOrgX,winOrgY,winW,winH;  // this window's pos & size
	unsigned char	Frozen;		// current iconify state
	unsigned char	DblBuffer;	// DblBuffer state
	int		Link[8];	// links to other windows (order defined by WinType)
	float   	Rng,OfsX,OfsY;  // display range(radial) and X/Y ofs
	float 		Ofs,Ht;		// RHI window always 2:1 aspect,Ofs in rng only
	LatLongHt	RefLatLongHt; 	// position of relocatable Ref Pt.
	short 		ViewAz,ViewEl;	// tenths of degrees
	float		ht_mult;

// ***SUPERSEDED BY MINVAL - KEPT FOR FILE COMPATIBILITY, SHOULD NOT BE USED *** min displayed Z level (0.5dBZ 0-160)
	int 		MinZ;		// old min dbz value no longer used   

	unsigned char 	interpmode;
	long		Unused1;        // window for data display
	int		showFilteredData;
	unsigned char	drawTerrainOlay,
	                allow3dview, 
	                winGroup, 
	                spare4;
	int		ReqEl,
			Unused5;         
	short 		station;
	float		MindBZ;		 
	float		Val;		 
	unsigned char	MergeMode;
	unsigned char	MultiMaps, ShowLatLongCursor;
	unsigned char	FontSize, LineThickness;
	unsigned char	OLayBG, RngRings, ShowText, Coast;
	e_data_type	datatype;
	e_scan_type     scantype;
	int		screen;		  // screen on which this screen is displayed
	int		autoImageCreateFlags;	// bit map for autocreate local/web jpeg/gif
	int             LinkGroup;        // link group id
	unsigned char   titanFlags[4];    // [0] = showTitan, [1] = titanThresh(0-255dBZ)
	unsigned char	wdssFlags[4];	  // [0] = showWDSS
	char		spare[16];	  // space for later additions
	// spare[0] bit 1 (0x1) is showObs flag
	// spare[0] bit 2 (0x2) is showLightning flag
	void writeASCII(FILE *outfile = stderr);
	bool readASCII(FILE *infile);
	void Clear() { memset(this, 0, sizeof(*this)); };
	DisplWinParams() { Clear(); };
	};

/* IMPORTANT - struct olayproperties in uiftovrc.h MUST BE DECLARED
 * IDENTICALLY TO THE FIRST VARIABLES PART OF THIS
 */
class	OverlayProperties {
public:
    int OLayBG, RngRings, ShowText, Coast, ScanData, FontSize, LineThickness;
    int LayerFlag[16], ValueChanged;
    void *EditWid;
    DisplWin *CallingWin;
    FONTHANDLE customfh;
    int  customfh_size;	    // if customfh_size != FontSize, allocate new font
    float winScale;
    float winScaleRatio;    // ratio of km_x/pixel_x to km_y/pixel_y
    bool antiAlias;
    void EditWidClosed();
    void CloseEditWid();
    void OpenEditWid();
    OverlayProperties(DisplWin *callingwin = 0);
    ~OverlayProperties();
    void NewVals();
};

class ImgRenderNode {
public:
	rdr_img		*ImgToRender;
	CData		*CDataToRender;
	time_t		RenderTime;	// time after which this should be rendered
	time_t		RenderDelay;	// delay time before rendering, esp for merge
	ImgRenderNode(rdr_img *renderimg, time_t delay = 0);
	ImgRenderNode(CData	*rendercdata, time_t delay = 0);
	~ImgRenderNode();
	void		UpdateRenderTime(time_t delay = -1);
	bool		IsRenderTime();
};

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

extern unsigned int AllXKeyMasks;

class mouseState {
public:
  int x, y;		/* pointer x, y coordinates in event window */
  int dx, dy;		/* change in mouse x, y coordinates from last event */
  int x_root, y_root;	/* coordinates relative to root */
  unsigned int state;	/* key or button mask */
  unsigned int laststate;  /* previous mouse event's key or button mask */
  unsigned int button;	/* detail */
  bool cleared;

  void clearState();
  void setState(XEvent *event);

  inline bool buttonDown(int buttonNo)
    {
      switch (buttonNo)
	{
	case 1 : return (state & Button1Mask);
	case 2 : return (state & Button2Mask);
	case 3 : return (state & Button3Mask);
	case 4 : return (state & Button4Mask);
	case 5 : return (state & Button5Mask);
	default : return false;
	}
    }
  inline bool ctlKey()
    {
      return (state & ControlMask);
    }
  inline bool shiftKey()
    {
      return (state & ShiftMask);
    }
  inline bool mod1Key()
    {
      return (state & Mod1Mask);
    }
  bool mod2Key()
    {
      return (state & Mod2Mask);
    }
  bool mod3Key()
    {
      return (state & Mod3Mask);
    }
  bool mod4Key()
    {
      return (state & Mod4Mask);
    }
  bool mod5Key()
    {
      return (state & Mod5Mask);
    }
  mouseState()
    {
      x = y = dx = dy = x_root = y_root = 0;
      state = laststate = button = 0;
      cleared = true;
    }

};

enum EWinSys {UNDEFINED, IRISGL, OGLX, XWIN, OGLWin};

class rdr_seq;

class DisplWin {
	friend	class rdr_seq;

 protected:
	int		WinNum;		// pos in list of displays
	char		winname[128];	// name of window
	char		lastWinTitle[256];	// name of window
	bool		Frozen;		// true if iconic
	bool		DblBuffer;	// switches double buffering
	bool		FastUpdate;	// draw using fast update approach
	bool		ClosedByUser;	// true if window closed by user
	bool            closeFlag;      // if true this display should be closed by rdrseq
	EWinSys		WinSys;		// type of window system
	DisplWin	*next,*prev;
	rdr_seq*	SeqController;	// rdr_seq obj controlling this window
	rdr_img*	CurrentImg;					
	time_t          currentImgTime()
	  {
	    if (CurrentImg)
	      return CurrentImg->img_time_t;
	    else
	      return 0;
	  };
	rdr_scan*	CurrentScan;
	time_t          currentScanTime()
	  {
	    if (CurrentScan)
	      return CurrentScan->scan_time_t;
	    else
	      return 0;
	  };
	rdr_scan*       lastDrawnScan;
	histogramClass	*Histogram;	// can use to calc histogram 
	int		station,	// "selected" station 
			thisstn;	// last displayed stn
	float		Rng,OfsX,OfsY;	// display range(radial) and X/Y ofs
	float 		Ofs,Ht;		// RHI window always 2:1 aspect,Ofs in rng only
	short		ViewAz,ViewEl;	// tenth of degrees
	bool		allow3dview;	// 
	
	bool		fixedWinAspectRatio;//default true, if false don't fix aspect window ratio
	    				// looks for flag file "free_aspect_ratio"
 public:
	long		winOrgX,winOrgY,winW,winH; // this window's pos & size in pixels
	virtual void    winCentre(long &x, long &y);// get window centre coords
	float		winWidth, winHeight,    // window dimensions etc in projection's units,   
	  winCentreX, winCentreY,               // e.g. degrees, kms
	  winStartCentreX, winStartCentreY,
	  winKmPerPixelX,
	  winKmPerPixelY,
	  winScale,	                        // winScale is zoom ratio
	  winScaleRatio,
	  winSizeScale;	                        // winSizeScale is scale against 512 pixel window
						    // may be used to scale text fonts
	int             _winGroup;       // window group identifier, 0=always shown
	virtual void    setWinGroup(int _wingroup)
	  {
	    _winGroup = _wingroup; 
	  };
	DisplWin	*nextWin() {return next; };
	DisplWin        *prevWin() { return prev; };

	float           viewLat, viewLong;
	float           viewStartLat, viewStartLong;
	bool		showFilteredData;// if true use filtered data, if available
	void            setTitanThreshIdx(int threshidx);
	float           getTitanThresh(int threshidx = -1);
	int             getTitanThreshIdx();
	int             getTitanStn();
	int             getWDSSStn();
	int             Station() { return station; };
        int             showTitan;
        int             showWDSS;
        int             showObs;
        int             showLightning;
	bool            showFlights;
	int             drawTerrainOlay;
	bool		allow3DView() { return allow3dview; };	// 
	virtual void	setAllow3DView(bool state = true) { allow3dview = state; };	// 
	bool            is3DView()
	  {
	    return allow3dview && (ViewEl);
	  };
	void            openEditOlayProps();
	virtual renderProperties* getRenderProps(renderProperties *renderprops);
	virtual renderProperties* getRenderProps();
 protected:
	rpProjection	Projection;	    // current projection
	rpProjUnits	ProjUnits;	    // units for current projection
	rdr_angle	BoundAz1, BoundAz2; // data export bounding box
	float		BoundRng1, BoundRng2;
	AzElRng		ACurAzElRng;	// Radar Rel. Cur Bearing & Rng
	float		ACurGndRng;	// Radar Rel. Gnd Range
	NthEast		ACurNthEast;	// Radar Rel. km north & east
	AzElRng		RCurAzElRng;	// Ref pnt rel. Cur Bearing & Rng
	float		RCurGndRng;	// Ref pnt Rel. Gnd Range
	NthEast		RCurNthEast;	// Ref pnt Rel. km north & east
	float		CurVal, CurVal2;// value under cursor, Lower/Upper limit
	int             CurValIdx;      // index value under cursor
	LatLongHt	OrigLLH;	// Lat/Long/Ht of origin 
	bool		RefPointDefined;// true if reference point defined
	LatLongHt	RefLLH;	        // Lat/Long/Ht of relocatable Ref Pt.
	time_t		RefPointTime;	// time of image on which ref was defined
	LatLongHt	CurLLH;	        // World Lat/Long/Ht of cursor
 	bool		MergeMode, MultiMaps;
	float		MaxMergeRadius; // outer limit of range from origin for merge
	int             mergeScanCount; // number of scans merged in render
	e_data_type	datatype;	// displayed data type
	e_scan_type     scantype;
	rdrfilter       *scanfilter;
	float 		MindBZ;		// min dBZ value
	void*		MindBZW;	// min dBZ setting widget
	float		MaxVal;		// max value found while rendering image
	int		MaxValIdx;	// index value of max
	float		MaxValRng;	// rng of max val    
	float		MaxValAz;	// az of max val
	bool            suppressRadar;  // if true don't display radar data
	void            setSuppressRadar(bool state);
	time_t          suppressRadarTimeout;
	virtual bool    scanMatches(rdr_scan *checkscan); // return if scan matches props
	void		checkForMaxVal(s_radl *radl);
	void		checkForMaxFVal(s_radl *radl);
	void		resetMaxVal();

	float 		Val;		// Window specific value, units to suit datatype
	void*		ValW;		// Window specific value setting widget
	void*		StnWid;

	long		cmapbits;	// no of dbl buff color map bits
	long		zbuffbits;	// no of normal
	short		palcmap[256]; 	// level to current cmap, max 256 entries
	short		cmap[256]; 	// level to global cmap, max 256 entries
	short		cmapbase;	// base index of color map
	short		cmapofs;	// value to color offset of color map
	short		cmapsz;		// number of cmap entries
	RGBPalette      *currentRGBPal; // pointer to current RGBPalette
	short		OverlayBits;	// number of bits available in overlay
	int		BGNormal,BGIncomplete;	// Screen Background color indexes
	void 		SetMaps(rdr_scan *rootscan = 0, 
				e_data_type dtype = e_dt_max); // set color map based on the root scan's level table
	// set the translation/rotation appropriate for scan origin vs the defined origin
	LevelTable	*lvltbl;	// level table for current drawing, set by SetCMap		
	float		ht_mult;

//	bool		TESTFLAG;
	bool		OLayBG;			// draw OLay in Background flag
	bool 	        ShowLatLongCursor;	// if true, draw synchronised lat/long cursor on this window
	LatLongHt 	CurrentLLCursPos;	// current position of lat/long cursor
	OverlayProperties *OlayProps;	// overlay properties
	renderProperties stdRenderProps;// std render properties
	bool            povChanged;
	bool		useCoverage;	// allow user to toggle coverage display
	bool		autoCreateJpeg;
	bool		autoCreateGif;
	bool            seqCreateJpegs;  // flag used to write out current sequence as Jpegs
	virtual void    setWriteSeqJpegs(bool flag); // set seq to start, turn on seqCreateJpegs flag, run seq to end, turn off flag
	virtual int     writeSeqJpeg(rdr_img *jpegimg); // write this img as jpeg
	bool		createDOSImageLink;
	bool		createNewImgFlag;
	bool		createAutoLatestLink;
	bool		lockPanZoom;
	char		autoImagePathname[256];	// default "./images/" path for auto generated images
	char		manImagePathname[256];	// default "./images/" path for man generated images
	char		seqImagePathname[256];	// default "./images/" path for man generated images
	list<ImgRenderNode*> RenderList; // linked list of radar images	
	spinlock	*RenderListLock;
	time_t          latestImgRenderTime;
	rdr_img         *latestImgRender_ImgPtr; // pointer to last used "latest" image 
	bool            autoRenderDrawing;  // set flag true if "auto render", allow some thing not to be drawn
public:
	WinType		Type;
	char		winTypeStr[32];
	char*		winTypeString();
	bool            hasFocus;

	long            polycount;      // number of polygons rendered
	long            rendertime;     // time to render (microseconds)
	long            polypersec;     // polygons per second
	bool            show_polyspersec; // if true display polyspersec
	bool            show_framespersec; // if true display polyspersec
	bool            show_polycount; // if true display polyspersec

	bool		ForceRedraw;	// if FALSE, redraws chg'd scan only
	bool            useLastScanDList;

	int             reqRedrawCount;
	bool            reqForcedRedraw;
	int             maxRedrawLag;            // if set, will force an actual redraw when value reached
	stormRelClass   stormRelVals;
    	bool		drawModifiedRadlVelSelected;// if true draw radl velocity relative to cursor value
    	virtual bool		drawModifiedRadlVel()     // if true draw radl velocity relative to cursor value
	  {
	    return drawModifiedRadlVelSelected ||
	      (stormRelVals.viewMode == sr_view_fixed);
	  };
	virtual void    checkStormRelVals() {};  // call if changed, sets gui
	virtual void    stormRelValsChanged() {};  // call if changed, sets gui

	void            setReqRedraw(int reqQLimit = 0); 
	void            clearReqRedraw(); 
	void            setReqForcedRedraw(int reqQLimit = 0); 
	void            doReqRedraw();

	CursorDataStruct CursorData;	// cursor data
	DisplWin(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName = 0,
		 DisplWinParams *params=0); 	// new window always appended
	virtual			~DisplWin();	// remove window from list
	virtual void	CloseWin();
	virtual void	CheckWin(rdr_img *Img = 0);		// perform housekeeping functions
	virtual void	GetWinSizePos() {};
	virtual void	MoveWin(long NewX,long NewY, long w=0, long h=0);
	virtual void	ResizeWin(long w, long h);
//	virtual int	DrawImg(rdr_img *Img = 0) { return(-1); }
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	SimpleDrawImg(rdr_img *Img = 0) { return DrawImg(Img); };
	virtual int	DrawImg(CData *pCData);	
	virtual int	SimpleDrawImg(CData *pCData) { return DrawImg(pCData); };	
	virtual int	DrawShowImg(rdr_img *Img = 0);
	virtual int	DrawShowImg(CData *pCData);	
	virtual void	CheckRedraw(rdr_img *Img = 0);	// check if redraw required, default is to DrawImg()
	virtual void    seqStop();
	virtual bool    seqStopped();
	virtual void    seqStart();
	virtual void    seqStepFwd();
	virtual void    seqStepBwd();
	virtual void    seqOldest();
	virtual void    seqLatest();
	virtual bool    inSeqLatestMode();
	virtual bool    imgIsLatest(rdr_img *_img);
	virtual rdr_img* showPrevRdrImg(rdr_img *thisimg, 
					time_t withinsecs = 0,
					time_t withinsecslatest = 0);
	virtual void	Show();
	virtual void	Hide();
	virtual void	Raise();
	virtual void	Lower();
	virtual void	Iconify();
	virtual bool	Iconic() { return Frozen; };
	virtual bool    Visible() { return true; };
	virtual void 	DrawRefPoint();
	virtual void 	DrawIndicators();
	virtual void	SetPointOfView(bool povchanged = false);
	virtual void	DrawAxis(float length = 0);
	virtual void	ZoomChanged(int pixels_dx, int pixels_dy) {};
	virtual void	PanChanged(int pixels_dx, int pixels_dy) {};
	virtual void	ClearLatLongCursor();
	virtual void	MoveLatLongCursor(LatLongHt *CursorLatLong);
	virtual void	DrawLatLongCursor(LatLongHt *CursorLatLong);
	virtual void    newLatLongCursorPos(LatLongHt *CursorLatLong = 0, 
					    bool forcefocus = false); // if has focus will call rdr_seq to update other wins
	virtual void    syncCursorData(CursorDataStruct *cursordata); 
 	virtual void	DrawScanData(char *prefix = 0);	// if ON, write scan data text in window
	virtual void	ShowNewImg(int seqpos = -1);  // Usual DrawImg to BG buffer, this shows newimg
	virtual void	CalcCurPos(CursorDataStruct *cursordata = 0); 
	// can be called with cursor data passed
	virtual void	GetCurData();
	virtual void	UpdateData();
	virtual void	updateSeqPos(int seqpos);
	virtual void	updateSeqSize(int seqsize);
	virtual void	FocusState(bool focusstate);
	virtual void	SetViewPt(DisplWin *ViewWin);
	virtual void	SetViewLatLong(float lat, float lng);
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks();
	// void SetWinParams(DisplWinParams *params=0);
	// set window name,pos,icon state, DblBuff - if params=0 use current
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	dBZWidClosed();
	virtual float	minVal(); 
	virtual bool	mergeMode() { return MergeMode; }; 
	virtual void    openMindBZW(float mindbz = -9999);
	virtual void	NewVal(float newval);    // set new min value
	virtual void	ValWidClosed();
	virtual void	OpenStnWid(char *newtitle = 0, int *stn = 0, int knownonly = -1);
	virtual void	StnWidClosed();
	virtual void    SelPrevStn();
	virtual void    SelNextStn();
	virtual void    SelPrevStn_StnWin();
	virtual void    SelNextStn_StnWin();
	virtual void    SelPrevStn_GroupWin();
	virtual void    SelNextStn_GroupWin();
	virtual void	SetNewStn(int stn);
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	SetMergeMode(bool mergemode);
	virtual void	ImgDeleted(rdr_img *delimg); // img deleted, check for references to it
	virtual void	drawCoverage(rdr_scan *scan) {};
	virtual void    drawCoverage(int stn, float maxrng) {};
	virtual void	drawMrgCoverage(list<rdr_scan*> &mrglist) {};
	virtual void	DrawAllUnderOverlay(int olaystn = 0, time_t rendertime = 0); // Overlay AND Underlay

  // coverage, terrain, underlay maps, rng rings etc.
	virtual void	DrawUnderlays(rdr_scan *scan) {};  // maps, rng rings etc.
	virtual void	DrawUnderlays(int olaystn = 0, float coverage_rng = -1) {};
	virtual void	DrawMrgUnderlays(list<rdr_scan*> &mrglist) {};  // maps, rng rings etc.

  // overlay maps, overlay plane bits, ref  
	virtual void	DrawOverlays(int olaystn = 0) {}; 
	virtual float   WinSizeScale() { return winSizeScale; };
	virtual float   WinScaleRatio() { return winScaleRatio; };
	virtual float   WinKmPerPixelX() { return winKmPerPixelX; };
	virtual float   WinKmPerPixelY() { return winKmPerPixelY; };
	virtual void	thisDOSName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL);
	virtual void	thisFileName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL, bool datedir = false);
	virtual void	latestFileName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL);
	virtual void	thisGifName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL, bool datedir = false);
	virtual void	latestGifName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL);
	virtual void	thisJpegName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL, bool datedir = false);
	virtual void	latestJpegName(char *namestr, char *pathstr = NULL, rdr_img *img = NULL);
	bool            outputPNG;
	virtual int	writeJpeg(char *filename, 
				CData *pCData = 0, 
				int quality = 100);
	virtual int	writeJpeg(char *filename, 
				rdr_img *Img, 
				int quality = 100);
	virtual int	writeGif(char *filename, 
				CData *pCData = 0);
	virtual int	writeGif(char *filename, 
				rdr_img *Img = 0);
	virtual void	CheckImageRender(CData *pCData);// check whether new data needs new image out
	virtual void	CheckImageRender(rdr_img *Img); // check whether new scan needs new image out
	virtual void	CheckImageRenderList(); // check whether new scan needs new image out
			// rendered to GIF/JPEG
	ImgRenderNode*	isInRenderList(CData *pCData);
	ImgRenderNode*	isInRenderList(rdr_img *Img);
	void            setDisplNeighborLinkMode(bool linkneighbors, float linkrng = 0);
	void            setDisplLinkState(bool link);

  mouseState    MouseState;
  bool          mouseMoved() { return (MouseState.dx || MouseState.dy); }
  virtual bool  mouseButtonDown(int button_no) { return MouseState.buttonDown(button_no); }
  inline bool   LeftMouse() { return mouseButtonDown(1); }
  inline bool   MiddleMouse() { return mouseButtonDown(2); }
  inline bool   RightMouse() { return mouseButtonDown(3); }
  inline bool   MouseShiftKey() { return MouseState.shiftKey(); };
  inline bool   MouseCtlKey() { return MouseState.ctlKey(); };
  inline bool   MouseAltKey() { return MouseState.mod1Key(); };
  bool          ShiftKey;
  bool          CtlKey;
  bool          AltKey;

  bool enablePalette, palVertical;
  WinXYWH palViewport;  // palette viewport
  virtual void drawPalette(float indval = 0, float indval2 = 0) {};
  virtual void setPaletteViewPort(int X, int Y, int W, int H)
    {
      palViewport.set(X, Y, W, H);
    };

#ifdef USE_TITAN_CLIENT
  drawTitan *titanRenderer;
  virtual void  setTitanRenderer(int stn = -1, int titanidx = -1);          
  virtual bool  doTitanRender(renderProperties *renderProps = NULL, 
			      time_t rendertime = 0,
			      int stn = -1);
  SimpleTitanPropsWindow *titanPropsWidget;
  void          openTitanPropsWidget();
  void          closeTitanPropsWidget();
  void          titanPropsWidgetClosed();
  //  TitanDataTable *titanTableWidget;
  void          openTitanTableWidget();
  void          closeTitanTableWidget();
  void          titanTableWidgetClosed();
#endif
#ifdef USE_WDSS_CLIENT
  drawWDSS *wdssRenderer;
  virtual void  setWDSSRenderer(int stn = -1);          
  virtual void  doWDSSRender(renderProperties *renderProps = NULL, 
			      time_t rendertime = 0,
			      int stn = -1);
  SimpleWDSSPropsWindowClass *wdssPropsWidget;
  void          openWDSSPropsWidget();
  void          closeWDSSPropsWidget();
  void          wdssPropsWidgetClosed();
#endif
  // wdssTableWidget is now really a CellTableWidget
  // and used by titan also so always include it
  WdssDataTable *wdssTableWidget;
  void          openWDSSTableWidget();
  void          closeWDSSTableWidget();
  void          hideWDSSTableWidget();
  void          wdssTableWidgetClosed();
#ifdef USE_RAINFIELDS_CLIENT
  virtual int	drawRainfieldImg(rdr_img *Img = 0) { return 0; };
  Ref<rainfieldImg> currentRfImage;
#endif
};

class ValuePal {
public:
	WinXYWH XYWH;
	float IndVal;
	long Win;
	int Size,Base;
	char titlestr[128];
	ValuePal(int size,int base,WinXYWH *init = 0);
	virtual ~ValuePal();
	virtual void UpDate();
	virtual void SetInd(float indval = 0, float indval2 = 0);
	virtual void popwin();
	void Close();
	void GetPos();									// determine current window pos.
	void GetState(WinXYWH *init);		// load init with current window state
	};

class ReflPal : public ValuePal {
public:
	ReflPal(int size,int base,WinXYWH *init = 0);
	virtual void UpDate();
	virtual void SetInd(float indval = 0, float indval2 = 0);
	};

class HeightPal : public ValuePal {
public:
	HeightPal(int size,int base,WinXYWH *init = 0);
	virtual void UpDate();	
	virtual void SetInd(float indval = 0, float indval2 = 0);
	};

class VILPal : public ValuePal {
public:
	VILPal(int size,int base,WinXYWH *init = 0);
	virtual void UpDate();	
	virtual void SetInd(float indval = 0, float indval2 = 0);
	};

class VelocPal : public ValuePal {
public:
	VelocPal(int size,int base,WinXYWH *init = 0);
	virtual void UpDate();	
	virtual void SetInd(float indval = 0, float indval2 = 0);
	};

class AccumPal : public ValuePal {
public:
	AccumPal(int size,int base,WinXYWH *init = 0);
	virtual void UpDate();	
	virtual void SetInd(float indval = 0, float indval2 = 0);
	};



#endif	/* __DISPL_H */
