
//////////////////////////////////////////////////////////////
//
// Header file for AnnotateWindowClass
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
#ifndef ANNOTATEWINDOWCLASS_H
#define ANNOTATEWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "draw.h"

enum annotateMode {thisWinThisImage, thisWinAllImage, 
    allWinThisImage, allWinAllImage};


//---- End editable code block: headers and declarations


//---- AnnotateWindowClass class declaration

class AnnotateWindowClass: public VkSimpleWindow {

  public:

    AnnotateWindowClass( const char * name, 
                         ArgList args = NULL,
                         Cardinal argCount = 0 );
    ~AnnotateWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: AnnotateWindowClass public

    char latLongString[128];
    DrawDataSymText *thisTextData;
    float posx, posy;
    static const char *const annotateAppliedCallback;
    
    virtual void setPosXY(float mposx, float mposy);

    // if textdata not defined, and thisTextData notdefined, create a new one for get
    virtual DrawDataSymText* getDrawText(DrawDataSymText *textdata = NULL);

    // allows edit of existing drawData
    virtual void setDrawText(DrawDataSymText *textdata); 
    
    virtual void MoveWin(long NewX, long NewY);
    
    //---- End editable code block: AnnotateWindowClass public


  protected:




    // Widgets created by this class

    Widget  _annotateTextField;
    Widget  _button96;
    Widget  _button97;
    Widget  _fontRotateScale;
    Widget  _fontSizeScale;
    Widget  _form37;
    Widget  _latLongLabel1;


    // Member functions called from callbacks

    virtual void annotateTextApplied ( Widget, XtPointer );
    virtual void annotateTextChanged ( Widget, XtPointer );
    virtual void closeActivated ( Widget, XtPointer );
    virtual void fontSizeChanged ( Widget, XtPointer );
    virtual void okActivated ( Widget, XtPointer );


    //---- Start editable code block: AnnotateWindowClass protected


    //---- End editable code block: AnnotateWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void annotateTextAppliedCallback ( Widget, XtPointer, XtPointer );
    static void annotateTextChangedCallback ( Widget, XtPointer, XtPointer );
    static void closeActivatedCallback ( Widget, XtPointer, XtPointer );
    static void fontSizeChangedCallback ( Widget, XtPointer, XtPointer );
    static void okActivatedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultAnnotateWindowClassResources[];


    //---- Start editable code block: AnnotateWindowClass private


    //---- End editable code block: AnnotateWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
