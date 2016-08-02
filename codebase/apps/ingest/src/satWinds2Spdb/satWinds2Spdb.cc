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
//   $Date: 2016/03/07 01:23:11 $
//   $Id: satWinds2Spdb.cc,v 1.6 2016/03/07 01:23:11 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * satWinds2Spdb: satWinds2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "satWinds2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

satWinds2Spdb *satWinds2Spdb::_instance = (satWinds2Spdb *)NULL;

const int satWinds2Spdb::TOKEN_YEAR_BEGIN = 18;
const int satWinds2Spdb::TOKEN_YEAR_LEN = 1;

const int satWinds2Spdb::TOKEN_MONTH_BEGIN = 19;
const int satWinds2Spdb::TOKEN_MONTH_LEN = 2;

const int satWinds2Spdb::TOKEN_DAY_BEGIN = 21;
const int satWinds2Spdb::TOKEN_DAY_LEN = 2;

const int satWinds2Spdb::TOKEN_HOUR_BEGIN = 25;
const int satWinds2Spdb::TOKEN_HOUR_LEN = 2;

const int satWinds2Spdb::TOKEN_MIN_BEGIN = 27;
const int satWinds2Spdb::TOKEN_MIN_LEN = 2;

const int satWinds2Spdb::TOKEN_LAT_BEGIN = 31;
const int satWinds2Spdb::TOKEN_LAT_LEN = 6;

const int satWinds2Spdb::TOKEN_LON_BEGIN = 40;
const int satWinds2Spdb::TOKEN_LON_LEN = 6;

const int satWinds2Spdb::TOKEN_PRESSURE_BEGIN = 49;
const int satWinds2Spdb::TOKEN_PRESSURE_LEN = 3;

const int satWinds2Spdb::TOKEN_WIND_SPEED_BEGIN = 55;
const int satWinds2Spdb::TOKEN_WIND_SPEED_LEN = 4;

const int satWinds2Spdb::TOKEN_WIND_DIRECTION_BEGIN = 61;
const int satWinds2Spdb::TOKEN_WIND_DIRECTION_LEN = 3;

const int satWinds2Spdb::MINIMUM_INPUT_LENGTH = 70;

/*********************************************************************
 * Constructor
 */

satWinds2Spdb::satWinds2Spdb(int argc, char **argv)
{
  static const string method_name = "satWinds2Spdb::satWinds2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (satWinds2Spdb *)NULL);
  
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
  char *params_path = "unknown";
  
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

satWinds2Spdb::~satWinds2Spdb()
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

satWinds2Spdb *satWinds2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (satWinds2Spdb *)NULL)
    new satWinds2Spdb(argc, argv);
  
  return(_instance);
}

satWinds2Spdb *satWinds2Spdb::Inst()
{
  assert(_instance != (satWinds2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool satWinds2Spdb::init()
{
  static const string method_name = "satWinds2Spdb::init()";
  
  // initialize data trigger

  if (_args->isRealtime())
    {
      DsLdataTrigger *ldata_trigger = new DsLdataTrigger();
      if (ldata_trigger->init(_params->realtime_input_dir,
			      _params->max_realtime_valid_age,
			      PMU_auto_register) != 0 )
	{
	  cerr << "ERROR! Error in initializing DsLdataTrigger." << endl;
	  return false;
	}

      // set data trigger for real-time mode

      if (_params->debug >= Params::DEBUG_NORM)
	cerr << "Setting ldata_trigger." << endl;

      _dataTrigger = ldata_trigger;
    }
  else
    {
      DsFileListTrigger *file_list_trigger = new DsFileListTrigger();

      vector<string> file_list;
      string file_name;
      while ((file_name = _args->nextFile()) != "")
	file_list.push_back(file_name);
      if (file_list_trigger->init(file_list) != 0)
	{
	  cerr << "ERROR! Error in initializing DsFileListTrigger." << endl;
	  return false;
	}
      // set data trigger for archive mode

      if (_params->debug >= Params::DEBUG_NORM)
	cerr << "Setting file_list_trigger." << endl;

      _dataTrigger = file_list_trigger;
    }

  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void satWinds2Spdb::run()
{
  static const string method_name = "satWinds2Spdb::run()";
  
  TriggerInfo trigger_info;
  while (_dataTrigger->next(trigger_info) == 0)
    _processData(trigger_info.getFilePath());
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

bool satWinds2Spdb::_convertDouble(const char* inputLine, const int begin, const int numChars, fl32 &return_value) const
{
  static char token[100];
  char *endptr;

  STRcopy(token, &(inputLine[begin]), numChars + 1);

  //test to see if the token consists of blank spaces; if so, set to
  //STATION_NAN

  bool allBlanks = true;

  for (int i=0; i < numChars; ++i)
    {
      if (token[i] != ' ')
        {
          allBlanks = false;
          break;
        }
    }
  
  if (allBlanks)
    {
      cerr << "WARNING! token consists of all blanks. Setting to STATION_NAN." << endl;
      return_value = STATION_NAN;
      return true;
    }

  return_value = strtod(token, &endptr);

  //if these values are the same, it means the whole token had errors in the conversion

  if (endptr == token)
    {
      if (_params->debug >= Params::DEBUG_NORM)
	{
	  cerr << "WARNING! endptr and token are the same: <" << endptr << ">. Setting to STATION_NAN." << endl;
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

DateTime satWinds2Spdb::_convertDate(const char* inputLine)
{

  int year, month, day, hour, min;

  if (!_convertInt(inputLine, TOKEN_YEAR_BEGIN, TOKEN_YEAR_LEN, year))
    {
      cerr << "Error parsing year token." << endl;
      return DateTime::NEVER;
    }

  if (!_convertInt(inputLine, TOKEN_MONTH_BEGIN, TOKEN_MONTH_LEN, month))
    {
      cerr << "Error parsing month token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_DAY_BEGIN, TOKEN_DAY_LEN, day))
    {
      cerr << "Error parsing day token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_HOUR_BEGIN, TOKEN_HOUR_LEN, hour))
    {
      cerr << "Error parsing hour token." << endl;
      return DateTime::NEVER;
    }
  if (!_convertInt(inputLine, TOKEN_MIN_BEGIN, TOKEN_MIN_LEN, min))
    {
      cerr << "Error parsing minute token." << endl;
      return DateTime::NEVER;
    }

  return DateTime(year + 2000, month, day, hour, min, 0.0);
}

/*********************************************************************
 * _convertInt()
 * 
 * Pulls indicated token out of input line, converts it to an int, and
 * returns true upon success
 */

bool satWinds2Spdb::_convertInt(const char* inputLine, const int begin, const int numChars, int &return_value)
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

bool satWinds2Spdb::_processData(const string& file_name)
{
  static const string method_name = "satWinds2Spdb::_processData()";
  
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
  
  // Skip any header lines

  int line_num = 0;
  while (line_num < _params->num_header_lines)
    {
      fgets(input_line, INPUT_LINE_LEN, input_file);
      
      if (_params->debug >= Params::DEBUG_VERBOSE)
	cerr << "Skipping header line: " << input_line << endl;
      ++line_num;
    }
      
  // Process the data lines

  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
    {
      if (_params->debug >= Params::DEBUG_VERBOSE)
        cerr << "Processing line #" << line_num << ": " << input_line << endl;
      ++line_num;

      // Check that the line is long enough/has data

      if (strlen(input_line) < (size_t)MINIMUM_INPUT_LENGTH)
	{
	  if (_params->debug >= Params::DEBUG_NORM)
	    cerr << "Skipping line; too short to process." << endl;
	  continue;
	}

    // Create the SPDB object with the file information and parse the tokens in the line
    
      station_report_t satWinds_station;
    
      satWinds_station.msg_id = STATION_REPORT;

      //units for time? UTC?

      DateTime report_time = _convertDate(input_line);

      if (report_time == DateTime::NEVER)
	{
	  cerr << "Error with report time. Skipping line." << endl;
	  continue;
	}
    
      satWinds_station.time = report_time.utime();

      satWinds_station.accum_start_time = 0;
      satWinds_station.weather_type = 0;

      if (!_convertDouble(input_line, TOKEN_LAT_BEGIN, TOKEN_LAT_LEN, satWinds_station.lat))
	{
	  cerr << "Error converting latitude token to double." << endl;
	  continue;
	}
      
      if (!_convertDouble(input_line, TOKEN_LON_BEGIN, TOKEN_LON_LEN, satWinds_station.lon))
	{
	  cerr << "Error converting wind speed token to double." << endl;
	  continue;
	}

      satWinds_station.alt = STATION_NAN;
      satWinds_station.temp = STATION_NAN;
      satWinds_station.dew_point = STATION_NAN;
      satWinds_station.relhum = STATION_NAN;

      if (!_convertDouble(input_line, TOKEN_WIND_SPEED_BEGIN, TOKEN_WIND_SPEED_LEN, satWinds_station.windspd))
	{
	  cerr << "Error converting wind speed token to double." << endl;
	  continue;
	}

      if (!_convertDouble(input_line, TOKEN_WIND_DIRECTION_BEGIN, TOKEN_WIND_DIRECTION_LEN, satWinds_station.winddir))
	{
	  cerr << "Error converting wind direction token to double." << endl;
	  continue;
	}
      
      satWinds_station.windgust = STATION_NAN;
      
      if (!_convertDouble(input_line, TOKEN_PRESSURE_BEGIN, TOKEN_PRESSURE_LEN, satWinds_station.pres))
	{
	  cerr << "Error converting pressure token to double." << endl;
	  continue;
	}

      satWinds_station.liquid_accum = STATION_NAN;
      satWinds_station.precip_rate = STATION_NAN;
      satWinds_station.visibility = STATION_NAN;
      satWinds_station.rvr = STATION_NAN;
      satWinds_station.ceiling = STATION_NAN;
      satWinds_station.shared.station.liquid_accum2 = STATION_NAN;
      satWinds_station.shared.station.Spare1 = STATION_NAN;
      satWinds_station.shared.station.Spare2 = STATION_NAN;

      // Set the station_label to null string
      satWinds_station.station_label[0] = '\0';

      if (_params->debug >= Params::DEBUG_VERBOSE)
	print_station_report(stderr, "", &satWinds_station);
      
      if (!_writeToDatabase(satWinds_station))
      	{
      	  cerr << "ERROR: " << method_name << endl;
      	  cerr << "Error writing station record to database" << endl;
      	  cerr << "*** Skipping record" << endl;
      	  
      	  continue;
      	}
      
    }  /* endwhile - lines left in input file */

  fclose(input_file); 
  return true;
}


/*********************************************************************
 * _writeToDatabase() - Write the given record to the SPDB database.
 *
 * Returns true on success, false on failure.
 */

bool satWinds2Spdb::_writeToDatabase(station_report_t& station)
{
  DsSpdb spdb;
  
  time_t valid_time = station.time;
  time_t expire_time = valid_time + _params->spdb_expire_secs;

  //swapping to big-endian
  station_report_to_be(&station);

  spdb.setPutMode(Spdb::putModeAddUnique);
  
  if (spdb.put(_params->output_spdb_url,
	       SPDB_STATION_REPORT_ID,
	       SPDB_STATION_REPORT_LABEL,
	       Spdb::hash4CharsToInt32(station.station_label),
	       valid_time,
	       expire_time,
	       sizeof(station_report_t),
	       &station) != 0)
    return false;

  //swapping back from big-endian
  station_report_from_be(&station);
  
  return true;
}

