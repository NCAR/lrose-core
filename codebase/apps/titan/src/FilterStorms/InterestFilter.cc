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
//   $Id: InterestFilter.cc,v 1.10 2016/03/04 01:28:11 dixon Exp $
//   $Revision: 1.10 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InterestFilter: Class for filtering Tstorms based on an interest
 *                 grid.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <euclid/geometry.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/MdvxPjg.hh>

#include "InterestFilter.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

InterestFilter::InterestFilter(const string& interest_url,
			       const string& interest_field_name,
			       const int interest_field_level_num,
			       const int max_valid_secs,
			       const bool get_max_value,
			       const double min_interest_value,
			       const double storm_growth_km,
			       const missing_input_action_t missing_input_action,
			       const bool debug) :
  MdvFilter(interest_url,
	    interest_field_name,
	    interest_field_level_num,
	    max_valid_secs,
	    storm_growth_km,
	    missing_input_action,
	    debug),
  _minInterestValue(min_interest_value),
  _getMaxValue(get_max_value)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

InterestFilter::~InterestFilter()
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _isPassing() - Determines if the storm passes the criteria.
 */

bool InterestFilter::_isPassing(const WorldPolygon2D *polygon,
				double &algorithm_value)
{
  // Get the maximum interest value in the grid

  Mdvx::field_header_t field_hdr = _field->getFieldHeader();
  
  if(_getMaxValue)
      algorithm_value = polygon->getGridMax(MdvxPjg(field_hdr),
					    field_hdr.missing_data_value,
					    field_hdr.bad_data_value,
					    (fl32 *)_field->getVol());
  else
      algorithm_value = polygon->getGridMin(MdvxPjg(field_hdr),
					    field_hdr.missing_data_value,
					    field_hdr.bad_data_value,
					    (fl32 *)_field->getVol());

  if (_debug)
  {
    if (algorithm_value == _field->getFieldHeader().missing_data_value)
      cerr << "---> Storm has interest value: missing" << endl;
    else
      cerr << "---> Storm has interest value: " << algorithm_value << endl;
  }
  
  if (algorithm_value == _field->getFieldHeader().missing_data_value ||
      algorithm_value < _minInterestValue)
    return false;

  return true;
}
