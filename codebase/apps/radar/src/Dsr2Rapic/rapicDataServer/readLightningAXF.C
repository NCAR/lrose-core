/*

  Implementation of readLightningAXFData class

e.g.

[DESCRIPTION]
system[30]="GPATS"
product[30]="Lightning"
aifstime[30]="20061011015414"
[$]

[POINT]
point[30]
"Data"
[$]

[Data]
aifstime[30],Lat,Lon,kAmps
"20061011015030",5.788934,114.106373,-19.2
"20061011015037",-51.993274,135.678025,-28.8
"20061011015217",-51.905874,135.595348,11.5
[$]

*/

#include "readLightningAXF.h"
#include "utils.h"
#include <iostream>
#include "errno.h"
#include <map>
#include "rdrutils.h"

  // Keep record of recent ltningAXF file times from 
  // [DESCRIPTION]
  // aifstime[30]="20061011015414"
  // entry - NOT FILE NAME
  // To be used to prevent re-reading same file
  int maxltningAXFFileMapSize = 200;
  std::map<time_t, string> ltningAXFFileMap;

// if duplicate time already in map return name of matching file
char *ltningAXFFileInMap(time_t tm)
{
  std::map<time_t, string>::iterator iter = ltningAXFFileMap.find(tm);
  
  if (iter != ltningAXFFileMap.end())
    return (char *)iter->second.c_str();
  else
    return NULL;
}

char *addLtningAXFFileToMap(time_t tm, char *fname)
{
  if (!fname)
    return NULL;
  char *tempstr = NULL;
  if ((tempstr = ltningAXFFileInMap(tm)))
    {
      fprintf(stdout, "addLtningAXFFileToMap - Failed to add file %s"
	      " - Has same time as %s\n",
	      fname, tempstr); 
      return tempstr;
    }
  else
    ltningAXFFileMap[tm] = fname;
  while (int(ltningAXFFileMap.size()) > maxltningAXFFileMapSize)
    ltningAXFFileMap.erase(ltningAXFFileMap.begin());
  return tempstr;
}

void ltningAXFFileMapClear()
{
  ltningAXFFileMap.clear();
}

readLightningAXF::readLightningAXF(char *axfname)
{
  init();
  if (axfname)
    setFName(axfname);
}

readLightningAXF::~readLightningAXF()
{
  if (axfFile)
    {
      fclose(axfFile);
      axfFile = NULL;
    }
}

void readLightningAXF::init()
{
  axfName = "latest.axf";
  axfFile = NULL;
  lastReadStartTime = 
    lastReadEndTime = 0;
  thisReadStartTime = 
    thisReadEndTime = 0;
  delimiter = ",";
  debug = 0;
  fieldData.resize(4);  // initially allow for 4 fields
  resetFPos();
}


void readLightningAXF::resetFPos()  // reset field positions
{
  timeF = latF = longF = kampsF = -1;
}

void readLightningAXF::setFName(char *axfname)
{
  if (axfname)
    axfName = axfname;
}

int readLightningAXF::readFile(char *axfname,
			 time_t start_time,
			 time_t end_time)
{
  int obscount = -1;
  time_t filetime=0;
  char *tempstr = NULL;
//   char tempstr[512];
  if (axfFile)
    {
      fclose(axfFile);
      axfFile = NULL;
    }
  if (!axfname)
    axfname = (char *)axfName.c_str();
  if (strlen(axfname))
    axfFile = fopen(axfname, "r");
  if (axfFile)
    {
      lastReadStartTime = thisReadStartTime;
      lastReadEndTime = thisReadEndTime;
      thisReadStartTime = thisReadEndTime = 0;
      lastReadObsCount = 
	lastReadAddedCount = 0;
      if ((filetime=readDescr()))
	{
	  if ((tempstr = addLtningAXFFileToMap(filetime, axfname)))
	    fprintf(stdout, 
		    "readLightningAXF::readFile - Duplicate filetime found (%s)"
		    "- Skipping file %s\n", tempstr, axfname);
	  else
	    obscount = readObs(start_time, end_time);
	}
      else
	fprintf(stdout, 
		"readLightningAXF::readFile - No valid filetime found"
		"- Skipping file%s\n", axfname);
      fclose(axfFile);
      char timestr[64], timestr2[64];
      if (obscount >= 0)
	fprintf(stdout, "readLightningAXF::readFile - Obs Read=%d Added=%d from %s\n"
		"File start=%s end=%s\n%d secs after prev end\n", 
		lastReadObsCount, lastReadAddedCount, axfname, 
		ShortTimeString(thisReadStartTime, timestr),
		ShortTimeString(thisReadEndTime, timestr2),
		int(thisReadEndTime - lastReadEndTime));
      axfFile = NULL;
    }
  else
    {
//       strerror_r(errno, tempstr, 512);
      fprintf(stdout, "readLightningAXF::readFile - Failed opening file %s\n",
	      axfname);
      perror(NULL);
    }
  return obscount;
}

time_t readLightningAXF::readDescr()
{
  char tempBuff[512];
  bool done = false;
  if (!axfFile) return 0;
  // skip to start of [DESCRIPTION] section
  while (!done)
    {
      if (fgets(tempBuff, 512, axfFile))
	done = strstr(tempBuff, "[DESCRIPTION]");
      else
	done = true;
    }
  if (!done) return 0;  // didn't find [DESCRIPTION]
  
  fieldCount = 0;
  int reqdfields = 0;   // some fields optional, 4 reqd fields
  done = false;
  time_t filetime = 0;
   while (fgets(tempBuff, 512, axfFile) && !done)
     {
       done = strstr(tempBuff, "[$]");
       if (!done)
 	{
 	  if (fieldCount > int(fieldData.size()))
 	    fieldData.resize(fieldCount);
 	  fieldData[fieldCount].readFieldData(tempBuff, fieldCount);
 	  if (fieldData[fieldCount].valid)
 	    {
 	      if (fieldData[fieldCount].fieldNameMatch("aifstime"))
 		{
		  timeF = fieldCount;
 		  reqdfields++;
		  char timestr[512];
		  //aifstime[30]="20061011015414"
		  if (sscanf(tempBuff, "aifstime[30]=%*c%14s", timestr) == 1)
		    filetime = DateTimeStr2UnixTime(timestr);
 		}
// 	      else if (fieldData[fieldCount].fieldNameMatch("lat"))
// 		{
// 		  latF = fieldCount;
// 		  reqdfields++;
// 		}
// 	      else if (fieldData[fieldCount].fieldNameMatch("lon"))
// 		{
// 		  longF = fieldCount;
// 		  reqdfields++;
// 		}
// 	      else if (fieldData[fieldCount].fieldNameMatch("kAmps"))
// 		{
// 		  tmpF = fieldCount;
// 		  reqdfields++;
// 		}
 	    }
 	  fieldCount++;  // pre-increment field count, decr below if not recognised
 	}
     }
//   return reqdfields >= 4;
  timeF = 0;            // hard wire fields - DESCRIPTION doesn't define them
  latF = 1;
  longF = 2;
  kampsF = 3;
  fieldData[timeF].set("aifstime", axf_string, timeF);
  fieldData[latF].set("lat", axf_float, latF);
  fieldData[longF].set("long", axf_float, longF);
  fieldData[kampsF].set("kAmps", axf_float, kampsF);
  fieldCount = 4;
  return filetime;
}

int readLightningAXF::getFieldOfs(string &obsString)  // get offsets for each field
{                                 // results in fieldsOfs
  if (fieldCount > int(fieldOfs.size()))
    fieldOfs.resize(fieldCount);
  string::size_type pos = 0;
  int count = 0;
  fieldOfs[0].ofs = 0; // first field at start of line
  while ((pos = obsString.find(delimiter, pos+1)) != string::npos)
    {
      fieldOfs[count].wid = pos - fieldOfs[count].ofs;
      count++;
      fieldOfs[count].ofs = pos+1; // point to char after delimiter
    }
  if (count)
    fieldOfs[count].wid = obsString.size() - fieldOfs[count].ofs;
  count++;
  return count;
}

// check the field pos from the DESCRIPTION matches the pos from the
// first DATA data line
// ****Not yet implemented
bool readLightningAXF::checkFPos(char *obsstring)
{
  if (!obsstring) return false;
  return true;
}

int readLightningAXF::readObs(time_t start_time,
			time_t end_time)
{
  char tempBuff[1024];
  bool done = false;
  if (!axfFile) return -1;
  // skip to start of [Data] section
  while (!done)
    {
      if (fgets(tempBuff, 1024, axfFile))
	done = strstr(tempBuff, "[Data]");
      else
	done = true;
    }
  if (!done) return false;  // didn't find [SURFACE]
  if (!fgets(tempBuff, 1024, axfFile)) // skip
    return false;
  else
    checkFPos(tempBuff);
  int obscount = 0;
  done = false;
  while (fgets(tempBuff, 1024, axfFile) && !done)
    {
      done = strstr(tempBuff, "[$]");
      if (!done)
	{
	  if (readObsString(tempBuff, start_time, end_time))
	    {
	      obscount++;
	    }
	}
    }
  return obscount;
}
 
bool readLightningAXF::readObsString(char *obsstr,
			       time_t start_time,
			       time_t end_time)
{
  if (!obsstr) return false;
  string obsString = obsstr;
  int fieldOfsCount = getFieldOfs(obsString);
  if (fieldOfsCount != fieldCount)
    {
      cout << "readLightningAXF::readObsString ERROR - fieldCount=" << fieldCount <<
	" fieldOfsCount=" << fieldOfsCount << " - " << obsString << endl;
      return false;
    }
  time_t temptime = getFieldTime(obsString, timeF);

  if (!thisReadStartTime || 
      (temptime < thisReadStartTime))
    thisReadStartTime = temptime;
  if (!thisReadEndTime || 
      (temptime > thisReadEndTime))
    thisReadEndTime = temptime;
  
  if (start_time || end_time)  // if out of time range, skip
    {
      if ((start_time &&
	   (temptime < start_time)) ||
	  (end_time &&
	   (temptime > end_time)))
	return false;
    }
  lightningData *newlightningdata = newLightningData();
  if (!newlightningdata) return false;
  newlightningdata->tm = temptime;
  newlightningdata->lat = getFieldFloat(obsString, latF);
  newlightningdata->lng = getFieldFloat(obsString, longF);
  newlightningdata->kamps = getFieldFloat(obsString, kampsF);
  if (!getGlobalObsData()->addLightning(newlightningdata))
    freeLightningData(newlightningdata);
  else
    lastReadAddedCount++;
  lastReadObsCount++;
  return true;
}

int readLightningAXF::getFieldInt(string &obsString, int fieldnum)     // returns int val for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readLightningAXF::getFieldInt - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
    
  if (fieldData[fieldnum].fieldType != axf_int)
    {
      cout << "readLightningAXF::getFieldInt - ERROR - Field type not int\n";
      return 0;
    }
  return atoi(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

float readLightningAXF::getFieldFloat(string &obsString, int fieldnum) // returns flaot value for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readLightningAXF::getFieldFloat - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_float)
    {
      cout << "readLightningAXF::getFieldFloat - ERROR - Field type not float\n";
      return 0;
    }
  return atof(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

// NOTE: strings are encapsulated in "", skip first and copy 2 less chars
void readLightningAXF::getFieldString(string &obsString, int fieldnum, std::string &str) // copy string
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readLightningAXF::getFieldString - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cout << "readLightningAXF::getFieldString - ERROR - Field type not string\n";
      str = "Error";
    }
  else
    str = obsString.substr(fieldOfs[fieldnum].ofs+1, fieldOfs[fieldnum].wid-2); 
}

#include "rdrutils.h"

time_t readLightningAXF::getFieldTime(std::string &obsString, 
				int fieldnum)           // convert aifstime to time_t
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readLightningAXF::getFieldTime - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cout << "readLightningAXF::getFieldTime - ERROR - Field type not string\n";
      return 0;
    }
  if (fieldOfs[fieldnum].wid < 16)
    {
      cout << "readLightningAXF::getFieldTime - ERROR - String too short\n";
      return 0;
    }
  char *timestr = (char *)obsString.c_str()+fieldOfs[fieldnum].ofs+1;  // skip "
  return DateTimeStr2UnixTime(timestr);
}

void readLightningAXF::dumpStatus(FILE *dumpfile)
{
  char timestr1[128], timestr2[128], timestr3[128], timestr4[128];
  if (!dumpfile) return;
  fprintf(dumpfile, "axfName=%s fieldCount=%d\n"
	  "lastReadObsCount=%d lastReadAddedCount=%d\n"
	  "lastReadStartTime=%s lastReadEndTime=%s\n"
	  "thisReadStartTime=%s thisReadEndTime=%s\n",
	  axfName.c_str(), fieldCount, 
	  lastReadObsCount,lastReadAddedCount,
	  ShortTimeString(lastReadStartTime, timestr1),
	  ShortTimeString(lastReadEndTime, timestr2),
	  ShortTimeString(thisReadStartTime, timestr3),
	  ShortTimeString(thisReadEndTime, timestr4));
}
	  
