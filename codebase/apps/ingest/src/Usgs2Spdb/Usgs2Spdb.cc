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
///////////////////////////////////////////////////////////////
//
// Usgs2Spdb.cc
//
// Jason Craig, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2006
//
///////////////////////////////////////////////////////////////
//
// Usgs2Spdb reads USGS data from argc or an ASCII file, converts them to
// usgsData_t format (rapformats library) and stores them in SPDB.
//
////////////////////////////////////////////////////////////////

#include <cerrno>
#include <stdio.h>
#include <time.h>
#include <toolsa/Path.hh>
#include <Spdb/Product_defines.hh>

#include "Usgs2Spdb.hh"

using namespace std;


Usgs2Spdb::Usgs2Spdb(Params *tdrpParams)
{
  _params = tdrpParams;
}

Usgs2Spdb::~Usgs2Spdb()
{
}

int Usgs2Spdb::processFile(const char *file_path)
{
  Path path(file_path);

  if (_params->debug)
    cout << "Opening file: " << file_path << endl;

  FILE *fp;
  if((fp = fopen(file_path, "r")) == NULL) {
    int errNum = errno;
    cerr << "ERROR - Usgs2Spdb::processFile" << endl;
    cerr << "  Cannot open file: "
	 << file_path << endl;
    cerr << "  " << strerror(errNum) << endl;
    return -1;
  }
   
  // read in line-by-line
  char line[BUFSIZ];
  char *pch, *pch2;
  int ret, lineNum = 1;
  while( fgets(line, BUFSIZ, fp) != NULL ) {

    if (_params->debug == Params::DEBUG_VERBOSE)
      cout << "Parsing line " << lineNum << ": \"" << line << "\"\n";

    pch = strstr(line,"-Volcano");
    if(pch != NULL) {

      char *sent, *title, *sender, *lat, *lon, *alt, *color, *time, *id;

      pch += 8;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      sent = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      title = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      sender = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      lat = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      lon = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      alt = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      color = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      pch2 = strchr(pch,',');
      if(pch2 == NULL) {
	cerr << "Error reading file: " << file_path << endl;
	cerr << "Unable to parse Volcano line " << lineNum << ": \"" << line << "\"\n";
	return -3;
      }
      time = pch;
      pch2[0] = char(0);

      pch = pch2 +2;
      // If end of line has a line feed, remove it
      pch2 = strchr(pch,char(10));
      if(pch2 != NULL) {
	pch2[0] = char(0);
      }
      id = pch;


      ret = saveVolcano(sent, title, sender, lat, lon, alt, color, time, id);
      if(ret != 0)
	return ret;

    } else {

      pch = strstr(line, "-Earthquake");
      if(pch != NULL) {


	char *sent, *title, *id, *sender, *version, *magnitude, *magnitudeType;
	char *time, *lat, *lon, *depth, *horizontalError, *verticalError;
	char *stations, *phases, *distance, *RMSError, *azimuthalGap;
	
	pch += 11;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	sent = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	title = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	id = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	sender = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	version = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	magnitude = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	  return -3;
	}
	magnitudeType = pch;
	pch2[0] = char(0);
	
	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	time = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	lat = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	lon = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	depth = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	horizontalError = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	verticalError = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	stations = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	phases = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	distance = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	pch2 = strchr(pch,',');
	if(pch2 == NULL) {
	  cerr << "Error reading file: " << file_path << endl;
	  cerr << "Unable to parse Earthquake line " << lineNum << ": \"" << line << "\"\n";
	return -3;
	}
	RMSError = pch;
	pch2[0] = char(0);

	pch = pch2 +2;
	// If end of line has a line feed, remove it
	pch2 = strchr(pch,char(10));
	if(pch2 != NULL) {
	  pch2[0] = char(0);
	}
	azimuthalGap = pch;
	

	ret = saveEarthquake(sent, title, id, sender, version, magnitude, magnitudeType,
			     time, lat, lon, depth, horizontalError, verticalError,
			     stations, phases, distance, RMSError, azimuthalGap);
	if(ret != 0)
	  return ret;


      }
    }
    lineNum ++;
  }

  fclose(fp);

  _writeData();
  return 0;
}

int Usgs2Spdb::saveVolcano(char *sent, char *title, char *sender, char *lat, char *lon, char *alt, char *color, char *time, char *id)
{
  UsgsData::usgsData_t data;
  data.data_type = UsgsData::USGS_DATA_VOLCANO;

  strncpy(data.eventTitle, title, UsgsData::EVNT_NAME_LEN-1);
  data.eventTitle[UsgsData::EVNT_NAME_LEN-1] = char(0);

  strncpy(data.sourceName, sender, UsgsData::SRC_NAME_LEN-1);
  data.sourceName[UsgsData::SRC_NAME_LEN-1] = char(0);

  strncpy(data.eventID, id, UsgsData::EVENT_ID_LEN-1);
  data.eventID[UsgsData::EVENT_ID_LEN-1] = char(0);

  data.time = _parseTime(time);
  data.lat = atof(lat);
  data.lon = atof(lon);
  data.alt = atof(alt) / 1000.0;
  if(color[0] == 'G')
    data.magnitude_type = UsgsData::COLOR_CODE_GREEN;
  else if(color[0] == 'Y')
    data.magnitude_type = UsgsData::COLOR_CODE_YELLOW;
  else if(color[0] == 'O')
    data.magnitude_type = UsgsData::COLOR_CODE_ORANGE;
  else if(color[0] == 'R')
    data.magnitude_type = UsgsData::COLOR_CODE_RED;
  else 
    data.magnitude_type = UsgsData::COLOR_CODE_UNKNOWN;

  si32 dataType = atoi(id);
  
  int rectime = _parseTime(sent);

  _saveData(data, rectime, dataType);
  return _writeData();
}

int Usgs2Spdb::saveEarthquake(char *sent, char *title, char *id, char *sender, char *version, char *magnitude, char *magnitudeType,
			      char *time, char *lat, char *lon, char *depth, char *horizontalError, char *verticalError,
			      char *stations, char *phases, char *distance, char *RMSError, char *azimuthalGap)
{

  UsgsData::usgsData_t data;
  data.data_type = UsgsData::USGS_DATA_EARTHQUAKE;

  strncpy(data.eventTitle, title, UsgsData::EVNT_NAME_LEN-1);
  data.eventTitle[UsgsData::EVNT_NAME_LEN-1] = char(0);

  strncpy(data.sourceName, sender, UsgsData::SRC_NAME_LEN-1);
  data.sourceName[UsgsData::SRC_NAME_LEN-1] = char(0);

  strncpy(data.eventID, id, UsgsData::EVENT_ID_LEN-1);
  data.eventID[UsgsData::EVENT_ID_LEN-1] = char(0);

  data.time = _parseTime(time);
  data.lat = atof(lat);
  data.lon = atof(lon);
  data.alt = atof(depth);
  if(version[0] == 'Q')
    data.version = -1;
  else if(version[0] == 'R')
    data.version = -2;
  else
    data.version = atoi(version);
  data.magnitude = atof(magnitude);
  if(magnitudeType[0] == 'd')
    data.magnitude_type = UsgsData::MAGNITUDE_DURATIAON;
  else if(magnitudeType[0] == 'L')
    data.magnitude_type = UsgsData::MAGNITUDE_LOCAL;
  else if(magnitudeType[0] == 's')
    data.magnitude_type = UsgsData::MAGNITUDE_SURFACE_WAVE;
  else if(magnitudeType[0] == 'w')
    data.magnitude_type = UsgsData::MAGNITUDE_MOMENT;
  else if(magnitudeType[0] == 'b')
    data.magnitude_type = UsgsData::MAGNITUDE_BODY;
  else 
    data.magnitude_type = UsgsData::MAGNITUDE_UNKOWN;
  data.horizontalError = atof(horizontalError);
  data.verticalError = atof(verticalError);
  data.minDistance = atof(distance);
  data.rmsTimeError = atof(RMSError);
  data.azimuthalGap = atof(azimuthalGap);
  data.numStations = atoi(stations);
  data.numPhases = atoi(phases);

  si32 dataType;
  // Hash the last four digits of the id as the dataType
  if(strlen(id) < 4)
    dataType = Spdb::hash4CharsToInt32(id);
  else
    dataType = Spdb::hash4CharsToInt32( id + strlen(id) - 4);
  

  int rectime = _parseTime(sent);

  _saveData(data, rectime, dataType);
  return _writeData();
}


int Usgs2Spdb::_parseTime(char *time)
{
  tzset();

  struct tm ctime;
  memset(&ctime, 0, sizeof(struct tm));
  strptime(time, "%FT%T%z", &ctime);
  // - timezone corrects for local offset from UTC of computer which mktime adds
  int utctime = mktime(&ctime) - timezone;

  // Correct for Offset from UTC in the data
  if(strlen(time) > 20 && (time[19] == '+' || time[19] == '-')) {
    char *utcOffset = time + 20;
    char *minutes = strchr(utcOffset, ':');

    int offset = (atoi(utcOffset) * 3600) + (atoi(minutes) * 60);
    if(time[19] == '+')
      offset *= -1;
    utctime += offset;
  }
  
  return utctime;
}

void Usgs2Spdb::_saveData(UsgsData::usgsData_t data, int time, si32 dataType)
{
  UsgsData Data;
  Data.setUsgsData(data);
  if (_params->debug)
    Data.print(cout);
  if (_params->debug == Params::DEBUG_VERBOSE)
    cout << "Saved for Time: " << DateTime::str(time) << endl;
  Data.assemble();

  void *buff = Data.getBufPtr();
  _spdb.addPutChunk( dataType,
		     time,
		     time + _params->expiry,
		     sizeof(UsgsData::usgsData_t), buff);

}

int Usgs2Spdb::_writeData()
{
  if (_spdb.put( _params->SpdbOutUrl, SPDB_USGS_ID, SPDB_USGS_LABEL))
  {
    cerr << "ERROR: Failed to put Spdb data\n";
    return -4;
  }
  return 0;
}
