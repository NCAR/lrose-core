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
 * Class for processing the station reports in an SMOS file.
 *  
 * @date 10/5/2009
 *
 */

#include <iostream>
#include <stdlib.h>

#include <toolsa/DateTime.hh>
#include <toolsa/Path.hh>
#include <toolsa/str.h>

#include "FileHandler.hh"

using namespace std;


// Globals

const int FileHandler::MAX_TOKENS = 50;
const int FileHandler::MAX_TOKEN_LEN = 1024;


/*********************************************************************
 * Constructors
 */

FileHandler::FileHandler(const string &file_path,
			 const vector< double > input_missing_values,
			 const bool negate_lon_values,
			 const bool debug_flag, const bool verbose_flag) :
  _debug(debug_flag),
  _verbose(verbose_flag),
  _inputMissingValues(input_missing_values),
  _filePath(file_path),
  _fileTime(DateTime::NEVER),
  _inputFile(0),
  _negateLonValues(negate_lon_values),
  _tokens(0)
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
  
  // Get the time from the file name

  if (!_getFileTimeFromPath())
    return false;
  
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
  
  // Process the lines in the file.  It looks like the sounding starts after
  // a header line of dashes making underscores and ends with a blank line.

  char *lineptr = 0;
  size_t linelen;
  
  while (getline(&lineptr, &linelen, _inputFile) != -1)
  {
    // Process the input line

    _processLine(lineptr, linelen);
    
  } /* endwhile - getline */
  
  free(lineptr);
  
  return true;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _getFileTimeFromPath()
 */

bool FileHandler::_getFileTimeFromPath()
{
  // Extract the file name from the path

  Path file_path(_filePath);
  
  string filename = file_path.getFile();
  
  // Extract the time values from the file name

  int year = atoi(filename.substr(12, 4).c_str());
  int month = atoi(filename.substr(16, 2).c_str());
  int day = atoi(filename.substr(18, 2).c_str());
  int hour = atoi(filename.substr(20, 2).c_str());
  
  if (_verbose)
  {
    cerr << "year = " << year << endl;
    cerr << "month = " << month << endl;
    cerr << "day = " << day << endl;
    cerr << "hour = " << hour << endl;
  }
  
  _fileTime.set(year, month, day, hour);
  
  return true;
}


/*********************************************************************
 * _initReport()
 */

void FileHandler::_initReport(station_report_t &report) const
{
  report.msg_id = PRESSURE_STATION_REPORT;
  report.time = 0;
  report.accum_start_time = 0;
  report.weather_type = 0;
  report.lat = STATION_NAN;
  report.lon = STATION_NAN;
  report.alt = STATION_NAN;
  report.temp = STATION_NAN;
  report.dew_point = STATION_NAN;
  report.relhum = STATION_NAN;
  report.windspd = STATION_NAN;
  report.winddir = STATION_NAN;
  report.windgust = STATION_NAN;
  report.pres = STATION_NAN;
  report.liquid_accum = STATION_NAN;
  report.precip_rate = STATION_NAN;
  report.visibility = STATION_NAN;
  report.rvr = STATION_NAN;
  report.ceiling = STATION_NAN;

  report.shared.pressure_station.stn_pres = STATION_NAN;
  report.shared.pressure_station.Spare1 = STATION_NAN;
  report.shared.pressure_station.Spare2 = STATION_NAN;
  
  report.station_label[0] = '\0';
}


/*********************************************************************
 * _processLine()
 */

void FileHandler::_processLine(const char *lineptr,
			       const size_t linelen)
{
  static const string method_name = "FileHandler::_processLine()";
  
  // Extract the values from the line

  int num_tokens_found;
  
  if ((num_tokens_found = STRparse(lineptr, _tokens, linelen,
				   MAX_TOKENS, MAX_TOKEN_LEN)) != NUM_TOKENS)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Unable to parse line: " << lineptr << endl;
    cerr << "Expected " << NUM_TOKENS << " tokens" << endl;
    cerr << "Found " << num_tokens_found << " tokens" << endl;
    cerr << "Skipping line" << endl;
    
    return;
  }
  
  string station_id = _tokens[STATION_ID_TOKEN];
  double station_lon = atof(_tokens[STATION_LON_TOKEN]);
  double station_lat = atof(_tokens[STATION_LAT_TOKEN]);
  double station_alt_m = atof(_tokens[STATION_ALT_M_TOKEN]);
  double surf_press_mb = atof(_tokens[SURF_PRESS_MB_TOKEN]);
  double temperature_c = atof(_tokens[TEMPERATURE_C_TOKEN]);
  double dewpoint_c = atof(_tokens[DEWPOINT_C_TOKEN]);
  double precip_accum_mm = atof(_tokens[PRECIP_ACCUM_MM_TOKEN]);
  double wind_speed_m_per_sec = atof(_tokens[WIND_SPEED_M_PER_SEC_TOKEN]);
  double wind_dir_deg = atof(_tokens[WIND_DIR_DEG_TOKEN]);
  
  // Create the station report

  station_report_t report;
  
  _initReport(report);
  
  report.time = _fileTime.utime();

  if (isValid(station_lat))
    report.lat = station_lat;

  if (isValid(station_lon))
  {
    report.lon = station_lon;
    if (_negateLonValues)
      report.lon = -report.lon;
  }
  
  if (isValid(station_alt_m))
    report.alt = station_alt_m;

  if (isValid(temperature_c))
    report.temp = temperature_c;

  if (isValid(dewpoint_c))
    report.dew_point = dewpoint_c;

  if (isValid(wind_speed_m_per_sec))
    report.windspd = wind_speed_m_per_sec;
  
  if (isValid(wind_dir_deg))
    report.winddir = wind_dir_deg;
  
  if (isValid(surf_press_mb))
  {
    report.pres = surf_press_mb;
    report.shared.pressure_station.stn_pres = surf_press_mb;
  }
  
  if (isValid(precip_accum_mm))
    report.liquid_accum = precip_accum_mm;
  
  STRcopy(report.station_label, station_id.c_str(), ST_LABEL_SIZE);
  
  // Add the station report to the list

  _reports.push_back(report);
}
