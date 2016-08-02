
//////////////////////////////////////////////////////////////
//
// Header file for OGlWin
//
//    This class is a subclass of VkWindow
//
// Normally, very little in this file should need to be changed.
// Create/add/modify menus using RapidApp.
//
// Restrict changes to those sections between
// the "//--- Start/End editable code block" markers
// Doing so will allow you to make changes using RapidApp
// without losing any changes you may have made manually
//
//////////////////////////////////////////////////////////////
#ifndef OGLWIN_H
#define OGLWIN_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- Start editable code block: headers and declarations

#include <X11/IntrinsicP.h>
#include <GL/glx.h>
#include "cdata.h"
#include "histogramDialog.h"
#include "HistogramWindowClass.h"
#include "XSectWindowClass.h"
#include "PaletteHighlightWindowClass.h"
#include "ManNavWindowClass.h"
#include <Vk/VkFileSelectionDialog.h>
#include "svissr_constants.h"
// #include "csatnav_grid.h"
#include "textureProj.h"
#include "AnnotateWindowClass.h"
#include "ColorEditWindowClass.h"
#include "UserDefinedWinSizeClass.h"
#include "oglwid.h"

class OGlDispl;

//---- End editable code block: headers and declarations


//---- OGlWin class declaration

class OGlWin: public VkWindow {

  public:

    OGlWin( const char * name, 
            ArgList args = NULL,
            Cardinal argCount = 0 );
    ~OGlWin();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: OGlWin public

    void		MoveWin(long NewX, long NewY);
    void		ResizeWin(long NewW, long NewH);
    void		GetWinSizePos(long *Wid, long *Ht, long *XOrg, long *YOrg);
    OGlDispl		*Owner;	    // pointer to OGlDispl instance that owns this
    void		SetOwner(OGlDispl *setOwner);
    Widget              createGlWidget(char *glWidName, Widget parentWid,
				       int rgba, int dblBuff, bool allocBG,
				       int alphaSize, int auxBuffs, int stencilSize,
				       int wdth, int hght);
    Widget		glWidget();
    oglwid_props        *oglwidProps;
    long		user_width, user_height;  
    int		        lastmpos_x, lastmpos_y;	// save last mouse pos;
    int		        lastbutton1, lastbutton2, lastbutton3;
    gmschannel		lastchannel;
    int			currentFont;
    void		setCurrentFont(int newfont);
    float		currentFontSize;
    void		setFontSize(float newsize);
    annotateMode	annotMode;

    void		setChannelToggle(gmschannel setchannel);
    gmschannel	        currentChToggle;
	
    void		setProjToggle(rpProjection setproj);
    rpProjection	currentProjToggle;
	
    void		setAutoResampleCurrToggle(bool state);
    void		setAutoCreateLocalToggle(bool state);
    void		setAutoCreateWebToggle(bool state);
    void		setHistEqToggle(bool state);
    void		setGreyScaleToggle(bool state);
    void		setCoastToggle(bool state);
	void		setMapTextToggle(bool state);
    void		setLatLongToggle(bool state);
    void		setLatLongLabelToggle(bool state);
	
    HistogramWindowClass *histWindow;
    void		drawHistogram(histogramClass *histogram = 0, 
				    unsigned char *cmap = 0, 
				    int *histeqtable = 0);
    void		HistogramWinDeleted(VkCallbackObject *, void *, void *);

    PaletteHighlightWindowClass *palHighlightWindow;
    void		PalHighlightWinDeleted(VkCallbackObject *, void *, void *);
    void		PalHighlightChanged(VkCallbackObject *, void *, void *);
    virtual void	setpalHighlightValues(palHighlightDataClass *highlightvals = 0);
    virtual void	getpalHighlightValues(palHighlightDataClass *highlightvals = 0);

    ManNavWindowClass	*manNavWindow;
    void		ManNavWindowDeleted(VkCallbackObject *, void *, void *);
    void		ManNavWindowChanged(VkCallbackObject *, void *, void *);

    void		newLatLongRes(float newres);
    
    void		editBlendFn();
    void		editRGBFn();
    void		blendApplyImg(VkCallbackObject *, void *, void *);
    void		blendApplySeq(VkCallbackObject *, void *, void *);

    void		openRightMouseMenu(int mposx, int mposy);
    void		closeRightMouseMenu();

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

    bool		RedrawFlag;

//---- End editable code block: OGlWin public


  protected:




    // Widgets created by this class

    Widget  _form27;
    Widget  _frame;
    Widget  _glwidget;

    // Menu items created by this class
    VkSubMenu  *_radioPane1;
    VkMenuToggle *_projNavSatToggle;
    VkMenuToggle *_projSatUnNavToggle;
    VkMenuToggle *_projMercToggle;
    VkMenuToggle *_projEquiCylToggle;
    VkMenuToggle *_projStereoToggle;
    VkMenuToggle *_proj3DOrthoToggle;
    VkMenuItem *_separator32;
    VkMenuItem *_button95;
    VkMenuItem *_button65;
    VkMenuItem *_separator27;
    VkSubMenu  *_menuPane4;
    VkMenuItem *_entry8;
    VkMenuItem *_entry9;
    VkMenuItem *_button61;
    VkMenuItem *_button68;
    VkSubMenu  *_radioPaneChannel;
    VkMenuToggle *_toggleIR1;
    VkMenuToggle *_toggleIR2;
    VkMenuToggle *_toggleWV;
    VkMenuToggle *_toggleVis;
    VkMenuToggle *_toggleChComb;
    VkMenuToggle *_toggle3ChRGB;
    VkMenuItem *_separator26;
    VkMenuItem *_button93;
    VkSubMenu  *_optionsPane;
    VkMenuToggle *_toggleAutoResampleCurrent;
    VkMenuItem *_button67;
    VkMenuItem *_button54;
    VkMenuItem *_button106;
    VkMenuItem *_button107;
    VkMenuItem *_separator20;
    VkSubMenu  *_radioPane3;
    VkMenuToggle *_option4;
    VkMenuToggle *_option5;
    VkMenuItem *_separator19;
    VkMenuToggle *_toggleAutoCreateLocal;
    VkMenuToggle *_toggleAutoCreateWeb;
    VkMenuItem *_button56;
    VkMenuItem *_button58;
    VkMenuItem *_editJpegNameButton;
    VkMenuItem *_separator18;
    VkMenuItem *_button69;
    VkMenuItem *_button62;
    VkMenuToggle *_toggleHistEq;
    VkMenuItem *_button63;
    VkMenuToggle *_toggleGreyScale;
    VkMenuItem *_editPalButton;
    VkMenuItem *_separator22;
    VkMenuItem *_button88;
    VkMenuItem *_button57;
    VkSubMenu  *_viewPane;
    VkSubMenu  *_radioPane5;
    VkMenuToggle *_option13;
    VkMenuToggle *_option14;
    VkMenuToggle *_option15;
    VkMenuToggle *_toggle13;
    VkMenuToggle *_toggleCoastLines;
    VkMenuToggle *_toggleMapText;
    VkMenuToggle *_toggleLatLongGrid;
    VkMenuToggle *_toggleLatLongLabels;
    VkSubMenu  *_latLongRadioPane;
    VkMenuToggle *_latLongResOption20;
    VkMenuToggle *_latLongResOption10;
    VkMenuToggle *_latLongResOption5;
    VkMenuToggle *_latLongResOption2;
    VkMenuToggle *_latLongResOption1;
    VkSubMenu  *_menuPane1;
    VkMenuItem *_entry10;
    VkMenuItem *_entry14;
    VkMenuItem *_entry13;
    VkMenuItem *_button99;
    VkMenuItem *_button100;
    VkMenuItem *_button101;
    VkSubMenu  *_editPane;
    VkSubMenu  *_textFontRadioPane;
    VkMenuToggle *_fontOption1;
    VkMenuToggle *_fontOption2;
    VkMenuToggle *_fontOption3;
    VkMenuToggle *_fontOption4;
    VkMenuToggle *_fontOption5;
    VkMenuToggle *_fontOption6;
    VkMenuToggle *_fontOption7;
    VkMenuToggle *_fontOption8;
    VkSubMenu  *_fontSizeRadioPane;
    VkMenuToggle *_fontSize1;
    VkMenuToggle *_fontSize2;
    VkMenuToggle *_fontSize3;
    VkMenuToggle *_fontSize4;
    VkMenuToggle *_fontSize5;
    VkMenuToggle *_fontSize6;
    VkMenuToggle *_fontSize7;
    VkMenuToggle *_fontSize8;
    VkMenuToggle *_fontSize9;
    VkMenuToggle *_fontSize10;
    VkMenuToggle *_applyDefToAllTextToggle;
    VkMenuToggle *_scaleTextToWinSizeToggle;
    VkSubMenu  *_annotateModeRadioPane;
    VkMenuToggle *_annotModeTHTI;
    VkMenuToggle *_annotMaodeTWAI;
    VkMenuToggle *_annotModeAWTI;
    VkMenuToggle *_annotModeAWAI;
    VkMenuItem *_button98;
    VkMenuItem *_separator28;
    VkMenuConfirmFirstAction *_action;
    VkSubMenu  *_menuPane5;
    VkMenuItem *_entry15;
    VkMenuItem *_entry16;
    VkMenuItem *_separator30;
    VkMenuItem *_entry17;
    VkMenuItem *_button102;
    VkMenuItem *_button103;
    VkMenuItem *_separator31;
    VkMenuItem *_button104;
    VkMenuItem *_button105;
    VkSubMenu  *_menuPane;
    VkMenuItem *_entry7;
    VkMenuItem *_entry11;
    VkMenuItem *_entry12;
    VkMenuItem *_button94;
    VkMenuItem *_button92;

    // Member functions called from callbacks

    virtual void OGLWinExposed ( Widget, XtPointer );
    virtual void OGLWinGInit ( Widget, XtPointer );
    virtual void OGLWinInput ( Widget, XtPointer );
    virtual void OGLWinResized ( Widget, XtPointer );
    virtual void ResampleThisSeq ( Widget, XtPointer );
    virtual void annotateModeChangedAWAI ( Widget, XtPointer );
    virtual void annotateModeChangedAWTI ( Widget, XtPointer );
    virtual void annotateModeChangedTWAI ( Widget, XtPointer );
    virtual void annotateModeChangedTWTI ( Widget, XtPointer );
    virtual void applyDefToAllText ( Widget, XtPointer );
    virtual void autoResampleChanged ( Widget, XtPointer );
    virtual void clearAnnotations ( Widget, XtPointer );
    virtual void dumpToLocalJPEG ( Widget, XtPointer );
    virtual void dumpToWebJPEG ( Widget, XtPointer );
    virtual void editAnnotateColor ( Widget, XtPointer );
    virtual void editCoastlineColor ( Widget, XtPointer );
    virtual void editColorPal ( Widget, XtPointer );
    virtual void editJpegName ( Widget, XtPointer );
    virtual void editLatLongLineColor ( Widget, XtPointer );
    virtual void editMapTexColor ( Widget, XtPointer );
    virtual void editTitleColor ( Widget, XtPointer );
    virtual void editUserAreaColor ( Widget, XtPointer );
    virtual void fontSizeChanged1 ( Widget, XtPointer );
    virtual void fontSizeChanged10 ( Widget, XtPointer );
    virtual void fontSizeChanged2 ( Widget, XtPointer );
    virtual void fontSizeChanged3 ( Widget, XtPointer );
    virtual void fontSizeChanged4 ( Widget, XtPointer );
    virtual void fontSizeChanged5 ( Widget, XtPointer );
    virtual void fontSizeChanged6 ( Widget, XtPointer );
    virtual void fontSizeChanged7 ( Widget, XtPointer );
    virtual void fontSizeChanged8 ( Widget, XtPointer );
    virtual void fontSizeChanged9 ( Widget, XtPointer );
    virtual void gifAutocreateLocalChanged ( Widget, XtPointer );
    virtual void gifAutocreateWebChanged ( Widget, XtPointer );
    virtual void greyScaleChange ( Widget, XtPointer );
    virtual void histEqualChanged ( Widget, XtPointer );
    virtual void latLongShowLabelChanged ( Widget, XtPointer );
    virtual void latestSeq ( Widget, XtPointer );
    virtual void lineWidth1Changed ( Widget, XtPointer );
    virtual void lineWidth2Changed ( Widget, XtPointer );
    virtual void lineWidth3Changed ( Widget, XtPointer );
    virtual void lineWidth4Changed ( Widget, XtPointer );
    virtual void magLinearChanged ( Widget, XtPointer );
    virtual void magNearestChanged ( Widget, XtPointer );
    virtual void newLatLongRes1 ( Widget, XtPointer );
    virtual void newLatLongRes10 ( Widget, XtPointer );
    virtual void newLatLongRes2 ( Widget, XtPointer );
    virtual void newLatLongRes20 ( Widget, XtPointer );
    virtual void newLatLongRes5 ( Widget, XtPointer );
    virtual void olayCoastChanged ( Widget, XtPointer );
    virtual void olayLatLongChanged ( Widget, XtPointer );
    virtual void olayTextChanged ( Widget, XtPointer );
    virtual void oldestSeq ( Widget, XtPointer );
    virtual void openHighlightWin ( Widget, XtPointer );
    virtual void openHistogram ( Widget, XtPointer );
    virtual void openManNavigation ( Widget, XtPointer );
    virtual void openUserDefinedWinSize ( Widget, XtPointer );
    virtual void print ( Widget, XtPointer );
    virtual void proj3DSpere ( Widget, XtPointer );
    virtual void renavAllSequence ( Widget, XtPointer );
    virtual void resampleAllSequence ( Widget, XtPointer );
    virtual void resampleThisImage ( Widget, XtPointer );
    virtual void resetProjection ( Widget, XtPointer );
    virtual void scaleTextToWinSizeChanged ( Widget, XtPointer );
    virtual void select3ChannelRGB ( Widget, XtPointer );
    virtual void selectCombination ( Widget, XtPointer );
    virtual void selectIR2 ( Widget, XtPointer );
    virtual void selectVis ( Widget, XtPointer );
    virtual void selectWater ( Widget, XtPointer );
    virtual void selectedIR1 ( Widget, XtPointer );
    virtual void setChAllSeq ( Widget, XtPointer );
    virtual void setHistEq ( Widget, XtPointer );
    virtual void setProjEquicyl ( Widget, XtPointer );
    virtual void setProjMercator ( Widget, XtPointer );
    virtual void setProjNavSatellite ( Widget, XtPointer );
    virtual void setProjSatellite ( Widget, XtPointer );
    virtual void setProjStereo ( Widget, XtPointer );
    virtual void startStopSeq ( Widget, XtPointer );
    virtual void stepBwdSeq ( Widget, XtPointer );
    virtual void stepFwdSeq ( Widget, XtPointer );
    virtual void textFontChanged1 ( Widget, XtPointer );
    virtual void textFontChanged2 ( Widget, XtPointer );
    virtual void textFontChanged3 ( Widget, XtPointer );
    virtual void textFontChanged4 ( Widget, XtPointer );
    virtual void textFontChanged5 ( Widget, XtPointer );
    virtual void textFontChanged6 ( Widget, XtPointer );
    virtual void textFontChanged7 ( Widget, XtPointer );
    virtual void textFontChanged8 ( Widget, XtPointer );
    virtual void viewGmsFile ( Widget, XtPointer );
    virtual void winSize1024 ( Widget, XtPointer );
    virtual void winSize256 ( Widget, XtPointer );
    virtual void winSize512 ( Widget, XtPointer );


    //---- Start editable code block: OGlWin protected

	Widget	newGMSFileSelectorWidget;
	virtual void openGMSFile(char *filename = 0);
	void newGMSFileSelectorDeleted(VkComponent *, void *, void *);
    //---- End editable code block: OGlWin protected


  private:


    // Callbacks to interface with Motif

    static void OGLWinExposedCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinGInitCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinInputCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinResizedCallback ( Widget, XtPointer, XtPointer );
    static void ResampleThisSeqCallback ( Widget, XtPointer, XtPointer );
    static void annotateModeChangedAWAICallback ( Widget, XtPointer, XtPointer );
    static void annotateModeChangedAWTICallback ( Widget, XtPointer, XtPointer );
    static void annotateModeChangedTWAICallback ( Widget, XtPointer, XtPointer );
    static void annotateModeChangedTWTICallback ( Widget, XtPointer, XtPointer );
    static void applyDefToAllTextCallback ( Widget, XtPointer, XtPointer );
    static void autoResampleChangedCallback ( Widget, XtPointer, XtPointer );
    static void clearAnnotationsCallback ( Widget, XtPointer, XtPointer );
    static void dumpToLocalJPEGCallback ( Widget, XtPointer, XtPointer );
    static void dumpToWebJPEGCallback ( Widget, XtPointer, XtPointer );
    static void editAnnotateColorCallback ( Widget, XtPointer, XtPointer );
    static void editCoastlineColorCallback ( Widget, XtPointer, XtPointer );
    static void editColorPalCallback ( Widget, XtPointer, XtPointer );
    static void editJpegNameCallback ( Widget, XtPointer, XtPointer );
    static void editLatLongLineColorCallback ( Widget, XtPointer, XtPointer );
    static void editMapTexColorCallback ( Widget, XtPointer, XtPointer );
    static void editTitleColorCallback ( Widget, XtPointer, XtPointer );
    static void editUserAreaColorCallback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged1Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged10Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged2Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged3Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged4Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged5Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged6Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged7Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged8Callback ( Widget, XtPointer, XtPointer );
    static void fontSizeChanged9Callback ( Widget, XtPointer, XtPointer );
    static void gifAutocreateLocalChangedCallback ( Widget, XtPointer, XtPointer );
    static void gifAutocreateWebChangedCallback ( Widget, XtPointer, XtPointer );
    static void greyScaleChangeCallback ( Widget, XtPointer, XtPointer );
    static void histEqualChangedCallback ( Widget, XtPointer, XtPointer );
    static void latLongShowLabelChangedCallback ( Widget, XtPointer, XtPointer );
    static void latestSeqCallback ( Widget, XtPointer, XtPointer );
    static void lineWidth1ChangedCallback ( Widget, XtPointer, XtPointer );
    static void lineWidth2ChangedCallback ( Widget, XtPointer, XtPointer );
    static void lineWidth3ChangedCallback ( Widget, XtPointer, XtPointer );
    static void lineWidth4ChangedCallback ( Widget, XtPointer, XtPointer );
    static void magLinearChangedCallback ( Widget, XtPointer, XtPointer );
    static void magNearestChangedCallback ( Widget, XtPointer, XtPointer );
    static void newLatLongRes1Callback ( Widget, XtPointer, XtPointer );
    static void newLatLongRes10Callback ( Widget, XtPointer, XtPointer );
    static void newLatLongRes2Callback ( Widget, XtPointer, XtPointer );
    static void newLatLongRes20Callback ( Widget, XtPointer, XtPointer );
    static void newLatLongRes5Callback ( Widget, XtPointer, XtPointer );
    static void olayCoastChangedCallback ( Widget, XtPointer, XtPointer );
    static void olayLatLongChangedCallback ( Widget, XtPointer, XtPointer );
    static void olayTextChangedCallback ( Widget, XtPointer, XtPointer );
    static void oldestSeqCallback ( Widget, XtPointer, XtPointer );
    static void openHighlightWinCallback ( Widget, XtPointer, XtPointer );
    static void openHistogramCallback ( Widget, XtPointer, XtPointer );
    static void openManNavigationCallback ( Widget, XtPointer, XtPointer );
    static void openUserDefinedWinSizeCallback ( Widget, XtPointer, XtPointer );
    static void printCallback ( Widget, XtPointer, XtPointer );
    static void proj3DSpereCallback ( Widget, XtPointer, XtPointer );
    static void renavAllSequenceCallback ( Widget, XtPointer, XtPointer );
    static void resampleAllSequenceCallback ( Widget, XtPointer, XtPointer );
    static void resampleThisImageCallback ( Widget, XtPointer, XtPointer );
    static void resetProjectionCallback ( Widget, XtPointer, XtPointer );
    static void scaleTextToWinSizeChangedCallback ( Widget, XtPointer, XtPointer );
    static void select3ChannelRGBCallback ( Widget, XtPointer, XtPointer );
    static void selectCombinationCallback ( Widget, XtPointer, XtPointer );
    static void selectIR2Callback ( Widget, XtPointer, XtPointer );
    static void selectVisCallback ( Widget, XtPointer, XtPointer );
    static void selectWaterCallback ( Widget, XtPointer, XtPointer );
    static void selectedIR1Callback ( Widget, XtPointer, XtPointer );
    static void setChAllSeqCallback ( Widget, XtPointer, XtPointer );
    static void setHistEqCallback ( Widget, XtPointer, XtPointer );
    static void setProjEquicylCallback ( Widget, XtPointer, XtPointer );
    static void setProjMercatorCallback ( Widget, XtPointer, XtPointer );
    static void setProjNavSatelliteCallback ( Widget, XtPointer, XtPointer );
    static void setProjSatelliteCallback ( Widget, XtPointer, XtPointer );
    static void setProjStereoCallback ( Widget, XtPointer, XtPointer );
    static void startStopSeqCallback ( Widget, XtPointer, XtPointer );
    static void stepBwdSeqCallback ( Widget, XtPointer, XtPointer );
    static void stepFwdSeqCallback ( Widget, XtPointer, XtPointer );
    static void textFontChanged1Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged2Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged3Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged4Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged5Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged6Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged7Callback ( Widget, XtPointer, XtPointer );
    static void textFontChanged8Callback ( Widget, XtPointer, XtPointer );
    static void viewGmsFileCallback ( Widget, XtPointer, XtPointer );
    static void winSize1024Callback ( Widget, XtPointer, XtPointer );
    static void winSize256Callback ( Widget, XtPointer, XtPointer );
    static void winSize512Callback ( Widget, XtPointer, XtPointer );

    static String  _defaultOGlWinResources[];


    //---- Start editable code block: OGlWin private

    static void newGMSFileCallback ( Widget, XtPointer, XtPointer );
    static void newGMSClosedCallback ( Widget, XtPointer, XtPointer );
    static void PointerMotionHandler(Widget, XtPointer, 
		XMotionEvent*, Boolean*);

    //---- End editable code block: OGlWin private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
