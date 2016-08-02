
//////////////////////////////////////////////////////////////
//
// Header file for RpmainWindowClass
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
#ifndef RPMAINWINDOWCLASS_H
#define RPMAINWINDOWCLASS_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- End generated headers

#include "rpcomms.h"

//---- RpmainWindowClass class declaration

class RpmainWindowClass: public VkWindow {

  public:

    RpmainWindowClass( const char * name, 
                       ArgList args = NULL,
                       Cardinal argCount = 0 );
    ~RpmainWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: RpmainWindowClass public

    virtual void ShowRadarMenus(unsigned char Switch = 1);
    virtual void ShowSatMenus(unsigned char Switch = 1);
    virtual void SetToggles();
    void openLoadNamedLayout();
    void openSaveNamedLayout();
	


  protected:



    // Classes created by this class

    class RpmainformClass *_rpmainformClassUI;


    // Widgets created by this class


    // Menu items created by this class
    VkSubMenu  *_seqPane1;
    VkSubMenu  *_addRdrWinPane;
    VkMenuItem *_label4;
    VkMenuItem *_addppiButton1;
    VkMenuItem *_addppivxButton1;
    VkMenuItem *_addppivx3dButton1;
    VkMenuItem *_add3dtopsButton1;
    VkMenuItem *_button6;
    VkMenuItem *_button7;
    VkMenuItem *_button59;
    VkSubMenu  *_seqLatestPane;
    VkMenuToggle *_option1;
    VkMenuToggle *_option2;
    VkMenuToggle *_option3;
    VkMenuItem *_separator3;
    VkSubMenu  *_winGroupsPane;
    VkMenuToggle *_winGroupsToggleAll;
    VkMenuToggle *_winGroupsToggleNone;
    VkMenuItem *_separator44;
    VkMenuToggle *_winGroupsToggle1;
    VkMenuToggle *_winGroupsToggle2;
    VkMenuToggle *_winGroupsToggle3;
    VkMenuToggle *_winGroupsToggle4;
    VkMenuToggle *_winGroupsToggle5;
    VkMenuToggle *_winGroupsToggle6;
    VkMenuToggle *_winGroupsToggle7;
    VkMenuToggle *_winGroupsToggle8;
    VkMenuToggle *_winGroupsToggle9;
    VkSubMenu  *_windowLayoutMenuPane;
    VkMenuItem *_saveasButton1;
    VkMenuItem *_printButton1;
    VkMenuItem *_closeButton1;
    VkMenuItem *_button16;
    VkMenuItem *_button17;
    VkMenuItem *_separator29;
    VkMenuToggle *_localTime_toggle;
    VkMenuToggle *_suppressFutureDataToggle;
    VkMenuToggle *_toggle1;
    VkMenuItem *_separator8;
    VkMenuConfirmFirstAction *_action1;
    VkMenuConfirmFirstAction *_action2;
    VkMenuItem *_separator11;
    VkMenuItem *_exitButton1;
    VkSubMenu  *_dbPane;
    VkMenuItem *_entry;
    VkMenuItem *_entry2;
    VkSubMenu  *_commsPane;
    VkMenuItem *_entry1;
    VkMenuItem *_entry3;
    VkMenuItem *_entry4;
    VkMenuItem *_button18;
    VkMenuItem *_button25;
    VkMenuItem *_button26;
    VkMenuItem *_button51;
    VkMenuToggle *_silenceAllAlertsToggle1;
    VkMenuToggle *_suppressAllAlertsToggle;
    VkSubMenu  *_menuPane6;
    VkMenuItem *_rainAccumButton;
    VkMenuItem *_setTitanButton;
    VkMenuItem *_setZRButton;

    // Member functions called from callbacks

    virtual void AllowReplayChanged ( Widget, XtPointer );
    virtual void ackAllAlerts ( Widget, XtPointer );
    virtual void openTestSat ( Widget, XtPointer );
    virtual void setWinGroupsToggle1 ( Widget, XtPointer );
    virtual void setWinGroupsToggle2 ( Widget, XtPointer );
    virtual void setWinGroupsToggle3 ( Widget, XtPointer );
    virtual void setWinGroupsToggle4 ( Widget, XtPointer );
    virtual void setWinGroupsToggle5 ( Widget, XtPointer );
    virtual void setWinGroupsToggle6 ( Widget, XtPointer );
    virtual void setWinGroupsToggle7 ( Widget, XtPointer );
    virtual void setWinGroupsToggle8 ( Widget, XtPointer );
    virtual void setWinGroupsToggle9 ( Widget, XtPointer );
    virtual void setWinGroupsToggleAll ( Widget, XtPointer );
    virtual void silenceAllAlertsChanged ( Widget, XtPointer );
    virtual void suppressAllAlertsChanged ( Widget, XtPointer );
    virtual void suppressFutureChanged ( Widget, XtPointer );
    virtual void uifDelAllSequence ( Widget, XtPointer );
    virtual void uifExitRequest ( Widget, XtPointer );
    virtual void uifOpenAccumMng ( Widget, XtPointer );
    virtual void uifOpenNewCommMng ( Widget, XtPointer );
    virtual void uifSaveSched ( Widget, XtPointer );
    virtual void uifSetZRCoeff ( Widget, XtPointer );
    virtual void uifaddCAPPI ( Widget, XtPointer );
    virtual void uifaddPPI ( Widget, XtPointer );
    virtual void uifaddPPIVX ( Widget, XtPointer );
    virtual void uifaddPPIVX3D ( Widget, XtPointer );
    virtual void uifaddTops ( Widget, XtPointer );
    virtual void uifaddVIL ( Widget, XtPointer );
    virtual void uifdeleteImg ( Widget, XtPointer );
    virtual void uiflatestComplete ( Widget, XtPointer );
    virtual void uiflatestScanbyScan ( Widget, XtPointer );
    virtual void uiflatestStatic ( Widget, XtPointer );
    virtual void uifloadRealTime ( Widget, XtPointer );
    virtual void uifloadcmapDefault ( Widget, XtPointer );
    virtual void uifloadcmapPrinter ( Widget, XtPointer );
    virtual void uifloadlayoutDefault ( Widget, XtPointer );
    virtual void uifloadlayoutNamed ( Widget, XtPointer );
    virtual void uiflocalTimeChanged ( Widget, XtPointer );
    virtual void uifopenDBBrowser ( Widget, XtPointer );
    virtual void uifopenEditReq ( Widget, XtPointer );
    virtual void uifopenEditSched ( Widget, XtPointer );
    virtual void uifopenReqData ( Widget, XtPointer );
    virtual void uifrestartComms ( Widget, XtPointer );
    virtual void uifsavelayoutNamed ( Widget, XtPointer );
    virtual void uifsetTitanParams ( Widget, XtPointer );


    //--- End generated code section




  private:


    // Callbacks to interface with Motif

    static void AllowReplayChangedCallback ( Widget, XtPointer, XtPointer );
    static void ackAllAlertsCallback ( Widget, XtPointer, XtPointer );
    static void openTestSatCallback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle1Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle2Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle3Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle4Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle5Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle6Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle7Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle8Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggle9Callback ( Widget, XtPointer, XtPointer );
    static void setWinGroupsToggleAllCallback ( Widget, XtPointer, XtPointer );
    static void silenceAllAlertsChangedCallback ( Widget, XtPointer, XtPointer );
    static void suppressAllAlertsChangedCallback ( Widget, XtPointer, XtPointer );
    static void suppressFutureChangedCallback ( Widget, XtPointer, XtPointer );
    static void uifDelAllSequenceCallback ( Widget, XtPointer, XtPointer );
    static void uifExitRequestCallback ( Widget, XtPointer, XtPointer );
    static void uifOpenAccumMngCallback ( Widget, XtPointer, XtPointer );
    static void uifOpenNewCommMngCallback ( Widget, XtPointer, XtPointer );
    static void uifSaveSchedCallback ( Widget, XtPointer, XtPointer );
    static void uifSetZRCoeffCallback ( Widget, XtPointer, XtPointer );
    static void uifaddCAPPICallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPICallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPIVXCallback ( Widget, XtPointer, XtPointer );
    static void uifaddPPIVX3DCallback ( Widget, XtPointer, XtPointer );
    static void uifaddTopsCallback ( Widget, XtPointer, XtPointer );
    static void uifaddVILCallback ( Widget, XtPointer, XtPointer );
    static void uifdeleteImgCallback ( Widget, XtPointer, XtPointer );
    static void uiflatestCompleteCallback ( Widget, XtPointer, XtPointer );
    static void uiflatestScanbyScanCallback ( Widget, XtPointer, XtPointer );
    static void uiflatestStaticCallback ( Widget, XtPointer, XtPointer );
    static void uifloadRealTimeCallback ( Widget, XtPointer, XtPointer );
    static void uifloadcmapDefaultCallback ( Widget, XtPointer, XtPointer );
    static void uifloadcmapPrinterCallback ( Widget, XtPointer, XtPointer );
    static void uifloadlayoutDefaultCallback ( Widget, XtPointer, XtPointer );
    static void uifloadlayoutNamedCallback ( Widget, XtPointer, XtPointer );
    static void uiflocalTimeChangedCallback ( Widget, XtPointer, XtPointer );
    static void uifopenDBBrowserCallback ( Widget, XtPointer, XtPointer );
    static void uifopenEditReqCallback ( Widget, XtPointer, XtPointer );
    static void uifopenEditSchedCallback ( Widget, XtPointer, XtPointer );
    static void uifopenReqDataCallback ( Widget, XtPointer, XtPointer );
    static void uifrestartCommsCallback ( Widget, XtPointer, XtPointer );
    static void uifsavelayoutNamedCallback ( Widget, XtPointer, XtPointer );
    static void uifsetTitanParamsCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultRpmainWindowClassResources[];


    //---- Start editable code block: RpmainWindowClass private

	virtual void afterRealizeHook();
	virtual void handleWmDeleteMessage();
	virtual void handleWmQuitMessage();
	


};
//---- End of generated code

extern RpmainWindowClass *MainWindow;

#endif
