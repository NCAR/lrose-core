/*
		siteinfo.c
		Radar site information utilities
Changelog:

16 Dec 2005    S.Dance
Add machine readable radar fault status file for Intelligent Alerts Google Maps dispaly
   (see IA system, radar_faults_doc.txt for documentation)

*/

#include "rdr.h"
#include "siteinfo.h"
#include "rdrutils.h"
// #include <bstring.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "utils.h"

stnSet radlVelLockSet; // set of radlVel lockout flags
char radlVelLockFName[] = "radlVelLocks";
char radlVelLockAllFName[] = "radlVelLockAll";

bool radlVelLockedOut(int stnid)
{
  return radlVelLockSet.isSet(stnid);
}

void readRadlVelLocks()
{
  radlVelLockSet.clear();
  if (FileExists(radlVelLockAllFName))
    {
      radlVelLockSet.setAll();
      fprintf(stdout, "radlVelLockAllFName found - locking Radl Velocity display for ALL radars\n");
      return;
    }
  
  char tempstr[128];
  int stnid = 0;
  FILE    *radlVelLockFile = fopen(radlVelLockFName, "r");  
  if (radlVelLockFile)
    {
      while (fgets(tempstr, 128, radlVelLockFile))
	{
	  if (tempstr[0] != '#') 
	    {
	      if (strstr(tempstr, "All"))
		{
		  radlVelLockSet.setAll();
		  fprintf(stdout, "radlVelLockFile found - locking Radl Velocity display for ALL radars\n");
		  fclose(radlVelLockFile);
		  return;
		}
	      else
		{
		  if ((stnid = decode_stnstr(tempstr)))
		    {
		      radlVelLockSet.set(stnid);
		      fprintf(stdout, "radlVelLockFile found - locking Radl Velocity display for %s\n", stn_name(stnid));
		    }
		}
	    }
	}
      fclose(radlVelLockFile);
    }
}      

const short  LastStn=StnDefCount - 1;  // ID of last defined stn
StnRecArray StnRec;
RdrRecArray RdrRec;
char BadStnIDStr[] = "Invalid";
char coveragePath[] = "coverage/";
void InitStnArray() {

  memset((void*)StnRec, 0, sizeof(StnRecArray));
}

void InitRdrArray() {

  memset((void*)RdrRec, 0, sizeof(RdrRecArray));
  strcpy(RdrRec[0].Name,"No Radar");
}

void ReadStnInfo(bool loadCoverage) {

  StnRecType 	LStnRec;
  FILE    *SiteFile;
  char    LineBuff[128];
  int	    StnID,x,y;
  int	    debug = FALSE;
  bool    coverageDirExists = DirExists(coveragePath);
  bool    coverageLoadFailed = false;
  float   lat, lng, ht;

  if ((SiteFile = fopen(SiteInfoFName,"r")) == 0) {
    printf("ERROR OPENING %s\n",SiteInfoFName);
    return;
  }
  while (fgets(LineBuff,128,SiteFile) &&		// skip to radar site data
	 (strncmp(LineBuff,"RADAR SITE DATA",15) != 0)) {
    if (debug) printf(LineBuff);
  }
  if (debug) printf(LineBuff);
  if (feof(SiteFile)) {
    fclose(SiteFile);
    printf("NO RADAR SITE DATA FOUND IN %s\n",SiteInfoFName);
    return;
  }
  while (fgets(LineBuff,128,SiteFile) && 			// read radar site data
	 (strncmp(LineBuff,"END RADAR SITE DATA",19) != 0)) {
    if (sscanf(LineBuff,"%d %s %f %f %f %d",&StnID,LStnRec.Name,&lat,
	       &lng,&ht,&LStnRec.Rdr) == 6) {
      // remove trailing '_' chars 
      ht *= 0.001;	// convert metres to km
      LStnRec.set(-lat, lng, ht); // siteinfo.txt file lat is +ve south, flip it to +ve north
      x = strlen(LStnRec.Name) - 1; // remove trailing '_'
      while (x && (LStnRec.Name[x] == '_')) {
	LStnRec.Name[x] = 0;
	x--;
      }
      LStnRec.safeFileName(LStnRec.fName);   // make name suitable for use in file names
      if ((StnID <= LastStn) && (StnID > 0)) 				// put rec into array
	StnRec[StnID] = LStnRec;
      if (loadCoverage && coverageDirExists)
	{
	  ReadCoverageData(StnID);
	  coverageLoadFailed |= !StnRec[StnID].coverage;
	}
      StnRec[StnID].valid = true;
    }
    else if (debug) printf("ERROR READING %s\n",LineBuff);
  }
  if (debug) {
    for (x=0;x<LastStn;x++) {
      LStnRec = StnRec[x];
      printf("%d %s %f %f %f %d\n",x,LStnRec.Name,
	     LStnRec.Lat(),LStnRec.Lng(),LStnRec.Ht(),LStnRec.Rdr);
    }
  }
  if (coverageDirExists && coverageLoadFailed) {
    y = 0;
    printf("ReadStnInfo - Error loading coverage data for - \n");
    for (x=0;x<LastStn;x++) {
      if (StnRec[x].valid && !StnRec[x].coverage)
	{
	  printf("%8s(%02d) ",StnRec[x].Name,x);
	  y++;
	  if ((y % 6) == 0)
	    printf("\n  ");
	}
    }
    printf("\n");
  }
  fclose(SiteFile);
  if (debug) printf("%s\n",LineBuff);
}

void ReadRdrInfo() {
    
  RadarParamType    LRdrRec;
  FILE	    *SiteFile;
  char	    LineBuff[128];
  int	    RdrID,x;
  int	    debug = FALSE;
  char	    *ch;
    
  if ((SiteFile = fopen(SiteInfoFName,"r")) == 0) {
    printf("ERROR OPENING %s\n",SiteInfoFName);
    return;
  }
  while (fgets(LineBuff,128,SiteFile) &&		// skip to radar type data
	 (strncmp(LineBuff,"RADAR TYPE DATA",15) != 0)) {
    //if (debug) printf(LineBuff);
  }
  if (debug) printf(LineBuff);
  if (feof(SiteFile)) {
    fclose(SiteFile);
    printf("NO RADAR TYPE DATA FOUND IN %s\n",SiteInfoFName);
    return;
  }
  while (fgets(LineBuff,128,SiteFile) && 			// read radar type data
	 (strncmp(LineBuff,"END RADAR TYPE DATA",19) != 0)) {
    if (sscanf(LineBuff,"%d %s %d %d %d",&RdrID,LRdrRec.Name,
	       &LRdrRec.WaveLength,&LRdrRec.AzBeamWidth,
	       &LRdrRec.ElBeamWidth) == 5) {
      ch = LRdrRec.Name;
      while (*ch) {   // repl '_' chars with ' ' 
	if (*ch == '_') *ch = ' ';
	ch++;
      }
      /*
	x = 0;
	while (LRdrRec.Name[x]) { 	// repl _ chars with space
	if (LRdrRec.Name[x] == '_') LRdrRec.Name[x] = ' ';
	x++;
	}
      */
      if ((RdrID < RdrTypeCount) && (RdrID > 0)) 				// put rec into array
	RdrRec[RdrID] = LRdrRec;
    }
    else if (debug) printf("ERROR READING %s\n",LineBuff);
  }
  if (debug) {
    for (x=0;x<RdrTypeCount;x++) {
      LRdrRec = RdrRec[x];
      printf("%d %s %d %d %d\n",x,
	     LRdrRec.Name,LRdrRec.WaveLength,
	     LRdrRec.AzBeamWidth,LRdrRec.ElBeamWidth);
    }
  }
  fclose(SiteFile);
  if (debug) printf("%s\n",LineBuff);
}

void	GetSiteInfo(bool loadCoverage) {
  int	debug = TRUE;

  InitStnArray();
  ReadStnInfo(loadCoverage);
  InitRdrArray();
  ReadRdrInfo();
  readRadlVelLocks();
  if (debug) printf("SITEINFO READ OK\n");
}

char* stn_name(int stn, bool fsafe) {
  if ((stn > 0) && (stn < LastStn))
    {
      if (fsafe)
	return StnRec[stn].fName;
      else
	return StnRec[stn].Name;
    }
  else return BadStnIDStr;
}
	
void StnRecType::safeFileName(char *namebuff)
{
  int chpos = 0;

  while (Name[chpos]) {	    // convert any problem fname chars to _
    if (Name[chpos] == '/') namebuff[chpos] = '_';
    else if (Name[chpos] == ' ') namebuff[chpos] = '_';
    else if (Name[chpos] == '*') namebuff[chpos] = '_';
    else if (Name[chpos] == '\'') namebuff[chpos] = '_';
    else if (Name[chpos] == '-') namebuff[chpos] = '_';
    else namebuff[chpos] = Name[chpos];
    chpos++;
  }
  namebuff[chpos] = '\0';
}

/*
 * This function decodes the given stnstr
 * If the string is a stnid,  the stnid value is returned
 * otherwise the stnid matching the given string
 * 
 */
int decode_stnstr(const char *stnstr, bool CaseSensitive) {
  int stnid = 0;
  if (isalpha(stnstr[0])) 
    stnid = get_stn_id(stnstr, CaseSensitive);	// NOT case sensitive by default
  else if (sscanf(stnstr, "%d", &stnid) != 1)
    return 0;   // couldn't convert to int, return fail
  return stnid;
}

int   get_stn_id(const char *stnstr, bool CaseSensitive) {
  bool match = FALSE;
  int x = 0;
    
  if (strlen(stnstr) == 0) return 0;
  // special value "ANY" used by filters, returns -1
  if (strcasecmp(stnstr, "ANY") == 0) return -1;
  if (CaseSensitive)
    while ((x <= LastStn) && (!strstr(StnRec[x].Name, stnstr)))
      x++;
  else    // convert
    while ((x <= LastStn) && !match) {
      match = strncasecmp(StnRec[x].Name, stnstr, NonWhiteLength(stnstr)) == 0;
      if (!match) x++;
    }
  if (x <= LastStn) return x;
  else return 0;
}

float ElBeamWidth(int stn) {
  return RdrRec[StnRec[stn].Rdr].ElBeamWidth/10;
}

float AzBeamWidth(int stn) {
  return RdrRec[StnRec[stn].Rdr].AzBeamWidth/10;
}

void  ReadCoverageData(int StnID)
{
  char filename[256], tempstr[128];
  cCoverage *newCoverage;

  StnRec[StnID].safeFileName(tempstr);
  sprintf(filename, "%s%s-coverage.txt", coveragePath, tempstr);
  newCoverage = new cCoverage(filename);
  if (newCoverage->valid)
    {
      if (StnRec[StnID].coverage)
	{
	  delete StnRec[StnID].coverage;
	  StnRec[StnID].coverage = 0;
	}
      StnRec[StnID].coverage = newCoverage;
    }
  else
    delete newCoverage;
}

bool cCoverage::readFile(char *fname)
{
  FILE *file = NULL;
  char linebuff[512];
  int az;
  float rng1, rng2;
    

  file = fopen(fname, "r");
  if (!file)
    {
      if (!quiet)
	{
	  fprintf(stderr, "cCoverage::readFile - FAILED to open file - %s\n", fname);
	  perror(0);
	}
      return false;
    }

  fgets(linebuff,512,file);
  if (sscanf(linebuff, "horizon1=%f", &hor1_height) != 1)
    {
      if (!quiet)
	{
	  fprintf(stderr, "cCoverage::readFile - Format error in file - %s\n", fname);
	  perror(0);
	  fprintf(stderr, "  Expected horizon1= - File line is %s\n", linebuff);
	}
      return false;
    }

  fgets(linebuff,512,file);
  if (sscanf(linebuff, "horizon2=%f", &hor2_height) != 1)
    {
      if (!quiet)
	{
	  fprintf(stderr, "cCoverage::readFile - Format error in file - %s\n", fname);
	  perror(0);
	  fprintf(stderr, "  Expected horizon2= - File line is %s\n", linebuff);
	}
      return false;
    }

  if (!horizon1)
    {
      horizon1 = new float[360];
    }
  if (!horizon2)
    {
      horizon2 = new float[360];
    }

  for (az = 0; az < 360; az++)
    {
      horizon1[az] = 440;
      horizon2[az] = 440;
    }

  while (!feof(file))
    {
      fgets(linebuff,512,file);
      if (sscanf(linebuff, " %d, %f, %f",
		 &az, &rng1, &rng2) == 3)
        {
	  if ((az >= 0) && (az < 360))
            {
	      horizon1[az] = rng1;
	      horizon2[az] = rng2;
            }
        }
    }
  valid = true;
  return true;
}

void stnSet::set(string stnstr)
{
  string::size_type len = stnstr.size();
  string::size_type next_delim = 0, prev_begin = 0;
  string tempstr;
  int stn_id = 0;
  clear();
  do 
    {
      next_delim = stnstr.find(",", next_delim+1);
      if (next_delim == string::npos)   // no further tokens after this
	tempstr = stnstr.substr(prev_begin, len - prev_begin);
      else
	tempstr = stnstr.substr(prev_begin, next_delim - prev_begin);
      if ((stn_id = decode_stnstr(tempstr.c_str(), false)))
	set(stn_id);
      prev_begin = next_delim+1; // point to next char after ','
    }
  while (next_delim != string::npos);
}

void stnSet::set(stnSet &_stnset)
{
  clear();
  stnset = _stnset.stnset;
}

void stnSet::stnList(string& stnlist, bool clearstr) 
{
  int count = 0;
  if (clearstr)
    stnlist.clear();
  for (int x = 0; x < int(stnset.size()); x++)
    {
      if (stnset[x])
	{
	  if (count)  // no "," before first entry
	    stnlist += ",";
	  stnlist += stn_name(x);
	  count++;
	}
    }
  listStr = stnlist;
}
      
const char *stnSet::stnListStr() 
{
  int count = 0;
  listStr.clear();
  if (stnset.size())
    for (int x = 0; x < int(stnset.size()); x++)
      {
	if (stnset[x])
	  {
	    if (count)  // no "," before first entry
	      listStr += ",";
	    listStr += stn_name(x);
	    count++;
	  }
      }
  else
    listStr = "None";
  return listStr.c_str();
}

int stnSet::stnNearest(float lat, float lng, float &stndist)
{
  float nearest_dist = -1;
  float dist;
  int nearest_stn = 0;
  LatLongHt LLH;
  LLH.set(lat, lng);
  
  for (int stn = 1; stn < int(stnset.size()); stn++)
    {
      if (stnset[stn])
	{
	  dist = LatLongStnKmDiff(&LLH, stn);
	  if (!nearest_stn || 
	      (nearest_dist < 0) ||
	      (dist < nearest_dist))
	    {
	      nearest_stn = stn;
	      nearest_dist = dist;
	    }
	}
    }
  stndist = nearest_dist;
  return nearest_stn;
}
  
void stnSetRefCount::stnList(string& stnlist, bool clearstr) 
{
  map<int, int>::iterator iter = stnrefmap.begin();
  map<int, int>::iterator iterend = stnrefmap.end();
  int count = 0;

  if (clearstr)
    stnlist.clear();
  while (iter != iterend)
    {
      if (count)  // no "," before first entry
	stnlist += ",";
      stnlist += stn_name(iter->first);
      count++;
      iter++;
    }
  listStr = stnlist;
}
      
  
const char *stnSetRefCount::stnListStr(string *stnlist) 
{
  if (!stnlist)
    stnlist = &listStr;
  stnlist->clear();

  map<int, int>::iterator iter = stnrefmap.begin();
  map<int, int>::iterator iterend = stnrefmap.end();
  int count = 0;
  if (stnrefmap.size())
    while (iter != iterend)
      {
	if (count)  // no "," before first entry
	  listStr += ",";
	*stnlist += stn_name(iter->first);
	iter++;
	count++;
      }
  else
    *stnlist = "None";
  return stnlist->c_str();
}

char radarFaultStatusFAULT[] = "UNDEFINED FAULT";

bool radarFaultStatus::setFaulty(int faultno,
				 char* faultstr,
				 time_t fault_time)
{
  bool changed = false;
  if (!fault_time)         // if not defined use "now"
    fault_time = time(NULL);
  if (lastUpdateTime != fault_time)
    {
      lastUpdateTime = fault_time;
      changed = true;
    }
  if (!faultstr)           // if not defined use UNDEFINED FAULT
    faultstr = radarFaultStatusFAULT;
  if (faultno != faultNo)  // new fault state
    {
      changed = true;
      if (faultno == 0)    // fault cleared, 
	{
	  prevFaultNo = faultNo;
	  lastResolvedTime = fault_time;
	  faultNo = 0;
	}
      else                 // new fault type
	{
	  if (!faultNo)    // previously not faulty, set start time
	    {
	      lastStartTime = fault_time;
	      lastFaultStr = faultstr;
	      prevFaultNo = faultNo = faultno;
	    }
	  else             // previously faulty, this is a new fault
	    {
	      cout << "radarFaultStatus::setFaulty - Changing fault state from "
		   << lastFaultStr << "(" << faultNo << ") to "
		   << faultstr << "(" << faultno << ")\n";
	      lastFaultStr = faultstr;  // change fault string, not start time
	      prevFaultNo = faultNo = faultno;
	    }
	}
    }
  return changed;
}

bool radarFaultStatus::setFaultCleared(time_t resolved_time) 
{
  return setFaulty(0, NULL, resolved_time);   // clears faulty state
}

char *latenessString(time_t secslate, char *strbuff)
{
  int days = secslate /(3600 * 24),
    hours = (secslate % (3600 * 24)) / 3600,
    mins = (secslate % 3600) / 60;
  if (secslate <= 900)   // allow 15 minutes
    strbuff[0] = '\0'; // null string
  else
    {
      if (days)
	sprintf(strbuff, "**LATE BY %dd %02dhr %02dmin", days, hours, mins);
      else
	sprintf(strbuff, "**LATE BY %02dhr %02dmin", hours, mins);
    }
  return strbuff;
}     

void radarFaultStatus::dumpStatus(time_t dumptime,
				  FILE *dumpfile, 
				  FILE *mcdumpfile, 
				  bool faulty_only,
				  bool show_last)
{
  char timestr[128], timestr2[128], timestr3[128];
  char latestrbuff[128];
  char faultStr[128];
  
  if (!dumpfile || !mcdumpfile ||
      (faulty_only && (faultNo == 0))) // skip non faulty
    return;

  if (faultNo) {
    fprintf(dumpfile, "%8s(%2d) **FAULT %d %s\n"
	    "     ==>Since %s Updated %s %s\n",
	    stn_name(radar), radar, faultNo, lastFaultStr.c_str(),
	    TimeString(lastStartTime, timestr, true, true),
	    TimeString(lastUpdateTime, timestr2, true, true),
	    latenessString(dumptime - lastUpdateTime - 600, latestrbuff));

    //remove commas so they wont break our comma separated file
    strcpy(faultStr,lastFaultStr.c_str());
    char* charpos;
    while((charpos = strchr(faultStr,','))) *charpos = ';';
    
    if (dumptime - lastUpdateTime - 600 <= 900)
      fprintf(mcdumpfile, "%2d, %d, %s, OK, %ld, %s, %f, %f\n",
	      radar, faultNo, faultStr,
	      lastUpdateTime,stn_name(radar),StnRec[radar].Lat(),StnRec[radar].Lng());
    else 
      fprintf(mcdumpfile, "%2d, %d, %s, LATE, %ld, %s, %f, %f\n",
	      radar, faultNo, faultStr,
	      lastUpdateTime,stn_name(radar),StnRec[radar].Lat(),StnRec[radar].Lng());
    
  }
  
  else
    {
      if (show_last && lastStartTime)
	fprintf(dumpfile, "%8s(%2d) OK Updated %s %s\n"
		"     ==>Last fault %d %s\n"
		"     ==>Started %s Cleared %s\n",
		stn_name(radar), radar, TimeString(lastUpdateTime, timestr3, true, true),
		latenessString(dumptime - lastUpdateTime - 600, latestrbuff),
		prevFaultNo, lastFaultStr.c_str(),
		TimeString(lastStartTime, timestr, true, true),
		TimeString(lastResolvedTime, timestr2, true, true));
      else
	fprintf(dumpfile, "%8s(%2d) OK Updated %s %s\n",
		stn_name(radar), radar,
		TimeString(lastUpdateTime, timestr2, true, true),
		latenessString(dumptime - lastUpdateTime - 600, latestrbuff));

      if (dumptime - lastUpdateTime - 600 <= 900)
	fprintf(mcdumpfile, "%2d, %d, NOFAULT, OK, %ld, %s, %f, %f\n",
		radar, faultNo, 
		lastUpdateTime,stn_name(radar),StnRec[radar].Lat(),StnRec[radar].Lng()); 
      else 
	fprintf(mcdumpfile, "%2d, %d, NOFAULT, LATE, %ld, %s, %f, %f\n",
		radar, faultNo, 
		lastUpdateTime,stn_name(radar),StnRec[radar].Lat(),StnRec[radar].Lng());
    }
  
  
  
}
  
bool currentRadarFaultStatus::setFaulty(int rdr, 
					int faultno,
					char* faultstr,
					time_t fault_time)
{
  lock->get_lock();
  bool changed = false;
  map<int, radarFaultStatus>::iterator iter = faultStatus.find(rdr);
  if (iter == faultStatus.end())
    {
      faultStatus[rdr].setRadar(rdr);
      iter = faultStatus.find(rdr);
      changed = true;
    }
  if (iter != faultStatus.end())   // an entry exists for this stn
    {
      changed |= iter->second.setFaulty(faultno, faultstr, fault_time);
      faultySet.set(rdr, faultno != 0);
    }
  else  // an entry for this doesn't exist for some reason, should never happen
    {
      cout << "currentRadarFaultStatus::setFaulty - ERROR - FAILED TO CREATE ENTRY FOR RADAR=" << rdr << " *****\n";
    }
  lock->rel_lock();
  if (changed)
    {
      dumpStatus();
    }
  return changed;
}

void currentRadarFaultStatus::setFaultCleared(int rdr, 
					      time_t resolved_time)
{
  setFaulty(rdr, 0, NULL, resolved_time);
}

int currentRadarFaultStatus::fault(int rdr)
{
  lock->get_lock();
  map<int, radarFaultStatus>::iterator iter = faultStatus.find(rdr);
  lock->rel_lock();
  if (iter == faultStatus.end())
    return 0;
  else
    return iter->second.fault();
}

void currentRadarFaultStatus::getFaultyRadarSet(stnSet &faultyset)
{
  faultyset.stnset = faultySet.stnset;
} 

string& currentRadarFaultStatus::faultString(int rdr)
{
  lock->get_lock();
  map<int, radarFaultStatus>::iterator iter = faultStatus.find(rdr);
  lock->rel_lock();
 if (iter == faultStatus.end())
    return notdefinedStatus.faultString();
  else
    return iter->second.faultString();
}

time_t currentRadarFaultStatus::faultStartedTime(int rdr)
{
  lock->get_lock();
  map<int, radarFaultStatus>::iterator iter = faultStatus.find(rdr);
  lock->rel_lock();
  if (iter == faultStatus.end())
    return notdefinedStatus.lastFaultStartedTime();
  else
    return iter->second.lastFaultStartedTime();
}

time_t currentRadarFaultStatus::faultResolvedTime(int rdr)
{
  lock->get_lock();
  map<int, radarFaultStatus>::iterator iter = faultStatus.find(rdr);
  lock->rel_lock();
  if (iter == faultStatus.end())
    return notdefinedStatus.lastFaultResolvedTime();
  else
    return iter->second.lastFaultResolvedTime();
}

char defaultRadarFaultDumpName[] = "radar_fault.status";
char McRadarFaultDumpNameNew[100];
char McRadarFaultDumpName[100];

void currentRadarFaultStatus::dumpStatus(char *dumpfilename)
{
  lock->get_lock();
  if (!dumpfilename)
    dumpfilename = defaultRadarFaultDumpName;
  
  //add .dat to dump file name for machine (mc) readable version
  strcpy(McRadarFaultDumpNameNew,dumpfilename);
  strcat(McRadarFaultDumpNameNew,".dat.new");
  strcpy(McRadarFaultDumpName,dumpfilename);
  strcat(McRadarFaultDumpName,".dat");
  
  FILE *dumpfile = fopen(dumpfilename, "w");
  if (!dumpfile)
    {
      cout << "currentRadarFaultStatus::dumpStatus FAILED to open file - "
	   << dumpfilename << endl;
      return;
    }
  FILE *mcdumpfile = fopen(McRadarFaultDumpNameNew, "w");
  if (!mcdumpfile)
    {
      cout << "currentRadarFaultStatus::dumpStatus FAILED to open machine readable file - "
	   << McRadarFaultDumpNameNew << endl;
      fclose(dumpfile);
      return;
    }
  char timestr[128];
  fprintf(dumpfile, "Radar Fault Status as at %s\n",
	  TimeString(time(NULL), timestr, true, true));
  fprintf(mcdumpfile,"radar,fault_no,fault_string,lateness,last_update,name,lat,long\n");
  
  
  map<int, radarFaultStatus>::iterator iter = faultStatus.begin();
  map<int, radarFaultStatus>::iterator iterend = faultStatus.end();
  while (iter != iterend)
    {
      iter->second.dumpStatus(time(NULL), dumpfile,mcdumpfile);
      iter++;
    }
  fclose(dumpfile);
  fclose(mcdumpfile);
  //rename so FAM wont trigger on start of file write
  if (rename(McRadarFaultDumpNameNew,McRadarFaultDumpName) == -1)
    {
      cerr << "\ncurrentRadarFaultStatus::dumpStatus: " << 
	"ERROR  Cannot rename machine readable output file from " << McRadarFaultDumpNameNew << " to " <<
	McRadarFaultDumpName << "\n";
      perror("currentRadarFaultStatus::dumpStatus");
    }
  lock->rel_lock();
}

currentRadarFaultStatus::currentRadarFaultStatus()
{
  lock = new spinlock("currentRadarFaultStatus", float(5.0));
}

currentRadarFaultStatus::~currentRadarFaultStatus()
{
  delete lock;
}

currentRadarFaultStatus currentRadarFaults;

void stnList::appendStn(int newstn)
{
  stnlist.push_back(newstn);
  listSet.set(newstn);
}

int stnList::firstStn()
{
  if (stnlist.size() > 0)
    return stnlist[0];
  else
    return 0;
}

int stnList::lastStn()
{
  if (stnlist.size() > 0)
    return stnlist[stnlist.size()-1];
  else
    return 0;
}

int stnList::nextStn(int thisstn)
{
  int pos = listPos(thisstn);
  if (pos < 0)
    return 0;
  if (pos >= int(stnlist.size()-1))
    return stnlist[0];
  else 
    return stnlist[pos+1];
}

int   stnList::prevStn(int thisstn)
{
  int pos = listPos(thisstn);
  if ((pos < 0) ||
      (pos > int(stnlist.size())))
    return 0;
  if (pos == 0)
    return stnlist[stnlist.size()-1];
  else 
    return stnlist[pos-1];
}

void  stnList::init(char *initstr) // comma separated list of stns by id or name
{
  if (!initstr || !strlen(initstr))
    return;
  string stnstr = initstr;
  string::size_type len = stnstr.size();
  string::size_type next_delim = 0, prev_begin = 0;
  string tempstr;
  int stn_id = 0;
  clear();
  do 
    {
      next_delim = stnstr.find(",", next_delim+1);
      if (next_delim == string::npos)   // no further tokens after this
	tempstr = stnstr.substr(prev_begin, len - prev_begin);
      else
	tempstr = stnstr.substr(prev_begin, next_delim - prev_begin);
      if ((stn_id = decode_stnstr(tempstr.c_str(), false)))
	appendStn(stn_id);
      prev_begin = next_delim+1; // point to next char after ','
    }
  while (next_delim != string::npos);
}

int stnList::listPos(int thisstn)
{
  int pos = 0;
  if (!listSet.isSet(thisstn))
    return -1;
  while ((stnlist[pos] != thisstn) &&
	 (pos < int(stnlist.size())))
    pos++;
  if (pos < int(stnlist.size()))
    return pos;
  else 
    return -1;
}
      
