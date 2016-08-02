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
//   $Id: ihopNcarIssCdf2Spdb.cc,v 1.3 2016/03/07 01:23:09 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ihopNcarIssCdf2Spdb: ihopNcarIssCdf2Spdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * August 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <toolsa/os_config.h>

//#include <netcdf/netcdfcpp.h>
#include <netcdfcpp.h>
#include <rapformats/station_reports.h>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/file_io.h>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "InputVariable.hh"
#include "MultiplierConverter.hh"
#include "KtoCTempConverter.hh"
#include "ihopNcarIssCdf2Spdb.hh"
#include "Params.hh"


// Global variables

ihopNcarIssCdf2Spdb *ihopNcarIssCdf2Spdb::_instance = (ihopNcarIssCdf2Spdb *)NULL;


/*********************************************************************
 * Constructor
 */

ihopNcarIssCdf2Spdb::ihopNcarIssCdf2Spdb(int argc, char **argv)

{
  static const string method_name = "ihopNcarIssCdf2Spdb::ihopNcarIssCdf2Spdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ihopNcarIssCdf2Spdb *)NULL);
  
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

ihopNcarIssCdf2Spdb::~ihopNcarIssCdf2Spdb()
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

ihopNcarIssCdf2Spdb *ihopNcarIssCdf2Spdb::Inst(int argc, char **argv)
{
  if (_instance == (ihopNcarIssCdf2Spdb *)NULL)
    new ihopNcarIssCdf2Spdb(argc, argv);
  
  return(_instance);
}

ihopNcarIssCdf2Spdb *ihopNcarIssCdf2Spdb::Inst()
{
  assert(_instance != (ihopNcarIssCdf2Spdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ihopNcarIssCdf2Spdb::init()
{
  static const string method_name = "ihopNcarIssCdf2Spdb::init()";

  //Because netCDF files may use different variable names, these variable names
  //must be passed in via the _params
  //use ncdump on the netCDF file to see the variable names

  //call _initInputVariable on each one

  _initInputVariable(_inputVar_pres, _params->pressure);
  _initInputVariable(_inputVar_tdry, _params->surfaceTemperature);
  _initInputVariable(_inputVar_rh, _params->relativeHumidity);
  _initInputVariable(_inputVar_wspd, _params->windSpeed);
  _initInputVariable(_inputVar_wdir, _params->windDirection);
  _initInputVariable(_inputVar_lat, _params->northLatitude);
  _initInputVariable(_inputVar_lon, _params->eastLongitude);
  _initInputVariable(_inputVar_alt, _params->altitude);

  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ihopNcarIssCdf2Spdb::run()
{
  static const string method_name = "ihopNcarIssCdf2Spdb::run()";
  
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
      cerr << "About to process: <" << file_name << ">" << endl;
      
      _processData(file_name);
    } /* endwhile - still files to process */    
  }
}


/**********************************************************************
 *              Private Methods
 **********************************************************************/


/*********************************************************************
 * _processData() - Process data in the given file
 *
 * Returns true on success, false on failure.
 */

bool ihopNcarIssCdf2Spdb::_processData(const string& file_name)
{
  static const string method_name = "ihopNcarIssCdf2Spdb::_processData()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "*** Processing file <" << file_name << ">" << endl;

  //create a netCDF file object

  NcFile netCDFfile(file_name.c_str());

  //check to ensure the file is valid
  
  if (!netCDFfile.is_valid())
    {
      cerr << "*** NcFile object is invalid for file: " << file_name << endl;
      exit(-1);
    }

  //call the InputVariable's init() method, passing the netCDF file name
  //if unsuccessful, let the user know and exit.

  if (!_inputVar_pres.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_pres.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_tdry.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_tdry.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_rh.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_rh.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_wspd.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_wspd.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_wdir.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_wdir.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_lat.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_lat.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_lon.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_lon.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_alt.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_alt.getVariableName() << endl;
      return false;
    }

  // will use the number of time offsets as the number of data readings in the file

  NcVar *offsetVar = netCDFfile.get_var(_params->timeOffset);
  int numRecs = offsetVar->num_vals();

  for (int i = 0; i < numRecs; ++i)
    {
      //create a station_report object and fill it out

      station_report_t netCDF_station;
      netCDF_station.msg_id = STATION_REPORT;

      // add the offset to the base time to determine each reading's time

      netCDF_station.time = (netCDFfile.get_var(_params->observationTime)->as_int(0)) + (netCDFfile.get_var(_params->timeOffset)->as_int(i));
      netCDF_station.accum_start_time = 0;
      netCDF_station.weather_type = 0;

      // a given file only has one lat/lon/alt 

      netCDF_station.lat = _inputVar_lat.getValue(0);
      netCDF_station.lon = _inputVar_lon.getValue(0);
      netCDF_station.alt = _inputVar_alt.getValue(0);

      netCDF_station.temp = _inputVar_tdry.getValue(i);
      netCDF_station.dew_point = STATION_NAN;
      netCDF_station.relhum = _inputVar_rh.getValue(i);
      netCDF_station.windspd = _inputVar_wspd.getValue(i);
      netCDF_station.winddir = _inputVar_wdir.getValue(i);
      netCDF_station.windgust = STATION_NAN;
      netCDF_station.pres = _inputVar_pres.getValue(i);
      netCDF_station.liquid_accum = STATION_NAN;
      netCDF_station.precip_rate = STATION_NAN;
      netCDF_station.visibility = STATION_NAN;
      netCDF_station.rvr = STATION_NAN;
      netCDF_station.ceiling = STATION_NAN;
      netCDF_station.shared.station.liquid_accum2 = STATION_NAN;
      netCDF_station.shared.station.Spare1 = STATION_NAN;
      netCDF_station.shared.station.Spare2 = STATION_NAN;

      // no station name; set to null

      netCDF_station.station_label[0] = '\0';

      if (_params->debug >= Params::DEBUG_VERBOSE)
	{
	cerr << "Current index (i) is: <" << i << ">" << endl;
	cerr << "Total number of values is: <" << numRecs << ">" << endl << endl;
	print_station_report(stderr, "", &netCDF_station);
	}
      
      //let the user know if the station record cannot be written to the database
      if (!_writeToDatabase(netCDF_station))
      	{
      	  cerr << "ERROR: " << method_name << endl;
      	  cerr << "Error writing station record to database" << endl;
      	  cerr << "*** Skipping record" << endl;
      	  continue;
      	}
    }
  
  return true;
}


/*********************************************************************
 * _writeToDatabase() - Write the given record to the SPDB database.
 *
 * Returns true on success, false on failure.
 */

bool ihopNcarIssCdf2Spdb::_writeToDatabase(station_report_t& station)
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

/*********************************************************************
 * _initInputVariable() - initialize an input variable from the param file
 *
 * 
 */

void ihopNcarIssCdf2Spdb::_initInputVariable(InputVariable &inputVar, 
					   const Params::input_variable_t &param_input) const
{

  //set the debug level of the InputVariable object to match the ihopNcarIssCdf2Spdb one

  inputVar.setDebug(_params->debug >= Params::DEBUG_VERBOSE);

  inputVar.setVariableName(param_input.variable_name);
 
  // FIX LATER - having trouble getting this to work.
  //  inputVar.setMissingValueAttName(param_input.missing_value_attr_name);
 
  //need a Converter object to handle any necessary units conversion

  Converter *converter;

  //different Converters convert differently...

  switch (param_input.convert_type)
    {
    case Params::MULTIPLIER:
      converter = new MultiplierConverter(param_input.multiplier_value);
      break;
    case Params::NO_MULTIPLIER:
      converter = new Converter();
      break;
    case Params::KELVIN_TO_C:
      converter = new KtoCTempConverter();
      break;
    }

  inputVar.setConverter(converter);

}

