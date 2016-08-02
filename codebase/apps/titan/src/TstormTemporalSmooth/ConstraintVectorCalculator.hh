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
/************************************************************************
 * ConstraintVectorCalculator : Base class for classes that calculate
 *                              constraint vectors for TstormTemporalSmooth.
 *
 * RAP, NCAR, Boulder CO
 *
 * January 2004
 *
 * Nancy Rehak
 *
 ************************************************************************/

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/04 01:28:14 $
 *   $Id: ConstraintVectorCalculator.hh,v 1.3 2016/03/04 01:28:14 dixon Exp $
 *   $Revision: 1.3 $
 *   $State: Exp $
 *
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

#ifndef ConstraintVectorCalculator_H
#define ConstraintVectorCalculator_H

#include <dsdata/Tstorm.hh>
#include <dsdata/TstormMgr.hh>

using namespace std;


class ConstraintVectorCalculator
{
public:
    
  /*********************************************************************
   * Constructors
   */

  ConstraintVectorCalculator();
  
  
  /*********************************************************************
   * Destructors
   */

  virtual ~ConstraintVectorCalculator(void);
  

  /*********************************************************************
   * getConstraintVector() - Get the constraint vector to use for the
   *                         given current storm.
   *
   * Returns true if a constraint vector was found, false otherwise.
   * Returns the constraint vector in the constraint_speed and 
   * constraint_dir parameters.
   */

  virtual bool getConstraintVector(const Tstorm &curr_storm,
				   const int delta_time,
				   TstormMgr &constraint_tstorm_mgr,
				   double &constraint_speed,
				   double &constraint_dir,
				   const bool valid_storms_only = false) const = 0;
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////


  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
