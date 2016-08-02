
//////////////////////////////////////////////////////////////
//
// Header file for ColorEditWindowClass
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
#ifndef COLOREDITWINDOWCLASS_H
#define COLOREDITWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations


//---- End editable code block: headers and declarations


//---- ColorEditWindowClass class declaration

class ColorEditWindowClass: public VkSimpleWindow {

  public:

    ColorEditWindowClass( const char * name, 
                          ArgList args = NULL,
                          Cardinal argCount = 0 );
    ~ColorEditWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: ColorEditWindowClass public

    void		colorEditFormChanged(VkCallbackObject *obj,
			void *clientData,
			void *callData);

    static const char *const colorChangedCallback;
    static const char *const colorAppliedCallback;

    virtual void	setRGB(float r, float g, float b);
    virtual void	setRGB(unsigned char r, unsigned char g, unsigned char b);
    virtual void	setDirectModifyRGB(float *modifyRGB);
    virtual void	getRGB(float *r, float *g, float *b);
    virtual void	getRGB(unsigned char *r, unsigned char *g, unsigned char *b);
    

    //---- End editable code block: ColorEditWindowClass public


  protected:



    // Classes created by this class

    class ColorEditFormClass *_colorEditFormClassUI2;


    // Widgets created by this class

    Widget  _closeButton2;
    Widget  _closeButton3;
    Widget  _form30;


    // Member functions called from callbacks

    virtual void colorEditApplied ( Widget, XtPointer );
    virtual void colorEditorClosed ( Widget, XtPointer );


    //---- Start editable code block: ColorEditWindowClass protected


    //---- End editable code block: ColorEditWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void colorEditAppliedCallback ( Widget, XtPointer, XtPointer );
    static void colorEditorClosedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultColorEditWindowClassResources[];


    //---- Start editable code block: ColorEditWindowClass private


    //---- End editable code block: ColorEditWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
