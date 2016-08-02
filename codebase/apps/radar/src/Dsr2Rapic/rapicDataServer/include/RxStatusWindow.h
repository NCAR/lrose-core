
//////////////////////////////////////////////////////////////
//
// Header file for RxStatusWindow
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
#ifndef RXSTATUSWINDOW_H
#define RXSTATUSWINDOW_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "rpcomms.h"

//---- End editable code block: headers and declarations


//---- RxStatusWindow class declaration

class RxStatusWindow: public VkSimpleWindow {

  public:

    RxStatusWindow( const char * name, 
                    ArgList args = NULL,
                    Cardinal argCount = 0 );
    ~RxStatusWindow();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: RxStatusWindow public

    void upDate(rxdevice *rxdev = 0);
    rxdevice *thisdevice;

    //---- End editable code block: RxStatusWindow public


  protected:



    // Classes created by this class

    class RxCommStatusForm *_rxCommStatusForm;


    // Widgets created by this class

    Widget  _button49;
    Widget  _form22;


    // Member functions called from callbacks

    virtual void closeWin ( Widget, XtPointer );


    //---- Start editable code block: RxStatusWindow protected


    //---- End editable code block: RxStatusWindow protected


  private:


    // Callbacks to interface with Motif

    static void closeWinCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultRxStatusWindowResources[];


    //---- Start editable code block: RxStatusWindow private


    //---- End editable code block: RxStatusWindow private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
