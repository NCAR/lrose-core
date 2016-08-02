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
//   $Date: 2016/03/04 02:04:38 $
//   $Id: Tstorms2GenPoly.cc,v 1.7 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.7 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * Tstorms2GenPoly: Tstorms2GenPoly program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * December 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <iostream>
#include <string>
#include <vector>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "Tstorms2GenPoly.hh"
#include "Params.hh"

using namespace std;


// Global variables

Tstorms2GenPoly *Tstorms2GenPoly::_instance = (Tstorms2GenPoly *)NULL;


/*********************************************************************
 * Constructor
 */

Tstorms2GenPoly::Tstorms2GenPoly(int argc, char **argv)
{
  static const string method_name = "Tstorms2GenPoly::Tstorms2GenPoly()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (Tstorms2GenPoly *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set the program name.

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _progName = STRdup(progname_parts.base);
  
  // display ucopyright message.

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

Tstorms2GenPoly::~Tstorms2GenPoly()
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

Tstorms2GenPoly *Tstorms2GenPoly::Inst(int argc, char **argv)
{
  if (_instance == (Tstorms2GenPoly *)NULL)
    new Tstorms2GenPoly(argc, argv);
  
  return(_instance);
}

Tstorms2GenPoly *Tstorms2GenPoly::Inst()
{
  assert(_instance != (Tstorms2GenPoly *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool Tstorms2GenPoly::init()
{
  static const string method_name = "Tstorms2GenPoly::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug >= Params::DEBUG_VERBOSE)
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
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void Tstorms2GenPoly::run()
{
  static const string method_name = "Tstorms2GenPoly::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool Tstorms2GenPoly::_processData(const DateTime &data_time)
{
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;
  
  // First read in and process the Titan storms.  This method will
  // fill in the _polygons vector for us.

  if (!_readTitanStorms(data_time))
    return false;

  // Then write the polygons to the output database

  if (!_writePolygons(data_time))
    return false;
  
  return true;
}


/*********************************************************************
 * _processTitanStorm() - Convert the given Titan storm to GenPoly
 *                        format and add the polygon to the _polygons
 *                        vector.
 *
 * Returns true on success, false on failure.
 */

bool Tstorms2GenPoly::_processTitanStorm(const Tstorm &tstorm)
{
  // Create the new polygon object

  GenPoly *polygon = new GenPoly();
  
  // Add the vertices to the new polygon

  Polyline *detection_poly = tstorm.getDetectionPoly();
  
  for (int i = 0; i < detection_poly->getNumPts(); ++i)
  {
    GenPoly::vertex_t vertex;
    
    vertex.lat = detection_poly->getY(i);
    vertex.lon = detection_poly->getX(i);
    
    polygon->addVertex(vertex);
  }
  
  // Add the specified output fields to the polygon

  for (int i = 0; i < _params->output_fields_n; ++i)
  {
    double field_value = 0.0;
    string field_units = "";
    
    switch (_params->_output_fields[i].titan_field)
    {
    case Params::AREA :
      field_value = tstorm.getArea();
      field_units = "km^2";
      break;
    case Params::SPEED :
      field_value = tstorm.getSpeed();
      field_units = "km/hr";
      break;
    case Params::DIRECTION :
      field_value = tstorm.getDirection();
      field_units = "deg";
      break;
    case Params::U :
      field_value = tstorm.getU();
      field_units = "m/s";
      break;
    case Params::V :
      field_value = tstorm.getV();
      field_units = "m/s";
      break;
    case Params::ASPECT_RATIO :
      field_value = tstorm.getAspectRatio();
      field_units = "none";
      break;
    case Params::SIMPLE_TRACK_ID :
      field_value = tstorm.getSimpleTrack();
      field_units = "none";
      break;
    case Params::COMPLEX_TRACK_ID :
      field_value = tstorm.getComplexTrack();
      field_units = "none";
      break;
    case Params::CENTROID_LAT :
    {
      double lat, lon;
      tstorm.getCentroid(lat, lon);
      field_value = lat;
      field_units = "deg";
      break;
    }
    case Params::CENTROID_LON :
    {
      double lat, lon;
      tstorm.getCentroid(lat, lon);
      field_value = lon;
      field_units = "deg";
      break;
    }
    } /* endswitch - _params->_output_fields[i].titan_field */
    
    // Add the information to the polygon

    polygon->addFieldInfo(_params->_output_fields[i].gen_poly_field_name,
			  field_units);
    
    polygon->addVal(field_value);
    
  } /* endfor - i */
  
  
  // Add the polygon to the list

  _polygons.push_back(polygon);
  
  return true;
}


/*********************************************************************
 * _readTitanStorms() - Read and process the Titan storms for the
 *                      given time.
 *
 * Returns true on success, false on failure.  Upon successful return,
 * the _polygons vector will be updated to include all of the storms
 * read in for this data time.
 */

bool Tstorms2GenPoly::_readTitanStorms(const DateTime &data_time)
{
  static const string method_name = "Tstorms2GenPoly::_readTitanStorms()";
  
  // Clear out the polygons vector to prepare for the new data

  _clearPolygons();
  
  // Read in the storms from the database

  TstormMgr tstorm_mgr;
  
  int num_storm_groups =
    tstorm_mgr.readTstorms(_params->input_url, data_time.utime(), 0);

  if (num_storm_groups < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading new storms for time: " << data_time << endl;

    return false;
  }

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   Found " << num_storm_groups
	 << " storm groups in the database" << endl;
  
  // Process all of the storms in all of the groups

  vector< TstormGroup* > tstorm_groups = tstorm_mgr.getGroups();
  vector< TstormGroup* >::iterator group_iter;
  
  for (group_iter = tstorm_groups.begin();
       group_iter != tstorm_groups.end(); ++group_iter)
  {
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "      Processing storm group...." << endl;
    
    TstormGroup *curr_group = *group_iter;
        
    vector< Tstorm* > curr_storms = curr_group->getTstorms();
    vector< Tstorm* >::iterator storm_iter;
    
    for (storm_iter = curr_storms.begin();
	 storm_iter != curr_storms.end(); ++storm_iter)
    {
      // Get a pointer to the current storm object.  This just makes
      // the syntax below a little more readable

      Tstorm *curr_storm = *storm_iter;
      
      // Only process valid storms

      if (_params->valid_forecasts_only && !curr_storm->isForecastValid())
	continue;
      
      if (!_processTitanStorm(*curr_storm))
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Error processing one of the Titan storms" << endl;
	cerr << "Skipping storm and continuing on...." << endl;
      }
      
    } /* endfor - storm_iter */
    
  } /* endfor - group_iter */

  return true;
}


/*********************************************************************
 * _writePolygons() - Write the polygons to the output database.
 *
 * Returns true on success, false on failure.
 */

bool Tstorms2GenPoly::_writePolygons(const DateTime &data_time)
{
  static const string method_name = "Tstorms2GenPoly::_writePolygons()";

  // Loop through the polygon vector, adding the appropriate data
  // chunk to spdb object

  DsSpdb gen_poly_db;
  
  for (size_t i = 0; i < _polygons.size(); ++i)
  {
    GenPoly *polygon = _polygons[i];
    
    // Assemble the GenPoly buffer for this polygon

    if (!polygon->assemble())
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error assembling polygon into SPDB buffer" << endl;
      cerr << polygon->getErrStr() << endl;
      cerr << "Skipping polygon..." << endl;
      
      continue;
    }
    
    int data_type = 0;
    time_t valid_time = data_time.utime();
    time_t expire_time = data_time.utime() + _params->expire_seconds;
    int chunk_len = polygon->getBufLen();
    const void *chunk_data = polygon->getBufPtr();

    gen_poly_db.addPutChunk(data_type,
                            valid_time,
                            expire_time,
                            chunk_len,
                            chunk_data);

  } /* endfor - i */
  
  // Write the chunks to the SPDB database

  gen_poly_db.setPutMode(Spdb::putModeAdd);
  
  if (gen_poly_db.put(_params->output_url,
		      SPDB_GENERIC_POLYLINE_ID,
		      SPDB_GENERIC_POLYLINE_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output to GenPoly SPDB database" << endl;
    cerr << "Output URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
