
//////////////////////////////////////////////////////////////
//
// Header file for BlendFnSimpleWindowClass
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
#ifndef BLENDFNSIMPLEWINDOWCLASS_H
#define BLENDFNSIMPLEWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

class CBlend;

//---- End editable code block: headers and declarations


//---- BlendFnSimpleWindowClass class declaration

class BlendFnSimpleWindowClass: public VkSimpleWindow {

  public:

    BlendFnSimpleWindowClass( const char * name, 
                              ArgList args = NULL,
                              Cardinal argCount = 0 );
    ~BlendFnSimpleWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: BlendFnSimpleWindowClass public

	void Set(CBlend *blend);
	void Get(CBlend *blend);
	static const char *const blendChangedCallback;
	static const char *const blendApplyImgCallback;
	static const char *const blendApplySeqCallback;
	void	blendChanged(VkCallbackObject *, void *, void *);
	void	blendApplyImg(VkCallbackObject *, void *, void *);
	void	blendApplySeq(VkCallbackObject *, void *, void *);
	

    //---- End editable code block: BlendFnSimpleWindowClass public


  protected:



    // Classes created by this class

    class BlendFunctionFormClass *_blendFunctionFormClassUI;


    // Widgets created by this class

    Widget  _button87;
    Widget  _button89;
    Widget  _button90;
    Widget  _form38;
    Widget  _separator23;


    // Member functions called from callbacks

    virtual void applyImgActivated ( Widget, XtPointer );
    virtual void applySeqActivated ( Widget, XtPointer );
    virtual void closeActivated ( Widget, XtPointer );


    //---- Start editable code block: BlendFnSimpleWindowClass protected


    //---- End editable code block: BlendFnSimpleWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void applyImgActivatedCallback ( Widget, XtPointer, XtPointer );
    static void applySeqActivatedCallback ( Widget, XtPointer, XtPointer );
    static void closeActivatedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultBlendFnSimpleWindowClassResources[];


    //---- Start editable code block: BlendFnSimpleWindowClass private


    //---- End editable code block: BlendFnSimpleWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
