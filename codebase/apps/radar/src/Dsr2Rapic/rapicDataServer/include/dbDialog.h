
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include "stormrec.h"


typedef struct {
  char     lines[MAXSTORMCOUNT+1][256];
  int      numLines;
} textData;
class dbDialog {

 public:
  
   dbDialog (int len,Widget mainP);
  ~dbDialog (void );
   virtual const char * className();

          int    asked_exit();
   static void   dispCallback(Widget,XtPointer,XtPointer);
   static void   dataCallback(Widget,XtPointer,XtPointer);
   static void   hdataCallback(Widget,XtPointer,XtPointer);
   static void   printCallback(Widget,XtPointer,XtPointer);
   static void   quitCallback(Widget,XtPointer,XtPointer);
   static void   nextCallback(Widget,XtPointer,XtPointer);
   void          dataChanged(int len);
   virtual void    reSize();
   void         setSizes(Dimension wid,Dimension hit);
   void         alterSize(Dimension wid,Dimension hit);
   int          isNextSet() { return nextset; }
   void         setNextSet(int val) { nextset = val; }

 protected:

   Boolean dbDialog::okToQuit();
   void    chkDate(void );
   void    reDraw();
   void    hreDraw();
 private:
   void         drawBorder();
   Widget	panel;
   Widget       workArea;
   Widget       vScrollBar;
   Widget       hScrollBar;
   Widget       next;
   Widget	print;
   Widget	quit;
   Widget       junkText;
   void         quitting();
   int		quitme;
   Widget       ptr;
   textData     textLines;
   int          startIndex;
   int          endIndex;
   int          startHpos;
   int          lenHeadStr;
   Dimension    workAWidth,workAHeight;
   Dimension    cWid,cHit;
   Dimension    effWidth,effHeight;
   int          nextset;
 };

