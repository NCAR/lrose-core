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
//////////////////////////////////////////////////////////
// MdvxContour.cc
//
// Class for computing Contours from an MdvxField object
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// June 2001
//
//////////////////////////////////////////////////////////
//
// See <Mdv/MdvxContour.hh> for details.
//
///////////////////////////////////////////////////////////

#include <functional>
#include <algorithm>
#include <deque>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxContour.hh>
#include <toolsa/toolsa_macros.h>
#include <toolsa/mem.h>
#include <unistd.h>
using namespace std;

const int MdvxContour::_NumSegs[16] =
{0, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 0};

// edge factors

const int MdvxContour::_ContourEdge[16][2][2] = {
  {{-1, -1}, {-1, -1}},    // 0
  {{ 0,  3}, {-1, -1}},    // 1
  {{ 0,  1}, {-1, -1}},    // 2
  {{ 1,  3}, {-1, -1}},    // 12
  {{ 1,  2}, {-1, -1}},    // 4
  {{ 0,  3}, { 1,  2}},    // 14
  {{ 0,  2}, {-1, -1}},    // 24
  {{ 2,  3}, {-1, -1}},    // 124
  {{ 2,  3}, {-1, -1}},    // 8
  {{ 0,  2}, {-1, -1}},    // 18
  {{ 0,  1}, { 2,  3}},    // 28
  {{ 1,  2}, {-1, -1}},    // 128
  {{ 1,  3}, {-1, -1}},    // 84
  {{ 0,  1}, {-1, -1}},    // 148
  {{ 0,  3}, {-1, -1}},    // 248
  {{-1, -1}, {-1, -1}},    // 1248
};  

// number of triangles in box

const int MdvxContour::_NumTriangles[16] =
{0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2};


// triangle factors

const int MdvxContour::_ContourTriangle[16][3][3] = {
  {{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}},    // 0
  {{ 0,  3,  4}, {-1, -1, -1}, {-1, -1, -1}},    // 1
  {{ 0,  5,  1}, {-1, -1, -1}, {-1, -1, -1}},    // 2
  {{ 3,  4,  5}, { 5,  1,  3}, {-1, -1, -1}},    // 12
  {{ 1,  6,  2}, {-1, -1, -1}, {-1, -1, -1}},    // 4
  {{ 0,  3,  4}, { 1,  6,  2}, {-1, -1, -1}},    // 14
  {{ 0,  5,  6}, { 6,  2,  0}, {-1, -1, -1}},    // 24
  {{ 3,  4,  5}, { 3,  5,  2}, { 2,  5,  6}},    // 124
  {{ 2,  7,  3}, {-1, -1, -1}, {-1, -1, -1}},    // 8
  {{ 0,  7,  4}, { 0,  2,  7}, {-1, -1, -1}},    // 18
  {{ 2,  7,  3}, { 0,  5,  1}, {-1, -1, -1}},    // 28
  {{ 4,  2,  7}, { 4,  1,  2}, { 4,  5,  1}},    // 128
  {{ 1,  6,  7}, { 1,  7,  3}, {-1, -1, -1}},    // 84
  {{ 7,  4,  0}, { 7,  0,  1}, { 7,  1,  6}},    // 148
  {{ 6,  7,  3}, { 6,  3,  0}, { 6,  0,  5}},    // 248
  {{ 4,  5,  6}, { 4,  6,  7}, {-1, -1, -1}},    // 1248
};  

////////////////////////////////////////////////////////////////////////
// Default constructor
//

MdvxContour::MdvxContour()
  
{
}

////////////////////////////
// Copy constructor

MdvxContour::MdvxContour(const MdvxContour &rhs)

{
  _copy(rhs);
}

/////////////////////////////
// Destructor

MdvxContour::~MdvxContour()

{
}

/////////////////////////////
// Assignment
//

MdvxContour &MdvxContour::operator=(const MdvxContour &rhs)

{
  return _copy(rhs);
}

//////////////////////////////////////////////////
// _copy - used by copy constructor and operator =
//

MdvxContour &MdvxContour::_copy(const MdvxContour &rhs)

{
  
  if (&rhs == this) {
    return *this;
  }

  return *this;
  
}

/////////////////////////////////////////
// set the contour vals - equal spacing

void MdvxContour::setVals(int n_vals, double min_value, double spacing)

{
  _vals.clear();
  for (int i = 0; i < n_vals; i++) {
    _vals.push_back(min_value + spacing * i);
  }
  _spacingIsEqual = true;
  _spacing = spacing;
  _minVal = min_value;
}

void MdvxContour::setVals(double min_value, double max_value, double spacing)

{
  int n_vals = (int) ((max_value - min_value) / spacing);
  setVals(n_vals, min_value, spacing);
}

///////////////////////////////////////////////
// set the contour vals - arbitrary spacing
//
// Contours must be added in ascending order

void MdvxContour::clearVals()
{
  _vals.clear();
}

void MdvxContour::addVal(double value)
{
  _vals.push_back(value);
  int nn = _vals.size();
  if (nn == 1) {
    _minVal = value;
    _spacing = value;
    _spacingIsEqual = true;
  } else {
    if (nn == 2) {
      if (_vals[1] > _vals[0]) {
	_spacingIsEqual = true;
	_spacing = _vals[1] - _vals[0];
      }
    }
    double delta = _vals[nn-1] - _vals[nn-2];
    double diff = fabs(delta - _spacing);
    if (diff > 1.0e-10) {
      _spacingIsEqual = false; 
    }
    if (_vals[nn-1] <= _vals[nn-2]) {
      cerr << "WARNING - MdvxContour::addVal()" << endl;
      cerr << "  Contours should be added in ascending order" << endl;
      _spacingIsEqual = false;
      sort(_vals.begin(), _vals.end());
    }
  }
}

////////////////////
// print the values

void MdvxContour::printVals(ostream &out)
{

  out << "N contour vals: " << _vals.size() << endl;
  out << "  Vals: ";
  for (size_t ii = 0; ii < _vals.size(); ii++) {
    out << _vals[ii];
    if (ii < _vals.size() - 1) {
      out << ", ";
    } else {
      out << endl;
    }
  }

}

///////////////////////////////////////////////////////////
// compute line segments - not for labelling
//
// Returns 0 on success, -1 on error

int MdvxContour::computeSegments(const MdvxField &in_field,
				 int plane_num /* = 0*/ )
  
{

  _levels.clear();

  if (in_field.getFieldHeader().encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  int nVals = _vals.size();
  if (nVals < 1) {
    cerr << "ERROR - MdvxContour::computeSegments" << endl;
    cerr << "  Must have at least 1 contour specified" << endl;
    return -1;
  }
  
  return _computeLines(in_field, plane_num, false);
 
}

//////////////////////////////////////////////////////////////
// compute contiguous lines contour lines - good for labelling
//
// Returns 0 on success, -1 on error

int MdvxContour::computeLines(const MdvxField &in_field,
			      int plane_num /* = 0*/ )
  
{

  _levels.clear();

  if (in_field.getFieldHeader().encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  int nVals = _vals.size();
  if (nVals < 1) {
    cerr << "ERROR - MdvxContour::computeLines" << endl;
    cerr << "  Must have at least 1 contour specified" << endl;
    return -1;
  }
  
  return _computeLines(in_field, plane_num, true);
 
}

///////////////////////////////////////////////////////////
// compute lines
//
// Returns 0 on success, -1 on error

int MdvxContour::_computeLines(const MdvxField &in_field,
			       int plane_num,
			       bool contigLines)
  
{

  // set up segment array for each level
  
  int nVals = _vals.size();
  list<_segment_t> *segsPerLevel = new list<_segment_t>[nVals];

  // make a copy of the plane for the field, convert field to float

  MdvxField field(in_field, plane_num);
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  field.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  field.transform2Linear();

  int nx, ny, nz;
  double minx, miny;
  double dx, dy;

  if (fhdr.proj_type == Mdvx::PROJ_VSECTION) {
    nx = fhdr.nx;
    ny = fhdr.nz;
    nz = fhdr.ny;
    minx = fhdr.grid_minx;
    miny = fhdr.grid_minz;
    dx = fhdr.grid_dx;
    dy = fhdr.grid_dz;
  } else {
    nx = fhdr.nx;
    ny = fhdr.ny;
    nz = fhdr.nz;
    minx = fhdr.grid_minx;
    miny = fhdr.grid_miny;
    dx = fhdr.grid_dx;
    dy = fhdr.grid_dy;
  }

  if (plane_num > nz - 1) {
    cerr << "ERROR - MdvxContour::_computeLines" << endl;
    cerr << "  Plane num too high: " << plane_num << endl;
    cerr << "  Max plane num: " << nz - 1 << endl;
    delete[] segsPerLevel;
    return -1;
  }
  
  fl32 *data = ((fl32 *) field.getVol()) + plane_num * nx * ny;
  fl32 missing = fhdr.missing_data_value;
  fl32 bad = fhdr.bad_data_value;
  int *square_thresh = new int[nVals];
  double xRange = dx * (nx + 1);

  double minVal = _vals[0];
  double maxVal = _vals[nVals-1];

  // loop through grid
  
  for (int iy = 0; iy < ny - 1; iy++) {

    bool copyPrev = false;
    int cindex0 = 0, cindex1 = 0, cindex2 = 0, cindex3 = 0;
    double yy;

    if (fhdr.proj_type == Mdvx::PROJ_VSECTION) {
      yy = vhdr.level[iy];
      dy = vhdr.level[iy+1] - vhdr.level[iy];
    } else {
      yy = miny + iy * dy;
    }
    
    for (int ix = 0; ix < nx - 1; ix++) {

      double xx = minx + ix * dx;

      // set the corner values
      // 0: lower-left
      // 1: lower-right
      // 2: upper-right
      // 3: upper-left
      
      fl32 *vv = data + (iy * nx + ix);
      fl32 v0 = *vv;
      vv++;
      fl32 v1 = *vv;
      vv += nx;
      fl32 v2 = *vv;
      vv--;
      fl32 v3 = *vv;

      int first_contour, last_contour;

      if (v0 == missing || v1 == missing || v2 == missing || v3 == missing ||
	  v0 == bad || v1 == bad || v2 == bad || v3 == bad) {
	
	copyPrev = false;
	continue;
	
      } else if (v0 < minVal && v1 < minVal && v2 < minVal && v3 < minVal) {

	copyPrev = false;
	continue;
	
      } else if (v0 > maxVal && v1 > maxVal && v2 > maxVal && v3 > maxVal) {

	copyPrev = false;
	continue;
	
      } else {
	
	// compute the index of the first contour below the value
	// of the corner points
	
	if (copyPrev) {
	  cindex0 = cindex1;
	  cindex3 = cindex2;
	} else {
	  cindex0 = _computeIndex(v0, _vals);
	  cindex3 = _computeIndex(v3, _vals);
	}
	cindex1 = _computeIndex(v1, _vals);
	cindex2 = _computeIndex(v2, _vals);
	copyPrev = true;
	
	for (int k = 0; k < nVals; k++) {
	  square_thresh[k] = 0;
	}
	for (int k = 0; k < cindex0; k++) {
	  square_thresh[k] = 1;
	}
	for (int k = 0; k < cindex1; k++) {
	  square_thresh[k] |= 2;
	}
	for (int k = 0; k < cindex2; k++) {
	  square_thresh[k] |= 4;
	}
	for (int k = 0; k < cindex3; k++) {
	  square_thresh[k] |= 8;
	}
	
	first_contour = 0;
	while (square_thresh[first_contour] == 15 && first_contour < nVals-1) {
	  first_contour++;
	}
	last_contour = nVals - 1;
	while (square_thresh[last_contour] == 0  && last_contour >= first_contour) {
	  last_contour--;
	}

      } // if (v0 == missing ...
	  
      for (int ic = first_contour; ic <= last_contour; ic++) {

	for (int kk = 0; kk < _NumSegs[square_thresh[ic]]; kk++) {
	  
	  MdvxContourPt points[2];

	  for (int ll = 0; ll < 2; ll++) {

	    switch (_ContourEdge[square_thresh[ic]][kk][ll]) {
	    case 0:
	      {
		double rdist0 = v1 - _vals[ic];
		double distance1 = v1 - v0;
		double interp = rdist0 / distance1;
		points[ll].x = xx + dx - dx * interp;
		points[ll].y = yy;
	      }
	    break;
	    case 1:
	      {
		double rdist0 = v2 - _vals[ic];
		double distance1 = v2 - v1;
		double interp = rdist0 / distance1;
		points[ll].x = xx + dx;
		points[ll].y = yy + dy - dy * interp;
	      }
	    break;
	    case 2:
	      {
		double rdist0 = v2 - _vals[ic];
		double distance1 = v2 - v3;
		double interp = rdist0 / distance1;
		points[ll].x = xx + dx - dx * interp;
		points[ll].y = yy + dy;
	      }
	    break;
	    case 3:
	      {
		double rdist0 = v3 - _vals[ic];
		double distance1 = v3 - v0;
		double interp = rdist0 / distance1;
		points[ll].x = xx;
		points[ll].y = yy + dy - dy * interp;
	      }
	    break;
	    case -1:
	      {
		cerr << "got -1 from sqth: " << square_thresh[ic] << endl;
		sleep(1);
	      }
	    break;

	    } // switch

	  } // ll

	  // store the segment

	  _segment_t seg;
          MEM_zero(seg);
	  seg.start.pt = points[0];
	  if (contigLines) {
	    seg.start.code = points[0].y * xRange + points[0].x;
	  }
	  seg.end.pt = points[1];
	  if (contigLines) {
	    seg.end.code = points[1].y * xRange + points[1].x;
	  }
	  segsPerLevel[ic].push_back(seg);
	  
	} // kk

      } // ic
      
    } // ix

  } // iy
  
  for (int ic = 0; ic < nVals; ic++) {

    MdvxContourLevel level;
    level.index = ic;
    level.val = _vals[ic];
    _levels.push_back(level);

    if (contigLines) {

      // contiguous lines
      // sort the segments, load up the contour lines
      
      _sortAndLoad(ic, segsPerLevel[ic], _levels[_levels.size()-1]);

    } else {

      // segments only

      list<_segment_t> &segList = segsPerLevel[ic];
      vector<MdvxContourSegment> &outSegs = _levels[ic].segments;

      list<_segment_t>::iterator ii;
      for (ii = segList.begin(); ii != segList.end(); ii++) {
	MdvxContourSegment outSeg;
	outSeg.start = (*ii).start.pt;
	outSeg.end = (*ii).end.pt;
	outSegs.push_back(outSeg);
      } // ii
      
    } // if (contigLines)
    
  } // ic

  delete[] segsPerLevel;
  delete[] square_thresh;

  return 0;

}

//////////////////////////////////////////////////////////////
// compute triangles for filled contours
//
// Returns 0 on success, -1 on error

int MdvxContour::computeTriangles(const MdvxField &in_field,
				 int plane_num /* = 0*/ )
  
{

  _levels.clear();

  if (in_field.getFieldHeader().encoding_type == Mdvx::ENCODING_RGBA32) {
    // no-op
    return 0;
  }

  int nVals = _vals.size();
  if (nVals < 1) {
    cerr << "ERROR - MdvxContour::computeTriangles" << endl;
    cerr << "  Must have at least 1 contour specified" << endl;
    return -1;
  }
  
  // make a copy of the plane for the field, convert field to float
  
  MdvxField field(in_field, plane_num);
  const Mdvx::field_header_t &fhdr = field.getFieldHeader();
  const Mdvx::vlevel_header_t &vhdr = field.getVlevelHeader();
  field.convertType(Mdvx::ENCODING_FLOAT32, Mdvx::COMPRESSION_NONE);
  field.transform2Linear();

  int nx, ny, nz;
  double minx, miny;
  double dx, dy;

  if (fhdr.proj_type == Mdvx::PROJ_VSECTION) {
    nx = fhdr.nx;
    ny = fhdr.nz;
    nz = fhdr.ny;
    minx = fhdr.grid_minx;
    miny = fhdr.grid_minz;
    dx = fhdr.grid_dx;
    dy = fhdr.grid_dz;
  } else {
    nx = fhdr.nx;
    ny = fhdr.ny;
    nz = fhdr.nz;
    minx = fhdr.grid_minx;
    miny = fhdr.grid_miny;
    dx = fhdr.grid_dx;
    dy = fhdr.grid_dy;
  }

  if (plane_num > nz - 1) {
    cerr << "ERROR - MdvxContour::computeTriangles" << endl;
    cerr << "  Plane num too high: " << plane_num << endl;
    cerr << "  Max plane num: " << nz - 1 << endl;
    return -1;
  }

  // copy the vals vector
  
  vector<double> workVals = _vals;

  // add an extra value at the top end if the max val is less than
  // the field max

  bool extra = false;
  if (workVals[nVals-1] < fhdr.max_value) {
    workVals.push_back(fhdr.max_value);
    extra = true;
  }
  int nWorkVals = workVals.size();

  // set up the level vector

  for (int ic = 0; ic < nVals; ic++) {
    MdvxContourLevel level;
    level.index = ic;
    level.val = _vals[ic];
    if (extra && ic == nVals - 1) {
      level.extra = true;
    } else {
      level.extra = false;
    }
    _levels.push_back(level);
  } // ic

  fl32 *data = ((fl32 *) field.getVol()) + plane_num * nx * ny;
  fl32 missing = fhdr.missing_data_value;
  fl32 bad = fhdr.bad_data_value;
  int *square_thresh = new int[nWorkVals];

  double minVal = _vals[0];
  double maxVal = _vals[nVals-1];
  
  // loop through grid
  
  for (int iy = 0; iy < ny - 1; iy++) {

    bool copyPrev = false;
    int cindex0 = 0, cindex1 = 0, cindex2 = 0, cindex3 = 0;
    double yy;

    if (fhdr.proj_type == Mdvx::PROJ_VSECTION) {
      yy = vhdr.level[iy];
      dy = vhdr.level[iy+1] - vhdr.level[iy];
    } else {
      yy = miny + iy * dy;
    }
    
    for (int ix = 0; ix < nx - 1; ix++) {

      double xx = minx + ix * dx;

      // set the corner values
      // 0: lower-left
      // 1: lower-right
      // 2: upper-right
      // 3: upper-left
      
      fl32 *vv = data + (iy * nx + ix);
      fl32 v0 = *vv;
      vv++;
      fl32 v1 = *vv;
      vv += nx;
      fl32 v2 = *vv;
      vv--;
      fl32 v3 = *vv;

      int first_contour, last_contour;

      if (v0 == missing || v1 == missing || v2 == missing || v3 == missing ||
	  v0 == bad || v1 == bad || v2 == bad || v3 == bad) {

	copyPrev = false;
	continue;

      } else if (v0 < minVal && v1 < minVal && v2 < minVal && v3 < minVal) {

	copyPrev = false;
	continue;
	
      } else if (v0 > maxVal && v1 > maxVal && v2 > maxVal && v3 > maxVal) {

	copyPrev = false;
	continue;
	
      } else {

	// compute the index of the first contour below the value
	// of the corner points
	
	if (copyPrev) {
	  cindex0 = cindex1;
	  cindex3 = cindex2;
	} else {
	  cindex0 = _computeIndex(v0, workVals);
	  cindex3 = _computeIndex(v3, workVals);
	}
	cindex1 = _computeIndex(v1, workVals);
	cindex2 = _computeIndex(v2, workVals);
	copyPrev = true;
	
	for (int k = 0; k < nWorkVals; k++) {
	  square_thresh[k] = 0;
	}
	for (int k = 0; k < cindex0; k++) {
	  square_thresh[k] = 1;
	}
	for (int k = 0; k < cindex1; k++) {
	  square_thresh[k] |= 2;
	}
	for (int k = 0; k < cindex2; k++) {
	  square_thresh[k] |= 4;
	}
	for (int k = 0; k < cindex3; k++) {
	  square_thresh[k] |= 8;
	}
	
	first_contour = cindex3;
	while (square_thresh[first_contour] != 15 && first_contour > 0) {
	  first_contour--;
	}
	      
	last_contour = first_contour;
	while (square_thresh[last_contour] != 0 && last_contour < nWorkVals) {
	  last_contour++;
	}

      } // if (v0 == missing ...

      for (int ic = first_contour; ic < last_contour; ic++) {

	MdvxContourLevel &level = _levels[ic];
	
	if (square_thresh[ic] == 15 && level.extra) {
	  continue;
	}

	for (int kk = 0; kk < _NumTriangles[square_thresh[ic]]; kk++) {
	  
	  MdvxContourTriangle triangle;
	  
	  for (int ll = 0; ll < 3; ll++) {

	    switch (_ContourTriangle[square_thresh[ic]][kk][ll]) {
	    case 0:
	      {
		double rdist0 = v1 - workVals[ic];
		double distance1 = v1 - v0;
		double interp = rdist0 / distance1;
		triangle.vertex[ll].x = xx + dx - dx * interp;
		triangle.vertex[ll].y = yy;
	      }
	    break;
	    case 1:
	      {
		double rdist0 = v2 - workVals[ic];
		double distance1 = v2 - v1;
		double interp = rdist0 / distance1;
		triangle.vertex[ll].x = xx + dx;
		triangle.vertex[ll].y = yy + dy - dy * interp;
	      }
	    break;
	    case 2:
	      {
		double rdist0 = v2 - workVals[ic];
		double distance1 = v2 - v3;
		double interp = rdist0 / distance1;
		triangle.vertex[ll].x = xx + dx - dx * interp;
		triangle.vertex[ll].y = yy + dy;
	      }
	    break;
	    case 3:
	      {
		double rdist0 = v3 - workVals[ic];
		double distance1 = v3 - v0;
		double interp = rdist0 / distance1;
		triangle.vertex[ll].x = xx;
		triangle.vertex[ll].y = yy + dy - dy * interp;
	      }
	    break;
	    case 4:
	      {
		triangle.vertex[ll].x = xx;
		triangle.vertex[ll].y = yy;
	      }
	    break;
	    case 5:
	      {
		triangle.vertex[ll].x = xx + dx;
		triangle.vertex[ll].y = yy;
	      }
	    break;
	    case 6:
	      {
		triangle.vertex[ll].x = xx + dx;
		triangle.vertex[ll].y = yy + dy;
	      }
	    break;
	    case 7:
	      {
		triangle.vertex[ll].x = xx;
		triangle.vertex[ll].y = yy + dy;
	      }
	    break;
	    case -1:
	      {
		cerr << "got -1 from sqth: " << square_thresh[ic] << endl;
		sleep(1);
	      }
	    break;
	    
	    } // switch
	    

	  } // ll
	  
	  // store the triangle
	  
	  level.triangles.push_back(triangle);
	  
	} // kk

      } // ic
      
    } // ix

  } // iy

  delete[] square_thresh;

  return 0;

}

void MdvxContour::_sortAndLoad(int contourNum,
			       list<_segment_t> &segs,
			       MdvxContourLevel &level)

{

  if (segs.size() < 1) {
    return;
  }

  // set up coord double-ended queue

  deque<_coord_t> coords;
  _coord_t startCoord, endCoord;

  while (segs.size() > 0) {
    
    // if queue is empty, load up the points from the first segment
    
    if (coords.size() == 0) {
      _segment_t seg = *segs.begin();
      segs.pop_front();
      coords.push_back(seg.start);
      coords.push_back(seg.end);
    }

    startCoord = coords[0];
    endCoord = coords[coords.size()-1];
    
    // search for points with matching codes
    
    list<_segment_t>::iterator ii;

    bool matchFound = false;

    for (ii = segs.begin(); ii != segs.end(); ii++) {
      
      _segment_t searchSeg = *ii;

      if (startCoord.code == searchSeg.end.code) {

	// search seg goes at front, in same order
	
	coords.push_front(searchSeg.start);
	segs.erase(ii);
	matchFound = true;
	break;
	
      } else if (startCoord.code == searchSeg.start.code) {
	
	// search seg goes at front, reverse the order
	
	coords.push_front(searchSeg.end);
	segs.erase(ii);
	matchFound = true;
	break;
	
      } else if (endCoord.code == searchSeg.start.code) {
	
	// search seg goes at back, in same order
	
	coords.push_back(searchSeg.end);
	segs.erase(ii);
	matchFound = true;
	break;
	
      } else if (endCoord.code == searchSeg.end.code) {
	
	// search seg goes at back, reverse the order
	
	coords.push_back(searchSeg.start);
	segs.erase(ii);
	matchFound = true;
	break;
	
      }
      
    } // ii

    // a contour is complete when either:
    //   (a) no match is found, or
    //   (b) it closes onto itself

    if (!matchFound || (coords[0].code == coords[coords.size()-1].code) ||
	segs.size() == 0) {

      // load up the line

      MdvxContourLine new_line;
      level.lines.push_back(new_line);
      MdvxContourLine &line = level.lines[level.lines.size() - 1];
  
      for (size_t ii = 0; ii < coords.size(); ii++) {
	line.pts.push_back(coords[ii].pt);
      }

      coords.clear();

    } // if (!matchFound ...

  } // while

}
  
//////////////////////////////////////////
// compute the contour index from a value

int MdvxContour::_computeIndex(double val,
			       const vector<double> &vals)

{

  int nn = vals.size();

  if (val < vals[0]) {
    return 0;
  } else if (val >= vals[nn-1]) {
    return nn - 1;
  }

  // equal spacing?

  if (_spacingIsEqual) {
    int pos = (int) ((val - _minVal) / _spacing) + 1;
    return pos;
  }
  
  // try bifurcation search
  
  int pos = nn / 2;
  int delta = nn / 2;
  for (int ic = 0; ic < nn; ic++) {
    delta /= 2;
    if (delta == 0) {
      delta = 1;
    }
    if (val < vals[pos]) {
      if (val >= vals[pos-1]) {
	return pos;
      } else {
	pos -= delta;
	if (pos == 0) {
	  pos = 1;
	}
      }
    } else {
      pos += delta;
      if (pos >= nn-1) {
	pos = nn-1;
      }
    }
  } // ic

  // bifurcation search failed - do linear search
  // of the corner points
  
  cerr << "WARNING - compute_contour_index - bifurcation seach failed" << endl;
  
  for (int ic = nn - 1; ic > 0; ic--) {
    if (val < vals[ic]) {
      return ic;
    }
  }

  cerr << "WARNING - compute_contour_index - should not reach here" << endl;
  
  return 0;

}

