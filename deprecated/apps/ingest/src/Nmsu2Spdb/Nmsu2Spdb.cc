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
//   $Date: 2016/03/07 01:23:04 $
//   $Id: Nmsu2Spdb.cc,v 1.6 2016/03/07 01:23:04 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Nmsu2Spdb: Nmsu2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <cassert>

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

#include "Nmsu2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

Nmsu2Spdb *Nmsu2Spdb::_instance = (Nmsu2Spdb *)NULL;

const int Nmsu2Spdb::MAX_TOKENS = 25;
const int Nmsu2Spdb::MAX_TOKEN_LEN = 100;
const double Nmsu2Spdb::UNAVAIL = -9999.0;


/*********************************************************************
 * Constructor
 */

Nmsu2Spdb::Nmsu2Spdb(int argc, char **argv) : _tokens(0)
{
  static const string method_name = "Nmsu2Spdb::Nmsu2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Nmsu2Spdb *)NULL);
  
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

Nmsu2Spdb::~Nmsu2Spdb()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;

if (_tokens !=0)
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

Nmsu2Spdb *Nmsu2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (Nmsu2Spdb *)NULL)
    new Nmsu2Spdb(argc, argv);
  
  return(_instance);
}

Nmsu2Spdb *Nmsu2Spdb::Inst()
{
  assert(_instance != (Nmsu2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Nmsu2Spdb::init()
{
  static const string method_name = "Nmsu2Spdb::init()";
  
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

void Nmsu2Spdb::run()
{
  static const string method_name = "Nmsu2Spdb::run()";
  
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
 * Returns the token converted to a double, resetting unavail data to
 * STATION_NAN.
 */

double Nmsu2Spdb::_convertDouble(const char* token) const
{
  double temp = atof(token);
  if (temp == UNAVAIL)
    return STATION_NAN;
  return temp;
}


/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool Nmsu2Spdb::_processData(const string& file_name)
{
  static const string method_name = "Nmsu2Spdb::_processData()";
  
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
      cerr << "Processing line: " << input_line << endl;

    //Parse the tokens in the line

    int num_tokens;

    if ((num_tokens = STRparse(input_line, _tokens, INPUT_LINE_LEN,
			       MAX_TOKENS, MAX_TOKEN_LEN)) != NUM_TOKENS)
      {
	if ((num_tokens = STRparse(input_line, _tokens, INPUT_LINE_LEN,
				   MAX_TOKENS, MAX_TOKEN_LEN)) != (NUM_TOKENS + 1))
	  {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "Error parsing input line: " << input_line << endl;
	    cerr << "Expect " << NUM_TOKENS << " tokens, found " << num_tokens << num_tokens
		 << " tokens" << endl;
	    cerr << "*** Skipping line" << endl;

	    continue;
	  }
      }

    // Create the SPDB object with the file information and parse the tokens in the line
    
    station_report_t nmsu_station;
    
    nmsu_station.msg_id = STATION_REPORT;

    //need to convert from 12-hr to 24-hr clock
    DateTime report_time;
    int hour;
    hour = atoi(_tokens[HOUR_TOKEN]);

    if (strstr(_tokens[HOUR_TOKEN], "12AM"))
      {
	hour = atoi(_tokens[HOUR_TOKEN]) - 12;
      }

    if (strstr(_tokens[HOUR_TOKEN], "PM"))
      {
	hour = atoi(_tokens[HOUR_TOKEN]) + 12;
      }

    if (strstr(_tokens[HOUR_TOKEN], "12PM"))
      {
	hour = atoi(_tokens[HOUR_TOKEN]);
      }

    report_time.set(atoi(_tokens[YEAR_TOKEN]) + 2000,
		    atoi(_tokens[MON_TOKEN]),
		    atoi(_tokens[DAY_TOKEN]),
		    hour);

    //time is MST (no daylight savings) so need to add 7 hour's worth of seconds
    report_time += DeltaTime(7 * 60 * 60);

    nmsu_station.time = report_time.utime();

    nmsu_station.accum_start_time = 0;
    nmsu_station.weather_type = 0;
    nmsu_station.lat = _params->station_info.latitude;
    nmsu_station.lon = _params->station_info.longitude;
    nmsu_station.alt = _params->station_info.altitude;

    nmsu_station.temp = _convertDouble(_tokens[AIR_TEMP_DEGF_TOKEN]);
    if (nmsu_station.temp != STATION_NAN)
      nmsu_station.temp = TEMP_F_TO_C(nmsu_station.temp);

    nmsu_station.dew_point = STATION_NAN;
    nmsu_station.relhum = _convertDouble(_tokens[PERCENT_REL_HUM_TOKEN]);

    nmsu_station.windspd = _convertDouble(_tokens[MPH_WIND_SPEED_TOKEN]);
    if (nmsu_station.windspd != STATION_NAN)
      nmsu_station.windspd *= MPH_TO_MS;

    nmsu_station.winddir = _convertDouble(_tokens[DEG_WIND_DIRECTION_TOKEN]);
    nmsu_station.windgust = STATION_NAN;
    nmsu_station.pres = STATION_NAN;

    nmsu_station.liquid_accum = _convertDouble(_tokens[INCHES_PRECIP_TOKEN]);
    if (nmsu_station.liquid_accum != STATION_NAN)
      nmsu_station.liquid_accum *= INCHES_TO_MM;

    nmsu_station.precip_rate = STATION_NAN;
    nmsu_station.visibility = STATION_NAN;
 
    nmsu_station.rvr = STATION_NAN;
    nmsu_station.ceiling = STATION_NAN;

    nmsu_station.shared.station.liquid_accum2 = STATION_NAN;
    nmsu_station.shared.station.Spare1 = STATION_NAN;
    nmsu_station.shared.station.Spare2 = STATION_NAN;

    STRcopy(nmsu_station.station_label, _params->station_info.station_id,
	    ST_LABEL_SIZE);
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      print_station_report(stderr, "", &nmsu_station);

    if (!_writeToDatabase(nmsu_station))
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

bool Nmsu2Spdb::_writeToDatabase(station_report_t& station)
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

