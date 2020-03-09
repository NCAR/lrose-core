// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/*  
  readObsAXF.C
  Class to parse axf format obs data into obsData class containers
  Passes fixedObsData instances to the obsDataMng singleton

  Reads Field declarations section to get fields available, names and 
  positions to locate required pre-defined field names
*/

#include "ReadObsAxf.hh"
#include <iostream>
#include <cerrno>
#include <toolsa/str.h>
#include <toolsa/DateTime.hh>
#include <rapformats/station_reports.h>

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

bool axfFieldData::fieldNameMatch(const char *matchstr)
{
  return (matchstr && (fieldName == matchstr));
}  

readObsAXF::readObsAXF(const Params &params) : _params(params)
{
  axfFile = NULL;
  lastReadStartTime = lastReadEndTime = 0;
  thisReadStartTime = thisReadEndTime = 0;
  delimiter = ",";
  debug = 0;
  fieldData.resize(1024);  // initially allow for 1024 fields
  resetFPos();
}

readObsAXF::~readObsAXF()
{
  if (axfFile)
    {
      fclose(axfFile);
      axfFile = NULL;
    }
}

void readObsAXF::resetFPos()  // reset field positions
{
  nameF = stationF = stnF = siteF = timeF = latF = longF = htF = 
    wdF = wsF = wgF = spF = tmpF = dpF = r9F = r10F = -1;
}

int readObsAXF::readFile(const char *axfname,
			 DsSpdb &spdb,
			 time_t start_time,
			 time_t end_time)
{

  int obscount = -1;
  if (axfFile)
    {
      fclose(axfFile);
      axfFile = NULL;
    }
  axfFile = fopen(axfname, "r");
  if (axfFile)
    {
      lastReadStartTime = thisReadStartTime;
      lastReadEndTime = thisReadEndTime;
      thisReadStartTime = thisReadEndTime = 0;
      lastReadObsCount = 
	lastReadAddedCount = 0;
      if (readDescr())
	obscount = readObs(spdb, start_time, end_time);
      fclose(axfFile);
      fprintf(stdout, "readObsAXF::readFile - Obs Read=%d Added=%d from %s\n"
	      "File start=%s end=%s\n%d secs after prev end\n", 
	      lastReadObsCount, lastReadAddedCount, axfname, 
	      DateTime::strm(thisReadStartTime).c_str(),
	      DateTime::strm(thisReadEndTime).c_str(),
	      int(thisReadEndTime - lastReadEndTime));
     axfFile = NULL;
    }
  else
    {
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
	      else if (fieldData[fieldCount].fieldNameMatch("vis"))
		visF = fieldCount;
	      else if (fieldData[fieldCount].fieldNameMatch("relh"))
		relhF = fieldCount;
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
	  cerr << "readObsAXF::getFieldOfs ERROR - FIELDS FOUND > fieldCount(" 
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

int readObsAXF::readObs(DsSpdb &spdb,
			time_t start_time,
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
	  if (readObsString(spdb,
			    tempBuff, start_time, end_time))
	    {
	      obscount++;
	    }
	}
    }
  return obscount;
}
 
bool readObsAXF::readObsString(DsSpdb &spdb,
			       char *obsstr,
			       time_t start_time,
			       time_t end_time)
{
  if (!obsstr) return false;
  string obsString = obsstr;
  int fieldOfsCount = getFieldOfs(obsString);
  if (fieldOfsCount != fieldCount)
    {
      cerr << "readObsAXF::readObsString ERROR - fieldCount=" << fieldCount <<
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

  // fill out station report

  station_report_t report;
  memset(&report, 0, sizeof(report));
  
  std::string name = getFieldString(obsString, nameF);
  STRncopy(report.station_label, name.c_str(), ST_LABEL_SIZE);

  report.msg_id = SENSOR_REPORT;
  time_t report_time = getFieldTime(obsString, timeF);
  report.time = report_time;

  report.lat = getFieldFloat(obsString, latF);
  report.lon = getFieldFloat(obsString, longF);
  report.alt = getFieldFloat(obsString, htF);
  if (report.alt < -999) {
    report.alt = 0;
  }
  report.temp = getFieldFloat(obsString, tmpF);
  if (report.temp < -999) {
    report.temp = STATION_NAN;
  }
  report.dew_point = getFieldFloat(obsString, dpF);
  if (report.dew_point < -999) {
    report.dew_point = STATION_NAN;
  }
  report.relhum = getFieldInt(obsString, relhF);
  if (report.relhum < 0) {
    report.relhum = STATION_NAN;
  }
  report.windspd = getFieldInt(obsString, wsF);
  if (report.windspd < -999) {
    report.windspd = STATION_NAN;
  }
  report.winddir = getFieldInt(obsString, wdF);
  if (report.winddir < -999) {
    report.winddir = STATION_NAN;
  }
  report.windgust = getFieldInt(obsString, wgF);
  if (report.windgust < -999) {
    report.windgust = STATION_NAN;
  }
  report.pres = getFieldFloat(obsString, spF);
  if (report.pres < -999) {
    report.pres = STATION_NAN;
  }
  report.liquid_accum = getFieldFloat(obsString, r9F);
  if (report.liquid_accum < 0) {
    report.liquid_accum = STATION_NAN;
  }
  report.precip_rate = getFieldFloat(obsString, r10F);
  if (report.precip_rate < 0) {
    report.precip_rate = STATION_NAN;
  }
  report.visibility = getFieldInt(obsString, visF);
  if (report.visibility < 0) {
    report.visibility = STATION_NAN;
  }

  if (_params.debug >= Params::DEBUG_VERBOSE) {
    cerr << "====== FOUND REPORT ======" << endl;
    print_station_report(stderr, "", &report);
    cerr << "==========================" << endl;
  }

  // swap bytes

  station_report_to_be(&report);

  // add chunk
    
  int stationId = Spdb::hash4CharsToInt32(name.c_str());
  spdb.addPutChunk(stationId,
		   report_time,
		   report_time + _params.expire_seconds,
		   sizeof(report), &report);
		   
  return true;

}

// get int field

int readObsAXF::getFieldInt(string &obsString, int fieldnum)     // returns int val for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cerr << "readObsAXF::getFieldInt - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
    
  if (fieldData[fieldnum].fieldType != axf_int)
    {
      cerr << "readObsAXF::getFieldInt - ERROR - Field type not int\n";
      return 0;
    }
  return atoi(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

// get float field

float readObsAXF::getFieldFloat(string &obsString, int fieldnum) // returns flaot value for field
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cerr << "readObsAXF::getFieldFloat - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_float)
    {
      cerr << "readObsAXF::getFieldFloat - ERROR - Field type not float\n";
      cerr << "  obsString: " << obsString << endl;
      cerr << "  fieldnum: " << fieldnum << endl;
      return 0;
    }
  return atof(obsString.c_str()+fieldOfs[fieldnum].ofs);
}

// get string field
// NOTE: strings are encapsulated in "", skip first and copy 2 less chars

std::string readObsAXF::getFieldString(string &obsString, int fieldnum)
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cerr << "readObsAXF::getFieldString - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cerr << "readObsAXF::getFieldString - ERROR - Field type not string\n";
      return "Error";
    }
  else
    return obsString.substr(fieldOfs[fieldnum].ofs+1, fieldOfs[fieldnum].wid-2); 
}

// get time field

time_t readObsAXF::getFieldTime(std::string &obsString, 
				int fieldnum)           // convert aifstime to time_t
{
  if ((fieldnum < 0) || (fieldnum >= fieldCount))
    {
      cerr << "readObsAXF::getFieldTime - ERROR - fieldnum(" << fieldnum <<
	") >  fieldcount(" << fieldCount << ")\n";
      return 0;
    }
  if (fieldData[fieldnum].fieldType != axf_string)
    {
      cerr << "readObsAXF::getFieldTime - ERROR - Field type not string\n";
      return 0;
    }
  if (fieldOfs[fieldnum].wid < 16)
    {
      cerr << "readObsAXF::getFieldTime - ERROR - String too short\n";
      return 0;
    }
  char *timestr = (char *)obsString.c_str()+fieldOfs[fieldnum].ofs+1;  // skip "

  int year = 0, month = 0,day = 0, hour = 0, min = 0, sec = 0;
  if (sscanf(timestr,"%04d%02d%02d%02d%02d%02d",
	     &year, &month, &day, &hour, &min, &sec) == 6) {
    DateTime dtime(year,month,day,hour,min,sec);
    return dtime.utime(); 
  } else {
    return 0;
  }

}

