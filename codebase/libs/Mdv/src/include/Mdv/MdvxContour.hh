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
////////////////////////////////////////////////////////////////////
// Mdv/MdvxContour.hh
//
// This class is used to compute contours for an MdvxField object.
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2001
//
////////////////////////////////////////////////////////////////////
//
// Initialization: set up the contour values
// ---------------
//
// Either set constant interval contours (fast) using
//
//    setVals()
//
// or arbitrary contour intervals (slower) using
//
//    clearVals()
//    loop through the values calling addVal()
//
// Line segments
// -------------
//
// These are line segments, with only 2 points, a start point and end
// point. Computing them is fast, but the placement is random so they
// are suitable for drawing lines, but not suitable for labelling.
//
// 1. Initialize the values - see above.
// 2. Call computeSegments().
// 3. Retrieve contour levels vector using getLevels(), then use the
//    segments vector for rendering.
//
// Contiguous lines
// ----------------
//
// These are contiguous lines suitable for labelling. Computing them takes
// longer than computing the segments.
//
// 1. Initialize the values - see above.
// 2. Call computeLines().
// 3. Retrieve contour levels vector using getLevels(), then use the lines
//    vector for rendering.
//
// Triangles for filled contours
// ----------------------------
//
// 1. Initialize the values - see above. Start with the minimum value
//    for the lowest contour interval, followed by the minimum value
//    for the next contour interval, and so on to the upper contour
//    interval. Then add the maximum value for the upper contour interval.
// 2. Call computeTriangles().
// 3. Retrieve contour levels vector using getLevels(), then use the
//    triangles vector for rendering. For each level there is a flag
//    called 'extra'. If extra is false, render in the color for the
//    contour interval. If extra is true, render in the background 
//    color. This is necessary to create a smooth upper edge for
//    the upper contour interval.
//
////////////////////////////////////////////////////////////////////

#ifndef MdvxContour_hh
#define MdvxContour_hh

#include <Mdv/MdvxField.hh>
#include <vector>
#include <list>
using namespace std;

class MdvxContourPt {
public:
  double x;
  double y;
};

class MdvxContourSegment {
public:
  MdvxContourPt start;
  MdvxContourPt end;
};

class MdvxContourTriangle
{
public:
  MdvxContourPt vertex[3];
};

class MdvxContourLine
{
public:
  vector<MdvxContourPt> pts;
};

class MdvxContourLevel
{
public:
  int index;
  double val;
  bool extra;
  vector<MdvxContourLine> lines;
  vector<MdvxContourSegment> segments;
  vector<MdvxContourTriangle> triangles;
};

class MdvxContour
{

public:

  ///////////////////////
  // default constructor
  
  MdvxContour();
  
  ////////////////////////////
  // Copy constructor

  MdvxContour(const MdvxContour &rhs);

  ///////////////////////
  // destructor

  virtual ~MdvxContour();
  
  // assignment
  
  MdvxContour & operator=(const MdvxContour &rhs);

  ///////////////////////////////////////////
  // set the contour vals - equal spacing

  void setVals(int n_vals, double min_value, double contour_spacing);
  void setVals(double min_value, double max_value, double spacing);

  ////////////////////////////////////////////////
  // set the contour vals - arbitrary spacing
  // Values must be added in ascending order

  void clearVals();
  void addVal(double value);

  //////////////////////////
  // print the contour vals

  void printVals(ostream &out);

  ///////////////////////////////////////////////////////////
  // compute line segments - not for labelling
  //
  // Returns 0 on success, -1 on error
  
  int computeSegments(const MdvxField &in_field, int plane_num = 0);

  //////////////////////////////////////////////////////////////
  // compute contiguous lines contour lines - good for labelling
  //
  // Returns 0 on success, -1 on error
  
  int computeLines(const MdvxField &in_field, int plane_num = 0);
  
  //////////////////////////////////////////////////////////////
  // compute triangles - for filled contours
  //
  // Returns 0 on success, -1 on error
  
  int computeTriangles(const MdvxField &in_field, int plane_num = 0);
  
  // access methods

  const vector<MdvxContourLevel> &getLevels() { return _levels; }

  int getNVals() { return _vals.size(); }
  const vector<double> &getVals() { return _vals; }
  bool isSpacingEqual() { return _spacingIsEqual; }
  double getMinVal() { return _minVal; }
  double getSpacing() { return _spacing; }

protected:

  typedef struct {
    MdvxContourPt pt;
    double code;
  } _coord_t;
  
  typedef struct {
    _coord_t start, end;
  } _segment_t;
  
  typedef struct {
    vector<_segment_t> segs;
    double val;
  } _contour_segments_t;
  
  // data

  // number of line contour segments that cross box

  static const int _NumSegs[16];

  // edge factors

  static const int _ContourEdge[16][2][2];

  // number of triangles in box

  static const int _NumTriangles[16];

  // triangle factors

  static const int _ContourTriangle[16][3][3];

  // contour levels

  vector<MdvxContourLevel> _levels;

  // contour values

  vector<double> _vals;
  double _minVal;
  double _spacing;

  // are contours on equal spacing?

  bool _spacingIsEqual;

  // functions

  MdvxContour & _copy(const MdvxContour &rhs);

  int _computeIndex(double val,
		    const vector<double> &vals);

  void _sortAndLoad(int contourNum, list<_segment_t> &segs,
		    MdvxContourLevel &level);
  
  int _computeLines(const MdvxField &in_field,
		    int plane_num,
		    bool contigLines);

private:

  static const int nAllocInc = 1000;

};

#endif











