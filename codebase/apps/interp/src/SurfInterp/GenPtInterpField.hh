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

/* RCS info
 *   $Author: dixon $
 *   $Locker:  $
 *   $Date: 2016/03/07 01:50:09 $
 *   $Id: GenPtInterpField.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * GenPtInterpField: Class for interpolated fields based on GenPt data.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef GenPtInterpField_H
#define GenPtInterpField_H

#include <iostream>
#include <string>

#include <rapformats/GenPt.hh>

#include "InterpField.hh"

using namespace std;


class GenPtInterpField : public InterpField
{
  
public:

  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   *
   * Note that this object takes control of the Interpolater pointer and
   * deletes the pointer in the destructor.
   */

  GenPtInterpField(const string &genpt_field_name,
		   const string &output_field_name,
		   const string &output_field_units,
		   Interpolater *interpolater,
		   const bool output_field_flag = true,
		   const bool debug_flag = false);
  
  GenPtInterpField(const string &genpt_field_name,
		   const double genpt_missing_value,
		   const string &output_field_name,
		   const string &output_field_units,
		   Interpolater *interpolater,
		   const bool output_field_flag = true,
		   const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~GenPtInterpField();


  /*********************************************************************
   * addObs() - Add the given observation to the intepolated field.
   *
   * Returns true on success, false on failure.
   */

  virtual bool addObs(const GenPt &obs,
		      const float *interp_distance_grid);


  /*********************************************************************
   * interpolate() - Interpolate the accumulation grids.
   *
   * Returns true on success, false on failure.
   */

  virtual bool interpolate()
  {
    return InterpField::interpolate(MISSING_DATA_VALUE);
  }
  

  /*********************************************************************
   * getMissingDataValue() - Get the missing data value.
   */

  virtual fl32 getMissingDataValue() const
  {
    return MISSING_DATA_VALUE;
  }
  

protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const float MISSING_DATA_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  string _genptFieldName;
  
  bool _checkMissing;
  double _inputMissingValue;
  
};

#endif
