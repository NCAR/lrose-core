/*
 *   fam_mon.c -- monitor arbitrary file or directory
 *                using fam
 *
 *   Author - Phil Purdam 5/11/2002
 *
    fam_mon allows monitoring of directories with rudimentary file name
    string matching and exclusion and event masking and of also of
    creation, modification and deletion of individual files
    Typically this mode would be used to watch for new files created 
    in a directory or files modified or deleted from a dir.
    Care needs to be taken where large numbers of files may exist
    in a directory as a FAMExists event will be generated for
    every file in that directory.

    Directory monitoring requires that the directory exists, if not
    the FAMObject manager will retry failed monitors periodically.

    File monitoring  

 *   
 */
 
#include "fam_mon.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <libgen.h>


/* fam_event_name() - return printable name of fam event code */
 
#include <iostream>
using namespace std;

char fileReqObjTitle[] = "fileReqObj::dumpStatus";
char urlReqObjTitle[] = "urlReqObj::dumpStatus";
char FAMReqObjTitle[] = "FAMReqObj::dumpStatus";

char *froTypeStr[] = {
  "FRO_FAM",
  "FRO_URL",
  "FRO_LTNING",
  "FRO_UNDEFINED"
};

char *getFroTypeStr(fileReqType ftype)
{
  if ((ftype < FRO_FAM) || 
      (ftype >= FRO_UNDEFINED))
    return froTypeStr[FRO_UNDEFINED];
  else
    return froTypeStr[ftype];
}

const char *famevent[] = {
  "",
  "FAMChanged",
  "FAMDeleted",
  "FAMStartExecuting",
  "FAMStopExecuting",
  "FAMCreated",
  "FAMMoved",
  "FAMAcknowledge",
  "FAMExists",
  "FAMEndExist"
};

const char *fam_event_name(int code)
{
  static char unknown_event[10];
 
  if (code < FAMChanged || code > FAMEndExist)
    {
      sprintf(unknown_event, "unknown (%d)", code);
      return unknown_event;
    }
  return famevent[code];
}

int fam_event_by_name(char *eventstr)
{
  int code = 1;
  bool match = false;

  while (!match && (code <= FAMEndExist))
    {
      match = strcmp(eventstr, famevent[code]) == 0;
      if (!match)
	code++;
    }
  if (match)
    return code;
  else
    return 0;
}

/* fam_event_bit() - return bit mask for fam event code */
 
int fam_event_bit(int code)
{
  if (code < FAMChanged || code > FAMEndExist)
    {
      return 0;
    }
  return (1 << (code-1));
}

void fam_decodeEventsStr(char *eventsstr, _event_mask &event_mask) 
{
  if (!eventsstr) return;
  event_mask.reset();
  int code = 1;
  while (code <= FAMEndExist)
    {
      if (strstr(eventsstr, famevent[code]))
	{
	  event_mask[code] = 1;
// 	  fprintf(stdout, "FAMReqObj::decodeEventsStr - adding event %s\n",
// 		  fro_id, famevent[code]);
	}
      code++;
    }
}

void fam_encodeEventsStr(std::string &eventmaskstr, _event_mask &event_mask)
{
  int code = 1;
  eventmaskstr.clear();
  while (code <= FAMEndExist)
    {
      if (event_mask[code])
	{
	  eventmaskstr += fam_event_name(code);
	  eventmaskstr += ",";
	}
      code++;
    }
}

FAMObject::FAMObject(int argc, char *argv[])
{
  setDefaults(argc, argv);
  if (argc)            // defer open until first file/dir monitor
    progName = argv[0];
  else
    progName = "FAMObject";
//   if (argc)            // defer open until first file/dir monitor
//     open(argv[0]);
//   else
//     open();
//   if (!famOpenFailed)
  init(argc, argv);
}

FAMObject::FAMObject(char *progname, char *initfname)
{
  setDefaults();
  if (initfname)
    iniFname = initfname;
  if (progname)
    progName = progname;
  else
    progName = "FAMObject";
//   open();  // defer open until first file/dir monitor
}

void FAMObject::setDefaults(int argc, char *argv[])
{
  iniFname = "fam_mon.ini";
  progName = "FAMObject";
  famOpenFailed = false;
  fc.fd = -1;
  famIsOpen = false;
  froCount = 0;
  defaultDoEventDelay = -1;
  defaultTimeout = 0;
  defaultEventMask.reset();
  timestamp = localtime = timestamp_timet = false;
  retryFailedMonitorPeriod = 120;
  retryFailedMonitorTime = time(0) + retryFailedMonitorPeriod;
  retryFailedMonitorCount = 0;
  restartFAMOnError = false;
  //  restartFAMOnError = true;
  debug = 0;
  Argc = argc;
  Argv = argv;
  defaultUseEventMap = false;
  dumpStatusTime = 0;
  dumpStatusPeriod = 60; // default - dump status every 60 secs 
  dumpStatusFName = "fam_mon.status";
  noExistsEvents = true;
}

FAMObject::~FAMObject()
{
  close();
}

bool FAMObject::open(char *progname)
{
  int result = 0;
  if (famIsOpen || fc.fd != -1)
    close();
  if (progname)
    {
      progName = progname;
    }
  result = FAMOpen2(&fc, (char *)progName.c_str());
  if (noExistsEvents)
    setNoExistsEvents();
//   else
//     enableExistsEvents();
  if (result != 0)
    {
      famOpenFailed = true;
      fprintf(stderr, "FAMObject::open FATAL ERROR - FAMOpen2 call FAILED - errno=%d %s\n",
	      FAMErrno, FamErrlist[FAMErrno]);
      fprintf(stderr, "PLEASE ENSURE THAT THE FAM SERVICE IS INSTALLED AND ENABLED\n");
    } 
  return ((famIsOpen = (result == 0)));
}

void FAMObject::close()
{
  close_all_fro();
  if (fc.fd != -1)
    {
      FAMClose(&fc);
      fc.fd = -1;
    }
}

// Current implementation only allows exists events to be turned off, cannot turn on again
// void FAMObject::enableExistsEvents()
// {
//   if (famIsOpen)
//     FAMNoExists(&fc)
//   suppressExistsEvents = false;  
// }

void FAMObject::setNoExistsEvents(bool state)
{
  if (state)
    {
#ifdef HAVE_FAMNOEXISTS
      if (famIsOpen)
	FAMNoExists(&fc)
#endif /* HAVE_FAMNOEXISTS */
	  noExistsEvents = true;
      fprintf(stdout, "FAMObject::setNoExistsEvents - FAMExists event disabled\n");
    }
// Current implementation only allows exists events to be turned off, cannot turn on again
  else
    fprintf(stdout, "FAMObject::setNoExistsEvents - API DOES NOT SUPPORT false call\n");
}

void FAMObject::close_all_fro()
{
  fro_iter = fro.begin();
  fprintf(stdout, "FAMObject::close_all_fro - closing %d fro\n",
	  froCount);
  while (fro_iter != fro.end())
    {
      fprintf(stdout, "  Closing %s\n",
	      (*fro_iter)->realFName.c_str());
      FAMCancelMonitor(&fc, &((*fro_iter)->fr));
      delete (*fro_iter);
      fro_iter++;
    }
  fro.clear();
  froCount = 0;
}


// Deprecated - command line use is generally not used - new functionality not being added
void FAMObject::init(int argc, char *argv[])
{
  int i, nmon;
  FAMReqObj *newfro;
  int tempint;
  
  if (argc)
    {
      Argc = argc;
      Argv = argv;
    }
//   if (!famIsOpen || fc.fd == -1)
//     if (!open())
//       return;
  if (froCount)
    close_all_fro();
  retryFailedMonitorPeriod = 120;
  retryFailedMonitorTime = time(0) + retryFailedMonitorPeriod;
  retryFailedMonitorCount = 0;
  for (nmon = 0, i = 1; i < Argc; i++)
    {
      if (Argv[i][0] != '-')
	{
	  if (!famIsOpen && !famOpenFailed)
	    open();
	  if (famIsOpen)
	  {
	    newfro = new FAMReqObj(NULL, froCount+1);
	    newfro->setFName(Argv[i]);
	    // if event and timeout exec strings not set and
	    // default event of timeout exec strings exist, copy them to newfro
	    if (!newfro->eventExecStr.size() && defaultEventExecStr.size())
	      newfro->eventExecStr = defaultEventExecStr;
	    if ((newfro->doEventDelay < 0) &&  defaultDoEventDelay)
	      newfro->doEventDelay = defaultDoEventDelay;
	    if (newfro->doEventDelay < 0)
	      newfro->doEventDelay = 0;
	    if (!newfro->timeout &&  defaultTimeout)
	      newfro->timeout = defaultTimeout;
	    if (newfro->timeout && !newfro->firstTimeoutExecStr.size() && 
		defaultFirstTmOutExecStr.size())
	      newfro->firstTimeoutExecStr = defaultFirstTmOutExecStr;
	    if (newfro->timeout && !newfro->reTimeoutExecStr.size() && 
		defaultReTmOutExecStr.size())
	      newfro->reTimeoutExecStr = defaultReTmOutExecStr;
	    if (!newfro->matchString.size() && 
		defaultMatchString.size())
	      newfro->matchString = defaultMatchString;
	    if (!newfro->excludeString.size() && 
		defaultExcludeString.size())
	      newfro->excludeString = defaultExcludeString;
	    if (!add_fro(newfro))
	      delete newfro;
	    else
	      nmon++;
	  }
	}
      else // parse options
	{
	  if (!strcmp(Argv[i], "-timestamp"))
	    {
	      timestamp = true;
	      fprintf(stdout, "Timestamps enabled\n");
	    }
	  if (!strcmp(Argv[i], "-local"))
	    {
	      localtime = timestamp = true;
	      fprintf(stdout, "Localtime stamps enabled\n");
	    }
	  if (!strcmp(Argv[i], "-time_t_stamp"))
	    {
	      timestamp_timet = true;
	      fprintf(stdout, "Time_t timestamps enabled\n");
	    }
	  // set process to run when event detected 
	  if (!strcmp(Argv[i], "-e"))
	    {
	      defaultEventExecStr = Argv[i+1];
	      i++;
	      fprintf(stdout,"defaultEventExecStr: %s\n",
		      defaultEventExecStr.c_str());
	    }
	  if (!strcmp(Argv[i], "-string"))
	    {
	      defaultMatchString = Argv[i+1];
	      i++;
	      fprintf(stdout,"defaultMatchString: %s\n",
		      defaultMatchString.c_str());
	    }
	  if (!strcmp(Argv[i], "-exclude"))
	    {
	      defaultExcludeString = Argv[i+1];
	      i++;
	      fprintf(stdout,"defaultExcludeString: %s\n",
		      defaultExcludeString.c_str());
	    }
	  if (!strcmp(Argv[i], "-i"))
	    {
	      iniFname = Argv[i+1];
	      i++;
	      fprintf(stdout,"iniFname: %s\n", iniFname.c_str());
	    }
	  if (strstr(Argv[i], "-debug"))
	    {
	      debug = 1;
	      if (sscanf(Argv[i], "-debug=%d", &tempint) == 1)
		debug = tempint;
	      fprintf(stdout, "Debug level = %d\n", debug);
	    }
	}
    }
  init();
}  

void FAMObject::setMonDefaults(char *monstr, FAMReqObj *newfro)
{
  if (!monstr || !newfro)
    return;
  setMonDefaults(monstr, (fileReqObj*)newfro);
  // if newfro events NOT defined and default IS defined
  // set fro event_mask to defined defaults
  if (!strstr(monstr, "events=") && 
      defaultEventMask.count())  // only use if some bits set
    newfro->event_mask = defaultEventMask;
}

void FAMObject::setMonDefaults(char *monstr, fileReqObj *newfro)
{
  if (!monstr || !newfro)
    return;
  // if event and timeout exec strings not set and
  // default event of timeout exec strings exist, 
  // copy them to newfro
  if (!newfro->eventExecStr.size() && defaultEventExecStr.size())
    newfro->eventExecStr = defaultEventExecStr;
  if ((newfro->doEventDelay < 0) &&  defaultDoEventDelay)
    newfro->doEventDelay = defaultDoEventDelay;
  if (newfro->doEventDelay < 0)
    newfro->doEventDelay = 0;
  if (!newfro->timeout &&  defaultTimeout)
    newfro->timeout = defaultTimeout;
  if (newfro->timeout && !newfro->firstTimeoutExecStr.size() && 
      defaultFirstTmOutExecStr.size())
    newfro->firstTimeoutExecStr = defaultFirstTmOutExecStr;
  if (newfro->timeout && !newfro->reTimeoutExecStr.size() && 
      defaultReTmOutExecStr.size())
    newfro->reTimeoutExecStr = defaultReTmOutExecStr;
  if (!newfro->matchString.size() && 
      defaultMatchString.size())
    newfro->matchString = defaultMatchString;
  if (!newfro->excludeString.size() && 
      defaultExcludeString.size())
    newfro->excludeString = defaultExcludeString;
}

void FAMObject::newFAMReqEntry(char *monstr)
{
  if (!monstr) return;
  if (!famIsOpen && !famOpenFailed)
    open();
  if (famIsOpen)
    {
      FAMReqObj *newfro = new FAMReqObj(monstr, froCount+1);
      setMonDefaults(monstr, newfro);
      if (!add_fro(newfro))
	delete newfro;  // duplicate, not added, delete it now
    }
}

void FAMObject::init(char *initfname)
{
  FILE *inifile;
  char linebuff[512];
  char *argstr;
  char tempstr[256];
  int tempint;
  
//   if (fc.fd == -1)
//     if (!open()
//       return;
  if (froCount)
    close_all_fro();
  if (initfname)
    iniFname = initfname;
  retryFailedMonitorPeriod = 120;
  retryFailedMonitorTime = time(0) + retryFailedMonitorPeriod;
  retryFailedMonitorCount = 0;
  if ((inifile = fopen(iniFname.c_str(), "r")))
    {
      while (fgets(linebuff, 512, inifile))
	if (linebuff[0] != '#')
	  {
	    if (strstr(linebuff, "file=") || strstr(linebuff, "dir="))
	      {
		newFAMReqEntry(linebuff);
	      }
	    else if (strstr(linebuff, "ltningurl="))
	      {
		newLtningURLReqEntry(linebuff);
	      }
	    else if (strstr(linebuff, "url="))
	      {
		newURLReqEntry(linebuff);
	      }
	    else   // no file or dir name, check for global arguments
	      {
		if (strstr(linebuff, "timestamp"))
		  {
		    timestamp = true;
		    fprintf(stdout, "Timestamps enabled\n");
		  }
		if (strstr(linebuff, "local"))
		  {
		    localtime = timestamp = true;
		    fprintf(stdout, "Localtime stamps enabled\n");
		  }
		if (strstr(linebuff, "time_t_stamp"))
		  {
		    timestamp_timet = true;
		    fprintf(stdout, "Time_t timestamps enabled\n");
		  }
		// set default process to run when event detected 
		if ((argstr = strstr(linebuff, "event_exec=")))
		  {
		    if (sscanf(argstr, "event_exec=%s", tempstr) == 1)
		      defaultEventExecStr = tempstr;
		    fprintf(stdout,"event_exec: %s\n",
			    defaultEventExecStr.c_str());
		  }
		// set default process to run when timeout detected 
		if ((argstr = strstr(linebuff, "timeout_exec=")))
		  {
		    if (sscanf(argstr, "timeout_exec=%s", tempstr) == 1)
		      defaultFirstTmOutExecStr = tempstr;
		    fprintf(stdout,"timeout_exec: %s\n",
			    defaultFirstTmOutExecStr.c_str());
		  }
		// set default process to run when first timeout detected 
		if ((argstr = strstr(linebuff, "first_timeout_exec=")))
		  {
		    if (sscanf(argstr, "first_timeout_exec=%s", tempstr) == 1)
		      defaultFirstTmOutExecStr = tempstr;
		    fprintf(stdout,"first_timeout_exec: %s\n",
			    defaultFirstTmOutExecStr.c_str());
		  }
		// set default process to run when re-timeout detected 
		if ((argstr = strstr(linebuff, "re_timeout_exec=")))
		  {
		    if (sscanf(argstr, "re_timeout_exec=%s", tempstr) == 1)
		      defaultReTmOutExecStr = tempstr;
		    fprintf(stdout,"re_timeout_exec: %s\n",
			    defaultReTmOutExecStr.c_str());
		  }
		if ((argstr = strstr(linebuff, "timeout=")))
		  {
		    if (sscanf(argstr, "timeout=%d", &tempint) == 1)
		      defaultTimeout = tempint;
		    fprintf(stdout,"defaultTimeout: %d\n",int(defaultTimeout));
		  }
		if ((argstr = strstr(linebuff, "dump_status_period=")))
		  {
		    if (sscanf(argstr, "dump_status_period=%d", &tempint) == 1)
		      dumpStatusPeriod = tempint;
		    fprintf(stdout,"dumpStatusPeriod: %d\n",
			    int(dumpStatusPeriod));
		  }
		if ((argstr = strstr(linebuff, "event_exec_delay=")))
		  {
		    if (sscanf(argstr, "event_exec_delay=%d", &tempint) == 1)
		      defaultDoEventDelay = tempint;
		    fprintf(stdout,"do_exec_delay: %d\n",
			    int(defaultDoEventDelay));
		  }
		if ((argstr = strstr(linebuff, "do_event_delay=")))
		  {
		    if (sscanf(argstr, "do_event_delay=%d", &tempint) == 1)
		      defaultDoEventDelay = tempint;
		    fprintf(stdout,"do_event_delay: %d\n",
			    int(defaultDoEventDelay));
		  }
		if ((argstr = strstr(linebuff, "retry_failed_mon_period=")))
		  {
		    if (sscanf(argstr, "retry_failed_mon_period=%d", &tempint) 
			== 1)
		      {
			retryFailedMonitorPeriod = tempint;
			retryFailedMonitorTime = time(0) + 
			  retryFailedMonitorPeriod;
			fprintf(stdout,"retry_failed_mon_period: %d\n",
				int(retryFailedMonitorPeriod));
		      }
		  }
		// set default string to match 
		if ((argstr = strstr(linebuff, "string=")))
		  {
		    if (sscanf(argstr, "string=%s", tempstr) == 1)
		      {
			defaultMatchString = tempstr;
			fprintf(stdout,"defaultMatchString: %s\n",
				defaultMatchString.c_str());
		      }
		  }
		if ((argstr = strstr(linebuff, "exclude=")))
		  {
		    if (sscanf(argstr, "exclude=%s", tempstr) == 1)
		      {
			defaultExcludeString = tempstr;
			fprintf(stdout,"exclude: %s\n",
				defaultExcludeString.c_str());
		      }
		  }
		if ((argstr = strstr(linebuff, "events=")))
		  {
		    fam_decodeEventsStr(argstr+strlen("events="), 
					defaultEventMask);
		  }
		if (strstr(linebuff, "disable_fam_restart_on_error"))
		  {
		    restartFAMOnError = false;
		    fprintf(stdout, "restartFAMOnError disabled\n");
		  }
		if (strstr(linebuff, "enable_fam_restart_on_error"))
		  {
		    restartFAMOnError = true;
		    fprintf(stdout, "restartFAMOnError enabled\n");
		  }
		if ((argstr = strstr(linebuff, "debug")))
		  {
		    debug = 1;
		    if (sscanf(argstr, "debug=%d", &tempint) == 1)
		      debug = tempint;
		    fprintf(stdout, "Debug level = %d\n", debug);
		  }
		if (strstr(linebuff, "use_event_queue"))
		  {
		    defaultUseEventMap = true;
		    fprintf(stdout, "defaultUseEventMap enabled\n");
		  }
		if (strstr(linebuff, "disable_exists_events"))
		  {
		    noExistsEvents = true;
		    fprintf(stdout, "Exists events disabled\n");
		  }
		if (strstr(linebuff, "enable_exists_events"))
		  {
		    noExistsEvents = false;
		    fprintf(stdout, "Exists events enabled\n");
		  }
	      }
	  }
    }
  if (inifile)
    fclose(inifile);
}

bool FAMObject::add_fro(FAMReqObj *monfro)   // add fro to vector
{
  if (!famIsOpen && !famOpenFailed)
    open();
  if (!monfro || !famIsOpen || famOpenFailed)
    return false;
  if (fro_exists(monfro) == -1)   // only add if not in vector already
    fro.push_back(monfro);
  else
    return false;             // duplicate, add_fro failed
  froCount++;
  monfro->fro_id = froCount;
  if ((monfro->froType == FRO_FAM) &&
      !monfro->monitor(&fc))
    retryFailedMonitorCount++;
  return true;                // success
}

bool FAMObject::add_fro(urlReqObj *monfro)   // FAMObject doesn't support URL
{                                            // need child class to implement
  return false;
}

void FAMObject::remove_fro(FAMReqObj *monfro)// remove fro from vector
{
  fro_iter = fro.begin();
  while ((fro_iter != fro.end()) &&
	 (*fro_iter != monfro))
    fro_iter++;
  if (fro_iter != fro.end())
    {
      fro.erase(fro_iter);
      fro_iter = fro.begin();
      froCount--;
    }
}

int  FAMObject::fro_exists(FAMReqObj *monfro)// check if fro in vector, return location
{
  int pos = 0;
  if (!fro.size())
    return -1;
  while ((fro[pos] != monfro) && (pos < int(fro.size())))
    pos++;
  if (pos < int(fro.size()))
    return pos;
  else
    return -1;
}

bool FAMObject::monitor(FAMReqObj *monfro)
{
  if (!famIsOpen && !famOpenFailed)
    open();
  if (!monfro || (monfro->froType != FRO_FAM) || famOpenFailed)
    return false;
  return monfro->monitor(&fc);
}

void FAMObject::cancelMonitor(FAMReqObj *monfro)
{
  if (!monfro || !monfro->monitorActive)
    return;
  monfro->stopMonitor();
}

int FAMObject::eventsPending()
{
  if (famIsOpen && !famOpenFailed)  
    return FAMPending(&fc);
  else return 0;
}

void FAMObject::check()
{
  time_t timenow = time(0);
  int famresult=eventsPending();
  if (famresult>0)
    processEvents();
  checkFROs();
  if (retryFailedMonitorCount && 
      retryFailedMonitorPeriod &&
      (timenow > retryFailedMonitorTime))
    {
      retryFailedMonitors();
      retryFailedMonitorTime = timenow + retryFailedMonitorPeriod;
    }
  if (timenow > dumpStatusTime)
    {
      dumpStatus();
      dumpStatusTime = timenow + dumpStatusPeriod;
    }
}

void FAMObject::checkFROs()
{
  time_t timenow = time(0);
  fro_iter = fro.begin();
  retryFailedMonitorCount = 0;
  while (fro_iter != fro.end())
    {
      (*fro_iter)->check(timenow);
      if ((*fro_iter)->MonitorFailed())
	retryFailedMonitorCount++;
      fro_iter++;
    }
}

void FAMObject::retryFailedMonitors()
{
  if (retryFailedMonitorCount)
    fprintf(stderr, "FAMObject::retryFailedMonitors - Retrying %d monitors\n", 
	    retryFailedMonitorCount);
  fro_iter = fro.begin();
  retryFailedMonitorCount = 0;
  while (fro_iter != fro.end())
    {
      if (((*fro_iter)->froType == FRO_FAM) &&
	  (*fro_iter)->MonitorFailed())
	{
	  fprintf(stderr, "FAMObject::retryFailedMonitors - Retrying %s id=%d\n", 
		  (*fro_iter)->reqFName.c_str(), 
		  (*fro_iter)->fro_id);
	  if (!monitor(*fro_iter))
	    retryFailedMonitorCount++;
	}
      fro_iter++;
    }
}

void FAMObject::processEvents()
{
  FAMEvent fe;
  FAMReqObj *thisfro = NULL;
  int famPendingVal = 0, famNextEventVal = 0;
  
  if (!famIsOpen || famOpenFailed)
    return;
  while ((famPendingVal = FAMPending(&fc)) > 0)
    {
      famNextEventVal = FAMNextEvent(&fc, &fe);
      if (famNextEventVal < 0)
	{
	  perror("FAMObject::processEvents - FAMNextEvent Error");
	  fprintf(stderr, "FAM Events Pending = %d\n", famPendingVal);
	  fprintf(stderr, "FAMErrno=%d errno=%d\n", FAMErrno, errno);
	  if (FamErrlist[FAMErrno])
	    fprintf(stderr, "FAMErrString=%s\n", FamErrlist[FAMErrno]);
	  fprintf(stderr, "FamConnection fd=%d client=%p\n", fc.fd, fc.client);
	  if (restartFAMOnError)
	    {
	      fprintf(stderr, "Restarting FAM\n");
	      close();
	      open();
	      init(Argc, Argv);
	      sleep(1);
	    }
	  else
	    if (famPendingVal == FAMPending(&fc))  // nothing taken off pending queue
	      {
		fprintf(stderr, "FAMPending count unchanged, stop processing queue\n");
		return;                              // avoid continuous loop
	      }
	}
      else
	{
          cout << "(" << famevent[fe.code] << " " << fe.filename << ")" << endl;
	  if (fe.userdata)
	    {
	      thisfro = (FAMReqObj *)fe.userdata;
	      if (thisfro->fr.reqnum == fe.fr.reqnum) // sanity check
		thisfro->processFAMEvent(&fe, timestamp, localtime, timestamp_timet);
	      else
		{
		  fprintf(stderr, 
			  "FAMObject::processEvents Error, FAMEvent reqnum(%d) doesn't match FAMReqObj reqnum(%d)\n"
			  "FAMEvent hostname=%s\nFAMEvent filename=%s\n"
			  "FAMEvent fc addr=%p fd=%d type=%s\n",
			  fe.fr.reqnum,thisfro->fr.reqnum,
			  fe.hostname, fe.hostname, fe.fc, fe.fc->fd, fam_event_name(fe.code));
		  
		}
	    }
	  else
	    fprintf(stderr, "FAMObject::processEvents Error, event had no FAMReqObj pointer\n");
	}
    }
  if (famPendingVal < 0)
    {
      perror("FAMObject::processEvents - FAMNextEvent Error");
      fprintf(stderr, "FAMErrno=%d - %s\n", FAMErrno, FamErrlist[FAMErrno]);
    }
}

void FAMObject::dumpStatus(char *fname)
{
  if (!fname)
    fname = (char *)dumpStatusFName.c_str();
  if (!fname) return;
  FILE *dumpfile = fopen(fname, "w");
  if (dumpfile)
    {
      dumpStatus(dumpfile);
      fclose(dumpfile);
    }
  else
    {
      fprintf(stderr, "FAMObject::dumpStatus - failed opening file %s\n",
	      fname);
      perror(0);
    }
}

char FAMObjectTitle[] = "FAMObject::dumpStatus";

void FAMObject::dumpStatus(FILE *dumpfile)
{
  dumpStatus(dumpfile, FAMObjectTitle);
}

void FAMObject::dumpStatus(FILE *dumpfile, char *title)
{
  if (!dumpfile)
    return;
  if (!title)
    title = FAMObjectTitle;
  char timestr[64], timestr2[64];
  fprintf(dumpfile, "%s - %s\n"
	  "froCount=%d famIsOpen=%d famOpenFailed=%d iniFName=%s\n"
	  "defaultEventExec=%s defaultDoEventDelay=%d\n"
	  "defaultFirstTmOutExec=%s defaultReTmOutExec=%s\n"
	  "dfltTimeout=%d retryFailMonitorPeriod=%d retryFailedMonCount=%d\n"
	  "defaultMatch=%s defaultExclude=%s\n"
	  "timestamp=%d localtime=%d timestamp_time=%s\n"
	  "debug=%d restartFAMOnError=%d\n",
	  title, famShortDateString(time(0), timestr),
	  froCount, famIsOpen, famOpenFailed, iniFname.c_str(),
	  defaultEventExecStr.c_str(),  int(defaultDoEventDelay),
	  defaultFirstTmOutExecStr.c_str(),
	  defaultReTmOutExecStr.c_str(),
	  int(defaultTimeout), int(retryFailedMonitorPeriod), 
	  retryFailedMonitorCount,
	  defaultMatchString.c_str(), defaultExcludeString.c_str(),
	  timestamp, localtime, famShortDateString(timestamp_timet, timestr2),
	  debug, restartFAMOnError);
  if (FAMErrno)
    fprintf(dumpfile, "FAMErrno=%d %s\n",
	    FAMErrno, FamErrlist[FAMErrno]);
  fprintf(dumpfile, "\n");
      
  fro_iter = fro.begin();
  while (fro_iter != fro.end())
    {
      (*fro_iter)->dumpStatus(dumpfile);
      fprintf(dumpfile, "\n");
      fro_iter++;
    }
}

void fileReqObj::resetTimeout(time_t timenow)
{
  if (timeout)
    {
      if (!timenow) timenow = time(0);
      nextTimeoutTime = time(0) + timeout;
    }
}
    
bool fileReqObj::confirmTimeout(time_t timenow)
{
  if (!timenow) timenow = time(0);
  // not implemented yet, but should use stat to check that a file 
  // event hasn't been missed, maybe due to a fam/gamin problem
  return true;
}
  
bool fileReqObj::timedOut(time_t timenow, bool runit)
{
  bool timedout = false;
  if (timeout && nextTimeoutTime)
    {
      if (!timenow) timenow = time(0);
      timedout = timenow > nextTimeoutTime;
      if (timedout && runit)
	doTimeout();
    }
  if (timedout && !confirmTimeout())
    return false;
  return timedout;
}
      

void fileReqObj::doTimeout()
{
  if (isTimedOut)
    doReTimeout();
  else
    doFirstTimeout();
  isTimedOut = true;
}

void fileReqObj::doFirstTimeout()
{
  firstTimeoutTime = time(0);
  execFirstTimeout();
  resetTimeout();
}

void fileReqObj::doReTimeout()
{
  execReTimeout();
  resetTimeout();
}

void fileReqObj::execFirstTimeout()
{
  char execstr[1024];

  if (firstTimeoutExecStr.size())
    {
      sprintf(execstr, "%s %s ", firstTimeoutExecStr.c_str(), 
	      realFName.c_str());
      if (userString.size())
	strcat(execstr, userString.c_str());
      if (debug)
	fprintf(stdout, "FAMReqObj::execFirstTimeout Running command: %s\n", 
		execstr);
      system(execstr);
    }
  else
    {
      fprintf(stdout, "FAMReqObj::execFirstTimeout Timeout on file: %s\n", 
	      realFName.c_str());
    }
}

void fileReqObj::execReTimeout()
{
  char execstr[1024];

  if (reTimeoutExecStr.size())
    {
      sprintf(execstr, "%s %s ", reTimeoutExecStr.c_str(), realFName.c_str());
      if (userString.size())
	strcat(execstr, userString.c_str());
      if (debug)
	fprintf(stdout, 
		"FAMReqObj::execReTimeout Running reTimeout command: %s\n", 
		execstr);
      system(execstr);
    }
  // if reTimeoutExecStr not defined, use firstTimeoutExecStr if defined
  else if (firstTimeoutExecStr.size())
    {
      sprintf(execstr, "%s %s ", firstTimeoutExecStr.c_str(), 
	      realFName.c_str());
      if (userString.size())
	strcat(execstr, userString.c_str());
      if (debug)
	fprintf(stdout, 
		"FAMReqObj::execReTimeout Running firstTimeout command: %s\n", 
		realFName.c_str());
      system(execstr);
    }
  else
    {
      fprintf(stdout, "FAMReqObj::execReTimeout - No Command - Timeout on file: %s\n", 
	      execstr);
    }
}

void fileReqObj::resetDoEventTime(time_t timenow)
{
  if (!timenow) timenow = time(0);
  doEventTime = timenow + doEventDelay;
  _eventDone = false;
}
    
bool fileReqObj::checkDoEventTime(time_t timenow, bool runit)
{
  bool doeventtime = false;
  if (!_eventDone && doEventTime)
    {
      if (!timenow) timenow = time(0);
      doeventtime = timenow >= doEventTime;
      if (doeventtime && runit)
	doEvent();
    }
  return doeventtime;
}
  
void fileReqObj::doEvent()
{
#ifndef COMPILE_FAM_MON_MAIN
  if (fReader)
    doFileReader();
  else
#endif
    execEvent();
  isTimedOut = false;
  lastEventTime = time(0);
  eventDone();
}

void fileReqObj::eventDone()
{
  doEventTime = 0; // this event handled, clear timer
  _eventDone = true;  // this event handled
}

void fileReqObj::doFileReader()
{
#ifndef COMPILE_FAM_MON_MAIN
  fReader->readFile((char *)realFName.c_str());
#endif
}

void fileReqObj::execEvent()
{
  char execstr[1024];
  char filestring[1024];
  
  strncpy(filestring, realFName.c_str(), 512);
  if (userString.size())
    strcat(filestring, userString.c_str());
  if (eventExecStr.size())
    {
      sprintf(execstr, "%s %s", eventExecStr.c_str(), filestring);
      if (debug)
	fprintf(stdout, "FAMReqObj::execEvent Running command: %s\n",
		execstr);
      system(execstr);
    }
  else
    {
      fprintf(stdout, "FAMReqObj::execEvent Event on file: %s\n", 
	      filestring);
    }
}

void fileReqObj::setTimeout(time_t newtmout)
{
  timeout = newtmout;
  resetTimeout();
}

void fileReqObj::setFName(char *newfname)
{
  if (newfname)
    reqFName = newfname;
  else
    reqFName.clear();
}

#ifndef COMPILE_FAM_MON_MAIN
fileReaderRapic* fileReqObj::addRapicReader(char *initstr)
{
  if (!initstr) return NULL;
  if (fReader) delete fReader;
  fReader = 
    new fileReaderRapic(initstr);
  return (fileReaderRapic*)fReader;
}

fileReaderNexrad* fileReqObj::addNexradReader(char *initstr)
{
  if (!initstr) return NULL;
  if (fReader) delete fReader;
  fReader = 
    new fileReaderNexrad(initstr);
  return (fileReaderNexrad*)fReader;
}

fileReaderObsAXF* fileReqObj::addObsAXFReader(char *initstr)
{
  if (!initstr) return NULL;
  if (fReader) delete fReader;
  fReader =
    new fileReaderObsAXF(initstr);
  return (fileReaderObsAXF*)fReader;
}

fileReaderLightningAXF* fileReqObj::addLightningAXFReader(char *initstr)
{
  if (!initstr) return NULL;
  if (fReader) delete fReader;
  fReader =
    new fileReaderLightningAXF(initstr);
  return (fileReaderLightningAXF*)fReader;
}

void fileReqObj::addReader(char *initstr)
{
  if (strstr(initstr, "reader=Nexrad"))
    addNexradReader(initstr);
  else if (strstr(initstr, "reader=Rapic"))
    addRapicReader(initstr);
  else if (strstr(initstr, "reader=ObsAXF"))
    addObsAXFReader(initstr);
  else if (strstr(initstr, "reader=LightningAXF"))
    addLightningAXFReader(initstr);
}
#endif

void fileReqObj::init(char *initstr)
{
  char *argstr;
  char tempstr[256];
  int tempint;

  if (!initstr)
    return;
  if ((argstr = strstr(initstr, "file=")))
    {
      if (sscanf(argstr, "file=%s", tempstr) == 1)
	setFName(tempstr);
    }
  else if ((argstr = strstr(initstr, "ltningurl=")))
    {
      if (sscanf(argstr, "ltningurl=%s", tempstr) == 1)
	setFName(tempstr);
    }
  else if ((argstr = strstr(initstr, "url=")))
    {
      if (sscanf(argstr, "url=%s", tempstr) == 1)
	setFName(tempstr);
    }
  if ((argstr = strstr(initstr, "event_exec=")))
    {
      if (sscanf(argstr, "event_exec=%s", tempstr) == 1)
	eventExecStr = tempstr;
    }

  // default mode uses same exec for both first and subsequent timeouts
  if ((argstr = strstr(initstr, "timeout_exec=")))
    {
      if (sscanf(argstr, "timeout_exec=%s", tempstr) == 1)
	{
	  firstTimeoutExecStr = tempstr;
	}
    }
  // new mode allows different exec for first and then subsequent timeouts
  if ((argstr = strstr(initstr, "first_timeout_exec=")))
    {
      if (sscanf(argstr, "first_timeout_exec=%s", tempstr) == 1)
	firstTimeoutExecStr = tempstr;
    }
  if ((argstr = strstr(initstr, "re_timeout_exec=")))
    {
      if (sscanf(argstr, "re_timeout_exec=%s", tempstr) == 1)
	reTimeoutExecStr = tempstr;
    }

  if ((argstr = strstr(initstr, "timeout=")))
    {
      if (sscanf(argstr, "timeout=%d", &tempint) == 1)
	timeout = tempint;
    }
  if ((argstr = strstr(initstr, "event_exec_delay=")))
    {
      if (sscanf(argstr, "do_event_delay=%d", &tempint) == 1)
	doEventDelay = tempint;
    }
  if ((argstr = strstr(initstr, "do_event_delay=")))
    {
      if (sscanf(argstr, "do_event_delay=%d", &tempint) == 1)
	doEventDelay = tempint;
    }
  if ((argstr = strstr(initstr, "matchstring=")))
    {
      if (sscanf(argstr, "matchstring=%s", tempstr) == 1)
	matchString = tempstr;
    }
  if ((argstr = strstr(initstr, "exclude=")))
    {
      if (!excludeString.size())  // first exclude
	{
	  if (sscanf(argstr, "exclude=%s", tempstr) == 1)
	    excludeString = tempstr;
	}
      // look for further exclude strings
      while ((argstr = strstr(argstr+1, "exclude=")))
	{
	  if (sscanf(argstr, "exclude=%s", tempstr) == 1)
	    excludeStrings.push_back(tempstr);
	}
    }
  if ((argstr = strstr(initstr, "userstring=")))
    {
      if (sscanf(argstr, "userstring=%s", tempstr) == 1)
	userString = tempstr;
    }
  if ((argstr = strstr(initstr, "debug")))
    {
      debug = 1;
      if (sscanf(argstr, "debug=%d", &tempint) == 1)
	debug = tempint;
    }
#ifndef COMPILE_FAM_MON_MAIN
  if (strstr(initstr, "reader="))
    addReader(initstr);
#endif
}

bool fileReqObj::check(time_t timenow)
{
  if (!timenow)
    timenow = time(0);
  if (timedOut(timenow))
    doTimeout();
  return checkDoEventTime(timenow);
}

void fileReqObj::dumpStatus(FILE *dumpfile, char *title)
{
  if (!dumpfile)
    return;
  string reqtype;
  if (isDir)
    reqtype = "Dir Monitor";
  else
    reqtype = "File Monitor";
  char timestr[64], timestr2[64], timestr3[64];
  if (!title)
    title = fileReqObjTitle;
  fprintf(dumpfile, "%s - %s reqFName=%s id=%d type=%s\n"
	  "realFName=%s checkIsDir=%d\n"
	  "lastEventTime=%s doEventDelay=%d eventExecStr=%s\n" 
	  "timeout=%d nextTimeout=%s\n"
	  "isTimedOut=%d firstTimedOut=%s\n"
	  "firstTimeoutExecStr=%s reTimeoutExecStr=%s\n"
	  "matchString=%s debug=%d excludeString=%s\n",
	  title, reqtype.c_str(), reqFName.c_str(), fro_id,
	  getFroTypeStr(froType), realFName.c_str(),
	  checkIsDir, famShortDateString(lastEventTime, timestr), 
	  doEventDelay, eventExecStr.c_str(),
	  int(timeout), famShortTimeString(nextTimeoutTime, timestr2), 
	  isTimedOut, famShortTimeString(firstTimeoutTime, timestr3), 
	  firstTimeoutExecStr.c_str(), reTimeoutExecStr.c_str(),
	  matchString.c_str(), debug, excludeString.c_str());
  if (excludeStrings.size())
    {
      fprintf(dumpfile, " excludeStrings=");
      for (int i = 0; i < int(excludeStrings.size()); i++)
	fprintf(dumpfile, "%s, ", excludeStrings[i].c_str());
    }
#ifndef COMPILE_FAM_MON_MAIN
  if (fReader)
    fReader->dumpStatus(dumpfile,"");
#endif
}

bool fileReqObj::containsExcludeStr(char *bname)
{
  if (excludeString.size() && strstr(bname, excludeString.c_str()))
    return true;
  bool strfound = false;
  if (excludeStrings.size())
    {
      int i = 0;
      while (!strfound && (i < int(excludeStrings.size())))
	{
	  strfound = strstr(bname, excludeStrings[i].c_str());
	  i++;
	}
    }
  return strfound;
}     

void FAMReqObj::init(char *initstr)
{
  char *argstr;
  char tempstr[256];

  if (!initstr)
    return;
  fileReqObj::init(initstr);
  if ((argstr = strstr(initstr, "dir=")))
    {
      if (sscanf(argstr, "dir=%s", tempstr) == 1)
	setDirName(tempstr);
    }
  if ((argstr = strstr(initstr, "events=")))
    {
      fam_decodeEventsStr(argstr+strlen("events="), event_mask);
    }
  if ((argstr = strstr(initstr, "use_event_queue")))
    {
      useEventMap = true;
    }
  int tempint;
  if ((argstr = strstr(initstr, "restart_dirmon_period=")))
    {
      if (sscanf(argstr, "restart_dirmon_period=%d", &tempint) == 1)
	{
	  if (tempint > 0)
	    restartDirMonitorPeriod = tempint * 60;
	  else
	    restartDirMonitorPeriod = 0;
	}
    }
}

void FAMReqObj::setFName(char *newfname)
{
  if (newfname)
    reqFName = newfname;
  else
    reqFName.clear();
  isDir = false;  // this may be changed by monitor method stat
  //  checkIsDir = true; // enable stat call to check if is dir
  checkIsDir = false; // file/dir exist requirements differ, don't check
  // need to allow file monitor to succeed if dir exists, but not file yet
}

void FAMReqObj::setDirName(char *newdirname)
{
  if (newdirname)
    reqFName = newdirname;
  else
    reqFName.clear();
  isDir = true;  // this may be changed by monitor method stat
  //  checkIsDir = true; // still allow stat call to check if is dir
  checkIsDir = false; // override stat call to check if is dir
}

bool FAMReqObj::monitor(FAMConnection *_fc)
{
  if (monitorActive && !monitorFailed)
    return true;  // already monitoring

  monitorActive = true; // monitor start attempt made

  //  ensure fc set up correctly
  if (_fc)
    fc = _fc;
  if (!fc || (fc->fd < 0))
    {
      fprintf(stderr, "FAMReqObj::monitor - error fc=0\n");
      monitorFailed = true;
      return false;
    }

  char realdirname[PATH_MAX];
  char tempfname[PATH_MAX];
  char *_dirname = NULL;
  char *_basename = NULL;
//   struct stat status;
//   bool dirExists = false;
  string reqDirName;

  if (isDir)
    reqDirName = reqFName;
  else
    {
      strncpy(tempfname, (char *)reqFName.c_str(), PATH_MAX);
      _dirname = dirname(tempfname);
      if (_dirname)
	reqDirName = _dirname;
    }
  realFName.clear();
  if (realpath((char *)reqDirName.c_str(), realdirname))
    {
      if (isDir)
	realFName = realdirname;
      else 
	{ // construct realfilename from dir and basename
	  realFName = realdirname;
	  realFName += "/";
	  strncpy(tempfname, (char *)reqFName.c_str(), PATH_MAX);
	  _basename = basename(tempfname);
	  if (_basename)
	    realFName += _basename;
	}
//       if (stat(realdirname, &status) < 0)
// 	{
// 	  status.st_mode = 0;
// 	  monitorFailed = true; // stat failed - no file
// 	  if (debug)
// 	    {
// 	      fprintf(stderr, "FAMReqObj::monitor - stat of dir %s failed\n",
// 		      realFName.c_str());
// 	      perror(NULL);
// 	    }
// 	  return false;
// 	}
//       if (!isDir && 
// 	  ((status.st_mode & S_IFMT) == S_IFDIR))
// 	{
// 	  fprintf(stdout, "File monitor error %s is a directory\n"
// 		  "Switching to monitoring as a directory\n", 
// 		  realFName.c_str());
// 	  isDir = true;
// 	}
      if (isDir)
	{
	  if (FAMMonitorDirectory(fc, realFName.c_str(), &fr, this) == 0)
	    {
	      fprintf(stdout, "Monitoring directory %s, id=%d\n", 
		      realFName.c_str(), fro_id);
	      monitorFailed = false;
	    }
	  else
	    {
	      fprintf(stdout, "Monitoring directory %s FAILED - FAMErrno=%d\n", 
		      realFName.c_str(), FAMErrno);
	      monitorFailed = true;
	    }
	}
      else
	{
	  if (FAMMonitorFile(fc, realFName.c_str(), &fr, this) == 0)
	    {
	      fprintf(stdout, "Monitoring file %s, id=%d\n", 
		      realFName.c_str(), fro_id);
	      monitorFailed = false;
	    }
	  else
	    {
	      fprintf(stdout, "Monitoring file %s FAILED - id=%d FAMErrno=%d\n", 
		      realFName.c_str(), fro_id, FAMErrno);
	      monitorFailed = true;
	    }
	}
    }
  else
    {
      if (isDir)
	fprintf(stdout, "Monitor of directory FAILED id=%d - directory path=%s\n  %s\n",
		fro_id, reqDirName.c_str(), strerror(errno));
      else
	fprintf(stdout, "Monitor of file FAILED id=%d %s - directory path=%s\n  %s\n",
		fro_id, reqFName.c_str(), reqDirName.c_str(), strerror(errno));
      monitorFailed = true;
    }
  resetTimeout();
  if (restartDirMonitorPeriod)
    restartDirMonitorTime = time(0) + restartDirMonitorPeriod;
  return !MonitorFailed();
}
   
 // if monitor had failed, try again, see if file/dir exists now
bool FAMReqObj::retryMonitor(FAMConnection *fc)
{
  return monitor(fc);
}

void FAMReqObj::stopMonitor()
{
  if (fc && (fc->fd >= 0) && monitorActive && !monitorFailed)
    FAMCancelMonitor(fc, &fr);
  monitorActive = false;
  monitorFailed = false;
  nextTimeoutTime = 0;  
}

bool FAMReqObj::restartMonitor()
{
  if (!monitorActive || !isDir || !fc ||
      !restartDirMonitorPeriod)
    return false;
  fprintf(stdout, "FAMReqObj::restartMonitor for id=%d period=%dmins"
	  "Dir path=%s\n",
	  fro_id, int(restartDirMonitorPeriod/60), reqFName.c_str());
  stopMonitor();
  restartDirMonitorTime = time(0) + restartDirMonitorPeriod;
  return monitor(fc);
}
	  
bool FAMReqObj::check(time_t timenow)
{
  if (restartDirMonitorPeriod &&
      time(0) >= restartDirMonitorTime)
    restartMonitor();
  return fileReqObj::check();
}

bool FAMReqObj::timedOut(time_t timenow, bool runit)
{
  bool timedout = false;
  if (timeout && nextTimeoutTime && monitorActive)
    {
      if (!timenow) timenow = time(0);
      timedout = timenow > nextTimeoutTime;
      if (timedout && runit)
	doTimeout();
    }
  return timedout;
}
      
void FAMReqObj::resetDoEventTime(famEventObj *eventobj,
				 time_t timenow)
{
  if (!eventobj)
    eventobj = &lastEvent;
  int localdelay = 0;
  switch (eventobj->eventCode)   // only delay exec for events that may still be changing
    {
    case FAMChanged :
    case FAMCreated :
      localdelay = doEventDelay;
      break;
    default :
      localdelay = 0;
    }
  if (!timenow) timenow = time(0);
  eventobj->doEventTime = timenow + localdelay;
  eventobj->eventDone = false;
}
    
bool FAMReqObj::checkDoEventTime(famEventObj *eventobj,
				 time_t timenow, bool runit)
{
  if (!eventobj)
    eventobj = &lastEvent;
  bool doeventtime = false;
  if (!eventobj->eventDone && eventobj->doEventTime && monitorActive)
    {
      if (!timenow) timenow = time(0);
      doeventtime = timenow >= eventobj->doEventTime;
      if (doeventtime && runit)
	doEvent(eventobj);
    }
  return doeventtime;
}
  
bool FAMReqObj::checkDoEventTime(time_t timenow, bool runit)
{  
  if (useEventMap)
    {
      int mapsize = eventMap.size();
      std::map<std::string, famEventObj>::iterator iter = eventMap.begin();
      bool anEventDone = false;
      while (iter != eventMap.end())
	{
	  if (checkDoEventTime(&((iter++)->second), timenow, runit))
	    anEventDone = true;
	}
      if (debug && (mapsize != int(eventMap.size())))
	fprintf(stdout, "FAMReqObj::checkDoEventTime map size changed from"
		" %d to %d\n",
		mapsize, int(eventMap.size()));
      return anEventDone;
    }
  else
    return checkDoEventTime(&lastEvent, timenow, runit);
}

void FAMReqObj::doEvent(famEventObj *eventobj)
{
#ifndef COMPILE_FAM_MON_MAIN
  if (fReader)
    doFileReader(eventobj);
  else
#endif
    execEvent(eventobj);
  isTimedOut = false;
  eventDone(eventobj);
}

void FAMReqObj::doFileReader(famEventObj *eventobj)
{
#ifndef COMPILE_FAM_MON_MAIN
  char filestring[1024];
  
  if (!eventobj)
    eventobj = &lastEvent;
  if (eventobj->eventFilename.size())
    {
      if (isDir && (eventobj->eventFilename[0] != '/')) 
	// event generated from a dir monitor, and isn't the dir name
// 	sprintf(filestring, "%s/%-64s %s ", realFName.c_str(), 
	sprintf(filestring, "%s/%s", realFName.c_str(), 
		eventobj->eventFilename.c_str());
      else
	sprintf(filestring, "%s", eventobj->eventFilename.c_str());
      fReader->readFile(filestring);
    }
#endif
}

void FAMReqObj::execEvent(famEventObj *eventobj)
{
  char execstr[1024];
  char filestring[1024];
  
  if (!eventobj)
    eventobj = &lastEvent;
  if (eventobj->eventFilename.size())
    {
      if (isDir && (eventobj->eventFilename[0] != '/')) 
	// event generated from a dir monitor, and isn't the dir name
// 	sprintf(filestring, "%s/%-64s %s ", realFName.c_str(), 
	sprintf(filestring, "%s/%s %s ", realFName.c_str(), 
		eventobj->eventFilename.c_str(),
		fam_event_name(eventobj->eventCode));
      else
// 	sprintf(filestring, "%-64s %s ", lastEventFilename.c_str(), 
	sprintf(filestring, "%s %s ", eventobj->eventFilename.c_str(), 
		fam_event_name(eventobj->eventCode));
      if (userString.size())
	strcat(filestring, userString.c_str());
      if (eventExecStr.size())
	{
	  sprintf(execstr, "%s %s", eventExecStr.c_str(), filestring);
	  if (debug)
	    fprintf(stdout, "FAMReqObj::execEvent id=%d Running command: %s\n",
		    fro_id, execstr);
	  system(execstr);
	}
      else
	{
	  fprintf(stdout, "FAMReqObj::execEvent Event on file: %s\n", 
		  filestring);
	}
      eventobj->eventFilename.clear();
    }
}

void FAMReqObj::processFAMEvent(FAMEvent *fe, 
				bool timestamp, bool localtime, 
				bool timestamp_timet)
{
  time_t timenow;
  char timestampstr[64];
  char eventstring[512];
  char *basec = NULL, *bname = NULL;
  famEventObj newevent;
  if (!fe)
    return;
  timenow = time(0);
  if (timestamp || timestamp_timet || localtime)
    {
      timestampstr[0] = 0;  // clear string
      if (timestamp || localtime)
	{
	  famShortDateString(timenow, timestampstr, localtime);
	}
      if (timestamp_timet)
	sprintf(timestampstr, "%u", (unsigned int)timenow);
    }
  if (isDir && (fe->filename[0] != '/')) // event generated from a dir monitor, and isn't the dir name
//     sprintf(eventstring, "%s/%-32s %s %s", realFName.c_str(), 
    sprintf(eventstring, "%s/%s %s %s", realFName.c_str(), 
	    fe->filename, fam_event_name(fe->code), timestampstr);
  else
//     sprintf(eventstring, "%-32s %s %s", fe->filename, 
    sprintf(eventstring, "%s %s %s", fe->filename, 
	    fam_event_name(fe->code), timestampstr);
  if (isDir)
    {
      basec = strdup(fe->filename);
      bname = basename(basec);
    }
  if (event_mask[fe->code])// event code must be in event_mask 
    {
      if (!isDir ||               // only use match & exclude for dir
	  !strstr(bname, ".done") &&  // ignore .done flag files
	  (!matchString.size() || // and either no matchString
	   // or matchstring is present in file basename (not dir)
	  (strstr(bname, matchString.c_str()))) && 
	  // and either no exclude string
	  (!containsExcludeStr(bname)))
	{
	  if (debug)
	    fprintf(stdout, "FAMReqObj::processFAMEvent Matching event detected - id=%d\n%s\n", 
		    fro_id, eventstring);
	  newevent.eventFilename = fe->filename;
	  newevent.eventCode = fe->code;
	  newevent.eventTime = timenow;
	  newevent.eventDone = false;
	  resetDoEventTime(&newevent, timenow);   // reset the event run time for this event
	  if (useEventMap)
	    {
	      checkDoEventTime(&newevent, timenow);  // run now if dotimeout is 0
	      if (!newevent.eventDone)
		mapNewEvent(newevent);
	    }
	  else
	    {
	      if (debug && !lastEvent.eventDone &&
		  (lastEvent.eventFilename != newevent.eventFilename))
		fprintf(stdout, "FAMReqObj::processFAMEvent - replacing event"
			" %s with %s before it was done\n",
			lastEvent.eventFilename.c_str(), 
			newevent.eventFilename.c_str());
	      lastEvent = newevent;
	      checkDoEventTime(timenow);  // run now if dotimeout is 0
	    }
	  resetTimeout(timenow);         // reset the timeout for this event
	}
      else if (debug > 1)
	fprintf(stdout, 
		"FAMReqObj::processFAMEvent  %s didn't match string(%s),"
		" or contained exclude string (%s) or .done - ignoring id=%d\n%s\n", 
		bname, matchString.c_str(), excludeString.c_str(), fro_id, 
		eventstring);
    }
  else if (debug)
    fprintf(stdout, "FAMReqObj::processFAMEvent Event didn't match event mask, ignored id=%d\n%s\n", fro_id, eventstring);
  lastEventTime = timenow;
  if (basec)
    free(basec);
}

bool FAMReqObj::mapNewEvent(famEventObj &newevent, time_t timenow)
{
  std::map<std::string, famEventObj>::iterator iter =
    eventMap.find(newevent.eventFilename);
  if (iter != eventMap.end()) // event already exists, update doEvent time
    {
      if (debug)
	fprintf(stdout, "FAMReqObj::mapNewEvent - event exists"
		", updating doEventTime, %s\n",
		newevent.eventFilename.c_str());
      resetDoEventTime(&(iter->second), timenow);
      return false;
    }
  else
    {
      if (debug)
	fprintf(stdout, "FAMReqObj::mapNewEvent - Adding event to map %s\n",
		newevent.eventFilename.c_str());
      newevent.eventMapped = true;
      eventMap[newevent.eventFilename] = newevent;
      return true;
    }
}

 void FAMReqObj::eventDone(famEventObj *eventobj)
{
  if (!eventobj)
    eventobj = &lastEvent;
  if (useEventMap && (eventobj->eventMapped))
    {
      std::map<std::string, famEventObj>::iterator iter =
	eventMap.find(eventobj->eventFilename);
      if (iter != eventMap.end()) // event already exists, update doEvent time
	{
	  if (debug)
	    fprintf(stdout, "FAMReqObj::eventDone - Unmapping event %s\n",
		    eventobj->eventFilename.c_str());
	  eventMap.erase(iter);
	  eventobj->eventMapped = false;
	}
      else
	{
	  fprintf(stderr, 
		  "FAMReqObj::eventDone - failed to unmap famEventObj %s\n",
		  eventobj->eventFilename.c_str());
	}
    }
  eventobj->doEventTime = 0; // this event handled, clear timer
  eventobj->eventDone = true;  // this event handled
}

void FAMReqObj::dumpStatus(FILE *dumpfile, char *title)
{
  if (!dumpfile)
    return;
  string eventmaskstr;
  char timestr[128];
  fam_encodeEventsStr(eventmaskstr, event_mask);
  if (!title)
    title = FAMReqObjTitle;
  fileReqObj::dumpStatus(dumpfile, title);
  fprintf(dumpfile, "fr.reqnum=%d active=%d failed=%d\n"
	  "eventMask=%s useEventMap=%d\n"
	  "restartDirMonitorPeriod=%d, restartDirMonitorTime=%s\n",
	  fr.reqnum, monitorActive, monitorFailed,
	  eventmaskstr.c_str(), useEventMap,
	  int(restartDirMonitorPeriod), 
	  famShortTimeString(int(restartDirMonitorTime), timestr, true));
}

void FAMReqObj::setEventMask(FAMCodes famcode, bool state)
{
  if (famcode < int(event_mask.size()))
  event_mask[famcode] = state;
}

char *famShortTimeString(time_t time, char *outstr, 
		      bool force_gmtime, 
		      bool localtime) {
  struct tm TmStruct;

  if (!outstr) return NULL;
  if (localtime && !force_gmtime) localtime_r(&time, &TmStruct);
  else gmtime_r(&time, &TmStruct);
  sprintf(outstr, "%02d:%02d:%02d %02d/%02d/%04d",
	  TmStruct.tm_hour, TmStruct.tm_min, TmStruct.tm_sec,
	  TmStruct.tm_mday, TmStruct.tm_mon+1, TmStruct.tm_year+1900);
  return outstr;
}

char *famShortDateString(time_t time, char *outstr,
			 bool force_gmtime,
			 bool localtime) {
  struct tm TmStruct;

  if (!outstr) return NULL;
  if (localtime && !force_gmtime) localtime_r(&time, &TmStruct);
  else gmtime_r(&time, &TmStruct);
  sprintf(outstr, "%02d/%02d/%04d %02d:%02d:%02d",
	  TmStruct.tm_mday, TmStruct.tm_mon+1, TmStruct.tm_year+1900,
	  TmStruct.tm_hour, TmStruct.tm_min, TmStruct.tm_sec);
  return outstr;
}

#ifdef COMPILE_FAM_MON_MAIN

int main(int argc, char *argv[])
{
  FAMObject *famobj;

  int i, nmon, rc;
  struct stat status;
  char realfname[PATH_MAX];
  bool timestamp = false;
  bool localtime = false;
  bool timestamp_timet = false;
  tm timestamp_tm;
  char timestampstr[64];
  time_t timenow;

  fprintf(stdout, "\n\nInitialising %s\n\n",argv[0]);

  famobj = new FAMObject(argc, argv);
//   if (famobj && !famobj->famOpenFailed)
//     famobj->init(argc, argv);
  if (!famobj->froCount && !famobj->famOpenFailed)
    {
      fprintf(stderr, "Usage: %s [-timestamp] [-local] [-time_t_stamp] [-i ini_fname] -e exec_str file_or_dir_names\n", argv[0]);
      
      fprintf(stderr, "fam_mon.ini file contains fam request entries as follows\n"
	      "Global settings:\n"
	      "[timestamp] [local] [time_t_stamp] [event_exec=globalexec] [event_exec_delay=globalexecdelay]\n" 
	      "[timeout=globaltmoutperiod] [timeout_exec=globaltmoutexec] [first_timeout_exec=globalfirsttmoutexec] \n"
	      "[re_timeout_exec=globalretmoutexec] [string=globalmatchstring] [debug]\n\n"
	      "Directory form: (Must be one continuous line)\n"
	      "dir=dirname [event_exec=eventexec] [do_event_delay=secs] [timeout=timeoutperiod] [timeout_exec=tmoutexec] \n"
	      "     [string=matchstring] [debug]\n\n"
	      "File form:(Must be one continuous line)\n"
	      "file=filename [event_exec=eventexec] [do_event_delay=secs] [timeout=timeoutperiod] [timeout_exec=tmoutexec]\n"
	      "    [debug]\n"
	      );      
      exit(1);
    }

  /* Loop forever. */
 
  fprintf(stdout, "\nStarting monitoring\n\n",argv[0]);
  
  while (!famobj->famOpenFailed)
    {
      famobj->check();
      sleep(1);
    }

  if (famobj)
    delete famobj;
}

#endif
