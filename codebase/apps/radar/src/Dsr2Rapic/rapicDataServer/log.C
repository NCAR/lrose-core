/*
 *  log.c   implementation of status logging module
 * 
 *  Data can be logged to syslog, console or file (auto-cycling)
 *  
 */
 
#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "spinlock.h"
#include "utils.h"
char defaultLogName[128] = "rapic.log";

int SysLogFlag[8] = {
    1, 1, 1, 1, 1, 1, 0, 0  // EMERG ALERT CRIT ERR WARNING NOTICE INFO DEBUG
};

int ConsoleLogFlag[8] = {
    1, 1, 1, 1, 1, 1, 1, 1  // EMERG ALERT CRIT ERR WARNING NOTICE INFO DEBUG
};


int AlwaysLogToStderr = 1;
char *syslog_heading = "3D-Rapic: ";
spinlock *loglock = 0;
int DfltLogLevel = LOG_INFO;
char LogFlagFile[] = "rplogflags";
char LogFlagFile2[] = "rplog.ini";
int LogFlagsLoaded = 0;
bool logOpened = false;
int logFacility = LOG_LOCAL0;
int LOG_COMMS = logFacility;
int LOG_CDATA = logFacility;
int LOG_INGEST = logFacility;
int localno = 0;

void CheckLogFlags() {
FILE *NewFlagFile = 0;
int NewSysLogFlags[8];
char flagstring[256];

    if (LogFlagsLoaded)	    // only do this once
	return;
    if (FileExists(LogFlagFile) || FileExists(LogFlagFile2)) {
      if (!(NewFlagFile = fopen(LogFlagFile, "r")))
	NewFlagFile = fopen(LogFlagFile2, "r");
      if (NewFlagFile)
	{	// 
	    if (fgets(flagstring, 256, NewFlagFile)) {
		if (sscanf(flagstring, "%d %d %d %d %d %d %d %d", 
			&NewSysLogFlags[0], &NewSysLogFlags[1], 
			&NewSysLogFlags[2], &NewSysLogFlags[3], 
			&NewSysLogFlags[4], &NewSysLogFlags[5], 
			&NewSysLogFlags[6], &NewSysLogFlags[7]) == 8) {
		    for (int x = 0; x < 8; x++)
			SysLogFlag[x] = NewSysLogFlags[x];
		}
	    }
	    if (fgets(flagstring, 256, NewFlagFile)) {
		if (sscanf(flagstring, "%d %d %d %d %d %d %d %d", 
			&NewSysLogFlags[0], &NewSysLogFlags[1], 
			&NewSysLogFlags[2], &NewSysLogFlags[3], 
			&NewSysLogFlags[4], &NewSysLogFlags[5], 
			&NewSysLogFlags[6], &NewSysLogFlags[7]) == 8) {
		    for (int x = 0; x < 8; x++)
			ConsoleLogFlag[x] = NewSysLogFlags[x];
		}
	    }
	    if (fgets(flagstring, 256, NewFlagFile)) {
	      if (sscanf(flagstring, "facility=local%d", &localno) == 1)
		{
		  if ((localno >= 0) && (localno <= 7))
		    {
		      logFacility = LOG_LOCAL0 + localno;
		      LOG_COMMS = logFacility;
		      LOG_CDATA = logFacility;
		      LOG_INGEST = logFacility;
		    }
		}
	    }
	fclose(NewFlagFile);
	}
    fprintf(stdout, "RapicLog syslog flags set to: \n"
	"  EMERG=%d ALERT=%d CRIT=%d ERR=%d WARN=%d NOTICE=%d INFO=%d DEBUG=%d\n", 
	SysLogFlag[0], SysLogFlag[1], 
	SysLogFlag[2], SysLogFlag[3], 
	SysLogFlag[4], SysLogFlag[5], 
	SysLogFlag[6], SysLogFlag[7]);		
    fprintf(stdout, "RapicLog console log flags set to: \n"
	"  EMERG=%d ALERT=%d CRIT=%d ERR=%d WARN=%d NOTICE=%d INFO=%d DEBUG=%d\n", 
	ConsoleLogFlag[0], ConsoleLogFlag[1], 
	ConsoleLogFlag[2], ConsoleLogFlag[3], 
	ConsoleLogFlag[4], ConsoleLogFlag[5], 
	ConsoleLogFlag[6], ConsoleLogFlag[7]);		
    fprintf(stdout, "RapicLog facility set to: LOG_LOCAL%d\n", 
	    logFacility - LOG_LOCAL0);
    fprintf(stdout, "RapicLog LOG_COMMS facility set to: LOG_LOCAL%d\n", 
	    LOG_COMMS - LOG_LOCAL0);
    fprintf(stdout, "RapicLog LOG_CDATA facility set to: LOG_LOCAL%d\n", 
	    LOG_CDATA - LOG_LOCAL0);
    fprintf(stdout, "RapicLog LOG_INGEST facility set to: LOG_LOCAL%d\n", 
	    LOG_INGEST - LOG_LOCAL0);
    LogFlagsLoaded = 1;
    }
}

void setDefaultLogName(char *logname)
{
  strncpy(defaultLogName, logname, 127);
}

void RapicLog(char *string,  int level, int LogToConsole,
              int LogToFile, char *logname, int logsize, short int rotate) {
/*
   string       - message
   level        - syslog logging level, SysLogFlag[level],
                  level also used when logging to a file.
   LogToConsole - output message to console, default=0 (disabled)
   LogToFile    - output message to a logfile, default=1 (enabled)
   logname      - logfile name, use defaultLogName if no logname specified (NULL)
   logsize      - logfile size, default=1000000 (1MB)
   rotate       - # of times to rotate logfile, default=4 

   Notes:
   - if !LogToConsole && !LogToFile then message is passed to syslog().
   - rplog.ini not updated to use LogToFile
*/
   struct stat statbuf;
   char *slog, *tlog;
   char timestr[128];
   FILE *logfp; // nb. local declaration, logfiles opened/closed locally
   short int i;
   time_t timenow = time(0);

/*
 * use lock to "serialise" calls to rapiclog from multiple threads
 */
    if (!loglock) {	// first time
	loglock = new spinlock("RapicLog->loglock", 100);	// 1 secs
	if (loglock) loglock->get_lock();
	CheckLogFlags();
	if (loglock) loglock->rel_lock();
	}
    if (loglock) loglock->get_lock();

    if (!logname)
      logname = defaultLogName;
	
    // check if log file exists and rotate if > logsize
    if (LogToFile && logname!=NULL && stat(logname, &statbuf)==0) {
      if (statbuf.st_size > logsize) {
        // first rotate/rename previous logs, ignore renaming errors here
        // default rotated log filename is rapicDataServer.log.[0-65535]
        slog = (char *)calloc(strlen(logname)+10, sizeof(char));
        tlog = (char *)calloc(strlen(logname)+10, sizeof(char));
        for (i=rotate; i>1; i--) {
          sprintf(tlog, "%s.%d", logname, i);
          sprintf(slog, "%s.%d", logname, i-1);
          if (stat(slog, &statbuf)==0)
            rename(slog, tlog);
        }
        // next rotate/rename current log, report renaming errors
        if (logOpened) fclose(logfp);
        sprintf(tlog, "%s.1", logname);
        rename(logname, tlog);
        free(slog);
        free(tlog);
      }
    }

    // open log files
    if (!logOpened) {
      if (LogToFile && (logfp=fopen(logname, "a"))==NULL) {
        perror("RapicLog");
      }
      else
	{
	  openlog(syslog_heading, LOG_PID, logFacility);
	  logOpened = true;
	}
    }

    // output
    if (SysLogFlag[level]) {  // don't log all
      if (LogToFile && logOpened) {
        // ensure output is always terminated with newline char
        if (string[strlen(string)-1]=='\n')
          fprintf(logfp, "%s %s[%d]: %s",
                  TimeString(timenow, timestr, true, true),
                  syslog_heading, getpid(), string);
        else
          fprintf(logfp, "%s %s[%d]: %s\n",
                  TimeString(timenow, timestr, true, true),
                  syslog_heading, getpid(), string);
      }
      else
	syslog(logFacility | level, "%s%6d %s", syslog_heading, getpid(), string);
    }
    if ((LogToConsole || AlwaysLogToStderr) && ConsoleLogFlag[level])
	fprintf(stderr, "%s\n", string);

    // cleanup
    //if (logfp) fclose(logfp);
    if (logOpened) fclose(logfp);
    logOpened = false;
    if (loglock) loglock->rel_lock();
}

