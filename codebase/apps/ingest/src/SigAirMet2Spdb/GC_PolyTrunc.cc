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

#include "GC_PolyTrunc.hh"
#include <iostream>
#include <cmath>
#include <toolsa/pjg_flat.h>
//
// Constructor - from arrays and from vectors.
// The points are closed if they were not already, ie.
// the starting point is also at the end of the array.
//
GC_PolyTrunc::GC_PolyTrunc(double *lats, 
			   double *lons, 
			   int nPoints){

  _lat.clear(); _lon.clear();
  for (int i=0; i < nPoints; i++){
    _lat.push_back( lats[i] );
    _lon.push_back( lons[i] );
  }
  //
  // push back the first point as the last, if this is not
  // already the case.
  //
  if (
      (_lat[0] != _lat[ _lat.size()-1 ]) ||
      (_lon[0] != _lon[ _lon.size()-1 ])
      ){
    _lat.push_back( lats[0] );
    _lon.push_back( lons[0] );
  }
}

GC_PolyTrunc::GC_PolyTrunc(vector <double> lats, 
			   vector <double> lons){
  _lat.clear(); _lon.clear();
  for (unsigned i=0; i < lats.size(); i++){
    _lat.push_back( lats[i] );
    _lon.push_back( lons[i] );
  }
  //
  // push back the first point as the last, if this is not
  // already the case.
  //
  if (
      (_lat[0] != _lat[ _lat.size()-1 ]) ||
      (_lon[0] != _lon[ _lon.size()-1 ])
      ){
    _lat.push_back( lats[0] );
    _lon.push_back( lons[0] );
  }
}
//
// Main method - truncate according to a Great Circle.
//
void GC_PolyTrunc::GreatCircleTrunc(double lat1, double lon1,
				   double lat2, double lon2,
				   double exclusionDir){
  //
  // First, see which of these points we want to keep, and
  // which ones we want to delete.
  //
  vector <bool> weWantIt;
  weWantIt.clear();

  for (unsigned i=0; i < _lat.size(); i++){

    double pLat = _lat[i]; double pLon = _lon[i];
    //
    // Move 50km in the exclude-to direction.
    //
    double qLat, qLon;
    //
    PJGLatLonPlusRTheta(pLat, pLon,
			50.0, exclusionDir,
                        &qLat, &qLon);
    //
    // Get the intersection of the two great circles.
    //
    double gLat, gLon;
    
    if(!_findGreatCircleIntersection(pLat, pLon, qLat, qLon,
				     lat1, lon1, lat2, lon2,
				     &gLat, &gLon)){
      cerr << "Could not find an intersection point." << endl;
      return;
    }

    //
    // Get the direction from P to G
    //
    double r,theta;
    PJGLatLon2RTheta(pLat, pLon, gLat, gLon, &r, &theta);

    //  cerr << i << " : " << exclusionDir << " " << theta;

    const double PI = 3.1415927;
    if (
	(fabs(cos(PI*exclusionDir/180.0) - cos(PI*theta/180.0)) < 0.05) &&
	(fabs(sin(PI*exclusionDir/180.0) - sin(PI*theta/180.0)) < 0.05)
	){
      weWantIt.push_back( true );
      // cerr << " Yes!" << endl;
    } else {
      weWantIt.push_back( false );
      // cerr << " No!" << endl;
    }
    
  }
  //
  // Now, replace the points as needed.
  //
  bool hasCrossed = (!(weWantIt[0]));
  //
  vector <double> newLat; newLat.clear();
  vector <double> newLon; newLon.clear();

  for (unsigned i=0; i < _lat.size(); i++){

    if (weWantIt[i]){
      //
      // We want this point.
      //
      if (hasCrossed){
	double nLat, nLon;
	if(_findGreatCircleIntersection(_lat[i], _lon[i], _lat[i-1], _lon[i-1],
				     lat1, lon1, lat2, lon2,
					&nLat, &nLon))
	  {
	     newLat.push_back(nLat);    newLon.push_back(nLon);
	     newLat.push_back(_lat[i]); newLon.push_back(_lon[i]);
	     hasCrossed = false;
	  }
      } else {
	newLat.push_back(_lat[i]); newLon.push_back(_lon[i]);
      }
    } else {
      //
      // We don't want this point.
      //
      if (!(hasCrossed)){
	hasCrossed = true;
	//
	double nLat, nLon;
	if(_findGreatCircleIntersection(_lat[i], _lon[i], _lat[i-1], _lon[i-1],
				     lat1, lon1, lat2, lon2,
					&nLat, &nLon)) {
	  newLat.push_back(nLat);    newLon.push_back(nLon);
	}
      }
    }
  }

  _lat.clear(); _lon.clear();
  for(unsigned i=0; i < newLat.size(); i++){
    //
    cerr << i << " : " << newLat[i] << ", " << newLon[i] << endl;
    //
    _lat.push_back(newLat[i]);  _lon.push_back(newLon[i]);
    //
  }

  //
  // push back the first point as the last, if this is not
  // already the case. Added by Niles May 2005.
  //
  if (
      (_lat[0] != _lat[ _lat.size()-1 ]) ||
      (_lon[0] != _lon[ _lon.size()-1 ])
      ){
    _lat.push_back( _lat[0] );
    _lon.push_back( _lon[0] );
  }
  return;
}

//
// Data retrieval - get the number of points, or the Nth point.
//
int GC_PolyTrunc::getNpoints(){
  return _lat.size();
}
//
//
//
double GC_PolyTrunc::getLat(int i){
  return _lat[i];
}

double GC_PolyTrunc::getLon(int i){
  return _lon[i];
}
//
// Print out the points we have.
//
void GC_PolyTrunc::print(){
  for (unsigned i=0; i < _lat.size(); i++){
    cerr << "Point " << i << " : ";
    cerr << _lat[i] << ", ";
    cerr << _lon[i] << endl;
  }
  cerr << endl;
}
//
// Destructor
//
GC_PolyTrunc::~GC_PolyTrunc(){
  
}
//
////////////////////////////////////////////////////////
//
// Private methods.
//
////////////////////////////////////////////////////////
//
// This routine finds the intersection of the two Great Circles
// defined by two arcs on the earth. Each arc is defined by
// two points, so we have a total of four input points.
//
// One point is returned, but it should be noted that
// that antipodal point (-ilat, ilon+180.0) is also
// a point of intersection. We choose to return the point
// closest to lat1, lon1.
//
// The code is based on an algorithm from the web at
//
// http://williams.best.vwh.net/intersect.htm
//
// Niles Oien May 2004
//
bool GC_PolyTrunc::_findGreatCircleIntersection(double lat1, double lon1,
					   double lat2, double lon2,
					   double lat3, double lon3,
					   double lat4, double lon4,
					   double *ilat, double *ilon){
  //
  const double PI = 3.1415927;
  //
  // Get earth centered vectors for the two arcs.
  //
  double eaX, eaY, eaZ;
  _formEarthCenteredVector(lat1, lon1, lat2, lon2, &eaX, &eaY, &eaZ);

  double ebX, ebY, ebZ;
  _formEarthCenteredVector(lat3, lon3, lat4, lon4, &ebX, &ebY, &ebZ);
  //
  // Form the cross product.
  //
  double eX, eY, eZ;
  eX = eaY*ebZ - ebY*eaZ;
  eY = eaZ*ebX - ebZ*eaX;
  eZ = eaX*ebY - eaY*ebX;

  //
  // test for cross product of parallel or nearly parallel segments. The 
  // cross poduct componenst will be zero.
  if((fabs(eX) < 0.00001) && (fabs(eY) < 0.00001) && (fabs(eZ) < 0.00001)) {
    return false;
  }

  //
  // Decode the result into a lat/lon point. Also calculate the antipodal
  // point.
  //
  double ansLat = 180.0 * atan2(eZ, sqrt(eX*eX + eY*eY))/PI;
  double ansLon = 180.0 * atan2(-eY, eX)/PI;
  //
  double antLat = -ansLat;
  double antLon = ansLon + 180.0;
  //
  if (antLon > 180.0) antLon -= 360.0;
  //
  // Mathematically, either of these two is correct.
  // Mathematically, either of these two is correct.
  // Return whichever is closest to lat1, lon1

  double ansDist, ansTheta;
  double antDist, antTheta;
  PJGLatLon2RTheta(lat1, lon1, ansLat, ansLon, &ansDist, &ansTheta);
  PJGLatLon2RTheta(lat1, lon1, antLat, antLon, &antDist, &antTheta);

  if (ansDist < antDist){
    *ilat = ansLat;
    *ilon = ansLon;
  } else {
    *ilat = antLat;
    *ilon = antLon;
  }
  //
  return true;
  //
}

//
// Form an earth centered vector for two points defining an arc.
//
void GC_PolyTrunc::_formEarthCenteredVector(double lat1, double lon1, 
					    double lat2, double lon2, 
					    double *X,  
					    double *Y,  
					    double *Z){ 
  
  //
  const double PI = 3.1415927;
  //
  //
  // Convert to radians for trig calls.
  //
  lat1 = PI*lat1/180.0;
  lat2 = PI*lat2/180.0;
  lon1 = PI*lon1/180.0;
  lon2 = PI*lon2/180.0;

  double x,y,z;

  x = sin(lat1-lat2)*sin((lon1+lon2)/2.0)*cos((lon1-lon2)/2.0)-
    sin(lat1+lat2)*cos((lon1+lon2)/2.0)*sin((lon1-lon2)/2.0);


  y = sin(lat1-lat2)*cos((lon1+lon2)/2.0)*cos((lon1-lon2)/2.0) +
    sin(lat1+lat2)*sin((lon1+lon2)/2.0)*sin((lon1-lon2)/2.0);

  z = cos(lat1)*cos(lat2)*sin(lon1-lon2);

  double mod = x*x + y*y + z*z; mod = sqrt(mod);

  *X = x / mod; *Y = y/mod; *Z = z/mod;

  return;

}

