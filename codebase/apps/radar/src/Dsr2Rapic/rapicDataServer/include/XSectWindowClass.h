
//////////////////////////////////////////////////////////////
//
// Header file for XSectWindowClass
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
#ifndef XSECTWINDOWCLASS_H
#define XSECTWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


//---- Start editable code block: headers and declarations

#include <GL/glx.h>

//---- End editable code block: headers and declarations


//---- XSectWindowClass class declaration

class XSectWindowClass: public VkSimpleWindow {

  public:

    XSectWindowClass( const char * name, 
                      ArgList args = NULL,
                      Cardinal argCount = 0 );
    ~XSectWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //---- Start editable code block: XSectWindowClass public

    virtual void drawSection(unsigned char *sectionBuff = 0, int numElems = 0, 
	    unsigned char *cmap = 0);
//    static const char *const histMouseButtonMoveCallback;
//    virtual int scaledHorizMPos();	// scaled mouse position, 0 - palette size
//    virtual void writeLabel(int val = 0);

    //---- End editable code block: XSectWindowClass public


  protected:




    // Widgets created by this class

    Widget  _xSectGlWid;


    // Member functions called from callbacks

    virtual void glWidExposed ( Widget, XtPointer );
    virtual void glWidGinit ( Widget, XtPointer );
    virtual void glWidInput ( Widget, XtPointer );
    virtual void winResized ( Widget, XtPointer );


    //---- Start editable code block: XSectWindowClass protected

    int		    elems;
    Window	    section_window;
    Display*	    section_display;
    GLXContext	    section_context;
    long	    section_w, section_h;
    unsigned char   *Cmap;
    int		    Cmap_size;
    unsigned char   *section_array;
    int		    section_array_size;
    
    //---- End editable code block: XSectWindowClass protected


  private:


    // Callbacks to interface with Motif

    static void glWidExposedCallback ( Widget, XtPointer, XtPointer );
    static void glWidGinitCallback ( Widget, XtPointer, XtPointer );
    static void glWidInputCallback ( Widget, XtPointer, XtPointer );
    static void winResizedCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultXSectWindowClassResources[];


    //---- Start editable code block: XSectWindowClass private


    //---- End editable code block: XSectWindowClass private


};
//---- Start editable code block: End of generated code


//---- End editable code block: End of generated code

#endif
