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
 *   $Id: NearestInterpolater.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * NearestInterpolater: Class for interpolating using the Nearest Neighbor
 *                      method.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef NearestInterpolater_H
#define NearestInterpolater_H

#include <iostream>
#include <string>

#include "Interpolater.hh"

using namespace std;


class NearestInterpolater : public Interpolater
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  NearestInterpolater(const Pjg &output_proj,
		      const double max_interp_dist_km,
		      const bool debug_flag);
  
  
  /*********************************************************************
   * Destructor
   */

  virtual ~NearestInterpolater();


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
  
  /////////////////////
  // Protected types //
  /////////////////////

  typedef struct
  {
    double min_dist;
    double value;
  } accum_info_t;
  
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  accum_info_t *_accumGrids;
  

};

#endif
