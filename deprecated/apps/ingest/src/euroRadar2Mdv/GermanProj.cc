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
// GermanProj.cc
//
// GermanProj object
//
//
///////////////////////////////////////////////////////////////

using namespace std;

#include "GermanProj.hh"

#include <iostream>
  
// constructor. Does some initial setup based on the
// projection parameters from the german folks.
GermanProj::GermanProj (double TangentLat, double TangentLon,
			double OriginLat, double OriginLon,
			int OriginIX, int OriginIY,
			double Dx, double Dy){

  // Make internal copies.
  
  _TangentLat = TangentLat;  _TangentLon = TangentLon;
  _OriginLat = OriginLat;    _OriginLon = OriginLon;
  _OriginIX = OriginIX;      _OriginIY = OriginIY;
  _Dx = Dx;                  _Dy = Dy;

  // Do a few calculations.

  _pi = acos(-1.0);
  _GermanEarthRadius = 6371.04;

  _tangent_lon_rad = _pi * _TangentLon / 180.0;
  _sin_tangent_lat = sin( _pi * TangentLat / 180.0 );

  getGermanXY(_OriginLat, _OriginLon, &_x0, &_y0 );

  return;
}

// 
void GermanProj::getGermanXY(double lat, double lon,
			     double *x,  double *y){


  double sin_lat1 = sin( _pi * lat / 180.0 );
  double cos_lat1 = cos( _pi * lat / 180.0 ); 
  double sin_delta_lon = sin(  (_pi * lon / 180.0) - _tangent_lon_rad);
  double cos_delta_lon = cos(  (_pi * lon / 180.0) - _tangent_lon_rad);

  double m = (1.0 + _sin_tangent_lat) / (1.0 + sin_lat1);

  *x =  _GermanEarthRadius * m * cos_lat1 * sin_delta_lon;
  *y =  -1.0 * _GermanEarthRadius * m * cos_lat1 * cos_delta_lon;
  
  return;

}

void GermanProj::getGermanXYIndex(double lat, double lon,
				  int *ix,  int *iy){

  double x,y;
  getGermanXY(lat, lon, &x, &y);

  *ix = _OriginIX + (int)rint((x - _x0) / _Dx);

  *iy = _OriginIY - (int)rint((y - _y0) / _Dy);

  return;

}
  
// destructor.
GermanProj::~GermanProj(){
  return;
}

  


