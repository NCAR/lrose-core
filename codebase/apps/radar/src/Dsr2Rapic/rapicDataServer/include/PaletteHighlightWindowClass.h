
//////////////////////////////////////////////////////////////
//
// Header file for PaletteHighlightWindowClass
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
#ifndef PALETTEHIGHLIGHTWINDOWCLASS_H
#define PALETTEHIGHLIGHTWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "PaletteHighlightFormClass.h"

//---- End editable code block: headers and declarations


//---- PaletteHighlightWindowClass class declaration

class PaletteHighlightWindowClass: public VkSimpleWindow {

  public:

    PaletteHighlightWindowClass( const char * name, 
                                 ArgList args = NULL,
                                 Cardinal argCount = 0 );
    ~PaletteHighlightWindowClass();
    const char *className();
    virtual Boolean okToQuit();
	
    //---- Start editable code block: PaletteHighlightWindowClass public

	void		palHighlightFormChanged(VkCallbackObject *obj,
                            void *clientData,
                            void *callData);

	static const char *const palHighlightChangedCallback;
	virtual void drawHistogram(histogramClass *histogram = 0, 
		unsigned char *cmap = 0, 
		int *histeqtable = 0);
	virtual void setValues(palHighlightDataClass *highlightvals = 0);
	virtual void getValues(palHighlightDataClass *highlightvals = 0);
		
    //---- End editable code block: PaletteHighlightWindowClass public


  protected:



    // Classes created by this class

    class PaletteHighlightFormClass *_paletteHighlightFormClassUI;


    // Widgets created by this class



    //---- Start editable code block: PaletteHighlightWindowClass protected

	
    //---- End editable code block: PaletteHighlightWindowClass protected


  private:


    static String  _defaultPaletteHighlightWindowClassResources[];


    //---- Start editable code block: PaletteHighlightWindowClass private


    //---- End editable code block: PaletteHighlightWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
