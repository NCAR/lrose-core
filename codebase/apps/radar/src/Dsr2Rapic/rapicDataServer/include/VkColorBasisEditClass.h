
//////////////////////////////////////////////////////////////
//
// Header file for VkColorBasisEditClass
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
#ifndef VKCOLORBASISEDITCLASS_H
#define VKCOLORBASISEDITCLASS_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- Start editable code block: headers and declarations


//---- End editable code block: headers and declarations


//---- VkColorBasisEditClass class declaration

class VkColorBasisEditClass: public VkWindow {

  public:

    VkColorBasisEditClass( const char * name, 
                           ArgList args = NULL,
                           Cardinal argCount = 0 );
    ~VkColorBasisEditClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: VkColorBasisEditClass public


    //---- End editable code block: VkColorBasisEditClass public


  protected:



    // Classes created by this class

    class ColorEditFormClass *_colorEditFormClass1;


    // Widgets created by this class

    Widget  _basisMap_glwidget;
    Widget  _form28;
    Widget  _frame2;
    Widget  _frame3;
    Widget  _frame4;
    Widget  _palValScale;
    Widget  _paletteBar_glwidget;
    Widget  _textfield4;

    // Menu items created by this class
    VkSubMenu  *_filePane;
    VkMenuItem *_newButton;
    VkMenuItem *_openButton;
    VkMenuItem *_saveButton;
    VkMenuItem *_saveasButton;
    VkMenuItem *_printButton;
    VkMenuItem *_separator14;
    VkMenuItem *_closeButton;
    VkMenuItem *_exitButton;
    VkSubMenu  *_editPane1;
    VkMenuItem *_theVkUndoManagerButton1;
    VkMenuItem *_cutButton1;
    VkMenuItem *_copyButton1;
    VkMenuItem *_pasteButton1;
    VkSubMenu  *_viewPane1;
    VkMenuItem *_viewControl1;
    VkMenuItem *_viewControl2;
    VkMenuItem *_viewControl3;
    VkSubMenu  *_optionsPane1;
    VkMenuToggle *_option11;
    VkMenuToggle *_option12;

    // Member functions called from callbacks

    virtual void close ( Widget, XtPointer );
    virtual void copy ( Widget, XtPointer );
    virtual void cut ( Widget, XtPointer );
    virtual void newFile ( Widget, XtPointer );
    virtual void newPalVal ( Widget, XtPointer );
    virtual void openFile ( Widget, XtPointer );
    virtual void paste ( Widget, XtPointer );
    virtual void print ( Widget, XtPointer );
    virtual void quit ( Widget, XtPointer );
    virtual void save ( Widget, XtPointer );
    virtual void saveas ( Widget, XtPointer );


    //---- Start editable code block: VkColorBasisEditClass protected


    //---- End editable code block: VkColorBasisEditClass protected


  private:


    // Callbacks to interface with Motif

    static void closeCallback ( Widget, XtPointer, XtPointer );
    static void copyCallback ( Widget, XtPointer, XtPointer );
    static void cutCallback ( Widget, XtPointer, XtPointer );
    static void newFileCallback ( Widget, XtPointer, XtPointer );
    static void newPalValCallback ( Widget, XtPointer, XtPointer );
    static void openFileCallback ( Widget, XtPointer, XtPointer );
    static void pasteCallback ( Widget, XtPointer, XtPointer );
    static void printCallback ( Widget, XtPointer, XtPointer );
    static void quitCallback ( Widget, XtPointer, XtPointer );
    static void saveCallback ( Widget, XtPointer, XtPointer );
    static void saveasCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultVkColorBasisEditClassResources[];


    //---- Start editable code block: VkColorBasisEditClass private


    //---- End editable code block: VkColorBasisEditClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
