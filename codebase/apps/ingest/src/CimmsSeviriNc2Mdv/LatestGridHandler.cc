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
 * @file LatestGridHandler.cc
 *
 * @class LatestGridHandler
 *
 * Class for accumulating the data for the gridded fields using the latest
 * value encountered for each grid square.
 *  
 * @date 1/28/2010
 *
 */

#include "LatestGridHandler.hh"

using namespace std;


/**********************************************************************
 * Constructor
 */

LatestGridHandler::LatestGridHandler(const Pjg &proj,
				     const bool debug_flag) :
  GridHandler(proj, debug_flag),
  _data(0)
{
}


/**********************************************************************
 * Destructor
 */

LatestGridHandler::~LatestGridHandler()
{
  _deleteGrid();
}
  

/**********************************************************************
 * clearGridData()
 */

void LatestGridHandler::clearGridData()
{
  static const string method_name = "LatestGridHandler::clearGridData()";
  
  // Fill the grids with the missing data value

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  for (size_t i = 0; i < grid_size; ++i)
  {
    _data[i] = MISSING_DATA_VALUE;
  }
}


/**********************************************************************
 * getGrid()
 */

fl32 *LatestGridHandler::getGrid()
{
  return _data;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/**********************************************************************
 * _addData()
 */

void LatestGridHandler::_addData(const double lat,
				 const double lon,
				 const double value)
{
  static const string method_name = "LatestGridHandler::_addData()";

  // Get the grid index

  int grid_index;

  if (_proj.latlon2arrayIndex(lat, lon, grid_index) != 0)
    return;
    
  // Set the data value

  _data[grid_index] = value;
}


/**********************************************************************
 * _deleteGrid()
 */

void LatestGridHandler::_deleteGrid()
{
  delete [] _data;
}


/**********************************************************************
 * _init()
 */

bool LatestGridHandler::_init()
{
  static const string method_name = "LatestGridHandler::_init()";
  
  // Allocate space for the grids

  size_t grid_size = _proj.getNx() * _proj.getNy();
  
  _data = new fl32[grid_size];

  if (_data == 0)
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
