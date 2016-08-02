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
//   $Id: LECoverTester.cc,v 1.2 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.2 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * LECoverTester: Class for testing less than or equal to coverage values.
 *
 * RAP, NCAR, Boulder CO
 *
 * July 2007
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "LECoverTester.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

LECoverTester::LECoverTester(const double thresh_value) :
  CoverTester(),
  _threshold(thresh_value)
{
}

  
/*********************************************************************
 * Destructor
 */

LECoverTester::~LECoverTester()
{
}


/*********************************************************************
 * isIncluded() - Determine whether the given data value should be included
 *                in the coverage calculation.
 *
 * Returns true if it should be included, false otherwise.
 */

bool LECoverTester::isIncluded(const double data_value)
{
  if (data_value == _badDataValue || data_value == _missingDataValue)
    return false;
  
  if (data_value > _threshold)
    return false;
  
  return true;
}

/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/
