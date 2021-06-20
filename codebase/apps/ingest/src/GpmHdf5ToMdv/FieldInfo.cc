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
/**
 *
 * @file FieldInfo.cc
 *
 * @class FieldInfo
 *
 * Class for maintaining information about a data field within an SDS field.
 *  
 * @date 11/7/2008
 *
 */

#include <iostream>

#include "FieldInfo.hh"
#include "SdsDataField.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

FieldInfo::FieldInfo(const bool debug_flag) :
  _debug(debug_flag),
  _scale(1.0),
  _bias(0.0)
{
}


/*********************************************************************
 * Destructor
 */

FieldInfo::~FieldInfo()
{
}


/*********************************************************************
 * calcMdvValue()
 */

double FieldInfo::calcMdvValue(const double raw_data_value,
			       const RadConvert &rad_convert,
			       const double sun_mag,
			       const double solar_zenith,
			       const double cos_solar_zenith) const
{
  // Check for a bad data value

  vector< double >::const_iterator bad_value;
  for (bad_value = _badValues.begin();
       bad_value != _badValues.end(); ++bad_value)
  {
    if (raw_data_value == *bad_value)
      return SdsDataField::MDV_BAD_DATA_VALUE;
  } /* endfor - bad_value */
  
  // Check for a missing data value

  vector< double >::const_iterator missing_value;
  for (missing_value = _missingValues.begin();
       missing_value != _missingValues.end(); ++missing_value)
  {
    if (raw_data_value == *missing_value)
      return SdsDataField::MDV_MISSING_DATA_VALUE;
  } /* endfor - missing_value */
  
  // Calculate the MDV data value

  double field_value = (raw_data_value / _scale) - _bias;

  // Convert radiance to brightness temperature, if appropriate

//  cerr << "Before convert, value = " << field_value << endl;
  
  field_value = rad_convert.getBT(field_value, _radConvertType,
				  sun_mag, solar_zenith, cos_solar_zenith);
  
//  cerr << "After convert, value = " << field_value << endl;
  
  return field_value;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
