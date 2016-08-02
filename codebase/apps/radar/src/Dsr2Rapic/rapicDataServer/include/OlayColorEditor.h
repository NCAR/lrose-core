
//////////////////////////////////////////////////////////////
//
// Header file for OlayColorEditor
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
#ifndef OLAYCOLOREDITOR_H
#define OLAYCOLOREDITOR_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- Start editable code block: headers and declarations


//---- End editable code block: headers and declarations


//---- OlayColorEditor class declaration

class OlayColorEditor: public VkSimpleWindow {

  public:

    OlayColorEditor( const char * name, 
                     ArgList args = NULL,
                     Cardinal argCount = 0 );
    ~OlayColorEditor();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: OlayColorEditor public


    //---- End editable code block: OlayColorEditor public


  protected:



    // Classes created by this class

    class ColorEditFormClass *_colorEditFormClassUI3;


    // Widgets created by this class

    Widget  _form32;

    VkOptionMenu  *_optionMenu2;
    VkOptionMenu  *_optionMenu3;

    VkMenuItem *_button70;
    VkMenuItem *_button71;
    VkMenuItem *_button72;
    VkMenuItem *_button73;
    VkMenuItem *_optionA4;
    VkMenuItem *_optionA5;
    VkMenuItem *_optionB4;
    VkMenuItem *_optionB5;


    // Member functions called from callbacks

    virtual void newLineWid ( Widget, XtPointer );
    virtual void newOlaySel ( Widget, XtPointer );


    //---- Start editable code block: OlayColorEditor protected


    //---- End editable code block: OlayColorEditor protected


  private:


    // Callbacks to interface with Motif

    static void newLineWidCallback ( Widget, XtPointer, XtPointer );
    static void newOlaySelCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultOlayColorEditorResources[];


    //---- Start editable code block: OlayColorEditor private


    //---- End editable code block: OlayColorEditor private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
