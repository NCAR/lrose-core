
//////////////////////////////////////////////////////////////
//
// Header file for OGlWinRdrPPIRHI
//
//    This class is a subclass of VkWindow
//
// Normally, very little in this file should need to be changed.
// Create/add/modify menus using RapidApp.
//
// Try to restrict any changes to adding members below the
// "//---- End generated code section" markers
// Doing so will allow you to make changes using RapidApp
// without losing any changes you may have made manually
//
//////////////////////////////////////////////////////////////
#ifndef OGLWINRDRPPIRHI_H
#define OGLWINRDRPPIRHI_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- End generated headers

#ifdef __NOTDEF
#include <X11/IntrinsicP.h>
#include <GL/glx.h>
#include "cdata.h"
#include "histogramDialog.h"
#include "HistogramWindowClass.h"
#include "ManNavWindowClass.h"
#include <Vk/VkFileSelectionDialog.h>
#include "svissr_constants.h"
#endif

#include "XSectWindowClass.h"
#include "ColorEditWindowClass.h"
#include "UserDefinedWinSizeClass.h"
#include "PaletteHighlightWindowClass.h"
#include "AnnotateWindowClass.h"
#include "StormRelSimpleWindow.h"
#include "oglwid.h"
#include "maps.h"

class GlPPIRHI;



//---- OGlWinRdrPPIRHI class declaration

class OGlWinRdrPPIRHI: public VkWindow {

  public:

    OGlWinRdrPPIRHI( const char * name, 
                     ArgList args = NULL,
                     Cardinal argCount = 0 );
    ~OGlWinRdrPPIRHI();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    Widget              createGlWidget(char *glWidName, Widget parentWid,
				       int rgba, int dblBuff, bool allocBG,
				       int alphaSize, int auxBuffs, int stencilSize,
				       int wdth, int hght);
    void		MoveWin(long NewX, long NewY);
    void		ResizeWin(long NewW, long NewH);
    void		GetWinSizePos(long *Wid, long *Ht, long *XOrg, long *YOrg);
    GlPPIRHI		*Owner;	    // pointer to GlPPIRHI instance that owns this
    void		SetOwner(GlPPIRHI *setOwner);
    Widget		glWidget();
    oglwid_props        *oglwidProps;
    oglwid_props        *oglRHIwidProps;
    long		user_width, user_height;  
    int		        lastmpos_x, lastmpos_y;	// save last mouse pos;
    int		        lastbutton1, lastbutton2, lastbutton3;
    int			currentFont;
    void		setCurrentFont(int newfont);
    float		currentFontSize;
    void		setFontSize(float newsize);
    annotateMode	annotMode;

    void		openRightMouseMenu(int mposx, int mposy);
    void		closeRightMouseMenu();

    void                updateCursorData(char *datastring);

    bool		RedrawFlag;

    int                 seq_pos;
    int                 seq_size;
    void                setSeqPos(int pos);
    void                setSeqSize(int sz);

    AnnotateWindowClass *annotateDialog;
    void		openAnnotateDialog(int mposx, int mposy, 
			    int winrootx = 0, int winrooty = 0);
    void		closeAnnotateDialog();
    void		annotateDialogClosed(VkCallbackObject *, void *, void *);
    void		annotateDialogApplied(VkCallbackObject *, void *, void *);

    StormRelSimpleWindow *stormRelDialog;
    void		openStormRelDialog();
    void		closeStormRelDialog();
    void		stormRelDialogClosed(VkCallbackObject *, void *, void *);
    void		stormRelDialogChanged(VkCallbackObject *, void *, void *);

    void                initOlayLayerState(overlayLayers& olaylayers);
    void                setOlayLayerState(overlayLayers& olaylayers);
    void                setOlayLayerStrings(overlayLayers& olaylayers);
    void                setOlayLayerString(int layer, const char *str);
    int                 olayLayerWid2Int(Widget w);
    VkMenuToggle*       olayLayerInt2VkToggle(int layer);
    void                setOlayLayerState(int layer, bool state);
    bool                getOlayLayerState(int layer);

    void                setDataTypeState(e_data_type datatype = e_refl);
    void                setScanTypeState(e_scan_type newstype = PPI);
    void                setMergeState(bool state = true);
    void                setFilterState(bool state = true);
    void                setInterpState(bool state = true);
    void                setView3DState(bool state = true);
    void                setLinkState(bool state = true); // 0=SameStn, 50=
    void                setLinkNeighborState(bool state, int neighborrng = 0); // 0=neighbor stn link off, else 50, 100 or 150
    void                setTitanOlayState(bool state = true); // 0=SameStn, 50=
    void                setWDSSOlayState(bool state = true); // 0=SameStn, 50=
    void                setObsOlayState(bool state = true); 
    void                setLtningOlayState(bool state = true); 
    
#ifdef COMMENTEDOUT
    void		setAutoCreateLocalToggle(bool state);
    void		setAutoCreateWebToggle(bool state);
    void		setCoastToggle(bool state);
	void		setMapTextToggle(bool state);
    void		setLatLongToggle(bool state);
    void		setLatLongLabelToggle(bool state);
	
    PaletteHighlightWindowClass *palHighlightWindow;
    void		PalHighlightWinDeleted(VkCallbackObject *, void *, void *);
    void		PalHighlightChanged(VkCallbackObject *, void *, void *);
    virtual void	setpalHighlightValues(palHighlightDataClass *highlightvals = 0);
    virtual void	getpalHighlightValues(palHighlightDataClass *highlightvals = 0);

    void		newLatLongRes(float newres);
    
    void		editBlendFn();
    void		editRGBFn();
    void		blendApplyImg(VkCallbackObject *, void *, void *);
    void		blendApplySeq(VkCallbackObject *, void *, void *);

    UserDefinedWinSizeClass *userDefWinSizeDialog;
    void		openUserDefWinSizeDialog();
    void		closeUserDefWinSizeDialog();
    void		userDefWinSizeClosed(VkCallbackObject *, void *, void *);
    void		userDefWinSizeDialogApplied(VkCallbackObject *, void *, void *);
    
    AnnotateWindowClass *annotateDialog;
    void		openAnnotateDialog(int mposx, int mposy, 
			    int winrootx = 0, int winrooty = 0);
    void		closeAnnotateDialog();
    void		annotateDialogClosed(VkCallbackObject *, void *, void *);
    void		annotateDialogApplied(VkCallbackObject *, void *, void *);
    
    ColorEditWindowClass *annotateColorEditor;
    void		openAnnotateColorEditor(float *annotcolor);
    void		closeAnnotateColorEditor();
    void		annotateColorChanged(VkCallbackObject *, void *, void *);
    void		annotateColorClosed(VkCallbackObject *, void *, void *);
    void		annotateColorApplied(VkCallbackObject *, void *, void *);

    ColorEditWindowClass *coastlineColorEditor;
    void		openCoastlineColorEditor(float *coastlinecolor);
    void		closeCoastlineColorEditor();
    void		coastlineColorChanged(VkCallbackObject *, void *, void *);
    void		coastlineColorClosed(VkCallbackObject *, void *, void *);
    void		coastlineColorApplied(VkCallbackObject *, void *, void *);

    ColorEditWindowClass *latlongColorEditor;
    void		openLatLongColorEditor(float *latlongcolor);
    void		closeLatLongColorEditor();
    void		latlongColorChanged(VkCallbackObject *, void *, void *);
    void		latlongColorClosed(VkCallbackObject *, void *, void *);
    void		latlongColorApplied(VkCallbackObject *, void *, void *);

    ColorEditWindowClass *mapTextColorEditor;
    void		openMapTextColorEditor(float *maptextcolor);
    void		closeMapTextColorEditor();
    void		mapTextColorChanged(VkCallbackObject *, void *, void *);
    void		mapTextColorClosed(VkCallbackObject *, void *, void *);
    void		mapTextColorApplied(VkCallbackObject *, void *, void *);
    
    ColorEditWindowClass *titleColorEditor;
    void		openTitleColorEditor(float *maptextcolor);
    void		closeTitleColorEditor();
    void		titleColorChanged(VkCallbackObject *, void *, void *);
    void		titleColorClosed(VkCallbackObject *, void *, void *);
    void		titleColorApplied(VkCallbackObject *, void *, void *);
    
    ColorEditWindowClass *usrDefAreaColorEditor;
    void		openUsrDefAreaColorEditor(float *maptextcolor);
    void		closeUsrDefAreaColorEditor();
    void		usrDefAreaColorChanged(VkCallbackObject *, void *, void *);
    void		usrDefAreaColorClosed(VkCallbackObject *, void *, void *);
    void		usrDefAreaColorApplied(VkCallbackObject *, void *, void *);
    
    XSectWindowClass	*xSectWindow;
    void		openxSectWindow();
    void		closexSectWindow();
    void		drawXSect(unsigned char *xSectBuf, int numelems, 
				unsigned char *cmap = 0);
    void		xSectWinDeleted(VkCallbackObject *, void *, void *);
#endif


  protected:



    // Classes created by this class

    class SeqCtlSmallForm *_seqCtlSmallForm1;


    // Widgets created by this class

    Widget  _glPPIRHI_PPIFrame;
    Widget  _glPPIRHI_PPIglwidget;
    Widget  _glPPIRHI_RHIFrame;
    Widget  _glPPIRHI_RHIglwidget;
    Widget  _panedwindow;
    Widget  _ppiRHIMainForm;
    Widget  _ppiRhiDataFrame;
    Widget  _ppirhiRadarDataTextField1;
    Widget  _seqPosScale1;

    // Menu items created by this class
    VkSubMenu  *_displMenuPane3;
    VkSubMenu  *_addRdrWinPane2;
    VkMenuItem *_label79;
    VkMenuItem *_addppiButton3;
    VkMenuItem *_addppivxButton3;
    VkMenuItem *_addppivx3dButton3;
    VkMenuItem *_add3dtopsButton3;
    VkMenuItem *_button129;
    VkMenuItem *_button130;
    VkSubMenu  *_windowGroupRadioPane1;
    VkMenuToggle *_winGroupNoneOption1;
    VkMenuToggle *_winGroupOption9;
    VkMenuToggle *_winGroupOption10;
    VkMenuToggle *_winGroupOption11;
    VkMenuToggle *_winGroupOption12;
    VkMenuToggle *_winGroupOption13;
    VkMenuToggle *_winGroupOption14;
    VkMenuToggle *_winGroupOption15;
    VkMenuToggle *_winGroupOption16;
    VkMenuToggle *_winGroupOption19;
    VkSubMenu  *_windowLayoutMenuPane2;
    VkMenuItem *_saveasButton3;
    VkMenuItem *_printButton3;
    VkMenuItem *_closeButton5;
    VkMenuItem *_button140;
    VkMenuItem *_button141;
    VkMenuItem *_separator52;
    VkSubMenu  *_rdrRngPane2;
    VkMenuItem *_rdrRng6;
    VkMenuItem *_rdrRng7;
    VkMenuItem *_rdrRng8;
    VkMenuItem *_rdrRng9;
    VkMenuItem *_rdrRng10;
    VkSubMenu  *_winSizePane3;
    VkMenuItem *_winSz9;
    VkMenuItem *_winSz10;
    VkMenuItem *_winSz11;
    VkMenuItem *_winSz12;
    VkSubMenu  *_dataTypePane2;
    VkMenuToggle *_dataTypeRefl2;
    VkMenuToggle *_dataTypeVel2;
    VkMenuToggle *_dataTypeVIL2;
    VkMenuToggle *_dataType3DTops2;
    VkMenuToggle *_dataTypeAccum2;
    VkMenuToggle *_dataTypeRawRefl1;
    VkSubMenu  *_displayTypeMenuPane1;
    VkMenuToggle *_cappiToggle1;
    VkMenuToggle *_pPIToggle1;
    VkMenuToggle *_mergeToggle1;
    VkMenuToggle *_filterToggle1;
    VkMenuToggle *_interpToggle1;
    VkMenuToggle *_view3DToggle1;
    VkMenuItem *_setMindBZ1;
    VkMenuItem *_openManStormRelVector1;
    VkMenuItem *_selStation3;
    VkMenuItem *_separator37;
    VkMenuToggle *_pointOfViewToggle;
    VkSubMenu  *_linkGroupRadioPane1;
    VkMenuToggle *_linkGroupStnOption1;
    VkMenuToggle *_linkStn50_toggle1;
    VkMenuToggle *_linkStn100_toggle1;
    VkMenuToggle *_linkStn150_toggle1;
    VkMenuItem *_separator38;
    VkSubMenu  *_seqMenuPane2;
    VkMenuItem *_seqStart2;
    VkMenuItem *_seqStop1;
    VkMenuItem *_seqStepFwd1;
    VkMenuItem *_seqStepBwd2;
    VkMenuItem *_seqStepLast2;
    VkMenuItem *_seqStepFirst2;
    VkMenuItem *_separator43;
    VkMenuItem *_seqSpdDepthButton1;
    VkSubMenu  *_toolsMenuPane1;
    VkMenuToggle *_wdssOverlayToggle1;
    VkMenuItem *_wDSSPropsButton1;
    VkMenuItem *_wdssTableButton1;
    VkMenuItem *_separator36;
    VkMenuToggle *_titanOverlayToggle1;
    VkMenuItem *_titanPropsButton1;
    VkMenuItem *_titanTableButton1;
    VkMenuToggle *_obsOverlayToggle1;
    VkMenuToggle *_ltningOverlayToggle1;
    VkMenuItem *_separator47;
    VkMenuItem *_mapPropsEntry1;
    VkMenuItem *_openManStormRelVector3;
    VkMenuItem *_separator45;
    VkMenuItem *_commsManagerButton1;
    VkMenuItem *_dBManagerButton1;
    VkSubMenu  *_overlayLayersMenuPane1;
    VkMenuToggle *_paletteToggle1;
    VkMenuToggle *_paletteHorizToggle1;
    VkMenuItem *_separator55;
    VkMenuToggle *_toggle3;
    VkMenuItem *_separator5;
    VkMenuToggle *_overlayLayer11;
    VkMenuToggle *_overlayLayer12;
    VkMenuToggle *_overlayLayer13;
    VkMenuToggle *_overlayLayer14;
    VkMenuToggle *_overlayLayer15;
    VkMenuToggle *_overlayLayer16;
    VkMenuToggle *_overlayLayer17;
    VkMenuToggle *_overlayLayer18;
    VkMenuToggle *_overlayLayer19;
    VkMenuToggle *_overlayLayer20;
    VkMenuItem *_separator57;
    VkMenuToggle *_terrainToggle1;
    VkMenuToggle *_terrain3DToggle1;

    // Member functions called from callbacks

    virtual void doMapPropsEntry ( Widget, XtPointer );
    virtual void doOpenRadlVelTools ( Widget, XtPointer );
    virtual void doRdrRng128 ( Widget, XtPointer );
    virtual void doRdrRng256 ( Widget, XtPointer );
    virtual void doRdrRng32 ( Widget, XtPointer );
    virtual void doRdrRng512 ( Widget, XtPointer );
    virtual void doRdrRng64 ( Widget, XtPointer );
    virtual void doSelStation ( Widget, XtPointer );
    virtual void doSeqStart ( Widget, XtPointer );
    virtual void doSeqStepBwd ( Widget, XtPointer );
    virtual void doSeqStepFirst ( Widget, XtPointer );
    virtual void doSeqStepFwd ( Widget, XtPointer );
    virtual void doSeqStepLast ( Widget, XtPointer );
    virtual void doSeqStop ( Widget, XtPointer );
    virtual void doSetMindBZ ( Widget, XtPointer );
    virtual void doTitanProps ( Widget, XtPointer );
    virtual void doTitanTableButton ( Widget, XtPointer );
    virtual void doWDSSProps ( Widget, XtPointer );
    virtual void doWdssTableButton ( Widget, XtPointer );
    virtual void doWinSz1024 ( Widget, XtPointer );
    virtual void doWinSz350 ( Widget, XtPointer );
    virtual void doWinSz512 ( Widget, XtPointer );
    virtual void doWinSz768 ( Widget, XtPointer );
    virtual void glPPIRHI_PPIWinExposed ( Widget, XtPointer );
    virtual void glPPIRHI_PPIWinGInit ( Widget, XtPointer );
    virtual void glPPIRHI_PPIWinInput ( Widget, XtPointer );
    virtual void glPPIRHI_PPIWinResized ( Widget, XtPointer );
    virtual void glPPIRHI_RHIWidExposed ( Widget, XtPointer );
    virtual void glPPIRHI_RHIWidResized ( Widget, XtPointer );
    virtual void glPPIRHI_RHIWidWinGInit ( Widget, XtPointer );
    virtual void glPPIRHI_RHIWidWinInput ( Widget, XtPointer );
    virtual void linkOtherRadars100 ( Widget, XtPointer );
    virtual void linkOtherRadars150 ( Widget, XtPointer );
    virtual void linkOtherRadars50 ( Widget, XtPointer );
    virtual void linkSameStnOnly ( Widget, XtPointer );
    virtual void openCommsManager ( Widget, XtPointer );
    virtual void openDBManager ( Widget, XtPointer );
    virtual void openSeqSpdDepthWindow ( Widget, XtPointer );
    virtual void pointOfViewLinked ( Widget, XtPointer );
    virtual void seqPosValueChanged ( Widget, XtPointer );
    virtual void seqPosValueDragged ( Widget, XtPointer );
    virtual void setDataType3DTops ( Widget, XtPointer );
    virtual void setDataTypeAccum ( Widget, XtPointer );
    virtual void setDataTypeCAPPI ( Widget, XtPointer );
    virtual void setDataTypePPI ( Widget, XtPointer );
    virtual void setDataTypeRefl ( Widget, XtPointer );
    virtual void setDataTypeVIL ( Widget, XtPointer );
    virtual void setDataTypeVel ( Widget, XtPointer );
    virtual void setFilterToggle ( Widget, XtPointer );
    virtual void setInterpToggle ( Widget, XtPointer );
    virtual void setMergeToggle ( Widget, XtPointer );
    virtual void setOverlayLayer ( Widget, XtPointer );
    virtual void setPaletteHorizToggle ( Widget, XtPointer );
    virtual void setPaletteToggle ( Widget, XtPointer );
    virtual void setTerrain3D ( Widget, XtPointer );
    virtual void setTerrainToggle ( Widget, XtPointer );
    virtual void setTitanOverlayToggle ( Widget, XtPointer );
    virtual void setObsOverlayToggle ( Widget, XtPointer );
    virtual void setLtningOverlayToggle ( Widget, XtPointer );
    virtual void setView3DToggle ( Widget, XtPointer );
    virtual void setWdssOverlayToggle ( Widget, XtPointer );
    virtual void setWinGroupNoneOption ( Widget, XtPointer );
    virtual void setWinGroupOption1 ( Widget, XtPointer );
    virtual void setWinGroupOption2 ( Widget, XtPointer );
    virtual void setWinGroupOption3 ( Widget, XtPointer );
    virtual void setWinGroupOption4 ( Widget, XtPointer );
    virtual void setWinGroupOption5 ( Widget, XtPointer );
    virtual void setWinGroupOption6 ( Widget, XtPointer );
    virtual void setWinGroupOption7 ( Widget, XtPointer );
    virtual void setWinGroupOption8 ( Widget, XtPointer );
    virtual void setWinGroupOption9 ( Widget, XtPointer );
    virtual void showCoverageChanged ( Widget, XtPointer );
    virtual void uifaddCAPPI ( Widget, XtPointer );
    virtual void uifaddPPI ( Widget, XtPointer );
    virtual void uifaddPPIVX ( Widget, XtPointer );
    virtual void uifaddPPIVX3D ( Widget, XtPointer );
    virtual void uifaddTops ( Widget, XtPointer );
    virtual void uifaddVIL ( Widget, XtPointer );
    virtual void uifloadcmapDefault ( Widget, XtPointer );
    virtual void uifloadcmapPrinter ( Widget, XtPointer );
    virtual void uifloadlayoutDefault ( Widget, XtPointer );
    virtual void uifloadlayoutNamed ( Widget, XtPointer );
    virtual void uifsavelayoutNamed ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void doMapPropsEntryCallback ( Widget, XtPointer, XtPointer );
    static void doOpenRadlVelToolsCallback ( Widget, XtPointer, XtPointer );
    static void doRdrRng128Callback ( Widget, XtPointer, XtPointer );
    static void doRdrRng256Callback ( Widget, XtPointer, XtPointer );
    static void doRdrRng32Callback ( Widget, XtPointer, XtPointer );
    static void doRdrRng512Callback ( Widget, XtPointer, XtPointer );
    static void doRdrRng64Callback ( Widget, XtPointer, XtPointer );
    static void doSelStationCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStartCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStepBwdCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStepFirstCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStepFwdCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStepLastCallback ( Widget, XtPointer, XtPointer );
    static void doSeqStopCallback ( Widget, XtPointer, XtPointer );
    static void doSetMindBZCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsCallback ( Widget, XtPointer, XtPointer );
    static void doTitanTableButtonCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSPropsCallback ( Widget, XtPointer, XtPointer );
    static void doWdssTableButtonCallback ( Widget, XtPointer, XtPointer );
    static void doWinSz1024Callback ( Widget, XtPointer, XtPointer );
    static void doWinSz350Callback ( Widget, XtPointer, XtPointer );
    static void doWinSz512Callback ( Widget, XtPointer, XtPointer );
    static void doWinSz768Callback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_PPIWinExposedCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_PPIWinGInitCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_PPIWinInputCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_PPIWinResizedCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_RHIWidExposedCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_RHIWidResizedCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_RHIWidWinGInitCallback ( Widget, XtPointer, XtPointer );
    static void glPPIRHI_RHIWidWinInputCallback ( Widget, XtPointer, XtPointer );
    static void linkOtherRadars100Callback ( Widget, XtPointer, XtPointer );
    static void linkOtherRadars150Callback ( Widget, XtPointer, XtPointer );
    static void linkOtherRadars50Callback ( Widget, XtPointer, XtPointer );
    static void linkSameStnOnlyCallback ( Widget, XtPointer, XtPointer );
    static void openCommsManagerCallback ( Widget, XtPointer, XtPointer );
    static void openDBManagerCallback ( Widget, XtPointer, XtPointer );
    static void openSeqSpdDepthWindowCallback ( Widget, XtPointer, XtPointer );
    static void pointOfViewLinkedCallback ( Widget, XtPointer, XtPointer );
    static void seqPosValueChangedCallback ( Widget, XtPointer, XtPointer );
    static void seqPosValueDraggedCallback ( Widget, XtPointer, XtPointer );
    static void setDataType3DTopsCallback ( Widget, XtPointer, XtPointer );
    static void setDataTypeAccumCallback ( Widget, XtPointer, XtPointer );
    static void setDataTypeCAPPICallback ( Widget, XtPointer, XtPointer );
    static void setDataTypePPICallback ( Widget, XtPointer, XtPointer );
    static void setDataTypeReflCallback ( Widget, XtPointer, XtPointer );
    static void setDataTypeVILCallback ( Widget, XtPointer, XtPointer );
    static void setDataTypeVelCallback ( Widget, XtPointer, XtPointer );
    static void setFilterToggleCallback ( Widget, XtPointer, XtPointer );
    static void setInterpToggleCallback ( Widget, XtPointer, XtPointer );
    static void setMergeToggleCallback ( Widget, XtPointer, XtPointer );
    static void setOverlayLayerCallback ( Widget, XtPointer, XtPointer );
    static void setPaletteHorizToggleCallback ( Widget, XtPointer, XtPointer );
    static void setPaletteToggleCallback ( Widget, XtPointer, XtPointer );
    static void setTerrain3DCallback ( Widget, XtPointer, XtPointer );
    static void setTerrainToggleCallback ( Widget, XtPointer, XtPointer );
    static void setTitanOverlayToggleCallback ( Widget, XtPointer, XtPointer );
    static void setObsOverlayToggleCallback ( Widget, XtPointer, XtPointer );
    static void setLtningOverlayToggleCallback ( Widget, XtPointer, XtPointer );
    static void setView3DToggleCallback ( Widget, XtPointer, XtPointer );
    static void setWdssOverlayToggleCallback ( Widget, XtPointer, XtPointer );
    static void setWinGroupNoneOptionCallback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption1Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption2Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption3Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption4Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption5Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption6Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption7Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption8Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupOption9Callback ( Widget, XtPointer, XtPointer );
    static void showCoverageChangedCallback ( Widget, XtPointer, XtPointer );
    static void uifaddCAPPICallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPICallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPIVXCallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPIVX3DCallback ( Widget, XtPointer, XtPointer );
    static void uifaddTopsCallback ( Widget, XtPointer, XtPointer );
    static void uifaddVILCallback ( Widget, XtPointer, XtPointer );
    static void uifloadcmapDefaultCallback ( Widget, XtPointer, XtPointer );
    static void uifloadcmapPrinterCallback ( Widget, XtPointer, XtPointer );
    static void uifloadlayoutDefaultCallback ( Widget, XtPointer, XtPointer );
    static void uifloadlayoutNamedCallback ( Widget, XtPointer, XtPointer );
    static void uifsavelayoutNamedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultOGlWinRdrPPIRHIResources[];


    //--- End generated code section

    static void PPIPointerMotionHandler(Widget, XtPointer, 
		XMotionEvent*, Boolean*);
    static void RHIPointerMotionHandler(Widget, XtPointer, 
		XMotionEvent*, Boolean*);
    static void EnterPPIWindowHandler(Widget, XtPointer, 
		XEnterWindowEvent*, Boolean*);
    static void EnterRHIWindowHandler(Widget, XtPointer, 
		XEnterWindowEvent*, Boolean*);
    static void KeyPressHandler(Widget, XtPointer, 
		XKeyEvent*, Boolean*);


};
//---- End of generated code

#endif
