#ifndef __LOG_H
#define __LOG_H

/*
 *  log.h   header for status logging module
 * 
 *  Data can be logged to syslog, console or file (auto-cycling)
 */
 
/*
 * level below is from syslog.h
 * 
 */
 
#include <syslog.h>

extern int LOG_COMMS;
extern int LOG_CDATA;
extern int LOG_INGEST;

extern int AlwaysLogToStderr;
extern int DfltLogLevel;
extern bool logOpened;
//void RapicLog(char *string,  int level, int LogToConsole = 0);
void setDefaultLogName(char *logname);
void RapicLog(char *string,  int level, int LogToConsole = 0,
              int LogToFile = 1,  // enabled by default
              char *logname = 0, // default filename
              int logsize = 1000000,       // keep max 1MB logs
              short int rotate = 4);       // # of times a log is rotated
void EndRapicLog();

#endif	/* __LOG_H */
