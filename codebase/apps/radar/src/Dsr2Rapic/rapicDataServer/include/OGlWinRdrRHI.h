
//////////////////////////////////////////////////////////////
//
// Header file for OGlWinRdrRHI
//
//    This class is a subclass of VkWindow
//
// Normally, very little in this file should need to be changed.
// Create/add/modify menus using RapidApp.
//
// Try to restrict any changes to adding members below the
// "//---- End generated code section" markers
// Doing so will allow you to make changes using RapidApp
// without losing any changes you may have made manually
//
//////////////////////////////////////////////////////////////
#ifndef OGLWINRDRRHI_H
#define OGLWINRDRRHI_H
#include <Vk/VkWindow.h>


class VkMenuItem;
class VkMenuToggle;
class VkMenuConfirmFirstAction;
class VkSubMenu;
class VkRadioSubMenu;

//---- End generated headers


//---- OGlWinRdrRHI class declaration

class OGlWinRdrRHI: public VkWindow {

  public:

    OGlWinRdrRHI( const char * name, 
                  ArgList args = NULL,
                  Cardinal argCount = 0 );
    ~OGlWinRdrRHI();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section



  protected:

    void afterRealizeHook();




    // Widgets created by this class

    Widget  _winRdrRHIDataFrame;
    Widget  _winRdrRHIDataLabel;
    Widget  _winRdrRHIForm;
    Widget  _winRdrRHIFrame;
    Widget  _winRdrRHI_glwidget;

    // Menu items created by this class
    VkSubMenu  *_menuPane7;
    VkMenuItem *_entry18;
    VkMenuItem *_entry19;
    VkMenuItem *_entry20;

    // Member functions called from callbacks

    virtual void OGLWinExposed ( Widget, XtPointer );
    virtual void OGLWinGInit ( Widget, XtPointer );
    virtual void OGLWinResized ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void OGLWinExposedCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinGInitCallback ( Widget, XtPointer, XtPointer );
    static void OGLWinResizedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultOGlWinRdrRHIResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
