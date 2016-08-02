#ifndef _titanTime_h
#define _titanTime_h

/******************************************************************************
 ** titanTime Class                                                          **
 **                                                                          **
 ** Responsibility:                                                          **
 **                                                                          **
 ** Responsible for  displaying date/time				     **
 **
 ** If the date/and time are zero OR CURRENT is activated time is taken      **
 ** from the system.
 ** Uses scale widgets to set the date and time.
 **                     
 *****************************************************************************/

#define MINYEAR 1980
#define MAXYEAR 2010

#define YEARID  1
#define MONTHID 2
#define DAYID   3
#define HOURID  4
#define MINID   5
#define SECID   6
#ifdef MAIN
struct timeLimits {
	char  *month;
	int   maxdate;
} timelimits[] =
{
	{"Jan "	,31},
	{"Feb "	,29},
	{"Mar "	,31},
	{"Apr "	,30},
	{"May "	,31},
	{"Jun "	,30},
	{"Jul "	,31},
	{"Aug "	,31},
	{"sep "	,30},
	{"Oct "	,31},
	{"Nov "	,30},
	{"Dec "	,31},
	{""	,0 }
};
#endif
#ifndef MAIN
extern struct timeLimits {
	char  *month;
	int   maxdate;
} timelimits[];
#endif

struct realTime {
int year;
int month;
int day;
int hour;
int min;
int sec;
};
 
#ifndef RES_CONVERT 
#define	RES_CONVERT( res_name, res_value) \
	XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1

#endif
 

class titanTime {
  public:
         titanTime(void );
         ~titanTime(void) ;
         void initOptions (Widget parent,
	              Widget shellW,
	              int pbx,
		      int pby,
		      char *pbLabel,
		      char *initVal,
		      char *context,
		      void (*funcPtr)(char *sele,void *thisPtr) = NULL,
		      void *thisPtr = NULL);
		      
	 void getResult(struct realTime *outVal);
	 void refresh(char *cptr);
	 void toggleTime(int flag);
  private:
          void update();
          void displayTime(struct realTime *timeP,Position sx1,Position sy1);
	  static void   pushButtonCB(Widget,XtPointer,XtPointer);
	  static void   quitCB(Widget,XtPointer,XtPointer);
	  void   quitCallback(Widget w);
	  void   pushButtonCallback(Widget w);
          int isleapYear();
	  Widget quitW;
	  Widget appShell;
	  Widget transShell;
	  Widget labelW;
          Widget Parent;
	  Widget pushButtonW;
          Widget yearScale;
	  Widget monthScale;
	  Widget dayScale;
	  Widget hourScale;
	  Widget minScale;
	  Widget secScale;
	  int    sx,sy;
	  int notActive;
	  struct realTime timeData;
	  char *timesData;
	  static void   scaleCB(Widget,XtPointer,XtPointer);
	  void   scaleCallback(Widget w);
	  char   *Context;
	  void   *objectPtr;
	  void ((*selectionChangedCB)(char *sele,void *thisPtr));
};

#endif

