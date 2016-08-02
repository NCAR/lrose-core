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
// DouglasPeuckerSmoother.cc
//
// Class for smoothing contour polygons using the Douglas-Peucker
// smoothing algorithm.
//
// Nancy Rehak, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// Sept 2004
//
///////////////////////////////////////////////////////////////

#include <iostream>

#include <contour/DouglasPeuckerSmoother.hh>


/*********************************************************************
 * Constructors
 */

DouglasPeuckerSmoother::DouglasPeuckerSmoother(const double epsilon,
					       const bool debug_flag) :
  ContourSmoother(debug_flag),
  _epsilon(epsilon),
  _inPts(0),
  _outPts(0),
  _ptsAlloc(0)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

DouglasPeuckerSmoother::~DouglasPeuckerSmoother()
{
  delete [] _inPts;
  delete [] _outPts;
}


/*********************************************************************
 * smoothContour() - Smooth the given contour.
 *
 * Returns true if successful, false otherwise.
 */

bool DouglasPeuckerSmoother::smoothContour(Contour &contour)
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

void DouglasPeuckerSmoother::_smoothPolyline(ContourPolyline &polyline)
{
  vector< ContourPoint > points = polyline.getPoints();
  int num_pts = points.size();
  
  if (num_pts > _ptsAlloc)
  {
    delete [] _inPts;
    delete [] _outPts;
    
    _inPts = new POINT[num_pts];
    _outPts = new int[num_pts];

    _ptsAlloc = num_pts;
  }

  // Convert polygon to format needed for smoother.  Note that we can't
  // repeat the beginning vertex at the end so we need to quit if we
  // find another point equal to the beginning point

  vector< ContourPoint >::const_iterator point;
  int i;
  
  for (point = points.begin(), i = 0; point != points.end(); ++point, ++i)
  {
    double input_x = point->getX();
    double input_y = point->getY();
    
    if (i > 0 &&
	_inPts[0][XX] == input_x &&
	_inPts[0][YY] == input_y)
      break;
    
    _inPts[i][XX] = input_x;
    _inPts[i][YY] = input_y;
  }

  num_pts = i;
  
  // Create dp_basic object
  DPbasic dp_basic(_inPts, num_pts);  

  // Call Douglas-Peucker algorithm
  int num_out_pts = dp_basic.dp(0, num_pts - 1, _epsilon, _outPts);

  // Print output
  if (_debug)
  {
    cerr << "num_out_pts = " << num_out_pts << endl;
    for (int i = 0; i < num_out_pts; ++i)
      cerr << "  " << _outPts[i] << ": " << _inPts[_outPts[i]][XX]
	   << ", " << _inPts[_outPts[i]][YY] << endl;
  }

  // Update the input polygon.  Add the beginning point again at the
  // end to complete the polygon.

  polyline.clearPoints();
  
  for (int i = 0; i < num_out_pts; ++i)
    polyline.addPoint(_inPts[_outPts[i]][XX], _inPts[_outPts[i]][YY]);

  polyline.addPoint(_inPts[_outPts[0]][XX], _inPts[_outPts[0]][YY]);
}
