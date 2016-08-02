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
// TstormSpdb2Ascii.cc
//
// TstormSpdb2Ascii object
//
// Dan Megenhardt, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
// August 2005
// 
//
///////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdio.h>
#include <cstring>

#include <toolsa/os_config.h>
#include <rapmath/math_macros.h>
#include <symprod/spdb_client.h>
#include <symprod/spdb_products.h>
#include <rapformats/tstorm_hull_smooth.h>
#include <rapformats/tstorm_spdb.h>
#include <toolsa/globals.h>
#include <toolsa/ldata_info.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/str.h>

#include "TstormSpdb2Ascii.hh"
#include "Args.hh"
#include "Params.hh"


// Global variables

TstormSpdb2Ascii *TstormSpdb2Ascii::_instance = (TstormSpdb2Ascii *)NULL;

// Global constants

const int Forever = TRUE;


/**************************************************************
 * Constructor
 */

TstormSpdb2Ascii::TstormSpdb2Ascii(int argc, char **argv)
{
  const char *routine_name = "Constructor";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TstormSpdb2Ascii *)NULL);
  
  // Initialize the okay flag.

  okay = true;
  
  // Set the singleton instance pointer

  _instance = this;

  // Set program name

  path_parts_t progname_parts;
  
  uparse_path(argv[0], &progname_parts);
  _programName = STRdup(progname_parts.base);
  
  // display ucopyright message.

  ucopyright(_programName);

  // Get command line args

  _args = new Args(argc, argv, _programName);

  if (!_args->okay)
  {
    fprintf(stderr, "ERROR: %s\n", _programName);
    fprintf(stderr, "Problem with command line args\n");

    okay = false;

    return;
  }

  _archiveStartTime = _args->startTime;
  _archiveEndTime = _args->endTime;
  
  // Get TDRP params

  _params = new Params();
  char *params_path = "unknown";
  
  if (_params->loadFromArgs(argc, argv,
			    _args->override.list,
			    &params_path))
  {
    fprintf(stderr,
	    "ERROR: %s::%s\n", _className(), routine_name);
    fprintf(stderr,
	    "Problem with TDRP parameters in file <%s>\n",
	    params_path);
    
    okay = false;
    
    return;
  }

  // Initialize process registration
  
  if (_params->mode == Params::REALTIME)
    PMU_auto_init(_programName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);
  
  // Initialize latest data handle

  LDATA_init_handle(&_ldataHandle,
		    _programName,
		    FALSE);
  
}


/**************************************************************
 * Destructor
 */

TstormSpdb2Ascii::~TstormSpdb2Ascii()
{
  // Unregister process

  if (_params->mode == Params::REALTIME)
    PMU_auto_unregister();

  // Free strings

  STRfree(_programName);
  
  // Call destructors

  delete _params;
  delete _args;

}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TstormSpdb2Ascii *TstormSpdb2Ascii::Inst(int argc, char **argv)
{
  if (_instance == (TstormSpdb2Ascii *)NULL)
    new TstormSpdb2Ascii(argc, argv);
  
  return(_instance);
}

TstormSpdb2Ascii *TstormSpdb2Ascii::Inst()
{
  assert(_instance != (TstormSpdb2Ascii *)NULL);
  
  return(_instance);
}


/**************************************************************
 * run()
 */

void TstormSpdb2Ascii::run()
{
  // Process new input files forever.
  
  while (Forever)
  {
    PMU_auto_register("Waiting for data");
      
    // Get the storm data
     
    if (!_getStormData())
    {
      if (_params->mode == Params::ARCHIVE)
	return;
      
      continue;
    }
    
    // We have all of the data so we can go ahead and convert
    // to ASCII

    _convertToAscii();
    
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
 * Returns TRUE if storm data was successfully retrieved,
 * FALSE otherwise.
 *
 * Upon successful return, _stormDataNchunks, _stormDataChunkHdrs
 * and _stormData are correctly set.
 */

int TstormSpdb2Ascii::_getStormData(void)
{
  if (_params->mode == Params::REALTIME)
  {
    // Wait for new storm data

    LDATA_info_read_blocking(&_ldataHandle,
			     _params->input_dir,
			     _params->max_valid_age,
			     _params->sleep_msecs,
			     PMU_auto_register);
    
    // Read in the storm data

    if (_retrieveStormData(_ldataHandle.info.latest_time))
      return(TRUE);

    return(FALSE);
    
  } /* end - REALTIME mode */
  else
  {
    time_t data_time;
    
    // Read in the proper storm data

    data_time = _retrieveArchiveStormData(_archiveStartTime,
					  _archiveEndTime);

    // See if we've processed all of the data

    if (data_time <= 0)
      return(FALSE);
    
    // Set the start time for the next time around

    _archiveStartTime = data_time + 1;

    return(TRUE);
      
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
 * Upon successful return, _stormDataNchunks, _stormDataChunkHdrs
 * and _stormData are correctly set.
 */

time_t TstormSpdb2Ascii::_retrieveArchiveStormData(time_t start_time,
						     time_t end_time)
{
  si32 time_margin = end_time - start_time + 1;
  
  if (SPDB_get_first_after(_params->input_dir,
			   SPDB_TSTORMS_ID,
			   0,
			   start_time,
			   time_margin,
			   &_stormDataNchunks,
			   &_stormDataChunkHdrs,
			   (void **)&_stormData) != 0)
  {
    fprintf(stderr,
	    "ERROR - ERROR retrieving archive storm data from <%s> between times %s and %s\n",
	    _params->input_dir,
	    utimstr(start_time),
	    utimstr(end_time));
    return(-1);
  }
    
  if (_stormDataNchunks <= 0)
    return(-1);
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr,
	    "Successfully retrieved %d chunks of archive storm data time %s\n",
	    _stormDataNchunks,
	    utimstr(_stormDataChunkHdrs[0].valid_time));
	  
  tstorm_spdb_buffer_from_BE((ui08 *)_stormData);
  
  return(_stormDataChunkHdrs[0].valid_time);
}


/**************************************************************
 * _retrieveStormData() - Retrieve the indicated storm data from 
 *                        the storm database.
 *
 * Returns TRUE if the storm data was successfully retrieved,
 * FALSE otherwise.
 *
 * Upon successful return, _stormDataNchunks, _stormDataChunkHdrs
 * and _stormData are correctly set.
 */

int TstormSpdb2Ascii::_retrieveStormData(time_t data_time)
{
  if (SPDB_get(_params->input_dir,
	       SPDB_TSTORMS_ID,
	       0,
	       data_time,
	       &_stormDataNchunks,
	       &_stormDataChunkHdrs,
	       (void **)&_stormData) != 0)
  {
    fprintf(stderr,
	    "ERROR - ERROR retrieving storm data from <%s> for time %s\n",
	    _params->input_dir,
	    utimstr(data_time));
    return(FALSE);
  }
    
  if (_stormDataNchunks <= 0)
    return(FALSE);
  
  if (_params->debug_level >= Params::DEBUG_NORM)
    fprintf(stderr,
	    "Successfully retrieved %d chunks of storm data for time %s\n",
	    _stormDataNchunks,
	    utimstr(_stormDataChunkHdrs[0].valid_time));
	  
  tstorm_spdb_buffer_from_BE((ui08 *)_stormData);
  
  return(TRUE);
}


/**************************************************************
 * _convertToAscii() - Convert the storm data to ASCII format
 *                     and write the data to the output directory.
 */

void TstormSpdb2Ascii::_convertToAscii(void)
{
  static char *routine_name = "_convertToAscii";
  
  char filename[MAX_PATH_LEN];
  FILE *output_file;
  
  // Process each chunk in the storm data.  Each chunk should
  // represent a different data time.

  for (ui32 chunk = 0; chunk < _stormDataNchunks; chunk++)
  {
      tstorm_spdb_header_t *header =
	  (tstorm_spdb_header_t *)(_stormData + _stormDataChunkHdrs[chunk].offset);
    
  // Open the output file

      date_time_t *file_time = udate_time(header->time);
    
      sprintf(filename, "%s/%04d%02d%02d_%02d%02d%02d.%s",
	      _params->output_dir,
	      file_time->year,
	      file_time->month,
	      file_time->day,
	      file_time->hour,
	      file_time->min,
	      file_time->sec,
	      _params->output_ext);
    
      if ((output_file = fopen(filename, "w")) == (FILE *)NULL)
      {
	  fprintf(stderr,
		  "ERROR: %s::%s()\n",
		  _programName, routine_name);
	  fprintf(stderr,
		  "Error opening file <%s> for writing\n", filename);
	  fprintf(stderr,
		  "Skipping to next storm chunk\n");
      
	  continue;
      }

      if (_params->debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr,
		  "Writing storms to file <%s>:\n", filename);
    
    // Output the header

	if (_params->print_header)
	    _printHeader(output_file);
    
      for(int fcst = 0; fcst < _params->forecast_lead_times_n; fcst++) 
      {

      if (_params->debug_level >= Params::DEBUG_NORM)
	  fprintf(stderr,
		  "Processing storms with lead time of %ld seconds\n", _params->_forecast_lead_times[fcst]);
	  
	for (int i = 0; i < header->n_entries; i++)
	{
	    tstorm_spdb_entry_t *entry =
		(tstorm_spdb_entry_t *)((char *)header +
					sizeof(tstorm_spdb_header_t) +
					(i * sizeof(tstorm_spdb_entry_t)));

	    if (!_params->valid_forecasts_only ||
		entry->forecast_valid)
	    {
	    /*
	     * Output the storm information to the ASCII file.  The storm
	     * information has the following format:
	     *
	     *     STORM <valid time, optional>
	     *     <forecast period> <num vertices> <centroid lat> <centroid lon> <speed> <direction> <top>
	     *     <vertex 1 lat> <vertex 1 lon>
	     *         .
	     *         .
	     *         .
	     *     <vertex n lat> <vertex n lon>
	     *     DETECTION
	     *     <num vertices> <centroid lat> <centroid lon>
	     *     <vertex 1 lat> <vertex 1 lon>
	     *         .
	     *         .
	     *         .
	     *     <vertex n lat> <vertex n lon>
	     *
	     * Notes:
	     *
	     *     - the given polygon is the storms position extrapolated
	     *       <forecast period> seconds.
	     *     - <speed> is in knots.
	     *     - <direction> is in degrees, North is 0 degress, degrees
	     *       increase clockwise.
	     *     - <top> is in 100's of feet.
	     */
      
	    // Create the forecast polygon


	    tstorm_polygon_t polygon;
	    int polygon_pts;
	
	    if (_params->smooth_polygons)
	    {
		tstorm_hull_smooth(header, entry,
				   _params->smooth_inner_multiplier,
				   _params->smooth_outer_multiplier,
				   &polygon, &polygon_pts,
				   _params->_forecast_lead_times[fcst],
				   _params->debug_level >= Params::DEBUG_EXTRA);
	    }
	    else
	    {
		tstorm_spdb_load_polygon(header, entry,
					 &polygon,
					 _params->_forecast_lead_times[fcst]);

		polygon_pts = header->n_poly_sides + 1;
	    }
	
	// Convert the necessary values.

	    double speed_knots = entry->speed / MPERSEC_TO_KMPERHOUR * MS_TO_KNOTS;
	    double top_100s_ft = entry->top / KM_PER_MI * FT_PER_MI / 100.0;
      
	    double forecast_lat;
	    double forecast_lon;
	
	    PJGLatLonPlusRTheta(entry->latitude, entry->longitude,
				entry->speed *
				((double)_params->_forecast_lead_times[fcst] / 3600.0),
				entry->direction,
				&forecast_lat, &forecast_lon);
	
	// Output the forecast header information

	    if (_params->output_valid_time)
		fprintf(output_file, "STORM %d\n", header->time);
	    else
		fprintf(output_file, "STORM\n");

	    fprintf(output_file, "%ld %d ",
		    _params->_forecast_lead_times[fcst],
		    polygon_pts);
	    fprintf(output_file, "%f %f ",
		    forecast_lat, forecast_lon);
	    fprintf(output_file, "%f %f ",
		    speed_knots, entry->direction);
	    fprintf(output_file, "%f\n",
		    top_100s_ft);
      
	    if (_params->debug_level >= Params::DEBUG_EXTRA)
	    {
		fprintf(stderr, "STORM\n");
		fprintf(stderr, "%ld %d ",
			_params->_forecast_lead_times[fcst],
			polygon_pts);
		fprintf(stderr, "%f %f ",
			forecast_lat, forecast_lon);
		fprintf(stderr, "%f %f ",
		  speed_knots, entry->direction);
		fprintf(stderr, "%f\n",
			top_100s_ft);
	    }
	
	// Output the forecast polygon vertices

	    for (int j = 0; j < polygon_pts; j++)
	    {
		fprintf(output_file, "  %f %f\n",
			polygon.pts[j].lat, polygon.pts[j].lon);
	
		if (_params->debug_level >= Params::DEBUG_EXTRA)
		    fprintf(stderr, "  %f %f\n",
			    polygon.pts[j].lat, polygon.pts[j].lon);
	    } /* endfor - j */

	// Output the detection header

	    fprintf(output_file, "DETECTION\n");
      
	    fprintf(output_file, "%d %f %f\n",
		    polygon_pts,
		    entry->latitude, entry->longitude);
      
	    if (_params->debug_level >= Params::DEBUG_EXTRA)
	    {
		fprintf(stderr, "DETECTION\n");
		fprintf(stderr, "%d %f %f\n",
			polygon_pts,
			entry->latitude, entry->longitude);
	    }
	
	// Create the detection polygon

	    if (_params->smooth_polygons)
	    {
		tstorm_hull_smooth(header, entry,
				   _params->smooth_inner_multiplier,
				   _params->smooth_outer_multiplier,
				   &polygon, &polygon_pts,
				   0,
				   _params->debug_level >= Params::DEBUG_EXTRA);
	    }
	    else
	    {
		tstorm_spdb_load_polygon(header, entry,
					 &polygon, 0);

		polygon_pts = header->n_poly_sides + 1;
	    }
      
	// Output the detection polygon vertices

	    for (int j = 0; j < polygon_pts; j++)
	    {
		fprintf(output_file, "  %f %f\n",
			polygon.pts[j].lat, polygon.pts[j].lon);
	
		if (_params->debug_level >= Params::DEBUG_EXTRA)
		    fprintf(stderr, "  %f %f\n",
			    polygon.pts[j].lat, polygon.pts[j].lon);
	    } /* endfor - j */
	
	} /* endif - valid forecast check */
      
      } /* endfor - i */
    
      fprintf(output_file, "\n");
    
      if (_params->debug_level >= Params::DEBUG_EXTRA)
	  fprintf(stderr, "\n");
    
  // Close the output file
    
      } /* endfor - fcst */
      
      fclose(output_file);

  } /* endfor - chunk */
    
  return;
}


/**************************************************************
 * _printHeader() - Print the file header to the given stream.
 */

void TstormSpdb2Ascii::_printHeader(FILE *stream)
{
  fprintf(stream,
	  "# The storm information has the following format:\n");
  fprintf(stream,
	  "#\n");

  if (_params->output_valid_time)
    fprintf(stream,
	    "#      STORM <generation time in seconds since Jan 1, 1970>\n");
  else
    fprintf(stream,
	    "#      STORM\n");

  fprintf(stream,
	  "#      <forecast period> <num vertices> <centroid lat> <centroid lon> <speed> <direction> <top>\n");
  fprintf(stream,
	  "#     <vertex 1 lat> <vertex 1 lon>\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#     <vertex n lat> <vertex n lon>\n");
  fprintf(stream,
	  "#     DETECTION\n");
  fprintf(stream,
	  "#     <num vertices> <centroid lat> <centroid lon>\n");
  fprintf(stream,
	  "#     <vertex 1 lat> <vertex 1 lon>\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#         .\n");
  fprintf(stream,
	  "#     <vertex n lat> <vertex n lon>\n");
  fprintf(stream,
	  "#\n");
  fprintf(stream,
	  "# Notes:\n");
  fprintf(stream,
	  "#\n");
  fprintf(stream,
	  "#     - the given polygon is the storms position extrapolated\n");
  fprintf(stream,
	  "#       <forecast period> seconds.\n");
  fprintf(stream,
	  "#     - <speed> is in knots.\n");
  fprintf(stream,
	  "#     - <direction> is in degrees, North is 0 degress, degrees\n");
  fprintf(stream,
	  "#       increase clockwise.\n");
  fprintf(stream,
	  "#     - <top> is in 100's of feet.\n");

  return;
}
