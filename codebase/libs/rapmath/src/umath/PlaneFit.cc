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
/////////////////////////////////////////////////////////////
// PlaneFit.cc
//
// Fit a plane to 3D data, using least-squares.
//
// Mike Dixon, EOL, NCAR, Boulder, CO, USA
//
// November 2020
//
///////////////////////////////////////////////////////////////
// Least-squared plane fit to 3-D data set.
//
// Fits a plane from a collection of points
// so that the summed squared distance to all points is minimized.
//
// Plane fit function is: z = ax + by + c
//
// Data x, y, z is passed in.
//
// When the fit is done, the coefficients a, b, c are determined.
//
////////////////////////////////////////////////////////////////////

#include <iostream>
#include <rapmath/PlaneFit.hh>
using namespace std;

////////////////////////////////////////////////////
// constructor

PlaneFit::PlaneFit()
        
{
  clear();
}

////////////////////////////////////////////////////
// destructor

PlaneFit::~PlaneFit()
  
{
  clear();
}

//////////////////////////////////////////////////////////////////
// clear the data values
  
void PlaneFit::clear()
{
  _xVals.clear();
  _yVals.clear();
  _zVals.clear();
}

//////////////////////////////////////////////////////////////////
// add a data point value

void PlaneFit::addPoint(double xx, double yy, double zz) 
{
  
  _xVals.push_back(xx);
  _yVals.push_back(yy);
  _zVals.push_back(zz);

}

//////////////////////////////////////////////////////////////////
// set the data values from vectors
    
void PlaneFit::setValues(const vector<double> &xVals,
                         const vector<double> &yVals,
                         const vector<double> &zVals)

{
  
  if (xVals.size() != yVals.size() ||
      xVals.size() != zVals.size()) {
    cerr << "ERROR - PlaneFit::setValues()" << endl;
    cerr << "  x, y and z value vector lengths do not match" << endl;
    cerr << "  xVals.size(): " << xVals.size() << endl;
    cerr << "  yVals.size(): " << yVals.size() << endl;
    cerr << "  zVals.size(): " << yVals.size() << endl;
    clear();
    return;
  }
    
  _xVals = xVals;
  _yVals = yVals;
  _zVals = zVals;

}

////////////////////////////////////////////////////////////////////
// do the fit
//
// Returns 0 on success, -1 on failure.

int PlaneFit::performFit()
  
{

  _aa = _bb = _cc = 0.0;
  
  size_t npts = _xVals.size();
  if (_yVals.size() != npts || _zVals.size() != npts) {
    cerr << "ERROR - uPlaneFit()" << endl;
    cerr << "  data array sizes do not match" << endl;
    cerr << "  sizes for x, y, z: "
         << _xVals.size() << ", "
         << _yVals.size() << ", "
         << _zVals.size() << endl;
    return -1;
  }
  
  if (npts < 3) {
    cerr << "ERROR - uPlaneFit, too few points: " << npts << endl;
    cerr << "Minimum npoints is 3" << endl;
    return -1;
  }

  double dnpts = npts;
  double sumx = 0.0; double sumy = 0.0; double sumz = 0.0;
  for (size_t ii = 0; ii < npts; ii++) {
    sumx += _xVals[ii];
    sumy += _yVals[ii];
    sumz += _zVals[ii];
  }
  double meanx = sumx / dnpts;
  double meany = sumy / dnpts;
  double meanz = sumz / dnpts;
  
  // Calc full 3x3 covariance matrix, excluding symmetries:
  
  double xx = 0.0; double xy = 0.0; double xz = 0.0;
  double yy = 0.0; double yz = 0.0; double zz = 0.0;

  for (size_t ii = 0; ii < npts; ii++) {
    double dx = _xVals[ii] - meanx;
    double dy = _yVals[ii] - meany;
    double dz = _zVals[ii] - meanz;
    xx += dx * dx;
    xy += dx * dy;
    xz += dx * dz;
    yy += dy * dy;
    yz += dy * dz;
    zz += dz * dz;
  }

  // compute the determinants
  
  double det_x = yy * zz - yz * yz;
  double det_y = xx * zz - xz * xz;
  double det_z = xx * yy - xy * xy;

  double det_max = det_x;
  if (det_max < det_y) {
    det_max = det_y;
  }
  if (det_max < det_z) {
    det_max = det_z;
  }
  if (det_max <= 0.0) {
    return -1; // The points don't span a plane
  }
  
  _aa = yz * xy - xz * yy;
  _bb = xy * xz - xx * yz;
  _cc = meanz;

  return 0;
  
#ifdef NOTNOW

  // Pick path with best conditioning:

  if (det_max == det_x) {
    x = det_x;
    y = xz * yz - xy * zz;
    z = xy * yz - xz * yy;
  } else if (det_max == det_y) {
    x = xz * yz - xy * zz;
    y = det_y;
    z = xy * xz - yz * xx;
  } else {
    x = xy * yz - xz * yy;
    y = xy * xz - yz * xx;
    z = det_z;
  }

#endif
    
}
