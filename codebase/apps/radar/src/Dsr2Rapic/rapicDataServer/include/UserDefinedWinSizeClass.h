
//////////////////////////////////////////////////////////////
//
// Header file for UserDefinedWinSizeClass
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
#ifndef USERDEFINEDWINSIZECLASS_H
#define USERDEFINEDWINSIZECLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "bool.h"

//---- End editable code block: headers and declarations


//---- UserDefinedWinSizeClass class declaration

class UserDefinedWinSizeClass: public VkSimpleWindow {

  public:

    UserDefinedWinSizeClass( const char * name, 
                             ArgList args = NULL,
                             Cardinal argCount = 0 );
    ~UserDefinedWinSizeClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: UserDefinedWinSizeClass public

	static const char *const newSizeAppliedCallback;
	void setVals(long Wid, long Ht);
	bool getVals(long *wid, long *ht);

    //---- End editable code block: UserDefinedWinSizeClass public


  protected:




    // Widgets created by this class

    Widget  _button108;
    Widget  _button109;
    Widget  _form33;
    Widget  _label69;
    Widget  _label70;
    Widget  _label71;
    Widget  _xSizeTextField;
    Widget  _ySizeTextField;


    // Member functions called from callbacks

    virtual void XSizeChanged ( Widget, XtPointer );
    virtual void YSizeChanged ( Widget, XtPointer );
    virtual void newWinSizeCancelCallback ( Widget, XtPointer );
    virtual void newWinSizeOKCallback ( Widget, XtPointer );


    //---- Start editable code block: UserDefinedWinSizeClass protected


    //---- End editable code block: UserDefinedWinSizeClass protected


  private:


    // Callbacks to interface with Motif

    static void XSizeChangedCallback ( Widget, XtPointer, XtPointer );
    static void YSizeChangedCallback ( Widget, XtPointer, XtPointer );
    static void newWinSizeCancelCallbackCallback ( Widget, XtPointer, XtPointer );
    static void newWinSizeOKCallbackCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultUserDefinedWinSizeClassResources[];


    //---- Start editable code block: UserDefinedWinSizeClass private


    //---- End editable code block: UserDefinedWinSizeClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
