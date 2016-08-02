/*

  fileReader.C

*/

#include "fileReader.h"
#include "fam_mon.h"
#include "ltningUrlReqObj.h"

char *fileReaderTypeStrings[] = 
  {"Undefined", "Rapic", "Nexrad", "ObsAXF", "LightningAXF"};

fileReaderMngThread *globalFileReaderMng = NULL;

fileReaderMngThread* getGlobalFileReaderMng()
{
  return globalFileReaderMng;
}   

fileReaderMngThread* initGlobalFileReaderMng(char *progname, char *inifile)
{
  if (!globalFileReaderMng)
    {
      globalFileReaderMng = new fileReaderMngThread(progname, inifile);
      globalFileReaderMng->startThread();
    }
  return globalFileReaderMng;
}   

void closeGlobalFileReaderMng()
{
  if (globalFileReaderMng)
    {
      globalFileReaderMng->stopThread();
      delete globalFileReaderMng;
      globalFileReaderMng = NULL;
    }
}

void fileReader::init(char *initstr)
{
  char *argstr = NULL;
  int tempint = 0;
  if ((argstr = strstr(initstr, "debug")))
    {
      debug = 1;
      if (sscanf(argstr, "debug=%d", &tempint) == 1)
	debug = tempint;
    }  
}

char fileReaderTitle[] = "fileReader::dumpStatus";

void fileReader::dumpStatus(FILE *dumpfile, char *title)
{
  if (!title)
    title = fileReaderTitle;
  if (dumpfile)
    fprintf(dumpfile, "%s\n", title);
}

fileReaderManager::fileReaderManager(char *progName, char *faminifile) :
  FAMObject(progName, faminifile)
{
  curlMultiHandle = NULL;
  inMultiCount = 0;
  tempURLDir = "temp";
}

fileReaderManager::~fileReaderManager()
{
  if (curlMultiHandle)
    {
      clearURLs();
      curl_multi_cleanup(curlMultiHandle);
      curlMultiHandle = NULL;
      curl_global_cleanup();
    }
}

void fileReaderManager::init(char *inifile)
{
  FAMObject::init(inifile);
  dumpStatusFName = "fileReader.status";
}

fileReaderType fileReaderManager::getReaderType(char *monstr)
{
  if (!monstr) return frt_Undefined;
  if (strstr(monstr, "reader=Nexrad"))
    return frt_Nexrad;
  else if (strstr(monstr, "reader=Rapic"))
    return frt_Rapic;
  else if (strstr(monstr, "reader=ObsAXF"))
    return frt_ObsAXF;
  else if (strstr(monstr, "reader=LightningAXF"))
    return frt_LightningAXF;
  else
    return frt_Undefined;
} 

void fileReaderManager::setMonDefaults(char *monstr, FAMReqObj *newfro)
{
  if (!monstr || !newfro)
    return;
  // if newfro events NOT defined and default IS defined
  // set fro event_mask to defined defaults
  if (!strstr(monstr, "events=") && 
      defaultEventMask.count())  // only use if some bits set
    newfro->event_mask = defaultEventMask;
  // FileReaders generaly won't use exec strings
  setMonDefaults(monstr, (fileReqObj*)newfro);
}

void fileReaderManager::setMonDefaults(char *monstr, fileReqObj *newfro)
{
  if (!monstr || !newfro)
    return;
  // FileReaders generaly won't use exec strings
  // so don't apply default exec values
  if ((newfro->doEventDelay < 0) &&  defaultDoEventDelay)
    newfro->doEventDelay = defaultDoEventDelay;
  if (newfro->doEventDelay < 0)
    newfro->doEventDelay = 0;
  if (!newfro->timeout &&  defaultTimeout)
    newfro->timeout = defaultTimeout;
  if (!newfro->matchString.size() && 
      defaultMatchString.size())
    newfro->matchString = defaultMatchString;
  if (!newfro->excludeString.size() && 
      defaultExcludeString.size())
    newfro->excludeString = defaultExcludeString;
}

void fileReaderManager::addCurlMulti(urlReqObj *uro)
{
  if (!uro || uro->inCurlMulti || !curlMultiHandle)
    return;
  if (debug)
    fprintf(stdout, "fileReaderManager::addCurlMulti - id=%d\n",
	    uro->fro_id);
  uro->setDataMode();
  curl_multi_add_handle(curlMultiHandle, uro->curlHandle);
  uro->inCurlMulti = true;
  inMultiCount++;
}

void fileReaderManager::removeCurlMulti(urlReqObj *uro)
{
  if (!uro || !uro->inCurlMulti || !curlMultiHandle)
    return;
  
  if (debug)
    fprintf(stdout, "fileReaderManager::removeCurlMulti - id=%d\n",
	    uro->fro_id);
  uro->setHeaderMode();
  curl_multi_remove_handle(curlMultiHandle, uro->curlHandle);
  uro->inCurlMulti = false;
  inMultiCount--;
}

urlReqObj* fileReaderManager::findUrlReqObj(CURL *curlhandle)
{
  urlReqObjsIter iter = urlReqObjs.begin();
  urlReqObjsIter iterend = urlReqObjs.end();
  while ((iter != iterend) &&
	 ((*iter)->curlHandle != curlhandle))
    iter++;
  if (iter != iterend)
    return (*iter);
  else
    return NULL;
} 

void fileReaderManager::removeCurlMulti(CURL *curlhandle)
{
  if (curlhandle || !curlMultiHandle)
    return;
  urlReqObj *uro = findUrlReqObj(curlhandle);
  if (uro)
    removeCurlMulti(uro);
  else
    fprintf(stdout, "fileReaderManager::removeCurlMulti - Unable to find matching urlReqObj\n");
}

void fileReaderManager::checkCurlMulti()
{
  if (!curlMultiHandle || (inMultiCount == 0)) return;
  int running_handles;
  if (debug)
    fprintf(stdout, "c1\n");
  CURLMcode curlmcode = curl_multi_perform(curlMultiHandle, &running_handles);
  if (running_handles != inMultiCount)
    fprintf(stdout, "fileReaderManager::checkCurlMulti - Count mismatch "
	    "running_handles=%d inMultiCount=%d\n",
	    running_handles, inMultiCount);
  int performcount = 1;
  if (debug)
    fprintf(stdout, "c2\n");
  while (curlmcode == CURLM_CALL_MULTI_PERFORM)
    {
      curlmcode = curl_multi_perform(curlMultiHandle, &running_handles);
      performcount++;
      if (debug && running_handles)
	fprintf(stdout, "m");
    }
  int curlMultiPerformLimit = 100;
  while (running_handles && (curlMultiPerformLimit > 0))
    {
      curlmcode = curl_multi_perform(curlMultiHandle, &running_handles);
      performcount++;
      if (debug && running_handles)
	{
	  if (curlmcode == CURLM_CALL_MULTI_PERFORM)
	    fprintf(stdout, "M");
	  else
	    fprintf(stdout, "p");
	}
      curlMultiPerformLimit--;
      sec_delay(0.01);
    }
  if (debug && (performcount > 1))
    fprintf(stdout, "fileReaderManager::checkCurlMulti performcount=%d"
	    " runninghandles=%d\n",
	    performcount, running_handles);
  int msgs_in_queue;
  CURLMsg *result = NULL;
  urlReqObj *uro = NULL;
  bool readCompleted = false;
  while ((result = curl_multi_info_read(curlMultiHandle, &msgs_in_queue)))
    {
      if (result->msg == CURLMSG_DONE)
	{
	  uro = findUrlReqObj(result->easy_handle);
	  if (uro)
	    {
	      uro->closeDataFile("checkCurlMulti");
	      removeCurlMulti(uro);
	      if (result->data.result == CURLE_OK)
		{
		  fprintf(stdout, "fileReaderManager::checkCurlMulti - "
			  "%s READ OK\n", uro->reqFName.c_str());
		  //	      uro->printStats(stdout, debug);
		}
	      else
		fprintf(stdout, "fileReaderManager::checkCurlMulti - "
			"%s READ FAILED %d %s\n", uro->reqFName.c_str(),
			int(result->data.result), uro->curlErrorStr);
	      readCompleted = true;
	    }
	  uro->doFileReader();  // perform fileReader or exec action
	}
    }  
  if (readCompleted)
    FAMObject::dumpStatus();
}

void fileReaderManager::checkURLs()
{
  urlReqObjsIter iter = urlReqObjs.begin();
  urlReqObjsIter iterend = urlReqObjs.end();
  urlReqObj *uro = NULL;
  while (iter != iterend)
    {
      if (*iter && 
	  ((*iter)->froType == FRO_URL) ||
	  ((*iter)->froType == FRO_LTNING))
	{
	  uro = *iter;
	  if (!uro->inCurlMulti)
	    uro->check();       // check for urlChanged
	  if (!uro->inCurlMulti && 
	      uro->checkDoEventTime()) // if doEventTime - add to curlMulti
	    addCurlMulti(uro);         // for data file download
	}                              
      iter++;
    }
  checkCurlMulti();     // when data download complete calls uro doFileReader
}

void fileReaderManager::newURLReqEntry(char *monstr)
{
  if (!monstr) return;
  urlReqObj *newfro = new urlReqObj(monstr, froCount+1);
  if (!add_fro(newfro))
    delete newfro;   // duplicate, not added, delete it now
}

void fileReaderManager::newLtningURLReqEntry(char *monstr)
{
  if (!monstr) return;
  ltningUrlReqObj *newfro = new ltningUrlReqObj(monstr, froCount+1);
  if (!add_fro((urlReqObj*)newfro))
    delete newfro;   // duplicate, not added, delete it now
}

bool fileReaderManager::add_fro(urlReqObj *monfro) 
{                                            
  if (!monfro) return false;
  if (!curlMultiHandle)
    {
      curl_global_init(CURL_GLOBAL_ALL);
      curlMultiHandle = curl_multi_init();
    }
  monfro->curlMultiRef = curlMultiHandle;
  urlReqObjs.push_back(monfro);
  froCount++;
  monfro->fro_id = froCount;
  return true;
}

void fileReaderManager::clearURLs()
{
  urlReqObjsIter iter = urlReqObjs.begin();
  urlReqObjsIter iterend = urlReqObjs.end();
  while(iter != iterend)
    {
      remove_fro(iter, false);
      iter++;
    }
  urlReqObjs.clear();
}

void fileReaderManager::remove_fro(urlReqObj *monfro, bool do_erase) 
{                     
  urlReqObjsIter iter = fro_exists(monfro);
  if (iter != urlReqObjs.end())
    remove_fro(iter, do_erase);
}

void fileReaderManager::remove_fro(urlReqObjsIter &iter, bool do_erase) 
{
  if (iter == urlReqObjs.end()) return;
  // if monfro in curl_multi stack remove it
  delete *iter;
  if (do_erase)
    urlReqObjs.erase(iter);
  froCount--;
}

urlReqObjsIter fileReaderManager::fro_exists(urlReqObj *monfro)
{    
  urlReqObjsIter iter = urlReqObjs.begin();
  urlReqObjsIter iterend = urlReqObjs.end();
  if (!monfro) return iterend;
  while ((iter != iterend) &&
	 (*iter != monfro))
    {
      iter++;
    }
  if (iter != iterend)
    return iter;
  else
    return iterend;
}

void fileReaderManager::check() // check urlReqObjs and then FAMObject 
{
  checkURLs();
  FAMObject::check();
}

char fileReaderMngTitle[] = "fileReaderManager::dumpStatus";

void fileReaderManager::dumpStatus(FILE *dumpfile)
{
  dumpStatus(dumpfile, fileReaderMngTitle);
}

void fileReaderManager::dumpStatus(FILE *dumpfile, char *title)
{
  if (!dumpfile)
    return;
  if (!title)
    title = fileReaderMngTitle;
  FAMObject::dumpStatus(dumpfile, title);
  fprintf(dumpfile, "inMultiCount=%d tempURLDir=%s\n",
	  inMultiCount, tempURLDir.c_str());
  urlReqObjsIter iter = urlReqObjs.begin();
  urlReqObjsIter iterend = urlReqObjs.end();
  while (iter != iterend)
    {
      (*iter)->dumpStatus(dumpfile, title);
      fprintf(dumpfile, "\n");
      iter++;
    }
}

fileReaderMngThread::fileReaderMngThread(char *progName, char *faminifile) : 
  ThreadObj()
{
  fileReaderMng = new fileReaderManager(progName, faminifile);
  doneInit = false;
}

fileReaderMngThread::~fileReaderMngThread()
{
  if (fileReaderMng)
    delete fileReaderMng;
}

void fileReaderMngThread::workProc()
{
  if (!doneInit && fileReaderMng && !fileReaderMng->famOpenFailed)
    {
      fileReaderMng->init();  
      doneInit = true;
    }
  if (fileReaderMng)
    {
      fileReaderMng->check();
      if (fileReaderMng->inMultiCount > 0)
	setLoopDelay(0.01);
      else
	setLoopDelay(1.0);
    }
}

void fileReaderMngThread::threadInit()
{
}

void fileReaderMngThread::threadExit()
{
}

