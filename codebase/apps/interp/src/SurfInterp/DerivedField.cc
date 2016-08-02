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
//   $Id: DerivedField.cc,v 1.4 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.4 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * DerivedField: Base class for derived fields.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "DerivedField.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

DerivedField::DerivedField(const string &field_name,
			   const string &field_units,
			   const Pjg &output_proj,
			   const bool output_field_flag,
			   const bool debug_flag) :
  _debug(debug_flag),
  _fieldName(field_name),
  _fieldUnits(field_units),
  _outputField(output_field_flag),
  _outputProj(output_proj),
  _derivedGrid(0),
  _missingDataValue(-999.0),
  _encodingType(Mdvx::ENCODING_INT16),
  _scalingType(Mdvx::SCALING_ROUNDED),
  _scale(1.0),
  _bias(0.0)
{
}

  
/*********************************************************************
 * Destructor
 */

DerivedField::~DerivedField()
{
  delete [] _derivedGrid;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _getFieldData() - Get the data for the given field from the given
 *                   field list.
 *
 * Returns a pointer to the static data on success, 0 on failure.
 */

fl32 *DerivedField::_getFieldData(map< int, StnInterpField* > &interp_field_list,
				  const int field_key,
				  fl32 &missing_value) const
{
  map< int, StnInterpField*, less<int> >::iterator field_iter;
  
  if ((field_iter = interp_field_list.find(field_key))
      == interp_field_list.end())
    return 0;
  
  StnInterpField *field = field_iter->second;
  
  missing_value = field->getMissingDataValue();
  
  return field->getInterpolation();
  
}


fl32 *DerivedField::_getFieldData(map< int, DerivedField* > &der_field_list,
				  const int field_key,
				  fl32 &missing_value) const
{
  map< int, DerivedField*, less<int> >::iterator field_iter;
  
  if ((field_iter = der_field_list.find(field_key))
      == der_field_list.end())
    return 0;
  
  DerivedField *field = field_iter->second;
  
  missing_value = field->getMissingDataValue();
  
  return field->getDerivation();
  
}
