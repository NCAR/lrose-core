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
// BdryMgr.cc
//
// BdryMgr object
//
//  RAP, NCAR P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
// November 2002
//
///////////////////////////////////////////////////////////////
//
// StormInitBdry produces SPDB output from TITAN binary files.
//
///////////////////////////////////////////////////////////////


#include "BdryMgr.hh"
#include <toolsa/pjg_flat.h>
#include <toolsa/umisc.h>
#include <math.h>
  
// constructor. Copies the TDRP params,
// the lat and the lon of the init point,
// and returns the number of boudaries found
// between start and end.
//
BdryMgr::BdryMgr (Params *TDRP_params,
		  time_t start, 
		  time_t end,
		  double lat,
		  double lon,
		  int *numBdryFound){

  //
  // Make a few copies.
  //
  _lat=lat; _lon = lon; _params = TDRP_params;

  //
  // Read the boundaries.
  //
 
  _Input.getInterval(_params->boundarySpdbUrl, 
		     start,
		     end);
  *numBdryFound = _Input.getNChunks();

  //
  // Do the byte swapping, etc.
  //
  _bdryProductHdrs = _Input.getChunkRefs();
  _bdryProductBuffer = (ui08 *) _Input.getChunkData();

  for (int ib=0; ib < *numBdryFound; ib++){
    
    ui08 *dataPtr = _bdryProductBuffer + _bdryProductHdrs[ib].offset;
    BDRY_spdb_product_t *prod;
 
    prod = ( BDRY_spdb_product_t *) dataPtr;
 
    BDRY_spdb_product_from_BE( prod );
    
    
  }
}
  
// destructor
//  
BdryMgr::~BdryMgr(){

}

//
// Load boundary number N, and compute the distance from the
// init point to the boundary. Store the distance in the global
// variable 'Dist'.
//
void BdryMgr::getBdryN(int bdryNum){

  //
  // Init to a large value - just to avoid compiler warnings.
  // This will be overwritten if the boundary has more than 0 points in it.
  //
  Dist = 1000000.0;

  double minDist=Dist;

  ui08 *dataPtr = _bdryProductBuffer + _bdryProductHdrs[bdryNum].offset;
  BDRY_spdb_product_t *prod;
 
  prod = ( BDRY_spdb_product_t *) dataPtr;
  BDRY_product_t *BP = BDRY_spdb_to_product( prod );

  if (_params->debug_bdry){
    cerr << "Number of polylines : " <<  BP->num_polylines << endl;
  }
  //
  // Return if no polylines.
  //
  if (BP->num_polylines == 0) return;
  //
  // Loop through the boundaries.
  // 
  for (int il=0; il < BP->num_polylines; il++){

    BDRY_polyline_t Line = BP->polylines[il];

    //
    // Get the minimum distance to the point.
    //
    double dummyTheta;
    int first = 1;
    
    if (_params->debug_bdry){
      cerr << "Number of points in polyline : " << il+1 << " ";
      cerr << Line.num_pts << endl;
    }

    for (int ip=0; ip < Line.num_pts; ip++){
      
      BDRY_point_t point = Line.points[ip];
      double dist;
      //
      // If it's the first point, make a note of the distance to the storm init point.
      //
      if (first){
	first = 0;
	PJGLatLon2RTheta(point.lat, point.lon, _lat, _lon,
			 &dist, &dummyTheta);
      }

      if (dist < minDist){
	minDist = dist;
      }

      //
      // In any case, see if we have to crawl along the
      // line rather than just zipping to the next point. If the
      // distance to the next point is less that the required proximity
      // distance divided by ten, then we are OK to just zip to the
      // next point - otherwise we should subsection the line. The
      // human entered boundaries require that we check this since they may
      // well have points entered a long way apart. Obviously do not
      // make this check on the last point.
      //
      if (ip < Line.num_pts -1){
	double distToNextPoint;
	double dirToNextPoint;

	double nextLat = Line.points[ip+1].lat;
	double nextLon = Line.points[ip+1].lon;
	//
	PJGLatLon2RTheta(point.lat, point.lon, nextLat, nextLon,
			 &distToNextPoint, &dirToNextPoint);
	//
	if (distToNextPoint > _params->boundaryProximity/10.0){
	  //
	  // OK - it looks like we have to crawl along the line
	  // to the next point. Work out how many steps we should take.
	  //
	  int stepsToTake = (int)ceil(10.0*distToNextPoint / _params->boundaryProximity);
	  double stepDist = distToNextPoint / double( stepsToTake );

	  if (_params->debug_bdry){
	    cerr << "Stepping for boundary " << bdryNum;
	    cerr << " from [" << point.lat << ", " << point.lon << "] ";
	    cerr << " to [" << nextLat << ", " << nextLon << "]" << endl;
	    cerr << stepsToTake << " steps of " << stepDist << " in direction " << dirToNextPoint << endl;
	  }

	  for (int is=0; is < stepsToTake; is++){
	    //
	    // Get the lat,lon of the intermediate point.
	    //
	    double midLat, midLon;
	    PJGLatLonPlusRTheta(point.lat, point.lon, stepDist*(double(is) + 0.5), dirToNextPoint,
                                &midLat, &midLon);
	    //
	    // Consider the distance from this point to the storm init point.
	    //
	    PJGLatLon2RTheta(midLat, midLon, _lat, _lon, &dist, &dummyTheta);
	    //

	    if (dist < minDist){
	      minDist = dist;
	    }

	    if (_params->debug_bdry){
	      cerr << is+1 << " step distance is " << dist;
	      cerr << " at point ";
	      cerr << "[" << midLat << ", " << midLon << "]";
	      cerr << " minDist is " << minDist << endl;
	    }



	  } // End of loop through points.
	} // End of if we have to subsection the line segment.
      } // End of if we are on the last point or not.
      //
      // Test the point itself. Do this regardless of if we had to subsection the line
      // or not, since the subsection goes right up to, but does not quite
      // arrive at, the next point.
      //
      PJGLatLon2RTheta(point.lat, point.lon, _lat, _lon,
		       &dist, &dummyTheta);

      if (dist < minDist){
	minDist = dist;
      }
    }
  }
  //
  // Set the global variable that the caller can retrieve.
  //
  Dist = minDist;

  if (_params->debug_bdry){
    cerr << "Minimum " << bdryNum+1 << " boundary distance is " << Dist << endl;
  }

}
  
