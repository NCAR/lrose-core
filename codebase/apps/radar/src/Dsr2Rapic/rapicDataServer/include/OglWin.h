
//////////////////////////////////////////////////////////////
//
// Header file for OglWin
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
#ifndef OGLWIN_H
#define OGLWIN_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- Start editable code block: headers and declarations


//---- End editable code block: headers and declarations


//---- OglWin class declaration

class OglWin: public VkWindow {

  public:

    OglWin( const char * name, 
            ArgList args = NULL,
            Cardinal argCount = 0 );
    ~OglWin();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: OglWin public


    //---- End editable code block: OglWin public


  protected:




    // Widgets created by this class

    Widget  _form27;
    Widget  _frame6;
    Widget  _glwidget1;
    Widget  _textfield2;
    Widget  _textfield3;

    // Menu items created by this class

    //---- Start editable code block: OglWin protected


    //---- End editable code block: OglWin protected


  private:


    static String  _defaultOglWinResources[];


    //---- Start editable code block: OglWin private


    //---- End editable code block: OglWin private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
