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
//   $Date: 2016/03/04 02:22:10 $
//   $Id: QualityControl.cc,v 1.2 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * QualityControl: Class that performs quality control on a data value.
 *
 * RAP, NCAR, Boulder CO
 *
 * May 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "QualityControl.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

QualityControl::QualityControl(const double max_value_diff,
			       const bool debug_flag) :
  _debug(debug_flag),
  _maxValueDiff(max_value_diff)
{
}

  
/*********************************************************************
 * Destructor
 */

QualityControl::~QualityControl()
{
}


/*********************************************************************
 * qcData() - Quality control the given data value.
 *
 * Returns the quality controlled data value to use.
 */

double QualityControl::qcValue(const double input_value,
			       const double prev_accum_value,
			       const double output_missing_data_value) const
{
  if (_maxValueDiff > 0.0 &&
      fabs(input_value - prev_accum_value) > _maxValueDiff)
    return prev_accum_value;
  
  return input_value;
}


double QualityControl::qcValue(const double input_value,
			       const double output_missing_data_value) const
{
  return input_value;
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

