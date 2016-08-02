/*  
  readObsAXF.C
  Class to parse axf format obs data into obsData class containers
  Passes fixedObsData instances to the obsDataMng singleton

  Reads Field declarations section to get fields available, names and 
  positions to locate required pre-defined field names
*/

#include "readObsAXF.h"
#include "utils.h"
#include <iostream>
#include "errno.h"

using namespace std;

bool debug = true;

axfFieldData::axfFieldData()
{
  init();
}

axfFieldData::axfFieldData(char *fieldname, axfFieldTypes fieldtype, 
			   int fieldpos)
{
  init();
  set(fieldname, fieldtype, fieldpos);
}

void axfFieldData::set(char *fieldname, axfFieldTypes fieldtype, 
		       int fieldpos)
{
  if (fieldname)
    fieldName = fieldname;
  fieldType = fieldtype;
  if (fieldpos != -1)
    fieldPos = fieldpos;
}

void axfFieldData::init()
{
  fieldName = "Undefined";
  fieldType = axf_undefined;
  fieldPos = -1;
  valid = false;
}

bool axfFieldData::readFieldData(char *fieldstr, int pos)
{
  valid = false;
  if (!fieldstr) return false;
  string::size_type 
    eqpos = string::npos,  // pos of "=" char in string - used to get name
    sqbrpos = string::npos,// pos of "[" char in string indicate string type
    fltpos = string::npos; // pos of "0.0" in string, indicates float type
    
  string fieldString = fieldstr;
  eqpos = fieldString.find('=');
  sqbrpos = fieldString.find('[');
  fltpos = fieldString.find("0.0");

  if (sqbrpos != string::npos)   // string type, get name
    {
      fieldName = fieldString.substr(0, sqbrpos); 
      fieldType = axf_string;
      fieldPos = pos;
      valid = true;
    }
  else if (eqpos != string::npos)
    {
      fieldName = fieldString.substr(0, eqpos); 
      fieldPos = pos;
      if (fltpos != string::npos)
	fieldType = axf_float;
      else
	fieldType = axf_int;
      valid = true;
    }
  return valid;
}

bool axfFieldData::fieldNameMatch(char *matchstr)
{
  return (matchstr && (fieldName == matchstr));
}  

readObsAXF::readObsAXF(char *axfname)
{
  init();
  if (axfname)
    setFName(axfname);
}

readObsAXF::~readObsAXF()
{
  if (axfFile)
    {
      fclose(axfFile);
      axfFile = NULL;
    }
}

void readObsAXF::init()
{
  axfName = "latest.axf";
  axfFile = NULL;
  lastReadStartTime = 
    lastReadEndTime = 0;
  thisReadStartTime = 
    thisReadEndTime = 0;
  delimiter = ",";
  debug = 0;
  fieldData.resize(20);  // initially allow for 20 fields
  resetFPos();
}


void readObsAXF::resetFPos()  // reset field positions
{
  nameF = stationF = stnF = siteF = timeF = latF = longF = htF = 
    wdF = wsF = wgF = spF = tmpF = dpF = r9F = r10F = -1;
}

void readObsAXF::setFName(char *axfname)
{
  if (axfname)
    axfName = axfname;
}

int readObsAXF::readFile(char *axfname,
			 time_t start_time,
			 time_t end_time)
{
  int obscount = -1;
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
      if (readDescr())
	obscount = readObs(start_time, end_time);
      fclose(axfFile);
      char timestr[64], timestr2[64];
      fprintf(stdout, "readObsAXF::readFile - Obs Read=%d Added=%d from %s\n"
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
      fprintf(stdout, "readObsAXF::readFile - Failed opening file %s\n",
	      axfname);
      perror(NULL);
    }
  return obscount;
}
      
  

bool readObsAXF::readDescr()
{
  char tempBuff[512];
  bool done = false;
  if (!axfFile) return false;
  // skip to start of [DESCRIPTION] section
  while (!done)
    {
      if (fgets(tempBuff, 512, axfFile))
	done = strstr(tempBuff, "[DESCRIPTION]");
      else
	done = true;
    }
  if (!done) return false;  // didn't find [DESCRIPTION]
  
  fieldCount = 0;
  int reqdfields = 0;   // some fields optional, 9 reqd fields
  done = false;
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
	      if (fieldData[fieldCount].fieldNameMatch("name"))
		{
		  nameF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("stn"))
		{
		  stnF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("aifstime"))
		{
		  timeF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("lat"))
		{
		  latF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("lon"))
		{
		  longF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("stn_ht"))
		{
		  htF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("wnd_dir"))
		{
		  wdF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("wnd_spd"))
		{
		  wsF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("max_wind_gust"))
		wgF = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("sea_press"))
		spF = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("air_temp"))
		{
		  tmpF = fieldCount;
		  reqdfields++;
		}
	      else if (fieldData[fieldCount].fieldNameMatch("dwpt"))
		dpF = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("q10mnt_prcp"))
		r10F = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("q9am_prcp"))
		r9F = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("site"))
		siteF = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("station"))
		stationF = fieldCount;
	    }
	  fieldCount++;  // pre-increment field count, decr below if not recognised
	}
    }
  return reqdfields >= 9;
}

int readObsAXF::getFieldOfs(string &obsString)  // get offsets for each field
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
      if (count >= fieldCount)
	{
	  cout << "readObsAXF::getFieldOfs ERROR - FIELDS FOUND > fieldCount(" 
	       << fieldCount << ") - " << obsString << endl;
	  return 0;                  // ERROR, too many fields 
	}
      fieldOfs[count].ofs = pos+1; // point to char after delimiter
    }
  if (count)
    fieldOfs[count].wid = obsString.size() - fieldOfs[count].ofs;
  count++;
  return count;
}

// check the field pos from the DESCRIPTION matches the pos from the
// first SURFACE data line
// ****Not yet implemented
bool readObsAXF::checkFPos(char *obsstring)
{
  if (!obsstring) return false;
  return true;
}

int readObsAXF::readObs(time_t start_time,
			time_t end_time)
{
  char tempBuff[1024];
  bool done = false;
  if (!axfFile) return -1;
  // skip to start of [SURFACE] section
  while (!done)
    {
      if (fgets(tempBuff, 512, axfFile))
	done = strstr(tempBuff, "[SURFACE]");
      else
	done = true;
    }
  if (!done) return false;  // didn't find [SURFACE]
  if (!fgets(tempBuff, 512, axfFile)) // skip
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
 
bool readObsAXF::readObsString(char *obsstr,
			       time_t start_time,
			       time_t end_time)
{
  if (!obsstr) return false;
  string obsString = obsstr;
  int fieldOfsCount = getFieldOfs(obsString);
  if (fieldOfsCount != fieldCount)
    {
      cout << "readObsAXF::readObsString ERROR - fieldCount=" << fieldCount <<
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
  int stn = getFieldInt(obsString, stnF);
  if (!getGlobalObsStnMap()->obsStnExists(stn))
    {
      obsStn *obsstn = new obsStn;
      obsstn->stn = stn;
      obsstn->site = getFieldInt(obsString, siteF);
      obsstn->lat = getFieldFloat(obsString, latF);
      obsstn->lng = getFieldFloat(obsString, longF);
      obsstn->ht = getFieldFloat(obsString, htF);
      if (obsstn->ht == -9999.0)
	obsstn->ht = 0;
      getFieldString(obsString, nameF, obsstn->name);
      getFieldString(obsString, stationF, obsstn->station);
      getGlobalObsStnMap()->addObsStn(obsstn);
      obsstn->dump(stdout);
    }
  if (getGlobalObsStnMap()->getObsStn(stn) &&
      getGlobalObsStnMap()->getObsStn(stn)->getType() == ot_moving)   
    {
      movingObsData *newmovingobs = newMovingObsData();
      if (readObsStringMoving(obsString, newmovingobs))
	{
	  if (!getGlobalObsData()->addObs(newmovingobs))
	    freeObsData((baseObsData *)newmovingobs);// no ref kept, free newMovingObsData
	  return true; // valid obs decoded
	}
      else
	{
	  freeObsData((baseObsData *)newmovingobs);
	  return false;
	}
    }
  else
    {
      fixedObsData *newfixedobs = newFixedObsData();
      if (readObsStringFixed(obsString, newfixedobs))
	{
	  if (!getGlobalObsData()->addObs(newfixedobs))
	    freeObsData((baseObsData *)newfixedobs);
	  else
	    lastReadAddedCount++;
	  lastReadObsCount++;
	  return true;
	}
      else
	{
	  freeObsData((baseObsData *)newfixedobs);
	  return false;
	}
    }
}
      
bool readObsAXF::readObsStringBase(std::string &obsString,
				   baseObsData *obs)
{
  if (!obs) return false;
  obs->stn = getFieldInt(obsString, stnF);
  obs->tm = getFieldTime(obsString, timeF);
  obs->setWD(getFieldInt(obsString, wdF));
  obs->setWS(getFieldInt(obsString, wsF));
  obs->setWG(getFieldInt(obsString, wgF));
  obs->setSPress(getFieldFloat(obsString, spF));
  obs->setTemp(getFieldFloat(obsString, tmpF));
  obs->setDP(getFieldFloat(obsString, dpF));
  return true;
}

bool readObsAXF::readObsStringFixed(std::string &obsString,
				    fixedObsData *obs)
{
  if (!obs) return false;
  readObsStringBase(obsString, obs);
  obs->setRain9am(getFieldFloat(obsString, r9F));
  obs->setRain10(getFieldFloat(obsString, r10F));  
  return true;
}

bool readObsAXF::readObsStringMoving(string &obsString,
				     movingObsData *obs)
{
  if (!obs) return false;
  readObsStringBase(obsString, obs);
  obs->lat = getFieldFloat(obsString, latF);
  obs->lng = getFieldFloat(obsString, longF);
  obs->ht = getFieldFloat(obsString, htF);
  return true;
}

int readObsAXF::getFieldInt(string &obsString, int fieldnum)     // returns int val for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readObsAXF::getFieldInt - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
    
  if (fieldData[fieldnum].fieldType != axf_int)
    {
      cout << "readObsAXF::getFieldInt - ERROR - Field type not int\n";
      return 0;
    }
  return atoi(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

float readObsAXF::getFieldFloat(string &obsString, int fieldnum) // returns flaot value for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readObsAXF::getFieldFloat - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_float)
    {
      cout << "readObsAXF::getFieldFloat - ERROR - Field type not float\n";
      return 0;
    }
  return atof(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

// NOTE: strings are encapsulated in "", skip first and copy 2 less chars
void readObsAXF::getFieldString(string &obsString, int fieldnum, std::string &str) // copy string
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readObsAXF::getFieldString - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cout << "readObsAXF::getFieldString - ERROR - Field type not string\n";
      str = "Error";
    }
  else
    str = obsString.substr(fieldOfs[fieldnum].ofs+1, fieldOfs[fieldnum].wid-2); 
}

#include "rdrutils.h"

time_t readObsAXF::getFieldTime(std::string &obsString, 
				int fieldnum)           // convert aifstime to time_t
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cout << "readObsAXF::getFieldTime - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cout << "readObsAXF::getFieldTime - ERROR - Field type not string\n";
      return 0;
    }
  if (fieldOfs[fieldnum].wid < 16)
    {
      cout << "readObsAXF::getFieldTime - ERROR - String too short\n";
      return 0;
    }
  char *timestr = (char *)obsString.c_str()+fieldOfs[fieldnum].ofs+1;  // skip "
  return DateTimeStr2UnixTime(timestr);
}

void readObsAXF::dumpStatus(FILE *dumpfile)
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
	  
