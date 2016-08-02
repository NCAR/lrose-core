#include <iostream.h>
#define MAXDATELEN 20
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/DialogS.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "dbDialog.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/select.h>  //SD add 21/12/99
#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif
extern int quitDisplay;
extern int checkForData(int realTime);
#ifdef __cplusplus
        }
#endif
#define NAME "statuswindow"


class dbWindow {

    public:

      dbWindow(int len);
      ~dbWindow();
      int  run(void);
      void generateEvent();
    private:

      void          exitDates(char *msg);
      XtAppContext  appCon;
      Widget        parentWin;
      dbDialog     *dateWin;
      Widget        app_shell;
};



