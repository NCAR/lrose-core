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
//   $Date: 2016/03/07 01:23:09 $
//   $Id: ihopNcarSfc2Spdb.cc,v 1.4 2016/03/07 01:23:09 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ihopNcarSfc2Spdb: ihopNcarSfc2Spdb program object.
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
#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <cassert>

#include "ihopNcarSfc2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

ihopNcarSfc2Spdb *ihopNcarSfc2Spdb::_instance = (ihopNcarSfc2Spdb *)NULL;

const int ihopNcarSfc2Spdb::MAX_TOKENS = 9;
const int ihopNcarSfc2Spdb::MAX_TOKEN_LEN = 100;

/*********************************************************************
 * Constructor
 */

ihopNcarSfc2Spdb::ihopNcarSfc2Spdb(int argc, char **argv) : _tokens(0)
{
  static const string method_name = "ihopNcarSfc2Spdb::ihopNcarSfc2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ihopNcarSfc2Spdb *)NULL);
  
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

ihopNcarSfc2Spdb::~ihopNcarSfc2Spdb()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;

  if (_tokens != 0)
    {
      for (int i = 0; i < MAX_TOKENS; ++i)
	delete [] _tokens[i];
      delete [] _tokens;
    }

  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

ihopNcarSfc2Spdb *ihopNcarSfc2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (ihopNcarSfc2Spdb *)NULL)
    new ihopNcarSfc2Spdb(argc, argv);
  
  return(_instance);
}

ihopNcarSfc2Spdb *ihopNcarSfc2Spdb::Inst()
{
  assert(_instance != (ihopNcarSfc2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ihopNcarSfc2Spdb::init()
{
  static const string method_name = "ihopNcarSfc2Spdb::init()";

  // Allocate space for the token parsing object

  _tokens = new char*[MAX_TOKENS];

  for (int i = 0; i < MAX_TOKENS; ++i)
    _tokens[i] = new char[MAX_TOKEN_LEN];

  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ihopNcarSfc2Spdb::run()
{
  static const string method_name = "ihopNcarSfc2Spdb::run()";
  
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

      int line_number = 0;
      
      _processData(file_name, line_number);
    } /* endwhile - still files to process */
    
  }
  
}


/**********************************************************************
 *              Private Methods
 **********************************************************************/

/*********************************************************************
 * _convertDouble()
 * 
 * Returns the token converted to a double
 */

double ihopNcarSfc2Spdb::_convertDouble(const char* token)
{
  double temp = atof(token);
  return temp;
}


/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool ihopNcarSfc2Spdb::_processData(const string& file_name, int& line_num)
{
  static const string method_name = "ihopNcarSfc2Spdb::_processData()";
  
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

  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
  {
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Processing line #" << line_num << ": " << input_line << endl;

    // Increment the line number since we've read the line in

    ++line_num;

    // Parse the tokens in the line

    int num_tokens;

    if ((num_tokens = STRparse_delim(input_line, _tokens, INPUT_LINE_LEN, ",",
				     MAX_TOKENS, MAX_TOKEN_LEN)) != NUM_TOKENS)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error parsing input line: " << input_line << endl;
	cerr << "Expect " << NUM_TOKENS << " tokens; found " << num_tokens
	     << " tokens" << endl;
	cerr << "**** Skipping line" << endl;

	continue;
      }

    // Create the SPDB object with the file information; fill out the station_report struct
    
    station_report_t ihopNcarSfc_station;

    ihopNcarSfc_station.msg_id = STATION_REPORT;

    //units for time? UTC?

    DateTime report_time;
    report_time.setByDayOfYear(2002, atoi(_tokens[JULIANDAY_TOKEN]));

    // Set hour/min of DateTime object; seconds default to zero

    report_time.setHour(atoi(_tokens[HHMM_TOKEN])/100);
    report_time.setMin(atoi(_tokens[HHMM_TOKEN])%100);

    ihopNcarSfc_station.time = report_time.utime();

    ihopNcarSfc_station.accum_start_time = 0;
    ihopNcarSfc_station.weather_type = 0;
    ihopNcarSfc_station.lat = _params->station_info.latitude;
    ihopNcarSfc_station.lon = _params->station_info.longitude;
    ihopNcarSfc_station.alt = _params->station_info.altitude;

    // Temperature (degrees C)

    ihopNcarSfc_station.temp = _convertDouble(_tokens[TEMPERATURE_TOKEN]);
    ihopNcarSfc_station.dew_point = STATION_NAN;

    // Humidity (%)

    ihopNcarSfc_station.relhum = _convertDouble(_tokens[HUMIDITY_TOKEN]);

    // Windspeed (m/s)

    ihopNcarSfc_station.windspd = _convertDouble(_tokens[WINDSPEED_TOKEN]);

    // Winddirection (degrees)

    ihopNcarSfc_station.winddir = _convertDouble(_tokens[WINDDIRECTION_TOKEN]);

    ihopNcarSfc_station.windgust = STATION_NAN;

    // Pressure (mb)

    ihopNcarSfc_station.pres = _convertDouble(_tokens[PRESSURE_TOKEN]);

    ihopNcarSfc_station.liquid_accum = STATION_NAN;
    ihopNcarSfc_station.precip_rate = STATION_NAN;
    ihopNcarSfc_station.visibility = STATION_NAN;
    ihopNcarSfc_station.rvr = STATION_NAN;
    ihopNcarSfc_station.ceiling = STATION_NAN;
    ihopNcarSfc_station.shared.station.liquid_accum2 = STATION_NAN;
    ihopNcarSfc_station.shared.station.Spare1 = STATION_NAN;
    ihopNcarSfc_station.shared.station.Spare2 = STATION_NAN;

    STRcopy(ihopNcarSfc_station.station_label, _params->station_info.station_id,
	    ST_LABEL_SIZE);
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      print_station_report(stderr, "", &ihopNcarSfc_station);

    if (!_writeToDatabase(ihopNcarSfc_station))
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

bool ihopNcarSfc2Spdb::_writeToDatabase(station_report_t& station)
{
  DsSpdb spdb;
  
  time_t valid_time = station.time;
  time_t expire_time = valid_time + _params->spdb_expire_secs;

  //swapping to big-endian
  station_report_to_be(&station);

  spdb.setPutMode(Spdb::putModeAdd);
  
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

