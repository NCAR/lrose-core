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
//   $Id: Asos6401toSpdb.cc,v 1.3 2016/03/07 01:22:59 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Asos6401toSpdb: Asos6401toSpdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <rapformats/metar_decode.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/Path.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Asos6401toSpdb.hh"
#include "Params.hh"

#include "AncStationFile.hh"
#include "NwsStationFile.hh"

using namespace std;

// Global variables

Asos6401toSpdb *Asos6401toSpdb::_instance =
     (Asos6401toSpdb *)NULL;

const int Asos6401toSpdb::MAX_LINE_LEN = 1024;


/*********************************************************************
 * Constructors
 */

Asos6401toSpdb::Asos6401toSpdb(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "Asos6401toSpdb::Asos6401toSpdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Asos6401toSpdb *)NULL);
  
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

Asos6401toSpdb::~Asos6401toSpdb()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

Asos6401toSpdb *Asos6401toSpdb::Inst(int argc, char **argv)
{
  if (_instance == (Asos6401toSpdb *)NULL)
    new Asos6401toSpdb(argc, argv);
  
  return(_instance);
}

Asos6401toSpdb *Asos6401toSpdb::Inst()
{
  assert(_instance != (Asos6401toSpdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Asos6401toSpdb::init()
{
  static const string method_name = "Asos6401toSpdb::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize the station file handler

  if (!_initStationFile())
    return false;
  
  // initialize process registration

  if (_params->trigger_mode != Params::FILE_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void Asos6401toSpdb::run()
{
  static const string method_name = "Asos6401toSpdb::run()";
  
  while (!_dataTrigger->endOfData())
  {
    TriggerInfo trigger_info;
    
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger information" << endl;
      cerr << "Trying again...." << endl;
      
      continue;
    }
    
    _processFile(trigger_info.getFilePath());

  } /* endwhile - !_dataTrigger->endOfData() */

}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _decodeReport() - Decode the line from the ASOS file into a station
 *                   report.
 *
 * Return true on success, false on failure
 */

bool Asos6401toSpdb::_decodeReport(const char *line,
				   station_report_t &report) const
{
  static const string method_name = "Asos6401toSpdb;:_decodeReport()";
  
  static char tmp_line[MAX_LINE_LEN];

  // Find a pointer to the beginning of the METAR portion of the message

  char *metar_ptr = strstr(line, "5-MIN");
    
  if (metar_ptr == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Trigger string <5-MIN> not found in input line: " << line << endl;
    cerr << "Skipping line" << endl;
      
    return false;
  }
    
  metar_ptr += strlen("5-MIN") + 1;
    
  // Copy the metar portion of the line into a temporary string so that it
  // can be processed by the metar decoder.  Note that this string will be
  // corrupted by the decoded because it uses strtok() on the string.

  STRcopy(tmp_line, metar_ptr, strlen(metar_ptr));
    
  if (_params->debug)
    cerr << "    METAR: <" << tmp_line << ">" << endl;
    
  Decoded_METAR decoded_metar;
    
  if (DcdMETAR(tmp_line, &decoded_metar, true) != 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error decoding metar: <" << tmp_line << ">" << endl;
    cerr << "Skipping line" << endl;
      
    return false;
  }
    
  // Extract the time from the line

  int year =
    ((line[13] - '0') * 1000) + ((line[14] - '0') * 100) +
    ((line[15] - '0') * 10) + (line[16] - '0');
  int month = ((line[17] - '0') * 10) + (line[18] - '0');
  int day = ((line[19] - '0') * 10) + (line[20] - '0');
  int hour = ((line[21] - '0') * 10) + (line[22] - '0');
  int minute = ((line[23] - '0') * 10) + (line[24] - '0');
    
  DateTime valid_time(year, month, day, hour, minute);
    
  if (_params->debug)
    cerr << "    valid time: " << valid_time << endl;
    
  // Get the station location

  double lat;
  double lon;
  double alt;
  
  if (!_getStationLoc(decoded_metar.stnid,
		      lat, lon, alt))
    return false;
  
  // Put the metar information into the station report structure
  
  switch (_params->output_report_type)
  {
  case Params::OUTPUT_METAR_REPORT :
    if (decoded_metar_to_report(&decoded_metar, &report,
				valid_time.utime(),
				lat, lon, alt) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error putting metar into station report" << endl;
      cerr << "Skipping report" << endl;

      return false;
    }
    break;
      
  case Params::OUTPUT_STATION_REPORT :
    if (decoded_metar_to_station_report(&decoded_metar, &report,
					valid_time.utime(),
					lat, lon, alt) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error putting metar into station report" << endl;
      cerr << "Skipping report" << endl;

      return false;
    }
    break;
      
  case Params::OUTPUT_PRESSURE_STATION_REPORT :
    if (decoded_metar_to_pressure_station_report(&decoded_metar, &report,
						 valid_time.utime(),
						 lat, lon, alt) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error putting metar into station report" << endl;
      cerr << "Skipping report" << endl;

      return false;
    }
    break;
      
  case Params::OUTPUT_METAR_WITH_REMARKS_REPORT :
    if (decoded_metar_to_report_with_remarks(&decoded_metar, &report,
					     valid_time.utime(),
					     lat, lon, alt) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error putting metar into station report" << endl;
      cerr << "Skipping report" << endl;

      return false;
    }
    break;
  } /* endswitch - _params->output_report_type */

  return true;
}


/*********************************************************************
 * _getStationLoc() - Get the location for this station from the station
 *                    file.
 *
 * Returns true on success, false on failure.
 */

bool Asos6401toSpdb::_getStationLoc(const string &station_id,
				    double &lat, double &lon,
				    double &alt) const
{
  static const string method_name = "Asos6401toSpdb;:_getStationLoc()";
  
  // Get the station location from the station file

  if (!_stationFile->getStationLoc(station_id, lat, lon, alt))
    return false;
  
  return true;
}


/*********************************************************************
 * _initStationFile() - Initialize the station file handler
 *
 * Returns true on success, false on failure.
 */

bool Asos6401toSpdb::_initStationFile()
{
  static const string method_name = "Asos6401toSpdb;:_initStationFile()";
  
  switch (_params->station_file_type)
  {
  case Params::STATION_FILE_ANC :
  {
    AncStationFile *station_file = new AncStationFile();
    
    bool init_status;
    
    switch (_params->stn_file_alt_units)
    {
    case Params::ALT_UNITS_METERS :
      init_status = station_file->init(AncStationFile::ALT_UNITS_METERS);
      break;
    case Params::ALT_UNITS_FEET :
      init_status = station_file->init(AncStationFile::ALT_UNITS_FEET);
      break;
    }
    
    if (!init_status)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing ANC station file handler" << endl;
      
      return false;
    }
    
    _stationFile = station_file;
    
    break;
  }
  
  case Params::STATION_FILE_NWS :
  {
    NwsStationFile *station_file = new NwsStationFile();
    
    _stationFile = station_file;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  // Read in the station information

  if (!_stationFile->readFile(_params->station_file))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading stations from station file: "
	 << _params->station_file << endl;
    
    return false;
  }
  
  return true;
}

    
/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 */

bool Asos6401toSpdb::_initTrigger()
{
  static const string method_name = "Asos6401toSpdb;:_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::FILE_LIST :
  {
    DsFileListTrigger *trigger = new DsFileListTrigger();
    if (trigger->init(_args->getFileList()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing FILE_LIST trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, false,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::INPUT_DIR_RECURSE :
  {
    DsInputDirTrigger *trigger = new DsInputDirTrigger();
    if (trigger->init(_params->input_dir,
		      _params->input_substring,
		      false, PMU_auto_register, true,
		      _params->exclude_substring) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing INPUT_DIR_RECURSE trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  case Params::LATEST_DATA :
  {
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_dir,
		      _params->max_valid_secs,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger" << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

return true;
}

    
/*********************************************************************
 * _processFile() - Process the given file.
 */

bool Asos6401toSpdb::_processFile(const string &input_file_path)
{
  static const string method_name = "Asos6401toSpdb::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << endl << "*** Processing file: " << input_file_path << endl;
  
  // Open the input file

  FILE *input_file;
  
  if ((input_file = fopen(input_file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file: " << input_file_path << endl;
    
    return false;
  }
  
  // Process each line in the file

  char line[MAX_LINE_LEN];
  
  DsSpdb spdb_buffer;
  
  while (fgets(line, MAX_LINE_LEN, input_file) != 0)
  {
    if (_params->debug)
      cerr << endl << "Line: <" << line << ">" << endl;
    
    // Decode the ASOS report

    station_report_t report;
    
    if (!_decodeReport(line, report))
      continue;

    if (_params->debug)
      print_station_report(stderr, "   ", &report);
    
    // Add the report to the output buffer

    int station_id = Spdb::hash4CharsToInt32(report.station_label);
    time_t valid_time = report.time;
    
    station_report_to_be(&report);

    spdb_buffer.addPutChunk(station_id,
			    valid_time,
			    valid_time + _params->expire_secs,
			    sizeof(report), &report);
    
  } /* endwhile - fgets */
  
  // Close the input file

  fclose(input_file);
  
  // Write the SPDB buffer

  if (spdb_buffer.put(_params->output_url,
		      SPDB_STATION_REPORT_ID,
		      SPDB_STATION_REPORT_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output to SPDB database: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
