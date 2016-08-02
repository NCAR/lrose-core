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
//   $Id: SimpleIdVecCalc.cc,v 1.3 2016/03/04 01:28:14 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//


/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * SimpleIdVecCalc : Class that calculates constraint vectors by using the
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

#include "SimpleIdVecCalc.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

SimpleIdVecCalc::SimpleIdVecCalc()
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

SimpleIdVecCalc::~SimpleIdVecCalc(void)
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

bool SimpleIdVecCalc::getConstraintVector(const Tstorm &curr_storm,
					  const int delta_time,
					  TstormMgr &constraint_tstorm_mgr,
					  double &constraint_speed,
					  double &constraint_dir,
					  const bool valid_storms_only) const
{
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
      
      // Check the simple ID of the storm

      if (curr_storm.getSimpleTrack() == constraint_storm->getSimpleTrack())
      {
	constraint_speed = constraint_storm->getSpeed();
	constraint_dir = constraint_storm->getDirection();
      
	return true;
      }
    
    } /* endfor - constraint_storm_iter */
  } /* endofr - constraint_group_iter */
  
  return false;
}


/*********************************************************************
 * PRIVATE FUNCTIONS
 *********************************************************************/
