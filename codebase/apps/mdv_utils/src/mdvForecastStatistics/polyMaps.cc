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
//
// Niles Oien, RAP, NCAR,
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// October 2006
//
/////////////////////////////////////////////////////////////

#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <euclid/boundary.h>
#include <euclid/geometry.h>

#include "polyMaps.hh"

/**
 *
 * @file polyMaps.cc
 *
 * Implementation of polyMaps class.
 *
 * @author Niles Oien
 *
 */

using namespace std;


  
polyMaps::polyMaps(string mapFilename,
		   Mdvx::master_header_t mhdr,
		   Mdvx::field_header_t fhdr){

  //
  // Init the pointer to NULL, open the input file, set up an MdvxProj object.
  //
  _mapMask = NULL;

  FILE *fp = fopen(mapFilename.c_str(),"r");
  if (fp == NULL) {
    cerr << "Unable to open " << mapFilename << endl;
    return;
  }

  MdvxProj Proj(mhdr, fhdr);
  

  //
  // Loop through the file. Pass over lines that do
  // not have the words INSIDE or OUTSIDE in them.
  //
  // Once we encounter one of these lines, read the points
  // from the ensuing lines.
  //
  char Line[1024];
  int lineNum = 0;
  while (NULL != fgets(Line, 1024, fp)){

    lineNum++;

    polyMaps::indexMap_t inMap;
    inMap.intPoints.clear();
    int numPoints=0;

    if (!(strncmp("OUTSIDE", Line, strlen("OUTSIDE")))){
      inMap.retainPointsInside = false;
      if (1 != sscanf(Line, "OUTSIDE %d", &numPoints)){
	cerr << mapFilename;
	cerr << " map format error at OUTSIDE statement, line ";
	cerr << lineNum << endl;
	fclose(fp);
	return;
      }
    }

    if (!(strncmp("INSIDE", Line, strlen("INSIDE")))){
      inMap.retainPointsInside = true;
      if (1 != sscanf(Line, "INSIDE %d", &numPoints)){
	cerr << "Map format error at INSIDE statement, line ";
	cerr << lineNum << endl;
	fclose(fp);
	return;
      }
    }

    if ((numPoints > 0) && (numPoints < 3)){
      cerr << mapFilename << " warning - map has only ";
      cerr << numPoints;
      cerr << " points, makes no sense, will be ignored, line ";
      cerr << lineNum << endl;
    }

    for (int i=0; i < numPoints; i++){
      if (NULL == fgets(Line, 1024, fp)){
	cerr << mapFilename;
	cerr << " premature end of file, line ";
	cerr << lineNum << endl;
	fclose(fp);
	return;
      }
      lineNum++;

      //
      // Go from lat/lon to ix/iy using the Proj object.
      //      
      double lat, lon;
     
      if (2 != sscanf(Line,"%lf %lf", &lat, &lon)){
	cerr << mapFilename;
	cerr << " formatting error reading lat/lon point, line ";
	cerr << lineNum << endl;
	fclose(fp);
	return;
      }

      polyMaps::intPoint_t mapPoint;

      if (Proj.latlon2xyIndex(lat, lon, mapPoint.ix, mapPoint.iy)){
	cerr << mapFilename;
	cerr << " WARNING : point " << lat << ", " << lon << " (line ";
	cerr << lineNum  << ") outside of grid, disregarding." << endl;
      } else {
	inMap.intPoints.push_back( mapPoint );
      }
    }
    if (inMap.intPoints.size() > 2){
      _Maps.push_back( inMap );
    }
  }

  fclose(fp);

  //
  // OK - we've read the map file - now set up the mask bytes.
  // Allocate space and init to polyMaps::polyMapRelevant
  //
  _mapMask = (ui08 *) malloc(sizeof(ui08) * fhdr.nx * fhdr.ny );
  if (_mapMask == NULL){
    cerr << "Malloc failed, size X= " <<  fhdr.nx << " Y= " <<  fhdr.ny << endl;
    exit(-1);
  }

  memset(_mapMask, polyMaps::polyMapRelevant, fhdr.nx * fhdr.ny );

  //
  // Loop through the points, looping through the maps to see
  // if the point should be set to polyMaps::polyMapNotRelevant
  //
  for (int ix=0; ix < fhdr.nx; ix++){
    for (int iy=0; iy < fhdr.ny; iy++){
      vector< polyMaps::indexMap_t >::iterator it;
      for (it = _Maps.begin(); it != _Maps.end(); it++){
	const indexMap_t &map = *it;

	bool isInside = polyMaps::_isInside(ix, iy, map.intPoints);

	if (
	    ((isInside) && (!(map.retainPointsInside))) ||
	    ((!(isInside)) && (map.retainPointsInside))
	    ){
	  //
	  // Over the line! Mark it polyMaps::polyMapNotRelevant.
	  // Also, no need to consider other maps - continue to next point.
	  //
	  _mapMask[iy * fhdr.nx + ix] = polyMaps::polyMapNotRelevant;
	  continue;
	}
      }
    }
  }

  return;
}


//
// Return pointer to the work we've done.
//
ui08 *polyMaps::getMapMask(){
  return _mapMask;
}

//
// Print maps to stderr.
//
void polyMaps::printMaps(){

  int mapNum = 0;
  vector< polyMaps::indexMap_t >::iterator it;
  for (it = _Maps.begin(); it != _Maps.end(); it++){
    const indexMap_t &map = *it;
    mapNum++;

    cerr << "Map number " << mapNum << " :" << endl;
    if (map.retainPointsInside)
      cerr << " points must be inside this polygon to be retained." << endl;
    else
      cerr << " points must be outside this polygon to be retained." << endl;

   cerr << "   " << map.intPoints.size() << " points" << endl;

    int pointNum = 0;

    for ( vector < polyMaps::intPoint_t >::const_iterator ip_t = map.intPoints.begin();
	  ip_t !=  map.intPoints.end();
	  ip_t++){

      pointNum++;
      cerr << "    point " << pointNum << " : ";
      cerr << ip_t->ix << ", " << ip_t->iy << endl;
    }
    cerr << endl;
  }
  return;
}    

//
// Determine if a point is inside a polygon or not. Uses euclid library.
//
bool polyMaps::_isInside(int ix, int iy, vector <polyMaps::intPoint_t> intPoints){

  //
  // To work with the somewhat dated euclid library,
  // get the points out of the intPoints vector and into
  // an array of type Point_d
  //
  Point_d *PtArray = (Point_d *)malloc(sizeof(Point_d) * intPoints.size());

  if (PtArray == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  int index = 0;
  for ( vector < polyMaps::intPoint_t >::const_iterator ip_t = intPoints.begin();
	ip_t !=  intPoints.end();
	ip_t++){
    PtArray[index].x = ip_t->ix;    PtArray[index].y = ip_t->iy;
    index++;
  }

  Point_d Pt;
  Pt.x=ix; Pt.y=iy;
  //
  // See if the point is inside the polygon.
  //
  int Inside = EG_point_in_polygon2(Pt, PtArray, intPoints.size());

  free(PtArray);

  if (Inside)
    return true;

  return false;

}


polyMaps::~polyMaps(){
  //
  // Free the map mask.
  //
  if (NULL != _mapMask)
    free(_mapMask);

  //
  // Free the vectors of integer points.
  //
  vector< polyMaps::indexMap_t >::iterator it;
  for (it = _Maps.begin(); it != _Maps.end(); it++){
    it->intPoints.clear();
  }

  //
  // Free the vector of maps.
  //
  _Maps.clear();

  return;
}
  
