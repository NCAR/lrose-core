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
///////////////////////////////////////////////////////////////
// UpdateStormFields.cc
//
// UpdateStormFields object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 1998
//
///////////////////////////////////////////////////////////////

#include <stdio.h>
#include <cstring>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <euclid/CircularTemplate.hh>
#include <euclid/geometry.h>
#include <euclid/GridPoint.hh>
#include <euclid/point.h>
#include <Mdv/MdvxProj.hh>
#include <Mdv/MdvxPjg.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/SimpleList.h>
#include <Spdb/Spdb.hh>
#include <rapformats/titan_grid.h>
#include <toolsa/DateTime.hh>
#include <toolsa/globals.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>

#include "UpdateStormFields.hh"
#include "Args.hh"
#include "Params.hh"


/**************************************************************
 * Constructor
 */

UpdateStormFields::UpdateStormFields(int argc, char **argv)
{
  static const string method_name = "UpdateStormFields::Constructor";
  
  okay = TRUE;
  done = FALSE;

  // Set program name

  char *slash_pos = strrchr(argv[0], '/');
  
  if (slash_pos == (char *)NULL)
    _programName = STRdup(argv[0]);
  else
    _programName = STRdup(slash_pos + 1);
  
  // Get command line args

  _args = new Args(argc, argv, _programName);

  _archiveStartTime = _args->startTime;
  _archiveEndTime = _args->endTime;
  
  // Get TDRP params

  _params = new Params();
  char *params_path = (char *) "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Problem with TDRP parameters in file <" << params_path <<
      ">" << endl;
    
    okay = false;
    
    return;
  }

}


/**************************************************************
 * Destructor
 */

UpdateStormFields::~UpdateStormFields()
{
  // Unregister process

  PMU_auto_unregister();

  // Free strings

  STRfree(_programName);
  
  // Call destructors

  delete _dataTrigger;
  
  delete _params;
  delete _args;

}


/**************************************************************
 * init()
 */

bool UpdateStormFields::init()
{
  static const string method_name = "UpdateStormFields::init()";
  
  // Initialize the data trigger

  char *trigger_url;
  
  switch (_params->trigger_field)
  {
  case Params::TRIGGER_OFF_STORM_DATA :
    trigger_url = _params->storm_data_url;
    break;
    
  case Params::TRIGGER_OFF_FIELD_DATA :
    trigger_url = _params->field_data_url;
    break;
  }
  
  switch (_params->trigger_mode)
  {
  case Params::REALTIME :
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
      cerr << "Initializing DsLdataTrigger for data triggering" << endl;
    
    DsLdataTrigger *ldata_trigger = new DsLdataTrigger();
    
    if (ldata_trigger->init(trigger_url,
			    _params->max_valid_age,
			    PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing DsLdataTrigger object" << endl;
      cerr << ldata_trigger->getErrString() << endl;
      
      return false;
    }
    
    _dataTrigger = ldata_trigger;
    
    break;
  } /* endcase - Params::REALTIME */

  case Params::TIME_LIST :
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      cerr << "Initializing DsTimeListTrigger for data triggering" << endl;
      cerr << "     url: " << trigger_url << endl;
      cerr << "     start time: " << DateTime::str(_archiveStartTime) << endl;
      cerr << "     end time: " << DateTime::str(_archiveEndTime) << endl;
    }
    
    DsTimeListTrigger *time_trigger = new DsTimeListTrigger();
    
    if (time_trigger->init(trigger_url,
			   _archiveStartTime, _archiveEndTime) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing DsTimeListTrigger object" << endl;
      cerr << time_trigger->getErrString() << endl;
      
      return false;
    }

    _dataTrigger = time_trigger;
    
    break;
  } /* endcase - Params::TIME_LIST */
  
  } /* endswitch - _params->trigger_mode */
  

  // Initialize algorithm parameters

  _fieldUrl = _params->field_data_url;
  _radiusOfInfl = _params->radius_of_influence;
  
  // Initialize process registration
  
  if (_params->trigger_mode == Params::REALTIME)
    PMU_auto_init(_programName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
  
  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void UpdateStormFields::run()
{
  static const string method_name = "UpdateStormFields::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
      cerr << "*** Processing data for time: " <<
	DateTime::str(trigger_time.utime()) << endl;
    
    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    if (!_processData(trigger_time.utime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " <<
        trigger_time.dtime() << endl;
      
      continue;
    }
    
  } /* endwhile - !_dataTrigger->endOfData() */
  
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**************************************************************
 * _calculateStormDistance() - Calculate the distance, in km,
 *                             between the two given storms.
 */

double UpdateStormFields::_calculateStormDistance(tstorm_spdb_header_t *storm_hdr,
						  tstorm_spdb_entry_t *storm,
						  tstorm_spdb_header_t *base_hdr,
						  tstorm_spdb_entry_t *base)
{
  static MdvxProj         storm_proj;
  static MdvxProj         base_proj;
  static titan_grid_t     storm_grid;
  static titan_grid_t     base_grid;
  static int first_time = TRUE;
  
  // Initialize calculations

  if (first_time ||
      storm_hdr->grid.proj_origin_lat != storm_grid.proj_origin_lat ||
      storm_hdr->grid.proj_origin_lon != storm_grid.proj_origin_lon)
  {
    storm_grid = storm_hdr->grid;
    
    if (storm_grid.proj_type == Mdvx::PROJ_FLAT)
      storm_proj.initFlat(storm_grid.proj_origin_lat,
			  storm_grid.proj_origin_lon,
			  0);
  }
  
  if (first_time ||
      base_hdr->grid.proj_origin_lat != base_grid.proj_origin_lat ||
      base_hdr->grid.proj_origin_lon != base_grid.proj_origin_lon)
  {
    base_grid = base_hdr->grid;
    
    if (base_grid.proj_type == Mdvx::PROJ_FLAT)
      base_proj.initFlat(base_grid.proj_origin_lat,
			 base_grid.proj_origin_lon,
			 0);
  }
  
  first_time = FALSE;
  
  // Convert the base storm shape into its polygon form so we have the
  // lat/lon location for each vertex.

  tstorm_polygon_t base_polygon;
  
  tstorm_spdb_load_polygon(base_hdr, base,
			   &base_polygon, 0);
  
  // Calculate the distance of each radial end of the base storm from the
  // centroid of the storm.  The shortest distance here will determine which
  // radial from the base storm to use as the closest radial.

  double min_distance = 0;
  int base_radial = 0;
  
  for (int i = 0; i < base_hdr->n_poly_sides; i++)
  {
    double lat_diff = storm->latitude - base_polygon.pts[i].lat;
    double lon_diff = storm->longitude - base_polygon.pts[i].lon;
    
    double curr_distance = (lat_diff * lat_diff) + (lon_diff * lon_diff);
    
    if (i == 0 ||
	curr_distance < min_distance)
    {
      min_distance = curr_distance;
      base_radial = i;
    }
  } /* endfor - i */
  
  // Now we need to find the radial to use from the storm itself.  This will 
  // be the radial closest to the line between the base radial end point 
  // and the storm centroid.

  double radial_distance;
  double radial_theta;
  
  PJGLatLon2RTheta(base_polygon.pts[base_radial].lat,
		   base_polygon.pts[base_radial].lon,
		   storm->latitude, storm->longitude,
		   &radial_distance, &radial_theta);
  
  while (radial_theta < storm_hdr->poly_start_az)
    radial_theta += 360.0;
  
  int storm_radial = (int)(((radial_theta - storm_hdr->poly_start_az) /
    storm_hdr->poly_delta_az) + 0.5);

  // Now calculate the distance from the end point of the base radial to
  // the end point of the storm radial.  This is the distance we return.

  double storm_radial_lat;
  double storm_radial_lon;
  
  double grid_range = (double)storm->polygon_radials[storm_radial] *
    storm->polygon_scale;
  double theta = storm_hdr->poly_start_az +
    (storm_hdr->poly_delta_az * (double)storm_radial);
    
  if (storm_grid.proj_type == Mdvx::PROJ_FLAT)
  {
    double centroid_x;
    double centroid_y;
    
    storm_proj.latlon2xy(storm->latitude, storm->longitude,
			 centroid_x, centroid_y);
    
    double delta_x = grid_range * sin(theta) * storm_grid.dx;
    double delta_y = grid_range * cos(theta) * storm_grid.dy;
    
    storm_proj.xy2latlon(centroid_x + delta_x, centroid_y + delta_y,
			 storm_radial_lat, storm_radial_lon);
  }
  else /* MDV_PROJ_LATLON */
  {
    storm_radial_lat = storm->latitude +
      (grid_range * cos(theta) * storm_grid.dy);
    storm_radial_lon = storm->longitude +
      (grid_range * sin(theta) * storm_grid.dx);
  }
  
  // Now calculate the distance between the two radial end points.

  PJGLatLon2RTheta(base_polygon.pts[base_radial].lat,
		   base_polygon.pts[base_radial].lon,
		   storm_radial_lat, storm_radial_lon,
		   &radial_distance, &radial_theta);
  
  // Finally, make sure the storms aren't overlapping at the given
  // radials.  If they are overlapping, the distance should be 0.

  double storm_radial_distance, storm_radial_theta;
  double base_radial_distance, base_radial_theta;
  
  PJGLatLon2RTheta(storm->latitude, storm->longitude,
		   base_polygon.pts[base_radial].lat,
		   base_polygon.pts[base_radial].lon,
		   &base_radial_distance, &base_radial_theta);
  
  PJGLatLon2RTheta(storm->latitude, storm->longitude,
		   storm_radial_lat, storm_radial_lon,
		   &storm_radial_distance, &storm_radial_theta);
  
  if (storm_radial_distance > base_radial_distance)
  {
    radial_distance = 0.0;
  
    if (_params->debug_level >= Params::DEBUG_EXTRA)
      cerr << "*** Storms overlap, setting distance to 0.0" << endl;
  }
  
  if (_params->debug_level >= Params::DEBUG_EXTRA)
  {
    cerr << "Calculated radial distance " << radial_distance << endl;
    cerr << "    Used base radial " << base_radial <<
      " and storm radial " << storm_radial << endl;
  }
  
  return radial_distance;
}


/**************************************************************
 * _processData()
 */

bool UpdateStormFields::_processData(const time_t trigger_time)
{
  static const string method_name = "UpdateStormFields::_processData()";
  
  PMU_auto_register("Processing data");
      
  // Get the storm data
     
  if (!_retrieveStormData(trigger_time,
			 _params->time_offset_max))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving storm data for time: " <<
      DateTime::str(trigger_time) << endl;
    
    return false;
  }
    
  // Get the vector data

  if (!_retrieveFieldData(trigger_time,
			  _params->time_offset_max))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving vector data for time: " <<
      DateTime::str(trigger_time) << endl;
    
    return false;
  }
      
  // We have all of the data so we can go ahead and update
  // the storm vectors.

  _updateStormFields();
    
  // Now write the updated output database

  _writeOutputDatabase();
  
  return true;
}


/**************************************************************
 * _retrieveStormData() - Retrieve the indicated storm data from 
 *                        the storm database.
 *
 * Returns true if the storm data was successfully retrieved,
 * false otherwise.
 *
 * Upon successful return, _stormChunks is correctly set.
 */

bool UpdateStormFields::_retrieveStormData(const time_t data_time,
					    const int max_time_offset)
{
  static const string method_name = "UpdateStormFields::_retrieveStormData()";
  
/// If in REALTIME mode, need to wait for data!!!
  
  if (_stormSpdb.getClosest(_params->storm_data_url,
			    data_time,
			    max_time_offset,
			    0) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving storm data from <" <<
      _params->storm_data_url << "> for time " <<
      DateTime::str(data_time) << endl;
    return false;
  }
    
  // Retrieve the chunks

  _stormChunks = _stormSpdb.getChunks();

  if (_stormChunks.size() <= 0)
    return false;
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "Successfully retrieved " << _stormChunks.size() <<
      " chunks of storm data for time " <<
      DateTime::str(_stormChunks[0].valid_time) << endl;
	  
  // Byte swap the data, if necessary, to get native format.

  vector< Spdb::chunk_t >::iterator storm_iter;
  
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
    tstorm_spdb_buffer_from_BE((ui08 *)storm_iter->data);
  
  return true;
}


/**************************************************************
 * _retrieveFieldData() - Retrieve the indicated vector data 
 *                         from the vector database.
 *
 * Returns true if the vector data was successfully retrieved,
 * false otherwise.
 *
 * Upon successful return, _vectorChunks is correctly set for
 * TITAN vector data, or _vectorMdvUField and _vectorMdvVField
 * are correctly updated for MDV vector data.
 */

bool UpdateStormFields::_retrieveFieldData(const time_t data_time,
					   const int max_time_offset)
{
  static const string method_name = "UpdateStormFields::_retrieveFieldData()";

  if (_fieldSpdb.getClosest(_fieldUrl.c_str(),
			     data_time,
			     max_time_offset,
			     0) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error retrieving vector data from <" <<
      _fieldUrl << "> for time " <<
      DateTime::str(data_time) << endl;

    return false;
  }
	  
  // Retrieve the chunks

  _fieldChunks = _fieldSpdb.getChunks();
    
  if (_fieldChunks.size() <= 0)
    return false;
    
  if (_params->debug_level >= Params::DEBUG_NORM)
    cerr << "Successfully retrieved " << _fieldChunks.size() <<
      " chunks of vector data for time " <<
      DateTime::str(_fieldChunks[0].valid_time) << endl;
	  
  // Byte swap the data, if necessary, to get native format.

  vector< Spdb::chunk_t >::iterator vector_iter;
    
  for (vector_iter = _fieldChunks.begin();
       vector_iter != _fieldChunks.end();
       ++vector_iter)
    tstorm_spdb_buffer_from_BE((ui08 *)vector_iter->data);
  
  return true;
}


/**************************************************************
 * _updateStormFields() - Update the motion vectors in the incoming
 *                      storms using vectors obtained from TITAN.
 *
 * Upon successful return, _stormChunks is updated with field
 * values from _fieldChunks.
 */

void UpdateStormFields::_updateStormFields(void)
{
  // For each _stormChunks storm, get the _fieldChunks field values.
  //
  // Note that each SPDB chunk will hold multiple storms.  We
  // must process each storm in each chunk for both the _stormChunks
  // and the _fieldChunks.

  vector< Spdb::chunk_t >::iterator storm_iter;
  
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
  {
    tstorm_spdb_header_t *storm_hdr =
      (tstorm_spdb_header_t *)storm_iter->data;
    
    int num_storms = storm_hdr->n_entries;
    
    for (int ii = 0; ii < num_storms; ii++)
    {
      if (_params->debug_level >= Params::DEBUG_EXTRA)
	cerr << "Storm " << ii << ":" << endl;
      
      tstorm_spdb_entry_t *storm_entry =
	(tstorm_spdb_entry_t *)((char *)storm_hdr +
				sizeof(tstorm_spdb_header_t) +
				(ii * sizeof(tstorm_spdb_entry_t)));
      
      // Initialize the accumulators that operate over the field storms

      tstorm_spdb_entry_t *closest_field_storm = 0;
      double closest_field_storm_distance = _radiusOfInfl + 1.0;
      vector< tstorm_spdb_entry_t* > influencing_field_storms;
      
//      double u_sum = 0.0;
//      double v_sum = 0.0;
//      double weight_sum = 0.0;
      
//      double speed_sum = 0.0;
//      int num_fields = 0;
//      SimpleList<std_dev_info_t> std_dev_info_list;
      
      if (_params->debug_level >= Params::DEBUG_NORM)
	cerr << "  Looking at " << _fieldChunks.size() <<
	  " field chunks" << endl;
      
      // Loop through the field storms to find the correct storm to
      // get the field information from.

      vector< Spdb::chunk_t >::iterator field_iter;
      
      for (field_iter = _fieldChunks.begin();
	   field_iter != _fieldChunks.end();
	   ++field_iter)
      {
	tstorm_spdb_header_t *field_hdr =
	  (tstorm_spdb_header_t *)(field_iter->data);
    
	int num_field_storms = field_hdr->n_entries;
    
	if (_params->debug_level >= Params::DEBUG_NORM)
	  cerr << "  Looking at " << num_field_storms <<
	    " field storms" << endl;
      
	for (int jj = 0; jj < num_field_storms; jj++)
	{
	  tstorm_spdb_entry_t *field_entry =
	    (tstorm_spdb_entry_t *)((char *)field_hdr +
				    sizeof(tstorm_spdb_header_t) +
				    (jj * sizeof(tstorm_spdb_entry_t)));

	  double distance;

	  // Skip invalid storms

	  if (!field_entry->forecast_valid)
	    continue;
	  
	  // If this storm is too far away then skip it

	  if ((distance = _calculateStormDistance(storm_hdr, storm_entry,
						  field_hdr, field_entry))
	      > _radiusOfInfl)
	    continue;
	  
	  // Keep track of the influencing storms

	  if (distance < closest_field_storm_distance)
	  {
	    closest_field_storm = field_entry;
	    closest_field_storm_distance = distance;
	  }
	  
	  influencing_field_storms.push_back(field_entry);
	  
	  // Calculate the weights for the influence of this storm's
	  // motion on the current storm's motion

//	  double weight = 1.0 -
//	    (distance / _radiusOfInfl);
	    
//	  double field_u = field_entry->speed *
//	    sin(field_entry->direction * DEG_TO_RAD);
//	  double field_v = field_entry->speed *
//	    cos(field_entry->direction * DEG_TO_RAD);

//	  double u_sum_added = field_u * weight;
//	  double v_sum_added = field_v * weight;

//	  u_sum += u_sum_added;
//	  v_sum += v_sum_added;
//	  weight_sum += weight;

	  if (_params->debug_level >= Params::DEBUG_EXTRA)
	  {
	    cerr << "   Field storm " << jj << ":" << endl;
	      
	    cerr << "      distance = " << distance << endl;
//	    cerr << "      weight = " << weight << endl;
	    cerr << "      speed = " <<  field_entry->speed << endl;
	    cerr << "      direction = " << field_entry->direction << endl;
//	    cerr << "      field_u = " << field_u << endl;
//	    cerr << "      field_v = " << field_v << endl;
	  }
	    
	} /* endfor - jj */
	
      } /* endfor - field_iter */

      // Make sure there was a storm close enough

      if (closest_field_storm == 0)
      {
	if (_params->debug_level >= Params::DEBUG_EXTRA)
	  cerr << "   No storms found close enough to base storm -- no changes made" << endl;
	
	continue;
      }
      
      // Update the value of each specified field

      for (int field_num = 0; field_num < _params->update_fields_n;
	   ++field_num)
      {
	// Update the storm field value with the field value from the
	// field storm.

	switch (_params->_update_fields[field_num])
	{
	case Params::DAREA_DT :
	  storm_entry->darea_dt = closest_field_storm->darea_dt;
	  break;
	case Params::TOP :
	  storm_entry->top = closest_field_storm->top;
	  break;
	case Params::ALGORITHM_VALUE :
	  storm_entry->algorithm_value = closest_field_storm->algorithm_value;
	  break;
	case Params::INTENSITY_TREND :
	  storm_entry->intensity_trend = closest_field_storm->intensity_trend;
	  break;
	case Params::SIZE_TREND :
	  storm_entry->size_trend = closest_field_storm->size_trend;
	  break;
	} /* endswitch - _params->update_fields[field_num] */
	
      } /* endfor - field_num */
      
//      if (weight_sum > 0.0)
//      {
//	double avg_u = u_sum / weight_sum;
//	double avg_v = v_sum / weight_sum;
	
//	if (avg_u != 0.0 && avg_v != 0.0)
//	  storm_entry->direction = atan2(avg_u, avg_v) * RAD_TO_DEG;
//	else
//	  storm_entry->direction = 0.0;
	
//	storm_entry->speed = sqrt((avg_u * avg_u) + (avg_v * avg_v));
	
//	storm_entry->forecast_valid = TRUE;
	
//	if (_params->debug_level >= Params::DEBUG_EXTRA)
//	{
//	  cerr << endl;
//	  cerr << "   u_sum = " << u_sum << endl;
//	  cerr << "   v_sum = " << v_sum << endl;
//	  cerr << "   weight_sum = " << weight_sum << endl;
//	  cerr << "   avg_u = " << avg_u << endl;
//	  cerr << "   avg_v = " << avg_v << endl;
//	  cerr << "   speed = " << storm_entry->speed << endl;
//	  cerr << "   direction = " << storm_entry->direction << endl;
//	}
	
//      }
//      if (_params->debug_level >= Params::DEBUG_NORM)
//      {
//	cerr << "Storm entry updated:" << endl;
//	cerr << "   weight_sum = " << weight_sum << endl;
//	cerr << "   speed = " <<  storm_entry->speed << endl;
//	cerr << "   direction = " << storm_entry->direction << endl;
//      }
      
    } /* endfor - ii */
    
  } /* endfor - i */
  
}


/**************************************************************
 * _writeOutputDatabase() - Write the updated storms to the output
 *                          database.
 */

void UpdateStormFields::_writeOutputDatabase(void)
{
  static const string method_name = "UpdateStormFields::_writeOutputDatabase()";
  
  // Accumulate the SPDB put buffer

  DsSpdb output_spdb;
  
  output_spdb.setPutMode(Spdb::putModeOver);
  
  vector< Spdb::chunk_t >::iterator storm_iter;
  
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
    tstorm_spdb_buffer_to_BE((ui08 *)storm_iter->data);
  
  // Send the SPDB put buffer to the output URL

  output_spdb.clearPutChunks();
    
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
    output_spdb.addPutChunk(storm_iter->data_type,
			    storm_iter->valid_time,
			    storm_iter->expire_time,
			    storm_iter->len,
			    storm_iter->data,
			    storm_iter->data_type2);
  
  if (output_spdb.put(_params->output_url,
		      SPDB_TSTORMS_ID,
		      SPDB_TSTORMS_LABEL) != 0)
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing data to URL <" <<
	  _params->output_url << ">" << endl;
    }
  }
  else if (_params->debug_level >= Params::DEBUG_NORM)
  {
    cerr << "*** Successfully wrote chunk to URL <" <<
      _params->output_url << ">" << endl;
  }
    
  return;
}
