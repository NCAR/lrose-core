/*

  Implementation of the ltningUrlReqObj class


*/

#include "ltningUrlReqObj.h"
#include "rdrutils.h"

// ensure ltningDirFName is terminated with a /
void ltningUrlReqObj::setFName(char *newfname)
{
  fileReqObj::setFName(newfname);
  if (reqFName[reqFName.size()-1] != '/')
    reqFName += '/';
  ltningDirFName = reqFName;
};

void ltningUrlReqObj::init(char *initstr)
{
  int tempint;
  char *argstr;

  if (!initstr)
    return;
  urlReqObj::init(initstr);
  if ((argstr = strstr(initstr, "realTimeLoadCount=")))
    {
      if (sscanf(argstr, "realTimeLoadCount=%d", &tempint) == 1)
	realTimeLoadCount = tempint;
    }
  if ((argstr = strstr(initstr, "loadTimePeriod=")))
    {
      if (sscanf(argstr, "loadTimePeriod=%d", &tempint) == 1)
	loadTimePeriod = tempint;
    }
//   if ((argstr = strstr(initstr, "show_progress")))
//     showProgress = true;
}

void ltningUrlReqObj::setMode(ltningUrlReqObj_modes setmode)
{
  char urlstr[512];
  time_t timenow = time(0);
  switch (setmode)
    {
    case LURO_loadingIndex :
      // Set up loading of index.txt file - 
      // When loaded doFileReaderLoadIndex runs will read
      // time file names and adds to timeFileIndex
      // also checks depth of real time files for correct realTimeLoadCount
      // Will then step to LURO_loadingTimeFiles mode
      sprintf(urlstr, "%sindex.txt", ltningDirFName.c_str());
      doEventDelay = 0;
      lastFileTime = 0;
      reqFName = urlstr;
      setCurlURL();
      if (urlChanged())
	{
	  resetDoEventTime(timenow);
	  mode = setmode;
	}
      else
	{
	  fprintf(stdout,
		  "ltningUrlReqObj::setMode - Failed loading %s"
		  "- Switching to real time mode\n",
		  urlstr);
	  setMode(LURO_realTime);
	}
      break;
    case LURO_loadingTimeFiles :
      // setLoadingTimeFiles will set up iterators for time file loading
      // and doFileReaderTimeFiles will start load from most recent back
      // When complete will step to LURO_loadingRealTimeFiles mode
      if (setLoadingTimeFiles(timenow - (loadTimePeriod * 60))) // 
	mode = setmode;
      else
	setMode(LURO_loadingRealTimeFiles);
      break;
    case LURO_loadingRealTimeFiles :
      // setLoadingTimeFiles will set up counters for realtime file loading
      // and start load from oldest forwards to get most recent last      
      // When complete will step to LURO_loadingRealTime mode
      if (setLoadingRealTimeFiles())
	mode = setmode;
      else
	setMode(LURO_realTime);
      break;
    case LURO_realTime :
      sprintf(urlstr, "%slgt_01.axf", ltningDirFName.c_str());
      doEventDelay = 0;
      lastFileTime = 0;
      reqFName = urlstr;
      setCurlURL();
      doEventDelay = realtimeEventDelay;
      mode = setmode;
     break;
    }
}

bool ltningUrlReqObj::checkRealTime(time_t timenow)
{
  return urlReqObj::check(timenow);
}

bool ltningUrlReqObj::checkLoadIndex() // return true if url date changed
{
  return false;
}

bool ltningUrlReqObj::checkLoadRealTime() // return true if url date changed
{
  return false;
}

bool ltningUrlReqObj::checkLoadTimeFiles() // return true if url date changed
{
  return false;
}

bool ltningUrlReqObj::check(time_t timenow)
{
  bool result = false;
  switch (mode)
    {
    case LURO_realTime:
      result = checkRealTime(timenow);
      break;
    case LURO_loadingIndex :
      result = checkLoadIndex();
      break;
    case LURO_loadingRealTimeFiles :
      result = checkLoadRealTime();
      break;
    case LURO_loadingTimeFiles :
      result = checkLoadTimeFiles();
      break;
    }
  return result;
}  

void ltningUrlReqObj::addTimeFileStr(char *str)
{
  if (!str) return;
  int y, m, d, h, mn, s;
  time_t tm;
  if (sscanf(str, "%4d%2d%2d%2d%2d%2d",
	     &y, &m, &d, &h, &mn, &s) == 6)
    {
      tm = DateTime2UnixTime(y, m, d, h, mn, s);
      timeFileIndex.insert(std::make_pair(tm,str));
    }
}

void ltningUrlReqObj::readIndexTxt()
{
  realTimeLoadCount = 0;
  timeFileIndex.clear();
  loadUrl_iter_start = loadUrl_iter_end = timeFileIndex.end();
  FILE *file = fopen((char *)dataFileName.c_str(), "r");
  if (!file)
    return;
  char tempstr[512];
  int tempint;
  while (fgets(tempstr, 512, file))
    {
      if (strstr(tempstr, "lgt_"))
	{
	  if (sscanf(tempstr, "lgt_%d", &tempint) == 1)
	    if (tempint > realTimeLoadCount)
	      realTimeLoadCount = tempint;
	}
      else if (tempstr[0] == '2')
	addTimeFileStr(tempstr);
    }
  fclose(file);    
}

void ltningUrlReqObj::doEvent()
{
  // Don't actually perform event here, 
  // fileReaderManager::checkURLs will add this to curlMulti when 
  // this->checkDoEventTime is true
  // fileReaderManager::checkCurlMulti
  //  will call doFileReader when data file download is done 
  if (debug)
    fprintf(stdout, 
	    "ltningUrlReqObj::doEvent called\n");
  eventDone();
}

bool ltningUrlReqObj::setLoadingTimeFiles(time_t starttime, time_t endtime)
{
  if (!timeFileIndex.size() || !starttime) return false;
  loadUrl_iter_start = timeFileIndex.lower_bound(starttime);
  if (loadUrl_iter_start == timeFileIndex.end())
    return false;
  if (endtime)
    {
      loadUrl_iter_end = timeFileIndex.lower_bound(endtime);
      if (loadUrl_iter_end != timeFileIndex.end())
      // following setNextTimeFile will step back to endtime
	loadUrl_iter_end++; 
    }
  else
    loadUrl_iter_end = timeFileIndex.end();
  return setNextTimeFile();
};

bool ltningUrlReqObj::setNextTimeFile()
{
  if (loadUrl_iter_end == loadUrl_iter_start)  // load is done - return false 
    return false;
  bool urlOK = false;
  char urlstr[512];
  while ((loadUrl_iter_end != loadUrl_iter_start)
	 && !urlOK)
    {
      loadUrl_iter_end--; // step to previous
      sprintf(urlstr, "%s%s", ltningDirFName.c_str(), 
	      loadUrl_iter_end->second.c_str());
      doEventDelay = 0;
      lastFileTime = 0;
      reqFName = urlstr;
      setCurlURL();
      if (urlChanged())
	{
	  fprintf(stdout,
		  "ltningUrlReqObj::setNextTimeFile - URL %s OK - reading\n",
		  urlstr);
	  resetDoEventTime(time(0));
	  urlOK = true;
	}
      else
	{
	  fprintf(stdout,
		  "ltningUrlReqObj::setNextTimeFile - Failed loading %s - skipping to next\n",
		  urlstr);
	}
    }
  return urlOK;
};

/*
  load from oldest realtime file to newest
*/

bool ltningUrlReqObj::setLoadingRealTimeFiles()
{
  if (!realTimeLoadCount)
    return false;
  // allow for setNextRealTimeFile decrement
  realTimeFileNo = realTimeLoadCount + 1;  
  return setNextRealTimeFile();
};

bool ltningUrlReqObj::setNextRealTimeFile()
{  
  if (realTimeFileNo == 1)  // load is done - return false 
    return false;
  bool urlOK = false;
  char urlstr[512];
  while ((realTimeFileNo > 1)
	 && !urlOK)
    {
      realTimeFileNo--; // step to previous
      sprintf(urlstr, "%slgt_%02d.axf", ltningDirFName.c_str(), 
	      realTimeFileNo);
      doEventDelay = 0;
      lastFileTime = 0;
      reqFName = urlstr;
      setCurlURL();
      if (urlChanged())
	{
	  fprintf(stdout,
		  "ltningUrlReqObj::setNextRealTimeFile - URL %s OK - reading\n",
		  urlstr);
	  resetDoEventTime(time(0));
	  urlOK = true;
	}
      else
	{
	  fprintf(stdout,
		  "ltningUrlReqObj::setNextRealTimeFile - Failed loading %s - skipping to next\n",
		  urlstr);
	}
    }
  return urlOK;
};

void ltningUrlReqObj::doFileReader()
{
  if (debug)
    fprintf(stdout, 
	    "urlReqObj::doFileReader called\n");
  switch (mode)
    {
    case LURO_realTime:
      doFileReaderRealTime();
      break;
    case LURO_loadingIndex :
      doFileReaderLoadIndex();
      break;
    case LURO_loadingRealTimeFiles :
      doFileReaderRealTimeFiles();
      break;
    case LURO_loadingTimeFiles :
      doFileReaderTimeFiles();
      break;
    }
  isTimedOut = false;
  lastEventTime = time(0);
}

void ltningUrlReqObj::doFileReaderLoadIndex()
{
  readIndexTxt();
  setMode(LURO_loadingTimeFiles);
}

void ltningUrlReqObj::doFileReaderTimeFiles()
{
  urlReqObj::doFileReader();
  if (!setNextTimeFile())
    setMode(LURO_loadingRealTimeFiles);
}

void ltningUrlReqObj::doFileReaderRealTimeFiles()
{
  urlReqObj::doFileReader();
  if (!setNextRealTimeFile())
    setMode(LURO_realTime);
}  

void ltningUrlReqObj::doFileReaderRealTime()
{
  urlReqObj::doFileReader();
}

void ltningUrlReqObj::dumpStatus(FILE *dumpfile, char *title)
{
  urlReqObj::dumpStatus(dumpfile, title);
  fprintf(dumpfile, "ltningDirFName=%s\n"
	  "realTimeLoadCount=%d loadTimePeriod=%dmins\n"
	  "timeFileIndex size=%d\n",
	  ltningDirFName.c_str(), realTimeLoadCount, loadTimePeriod,
	  int(timeFileIndex.size()));
}

