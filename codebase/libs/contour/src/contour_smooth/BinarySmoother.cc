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
// BinarySmoother.cc
//
// Class for smoothing contour polygons using a simple binary
// algorithm.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Jan 2005
//
///////////////////////////////////////////////////////////////

#include <contour/BinarySmoother.hh>


/*********************************************************************
 * Constructors
 */

BinarySmoother::BinarySmoother(const int max_num_pts,
			       const bool debug_flag) :
  ContourSmoother(debug_flag),
  _maxNumPts(max_num_pts)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

BinarySmoother::~BinarySmoother()
{
  // Do nothing
}


/*********************************************************************
 * smoothContour() - Smooth the given contour.
 *
 * Returns true if successful, false otherwise.
 */

bool BinarySmoother::smoothContour(Contour &contour)
{
  // Loop through the contour so we can smooth each contour level

  map< double, ContourLevel > levels = contour._contourLevels;
  map< double, ContourLevel >::iterator level_iter;
  
  for (level_iter = levels.begin(); level_iter != levels.end(); ++level_iter)
  {
    ContourLevel *level = &(level_iter->second);
    
    // Now loop through the polylines in the contour level
    
    for (size_t i = 0; i < level->_contourPolylines.size(); ++i)
    {
      if (_debug)
      {
	cerr << "Original polyline:" << endl;
	level->_contourPolylines[i].print(cerr);
      }
      _smoothPolyline(level->_contourPolylines[i]);

      if (_debug)
      {
	cerr << "Polyline right after smoothing:" << endl;
	level->_contourPolylines[i].print(cerr);
      }

    } /* endfor - polyline */
    
    contour._contourLevels[level_iter->first] = *level;
    
  } /* endfor - level_iter */
  
  return true;
}


/**************************************************************
 * PRIVATE MEMBER FUNCTIONS
 **************************************************************/

/*********************************************************************
 * _smoothPolyline() - Smooth the given polyline.
 */

void BinarySmoother::_smoothPolyline(ContourPolyline &polyline)
{
  vector< ContourPoint > points = polyline.getPoints();
  int num_pts = points.size();
  
  // See if we really need to do anything

  if (num_pts <= _maxNumPts)
    return;
  
  // Calculate how many points should be removed between each of the
  // saved pts.

  int num_pts_removed = num_pts / _maxNumPts;
  if (num_pts % _maxNumPts == 0)
    --num_pts_removed;
  
  // Create the new polyline.

  ContourPolyline new_polyline;
  const vector< ContourPoint > polyline_pts = polyline.getPoints();
  
  for (int i = 0; i < num_pts; i += num_pts_removed + 1)
    new_polyline.addPoint(polyline_pts[i]);
  
  polyline = new_polyline;
}
