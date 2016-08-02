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
//   $Id: PartitionFilter.cc,v 1.5 2016/03/04 01:28:11 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PartitionFilter: Class for filtering Tstorms based on a partition
 *                  grid.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2003
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <Mdv/MdvxPjg.hh>

#include "PartitionFilter.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

PartitionFilter::PartitionFilter(const string& partition_url,
				 const string& partition_field_name,
				 const int partition_field_level_num,
				 const int max_valid_secs,
				 const double partition_value,
				 const double partition_percent,
				 const double storm_growth_km,
				 const missing_input_action_t missing_input_action,
				 const bool debug) :
  MdvFilter(partition_url,
	    partition_field_name,
	    partition_field_level_num,
	    max_valid_secs,
	    storm_growth_km,
	    missing_input_action,
	    debug),
  _partitionValue(partition_value),
  _partitionPercent(partition_percent)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

PartitionFilter::~PartitionFilter()
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _isPassing() - Determines if the storm passes the criteria.
 */

bool PartitionFilter::_isPassing(const WorldPolygon2D *polygon,
				 double &algorithm_value)
{
  // Get the maximum interest value in the grid

  Mdvx::field_header_t field_hdr = _field->getFieldHeader();
  MdvxPjg projection(field_hdr);
  
  int grid_size = polygon->getGridSize(projection);

  int num_values = polygon->getGridNumValues(projection,
					     _partitionValue,
					     (fl32 *)_field->getVol());

  if (_debug)
    cerr << "---> Storm has " << num_values << " partition values in "
	 << grid_size << " total grid squares" << endl;
  
  algorithm_value = (double)num_values / (double)grid_size;
  
  if (algorithm_value < _partitionPercent)
    return false;

  return true;
}
