
//////////////////////////////////////////////////////////////
//
// Header file for RpFileSelectWindowClass
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
#ifndef RPFILESELECTWINDOWCLASS_H
#define RPFILESELECTWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "RpFileSelectionForm.h"
//---- End editable code block: headers and declarations


//---- RpFileSelectWindowClass class declaration

class RpFileSelectWindowClass: public VkSimpleWindow {

  public:

    RpFileSelectWindowClass( const char * name, 
                             ArgList args = NULL,
                             Cardinal argCount = 0 );
    ~RpFileSelectWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: RpFileSelectWindowClass public
    
    // CallbackMethod must be void method(char *newfilename)
    virtual void SetLabel(char *NewLabel);
    virtual void SetTitle(char *NewTitle);
    virtual void SetText(char *NewText);
    virtual void SetDir(char *NewDir);
    virtual void SetFilter(char *NewFilterStr);
    virtual void LoadDirList();
    virtual char *GetCurrentDirStr();
    virtual char *GetCurrentTextStr();
    virtual RpFileSelectionForm *GetrpFileSelectionForm() {
	return _rpFileSelectionForm;
	}
    
    //---- End editable code block: RpFileSelectWindowClass public


  protected:



    // Classes created by this class

    class RpFileSelectionForm *_rpFileSelectionForm;


    // Widgets created by this class



    //---- Start editable code block: RpFileSelectWindowClass protected


    //---- End editable code block: RpFileSelectWindowClass protected


  private:


    static String  _defaultRpFileSelectWindowClassResources[];


    //---- Start editable code block: RpFileSelectWindowClass private


    //---- End editable code block: RpFileSelectWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
