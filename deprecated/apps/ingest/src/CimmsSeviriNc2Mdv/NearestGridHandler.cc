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
/**
 *
 * @file NearestGridHandler.cc
 *
 * @class NearestGridHandler
 *
 * Class for accumulating the data for the gridded fields using the DMSP
 * value that falls closest to the center of the grid square.
 *  
 * @date 1/28/2010
 *
 */

#include "NearestGridHandler.hh"

using namespace std;

// Global constants

const double NearestGridHandler::MISSING_DISTANCE_VALUE = -9999.9;



/**********************************************************************
 * Constructor
 */

NearestGridHandler::NearestGridHandler(const double radius_of_influence,
				       const Pjg &proj,
				       const bool debug_flag) :
  GridHandler(proj, debug_flag),
  _data(0),
  _distances(0),
  _radiusOfInfl(radius_of_influence)
{
  _xRadius = (int)proj.km2xGrid(_radiusOfInfl) + 2;
  _yRadius = (int)proj.km2yGrid(_radiusOfInfl) + 2;
}


/**********************************************************************
 * Destructor
 */

NearestGridHandler::~NearestGridHandler()
{
  _deleteGrid();
}
  

/**********************************************************************
 * clearGridData()
 */

void NearestGridHandler::clearGridData()
{
  static const string method_name = "NearestGridHandler::clearGridData()";
  
  // Fill the grids with the missing data value

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  for (size_t i = 0; i < grid_size; ++i)
  {
    _data[i] = MISSING_DATA_VALUE;
    _distances[i] = MISSING_DISTANCE_VALUE;
  }
}


/**********************************************************************
 * getGrid()
 */

fl32 *NearestGridHandler::getGrid()
{
  return _data;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addData()
 */

void NearestGridHandler::_addData(const double lat,
				  const double lon,
				  const double value)
{
  static const string method_name = "NearestGridHandler::_addData()";

  // Get the grid index

  int x_index, y_index;

  if (_proj.latlon2xyIndex(lat, lon, x_index, y_index) != 0)
    return;

  // Find the grid squares affected by this data value

  for (int x = x_index - _xRadius; x <= x_index + _xRadius; ++x)
  {
    if (x < 0 || x >= _proj.getNx())
      continue;

    for (int y = y_index - _yRadius; y <= y_index + _yRadius; ++y)
    {
      if (y < 0 || y >= _proj.getNy())
	continue;

      // Calculate the distance between the grid center and the
      // data location

      double center_lat, center_lon;

      _proj.xyIndex2latlon(x, y, center_lat, center_lon);

      double distance, theta;

      _proj.latlon2RTheta(lat, lon, center_lat, center_lon,
			  distance, theta);

      // See if this grid square is within the radius of influence

      if (distance > _radiusOfInfl)
	continue;

      // Set the data values

      int grid_index = _proj.xyIndex2arrayIndex(x, y);

      if (grid_index < 0)
	continue;

      if (_distances[grid_index] != MISSING_DISTANCE_VALUE &&
	  distance >= _distances[grid_index])
	continue;

      // Set the data value

      _data[grid_index] = value;
      _distances[grid_index] = distance;

    } /* endfor - y */

  } /* endfor - x */
}


/**********************************************************************
 * _deleteGrid()
 */

void NearestGridHandler::_deleteGrid()
{
  delete [] _data;
  delete [] _distances;
}


/**********************************************************************
 * _init()
 */

bool NearestGridHandler::_init()
{
  static const string method_name = "NearestGridHandler::_init()";
  
  // Allocate space for the grids

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  _data = new fl32[grid_size];
  _distances = new double[grid_size];
  
  if (_data == 0 || _distances == 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error allocating space for internal grid" << endl;
    
    _deleteGrid();
    
    return false;
  }
  
  // Fill the grids with the missing data value

  clearGridData();
  
  return true;
}
