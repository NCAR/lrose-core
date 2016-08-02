#ifndef	__GLDISPL_H
#define __GLDISPL_H

#include "gldtawin.h"
#include <X11/Intrinsic.h>
#include <GL/gl.h>
#include <device.h>
#include "vil.h"
#include "vxsect.h"

struct covObj {
    GL_Object covobj1, covobj2;
    float  maxrng;
    covObj(GL_Object ob1 = NULL, GL_Object ob2 = NULL, float rng = 440);
    ~covObj();
};

class GlWin : public DisplWin {
protected:
	GlWin(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName = 0,
		DisplWinParams *params=0); 	// new window always appended
	~GlWin();
	// virtual void Lvl2Color();
	short		CurPosX,CurPosY;	// rel. cursor pos in window
	rdr_angle	ReqEl,SetEl;		// requested elev. (set to closest)
	rdr_angle	ReqAz,SetAz;		// requested azim. (set to closest)
#ifdef TITAN
        int             showTitan;
#endif

public:
	long		WinHandle;
	GL_Object	overlayObj;
	bool		overlayObjValid;
	int		overlayObjStn, overlayObjRng;
	char		overlayFileName[256];
	bool		overlayFileExists;
	bool		allow3DView;
	virtual void	CalcCurPos();				 
	virtual void	GetCurData();
	virtual void	NewTitle(char *newtitle = 0);
	virtual long	OpenWin(char*);
	virtual void	Raise();
	virtual void	Lower();
	virtual void	CloseWin();
	virtual void	MoveWin(long NewX,long NewY, long w = 0, long h = 0);
//	virtual void	MoveWin(long NewX,long NewY);
	virtual void	ResizeWin(long w, long h);
	virtual void	DrawScanData(char *prefix = 0);	// if ON, write scan data text in window
	virtual void	DrawAxis(float length = 0);
	virtual void	ClearLatLongCursor();
	virtual void	MoveLatLongCursor(LatLongHt *CursorLatLong);
	virtual void	DrawLatLongCursor(LatLongHt *CursorLatLong);
	virtual void 	DrawRefPoint();
	virtual void 	DrawIndicators();
	virtual void	drawCoverage(rdr_scan *scan);
	virtual void	drawCoverage(int stn, float maxrng = 0);
	virtual void	DrawAllOverlay(int olaystn = 0); // Overlay AND Underlay
	virtual void	DrawUnderlay(int olaystn = 0);	 // maps, rng rings etc.
	virtual void	DrawOverlay(int olaystn = 0);	 // overlay plane bits, ref pt, az ind, sync cursor etc.
	virtual void	SetPointOfView();   // by default set 2D Point of view, 3d windows must override
	virtual void	SetNewStn(int stn);
	virtual void	SetMergeMode(bool mergemode);
	virtual void	HndlUsrIFace(short dev,short val,bool& done);
	virtual void    HndlKBD(short &val,bool &done);
	virtual void    ShowStorms(rdr_img *Img ,rdr_scan *TempScan);
	virtual int     SetStormTrack(void );
	virtual long	ReadHandle();
	virtual void	ShowNewImg();  // Usual DrawImg to BG buffer, this shows newimg
	virtual int	writeJpeg(char *filename, 
				rdr_img *Img = 0, 
				int quality = 90);
	virtual int	writeGif(char *filename, 
				rdr_img *Img = 0);
	virtual void	CIRCF(Coord x,Coord y,Coord rad);
	virtual bool	WinXY2LatLong(float winX, float winY,		   
				float &winLat, float &winLong);
	};
	
class Gl3DPPIRHI : public GlWin {
	friend	class	rdr_seq;

	bool		ShowAllPPIRHI;		// if true draw everything
	proj_mode	ProjMode;
	virtual long	OpenWin(char*);
	virtual int	DrawImg(rdr_img *Img = 0);
	int		DrawScan(rdr_scan*);
	int		DrawRadl3d(s_radl*);
	int		DrawRadlBlk3d(s_radl*);
	void		HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
	void		NewReqEl(rdr_angle);
	void		NewReqAz(rdr_angle);
	int		DeriveRHI(rdr_img*);
	int		DrawDerivedRHI(rdr_img *Img = 0);
	Gl3DPPIRHI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	~Gl3DPPIRHI();
	virtual void	CloseWin();
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	};

/* 
	Traditional 2D PPI with infinite pan/zoom capabilities
	May be linked to an RHI to allow RHI az control from PPI and 
	vice-versa
*/
class GlPPI : public GlWin {
	friend class GlRHI;
	friend class GlPPIRHI;
	friend class rdr_seq;

protected:
	float		AzInd;			// Currently highlighted Az
	float		vel_zaxis_factor;	// velocity z-axis multiplier
	rdr_angle	TitleEl;		// 
	float		MinRng,MaxRng;
	GlRHI		*RHILink;		// this PPI's matching RHI
	GlPPIRHI	*PPIRHILink;		// this PPI's matching PPIRHI window
	int		RHILinkNo;		// WinNum of link
	int 		PPIRHILinkNo;		// WinNum of link
	bool		interpmode;
	virtual long	OpenWin(char*);
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawMrgImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
	virtual int	DrawRadl(s_radl*, bool IgnoreValues = FALSE);
	virtual int	DrawInterpScan(rdr_scan*);
	virtual int	DrawInterpRadl(s_radl* Radl1, s_radl* Radl2, 
			bool IgnoreValues = FALSE);
	virtual int	NextPPIUp();
	virtual int	NextPPIDown();
	virtual void	CalcCurPos();				 
	virtual void	UpdateData();
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
	virtual void	FocusState(bool focusstate = false);
	virtual void	NewReqEl(rdr_angle);
	VertXSect	*VXSect, *VXSect1;	// vert xsect objects, (VXSect1 for dual section)
	VXSECTMODE	VXSectMode;		// RHI/VXSect
	float		VXSkmn1, VXSkmn2, VXSkme1, VXSkme2, 
			DVXSkmn, DVXSkme, DVXSrng;
	float		RadlVelIndAz, RadlVelIndRng;
    	bool		ShowRadlVelInd;
	GlPPI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	~GlPPI();
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
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	SetNewStn(int stn);
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks(); 
	virtual void	WriteASCIILatLongHt(char *fname, bool FullVolume = FALSE);
	virtual void	WriteASCIILatticeCoords(s_radl* Radl1, int MaxCells, LatLongHt *origin, FILE *file);
	virtual bool ShouldMerge(int stnid);	    // true if stn will contribute to merge in this window
	};

/*
	Builds on GlPPI, with addition of RHI and off axis viewing
	Az & El controlled via NewReqAz & NewReqEl
*/
class GlPPIRHI : public GlPPI {
	friend class GlRHI;
	friend class GlPPI;
	friend class rdr_seq;
	
	rdr_angle	TitleAz;	// requested elev. (set to closest)
	float		ElInd;			// Currently highlighted El
	bool		RHIGridSw;		// enable/disable RHI grid display
	rdr_scan	*CurrentPPI,*CurrentRHI;

	virtual void	NewReqAz(rdr_angle);
	virtual void	NewReqEl(rdr_angle);
	virtual void	NewVXSect();
	virtual void 	DrawAzElInd();
	virtual int	DeriveRHI(rdr_img*);
	virtual int	DrawDerivedRHI(rdr_img *Img = 0);
	virtual void	AddRadlNode(s_radl_node*);
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
	s_radl_node	*StartNode,*EndNode;
	int		NodeCount;
	virtual long	OpenWin(char*);
	virtual int	DrawImg(rdr_img *Img = 0);
//	virtual void	SetPointOfView();
	virtual int	DrawPPI(rdr_img*);
	virtual int	DrawRHI(rdr_img*);
	virtual int	DrawVXSect(rdr_img *Img = 0);
	virtual void 	DrawVXSectInd();    // use RHILink's VXSect for data
	virtual void 	DrawIndicators();
	GlPPIRHI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	~GlPPIRHI();
	virtual void	CloseWin();
	GlPPI		*PPILink; 
	int		RHILinkNo;		// WinNum of link
	int 		PPILinkNo;		// WinNum of link
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks(); 
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	};

class GlRHI : public GlWin {
	friend class GlPPI;
	friend class GlPPIRHI;
	friend class rdr_seq;
	
	float		ElInd;			// Currently highlighted El
	rdr_angle	TitleAz;	// requested azim. (set to closest)
	float		MinRng,MaxRng;
	float		MinHeight,MaxHeight;
	float		RHILower;
	GlPPI		*PPILink;		// this RHI's matching PPI 
	GlPPIRHI	*PPIRHILink;		// this RHI's matching PPIRHI window
	int		PPILinkNo;		// WinNum of link
	int 		PPIRHILinkNo;		// WinNum of link
	virtual long	OpenWin(char*);
	virtual void	SetPointOfView();
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
	virtual int	DrawRadl(s_radl*);
	virtual int	DrawInterpRadl(s_radl*,s_radl*);
	virtual int	DrawVXSect(rdr_img *Img = 0);
	virtual void	NewVXSect();
	virtual bool	NewCentreRng(float newrng);
	virtual void	CalcCurPos();				 
	virtual void	UpdateData();
	virtual void 	DrawIndicators();
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
	virtual void	FocusState(bool focusstate = false);
	virtual void	NewReqAzRng(rdr_angle newaz, float newrng);
	virtual int	DeriveRHI(rdr_img*);
	virtual int	DrawDerivedRHI(rdr_img *Img = 0);
	virtual void	AddRadlNode(s_radl_node*);
	s_radl_node	*StartNode,*EndNode;
	int		NodeCount;
	GlRHI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	~GlRHI();
	virtual void	CloseWin();
	virtual void	SetRngOfs(float SRng,float SOfsX,float SOfsY);
	virtual void 	SetElInd(float NewEl);
	virtual void 	DrawElInd();
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	// must SetLinks after all windows loaded
	virtual void	SetLinks(); 
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	};

#include "cappi.h"

class GlCAPPI : public GlPPI {
	friend class rdr_seq;

	CAPPIElevSet	*CAPPIElSet;
	int		CAPPIHt;    // height in metres
	GlCAPPI(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
			DisplWinParams *params=0);
	~GlCAPPI();
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
//	virtual int	DrawRadl(s_radl*);
	virtual int	DrawInterpScan(rdr_scan*);
//	virtual int	DrawInterpRadl(s_radl* Radl1, s_radl* Radl2);
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
public:
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewdBZ(float newdbz);    // set new min dBZ value
	virtual void	NewVal(float newval);    // set new min value
	void		SetMode(CAPPImode newmode); // set nearest/interp mode
	int		WriteCAPPIFile(rdr_scan* RScan = NULL);
	};

class GlVIL : public GlPPI {
	friend class rdr_seq;

	VILUtils    *VILUtil;
	float	    vil2cmap_scalefactor;
	float	    dBZHailLimit;   // max value within this Image
	bool		VILOverflow;    // true if a VIL value exceeded the VILTop 
	bool		useVILProductScans;
	GlVIL(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
			DisplWinParams *params=0);
	~GlVIL();
	void		purgeVILScanProducts();
	rdr_scan*	getVILScan(rdr_scan* RScan);
	virtual int	getRadlAtAz(s_radl *radl, rdr_angle Az,
			rdr_scan *vilscan = NULL);
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawScan(rdr_scan*);
	virtual int	DrawInterpScan(rdr_scan*);
	virtual void	HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
	virtual void	CalcCurPos();				 
	virtual void	GetCurData();
	virtual void	UpdateData();
	virtual void 	SetVILCMap(); // set color map based on the root scan's level table
public:
	virtual void	NewTitle(char *newtitle = 0);
	virtual void	NewVILVal(float NewVILLimit, float NewdBZHailLimit);    // set new min value
	virtual void	SetNewStn(int stn);
	};

typedef	unsigned short	TopRadl[512];

class GlTops : public GlWin {
    friend class rdr_seq;
    
    int		RdrHtMapBase;
//    int		htmapsize;
    int		DrawRes;
    float MinRng,MaxRng;
    TopRadl *TopArray;	// result array	
    bool TopsOverflow;
    // result polar array 1deg x 512km
											// Tops elements in metres abv sea level
    virtual long OpenWin(char*);
    void CalcTopArray(rdr_img*);
    int	DrawAz1,DrawAz2;
    int	CurrentScan_ScanCount;		// to check for changed scan set
	int DrawTopArr();
    int DrawTopsRadl();
    void CalcCurPos();        
    virtual void	GetCurData();
    void UpdateData();
    void HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
//    void FocusState(bool focusstate = false);
    void ReduceRadials(unsigned short*,unsigned short*,int&,int&);
    GlTops(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		    DisplWinParams *params=0);
    ~GlTops();
    virtual void	CloseWin();
    virtual void PutParams(DisplWinParams *params);
    virtual void GetParams(DisplWinParams *params);
    // virtual void NewTitle(char *newtitle = 0);
    virtual void	NewdBZ(float newdbz);    // set new min dBZ value
//    virtual void	SetPointOfView();
public:
    virtual int	DrawImg(rdr_img *Img = 0);
    int DrawTopsImg(rdr_img* Img,bool ForceNewArray = FALSE);
    virtual void	SetNewStn(int stn);
    };

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
/*  
    0-az1el1r1 1-az1el2r1 2-az2el1r1 3-az2el2r1
    4-az1el1r2 5-az1el2r2 6-az2el1r2 7-az2el2r2
*/

    void	CalcCentreVertices(float r1, float r2);
    float       centrev[2][3];
/*    
    0-azelr1   1-azelr2
*/
    };

class GlCZBlocks : public GlWin {
	friend	class	rdr_seq;

    int		RdrHtMapBase;
    float	MinRng,MaxRng;
//    int		htmapsize;
											// Tops elements in metres abv sea level
    CZBlkParams	params;
    virtual long OpenWin(char*);
//    virtual void	SetPointOfView();
    virtual int	DrawImg(rdr_img *Img = 0);
    virtual int	DrawScan(rdr_scan*);
    void DrawBox(float r1 = -1,  float r2 = -1);
    int DrawCZBlkRadl(s_radl *radl);
    void CalcCurPos();        
    virtual void	GetCurData();
    void UpdateData();
    void HndlUsrIFace(short,short,bool&);
	virtual void    HndlKBD(short &val,bool &done);
//    void FocusState(bool focusstate = false);
    GlCZBlocks(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		    DisplWinParams *params=0);
    ~GlCZBlocks();
    virtual void	CloseWin();
    virtual void PutParams(DisplWinParams *params);
    virtual void GetParams(DisplWinParams *params);
    // virtual void NewTitle(char *newtitle = 0);
    virtual void	NewdBZ(float newdbz);    // set new min dBZ value
    };

#endif	/* __GLDISPL_H */
