#ifndef	__OGLDISPLSAT_H
#define __OGLDISPLSAT_H

//Phil Purdams workaround for name clash between Inventor and
//something in the gl.h area.  Used in Rapic 372. SD 3/2/99
//Was in /usr/include/Inventor/errors/SoDebugError.h but friendlier to have it here.
#undef WARNING

#ifdef USE_INVENTOR
#include <Inventor/SoDB.h>
#include <Inventor/actions/SoGLRenderAction.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoGetBoundingBoxAction.h>
#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoText2.h>
#include <Inventor/nodes/SoText3.h>
#include <Inventor/nodes/SoBaseColor.h>
#include <Inventor/nodes/SoFont.h>
#endif

#include "OGlWin.h"
#include "ogldispl.h"
#include "cdata.h"
#include "cdatasat.h"
#include "cdatasat_svissr.h"
#include "rdrutils.h"
#include "satdatamng.h"
// #include "csatnav_grid.h"
#include "textureProj.h"
#include "drawtext.h"

typedef unsigned char rgbPixel[3];

class ogldisplsatDisplWinParams {				// Display Window save parameters
friend class DisplWin;
public:
	int 		WinNum;				// Win pos in seq list (1=first)
	WinType		Type;
//	char		WinName[128];
	long		winOrgX,winOrgY,winW,winH;  // this window's pos & size
	bool		Frozen;			// current iconify state
	float		winWidth, winHeight,    // window dimensions etc in projection units,   
	  winCentreX, winCentreY,               // e.g. degrees, kms
	  winStartCentreX, winStartCentreY,
	  winScale,	                        // winScale is zoom ratio
	  winSizeScale;	                        // winSizeScale is scale against 512 pixel window
						    // may be used to scale text fonts
	rpProjection	Projection;	    // current projection
	rpProjUnits	ProjUnits;	    // units for current projection
	int		AutoResample;
	int		AutoSeqResample;
	int		TexMinFunction;
	int		TexMagFunction;
	SatSubsetParams	subsetparams;
	CBlend		BlendFn;
	bool		drawMap,
			drawMapText, 
			drawAnnotations,  
			drawLatLongGrid;
	bool		drawlatLongLabels;
	float		latLongGridRes;
	float		mapColor[3];	
	float		mapTextColor[3];	
	float		latlongGridColor[3];	
	float		ImgTitleColor[3];	
	float		usrDefAreaColor[3];	
	int		mapLineWidth;
	float		textsize;
	float		annotColor[3];
	bool		scaleTextToWinSize;
	bool		applyGreyScale;
	bool		applyHistEq;
void Clear() { memset(this, 0, sizeof(*this)); };
	ogldisplsatDisplWinParams() { Clear(); };
	};

class OGlDisplSat : public OGlDispl {
public:
	OGlDisplSat(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName=0,
		DisplWinParams *params=0);
	virtual ~OGlDisplSat();
	
	OGlWin          *satWindow;         // actual OGLWin window instance

	unsigned char	*rawbuffer;
	bool		UseTextureCmap;
	bool		showVertices;
	int		AutoResample;
	bool		autoResampleUseManager; // use manager to resample?
	int		AutoSeqResample;
	char		testsatfname[256];  // test file name
	char		webJpegPathname[256];
	char		localJpegPathname[256];
	char		autoJpegFilename[64];
	char		manJpegFilename[256];
	virtual void    thisJpegName(char *namestr, char *pathstr = 0, 
				CDataSatSubset *subset = NULL);
	void		setAutoWebJpegMode(bool state);
	void		setAutoLocalJpegMode(bool state);

	CDataSatSvissr	*satDataSvissr;
	CDataSatSubset	*satSubset;	    // pointer to last drawn subset
	CDataSatSubset	*tempSatSubset;	// actual instance of subset for autoresample
	SatSubsetParams	*subsetparams;
	SatSubsetParams	*autoResampleSubsetparams;
	SatSubsetParams	*seqResampleSubsetparams;
	int		subsetIdent;
	projGrid	*lastRenderProjGrid,	// pointer to last drawn projection grid
			*currentProjGrid,	// pointer to selected projection grid
			*refSatProjGrid;	// ref sat projection grid for nav sat projection
	CSatNavSvissrSm	*refSvissrNavigation;	// ref sat nav for nav sat projection
	bool		disableHistUpdate;
	
	unsigned char	*satCmap, *histeqCmap;
	cmapbasis	*satcmapbasis;	
	int		satcmapbasispoints;
	bool		applyGreyScale;
	int		*hist_eq_table;	// histogram equalisation translation table
	bool		applyHistEq;
	void		enableHistEq(bool Switch = true);
	void		setHistEq(CDataSat *histEqSat = 0);

	virtual void	drawHistogram(CDataSatSubset *satsubset = 0);

	virtual bool	OpenWin(char* WinName);
	virtual void	CheckWin(rdr_img *Img = 0);		// perform housekeeping functions
	virtual void	CheckRedraw(rdr_img *Img = 0);	// check if redraw required
	virtual void	CheckImageRender(CData *pCData); // check whether this image should be rendered
	virtual int	DrawImg(rdr_img *Img = 0);
	virtual int	DrawImg(CData *pCData);		// if pSvissr is zero, draw using satSubset
	virtual int	SimpleDrawImg(CData *pCData);	
	virtual int	DrawImgTexCMap(CDataSatSubset  *drawSubset = 0);
	virtual int	DrawImgTexRGB(CDataSatSubset  *drawSubset = 0);
	virtual void	noDataWindow(char *reason = 0);
	virtual void	TexMapProjGrid(CDataSatSubset  *drawSubset = 0);   // apply texture using current ProjGrid
	virtual void	ResetProjection(rpProjection proj, bool resetPOV = false); // reset to new projection
	virtual void	ResetProjection();		    // reset existing projection
	virtual void	SetProjGrid(rpProjection proj);
	virtual void	polarView(GLdouble distance, GLdouble twist, 
				GLdouble elevation, GLdouble azimuth);
	virtual void	SetPointOfView(bool winSizeChanged = false);
	virtual void	CursorPosChanged(int new_x, int new_y);  
	virtual void	ZoomChanged(int pixels_dx, int pixels_dy);  
	virtual void	PanChanged(int pixels_dx, int pixels_dy);
	virtual void	RotChanged(int pixels_dx, int pixels_dy);
	virtual void	FinishedPointOfViewChange();
	virtual int	ReSample(bool DrawNew = true, 
				bool ForceResample = false);// fetch new rawbuffer to suit window
	virtual int	ReSampleSeq(gmschannel newchannel = gms_ch_undef);				    // fetch new rawbuffer to suit window
	virtual int	ReSampleThis(bool ResampleInSeq = false);				    // fetch new rawbuffer to suit window
	virtual void	ReNavSeq();
	virtual int	DrawOverlay();
	virtual int	DrawASCIILatLongMap(char *mapname, bool SwapLatLongOrder = FALSE);
	virtual int	DrawLatLongGrid(float LatLongRes);
	virtual void	DrawAnnotations();
	virtual void	DrawAnnotations(DrawDataList *annotationlist,
					renderProperties *renderProps = NULL);
	virtual void	DrawVectorData(DrawDataVector *vectordata);
	virtual void	DrawSectionData(unsigned char *sectionBuff, int numElems);
	virtual void	SectionWinClosed();
	virtual gmschannel	getChannel();
	virtual void	setChannelImg(gmschannel newchannel);
	virtual void	setChannelSeq(gmschannel newchannel);
	
	char		title[256];
	virtual void	NewSubsetTitle(CDataSatSubset *satsubset, char *newtitle = 0);
	virtual void	DrawTitle(char *newtitle = 0);
	virtual void	SetWindowMenuState();
	
	rgbPixel	*rgbbuffer;
	virtual void	MoveLatLongCursor(LatLongHt *CursorLatLong);
	virtual void	SetManNavOffsets(int elem_ofs, int line_ofs, 
			unsigned char applyThis	= 1, unsigned char applyAll = 0);
	virtual void	ClearManNavOffsets();
	virtual void	openGMSFile(char *filename);
	virtual void	PutParams(DisplWinParams *params);
	virtual void	GetParams(DisplWinParams *params);
	
	};
	
#endif	/* __OGLDISPLSAT_H */
