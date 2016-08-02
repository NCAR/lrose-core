#ifndef stormrec__h
#define stormrec__h

#define NODATA -1
#define MAXSTORMCOUNT 100 

#define PASTSTORM    1
#define CURRENTSTORM 2
#define FUTURESTORM  3
#define PASTFORECASTSTORM    4
#define CURRFORECASTSTORM    5

struct stormRec 
{
  int    type;
  int    color;
  char   number[40];
  long   xtime;
  long   ytime;
  long   stime;
  double top;
  double base;
  double volume;
  double mass;
  double speed;
  double direct;
  double maxdbz;
  double meandbz; 
  double height;
  double reflx;
  double refly;
  double range;
  double theta;
  double rlat;
  double rlon;

};

struct messageRec {
int token;
char buffer[sizeof(struct stormRec) + 1];
};


#endif

