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
// /PjgObliqueStereoMath.hh
//
// Low-level math for Stereographic projection in the
// oblique aspect
//
// Mike Dixon, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// July 2008
//
////////////////////////////////////////////////////////////////////
// NOTE: do not include this file directly.
// Include only the base class file, <euclid/PjgMath.hh>
////////////////////////////////////////////////////////////////////

#ifndef PjgObliqueStereoMath_hh
#define PjgObliqueStereoMath_hh

#include <euclid/PjgMath.hh>

class PjgObliqueStereoMath : public PjgMath
{

public:

  ///////////////////////
  /// constructor
  
  PjgObliqueStereoMath(double tangent_lat,
                       double tangent_lon,
                       double central_scale = 1.0);
  
  ////////////////////////////////////////////////
  /// functions for XY to LatLon / LatLon to XY

  /// convert lat/lon to x/y
  /// lat/lon in deg
  /// x, y in km
  /// z ignored for this projection
  
  virtual void latlon2xy(double lat, double lon,
                         double &x, double &y,
                         double z = -9999) const;
  
  /// convert x/y to lat/lon
  /// lat/lon in deg
  /// x, y in km
  /// z ignored for this projection
  
  virtual void xy2latlon(double x, double y,
                         double &lat, double &lon,
                         double z = -9999) const;
  
 /////////////////////
  /// print the object
  
  virtual void print(ostream &out) const; // basic params only
 
  /// print all details

  virtual void printDetails(ostream &out) const; // includes derived params
  
  /// get proj string
  
  virtual string getProjStr() const;
  
protected:
  
  double _tangent_lat_rad;
  double _tangent_lon_rad;
  double _sin_tangent_lat;
  double _cos_tangent_lat; 

private:

};

#endif











