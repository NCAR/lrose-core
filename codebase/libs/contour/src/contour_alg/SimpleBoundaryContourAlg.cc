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
/*
 *  $Id: SimpleBoundaryContourAlg.cc,v 1.5 2016/03/03 18:17:41 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

/////////////////////////////////////////////////////////////////////////
//
// Class:	SimpleBoundaryContourAlg
//
// Author:	G. M. Cunning
//
// Date:	Wed Jan  7 14:33:55 2004
//
// Description: The simple boundary contour algorithm. This algorithm uses 
//		the RAP clumping and boundary generation libraries, located 
//		in the euclid library. 
// 
// 


// C++ include files

// System/RAP include files

// Local include files
#include <contour/SimpleBoundaryContourAlg.hh>
#include <contour/Contour.hh>

using namespace std;


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Constructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

SimpleBoundaryContourAlg::SimpleBoundaryContourAlg(const int min_overlap,
						   const int min_num_poly_pts,
						   const grid_ref_t grid_ref,
						   const bool& debug_flag) :
  ContourAlg(debug_flag),
  _minOverlap(min_overlap),
  _minNumPolyPts(min_num_poly_pts),
  _gridRef(grid_ref),
  _intervals(0),
  _intervalOrder(0),
  _clump(0),
  _clumps(0),
  _rowHeaders(0),
  _numRowHdrsAlloc(0),
  _numClumpsAlloc(0),
  _numIntervals(0),
  _numIntervalsAlloc(0),
  _boundaryList(0),
  _boundaryPoints(0),
  _nodes(0),
  _numNodesAlloc(0)
{
}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Destructors
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
  
SimpleBoundaryContourAlg::~SimpleBoundaryContourAlg()
{
  EG_free_nodes(&_numNodesAlloc, &_boundaryList, &_boundaryPoints, 
		&_nodes);
  EG_free_rowh(&_numRowHdrsAlloc, &_rowHeaders);
  EG_free_intervals(&_intervals, &_numIntervalsAlloc);
  int numIntervalsAlloc = _numIntervalsAlloc;
  EG_free_clumps(&numIntervalsAlloc, &_clumps,
		 &_intervalOrder);
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Public Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SimpleBoundaryContourAlg::generateContour
//
// Description:	initializes the object.
//
// Returns:	pointer to Contour object if successful, and 0 otherwise.
//
// Notes:	uses the clumping an boundary functions from the euclid
//		library
//

Contour *
SimpleBoundaryContourAlg::generateContour(const int& nx, const int& ny,
					  const float& dx, const float& dy,
					  const float& min_x, const float& min_y,
					  const vector<float>& levels, const float* data)
{
  const string methodName = "SimpleBoundaryContourAlg::generateContour";

  Contour *contour = new Contour();
  
  //
  // clump the data
  //
  
  // allocate space for rows
  EG_alloc_rowh(ny, &_numRowHdrsAlloc, &_rowHeaders);
  
  // loop over all the requested levels
  for(size_t i = 0; i < levels.size(); i++) {
    int numClumps = _clumpData(nx, ny, levels[i], data);
  
    // the 0th clump is not used and _clumps array is numClumps + 1 in size
    _clump = &(_clumps[1]);

    // loop through all the clumps 
    for (int clump_id = 1; clump_id <= numClumps; clump_id++, _clump++) {
      if(_clump->pts < _minNumPolyPts) {
	cerr << "WARNING: " << methodName << endl;
	cerr << "polyline has " << _clump->pts << " points, which is less than the " 
	     << _minNumPolyPts << " point minimum." << endl;
	continue;
      } // endif -- _clump->pts < _minPoints

      if (!_generateBoundary(nx, ny, 4 * _numIntervals, clump_id))
      {
	cerr << "ERROR: " << methodName << endl;
	cerr << "Cannot compute boundary." << endl;
	delete contour;
	return 0;
      }
      
      // add a new polyline
      _addPolyline(_boundaryPoints, _boundarySize,
		   dx, dy, min_x, min_y, levels[i], _gridRef, contour);
    } // endfor -- clump_id
  } // endfor -- i

  return contour;
}


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//
// Private Methods
//
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SimpleBoundaryContourAlg::_addPolyline
//
// Description:	adds a new Polyline to contour.
//
// Returns:	
//
// Notes:
//
//

void 
SimpleBoundaryContourAlg::_addPolyline(const Point_d *bdry_pts, const int& bdry_size,
				       const float& dx, const float& dy,
				       const float& min_x, const float min_y, 
				       const float& level, const grid_ref_t& grid_ref,
				       Contour* contour)
{
  ContourPolyline polyline;

  double xOffset;
  double yOffset;
  
  switch(grid_ref) {
  case LOWER_LEFT_REF:
    xOffset = 0.0;
    yOffset = 0.0;
    break;
  case LOWER_RIGHT_REF:
    xOffset = -1.0;
    yOffset = 0.0;
    break;
  case CENTER_REF:
    xOffset = -0.5;
    yOffset = -0.5;
    break;
  case UPPER_LEFT_REF:
    xOffset = 0.0;
    yOffset = -1.0;
    break;
  case UPPER_RIGHT_REF:
    xOffset = -1.0;
    yOffset = -1.0;
    break;
  default:
    cerr << "Unknown grid reference." << endl;
     return;
  } // endswitch -- _gridReference
  
  for(int i = 0; i < bdry_size; i++) {

    double x = min_x + (bdry_pts[i].x + xOffset) * dx;
    double y = min_y + (bdry_pts[i].y + yOffset) * dy;
    ContourPoint point(x, y);
    polyline.addPoint(point);
  } // endfor -- int i = 0; i < bdry_size; i++

  // close the contour
  //
  // add test to see if copntour should be closed
  // test should be if first and last points are
  // within 2dr, where dr^2 = dx^2 + dy^2
  //
  double x = min_x + (bdry_pts[0].x + xOffset) * dx;
  double y = min_y + (bdry_pts[0].y + yOffset) * dy;
  ContourPoint point(x, y);
  polyline.addPoint(point);
  
  // add polyline to contour
  contour->addPolyline(level, polyline);
}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SimpleBoundaryContourAlg::_clumpData
//
// Description:	clump the data that is located within each contour
//
// Returns:     number of clumps found
//
// Notes:
//
//

int 
SimpleBoundaryContourAlg::_clumpData(const int nx, const int ny,
				     const float level,
				     const float *data)
{
  // find the intervals
  _numIntervals = 
    EG_find_intervals_float(ny, nx, data,
			    &_intervals, &_numIntervalsAlloc,
			    _rowHeaders, level);

  // Allocate space for clumps and reset clump id's
  EG_alloc_clumps(_numIntervals, &_numClumpsAlloc, &_clumps, &_intervalOrder);
  EG_reset_clump_id(_intervals, _numIntervals);

  // find all clumps using previously found intervals
  return EG_rclump_2d(_rowHeaders, ny, 1, _minOverlap,
		      _intervalOrder, _clumps);

}


/////////////////////////////////////////////////////////////////////////
//
// Method Name:	SimpleBoundaryContourAlg::_generateBoundary
//
// Description:	generate the boundary for the given clump
//
// Returns:     true on success, false on failure
//
// Notes:
//
//

bool
SimpleBoundaryContourAlg::_generateBoundary(const int nx, const int ny,
					    const int num_nodes,
					    const int clump_id)
{
  // allocate memory for arrays and intialize

  EG_alloc_nodes(num_nodes, &_numNodesAlloc,
		 &_boundaryList, &_boundaryPoints, &_nodes);
      
  // setup the graph for the boundary

  if (EG_bdry_graph(_rowHeaders, ny, nx, _nodes, num_nodes, clump_id))
    return false;

  // traverse the graph

  _boundarySize = EG_traverse_bdry_graph(_nodes, 2L, _boundaryList);

  // generate the array of boundary points

  EG_gen_bdry(_boundaryPoints, _nodes, _boundaryList, _boundarySize);

  return true;
}

