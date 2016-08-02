/*

  urlReqObj.C

  Implementation of urlReqObj class

  The urlReqObj checks for an update of the url's header filetime
  then after the doEventDelay the urlReqObj will be added by the 
  fileReaderManager's addCurlMulti
  When the url's data file download is complete the urlReqObj's 
  doFileReader method will called by the fileReaderManager

*/

#include "fam_mon.h"

urlReqObj::~urlReqObj()
{
  if (inCurlMulti && curlMultiRef)
    curl_multi_remove_handle(curlMultiRef, curlHandle);
  curl_easy_cleanup(curlHandle);
  if (curlErrorStr)
    delete[] curlErrorStr;
  if (headerFile)
    fclose(headerFile);
  if (dataFile)
    fclose(dataFile);
}

void urlReqObj::init(char *initstr)
{
  int tempint;
  char *argstr;

  if (!initstr)
    return;
  fileReqObj::init(initstr);
  if ((argstr = strstr(initstr, "poll_period=")))
    {
      if (sscanf(argstr, "poll_period=%d", &tempint) == 1)
	pollPeriod = tempint;
    }
  if ((argstr = strstr(initstr, "connect_timeout=")))
    {
      if (sscanf(argstr, "transfer_timeout=%d", &tempint) == 1)
	transferTimeout = tempint;
    }
//   if ((argstr = strstr(initstr, "show_progress")))
//     showProgress = true;
}

char* urlReqObj::writeFileName(char *strbuff)
{
  if (!strbuff) return strbuff;
  sprintf(strbuff, "urlReq_%d", fro_id);
  if (fReader)
    {
      strcat(strbuff, "_");
      strcat(strbuff, fReader->typeStr());
    }
  return strbuff;
}


void urlReqObj::setCurlHandle(char *urlfname)
{
  if (!curlHandle)
    curlHandle = curl_easy_init();
  if (!curlHandle) return;
  if (debug > 1)
    curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);
//   if (showProgress)
  curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, NULL);
  if (curlErrorStr) 
    curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, curlErrorStr);
  setCurlURL(urlfname);
  curl_easy_setopt(curlHandle, CURLOPT_DNS_CACHE_TIMEOUT, 30 * 60); // 30 mins
  curl_easy_setopt(curlHandle, CURLOPT_FILETIME, 1); // 30 mins
  curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, transferTimeout); 
  curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 1);// only fetch header 
  curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writeData);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, writeHeader);
  curl_easy_setopt(curlHandle, CURLOPT_WRITEHEADER, this);
  char tempstr[256];
  char tempstr2[256];
  writeFileName(tempstr2);
  sprintf(tempstr, "%s.header", tempstr2);
  headerFileName = tempstr;
  sprintf(tempstr, "%s.data", tempstr2);
  dataFileName = tempstr;
}

void urlReqObj::setCurlURL(char *urlfname)
{
  if (!curlHandle)
    setCurlHandle(urlfname);
  else
    {
      if (!urlfname)
	urlfname = (char*)reqFName.c_str();
      else
	reqFName = urlfname;
      curl_easy_setopt(curlHandle, CURLOPT_URL, reqFName.c_str());
    }
};  

void urlReqObj::setTimeCondition(time_t timevalue)
{
  if (!curlHandle)
    setCurlHandle();
  if (curlHandle)
    {
      curl_easy_setopt(curlHandle, CURLOPT_TIMEVALUE, timevalue);
      if (timevalue)
	curl_easy_setopt(curlHandle, CURLOPT_TIMECONDITION, 
			 CURL_TIMECOND_IFMODSINCE);
      else
	curl_easy_setopt(curlHandle, CURLOPT_TIMECONDITION, 
			 CURL_TIMECOND_NONE);
    }
}

void urlReqObj::closeHeaderFile(char *str)
{
  if (headerFile)
    {
      if (debug) 
	fprintf(stdout, "urlReqObj::closeHeaderFile(%d) %s closing headerFile"
		" writtenSize=%d writeEvents=%d\n",
		fro_id, str,
		int(headerWrittenSize), headerWriteEvents);
      fclose(headerFile);
      headerFile = NULL;
    }
}

void urlReqObj::dumpStatus(FILE *dumpfile, char *title)
{
  fileReqObj::dumpStatus(dumpfile, title);
  fprintf(dumpfile, "headerFileName=%s dataFileName=%s\n"
	  "transferTimeout=%d\n",
	  headerFileName.c_str(), dataFileName.c_str(),
	  int(transferTimeout));
  transferTimes.dumpStatus(dumpfile);
}

void urlReqObj::printStats(FILE *file, int debuglevel)
{
  curl_easy_getinfo(curlHandle, CURLINFO_TOTAL_TIME, &transferTime);
  curl_easy_getinfo(curlHandle, CURLINFO_NAMELOOKUP_TIME, &nameLookupTime);
  curl_easy_getinfo(curlHandle, CURLINFO_CONNECT_TIME, &connectTime);
  curl_easy_getinfo(curlHandle, CURLINFO_PRETRANSFER_TIME, &preTransferTime);
  curl_easy_getinfo(curlHandle, CURLINFO_STARTTRANSFER_TIME, &startTransferTime);
  curl_easy_getinfo(curlHandle, CURLINFO_REDIRECT_TIME, &reDirectTime);
  curl_easy_getinfo(curlHandle, CURLINFO_SPEED_DOWNLOAD, &downloadSpeed);
  curl_easy_getinfo(curlHandle, CURLINFO_SIZE_DOWNLOAD, &downloadSize);
  fprintf(file, 
	  " writtenSize=%d writeEvents=%d\n"
	  " transferTime=%1.1f nameLookupTime=%1.1f connectTime=%1.1f\n"
	  " preTransferTime=%1.1f startTransferTime=%1.1f reDirectTime=%1.1f\n"
	  " downloadSpeed=%1.1f downloadSize=%1.0f\n",
	  int(dataWrittenSize), dataWriteEvents,
	  transferTime, nameLookupTime, connectTime,
	  preTransferTime, startTransferTime, reDirectTime,
	  downloadSpeed, downloadSize);
}

void urlReqObj::closeDataFile(char *str)
{
  if (dataFile)
    {
      if (debug) 
	{
	  fprintf(stdout, "urlReqObj::closeDataFile(%d) %s closing dataFile\n",
		  fro_id, str);
	  printStats(stdout);
	}
      else
	curl_easy_getinfo(curlHandle, CURLINFO_TOTAL_TIME, &transferTime);
      transferTimes.addTime(transferTime);
      fclose(dataFile);
      dataFile = NULL;
    }
}

void urlReqObj::setHeaderMode()
{
  if (debug) fprintf(stdout, "urlReqObj::setHeaderMode(%d) called\n", fro_id);
  curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 1);// don't fetch body
  curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 0);// only fetch header 
  if (dataFile)
    closeDataFile("setHeaderMode");
  if (headerFile)
    closeHeaderFile("setHeaderMode");
}

void urlReqObj::setDataMode()
{
  if (debug) fprintf(stdout, "urlReqObj::setDataMode(%d) called\n", fro_id);
  curl_easy_setopt(curlHandle, CURLOPT_NOBODY, 0);// only fetch header 
  curl_easy_setopt(curlHandle, CURLOPT_HTTPGET, 1);// only fetch header 
  curl_easy_setopt(curlHandle, CURLOPT_HEADER, 0);// don't fetch header 
  if (headerFile)
    closeHeaderFile("setDataMode");
  if (dataFile)
    closeDataFile("setDataMode");
}

bool urlReqObj::urlChanged()
{
  char timestr1[256], timestr2[256];
  time_t filetime = 0;
  double filesize;

  if (!curlHandle)
    setCurlHandle();
  setHeaderMode();
  CURLcode result = curl_easy_perform(curlHandle);
  if (headerFile)
    closeHeaderFile("urlChanged");
  if ((result == CURLE_OK) &&
      (curl_easy_getinfo(curlHandle, CURLINFO_FILETIME, &filetime) ==
       CURLE_OK))
    {
      bool newer = filetime > lastFileTime;
      if (debug && newer)
	fprintf(stdout, "urlReqObj::urlChanged(%d) - Newer detected "
		"last=%s this=%s\n",
		fro_id, ShortTimeString(lastFileTime, timestr1),
		ShortTimeString(filetime, timestr2));
      lastFileTime = filetime;
      curl_easy_getinfo(curlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, 
			&filesize);
      if (filesize != lastFileSize)
	{
	  newer = true;
	  fprintf(stdout, "urlReqObj::urlChanged(%d) - New filesize detected "
		  "last=%1.0f this=%1.0f\n",
		  fro_id, lastFileSize, filesize);
	  lastFileSize = filesize;
	}
      return newer;
    }
  else
    {
      fprintf(stdout, "urlReqObj::urlChanged(%d) - ERROR %s - URL=%s\n",
	      fro_id,
	      curlErrorStr, reqFName.c_str());
      return false;
    }
}

bool urlReqObj::check(time_t timenow)
{
  if (!timenow)
    timenow = time(0);
  if (timenow < nextPollTime) 
    return false;
  else
    nextPollTime = timenow + pollPeriod;
  if (urlChanged())
    {
      if (debug)
	fprintf(stdout, 
		"urlReqObj::check - urlChanged - resetDoEventTime called\n");
      resetDoEventTime(timenow);  // arm doEventTime delay
      return true;
    }
  else
    return false;
}  

size_t urlReqObj::writeHeader(void *buffer, size_t size, 
			    size_t nmemb, void *userp)
{
  if (!buffer || !size || !nmemb || !userp)
    return 0;
  urlReqObj *uro = (urlReqObj*)userp;
  if (!uro->headerFile)
    {
      if (uro->debug) 
	fprintf(stdout, "urlReqObj::writeHeader(%d) opening headerFile\n",
		uro->fro_id);
      uro->headerWrittenSize = 0;
      uro->headerWriteEvents = 0;
      uro->headerFile = fopen(uro->headerFileName.c_str(), "w");
    }
  size_t numwritten = 0;
  if (uro->headerFile)
    numwritten = fwrite(buffer, size, nmemb, uro->headerFile);
  numwritten *= size; // convert items to bytes
  uro->headerWrittenSize += numwritten;
  uro->headerWriteEvents++;
  return numwritten;
}

size_t urlReqObj::writeData(void *buffer, size_t size, 
			  size_t nmemb, void *userp)
{
  if (!buffer || !size || !nmemb || !userp)
    return 0;
  urlReqObj *uro = (urlReqObj*)userp;
  if (!uro->dataFile)
    {
      if (uro->debug) 
	fprintf(stdout, "urlReqObj::writeData(%d) opening dataFile\n",
		uro->fro_id);
      uro->dataWrittenSize = 0;
      uro->dataWriteEvents = 0;
      uro->dataFile = fopen(uro->dataFileName.c_str(), "w");
    }
  size_t numwritten = 0;
  if (uro->dataFile)
    numwritten = fwrite(buffer, size, nmemb, uro->dataFile);
  numwritten *= size; // convert items to bytes
  uro->dataWrittenSize += numwritten;
  uro->dataWriteEvents++;
  if (uro->debug) 
    fprintf(stdout, "w%d", uro->dataWriteEvents);
  return numwritten;
}
    
void urlReqObj::doEvent()
{
  // Don't actually perform event here, 
  // fileReaderManager::checkURLs will add this to curlMulti when 
  // this->checkDoEventTime is true
  // fileReaderManager::checkCurlMulti
  //  will call doFileReader when data file download is done 
  if (debug)
    fprintf(stdout, 
	    "urlReqObj::doEvent called\n");
  eventDone();
}

void urlReqObj::doFileReader()
{
  if (debug)
    fprintf(stdout, 
	    "urlReqObj::doFileReader called\n");
#ifndef COMPILE_FAM_MON_MAIN
  if (fReader)
    fReader->readFile((char *)dataFileName.c_str());
  else
#endif
    execEvent();
  isTimedOut = false;
  lastEventTime = time(0);
}

void urlReqObj::execEvent()
{
  char execstr[1024];
  char filestring[1024];
  
  strncpy(filestring, dataFileName.c_str(), 512);
  if (userString.size())
    strcat(filestring, userString.c_str());
  if (eventExecStr.size())
    {
      sprintf(execstr, "%s %s", eventExecStr.c_str(), filestring);
      if (debug)
	fprintf(stdout, "urlReqObj::execEvent Running command: %s\n",
		execstr);
      system(execstr);
    }
  else
    {
      fprintf(stdout, "urlReqObj::execEvent Event on file: %s\n", 
	      filestring);
    }
}

