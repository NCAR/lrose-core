
//////////////////////////////////////////////////////////////
//
// Header file for RainAccumSimpleWindowClass
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
#ifndef RAINACCUMSIMPLEWINDOWCLASS_H
#define RAINACCUMSIMPLEWINDOWCLASS_H
#include <Vk/VkSimpleWindow.h>


class VkOptionMenu;
class VkMenuItem;

//---- End generated headers


//---- RainAccumSimpleWindowClass class declaration

class RainAccumSimpleWindowClass: public VkSimpleWindow {

  public:

    RainAccumSimpleWindowClass( const char * name, 
                                ArgList args = NULL,
                                Cardinal argCount = 0 );
    ~RainAccumSimpleWindowClass();
    const char *className();
    virtual Boolean okToQuit();

    //--- End generated code section



  protected:




    // Widgets created by this class

    Widget  _accumPeriodTextField1;
    Widget  _cappiHeightTextField1;
    Widget  _form56;
    Widget  _label80;
    Widget  _label81;
    Widget  _label82;
    Widget  _label83;
    Widget  _rainRateClipTextField1;

    VkOptionMenu  *_accumPeriodOptionMenu1;
    VkOptionMenu  *_cappiHeightOptionMenu1;

    VkMenuItem *_option0metres1;
    VkMenuItem *_option1000metres1;
    VkMenuItem *_option12hour1;
    VkMenuItem *_option1500metres1;
    VkMenuItem *_option1hour1;
    VkMenuItem *_option24hour1;
    VkMenuItem *_option2hour1;
    VkMenuItem *_option3000metres1;
    VkMenuItem *_option30min1;
    VkMenuItem *_option3hour1;
    VkMenuItem *_option48hour1;
    VkMenuItem *_option5000metres1;
    VkMenuItem *_option500metres1;
    VkMenuItem *_option6hour1;
    VkMenuItem *_optionAccumPeriod1;
    VkMenuItem *_optionCAPPIHeight1;
    VkMenuItem *_separator49;
    VkMenuItem *_separator50;


    // Member functions called from callbacks

    virtual void uifAccPeriod12hr ( Widget, XtPointer );
    virtual void uifAccPeriod1hr ( Widget, XtPointer );
    virtual void uifAccPeriod24hr ( Widget, XtPointer );
    virtual void uifAccPeriod2hr ( Widget, XtPointer );
    virtual void uifAccPeriod30min ( Widget, XtPointer );
    virtual void uifAccPeriod3hr ( Widget, XtPointer );
    virtual void uifAccPeriod48hr ( Widget, XtPointer );
    virtual void uifAccPeriod6hr ( Widget, XtPointer );
    virtual void uifAccumPeriodText ( Widget, XtPointer );
    virtual void uifCAPPIHeightText ( Widget, XtPointer );
    virtual void uifCAPPIHt0 ( Widget, XtPointer );
    virtual void uifCAPPIHt1000 ( Widget, XtPointer );
    virtual void uifCAPPIHt1500 ( Widget, XtPointer );
    virtual void uifCAPPIHt3000 ( Widget, XtPointer );
    virtual void uifCAPPIHt500 ( Widget, XtPointer );
    virtual void uifCAPPIHt5000 ( Widget, XtPointer );
    virtual void uifRainRateClipText ( Widget, XtPointer );


    //--- End generated code section



  private:


    // Callbacks to interface with Motif

    static void uifAccPeriod12hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod1hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod24hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod2hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod30minCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod3hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod48hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccPeriod6hrCallback ( Widget, XtPointer, XtPointer );
    static void uifAccumPeriodTextCallback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHeightTextCallback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt0Callback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt1000Callback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt1500Callback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt3000Callback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt500Callback ( Widget, XtPointer, XtPointer );
    static void uifCAPPIHt5000Callback ( Widget, XtPointer, XtPointer );
    static void uifRainRateClipTextCallback ( Widget, XtPointer, XtPointer );

    static String  _defaultRainAccumSimpleWindowClassResources[];


    //--- End generated code section



};
//---- End of generated code

#endif
