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
 * @file GridHandler.cc
 *
 * @class GridHandler
 *
 * Base class for classes that accumulate the data for the gridded fields.
 *  
 * @date 1/28/2010
 *
 */

#include "GridHandler.hh"

using namespace std;

// Global constants

const double GridHandler::MISSING_DATA_VALUE = -9999.9;


/**********************************************************************
 * Constructor
 */

GridHandler::GridHandler(const Pjg &proj, const bool debug_flag) :
  _debug(debug_flag),
  _proj(proj)
{
}


/**********************************************************************
 * Destructor
 */

GridHandler::~GridHandler()
{
}
  

/**********************************************************************
 * addData()
 */

void GridHandler::addData(const double lat,
			  const double lon,
			  const double value)
{
  static const string method_name = "GridHandler::addData()";

  // Normalize the longitude value

  double lon_norm = lon;
  while (lon_norm < _proj.getMinx())
    lon_norm += 360.0;
  while (lon_norm >= _proj.getMinx() + 360.0)
    lon_norm -= 360.0;
    
  // Add the data value to the grid

  _addData(lat, lon_norm, value);
  
}


/**********************************************************************
 * init()
 */

bool GridHandler::init()
{
  static const string method_name = "GridHandler::init()";
  
  // Initialize the derived class

  if (!_init())
    return false;

  return true;
}
  

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
