
//////////////////////////////////////////////////////////////
//
// Header file for SelStnWindow
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
#ifndef SELSTNWINDOW_H
#define SELSTNWINDOW_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "displ.h"
#include "SelStnFormClass.h"

//---- End editable code block: headers and declarations


//---- SelStnWindow class declaration

class SelStnWindow: public VkSimpleWindow {

  public:

    SelStnWindow( const char * name, 
                  ArgList args = NULL,
                  Cardinal argCount = 0 );
    ~SelStnWindow();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: SelStnWindow public

    DisplWin	*CallingWin;	// so calling win can be notified of close
    class SelStnFormClass *StnForm();

    //---- End editable code block: SelStnWindow public


  protected:



    // Classes created by this class

    class SelStnFormClass *_selStnFormClassUI;


    // Widgets created by this class



    //---- Start editable code block: SelStnWindow protected


    //---- End editable code block: SelStnWindow protected


  private:


    static String  _defaultSelStnWindowResources[];


    //---- Start editable code block: SelStnWindow private


    //---- End editable code block: SelStnWindow private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
