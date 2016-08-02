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
 * @file BuoyObs2Spdb.cc
 *
 * @class BuoyObs2Spdb
 *
 * BuoyObs2Spdb program object.
 *  
 * @date 7/25/2011
 *
 */

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <toolsa/os_config.h>
#include <dsdata/DsFileListTrigger.hh>
#include <dsdata/DsInputDirTrigger.hh>
#include <dsdata/DsLdataTrigger.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "BuoyObs2Spdb.hh"

using namespace std;

// Global variables

BuoyObs2Spdb *BuoyObs2Spdb::_instance =
     (BuoyObs2Spdb *)NULL;

const size_t BuoyObs2Spdb::INPUT_BUFFER_SIZE = 80;
const size_t BuoyObs2Spdb::BUOY_OBS_RECORD_LEN = 38;  // include CR


/*********************************************************************
 * Constructors
 */

BuoyObs2Spdb::BuoyObs2Spdb(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "BuoyObs2Spdb::BuoyObs2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (BuoyObs2Spdb *)NULL);
  
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
  char *params_path = new char[strlen("unknown") + 1];
  strcpy(params_path, "unknown");
  
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

BuoyObs2Spdb::~BuoyObs2Spdb()
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
 * Inst()
 */

BuoyObs2Spdb *BuoyObs2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (BuoyObs2Spdb *)NULL)
    new BuoyObs2Spdb(argc, argv);
  
  return(_instance);
}

BuoyObs2Spdb *BuoyObs2Spdb::Inst()
{
  assert(_instance != (BuoyObs2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init()
 */

bool BuoyObs2Spdb::init()
{
  static const string method_name = "BuoyObs2Spdb::init()";
  
  // If we are in verbose mode, also set the debug flag

  if (_params->verbose)
    _params->debug = pTRUE;
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run()
 */

void BuoyObs2Spdb::run()
{
  static const string method_name = "BuoyObs2Spdb::run()";
  
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

  // If requested, create the map file of the buoy locations

  if (_params->create_map_file)
    _createBuoyLocationsMapFile();
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _createBuoyLocationsMapFile()
 */

void BuoyObs2Spdb::_createBuoyLocationsMapFile() const
{
  static const string method_name = "BuoyObs2Spdb;:_createBuoyLocationsMapFile()";
  
  // Print out the list of station locations

  if (_params->verbose)
  {
    cerr << "List of processed stations:" << endl;
    
    map< string, pair< double, double > >::const_iterator buoy;
    
    for (buoy = _buoyLocations.begin(); buoy != _buoyLocations.end(); ++buoy)
    {
      cerr << "   " << buoy->first;
      cerr << "   " << buoy->second.first;
      cerr << ", " << buoy->second.second << endl;
    } /* endfor - buoy */
  }
  
  // Open the map file

  FILE *map_file;
  
  if ((map_file = fopen(_params->map_file_path, "w")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening map file for writing" << endl;
    perror(_params->map_file_path);
    
    return;
  }
  
  // Write the header and icon definition

  fprintf(map_file, "MAP_NAME BUOYS\n");
  fprintf(map_file, "ICONDEF BUOY %d\n", _params->map_icon_n);

  for (int i = 0; i < _params->map_icon_n; ++i)
    fprintf(map_file, "%ld %ld\n",
	    _params->_map_icon[i].pen_x,
	    _params->_map_icon[i].pen_y);
  
  // Write the station locations

  map< string, pair< double, double > >::const_iterator buoy;
    
  for (buoy = _buoyLocations.begin(); buoy != _buoyLocations.end(); ++buoy)
  {
    fprintf(map_file, "ICON BUOY %f %f %ld %ld %s\n",
	    buoy->second.first,   // lat value
	    buoy->second.second,  // lon value
	    _params->map_label_x_offset,
	    _params->map_label_y_offset,
	    buoy->first.c_str()); // station label
  } /* endfor - buoy */

  // Close the map file

  fclose(map_file);
  
}


/*********************************************************************
 * _initTrigger()
 */

bool BuoyObs2Spdb::_initTrigger()
{
  static const string method_name = "BuoyObs2Spdb;:_initTrigger()";
  
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
 * _processFile()
 */

bool BuoyObs2Spdb::_processFile(const string &file_path)
{
  static const string method_name = "BuoyObs2Spdb::_processFile()";
  
  PMU_auto_register("Processing file...");

  if (_params->debug)
    cerr << "*** Processing file: " << file_path << endl;
  
  // Open the input file

  FILE *input_file;
  
  if ((input_file = fopen(file_path.c_str(), "r")) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error opening input file" << endl;
    perror(file_path.c_str());
    
    return false;
  }
  
  // Get the SPDB database ready

  DsSpdb spdb;
  
  spdb.setPutMode(Spdb::putModeAdd);
  
  // Read the data from the input file.

  char *buffer = new char[INPUT_BUFFER_SIZE];
  size_t line_num = 1;
  
  while (fgets(buffer, INPUT_BUFFER_SIZE, input_file) != 0)
  {
    // Initialize the station report

    station_report_t station_report;
    memset(&station_report, 0, sizeof(station_report));
    
    /// UPDATE - Move this to station_reports.c

    station_report.msg_id = PRESSURE_STATION_REPORT;
    station_report.lat = STATION_NAN;
    station_report.lon = STATION_NAN;
    station_report.alt = STATION_NAN;
    station_report.temp = STATION_NAN;
    station_report.dew_point = STATION_NAN;
    station_report.relhum = STATION_NAN;
    station_report.windspd = STATION_NAN;
    station_report.winddir = STATION_NAN;
    station_report.windgust = STATION_NAN;
    station_report.pres = STATION_NAN;
    station_report.liquid_accum = STATION_NAN;
    station_report.precip_rate = STATION_NAN;
    station_report.visibility = STATION_NAN;
    station_report.rvr = STATION_NAN;
    station_report.ceiling = STATION_NAN;
    
    // We just read the first line of the observation.  Process it.

    int num_records;
    
    if ((num_records = _processObsLine1(buffer, line_num, station_report)) < 0)
    {
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
    
    if (num_records < 4 || num_records > 6)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Invalid number of records in observation" << endl;
      cerr << "Observation has " << num_records << " records" << endl;
      cerr << "Should have 4-6 records" << endl;
      
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
    
    ++line_num;
    
    // Read and process the second line of the observation.

    if (fgets(buffer, INPUT_BUFFER_SIZE, input_file) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading 2nd line of observation from input file" << endl;
      
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
      
    if (!_processObsLine2(buffer, line_num, station_report))
    {
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
    
    ++line_num;
    
    // Read and process the third line of the observation.

    if (fgets(buffer, INPUT_BUFFER_SIZE, input_file) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading 3rd line of observation from input file" << endl;
      
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
      
    if (!_processObsLine3(buffer, line_num, station_report))
    {
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
    
    ++line_num;
    
    // Read and process the fourth line of the observation.

    if (fgets(buffer, INPUT_BUFFER_SIZE, input_file) == 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error reading 4th line of observation from input file" << endl;
      
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
      
    if (!_processObsLine4(buffer, line_num, station_report))
    {
      delete [] buffer;
      fclose(input_file);
      
      return false;
    }
    
    ++line_num;
    
    // Read and skip any extra lines in the observation.  We don't use any
    // information from the optional lines.

    for (int i = 0; i < num_records - 4; ++i)
    {
      if (fgets(buffer, INPUT_BUFFER_SIZE, input_file) == 0)
      {
	cerr << "ERROR: " << method_name << endl;
	cerr << "Error reading optional line of observation from input file"
	     << endl;
      
	delete [] buffer;
	fclose(input_file);
      
	return false;
      }
      
      ++line_num;
    }
    
    // Add the buoy to our list of buoy locations.  Since this is a map,
    // if the buoy already exists it will just be overwritten.

    _buoyLocations[station_report.station_label] =
      pair< double, double >(station_report.lat, station_report.lon);
    
    // Add the report to the put buffer

    int data_type = Spdb::hash4CharsToInt32(station_report.station_label);
    
    if (_params->verbose)
    {
      cerr << endl;
      print_station_report(stderr, "", &station_report);
    }
    
    time_t report_time = station_report.time;
    
    station_report_to_be(&station_report);
    
    spdb.addPutChunk(data_type,
		     report_time,
		     report_time + _params->expire_secs,
		     sizeof(station_report_t),
		     &station_report);
    
  } /* endwhile - fgets */

  delete [] buffer;
  fclose(input_file);

  // Write all of the read station reports to the database

  if (spdb.put(_params->output_url,
	       SPDB_STATION_REPORT_ID, SPDB_STATION_REPORT_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing station reports to database" << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  if (_params->debug)
    cerr << endl << endl;
  
  return true;
}


/*********************************************************************
 * _processObsLine1()
 */

int BuoyObs2Spdb::_processObsLine1(const string &buffer,
				   const size_t line_num,
				   station_report_t &report) const
{
  static const string method_name = "BuoyObs2Spdb::_processObsLine1()";
  
  if (_params->verbose)
    cerr << "---> Processing line " << line_num << ": " << buffer;
  
  if (buffer.size() != BUOY_OBS_RECORD_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected " << BUOY_OBS_RECORD_LEN << " bytes" << endl;
    cerr << "Found " << buffer.size() << " bytes" << endl;
      
    return -1;
  }
    
  ///// Extract and process each of the characters in the line.  The pieces
  ///// of information that we aren't using are commented out for speed, but
  ///// left in if ever needed for debugging or data purposes in the future.

  // Identification flag - 1 char

  string id_flag = buffer.substr(0, 1);
  
  if (id_flag != "*")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "First observation line doesn't begin with '*' as expected" << endl;
    
    return -1;
  }
  
  if (_params->verbose)
    cerr << "    id_flag = " << id_flag << endl;
  
  // Data-source index - 2 chars

  string data_source_index = buffer.substr(1, 2);
  
  if (data_source_index != "81")
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Data-source index should be '81'" << endl;
    cerr << "Got " << data_source_index << " instead" << endl;
    
    return -1;
  }
  
  if (_params->verbose)
    cerr << "    data_source_index = " << data_source_index << endl;
  
  // Drifting buoy no - 5 chars

  string dribkey_string = buffer.substr(3, 5);
  
  if (_params->verbose)
    cerr << "    dribkey_string = " << dribkey_string << endl;
  
  STRcopy(report.station_label, dribkey_string.c_str(), ST_LABEL_SIZE);
  
//  // Index for wind - 2 chars
//
//  string windtr_string = buffer.substr(8, 2);
//  
//  if (_params->verbose)
//    cerr << "    windtr_string = " << windtr_string << endl;
//  
//  // Not used - 2 chars
//
//  string not_used = buffer.substr(10, 2);
//  
//  if (_params->verbose)
//    cerr << "    not_used = " << not_used << endl;
//  
  // Latitude (degree*100<-9000~9000>) - 5 chars

  string la_string = buffer.substr(12, 5);
  report.lat = (double)atoi(la_string.c_str()) / 100.0;
  
  if (_params->verbose)
  {
    cerr << "    la_string = " << la_string << endl;
    cerr << "    lat = " << report.lat << endl;
  }
  
  // Longitude (degree*100<0~3600>) - 5 chars

  string lo_string = buffer.substr(17, 5);
  report.lon = (double)atoi(lo_string.c_str()) / 100.0;
  
  if (_params->verbose)
  {
    cerr << "    lo_string = " << lo_string << endl;
    cerr << "    lon = " << report.lon << endl;
  }
  
//  // Levels no. under sea surface - 2 chars
//
//  string its_icu_string = buffer.substr(22, 2);
//  
//  if (_params->verbose)
//    cerr << "    its_icu_string = " << its_icu_string << endl;
//  
  // Year - 2 chars

  string gyr_string = buffer.substr(24, 2);
  int year = atoi(gyr_string.c_str()) + 2000;
  
  if (_params->verbose)
    cerr << "    gyr_string = " << gyr_string << endl;
  
  // Month - 2 chars

  string gmo_string = buffer.substr(26, 2);
  int month = atoi(gmo_string.c_str());
  
  if (_params->verbose)
    cerr << "    gmo_string = " << gmo_string << endl;
  
  // Day - 2 chars

  string gda_string = buffer.substr(28, 2);
  int day = atoi(gda_string.c_str());
  
  if (_params->verbose)
    cerr << "    gda_string = " << gda_string << endl;
  
  // Hour - 2 chars

  string ghr_string = buffer.substr(30, 2);
  int hour = atoi(ghr_string.c_str());
  
  if (_params->verbose)
    cerr << "    ghr_string = " << ghr_string << endl;
  
  // Minute - 2 chars

  string gmn_string = buffer.substr(32, 2);
  int minute = atoi(gmn_string.c_str());
  
  if (_params->verbose)
    cerr << "    gmn_string = " << gmn_string << endl;
  
  // Calculate the UNIX time for the data time

  DateTime data_time(year, month, day, hour, minute);
  
  if (_params->verbose)
    cerr << "    data_time = " << data_time << endl;
  
  report.time = data_time.utime();
  
  // Total logical record number - 3 chars

  string rno_string = buffer.substr(34, 3);
  int rec_num = atoi(rno_string.c_str());
  
  if (_params->verbose)
    cerr << "    rec_num = " << rec_num << endl;
  
  return rec_num;
}


/*********************************************************************
 * _processObsLine2()
 */

bool BuoyObs2Spdb::_processObsLine2(const string &buffer,
				    const size_t line_num,
				    station_report_t &report) const
{
  static const string method_name = "BuoyObs2Spdb::_processObsLine2()";
  
  if (_params->verbose)
    cerr << "---> Processing line " << line_num << ": " << buffer;
  
  if (buffer.size() != BUOY_OBS_RECORD_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected " << BUOY_OBS_RECORD_LEN << " bytes" << endl;
    cerr << "Found " << buffer.size() << " bytes" << endl;
      
    return -1;
  }
    
  ///// Extract and process each of the characters in the line.  The pieces
  ///// of information that we aren't using are commented out for speed, but
  ///// left in if ever needed for debugging or data purposes in the future.

  // Wind direction (degree) - 3 chars

  string ddsuf_string = buffer.substr(0, 3);
  double wind_dir = (double)atoi(ddsuf_string.c_str());
  
  if (_params->verbose)
    cerr << "    ddsuf_string = " << ddsuf_string << endl;
  
  // Wind speed (meter/sec) - 3 chars

  string ffsuf_string = buffer.substr(3, 3);
  double wind_speed = (double)atoi(ffsuf_string.c_str());
  
  if (_params->verbose)
    cerr << "    ffsuf_string = " << ffsuf_string << endl;
  
  // QC indicator of wind - 1 char

  string qwind_string = buffer.substr(6, 1);
  int wind_qc = atoi(qwind_string.c_str());
  
  if (_params->verbose)
    cerr << "    qwind_string = " << qwind_string << endl;
  
  if (wind_qc == QC_NO_CHECK || wind_qc == QC_CORRECT)
  {
    report.windspd = wind_speed;
    report.winddir = wind_dir;
  }
  
  // Air temperature (degree c * 10) - 4 chars

  string ttair_string = buffer.substr(7, 4);
  double temp = (double)atoi(ttair_string.c_str()) / 10.0;
  
  if (_params->verbose)
    cerr << "    ttair_string = " << ttair_string << endl;
  
  // QC indicator of air temperature - 1 char

  string qairt_string = buffer.substr(11, 1);
  int temp_qc = atoi(qairt_string.c_str());
  
  if (_params->verbose)
    cerr << "    qairt_string = " << qairt_string << endl;
  
  if (temp_qc == QC_NO_CHECK || temp_qc == QC_CORRECT)
    report.temp = temp;
  
  // Average SLP (mb * 10) - 5 chars

  string ppsuf_string = buffer.substr(12, 5);
  double slp = (double)atoi(ppsuf_string.c_str()) / 10.0;
  
  if (_params->verbose)
    cerr << "    ppsuf_string = " << ppsuf_string << endl;
  
  // QC indicator of average SLP - 1 char

  string qpres_string = buffer.substr(17, 1);
  int slp_qc = atoi(qpres_string.c_str());
  
  if (_params->verbose)
    cerr << "    qpres_string = " << qpres_string << endl;
  
  if (slp_qc == QC_NO_CHECK || slp_qc == QC_CORRECT)
    report.pres = slp;
  
//  // 3-hr pressure tendency (code Table 0200) - 1 char
//
//  string ptend_string = buffer.substr(18, 1);
//  
//  if (_params->verbose)
//    cerr << "    ptend_string = " << ptend_string << endl;
//  
//  // 3-hr pressure change (mb * 10) - 3 chars
//
//  string press_string = buffer.substr(19, 3);
//  
//  if (_params->verbose)
//    cerr << "    press_string = " << press_string << endl;
//  
//  // Quality of time consistency - 1 char
//
//  string time_consist_qc_string = buffer.substr(22, 1);
//  
//  if (_params->verbose)
//    cerr << "    time_consist_qc_string = " << time_consist_qc_string << endl;
//  
//  // Quality of internal consistency - 1 char
//
//  string internal_consist_qc_string = buffer.substr(23, 1);
//  
//  if (_params->verbose)
//    cerr << "    internal_consis_qc_string = "
//	 << internal_consist_qc_string << endl;
//  
//  // Quality of climate limits - 1 char
//
//  string climate_limits_qc_string = buffer.substr(24, 1);
//  
//  if (_params->verbose)
//    cerr << "    climate_limits_qc_string = "
//	 << climate_limits_qc_string << endl;
//  
//  // Quality of physical limits - 1 char
//
//  string physical_limits_qc_string = buffer.substr(25, 1);
//  
//  if (_params->verbose)
//    cerr << "    physical_limits_qc_string = "
//	 << physical_limits_qc_string << endl;
//  
//  // Quality of horizontal consistency - 1 char
//
//  string horiz_consist_qc_string = buffer.substr(26, 1);
//  
//  if (_params->verbose)
//    cerr << "    horiz_consist_qc_string = " << horiz_consist_qc_string << endl;
//  
//  // Indicator of barometric pressure error - 2 chars
//
//  string pressure_error = buffer.substr(27, 2);
//  
//  if (_params->verbose)
//    cerr << "    pressure_error = " << pressure_error << endl;
//  
//  // Indicator of sea surface temperature error - 2 chars
//
//  string sst_error = buffer.substr(29, 2);
//  
//  if (_params->verbose)
//    cerr << "    sst_error = " << sst_error << endl;
//  
//  // Not used - 2 chars
//
//  string not_used = buffer.substr(31, 2);
//  
//  if (_params->verbose)
//    cerr << "    not_used = " << not_used << endl;
//  
  // The second part of the data - 1 char

  string part_num_string = buffer.substr(33, 1);
  int part_num = atoi(part_num_string.c_str());
  
  if (_params->verbose)
    cerr << "    part_num_string = " << part_num_string << endl;
  
  if (part_num != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected part number 2" << endl;
    cerr << "Found part number " << part_num_string << endl;
    
    return false;
  }
  
  // Logical record number - 3 chars

  string rno_string = buffer.substr(34, 3);
  int record_num = atoi(rno_string.c_str());
  
  if (_params->verbose)
    cerr << "    rno_string = " << rno_string << endl;
  
  if (record_num != 2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected record number 2" << endl;
    cerr << "Found record number " << rno_string << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _processObsLine3()
 */

bool BuoyObs2Spdb::_processObsLine3(const string &buffer,
				    const size_t line_num,
				    station_report_t &report) const
{
  static const string method_name = "BuoyObs2Spdb::_processObsLine3()";
  
  if (_params->verbose)
    cerr << "---> Processing line " << line_num << ": " << buffer;
  
  if (buffer.size() != BUOY_OBS_RECORD_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected " << BUOY_OBS_RECORD_LEN << " bytes" << endl;
    cerr << "Found " << buffer.size() << " bytes" << endl;
      
    return -1;
  }
    
  ///// Extract and process each of the characters in the line.  The pieces
  ///// of information that we aren't using are commented out for speed, but
  ///// left in if ever needed for debugging or data purposes in the future.

//  // SST (degree c * 10) - 4 chars
//
//  string ttsuf_string = buffer.substr(0, 4);
//  
//  if (_params->verbose)
//    cerr << "    ttsuf_string = " << ttsuf_string << endl;
//  
//  // QC indicator of SST - 1 char
//
//  string qseat_string = buffer.substr(4, 1);
//  
//  if (_params->verbose)
//    cerr << "    qseat_string = " << qseat_string << endl;
//  
//  // Wave period (sec * 10) - 3 chars
//
//  string wperl_string = buffer.substr(5, 3);
//  
//  if (_params->verbose)
//    cerr << "    wperl_string = " << wperl_string << endl;
//  
//  // QC indicator of wave period - 1 char
//
//  string qwper_string = buffer.substr(8, 1);
//  
//  if (_params->verbose)
//    cerr << "    qwper_string = " << qwper_string << endl;
//  
//  // Wave height (meter * 10) - 3 chars
//
//  string whigl_string = buffer.substr(9, 3);
//  
//  if (_params->verbose)
//    cerr << "    whigl_string = " << whigl_string << endl;
//  
//  // QC indicator of wave height - 1 char
//
//  string qwhig_string = buffer.substr(12, 1);
//  
//  if (_params->verbose)
//    cerr << "    qwhig_string = " << qwhig_string << endl;
//  
//  // Method to measure the salinity and depth - 1 char
//
//  string smeth_string = buffer.substr(13, 1);
//  
//  if (_params->verbose)
//    cerr << "    smeth_string = " << smeth_string << endl;
//  
//  // Method to remove ship speed - 1 char
//
//  string mrmsh_string = buffer.substr(14, 1);
//  
//  if (_params->verbose)
//    cerr << "    mrmsh_string = " << mrmsh_string << endl;
//  
//  // Duration and time of current measurement (code table 2264) - 2 chars
//
//  string dtime_string = buffer.substr(15, 2);
//  
//  if (_params->verbose)
//    cerr << "    dtime_string = " << dtime_string << endl;
//  
//  // Levels no. of measuring temperature and salinity under sea surface -
//  // 2 chars
//
//  string its_string = buffer.substr(17, 2);
//  
//  if (_params->verbose)
//    cerr << "    its_string = " << its_string << endl;
//  
//  // Levels no. of measuring current under sea surface - 2 chars
//
//  string icu_string = buffer.substr(19, 2);
//  
//  if (_params->verbose)
//    cerr << "    icu_string = " << icu_string << endl;
//  
  // Pressure at station level (mb * 10) - 5 chars

  string ppstl_string = buffer.substr(21, 5);
  double station_press = (double)atoi(ppstl_string.c_str()) / 10.0;
  
  if (_params->verbose)
    cerr << "    ppstl_string = " << ppstl_string << endl;
  
  // QC indicatory of ppstl - 1 char

  string qpsl_string = buffer.substr(26, 1);
  int station_press_qc = atoi(qpsl_string.c_str());
  
  if (_params->verbose)
    cerr << "    qpsl_string = " << qpsl_string << endl;
  
  if (station_press_qc == QC_NO_CHECK || station_press_qc == QC_CORRECT)
    report.shared.pressure_station.stn_pres = station_press;
  
  // Dew-point temperature (degree c * 10) - 5 chars

  string dewpt_string = buffer.substr(27, 5);
  double dewpt = (double)atoi(dewpt_string.c_str()) / 10.0;
  
  if (_params->verbose)
    cerr << "    dewpt_string = " << dewpt_string << endl;
  
  // QC indicator of tdew - 1 char

  string dewpt_qc_string = buffer.substr(32, 1);
  int dewpt_qc = atoi(dewpt_qc_string.c_str());
  
  if (_params->verbose)
    cerr << "    dewpt_qc_string = " << dewpt_qc_string << endl;
  
  if (dewpt_qc == QC_NO_CHECK || dewpt_qc == QC_CORRECT)
    report.dew_point = dewpt;
  
  // The third part of the data - 1 char

  string part_num_string = buffer.substr(33, 1);
  int part_num = atoi(part_num_string.c_str());
  
  if (_params->verbose)
    cerr << "    part_num_string = " << part_num_string << endl;
  
  if (part_num != 3)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected part number 3" << endl;
    cerr << "Found part number " << part_num_string << endl;
    
    return false;
  }
  
  // Logical record number - 3 chars

  string record_num_string = buffer.substr(34, 3);
  int record_num = atoi(record_num_string.c_str());
  
  if (_params->verbose)
    cerr << "    record_num_string = " << record_num_string << endl;
  
  if (record_num != 3)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected record number 3" << endl;
    cerr << "Found record number " << record_num_string << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _processObsLine4()
 */

bool BuoyObs2Spdb::_processObsLine4(const string &buffer,
				    const size_t line_num,
				    station_report_t &report) const
{
  static const string method_name = "BuoyObs2Spdb::_processObsLine4()";
  
  if (_params->verbose)
    cerr << "---> Processing line " << line_num << ": " << buffer;
  
  if (buffer.size() != BUOY_OBS_RECORD_LEN)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected " << BUOY_OBS_RECORD_LEN << " bytes" << endl;
    cerr << "Found " << buffer.size() << " bytes" << endl;
      
    return -1;
  }
    
  ///// Extract and process each of the characters in the line.  The pieces
  ///// of information that we aren't using are commented out for speed, but
  ///// left in if ever needed for debugging or data purposes in the future.

//  // Quality of pressure measuring - 1 char
//
//  string quapr_string = buffer.substr(0, 1);
//  
//  if (_params->verbose)
//    cerr << "    quapr_string = " << quapr_string << endl;
//  
//  // Quality of technical parameters - 1 char
//
//  string quaho_string = buffer.substr(1, 1);
//  
//  if (_params->verbose)
//    cerr << "    quaho_string = " << quaho_string << endl;
//  
//  // Quality of SST - 1 char
//
//  string quatw_string = buffer.substr(2, 1);
//  
//  if (_params->verbose)
//    cerr << "    quatw_string = " << quatw_string << endl;
//  
//  // Quality of air temperature - 1 char
//
//  string quaat_string = buffer.substr(3, 1);
//  
//  if (_params->verbose)
//    cerr << "    quaat_string = " << quaat_string << endl;
//  
//  // Quality of satellite transmission - 1 char
//
//  string quats_string = buffer.substr(4, 1);
//  
//  if (_params->verbose)
//    cerr << "    quats_string = " << quats_string << endl;
//  
//  // Quality of location - 1 char
//
//  string qualo_string = buffer.substr(5, 1);
//  
//  if (_params->verbose)
//    cerr << "    qualo_string = " << qualo_string << endl;
//  
//  // Second possible latitude (deg*100<-9000~9000>) - 5 char
//
//  string la2_string = buffer.substr(6, 5);
//  
//  if (_params->verbose)
//    cerr << "    la2_string = " << la2_string << endl;
//  
//  // Second possible longitude (degree*100<0~36000>) - 5 chars
//
//  string lo2_string = buffer.substr(11, 5);
//  
//  if (_params->verbose)
//    cerr << "    lo2_string = " << lo2_string << endl;
//  
//  // Location quality class (code table 3302) - 1 char
//
//  string quacl_string = buffer.substr(16, 1);
//  
//  if (_params->verbose)
//    cerr << "    quacl_string = " << quacl_string << endl;
//  
//  // Drift speed (cm/sec) - 2 chars
//
//  string drspd_string = buffer.substr(17, 2);
//  
//  if (_params->verbose)
//    cerr << "    drspd_string = " << drspd_string << endl;
//  
//  // Drift direction (degree) - 3 chars
//
//  string drdir_string = buffer.substr(19, 3);
//  
//  if (_params->verbose)
//    cerr << "    drdir_string = " << drdir_string << endl;
//  
//  // Engineering state - 4 chars
//
//  string ensta_string = buffer.substr(22, 4);
//  
//  if (_params->verbose)
//    cerr << "    ensta_string = " << ensta_string << endl;
//  
//  // Drogue type - 1 char
//
//  string drtyp_string = buffer.substr(26, 1);
//  
//  if (_params->verbose)
//    cerr << "    drtyp_string = " << drtyp_string << endl;
//  
//  // Cable length (meter) - 3 chars
//
//  string cable_string = buffer.substr(27, 3);
//  
//  if (_params->verbose)
//    cerr << "    cable_string = " << cable_string << endl;
//  
  // Relative humidity of the air - 3 chars

  string rhuma_string = buffer.substr(30, 3);
  double rel_hum = (double)atoi(rhuma_string.c_str());
  
  if (_params->verbose)
    cerr << "    rhuma_string = " << rhuma_string << endl;
  
  if (rel_hum >= 0.0)
    report.relhum = rel_hum;
  
  // The fourth part of the data - 1 char

  string part_num_string = buffer.substr(33, 1);
  int part_num = atoi(part_num_string.c_str());
  
  if (_params->verbose)
    cerr << "    part_num_string = " << part_num_string << endl;
  
  if (part_num != 4)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected part number 4" << endl;
    cerr << "Found part number " << part_num_string << endl;
    
    return false;
  }
  
  // Logical record number - 3 chars

  string record_num_string = buffer.substr(34, 3);
  int record_num = atoi(record_num_string.c_str());
  
  if (_params->verbose)
    cerr << "    record_num_string = " << record_num_string << endl;
  
  if (record_num != 4)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error processing line " << line_num << " of input file" << endl;
    cerr << "Expected record number 4" << endl;
    cerr << "Found record number " << record_num_string << endl;
    
    return false;
  }
  
  return true;
}
