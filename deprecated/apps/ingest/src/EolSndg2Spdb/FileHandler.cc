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
/**
 *
 * @file FileHandler.cc
 *
 * @class FileHandler
 *
 * Class for processing the soundings in a file.
 *  
 * @date 9/28/2009
 *
 */

#include <iostream>
#include <stdlib.h>

#include <toolsa/DateTime.hh>
#include <toolsa/str.h>

#include "FileHandler.hh"

using namespace std;


// Globals

const string FileHandler::LAUNCH_LOCATION_LABEL =
  "Launch Location (lon,lat,alt):";
const string FileHandler::LAUNCH_TIME_LABEL = "UTC Launch Time (y,m,d,h,m,s):";

const int FileHandler::MAX_TOKENS = 50;
const int FileHandler::MAX_TOKEN_LEN = 1024;

const double FileHandler::EOL_MISSING_VALUE = -999.0;


/*********************************************************************
 * Constructors
 */

FileHandler::FileHandler(const string &file_path,
			 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _filePath(file_path),
  _inputFile(0)
{
}


/*********************************************************************
 * Destructor
 */

FileHandler::~FileHandler()
{
  // Close the input file

  if (_inputFile != 0)
    fclose(_inputFile);

  // Free the token buffer

  for (int i = 0; i < MAX_TOKENS; ++i)
    delete [] _tokens[i];
  
  delete [] _tokens;
}


/*********************************************************************
 * init()
 */

bool FileHandler::init()
{
  static const string method_name = "FileHandler::init()";
  
  // Open the file

  if ((_inputFile = fopen(_filePath.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << _filePath << endl;
    
    return false;
  }
  
  // Initialize the token buffer

  _tokens = new char*[MAX_TOKENS];
  
  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];
  
  return true;
}


/*********************************************************************
 * processFile()
 */

bool FileHandler::processFile()
{
  static const string method_name = "FileHandler::processFile()";
  
  // Initialize the sounding object

  Sndg sounding;
  
  _initSounding(sounding);
  
  // Process the lines in the file.  It looks like the sounding starts after
  // a header line of dashes making underscores and ends with a blank line.

  char *lineptr = 0;
  size_t linelen;
  bool in_sounding = false;
  
  while (getline(&lineptr, &linelen, _inputFile) != -1)
  {
    if (_verbose)
    {
      cerr << "Next line: <" << lineptr << ">" << endl;
    }
    
    // Process the line based on our current state
    if (in_sounding)
    {
      // If we are in a sounding and get a blank line, the sounding is
      // finished.

      if (lineptr[0] == '\0')
      {
	if (_verbose)
	  cerr << "... Found end of sounding" << endl;
	
	_soundings.push_back(sounding);
	
	_initSounding(sounding);
	
	in_sounding = false;
	continue;
      }
      
      // If we get here, we are processing a sounding and have a line
      // containing information for the next point in the sounding.

      _addPoint(lineptr, sounding);
      
    }
    else
    {
      // If we aren't in a sounding yet, look for the sounding start.

      if (_verbose)
	cerr << "... Looking for sounding start" << endl;

      if (lineptr[0] == '-')
      {
	in_sounding = true;
	
	if (_verbose)
	{
	  cerr << "    Found sounding start" << endl;
	  cerr << "Sounding header:" << endl;
	  sounding.print_header(cerr);
	}
      }
      else
      {
	_updateHeaderValue(lineptr, sounding);
      }
      
      continue;
    }
  } /* endwhile - getline */
  
  // Save the last souunding from the file

  _soundings.push_back(sounding);
  
  free(lineptr);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _addPoint()
 */

void FileHandler::_addPoint(const string &line,
			    Sndg &sounding) const
{
  static const string method_name = "FileHandler::_addPoint()";
  
  // Extract the values from the line

  if (STRparse(line.c_str(), _tokens, line.length(),
	       MAX_TOKENS, MAX_TOKEN_LEN) != 17)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Unable to parse sounding line: " << line << endl;
    cerr << "Skipping line" << endl;
    
    return;
  }
  
  double seconds_offset = atof(_tokens[0]);
//  int hour = atoi(_tokens[1]);
//  int minute = atoi(_tokens[2]);
//  int second = atoi(_tokens[3]);
  double pressure_mb = atof(_tokens[4]);
  double temp_c = atof(_tokens[5]);
  double dewpt_c = atof(_tokens[6]);
  double rh = atof(_tokens[7]);
  double uwind = atof(_tokens[8]);
  double vwind = atof(_tokens[9]);
  double wind_speed = atof(_tokens[10]);
  double wind_dir = atof(_tokens[11]);
//  double dz = atof(_tokens[12]);
  double geo_po_alt_m = atof(_tokens[13]);
  double lon = atof(_tokens[14]);
  double lat = atof(_tokens[15]);
//  double gpsa_m = atof(_tokens[16]);
  
  // Create the point

  Sndg::point_t sndg_pt;
  
  sndg_pt.time = seconds_offset;
  
  if (pressure_mb == EOL_MISSING_VALUE)
    sndg_pt.pressure = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.pressure = pressure_mb;

  if (geo_po_alt_m == EOL_MISSING_VALUE)
    sndg_pt.altitude = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.altitude = geo_po_alt_m;
  
  if (uwind == EOL_MISSING_VALUE)
    sndg_pt.u = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.u = uwind;
  
  if (vwind == EOL_MISSING_VALUE)
    sndg_pt.v = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.v = vwind;
  
  sndg_pt.w = Sndg::VALUE_UNKNOWN;
  
  if (rh == EOL_MISSING_VALUE)
    sndg_pt.rh = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.rh = rh;
  
  if (temp_c == EOL_MISSING_VALUE)
    sndg_pt.temp = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.temp = temp_c;
  
  if (dewpt_c == EOL_MISSING_VALUE)
    sndg_pt.dewpt = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.dewpt = dewpt_c;
  
  if (wind_speed == EOL_MISSING_VALUE)
    sndg_pt.windSpeed = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.windSpeed = wind_speed;
  
  if (wind_dir == EOL_MISSING_VALUE)
    sndg_pt.windDir = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.windDir = wind_dir;
  
  sndg_pt.ascensionRate = Sndg::VALUE_UNKNOWN;

  if (lon == EOL_MISSING_VALUE)
    sndg_pt.longitude = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.longitude = lon;
  
  if (lat == EOL_MISSING_VALUE)
    sndg_pt.latitude = Sndg::VALUE_UNKNOWN;
  else
    sndg_pt.latitude = lat;
  
  sndg_pt.pressureQC = Sndg::QC_MISSING;
  sndg_pt.tempQC = Sndg::QC_MISSING;
  sndg_pt.humidityQC = Sndg::QC_MISSING;
  sndg_pt.uwindQC = Sndg::QC_MISSING;
  sndg_pt.vwindQC = Sndg::QC_MISSING;
  sndg_pt.ascensionRateQC = Sndg::QC_MISSING;
  
  for (int i = 0; i < Sndg::PT_SPARE_FLOATS; ++i)
    sndg_pt.spareFloats[i] = Sndg::VALUE_UNKNOWN;
  
  // Add the point to the sounding

  sounding.addPoint(sndg_pt, true);
  
}


/*********************************************************************
 * _extractLaunchLocation()
 */

void FileHandler::_extractLaunchLocation(const string &line,
					 Sndg &sounding) const
{
  // Get the sounding header

  Sndg::header_t header = sounding.getHeader();
  
  // Extract the string containing the location values

  string value_string =
    line.substr(LAUNCH_LOCATION_LABEL.length());
  
  if (_verbose)
    cerr << "Launch location value string = <" << value_string << ">" << endl;
  
  // Parse the location values

  if (STRparse_delim(value_string.c_str(), _tokens, value_string.length(),
		     ",", MAX_TOKENS, MAX_TOKEN_LEN) != 3)
    return;
  
  string lon_string = _tokens[0];
  string lat_string = _tokens[1];
  header.alt = atof(_tokens[2]);
  
  if (_verbose)
  {
    cerr << "lon_string = <" << lon_string << ">" << endl;
    cerr << "lat_string = <" << lat_string << ">" << endl;
    cerr << "alt = " << header.alt << endl;
  }
  
  // Update the longitude value

  size_t space_pos = lon_string.rfind(" ");
  
  if (space_pos != string::npos)
  {
    lon_string = lon_string.substr(space_pos);

    if (_verbose)
      cerr << "lon_string = <" << lon_string << ">" << endl;
    
    header.lon = atof(lon_string.c_str());
  }
  
  // Update the latitude value

  space_pos = lat_string.rfind(" ");
  
  if (space_pos != string::npos)
  {
    lat_string = lat_string.substr(space_pos);

    if (_verbose)
      cerr << "lat_string = <" << lat_string << ">" << endl;
    
    header.lat = atof(lat_string.c_str());
  }
  
  // Update the header values

  sounding.setHeader(header);
}


/*********************************************************************
 * _extractLaunchTime()
 */

void FileHandler::_extractLaunchTime(const string &line,
				     Sndg &sounding) const
{
  // Get the sounding header

  Sndg::header_t header = sounding.getHeader();
  
  // Extract the string containing the time values

  string value_string =
    line.substr(LAUNCH_TIME_LABEL.length());
  
  if (_verbose)
    cerr << "Launch time value string = <" << value_string << ">" << endl;
  
  // Parse the time values

  if (STRparse_delim(value_string.c_str(), _tokens, value_string.length(),
		     ",", MAX_TOKENS, MAX_TOKEN_LEN) != 4)
    return;
  
  int year = atoi(_tokens[0]);
  int month = atoi(_tokens[1]);
  int day = atoi(_tokens[2]);
  string time_string = _tokens[3];
  
  if (_verbose)
  {
    cerr << "year = " << year << endl;
    cerr << "month = " << month << endl;
    cerr << "day = " << day << endl;
    cerr << "time_string = <" << time_string << ">" << endl;
  }
  
  if (STRparse_delim(time_string.c_str(), _tokens, time_string.length(),
		     ":", MAX_TOKENS, MAX_TOKEN_LEN) != 3)
    return;
  
  int hour = atoi(_tokens[0]);
  int minute = atoi(_tokens[1]);
  int second = atoi(_tokens[2]);
  
  if (_verbose)
  {
    cerr << "hour = " << hour << endl;
    cerr << "minute = " << minute << endl;
    cerr << "second = " << second << endl;
  }
  
  DateTime launch_time(year, month, day, hour, minute, second);
  
  if (_verbose)
    cerr << "Launch time = " << launch_time << endl;
  
  header.launchTime = launch_time.utime();
  
  // Update the header values

  sounding.setHeader(header);
}


/*********************************************************************
 * _initSounding()
 */

void FileHandler::_initSounding(Sndg &sounding) const
{
  // Remove any existing points

  sounding.clearPoints();
  
  // Initialize the header

  Sndg::header_t header;
  
  header.launchTime = 0;
  header.nPoints = 0;
  header.sourceId = 0;
  header.leadSecs = 0;
  for (int i = 0; i < Sndg::HDR_SPARE_INTS;++i)
    header.spareInts[i] = 0;

  header.lat = Sndg::VALUE_UNKNOWN;
  header.lon = Sndg::VALUE_UNKNOWN;
  header.alt = Sndg::VALUE_UNKNOWN;
  for (int i = 0; i < Sndg::HDR_SPARE_FLOATS; ++i)
    header.spareFloats[i] = Sndg::VALUE_UNKNOWN;

  header.sourceName[0] = '\0';
  header.sourceFmt[0] = '\0';
  header.siteName[0] = '\0';
  
  sounding.setHeader(header);
}


/*********************************************************************
 * _updateHeaderValue()
 */

void FileHandler::_updateHeaderValue(const string &line,
				     Sndg &sounding) const
{
  if (line.find(LAUNCH_TIME_LABEL) != string::npos)
  {
    _extractLaunchTime(line, sounding);
  }
  else if (line.find(LAUNCH_LOCATION_LABEL) != string::npos)
  {
    _extractLaunchLocation(line, sounding);
  }
  
}
