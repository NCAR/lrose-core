#include <malloc.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "rdrutils.h"

#ifdef HAVE_MALLINFO
struct mallinfo	mi, prevmi;

#endif

char	MEMORYDUMPFILE[128] = "memdump.data";

int MaxMemoryStatusFileSize = 1000000;

void AppendMemoryStatus()
{
  AppendMemoryStatus(NULL, 0, false);
}

void AppendMemoryStatus(char *fname)
{
  AppendMemoryStatus(fname, 0, false);
}
  
void AppendMemoryStatus(char *fname, int maxsize)
{
  AppendMemoryStatus(fname, maxsize, false);
}
  
void CloseMemoryStatus()
{
  AppendMemoryStatus(NULL, 0, true);
}
  
void CloseMemoryStatus(char *fname)
{
  AppendMemoryStatus(fname, 0, true);
}

void AppendMemoryStatus(char *fname, int maxsize, bool closing) {
  struct stat	statbuf;
  char	newname[128];
  FILE	*dumpfile;
  if (!fname) fname = MEMORYDUMPFILE;
  if (stat(fname, &statbuf) == 0) {
    if (!maxsize) maxsize = MaxMemoryStatusFileSize;
    if (statbuf.st_size > maxsize) {    
      strcpy(newname, fname);	// cycle memory status file
      strcat(newname, ".sav");
      rename(fname, newname);
    }
  }
  dumpfile = fopen(fname, "a");  //open for append    
  if (!dumpfile) {
    fprintf(stderr, "AppendMemoryStatus FAILED,  Unable to open %s\n", fname);
    perror(0);
    return;
  }
  else DumpMemoryStatus(dumpfile, closing);
  fclose(dumpfile);
}

void	Check_mallinfo() {
#ifdef HAVE_MALLINFO
  prevmi = mi;
  mi = mallinfo();
#endif
}

/* From malloc.h
struct mallinfo {
  int arena;    // non-mmapped space allocated from system
  int ordblks;  // number of free chunks
  int smblks;   // number of fastbin blocks
  int hblks;    // number of mmapped regions
  int hblkhd;   // space in mmapped regions
  int usmblks;  // maximum total allocated space
  int fsmblks;  // space available in freed fastbin blocks
  int uordblks; // total allocated space
  int fordblks; // total free space
  int keepcost; // top-most, releasable (via malloc_trim) space
};
*/ 
void Print_mallinfo(FILE *file) {
  if (!file) file = stderr;
#ifdef HAVE_MALLINFO
  fprintf(file,"CURRENT TotalAlloc=%1.1fMB (Delta=%1.1fMB) NonMmapped=%1.1fMB Mmapped=%1.1fMB\n"
               "ordblks=%d(%1.1fMBused %1.1fMBfree) MmapRegions=%d\n",
	  float(mi.arena+mi.hblkhd)/1000000.0,
          float((mi.arena+mi.hblkhd)-(prevmi.arena+prevmi.hblkhd))/1000000.0,
	  float(mi.arena)/1000000.0, float(mi.hblkhd)/1000000.0,
	  mi.ordblks, float(mi.uordblks) / 1000000.0, float(mi.fordblks) / 1000000.0,
	  mi.hblks);
  fprintf(file,"smlblks=%d(%1.1fMBused %1.1fMBfree)\n",
          mi.smblks, float(mi.usmblks) / 1000000.0, float(mi.fsmblks) / 1000000.0);
#ifdef sgi
  fprintf(file,"File descriptors used=%d of %d\n", getdtablehi(), getdtablesize());
#endif
#endif
}

extern pid_t mainpid;
void printVmSize(FILE *file)
{
#ifdef LINUX
  // look for VmSize:   841344 kB
  if (!file)
    return;
  char fname[128];
  char str[256];
  sprintf(fname, "/proc/%d/status", mainpid);
  FILE *statusfile = fopen(fname, "r");
  if (statusfile)
    {
      while (fgets(str, 255, statusfile))
	{
	  StripTrailingWhite(str); // remove cr
	  if (strstr(str, "VmSize:"))
	    fprintf(file, "%s ", str);
	  if (strstr(str, "VmRSS:"))
	    fprintf(file, "%s\n", str);
	  if (strstr(str, "VmData:"))
	    fprintf(file, "%s ", str);
	  if (strstr(str, "VmStk:"))
	    fprintf(file, "%s ", str);
	}
      fprintf(file, "\n");
    }
  fclose(statusfile);
  statusfile = fopen("/proc/meminfo", "r");
  if (statusfile)
    {
      while (fgets(str, 255, statusfile))
	{
	  StripTrailingWhite(str); // remove cr
	  if (strstr(str, "MemFree"))
	    fprintf(file, "%s ", str);
	  else if (strstr(str, "SwapFree"))
	    fprintf(file, "%s ", str);
	  else if (strstr(str, "SwapCached"))
	    ;
	  else if (strstr(str, "Cached"))
	    fprintf(file, "%s\n", str);
	}
      fprintf(file, "\n");
    }
  
  fclose(statusfile);
#endif
}

