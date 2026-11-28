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
 * @file MeanGridHandler.cc
 *
 * @class MeanGridHandler
 *
 * Class for accumulating the data for the gridded fields using the mean data
 * value within the defined radius of influence of the center of the grid
 * square.
 *  
 * @date 1/28/2010
 *
 */

#include <string.h>

#include "MeanGridHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

MeanGridHandler::MeanGridHandler(const double radius_of_influence,
				 const Pjg &proj,
				 const bool debug_flag) :
  GridHandler(proj, debug_flag),
  _numValues(0),
  _sum(0),
  _data(0),
  _radiusOfInfl(radius_of_influence)
{
  _xRadius = (int)proj.km2xGrid(_radiusOfInfl) + 2;
  _yRadius = (int)proj.km2yGrid(_radiusOfInfl) + 2;
}


/**********************************************************************
 * Destructor
 */

MeanGridHandler::~MeanGridHandler()
{
  _deleteGrid();
}
  

/**********************************************************************
 * clearGridData()
 */

void MeanGridHandler::clearGridData()
{
  static const string method_name = "MeanGridHandler::clearGridData()";
  
  // Fill the grids with the missing data value

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  memset(_numValues, 0, grid_size * sizeof(int));
  memset(_sum, 0, grid_size * sizeof(double));
}


/**********************************************************************
 * getGrid()
 */

fl32 *MeanGridHandler::getGrid()
{
  cerr << "---> Entering MeanGridHandler::getGrid()";
  
  delete [] _data;
  
  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  _data = new fl32[grid_size];
  
  for (size_t i = 0; i < grid_size; ++i)
  {
    if (_numValues[i] <= 0)
      _data[i] = MISSING_DATA_VALUE;
    else
      _data[i] = _sum[i] / (double)_numValues[i];
  }
  
  return _data;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addData()
 */

void MeanGridHandler::_addData(const double lat,
			       const double lon,
			       const double value)
{
  static const string method_name = "MeanGridHandler::_addData()";

  // Get the grid index

  int x_index, y_index;

  _proj.latlon2xyIndex(lat, lon, x_index, y_index);

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

      _sum[grid_index] += value;

      _numValues[grid_index]++;

    } /* endfor - y */

  } /* endfor - x */
}


/**********************************************************************
 * _deleteGrid()
 */

void MeanGridHandler::_deleteGrid()
{
  delete [] _numValues;
  delete [] _sum;
  delete [] _data;
}


/**********************************************************************
 * _init()
 */

bool MeanGridHandler::_init()
{
  static const string method_name = "MeanGridHandler::_init()";
  
  // Allocate space for the grids

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  _numValues = new int[grid_size];
  _sum = new double[grid_size];

  if (_numValues == 0 || _sum == 0)
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
