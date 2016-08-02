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
//   $Id: FltCatDerivedField.cc,v 1.2 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * FltCatDerivedField: Class for calculating the flight category derived field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <physics/PhysicsLib.hh>

#include "FltCatDerivedField.hh"
#include "Params.hh"

using namespace std;

const string FltCatDerivedField::FIELD_NAME = "Flight_Cat";
const string FltCatDerivedField::FIELD_UNITS = "none";
const fl32 FltCatDerivedField::MISSING_DATA_VALUE = -999.0;


/*********************************************************************
 * Constructors
 */

FltCatDerivedField::FltCatDerivedField(const float bad_ceiling_value,
				       const float max_alt_error,
				       const vector< flt_cat_thresh_t > &flt_cat_thresholds,
				       const Pjg &output_proj,
				       const bool output_field_flag,
				       const bool debug_flag) :
  DerivedField(FIELD_NAME, FIELD_UNITS,
	       output_proj, output_field_flag, debug_flag),
  _badCeilingValue(bad_ceiling_value),
  _maxAltError(max_alt_error),
  _fltCatThresholds(flt_cat_thresholds)
{
  _missingDataValue = MISSING_DATA_VALUE;
}

  
/*********************************************************************
 * Destructor
 */

FltCatDerivedField::~FltCatDerivedField()
{
}


/*********************************************************************
 * derive() - Calculate the derivation.
 *
 * Returns true on success, false on failure.
 */

bool FltCatDerivedField::derive(map< int, StnInterpField* > &interp_field_list,
				map< int, DerivedField* > &der_field_list)
{
  static const string method_name = "FltCatDerivedField::getDerivation()";
  
  // Get pointers to the needed fields

  fl32 *trc_data;
  fl32 trc_missing_value;
  
  if ((trc_data = _getFieldData(der_field_list,
				Params::TERRAIN_RELATIVE_CEILING,
				trc_missing_value)) == 0)
    return false;
  
  fl32 *vis_data;
  fl32 vis_missing_value;
  
  if ((vis_data = _getFieldData(interp_field_list,
				Params::VISIBILITY,
				vis_missing_value)) == 0)
    return false;
  
  fl32 *alt_data;
  fl32 alt_missing_value;
  
  if ((alt_data = _getFieldData(interp_field_list,
				Params::ALTITUDE,
				alt_missing_value)) == 0)
    return false;
  
  fl32 *terr_data;
  fl32 terr_missing_value;
  
  if ((terr_data = _getFieldData(der_field_list,
				 Params::TERRAIN,
				 terr_missing_value)) == 0)
    return false;
  
  // Allocate space for the derived data

  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  delete [] _derivedGrid;
  _derivedGrid = new fl32[grid_size];
  memset(_derivedGrid, 0, grid_size * sizeof(fl32));
  
  // Calculate the flight categories

  for (int i = 0; i < grid_size; ++i)
  {
    // Get the ceiling value to use

    float ceiling;
    
    if (trc_data[i] == trc_missing_value)
    {
      if (_badCeilingValue < 0.0)
	ceiling = trc_missing_value;
      else
	ceiling = _badCeilingValue;
    }
    else
    {
      ceiling = trc_data[i];
    }
    
    // If ceiling or visibility are missing, then the flight category is
    // also missing.

    if (ceiling == trc_missing_value || vis_data[i] == vis_missing_value)
    {
      _derivedGrid[i] = MISSING_DATA_VALUE;
      continue;
    }
    
    // Check for altitude errors

    if (_maxAltError > 0.0 &&
	fabs(alt_data[i] - terr_data[i]) > _maxAltError)
    {
      _derivedGrid[i] = MISSING_DATA_VALUE;
      continue;
    }

    // If we get here, then we can put in a value for the flight category

    vector< flt_cat_thresh_t >::const_iterator flt_cat_thresh;
    
    _derivedGrid[i] = 10.0;
    
    for (flt_cat_thresh = _fltCatThresholds.begin();
	 flt_cat_thresh != _fltCatThresholds.end(); ++flt_cat_thresh)
    {
      if (ceiling >= flt_cat_thresh->ceil_thresh &&
	  vis_data[i] >= flt_cat_thresh->vis_thresh)
	_derivedGrid[i] += 10.0;
      
    } /* endfor - flt_cat_thresh */
    
  } /* endfor - i */
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
