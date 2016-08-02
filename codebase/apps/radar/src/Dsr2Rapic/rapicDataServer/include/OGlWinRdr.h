
//////////////////////////////////////////////////////////////
//
// Header file for OGlWinRdr
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
#ifndef OGLWINRDR_H
#define OGLWINRDR_H
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

class OGlDisplRdr;


//---- OGlWinRdr class declaration

class OGlWinRdr: public VkWindow {

  public:

    OGlWinRdr( const char * name, 
               ArgList args = NULL,
               Cardinal argCount = 0 );
    ~OGlWinRdr();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    void		MoveWin(long NewX, long NewY);
    void		ResizeWin(long NewW, long NewH);
    void		GetWinSizePos(long *Wid, long *Ht, long *XOrg, long *YOrg);
    OGlDisplRdr		*Owner;	    // pointer to OGlDispl instance that owns this
    void		SetOwner(OGlDisplRdr *setOwner);
    Widget              createGlWidget(char *glWidName, Widget parentWid,
				       int rgba, int dblBuff, bool allocBG,
				       int alphaSize, int auxBuffs, int stencilSize,
				       int wdth, int hght);
    Widget		glWidget();
    oglwid_props        *oglwidProps;
    long		user_width, user_height;  
    int		        lastmpos_x, lastmpos_y;	// save last mouse pos;
    int		        lastbutton1, lastbutton2, lastbutton3;
    int			currentFont;
    void		setCurrentFont(int newfont);
    float		currentFontSize;
    void		setFontSize(float newsize);
    annotateMode	annotMode;

    void		setAutoCreateLocalToggle(bool state);
    void		setAutoCreateWebToggle(bool state);
    void		setCoastToggle(bool state);
    void		setMapTextToggle(bool state);
    void		setLatLongToggle(bool state);
    void		setLatLongLabelToggle(bool state);
	
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
    void                setTitanOlayState(bool state = true); 
    void                setWDSSOlayState(bool state = true); 
    void                setObsOlayState(bool state = true); 
    void                setLtningOlayState(bool state = true); 
    
#ifdef COMMENTEDOUT
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

    class SeqCtlSmallForm *_seqCtlSmallForm2;


    // Widgets created by this class

    Widget  _glPPI_glwidget;
    Widget  _radarDataTextField;
    Widget  _seqPosScale;
    Widget  _winRdrForm;
    Widget  _winRdrPPIDataFrame;
    Widget  _winRdrPPIFrame;

    // Menu items created by this class
    VkSubMenu  *_displMenuPane1;
    VkSubMenu  *_addRdrWinPane1;
    VkMenuItem *_label78;
    VkMenuItem *_addppiButton2;
    VkMenuItem *_addppivxButton2;
    VkMenuItem *_addppivx3dButton2;
    VkMenuItem *_add3dtopsButton2;
    VkMenuItem *_button110;
    VkMenuItem *_button116;
    VkSubMenu  *_windowGroupRadioPane;
    VkMenuToggle *_winGroupNoneOption;
    VkMenuToggle *_winGroupOption1;
    VkMenuToggle *_winGroupOption2;
    VkMenuToggle *_winGroupOption3;
    VkMenuToggle *_winGroupOption4;
    VkMenuToggle *_winGroupOption5;
    VkMenuToggle *_winGroupOption6;
    VkMenuToggle *_winGroupOption7;
    VkMenuToggle *_winGroupOption8;
    VkMenuToggle *_winGroupOption17;
    VkSubMenu  *_windowLayoutMenuPane1;
    VkMenuItem *_saveasButton2;
    VkMenuItem *_printButton2;
    VkMenuItem *_closeButton4;
    VkMenuItem *_button127;
    VkMenuItem *_button128;
    VkMenuItem *_separator48;
    VkSubMenu  *_rdrRngPane1;
    VkMenuItem *_rdrRng1;
    VkMenuItem *_rdrRng2;
    VkMenuItem *_rdrRng3;
    VkMenuItem *_rdrRng4;
    VkMenuItem *_rdrRng5;
    VkSubMenu  *_winSizePane1;
    VkMenuItem *_winSz1;
    VkMenuItem *_winSz2;
    VkMenuItem *_winSz3;
    VkMenuItem *_winSz4;
    VkSubMenu  *_dataTypePane1;
    VkMenuToggle *_dataTypeRefl1;
    VkMenuToggle *_dataTypeVel1;
    VkMenuToggle *_dataTypeVIL1;
    VkMenuToggle *_dataType3DTops1;
    VkMenuToggle *_dataTypeAccum1;
    VkMenuToggle *_dataTypeRawRefl;
    VkSubMenu  *_displayTypeMenuPane;
    VkMenuToggle *_cappiToggle;
    VkMenuToggle *_pPIToggle;
    VkMenuToggle *_mergeToggle;
    VkMenuToggle *_filterToggle;
    VkMenuToggle *_interpToggle2;
    VkMenuToggle *_view3DToggle;
    VkMenuItem *_setMindBZ;
    VkMenuItem *_openManStormRelVector;
    VkMenuItem *_selStation1;
    VkMenuItem *_separator21;
    VkMenuToggle *_pointOfViewToggle1;
    VkSubMenu  *_linkGroupRadioPane;
    VkMenuToggle *_linkSameStn_toggle;
    VkMenuToggle *_linkStn50_toggle;
    VkMenuToggle *_linkStn100_toggle;
    VkMenuToggle *_linkStn150_toggle;
    VkMenuItem *_separator33;
    VkMenuItem *_separator39;
    VkSubMenu  *_seqMenuPane1;
    VkMenuItem *_seqStart1;
    VkMenuItem *_seqStop2;
    VkMenuItem *_seqStepFwd2;
    VkMenuItem *_seqStepBwd1;
    VkMenuItem *_seqStepLast1;
    VkMenuItem *_seqStepFirst1;
    VkMenuItem *_separator42;
    VkMenuItem *_seqSpdDepthButton;
    VkSubMenu  *_toolsMenuPane;
    VkMenuToggle *_wdssOverlayToggle;
    VkMenuItem *_wDSSPropsButton;
    VkMenuItem *_wdssTableButton;
    VkMenuItem *_separator53;
    VkMenuToggle *_titanOverlayToggle;
    VkMenuItem *_titanPropsButton;
    VkMenuItem *_titanTableButton;
    VkMenuToggle *_obsOverlayToggle;
    VkMenuToggle *_ltningOverlayToggle;
    VkMenuItem *_separator54;
    VkMenuItem *_mapPropsEntry;
    VkSubMenu  *_overlayLayersMenuPane2;
    VkMenuToggle *_paletteToggle;
    VkMenuToggle *_paletteHorizToggle;
    VkMenuItem *_separator58;
    VkMenuToggle *_toggle2;
    VkMenuItem *_separator56;
    VkMenuToggle *_overlayLayer1;
    VkMenuToggle *_overlayLayer2;
    VkMenuToggle *_overlayLayer3;
    VkMenuToggle *_overlayLayer4;
    VkMenuToggle *_overlayLayer5;
    VkMenuToggle *_overlayLayer6;
    VkMenuToggle *_overlayLayer7;
    VkMenuToggle *_overlayLayer8;
    VkMenuToggle *_overlayLayer9;
    VkMenuToggle *_overlayLayer10;
    VkMenuItem *_separator51;
    VkMenuToggle *_terrainToggle;
    VkMenuToggle *_terrain3DToggle;
    VkMenuItem *_openManStormRelVector2;
    VkMenuItem *_separator46;
    VkMenuItem *_separator55;
    VkMenuItem *_commsManagerButton;
    VkMenuItem *_dBManagerButton;

    // Member functions called from callbacks

    virtual void OGLWinExposed ( Widget, XtPointer );
    virtual void OGLWinGInit ( Widget, XtPointer );
    virtual void OGLWinInput ( Widget, XtPointer );
    virtual void OGLWinResized ( Widget, XtPointer );
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

    static void OGLWinExposedCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinGInitCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinInputCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinResizedCallback ( Widget, XtPointer, XtPointer );
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

    static String  _defaultOGlWinRdrResources[];


    //--- End generated code section

    static void PointerMotionHandler(Widget, XtPointer, 
		XMotionEvent*, Boolean*);
    static void EnterWindowHandler(Widget, XtPointer, 
		XEnterWindowEvent*, Boolean*);
    static void KeyPressHandler(Widget, XtPointer,
  		XEvent*, Boolean*);



};
//---- End of generated code

#endif
