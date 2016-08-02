
//////////////////////////////////////////////////////////////
//
// Header file for HistogramWindowClass
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
#ifndef HISTOGRAMWINDOWCLASS_H
#define HISTOGRAMWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include "histogram.h"

//---- End editable code block: headers and declarations


//---- HistogramWindowClass class declaration

class HistogramWindowClass: public VkSimpleWindow {

  public:

    HistogramWindowClass( const char * name, 
                          ArgList args = NULL,
                          Cardinal argCount = 0 );
    ~HistogramWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: HistogramWindowClass public

	virtual void drawHistogram(histogramClass *histogram = 0, 
		unsigned char *cmap = 0, 
		int *histeqtable = 0);
	virtual void drawPalette(int cmap_size = 0, 
		unsigned char *cmap = 0);

	//---- End editable code block: HistogramWindowClass public


  protected:



    // Classes created by this class

    class HistogramFormClass *_histogramFormClass2;


    // Widgets created by this class



    //---- Start editable code block: HistogramWindowClass protected

    //---- End editable code block: HistogramWindowClass protected


  private:


    static String  _defaultHistogramWindowClassResources[];


    //---- Start editable code block: HistogramWindowClass private


    //---- End editable code block: HistogramWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
