
//////////////////////////////////////////////////////////////
//
// Header file for AccumMngWindowClass
//
//    This class is a subclass of VkSimpleWindow
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
#ifndef ACCUMMNGWINDOWCLASS_H
#define ACCUMMNGWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#ifdef __ACCUM_H	    // this is being called by rain_acc.h
class accum_mngr;	    // forward declare accum_mngr
#endif

#include "rain_acc.h"

//---- End editable code block: headers and declarations


//---- AccumMngWindowClass class declaration

class AccumMngWindowClass: public VkSimpleWindow {

  public:

    AccumMngWindowClass( const char * name, 
                         ArgList args = NULL,
                         Cardinal argCount = 0 );
    ~AccumMngWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section

    void	UpdateProductList();
    void	SetMng(accum_mngr *accmng);
    accum_mngr	*AccumManager;
    

    //---- End editable code block: AccumMngWindowClass public


  protected:



    // Classes created by this class

    class AccumMngFormClass *_accumMngFormClassUI;


    // Widgets created by this class

    Widget  _button47;
    Widget  _button48;
    Widget  _button50;
    Widget  _form35;


    // Member functions called from callbacks

    virtual void uifClose ( Widget, XtPointer );
    virtual void uifReadFile ( Widget, XtPointer );
    virtual void uifWriteFile ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void uifCloseCallback ( Widget, XtPointer, XtPointer );
    static void uifReadFileCallback ( Widget, XtPointer, XtPointer );
    static void uifWriteFileCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultAccumMngWindowClassResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
