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
// PlaneFit.hh
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

#ifndef PlaneFit_hh
#define PlaneFit_hh

#include <vector>
using namespace std;

class PlaneFit {
  
public:
  
  // constructor

  PlaneFit();
  
  // destructor
  
  virtual ~PlaneFit();
  
  // clear the data values
  
  void clear();
  
  // add a single data point
  
  void addPoint(double xx, double yy, double zz);
  
  // set the full set of data values
  
  void setValues(const vector<double> &xVals,
                 const vector<double> &yVals,
                 const vector<double> &zVals);

  // perform the fit
  // values must have been set
  // returns 0 on success, -1 on failure
  
  int performFit();
  
  // get results after the fit
  
  double getCoeffA() const { return _aa; }
  double getCoeffB() const { return _bb; }
  double getCoeffC() const { return _cc; }
  size_t getNPoints() const { return _xVals.size(); }
  
protected:
private:

  vector<double> _xVals, _yVals, _zVals; // observations

  double _aa, _bb, _cc;
  
};

#endif
