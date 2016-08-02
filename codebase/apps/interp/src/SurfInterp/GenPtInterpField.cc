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
//   $Id: GenPtInterpField.cc,v 1.2 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * GenPtInterpField: Class for interpolated fields based on GenPt data.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "GenPtInterpField.hh"

using namespace std;

const float GenPtInterpField::MISSING_DATA_VALUE = -99999.0;


/*********************************************************************
 * Constructors
 */

GenPtInterpField::GenPtInterpField(const string &genpt_field_name,
				   const string &output_field_name,
				   const string &output_field_units,
				   Interpolater *interpolater,
				   const bool output_field_flag,
				   const bool debug_flag) :
  InterpField(output_field_name, output_field_units, interpolater,
	      output_field_flag, debug_flag),
  _genptFieldName(genpt_field_name),
  _checkMissing(false),
  _inputMissingValue(-9999.0)
{
}

  
GenPtInterpField::GenPtInterpField(const string &genpt_field_name,
				   const double genpt_missing_value,
				   const string &output_field_name,
				   const string &output_field_units,
				   Interpolater *interpolater,
				   const bool output_field_flag,
				   const bool debug_flag) :
  InterpField(output_field_name, output_field_units, interpolater,
	      output_field_flag, debug_flag),
  _genptFieldName(genpt_field_name),
  _checkMissing(true),
  _inputMissingValue(genpt_missing_value)
{
}


/*********************************************************************
 * Destructor
 */

GenPtInterpField::~GenPtInterpField()
{
}


/*********************************************************************
 * addObs() - Add the given observation to the intepolated field.
 *
 * Returns true on success, false on failure.
 */

bool GenPtInterpField::addObs(const GenPt &obs,
			      const float *interp_distance_grid)
{
  int field_num;
  float obs_value;
  
  if ((field_num = obs.getFieldNum(_genptFieldName)) < 0)
  {
    obs_value = MISSING_DATA_VALUE;
  }
  else
  {
    obs_value = obs.get1DVal(field_num);
    
    if (_checkMissing && obs_value == _inputMissingValue)
      obs_value = MISSING_DATA_VALUE;
  }
  
  _interpolater->addObs(obs_value, obs.getLat(), obs.getLon(),
			MISSING_DATA_VALUE, interp_distance_grid);
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

