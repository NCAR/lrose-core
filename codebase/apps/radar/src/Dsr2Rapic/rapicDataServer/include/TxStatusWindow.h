
//////////////////////////////////////////////////////////////
//
// Header file for TxStatusWindow
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
#ifndef TXSTATUSWINDOW_H
#define TXSTATUSWINDOW_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "rpcomms.h"

//---- End editable code block: headers and declarations


//---- TxStatusWindow class declaration

class TxStatusWindow: public VkSimpleWindow {

  public:

    TxStatusWindow( const char * name, 
                    ArgList args = NULL,
                    Cardinal argCount = 0 );
    ~TxStatusWindow();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: TxStatusWindow public

    void upDate(txdevice *txdev = 0);
    txdevice *thisdevice;


    //---- End editable code block: TxStatusWindow public


  protected:



    // Classes created by this class

    class TxCommStatusForm *_txCommStatusForm2;


    // Widgets created by this class

    Widget  _button46;
    Widget  _form23;


    // Member functions called from callbacks

    virtual void closeWin ( Widget, XtPointer );


    //---- Start editable code block: TxStatusWindow protected


    //---- End editable code block: TxStatusWindow protected


  private:


    // Callbacks to interface with Motif

    static void closeWinCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultTxStatusWindowResources[];


    //---- Start editable code block: TxStatusWindow private


    //---- End editable code block: TxStatusWindow private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
