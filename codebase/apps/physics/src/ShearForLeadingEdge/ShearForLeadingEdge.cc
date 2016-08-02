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
//   $Date: 2016/03/06 23:15:37 $
//   $Id: ShearForLeadingEdge.cc,v 1.8 2016/03/06 23:15:37 dixon Exp $
//   $Revision: 1.8 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ShearForLeadingEdge: ShearForLeadingEdge program object.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2005
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <assert.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <string>

#include <toolsa/os_config.h>
#include <dsdata/DsLdataTrigger.hh>
#include <dsdata/DsTimeListTrigger.hh>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>
#include <rapmath/math_macros.h>
#include <Spdb/DsSpdb.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pmu.h>
#include <toolsa/procmap.h>
#include <toolsa/str.h>
#include <toolsa/umisc.h>

#include "ShearForLeadingEdge.hh"
#include "Params.hh"

using namespace std;

// Global variables

ShearForLeadingEdge *ShearForLeadingEdge::_instance =
     (ShearForLeadingEdge *)NULL;


/*********************************************************************
 * Constructor
 */

ShearForLeadingEdge::ShearForLeadingEdge(int argc, char **argv) :
  _dataTrigger(0),
  _uField(0),
  _vField(0),
  _capeField(0)
{
  static const string method_name = "ShearForLeadingEdge::ShearForLeadingEdge()";
  
  // Make sure the singleton wasn't already created.

  assert(_instance == (ShearForLeadingEdge *)NULL);
  
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

ShearForLeadingEdge::~ShearForLeadingEdge()
{
  // Unregister process

  PMU_auto_unregister();

  // Free contained objects

  delete _params;
  delete _args;
  
  delete _dataTrigger;
  
  delete _uField;
  delete _vField;
  delete _capeField;
  
  // Free included strings

  STRfree(_progName);
}


/*********************************************************************
 * Inst() - Retrieve the singleton instance of this class.
 */

ShearForLeadingEdge *ShearForLeadingEdge::Inst(int argc, char **argv)
{
  if (_instance == (ShearForLeadingEdge *)NULL)
    new ShearForLeadingEdge(argc, argv);
  
  return(_instance);
}

ShearForLeadingEdge *ShearForLeadingEdge::Inst()
{
  assert(_instance != (ShearForLeadingEdge *)NULL);
  
  return(_instance);
}


/*********************************************************************
 * init() - Initialize the local data.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool ShearForLeadingEdge::init()
{
  static const string method_name = "ShearForLeadingEdge::init()";
  
  // Initialize the data trigger

  if (!_initTrigger())
    return false;
  
  // Initialize process registration.  Don't register with the
  // process mapper if we are running in an archive mode.

  if (_params->trigger_mode != Params::TIME_LIST)
    PMU_auto_init(_progName, _params->instance,
		  PROCMAP_REGISTER_INTERVAL);

  return true;
}


/*********************************************************************
 * run() - run the program.
 */

void ShearForLeadingEdge::run()
{
  static const string method_name = "ShearForLeadingEdge::run()";
  
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
 * _calcCapeMeans() - Calculate the horizontally averaged CAPE values
 *                    for the points in the template.
 */

void ShearForLeadingEdge::_calcCapeMeans(const vector< GridPoint > &point_list,
					 const int point_x, const int point_y,
					 vector< double > &cape_means) const
{
  Mdvx::field_header_t cape_field_hdr = _capeField->getFieldHeader();
  
  double cape_sum = 0.0;
  int num_cape_values = 0;
  
  for (int z = 0; z < cape_field_hdr.nz; ++z)
  {
    fl32 *cape_data = (fl32 *)_capeField->getPlane(z);
    
    vector< GridPoint >::const_iterator point;
    
    for (point = point_list.begin(); point != point_list.end(); ++point)
    {
      fl32 cape_data_value = cape_data[point->getIndex(cape_field_hdr.nx,
						       cape_field_hdr.ny)];
      
      if (cape_data_value == cape_field_hdr.bad_data_value ||
	  cape_data_value == cape_field_hdr.missing_data_value)
      {
	if (_params->print_missing_value_message)
	  cerr << "WARNING: Missing CAPE data value" << endl;
	
	continue;
      }
      
      cape_sum += cape_data_value;
      ++num_cape_values;
    } /* endfor - point */
    
    double cape_mean;
    
    if (num_cape_values > 0)
      cape_mean = cape_sum / (double)num_cape_values;
    else
      cape_mean = cape_field_hdr.missing_data_value;
    
    cape_means.push_back(cape_mean);
    
  } /* endfor - z */
}


/*********************************************************************
 * _calcInflows() - Calculate the average inflows for each vertical
 *                  level within the vertical shear layer.
 */

void ShearForLeadingEdge::_calcInflows(const int kmin_layer,
				       const int kmax_layer,
				       const double point_u,
				       const double point_v,
				       const int point_x,
				       const int point_y,
				       const vector< GridPoint > &point_list,
				       vector< double > &inflows) const
{
  double edge_angle = atan(point_v / point_u);
  
  Mdvx::field_header_t u_field_hdr = _uField->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = _vField->getFieldHeader();
  
  int nx = u_field_hdr.nx;
  int ny = u_field_hdr.ny;
  
  for (int z = kmin_layer; z <= kmax_layer; ++z)
  {
    fl32 *u_data = (fl32 *)_uField->getPlane(z);
    fl32 *v_data = (fl32 *)_vField->getPlane(z);
    
    vector< GridPoint >::const_iterator point;
    
    double sum_inflow = 0.0;
    int num_points = 0;
    
    for (point = point_list.begin(); point != point_list.end(); ++point)
    {
      int index = point->getIndex(nx, ny);
      
      double u_data_value = u_data[index];
      double v_data_value = v_data[index];
      
      if (u_data_value == u_field_hdr.bad_data_value ||
	  u_data_value == u_field_hdr.missing_data_value ||
	  v_data_value == v_field_hdr.bad_data_value ||
	  v_data_value == v_field_hdr.missing_data_value)
	continue;
      
      double ruc_angle = 0.0;

      if (u_data_value == 0.0 && v_data_value == 0.0)
	ruc_angle = 0.0;
      else if (u_data_value >= 0.0 && v_data_value < 0.0)
	ruc_angle = - (atan(u_data_value / v_data_value) + M_PI_2);
      else if (u_data_value < 0 && v_data_value <= 0.0)
	ruc_angle = atan(v_data_value / u_data_value) - M_PI;
      else if (u_data_value <= 0.0 && v_data_value > 0.0)
	ruc_angle = M_PI_2 - atan(u_data_value / v_data_value);
      else
	ruc_angle = atan(v_data_value / u_data_value);
      
      double ruc_speed = sqrt(u_data_value * u_data_value +
			      v_data_value * v_data_value);
      
      double inflow = ruc_speed * cos(edge_angle - ruc_angle);
      sum_inflow += inflow;
      ++num_points;
      
    } /* endfor - point */
    
    double mean_inflow;
    
    if (num_points > 0)
      mean_inflow = sum_inflow / (double)num_points;
    else
      mean_inflow = 0.0;
    
    inflows[z] = mean_inflow;
    
  } /* endfor - z */
  
}


/*********************************************************************
 * _clearBoundaries() - Clear out the current boundaries
 */

void ShearForLeadingEdge::_clearBoundaries()
{
  _boundaries.erase(_boundaries.begin(), _boundaries.end());
}


/*********************************************************************
 * _findVerticalShearLayer() - Find the vertical shear layer.
 *
 * If the vertical shear layer isn't found, returns -1 for kmin_layer
 * and kamx_layer.
 */

void ShearForLeadingEdge::_findVerticalShearLayer(const vector< double > cape_means,
						  int &kmin_layer,
						  int &kmax_layer,
						  double &zbar_cape) const
{
  Mdvx::field_header_t cape_field_hdr = _capeField->getFieldHeader();
  
  int num_shear_layers;

  if (_params->vertical_shear_depth.use_num_levels)
    num_shear_layers = _params->vertical_shear_depth.num_levels;
  else
    num_shear_layers =
      (int)(_params->vertical_shear_depth.level_depth /
	    fabs(cape_field_hdr.grid_dz));
  
  kmin_layer = -1;
  kmax_layer = -1;
  zbar_cape = cape_field_hdr.missing_data_value;
  
  for (int z = 0; z < cape_field_hdr.nz - num_shear_layers; ++z)
  {
    double layer_cape_sum = 0.0;
    int num_values = 0;
    
    for (int i = z; i <= z + num_shear_layers; ++i)
    {
      if (cape_means[z] == cape_field_hdr.missing_data_value)
	continue;
      
      layer_cape_sum += cape_means[z];
      ++num_values;
      
    } /* endfor - i */
    
    if (num_values == 0)
      continue;
    
    double layer_cape_mean = layer_cape_sum / (double)num_values;
    
    if (zbar_cape == cape_field_hdr.missing_data_value ||
	layer_cape_mean > zbar_cape)
    {
      zbar_cape = layer_cape_mean;
      kmin_layer = z;
      kmax_layer = z + num_shear_layers - 1;
    }
    
  } /* endfor - z */
}


/*********************************************************************
 * _getPointList() - Get the point list for the given point.
 *
 * Returns the calculated point template.
 */

vector< GridPoint > ShearForLeadingEdge::_getPointList(const double point_lat,
						       const double point_lon,
						       const double point_u,
						       const double point_v) const
{
  vector< GridPoint > point_list;
  
  MdvxPjg proj(_capeField->getFieldHeader());
  
  // Calculate the maximum number of grid points away from the
  // current location that would be of interest to us.  Look at a couple
  // of extra grid squares in each direction, just in case.

  int max_grid_pts;

  if (_params->max_edge_normal_distance > _params->max_edge_parallel_distance)
    max_grid_pts = (int)proj.km2xGrid(_params->max_edge_normal_distance);
  else
    max_grid_pts = (int)proj.km2xGrid(_params->max_edge_parallel_distance);
  
  max_grid_pts += 10;
  
  // Now remap the point to an actual grid location so we have a starting
  // point.

  int point_x, point_y;
  
  proj.latlon2xyIndex(point_lat, point_lon, point_x, point_y);
  
  // Loop through the grid points to see which ones to put in the template

  for (int grid_x = point_x - max_grid_pts;
       grid_x <= point_x + max_grid_pts; ++grid_x)
  {
    for (int grid_y = point_y - max_grid_pts;
	 grid_y <= point_y + max_grid_pts; ++grid_y)
    {
      if (grid_x < 0 || grid_y < 0 ||
	  grid_x >= proj.getNx() || grid_y >= proj.getNy())
	continue;
      
      // Get the lat/lon location of the center of the grid point

      double grid_lat, grid_lon;
      
      proj.xyIndex2latlon(grid_x, grid_y, grid_lat, grid_lon);
      
      // Get the distance of the grid point from the leading edge point

      double r, grid_angle;
  
      Pjg::latlon2RTheta(point_lat, point_lon, grid_lat, grid_lon,
			 r, grid_angle);

      // Get the angle between the leading edge normal and the grid point

      double uv_angle;
      
      if (point_v == 0.0)
	uv_angle = 0.0;
      else
	uv_angle = atan(point_u / point_v);
      
      double normal_angle = grid_angle - uv_angle;
      
      // Get the normal and parallel distances

      double normal_distance = r * cos(normal_angle);
      
      if (normal_distance > _params->max_edge_normal_distance)
	continue;
      
      double parallel_distance = r * sin(normal_angle);
      
      if (parallel_distance > _params->max_edge_parallel_distance)
	continue;

      // If we get here, this is a grid point we should process so add
      // it to our template

      point_list.push_back(GridPoint(grid_x, grid_y));
  
    } /* endfor - grid_y */
  } /* endfor - grid_x */
  
  return point_list;
}


/*********************************************************************
 * _initTrigger() - Initialize the data trigger.
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_initTrigger(void)
{
  static const string method_name = "ShearForLeadingEdge::_initTrigger()";
  
  switch (_params->trigger_mode)
  {
  case Params::LATEST_DATA :
  {
    if (_params->debug)
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
    
    if (_params->debug)
    {
      cerr << "Initializing TIME_LIST trigger: " << endl;
      cerr << "   url: " << _params->latest_data_trigger << endl;
      cerr << "   start time: " << start_time << endl;
      cerr << "   end time: " << end_time << endl;
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

  return true;
}


/*********************************************************************
 * _processData() - Process data for the given trigger time.
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_processData(const DateTime &trigger_time)
{
  static const string method_name = "ShearForLeadingEdge::_processData()";
  
  if (_params->debug)
    cerr << endl << "*** Processing data for time: " << trigger_time << endl;
  
  string pmu_string = "Processing data for time: " + trigger_time.getStr();
  
  if (_params->trigger_mode != Params::TIME_LIST) 
    PMU_force_register(pmu_string.c_str());
  
  // Clear out all of the old data

  _clearBoundaries();
  delete _uField;
  _uField = 0;
  delete _vField;
  _vField = 0;
  delete _capeField;
  _capeField = 0;
  
  // Read in the leading edge data

  if (!_readLeadingEdgeData(trigger_time))
    return false;
  
  // Read in the MDV fields

  if (!_readMdvData(trigger_time))
    return false;
  
  // Process each of the points in each of the leading edges

  if (!_processPoints())
    return false;
  
  // Write the output

  DsSpdb spdb;
  spdb.setPutMode(Spdb::putModeAdd);
  
  vector< Bdry >::iterator boundary;
  
  for (boundary = _boundaries.begin(); boundary != _boundaries.end();
       ++boundary)
  {
    boundary->assemble();
    
    if (spdb.put(_params->output_url,
		 SPDB_BDRY_ID,
		 SPDB_BDRY_LABEL,
		 0,
		 boundary->getDataTime().utime(),
		 boundary->getExpireTime().utime(),
		 boundary->getBufLen(),
		 boundary->getBufPtr()) != 0)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Error writing boundary to output URL: "
	   << _params->output_url << endl;

      return false;
    }
  } /* endfor - boundary */

  return true;
}


/*********************************************************************
 * _processPoint() - Calculate the following values for the given point:
 *                     - number of horizontal points going with the leading
 *                       edge point (num_pts)
 *                     - CAPE value in shear layer (zbar_cape)
 *                     - maximum shear in the layer (max_shear)
 *                     - mean shear in the layer (mean_shear)
 *                     - bottom level of shear layer (kmin)
 *                     - top level of shear layer (kmax)
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_processPoint(BdryPoint &point) const
{
  static const string method_name = "ShearForLeadingEdge::_processPoint()";
  
  PMU_auto_register ("Processing point");
  
  BdryPointShearInfo shear_info;
  
  // Find the given point in the model data grid.  If the point is
  // outside of the grid, don't process it.

  int point_x, point_y;
  
  MdvxPjg proj(_uField->getFieldHeader());
  
  proj.latlon2xyIndex(point.getLat(), point.getLon(),
		      point_x, point_y);
  
  // Determine the horizontal grid points to use

  vector< GridPoint > point_list =
    _getPointList(point.getLat(), point.getLon(),
		  point.getUComp(), point.getVComp());
  
  shear_info.setNumPoints(point_list.size());

  // Calculate horizontally averaged CAPE

  vector< double > cape_means;
  
  _calcCapeMeans(point_list, point_x, point_y, cape_means);
  
  // Find vertical shear layer

  int kmin_layer, kmax_layer;
  double zbar_cape;
  
  _findVerticalShearLayer(cape_means, kmin_layer, kmax_layer, zbar_cape);
  shear_info.setZbarCape(zbar_cape);
  
  if (kmin_layer == -1 || kmax_layer == -1)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error finding vertical shear layer" << endl;
    cerr << "Cannot process point" << endl;
    
    return false;
  }
  
  Mdvx::vlevel_header_t cape_vlevel_hdr = _capeField->getVlevelHeader();
  
  shear_info.setKMin(cape_vlevel_hdr.level[kmin_layer]);
  shear_info.setKMax(cape_vlevel_hdr.level[kmax_layer]);
  
  // Calculate wind components for each shear layer

  vector< double > inflows;
  
  for (int z = 0; z < _uField->getFieldHeader().nz; ++z)
    inflows.push_back(0.0);
  
  _calcInflows(kmin_layer, kmax_layer, point.getUComp(), point.getVComp(),
	       point_x, point_y, point_list, inflows);
  
  // Calculate the maximum and mean shear values

  shear_info.setMeanShear(inflows[kmax_layer] - inflows[kmin_layer]);
  
  double max_shear = 0.0;
  
  for (int kk = kmin_layer; kk < kmax_layer; ++kk)
  {
    for (int k = kk; k <= kmax_layer; ++k)
    {
      double inflow_diff = inflows[k] - inflows[kk];
      
      if (inflow_diff > max_shear)
	max_shear = inflow_diff;
    } /* endfor - k */
    
  } /* endfor - kk */
  
  shear_info.setMaxShear(max_shear);
  
  point.setShearInfo(shear_info);
  
  return true;
}


/*********************************************************************
 * _processPoints() - Process all of the points in all of the boundaries.
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_processPoints()
{
  static const string method_name = "ShearForLeadingEdge::_processPoints()";
  
  vector< Bdry >::iterator boundary;
  for (boundary = _boundaries.begin(); boundary != _boundaries.end();
       ++boundary)
  {
    vector< BdryPolyline > &polylines = boundary->getPolylinesEditable();

    if (_params->debug)
      cerr << endl << "Processing polyline:" << endl;
    
    vector< BdryPolyline >::iterator polyline;
    for (polyline = polylines.begin(); polyline != polylines.end();
	 ++polyline)
    {
      vector< BdryPoint > &points = polyline->getPointsEditable();
      
      if (_params->debug)
	cerr << endl << "Processing point:" << endl;
      
      vector< BdryPoint >::iterator point;
      for (point = points.begin(); point != points.end(); ++point)
      {
	if (_params->debug)
	{
	  cerr << "Input point:" << endl;
	  point->print(cerr);
	}
	
	if (!_processPoint(*point))
	{
	  cerr << "ERROR: " << method_name << endl;
	  cerr << "Error processing point:" << endl;
	  point->print(stderr);
	  
	  return false;
	}
	    
	if (_params->debug)
	{
	  cerr << "Output point:" << endl;
	  point->print(cerr);
	}
	
      } /* endfor - point */

      if (_params->debug)
      {
	cerr << "Output polyline:" << endl;
	polyline->print(cerr, true);
      }
      
    } /* endfor - polyline */

  } /* endfor - boundary */

  return true;
}


/*********************************************************************
 * _projectionsMatch() - Compare the projections of the two fields.
 *
 * Returns true if the projections match, false otherwise.
 */

bool ShearForLeadingEdge::_projectionsMatch(const MdvxField &field1,
					    const MdvxField &field2)
{
  static const string method_name = "ShearForLeadingEdge::_projectionsMatch()";
  
  MdvxPjg proj1(field1.getFieldHeader());
  MdvxPjg proj2(field2.getFieldHeader());
  
  if (proj1 != proj2)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << field1.getFieldName() << " and " << field2.getFieldName()
	 << " projections don't match" << endl;
    cerr << field1.getFieldName() << " projection:" << endl;
    proj1.print(cerr);
    cerr << field2.getFieldName() << " projection:" << endl;
    proj2.print(cerr);
    
    return false;
  }
  
  Mdvx::vlevel_header_t vlevel1 = field1.getVlevelHeader();
  Mdvx::vlevel_header_t vlevel2 = field2.getVlevelHeader();
  
  for (int z = 0; z < proj1.getNz(); ++z)
  {
    if (vlevel1.type[z] != vlevel2.type[z] ||
	vlevel1.level[z] != vlevel2.level[z])
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << field1.getFieldName() << " and " << field2.getFieldName()
	   << " Z levels don't match" << endl;
      
      return false;
    }
    
  }
  
  return true;
}


/*********************************************************************
 * _readLeadingEdgeData() - Read in the leading edge data.
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_readLeadingEdgeData(const DateTime &trigger_time)
{
  static const string method_name = "ShearForLeadingEdge::_readLeadingEdgeData()";

  // Read the leading edge data from the SPDB database

  DsSpdb spdb;
  
  if (spdb.getClosest(_params->leading_edge_url,
		      trigger_time.utime(),
		      _params->max_input_valid_secs) != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading leading edge data:" << endl;
    cerr << "   URL: " << _params->leading_edge_url << endl;
    cerr << "   request time: " << trigger_time << endl;
    cerr << "   time margin: " << _params->max_input_valid_secs << endl;
    
    return false;
  }
  
  // Process each of the chunks

  const vector< Spdb::chunk_t > chunks = spdb.getChunks();
  vector< Spdb::chunk_t >::const_iterator chunk;
  
  for (chunk = chunks.begin(); chunk != chunks.end(); ++chunk)
  {
    Bdry boundary;
    
    boundary.disassemble(chunk->data, chunk->len);

    _boundaries.push_back(boundary);
    
  } /* endfor - chunk */
  
  return true;
}


/*********************************************************************
 * _readMdvData() - Read in the MDV data.
 *
 * Returns true on success, false on failure.
 */

bool ShearForLeadingEdge::_readMdvData(const DateTime &trigger_time)
{
  static const string method_name = "ShearForLeadingEdge::_readMdvData()";

  if ((_uField =
       _readMdvField(_params->u_field_info.url,
		     _params->u_field_info.field_name,
		     _params->u_field_info.field_num,
		     trigger_time)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading U field:" << endl;
    cerr << "    URL: " << _params->u_field_info.url << endl;
    cerr << "    field name: " << _params->u_field_info.field_name << endl;
    cerr << "    field number: " << _params->u_field_info.field_num << endl;
    cerr << "    trigger time: " << trigger_time << endl;
    
    return false;
  }
  
  if ((_vField =
       _readMdvField(_params->v_field_info.url,
		     _params->v_field_info.field_name,
		     _params->v_field_info.field_num,
		     trigger_time)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading V field:" << endl;
    cerr << "    URL: " << _params->v_field_info.url << endl;
    cerr << "    field name: " << _params->v_field_info.field_name << endl;
    cerr << "    field number: " << _params->v_field_info.field_num << endl;
    cerr << "    trigger time: " << trigger_time << endl;
    
    return false;
  }
  
  if ((_capeField =
       _readMdvField(_params->cape_field_info.url,
		     _params->cape_field_info.field_name,
		     _params->cape_field_info.field_num,
		     trigger_time)) == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading CAPE field:" << endl;
    cerr << "    URL: " << _params->cape_field_info.url << endl;
    cerr << "    field name: " << _params->cape_field_info.field_name << endl;
    cerr << "    field number: " << _params->cape_field_info.field_num << endl;
    cerr << "    trigger time: " << trigger_time << endl;
    
    return false;
  }
  
  // Make sure that all of our projections match

  if (!_projectionsMatch(*_uField, *_vField) ||
      !_projectionsMatch(*_uField, *_capeField))
    return false;
  
  // Go ahead and check for dz_constant here.  Currently, we can't
  // process files with varying dz values.

  Mdvx::field_header_t u_field_hdr = _uField->getFieldHeader();
  Mdvx::field_header_t v_field_hdr = _vField->getFieldHeader();
  Mdvx::field_header_t cape_field_hdr = _capeField->getFieldHeader();
  
  if (!u_field_hdr.dz_constant || !v_field_hdr.dz_constant ||
      !cape_field_hdr.dz_constant)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "One of the input MDV fields doesn't have constant dz" << endl;
    cerr << "Currently, fields must have constant dz" << endl;
    
    return false;
  }
  
  // Check to see that the algorithm parameters set in the parameter file
  // will work with this dataset.

  if (_params->vertical_shear_depth.use_num_levels)
  {
    if (cape_field_hdr.nz < _params->vertical_shear_depth.num_levels)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Current dataset can't work with current parameter file" << endl;
      cerr << "CAPE dataset has " << cape_field_hdr.nz << " vertical levels" << endl;
      cerr << "Parameter file has vertical_shear_depth.num_levels set to "
	   << _params->vertical_shear_depth.num_levels << endl;
      cerr << "Either reduce the num_levels parameter or increase the number of levels in your input datasets" << endl;
      
      return false;
    }
  }
  else
  {
    double cape_depth = fabs(cape_field_hdr.nz * cape_field_hdr.grid_dz);
    if (cape_depth < _params->vertical_shear_depth.level_depth)
    {
      cerr << "ERROR: " << method_name << endl;
      cerr << "Current dataset can't work with current parameter file" << endl;
      cerr << "CAPE dataset has a vertical depth of " << cape_depth << endl;
      cerr << "Parameter file has vertical_shear_depth.level_depth set to "
	   << _params->vertical_shear_depth.level_depth << endl;
      cerr << "Either reduce the level_depth parameter or increase the vertical depth in your input datasets" << endl;

      return false;
    }
  }
  
  // Set the plane pointers for all of the fields since we'll need these
  // in our calculations.

  _uField->setPlanePtrs();
  _vField->setPlanePtrs();
  _capeField->setPlanePtrs();

  return true;
}


/*********************************************************************
 * _readMdvField() - Read the indicated field from an MDV file.
 *
 * Returns a pointer to the read field on success, 0 on failure.
 */

MdvxField *ShearForLeadingEdge::_readMdvField(const string &url,
					      const string &field_name,
					      const int field_num,
					      const DateTime &trigger_time) const
{
  static const string method_name = "ShearForLeadingEdge::_readMdvField()";
  
  DsMdvx mdvx;
  
  mdvx.setReadTime(Mdvx::READ_CLOSEST,
		   url, _params->max_input_valid_secs,
		   trigger_time.utime());
  
  if (field_name == "")
    mdvx.addReadField(field_num);
  else
    mdvx.addReadField(field_name);
  
  if (_params->input_level_limits.use_level_nums)
    mdvx.setReadPlaneNumLimits(_params->input_level_limits.min_level_num,
			       _params->input_level_limits.max_level_num);
  else
    mdvx.setReadVlevelLimits(_params->input_level_limits.min_level_value,
			     _params->input_level_limits.max_level_value);
  
  mdvx.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  mdvx.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  mdvx.setReadScalingType(Mdvx::SCALING_NONE);
  
  if (_params->debug)
    mdvx.printReadRequest(cerr);
  
  if (mdvx.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading field:" << endl;
    cerr << "    URL: " << url << endl;
    if (field_name == "")
      cerr << "    field number: " << field_num << endl;
    else
      cerr << "    field name: " << field_name << endl;
    cerr << "    trigger time: " << trigger_time << endl;
    if (_params->input_level_limits.use_level_nums)
    {
      cerr << "    min level num: " << _params->input_level_limits.min_level_num << endl;
      cerr << "    max level num: " << _params->input_level_limits.max_level_num << endl;
    }
    else
    {
      cerr << "    min level: " << _params->input_level_limits.min_level_value << endl;
      cerr << "    max level: " << _params->input_level_limits.max_level_value << endl;
    }
    
    return 0;
  }
  
  return new MdvxField(*mdvx.getField(0));
}
