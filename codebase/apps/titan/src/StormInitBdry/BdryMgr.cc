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
using namespace std;
  
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
 
  _Input.getInterval(_params->bdryUrl, 
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


void BdryMgr::getBdryN(int bdryNum, BdryMgr::bdryStats_t *bdry){


  ui08 *dataPtr = _bdryProductBuffer + _bdryProductHdrs[bdryNum].offset;
  BDRY_spdb_product_t *prod;
 
  prod = ( BDRY_spdb_product_t *) dataPtr;
  BDRY_product_t *BP = BDRY_spdb_to_product( prod );

  time_t dataTime = BP->data_time;

  //  cerr << "Data time : " << utimstr(dataTime) << endl;
  //  cerr << BP->num_polylines << " polylines in this product." << endl;

  bdry->bdryTime = dataTime;
  bdry->groupID = BP->group_id;
  bdry->sequenceNum = BP->sequence_num;
 
  for (int il=0; il < BP->num_polylines; il++){

    BDRY_polyline_t Line = BP->polylines[il];
    //
    // Work out the number of seconds that the data were
    // extrapolated over. This is complicated by the fact that
    // although Line.num_secs is documented as being the number
    // of extrapolation seconds, it is often the actual time
    // that the extrapolation applies to, so that instead of
    //
    // Line.num_secs = extrapDelta; (1)
    //
    // We have :
    //
    // Line.num_secs = dataTime + extrapDelta; (2)
    //
    // So what I do is to assume that if Line.num_secs is less than
    // the number of seconds in 5 days, then case (1) above applies,
    // otherwise it's case (2).
    //
    int extrapSecs=0;
    const long secsPerDay = 86400L;
    if (Line.num_secs < 5*secsPerDay){
      extrapSecs=Line.num_secs;
    } else {
      extrapSecs=Line.num_secs - dataTime;
    }
    //
    // Get the average U,V and the minimum distance to the point.
    //
    double Umean=0.0; double Vmean=0.0;
    int closestPointIndex=0;
    double minDist=0;
    double minTheta = 0.0;
    int first = 1;
    
    for (int ip=0; ip < Line.num_pts; ip++){
      
      BDRY_point_t point = Line.points[ip];
      double dist, theta;
      
      if (first){
	first = 0;
	PJGLatLon2RTheta(point.lat, point.lon, _lat, _lon,
			 &minDist, &minTheta);
      } else {
	PJGLatLon2RTheta(point.lat, point.lon, _lat, _lon,
			 &dist, &theta);
	if (dist < minDist){
	  minDist = dist; minTheta = theta; closestPointIndex=ip;
	} 
	Umean = Umean + point.u_comp;
	Vmean = Vmean + point.v_comp;
      }
    }
    //
    Umean = Umean / Line.num_pts;
    Vmean = Vmean / Line.num_pts;

    double vel = sqrt(Umean*Umean + Vmean*Vmean);

    double normU = Umean/vel;
    double normV = Vmean/vel;

    const double pi = 3.1415927;

    double rX = sin(minTheta*pi/180.0);
    double rY = cos(minTheta*pi/180.0);

    double dot = rX*normU + rY*normV;
    double angle = 180.0*acos( dot )/pi;
    double modVel = sqrt(Umean*Umean + Vmean*Vmean);
    
    /*
    cerr << "Dist from point to bdry : " << minDist << " " << minTheta << endl;
    cerr << "Bdry velocity : " << Umean << " " << Vmean << " " << modVel << endl;
    cerr << "Init point : " << _lat << " " << _lon << endl;
    cerr << "Bdry point : " << Line.points[closestPointIndex].lat;
    cerr << " " << Line.points[closestPointIndex].lon << endl;
    cerr << "Closest point is " << closestPointIndex+1 << " of ";
    cerr << Line.num_pts << endl;
    cerr << "Dot product : " << dot << " " << angle << endl;
    cerr << "Extrapolation seconds : " << extrapSecs << endl << endl;    
    */
    //
    // Load up the struct for output.
    //
    bdry->Dist = minDist;
    bdry->Angle = angle;
    bdry->Vel = modVel;
    bdry->ExtrapSecs = extrapSecs;
    bdry->Umean = Umean;
    bdry->Vmean = Vmean;
    bdry->BdryLat = Line.points[closestPointIndex].lat;
    bdry->BdryLon = Line.points[closestPointIndex].lon;

  }
}
  
