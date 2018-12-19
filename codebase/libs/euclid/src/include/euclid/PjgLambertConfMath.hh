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
// /PjgLambertConfMath.hh
//
// Low-level math for Lambert Conformal Conic projection
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

#ifndef PjgLambertConfMath_hh
#define PjgLambertConfMath_hh

#include <euclid/PjgMath.hh>

class PjgLambertConfMath : public PjgMath
{

public:

  ///////////////////////
  /// constructor
  
  PjgLambertConfMath(double origin_lat,
                     double origin_lon,
                     double lat1,
                     double lat2);
  
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
  
protected:
  
  // angles in radians

  double _origin_lat_rad;
  double _origin_lon_rad;
  double _origin_colat_rad;

  // sin and cosine pre-computed
  
  double _lat1_rad;
  double _lat2_rad;
  double _n;
  double _F;
  double _rho;
  double _tan0;
  double _sin0;
  bool _2tan_line;
  
  // methods

  void _latlon2xy_2tan(double lat, double lon,
                       double &x, double &y) const;
  
  void _latlon2xy_1tan(double lat, double lon,
                       double &x, double &y) const;
  
  void _xy2latlon_2tan(double x, double y,
                       double &lat, double &lon) const;
  
  void _xy2latlon_1tan(double x, double y,
                       double &lat, double &lon) const;

private:

};

#endif











