#ifndef	__OGLDISPL_H
#define __OGLDISPL_H


#include "displ.h"
#include "cdata.h"
#include "drawtext.h"
#include "oglwid.h"
#include <Vk/VkWindow.h>
#include "PaletteHighlightFormClass.h"
#include "oglpalette.h"
#include "GLwMDrawA.h"
#include "drawOGlLLTexture.h"

enum textRenderModes {trm_OGLX, trm_GLC, trm_Inv};

extern bool glWinOpened;
extern bool glCMapsInitiated;

#define OGLKEYBUFFLEN 65

class OGlDispl : public DisplWin {
protected:
  VkWindow	*Window;	    // pointer to actual window instance
  oglwid_props  *oglwidProps;     // pointer to the main opengl widget properties

  //	GLXPbuffer	*pbuffer;   // pointer to pix buffer for memory context render
  int           auxBuffers;        // number of aux buffers available

  short		CurPosX,CurPosY;    // rel. cursor pos in window
  bool		autoCreateLocalJPEG;
  bool		autoCreateWebJPEG;
    
 public:

  GLuint        circlist;
  int           circlistangres;
  KeySym        lastKeySym;
  char          lastKeyBuffer[OGLKEYBUFFLEN];
  textRenderModes	textRenderMode;
  int		TexMinFunction;
  int		TexMagFunction;
  bool		moveCamera;		// if true use camera model for interaction
  GLuint	mapDListIndex,	    // map display list index
    annotateDListIndex, // annotation display list
    texDListIndex;	    // texture display list index
  bool dListIndexInitialised,  // true if display list indexes initialised
    mapDListValid,     // true if map list is valid, else needs to generated
    annotateDListValid,// true if map list is valid, else needs to generated
    texDListValid;     // true if map list is valid, else needs to generated
  bool drawMap,
    drawMapText, 
    drawAnnotations,  
    drawLatLongGrid;
  DrawDataList	annotationList;         // annots in lat/long coords
  DrawDataList	otherAnnotationList;	// UsrDefinedArea etc.
  vector<DrawDataList*>	projAnnotationList; // projected annot coords
  vector<DrawDataList*>	projOtherAnnotationList; // UsrDefinedArea etc.
  DrawDataVectorArea *UsrDefinedArea;
  DrawDataVector *UsrDefinedSection;
  virtual void	DrawAnnotations(int stn = -1);
  virtual void	DrawAnnotations(DrawDataList *annotationlist,
				int stn = -1, 
				renderProperties *renderProps = NULL);

  drawOGlLLTexture *oglTexRenderer; // ogl texture tile renderer
  
#ifdef USE_INVENTOR

#include <Inventor/Sb.h>

  //	bool		useInventor;
  void		initInventor();
  void		closeInventor();
  SbViewportRegion	*inv_Viewport;
  SoGLRenderAction	*inv_RenderAction;
  SoSeparator	*inv_Root;
	
  SoSeparator	*inv_AnnotRoot;
  void		inv_initAnnotations(SoGroup *root);
  void		inv_addAnnotation(SoGroup *baseGroup, DrawData *thisDrawData);
  SoFont		*inv_AnnotDefaultFont;
  SoBaseColor	*inv_AnnotDefaultColor;
  SoSeparator	*inv_AnnotTextRoot;

  void		inv_initTitle(SoGroup *root);
  SoSeparator	*inv_TitleRoot;
  SoFont		*inv_TitleFont;
  SoBaseColor	*inv_TitleColor;
  SoTranslation	*inv_TitleXlat;
  SoText2		*inv_TitleText;
  //	SoText3		*inv_TitleText;
  SbBox3f		invTitleBBox;
	
  SoWriteAction	inv_DebugWrite;
#endif
  bool		drawlatLongLabels;


  bool          rgbaMode;
  unsigned char	*Cmap;
  cmapbasis	*Cmapbasis;	
  int		Cmapbasispoints;
  float		latLongGridRes;
  float		mapColor[3];	
  float		mapTextColor[3];	
  float		latlongGridColor[3];	
  float		ImgTitleColor[3];	
  float		usrDefAreaColor[3];	
  int		mapLineWidth;
  void          initColors();
  void          setPalColor(int index); 
  bool          usePalColorIndexAlpha;
  void          setPalColorIndexAlpha(int index); 
  uchar*        getPalColor(int index); 
  void          setClearColor(int index);
  
  DrawOGLXBitmapText	*text;
#ifdef USE_GLC
  DrawGLCText	*glctext; // use global glctext server instead
#endif
#ifdef USE_GLF
  DrawGLFText   *textRenderer;
  int           textFontID;  // default font id for textRenderer
#endif
  float		textsize;
  float		annotColor[3];
  void		setAnnotateFont(char *fontName);
  void		setAnnotateFontSize(float newsize);
  bool		scaleTextToWinSize;


  palHighlightDataClass palHighlight;

  void		addAnnotation(DrawData *newannot);
  void		pickAnnotation(int mposx, int mposy);
  void		clearNearestAnnotation(int mposx, int mposy);
  virtual void	openAnnotateDialog(int mposx, int mposy,
				   int mrootx, int mrooty) {};
  void		clearAllAnnotations();
  void          annotationsChanged();

  OGlDispl(DisplWin* PrevWin=0,rdr_seq *CtlSeq=0,char* WinName = 0,
	   DisplWinParams *params=0); 	// new window always appended
  virtual ~OGlDispl();
  virtual void	NewTitle(char *newtitle = 0);
  virtual bool	OpenWin(char* WinName);
  virtual void	CloseWin();
  virtual void	MoveWin(long NewX,long NewY, long w = 0, long h = 0);
  virtual void	ResizeWin(long w, long h);
  virtual void	GetWinSizePos();
  virtual void  winCentre(long &x, long &y);// get window centre coords
  virtual bool	makeGlWidgetCurrent(oglwid_props  *_oglwidprops = NULL);
  virtual void	ShowNewImg(int seqpos = -1);
  virtual void	DrawAxis(float length = 0);
  virtual void	SetPointOfView(bool povchanged = false) {};
  virtual void	ZoomChanged(int pixels_dx, int pixels_dy) {};
  virtual void	PanChanged(int pixels_dx, int pixels_dy) {};
  virtual void	RotChanged(int pixels_dx, int pixels_dy) {};
  virtual void	SetMergeMode(bool mergemode);
  virtual void	CursorPosChanged(int new_x, int new_y);  
  virtual void	FocusState(bool focusstate);
  virtual void	Show();
  virtual void	Hide();
  virtual void	Raise();
  virtual void	Lower();
  virtual void	Iconify();
  virtual bool	Iconic();
  virtual bool  Visible();

  virtual void  HndlXEvent(XEvent *xEvent);
  virtual void  HndlKBD(XKeyEvent *keyEvent);
  virtual void  HndlMouse(XEvent *xEvent);
  virtual void	UpdateData();
  virtual void	OGlWinClosed();		// called by OglWin when it is closed by user
  virtual void	PutParams(DisplWinParams *params);
  virtual void	GetParams(DisplWinParams *params);
  virtual void	SetWindowMenuState();
  virtual void	SetAllGUIState() {};       // called after GUI realized
  virtual void	winContextCreated();	// called by OGlWin when context has been created
  //virtual void	openPBuffer();
  //virtual void	closePBuffer();
  virtual int   writeSeqJpeg(rdr_img *jpegimg); // write this img as jpeg
  virtual int	writeJpeg(char *filename, 
			  CData *pCData = 0, 
			  int quality = 100);
  virtual int	writeJpeg(char *filename, 
			  rdr_img *Img, 
			  int quality = 100);
  virtual int	glWriteJpeg(char *filename, 
			  CData *pCData = 0,
			  oglwid_props  *_oglwidprops = NULL, 
			  int quality = 90);
  virtual int	glWriteJpeg(char *filename, 
			  rdr_img *Img,
			  oglwid_props  *_oglwidprops = NULL, 
			  int quality = 90);
  virtual int	glWriteJpeg(char *filename,        // single implementation for both rdrimg and pCData
			  rdr_img *Img, 
			  CData *pCData,
			  oglwid_props  *_oglwidprops = NULL, 
			  int quality = 90);
  virtual int	glWritePNG(char *filename,        // single implementation for both rdrimg and pCData
			 rdr_img *Img, 
			 CData *pCData,
			 oglwid_props  *_oglwidprops = NULL, 
			 int quality = 90);
/*   virtual int	checkAutoWriteJpeg(CData *pCData,  */
/* 				   int quality); */
  virtual int	writeGif(char *filename, 
			 rdr_img *Img = 0);
  virtual void  CIRCF(float x,float y,float z,float rad);
  virtual void  drawCirc(float ofsx, float ofsy, float ofsz, 
			    float radius, int angres = 5);
  virtual void  drawCircF(float ofsx, float ofsy, float ofsz, 
			    float radius, int angres = 5);
  virtual void  drawEllipse(float ofsx, float ofsy, float ofsz, 
			    float radius_minor, float radius_major,
			    float rot_degs = 0, int angres = 5);
  virtual void  drawEllipseF(float ofsx, float ofsy, float ofsz, 
			    float radius_minor, float radius_major,
			    float rot_degs = 0, int angres = 5);
#ifdef USE_TITAN_CLIENT
  virtual bool  doTitanRender(renderProperties *renderProps = NULL, 
			      time_t rendertime = 0,
			      int stn = -1, bool area_flag = false);
#endif
#ifdef USE_WDSS_CLIENT
  virtual void  doWDSSRender(renderProperties *renderProps = NULL, 
			      time_t rendertime = 0,
			      int stn = -1);
#endif
  
  OGlPalette palette;
  virtual void drawPalette(float indval = 0, float indval2 = 0);
  virtual void drawTerrainHtPalette(float indval = 0, float indval2 = 0);
  
};
	
#endif	/* __OGLDISPL_H */
