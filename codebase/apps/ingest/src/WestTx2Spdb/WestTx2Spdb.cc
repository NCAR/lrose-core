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
//   $Date: 2016/03/07 01:23:07 $
//   $Id: WestTx2Spdb.cc,v 1.3 2016/03/07 01:23:07 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * WestTx2Spdb: WestTx2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sep 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <ctype.h>
#include <iostream>
#include <string>

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

#include "WestTx2Spdb.hh"
#include "Params.hh"
using namespace std;

// Global variables

WestTx2Spdb *WestTx2Spdb::_instance = (WestTx2Spdb *)NULL;

const int WestTx2Spdb::MAX_TOKENS = 25;
const int WestTx2Spdb::MAX_TOKEN_LEN = 100;
const double WestTx2Spdb::UNAVAIL = -6999;
const double WestTx2Spdb::UNAVAILABLE = 6999;


/*********************************************************************
 * Constructor
 */

WestTx2Spdb::WestTx2Spdb(int argc, char **argv) :
  _tokens(0)
{
  static const string method_name = "WestTx2Spdb::WestTx2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (WestTx2Spdb *)NULL);
  
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

WestTx2Spdb::~WestTx2Spdb()
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

WestTx2Spdb *WestTx2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (WestTx2Spdb *)NULL)
    new WestTx2Spdb(argc, argv);
  
  return(_instance);
}

WestTx2Spdb *WestTx2Spdb::Inst()
{
  assert(_instance != (WestTx2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool WestTx2Spdb::init()
{
  static const string method_name = "WestTx2Spdb::init()";
  
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

void WestTx2Spdb::run()
{
  static const string method_name = "WestTx2Spdb::run()";
  
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
 *
 * Returns the token converted to a double, resetting unavailable data to
 * STATION_NAN.
 */

double WestTx2Spdb::_convertDouble(const char* token)
{
  double temp = atof(token);
  if (temp == UNAVAIL || temp == UNAVAILABLE)
    {
      cerr << "WARNING! Found missing_data value; setting to STATION_NAN." << endl;
      return STATION_NAN;
    }
  return temp;
}


/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool WestTx2Spdb::_processData(const string& file_name)
{
  static const string method_name = "WestTx2Spdb::_processData()";
  
  static const int INPUT_LINE_LEN = 1024;

  char input_line[INPUT_LINE_LEN];
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "*** Processing file <" << file_name << ">" << endl;

  // Open the input file

  FILE *input_file;
  
  if ((input_file = ta_fopen_uncompress((char *)file_name.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << file_name << endl;
    
    return false;
  }
  
  station_report_t west_tx_station;
  
  // Process the data lines

  while (fgets(input_line, INPUT_LINE_LEN, input_file) != 0)
    {
      if (_params->debug >= Params::DEBUG_VERBOSE)
	cerr << "Processing line: " << input_line << endl;

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

      if (atoi(_tokens[ID_TOKEN]) != 1)
	{
	  cerr << "Skipping line; not surface met data (could be soil or radiation data)." << endl;
	  continue;
	}
      
      // Create the SPDB object with the file information
      // involves filling out the station_report struct
    
      west_tx_station.msg_id = STATION_REPORT;

      DateTime report_time;
      int hrs, min;

      //add 6 hours to local standard time to get UTC

      hrs = (atoi(_tokens[LOCAL_TIME_TOKEN]) / 100) + 6;
      min = atoi(_tokens[LOCAL_TIME_TOKEN]) % 100;
      report_time.setByDayOfYear(2002,
				 atoi(_tokens[JULIAN_DAY_TOKEN]),
				 hrs, min);

      west_tx_station.time = report_time.utime();

      west_tx_station.accum_start_time = 0;
      west_tx_station.weather_type = 0;

      west_tx_station.lat = _params->station_info.latitude;
      west_tx_station.lon = _params->station_info.longitude;

      //convert feet to meters
      west_tx_station.alt = (_params->station_info.altitude) * 0.3048;

      west_tx_station.temp = _convertDouble(_tokens[TEMP_1POINT5_TOKEN]);
      west_tx_station.dew_point = _convertDouble(_tokens[DEWPOINT_DEGC_TOKEN]);
      west_tx_station.relhum = _convertDouble(_tokens[RH_1POINT5_TOKEN]);
      west_tx_station.windspd = _convertDouble(_tokens[WINDSPD_10M_SCALER_TOKEN]);
      west_tx_station.winddir = _convertDouble(_tokens[WINDDIR_10M_TOKEN]);
      west_tx_station.windgust = _convertDouble(_tokens[PEAKWINDSPD_10M_TOKEN]);
      west_tx_station.pres = _convertDouble(_tokens[PRESSURE_TOKEN]);

      //CODIAC dataset description says to add 600 to get correct value in mb
      if (west_tx_station.pres != STATION_NAN)
	west_tx_station.pres = west_tx_station.pres + 600;

      //precip in inches gets converted to mm
      west_tx_station.liquid_accum = _convertDouble(_tokens[RAINFALL_IN_TOKEN]) * INCHES_TO_MM;

      west_tx_station.precip_rate = STATION_NAN;
      west_tx_station.visibility = STATION_NAN;
      west_tx_station.rvr = STATION_NAN;
      west_tx_station.ceiling = STATION_NAN;
      west_tx_station.shared.station.liquid_accum2 = STATION_NAN;
      west_tx_station.shared.station.Spare1 = STATION_NAN;
      west_tx_station.shared.station.Spare2 = STATION_NAN;

      STRcopy(west_tx_station.station_label, _params->station_info.station_id, ST_LABEL_SIZE);

      if (_params->debug >= Params::DEBUG_VERBOSE)
	print_station_report(stderr, "", &west_tx_station);

      if (!_writeToDatabase(west_tx_station))
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

bool WestTx2Spdb::_writeToDatabase(station_report_t& station)
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
