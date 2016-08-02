#ifndef	__OGLDISPLRDR_H
#define __OGLDISPLRDR_H

#include "vxsect.h"
#include "cappi.h"
#include "vil.h"
#include "tops.h"
#include "ogldispl.h"
#include "OGlWinRdr.h"
#include "OGlWinRdrPPIRHI.h"
#include "maps.h"
#ifdef USE_TITAN_CLIENT
#include "drawTitan.h"
#endif
#include "rain_acc.h"

class vaRadlRender;

extern float glOrthoDepthNear;
extern float glOrthoDepthFar;

class OGlDisplRdr : public OGlDispl {

 private:
  bool		interpMode;
 public:

  OGlWinRdr     *rdrWindow;   // reference to VkWindow 
  //  virtual bool  rdrWindowIsVisible();
  accum_product *currentAccumProd;
  void          nextRainAccumProd();
  GLuint	lastScanDList;
  bool		lastScanDListValid;
  void          clearLastScanDList();
  int           lastPolyCount;
/*   GLuint	overlayObj; */
/*   bool		overlayObjValid; */
/*   GLuint	underlayObj; */
/*   bool		underlayObjValid; */
  int		overlayObjStn, overlayObjRng;
  char		overlayFileName[256];
  bool		overlayFileExists;
  int           volID;     // preferred volume id to display
  void          setNextVolID();
  bool          volIDMatch(rdr_scan *volScan, int imgvolsstn);
  bool          interpmode() { return interpMode; };
  virtual void  setInterpState(bool state = true, bool force = false);
  virtual void  setDataTypeState(e_data_type datatype = e_refl);
  virtual void  setScanTypeState(e_scan_type newstype);
  virtual void  setMergeState(bool state = true);
  virtual void  setFilterState(bool state = true);
  virtual void	setAllow3DView(bool state = true);	// 
  virtual void  setLinkState(bool state = true); // 0=SameStn, 50=
  virtual void  setLinkNeighborState(bool state, int neighborrng = 0); // 0=neighbor stn link off, else 50, 100 or 150
  virtual void  setTitanOlayState(bool state = true);
  virtual void  setWDSSOlayState(bool state = true);
  virtual void  setObsOlayState(bool state = true);
  virtual void  setLtningOlayState(bool state = true);
  virtual void  setAllGUIState();
  
  CAPPIElevSet *cappiCreator;
  Widget  cappiHtWid;

  tops *topsCreator;
  float MinTopsdBZ;

  virtual void  setStormRelVals(stormRelClass &newvals);
  virtual void  getStormRelVals(stormRelClass &getvals);
  virtual void  checkStormRelVals();  // check for storm rel changes
  virtual void  stormRelValsChanged();  // call if changed, sets gui
  virtual int   getStormRelPalOfs(float az);

  virtual void  HndlXEvent(XEvent *xEvent);
  virtual void  HndlKBD(XKeyEvent *keyEvent);
  virtual void  HndlMouse(XEvent *xEvent);
  virtual void	winContextCreated();	// called by OGlWin when context has been created
  virtual void	CloseWin();
  virtual void	MoveWin(long NewX,long NewY, long w = 0, long h = 0);
  virtual void	ResizeWin(long w, long h);
  virtual void	GetWinSizePos();
  virtual void	NewTitle(char *newtitle = 0);
  virtual void	CalcCurPos(CursorDataStruct *cursordata = 0);
  virtual void	GetCurData();
  virtual void  SetPointOfView(bool povchanged = false);
  virtual void  setRng(float rng, bool forceredraw = false);
  virtual void  setOfs(float ofsx, float ofsy, bool forceredraw = false);
  virtual void  setOfsDelta(float ofsdeltax, float ofsdeltay, bool forceredraw = false);
  virtual void  setRotView(short viewaz, short viewel, bool forceredraw = false);
  virtual void  UpdateCursorData(char *datastring);
  virtual void	UpdateData();
  virtual void	updateSeqPos(int seqpos);
  virtual void	updateSeqSize(int seqsize);
  virtual void  drawCoverage(rdr_scan *scan);
  virtual void  drawCoverage(int stn, float maxrng);
  virtual void	drawMrgCoverage(list<rdr_scan*> &mrglist);
  virtual void  drawLegacyMap(rpmap *legacyMap, float ViewRng, 
			      OverlayProperties *OlayProps,
			      renderProperties *renderProps = 0);
  virtual void  drawMaps(int olaystn);
  virtual renderProperties* getRenderProps(renderProperties *renderprops);
  virtual renderProperties* getRenderProps()
    {
      return DisplWin::getRenderProps();
    };
  virtual void	DrawAllUnderOverlay(int olaystn = 0, time_t rendertime = 0); // Overlay AND Underlay
  // coverage, terrain, underlay maps, rng rings etc.
  virtual void	DrawMrgUnderlays(list<rdr_scan*> &mrglist);  // maps, rng rings etc.
  virtual void	DrawUnderlays(rdr_scan *scan);  // maps, rng rings etc.
  virtual void	DrawUnderlays(int olaystn = 0, float coverage_rng = -1); 

  // overlay maps, overlay plane bits, ref  
  virtual void	DrawMrgOverlays(list<rdr_scan*> &mrglist);  // maps, rng rings etc.
  virtual void	DrawOverlays(int olaystn = 0);

  virtual void  drawUnderlayRings();
  virtual void  drawRngRings(float NorthOfs,float EastOfs,float RngInc,int NumRings);
  virtual void  draw_RHI_grid(float ViewHt, float rng, float rngofs, OverlayProperties *olayprops = 0);
  virtual void  draw_3DRHI_grid(float ViewHt,rdr_angle az, 
				float ECCMult = 1.0);
  virtual void  draw_VxSect_grid(float ViewHt, 
				 VertXSect *vxsect, OverlayProperties *olayprops = 0);
  virtual void  draw_3DVxSect_grid(float ViewHt, float ViewRng, 
				   VertXSect *vxsect, OverlayProperties *olayprops = 0);
  overlayLayers olayLayers;
  virtual void  setOlayLayerState();
  virtual void  setOlayLayerState(overlayLayers& olaylayers);
  virtual void  initOlayLayerState();
  virtual void  initOlayLayerState(overlayLayers& olaylayers);
  virtual void  toggleOlayLayerState(int layer);
  virtual void  setOlayLayerState(int layer, bool state);
  virtual bool  getOlayLayerState(int layer);

  virtual void  setShowWDSS(bool state = true);
  virtual void  setShowTitan(bool state = true);
  virtual void  setShowObs(bool state = true);
  virtual void  setShowLightning(bool state = true);

  virtual void	SetNewStn(int stn);
  virtual bool  setDataType(e_data_type newdtype);
  virtual void	ClearLatLongCursor();
  virtual void	MoveLatLongCursor(LatLongHt *CursorLatLong);
  virtual void	DrawLatLongCursor(LatLongHt *CursorLatLong);
  virtual void 	DrawRefPoint();
  virtual void	DrawScanData(char *prefix = 0);	// if ON, write scan data text in window
  virtual int   DrawLatLongGrid(float LatLongRes);
  
  GLuint	worldCoreDlist;
  virtual int   DrawWorldCore(float LatLongRes = 5.0);
  OGlDisplRdr(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
	      DisplWinParams *params=0);
  virtual ~OGlDisplRdr();
  virtual void	WriteCurrentScanFile(char *fname = 0);
  virtual void	WriteASCIILatLongHt(char *fname, bool FullVolume = FALSE) {};
#ifdef USE_TITAN_CLIENT
  virtual void	DrawMrgTitan(rdr_img *Img = 0);
  virtual void	DrawMrgTitan(list<rdr_scan*> &mrglist);
#endif
#ifdef USE_WDSS_CLIENT
  virtual void	DrawMrgWDSS(rdr_img *Img = 0);
  virtual void	DrawMrgWDSS(list<rdr_scan*> &mrglist);
#endif
  virtual bool		drawModifiedRadlVel()     // if true draw radl velocity relative to cursor value
  {
    return DisplWin::drawModifiedRadlVel() && rdrWindow &&
      rdrWindow->stormRelDialog;
  };
  virtual void	openAnnotateDialog(int mposx, int mposy,
				   int mrootx, int mrooty);
};

class Gl3DPPIRHI;
class rpRadlVertBuffer;
/* 
	Traditional 2D PPI with infinite pan/zoom capabilities
	May be linked to an RHI to allow RHI az control from PPI and 
	vice-versa
*/

class GlPPI : public OGlDisplRdr {
	friend class GlRHI;
	friend class GlPPIRHI;
	friend class Gl3DPPIRHI;
	friend class rdr_seq;

protected:
	float		AzInd;			// Currently highlighted Az
	float		vel_zaxis_factor;	// velocity z-axis multiplier
	float		RadlVelIndAz, RadlVelIndRng;
    	bool		ShowRadlVelInd;
	short		CurPosX,CurPosY;	// rel. cursor pos in window
	rdr_angle	ReqEl,SetEl,TitleEl;	// requested azim. (set to closest)
	float		MinRng,MaxRng;
	int		RHILinkNo;		// WinNum of link
	int 		PPIRHILinkNo;		// WinNum of link
	bool            drawVXSectInd;          // allow child class to enable vxsect ind drawing
	Gl3DPPIRHI	*Gl3DPPIRHILink;        // link to 3DPPIRHI window
	int 		Gl3DPPIRHILinkNo;	// WinNum of link
	bool            testMrgRadius;
	stnSet          mergeStnSet;
	stnSet          volStnSet;
	virtual bool	OpenWin(char* WinName);
	virtual bool    scanMatches(rdr_scan *checkscan); // return if scan matches props
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawMrgImg(rdr_img *Img = 0);
#ifdef USE_RAINFIELDS_CLIENT
	virtual int	drawRainfieldImg(rdr_img *Img = 0);
#endif
	int             rfDisplayProd;  // rf product selected 
	int             rfDisplayProdMax;  // rf product selected 
	oglTextureTile  *rfTile;
	
	virtual int	DrawScan(rdr_scan* RScan);
	virtual int	DrawRadl(s_radl* radl, bool IgnoreValues = FALSE);
	virtual int	DrawRadlNew(s_radl* radl, bool IgnoreValues = FALSE);

	void            allocVARender(); // endure vaRadlRenderer alloc'd
	void            setupVARender(rdr_scan* RScan);
	void            finishVARender();
	virtual int	DrawRadlVA(s_radl* radl, bool IgnoreValues = FALSE);
	bool            useVARender;
	rpRadlVertBuffer *vaRadlRenderer;

	virtual int	DrawInterpScan(rdr_scan* RScan);
	virtual int	DrawInterpRadl(s_radl* Radl1, s_radl* Radl2, 
			bool IgnoreValues = FALSE);
	virtual int	NextPPIUp();
	virtual int	NextPPIDown();
	virtual void	CalcCurPos(CursorDataStruct *cursordata = 0);				 
	// virtual void	OpenDataWin();
	// virtual void	CloseDataWin();
	virtual void	UpdateData();
	virtual void    UpdateCursorData(char *string);
	virtual void	HndlXEvent(XEvent *xEvent);
	virtual void	HndlKBD(XKeyEvent *keyEvent);
	virtual void    HndlMouse(XEvent *xEvent);
	virtual void	NewReqEl(rdr_angle);
	VertXSect	*VXSect, *VXSect1;	// vert xsect objects, (VXSect1 for dual section)
	VXSECTMODE	VXSectMode;		// RHI/VXSect
	float		VXSkmn1, VXSkmn2, VXSkme1, VXSkme2, 
			DVXSkmn, DVXSkme, DVXSrng;

	rdr_scan *getProdScan(rdr_scan *baseScan);
	VILUtils *VILUtil;
	float MaxVIL;
	bool VILOverflow; 
	float VILTop;


    	GlPPI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	virtual ~GlPPI();
	virtual void	CloseWin();
	virtual void 	SetAzInd(float NewAz);
	virtual void 	DrawAzInd();
	virtual void 	MoveAzInd(float NewAz);
	virtual void 	SetRadlVelInd(float NewAz, float NewRng);
	virtual void 	DrawRadlVelInd();
	virtual void 	MoveRadlVelInd(float NewAz, float NewRng);
	virtual void 	SetVXSectInd();
	virtual void 	DrawVXSectInd();    // use RHILink's VXSect for data
	virtual void 	DrawIndicators();
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	SetNewStn(int stn);
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks(); 
	virtual void	thisFileName(char *namestr, char *pathstr = 0, rdr_img *img = 0);
	virtual void	WriteASCIILatLongHt(char *fname, bool FullVolume = FALSE);
	virtual void	WriteASCIILatticeCoords(s_radl* Radl1, int MaxCells, LatLongHt *origin, FILE *file);
	virtual bool    ShouldMerge(int stnid);	    // true if stn will contribute to merge in this window
 public:
	virtual float   velZaxisFactor() { return vel_zaxis_factor; };
	virtual void	SetMergeMode(bool mergemode);
	virtual void	NewTitle(char *newtitle = 0);
	virtual void    openMindBZW(float mindbz = -9999);
	virtual void	FocusState(bool focusstate);

	virtual int	DeriveRHI(rdr_img*);
	virtual int	DrawDerivedRHI(rdr_scan *scan) { return -1; };
	virtual void	AddRadlNode(s_radl_node*);
	virtual void    setEl1El2NodeValues(rdr_scan *scan);
	virtual bool    setDataType(e_data_type newdtype);
	virtual bool    setScanType(e_scan_type newstype);
	virtual bool    setCAPPIMode();
	virtual bool    setPPIMode();
	s_radl_node	*StartNode,*EndNode;
	int		NodeCount;
	rdr_angle	ReqAz,SetAz,TitleAz;	// requested azim. (set to closest)
	void    cappiWidClosed();
	void	NewCAPPIMode(CAPPImode newmode); // set nearest/interp mode
	void	NewCAPPIHt(float newht); // set nearest/interp mode
	int             rngres_reduce_factor;
	e_rngres_reduce_mode rngres_reduce_mode;
	bool            useVxSectSubSteps;
	virtual void    setWinGroup(int _wingroup);
	bool useCgFragmentPalette();
#ifdef USE_CG
	void setCgFragmentPalette();
	void closeCgFragmentPalette();
#endif
};

class GlPPIRHI : public GlPPI {
	friend class rdr_seq;
	
	OGlWinRdrPPIRHI *rdrPPIRHIWindow;       // pointer to PPIRHI Window
	oglwid_props    *oglRHIwidProps;        // pointer to the RHI opengl widget properties
	OverlayProperties *RHIOlayProps;
	
	long		RHIOrgX,RHIOrgY,RHIW,RHIH;	// RHI ogl widget's pos & size in pixels
	float		RHIwinWidth, RHIwinHeight,    // window dimensions etc in projection units,   
	  RHIwinCentreX, RHIwinCentreY,               // e.g. degrees, kms
	  RHIwinStartCentreX, RHIwinStartCentreY,
	  RHIwinScale,	                        // winScale is zoom ratio
	  RHIwinSizeScale,	                        // winSizeScale is scale against 512 pixel window
	  RHIwinScaleRatio,
	  RHIwinKmPerPixelX, RHIwinKmPerPixelY;
	short		CurPosX,CurPosY;	// rel. cursor pos in window
	float		ElInd;			// Currently highlighted El
	float		RHIRng,RHIOfs;
	float		MinRHIRng,MaxRHIRng;
	float		MinHeight,MaxHeight;
	float		RHILower;
	bool            redrawRHIOnly;
	bool            redrawPPIOnly;
	virtual bool	OpenWin(char* WinName);
	virtual bool	makeGlRHIWidgetCurrent();
	virtual void	CheckWin(rdr_img *Img = 0);		// perform housekeeping functions
	virtual void	ShowNewImg(int seqpos);
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
	virtual int	DrawRHIRadl(s_radl*, bool drawallcells = false);
	virtual int	DrawRHIInterpRadl(s_radl*,s_radl*);
	virtual int	DrawVXSect(rdr_img *Img = 0);
	virtual void	NewVXSect();
	virtual void 	SetVXSectInd();
	virtual void 	DrawVXSectInd();
	virtual void 	SetElInd(float NewEl);
	virtual void 	DrawElInd();
	virtual void	MoveLatLongCursor(LatLongHt *CursorLatLong);
	virtual void 	DrawIndicators();
	virtual void    UpdateCursorData(char *datastring);
	virtual bool	NewCentreRng(float newrng);
	virtual void	CalcCurPos(CursorDataStruct *cursordata = 0);				 
	virtual void	CalcCurPosRHI(CursorDataStruct *cursordata = 0);				 
	virtual void	CalcCurPosVxSect(CursorDataStruct *cursordata = 0);				 
	//	virtual void	OpenDataWin();
	//	virtual void	CloseDataWin();
	virtual void	UpdateData();
	virtual void	updateSeqPos(int seqpos);
	virtual void	updateSeqSize(int seqsize);
	virtual void	HndlKBD(XKeyEvent *keyEvent);
	virtual void    HndlMouse(XEvent *xEvent);
	virtual void    HndlMouseRHI(XEvent *xEvent);
	virtual void    HndlMousePPI(XEvent *xEvent);
	virtual void	NewReqAzRng(rdr_angle newaz, float newrng);
	virtual int	DrawDerivedRHI(rdr_scan *scan);
	GlPPIRHI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	virtual ~GlPPIRHI();
	virtual void	CloseWin();
	virtual void	SetRngOfs(float SRng,float SOfsX,float SOfsY);
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks(); 
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
 public:
	virtual void	NewTitle(char *newtitle = 0);
	bool            hasFocusRHI;
	virtual void	FocusState(bool focusstate);
	// virtual void	FocusState(bool focusstate, bool isRHI);
	virtual void	SetPointOfView(int vxsectnum = 1,
				       bool povchanged = false);
	virtual void	HndlXEvent(XEvent *xEvent);
	virtual void	HndlXEventRHI(XEvent *xEvent);
	virtual void	openAnnotateDialog(int mposx, int mposy,
					   int mrootx, int mrooty);

	virtual void  setInterpState(bool state = true);
	virtual void  setDataTypeState(e_data_type datatype = e_refl);
	virtual void  setScanTypeState(e_scan_type newstype);
	virtual void  setMergeState(bool state = true);
	virtual void  setFilterState(bool state = true);
	virtual void  setAllow3DView(bool state = true);	// 
	virtual void  setLinkState(bool state = true); // 0=SameStn, 50=
	virtual void  setLinkNeighborState(bool state, int neighborrng = 0); // 0=neighbor stn link off, else 50, 100 or 150
	virtual void  setTitanOlayState(bool state = true); // 0=SameStn, 50=
	virtual void  setWDSSOlayState(bool state = true); // 0=SameStn, 50=
	virtual void  setAllGUIState();

	virtual void  setOlayLayerState();
	virtual void  setOlayLayerState(overlayLayers& olaylayers);
	virtual void  initOlayLayerState();
	virtual void  initOlayLayerState(overlayLayers& olaylayers);
	virtual void  toggleOlayLayerState(int layer);
	virtual void  setOlayLayerState(int layer, bool state);
	virtual bool  getOlayLayerState(int layer);
	virtual int   glWriteJpeg(char *filename,        // single implementation for both rdrimg and pCData
				rdr_img *Img, 
				CData *pCData,
				oglwid_props  *_oglwidprops = NULL, 
				int quality = 90);
	virtual void  stormRelValsChanged();  // call if changed, sets gui
	virtual bool		drawModifiedRadlVel()     // if true draw radl velocity relative to cursor value
	{
	  return DisplWin::drawModifiedRadlVel() && rdrPPIRHIWindow &&
	    rdrPPIRHIWindow->stormRelDialog;
	};
};

/*
	Builds on GlPPI, with addition of RHI and off axis viewing
	Az & El controlled via NewReqAz & NewReqEl
	Typically a GlPPIRHI will link to this to control the az/el

*/
class Gl3DPPIRHI : public GlPPI {
	friend class GlPPI;
	friend class GlPPIRHI;
	friend class rdr_seq;
	
	float		ht_mult;
	float		ElInd;			// Currently highlighted El
	bool		RHIGridSw;		// enable/disable RHI grid display
	rdr_scan	*CurrentPPI,*CurrentRHI;

	virtual void	NewReqAz(rdr_angle NewAz);
	virtual void	NewReqEl(rdr_angle NewEl);
	virtual void	NewVXSect();
	virtual void 	DrawAzElInd();
	//	virtual int	DeriveRHI(rdr_img*);
	virtual int	DrawDerivedRHI(rdr_scan *scan);
	//	virtual int	DrawDerivedRHI(rdr_img *Img = 0);
	virtual void	HndlXEvent(XEvent *xEvent);
	virtual void	HndlKBD(XKeyEvent *keyEvent);
	virtual void    HndlMouse(XEvent *xEvent);
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawPPI(rdr_img*);
	virtual int	DrawRHI(rdr_img*);
	virtual int	DrawVXSect(rdr_img *Img = 0);
	Gl3DPPIRHI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	virtual ~Gl3DPPIRHI();
	virtual void 	DrawVXSectInd();    // use RHILink's VXSect for data
	virtual void 	DrawIndicators();
	virtual void	CloseWin();
	GlPPI	        *PPILink; 
	int 		PPILinkNo;		// WinNum of link
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetPointOfView(bool povchanged = false,
				       bool allowHtScale = false);
	virtual void	SetLinks(); 
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	SetNewStn(int stn);
	virtual int	NextPPIUp();
	virtual int	NextPPIDown();
	};

class GlCAPPI : public GlPPI {
	friend class rdr_seq;

	CAPPIElevSet	*CAPPIElSet;
	GlCAPPI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
			DisplWinParams *params=0);
	virtual ~GlCAPPI();
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
//	virtual int	DrawRadl(s_radl*);
	virtual int	DrawInterpScan(rdr_scan*);
//	virtual int	DrawInterpRadl(s_radl* Radl1, s_radl* Radl2);
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void	HndlKBD(short, bool&);
public:
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	NewVal(float newval);    // set new min value
	void		SetMode(CAPPImode newmode); // set nearest/interp mode
	};

class GlVIL : public GlPPI {
	friend class rdr_seq;

	VILUtils    *VILUtil;
	float	    MaxVIL; // max value within this Image
	bool	    VILOverflow;    // true if a VIL value exceeded the VILTop 
	GlVIL(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
			DisplWinParams *params=0);
	virtual ~GlVIL();
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
	virtual int	DrawInterpScan(rdr_scan*);
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void	HndlKBD(short, bool&);
	virtual void	CalcCurPos(CursorDataStruct *cursordata = 0);				 
	virtual void	UpdateData();
	virtual void	ScaleVILRadl(s_radl *vilradl);	// convert vil values to index values
	virtual void 	SetVILCMap(); // set color map based on the root scan's level table
public:
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewVal(float newval);    // set new min value
	};

typedef	unsigned short	TopRadl[512];
typedef TopRadl TopArr[360];

class GlTops : public OGlDisplRdr {
    friend class rdr_seq;
    
    float	ht_mult;		
    int		RdrHtMapBase;
//    int		htmapsize;
    int		DrawRes;
    short CurPosX,CurPosY;          // rel. cursor pos in window
    float MinRng,MaxRng;
    TopRadl *TopArray;								// result array	
    bool TopsOverflow;
    // result polar array 1deg x 512km
											// Tops elements in metres abv sea level
    virtual bool	OpenWin(char* WinName);
    void CalcTopArray(rdr_img*);
    int	DrawAz1,DrawAz2;
    int DrawTopArr();
    int DrawTopsRadl();
    void ShowNewImg(int seqpos);
    void CalcCurPos(CursorDataStruct *cursordata = 0);        
    //    void OpenDataWin();
    // void CloseDataWin();
    void UpdateData();
    void HndlUsrIFace(short,short,bool&);
    void HndlKBD(short, bool&);
    void FocusLost();
    void ReduceRadials(unsigned short*,unsigned short*,int&,int&);
    GlTops(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		    DisplWinParams *params=0);
    virtual ~GlTops();
    virtual void	CloseWin();
    virtual void PutParams(DisplWinParams *params);
    virtual void GetParams(DisplWinParams *params);
    // virtual void NewTitle(char *newtitle = 0);
    virtual void	NewdBZ(float newdbz);    // set new min dBZ value
public:
    int DrawImg(rdr_img *Img = 0);
    int DrawTopsImg(rdr_img* Img,bool ForceNewArray = FALSE);
    };

/*
class CZBlkParams {
public:
    rdr_angle	az, az1, az2, el, el1, el2;
    float	SinEl1,SinEl2,CosEl1,CosEl2, 
		SinAz1CosEl1,SinAz1CosEl2,
		SinAz2CosEl1,SinAz2CosEl2,
		CosAz1CosEl1,CosAz1CosEl2,
		CosAz2CosEl1,CosAz2CosEl2,
		SinEl, CosEl, 
		SinAzCosEl, CosAzCosEl;

// use s_radl to set angles OR set public angles, then call this
    void	CalcAngles(s_radl *setradl = 0); 
    CZBlkParams();

    void	CalcBoxVertices(float r1, float r2); // calc box vertices
    float	boxv[8][3];    // eight vertices, in the following order
*/
/*  
    0-az1el1r1 1-az1el2r1 2-az2el1r1 3-az2el2r1
    4-az1el1r2 5-az1el2r2 6-az2el1r2 7-az2el2r2
*/
/*
    void	CalcCentreVertices(float r1, float r2);
    float       centrev[2][3];
*/
/*    
    0-azelr1   1-azelr2
*/
/*
    };
*/
/*
class OGlCZBlocks : public OGlDispl {
	friend	class	rdr_seq;

    float	ht_mult;		
    int		RdrHtMapBase;
    float	MinRng,MaxRng;
    int		htmapsize;
    short	CurPosX,CurPosY;          // rel. cursor pos in window
											// Tops elements in metres abv sea level
    CZBlkParams	params;
    virtual OGlWin	*OpenWin(char* WinName);
    int DrawImg(rdr_img *Img = 0);
    virtual int	DrawScan(rdr_scan*);
    void DrawBox(float r1 = -1,  float r2 = -1);
    int DrawCZBlkRadl(s_radl *radl);
    void ShowNewImg(int seqpos);
    void CalcCurPos(CursorDataStruct *cursordata = 0);        
    void OpenDataWin();
    void CloseDataWin();
    void UpdateData();
    void HndlUsrIFace(short,short,bool&);
    void HndlKBD(short, bool&);
    void FocusLost();
    OGlCZBlocks(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		    DisplWinParams *params=0);
    ~OGlCZBlocks();
    virtual void	CloseWin();
    virtual void PutParams(DisplWinParams *params);
    virtual void GetParams(DisplWinParams *params);
    // virtual void NewTitle(char *newtitle = 0);
    virtual void	NewdBZ(float newdbz);    // set new min dBZ value
    };
*/

enum radlVertHtMode { rvh_undef, rvh_noCorr, rvh_beamRefr, rvh_earthCurv };

class radlVertGeom
{
 public:
  float cosEl, rngCosEl,
    sinEl, rngSinEl,
    sinAz1CosEl, sinAz2CosEl,
    cosAz1CosEl, cosAz2CosEl;
  float startRng, rngRes, rng;
  radlVertHtMode htMode;
  GLfloat *vertArr;
  // only need to set cosel & vertarr once per PPI
  radlVertGeom()
    {
      vertArr = NULL;
      htMode = rvh_undef;
    };
  void setScan(rdr_scan* RScan, GLfloat *vertarr)
  {
    if (RScan)
      {
	switch (RScan->scan_type)
	  {
	  case PPI:
	  case VOL:
	    cosEl = cos(RScan->set_angle * RDRANG2RAD);
	    sinEl = sin(RScan->set_angle * RDRANG2RAD);
	    htMode = rvh_beamRefr;
	    break;
	  case CompPPI:
	  case CAPPI:
	  default:
	    cosEl = 1.0;
	    sinEl = 0.0;
	    htMode = rvh_earthCurv;
	    break;
	  }
	startRng = float(RScan->start_rng)/1000.0;
	rngRes = float(RScan->rng_res)/1000.0;
      }
    else
      cosEl = 1.0;
    vertArr = vertarr;
  };
  // set up the geometry factors for radl
  void setRadl(s_radl *radl)
  {
    if (!radl) return;
    sinAz1CosEl = sin(radl->az1 * RDRANG2RAD) * cosEl;
    cosAz1CosEl = cos(radl->az1 * RDRANG2RAD) * cosEl;
    sinAz2CosEl = sin(radl->az2 * RDRANG2RAD) * cosEl;
    cosAz2CosEl = cos(radl->az2 * RDRANG2RAD) * cosEl;
  };
  // write x y z coords for radl index
  void setVerts(int vertidx, int cellidx)
  {
    GLfloat *xptr = vertArr + (vertidx*3);
    rng =  startRng + (cellidx * rngRes);
    rngCosEl = rng * cosEl;
    rngSinEl = rng * sinEl;
    float ht = 0.0;
    switch (htMode)
      {
      case rvh_beamRefr :
	ht = BeamRefrCorr(rng) + rngSinEl;  // 
	break;
      case rvh_earthCurv :
	ht = -EarthCurvCorr(rng);  // 
	break;
      default :
	break;
      }
    *(xptr++) = sinAz1CosEl * rng;
    *(xptr++) = cosAz1CosEl * rng;
    *(xptr++) = ht; // 
    *(xptr++) = sinAz2CosEl * rng;
    *(xptr++) = cosAz2CosEl * rng;
    *(xptr++) = ht; // 
  };
};

// radial specific vertex array class
// Currently suitable for flat shaded PPI radial only
// NOTE: 2 vertices per element, so mult buffsize x 2
// Need to choose ht mode - 
//   rvh_noCorr   - straight uncorrected elev beam
//   rvh_beamRefr - 4/3 earth curvature corrected hts - std PPI mode
//   rvh_earthCurv- follow earth curvature - merged compPPI & CAPPI
// Assumes az = 0, use glRotate before using as appropriate for radial az
// Xlat z axis for CAPPI ht
// Use indices in alpha for min val threshold hiding using glAlphaFunc 
// Designed for use with OpenGL Vertex Array or VertexBufferObj 
// infrastructure - 
// NOTE - VertexBufferObject implementation currently crashes - 
// so not being used
// (OGlV1.2 onwards)

class rpRadlVertBuffer : public rpVertexBuffer 
{
 public:
  rpRadlVertBuffer(radlVertHtMode htmode = rvh_undef, 
		   int buffsize = 1024) : rpVertexBuffer(0)
    {
      init();
      htMode = htmode;
      setBuffSize(buffsize);
    };
  
  rpRadlVertBuffer(rdr_scan* RScan) : rpVertexBuffer(0)
    {
      init();
      setBuffSize(radlVerts(1024));    // start with reasonable default
      setHtMode(RScan);
      /*       setVerts(); */     // will be set by setHtMode
    };
  rpRadlVertBuffer(rpRadlVertBuffer *srcbuffer) 
    : rpVertexBuffer(0)
    {
      if (srcbuffer)
	{
	  copy(srcbuffer);  // deep copy of srcbuffer
	}
    };
  void init();
  radlVertHtMode htMode;
  float startrng, rngres, azres;
  GLfloat *htBuff;

  bool setUsePackedVerts(bool state = true); // 
  GLfloat *_radlVBuff; // radial vertex geometry buffer
  void setPackedVerts(int idx, int cell); // copy 2 verts for cell
  void setPackedVert(int idx, int cell); // copy singele vert for cell
  void setMRPackedVerts(int idx, int cell);
  bool setUseMultiRadlVA(bool state = true);
  bool useMultiRadlVA; // if set use mutli radl packed writes
  void mrCopyLast();   // copy last vertex pair to start of new VA
  void mrRenderBuffs(bool copylast = true);
  int  mrCount;       // multi radl packed VA index
  radlVertGeom *mrRadlVertGeom;
  void printState();
  float minVal;
  int   minData;  // min data index value
  float htModFactor; 
  e_scan_type scan_type;
  RGBPalette *currentRGBPal; // pointer to current RGBPalette
  short *palCMap;
  bool htBuffChanged;
  bool vBuffSame;
  bool radlMatch(s_radl *radl);
  void restoreHtBuff();  // 
  virtual ~rpRadlVertBuffer();  
  int setRadl(s_radl *radl, int celldataofs = 0);
  int radlVerts(int cellcount); // convert cell count to reqd vertexes
  int buffCellSize();    // get cell limit from buffSize (size/2 - 1)
  virtual bool setBuffSize(int buffsize);
  virtual bool setBuffSize(s_radl *radl);
  void setVerts(s_radl *radl); // set radl Vertices
  void setHtMode(radlVertHtMode htmode);
  void setHtMode(rdr_scan* RScan);
  // default to copying only copying vbuff, color & indices vary per radial
  virtual void copy(rpRadlVertBuffer *srcbuffer,
		    bool do_v = true, 
		    bool do_c = false, bool do_i = false)
    {
      if (srcbuffer)
	{
	  rpVertexBuffer::copy(srcbuffer, do_v, do_c, do_i);
	  htMode = srcbuffer->htMode;
	}
    };
  GLfloat *vBuffHt(int index = 0)  // return pointer to vBuffHt (z) at index
    {
      if (index >= buffSize) index = buffSize-1;
      if (_vBuff) return _vBuff + (index * 3) + 2;
      else return NULL;
    };
  // set the colors & indices for radl
  int setColorsIndices(s_radl *radl, int celldataofs = 0); 
  /* 	int mrRenderRadl(s_radl *radl); */
  // modulate ht if reqd
  int renderRadl(s_radl *radl, int celldataofs = 0);
  void setInitialGeometry(rdr_scan* RScan);
  void setupRender(rdr_scan* RScan, 
		   RGBPalette *current_pal,
		   short *palcmap,   // map indexes to pal offsets
		   float minval,
		   float ht_mult_factor = 0.0);
  void finishRender();
};

#endif	/* __OGLDISPLRDR_H */
