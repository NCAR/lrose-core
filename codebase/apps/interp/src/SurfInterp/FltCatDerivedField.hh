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
 *   $Id: FltCatDerivedField.hh,v 1.2 2016/03/07 01:50:09 dixon Exp $
 *   $Revision: 1.2 $
 *   $State: Exp $
 */
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/************************************************************************
 * FltCatDerivedField: Class for calculating the flight category derived field.
 *
 * RAP, NCAR, Boulder CO
 *
 * September 2007
 *
 * Nancy Rehak
 *
 ************************************************************************/

#ifndef FltCatDerivedField_H
#define FltCatDerivedField_H

#include <iostream>
#include <string>
#include <vector>

#include "DerivedField.hh"

using namespace std;


class FltCatDerivedField : public DerivedField
{
  
public:

  //////////////////
  // Public types //
  //////////////////

  typedef struct
  {
    float ceil_thresh;
    float vis_thresh;
  } flt_cat_thresh_t;
  
  
  ////////////////////
  // Public methods //
  ////////////////////

  /*********************************************************************
   * Constructors
   */

  FltCatDerivedField(const float bad_ceiling_value,
		     const float max_alt_error,
		     const vector< flt_cat_thresh_t > &flt_cat_thresholds,
		     const Pjg &output_proj,
		     const bool output_field_flag = true,
		     const bool debug_flag = false);
  

  /*********************************************************************
   * Destructor
   */

  virtual ~FltCatDerivedField();


  /*********************************************************************
   * derive() - Calculate the derivation.
   *
   * Returns true on success, false on failure.
   */

  virtual bool derive(map< int, StnInterpField* > &interp_field_list,
		      map< int, DerivedField* > &der_field_list);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////
  
  static const string FIELD_NAME;
  static const string FIELD_UNITS;
  static const fl32 MISSING_DATA_VALUE;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  float _badCeilingValue;
  float _maxAltError;
  
  vector< flt_cat_thresh_t > _fltCatThresholds;
  

};

#endif
