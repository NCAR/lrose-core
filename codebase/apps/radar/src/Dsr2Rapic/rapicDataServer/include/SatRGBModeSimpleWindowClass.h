
//////////////////////////////////////////////////////////////
//
// Header file for SatRGBModeSimpleWindowClass
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
#ifndef SATRGBMODESIMPLEWINDOWCLASS_H
#define SATRGBMODESIMPLEWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- Start editable code block: headers and declarations

class CBlend;

//---- End editable code block: headers and declarations


//---- SatRGBModeSimpleWindowClass class declaration

class SatRGBModeSimpleWindowClass: public VkSimpleWindow {

  public:

    SatRGBModeSimpleWindowClass( const char * name, 
                                 ArgList args = NULL,
                                 Cardinal argCount = 0 );
    ~SatRGBModeSimpleWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: SatRGBModeSimpleWindowClass public

	void Set(CBlend *blend);
	void Get(CBlend *blend);
	static const char *const rgbChangedCallback;
	static const char *const rgbApplyImgCallback;
	static const char *const rgbApplySeqCallback;
	void	rgbChanged(VkCallbackObject *, void *, void *);

    //---- End editable code block: SatRGBModeSimpleWindowClass public


  protected:




    // Widgets created by this class

    Widget  _button85;
    Widget  _button86;
    Widget  _button91;
    Widget  _form39;
    Widget  _label65;
    Widget  _label66;
    Widget  _label67;
    Widget  _label68;
    Widget  _separator24;

    VkOptionMenu  *_blueChOptionMenu;
    VkOptionMenu  *_greenChOptionMenu;
    VkOptionMenu  *_redChOptionMenu;

    VkMenuItem *_button79;
    VkMenuItem *_button80;
    VkMenuItem *_button81;
    VkMenuItem *_button82;
    VkMenuItem *_button83;
    VkMenuItem *_button84;
    VkMenuItem *_optionA10;
    VkMenuItem *_optionA11;
    VkMenuItem *_optionA12;
    VkMenuItem *_optionB10;
    VkMenuItem *_optionB11;
    VkMenuItem *_optionB12;


    // Member functions called from callbacks

    virtual void applyImgActivated ( Widget, XtPointer );
    virtual void applySeqActivated ( Widget, XtPointer );
    virtual void blueChChanged ( Widget, XtPointer );
    virtual void closeActivated ( Widget, XtPointer );
    virtual void greenChChanged ( Widget, XtPointer );
    virtual void redChChanged ( Widget, XtPointer );


    //---- Start editable code block: SatRGBModeSimpleWindowClass protected


    //---- End editable code block: SatRGBModeSimpleWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void applyImgActivatedCallback ( Widget, XtPointer, XtPointer );
    static void applySeqActivatedCallback ( Widget, XtPointer, XtPointer );
    static void blueChChangedCallback ( Widget, XtPointer, XtPointer );
    static void closeActivatedCallback ( Widget, XtPointer, XtPointer );
    static void greenChChangedCallback ( Widget, XtPointer, XtPointer );
    static void redChChangedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultSatRGBModeSimpleWindowClassResources[];


    //---- Start editable code block: SatRGBModeSimpleWindowClass private


    //---- End editable code block: SatRGBModeSimpleWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
