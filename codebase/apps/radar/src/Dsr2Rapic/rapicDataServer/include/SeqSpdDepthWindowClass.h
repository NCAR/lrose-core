
//////////////////////////////////////////////////////////////
//
// Header file for SeqSpdDepthWindowClass
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
#ifndef SEQSPDDEPTHWINDOWCLASS_H
#define SEQSPDDEPTHWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- End generated headers


//---- SeqSpdDepthWindowClass class declaration

class SeqSpdDepthWindowClass: public VkSimpleWindow {

  public:

    SeqSpdDepthWindowClass( const char * name, 
                            ArgList args = NULL,
                            Cardinal argCount = 0 );
    ~SeqSpdDepthWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section



  protected:



    // Classes created by this class

    class RpSeqSpdDepthFormClass *_rpSeqSpdDepthFormClassUI1;


    // Widgets created by this class

    Widget  _button111;
    Widget  _form48;


    // Member functions called from callbacks

    virtual void doClose ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void doCloseCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultSeqSpdDepthWindowClassResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
