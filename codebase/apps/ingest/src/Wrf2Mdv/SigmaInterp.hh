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
// SigmaInterp.h
//
// This class wraps interpolation of Sigma levels to other
// vertical level types
//
// Mike Dixon, RAP, NCAR, P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// March 2003
//
/////////////////////////////////////////////////////////////

#ifndef SigmaInterp_H
#define SigmaInterp_H

#include <string>
#include <vector>
#include <physics/IcaoStdAtmos.hh> 
using namespace std;

typedef struct {
  int valid;
  int upperIsigma;
  int lowerIsigma;
  double wtUpper;
  double wtLower;
} SigmaInterp_interp_t;

class SigmaInterp {
  
public:

  // constructor
  
  SigmaInterp ();
  
  // destructor
  
  ~SigmaInterp ();
  
  // Before calling prepareInterp, you need to set pressure, height
  // or flight levels. So you need to call one of the following:
  //  - setPressureLevels()
  //  - setHeightLevels()
  //  - setFlightLevels()
  
  void setPressureLevels(int n_levels, const double *pressure);
  void setPressureLevels(const vector<double> pressure);

  void setHeightLevels(int n_levels, const double *height);
  void setHeightLevels(const vector<double> height);

  void setFlightLevels(int n_levels, const double *flevel);
  void setFlightLevels(const vector<double> flevel);

  // Prepare interpolation info for a sigma pressure profile for
  // a given point.
  
  void prepareInterp(const vector<double> &p_sigma);
  
  // Interpolate a field from sigma to flight levels.
  //
  // Passed in is the array for the field column in sigma levels.
  //
  // If copy_lowest_downwards is true, the lowest non-missing data will
  // be copied down to the lowest level.
  //
  // Fills out vector with interpolated data.
  
  void doInterp(const vector<double> &field_sigma,
		vector<double> &interp_data,
		bool copy_lowest_downwards);

  // print pressure array

  void printPressureArray(ostream &out);

  // flags to indicate whether a given sigma level is
  // required for interpolation
  
  const vector<bool> &getSigmaNeeded() { return _sigmaNeeded; }

protected:
  
private:

  static const double MISSING_DOUBLE;

  // interp index
  
  vector<SigmaInterp_interp_t> _interpIndex;

  // flags to indicate whether a given sigma level is
  // required for interpolation

  vector<bool> _sigmaNeeded;

  // pressures

  vector<double> _pressure;
  
  // ICAO standard atmosphere object
  
  IcaoStdAtmos _isa;

};

#endif

