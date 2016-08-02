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
//   $Id: TstormGrow.cc,v 1.5 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TstormGrow: TstormGrow program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * Nov 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <dsdata/Tstorm.hh>
#include <dsdata/TstormGroup.hh>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TstormGrow.hh"
#include "Params.hh"

using namespace std;

// Global variables

TstormGrow *TstormGrow::_instance =
     (TstormGrow *)NULL;


/*********************************************************************
 * Constructor
 */

TstormGrow::TstormGrow(int argc, char **argv) :
  _leadTimeHrs(0.0),
  _dataTrigger(0),
  _polygonGrid(0),
  _polygonGridSize(0)
{
  static const string method_name = "TstormGrow::TstormGrow()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TstormGrow *)NULL);
  
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

TstormGrow::~TstormGrow()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete [] _polygonGrid;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TstormGrow *TstormGrow::Inst(int argc, char **argv)
{
  if (_instance == (TstormGrow *)NULL)
    new TstormGrow(argc, argv);
  
  return(_instance);
}

TstormGrow *TstormGrow::Inst()
{
  assert(_instance != (TstormGrow *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TstormGrow::init()
{
  static const string method_name = "TstormGrow::init()";
  
  // Calculate values based on parameter values

  _leadTimeHrs = _params->fcst_lead_time_secs / 3600.0;
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // initialize process registration

  PMU_auto_init(_progName, _params->instance,
		PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void TstormGrow::run()
{
  static const string method_name = "TstormGrow::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time))
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
 * _applyGrowthToStorm() - Apply the growth and decay to the given
 *                         storm in the grid.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_applyGrowthToStorm(MdvxField &growth_field,
				     fl32 *orig_grid,
				     const Tstorm &storm) const
{
  static const string method_name = "TstormGrow::_applyGrowthToStorm()";
  
  // Only process valid storms, if requested

  if (_params->valid_storms_only && !storm.isForecastValid())
    return true;
  
  // Make sure that the storm meets the size requirements

  if (storm.getArea() > _params->max_storm_size)
  {
    if (_params->verbose)
      cerr << "Storm too large to apply growth: " << storm.getArea()
	   << " km" << endl;
      
    return true;
  }
    
  // If the storm size isn't changing then we don't need to do anything

  if (storm.getDareaDt() == 0.0)
  {
    if (_params->verbose)
      cerr << "Storm size not changing -- no growth applied" << endl;
    
    return true;
  }

  // Grow or decay the storm as indicated by the change in area

  if (storm.getDareaDt() < 0.0 && _params->apply_decay)
    return _decayStorm(growth_field, orig_grid, storm);

  if (storm.getDareaDt() > 0.0 && _params->apply_growth)
    return _growStorm(growth_field, orig_grid, storm);

  return true;
}


/*********************************************************************
 * _applyGrowthToStorms() - Apply the growth and decay to the storms
 *                          in the grid.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_applyGrowthToStorms(DsMdvx &mdvx)
{
  static const string method_name = "TstormGrow::_applyGrowthToStorms()";
  
  // Get pointers to the data in the Mdv object

  MdvxField *field = mdvx.getField(0);
  
  Mdvx::field_header_t field_hdr = field->getFieldHeader();
  fl32 *fcst_data = (fl32 *)field->getVol();
  
  // We only operate on 2D data

  if (field_hdr.nz != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "We only operate on 2D data" << endl;
    cerr << "Our field nz = " << field_hdr.nz << endl;
    
    return false;
  }
  
  // Make sure that the polygon grid is large enough for the current field

  int grid_size = field_hdr.nx * field_hdr.ny;
  
  if (grid_size > _polygonGridSize)
  {
    delete [] _polygonGrid;
    _polygonGrid = new unsigned char[grid_size];
    _polygonGridSize = grid_size;
  }
  
  // Create a copy of the data grid to use for calculating growth/decay.
  // Then we can update the grid in the Mdv object in place.

  int plane_size = field_hdr.nx * field_hdr.ny;
  
  fl32 *curr_data = new fl32[plane_size];
  memcpy(curr_data, fcst_data, plane_size * sizeof(fl32));
  
  // Loop through each of the storms changing intensity values as necessary

  vector< TstormGroup* > groups = _tstormMgr.getGroups();
  vector< TstormGroup* >::const_iterator group_iter;
  
  for (group_iter = groups.begin(); group_iter != groups.end(); ++group_iter)
  {
    TstormGroup *group = *group_iter;
    
    vector< Tstorm* > storms = group->getTstorms();
    vector< Tstorm* >::const_iterator storm_iter;
    
    for (storm_iter = storms.begin(); storm_iter != storms.end(); ++storm_iter)
    {
      Tstorm *storm = *storm_iter;
      
      if (!_applyGrowthToStorm(*field, curr_data, *storm))
      {
	cerr << "WARNING: " << method_name << endl;
	cerr << "Error applying growth to storm" << endl;
      }
      
    } /* endfor - storm_iter */
    
  } /* endfor - group_iter */
  
  // Reclaim memory

  delete [] curr_data;
  
  return true;
}


/*********************************************************************
 * _decayStorm() - Apply the decay to the given storm in the grid.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_decayStorm(MdvxField &growth_field,
			     fl32 *orig_grid,
			     const Tstorm &storm) const
{
  static const string method_name = "TstormGrow::_decayStorm()";
  
  // Get the needed field information

  Mdvx::field_header_t field_hdr = growth_field.getFieldHeader();
  fl32 *growth_grid = (fl32 *)growth_field.getVol();
  MdvxPjg proj(field_hdr);
  
  // To apply the decay, we will reduce the intensity of each of the
  // grid points within the original storm polygon.  Reducing the
  // intensities will effectively reduce the storm size since the edges
  // of the storm should no longer meet the defined storm intensity
  // requirements.

  // Get the storm polygon grid.

  memset(_polygonGrid, 0, _polygonGridSize);
  
  int min_x, min_y;
  int max_x, max_y;
  
  if (!storm.getPolygonGrid(proj, _polygonGrid,
			    min_x, min_y, max_x, max_y))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting polygon grid for storm" << endl;
    
    return false;
  }
  
  // Calculate the percentage value change for the storm

  double percent_change =
    storm.getDareaDt() / storm.getArea() * _leadTimeHrs;
  
  // Apply the value change to each grid point within the storm

  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      int index = proj.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[index] == 0)
	continue;
      
      double value_change =
	fabs(orig_grid[index] - _params->intensity_base_value) *
	percent_change;
      
      if (_params->data_is_descending)
	growth_grid[index] = orig_grid[index] - value_change;
      else
	growth_grid[index] = orig_grid[index] + value_change;

    } /* endfor - y */
  } /* endfor - x */
  
  return true;
}


/*********************************************************************
 * _growStorm() - Apply the growth to the given storm in the grid.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_growStorm(MdvxField &growth_field,
			    fl32 *orig_grid,
			    const Tstorm &storm) const
{
  static const string method_name = "TstormGrow::_growStorm()";
  
  // Get the needed field information

  Mdvx::field_header_t field_hdr = growth_field.getFieldHeader();
  fl32 *growth_grid = (fl32 *)growth_field.getVol();
  MdvxPjg proj(field_hdr);
  
  // To apply the growth, we will increase the intensity of each of the
  // grid points within and arounde storm polygon.  We use the rate of change
  // of the area to decide how far around the polygon to do the growth.

  // Calculate the percentage value change for the storm

  double percent_change =
    storm.getDareaDt() / storm.getArea() * _leadTimeHrs;
  
  // Get the storm polygon grid, growing the storm by the given percentage.

  memset(_polygonGrid, 0, _polygonGridSize);
  
  int min_x, min_y;
  int max_x, max_y;
  
  if (!storm.getPolygonGridGrowPercent(proj, _polygonGrid,
				       min_x, min_y, max_x, max_y,
				       percent_change))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error getting polygon grid for storm" << endl;
    
    return false;
  }
  
  // Apply the value change to each grid point within the storm

  for (int x = min_x; x <= max_x; ++x)
  {
    for (int y = min_y; y <= max_y; ++y)
    {
      int index = proj.xyIndex2arrayIndex(x, y);
      
      if (_polygonGrid[index] == 0)
	continue;
      
      double value_change =
	fabs(orig_grid[index] - _params->intensity_base_value) *
	percent_change;
      
      if (_params->data_is_descending)
	growth_grid[index] = orig_grid[index] - value_change;
      else
	growth_grid[index] = orig_grid[index] + value_change;

    } /* endfor - y */
  } /* endfor - x */
  
  return true;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_initTrigger(void)
{
  static const string method_name = "TstormGrow::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug || _params->verbose)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->trigger_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->trigger_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->trigger_url << endl;
      cerr << trigger->getErrStr() << endl;
      
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
    
    if (_params->debug || _params->verbose)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->trigger_url << endl;
      cerr << "   start time: " << DateTime::str(start_time) << endl;
      cerr << "   end time: " << DateTime::str(end_time) << endl;
    }
    
    
    DsTimeListTrigger *trigger = new DsTimeListTrigger();
    if (trigger->init(_params->trigger_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->trigger_url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      cerr << trigger->getErrStr() << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  
  } /* endswitch - _params->trigger_mode */

  return true;
}


/*********************************************************************
 * _performRead() - Perform the read.  This method just consolidates
 *                  the parts of the reading of the input fields that
 *                  is common between fields.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_performRead(DsMdvx &mdvx,
			      const string &url,
			      const DateTime &trigger_time,
			      const int max_input_secs) const
{
  static const string method_name = "TstormGrow::_performRead()";
  
  // Finish setting up the read request

  mdvx.setReadTime(Mdvx::READ_FIRST_BEFORE,
		   url,
		   max_input_secs,
		   trigger_time.utime());
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);

  mdvx.setReadFieldFileHeaders();
  
  // Read the data

  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading input volume from: " << url << endl;
    cerr << mdvx.getErrStr() << endl;
    
    return false;
  }
  
  // Make sure that we got a field.  I think the read will fail if we don't
  // get the field, but we assume we have a field later on so it doesn't
  // hurt to check just in case.

  if (mdvx.getNFields() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error with input volume" << endl;
    cerr << "Expected 1 field, got " << mdvx.getNFields() << " fields" << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_processData(const DateTime &trigger_time)
{
  static const string method_name = "TstormGrow::_processData()";
  
  if (_params->debug || _params->verbose)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  // Read in the gridded input

  DsMdvx mdvx;
  bool status;
  
  if (_params->gridded_field.use_field_name)
    status = _readInput(mdvx,
			_params->gridded_field.url,
			_params->gridded_field.field_name,
			trigger_time,
			_params->gridded_field.max_input_secs);
  else
    status = _readInput(mdvx,
			_params->gridded_field.url,
			_params->gridded_field.field_num,
			trigger_time,
			_params->gridded_field.max_input_secs);
  
  if (!status)
    return false;
  
  // Read in the storm data

  _tstormMgr.clearData();
  
  if (_tstormMgr.readTstorms(_params->storms_url,
			     trigger_time.utime(),
			     _params->storms_max_input_secs) < 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading storm data from URL: "
	 << _params->storms_url << endl;
    cerr << "    Search time: " << trigger_time << endl;
    cerr << "    Search margin: " << _params->storms_max_input_secs << endl;
    
    return false;
  }
  
  if (_tstormMgr.getNumGroups() < 1)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "No storm data found for time: " << trigger_time << endl;
    
    return true;
  }
  
  // Update the grid

  if (!_applyGrowthToStorms(mdvx))
    return false;
  
  // Compress the output fields

  for (int i = 0; i < mdvx.getNFields(); ++i)
  {
    MdvxField *field = mdvx.getField(i);
    
    Mdvx::field_header_t file_field_hdr = *(field->getFieldHeaderFile());
    
    if (file_field_hdr.encoding_type == Mdvx::ENCODING_INT8)
      field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_SPECIFIED,
			 file_field_hdr.scale,
			 file_field_hdr.bias);
    else
      field->convertType(Mdvx::ENCODING_INT8,
			 Mdvx::COMPRESSION_BZIP,
			 Mdvx::SCALING_DYNAMIC);
  }
  
  // Write the output

  mdvx.setWriteLdataInfo();
  
  if (mdvx.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    cerr << mdvx.getErrStr();
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readInput() - Read the indicated input.
 *
 * Returns true on success, false on failure.
 */

bool TstormGrow::_readInput(DsMdvx &mdvx,
			    const string &url,
			    const string &field_name,
			    const DateTime &trigger_time,
			    const int max_input_secs) const
{
  static const string method_name = "TstormGrow::_readInput()";
  
  // Set up the read request

  mdvx.addReadField(field_name);
  
  return _performRead(mdvx, url, trigger_time, max_input_secs);
}


bool TstormGrow::_readInput(DsMdvx &mdvx,
			    const string &url,
			    const int field_num,
			    const DateTime &trigger_time,
			    const int max_input_secs) const
{
  static const string method_name = "TstormGrow::_readInput()";
  
  // Set up the read request

  mdvx.addReadField(field_num);
  
  return _performRead(mdvx, url, trigger_time, max_input_secs);
}
