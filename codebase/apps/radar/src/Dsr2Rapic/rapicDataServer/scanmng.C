/*

scanmng.c

Implementation of the scan_mng class

*/

#ifdef sgi
#include <sys/bsd_types.h>
#endif

#include "rdr.h"
#include "rdrscan.h"
#ifdef sgi
// #include <ulocks.h>
#endif
#include <time.h>
#include "utils.h"
#include <sys/types.h>
#include <malloc.h>
#include <sys/stat.h>
#include <iostream>

#ifdef USE_TITAN_CLIENT
#include "rapicTitanClientMng.h"
#endif

#ifdef USE_RAINFIELDS_CLIENT
#include "rainfieldsClient.h"
#endif

#ifdef USE_WDSS_CLIENT
#include "wdssData.h"
#endif

char *scanclienttype_strings[] = {
  "UNDEF_CLIENT", 
  "SEQ_CLIENT", 
  "DB_CLIENT", 
  "TXDEV_CLIENT", 
  "COMMMNG_CLIENT", 
  "RAINACC_CLIENT", 
  "SATDATAMNG_CLIENT", 
  "NEXRADMNG_CLIENT", 
  "NEXRADSTN_CLIENT", 
  "UFMNG_CLIENT",
  "UFSTN_CLIENT",
  "TITAN_CLIENT"
};

extern time_t RapicStartedTime;
 
scan_registry *ScanRegistry = NULL;
 
scan_mng *ScanMng = 0;
 
// scan_client::scan_client() {
scan_client::scan_client() : ThreadObj() {
  nextclient = prevclient = 0;
  RealTime = TRUE;
  rdrimg_tmwin = 600;		// allow a 600sec time window for same image
  AcceptDuplicates = TRUE;	// by default, scan clients will accept duplicates
  AcceptNewScans = TRUE;	// by default accept new scans via NewDataAvail
  AcceptFinishedScans = FALSE;// by default DON'T accept new scans via NewDataAvail
  newscans = finishedscans = checkedscans = 0;
  _scan_node_count = 0;
  newcdata = checkedcdata = 0;// new scans, usually added by other threads 
  cdatanode_count = 0;        
  ClientType = SC_UNDEFINED;	// client instances should set this 
  if (FileExists("IgnoreFutureData.flag"))
    ignoreFutureData = true;
  else
    ignoreFutureData = false;	// default is to NOT ignore future data
  futureDataTmWin = 60 * 30;	// default to 30 minute "future" time window 
  AllowReplayMode = false;	// by default don't allow replay mode
  AllowDBReviewMode = false;	// by default don't allow DBREVIEW mode
#ifdef USE_RAINFIELDS_CLIENT
  acceptRFImages = false;       // NOTE: DO NOT ACCEPT RF IMAGES BY DEFAULT
#endif
  DataMode = REALTIMEMODE;	// default to REALTIME mode
  lock = 0;
}				// comms clients should not, to avoid loops

scan_client::~scan_client() {		// remove links before deletion
  if (nextclient || prevclient)	// should be done by client before this
    fprintf(stderr,"scan_client(%s)::~scan_client - Client Links not cleared, clearing now\n", 
	    scanclienttype_strings[ClientType]);
  if (nextclient)
    nextclient->prevclient = prevclient;
  if (prevclient)
    prevclient->nextclient = nextclient;
  if (lock) {
    delete lock;
    lock = 0;
  }
  if (getScanNodeCount())
    fprintf(stderr, "scan_client(%s)::~scan_client: Scan Node Count = %d\n", 
	    scanclienttype_strings[ClientType], getScanNodeCount());
  if (cdatanode_count)
    fprintf(stderr, "scan_client(%s)::~scan_client: CData Node Count = %d\n", 
	    scanclienttype_strings[ClientType], cdatanode_count);
  PurgeNewData();
  PurgeCheckedData();
}

#ifdef USE_RAINFIELDS_CLIENT
bool scan_client::ScanSourceOK(Ref<rainfieldImg>& rfimg)
{
  if (DataMode == REPLAYMODE)
    return (rfimg->data_mode == RF_REPLAY);
  if (DataMode == DBREVIEWMODE)
    return (rfimg->data_mode == RF_DB);
  if (DataMode == REALTIMEMODE)
    return (rfimg->data_mode == RF_REALTIME);
  else
    return false;
}
#endif

bool scan_client::ScanSourceOK(rdr_scan *srcscan)
{
  bool OK = false;

  if (DataMode == REPLAYMODE)
    switch (srcscan->data_source)
      {
      case PROD_ACCUM:
      case REPLAY:
	OK = true;
	break;
      default:
	break;
      }
  if (DataMode == DBREVIEWMODE)
    switch (srcscan->data_source)
      {
      case PROD_ACCUM:
      case DB:
	OK = true;
	break;
      default:
	break;
      }
  if (DataMode == REALTIMEMODE)
    switch (srcscan->data_source)
      {
      case DBRELOADREALTIME:
      case COMM:
      case COMMREQ:
      case PROD_ACCUM:
      case PROD_XLAT:
      case RADARCONVERT:
	OK = true;
	break;
      default:
	break;
      }
  return OK;
}

void scan_client::SetDataMode(e_scan_client_mode newmode)	// try to set data mode
{
  if (newmode == DataMode)	// no change, 
    return;	
  if ((newmode == REPLAYMODE) && AllowReplayMode)
    {
      DataMode = newmode;
      DataModeChanged();
    }
  else if ((newmode == DBREVIEWMODE) && AllowDBReviewMode)
    {
      DataMode = newmode;
      DataModeChanged();
    }
  else if (newmode == REALTIMEMODE) // by default all clients can run REALTIME
    {
      DataMode = newmode;
      DataModeChanged();
    }
}

// clients should override this if they want to initialise
// things on mode change, e.g. rdrseq would probably clear the
// current sequence
void scan_client::DataModeChanged()	// default to doing nothing
{																		
  return;
}

/*
 * NewDataAvail will usually be called by other than the scan client thread
 * By default scan clients NewDataAvail call will add new scan to a simple 
 * list for subsequent processing by the client thread,  using the 
 * CheckNewScans call.
 * 
 * Child NewDataAvail and CheckNewScans should use locks to prevent 
 * linked list conflicts between threads. 
 */
int  scan_client::NewDataAvail(rdr_scan *newscan) {
  rdr_scan_node   *newscannode;	
  char tempstr[128];

  if (newscan && AcceptNewScans && ScanSourceOK(newscan)) {
    sprintf(tempstr, "scan_client(%s)::NewDataAvail", 
	    scanclienttype_strings[ClientType]);
    newscannode = new rdr_scan_node(this, tempstr, newscan);
    incScanNodeCount();
    newscannode->prev = 0;
    newscannode->next = newscans;
    if (newscans) newscans->prev = newscannode;
    newscans = newscannode;
    return 1;
  }
  else {
    return 0;
  }
}

#ifdef USE_RAINFIELDS_CLIENT
  /*	At this stage default to not keeping a reference. 
   *  Child classes must override this if they want the new rfimg
 * Child NewDataAvail and CheckNewScans should use locks to prevent 
 * linked list conflicts between threads. 
   */
int scan_client::NewDataAvail(Ref<rainfieldImg>& rfimg) {
  if (!acceptRFImages) return 0;
  fprintf(stderr, "scan_client::NewDataAvail - Adding rfimg - %s\n", rfimg->getRFKey());
  new_rf_Images[rfimg->getRFKey()] = rfimg;  // if it already exists, the new Ref will cause deref of previous
  return 1;
}

/*
  Purge new_rf_Images
  This needs to be implemented in child classes if rfimages are to be kept
*/
void scan_client::PurgeNewRFImages() // purge the new_rf_Images
{
  map<string, Ref<rainfieldImg> >::iterator iter = new_rf_Images.begin();
  map<string, Ref<rainfieldImg> >::iterator iter_end = new_rf_Images.end();
  while (iter != iter_end)
    {
      iter->second = NULL;   // dereference
      iter++;
    }
  new_rf_Images.clear();
}

void scan_client::ProcessNewRFImages() // process the new_rf_Images
{
  lock->get_lock();
  PurgeNewRFImages();
  lock->rel_lock();
}

#endif

void scan_client::incScanNodeCount()
{
  _scan_node_count++;
}

bool scan_client::decScanNodeCount()
{
  bool ok = true;
  if (_scan_node_count > 0)
    _scan_node_count--;
  else
    {
      fprintf(stderr, "scan_client::decScanNodeCount(%s) - Error - Count=%d\n",
	      scanclienttype_strings[ClientType],
	      _scan_node_count);
      ok = false;
    } 
  return ok;
}

int  scan_client::NewDataAvail(CData *newdata) {
  // CDataNode   *newdatanode;	

  /*	At this stage default to not keeping a reference. 
   *  Child classes must override this if they want the new cdata
   */
  /*
    if (newdata && AcceptNewScans) {
    newdatanode = new CDataNode(newdata);
    cdatanode_count++;
    newdatanode->PreLink(newcdata);
    newcdata = newdatanode;
    return 1;
    }
    else return 0;
  */
  return 0;
}

// Don't add scan via both New and FInished methods
// If AcceptNewScans defined, don't use FinishedDataAvail
int  scan_client::FinishedDataAvail(rdr_scan *finishedscan) {
  rdr_scan_node   *newscannode;	
  char tempstr[128];

  if (finishedscan && 
      !AcceptNewScans && AcceptFinishedScans && 
      ScanSourceOK(finishedscan)) {
    sprintf(tempstr, "scan_client(%s)::FinishedDataAvail", 
	    scanclienttype_strings[ClientType]);
    newscannode = new rdr_scan_node(this, tempstr, finishedscan);
    incScanNodeCount();
    newscannode->prev = 0;
    newscannode->next = newscans;
    if (newscans) newscans->prev = newscannode;
    newscans = newscannode;
    return 1;
  }
  else {
    return 0;
  }
}

// Don't add scan via both New and FInished methids
// If AcceptNewScans defined, don't use FinishedDataAvail
int  scan_client::FinishedDataAvail(CData *finisheddata) {
  // CDataNode   *newdatanode;	

  /*	At this stage default to not keeping a reference. 
   *  Child classes must override this if they want the new cdata
   */
  /*
    if (finisheddata && !AcceptNewScans && AcceptFinishedScans) {
    newdatanode = new CDataNode(finisheddata);
    cdatanode_count++;
    newdatanode->PreLink(newcdata);
    newcdata = newdatanode;
    return 1;
    }
    else return 0;
  */
  return 0;
}

/*
 * CheckNewScans should be defined in the sub-class implementation
 * If not,  this call will simply clear the newscans queue
 * 
 * IMPORTANT NOTE: AS THE NEWSCANS QUEUE IS OFTEN ACCESSED BY SEPARATE
 * THREADS THE NEWDataAvail,  FINISHEDDataAvail AND CHECKNEWSCANS
 * SHOULD ALL BE "THREAD SAFE" I.E. SERIALIZED BY MUTEXES
 * PROCESSCHECKEDSCANS DOES NOT NEED TO BE THREAD SAFE
 */

void  scan_client::CheckNewData(bool MoveToChecked, 
				bool RejectFaulty) {
  rdr_scan_node   *tempscannode;	

  //    fprintf(stderr,"scan_client::CheckNewData() - WARNING Should be defined in subclass - Clearing newscans queue\n");
  if (lock) lock->get_lock();
  while (newscans) {
    tempscannode = newscans->next;
    if (MoveToChecked && (!(RejectFaulty && newscans->scan->Faulty()))) {
      newscans->remove_from_list();
      newscans->next = checkedscans; 
      newscans->prev = 0;
      if (checkedscans)
	checkedscans->prev = newscans;
      checkedscans = newscans;
    }
    else {
      delete newscans;
      decScanNodeCount();
    }
    newscans = tempscannode;
  }
  if (lock) lock->rel_lock();
}

void  scan_client::PurgeNewData() {
  rdr_scan_node   *tempscannode;	
  int scanspurged = 0;

  if (newscans == NULL)
    return;
  if (lock) lock->get_lock();
  while (newscans) {
    tempscannode = newscans->next;
    delete newscans;
    decScanNodeCount();
    scanspurged++;
    newscans = tempscannode;
  }
  if (lock) lock->rel_lock();
  fprintf(stderr,"scan_client(%s)::PurgeNewData() - Cleared %d newscans - %d nodes remaining\n", 
	  scanclienttype_strings[ClientType],scanspurged, getScanNodeCount());
#ifdef USE_RAINFIELDS_CLIENT
  PurgeNewRFImages();
#endif
}

void  scan_client::ProcessCheckedData(int maxscans) {
  rdr_scan_node   *tempscannode;	

  if (maxscans);
  fprintf(stderr,"scan_client(%s)::ProcessCheckedData() - WARNING Should be defined in subclass - Clearing newscans queue\n",
	  scanclienttype_strings[ClientType]);
  while (checkedscans) {
    tempscannode = checkedscans->next;
    delete checkedscans;
    decScanNodeCount();
    checkedscans = tempscannode;
  }
#ifdef USE_RAINFIELDS_CLIENT
  ProcessNewRFImages();
#endif

}

void  scan_client::PurgeCheckedData() {
  rdr_scan_node   *tempscannode;	
  int scanspurged = 0;

  if (checkedscans == NULL)
    return;
  if (lock) lock->get_lock();
  while (checkedscans) {
    tempscannode = checkedscans->next;
    delete checkedscans;
    decScanNodeCount();
    scanspurged++;
    checkedscans = tempscannode;
  }
  if (lock) lock->rel_lock();
  fprintf(stderr,"scan_client(%s)::PurgeCheckedData() - Cleared %d checkedscans - %d nodes remaining\n", 
	  scanclienttype_strings[ClientType], scanspurged, getScanNodeCount());
}

bool scan_client::Full(int passedSize, bool forcecheck) {
  fprintf(stderr,"scan_client(%s)::Full() - WARNING Must be defined in subclass - returning TRUE\n", 
	  scanclienttype_strings[ClientType]);
  return TRUE;
}

int scan_client::percentFull(int passedSize) {
  fprintf(stderr,"scan_client(%s)::percentFull() - WARNING Must be defined in subclass - returning TRUE\n", 
	  scanclienttype_strings[ClientType]);
  return TRUE;
}

int scan_client::NumImgs() {
  fprintf(stderr,"scan_client(%s)::NumImgs() - WARNING Should be defined in subclass - returning 0\n", 
	  scanclienttype_strings[ClientType]);
  return 0;
}

bool scan_client::MergeScans() {
  return FALSE;
}

bool scan_client::IsDuplicate(rdr_scan *dupscan,  // return true if known ie already in seq or database etc
			      bool FinishedOnly) {
  return FALSE;
}

bool scan_client::IsDuplicate(CData *dupdata,  // return true if known ie already in seq or database etc
			      bool FinishedOnly) {
  return FALSE;
}

void scan_client::PrintScanUsage(FILE *file, bool verbose) {
  int newcount = 0, newsize = 0, checkedcount = 0, checkedsize = 0;
  if (!file) file = stderr;
  if (lock) lock->get_lock();
  if (newscans) {
    newcount = newscans->ListRootCount();
    newsize = newscans->ListSize();
  }
  if (checkedscans) {
    checkedcount = checkedscans->ListRootCount();
    checkedsize = checkedscans->ListSize();
  }
  fprintf(file, "scan_client(%s)::PrintScanUsage, NewScans: %d:%1.3fMB,  CompleteScans %d:%1.3fMB, scan_node_count=%d\n", 
	  scanclienttype_strings[ClientType], 
	  newcount, newsize/1000000.0, 
	  checkedcount, checkedsize/1000000.0,
	  getScanNodeCount());
  if (lock) lock->rel_lock();
}

scan_mng::scan_mng() {
  ClientList = 0;
  lock = new spinlock("scan_mng->lock", 500);	// wait max. 5secs for spinlock
  CheckDuplicates = true;	    // allow duplicates to pass to clients
  reportDuplicates = true;
  currentDataMode = REALTIMEMODE;
  useRecentScansCache = true;
  keepScansInCache = false; // if true, ensure scan not deleted by inc'ing usercount
  recentCachePeriod = 120;    // mins of rdr_scan keys to keep
  scanCacheLock = new spinlock("scan_mng->scanCacheLock", 500);	// wait max. 5secs for spinlock
  lastCacheSize = 0;
  firstCacheTime = lastCacheTime = 0;
  startupLatestTime = 0;
  CheckTime = 0;
  CheckPeriod = 60;
  appClosing = false;
}				    // scan clients responsible for duplicate scan management

scan_mng::~scan_mng() {
  if (lock) delete lock;
  
  clearRecentScans();
  if (ScanRegistry)
    ScanRegistry->clear(true);  // tidy up any remaining rdr_scans
}

// add new client to linked list

void scan_mng::AddClient(scan_client *new_client) {
  scan_client *temp = 0,*next = 0;
  if (!new_client) return;
  lock->get_lock();
  temp = ClientList;				// traverse list to last
  if (temp) next = temp->nextclient;
  while (next) {
    temp = next;
    next = temp->nextclient;
  }
  if (temp) 								// new client becomes last
    temp->nextclient = new_client;
  else ClientList = new_client;	// no clients yet, must be first
  new_client->prevclient = temp;// set prev pointer
  new_client->nextclient = 0;		// always last, no next
  lock->rel_lock();
}

void scan_mng::RemClient(scan_client *rem_client) {
  scan_client *temp = 0;
  // *next = 0;
  lock->get_lock();
  temp = ClientList;				// traverse list to last
  while (temp) {
    if (temp == rem_client) {
      if (temp == ClientList) ClientList = temp->nextclient;
      if (temp->prevclient) 
	temp->prevclient->nextclient = temp->nextclient;
      if (temp->nextclient) 
	temp->nextclient->prevclient = temp->prevclient;
      temp->prevclient = 0;
      temp->nextclient = 0;
      temp = 0;
    }
    else {
      temp = temp->nextclient;
      if (!temp) fprintf(stderr,"scan_mng::RemClient - rem_client not found\n");
    }
  }
  lock->rel_lock();
}

// add new event client to linked list

void scan_mng::AddEventClient(scanEventClient *new_client) {
  if (!new_client) return;
  lock->get_lock();
  list<scanEventClient*>::iterator iter_this = 
    scanEventClients.begin();
  list<scanEventClient*>::iterator iter_end = 
    scanEventClients.end();
  while ((iter_this != iter_end) && 
	 (*iter_this != new_client))
    iter_this++;
  if (iter_this == iter_end)  // didn't find duplicate pointer, add new
    scanEventClients.push_back(new_client);
  lock->rel_lock();
}

void scan_mng::RemEventClient(scanEventClient *rem_client) {
  lock->get_lock();
  list<scanEventClient*>::iterator iter_this = 
    scanEventClients.begin();
  list<scanEventClient*>::iterator iter_end = 
    scanEventClients.end();
  while ((iter_this != iter_end) && 
	 (*iter_this != rem_client))
  if (iter_this != scanEventClients.end())
    scanEventClients.erase(iter_this);
  else
    fprintf(stderr, "scan_mng::RemEventClient - failed - no matching client found\n");
  lock->rel_lock();
}

void scan_mng::clearRecentScans() // purge all recent scan before this time
{
  if (scanCacheLock)
    scanCacheLock->get_lock();

  if (keepScansInCache)
    {
      rdrStrScanMap::iterator recent_begin = recentScans.begin();
      rdrStrScanMap::iterator recent_end = recentScans.end();
      while (recent_begin != recent_end)
	{
	  if (recent_begin->second &&
	      recent_begin->second->ShouldDelete(this, "scan_mng::clearRecentScans"))
	    {
	      delete recent_begin->second;
	    }
	  recent_begin++;
	}
    }
  recentScans.clear();
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  cachedScansSize();
}

time_t scan_mng::mostRecentScanTime()   // return time of the most recent scan
{
  time_t result = 0;
  if (scanCacheLock)
    scanCacheLock->get_lock();
  rdrStrScanMap::iterator recent_end = recentScans.end();
  if (recentScans.size())
    {
      recent_end--;
      result = rdrScanKeyTimet(recent_end->first.c_str());
    }
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  return result;
}

void scan_mng::purgeRecentScans(time_t purge_before_time) // purge all recent scan before this time
{
  if (purge_before_time == 0)
    {
      purge_before_time = mostRecentScanTime();
      purge_before_time -= int(recentCachePeriod * 60.0);
    }
  tm purgetm;
  gmtime_r(&purge_before_time, &purgetm);
  char localstr[32];
  if (scanCacheLock)
    scanCacheLock->get_lock();

  sprintf(localstr, "%04d%02d%02d%02d%02d%02d",
	        purgetm.tm_year + 1900, purgetm.tm_mon+1, purgetm.tm_mday, 
          purgetm.tm_hour, purgetm.tm_min, purgetm.tm_sec);
//   cout << "scan_mng::purgeRecentScans - Checking for scan keys before " << 
//     localstr << " to purge - Scans in cache=" <<
//     recentScans.size() << endl;
  string timekey = localstr;
  rdrStrScanMap::iterator recent_begin = recentScans.begin();
  rdrStrScanMap::iterator recent_purgetime = recentScans.lower_bound(timekey);
  rdrStrScanMap::iterator recent_end = recentScans.end();
  if (recent_purgetime != recent_end)
    {
      if (keepScansInCache)
	while (recent_begin->first < recent_purgetime->first)
	  {
	    if (recent_begin->second &&
		recent_begin->second->ShouldDelete(this, "scan_mng::purgeRecentScans"))
	      {
		delete recent_begin->second;
	      }
	    recent_begin++;
	  }
//       cout << "scan_mng::purgeRecentScans - Purging from " << 
//             recent_begin->first << " to " << recent_purgetime->first << endl;
      recentScans.erase(recentScans.begin(), recent_purgetime);
    }
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  cachedScansSize();
}

double scan_mng::cachedScansSize()
{
  if (!keepScansInCache)
    return 0;

  rdrStrScanMap::iterator recent_iter = recentScans.begin();
  rdrStrScanMap::iterator recent_end = recentScans.end();
  double total=0;

  if (scanCacheLock)
    scanCacheLock->get_lock();
  while (recent_iter != recent_end)
    {
      total += recent_iter->second->scanSetSize();
      recent_iter++;
    }
  lastCacheSize = total;
  if (recentScans.size())
    {
      firstCacheTime = rdrScanKeyTimet(recentScans.begin()->first.c_str());
      rdrStrScanMap::iterator last_iter = recentScans.end();
      last_iter--;
      lastCacheTime = rdrScanKeyTimet(last_iter->first.c_str());
    }
  else
    firstCacheTime = lastCacheTime = 0;
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  return total;
}

int scan_mng::cachePercentFull()
{
  if (recentCachePeriod == 0)
    return 100;
  if (recentScans.size() <= 1)
    return 0;
  if (scanCacheLock)
    scanCacheLock->get_lock();
  firstCacheTime = rdrScanKeyTimet(recentScans.begin()->first.c_str());
  rdrStrScanMap::iterator last_iter = recentScans.end();
  last_iter--;
  lastCacheTime = rdrScanKeyTimet(last_iter->first.c_str());
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  int mins_diff =  (lastCacheTime - firstCacheTime)/60;
  return int((float(mins_diff) / float(recentCachePeriod)) * 100.0);
}

// return true if a matching scan Key is in the recent scans cache
RSC_RESULT scan_mng::scanKeyInRecentScans(rdr_scan *newscan)
{
  RSC_RESULT result = NOT_IN_CACHE;
  if (!newscan->HeaderValid()) return NOT_IN_CACHE;
  if (scanCacheLock)
    scanCacheLock->get_lock();
  string scankey = newscan->ScanKey();
  rdrStrScanMap::iterator cache_scan = recentScans.find(scankey);
  rdrStrScanMap::iterator cache_end = recentScans.end();
  
  if (cache_scan == cache_end)
    {
      newscan->ScanMngDuplicate = false;
      result = NOT_IN_CACHE;
    }
  else if (cache_scan->second == newscan)
    {
//       fprintf(stderr, "scan_mng::scanKeyInRecentScans - Found IN_CACHE_SAME_INST - %s\n",
// 	      newscan->ScanString());
      newscan->ScanMngDuplicate = false;
      result = IN_CACHE_SAME_INST;
    }
  else
    {
//       char tempstr[256];
//       fprintf(stderr, "scan_mng::scanKeyInRecentScans - Found IN_CACHE_DUPL - %s\n",
// 	      newscan->ScanString2(tempstr));
      newscan->ScanMngDuplicate = true;
      result = IN_CACHE_DUPL;
    }
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  return result;
}

// return true if a matching scan instance is held in the recent scans cache
RSC_RESULT scan_mng::scanInRecentScans(rdr_scan *newscan) // returns scan instance in cache status
{
  if (!keepScansInCache)
    return NOT_IN_CACHE;
  else
    return scanKeyInRecentScans(newscan);
}

bool scan_mng::addRecentScanOK(rdr_scan *newscan)
{
  if (!useRecentScansCache)
    return true;
  RSC_RESULT incache = scanKeyInRecentScans(newscan);
  if (incache == IN_CACHE_DUPL)
    return false;
  else if (incache == NOT_IN_CACHE)
    {
      string scankey = newscan->ScanKey();
      if (scanCacheLock)
	scanCacheLock->get_lock();
      lastCacheSize += newscan->scanSetSize(); // may be growing rdr_scan - approx only 
      recentScans[scankey] = newscan;
      if (scanCacheLock)
	scanCacheLock->rel_lock();
      if (keepScansInCache)
	newscan->IncUserCount(this, "scan_mng::addRecentScanOK");
      return true;
    }
  else                     // incache must be IN_CACHE_SAME_INST
    return true;
}

int scan_mng::getRecentScans(int minutes, time_t getfrom, 
			      scan_client *client, bool reverseorder)
{
  if (!client || !useRecentScansCache || !keepScansInCache)
    return 0;
  if (scanCacheLock)
    scanCacheLock->get_lock();

  bool debug = false;
  char tempstr1[256], tempstr2[256];

  time_t lasttime = time(0);
  rdrStrScanMap::reverse_iterator riter = recentScans.rbegin();
  rdrStrScanMap::reverse_iterator rend = recentScans.rend();
  time_t earliesttime = lasttime - (minutes * 60);
  if (riter->second)
    {
      lasttime = riter->second->scan_time_t;
      earliesttime = lasttime - (minutes * 60);
      if (getfrom > earliesttime)
	earliesttime = getfrom;
    }
  if (debug)
    fprintf(stdout, "scan_mng::getRecentScans - earliest=%s last=%s\n",
	   ShortTimeString(earliesttime, tempstr1), 
	   ShortTimeString(lasttime, tempstr2));
  bool done = false;
  if (!reverseorder)
    {
      // use reverse iterators to load from latest to oldest into newdataavail
      // earliest will then be "first" in new scans list
      // checknewscans typically starts with first in new scans list
      // so earliest will be first onto list, latest will then be at start of checked list
      while ((riter != rend) && !done)
	{
		    
	  if (riter->second)
	    {
	      if (debug)
		fprintf(stdout, "scan_mng::getRecentScans - Passing to client  - %s\n",
			riter->second->ScanString2(tempstr1));
	      client->NewDataAvail(riter->second);
	      client->FinishedDataAvail(riter->second);
	    }
	  done = riter->second->scan_time_t < earliesttime;
	  riter++;
	}
    }
  else
    {
      // use fwd iterators to load from earliest to latest into newdataavail
      // latest will then be "first" in new scans list
      // checknewscans typically starts with first in new scans list
      // so latest be first onto list, earliest will then be at start of checked list
      rdrStrScanMap::iterator iter = recentScans.begin();
      rdrStrScanMap::iterator iter_end = recentScans.end();
      // step through to earliest time
      while ((iter != iter_end) && 
	     (iter->second && (iter->second->scan_time_t < earliesttime)))
	iter++;
      while ((iter != iter_end))
	{
	  if (iter->second)
	    {
	      client->NewDataAvail(iter->second);
	      client->FinishedDataAvail(iter->second);
	    }
	  iter++;
	}
    }
  if (scanCacheLock)
    scanCacheLock->rel_lock();
  return (lasttime - earliesttime) / 60;
}
  

bool scan_mng::IsDuplicate(rdr_scan *new_scan,  // return true if known ie already in seq or database etc
			   bool FinishedOnly) {
  if (!new_scan) return false;
  if (useRecentScansCache)
    {
      if (!addRecentScanOK(new_scan))  // addRecentScan only returns false if matching scan already in cache
        return true;
      else
        return false;
    }
  scan_client *temp = ClientList;
  while (temp) {
    if (temp->IsDuplicate(new_scan, FinishedOnly)) 
      return true;
    temp = temp->nextclient;
  }
  return false;
}
    
bool scan_mng::IsDuplicate(CData *newdata,  // return true if known ie already in seq or database etc
			   bool FinishedOnly) {
  scan_client *temp = ClientList;
  while (temp) {
    if (temp->IsDuplicate(newdata, FinishedOnly)) return TRUE;
    temp = temp->nextclient;
  }
  return FALSE;
}
    
void scan_mng::NewDataAvail(rdr_scan *new_scan) {
  scan_client *temp;
  bool IsADuplicate = FALSE;
  int elapsedtime;
  char tempstr[128];

  if (appClosing) return;
  if (!new_scan || new_scan->ShownToScanClients || !new_scan->HeaderValid()) 
    {
      if (!new_scan->HeaderValid())
	fprintf(stderr, "scan_mng::NewDataAvail - ERROR new_scan presented without HeaderValid set\n");
      return;	// this scan has already been presented, or no header yet
    }
  // new_scans without headers should not be shown through NewDataAvail
  lock->get_lock();
  temp = ClientList;
  if (CheckDuplicates)
    IsADuplicate = IsDuplicate(new_scan);
  if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
    fprintf(stderr, "scan_mng::NewDataAvail lock wait time exceeded. (%1.3fsecs) Servicing IsDuplicate. Client type = %s\n", 
	    elapsedtime/100., scanclienttype_strings[temp->ClientType]);
  if (IsADuplicate && reportDuplicates)
    fprintf(stderr, "scan_mng::NewDataAvail Duplicate scan detected - %s\n", new_scan->ScanString2(tempstr));
  while (temp) {
    //		if (temp->RealTime) // don't filter, give client a chance to keep even if not real time
    if ((!IsADuplicate || temp->AcceptDuplicate(new_scan)) &&
	temp->AcceptNewScans)
      temp->NewDataAvail(new_scan);
    if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
      fprintf(stderr, "scan_mng::NewDataAvail lock wait time exceeded. (%1.3fsecs) Client type = %s\n", 
	      elapsedtime/100., scanclienttype_strings[temp->ClientType]);
    temp = temp->nextclient;
  }
  list<scanEventClient*>::iterator iter_this = scanEventClients.begin();
  while (iter_this != scanEventClients.end())
    {
      if (*iter_this)
	if ((!IsADuplicate || (*iter_this)->AcceptDuplicates) &&
	    (*iter_this)->AcceptNewScans)
	  (*iter_this)->NewDataAvail(new_scan);
      iter_this++;
    }
  new_scan->ShownToScanClients = TRUE;
  lock->rel_lock();
}

#ifdef USE_RAINFIELDS_CLIENT
void scan_mng::NewDataAvail(Ref<rainfieldImg>& rfimg) 
{
  scan_client *temp;
  int elapsedtime;
    
  if (appClosing) return;
  lock->get_lock();
  temp = ClientList;
  while (temp) {
    //		if (temp->RealTime) // don't filter, give client a chance to keep even if not real time
    temp->NewDataAvail(rfimg);
    if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
      fprintf(stderr, "scan_mng::NewDataAvail lock wait time exceeded. (%1.3fsecs) Client type = %s\n", 
	      elapsedtime/100., scanclienttype_strings[temp->ClientType]);
    temp = temp->nextclient;
  }
  lock->rel_lock();
}
#endif

void scan_mng::NewDataAvail(CData *newdata) {
  scan_client *temp;
  bool IsADuplicate = FALSE;
  int elapsedtime;
    
  if (appClosing) return;
  if (!newdata || newdata->IsShownToClientsNew()) 
    return;	// this scan has already been presented
  lock->get_lock();
  temp = ClientList;
  if (CheckDuplicates)
    IsADuplicate = IsDuplicate(newdata);
  if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
    fprintf(stderr, "scan_mng::NewDataAvail lock wait time exceeded. (%1.3fsecs) Servicing IsDuplicate. Client type = %s\n", 
	    elapsedtime/100., scanclienttype_strings[temp->ClientType]);
  if (IsADuplicate && reportDuplicates)
    fprintf(stderr, "scan_mng::NewDataAvail Duplicate scan detected\n");
  while (temp) {
    //		if (temp->RealTime) // don't filter, give client a chance to keep even if not real time
    if (!IsADuplicate || temp->AcceptDuplicates)
      temp->NewDataAvail(newdata);
    if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
      fprintf(stderr, "scan_mng::NewDataAvail lock wait time exceeded. (%1.3fsecs) Client type = %s\n", 
	      elapsedtime/100., scanclienttype_strings[temp->ClientType]);
    temp = temp->nextclient;
  }
  newdata->SetShownToClientsNew(TRUE);
  lock->rel_lock();
}

void scan_mng::FinishedDataAvail(rdr_scan *finished_scan) {
  scan_client *temp;
  bool IsADuplicate = FALSE;
  int elapsedtime;
  char tempstr[128];
  //    if (!finished_scan || finished_scan->ShownToScanClients) 
  //	return;	// this scan has already been presented
  if (!finished_scan || appClosing) {
    fprintf(stderr, "scan_mng::FinishedDataAvail- no scan, exiting\n");
    return;
  }
  lock->get_lock();
  temp = ClientList;
  if (CheckDuplicates)
    IsADuplicate = IsDuplicate(finished_scan, FINISHEDONLY);	// duplicate only if finished
  if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
    fprintf(stderr, "scan_mng::FinishedDataAvail lock wait time exceeded. (%1.3fsecs) Servicing IsDuplicate. Client type = %s\n", 
	    elapsedtime/100., scanclienttype_strings[temp->ClientType]);
  if (IsADuplicate && reportDuplicates)
    fprintf(stderr, "scan_mng::FinishedDataAvail Duplicate scan detected - %s\n", finished_scan->ScanString2(tempstr));
  while (temp) {
    //		if (temp->RealTime) // don't filter, give client a chance to keep even if not real time
    if ((!IsADuplicate || temp->AcceptDuplicate(finished_scan)) && 	// BEWARE, VERY LIKELY TO BE TRUE FOR FINISHEDDataAvail
	temp->AcceptFinishedScans)
      temp->FinishedDataAvail(finished_scan);
    else if (temp->AcceptFinishedScans && false)
      fprintf(stderr, "scan_mng::FinishedDataAvail (%s) - Duplicate not shown to client\n",
	      temp->threadLabel());
    if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
      fprintf(stderr, "scan_mng::FinishedDataAvail lock wait time exceeded. (%1.3fsecs) Client type = %s\n", 
	      elapsedtime/100., scanclienttype_strings[temp->ClientType]);
    temp = temp->nextclient;
  }
  list<scanEventClient*>::iterator iter_this = scanEventClients.begin();
  while (iter_this != scanEventClients.end())
    {
      if (*iter_this)
	if ((!IsADuplicate || (*iter_this)->AcceptDuplicates) &&
	    (*iter_this)->AcceptFinishedScans)
	  (*iter_this)->FinishedDataAvail(finished_scan);
      iter_this++;
    }
  finished_scan->ShownToScanClients = TRUE;
  lock->rel_lock();
}

void scan_mng::FinishedDataAvail(CData *finisheddata) {
  scan_client *temp;
  bool IsADuplicate = FALSE;
  int elapsedtime;
    
  if (!finisheddata || finisheddata->IsShownToClientsFin() || appClosing) {
    fprintf(stderr, "scan_mng::FinishedDataAvail early exit\n");
    return;	// this scan has already been presented
  }
  lock->get_lock();
  temp = ClientList;
  if (CheckDuplicates)
    IsADuplicate = IsDuplicate(finisheddata, FINISHEDONLY);	// duplicate only if finished
  if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
    fprintf(stderr, "scan_mng::FinishedDataAvail lock wait time exceeded. (%1.3fsecs) Servicing IsDuplicate. Client type = %s\n", 
	    elapsedtime/100., scanclienttype_strings[temp->ClientType]);
  //    if (IsADuplicate)
  //	fprintf(stderr, "scan_mng::FinishedDataAvail Duplicate scan detected\n");
  while (temp) {
    //		if (temp->RealTime) // don't filter, give client a chance to keep even if not real time
    if ((!IsADuplicate || temp->AcceptDuplicates) &&	// BEWARE, VERY LIKELY TO BE TRUE FOR FINISHEDDataAvail
	temp->AcceptFinishedScans)	
      temp->FinishedDataAvail(finisheddata);
    temp = temp->nextclient;
    if ((elapsedtime = lock->elapsed_time()) > lock->lockwaitmax)
      fprintf(stderr, "scan_mng::FinishedDataAvail lock wait time exceeded. (%1.3fsecs) Client type = %s\n", 
	      elapsedtime/100., scanclienttype_strings[temp->ClientType]);
  }
  finisheddata->SetShownToClientsFin(TRUE);
  lock->rel_lock();
}

void scan_mng::PrintScanUsage(FILE *file, bool verbose) {
  scan_client *temp;
  char timestr1[64], timestr2[64];
  if (!file) file = stderr;
  temp = ClientList;
  double cachedsz = cachedScansSize();
  fprintf(file, "scan_mng - useRecentScansCache=%d recentCachePeriod=%d keepScansInCache=%d cacheScanCount=%d\n"
	  "  cachedScansSize=%dMB cachePercentFull=%d%% firstTime=%s lastTime=%s\n",
	  useRecentScansCache, recentCachePeriod, keepScansInCache, 
	  int(recentScans.size()), int(cachedsz/1000000),
	  cachePercentFull(), ShortTimeString(firstCacheTime, timestr1),
	  ShortTimeString(lastCacheTime, timestr2));
  while (temp) {
    temp->PrintScanUsage(file, verbose);	
    temp = temp->nextclient;
  }
  if (ScanRegistry) 
    {
      ScanRegistry->UsageReport(file);
      if (writeDetailedScanReport)
	ScanRegistry->DetailedReport();
    }
}

void scan_mng::SetDataMode(e_scan_client_mode newmode) {
  scan_client *temp;
    
  temp = ClientList;
  while (temp) {
    temp->SetDataMode(newmode);	
    temp = temp->nextclient;
  }
  currentDataMode = newmode;
}

void scan_mng::Check()
{
  purgeRecentScans();
  CheckTime = time(0) + CheckPeriod;  
}

void scan_mng::NewSeqLoaded()
{
#ifdef USE_TITAN_CLIENT
  if (rapicTitanClientMgr)
    rapicTitanClientMgr->checkAllClients();
#endif
#ifdef USE_RAINFIELDS_CLIENT
  if (rainfieldsMgr)
    rainfieldsMgr->newSeqLoaded();
#endif
#ifdef USE_WDSS_CLIENT
  if (wdssMng)
    wdssMng->newSeqLoaded();
#endif
}

scan_registry::scan_registry()
{
  lock = new spinlock("scan_registry", 200);
}

scan_registry::~scan_registry()
{
  clear();
  if (lock)
    delete lock;
}

void scan_registry::add_new_scan(rdr_scan *addscan, string scanstr)
{
  char tempstr[128];
  if (addscan && addscan->isRootScan())      // only add root scans
    {
      if (lock) lock->get_lock();
      map<rdr_scan*, string>::iterator node = new_rdrscan_registry.find(addscan);
      if (node != new_rdrscan_registry.end())
	{
	  sprintf(tempstr, "scan_registry::add_new_scan ERROR, - Attempting to add duplicate scan by %s\n", scanstr.c_str());
	  RapicLog(tempstr, LOG_INFO);
	}
      else
	new_rdrscan_registry.insert(make_pair(addscan,scanstr));
      if (lock) lock->rel_lock();
    }
} 

bool scan_registry::remove_scan(rdr_scan *remscan)
{
  char tempstr[256];
  map<rdr_scan*, string>::iterator new_node;
  typedef multimap<string, rdr_scan*>::iterator mm_iter;
  pair<mm_iter, mm_iter> mm_rng;

  if (remscan && remscan->isRootScan())      // only add root scans
    {
      if (lock) lock->get_lock();
      new_node = new_rdrscan_registry.find(remscan);
      if ((new_node = new_rdrscan_registry.find(remscan)) != new_rdrscan_registry.end())
	{
	  new_rdrscan_registry.erase(new_node);
	  if (lock) lock->rel_lock();
	  return true;
	}
      else 
	{
	  mm_rng = rdrscan_registry.equal_range(remscan->ScanKey());
	  mm_iter node = mm_rng.first;
	  bool found = false;
	  while (!found && (node != mm_rng.second))
	    {
	      if ((found = node->second == remscan))
		rdrscan_registry.erase(node);
	      node++;
	    }
	  if (found)
	    {
	      if (lock) lock->rel_lock();
	      return true;
	    }
	}
      sprintf(tempstr, "scan_registry::remove_scan SERIOUS ERROR, unable to find rdr_scan in registry - %s",
	      remscan->ScanString());
      RapicLog(tempstr, LOG_INFO);
      if (lock) lock->rel_lock();
      return false;
    }       
  else
    return true;
}

/*
  This should only be called by scan_mng or scan_registry destructor on program 
  exit after all other rdr_scan users have already closed and deleted all of their rdr_scans.
  This should not actually need to delete anything.
  NOTE: - it does not honor the rdr_scan UserCount/ShouldDelete reference counting
*/ 
void scan_registry::clear(bool deletescans)
{
  if (lock) lock->get_lock();
  if (deletescans)
    {
      map<rdr_scan*, string>::iterator new_node = new_rdrscan_registry.begin();
      map<rdr_scan*, string>::iterator new_endnode = new_rdrscan_registry.end();
      while (new_node != new_endnode)
	{
	  if (new_node->first)
	    delete new_node->first;
	  new_node++;
	}
      multimap<string, rdr_scan*>::iterator node = rdrscan_registry.begin();
      multimap<string, rdr_scan*>::iterator endnode = rdrscan_registry.end();
      while (node != endnode)
	{
	  if (node->second)
	    delete node->second;
	  node++;
	}
    }
  new_rdrscan_registry.clear();
  rdrscan_registry.clear();
  if (lock) lock->rel_lock();
}

long scan_registry::total_scan_size(bool newscanflag, bool validscanflag)
{
  long total_size = 0;
  if (lock) lock->get_lock();
  if (newscanflag)
    {
      map<rdr_scan*, string>::iterator new_node = new_rdrscan_registry.begin();
      map<rdr_scan*, string>::iterator new_endnode = new_rdrscan_registry.end();
      while (new_node != new_endnode)
	{
	  total_size += new_node->first->scanSetSize();
	  new_node++;
	}
    }
  if (validscanflag)
    {
      multimap<string, rdr_scan*>::iterator node = rdrscan_registry.begin();
      multimap<string, rdr_scan*>::iterator endnode = rdrscan_registry.end();
      while (node != endnode)
	{
	  if (node->second)
	    total_size += node->second->scanSetSize();
	  node++;
	}
    }
  if (lock) lock->rel_lock();
  return total_size;
}

int scan_registry::total_scan_count(bool newscanflag, bool validscanflag)
{
  if (lock) lock->get_lock();
  int size = 0;
  if (newscanflag)
    size += new_rdrscan_registry.size();
  if (validscanflag)
    size += rdrscan_registry.size();
  if (lock) lock->rel_lock();
  return size;
}

void scan_registry::UsageReport(FILE *file)
{
  fprintf(file, "scan_registry contains %d entries, totalsize=%0.1fMB\n",
	  int(total_scan_count()), float(total_scan_size()/1000000));
}

char detailedScanReportName[] = "rp_scan_report.txt";

void scan_registry::DetailedReport(char *fname)
{
  if (!fname) fname = detailedScanReportName;
  
  char tempstr[256];
  FILE *file = fopen(fname, "w");
  if (!file)
    {
      fprintf(stderr, "scan_registry::DetailedReport - FAILED - opening file %s\n", fname);
      perror(0);
      return;
    }
  if (lock) lock->get_lock();
  fprintf(file, "scan_registry::DetailedReport\n"
	  "New scan count   = %5d - New scan size   = %0.2fMB\n"
	  "Valid scan count = %5d - Valid scan size = %0.1fMB\n",
	  int(total_scan_count(true, false)), float(total_scan_size(true, false)/1000000.0),
	  int(total_scan_count(false, true)), float(total_scan_size(false, true)/1000000.0));
  multimap<string, rdr_scan*>::iterator node = rdrscan_registry.begin();
  multimap<string, rdr_scan*>::iterator endnode = rdrscan_registry.end();
  while (node != endnode)
    {
      if (node->second)
	{
	  fprintf(file, "%s - %s\n    Created by-%s Size=%0.3fMB\n ", 
		  node->first.c_str(), node->second->ScanString2(tempstr),
		  node->second->scanCreatorString.c_str(), 
		  float(node->second->scanSetSize()/1000000.0));
	  node->second->DumpUserList(file, "    ");
	}
      node++;
    }
  if (lock) lock->rel_lock();
  fclose(file);
}

// traverse new_rdrscan_registry, move scans with valid headers to rdrscan_registry
void scan_registry::check()
{
  writeDetailedScanReport = FileExists("writeDetailedScanReport");

  if (lock) lock->get_lock();
  map<rdr_scan*, string>::iterator first = new_rdrscan_registry.begin();  
  map<rdr_scan*, string>::iterator this_iter;
  map<rdr_scan*, string>::iterator last = new_rdrscan_registry.end();
  
  while (first != last)
    {
      if (first->first && first->first->HeaderValid())
	{
	  this_iter = first;
	  first++;
	  rdrscan_registry.insert(make_pair(this_iter->first->ScanKey(), this_iter->first));  // store by ScanKey
	  new_rdrscan_registry.erase(this_iter);
	}
      else
	first++;
    }
  if (lock) lock->rel_lock();
}
