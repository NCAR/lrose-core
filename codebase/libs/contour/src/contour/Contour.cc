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
// Contour.cc
//
// Class representing a contour of a rectangular grid.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2000
//
///////////////////////////////////////////////////////////////

#include <iostream>

#include <contour/Contour.hh>
#include <contour/ContourPoint.hh>


/*********************************************************************
 * Constructors
 */

Contour::Contour(const bool& debug_flag) :
  _debugFlag(debug_flag)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

Contour::~Contour()
{
  // Do nothing
}


/*********************************************************************
 * addPolyline() - Add the given polyline to the given level of the
 *                 contour.
 */

void Contour::addPolyline(const double& level_value,
			  const ContourPolyline& polyline)
{
  // See if we currently have any polylines at the given level

  map< double, ContourLevel >::iterator level_iter;
  
  if ((level_iter = _contourLevels.find(level_value)) ==
      _contourLevels.end())
  {
    // First polyline for the given level

    ContourLevel contour_level(level_value);
    
    contour_level.addPolyline(polyline);
    
    _contourLevels[level_value] = contour_level;
  }
  else
  {
    // Add this polyline to an already existing level

    level_iter->second.addPolyline(polyline);
  }
  
}


/*********************************************************************
 * print() - Print the contents of the contour object to the given
 *           stream.
 */

void Contour::print(ostream &out) const
{
  map< double, ContourLevel >::iterator level_iter;
  
  for (level_iter = _contourLevels.begin(); level_iter != _contourLevels.end();
       ++level_iter)
  {
    double level_value = level_iter->first;
    ContourLevel level = level_iter->second;
    
    out << "Contour for level " << level_value << ":" << endl;
    level.print(out);
    
  } /* endfor - level_iter */
  
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

