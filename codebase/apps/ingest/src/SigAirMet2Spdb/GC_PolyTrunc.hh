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
#ifndef _GC_POLY_INC_
#define _GC_POLY_INC_

#include <vector>

using namespace std;
//
// This is a class that can be used to truncate a polygon in
// lat/lon space by getting rid of all the points that lie on the
// wrong side of a great circle. It works as follows.
//
// The class is constructed with the polygon lat/lons as
// arguments to the constructor, either as arrays or vectors.
//
// Note that if the last point in the input is not the same as
// the first point, then this class will append the first point to
// the end of the list, ie. it will close the polygon.
//
// Once the class is constructed, points that lie on the wrong
// side of a great circle line can be deleted by calling
// GreatCircleTrunc() - this can be called repeatedly. The
// lat1, lon1, lat2 and lon2 arguments define the Great Circle and
// the exclusionDir argument is in degrees, from 0.0 to 360.0.
//
// Extra points are inserted where points are deleted so that
// the output polygon travels along the Great Circle and
// will be drawn correctly. The polygon can be obtained
// with the 'get' functions below (remember that the last
// point will be the same as the first and so may be redundant).
//
// Typical settings for the exclusionDir and their meaning
// are as follows :
//
//   exclusionDir=0.0  => Exclude to the North of the Great Circle
//   exclusionDir=90.0  => Exclude to the East of the Great Circle
//   exclusionDir=180.0  => Exclude to the South of the Great Circle
//   exclusionDir=270.0  => Exclude to the West of the Great Circle
//
// The following calls are listed as examples :
//
// GreatCircleTrunc(lat1 = 45.0, lon1 = -80.0, lat2 = -45.0, lon2 = -80.0, exclusionDir = 90.0);
//  +--> Get rid of points East of longitude -80.0
//
// GreatCircleTrunc(lat1 = -60.0, lon1 = -80.0, lat2 = -75.0, lon2 = -80.0, exclusionDir = 90.0);
//  +--> Get rid of points East of longitude -80.0 - it's the same Great Circle at longitude -80.0
//
// GreatCircleTrunc(lat1 = 45.0, lon1 = -80.0, lat2 = -45.0, lon2 = -80.0, exclusionDir = 270.0);
//  +--> Get rid of points West of longitude -80.0
//
//
// Getting rid of points to the East or West is exact, but getting rid of
// points to the North or South is only approximately correct over a
// longitude range :
//
// GreatCircleTrunc(lat1 = 45.0, lon1 = -81.0, lat2 = 45.0, lon2 = -82.0, exclusionDir = 0.0);
//  +--> For longitudes near -81.0 and -82.0 this will get rid of points to the north
//       of approximately latitude 45N.
//
// GreatCircleTrunc(lat1 = 45.0, lon1 = -81.0, lat2 = 45.0, lon2 = -82.0, exclusionDir = 180.0);
//  +--> For longitudes near -81.0 and -82.0 this will get rid of points to the SOUTH
//       of approximately latitude 45N. The approximation is a good one.
//
//
// The most general instance of the call is something like this :
//
// GreatCircleTrunc(lat1 = -33.0, lon1 = 160.0, lat2 = -35.0, lon2 = -170.0, exclusionDir = 180.0);
//  +--> Get rid of points to the south of the line connecting 33S, 160E and 35S, 170W
//
//
// Niles Oien May 2004
//
class GC_PolyTrunc {

public:
  //
  // Constructor - from arrays and from vectors.
  //
  GC_PolyTrunc(double *lats, double *lons, int nPoints);
  GC_PolyTrunc(vector <double> lats, vector <double> lons);
  //
  // Main method - truncate according to a Great Circle.
  //
  // Returns 0 for success and 1 for failure. A failure can occurs
  // when truncating across points that are parallel. This will result
  // in a zero-length cross product.
  // 
  //
  void GreatCircleTrunc(double lat1, double lon1,
		   double lat2, double lon2,
		   double exclusionDir);
  //
  // Data retrieval - get the number of points, or the Nth point.
  //
  int getNpoints();
  //
  //
  //
  double getLat(int i);
  double getLon(int i);
  //
  // Print out the points we have.
  //
  void print();
  //
  // Destructor
  //
  ~GC_PolyTrunc();

  private :

  vector <double> _lat;
  vector <double> _lon;


bool _findGreatCircleIntersection(double lat1, double lon1,
				  double lat2, double lon2,
				  double lat3, double lon3,
				  double lat4, double lon4,
				  double *ilat, double *ilon);

void  _formEarthCenteredVector(double lat1, double lon1, 
			       double lat2, double lon2, 
			       double *X,  
			       double *Y,  
			       double *Z);

};

#endif
