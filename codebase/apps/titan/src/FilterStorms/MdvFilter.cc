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
//   $Id: MdvFilter.cc,v 1.5 2016/03/04 01:28:11 dixon Exp $
//   $Revision: 1.5 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MdvFilter: Base class for filtering Tstorms based on a 2-dimensional
 *            MDV field.
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

#include "MdvFilter.hh"
using namespace std;


/*********************************************************************
 * Constructor
 */

MdvFilter::MdvFilter(const string& url,
		     const string& field_name,
		     const int level_num,
		     const int max_valid_secs,
		     const double storm_growth_km,
		     const missing_input_action_t missing_input_action,
		     const bool debug) :
  Filter(debug),
  _url(url),
  _fieldName(field_name),
  _levelNum(level_num),
  _maxValidSecs(max_valid_secs),
  _missingInputAction(missing_input_action),
  _stormGrowthKm(storm_growth_km),
  _filterInitialized(false),
  _mdvDataAvailable(false),
  _field(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

MdvFilter::~MdvFilter()
{
  delete _field;
}


/*********************************************************************
 * initFilter() - Initialize the filter based on the data time of the
 *                storms.  This method must be called before the first
 *                call to isPassing() and must be called again when
 *                processing a storm with a new data time.
 *
 * Returns true if the initialization was successful, false otherwise.
 */

bool MdvFilter::initFilter(const DateTime& data_time)
{
  // Set the initialization flag to false so we don't try to
  // filter any storms unless we have a grid read in.

  _filterInitialized = false;
  _mdvDataAvailable = false;
  
  // Read in the grid

  if (!_readGrid(data_time))
  {
    // See what our missing input action is and set things up
    // accordingly so that later processing will work.

    switch (_missingInputAction)
    {
    // If we don't want any output when the input data is missing,
    // just return false.  This will cause the main program to just
    // skip processing the storms.

    case MISSING_NO_OUTPUT :
      return false;

      // If we send all of the input storms to one of the output databases
      // when the MDV input is missing, indicate that the filter was
      // successfully initialized, but that we don't have any MDV data.
      // This allows the isPassing() method to work in this case.

    case MISSING_PASSING :
    case MISSING_FAILING :
      _filterInitialized = true;
      _mdvDataAvailable = false;
      return true;
    }
  }
  
  _filterInitialized = true;
  _mdvDataAvailable = true;
  
  return true;
}


/*********************************************************************
 * isPassing() - Returns true if the storm passes the filter criteria,
 *               false otherwise.
 */

bool MdvFilter::isPassing(const Tstorm& storm,
			  double &algorithm_value)
{
  static const string method_name = "MdvFilter::isPassing()";
  
  // Make sure we have a grid to work with.  If we don't
  // have a grid, nothing should pass the test.

  if (!_filterInitialized)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Programming error!!" << endl;
    cerr << "Filter not initialized" << endl;
    
    return false;
  }
  
  // Make sure that we have some MDV data to work with.  If we don't,
  // respond accordingly.

  if (!_mdvDataAvailable)
  {
    switch (_missingInputAction)
    {
    case MISSING_NO_OUTPUT :
      cerr << "ERROR: " << method_name << endl;
      cerr << "Programming error!!" << endl;
      cerr << "Missing input MDV data with MISSING_NO_OUTPUT action" << endl;
      
      return false;
      
    case MISSING_PASSING :
      return true;
      
    case MISSING_FAILING :
      return false;
      
    } /* endswitch - _missingInputAction */
  }
  
  // Create a polygon from the storm polygon and grow this polygon
  // if requested

  WorldPolygon2D *polygon = storm.forecastWorldPoly(0, false);
  
  if (_stormGrowthKm != 0.0)
    polygon->growKm(_stormGrowthKm);
  
  // Now let the derrived class tell us if this storm passes the
  // appropriate test.

  bool return_value = _isPassing(polygon, algorithm_value);
  
  delete polygon;
  
  return return_value;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _readGrid() - Read in the appropriate interest grid.
 */

bool MdvFilter::_readGrid(const DateTime& data_time)
{
  static const string method_name = "MdvFilter::_readGrid()";
  
  // Get rid of any previous interest field

  delete _field;
  
  // Read in the new MDV file

  DsMdvx input_file;
  
  input_file.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  input_file.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  input_file.setReadScalingType(Mdvx::SCALING_NONE);
  
  input_file.clearReadFields();
  input_file.addReadField(_fieldName);
  
  if (_levelNum < 0)
    input_file.setReadComposite();
  else
    input_file.setReadPlaneNumLimits(_levelNum,
				     _levelNum);
  
  input_file.setReadTime(Mdvx::READ_FIRST_BEFORE,
			 _url,
			 _maxValidSecs,
			 data_time.utime());
  
  if (input_file.readVolume() != 0)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error reading interest grid:" << endl;
    input_file.printReadRequest(cerr);

    return false;
  }
  
  // Save the interest field

  _field = new MdvxField(*input_file.getFieldByNum(0));
  if (_field == 0)
    return false;
  
  return true;
}
