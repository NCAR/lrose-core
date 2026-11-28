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
//   $Id: southWestKs2Spdb.cc,v 1.9 2016/03/07 01:23:11 dixon Exp $
//   $Revision: 1.9 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * southWestKs2Spdb: southWestKs2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <ctype.h>
#include <iostream>
#include <string>
#include <cassert>

#include <toolsa/os_config.h>
#include <rapformats/station_reports.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>
#include <rapmath/math_macros.h>

#include "southWestKs2Spdb.hh"
#include "Params.hh"
using namespace std;

// Global variables

southWestKs2Spdb *southWestKs2Spdb::_instance = (southWestKs2Spdb *)NULL;

const int southWestKs2Spdb::MAX_TOKENS = 25;
const int southWestKs2Spdb::MAX_TOKEN_LEN = 100;
const double southWestKs2Spdb::UNAVAIL = -9999.0;
const double southWestKs2Spdb::UNAVAILABLE = -99.900;


/*********************************************************************
 * Constructor
 */

southWestKs2Spdb::southWestKs2Spdb(int argc, char **argv) :
  _tokens(0)
{
  static const string method_name = "southWestKs2Spdb::southWestKs2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (southWestKs2Spdb *)NULL);
  
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

southWestKs2Spdb::~southWestKs2Spdb()
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

southWestKs2Spdb *southWestKs2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (southWestKs2Spdb *)NULL)
    new southWestKs2Spdb(argc, argv);
  
  return(_instance);
}

southWestKs2Spdb *southWestKs2Spdb::Inst()
{
  assert(_instance != (southWestKs2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool southWestKs2Spdb::init()
{
  static const string method_name = "southWestKs2Spdb::init()";
  
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

void southWestKs2Spdb::run()
{
  static const string method_name = "southWestKs2Spdb::run()";
  
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
 *
 * Returns the token converted to a double, resetting unavail data to
 * STATION_NAN.
 */

double southWestKs2Spdb::_convertDouble(const char* token)
{
  double temp = atof(token);
  if (temp == UNAVAIL || temp == UNAVAILABLE)
    return STATION_NAN;
  return temp;
}


/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool southWestKs2Spdb::_processData(const string& file_name,
			     int& line_num)
{
  static const string method_name = "southWestKs2Spdb::_processData()";
  
  static const int INPUT_LINE_LEN = 1024;

  char input_line[INPUT_LINE_LEN];
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "*** Processing file <" << file_name << "> starting at line "
	 << line_num << endl;

  // Open the input file

  FILE *input_file;
  
  if ((input_file = ta_fopen_uncompress((char *)file_name.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << file_name << endl;
    
    return false;
  }
  
  // Skip to the indicated line number

  for (int i = 0; i < line_num; ++i)
    fgets(input_line, INPUT_LINE_LEN, input_file);

  //parse the file name; this is used later to extract the 3-letter station code

  path_parts_t pathInfo;
  uparse_path((char*) file_name.c_str(), &pathInfo);
  if (_params->debug >= Params::DEBUG_VERBOSE)
    {
      cerr << "Directory: " << pathInfo.dir << ", Filename: " << pathInfo.name << ", Filename base: "
	   << pathInfo.base << ", Filename extension: " << pathInfo.ext << endl;
    }

  // Skip any header lines

  DateTime report_time;
  station_report_t sw_ks_station;
  
  while (line_num < _params->num_header_lines)
    {
      fgets(input_line, INPUT_LINE_LEN, input_file);
  
      //report_time.set(year, mon, day);
      //sw_ks_station.alt = alt;

      if (_params->debug >= Params::DEBUG_VERBOSE)
	cerr << "Skipping header line: " << input_line << endl;
      ++line_num;
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
	  cerr << "Expected " << NUM_TOKENS << " tokens, found " << num_tokens
	       << " tokens" << endl;
	  cerr << "*** Skipping line" << endl;
	  
	  continue;
	}
      
      // Create the SPDB object with the file information
      // involves filling out the station_report struct
    
      sw_ks_station.msg_id = STATION_REPORT;

      DateTime report_time;
      report_time.setByDayOfYear(atoi(_tokens[YEAR_TOKEN]),
				 atoi(_tokens[JULIAN_DAY_TOKEN]));

      //set hour/min of DateTime object; secs default to zero
      //time comes in as CST
      report_time.setHour((atoi(_tokens[TIME_HHMM_TOKEN])/100) + 6);
      report_time.setMin(atoi(_tokens[TIME_HHMM_TOKEN])%100);
      sw_ks_station.time = report_time.utime();

      sw_ks_station.accum_start_time = 0;
      sw_ks_station.weather_type = 0;
      sw_ks_station.lat = _params->station_info.latitude;
      sw_ks_station.lon = _params->station_info.longitude;
      sw_ks_station.alt = STATION_NAN;         //meters

      sw_ks_station.temp = TEMP_F_TO_C(_convertDouble(_tokens[AVER_AIR_TEMP_DEGF_TOKEN]));

      sw_ks_station.dew_point = STATION_NAN;       //degrees C
      sw_ks_station.relhum = _convertDouble(_tokens[AVER_RH_TOKEN]);   //percent

      //convert MPH to m/s in below assignment
      sw_ks_station.windspd = _convertDouble(_tokens[AVER_WIND_SPEED_MPH_TOKEN]) * MPH_TO_MS; //m/sec
      sw_ks_station.winddir = _convertDouble(_tokens[WIND_DIRECTION_TOKEN]);      //deg

      //convert MPH to m/s in below assignment
      sw_ks_station.windgust = _convertDouble(_tokens[MAX_WIND_GUST_MPH_TOKEN]) * MPH_TO_MS;  //m/sec

      // thought this was in millibars but it appears to be inches of mercury
      sw_ks_station.pres = _convertDouble(_tokens[AVER_PRES_TOKEN]) * 33.864;

      //precip in inches gets converted to mm
      sw_ks_station.liquid_accum = _convertDouble(_tokens[PRECIP_IN_TOKEN]) * INCHES_TO_MM;

      sw_ks_station.precip_rate = STATION_NAN;        //mm/hr
      sw_ks_station.visibility = STATION_NAN;         //km
      sw_ks_station.rvr = STATION_NAN;         //runway visual range km
      sw_ks_station.ceiling = STATION_NAN;

      sw_ks_station.shared.station.liquid_accum2 = STATION_NAN;
      sw_ks_station.shared.station.Spare1 = STATION_NAN;
      sw_ks_station.shared.station.Spare2 = STATION_NAN;

    //use first characters of the base part of the parsed filename, converted to lowercase

      STRcopy(sw_ks_station.station_label, pathInfo.base,
	      4);
      sw_ks_station.station_label[0] = tolower(sw_ks_station.station_label[0]);
      sw_ks_station.station_label[1] = tolower(sw_ks_station.station_label[1]);
      sw_ks_station.station_label[2] = tolower(sw_ks_station.station_label[2]);
    
      if (_params->debug >= Params::DEBUG_VERBOSE)
	print_station_report(stderr, "", &sw_ks_station);

      if (!_writeToDatabase(sw_ks_station))
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

bool southWestKs2Spdb::_writeToDatabase(station_report_t& station)
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
