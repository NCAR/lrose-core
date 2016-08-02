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
//   $Id: ConvDerivedField.cc,v 1.3 2016/03/07 01:50:09 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * ConvDerivedField: Class for calculating the convergence derived field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <physics/PhysicsLib.hh>

#include "ConvDerivedField.hh"
#include "Params.hh"

using namespace std;

const string ConvDerivedField::FIELD_NAME = "Convergence";
const string ConvDerivedField::FIELD_UNITS = "10**-3s-1";


/*********************************************************************
 * Constructors
 */

ConvDerivedField::ConvDerivedField(const int conv_dxdy,
				   const Pjg &output_proj,
				   const bool output_field_flag,
				   const bool debug_flag) :
  DerivedField(FIELD_NAME, FIELD_UNITS,
	       output_proj, output_field_flag, debug_flag),
  _span(conv_dxdy)
{
}

  
/*********************************************************************
 * Destructor
 */

ConvDerivedField::~ConvDerivedField()
{
}


/*********************************************************************
 * derive() - Calculate the derivation.
 *
 * Returns true on success, false on failure.
 */

bool ConvDerivedField::derive(map< int, StnInterpField* > &interp_field_list,
			      map< int, DerivedField* > &der_field_list)
{
  static const string method_name = "ConvDerivedField::getDerivation()";
  
  // Get pointers to the U and V wind fields

  fl32 *u_data;
  fl32 u_missing_value;
  
  if ((u_data = _getFieldData(interp_field_list, Params::UWIND,
			      u_missing_value)) == 0)
    return false;
  
  fl32 *v_data;
  fl32 v_missing_value;
  
  if ((v_data = _getFieldData(interp_field_list, Params::VWIND,
			      v_missing_value)) == 0)
    return false;
  
  // Make sure that the fields use the same missing data value

  if (u_missing_value != v_missing_value)
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "U and V fields don't have same missing data value" << endl;
    cerr << "U missing data value is " << u_missing_value << endl;
    cerr << "V missing data value is " << v_missing_value << endl;
    cerr << "Cannot process" << endl;
    
    return false;
  }
  
  // Allocate space for the derived data

  int grid_size = _outputProj.getNx() * _outputProj.getNy();
  
  delete [] _derivedGrid;
  _derivedGrid = new fl32[grid_size];
  
  for (int i = 0; i < grid_size; ++i)
    _derivedGrid[i] = u_missing_value;
  
  _missingDataValue = u_missing_value;
  
  // Calculate some needed values

  double dxkm = _outputProj.x2km(_outputProj.getDx());
  double dykm = _outputProj.x2km(_outputProj.getDy());
  
  if (!PhysicsLib::convergence(u_data, v_data, _derivedGrid,
			       u_missing_value, u_missing_value,
			       dxkm, dykm,
			       _outputProj.getNx(), _outputProj.getNy(), 0,
			       _span))
  {
    cerr << "ERROR: " << method_name << endl;
    cerr << "Error calculating convergence" << endl;
    
    return false;
  }
  
  return true;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
