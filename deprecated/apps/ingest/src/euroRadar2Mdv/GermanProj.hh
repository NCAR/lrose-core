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
// GermanProj.hh
//
// GermanProj object
//
//
///////////////////////////////////////////////////////////////

#ifndef GermanProj_H
#define GermanProj_H

#include <cmath>

using namespace std;

class GermanProj {
  
public:
  
  // constructor. Does some initial setup based on the
  // projection parameters from the german folks.
  GermanProj (double TangentLat, double TangentLon,
	      double OriginLat, double OriginLon,
	      int OriginIX, int OriginIY,
	      double Dx, double Dy);

  // 
  void getGermanXY(double lat, double lon,
		   double *x,  double *y);
  

  void getGermanXYIndex(double lat, double lon,
			int *ix,  int *iy);
  

  // destructor.
  ~GermanProj();

  
protected:
  
private:

  double _TangentLat, _TangentLon;
  double _OriginLat, _OriginLon;
  int _OriginIX, _OriginIY;
  double _Dx, _Dy;
  double _GermanEarthRadius;

  double _tangent_lon_rad;
  double _pi;
  double _sin_tangent_lat;

  double _x0, _y0;

};

#endif
