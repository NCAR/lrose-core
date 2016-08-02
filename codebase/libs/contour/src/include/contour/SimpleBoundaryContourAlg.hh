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
 *  $Id: SimpleBoundaryContourAlg.hh,v 1.4 2016/03/03 18:17:42 dixon Exp $
 *
 */
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/

//////////////////////////////////////////////////////////////////////////
// 
// Header:	SimpleBoundaryContourAlg
// 
// Author:	G. M. Cunning
// 
// Date:	Wed Jan  7 13:52:36 2004
// 
// Description:	The simple boundary contour algorithm. This algorithm uses 
//		the RAP clumping and boundary generation libraries, located 
//		in the euclid library. 
// 
// 


# ifndef    SIMPLE_CONTOUR_ALG_H
# define    SIMPLE_CONTOUR_ALG_H

// C++ include files
#include <string>
#include <vector>

// System/RAP include files
#include <euclid/clump.h>
#include <euclid/boundary.h>
#include <contour/ContourAlg.hh>

// Local include files

using namespace std;


class Contour;

class SimpleBoundaryContourAlg : public ContourAlg {
  
public:

  ////////////////////
  // public members //
  ////////////////////

  //
  // In computing x and y, we add an offset to take into account
  // the bdry_pts from the clumping library is relative to the bottom left-
  // hand corner. grid_ref_t is used to apply the appropriate offset.
  //
  typedef enum {
    LOWER_LEFT_REF,
    LOWER_RIGHT_REF,
    CENTER_REF,
    UPPER_LEFT_REF,
    UPPER_RIGHT_REF
  } grid_ref_t;

  ////////////////////
  // public methods //
  ////////////////////

  // constructor
  // min_overlap - controls the minumum amount of overlap required to form a clump. 
  //   The definition of min_overlap is:
  //     If min_overlap is +n, then the intervals must overlap by n 
  //     grid positions.
  //     If min_overlap is 0, the intervals 'overlap' if their corners
  //     touch diagonally.
  //     If min_overlap is -n, the intervals are considered to
  //     overlap if there is a gap of n grid positions or less between them.
  // min_num_poly_pts - minimum number of points for a polyline to considered
  // grid_ref - the grid reference type

  SimpleBoundaryContourAlg(const int min_overlap = 0,
			   const int min_num_poly_pts = 10,
			   const grid_ref_t grid_ref = CENTER_REF,
			   const bool& debug_flag = false);
 
  // min_overlap - controls the minumum amount of overlap required to form a clump. 
  //   The definition of min_overlap is:
  //     If min_overlap is +n, then the intervals must overlap by n 
  //     grid positions.
  //     If min_overlap is 0, the intervals 'overlap' if their corners
  //     touch diagonally.
  //     If min_overlap is -n, the intervals are considered to
  //     overlap if there is a gap of n grid positions or less between them.

  void setMinOverlap(const int min_overlap)
  {
    _minOverlap = min_overlap;
  }
  
  // min_num_poly_pts - minimum number of points for a polyline to considered

  void setMinNumPolyPts(const int min_num_poly_pts)
  {
    _minNumPolyPts = min_num_poly_pts;
  }
  
  // grid_ref - the grid reference type

  void setGridRef(const grid_ref_t grid_ref)
  {
    _gridRef = grid_ref;
  }
  
  // calculate the contours
  //
  // nx - number of points in x-direction
  // ny - number of points in y-direction
  // dx - grid spacing in x-direction
  // dy - grid spacing in y-direction
  // min_x - leftmost value of grid in x-direction
  // min_y - lowest value of grid in y-direction
  // levels - list of contour levsls
  // data - the data of which we will contour
  //
  // Returns the calculated contour on success, 0 on failure

  virtual Contour* generateContour(const int& nx, const int& ny,
				   const float& dx, const float& dy,
				   const float& min_x, const float& min_y,
				   const vector<float>& levels,
				   const float* data);

  // destructor
  virtual ~SimpleBoundaryContourAlg();

protected:

  ///////////////////////
  // protected members //
  ///////////////////////
  
  int _minOverlap;
  int _minNumPolyPts;
  grid_ref_t _gridRef;
  
 // clumping members
  Interval *_intervals;
  Interval **_intervalOrder;
  Clump_order *_clump;
  Clump_order *_clumps;
  Row_hdr *_rowHeaders;
  int _numRowHdrsAlloc;
  int _numClumpsAlloc;
  int _numIntervals;
  int _numIntervalsAlloc;

  // boundary members
  int *_boundaryList;
  Point_d *_boundaryPoints;
  int _boundarySize;
  Node *_nodes;
  int _numNodesAlloc;

  ///////////////////////
  // protected methods //
  ///////////////////////

  void _addPolyline(const Point_d *bdry_pts, const int& bdry_size,
		    const float& dx, const float& dy,
		    const float& min_x, const float min_y, 
		    const float& level, const grid_ref_t& grid_ref,
		    Contour* contour);

  virtual int _clumpData(const int nx, const int ny,
			 const float level,
			 const float *data);
  
  virtual bool _generateBoundary(const int nx, const int ny,
				 const int num_nodes,
				 const int clump_id);
  
};


# endif     /* SIMPLE_CONTOUR_ALG_H */
