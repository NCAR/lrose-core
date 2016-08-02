
//////////////////////////////////////////////////////////////
//
// Header file for SimpleTitanPropsWindow
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
#ifndef SIMPLETITANPROPSWINDOW_H
#define SIMPLETITANPROPSWINDOW_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- End generated headers

#include "drawTitan.h"

//---- SimpleTitanPropsWindow class declaration

class SimpleTitanPropsWindow: public VkSimpleWindow {

  public:

    SimpleTitanPropsWindow( const char * name, 
                            ArgList args = NULL,
                            Cardinal argCount = 0 );
    ~SimpleTitanPropsWindow();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    drawTitan *drawTitanObj;
    void setDrawTitanObj(drawTitan *drawtitanobj);
    void setGUIState();  // set GUI buttons etc to drawTitanObj state
    

  protected:




    // Widgets created by this class

    Widget  _form49;
    Widget  _form50;
    Widget  _form51;
    Widget  _frame11;
    Widget  _frame13;
    Widget  _label74;
    Widget  _label75;
    Widget  _showFwdFcstToggle;
    Widget  _showTitanToggle;
    Widget  _titanHistoryFadeToggle;
    Widget  _titanPropsApplyAllWinsButton;
    Widget  _titanPropsCloseButton;
    Widget  _titanPropsFillShapeToggle1;
    Widget  _titanVILscale;

    VkOptionMenu  *_maxFadeMenu1;
    VkOptionMenu  *_pastTimeMenu;
    VkOptionMenu  *_pastTimeMenu1;
    VkOptionMenu  *_titanAnnotMenu;
    VkOptionMenu  *_titanShapeMenu1;
    VkOptionMenu  *_titanShapeThicknessMenu;
    VkOptionMenu  *_titanThresholdOptionMenu;
    VkOptionMenu  *_titanTrackPathMenu1;
    VkOptionMenu  *_titanTrkThicknessMenu;

    VkMenuItem *_fwd0min;
    VkMenuItem *_fwd30min;
    VkMenuItem *_fwd60min;
    VkMenuItem *_fwd90min;
    VkMenuItem *_fwdAll;
    VkMenuItem *_past0min;
    VkMenuItem *_past30min;
    VkMenuItem *_past60min;
    VkMenuItem *_past90min;
    VkMenuItem *_pastAll;
    VkMenuItem *_titanAnnotAllOption;
    VkMenuItem *_titanAnnotCursOption;
    VkMenuItem *_titanAnnotNoneOption;
    VkMenuItem *_titanMaxFade0_25;
    VkMenuItem *_titanMaxFade0_50;
    VkMenuItem *_titanMaxFade0_75;
    VkMenuItem *_titanShapeEllipse1;
    VkMenuItem *_titanShapeNoShape1;
    VkMenuItem *_titanShapePoly1;
    VkMenuItem *_titanShapeThickness1;
    VkMenuItem *_titanShapeThickness2;
    VkMenuItem *_titanShapeThickness3;
    VkMenuItem *_titanShapeThickness4;
    VkMenuItem *_titanShapeThickness5;
    VkMenuItem *_titanThreshold1Option;
    VkMenuItem *_titanThreshold2Option;
    VkMenuItem *_titanThreshold3Option;
    VkMenuItem *_titanTrackPathArrow1;
    VkMenuItem *_titanTrackPathLine1;
    VkMenuItem *_titanTrackPathNone1;
    VkMenuItem *_titanTrkThickness1;
    VkMenuItem *_titanTrkThickness2;
    VkMenuItem *_titanTrkThickness3;
    VkMenuItem *_titanTrkThickness4;
    VkMenuItem *_titanTrkThickness5;


    // Member functions called from callbacks

    virtual void AnnotCursor ( Widget, XtPointer );
    virtual void annotAll ( Widget, XtPointer );
    virtual void annotNone ( Widget, XtPointer );
    virtual void doThreshold1 ( Widget, XtPointer );
    virtual void doThreshold2 ( Widget, XtPointer );
    virtual void doThreshold3 ( Widget, XtPointer );
    virtual void doTitanPropsCloseButton ( Widget, XtPointer );
    virtual void doTitanPropsMaxFade25Option ( Widget, XtPointer );
    virtual void doTitanPropsMaxFade50Option ( Widget, XtPointer );
    virtual void doTitanPropsMaxFade75Option ( Widget, XtPointer );
    virtual void doTitanPropsShapeThickness1Option ( Widget, XtPointer );
    virtual void doTitanPropsShapeThickness2Option ( Widget, XtPointer );
    virtual void doTitanPropsShapeThickness3Option ( Widget, XtPointer );
    virtual void doTitanPropsShapeThickness4Option ( Widget, XtPointer );
    virtual void doTitanPropsShapeThickness5Option ( Widget, XtPointer );
    virtual void doTitanPropsTrkThickness1Option ( Widget, XtPointer );
    virtual void doTitanPropsTrkThickness2Option ( Widget, XtPointer );
    virtual void doTitanPropsTrkThickness3Option ( Widget, XtPointer );
    virtual void doTitanPropsTrkThickness4Option ( Widget, XtPointer );
    virtual void doTitanPropsTrkThickness5Option ( Widget, XtPointer );
    virtual void doTitanShapeEllipse ( Widget, XtPointer );
    virtual void doTitanShapeNoShape ( Widget, XtPointer );
    virtual void doTitanShapePoly ( Widget, XtPointer );
    virtual void doTitanTrackPathArrow ( Widget, XtPointer );
    virtual void doTitanTrackPathLine ( Widget, XtPointer );
    virtual void doTitanTrackPathNone ( Widget, XtPointer );
    virtual void doWDSSApplyAllWinsButton ( Widget, XtPointer );
    virtual void fwdTime0min ( Widget, XtPointer );
    virtual void fwdTime30min ( Widget, XtPointer );
    virtual void fwdTime60min ( Widget, XtPointer );
    virtual void fwdTime90min ( Widget, XtPointer );
    virtual void fwdTimeAll ( Widget, XtPointer );
    virtual void minTitanCellVILChanged ( Widget, XtPointer );
    virtual void pastTime0min ( Widget, XtPointer );
    virtual void pastTime30min ( Widget, XtPointer );
    virtual void pastTime60min ( Widget, XtPointer );
    virtual void pastTime90min ( Widget, XtPointer );
    virtual void pastTimeAll ( Widget, XtPointer );
    virtual void setHistoryFade ( Widget, XtPointer );
    virtual void setShowFwdFcstToggle ( Widget, XtPointer );
    virtual void setShowTitanToggle ( Widget, XtPointer );
    virtual void setTitanPropsFillShapeToggle ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void AnnotCursorCallback ( Widget, XtPointer, XtPointer );
    static void annotAllCallback ( Widget, XtPointer, XtPointer );
    static void annotNoneCallback ( Widget, XtPointer, XtPointer );
    static void doThreshold1Callback ( Widget, XtPointer, XtPointer );
    static void doThreshold2Callback ( Widget, XtPointer, XtPointer );
    static void doThreshold3Callback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsCloseButtonCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsMaxFade25OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsMaxFade50OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsMaxFade75OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsShapeThickness1OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsShapeThickness2OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsShapeThickness3OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsShapeThickness4OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsShapeThickness5OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsTrkThickness1OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsTrkThickness2OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsTrkThickness3OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsTrkThickness4OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanPropsTrkThickness5OptionCallback ( Widget, XtPointer, XtPointer );
    static void doTitanShapeEllipseCallback ( Widget, XtPointer, XtPointer );
    static void doTitanShapeNoShapeCallback ( Widget, XtPointer, XtPointer );
    static void doTitanShapePolyCallback ( Widget, XtPointer, XtPointer );
    static void doTitanTrackPathArrowCallback ( Widget, XtPointer, XtPointer );
    static void doTitanTrackPathLineCallback ( Widget, XtPointer, XtPointer );
    static void doTitanTrackPathNoneCallback ( Widget, XtPointer, XtPointer );
    static void doWDSSApplyAllWinsButtonCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime0minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime30minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime60minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTime90minCallback ( Widget, XtPointer, XtPointer );
    static void fwdTimeAllCallback ( Widget, XtPointer, XtPointer );
    static void minTitanCellVILChangedCallback ( Widget, XtPointer, XtPointer );
    static void pastTime0minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime30minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime60minCallback ( Widget, XtPointer, XtPointer );
    static void pastTime90minCallback ( Widget, XtPointer, XtPointer );
    static void pastTimeAllCallback ( Widget, XtPointer, XtPointer );
    static void setHistoryFadeCallback ( Widget, XtPointer, XtPointer );
    static void setShowFwdFcstToggleCallback ( Widget, XtPointer, XtPointer );
    static void setShowTitanToggleCallback ( Widget, XtPointer, XtPointer );
    static void setTitanPropsFillShapeToggleCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultSimpleTitanPropsWindowResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
