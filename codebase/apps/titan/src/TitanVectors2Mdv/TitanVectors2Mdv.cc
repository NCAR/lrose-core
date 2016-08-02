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
//   $Id: TitanVectors2Mdv.cc,v 1.11 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.11 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TitanVectors2Mdv: TitanVectors2Mdv program object.
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
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TitanVectors2Mdv.hh"
#include "Params.hh"

using namespace std;


// Global variables

TitanVectors2Mdv *TitanVectors2Mdv::_instance = (TitanVectors2Mdv *)NULL;

const float TitanVectors2Mdv::MISSING_DATA_VALUE = -999.0;

/*********************************************************************
 * Constructor
 */

TitanVectors2Mdv::TitanVectors2Mdv(int argc, char **argv) :
  _gridSize(0),
  _uGrid(0),
  _vGrid(0),
  _uSum(0),
  _vSum(0),
  _numVectors(0),
  _stormGrid(0)
{
  static const string method_name = "TitanVectors2Mdv::TitanVectors2Mdv()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TitanVectors2Mdv *)NULL);
  
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

TitanVectors2Mdv::~TitanVectors2Mdv()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  delete [] _uSum;
  delete [] _vSum;
  delete [] _numVectors;
  delete [] _stormGrid;
  delete [] _uGrid;
  delete [] _vGrid;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TitanVectors2Mdv *TitanVectors2Mdv::Inst(int argc, char **argv)
{
  if (_instance == (TitanVectors2Mdv *)NULL)
    new TitanVectors2Mdv(argc, argv);
  
  return(_instance);
}

TitanVectors2Mdv *TitanVectors2Mdv::Inst()
{
  assert(_instance != (TitanVectors2Mdv *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TitanVectors2Mdv::init()
{
  static const string method_name = "TitanVectors2Mdv::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->latest_data_trigger,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->latest_data_trigger << endl;
      
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
    if (trigger->init(_params->time_list_trigger.url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->time_list_trigger.url << endl;
      cerr << "    Start time: " << _params->time_list_trigger.start_time <<
	endl;
      cerr << "    End time: " << _params->time_list_trigger.end_time << endl;
      
      return false;
    }
    
    _dataTrigger = trigger;
    
    break;
  }
  } /* endswitch - _params->trigger_mode */
  
  // Initialize the output projection

  switch (_params->output_projection.proj_type)
  {
  case Params::PROJ_LATLON :
    _gridProjection.initLatlon(_params->output_projection.nx,
			       _params->output_projection.ny,
			       1,
			       _params->output_projection.dx,
			       _params->output_projection.dy,
			       1.0,
			       _params->output_projection.minx,
			       _params->output_projection.miny,
			       0.0);
    break;
    
  case Params::PROJ_FLAT :
    _gridProjection.initFlat(_params->output_projection.origin_lat,
			     _params->output_projection.origin_lon,
			     _params->output_projection.rotation,
			     _params->output_projection.nx,
			     _params->output_projection.ny,
			     1,
			     _params->output_projection.dx,
			     _params->output_projection.dy,
			     1.0,
			     _params->output_projection.minx,
			     _params->output_projection.miny,
			     0.0);
    break;
  } /* endswitch - _params->output_projection.proj_type */
  
  // Initialize the intermediate grids

  _gridSize = _gridProjection.getNx() * _gridProjection.getNy();
  
  _uSum = new float[_gridSize];
  _vSum = new float[_gridSize];
  _numVectors = new int[_gridSize];
  _stormGrid = new unsigned char[_gridSize];
  _uGrid = new fl32[_gridSize];
  _vGrid = new fl32[_gridSize];
  
  // initialize process registration

  if (_params->trigger_mode == Params::LATEST_DATA)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}
  


/*********************************************************************
 * run() - run the program.
 */

void TitanVectors2Mdv::run()
{
  static const string method_name = "TitanVectors2Mdv::run()";
  
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
 * _calcGriddedStorms() - Calculate the gridded storms for the given
 *                        data time.  Updates _uSum, _vSum, _numVectors,
 *                        _uGrid and _vGrid with the gridded information
 *                        for this set of storms.
 *
 * Returns true on success, false on failure.
 */

bool TitanVectors2Mdv::_calcGriddedStorms(const DateTime &data_time)
{
  static const string method_name = "TitanVectors2Mdv::_calcGriddedStorms()";
  
  // Read in the storms from the databases

  TstormMgr tstorm_mgr;
  
  for (int i = 0; i < _params->titan_input_urls_n; i++)
  {
      if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "Processing storms for url " << _params->_titan_input_urls[i] << endl;
      
      int num_storm_groups =
	  tstorm_mgr.readTstorms(_params->_titan_input_urls[i], data_time.utime(), _params->storm_time_margin);

      if (num_storm_groups < 0)
      {
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error reading new storms for time: " << data_time << endl;

	  return false;
      }

      if (_params->debug >= Params::DEBUG_NORM)
	  cerr << "   Found " << num_storm_groups
	       << " storm groups in the database" << endl;
  
  }
// Clear out the intermediate grids

  memset(_uSum, 0, _gridSize * sizeof(float));
  memset(_vSum, 0, _gridSize * sizeof(float));
  memset(_numVectors, 0, _gridSize * sizeof(int));

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

      DateTime group_time;
      group_time = curr_group->getDataTime();
      int fcst_secs = _params->forecast_secs + (data_time.utime() - group_time.utime());
      
      if (_params->debug >= Params::DEBUG_NORM) 
      {
	  cerr << "group time = " << group_time << endl;
	  cerr << "fcst_secs = " << fcst_secs << endl;
      }
      
    
      for (storm_iter = curr_storms.begin();
	   storm_iter != curr_storms.end(); ++storm_iter)
      {
      // Get a pointer to the current storm object.  This just makes
      // the syntax below a little more readable

	  Tstorm *curr_storm = *storm_iter;
      
      // Only process valid storms

	  if (_params->valid_forecasts_only && !curr_storm->isForecastValid())
	      continue;
      
      // Clear out the current storm grid

	  memset(_stormGrid, 0, _gridSize * sizeof(unsigned char));
	  
      // Get the grid for the current polygon

	  int min_x, min_y;
	  int max_x, max_y;
      
	  if (!curr_storm->getPolygonGridGrowKm(_gridProjection,
						_stormGrid,
						min_x, min_y,
						max_x, max_y,
						_params->polygon_growth_km,
						fcst_secs))
	  {
	      cerr << "WARNING: " << method_name << endl;
	      cerr << "Error converting Titan storm to gridded format" << endl;
	      cerr << "*** Skipping storm ***" << endl;
	      
	      continue;
	  }
      
      // Loop through the current storm grid points, updating our
      // intermediate grids

	  for (int x = min_x; x <= max_x; ++x)
	  {
	      for (int y = min_y; y <= max_y; ++y)
	      {
	      // Get the grid index
		  
		  int grid_index = _gridProjection.xyIndex2arrayIndex(x, y);
		  
		  if (grid_index < 0)
		  {
		      cerr << "WARNING: " << method_name << endl;
		      cerr << "Point (" << x << ", " << y << ") returned from getPolygonGrid() is outside of output grid" << endl;
	    
		      continue;
		  }
	  
		  if (_stormGrid[grid_index] <= 0)
		      continue;
	  
	      // Update the intermediate grids

		  _uSum[grid_index] += curr_storm->getU();
		  _vSum[grid_index] += curr_storm->getV();
		  ++_numVectors[grid_index];
	  
	      } /* endfor - y */
	  } /* endfor - x */
      
      } /* endfor - storm_iter */
	  
  } /* endfor - group_iter */

// Calculate the average U/V vectors

  for (int i = 0; i < _gridSize; ++i)
  {
      if (_numVectors[i] > 0)
      {
	  _uGrid[i] = _uSum[i] / (float)_numVectors[i];
	  _vGrid[i] = _vSum[i] / (float)_numVectors[i];
      }
      else
      {
	  _uGrid[i] = MISSING_DATA_VALUE;
	  _vGrid[i] = MISSING_DATA_VALUE;
      }
      
  } /* endfor - i */
  
  
  return true;
}


/*********************************************************************
 * _createField() - Create a blank field for the output file.
 *
 * Returns a pointer to the newly created field on success, 0 on
 * failure.  The calling method is responsible for deleting the
 * returned pointer.
 */

MdvxField *TitanVectors2Mdv::_createField(const DateTime &data_time,
					  const string &field_name_long,
					  const string &field_name,
					  const fl32 *data) const
{
  // Create the field header

  Mdvx::field_header_t field_hdr;
  memset(&field_hdr, 0, sizeof(field_hdr));
  
  _gridProjection.syncToFieldHdr(field_hdr);
  
  field_hdr.forecast_time = data_time.utime();
  field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  field_hdr.data_element_nbytes = 4;
  field_hdr.volume_size =
    field_hdr.nx * field_hdr.ny * field_hdr.nz * field_hdr.data_element_nbytes;
  field_hdr.compression_type = Mdvx::COMPRESSION_NONE;
  field_hdr.transform_type = Mdvx::DATA_TRANSFORM_NONE;
  field_hdr.scaling_type = Mdvx::SCALING_NONE;
  field_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  field_hdr.dz_constant = 1;
  field_hdr.scale = 1.0;
  field_hdr.bias = 0.0;
  field_hdr.bad_data_value = MISSING_DATA_VALUE;
  field_hdr.missing_data_value = MISSING_DATA_VALUE;
  STRcopy(field_hdr.field_name_long, field_name_long.c_str(),
	  MDV_LONG_FIELD_LEN);
  STRcopy(field_hdr.field_name, field_name.c_str(), MDV_SHORT_FIELD_LEN);
  STRcopy(field_hdr.units, "m/s", MDV_UNITS_LEN);
  
  // Create the vlevel header

  Mdvx::vlevel_header_t vlevel_hdr;
  memset(&vlevel_hdr, 0, sizeof(vlevel_hdr));
  
  vlevel_hdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  vlevel_hdr.level[0] = 0.5;
  
  // Create and return the new field

  return new MdvxField(field_hdr, vlevel_hdr, data);
}


/*********************************************************************
 * _processData() - Process data for the given time.
 *
 * Returns true on success, false on failure.
 */

bool TitanVectors2Mdv::_processData(const DateTime &data_time)
{
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;
  
  if (!_calcGriddedStorms(data_time))
    return false;
  
  if (_params->use_mdv_vectors_for_missing)
    _updateWithMdvVectors(data_time);
  
  if (!_writeOutputGrid(data_time))
    return false;
  
  return true;
}


/*********************************************************************
 * _updateMasterHeader() - Update the values in the output file master
 *                         header.
 */

void TitanVectors2Mdv::_updateMasterHeader(DsMdvx &output_file,
					   const DateTime &data_time) const
{
  Mdvx::master_header_t master_hdr;
  memset(&master_hdr, 0, sizeof(master_hdr));
  
  master_hdr.time_gen = time(0);
  master_hdr.time_begin = data_time.utime();
  master_hdr.time_end = data_time.utime();
  master_hdr.time_centroid = data_time.utime();
  master_hdr.time_expire = master_hdr.time_end;
  master_hdr.data_dimension = 2;
  master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  master_hdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  master_hdr.vlevel_included = 1;
  master_hdr.grid_orientation = Mdvx::ORIENT_SN_WE;
  master_hdr.data_ordering = Mdvx::ORDER_XYZ;
  STRcopy(master_hdr.data_set_info, "TitanVectors2Mdv output", MDV_INFO_LEN);
  STRcopy(master_hdr.data_set_name, "TitanVectors2Mdv", MDV_NAME_LEN);
  STRcopy(master_hdr.data_set_source, _params->_titan_input_urls[0], MDV_NAME_LEN);
  
  output_file.setMasterHeader(master_hdr);
}


/*********************************************************************
 * _updateWithMdvVectors() - Update the vectors in _uGrid and _vGrid
 *                           with the corresponding MDV vectors.  We
 *                           update with MDV vectors by simply replacing
 *                           missing vectors with the corresponding
 *                           MDV vectors.
 */

void TitanVectors2Mdv::_updateWithMdvVectors(const DateTime &data_time)
{
  static const string method_name = "TitanVectors2Mdv::_updateWithMdvVectors()";
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "Entering" << method_name << endl;
    
  // Read in the MDV fields

  DsMdvx input_mdv;
  
  input_mdv.setReadTime(Mdvx::READ_CLOSEST,
			_params->mdv_vector_info.url,
			_params->mdv_vector_info.max_valid_secs,
			data_time.utime());
  
  if (_params->mdv_vector_info.use_field_names)
  {
    input_mdv.addReadField(_params->mdv_vector_info.u_field_name);
    input_mdv.addReadField(_params->mdv_vector_info.v_field_name);
  }
  else
  {
    input_mdv.addReadField(_params->mdv_vector_info.u_field_num);
    input_mdv.addReadField(_params->mdv_vector_info.v_field_num);
  }
  
  input_mdv.setReadPlaneNumLimits(_params->mdv_vector_info.level_num,
				  _params->mdv_vector_info.level_num);
  
  input_mdv.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_mdv.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_mdv.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug >= Params::DEBUG_NORM)
    input_mdv.printReadRequest(cerr);
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   About to read volume...." << endl;
    
  if (input_mdv.readVolume() != 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Unable to read MDV vectors from URL: "
	 << _params->mdv_vector_info.url << endl;
    cerr << "Request time: " << data_time << endl;
    cerr << "Time margin: " << _params->mdv_vector_info.max_valid_secs << endl;
    cerr << "Not updating missing TITAN vectors" << endl;
    
    return;
  }
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "     .....volume successfully read" << endl;
    
  // Get pointers to the U/V field information

  MdvxField *u_field = input_mdv.getField(0);
  MdvxField *v_field = input_mdv.getField(1);
  
  if (u_field == 0 || v_field == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error getting pointer to MDV vector field" << endl;
    cerr << "Not updating missing TITAN vectors" << endl;
    
    return;
  }
  
  // Remap the U and V fields.  Do this now rather than during the
  // read operation so we can save the look-up table between files
  // since the projection of the motion fields probably won't change
  // from time to time.

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   About to remap data...." << endl;
    
  switch (_params->output_projection.proj_type)
  {
  case Params::PROJ_LATLON :
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "     About to remap U field to lat/lon...." << endl;
    u_field->remap2Latlon(_remapLut,
			  _params->output_projection.nx,
			  _params->output_projection.ny,
			  _params->output_projection.minx,
			  _params->output_projection.miny,
			  _params->output_projection.dx,
			  _params->output_projection.dy);
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "     About to remap V field to lat/lon...." << endl;
    v_field->remap2Latlon(_remapLut,
			  _params->output_projection.nx,
			  _params->output_projection.ny,
			  _params->output_projection.minx,
			  _params->output_projection.miny,
			  _params->output_projection.dx,
			  _params->output_projection.dy);
    break;
    
  case Params::PROJ_FLAT :
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "     About to remap U field to flat...." << endl;
    u_field->remap2Flat(_remapLut,
			_params->output_projection.nx,
			_params->output_projection.ny,
			_params->output_projection.minx,
			_params->output_projection.miny,
			_params->output_projection.dx,
			_params->output_projection.dy,
			_params->output_projection.origin_lat,
			_params->output_projection.origin_lon,
			_params->output_projection.rotation);
    if (_params->debug >= Params::DEBUG_NORM)
      cerr << "     About to remap V field to flat...." << endl;
    v_field->remap2Flat(_remapLut,
			_params->output_projection.nx,
			_params->output_projection.ny,
			_params->output_projection.minx,
			_params->output_projection.miny,
			_params->output_projection.dx,
			_params->output_projection.dy,
			_params->output_projection.origin_lat,
			_params->output_projection.origin_lon,
			_params->output_projection.rotation);
    break;
    
  } /* endswitch - _params->output_projection.proj_type */
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "     ....data successfully remapped" << endl;
    
  Mdvx::field_header_t u_field_hdr = u_field->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = v_field->getFieldHeader();
  
  fl32 *mdv_u_data = (fl32 *)u_field->getVol();
  fl32 *mdv_v_data = (fl32 *)v_field->getVol();
  
  if (mdv_u_data == 0 || mdv_v_data == 0)
  {
    cerr << "WARNING: " << method_name << endl;
    cerr << "Error getting pointers to MDV U/V field data" << endl;
    cerr << "Not updating missing TITAN vectors" << endl;
    
    return;
  }
  
  // Update the vectors

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "   Updating TITAN vectors with MDV vectors...." << endl;
    
  for (int i = 0; i < _gridSize; ++i)
  {
    if (_uGrid[i] == MISSING_DATA_VALUE ||
	_vGrid[i] == MISSING_DATA_VALUE)
    {
      if (mdv_u_data[i] != u_field_hdr.missing_data_value &&
	  mdv_u_data[i] != u_field_hdr.bad_data_value &&
	  mdv_v_data[i] != v_field_hdr.missing_data_value &&
	  mdv_v_data[i] != v_field_hdr.bad_data_value)
      {
	_uGrid[i] = mdv_u_data[i];
	_vGrid[i] = mdv_v_data[i];
      }
      
    }
  } /* endfor - i */
  
  return;
}


/*********************************************************************
 * _writeOutputGrid() - Write the _uGrid and _vGrid data to the output
 *                      file for the given time.
 *
 * Returns true on success, false on failure.
 */

bool TitanVectors2Mdv::_writeOutputGrid(const DateTime &data_time)
{
  static const string method_name = "TitanVectors2Mdv::_writeOutputGrid()";
  
  // Create the output fields

  MdvxField *u_field = _createField(data_time, "U component", "U", _uGrid);
  MdvxField *v_field = _createField(data_time, "V component", "V", _vGrid);
  
  if (u_field == 0 || v_field == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error creating output fields" << endl;
    
    delete u_field;
    delete v_field;
    
    return false;
  }
  
  // Write the output file

  DsMdvx output_file;
  
  _updateMasterHeader(output_file, data_time);
  
  u_field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_RLE,
		       Mdvx::SCALING_DYNAMIC);
  v_field->convertType(Mdvx::ENCODING_INT8,
		       Mdvx::COMPRESSION_RLE,
		       Mdvx::SCALING_DYNAMIC);
  
  output_file.addField(u_field);
  output_file.addField(v_field);
  
  output_file.setWriteLdataInfo();
  
  if (output_file.writeToDir(_params->output_url) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing output file to URL: " << _params->output_url << endl;
    
    return false;
  }
  
  return true;
}
