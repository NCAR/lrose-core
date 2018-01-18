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
// TstormReflFcst.cc
//
// TstormReflFcst object
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// April 1998
//
///////////////////////////////////////////////////////////////

#include <cstdio>
#include <cstring>

#include <toolsa/os_config.h>
#include <euclid/geometry.h>
#include <euclid/point.h>
#include <rapformats/tstorm_hull_smooth.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/globals.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TstormReflFcst.hh"
#include "Args.hh"
#include "Params.hh"
using namespace std;


const int Forever = TRUE;


/**************************************************************
 * Constructor
 */

TstormReflFcst::TstormReflFcst(int argc, char **argv) :
  _polygonGrid(0),
  _polygonGridAlloc(0)
{
  okay = TRUE;

  // Set program name

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _programName = STRdup(progname_parts.base);
  
  // Get command line args

  _args = new Args(argc, argv, _programName);

  if (!_args->okay)
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with command line args\n");

    okay = FALSE;

    return;
  }

  _archiveStartTime = _args->startTime.utime();
  _archiveEndTime = _args->endTime.utime();
  
  // Get TDRP params

  _params = new Params();
  char *params_path = (char *)"unknown";

  if(_params->loadFromArgs(argc, argv,
			   _args->override.list,
			   &params_path))
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with TDRP parameters\n");

    okay = FALSE;

    return;
  }

  // Check any TDRP param values that can't be checked by TDRP
  // itself.

  if (_params->forecast_durations_n < 1)
  {
    fprintf(stderr,
	    "ERROR: %s\n", _programName);
    fprintf(stderr,
	    "No forecast durations specified.  Set forecast_durations in parameter file and try again.\n");
    okay = FALSE;
    return;
  }
  
  if (_params->mode == Params::ARCHIVE &&
      (_archiveStartTime < 0 || _archiveEndTime < 0))
  {
    fprintf(stderr,
	    "ERROR: %s\n", _programName);
    fprintf(stderr,
	    "-starttime and -endtime MUST be specified in ARCHIVE mode\n");
    okay = FALSE;
    return;
  }
  
  // Initialize process registration
  
  if (_params->mode == Params::REALTIME)
    PMU_auto_init(_programName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
  
  // Initialize latest data handle used for triggering processing

  _stormLdata.setDirFromUrl(_params->storm_data_url);
  
  // Initialize constant read request parameters

  _griddedMdvx.clearReadFields();

  if (_params->use_gridded_field_num)
    _griddedMdvx.addReadField(_params->gridded_field_num);
  else
    _griddedMdvx.addReadField(_params->gridded_field_name);
  
  _griddedMdvx.setReadComposite();

  if (_params->set_vlevel_limits) {
    _griddedMdvx.setReadVlevelLimits(_params->lower_vlevel,
				     _params->upper_vlevel);
  }
  _griddedMdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _griddedMdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _griddedMdvx.setReadScalingType(Mdvx::SCALING_NONE);
}


/**************************************************************
 * Destructor
 */

TstormReflFcst::~TstormReflFcst()
{
  // Unregister process

  if (_params->mode == Params::REALTIME)
    PMU_auto_unregister();

  // Free strings

  STRfree(_programName);
  
  // Free other memory

  ufree(_polygonGrid);
  
  // Call destructors

  delete _params;
  delete _args;

}


/**************************************************************
 * run()
 */

void TstormReflFcst::run()
{
  // Process new input files forever.
  
  while (Forever)
  {
    PMU_auto_register("Waiting for data");
      
    if (_params->debug_level >= Params::DEBUG_EXTRA)
      fprintf(stderr, "\n");
    
    // Get the storm data
     
    time_t storm_time;
      
    if ((storm_time = _getStormData()) < 0)
    {
      if (_params->mode == Params::ARCHIVE)
	return;
      
      continue;
    }
    
    // Get the matching gridded data

    if (!_getGriddedData(storm_time))
      continue;
      
    // We have all of the data so we can go ahead and
    // calculate the forecast

    _generateForecast();
    
  } /* endwhile - Forever */
    
  return;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/**************************************************************
 * _getStormData() - Wait for new storm data and fill an internal
 *                   structure when it arrives.
 *
 * Returns the data time of the storm data in UNIX format if
 * storm data was successfully retrieved, -1 otherwise.
 *
 * Upon successful return, _stormSpdb contains the new storm
 * data retrieved from the database, byte-swapped to native
 * format.
 */

time_t TstormReflFcst::_getStormData(void)
{
  if (_params->mode == Params::REALTIME)
  {
    // Wait for new storm data

    _stormLdata.readBlocking(_params->max_valid_age,
			     _params->sleep_msecs,
			     PMU_auto_register);
    
    // Read in the storm data

    if (_retrieveStormData(_stormLdata.getLatestTime()))
      return(_stormLdata.getLatestTime());

    return(-1);

  } /* end - REALTIME mode */
  else
  {
    time_t data_time;
    
    // Read in the proper storm data

    data_time = _retrieveArchiveStormData(_archiveStartTime,
					  _archiveEndTime);
    _archiveStartTime = data_time + 1;

    return(data_time);
    
  } /* end - ARCHIVE mode */

  return(-1);
}


/**************************************************************
 * _getGriddedData() - Get the gridded data matching the time
 *                     of the storm data.
 *
 * Returns TRUE if gridded data was successfully retrieved,
 * FALSE otherwise.
 *
 * Upon successful return, _griddedMdvx has the new gridded
 * data.
 */

int TstormReflFcst::_getGriddedData(time_t storm_data_time)
{
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr,
	    "Trying to find gridded data matching time %s\n",
	    utimstr(storm_data_time));
  
  if (_params->mode == Params::REALTIME)
  {
    // Wait for new gridded data.  Note that we'll return from
    // within this loop if we don't receive gridded data within
    // _params->time_offset_max seconds of receiving the
    // storm data.

    while (Forever)
    {
      // See if the latest data matches the storm data

      if (_retrieveClosestGriddedData(storm_data_time,
				      _params->time_offset_max))
      {
	if (_params->debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr,
		  "Successfully read gridded data\n");
	
	return true;
      }
      else
      {
	// Let the user know we couldn't get the data

	if (_params->debug_level >= Params::DEBUG_EXTRA)
	  fprintf(stderr,
		  "ERROR - Error reading gridded data file matching storm data file from URL: %s\n",
		  _params->gridded_data_url);

	// See if we've timed out waiting for the data

	if (time(NULL) - storm_data_time > _params->time_offset_max)
	{
	  if (_params->debug_level >= Params::DEBUG_NORM)
	    fprintf(stderr,
		    "Gridded data too late -- skipping\n");
	
	  return FALSE;
	}
      
	// Sleep for a little while and then try again

	PMU_auto_register("Waiting for gridded data");
	sleep(1);
      }
	
    } /* endwhile - Forever */
      
  } /* end - REALTIME mode */
  else
  {
    if (_params->debug_level >= Params::DEBUG_NORM)
      fprintf(stderr,
	      "Getting matching CLOSEST archive gridded data\n");
	
    return(_retrieveClosestGriddedData(storm_data_time,
				       _params->time_offset_max));

  } /* end - ARCHIVE mode */

  return(FALSE);
  
}


/**************************************************************
 * _retrieveArchiveStormData() - Retrieve the earliest storm
 *                               data in the given time range
 *                               from the storm database.
 *
 * Returns the data time of the storm data if the storm data was
 * successfully retrieved, -1 otherwise.
 *
 * Upon successful return, _stormSpdb is updated with the latest
 * data read from the database, byte-swapped to native format.
 */

time_t TstormReflFcst::_retrieveArchiveStormData(time_t start_time,
						     time_t end_time)
{
  si32 time_margin = end_time - start_time + 1;
  
  if (_stormSpdb.getFirstAfter(_params->storm_data_url,
			       start_time,
			       time_margin) != 0)
  {
    fprintf(stderr,
	    "ERROR - ERROR retrieving archive storm data from <%s> between times %s and %s\n",
	    _params->storm_data_url,
	    utimstr(start_time),
	    utimstr(end_time));
    return -1;
  }
    
  _stormChunks = _stormSpdb.getChunks();
  
  if (_stormSpdb.getNChunks() <= 0)
    return -1;
  
  if (_params->debug_level >= Params::DEBUG_NORM)
  {
    fprintf(stderr,
	    "Successfully retrieved %d chunks of archive storm data time %s\n",
	    _stormSpdb.getNChunks(),
	    utimstr(_stormChunks[0].valid_time));
  }
  
  // Byte swap the data, if necessary, to get native format.

  vector< Spdb::chunk_t >::const_iterator storm_iter;
  
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
    tstorm_spdb_buffer_from_BE((ui08 *)storm_iter->data);
  
  return _stormChunks[0].valid_time;
}


/**************************************************************
 * _retrieveClosestGriddedData() - Retrieve the gridded data
 *                                 closest to the indicated time,
 *                                 within the indicated time
 *                                 margin, from the gridded data
 *                                 directory.
 *
 * Returns TRUE if the gridded data was successfully retrieved,
 * FALSE otherwise.
 *
 * Upon successful return, _griddedMdvx has the new gridded data.
 */

bool TstormReflFcst::_retrieveClosestGriddedData(const time_t data_time,
						 const int time_margin)
{
  const string method_name = "TstormReflFcst::_retrieveClosestGriddedData()";
  
  // Read the data file

  _griddedMdvx.setReadTime(Mdvx::READ_CLOSEST,
			   _params->gridded_data_url,
			   time_margin,
			   data_time);
  
  if (_griddedMdvx.readVolume() != 0)
  {
    if (_params->debug_level >= Params::DEBUG_WARNINGS)
    {
      fprintf(stderr,
	      "ERROR: %s\n", method_name.c_str());
      fprintf(stderr,
	      "Error reading gridded data from URL: %s\n",
	      _params->gridded_data_url);
    }
    
    return false;
  }
  
  // Set the gridded data projection information

  _griddedProj.init(_griddedMdvx);
  
  return true;
}


/**************************************************************
 * _retrieveStormData() - Retrieve the indicated storm data from 
 *                        the storm database.
 *
 * Returns TRUE if the storm data was successfully retrieved,
 * FALSE otherwise.
 *
 * Upon successful return, _stormSpdb and _stormChunks are set.
 */

int TstormReflFcst::_retrieveStormData(time_t data_time)
{
  if (_stormSpdb.getExact(_params->storm_data_url,
			  data_time) != 0)
  {
    fprintf(stderr,
	    "ERROR - ERROR retrieving storm data from <%s> for time %s\n",
	    _params->storm_data_url,
	    utimstr(data_time));
    return FALSE;
  }
    
  _stormChunks = _stormSpdb.getChunks();
  
  if (_stormSpdb.getNChunks() <= 0)
    return FALSE;
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr,
	    "Successfully retrieved %d chunks of storm data for time %s\n",
	    _stormSpdb.getNChunks(),
	    utimstr(_stormChunks[0].valid_time));
	  
  // Byte swap the data, if necessary, to get native format.

  vector< Spdb::chunk_t >::const_iterator storm_iter;
  
  for (storm_iter = _stormChunks.begin();
       storm_iter != _stormChunks.end();
       ++storm_iter)
    tstorm_spdb_buffer_from_BE((ui08 *)storm_iter->data);
  
  return TRUE;
}


/**************************************************************
 * _generateForecast() - Generate the reflectivity forecast
 *                       and write the output file.
 */

bool TstormReflFcst::_generateForecast(void)
{
  const string method_name = "TstormReflFcst::_generateForecast()";
  
  // Generate the map for each forecast duration

  int num_forecasts;
  
  if (_params->file_time_stamp == Params::GENERATE_TIME)
    num_forecasts = 1;
  else
    num_forecasts = _params->forecast_durations_n;
  
  time_t scan_time = _stormChunks[0].valid_time;

  tstorm_spdb_header_t *header =
    (tstorm_spdb_header_t *)_stormChunks[0].data;
  tstorm_spdb_entry_t *entries =
    (tstorm_spdb_entry_t *)((char *)_stormChunks[0].data +
			    sizeof(tstorm_spdb_header_t));
    
  for (int i = 0; i < num_forecasts; i++)
  {
    long   forecast_secs = _params->_forecast_durations[i];

    if (_params->debug_level >= Params::DEBUG_NORM)
      cerr << "Calculating " << forecast_secs << " second forecast...." << endl;

    // Compute the forecast time

    time_t forecast_time = scan_time + forecast_secs;
    
    if (_params->round_forecast_times)
    {
      forecast_time =
	(time_t)((si32)floor((double)forecast_time /
			     (double)_params->rounding_interval + 0.5) *
		 _params->rounding_interval);
    }
    
    // Initialize the output objects

    DsMdvx forecast_file;
    MdvxField *forecast_field;

    switch (_params->file_time_stamp)
    {
      case Params::GENERATE_TIME :
        _setMasterHeader(forecast_file, scan_time, forecast_time, false);
        forecast_field = _createOutputField(scan_time, forecast_secs, false);
        break;

      case Params::FORECAST_TIME :
        _setMasterHeader(forecast_file, forecast_time, forecast_time, false);
        forecast_field = _createOutputField(forecast_time, 0, false);
        break;

      case Params::FORECAST_DIR :
        forecast_file.setWriteAsForecast();
        _setMasterHeader(forecast_file, scan_time, forecast_time, true);
        forecast_field = _createOutputField(scan_time, forecast_secs, true);
        break;
        
      default:
        cerr << "ERROR: " << method_name << endl;
        cerr << "Invalid file_time_stamp value encountered" << endl;
        return false;
    } /* endswitch - _params->file_time_stamp */

    // Compute the appropriate forecast

    bool forecast_return;
    
    if (_params->thresholded_forecast)
      forecast_return =
	_thresholdedForecast(*forecast_field,
			     header, entries,
			     forecast_secs);
    else
      forecast_return =
	_unthresholdedForecast(*forecast_field,
			       header, entries,
			       forecast_secs);
    
    if (!forecast_return)
      break;
    
    // Write the output file

    forecast_field->convertType(Mdvx::ENCODING_INT16,
				Mdvx::COMPRESSION_RLE,
				Mdvx::SCALING_DYNAMIC);
    
    forecast_file.addField(forecast_field);
    
    forecast_file.setWriteLdataInfo();
    forecast_file.writeToDir(_params->output_url);
    
  } /* endfor - i */
  
  return true;
}


/**************************************************************
 * _thresholdedForecast() - Generate the thresholded reflectivity
 *                          forecast.
 *
 * Returns true if successful, false otherwise.
 */

bool TstormReflFcst::_thresholdedForecast(MdvxField &output_field,
					  const tstorm_spdb_header_t *header,
					  const tstorm_spdb_entry_t *entries,
					  const int forecast_secs)
{
  // Get a pointer to the output data volume

  fl32 *map_data = (fl32 *)output_field.getVol();
  
  // Get a pointer to the gridded data

  MdvxField *gridded_field = _griddedMdvx.getField(0);
  fl32 *gridded_data = (fl32 *)gridded_field->getVol();

  // Convert the lead time from seconds to hours since that's what we need
  // for our calculations
  
  double lead_time_hr = (double)forecast_secs/3600.0;
      
  // Forecast each storm entry

  bool valid_forecast_found = false;
  
  for (int entry = 0; entry < header->n_entries; entry++)
  {
    // Don't include invalid forecasts

    if(_params->valid_forecasts_only &&
       !entries[entry].forecast_valid)
    {
      if (_params->debug_level >= Params::DEBUG_EXTRA)
	fprintf(stderr,
		"*** Skipping invalid storm\n");
      
      continue;
    }
    
    valid_forecast_found = true;
    
    // Calculate rate of growth/decay of the storm

    double delta_area;
    double forecast_area;
    
    if (entries[entry].darea_dt < 0.0 && _params->use_decay_trending)
    {
      // Linear trend for decay

      delta_area = entries[entry].darea_dt * lead_time_hr;
    }
    else if(entries[entry].darea_dt > 0.0 && _params->use_growth_trending)
    {
      // Second order trend for growth

      double time_to_zero_growth = 1000.0;
    
      if (lead_time_hr >= time_to_zero_growth)
	delta_area = 0.0;
      else
	delta_area = (entries[entry].darea_dt * lead_time_hr -
		      (entries[entry].darea_dt * lead_time_hr * lead_time_hr) /
		      (time_to_zero_growth * 2.0));
    } 
    else
    {
	delta_area = 0.0;
    }
    
    
    forecast_area = entries[entry].area + delta_area;
    
    if (_params->debug_level >= Params::DEBUG_EXTRA)
      fprintf(stderr,
	      "current area = %f, delta_area = %f, forecast_area = %f\n",
	      entries[entry].area, delta_area, forecast_area);
    
    if (forecast_area <= 0.0)
    {
      if (_params->debug_level >= Params::DEBUG_EXTRA)
	fprintf(stderr,
		"*** Skipping storm: forecast_area <= 0.0\n");
      
      continue;
    }
    
    /*
     * Get the gridded polygon for the storm in the original
     * position.
     */

    // Convert the SPDB polygon to lat/lon

    tstorm_polygon_t current_polygon;
    int num_vertices;
    
    static Point_d *current_vertices = (Point_d *)NULL;
    static int current_vertices_alloc = 0;
    
    if (_params->smooth_polygons)
    {
      tstorm_hull_smooth(header, &entries[entry],
			 _params->smooth_inner_multiplier,
			 _params->smooth_outer_multiplier,
			 &current_polygon, &num_vertices,
			 0.0,
			 _params->debug_level >= Params::DEBUG_EXTRA);
    }
    else
    {
      tstorm_spdb_load_polygon(header, &entries[entry],
			       &current_polygon, 0.0);
      num_vertices = header->n_poly_sides + 1;
    }
    
    if (current_vertices_alloc < num_vertices)
    {
      current_vertices_alloc = num_vertices;
      
      if (current_vertices == (Point_d *)NULL)
	current_vertices = (Point_d *)umalloc(current_vertices_alloc *
					      sizeof(Point_d));
      else
	current_vertices = (Point_d *)urealloc(current_vertices,
					       current_vertices_alloc *
					       sizeof(Point_d));
    }
    
    // Convert the lat/lon polygon to the format needed for the
    // fill_polygon routine.  Get the bounding box while we're
    // here

    double min_x, min_y, max_x, max_y;
    
    for (int i = 0; i < num_vertices; i++)
    {
      int x, y;
      
      _griddedProj.latlon2xyIndex(current_polygon.pts[i].lat,
				  current_polygon.pts[i].lon,
				  x, y);

      current_vertices[i].x = (double)x;
      current_vertices[i].y = (double)y;
      
      if (i == 0)
      {
	min_x = current_vertices[i].x;
	max_x = current_vertices[i].x;
	min_y = current_vertices[i].y;
	max_y = current_vertices[i].y;
      }
      else
      {
	if (min_x > current_vertices[i].x)
	  min_x = current_vertices[i].x;
	if (max_x < current_vertices[i].x)
	  max_x = current_vertices[i].x;
	if (min_y > current_vertices[i].y)
	  min_y = current_vertices[i].y;
	if (max_y < current_vertices[i].y)
	  max_y = current_vertices[i].y;
      }
      
    } /* endfor - i */
    
    // Get the gridded filled polygon.  This is the polygon for the
    // storm in its original position.

    int grid_nx = (int)max_x - (int)min_x + 1;
    int grid_ny = (int)max_y - (int)min_y + 1;
      
    if (_params->debug_level >= Params::DEBUG_EXTRA)
    {
      fprintf(stderr,
	      "Grid: min_x = %f, max_x = %f, min_y = %f, max_y = %f\n",
	      min_x, max_x, min_y, max_y);
      fprintf(stderr,
	      "      grid_nx = %d, grid_ny = %d\n",
	      grid_nx, grid_ny);
    }
      
    // Allocate space for the gridded polygon.

    int grid_size = grid_nx * grid_ny * sizeof(unsigned char);
    
    if (grid_size > _polygonGridAlloc)
    {
      _polygonGridAlloc = grid_size;
	
      if (_polygonGrid == 0)
      {
	_polygonGrid = (unsigned char *)umalloc(_polygonGridAlloc);
      }
      else
      {
	_polygonGrid = (unsigned char *)urealloc(_polygonGrid,
						 _polygonGridAlloc);
      }
    }
    
    memset(_polygonGrid, 0, grid_size);
    
    EG_fill_polygon(current_vertices,
		    num_vertices,
		    grid_nx, grid_ny,
		    min_x, min_y,
		    1.0, 1.0,
		    _polygonGrid,
		    1);
      
    if (_params->debug_level >= Params::DEBUG_EXTRA)
    {
      fprintf(stderr,
	      "Filled polygon (grid_nx = %d, grid_ny = %d:\n",
	      grid_nx, grid_ny);
      for (int y = grid_ny -1; y >= 0; y--)
      {
	fprintf(stderr, "   ");
	  
	for (int x = 0; x < grid_nx; x++)
	{
	  fprintf(stderr,
		  "%2d ", _polygonGrid[(y * grid_nx) + x]);
	} /* endfor - x */
	  
	fprintf(stderr, "\n");
	  
      } /* endfor - y */
    } /* endif - debug */
      
    // Calculate the location and size of the forecasted polygon.
    // length_ratio gives us the growth or decay in the x and y
    // directions for changing the size of the storm.

    double area_ratio = forecast_area / entries[entry].area;
    double length_ratio = sqrt(area_ratio);
    
    // Offset min_x and min_y so the gridded polygon now becomes
    // grid offsets from the centroid of the original storm.

    int storm_x, storm_y;
    
    _griddedProj.latlon2xyIndex(entries[entry].latitude,
				entries[entry].longitude,
				storm_x, storm_y);
    
    min_x -= storm_x;
    min_y -= storm_y;
    
    if (_params->debug_level >= Params::DEBUG_EXTRA)
    {
      fprintf(stderr, "Storm: lat = %f, lon = %f, x = %d, y = %d\n",
	      entries[entry].latitude, entries[entry].longitude,
	      storm_x, storm_y);
      fprintf(stderr, "Offset min_x = %f, min_y = %f\n",
	      min_x, min_y);
    }
    
    // Calculate where the forecast polygon should appear.
    
    int forecast_min_x = (int)ROUND((double)min_x * length_ratio);
    int forecast_min_y = (int)ROUND((double)min_y * length_ratio);

    int forecast_nx = (int)ROUND((double)grid_nx * length_ratio);
    int forecast_ny = (int)ROUND((double)grid_ny * length_ratio);
    
    double forecast_lat, forecast_lon;
    
    PJGLatLonPlusRTheta(entries[entry].latitude, entries[entry].longitude,
			entries[entry].speed * lead_time_hr,
			entries[entry].direction,
			&forecast_lat, &forecast_lon);
    
    int forecast_x, forecast_y;
    
    _griddedProj.latlon2xyIndex(forecast_lat, forecast_lon,
				forecast_x, forecast_y);
    
    if (_params->debug_level >= Params::DEBUG_EXTRA)
    {
      fprintf(stderr,
	      "Forecast: speed = %f, distance = %f, direction = %f, darea_dt = %f, length_ratio = %f\n",
	      entries[entry].speed, entries[entry].speed * lead_time_hr,
	      entries[entry].direction,
	      entries[entry].darea_dt, length_ratio);
      fprintf(stderr,
	      "Forecast loc: lat = %f, lon = %f, x = %d, y = %d\n",
	      forecast_lat, forecast_lon, forecast_x, forecast_y);
      fprintf(stderr,
	      "Forecast grid: min_x = %d, min_y = %d, nx = %d, ny = %d\n",
	      forecast_min_x, forecast_min_y, forecast_nx, forecast_ny);
    }
    
    // Update the map grid with the polygon in its new location

    for (int x_offset = forecast_min_x;
	 x_offset < forecast_min_x + forecast_nx; x_offset++)
    {
      for (int y_offset = forecast_min_y;
	   y_offset < forecast_min_y + forecast_ny; y_offset++)
      {
	int grid_x = (int)min_x +
	  (int)ROUND((double)(x_offset - forecast_min_x) / length_ratio);
	int grid_y = (int)min_y +
	  (int)ROUND((double)(y_offset - forecast_min_y) / length_ratio);
	
	int grid_x_index = grid_x - (int)min_x;
	int grid_y_index = grid_y - (int)min_y;
	
	if (grid_x_index < 0 || grid_x_index >= grid_nx ||
	    grid_y_index < 0 || grid_y_index >= grid_ny)
	  continue;
	
	if (_polygonGrid[(grid_y_index * grid_nx) + grid_x_index] <= 0)
	  continue;
	
	int map_x = forecast_x + x_offset;
	int map_y = forecast_y + y_offset;
	
	if (map_x < 0 || map_x >= _griddedProj.getNx() ||
	    map_y < 0 || map_y >= _griddedProj.getNy())
	  continue;
	
	int map_index = (map_y * _griddedProj.getNx()) + map_x;
	
	int gridded_x = storm_x + grid_x;
	int gridded_y = storm_y + grid_y;
	
	if (gridded_x < 0 || gridded_x >= _griddedProj.getNx() ||
	    gridded_y < 0 || gridded_y >= _griddedProj.getNy())
	  continue;
	
	int gridded_index = (gridded_y * _griddedProj.getNx()) + gridded_x;
	
	if (_params->debug_level >= Params::DEBUG_EXTRA)
	{
	  cerr << "Translation: x_offset = " << x_offset
	       << ", y_offset = " << y_offset
	       << ", grid_x = " << grid_x << ", grid_y = " << grid_y << endl;
	  cerr << "              map_x = " << map_x << ", map_y = " << map_y
	       << ", gridded_x = " << gridded_x
	       << ", gridded_y = " << gridded_y << endl;
	}
	
	double forecast_value = gridded_data[gridded_index];
	
	if (entries[entry].area < _params->max_intensity_trend_size)
	{
	  if ((_params->use_decay_intensity_trending &&
	       entries[entry].darea_dt < 0.0) ||
	      (_params->use_growth_intensity_trending &&
	       entries[entry].darea_dt > 0.0))
	  {
	    double percent_change =
	      entries[entry].darea_dt / entries[entry].area * lead_time_hr;
	    
	    double value_change =
	      fabs(gridded_data[gridded_index] -
		   _params->intensity_base_value) * percent_change;
	    
	    if (_params->negate_intensity_changes)
	      forecast_value =
		gridded_data[gridded_index] - value_change;
	    else
	      forecast_value =
		gridded_data[gridded_index] + value_change;
	    
//	    cerr << "---> original value: " << gridded_data[gridded_index] << endl;
//	    cerr << "     base value: " << _params->intensity_base_value << endl;
//	    cerr << "     percent change: " << percent_change << endl;
//	    cerr << "     value change: " << value_change << endl;
//	    cerr << "     forecast value: " << forecast_value << endl;
	    
	  }
	}
	
	map_data[map_index] = forecast_value;
	
      } /* endfor - y_offset */
      
    } /* endfor - x_offset */
    
    // Print out an ASCII version of the polygon for debugging

    if (_params->debug_level >= Params::DEBUG_EXTRA)
    {
      cerr << "*** ASCII version of polygon ***" << endl;
      
      for (int x_offset = forecast_min_x;
	   x_offset < forecast_min_x + forecast_nx; ++x_offset)
      {
	for (int y_offset = forecast_min_y + forecast_ny - 1;
	     y_offset >= forecast_min_y; --y_offset)
	{
	  int grid_x = (int)min_x +
	    (int)ROUND((double)(x_offset - forecast_min_x) / length_ratio);
	  int grid_y = (int)min_y +
	    (int)ROUND((double)(y_offset - forecast_min_y) / length_ratio);
	
	  int grid_x_index = grid_x - (int)min_x;
	  int grid_y_index = grid_y - (int)min_y;
	
	  if (grid_x_index < 0 || grid_x_index >= grid_nx ||
	      grid_y_index < 0 || grid_y_index >= grid_ny)
	    continue;
	
	  cerr << " " << _polygonGrid[(grid_y_index * grid_nx) + grid_x_index];
	  
	} /* endfor - y_offset */

	cerr << endl;
      } /* endfor - x_offset */
    } /* endif - _params->debug_level >= Params::DEBUG_EXTRA */
    
  } /* endfor - entry */

  if (_params->output_empty_grids)
    return(true);
  else
    return(valid_forecast_found);
}


/**************************************************************
 * _unthresholdedForecast() - Generate the unthresholded
 *                            reflectivity forecast.
 *
 * Returns TRUE if successful, FALSE otherwise.
 */

bool TstormReflFcst::_unthresholdedForecast(MdvxField &output_field,
					    const tstorm_spdb_header_t *header,
					    const tstorm_spdb_entry_t *entries,
					    const int forecast_secs)
{
  static char *routine_name = (char *)"_unthresholdedForecast()";
  
  fprintf(stderr,
	  "ERROR: TstormReflFcst::%s\n",
	  routine_name);
  fprintf(stderr,
	  "Unthresholded forecast not yet implemented\n");
  
  return(false);
}


/**************************************************************
 * _setMasterHeader() - Set the master header values in the
 *                      output file.
 */

void TstormReflFcst::_setMasterHeader(DsMdvx &output_file,
				      const time_t gen_time,
                                      const time_t valid_time,
                                      const bool forecast_file)
{
  // Get the master header from the input file so we can set
  // the output field values based on the input field values.

  Mdvx::master_header_t gridded_master_hdr = _griddedMdvx.getMasterHeader();
  
  // Set the master header values

  Mdvx::master_header_t fcst_master_hdr;
  
  memset(&fcst_master_hdr, 0, sizeof(fcst_master_hdr));

  if (forecast_file)
  {
    fcst_master_hdr.time_gen = gen_time;
    fcst_master_hdr.time_centroid = valid_time;
    fcst_master_hdr.data_collection_type = Mdvx::DATA_FORECAST;
  }
  else
  {
    fcst_master_hdr.time_gen = time((time_t *)NULL);
    fcst_master_hdr.time_centroid = gen_time;
    fcst_master_hdr.data_collection_type = Mdvx::DATA_EXTRAPOLATED;
  }
  
  fcst_master_hdr.time_begin = gen_time +
    _params->output_time_offsets.begin_time_offset;
  fcst_master_hdr.time_end = gen_time +
    _params->output_time_offsets.end_time_offset;
  fcst_master_hdr.time_expire = fcst_master_hdr.time_end;
  fcst_master_hdr.num_data_times = 0;
  fcst_master_hdr.index_number = 0;
  fcst_master_hdr.data_dimension = 2;
  fcst_master_hdr.user_data = 0;
  fcst_master_hdr.native_vlevel_type = gridded_master_hdr.native_vlevel_type;
  fcst_master_hdr.vlevel_type = gridded_master_hdr.vlevel_type;
  fcst_master_hdr.vlevel_included = FALSE;
  fcst_master_hdr.grid_orientation = gridded_master_hdr.grid_orientation;
  fcst_master_hdr.data_ordering = gridded_master_hdr.data_ordering;
  fcst_master_hdr.n_fields = 0;
  fcst_master_hdr.max_nx = _griddedProj.getNx();
  fcst_master_hdr.max_ny = _griddedProj.getNy();
  fcst_master_hdr.max_nz = 1;
  fcst_master_hdr.n_chunks = 0;
  
  fcst_master_hdr.field_grids_differ = FALSE;

  fcst_master_hdr.sensor_lon = gridded_master_hdr.sensor_lon;
  fcst_master_hdr.sensor_lat = gridded_master_hdr.sensor_lat;
  fcst_master_hdr.sensor_alt = gridded_master_hdr.sensor_alt;
  
  STRcopy(fcst_master_hdr.data_set_info,
	  "tstorm_refl_forecast generated",
	  MDV_INFO_LEN);
  STRcopy(fcst_master_hdr.data_set_name,
	  "tstorm_refl_fcst", MDV_NAME_LEN);
  STRcopy(fcst_master_hdr.data_set_source,
	  _griddedMdvx.getPathInUse().c_str(), MDV_NAME_LEN);
  
  output_file.setMasterHeader(fcst_master_hdr);
  
  return;
}
  

/**************************************************************
 * _createOutputField() - Create a new output field.
 */

MdvxField *TstormReflFcst::_createOutputField(const time_t field_time,
					      const int forecast_duration,
                                              const bool forecast_file)
{
  // Get the field header for the input field to use for filling
  // in some of the output field values.

  MdvxField *gridded_field = _griddedMdvx.getField(0);
  Mdvx::field_header_t gridded_field_hdr = gridded_field->getFieldHeader();
  
  // Set the field header values

  Mdvx::field_header_t fcst_field_hdr;
  
  memset(&fcst_field_hdr, 0, sizeof(fcst_field_hdr));
  
  fcst_field_hdr.field_code = 0;
  if (forecast_file)
  {
    fcst_field_hdr.forecast_delta = forecast_duration;
    fcst_field_hdr.forecast_time = field_time + forecast_duration;
  }
  else
  {
    fcst_field_hdr.forecast_delta = 0;
    fcst_field_hdr.forecast_time = 0;
  }
  fcst_field_hdr.nx = gridded_field_hdr.nx;
  fcst_field_hdr.ny = gridded_field_hdr.ny;
  fcst_field_hdr.nz = 1;
  fcst_field_hdr.proj_type = gridded_field_hdr.proj_type;
  fcst_field_hdr.encoding_type = Mdvx::ENCODING_FLOAT32;
  fcst_field_hdr.data_element_nbytes = 4;
  fcst_field_hdr.volume_size = fcst_field_hdr.nx * fcst_field_hdr.ny *
    fcst_field_hdr.data_element_nbytes;
  fcst_field_hdr.compression_type = gridded_field_hdr.compression_type;
  fcst_field_hdr.transform_type = gridded_field_hdr.transform_type;
  fcst_field_hdr.scaling_type = gridded_field_hdr.scaling_type;
  fcst_field_hdr.native_vlevel_type = gridded_field_hdr.native_vlevel_type;
  fcst_field_hdr.vlevel_type = Mdvx::VERT_TYPE_Z;
  fcst_field_hdr.dz_constant = 1;
  
  fcst_field_hdr.proj_origin_lat = gridded_field_hdr.proj_origin_lat;
  fcst_field_hdr.proj_origin_lon = gridded_field_hdr.proj_origin_lon;
  fcst_field_hdr.proj_param[0] = 1.0;
  fcst_field_hdr.vert_reference = 0;
  fcst_field_hdr.vlevel_type = gridded_field_hdr.vlevel_type;
  fcst_field_hdr.grid_dx = gridded_field_hdr.grid_dx;
  fcst_field_hdr.grid_dy = gridded_field_hdr.grid_dy;
  fcst_field_hdr.grid_dz = 1.0;
  fcst_field_hdr.grid_minx = gridded_field_hdr.grid_minx;
  fcst_field_hdr.grid_miny = gridded_field_hdr.grid_miny;
  fcst_field_hdr.grid_minz = 1.0;
  fcst_field_hdr.scale = 1.0;
  fcst_field_hdr.bias = 0.0;
  fcst_field_hdr.bad_data_value = gridded_field_hdr.bad_data_value;
  fcst_field_hdr.missing_data_value = gridded_field_hdr.missing_data_value;
  fcst_field_hdr.proj_rotation = gridded_field_hdr.proj_rotation;
  
  STRcopy(fcst_field_hdr.field_name_long,
	  gridded_field_hdr.field_name_long, MDV_LONG_FIELD_LEN);
  STRcopy(fcst_field_hdr.field_name,
	  gridded_field_hdr.field_name, MDV_SHORT_FIELD_LEN);
  STRcopy(fcst_field_hdr.units, gridded_field_hdr.units, MDV_UNITS_LEN);
  STRcopy(fcst_field_hdr.transform, "tstorm_refl_fcst", MDV_TRANSFORM_LEN);
  
  // Create the field

  MdvxField *fcst_field = new MdvxField(fcst_field_hdr,
					gridded_field->getVlevelHeader(),
					(void *)NULL,
					true);
  
  return fcst_field;
}
