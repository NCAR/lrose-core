#include "obsData.h"
#include <stack>
#include "utils.h"

obsStnMap *globalObsStnMap = NULL;
obsDataMng *globalObsData = NULL;
bool reloadDrawObsSettings = false;
bool reloadDrawLtningSettings = false;
char obsDataSettingsReloadFname[] = "obsDataSettings.ini.reload";

char *obsTypeStr[] = {"base", "fixed", "moving"};
char obstype_undef[] = "undef";

char *getObsTypeStr(obsType obstype)
{
  if ((obstype >= ot_base) &&
      (obstype <= ot_moving))
    return obsTypeStr[obstype];
  else
    return obstype_undef;
}

obsStnMap* getGlobalObsStnMap()
{
  if (!globalObsStnMap)
    globalObsStnMap = new obsStnMap();
  return globalObsStnMap;
}

obsStn* getGlobalObsStn(int _stn)
{
  obsStn* obsstn = NULL;
  obsStnMap* obsstnmap = getGlobalObsStnMap();
  if (obsstnmap)
    obsstn = obsstnmap->getObsStn(_stn);
  return obsstn;
}

obsDataMng* getGlobalObsData(bool autoCreate)
{
  if (!globalObsData && autoCreate)
    globalObsData = new obsDataMng();
  return globalObsData;
}

char *shObsStr(short val, int scale,  char *precstr, char *outstr) 
{
  if (shObsUndef(val) || !outstr)
    return obstype_undef;
  sprintf(outstr, precstr, float(val)/float(scale));
  return outstr;
}   

baseObsData::baseObsData()
{
  init();
}

baseObsData::baseObsData(time_t _tm, int _stn,
			 float ws, float wd, float tmp,  // required fields
			 float wg, float dp,  // optional fields 
			 float spress)
{
  set(_tm, _stn, ws, wd, tmp, wg, dp, spress);
}

baseObsData::~baseObsData()
{
}

void baseObsData::init()
{
  obstype = ot_base;
  tm = 0;
  _ws = _wd = _wg = 
    _temp = _dp = -9999;
  _spress = 0;
  stn = -1;
  valid = false;
}

void baseObsData::set(time_t _tm, int _stn,
		      float ws, float wd, float tmp,  // required fields
		      float wg, float dp, // optional fields
		      float spress)  
{
  setWS(ws);
  setWD(wd);
  setTemp(tmp);
  if (wg >= 0.0)
    setWG(wg);
  if (dp != -9999.0)
    setDP(dp);
  if (spress > 0.0)
    setSPress(spress);
  setTm(_tm);
  setStn(_stn);
  valid = true;
}

void baseObsData::dumphdr(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  fprintf(dumpfile, "stn   time                temp  dp    wd  ws  wg  press");
  if (endline) 
    fprintf(dumpfile, "\n");
}

void baseObsData::dump(FILE *dumpfile, bool endline)
{
  char timestr[128];
  if (!dumpfile) return;
//   fprintf(dumpfile, "%5d %s %5.1f %5.1f %3d %3d %3d %6.1f",
// 	  stn, ShortTimeString(tm, timestr, true), 
// 	  getTemp(),getDP(), int(getWD()), int(getWS()), int(getWG()),
// 	  getSPress());
  char tempstr[12], dpstr[12], wdstr[12], 
    wsstr[12], wgstr[12],spstr[12];
  fprintf(dumpfile, "%5d %s %s %s %s %s %s %s",
	  stn, ShortTimeString(tm, timestr, true), 
	  shObsStr(_temp, 100, "%5.1f", tempstr),
	  shObsStr(_dp, 100, "%5.1f", dpstr),
	  shObsStr(_wd, 10, "%3.0f", wdstr),
	  shObsStr(_ws, 100, "%3.0f", wsstr),
	  shObsStr(_wg, 100, "%3.0f", wgstr),
	  shObsStr(_spress, 10, "%6.1f", spstr));
  if (endline) 
    fprintf(dumpfile, "\n");
}

fixedObsData::fixedObsData()
{
  init();
}

void fixedObsData::init()
{
  baseObsData::init();
  obstype = ot_fixed;
  _rain9am = 0;
  _rain10 = 0;
}

fixedObsData::fixedObsData(time_t _tm, int _stn,
			   float ws, float wd, float tmp,  // required fields
			   float rain9am, float rain10,
			   float wg, float dp, // optional fields 
			   float spress) 
{
  set(_tm, _stn, ws, wd, tmp, rain9am, rain10, wg, dp, spress);
}

fixedObsData::~fixedObsData()
{
}

void fixedObsData::set(time_t _tm, int _stn,
		       float ws, float wd, float tmp,  // required fields
		       float rain9am, float rain10,
		       float wg, float dp,  // optional fields 
		       float spress)
{
  baseObsData::set(_tm, _stn, ws, wd, tmp, wg, dp, spress);
  setRain9am(rain9am);
  setRain10(rain10);
}

void fixedObsData::dumphdr(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  baseObsData::dumphdr(dumpfile, false);
  fprintf(dumpfile, " rain9am rain10m");
  if (endline) 
    fprintf(dumpfile, "\n");
}

void fixedObsData::dump(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  baseObsData::dump(dumpfile, false);
  char r9str[12], r10str[12];
  fprintf(dumpfile, " %s %s", 
	  shObsStr(_rain9am, 100, "%7.1f", r9str),
	  shObsStr(_rain10, 10, "%7.1f", r10str));
  if (endline) 
    fprintf(dumpfile, "\n");
}

movingObsData::movingObsData()
{
  init();
}

void movingObsData::init()
{
  baseObsData::init();
  obstype = ot_moving;
  lat = lng = ht = 0.0;
}

void movingObsData::dumphdr(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  baseObsData::dumphdr(dumpfile, false);
  fprintf(dumpfile, " lat    lng     ht");
  if (endline) 
    fprintf(dumpfile, "\n");
}

void movingObsData::dump(FILE *dumpfile, bool endline)
{
  if (!dumpfile) return;
  baseObsData::dump(dumpfile, false);
  fprintf(dumpfile, " %7.3f %7.3f %5.1f", 
	  lat, lng, ht);
  if (endline) 
    fprintf(dumpfile, "\n");
}

bool useObsDataFreelist = true;
std::stack<baseObsData*> baseObsDataFreeStack;
std::stack<fixedObsData*> fixedObsDataFreeStack;
std::stack<movingObsData*> movingObsDataFreeStack;
std::stack<lightningData*> lightningDataFreeStack;
spinlock *obsDataFreeListLock = NULL;

bool lockObsDataFreeListLock()
{
  if (obsDataFreeListLock == NULL)
    obsDataFreeListLock = new spinlock("obsDataFreeListLock", float(1.0));
  if (obsDataFreeListLock)
    return obsDataFreeListLock->get_lock();
  else
    return false;
}

void unlockObsDataFreeListLock()
{
  if (obsDataFreeListLock)
    obsDataFreeListLock->rel_lock();
}

// return new baseObsData from freelist if avail else alloc
baseObsData* newBaseObsData()
{
  if (useObsDataFreelist &&
      baseObsDataFreeStack.size())
    {
      lockObsDataFreeListLock();
      baseObsData* retval = baseObsDataFreeStack.top();
      baseObsDataFreeStack.pop();
      unlockObsDataFreeListLock();
      return retval;
    }
  else
    return new baseObsData();
}

// return new fixedObsData from freelist if avail else alloc
fixedObsData* newFixedObsData()
{
  if (useObsDataFreelist &&
      fixedObsDataFreeStack.size())
    {
      lockObsDataFreeListLock();
      fixedObsData* retval = fixedObsDataFreeStack.top();
      fixedObsDataFreeStack.pop();
      unlockObsDataFreeListLock();
      return retval;
    }
  else
    return new fixedObsData();
}

// return new movingObsData from freelist if avail else alloc
movingObsData* newMovingObsData()
{
  if (useObsDataFreelist &&
      movingObsDataFreeStack.size())
    {
      lockObsDataFreeListLock();
      movingObsData* retval = movingObsDataFreeStack.top();
      movingObsDataFreeStack.pop();
      unlockObsDataFreeListLock();
      return retval;
    }
  else
    return new movingObsData();
}

// add fixedObsData instance to freelist
void freeObsData(baseObsData *freeobs)
{
  if (!freeobs) return;   // don't free null
  if (useObsDataFreelist)
    {
      lockObsDataFreeListLock();
      switch (freeobs->obstype)
	{
	case ot_base : 
	  baseObsDataFreeStack.push(freeobs);
	  break;
	case ot_fixed :
	  fixedObsDataFreeStack.push((fixedObsData *)freeobs);
	  break;
	case ot_moving :
	  movingObsDataFreeStack.push((movingObsData *)freeobs);
	  break;
	default :
	  delete freeobs;  // if type not supported - delete it
	}
      unlockObsDataFreeListLock();
    }
  else
    delete freeobs;
}

// return lightningData from freelist if avail else alloc
lightningData* newLightningData()
{
  if (useObsDataFreelist &&
      lightningDataFreeStack.size())
    {
      lockObsDataFreeListLock();
      lightningData* retval = lightningDataFreeStack.top();
      lightningDataFreeStack.pop();
      unlockObsDataFreeListLock();
      return retval;
    }
  else
    return new lightningData();
}

// add lightningData instance to freelist
void freeLightningData(lightningData *freeobs)
{
  if (!freeobs) return;   // don't free null
  if (useObsDataFreelist)
    {
      lockObsDataFreeListLock();
      lightningDataFreeStack.push(freeobs);
      unlockObsDataFreeListLock();
    }
  else
    delete freeobs;
}

// clear obsDataFreeList
void ClearFreeObsData()
{
  lockObsDataFreeListLock();
  while (baseObsDataFreeStack.size())
    {
      delete baseObsDataFreeStack.top();
      baseObsDataFreeStack.pop();
    }
  while (fixedObsDataFreeStack.size())
    {
      delete fixedObsDataFreeStack.top();
      fixedObsDataFreeStack.pop();
    }
  while (movingObsDataFreeStack.size())
    {
      delete movingObsDataFreeStack.top();
      movingObsDataFreeStack.pop();
    }
  unlockObsDataFreeListLock();
}

int freeObsDataSize()
{
  return 
    baseObsDataFreeStack.size() * sizeof(baseObsData) +
    fixedObsDataFreeStack.size() * sizeof(fixedObsData) +
    movingObsDataFreeStack.size() * sizeof(movingObsData);
}

stnObsData::stnObsData(int stn)
{
  mapLock = new spinlock("stnObsData", float(5.0));
  init();
  stnID = stn;
  _obsstn = NULL;
  obsstn();
}

stnObsData::~stnObsData()
{
  if (mapLock) delete mapLock;
  clear();
}

obsStn* stnObsData::obsstn()
{
  if (!_obsstn)
    if (getGlobalObsStnMap())
      _obsstn = getGlobalObsStnMap()->getObsStn(stnID);
  return _obsstn;
}

void stnObsData::init()
{
  check();
}

void stnObsData::check(bool uselock)
{
  if (uselock)
    mapLock->get_lock();
  obsCount = obsDataMap.size();
  if (obsCount)
    {
      obsDataMap_iter iter = obsDataMap.begin();
      firstTm = iter->first;
      obsDataMap_iter iterend = obsDataMap.end();
      iterend--;
      lastTm = iterend->first;
    }
  else
    firstTm = lastTm = 0;
  if (uselock)
    mapLock->rel_lock();
};    

void stnObsData::clear()
{
  mapLock->get_lock();  
  obsDataMap_iter iter = obsDataMap.begin();
  obsDataMap_iter iterend = obsDataMap.end();
  
  while (iter != iterend)
    {
      if (iter->second)
	freeObsData(iter->second);
      iter++;
    }
  obsDataMap.clear();
  mapLock->rel_lock();
  check();
}

bool obsDataDebug = true;

void stnObsData::purgeBefore(time_t beforetm)
{
  char beforetmstr[64],
    beginstr[64], laststr[64],
    beforestr[64], iterstr[64];

  if (!obsDataMap.size())
    return;

  mapLock->get_lock();  
  obsDataMap_iter iter = obsDataMap.begin();
  obsDataMap_iter iterend = obsDataMap.end();
  obsDataMap_iter iterlast = iterend;
  iterlast--;
  if (obsDataDebug)
    {
      ShortTimeString(beforetm, beforetmstr);
      ShortTimeString(iter->first, beginstr);
      ShortTimeString(iterlast->first, laststr);
    }
  if (iter->first >= beforetm)
    {
      mapLock->rel_lock();
      return;
    }
  obsDataMap_iter iter_gte_beforetm = 
    obsDataMap.lower_bound(beforetm); // first entry >= time
  if (obsDataDebug)
    {
      ShortTimeString(iter_gte_beforetm->first, beforestr);
    }
  
  while ((iter != iterend) &&
	 (iter != iter_gte_beforetm))
    {
      if (obsDataDebug)
	ShortTimeString(iter->first, iterstr);
      if (iter->second)
	freeObsData(iter->second);
      iter++;
    }
  obsDataMap.erase(obsDataMap.begin(), iter);
  mapLock->rel_lock();
  check();
}

void stnObsData::purgeAfter(time_t aftertm)
{
  if (!obsDataMap.size())
    return;

  mapLock->get_lock();  
  obsDataMap_iter iter = 
    obsDataMap.upper_bound(aftertm);    // first entry > time
  obsDataMap_iter iterfirst = iter;
  obsDataMap_iter iterend = obsDataMap.end();


  if (iter == iterend)
    {
      mapLock->rel_lock();
      return;
    }
  while (iter != iterend)
    {
      if (iter->second)
	freeObsData(iter->second);
      iter++;
    }
  obsDataMap.erase(iterfirst, iterend);
  mapLock->rel_lock();
  check();
}

bool stnObsData::obsExists(time_t obstime)
{
  return obsDataMap.find(obstime) != obsDataMap.end();
}

  // if tm_window not defined get any obs >= time
baseObsData* stnObsData::firstObsAfterTm(time_t obstime, 
					 time_t tm_window)
{
  if (!obsDataMap.size())
    return NULL;
  baseObsData* result = NULL;
  mapLock->get_lock();  
  obsDataMap_iter iter = 
    obsDataMap.lower_bound(obstime);
  
  if ((iter == obsDataMap.end()) ||   // return null - if none after time
      ((tm_window != -1) &&     // or tmwin defined and entry outside it
       ((iter->first - obstime) > tm_window)))
    result = NULL;
  else
    result = iter->second;
  mapLock->rel_lock();
  return result;
}

// if tm_window not defined get any obs after time
baseObsData* stnObsData::obsNearestTm(time_t obstime, 
				      time_t tm_window)
{
  if (!obsDataMap.size())
    return NULL;
  baseObsData* result = NULL;
  mapLock->get_lock();  
  obsDataMap_iter iter_gte = 
    obsDataMap.lower_bound(obstime); // get 1st item >= time

  // time is > last, last is nearest
  if (iter_gte == obsDataMap.end())  
    {
      iter_gte--;               // step back to last item
      if ((tm_window < 0) ||  // no time window, return last
	  ((obstime-iter_gte->first) <= tm_window)) // last tm is within window
	result = iter_gte->second;
      else
	result = NULL;
      mapLock->rel_lock();
      return result;
    }

  //  time is <= first, first is nearest
  if (iter_gte == obsDataMap.begin()) 
    {
      if ((tm_window < 0) ||  // no time window, return last
	  ((iter_gte->first-obstime) <= tm_window)) // last tm is within window
	result = iter_gte->second;
      else
	result = NULL;
      mapLock->rel_lock();
      return result;
    } 

  // time is btwn first and last entry, 
  // and iter_gte must be gte time 
  // and  is known NOT to be begin() or end()
  obsDataMap_iter iter_lt = iter_gte;
  iter_lt--;                // step back to first entry < time
  time_t gte_diff = iter_gte->first-obstime;
  time_t lt_diff = obstime-iter_lt->first;
  if (gte_diff <= lt_diff)  // gte entry nearer then lt entry
    {
      if ((tm_window < 0) ||
	  (gte_diff <= tm_window))
	result = iter_gte->second;
      else
	result = NULL;
      mapLock->rel_lock();
      return result;
    }
  else                    // lt entry is nearer than gte entry
    {
      if ((tm_window < 0) ||
	  (lt_diff <= tm_window))
	result = iter_lt->second;
      else
	result = NULL;
      mapLock->rel_lock();
      return result;
    }
}

bool stnObsData::addObs(baseObsData *newobs)
{
  mapLock->get_lock();  
  if (!newobs || obsExists(newobs->tm)) 
    {
      mapLock->rel_lock();        
      return false;
    }
  else
    {
      obstype = newobs->obstype;
      obsDataMap[newobs->tm] = newobs;
      mapLock->rel_lock();        
      check();
      return true;
    }
}

long stnObsData::dataSize()
{
  int obssize;
  switch (obstype)
    {
    case ot_base :
      obssize = sizeof(baseObsData);
      break;
    case ot_fixed :
      obssize = sizeof(fixedObsData);
      break;
    case ot_moving :
      obssize = sizeof(movingObsData);
      break;
    }
  return obsCount * obssize;
}

void stnObsData::dump(FILE *dumpfile, bool summary)
{
  obsStn* thisstn = NULL;
  if (!dumpfile) return;
  fprintf(dumpfile, "\n");
  if (getGlobalObsStnMap())
    thisstn = getGlobalObsStnMap()->getObsStn(stnID);
  if (thisstn)
    thisstn->dump(dumpfile);
  else
    fprintf(dumpfile, "obsStn %d not found in globalObsStnMap\n",
	    stnID);
  char timestr[128], timestr2[128], sizestr[32];
  fprintf(dumpfile, "obsCount=%d firstTm=%s lastTm=%s dataSize=%s\n",
	  obsCount, ShortTimeString(firstTm, timestr),
	  ShortTimeString(lastTm, timestr2), 
	  scaled_kMGTSizeString(dataSize(), sizestr));
  if (summary) return;
  mapLock->get_lock();  
  obsDataMap_iter iter = obsDataMap.begin();
  obsDataMap_iter iterend = obsDataMap.end();
  if (iter != iterend)
    iter->second->dumphdr(dumpfile);
  while (iter != iterend)
    {
      iter->second->dump(dumpfile);
      iter++;
    }
  mapLock->rel_lock();        
}  

bool stnObsData::inLLBoundingBox(llBoundingBox &llbb)
{
  if (obsstn())
    return obsstn()->inLLBoundingBox(llbb);
  else
    return false;
}

obsDataMng::obsDataMng()
{
  purgeDepth = 12 * 60 * 60;  // purge > 12 hours older than latest obs data
  firstTm = lastTm = 0;
  debug = false;
  lightningLock = new spinlock("obsDataMng lightning lock", float(10.0));
}

obsDataMng::~obsDataMng()
{
  clear();
  if (lightningLock)
    delete lightningLock;
}

bool obsDataMng::addObs(baseObsData *newobs)
{
  if (!newobs) return false;
  stnObsData *newStnObsData = NULL;
  if (stnObsDataMap.find(newobs->stn) == stnObsDataMap.end())
    {
      newStnObsData = new stnObsData(newobs->stn);
      if (newStnObsData)
	{
	  stnObsDataMap[newobs->stn] = newStnObsData;
	  if (debug)
	    fprintf(stdout, "obsDataMng::addObs - New stn - adding stn=%d\n",
		    newobs->stn);
	}
      else
	fprintf(stdout, 
		"obsDataMng::addObs - Error Failed allocatin New stn - %d\n",
		newobs->stn);
    }
  if (stnObsDataMap.find(newobs->stn) != stnObsDataMap.end())
    {
      stnObsDataMap_iter iter = 
	stnObsDataMap.find(newobs->stn);
      if (iter != stnObsDataMap.end())
	return iter->second->addObs(newobs);
      else
	return false;
  }
  else
    return false;
}

bool obsDataMng::addLightning(lightningData *newobs)
{
  if (!newobs) return false;
  lightningLock->get_lock();
  lightningDataMMap.insert(make_pair(newobs->tm, newobs));
  lightningLock->rel_lock();
  return true;
}

void obsDataMng::clear()
{
  
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();
  
  while (iter != iterend)
    {
      if (iter->second)
	delete iter->second;
      iter++;
    }
  stnObsDataMap.clear();

  lightningLock->get_lock();
  lightningDataMMap_iter liter = lightningDataMMap.begin();
  lightningDataMMap_iter literend = lightningDataMMap.end();
  
  while (liter != literend)
    {
      if (liter->second)
	delete liter->second;
      liter++;
    }
  lightningDataMMap.clear();
  lightningLock->rel_lock();

  firstTm = lastTm = 0;
}

void obsDataMng::purgeBefore(time_t beforetm)
{
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();
  while (iter != iterend)
    {
      if (iter->second)
	iter->second->purgeBefore(beforetm);
      iter++;
    }

  lightningLock->get_lock();
  lightningDataMMap_iter iter_before = lightningDataMMap.lower_bound(beforetm);
  lightningDataMMap_iter liter = lightningDataMMap.begin();
  if (iter_before != lightningDataMMap.end() &&
      (iter_before->first <= beforetm))
    {
      while (liter != iter_before)
	{
	  freeLightningData(liter->second);
	  liter++;
	}
      lightningDataMMap.erase(lightningDataMMap.begin(), iter_before);
    }
  lightningLock->rel_lock();
}

void obsDataMng::purgeAfter(time_t aftertm)
{
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();
  while (iter != iterend)
    {
      if (iter->second)
	iter->second->purgeAfter(aftertm);
      iter++;
    }

  lightningLock->get_lock();
  lightningDataMMap_iter iter_after = lightningDataMMap.upper_bound(aftertm);
  lightningDataMMap_iter liter = iter_after;
  if (iter_after != lightningDataMMap.end() &&
      (iter_after->first >= aftertm))
    {
      while (liter != lightningDataMMap.end())
	{
	  freeLightningData(liter->second);
	  liter++;
	}	  
      lightningDataMMap.erase(iter_after, lightningDataMMap.end());
    }
  lightningLock->rel_lock();
}

bool obsDataMng::getLightningPeriodIters(lightningPeriodIters 
					 &lightning_iters)
{
  lightningLock->get_lock(); 
  lightning_iters.iter_start = 
    lightningDataMMap.lower_bound(lightning_iters.starttime);
  lightning_iters.iter_end = 
    lightningDataMMap.upper_bound(lightning_iters.endtime);
  lightning_iters.mmap_end = lightningDataMMap.end();
  if (lightning_iters.iter_start->first < lightning_iters.endtime)
    lightning_iters.valid = true;
  else
    lightning_iters.valid = false;
  return lightning_iters.valid;
  
  // NOTE: lightningLock is LOCKED BUT NOT RELEASED HERE - 
  // USER MUST CALL relLightningLock when finished using lightning_iters
} 

long obsDataMng::dataSize()
{
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();
  long totalSize = 0;
  while (iter != iterend)
    {
      if (iter->second)
	totalSize += iter->second->dataSize();
      iter++;
    }
  totalSize += lightningDataMMap.size() * sizeof(lightningData);
  return totalSize;
}

void obsDataMng::dump(FILE *dumpfile, bool summary)
{
  
  if (!dumpfile) return;
  char sizestr[16];
  char timestr[128], timestr2[128];
  fprintf(dumpfile, "obsDataMng::dump\n"
	  "Stns in map=%d totalSize=%sB\n"
	  "Lightning size = %d\n"
	  "firstTm=%s lastTm=%s\n"
	  "useObsDataFreelist = %d\n",
	  stnObsDataMap.size(),
	  scaled_kMGTSizeString(dataSize(), sizestr),
	  int(lightningDataMMap.size()),
	  ShortTimeString(getGlobalObsData()->firstTm, timestr),
	  ShortTimeString(getGlobalObsData()->lastTm, timestr2),
	  useObsDataFreelist);
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();
  int totalSize = 0;
  while (iter != iterend)
    {
      if (iter->second)
	iter->second->dump(dumpfile, summary);
      totalSize += iter->second->dataSize();
      iter++;
    }
}  

void obsDataMng::dump(char *fname, bool summary)
{
  FILE *dumpfile = fopen(fname, "w");
  if (dumpfile)
    {
      dump(dumpfile, summary);
      fclose(dumpfile);
    }
  else
    {
      fprintf(stderr, "obsDataMng::dump - Failed opening file - %s\n",
	      fname);
      perror(0);
    }
}

void obsDataMng::getStnsInLLBB(llBoundingBox &llbb, 
			       stnObsDataSet &llBoxStnSet)
{
  // if source stnset unchanged and llbb unchanged
  // existing set is valid, so return
  if ((llBoxStnSet.totalObsStns == int(stnObsDataMap.size())) &&
      (llBoxStnSet.llbb.isSame(llbb)))
    return;

  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();

  llBoxStnSet.clear();
  while (iter != iterend)
    {
      if (iter->second && iter->second->inLLBoundingBox(llbb))
	llBoxStnSet.add(iter->first, iter->second);
      iter++;
    }
  llBoxStnSet.totalObsStns = int(stnObsDataMap.size());
  llBoxStnSet.llbb.copy(llbb);
}

void obsDataMng::check()
{
  stnObsDataMap_iter iter = stnObsDataMap.begin();
  stnObsDataMap_iter iterend = stnObsDataMap.end();

  firstTm = lastTm = 0;
  while (iter != iterend)
    {
      if (iter->second)
	{
	  if (!firstTm ||
	      (iter->second->firstTm &&
	      (iter->second->firstTm < firstTm)))
	    firstTm = iter->second->firstTm;
	  if (!lastTm ||
	      (iter->second->lastTm > lastTm))
	    lastTm = iter->second->lastTm;
	}
      iter++;
    }
  purgeBefore(lastTm - purgeDepth);
  if (FileExists(obsDataSettingsReloadFname, true, true))
    setReloadSettings();
}

void obsDataMng::setReloadSettings()
{
    reloadDrawObsSettings = 
      reloadDrawLtningSettings = true;
}

obsStn::obsStn(int _stn, int _site,
		 float _lat, float _lng, float _ht,
		 std::string _name, std::string _station)
{
  set(_stn, _site, _lat, _lng, _ht, _name, _station);
}

obsStn::obsStn()
{	
  init();
}

obsStn::~obsStn()
{
}

void obsStn::init()
{
  stn = site = -1;
  lat = lng = ht = 0;
  name = "NoName";
  station = "NoStation";
  obstype = ot_fixed;
}

void obsStn::set(int _stn, int _site,
		  float _lat, float _lng, float _ht,
		  std::string _name, std::string _station)
{
  stn = _stn;
  site = _site;
  if (site == -9999)
    obstype = ot_moving;
  else
    obstype = ot_fixed;
  lat = _lat;
  lng = _lng;
  ht = _ht;
  name = _name;
  station = _station;
}

obsType obsStn::getType()
{
  if (site == -9999)
    obstype = ot_moving;
  else
    obstype = ot_fixed;
  return obstype;
}

void obsStn::dump(FILE *dumpfile)
{
  if (!dumpfile) return;
  fprintf(dumpfile, "%5.5s(%1.1s) %14.14s stn=%6d site=%6d llh=%7.3f/%7.3f/%1.3f\n",
	  name.c_str(), getObsTypeStr(obstype), station.c_str(), stn, site, lat, lng, ht);
}

obsStnMap::obsStnMap()
{
  mapLock = new spinlock("obsStnMap", float(2.0));
}

obsStnMap::~obsStnMap()
{
  clear();
  if (mapLock)
    delete mapLock;
}

bool obsStnMap::addObsStn(obsStn *newstn)  // return true if added, false if duplicate (not added)
{
  if (!newstn) return false;
  bool result = !obsStnExists(newstn->stn);
  if (mapLock) mapLock->get_lock();
  stnMap[newstn->stn] = newstn;
  if (mapLock) mapLock->rel_lock();
  return result;
}

bool obsStnMap::obsStnExists(int _stn)
{
  if (mapLock) mapLock->get_lock();
  bool exists = stnMap.find(_stn) != stnMap.end();
  if (mapLock) mapLock->rel_lock();
  return exists;
}

obsStn* obsStnMap::getObsStn(int _stn)        // return NULL if not found
{
  std::map<int, obsStn*>::iterator stnMapIter = stnMap.find(_stn);
  if (stnMapIter != stnMap.end())
    return stnMapIter->second;
  else
    return NULL;
}

void obsStnMap::clear()
{
  if (mapLock) mapLock->get_lock();
  std::map<int, obsStn*>::iterator iter = stnMap.begin();
  std::map<int, obsStn*>::iterator iterend = stnMap.end();
  
  while (iter != iterend)
    {
      if (iter->second)
	delete iter->second;
      iter++;
    }
  stnMap.clear();
  if (mapLock) mapLock->rel_lock();
}

void obsStnMap::dump(FILE *dumpfile)
{
  if (!dumpfile) return;
  if (mapLock) mapLock->get_lock();
  std::map<int, obsStn*>::iterator iter = stnMap.begin();
  while (iter != stnMap.end())
    {
      iter->second->dump(dumpfile);
      iter++;
    }
  if (mapLock) mapLock->rel_lock();
}  

void obsStnMap::dump(char *fname)
{
  FILE *dumpfile = fopen(fname, "w");
  if (dumpfile)
    {
      dump(dumpfile);
      fclose(dumpfile);
    }
  else
    {
      fprintf(stderr, "obsStnMap::dump - Failed opening file - %s\n",
	      fname);
      perror(0);
    }
}

