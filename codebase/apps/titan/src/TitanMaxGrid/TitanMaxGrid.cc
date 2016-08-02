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
//   $Id: TitanMaxGrid.cc,v 1.3 2016/03/04 02:04:38 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * TitanMaxGrid: TitanMaxGrid program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * February 2015
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
#include <euclid/geometry.h>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <Spdb/Product_defines.hh>
#include <toolsa/MemBuf.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "TitanMaxGrid.hh"
#include "Params.hh"

using namespace std;


// Global variables

TitanMaxGrid *TitanMaxGrid::_instance = (TitanMaxGrid *)NULL;


/*********************************************************************
 * Constructor
 */

TitanMaxGrid::TitanMaxGrid(int argc, char **argv) :
  _polygonGrid(0),
  _polygonGridSize(0)
{
  static const string method_name = "TitanMaxGrid::TitanMaxGrid()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (TitanMaxGrid *)NULL);
  
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
  char *params_path = (char *)"unknown";
  
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

TitanMaxGrid::~TitanMaxGrid()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  delete _dataTrigger;
  
  _clean();
  
  delete [] _polygonGrid;
  _polygonGrid = 0;
  _polygonGridSize = 0;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

TitanMaxGrid *TitanMaxGrid::Inst(int argc, char **argv)
{
  if (_instance == (TitanMaxGrid *)NULL)
    new TitanMaxGrid(argc, argv);
  
  return(_instance);
}

TitanMaxGrid *TitanMaxGrid::Inst()
{
  assert(_instance != (TitanMaxGrid *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool TitanMaxGrid::init()
{
  static const string method_name = "TitanMaxGrid::init()";
  
  // Initialize the data trigger

  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug >= Params::DEBUG_VERBOSE)
      cerr << "Initializing LATEST_DATA trigger using url: " <<
	_params->storms_url << endl;
    
    DsLdataTrigger *trigger = new DsLdataTrigger();
    if (trigger->init(_params->storms_url,
		      -1,
		      PMU_auto_register) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing LATEST_DATA trigger using url: " <<
	_params->storms_url << endl;
      
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
    if (trigger->init(_params->storms_url,
		      start_time, end_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error initializing TIME_LIST trigger for url: " <<
	_params->storms_url << endl;
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

void TitanMaxGrid::run()
{
  static const string method_name = "TitanMaxGrid::run()";
  
  DateTime trigger_time;
  
  while (!_dataTrigger->endOfData())
  {
    // Get the trigger time

    if (_dataTrigger->nextIssueTime(trigger_time) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error getting next trigger time" << endl;
      
      continue;
    }
    
    // Process the data

    if (!_processData(trigger_time.utime()))
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error processing data for time: " << trigger_time << endl;
      
      continue;
    }
    
    // Clean up memory

    _clean();
    
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

bool TitanMaxGrid::_processData(const DateTime &data_time)
{
  if (_params->debug >= Params::DEBUG_WARNINGS)
    cerr << endl << "*** Processing storms for time: " << data_time << endl;
  
  // First read in and process the Titan storms.  This method will
  // fill in the _storms vector for us.

  if (!_readTitanStorms(data_time))
    return false;

  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "    Storms read successfully" << endl;
  
  // Now read the associated gridded data. This method will fill in
  // the _grid member.

  if (!_readGriddedData(data_time))
    return false;
  
  if (_params->debug >= Params::DEBUG_NORM)
    cerr << "    Grid read successfully for time: "
	 << DateTime::str(_grid.getValidTime()) << endl;
  
  // Process the storms

  if (!_processStorms())
    return false;
  
  return true;
}


/*********************************************************************
 * _processStorms() - Process the storms.  When finished, write the
 *                    output to the SPDB database.
 *
 * Returns true on success, false on failure.
 */

bool TitanMaxGrid::_processStorms()
{
  static const string method_name = "TitanMaxGrid::_processStorms()";
  
  // Create the output object

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAdd);
  
  // Get the grid information.  If we get here, we know that _grid
  // has a single field with a single level so we don't need to do
  // any error checking.

  MdvxField *grid_field = _grid.getField(0);
  Mdvx::field_header_t grid_field_hdr = grid_field->getFieldHeader();
  MdvxPjg grid_proj(_grid.getMasterHeader(), grid_field_hdr);
  fl32 *grid_data = (fl32 *)grid_field->getVol();
  
  // Process each of the storms

  vector< Tstorm* >::const_iterator storm_iter;
  for (storm_iter = _storms.begin(); storm_iter != _storms.end();
       ++storm_iter)
  {
    Tstorm *storm = *storm_iter;
    TstormGrid storm_proj = storm->getGrid();
    
    // Convert the storm vertices to the format needed for the
    // EG_fill_polygon routine.  We store the vertices as grid indices
    // so we can do the polygon filling on a subset of our grid for
    // efficiency.

    Polyline *detection_poly = storm->getDetectionPoly();
    Point_d euclid_poly[detection_poly->getNumPts()];
    
    double min_x, min_y, max_x, max_y;
    
    for (int i = 0; i < detection_poly->getNumPts(); ++i)
    {
      double poly_lat, poly_lon;
      storm_proj.xy2latlon(detection_poly->getX(i),
			   detection_poly->getY(i),
			   poly_lat, poly_lon);
      
      int x, y;
      
      grid_proj.latlon2xyIndex(poly_lat, poly_lon,
			       x, y);
      
      double double_x = (double)x;
      double double_y = (double)y;
      
      euclid_poly[i].x = double_x;
      euclid_poly[i].y = double_y;

      if (i == 0)
      {
	min_x = double_x;
	max_x = double_x;
	min_y = double_y;
	max_y = double_y;
      }
      else
      {
	if (min_x > double_x)
	  min_x = double_x;
	if (max_x < double_x)
	  max_x = double_x;
	if (min_y > double_y)
	  min_y = double_y;
	if (max_y < double_y)
	  max_y = double_y;
      }
      
    } /* endfor - i */

    // Get the gridded filled polygon.

    int grid_nx = (int)max_x - (int)min_x + 1;
    int grid_ny = (int)max_y - (int)min_y + 1;
    
    _allocPolygonGrid(grid_nx, grid_ny);

    EG_fill_polygon(euclid_poly,
		    detection_poly->getNumPts(),
		    grid_nx, grid_ny,
		    min_x, min_y,
		    1.0, 1.0,
		    _polygonGrid, 1);
    
    // Loop through the polygon grid looking for the 
    // maximum data value.

    vector< location_t > max_locations;
    double max_value;
    
    for (int y = min_y; y <= max_y; ++y)
    {
      for (int x = min_x; x <= max_x; ++x)
      {
	// If the polygon value is 0, then we are outside
	// of the polygon so skip this point

	int polygon_index = ((y - min_y) * grid_nx) + (x - min_x);
	
	if (_polygonGrid[polygon_index] <= 0.0)
	  continue;
	
	// Now process the grid point

	int grid_index = grid_proj.xyIndex2arrayIndex(x, y);
      
	if (grid_index < 0)
	  continue;
	
	// If the data value is missing, don't do anything

	if (grid_data[grid_index] == grid_field_hdr.bad_data_value ||
	    grid_data[grid_index] == grid_field_hdr.missing_data_value)
	  continue;
	
	// If this is the first data value, just use this one
	// and move on

	if (max_locations.size() == 0)
	{
	  location_t location;
	  grid_proj.xyIndex2latlon(x, y, location.lat, location.lon);
	  
	  max_value = grid_data[grid_index];
	  max_locations.push_back(location);
	  
	  continue;
	}

	// If this data value is less than the mas, do nothing

	if (grid_data[grid_index] < max_value)
	  continue;
	
	// If we get here, then we have a maximum value.  If it's a
	// new maximum, clear out the old locations and save the
	// new value. If it's not new, just add the location to our
	// list.

	if (grid_data[grid_index] > max_value)
	{
	  max_locations.clear();
	  max_value = grid_data[grid_index];
	}
	
	location_t location;
	grid_proj.xyIndex2latlon(x, y, location.lat, location.lon);
	max_locations.push_back(location);

      } /* endfor - x */
    } /* endfor - y */
    
    // Determine the final location to add to the output database

    if (max_locations.size() == 0)
      continue;
    
    size_t location_index = 0;
    
    if (max_locations.size() > 1)
    {
      double centroid_lat, centroid_lon;
      storm->getCentroid(centroid_lat, centroid_lon);
      
      double min_r, theta;
      location_index = 0;
      
      Pjg::latlon2RTheta(centroid_lat, centroid_lon,
			 max_locations[0].lat, max_locations[0].lon,
			 min_r, theta);
      
      for (size_t i = 1; i < max_locations.size(); ++i)
      {
	double r;
	Pjg::latlon2RTheta(centroid_lat, centroid_lon,
			   max_locations[0].lat, max_locations[0].lon,
			   r, theta);

	if (min_r > r)
	{
	  min_r = r;
	  location_index = i;
	}
      } /* endfor - i */
      
    } /* endif - max_locations.size() > 1 */
    
    GenPt gen_pt;
    gen_pt.setLat(max_locations[location_index].lat);
    gen_pt.setLon(max_locations[location_index].lon);
    gen_pt.setTime(storm->getValidTime());
    gen_pt.setId(storm->getSimpleTrack());
    gen_pt.addFieldInfo(grid_field_hdr.field_name,
			grid_field_hdr.units);
    gen_pt.addVal(max_value);
    
    gen_pt.assemble();
    
    spdb.addPutChunk(storm->getSimpleTrack(),
		     storm->getValidTime(),
		     storm->getValidTime() + _params->expire_seconds,
		     gen_pt.getBufLen(),
		     gen_pt.getBufPtr());
    
  } /* endfor - storm_iter */
  
  // Now write the points to the dtabase

  if (spdb.put(_params->output_url,
	       SPDB_GENERIC_POINT_ID,
	       SPDB_GENERIC_POINT_LABEL) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error writing GenPts to URL: " << _params->output_url << endl;
    cerr << spdb.getErrStr() << endl;
    
    return false;
  }
  
  return true;
}


/*********************************************************************
 * _readGriddedData() - Read the gridded data for the given time.
 *
 * Returns true on success, false on failure.  Upon successful return,
 * the _grid member will contain the gridded data.
 */

bool TitanMaxGrid::_readGriddedData(const DateTime &data_time)
{
  static const string method_name = "TitanMaxGrid::_readGriddedData()";
  
  // Set up the read request

  _grid.clearRead();
  
  _grid.setReadTime(Mdvx::READ_FIRST_BEFORE,
		    _params->grid_info.url,
		    _params->grid_info.max_valid_secs,
		    data_time.utime());
  _grid.addReadField(_params->grid_info.field_name);
  if (_params->grid_info.vlevel_index < 0)
    _grid.setReadComposite();
  else
    _grid.setReadPlaneNumLimits(_params->grid_info.vlevel_index,
				_params->grid_info.vlevel_index);
  
  _grid.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  _grid.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  _grid.setReadScalingType(Mdvx::SCALING_NONE);
  
  // Read the volume
  
  if (_grid.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading gridded data for time: " << data_time << endl;
    cerr << _grid.getErrStr() << endl;
    
    return false;
  }
  
  // Check the returned data. There should be exactly one field with one
  // vertical level in the returned volume

  if (_grid.getNFields() != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Expected exactly one field in the read gridded data" << endl;
    cerr << "Found " << _grid.getNFields() << " fields" << endl;
    cerr << "Cannot process data" << endl;
    
    return false;
  }
  
  int num_vlevels = _grid.getField(0)->getFieldHeader().nz;
  if (num_vlevels != 1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Expected exactly one vertical level in the read gridded data" << endl;
    cerr << "Found " << num_vlevels << " vertical levels" << endl;
    cerr << "Cannot process data" << endl;
    
    return false;
  }
  
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

bool TitanMaxGrid::_readTitanStorms(const DateTime &data_time)
{
  static const string method_name = "TitanMaxGrid::_readTitanStorms()";
  
  // Read in the storms from the database

  int num_storm_groups =
    _tstormMgr.readTstorms(_params->storms_url, data_time.utime(), 0);

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

  vector< TstormGroup* > tstorm_groups = _tstormMgr.getGroups();
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
      
      _storms.push_back(curr_storm);
      
    } /* endfor - storm_iter */
    
  } /* endfor - group_iter */

  return true;
}


///*********************************************************************
// * _writePolygons() - Write the polygons to the output database.
// *
// * Returns true on success, false on failure.
// */
//
//bool TitanMaxGrid::_writePolygons(const DateTime &data_time)
//{
//  static const string method_name = "Tstorms2GenPoly::_writePolygons()";
//
//  // Loop through the polygon vector, adding the appropriate data
//  // chunk to spdb object
//
//  DsSpdb gen_poly_db;
//  
//  for (size_t i = 0; i < _polygons.size(); ++i)
//  {
//    GenPoly *polygon = _polygons[i];
//    
//    // Assemble the GenPoly buffer for this polygon
//
//    if (!polygon->assemble())
//    {
//      cerr << "ERROR: " << method_name << endl;
//      cerr << "Error assembling polygon into SPDB buffer" << endl;
//      cerr << polygon->getErrStr() << endl;
//      cerr << "Skipping polygon..." << endl;
//      
//      continue;
//    }
//    
//    int data_type = 0;
//    time_t valid_time = data_time.utime();
//    time_t expire_time = data_time.utime() + _params->expire_seconds;
//    int chunk_len = polygon->getBufLen();
//    const void *chunk_data = polygon->getBufPtr();
//
//    gen_poly_db.addPutChunk(data_type,
//                            valid_time,
//                            expire_time,
//                            chunk_len,
//                            chunk_data);
//
//  } /* endfor - i */
//  
//  // Write the chunks to the SPDB database
//
//  gen_poly_db.setPutMode(Spdb::putModeAdd);
//  
//  if (gen_poly_db.put(_params->output_url,
//		      SPDB_GENERIC_POLYLINE_ID,
//		      SPDB_GENERIC_POLYLINE_LABEL) != 0)
//  {
//    cerr << "ERROR: " << method_name << endl;
//    cerr << "Error writing output to GenPoly SPDB database" << endl;
//    cerr << "Output URL: " << _params->output_url << endl;
//    
//    return false;
//  }
//  
//  return true;
//}
