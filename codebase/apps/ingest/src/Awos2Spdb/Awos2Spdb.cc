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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:22:59 $
//   $Id: Awos2Spdb.cc,v 1.11 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Awos2Spdb: Awos2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <rapformats/station_reports.h>
#include <toolsa/toolsa_macros.h>
#include <Spdb/Product_defines.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <cassert>

#include "Awos2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

Awos2Spdb *Awos2Spdb::_instance = (Awos2Spdb *)NULL;

const int Awos2Spdb::TOKEN_DATE_BEGIN_YY = 5;
const int Awos2Spdb::TOKEN_DATE_LEN_YY = 2;
const int Awos2Spdb::TOKEN_DATE_BEGIN_MON = 7;
const int Awos2Spdb::TOKEN_DATE_LEN_MON = 2;
const int Awos2Spdb::TOKEN_DATE_BEGIN_DAY = 9;
const int Awos2Spdb::TOKEN_DATE_LEN_DAY = 2;
const int Awos2Spdb::TOKEN_DATE_BEGIN_HOUR = 11;
const int Awos2Spdb::TOKEN_DATE_LEN_HOUR = 2;
const int Awos2Spdb::TOKEN_DATE_BEGIN_MIN = 13;
const int Awos2Spdb::TOKEN_DATE_LEN_MIN = 2;
const int Awos2Spdb::TOKEN_TEMP_BEGIN = 44;
const int Awos2Spdb::TOKEN_TEMP_LEN = 3;
const int Awos2Spdb::TOKEN_WIND_GUST_BEGIN = 58;
const int Awos2Spdb::TOKEN_WIND_GUST_LEN = 3;
const int Awos2Spdb::TOKEN_WIND_SPEED_BEGIN = 55;
const int Awos2Spdb::TOKEN_WIND_SPEED_LEN = 3;
const int Awos2Spdb::TOKEN_DEWPOINT_BEGIN = 48;
const int Awos2Spdb::TOKEN_DEWPOINT_LEN = 3;
const int Awos2Spdb::TOKEN_PRECIP_BEGIN = 77;
const int Awos2Spdb::TOKEN_PRECIP_LEN = 2;
const int Awos2Spdb::TOKEN_VISIB_BEGIN = 38;
const int Awos2Spdb::TOKEN_VISIB_LEN = 2;
const int Awos2Spdb::TOKEN_WIND_DIRECTION_BEGIN = 52;
const int Awos2Spdb::TOKEN_WIND_DIRECTION_LEN = 2;

const int Awos2Spdb::MINIMUM_INPUT_LENGTH = 65;

/*********************************************************************
 * Constructor
 */

Awos2Spdb::Awos2Spdb(int argc, char **argv)
{
  static const string method_name = "Awos2Spdb::Awos2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Awos2Spdb *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // Display ucopyright message.

  ucopyright(_progName);

  // Get the command line arguments.

  _args = new Args(argc, argv, _progName);
  
  // Get TDRP parameters.

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file: " << params_path << endl;
    
    okay = false;
    
    return;
  }
}


/*********************************************************************
 * Destructor
 */

Awos2Spdb::~Awos2Spdb()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Awos2Spdb *Awos2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (Awos2Spdb *)NULL)
    new Awos2Spdb(argc, argv);
  
  return(_instance);
}

Awos2Spdb *Awos2Spdb::Inst()
{
  assert(_instance != (Awos2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Awos2Spdb::init()
{
  static const string method_name = "Awos2Spdb::init()";
  
  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Awos2Spdb::run()
{
  static const string method_name = "Awos2Spdb::run()";
  
  if (_args->isRealtime())
  {
    cerr << "REALTIME mode not yet implemented..." << endl;
  }
  else
  {
    // Process the files specified on the command line

    while (true)
    {
      const string file_name = _args->nextFile();
      
      if (file_name == "")
	break;
      
      _processData(file_name);
    } /* endwhile - still files to process */
    
  }
  
}


/**********************************************************************
 *              Private Methods
 **********************************************************************/

/*********************************************************************
 * _convertDouble()
 * 
 * Pulls indicated token out of input line, converts it to a double, and
 * returns true upon success
 */

bool Awos2Spdb::_convertDouble(const char* inputLine, const int begin, const int numChars, fl32 &return_value) const
{
  static char token[100];
  char *endptr;


  STRcopy(token, &(inputLine[begin]), numChars + 1);
  if (token[0] == ' ')
    {
      return_value = STATION_NAN;
      return true;
    }

  return_value = strtod(token, &endptr);

  //if these values are the same, it means the whole token had errors in the conversion
  if (endptr == token)
    {
      if (_params->debug >= Params::DEBUG_NORM)
	{
	  cerr << "endptr and token are the same: <" << endptr << ">. Setting to STATION_NAN." << endl;
	}
      return_value = STATION_NAN;
      return true;
    }

  if (endptr[0] != '\0')
    {
      if (_params->debug >= Params::DEBUG_NORM)
	{
	  cerr << "Error parsing token." << endl;
	  cerr << "endptr: <" << endptr << ">" << endl;
	  cerr << "token: <" << token << ">" << endl;
	}
      return false;
    }
  return true;
}

/*********************************************************************
 * _convertDate()
 * 
 * Pulls date information out of input line, calls convertInt() for each token,
 * returns a DateTime object
 */

DateTime Awos2Spdb::_convertDate(const char* inputLine)
{

  int year, month, day, hour, min;
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_YY, TOKEN_DATE_LEN_YY, year))
    {
      cerr << "Error parsing year token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_MON, TOKEN_DATE_LEN_MON, month))
    {
      cerr << "Error parsing month token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_DAY, TOKEN_DATE_LEN_DAY, day))
    {
      cerr << "Error parsing day token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_HOUR, TOKEN_DATE_LEN_HOUR, hour))
    {
      cerr << "Error parsing hour token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DATE_BEGIN_MIN, TOKEN_DATE_LEN_MIN, min))
    {
      cerr << "Error parsing min token." << endl;
      return DateTime::NEVER;
    }
  
  return DateTime(year + 2000, month, day, hour, min);
}

/*********************************************************************
 * _convertInt()
 * 
 * Pulls indicated token out of input line, converts it to an int, and
 * returns true upon success
 */

bool Awos2Spdb::_convertInt(const char* inputLine, const int begin, const int numChars, int &return_value)
{
  static char token[100];
  char *endptr;

  STRcopy(token, &(inputLine[begin]), numChars + 1);

  return_value = strtol(token, &endptr, 10);

  if (endptr[0] != '\0')
    {
      cerr << "Error parsing date token." << endl;
      return false;
    }
  return true;
}

/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool Awos2Spdb::_processData(const string& file_name)
{
  static const string method_name = "Awos2Spdb::_processData()";
  
  static const int INPUT_LINE_LEN = 1024;

  char input_line[INPUT_LINE_LEN];
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "*** Processing file <" << file_name << ">" << endl;

  // Open the input file

  FILE *input_file;
  
  if ((input_file = ta_fopen_uncompress((char *)file_name.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << file_name << endl;
    
    return false;
  }
  
  // Process the data lines

  DsSpdb spdb;
  
  bool passedHeader = false;
  char* token = new char[100];
  
  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
  {
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Processing line: " << input_line << endl;

    // Skip any header lines - start processing with line that begins with '0000'

    if (!passedHeader)
      {
	STRcopy(token, input_line, 5);

	if (STRequal_exact(token, "0000"))
	  {
	    passedHeader = true;
	  }
	else continue;
      }

    // Check for errors

    if (strlen(input_line) < (size_t)MINIMUM_INPUT_LENGTH)
      {
	if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "Skipping line; too short to process." << endl;
	continue;
      }
    if (strstr(input_line, "not available") != NULL)
      {
	if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "Skipping line; data not available." << endl;
	continue;
      }

    // Create the SPDB object with the file information and parse the tokens in the line
    
    station_report_t awos_station;
    
    awos_station.msg_id = STATION_REPORT;

    //units for time? UTC?
    DateTime report_time = _convertDate(input_line);
    if (report_time == DateTime::NEVER)
      {
	cerr << "Error with report time. Skipping line." << endl;
	continue;
      }
    
    awos_station.time = report_time.utime();

    awos_station.accum_start_time = 0;
    awos_station.weather_type = 0;
    awos_station.lat = _params->station_info.latitude;
    awos_station.lon = _params->station_info.longitude;
    awos_station.alt = _params->station_info.altitude;

    if (!_convertDouble(input_line, TOKEN_TEMP_BEGIN, TOKEN_TEMP_LEN, awos_station.temp))
      {
	cerr << "Error converting temperature token to double." << endl;
	continue;
      }

    if (!_convertDouble(input_line, TOKEN_DEWPOINT_BEGIN, TOKEN_DEWPOINT_LEN, awos_station.dew_point))
      {
	cerr << "Error converting dewpoint token to double." << endl;
	continue;
      }

    awos_station.relhum = STATION_NAN;

    if (!_convertDouble(input_line, TOKEN_WIND_SPEED_BEGIN, TOKEN_WIND_SPEED_LEN, awos_station.windspd))
      {
	cerr << "Error converting wind speed token to double." << endl;
	continue;
      }

    if (awos_station.windspd != STATION_NAN)
      awos_station.windspd *= KNOTS_TO_MS;

    if (!_convertDouble(input_line, TOKEN_WIND_DIRECTION_BEGIN, TOKEN_WIND_DIRECTION_LEN, awos_station.winddir))
      {
	cerr << "Error converting wind direction token to double." << endl;
	continue;
      }

    //winddir comes in as tens of degrees so need to convert
    if (awos_station.winddir != STATION_NAN)
      awos_station.winddir *= 10;

    if (!_convertDouble(input_line, TOKEN_WIND_GUST_BEGIN, TOKEN_WIND_GUST_LEN, awos_station.windgust))
      {
	cerr << "Error converting wind gust token to double." << endl;
	continue;
      }

    if (awos_station.windgust != STATION_NAN)
      awos_station.windgust *= KNOTS_TO_MS;

    awos_station.pres = STATION_NAN;

    //unsure of units for precipitation...
    if (!_convertDouble(input_line, TOKEN_PRECIP_BEGIN, TOKEN_PRECIP_LEN, awos_station.liquid_accum))
      {
	cerr << "Error converting liquid accum token to double." << endl;
	continue;
      }

    awos_station.precip_rate = STATION_NAN;

    //no check on return value; it's the one token that doesn't always have leading 0's.
    _convertDouble(input_line, TOKEN_VISIB_BEGIN, TOKEN_VISIB_LEN, awos_station.visibility);

    //stations report visibility in miles
    if (awos_station.visibility != STATION_NAN)
      awos_station.visibility *= KM_PER_MI;
 
    awos_station.rvr = STATION_NAN;
    awos_station.ceiling = STATION_NAN;

    awos_station.shared.station.liquid_accum2 = STATION_NAN;
    awos_station.shared.station.Spare1 = STATION_NAN;
    awos_station.shared.station.Spare2 = STATION_NAN;

    STRcopy(awos_station.station_label, _params->station_info.station_id,
	    ST_LABEL_SIZE);
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      print_station_report(stderr, "", &awos_station);

    // Add the record to the database buffer.  We will write the entire
    // buffer to the database after processing the file to save overhead.

    if (!_addToDatabase(spdb, awos_station))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error adding station record to database buffer" << endl;
      cerr << "*** Skipping record" << endl;
      
      continue;
    }

  }  /* endwhile - lines left in input file */

  fclose(input_file); 

  // Write the chunk buffer to the database

  if (spdb.put(_params->output_spdb_url,
	       SPDB_STATION_REPORT_ID,
	       SPDB_STATION_REPORT_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing chunk buffer to database: "
	 << _params->output_spdb_url << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _addToDatabase() - Add the given record to the SPDB database buffer.
 *
 * Returns true on success, false on failure.
 */

bool Awos2Spdb::_addToDatabase(DsSpdb &spdb, station_report_t& station)
{
  time_t valid_time = station.time;
  time_t expire_time = valid_time + _params->spdb_expire_secs;

  //swapping to big-endian
  station_report_to_be(&station);

  spdb.setPutMode(Spdb::putModeAdd);
  
  spdb.addPutChunk(Spdb::hash4CharsToInt32(station.station_label),
		   valid_time,
		   expire_time,
		   sizeof(station_report_t),
		   &station);

  //swapping back from big-endian
  station_report_from_be(&station);
  
  return true;
}

