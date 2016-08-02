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

// RCS info
//   $Author: dixon $
//   $Locker:  $
//   $Date: 2016/03/07 01:50:09 $
//   $Id: NearestInterpolater.cc,v 1.2 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * NearestInterpolater: Class for interpolating using the Nearest Neighbor
 *                      method.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "NearestInterpolater.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

NearestInterpolater::NearestInterpolater(const Pjg &output_proj,
					 const double max_interp_dist_km,
					 const bool debug_flag) :
  Interpolater(output_proj, max_interp_dist_km, debug_flag)
{
  // Allocate space for the accumulation grids

  _accumGrids = new accum_info_t[_outputProj.getNx() * _outputProj.getNy()];
  
}

  
/*********************************************************************
 * Destructor
 */

NearestInterpolater::~NearestInterpolater()
{
  delete [] _accumGrids;
}


/*********************************************************************
 * init() - Initialize all of the accumulation grids so we can start a
 *          new interpolation.
 *
 * Returns true on success, false on failure.
 */

bool NearestInterpolater::init()
{
  // Clear out all of the current data

  for (int i = 0; i < _outputProj.getNx() * _outputProj.getNy(); ++i)
  {
    _accumGrids[i].min_dist = -1.0;
    _accumGrids[i].value = 0.0;
  }
  
  return true;
}


/*********************************************************************
 * addObs() - Add the given observation to the interpolation.
 *
 * Returns true on success, false on failure.
 */

bool NearestInterpolater::addObs(const double obs_value,
				 const double obs_lat,
				 const double obs_lon,
				 const double bad_obs_value,
				 const float *interp_distance_grid)
{
  if (obs_value == bad_obs_value)
    return true;
  
  for (int i = 0; i < _outputProj.getNx() * _outputProj.getNy(); ++i)
  {
    if (interp_distance_grid[i] < 0.0 ||
	interp_distance_grid[i] > _maxInterpDistKm)
      continue;
    
    if (_accumGrids[i].min_dist < 0.0 ||
	_accumGrids[i].min_dist > interp_distance_grid[i])
    {
      _accumGrids[i].min_dist = interp_distance_grid[i];
      _accumGrids[i].value = obs_value;
      }
  } /* endfor - i */
  
  return true;
}


/*********************************************************************
 * getInterpolation() - Get the interpolated grid.
 *
 * Returns a pointer to the interpolation grid.  The caller must free
 * this pointer when finished with the data.
 */

fl32 *NearestInterpolater::getInterpolation(const fl32 bad_data_value)
{
  cerr << "Entering NearestInterpolater::getInterpolation()" << endl;
  
  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  fl32 *interp_grid = new fl32[grid_size];
  
  for (int i = 0; i < grid_size; ++i)
  {
    if (_accumGrids[i].min_dist < 0.0 ||
	_accumGrids[i].min_dist > _maxInterpDistKm)
      interp_grid[i] = bad_data_value;
    else
      interp_grid[i] = _accumGrids[i].value;
  } /* endfor - i */
  
  return interp_grid;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
