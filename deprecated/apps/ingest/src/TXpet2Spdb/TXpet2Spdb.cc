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
//   $Date: 2016/03/07 01:23:06 $
//   $Id: TXpet2Spdb.cc,v 1.8 2016/03/07 01:23:06 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TXpet2Spdb: TXpet2Spdb program object.
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

#include "TXpet2Spdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

TXpet2Spdb *TXpet2Spdb::_instance = (TXpet2Spdb *)NULL;

const int TXpet2Spdb::MAX_TOKENS = 25;
const int TXpet2Spdb::MAX_TOKEN_LEN = 100;
const double TXpet2Spdb::UNAVAIL = -9999.0;
const double TXpet2Spdb::UNAVAILABLE = -99.900;


/*********************************************************************
 * Constructor
 */

TXpet2Spdb::TXpet2Spdb(int argc, char **argv) :
  _tokens(0)
{
  static const string method_name = "TXpet2Spdb::TXpet2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TXpet2Spdb *)NULL);
  
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

TXpet2Spdb::~TXpet2Spdb()
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

TXpet2Spdb *TXpet2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (TXpet2Spdb *)NULL)
    new TXpet2Spdb(argc, argv);
  
  return(_instance);
}

TXpet2Spdb *TXpet2Spdb::Inst()
{
  assert(_instance != (TXpet2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TXpet2Spdb::init()
{
  static const string method_name = "TXpet2Spdb::init()";
  
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

void TXpet2Spdb::run()
{
  static const string method_name = "TXpet2Spdb::run()";
  
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

double TXpet2Spdb::_convertDouble(const char* token)
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

bool TXpet2Spdb::_processData(const string& file_name,
			     int& line_num)
{
  static const string method_name = "TXpet2Spdb::_processData()";
  
  static const int INPUT_LINE_LEN = 1024;

  char input_line[INPUT_LINE_LEN];
  
  if (_params->debug >= Params::DEBUG_VERBOSE)
    cerr << "*** Processing file <" << file_name << "> starting at line "
	 << line_num << endl;

  // Open the input file

  cerr << "opening input file..." << endl;

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
  station_report_t tx_net_station;

  while (line_num < _params->num_header_lines)
  {
    fgets(input_line, INPUT_LINE_LEN, input_file);

    //extract the station location information from the 0th header line, beginning with the word 'Long'

    if (line_num == 0)
      {
	char *substring_begin;
	if ((substring_begin = strstr(input_line, "Long")) == NULL)
	  {
	    cerr << "Unable to find beginning of station information in file." << endl;
	    return false;
	  }

	int lonDeg, lonMin, latDeg, latMin, numToks;

	if ((numToks = sscanf(substring_begin, "Long %d deg %d min  Lat %d"
			      " deg %d min", &lonDeg, &lonMin, &latDeg, &latMin)) !=4)
	  {
	    cerr << "Unable to extract station location information from file." << endl;
	    cerr << "numToks: " << numToks << " lonDeg: " << lonDeg << ", lonMin: "
		 << lonMin << ", latDeg: " << latDeg << ", latMin: " << latMin << endl;
	    return false;
	  }
      	
	tx_net_station.lat = (double)latDeg + (double)latMin/60.0;
	tx_net_station.lon = -((double)lonDeg + (double)lonMin/60.0);
	
        if (_params->debug >= Params::DEBUG_VERBOSE)
          {
	    cerr << "numToks: " << numToks << " lonDeg: " << lonDeg << ", lonMin: "
		 << lonMin << ", latDeg: " << latDeg << ", latMin: " << latMin << endl;
	    cerr << "Station lat: " << tx_net_station.lat << ", Station lon: "
		 << tx_net_station.lon << endl;
	  }
      }
	
    //extract the date/altitude information from the 1st header line 

    if (line_num == 1)
      {
        int mon, day, year, alt;
        if (sscanf(input_line, "     Date:%d/%d/%d %*s Elev: %d", &mon, &day, &year, &alt) !=4)
	  {
	    cerr << "Unable to extract date/altitude information from file." << endl;
	    return false;
	  }

	report_time.set(year, mon, day);
	tx_net_station.alt = alt;
      }

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
    
    if ((num_tokens = STRparse(input_line, _tokens, INPUT_LINE_LEN, 
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
    
    
    tx_net_station.msg_id = STATION_REPORT;

    //set hour of DateTime object; min/secs default to zero
    // Add 6 hours to convert data from CDT to GMT
    report_time.setTime((atoi(_tokens[TIME_TOKEN])/100) + 6);

    tx_net_station.time = report_time.utime();

    tx_net_station.accum_start_time = 0;
    tx_net_station.weather_type = 0;
    tx_net_station.temp = _convertDouble(_tokens[AIR_TEMP_DEGC_TOKEN]);
    tx_net_station.dew_point = _convertDouble(_tokens[DEWPOINT_TEMP_DEGC_TOKEN]);
    tx_net_station.relhum = _convertDouble(_tokens[PERCENT_REL_HUM_TOKEN]);
    tx_net_station.windspd = _convertDouble(_tokens[WIND_SPEED_TOKEN]);
    tx_net_station.winddir = _convertDouble(_tokens[WIND_DIRECTION_TOKEN]);
    tx_net_station.windgust = STATION_NAN;
    tx_net_station.pres = _convertDouble(_tokens[BAROM_PRES_TOKEN]);
    tx_net_station.liquid_accum = _convertDouble(_tokens[PRECIP_MM_TOKEN]);
    tx_net_station.precip_rate = STATION_NAN;
    tx_net_station.visibility = STATION_NAN;
    tx_net_station.rvr = STATION_NAN;         //runway visual range km
    tx_net_station.ceiling = STATION_NAN;

    tx_net_station.shared.station.liquid_accum2 = STATION_NAN;
    tx_net_station.shared.station.Spare1 = STATION_NAN;
    tx_net_station.shared.station.Spare2 = STATION_NAN;

    //use first characters of the base part of the parsed filename, converted to lowercase

    STRcopy(tx_net_station.station_label, pathInfo.base,
	    4);
    tx_net_station.station_label[0] = tolower(tx_net_station.station_label[0]);
    tx_net_station.station_label[1] = tolower(tx_net_station.station_label[1]);
    tx_net_station.station_label[2] = tolower(tx_net_station.station_label[2]);
    
    if (_params->debug >= Params::DEBUG_VERBOSE)
      print_station_report(stderr, "", &tx_net_station);

    if (!_writeToDatabase(tx_net_station))
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

bool TXpet2Spdb::_writeToDatabase(station_report_t& station)
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
