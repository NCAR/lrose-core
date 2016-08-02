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
//   $Date: 2016/03/04 01:28:11 $
//   $Id: AreaFilter.cc,v 1.4 2016/03/04 01:28:11 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * AreaFilter: Class for filtering Tstorms based on storm area.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "AreaFilter.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

AreaFilter::AreaFilter(const double min_storm_size,
		       const bool debug) :
  Filter(debug),
  _minStormSize(min_storm_size)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

AreaFilter::~AreaFilter()
{
  // Do nothing
}


/*********************************************************************
 * isPassing() - Returns true if the storm passes the filter criteria,
 *               false otherwise.
 */

bool AreaFilter::isPassing(const Tstorm& storm,
			   double &algorithm_value)
{
  algorithm_value = storm.getArea();
  
  if (_debug)
    cerr << "---> Storm has area: " << algorithm_value << endl;
  
  if (algorithm_value >= _minStormSize)
    return true;
  
  return false;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
