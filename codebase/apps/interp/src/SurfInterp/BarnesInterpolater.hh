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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:50:09 $
 *   $Id: BarnesInterpolater.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
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
 ************************************************************************/

#ifndef BarnesInterpolater_H
#define BarnesInterpolater_H

#include <iostream>
#include <string>
#include <vector>

#include "Interpolater.hh"

using namespace std;


class BarnesInterpolater : public Interpolater
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  BarnesInterpolater(const Pjg &output_proj,
		     const double r,
		     const double rmax,
		     const double gamma,
		     const double arc_max,
		     const double rclose,
		     const double min_weight,
		     const double max_interp_dist_km,
		     const bool debug_flag);
  
  
  /*********************************************************************
   * Destructor
   */

  virtual ~BarnesInterpolater();


  /*********************************************************************
   * init() - Initialize all of the accumulation grids so we can start a
   *          new interpolation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool init();


  /*********************************************************************
   * addObs() - Add the given observation to the interpolation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool addObs(const double obs_value,
		      const double obs_lat,
		      const double obs_lon,
		      const double bad_obs_value,
		      const float *interp_distance_grid);
  

  /*********************************************************************
   * getInterpolation() - Get the interpolated grid.
   *
   * Returns a pointer to the interpolation grid.  The caller must free
   * this pointer when finished with the data.
   */

  virtual fl32 *getInterpolation(const fl32 bad_data_value);
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const double BIG;
  static const double RFAC;
  

  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double wsum;
    double sum;
    int n_obs;
    vector< double > angles_rad;
  } accum_info_t;
  
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  accum_info_t *_accumGrids;
  
  double _r;
  double _rmax;
  double _gamma;
  double _arcMaxRad;
  double _rclose;
  double _minWeight;

  bool _doArc;
  
  double _rscale;
  double _dmax;
  double _dclose;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _angleMax() - Compute the maximum angle in the list of angles.  The
   *               angle values are assumed to be specified in radians.
   *
   * Returns the computed maximum angle.
   */

  static double _angleMax(vector< double > &angles);
  

  /*********************************************************************
   * _updateAccumGrids() - Update the accumulation grids with the given
   *                       observation information.
   */

  void _updateAccumGrids(const double obs_value,
			 const double obs_lat,
			 const double obs_lon,
			 const int x_index,
			 const int y_index,
			 const int grid_index,
			 const double interp_dist_km);
  

};

#endif
