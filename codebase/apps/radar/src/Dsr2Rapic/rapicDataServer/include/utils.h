#ifndef __UTILS_H
#define __UTILS_H



/*

	utils.h
	
	General purpose utilities 

*/

#include <sys/select.h> //SD add 21/12/99
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define MUSTBEWRITABLE 1
#define DELETEFLAGFILE 1

struct strnode {

public:

	strnode *next,*prev;
	char *str;
	strnode(char *newstr);
	~strnode();
	};

class strlist {
  strnode *head,*tail,*This,*temp;

public:
  void addstr(char *newstr);        // add the given string to this list
  char* match(char *strtomatch); // do a strstr with each str in list
  void reset();
  char* this_str();               // return this string(0 if none)
  strnode* step();
  strlist(); 
  ~strlist(); 
  };

void  Check_mallinfo();
void  Print_mallinfo(FILE *file = stderr);
void printVmSize(FILE *file = stderr);
void DumpMemoryStatus(FILE *file = stderr, bool closing = false);
void AppendMemoryStatus();
void AppendMemoryStatus(char *fname);
void AppendMemoryStatus(char *fname, int maxsize);
void CloseMemoryStatus();
void CloseMemoryStatus(char *fname);
void AppendMemoryStatus(char *fname, int maxsize, bool closing);

void RingBells(char *message);

char *TimeString(time_t time, char *outstr, 
		 bool showzone, bool force_gmtime = false);

char *ShortTimeString(time_t time, char *outstr, 
		      bool force_gmtime = false,
		      bool localtime = false);

char *ShortDateString(time_t time, char *outstr, 
		      bool force_gmtime = false,
		      bool localtime = false);

int FileExists(char *fname, 
	    int mustbewritable = 0, 
	    int deleteflagfile = 0);
int FileExists2(char *pathname, 
	       char *fname, 
	    int mustbewritable = 0, 
	    int deleteflagfile = 0);
int FileExists(const char *fname, 
	    int mustbewritable = 0, 
	    int deleteflagfile = 0);
int DirExists(char *fname, 
	      int mustbewritable = 0, 
	      int createdir = 0,
	      mode_t mode = 0755);
int CreateDir(char *fname, 
	      mode_t mode = 0755);

/*
 * return free bytes,  -1 if error
 */
double DiskFree(char *pathname);
double DiskFree(int fildes);

// if no vals passed, reset to 0;
inline void timeval_set(timeval &val, 
		 long tvsecs = 0,
		 long tvusecs = 0)
{
  val.tv_sec = tvsecs;
  val.tv_usec = tvusecs;
};

inline void timeval_set(timeval *val, 
		 long tvsecs = 0,
		 long tvusecs = 0)
{
  val->tv_sec = tvsecs;
  val->tv_usec = tvusecs;
};

bool timeval_gte(timeval time1, timeval time2); // compare times from gettimeofday call
bool timeval_gt(timeval time1, timeval time2);  // compare times from gettimeofday call
bool timeval_lte(timeval time1, timeval time2); // compare times from gettimeofday call
bool timeval_lt(timeval time1, timeval time2);  // compare times from gettimeofday call
double timeval_diff(timeval time1, timeval time2);// return time difference between times as double

bool timenow_gte_time(timeval reftime);         // compare reftime to current time
bool timenow_gt_time(timeval reftime);          // compare reftime to current time
bool timenow_lte_time(timeval reftime);         // compare reftime to current time
bool timenow_lt_time(timeval reftime);          // compare reftime to current time
double timenow_diff(timeval reftime);          // return time diff btwn now & ref time as double

void time_plus_secs(timeval *TM, double addsecs);  // add seconds to passed timeval
void timenow_plus_secs(timeval *TM, double addsecs); // gettimeofday and add seconds, return new TM

int argFound(int argc, char **argv, char *argstr); // return arg number if found, else -1

// scaled value string scaled to k(ilo) M(ega) G(iga) T(era)
// field size is 4+suffix, i.e. up to 9999
char* scaled_kMGTSizeString(long sizein, char*strout);

// return "true" or "false" string
char* boolStr(bool state);

inline int max(int int1, int int2) {
  int _max = (int1 > int2)?int1:int2;
  return _max;
};

inline int min(int int1, int int2) {
  int _min = (int1 < int2)?int1:int2;
  return _min;
};

inline float max(float float1, float float2) {
  float _max = (float1 > float2)?float1:float2;
  return _max;
};

inline float min(float float1, float float2) {
  float _min = (float1 < float2)?float1:float2;
  return _min;
};

inline float clip(float val, float lower, float upper) {
  if (val < lower)
    return lower;
  else if (val > upper)
    return upper;
  else
    return val;
};

inline int clip(int val, int lower, int upper) {
  if (val < lower)
    return lower;
  else if (val > upper)
    return upper;
  else
    return val;
};

inline float clip_0_1(float val) {
  if (val < 0.0)
    return 0.0;
  else if (val > 1.0)
    return 1.0;
  else
    return val;
};

inline float lerp(float val1, float val2, float lerpVal) {
  lerpVal = clip(lerpVal, 0.0, 1.0);
  return val1 + (lerpVal * (val2 - val1));
};


#endif /* __UTILS_H */
