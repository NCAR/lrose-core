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
//   $Id: MeanComputer.cc,v 1.3 2016/03/04 02:22:10 dixon Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * MeanComputer: Class that computes the 2D value as the mean of the
 *               non-missing data values in each column.
 *
 * RAP, NCAR, Boulder CO
 *
 * March 2008
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include "MeanComputer.hh"

using namespace std;


/*********************************************************************
 * Constructors
 */

MeanComputer::MeanComputer() :
  Computer("Mean ")
{
}

  
/*********************************************************************
 * Destructor
 */

MeanComputer::~MeanComputer()
{
}


/**********************************************************************
 *              Protected/Private Member Functions                    *
 **********************************************************************/

/*********************************************************************
 * _initCompute() - Initialize the computation for this data column.
 */

void MeanComputer::_initCompute()
{
  _currentSum = 0.0;
  _numCurrentValues = 0;
}


/*********************************************************************
 * _addValue() - Add the given value to the current column computation.
 */

void MeanComputer::_addValue(const fl32 data_value)
{
  if (data_value != _badDataValue && data_value != _missingDataValue)
  {
    _currentSum += data_value;
    ++_numCurrentValues;
  }
}


/*********************************************************************
 * _computeValue() - Compute the final 2D value.
 *
 * Returns the value computed.
 */

fl32 MeanComputer::_computeValue()
{
  if (_numCurrentValues <= 0)
    return _missingDataValue;
  
  return _currentSum / (fl32)_numCurrentValues;
}
