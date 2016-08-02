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
//   $Date: 2016/03/04 01:28:14 $
//   $Id: WeightedAvgVecCalc.cc,v 1.6 2016/03/04 01:28:14 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//


/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * WeightedAvgVecCalc : Class that calculates constraint vectors by using the
 *                   motion vector for the storm in the constraint database
 *                   that has the same simple track ID as the current storm.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2004
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/Pjg.hh>
#include <toolsa/toolsa_macros.h>

#include "WeightedAvgVecCalc.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

WeightedAvgVecCalc::WeightedAvgVecCalc(const double weighted_average_radius_km,
				       const bool use_current_location) :
  _weightedAvgRadiusKm(weighted_average_radius_km),
  _useCurrLocation(use_current_location)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

WeightedAvgVecCalc::~WeightedAvgVecCalc(void)
{
  // Do nothing
}


/*********************************************************************
 * getConstraintVector() - Get the constraint vector to use for the
 *                         given current storm.
 *
 * Returns true if a constraint vector was found, false otherwise.
 * Returns the constraint vector in the constraint_speed and 
 * constraint_dir parameters.
 */

bool WeightedAvgVecCalc::getConstraintVector(const Tstorm &curr_storm,
					     const int delta_time,
					     TstormMgr &constraint_tstorm_mgr,
					     double &constraint_speed,
					     double &constraint_dir,
					     const bool valid_storms_only) const
{
  // Initialize the weighted average values

  double total_weights = 0.0;
  double total_weighted_u = 0.0;
  double total_weighted_v = 0.0;
  
  // Get the storm location to use for the weighted averages

  double curr_storm_lat = curr_storm.getCentroidLat();
  double curr_storm_lon = curr_storm.getCentroidLon();
  
  if (!_useCurrLocation)
  {
    Pjg::latlonPlusRTheta(curr_storm_lat, curr_storm_lon,
			  curr_storm.getSpeed() * delta_time / 60.0,
			  (curr_storm.getDirection() + 180.0) * DEG_TO_RAD,
			  curr_storm_lat, curr_storm_lon);
  }
  
  // Look at all of the storms in all of the constraint groups

  vector< TstormGroup* > constraint_groups = constraint_tstorm_mgr.getGroups();
  vector< TstormGroup* >::iterator constraint_group_iter;
  
  for (constraint_group_iter = constraint_groups.begin();
       constraint_group_iter != constraint_groups.end();
       ++constraint_group_iter)
  {
    vector< Tstorm* > constraint_storms =
      (*constraint_group_iter)->getTstorms();
    vector< Tstorm* >::iterator constraint_storm_iter;
    
    for (constraint_storm_iter = constraint_storms.begin();
	 constraint_storm_iter != constraint_storms.end();
	 ++constraint_storm_iter)
    {
      // Get a pointer to the Tstorm object for easier syntax.

      Tstorm *constraint_storm = *constraint_storm_iter;
      
      // Check the storm validity

      if (valid_storms_only &&
	  !constraint_storm->isForecastValid())
	continue;
      
      // Calculate the distance between the storm centroids.
      // If this constraint storm isn't close enough to the current
      // storm to be used, continue on to the next storm.

      double distance;
      double theta;
      
      Pjg::latlon2RTheta(curr_storm_lat, curr_storm_lon,
			 constraint_storm->getCentroidLat(),
			 constraint_storm->getCentroidLon(),
			 distance, theta);
      
      if (distance > _weightedAvgRadiusKm)
	continue;
      
      // Update the weighted totals

      double weight = (_weightedAvgRadiusKm - distance) / _weightedAvgRadiusKm;
      
      total_weights += weight;
      total_weighted_u += (constraint_storm->getU() * weight);
      total_weighted_v += (constraint_storm->getV() * weight);
      
    } /* endfor - constraint_storm_iter */
  } /* endfor - constraint_group_iter */
  
  // Calculate the constraint speed and direction

  if (total_weights <= 0.0)
    return false;

  double average_u = total_weighted_u / total_weights;
  double average_v = total_weighted_v / total_weights;
  
  constraint_speed = sqrt((average_u * average_u) +
			  (average_v * average_v)) * 0.001 * 3600.0;
  
  if (average_u != 0.0 && average_v != 0.0)
    constraint_dir = atan2(average_u, average_v) * RAD_TO_DEG;
  else
    constraint_dir = 0.0;
  
  return true;
}


/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/
