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
//   $Id: InterpField.cc,v 1.6 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.6 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * InterpField: Base class for interpolated fields.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "InterpField.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

InterpField::InterpField(const string &field_name,
			 const string &field_units,
			 Interpolater *interpolater,
			 const bool output_field_flag,
			 const bool debug_flag) :
  _debug(debug_flag),
  _fieldName(field_name),
  _fieldUnits(field_units),
  _interpolater(interpolater),
  _outputField(output_field_flag),
  _interpGrid(0),
  _encodingType(Mdvx::ENCODING_INT16),
  _scalingType(Mdvx::SCALING_ROUNDED),
  _scale(1.0),
  _bias(0.0)
{
}

  
/*********************************************************************
 * Destructor
 */

InterpField::~InterpField()
{
  delete _interpolater;
  delete [] _interpGrid;
}


/*********************************************************************
 * init() - Initialize all of the accumulation grids so we can start a
 *          new interpolation.
 *
 * Returns true on success, false on failure.
 */

bool InterpField::init()
{
  return _interpolater->init();
}


/*********************************************************************
 * interpolate() - Interpolate the accumulation grids.
 *
 * Returns true on success, false on failure.
 */

bool InterpField::interpolate(const fl32 bad_data_value)
{
  delete [] _interpGrid;
  
  _interpGrid = _interpolater->getInterpolation(bad_data_value);
  
  if (_interpGrid == 0)
    return false;
  
  return true;
}

  
/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _addObs() - Add the given observation to the intepolated field.
 *
 * Returns true on success, false on failure.
 */

bool InterpField::_addObs(const double obs_value,
			  const double obs_lat,
			  const double obs_lon,
			  const double bad_obs_value,
			  const float *interp_distance_grid)
{
  return _interpolater->addObs(obs_value, obs_lat, obs_lon,
			       bad_obs_value, interp_distance_grid);
}
