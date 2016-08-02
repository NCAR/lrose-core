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
 *   $Id: StnInterpField.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * StnInterpField: Base class for interpolated fields based on surface
 *                 station data.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef StnInterpField_H
#define StnInterpField_H

#include <iostream>
#include <string>

#include <rapformats/station_reports.h>

#include "InterpField.hh"

using namespace std;


class StnInterpField : public InterpField
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

  StnInterpField(const string &field_name,
		 const string &field_units,
		 Interpolater *interpolater,
		 const bool output_field_flag = true,
		 const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~StnInterpField();


  /*********************************************************************
   * addObs() - Add the given observation to the intepolated field.
   *
   * Returns true on success, false on failure.
   */

  virtual bool addObs(const station_report_t &report,
		      const float *interp_distance_grid);


  /*********************************************************************
   * interpolate() - Interpolate the accumulation grids.
   *
   * Returns true on success, false on failure.
   */

  virtual bool interpolate()
  {
    return InterpField::interpolate(STATION_NAN);
  }
  

  /*********************************************************************
   * getMissingDataValue() - Get the missing data value.
   */

  virtual fl32 getMissingDataValue() const
  {
    return STATION_NAN;
  }
  

protected:
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _calcValue() - Calculates the appropriate value for this field.
   *
   * Returns the calculated value on success, STATION_NAN on failure.
   */

  virtual float _calcValue(const station_report_t &report) = 0;


};

#endif
