
//////////////////////////////////////////////////////////////
//
// Header file for StormRelSimpleWindow
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
#ifndef STORMRELSIMPLEWINDOW_H
#define STORMRELSIMPLEWINDOW_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- End generated headers

#include "stormRelClass.h"

//---- StormRelSimpleWindow class declaration

class StormRelSimpleWindow: public VkSimpleWindow {

  public:

    StormRelSimpleWindow( const char * name, 
                          ArgList args = NULL,
                          Cardinal argCount = 0 );
    ~StormRelSimpleWindow();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    static const char *const stormRelChangedCallback;
    
    stormRelClass stormRelParams;
    void  setGUI();
    void  setStormRelParams(stormRelClass &newStormRelParams);
  protected:




    // Widgets created by this class

    Widget  _label13;
    Widget  _manStormRelCloseButton1;
    Widget  _manStormRelDirScale2;
    Widget  _manStormRelVelScale2;
    Widget  _stormrelform;

    VkOptionMenu  *_stormRelOptionMenu1;
    VkOptionMenu  *_stormRelViewMenu2;

    VkMenuItem *_button9;
    VkMenuItem *_cursorRelVelOption;
    VkMenuItem *_stormRelDisabled1;
    VkMenuItem *_stormRelManualOption1;
    VkMenuItem *_stormRelTitanAvgOption1;
    VkMenuItem *_stormRelTitanCellOption1;
    VkMenuItem *_stormRelViewFixed;
    VkMenuItem *_stormRelViewTransient;


    // Member functions called from callbacks

    virtual void ManStormRelDirChanged ( Widget, XtPointer );
    virtual void ManStormRelDirDragged ( Widget, XtPointer );
    virtual void ManStormRelVelChanged ( Widget, XtPointer );
    virtual void ManStormRelVelDragged ( Widget, XtPointer );
    virtual void doCurRelShear ( Widget, XtPointer );
    virtual void doCurRelVel ( Widget, XtPointer );
    virtual void doManStormRelCloseButton ( Widget, XtPointer );
    virtual void doStormRelDisabled ( Widget, XtPointer );
    virtual void doStormRelFixed ( Widget, XtPointer );
    virtual void doStormRelManualOption ( Widget, XtPointer );
    virtual void doStormRelTitanAvgOption ( Widget, XtPointer );
    virtual void doStormRelTitanCellOption ( Widget, XtPointer );
    virtual void doStormRelTransient ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void ManStormRelDirChangedCallback ( Widget, XtPointer, XtPointer );
    static void ManStormRelDirDraggedCallback ( Widget, XtPointer, XtPointer );
    static void ManStormRelVelChangedCallback ( Widget, XtPointer, XtPointer );
    static void ManStormRelVelDraggedCallback ( Widget, XtPointer, XtPointer );
    static void doCurRelShearCallback ( Widget, XtPointer, XtPointer );
    static void doCurRelVelCallback ( Widget, XtPointer, XtPointer );
    static void doManStormRelCloseButtonCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelDisabledCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelFixedCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelManualOptionCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelTitanAvgOptionCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelTitanCellOptionCallback ( Widget, XtPointer, XtPointer );
    static void doStormRelTransientCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultStormRelSimpleWindowResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
