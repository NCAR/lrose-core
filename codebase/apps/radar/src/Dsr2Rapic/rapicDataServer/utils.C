/*

utils.c++
implementation of general purpose utilities 
	
*/

#include <string.h>
#include "utils.h"
#include <limits.h>
#include "rdr.h"
#include "rdrscan.h"
#include "threadobj.h"
#include <time.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include <libgen.h>

strnode::strnode(char *newstr) {
  int len;
  next = prev = 0;
  len = strlen(newstr);
  str = new char[len+1];
  strcpy(str,newstr);
};

strnode::~strnode() {
  delete[] str;
  if (next) next->prev = prev;  // putting these in allows editing
  if (prev) prev->next = next;  // of existing list by deletion 
};

/*

strlist is a utility for the management of a list of strings for 
tesing against another string, developed for use with comms
services, e.g. x.28 & hayes interfaces.
	
*/

void strlist::addstr(char *newstr) {// add the given string to this list
  temp = new strnode(newstr);
  temp->prev = tail;
  if (tail) tail->next  = temp;	// link prev tail to new tail	
  tail = temp;									// tail is always the new str
  if (!head) head = temp;       // if new linked list head is new str
}
														 
char* strlist::match(char *strtomatch) {
  char *matchpos = 0;
  This = head;
  while ((This) && !(matchpos = strstr(strtomatch,This->str)))
    This = This->next;
  return matchpos;
}

void strlist::reset() {
  This = head;
}

char* strlist::this_str() {      // return this string(0 if none)
  if (This) return This->str;
  else return 0;
}

strnode* strlist::step() {        // step to next string in list
  if (This) This = This->next;
  return temp;
}

strlist::strlist() {
  head = tail = temp = This = 0;
}

strlist::~strlist() {
  temp = head;
  while (head) {
    temp = head->next;
    delete head;
    head = temp;
  }
}

void RingBells(char *message) {
  int repeat = 1;
  while (repeat--) {
    fprintf(stderr,"%s\n",message);
    for (int x = 0; x <2; x++) {
      fprintf(stderr,"\a");
      sec_delay(0.50);
    }
    sleep(1);
  }
}

char *TimeString(time_t time, char *outstr, 
		 bool showzone, bool force_gmtime) {
  char TimeStr[64];
  struct tm TmStruct;

  if (!outstr) return NULL;
  if (time == 0)
    {
      sprintf(outstr, "NULL Time");
      return outstr;
    }
  if (LocalTime && !force_gmtime) localtime_r(&time, &TmStruct);
  else gmtime_r(&time, &TmStruct);
#ifdef SUN
  asctime_r(&TmStruct, TimeStr, 64);
#else
  asctime_r(&TmStruct, TimeStr);
#endif
  if (showzone)
    {
      if (LocalTime && !force_gmtime && tzname[0])
	strcat(TimeStr, tzname[0]);
      else
	strcat(TimeStr, "UTC");
    }
  TimeStr[24] = 0;
  strncpy(outstr, TimeStr, 25);
  return outstr;
}

char *ShortTimeString(time_t time, char *outstr, 
		      bool force_gmtime,
		      bool localtime) {
  struct tm TmStruct;

  if (!outstr) return NULL;
  if ((LocalTime || localtime) && !force_gmtime) 
    localtime_r(&time, &TmStruct);
  else gmtime_r(&time, &TmStruct);
  sprintf(outstr, "%02d:%02d:%02d %02d/%02d/%04d",
	  TmStruct.tm_hour, TmStruct.tm_min, TmStruct.tm_sec,
	  TmStruct.tm_mday, TmStruct.tm_mon+1, TmStruct.tm_year+1900);
  return outstr;
}

char *ShortDateString(time_t time, char *outstr, 
		      bool force_gmtime,
		      bool localtime) {
  struct tm TmStruct;

  if (!outstr) return NULL;
  if ((LocalTime || localtime) && !force_gmtime) 
    localtime_r(&time, &TmStruct);
  else gmtime_r(&time, &TmStruct);
  sprintf(outstr, "%02d/%02d/%04d %02d:%02d:%02d",
	  TmStruct.tm_mday, TmStruct.tm_mon+1, TmStruct.tm_year+1900,
	  TmStruct.tm_hour, TmStruct.tm_min, TmStruct.tm_sec);
  return outstr;
}

int FileExists(char *fname, 
	       int mustbewritable, 
	       int deleteflagfile) {
  return FileExists2(NULL, fname, mustbewritable, deleteflagfile);
}

int FileExists2(char *pathname, 
	       char *fname, 
	       int mustbewritable, 
	       int deleteflagfile) {
  struct stat buf;
  int flag = 0;
  char tempstr[256] = "";  
  
  if (pathname)
    strcpy(tempstr, pathname);
  if (fname)
    strcat(tempstr, fname);
  else
    return false;
  flag = (stat(tempstr, &buf) == 0) &&	// if file exists
    S_ISREG(buf.st_mode);
  if (flag && mustbewritable) {	// check if writable
    flag &= access(tempstr, W_OK) == 0;
    if (!flag) 
      fprintf(stderr, "FileExists ERROR: %s exists but is not writeable\n", fname);
  }
  if (flag && deleteflagfile)
    unlink(tempstr);		
  return flag;
}

int FileExists(const char *fname, 
	       int mustbewritable, 
	       int deleteflagfile) {
  return FileExists2(NULL, (char *)fname, mustbewritable, deleteflagfile);
//   struct stat buf;
//   int flag = 0;
    
//   flag = (stat(fname, &buf) == 0) &&	// if file exists
//     S_ISREG(buf.st_mode);
//   if (flag && mustbewritable) {	// check if writable
//     flag &= access(fname, W_OK) == 0;
//     if (!flag) 
//       fprintf(stderr, "FileExists ERROR: %s exists but is not writeable\n", fname);
//   }
//   if (flag && deleteflagfile)
//     unlink(fname);		
//   return flag;
}

int DirExists(char *dirname, 
	      int mustbewritable, 
	      int createdir, 
	      mode_t mode) {
  struct stat buf;
  int flag = 0;
    
  flag = (stat(dirname, &buf) == 0) &&	// if file exists
    S_ISDIR(buf.st_mode);
  if (flag && mustbewritable) {	        // check if writable
    flag &= access(dirname, W_OK) == 0;
    if (!flag) 
      fprintf(stderr, "DirExists ERROR: %s exists but is not writeable\n", dirname);
  }
  else if (createdir) {
    flag = CreateDir(dirname, mode);
    if (!flag) 
      fprintf(stderr, "DirExists ERROR: Unable to create directory - %s\n", dirname);
  }
  return flag;
}

int CreateDir(char *dirname, mode_t mode) {
  char temppath[256], *ptr, tempchar;
  int result = 0;
  bool done;

  if (!dirname)
    return -1;
  result = mkdir(dirname, mode);  // try to create the easy way
  if ((result == -1) &&
      (errno == ENOENT))          // some part of path not present, 
    {                             // try to create the directory heirarchy
      strncpy(temppath, dirname, 256);
      ptr = temppath;
      done = false;
      while (*ptr && !done)
	{
	  while (*ptr && 
		 (*ptr != '/') &&
		 (*ptr != '\\'))
	    {
	      if (*ptr)
		ptr++;
	    }
	  if (*ptr)  // must be '/' or '\'
	    {
	      tempchar = *(ptr+1);   // save cahar after path delimiter
	      *(ptr+1) = 0;          // insert string terminator into temppath
	      if (!DirExists(temppath, false, false)) // if path doesn't exist try to mkdir it
		{
		  result = mkdir(temppath, mode);  // 
		  done = (result == -1);            // unable to create directory, CreateDir failed
		}
	      if (!done)
		*(ptr+1) = tempchar;           // put char after delimitor character back into string

	      ptr++;
	    }
	}
    }
  return result == 0;
}

void ASSERT(void *ptr) {
  if (!ptr) 
    fprintf(stderr, "ASSERT: NULL pointer detected\n");
}

double DiskFree(char *pathname)
{
  double freebytes = 0;
  struct statvfs statvfsbuf;
  char tempname[256];

  strncpy(tempname, pathname, 255);
	
  if (statvfs(dirname(tempname), &statvfsbuf) == 0)
    {
      freebytes = double(statvfsbuf.f_bavail) * double(statvfsbuf.f_frsize);
      return freebytes;
    }
  else 
    {
      fprintf(stderr, "DiskFree call failed on %s, %s\n", tempname, strerror(errno));
      freebytes = -1;
      return freebytes;
    }
}

double DiskFree(int fildes)
{
  double freebytes = 0;
  struct statvfs statvfsbuf;
	
  if (fstatvfs(fildes, &statvfsbuf) == 0)
    {
      freebytes = double(statvfsbuf.f_bavail) * double(statvfsbuf.f_frsize);
      return freebytes;
    }
  else 
    {
      fprintf(stderr, "DiskFree call failed on fildes=%d, %s\n", fildes, strerror(errno));
      freebytes = -1;
      return freebytes;
    }
}

bool timeval_gte(timeval time1, timeval time2)
{
  if (time2.tv_sec < time1.tv_sec)    // test for lt first, assume most likely to succeed
    return false;
  else if (time2.tv_sec == time1.tv_sec)
    return (time2.tv_usec >= time1.tv_usec);
  else
    return true;
}

bool timeval_gt(timeval time1, timeval time2)
{
  if (time2.tv_sec < time1.tv_sec)    // test for lt first, assume lt most often the case
    return false;
  else if (time2.tv_sec == time1.tv_sec)
    return (time2.tv_usec > time1.tv_usec);
  else
    return true;
}

bool timeval_lte(timeval time1, timeval time2)
{
  if (time2.tv_sec < time1.tv_sec)    // test for lt first, assume most likely to succeed
    return true;
  else if (time2.tv_sec == time1.tv_sec)
    return (time2.tv_usec <= time1.tv_usec);
  else
    return false;
}

bool timeval_lt(timeval time1, timeval time2)
{
  if (time2.tv_sec < time1.tv_sec)    // test for lt first, assume most likely to succeed
    return true;
  else if (time2.tv_sec == time1.tv_sec)
    return (time2.tv_usec < time1.tv_usec);
  else
    return false;
}

double timeval_secs(timeval time1)
{
  return double(time1.tv_sec) + double(time1.tv_usec)/1000000.0;
}

double timeval_diff(timeval time1, timeval time2)
{
  return timeval_secs(time2) - timeval_secs(time1);
}

bool timenow_gte_time(timeval reftime)         // compare reftime to current time
{
  timeval timenow;
  
  bool ok = gettimeofday(&timenow, 0) == 0;
  
  return ok && timeval_gte(reftime, timenow);
}

bool timenow_gt_time(timeval reftime)          // compare reftime to current time
{
  timeval timenow;
  
  bool ok = gettimeofday(&timenow, 0) == 0;
  
  return ok && timeval_gt(reftime, timenow);
}

bool timenow_lte_time(timeval reftime)         // compare reftime to current time
{
  timeval timenow;
  
  bool ok = gettimeofday(&timenow, 0) == 0;
  
  return ok && timeval_lte(reftime, timenow);
}

bool timenow_lt_time(timeval reftime)          // compare reftime to current time
{
  timeval timenow;
  
  bool ok = gettimeofday(&timenow, 0) == 0;
  
  return ok && timeval_lt(reftime, timenow);
}

double timenow_diff(timeval reftime)          // return time difference between now and ref time as double
{
  timeval timenow;
  
  gettimeofday(&timenow, 0);
  
  return timeval_diff(reftime, timenow);
}

void time_plus_secs(timeval *TM, double addsecs)  // add seconds to passed TM
{
  if (!TM) return;
  
  long secs = long(floor(addsecs));
  long usecs = long((addsecs - floor(addsecs)) * 1000000);
  
  TM->tv_sec += secs;
  TM->tv_usec += usecs;
  if (TM->tv_usec > 1000000)
    {
      TM->tv_sec += TM->tv_usec / 1000000; // add whole seconds
      
      TM->tv_usec = TM->tv_usec % 1000000; // remove whole seonds from usecs
    }
}

void timenow_plus_secs(timeval *TM, double addsecs) // gettimeofday and add seconds, return timeval
{
  if (!TM) return;

  if (gettimeofday(TM, NULL) != 0) return;
  time_plus_secs(TM, addsecs);
}



int argFound(int argc, char **argv, char *argstr)
{
  bool found = false;
  int arg = 1;   // skip arg[0] - it is prog name not a passed arg
  while (!found && (arg < argc))
    {
      found = strstr(argv[arg], argstr);
      if (!found) arg++;
    }
  if (!found) arg = 0;
  return arg;
}

// scaled value string scaled to k(ilo) M(ega) G(iga) T(era)
// field size is 4+suffix, i.e. up to 9999
char badstringptr[] = "Invalid String Ptr";
char* scaled_kMGTSizeString(long sizein, char *strout)
{
  if (!strout) return badstringptr;
  float kb = 1024.0;
  if (float(sizein) < float(10.0 * kb))                     // up to 10k use units
    sprintf(strout, "%d", int(sizein));
  else if (float(sizein) < float(10.0 * kb * kb))           // up to 10M use k units
    sprintf(strout, "%dk", int(sizein/kb));
  else if (float(sizein) < float(10 * kb * kb * kb))      // up to 10G use M units
    sprintf(strout, "%dM", int(sizein/(kb * kb)));
  else if (float(sizein) < float(10 * kb * kb * kb * kb))// up to 10T use G units
    sprintf(strout, "%dG", int(sizein/(kb * kb * kb)));
  else                                        // then use T units
    sprintf(strout, "%dT", int(sizein/(kb * kb * kb * kb)));
  return strout;
}    

char __truestr[] = "true";
char __falsestr[] = "false";
char* boolStr(bool state) {
  if (state)
    return __truestr;
  else
    return __falsestr;
}
