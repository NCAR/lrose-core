
//////////////////////////////////////////////////////////////
//
// Header file for ManNavWindowClass
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
#ifndef MANNAVWINDOWCLASS_H
#define MANNAVWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- Start editable code block: headers and declarations

enum manNavChangeType {Dragged, ValChanged};	// types of callback events

//---- End editable code block: headers and declarations


//---- ManNavWindowClass class declaration

class ManNavWindowClass: public VkSimpleWindow {

  public:

    ManNavWindowClass( const char * name, 
                       ArgList args = NULL,
                       Cardinal argCount = 0 );
    ~ManNavWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: ManNavWindowClass public

	void setValues(int elem_ofs, int line_ofs, 
		unsigned char applyThis	= 1, unsigned char applyAll = 0);
	void getValues(int *elem_ofs, int *line_ofs, 
		unsigned char *applyThis, unsigned char *applyAll);
	static const char *const manNavChangedCallback;

    //---- End editable code block: ManNavWindowClass public


  protected:




    // Widgets created by this class

    Widget  _clearButton;
    Widget  _elemOfsScale;
    Widget  _elemOfsTextField;
    Widget  _form29;
    Widget  _label61;
    Widget  _lineOfsScale;
    Widget  _lineOfsTextField;
    Widget  _manNavCloseButton;
    Widget  _toggle16;
    Widget  _toggle17;

    VkOptionMenu  *_optionMenu4;

    VkMenuItem *_button66;
    VkMenuItem *_button74;
    VkMenuItem *_button75;
    VkMenuItem *_button76;
    VkMenuItem *_optionA6;
    VkMenuItem *_optionB6;


    // Member functions called from callbacks

    virtual void applyAllWInChanged ( Widget, XtPointer );
    virtual void applyThisWinChanged ( Widget, XtPointer );
    virtual void clearOffsets ( Widget, XtPointer );
    virtual void elemOffsetChanged ( Widget, XtPointer );
    virtual void elemOffsetDragged ( Widget, XtPointer );
    virtual void elemOfsTextActivated ( Widget, XtPointer );
    virtual void elemOfsTextChanged ( Widget, XtPointer );
    virtual void lineOffsetChanged ( Widget, XtPointer );
    virtual void lineOffsetDragged ( Widget, XtPointer );
    virtual void lineOfsTextActivated ( Widget, XtPointer );
    virtual void lineOfsTextChanged ( Widget, XtPointer );
    virtual void newOffsetScale ( Widget, XtPointer );
    virtual void winClosed ( Widget, XtPointer );


    //---- Start editable code block: ManNavWindowClass protected


    //---- End editable code block: ManNavWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void applyAllWInChangedCallback ( Widget, XtPointer, XtPointer );
    static void applyThisWinChangedCallback ( Widget, XtPointer, XtPointer );
    static void clearOffsetsCallback ( Widget, XtPointer, XtPointer );
    static void elemOffsetChangedCallback ( Widget, XtPointer, XtPointer );
    static void elemOffsetDraggedCallback ( Widget, XtPointer, XtPointer );
    static void elemOfsTextActivatedCallback ( Widget, XtPointer, XtPointer );
    static void elemOfsTextChangedCallback ( Widget, XtPointer, XtPointer );
    static void lineOffsetChangedCallback ( Widget, XtPointer, XtPointer );
    static void lineOffsetDraggedCallback ( Widget, XtPointer, XtPointer );
    static void lineOfsTextActivatedCallback ( Widget, XtPointer, XtPointer );
    static void lineOfsTextChangedCallback ( Widget, XtPointer, XtPointer );
    static void newOffsetScaleCallback ( Widget, XtPointer, XtPointer );
    static void winClosedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultManNavWindowClassResources[];


    //---- Start editable code block: ManNavWindowClass private


    //---- End editable code block: ManNavWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
