
//////////////////////////////////////////////////////////////
//
// Header file for AccumProdWindowClass
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
#ifndef ACCUMPRODWINDOWCLASS_H
#define ACCUMPRODWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#ifdef __ACCUM_H	    // this is being called by rain_acc.h
class accum_product;	    // forward declare accum_product
#endif

#include "rain_acc.h"

//---- End editable code block: headers and declarations

//---- AccumProdWindowClass class declaration

class AccumProdWindowClass: public VkSimpleWindow {

  public:

    AccumProdWindowClass( const char * name, 
                          ArgList args = NULL,
                          Cardinal argCount = 0 );
    ~AccumProdWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: AccumProdWindowClass public

    /*
     * SetProd initialises AccumProdFormClass
     */
    void	    SetProd(accum_product *setprod);

    /*
     *	UpdateProd copies the contents of AccumProdFormClass's
     *	LocalAccumProduct to getprod
     *	or EditAccumProduct if getprod not defined
     *	EditAccumProduct is a pointer to the original SetProd class
     */
    void	    ReturnProd(accum_product *retprod = 0);
    accum_product   *EditAccumProduct;
    
    

    //---- End editable code block: AccumProdWindowClass public


  protected:



    // Classes created by this class

    class AccumProdFormClass *_accumProdFormClassUI;


    // Widgets created by this class

    Widget  _button52;
    Widget  _button53;
    Widget  _form26;


    // Member functions called from callbacks

    virtual void Cancel ( Widget, XtPointer );
    virtual void OK ( Widget, XtPointer );


    //---- Start editable code block: AccumProdWindowClass protected


    //---- End editable code block: AccumProdWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void CancelCallback ( Widget, XtPointer, XtPointer );
    static void OKCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultAccumProdWindowClassResources[];


    //---- Start editable code block: AccumProdWindowClass private


    //---- End editable code block: AccumProdWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
