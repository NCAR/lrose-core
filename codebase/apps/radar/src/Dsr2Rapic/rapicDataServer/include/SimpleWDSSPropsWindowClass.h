
//////////////////////////////////////////////////////////////
//
// Header file for SimpleWDSSPropsWindowClass
//
//    This class is a subclass of VkSimpleWindow
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
#ifndef SIMPLEWDSSPROPSWINDOWCLASS_H
#define SIMPLEWDSSPROPSWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- End generated headers

#include "drawWDSS.h"

//---- SimpleWDSSPropsWindowClass class declaration

class SimpleWDSSPropsWindowClass: public VkSimpleWindow {

  public:

    SimpleWDSSPropsWindowClass( const char * name, 
                                ArgList args = NULL,
                                Cardinal argCount = 0 );
    ~SimpleWDSSPropsWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    drawWDSS *drawWDSSObj;
    void setDrawWDSSObj(drawWDSS *drawwdssobj);
    void setGUIState();


  protected:




    // Widgets created by this class

    Widget  _form53;
    Widget  _form54;
    Widget  _form55;
    Widget  _frame12;
    Widget  _frame14;
    Widget  _label76;
    Widget  _label77;
    Widget  _showFwdFcstToggle1;
    Widget  _showWDSSToggle;
    Widget  _titanPropsCloseButton1;
    Widget  _wdsHistoryFadeToggle;
    Widget  _wdssPropsApplyAllWinsButton;
    Widget  _wdssValScale;

    VkOptionMenu  *_wdssAnnotDetailMenu;
    VkOptionMenu  *_wdssAnnotModeMenu;
    VkOptionMenu  *_wdssCellScaleMenu;
    VkOptionMenu  *_wdssFilterTypeMenu;
    VkOptionMenu  *_wdssFwdTimeMenu;
    VkOptionMenu  *_wdssFwdTrkMenu;
    VkOptionMenu  *_wdssMaxFadeMenu;
    VkOptionMenu  *_wdssPastTimeMenu;
    VkOptionMenu  *_wdssShapeThicknessMenu;
    VkOptionMenu  *_wdssTrackPathMenu;
    VkOptionMenu  *_wdssTrkOptionMenu;
    VkOptionMenu  *_wdssTrkThicknessMenu;

    VkMenuItem *_wdssAnnotAll;
    VkMenuItem *_wdssAnnotCurs;
    VkMenuItem *_wdssAnnotDetailMax;
    VkMenuItem *_wdssAnnotDetailMed;
    VkMenuItem *_wdssAnnotDetailMin;
    VkMenuItem *_wdssAnnotNone;
    VkMenuItem *_wdssCellScale0_1;
    VkMenuItem *_wdssCellScale0_2;
    VkMenuItem *_wdssCellScale0_5;
    VkMenuItem *_wdssCellScale1_0;
    VkMenuItem *_wdssCellScale2_0;
    VkMenuItem *_wdssCellScale4_0;
    VkMenuItem *_wdssFilterMaxdBZ;
    VkMenuItem *_wdssFilterProbHail;
    VkMenuItem *_wdssFilterVIL;
    VkMenuItem *_wdssFwd0min;
    VkMenuItem *_wdssFwd30min;
    VkMenuItem *_wdssFwd60min;
    VkMenuItem *_wdssFwd90min;
    VkMenuItem *_wdssFwdAll;
    VkMenuItem *_wdssFwdTrkCell;
    VkMenuItem *_wdssFwdTrkCellsTrk;
    VkMenuItem *_wdssFwdTrkOnly;
    VkMenuItem *_wdssMaxFade0_25;
    VkMenuItem *_wdssMaxFade0_50;
    VkMenuItem *_wdssMaxFade0_75;
    VkMenuItem *_wdssPast0min;
    VkMenuItem *_wdssPast30min;
    VkMenuItem *_wdssPast60min;
    VkMenuItem *_wdssPast90min;
    VkMenuItem *_wdssPastAll;
    VkMenuItem *_wdssPastTrkCell;
    VkMenuItem *_wdssPastTrkCellsTrk;
    VkMenuItem *_wdssPastTrkOnly;
    VkMenuItem *_wdssShapeThickness1;
    VkMenuItem *_wdssShapeThickness2;
    VkMenuItem *_wdssShapeThickness3;
    VkMenuItem *_wdssShapeThickness4;
    VkMenuItem *_wdssShapeThickness5;
    VkMenuItem *_wdssTrackPathArrow;
    VkMenuItem *_wdssTrackPathLine;
    VkMenuItem *_wdssTrackPathNone;
    VkMenuItem *_wdssTrkThickness1;
    VkMenuItem *_wdssTrkThickness2;
    VkMenuItem *_wdssTrkThickness3;
    VkMenuItem *_wdssTrkThickness4;
    VkMenuItem *_wdssTrkThickness5;


    // Member functions called from callbacks

    virtual void AnnotCursor ( Widget, XtPointer );
    virtual void DrawFwdCellsOnly ( Widget, XtPointer );
    virtual void DrawFwdTrksOnly ( Widget, XtPointer );
    virtual void DrawPastCellsOnly ( Widget, XtPointer );
    virtual void DrawPastTrksOnly ( Widget, XtPointer );
    virtual void annotAll ( Widget, XtPointer );
    virtual void annotDetailMax ( Widget, XtPointer );
    virtual void annotDetailMed ( Widget, XtPointer );
    virtual void annotDetailMin ( Widget, XtPointer );
    virtual void annotNone ( Widget, XtPointer );
    virtual void doWDSSApplyAllWinsButton ( Widget, XtPointer );
    virtual void doWDSSCloseButton ( Widget, XtPointer );
    virtual void doWDSSMaxFade25Option ( Widget, XtPointer );
    virtual void doWDSSMaxFade50Option ( Widget, XtPointer );
    virtual void doWDSSMaxFade75Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale01Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale02Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale05Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale10Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale20Option ( Widget, XtPointer );
    virtual void doWDSSShapeScale40Option ( Widget, XtPointer );
    virtual void doWDSSShapeThickness1Option ( Widget, XtPointer );
    virtual void doWDSSShapeThickness2Option ( Widget, XtPointer );
    virtual void doWDSSShapeThickness3Option ( Widget, XtPointer );
    virtual void doWDSSShapeThickness4Option ( Widget, XtPointer );
    virtual void doWDSSShapeThickness5Option ( Widget, XtPointer );
    virtual void doWDSSTrackPathArrow ( Widget, XtPointer );
    virtual void doWDSSTrackPathLine ( Widget, XtPointer );
    virtual void doWDSSTrackPathNone ( Widget, XtPointer );
    virtual void doWDSSTrkThickness1Option ( Widget, XtPointer );
    virtual void doWDSSTrkThickness2Option ( Widget, XtPointer );
    virtual void doWDSSTrkThickness3Option ( Widget, XtPointer );
    virtual void doWDSSTrkThickness4Option ( Widget, XtPointer );
    virtual void doWDSSTrkThickness5Option ( Widget, XtPointer );
    virtual void drawFwdCellsTracks ( Widget, XtPointer );
    virtual void drawPastCellsTracks ( Widget, XtPointer );
    virtual void filterMaxdBZ ( Widget, XtPointer );
    virtual void filterProbHail ( Widget, XtPointer );
    virtual void filterVIL ( Widget, XtPointer );
    virtual void fwdTime0min ( Widget, XtPointer );
    virtual void fwdTime30min ( Widget, XtPointer );
    virtual void fwdTime60min ( Widget, XtPointer );
    virtual void fwdTime90min ( Widget, XtPointer );
    virtual void fwdTimeAll ( Widget, XtPointer );
    virtual void pastTime0min ( Widget, XtPointer );
    virtual void pastTime30min ( Widget, XtPointer );
    virtual void pastTime60min ( Widget, XtPointer );
    virtual void pastTime90min ( Widget, XtPointer );
    virtual void pastTimeAll ( Widget, XtPointer );
    virtual void setHistoryFade ( Widget, XtPointer );
    virtual void setShowFwdFcstToggle ( Widget, XtPointer );
    virtual void setShowWDSSToggle ( Widget, XtPointer );
    virtual void wdssFilterValScaleChanged ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void AnnotCursorCallback ( Widget, XtPointer, XtPointer );
    static void DrawFwdCellsOnlyCallback ( Widget, XtPointer, XtPointer );
    static void DrawFwdTrksOnlyCallback ( Widget, XtPointer, XtPointer );
    static void DrawPastCellsOnlyCallback ( Widget, XtPointer, XtPointer );
    static void DrawPastTrksOnlyCallback ( Widget, XtPointer, XtPointer );
    static void annotAllCallback ( Widget, XtPointer, XtPointer );
    static void annotDetailMaxCallback ( Widget, XtPointer, XtPointer );
    static void annotDetailMedCallback ( Widget, XtPointer, XtPointer );
    static void annotDetailMinCallback ( Widget, XtPointer, XtPointer );
    static void annotNoneCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSApplyAllWinsButtonCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSCloseButtonCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSMaxFade25OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSMaxFade50OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSMaxFade75OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale01OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale02OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale05OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale10OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale20OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeScale40OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeThickness1OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeThickness2OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeThickness3OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeThickness4OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSShapeThickness5OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrackPathArrowCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrackPathLineCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrackPathNoneCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrkThickness1OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrkThickness2OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrkThickness3OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrkThickness4OptionCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSTrkThickness5OptionCallback ( Widget, XtPointer, XtPointer );
    static void drawFwdCellsTracksCallback ( Widget, XtPointer, XtPointer );
    static void drawPastCellsTracksCallback ( Widget, XtPointer, XtPointer );
    static void filterMaxdBZCallback ( Widget, XtPointer, XtPointer );
    static void filterProbHailCallback ( Widget, XtPointer, XtPointer );
    static void filterVILCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime0minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime30minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime60minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime90minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTimeAllCallback ( Widget, XtPointer, XtPointer );
    static void pastTime0minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime30minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime60minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime90minCallback ( Widget, XtPointer, XtPointer );
    static void pastTimeAllCallback ( Widget, XtPointer, XtPointer );
    static void setHistoryFadeCallback ( Widget, XtPointer, XtPointer );
    static void setShowFwdFcstToggleCallback ( Widget, XtPointer, XtPointer );
    static void setShowWDSSToggleCallback ( Widget, XtPointer, XtPointer );
    static void wdssFilterValScaleChangedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultSimpleWDSSPropsWindowClassResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
