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
//   $Date: 2016/03/07 01:39:55 $
//   $Id: AverageSpdbSndg.cc,v 1.3 2016/03/07 01:39:55 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AverageSpdbSndg: AverageSpdbSndg program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>

#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <rapformats/GenPt.hh>
#include <rapformats/Sndg.hh>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>

#include "AverageSpdbSndg.hh"
#include "Params.hh"

#include "SndgPlusReader.hh"
#include "SndgReader.hh"

using namespace std;


// Global variables

AverageSpdbSndg *AverageSpdbSndg::_instance =
     (AverageSpdbSndg *)NULL;



/*********************************************************************
 * Constructor
 */

AverageSpdbSndg::AverageSpdbSndg(int argc, char **argv) :
  _dataTrigger(0)
{
  static const string method_name = "AverageSpdbSndg::AverageSpdbSndg()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (AverageSpdbSndg *)NULL);
  
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

AverageSpdbSndg::~AverageSpdbSndg()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _reader;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

AverageSpdbSndg *AverageSpdbSndg::Inst(int argc, char **argv)
{
  if (_instance == (AverageSpdbSndg *)NULL)
    new AverageSpdbSndg(argc, argv);
  
  return(_instance);
}

AverageSpdbSndg *AverageSpdbSndg::Inst()
{
  assert(_instance != (AverageSpdbSndg *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool AverageSpdbSndg::init()
{
  static const string method_name = "AverageSpdbSndg::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->input_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->input_url << endl;
      
      return false;
    }

    _dataTrigger = trigger;
    
    if (_params->debug)
    {
      cerr << "Successfully initialized LATEST_DATA trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
    }
    
    break;
  }
  
  case Params::TIME_LIST :
  {
    time_t start_time =
      DateTime::parseDateTime(_params->time_list_trigger.start_time);
    time_t end_time
      = DateTime::parseDateTime(_params->time_list_trigger.end_time);
    
    if (start_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing start_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.start_time << endl;
      
      return false;
    }
    
    if (end_time == DateTime::NEVER)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error parsing end_time string for TIME_LIST trigger: " <<
	_params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->input_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->input_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    if (_params->debug)
    {
      cerr << "Successfully initialized TIME_LIST trigger:" << endl;
      cerr << "    URL: " << _params->input_url << endl;
      cerr << "    start time: " << DateTime::str(start_time) << endl;
      cerr << "    end time: " << DateTime::str(end_time) << endl;
    }
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the database reader object

  switch (_params->input_database_format)
  {
  case Params::SNDG_PLUS_FORMAT:
    _reader = new SndgPlusReader(_params->input_url, _params->debug);
    break;
    
  case Params::SNDG_OLD_FORMAT :
    _reader = new SndgReader(_params->input_url, _params->debug);
    break;
  } /* endswitch - _params->input_database_format */
  
  // Make sure that the pressure limits are ordered the way we want

  if (_params->pressure_limits.max_pressure <
      _params->pressure_limits.min_pressure)
  {
    double temp = _params->pressure_limits.min_pressure;
    _params->pressure_limits.min_pressure =
      _params->pressure_limits.max_pressure;
    _params->pressure_limits.max_pressure = temp;
  }
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void AverageSpdbSndg::run()
{
  static const string method_name = "AverageSpdbSndg::run()";
  
  TriggerInfo trigger_info;

  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->next(trigger_info) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_info.getIssueTime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " 
	   << DateTime(trigger_info.getIssueTime()) << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcAvg() - Calculate the indicated average value for the given
 *              sounding.
 *
 * Returns the calculated average value on success, Sndg::VALUE_UNKNOWN
 * on failure.
 */

double AverageSpdbSndg::_calcAvg(const Sndg &sounding,
				 const Params::sndg_field_t field_name) const
{
  double field_total = 0.0;
  int num_values = 0;
  
  vector< Sndg::point_t > points = sounding.getPoints();
  vector< Sndg::point_t >::const_iterator point;
  
  for (point = points.begin(); point != points.end(); ++point)
  {
    // See it the point lies between the specified pressure limits

    if (point->pressure < _params->pressure_limits.min_pressure ||
	point->pressure > _params->pressure_limits.max_pressure)
      continue;
    
    // Get the field value for the point

    double field_value = 0.0;
    
    switch (field_name)
    {
    case Params::U :
      field_value = point->u;
      break;
      
    case Params::V :
      field_value = point->v;
      break;
      
    case Params::W :
      field_value = point->w;
      break;
      
    case Params::RH :
      field_value = point->rh;
      break;
      
    case Params::TEMP :
      field_value = point->temp;
      break;
      
    case Params::DEWPT :
      field_value = point->dewpt;
      break;
      
    case Params::WIND_SPEED :
      field_value = point->windSpeed;
      break;
      
    case Params::WIND_DIR :
      field_value = point->windDir;
      break;
    } /* endcase - field_name */
    
    if (field_value == Sndg::VALUE_UNKNOWN)
      continue;
    
    // Update the field totals

    field_total += field_value;
    ++num_values;
    
  } /* endfor - point */

  if (num_values == 0)
    return Sndg::VALUE_UNKNOWN;
  
  return field_total / (double)num_values;
}


/*********************************************************************
 * _genptFieldName() - Return the name to be used in the GenPt database
 *                     for the given field.
 */

string AverageSpdbSndg::_genptFieldName(const Params::sndg_field_t field_name)
{
  switch (field_name)
  {
  case Params::U :
    return "U";
      
  case Params::V :
    return "V";
      
  case Params::W :
    return "W";
      
  case Params::RH :
    return "rh";
      
  case Params::TEMP :
    return "temp";
      
  case Params::DEWPT :
    return "dewpt";
      
  case Params::WIND_SPEED :
    return "windSpeed";
      
  case Params::WIND_DIR :
    return "windDir";
    
  } /* endcase - field_name */
    
  return "unknown";
}


/*********************************************************************
 * _genptFieldUnits() - Return the units to be used in the GenPt database
 *                     for the given field.
 */

string AverageSpdbSndg::_genptFieldUnits(const Params::sndg_field_t field_name)
{
  switch (field_name)
  {
  case Params::U :
    return "m/s";
      
  case Params::V :
    return "m/s";
      
  case Params::W :
    return "m/s";
      
  case Params::RH :
    return "%";
      
  case Params::TEMP :
    return "C";
      
  case Params::DEWPT :
    return "C";
      
  case Params::WIND_SPEED :
    return "m/s";
      
  case Params::WIND_DIR :
    return "degN";
    
  } /* endcase - field_name */
    
  return "unknown";
}


/*********************************************************************
 * _processData() - Process the data for the given time.
 */

bool AverageSpdbSndg::_processData(const DateTime &trigger_time)
{
  static const string method_name = "AverageSpdbSndg::_processData()";
  
  PMU_auto_register("Processing data...");
  
  if (_params->debug)
    cerr << endl
	 << "*** Processing data for time: " << trigger_time << endl;
  
  // Get the soundings from the database

  vector< Sndg > soundings;
  
  if (!_reader->readSoundings(trigger_time, soundings))
    return false;
  
  // Calculate the requested average values for each of the soundings
  // and add it to the output database object.

  DsSpdb out_spdb;
  out_spdb.setPutMode(Spdb::putModeAddUnique);

  vector< Sndg >::const_iterator sounding;
  
  for (sounding = soundings.begin(); sounding != soundings.end(); ++sounding)
  {
    GenPt point;
    Sndg::header_t sndg_hdr = sounding->getHeader();
    
    point.setName(sndg_hdr.sourceName);
    point.setId(sndg_hdr.sourceId);
    point.setTime(sndg_hdr.launchTime);
    point.setLat(sndg_hdr.lat);
    point.setLon(sndg_hdr.lon);
    
    for (int i = 0; i < _params->average_fields_n; ++i)
    {
      double avg_value = _calcAvg(*sounding, _params->_average_fields[i]);

      if (avg_value == Sndg::VALUE_UNKNOWN)
	continue;
    
      point.addVal(avg_value);
      point.addFieldInfo(_genptFieldName(_params->_average_fields[i]),
			 _genptFieldUnits(_params->_average_fields[i]));
      
    } /* endfor - i */
    
    if (_params->debug)
      point.print(cerr);
    
    // Add the point to the output database buffer

    if (point.assemble() != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error assembling GenPt object into format needed for database" << endl;
      cerr << "Skipping point..." << endl;
      
      continue;
    }
    
    out_spdb.addPutChunk(0,
			 point.getTime(),
			 point.getTime(),
			 point.getBufLen(),
			 point.getBufPtr());
    
  } /* endfor - sounding */
  
  // Write the average values to the output database

  if (out_spdb.put(_params->output_url,
		   SPDB_GENERIC_POINT_ID,
		   SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing GenPt data to URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
