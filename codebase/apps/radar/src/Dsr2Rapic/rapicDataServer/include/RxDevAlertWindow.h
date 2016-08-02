
//////////////////////////////////////////////////////////////
//
// Header file for RxDevAlertWindow
//
//    This class is a subclass of VkSimpleWindow
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
#ifndef RXDEVALERTWINDOW_H
#define RXDEVALERTWINDOW_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "rpcomms.h"

//---- End editable code block: headers and declarations


//---- RxDevAlertWindow class declaration

class RxDevAlertWindow: public VkSimpleWindow {

  public:

    RxDevAlertWindow( const char * name, 
                      ArgList args = NULL,
                      Cardinal argCount = 0 );
    ~RxDevAlertWindow();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: RxDevAlertWindow public

    void upDate(rxdevice *thisdev);
    void upDate(txdevice *thisdev);
    void upDate();

    //---- End editable code block: RxDevAlertWindow public


  protected:




    // Widgets created by this class

    Widget  _ackButton1;
    Widget  _alertPostedTimeLabel1;
    Widget  _alertTitleLabel1;
    Widget  _alertTypeLabel1;
    Widget  _commentLabel;
    Widget  _devNameLabel1;
    Widget  _form21;
    Widget  _label32;
    Widget  _label47;
    Widget  _label50;
    Widget  _label51;
    Widget  _label52;
    Widget  _reAlertPeriodText;
    Widget  _silenceFutureAlertToggle;
    Widget  _silenceReAlertToggle;


    // Member functions called from callbacks

    virtual void acknowledged ( Widget, XtPointer );
    virtual void changeRealertPeriod ( Widget, XtPointer );
    virtual void silenceFutureAlertsChanged ( Widget, XtPointer );
    virtual void silenceReAlertsChanged ( Widget, XtPointer );


    //---- Start editable code block: RxDevAlertWindow protected


    //---- End editable code block: RxDevAlertWindow protected


  private:


    // Callbacks to interface with Motif

    static void acknowledgedCallback ( Widget, XtPointer, XtPointer );
    static void changeRealertPeriodCallback ( Widget, XtPointer, XtPointer );
    static void silenceFutureAlertsChangedCallback ( Widget, XtPointer, XtPointer );
    static void silenceReAlertsChangedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultRxDevAlertWindowResources[];


    //---- Start editable code block: RxDevAlertWindow private

    rxdevice *rxdev;
    txdevice *txdev;
    
    //---- End editable code block: RxDevAlertWindow private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
