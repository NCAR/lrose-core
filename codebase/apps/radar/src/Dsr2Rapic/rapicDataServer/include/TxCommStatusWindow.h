
//////////////////////////////////////////////////////////////
//
// Header file for TxCommStatusWindow
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
#ifndef TXCOMMSTATUSWINDOW_H
#define TXCOMMSTATUSWINDOW_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations


//---- End editable code block: headers and declarations


//---- TxCommStatusWindow class declaration

class TxCommStatusWindow: public VkSimpleWindow {

  public:

    TxCommStatusWindow( const char * name, 
                        ArgList args = NULL,
                        Cardinal argCount = 0 );
    ~TxCommStatusWindow();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: TxCommStatusWindow public


    //---- End editable code block: TxCommStatusWindow public


  protected:



    // Classes created by this class

    class TxCommStatusWindowForm *_txCommStatusWindowForm;


    // Widgets created by this class



    //---- Start editable code block: TxCommStatusWindow protected


    //---- End editable code block: TxCommStatusWindow protected


  private:


    static String  _defaultTxCommStatusWindowResources[];


    //---- Start editable code block: TxCommStatusWindow private


    //---- End editable code block: TxCommStatusWindow private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
