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
 *   $Id: PrecipRateInterpField.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * PrecipRateInterpField: Class for controlling the creation of the precip
 *                        rate interpolated field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef PrecipRateInterpField_H
#define PrecipRateInterpField_H

#include <iostream>
#include <string>

#include "StnInterpField.hh"

using namespace std;


class PrecipRateInterpField : public StnInterpField
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

  PrecipRateInterpField(Interpolater *interpolater,
			const bool output_field_flag = true,
			const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~PrecipRateInterpField();


protected:
  
  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const string FIELD_NAME;
  static const string UNITS;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /*********************************************************************
   * _calcValue() - Calculates the appropriate value for this field.
   *
   * Returns the calculated value on success, STATION_NAN on failure.
   */

  virtual float _calcValue(const station_report_t &report);



};

#endif
