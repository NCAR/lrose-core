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
//   $Date: 2016/03/07 01:23:10 $
//   $Id: netCDF2StnRptSpdb.cc,v 1.11 2016/03/07 01:23:10 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * netCDF2StnRptSpdb: netCDF2StnRptSpdb program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * April 2003
 *
 * Kay Levesque
 *
 *********************************************************************/

#include <iostream>
//#include <netcdf/netcdfcpp.h>
#include <netcdfcpp.h>
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

#include "InputVariable.hh"
#include "MultiplierConverter.hh"
#include "KtoCTempConverter.hh"
#include "netCDF2StnRptSpdb.hh"
#include "Params.hh"
using namespace std;


// Global variables

netCDF2StnRptSpdb *netCDF2StnRptSpdb::_instance = (netCDF2StnRptSpdb *)NULL;


/*********************************************************************
 * Constructor
 */

netCDF2StnRptSpdb::netCDF2StnRptSpdb(int argc, char **argv)

{
  static const string method_name = "netCDF2StnRptSpdb::netCDF2StnRptSpdb()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (netCDF2StnRptSpdb *)NULL);
  
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

netCDF2StnRptSpdb::~netCDF2StnRptSpdb()
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

netCDF2StnRptSpdb *netCDF2StnRptSpdb::Inst(int argc, char **argv)
{
  if (_instance == (netCDF2StnRptSpdb *)NULL)
    new netCDF2StnRptSpdb(argc, argv);
  
  return(_instance);
}

netCDF2StnRptSpdb *netCDF2StnRptSpdb::Inst()
{
  assert(_instance != (netCDF2StnRptSpdb *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool netCDF2StnRptSpdb::init()
{
  static const string method_name = "netCDF2StnRptSpdb::init()";

  //Because netCDF files may use different variable names, these variable names
  //must be passed in via the _params
  //use ncdump on the netCDF file to see the variable names

  //call _initInputVariable on each one

  _initInputVariable(_inputVar_lat, _params->latitude);
  _initInputVariable(_inputVar_lon, _params->longitude);
  _initInputVariable(_inputVar_alt, _params->altitude);
  _initInputVariable(_inputVar_temp, _params->temperature);
  _initInputVariable(_inputVar_dew_point, _params->dewpoint);
  _initInputVariable(_inputVar_relhum, _params->relHumidity);
  _initInputVariable(_inputVar_windspd, _params->windSpeed);
  _initInputVariable(_inputVar_winddir, _params->windDir);
  _initInputVariable(_inputVar_windgust, _params->windGust);
  _initInputVariable(_inputVar_pres, _params->pressure);
  _initInputVariable(_inputVar_liquid_accum, _params->precipAccum);
  _initInputVariable(_inputVar_visibility, _params->visibility);

  // initialize process registration

  if (_args->isRealtime())
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void netCDF2StnRptSpdb::run()
{
  static const string method_name = "netCDF2StnRptSpdb::run()";
  
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

bool netCDF2StnRptSpdb::_processData(const string& file_name)
{
  static const string method_name = "netCDF2StnRptSpdb::_processData()";
  
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
  if (!_inputVar_temp.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_temp.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_dew_point.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_dew_point.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_relhum.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_relhum.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_windspd.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_windspd.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_winddir.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_winddir.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_windgust.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_windgust.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_pres.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_pres.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_liquid_accum.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_liquid_accum.getVariableName() << endl;
      return false;
    }
  if (!_inputVar_visibility.init(netCDFfile))
    {
      cerr << "Error: init() failed on: " 
	   << _inputVar_visibility.getVariableName() << endl;
      return false;
    }

  //create an NcVar pointer for the dataProvider variable name

  NcVar *dataProviderVar = netCDFfile.get_var(_params->dataProviderVar);

  //need to know the string size since netCDF uses char *'s not strings

  int stringSize = dataProviderVar->get_dim(1)->size();

  //create an NcVar pointer for the stationID variable name; need its size as well

  NcVar *stationIDVar = netCDFfile.get_var(_params->stationIDVar);
  int stringSizeTwo = stationIDVar->get_dim(1)->size();

  //netCDF uses char *'s so need to divide by that to get the number of values
  int numRecs = dataProviderVar->num_vals()/stringSize;

  for (int i = 0; i < numRecs; ++i)
    {
      int stringIndex = i * stringSize;
      
      //check to see if the value of the current data provider variable
      //matches the one the user is interested in; if so, get the remaining
      //values and write it to a station_report

      //adding the explicit delete fixed memory leaks
      //as_string() must use the new operator...

      char *dataProvider = dataProviderVar->as_string(stringIndex);

      if (!STRequal_exact(dataProvider,
			  _params->dataProviderValue))
	{
	  delete [] dataProvider; 
	  continue;
	}
      delete [] dataProvider;

      //create a station_report object and fill it out

      station_report_t netCDF_station;
      netCDF_station.msg_id = STATION_REPORT;
      netCDF_station.time = netCDFfile.get_var(_params->observationTime)->as_int(i);
      netCDF_station.accum_start_time = 0;
      netCDF_station.weather_type = 0;
      netCDF_station.lat = _inputVar_lat.getValue(i);
      netCDF_station.lon = _inputVar_lon.getValue(i);
      netCDF_station.alt = _inputVar_alt.getValue(i);
      netCDF_station.temp = _inputVar_temp.getValue(i);
      netCDF_station.dew_point = _inputVar_dew_point.getValue(i);
      netCDF_station.relhum = _inputVar_relhum.getValue(i);
      netCDF_station.windspd = _inputVar_windspd.getValue(i);
      netCDF_station.winddir = _inputVar_winddir.getValue(i);
      netCDF_station.windgust = _inputVar_windgust.getValue(i);
      netCDF_station.pres = _inputVar_pres.getValue(i);
      netCDF_station.liquid_accum = _inputVar_liquid_accum.getValue(i);
      netCDF_station.precip_rate = STATION_NAN;
      netCDF_station.visibility = _inputVar_visibility.getValue(i);
      netCDF_station.rvr = STATION_NAN;
      netCDF_station.ceiling = STATION_NAN;
      netCDF_station.shared.station.liquid_accum2 = STATION_NAN;
      netCDF_station.shared.station.Spare1 = STATION_NAN;
      netCDF_station.shared.station.Spare2 = STATION_NAN;

      char *stationLabel = stationIDVar->as_string(i * stringSizeTwo);
      
      STRcopy(netCDF_station.station_label, 
	      stationLabel, ST_LABEL_SIZE);

      //adding the explicit delete fixed memory leaks
      //as_string() must use the new operator...

      delete [] stationLabel;
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

bool netCDF2StnRptSpdb::_writeToDatabase(station_report_t& station)
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

void netCDF2StnRptSpdb::_initInputVariable(InputVariable &inputVar, 
					   const Params::input_variable_t &param_input) const
{

  //set the debug level of the InputVariable object to match the netCDF2StnRptSpdb one

  inputVar.setDebug(_params->debug >= Params::DEBUG_VERBOSE);

  inputVar.setVariableName(param_input.variable_name);
  inputVar.setMissingValueAttName(param_input.missing_value_attr_name);
 
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

