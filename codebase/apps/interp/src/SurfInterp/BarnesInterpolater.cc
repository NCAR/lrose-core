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
//   $Id: BarnesInterpolater.cc,v 1.5 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * BarnesInterpolater: Class for interpolating using the Barnes method.
 *
 * This implementation of the Barnes scheme limits you to a single pass,
 * which is what was being used by SurfInterp when I refactored it.
 * SurfInterp would have to be refactored again if a multiple pass Barnes
 * analysis is needed because each pass through the analysis requires another
 * pass through the original data.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <algorithm>

#include <rapmath/math_macros.h>

#include "BarnesInterpolater.hh"

using namespace std;

const double BarnesInterpolater::BIG = 9.9e29;
const double BarnesInterpolater::RFAC = log(0.1);


/*********************************************************************
 * Constructors
 */

BarnesInterpolater::BarnesInterpolater(const Pjg &output_proj,
				       const double r,
				       const double rmax,
				       const double gamma,
				       const double arc_max_deg,
				       const double rclose,
				       const double min_weight,
				       const double max_interp_dist_km,
				       const bool debug_flag) :
  Interpolater(output_proj, max_interp_dist_km, debug_flag),
  _r(r),
  _rmax(rmax),
  _gamma(gamma),
  _arcMaxRad(arc_max_deg * DEG_TO_RAD),
  _rclose(rclose),
  _minWeight(min_weight),
  _doArc(arc_max_deg > 0.0 && arc_max_deg < 360.0)
{
  if (_r <= 0.0)
    _rscale = _maxInterpDistKm / 2.0;
  else
    _rscale = _r * _r;
  
  if (_rmax < 0.0)
    _dmax = BIG;
  else if (_rmax == 0.0)
    _dmax = _rscale * 4.0;
  else
    _dmax = _rmax * _rmax;
  
  if (_rclose > 0.0)
    _dclose = _rclose * _rclose;
  else if (_rclose == 0.0)
    _dclose = _rscale;
  else
    _dclose = BIG;
  
  // Allocate space for the accumulation grids

  _accumGrids = new accum_info_t[_outputProj.getNx() * _outputProj.getNy()];
  
}

  
/*********************************************************************
 * Destructor
 */

BarnesInterpolater::~BarnesInterpolater()
{
  delete [] _accumGrids;
}


/*********************************************************************
 * init() - Initialize all of the accumulation grids so we can start a
 *          new interpolation.
 *
 * Returns true on success, false on failure.
 */

bool BarnesInterpolater::init()
{
  // Clear out all of the current data

  for (int i = 0; i < _outputProj.getNx() * _outputProj.getNy(); ++i)
  {
    _accumGrids[i].wsum = 0.0;
    _accumGrids[i].sum = 0.0;
    _accumGrids[i].n_obs = 0;
    _accumGrids[i].angles_rad.clear();
  }
  
  return true;
}


/*********************************************************************
 * addObs() - Add the given observation to the interpolation.
 *
 * Returns true on success, false on failure.
 */

bool BarnesInterpolater::addObs(const double obs_value,
				const double obs_lat,
				const double obs_lon,
				const double bad_obs_value,
				const float *interp_distance_grid)
{
  if (obs_value == bad_obs_value)
    return true;
  
  int data_index = 0;
  
  int nx = _outputProj.getNx();
  int ny = _outputProj.getNy();
  
  for (int y = 0; y < ny; ++y)
  {
    for (int x = 0; x < nx; ++x, ++data_index)
    {
      if (interp_distance_grid[data_index] < 0.0 ||
	  interp_distance_grid[data_index] > _maxInterpDistKm)
	continue;
      
      _updateAccumGrids(obs_value, obs_lat, obs_lon, x, y, data_index,
			interp_distance_grid[data_index]);
      
    } /* endfor - x */
  } /* endfor - y */
  
  return true;
}


/*********************************************************************
 * getInterpolation() - Get the interpolated grid.
 *
 * Returns a pointer to the interpolation grid.  The caller must free
 * this pointer when finished with the data.
 */

fl32 *BarnesInterpolater::getInterpolation(const fl32 bad_data_value)
{
  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  fl32 *interp_grid = new fl32[grid_size];
  
  for (int i = 0; i < grid_size; ++i)
  {
    if (_accumGrids[i].n_obs <= 0 ||
	_accumGrids[i].wsum < _minWeight)
    {
      interp_grid[i] = bad_data_value;
    }
    else if (_doArc)
    {
      if (_angleMax(_accumGrids[i].angles_rad) <= _arcMaxRad)
	interp_grid[i] = _accumGrids[i].sum / _accumGrids[i].wsum;
      else
	interp_grid[i] = bad_data_value;
    }
    else
    {
      interp_grid[i] = _accumGrids[i].sum / _accumGrids[i].wsum;
    }
    
  }
  
  return interp_grid;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _angleMax() - Compute the maximum angle in the list of angles.  The
 *               angle values are assumed to be specified in radians.
 *
 * Returns the computed maximum angle.
 */

double BarnesInterpolater::_angleMax(vector< double > &angles)
{
  // Handle the case where we don't have enough angles to do the processing

  if (angles.size() <= 1)
    return M_2_PI;
  
  // Sort the angles in the vector

  sort(angles.begin(), angles.end());
  
  // Loop through the angles, calculating the angle differences and
  // saving the maximum one.

  vector< double >::const_iterator angle = angles.begin();
  
  double prev_angle = *angle;
  double max_angle_diff = -1.0;
  
  for (++angle; angle != angles.end(); ++angle)
  {
    double curr_angle_diff = *angle - prev_angle;
    
    if (max_angle_diff < 0.0 || curr_angle_diff > max_angle_diff)
      max_angle_diff = curr_angle_diff;
    
    prev_angle = *angle;
    
  } /* endfor - angle */
  
  // Handle the difference between the first and last angles

  double curr_angle_diff =
    angles[0] - angles[angles.size() - 1] + M_2_PI;
  
  if (curr_angle_diff > max_angle_diff)
    max_angle_diff = curr_angle_diff;
  
  return max_angle_diff;
}


/*********************************************************************
 * _updateAccumGrids() - Update the accumulation grids with the given
 *                       observation information.
 */

void BarnesInterpolater::_updateAccumGrids(const double obs_value,
					   const double obs_lat,
					   const double obs_lon,
					   const int x_index,
					   const int y_index,
					   const int grid_index,
					   const double interp_dist_km)
{
  double weight = exp(RFAC * interp_dist_km / _rscale);
  
  _accumGrids[grid_index].wsum += weight;
  _accumGrids[grid_index].sum += obs_value * weight;
  ++_accumGrids[grid_index].n_obs;
  if (_doArc)
  {
    double grid_lat, grid_lon;
    double r, theta;
    
    _outputProj.xyIndex2latlon(x_index, y_index, grid_lat, grid_lon);
    _outputProj.latlon2RTheta(obs_lat, obs_lon, grid_lat, grid_lon,
			      r, theta);
    
    _accumGrids[grid_index].angles_rad.push_back(theta);
  }
  
}
