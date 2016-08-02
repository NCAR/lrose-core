
//////////////////////////////////////////////////////////////
//
// Header file for CommMngWindowClass
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
#ifndef COMMMNGWINDOWCLASS_H
#define COMMMNGWINDOWCLASS_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- Start editable code block: headers and declarations

#include "rpcomms.h"

//---- End editable code block: headers and declarations


//---- CommMngWindowClass class declaration

class CommMngWindowClass: public VkWindow {

  public:

    CommMngWindowClass( const char * name, 
                        ArgList args = NULL,
                        Cardinal argCount = 0 );
    ~CommMngWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: CommMngWindowClass public

    void upDate(RPCommMng *commmngr = 0, int force = 0);
    RPCommMng *lastCommMngr;
    
    //---- End editable code block: CommMngWindowClass public


  protected:



    // Classes created by this class

    class CommMngTabbedDeckClass *_deck;


    // Widgets created by this class


    // Menu items created by this class
    VkSubMenu  *_menuPane2;
    VkMenuItem *_separator10;
    VkMenuItem *_entry5;
    VkMenuItem *_button32;
    VkMenuItem *_separator1;
    VkMenuItem *_button39;
    VkSubMenu  *_menuPane3;
    VkMenuItem *_entry6;
    VkMenuItem *_button40;
    VkMenuItem *_button44;
    VkMenuToggle *_silenceAllAlertsToggle;
    VkMenuToggle *_suppressAllAlertsToggle2;

    // Member functions called from callbacks

    virtual void acknowledgeAllAlerts ( Widget, XtPointer );
    virtual void addRxDev ( Widget, XtPointer );
    virtual void addTxDev ( Widget, XtPointer );
    virtual void silenceAllAlertsChanged ( Widget, XtPointer );
    virtual void suppressAllAlertsChanged ( Widget, XtPointer );
    virtual void uifCloseManager ( Widget, XtPointer );
    virtual void uifSetAllRealertPeriod ( Widget, XtPointer );
    virtual void uifShowAllAlerts ( Widget, XtPointer );


    //---- Start editable code block: CommMngWindowClass protected


    //---- End editable code block: CommMngWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void acknowledgeAllAlertsCallback ( Widget, XtPointer, XtPointer );
    static void addRxDevCallback ( Widget, XtPointer, XtPointer );
    static void addTxDevCallback ( Widget, XtPointer, XtPointer );
    static void silenceAllAlertsChangedCallback ( Widget, XtPointer, XtPointer );
    static void suppressAllAlertsChangedCallback ( Widget, XtPointer, XtPointer );
    static void uifCloseManagerCallback ( Widget, XtPointer, XtPointer );
    static void uifSetAllRealertPeriodCallback ( Widget, XtPointer, XtPointer );
    static void uifShowAllAlertsCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultCommMngWindowClassResources[];


    //---- Start editable code block: CommMngWindowClass private


    //---- End editable code block: CommMngWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
