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
//   $Date: 2016/03/07 01:23:05 $
//   $Id: TXSchoolnet2Spdb.cc,v 1.4 2016/03/07 01:23:05 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TXSchoolnet2Spdb: TXSchoolnet2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Sept 2003
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

#include "TXSchoolnet2Spdb.hh"
#include "Params.hh"


// Global variables

TXSchoolnet2Spdb *TXSchoolnet2Spdb::_instance = (TXSchoolnet2Spdb *)NULL;

const int TXSchoolnet2Spdb::MAX_TOKENS = 25;
const int TXSchoolnet2Spdb::MAX_TOKEN_LEN = 100;

const double TXSchoolnet2Spdb::directionTable[] = {0.0, 0.0, 22.5, 45.0, 67.5, 90.0, 112.5,
						   135.0, 157.5, 180.0, 202.5, 225.0, 247.5,
						   270.0, 292.5, 315.0, 337.5};

/*********************************************************************
 * Constructor
 */

TXSchoolnet2Spdb::TXSchoolnet2Spdb(int argc, char **argv) : _tokens(0)
{
  static const string method_name = "TXSchoolnet2Spdb::TXSchoolnet2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TXSchoolnet2Spdb *)NULL);
  
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

TXSchoolnet2Spdb::~TXSchoolnet2Spdb()
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

TXSchoolnet2Spdb *TXSchoolnet2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (TXSchoolnet2Spdb *)NULL)
    new TXSchoolnet2Spdb(argc, argv);
  
  return(_instance);
}

TXSchoolnet2Spdb *TXSchoolnet2Spdb::Inst()
{
  assert(_instance != (TXSchoolnet2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TXSchoolnet2Spdb::init()
{
  static const string method_name = "TXSchoolnet2Spdb::init()";
  
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

void TXSchoolnet2Spdb::run()
{
  static const string method_name = "TXSchoolnet2Spdb::run()";
  
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

double TXSchoolnet2Spdb::_convertDouble(const char* token) const
{
  //TXSchoolNet files use '- -' to indicate missing data
  //I check for 2 hyphens so as not to mistake a minus sign for missing data

  if (token[0] == '-')
    {
      if (token[2] == '-')
	{
	  cerr << "WARNING! Missing data; setting to STATION_NAN." << endl;
	  cerr << "         token: <" << token << ">" << endl;
	  return STATION_NAN;
	}
    }

  double temp = atof(token);
  return temp;
}


/*********************************************************************
 * _processData() - Process data in the given file, starting at the
 *                  given line number.
 *
 * Returns true on success, false on failure.
 */

bool TXSchoolnet2Spdb::_processData(const string& file_name)
{
  static const string method_name = "TXSchoolnet2Spdb::_processData()";
  
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

    //Parse the tokens in the line using STRparse_delim(); tab is the delimiter
    //Check to see that the line contains all the tokens

    int num_tokens;
    if ((num_tokens = STRparse_delim(input_line, _tokens, INPUT_LINE_LEN, "\t",
			       MAX_TOKENS, MAX_TOKEN_LEN)) != NUM_TOKENS)
      {
	    cerr << "ERROR: " << method_name << endl;
	    cerr << "Error parsing input line: " << input_line << endl;
	    cerr << "Expected " << NUM_TOKENS << " tokens; found " << num_tokens << " tokens" << endl;
	    cerr << "*** Skipping line" << endl;

	    continue;
      }

    if (_params->debug >= Params::DEBUG_VERBOSE)
      {
	cerr << "Expecting: " << NUM_TOKENS << " tokens." << endl;
	cerr << "Found: " << num_tokens << " tokens." << endl;
      }

    // Create the SPDB object with the file information and parse the tokens in the line
    
    station_report_t txSchoolNet_station;
    
    txSchoolNet_station.msg_id = STATION_REPORT;

    // Sift out the date/time parts of the DATE_TOKEN and HOUR_CST_TOKEN
    // Need to ignore the quotation marks that surround these tokens

    int month, day, year;
    if (sscanf(_tokens[DATE_TOKEN], "\"%d/%d/%d\"", &month, &day, &year) != 3)
      {
	cerr << "ERROR parsing MM/DD/YYYY token." << endl;
	cerr << "*** Skipping line" << endl;
	continue;
      }

    int hour, min;
    if (sscanf(_tokens[HOUR_CST_TOKEN], "\"%d:%d\"", &hour, &min) != 2)
      {
	cerr << "ERROR parsing HH:MM token." << endl;
	cerr << "*** Skipping line" << endl;
	continue;
      }

    DateTime report_time;
    report_time.set(year, month, day, hour, min);

    //Time is Central Standard Time which lags UTC by 6 hours so need to add 6 hour's worth of seconds
    report_time += DeltaTime(6 * 60 * 60);

    txSchoolNet_station.time = report_time.utime();
    txSchoolNet_station.accum_start_time = 0;
    txSchoolNet_station.weather_type = 0;
    txSchoolNet_station.lat = _params->station_info.latitude;
    txSchoolNet_station.lon = _params->station_info.longitude;
    txSchoolNet_station.alt = _params->station_info.altitude;

    txSchoolNet_station.temp = _convertDouble(_tokens[AIR_TEMP_DEGF_TOKEN]);
    if (txSchoolNet_station.temp != STATION_NAN)
      txSchoolNet_station.temp = TEMP_F_TO_C(txSchoolNet_station.temp);

    txSchoolNet_station.dew_point = _convertDouble(_tokens[DEWPOINT_DEGF_TOKEN]);
    if (txSchoolNet_station.dew_point != STATION_NAN)
      txSchoolNet_station.dew_point = TEMP_F_TO_C(txSchoolNet_station.dew_point);

    txSchoolNet_station.relhum = _convertDouble(_tokens[PERCENT_REL_HUM_TOKEN]);

    txSchoolNet_station.windspd = _convertDouble(_tokens[MPH_WIND_SPEED_TOKEN]);

    //Convert from MPH to MS
    if (txSchoolNet_station.windspd != STATION_NAN)
      txSchoolNet_station.windspd *= MPH_TO_MS;

    //Convert 16-point compass to degrees
    //directionTable[] is zero-based; hence the extra zero before the zero corresponding to north

    txSchoolNet_station.winddir = directionTable[atoi(_tokens[WIND_DIRECTION_TOKEN])];

    txSchoolNet_station.windgust = _convertDouble(_tokens[PEAK_MPH_WIND_SPEED_TOKEN]);

    //Convert from MPH to MS
    if (txSchoolNet_station.windgust != STATION_NAN)
      txSchoolNet_station.windgust *= MPH_TO_MS;

    txSchoolNet_station.pres = _convertDouble(_tokens[SEA_LEVEL_PRESS_INC_MERC_TOKEN]);

    //Convert pressure from inches of mercury to millibars
    if (txSchoolNet_station.pres != STATION_NAN)
      txSchoolNet_station.pres = txSchoolNet_station.pres * 33.864;

    txSchoolNet_station.liquid_accum = _convertDouble(_tokens[INCHES_PRECIP_TOKEN]);

    //Convert inches to mm
    if (txSchoolNet_station.liquid_accum != STATION_NAN)
      txSchoolNet_station.liquid_accum *= INCHES_TO_MM;

    txSchoolNet_station.precip_rate = _convertDouble(_tokens[RAINRATE_INC_HR_TOKEN]);
    //Convert inches to mm
    if (txSchoolNet_station.precip_rate != STATION_NAN)
      txSchoolNet_station.precip_rate *= INCHES_TO_MM;

    txSchoolNet_station.visibility = STATION_NAN; 
    txSchoolNet_station.rvr = STATION_NAN;
    txSchoolNet_station.ceiling = STATION_NAN;
    txSchoolNet_station.shared.station.liquid_accum2 = STATION_NAN;

    //    txSchoolNet_station.shared.station.Spare1 = STATION_NAN;
    txSchoolNet_station.shared.station.Spare1 = _convertDouble(_tokens[PRESS_TEND_INC_MERC_TOKEN]);

    //Convert from inches of mercury to millibars
    if (txSchoolNet_station.shared.station.Spare1 != STATION_NAN)
      txSchoolNet_station.shared.station.Spare1 = txSchoolNet_station.shared.station.Spare1 * 33.864;

    txSchoolNet_station.shared.station.Spare2 = STATION_NAN;

    STRcopy(txSchoolNet_station.station_label, _params->station_info.station_id,
	    ST_LABEL_SIZE);
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      print_station_report(stderr, "", &txSchoolNet_station);

    if (!_writeToDatabase(txSchoolNet_station))
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

bool TXSchoolnet2Spdb::_writeToDatabase(station_report_t& station)
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

