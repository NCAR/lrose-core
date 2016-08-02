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
///////////////////////////////////////////////////////////////
// ContourLevel.cc
//
// Class representing a single level of a contour of a
// rectangular grid.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>

#include <contour/ContourLevel.hh>


/*********************************************************************
 * Constructors
 */

ContourLevel::ContourLevel(const double& level_value,
			   const bool& debug_flag) :
  _debugFlag(debug_flag),
  _levelValue(level_value),
  _contourPolylinesIter(_contourPolylines.end())
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

ContourLevel::~ContourLevel()
{
  // Do nothing
}


/*********************************************************************
 * print() - Print the contents of the contour object to the given
 *           stream.
 */

void ContourLevel::print(ostream &out) const
{
  vector< ContourPolyline >::const_iterator polyline;
  int i;
  
  for (polyline = _contourPolylines.begin(), i = 0;
       polyline != _contourPolylines.end(); ++polyline, ++i)
  {
    out << "Polyline " << i << ":" << endl;
    polyline->print(out);
  }
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ***********************************\***************************/

