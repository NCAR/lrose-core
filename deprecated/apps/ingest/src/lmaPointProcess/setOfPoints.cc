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

#include "setOfPoints.hh"
#include <toolsa/umisc.h>
#include<toolsa/pjg_flat.h>
#include <rapformats/GenPt.hh>
#include <Spdb/DsSpdb.hh>
#include <cstdio>
#include <rapformats/ltg.h>

//
// Constructor.
//
setOfPoints::setOfPoints(Params *TDRP_Params,
			 double lat,
			 double lon,
			 double alt,
			 double t){

  _params = TDRP_Params;
  _lastLat = lat;   _lastLon = lon;   _lastAlt = alt;   _lastT = t;
  _firstLat = lat;  _firstLon = lon;  _firstAlt = alt;  _firstT = t;
  _numPoints = 1;
  _maxAlt = alt;  _minAlt = alt;  _altVector.push_back(alt);

}
//
// Method to add a point. Returns 0 if the point
// was added, 1 if the point was not but it is possible that
// another one could be (by looking at the time) and -1 if
// the point was not added and in fact there is no way that
// another point can be added at this time.
//
int setOfPoints::addPoint(double lat,
			  double lon,
			  double alt,
			  double t){

  //
  // If we are out of temporal range, return -1.
  //
  if (t - _lastT > _params->maxAllowedTime){
    return -1;
  }
  //
  // If we are within spatial range, add the point and return 0.
  // Requires that the distances be calculated.
  //
  double r,theta, dist;
  PJGLatLon2RTheta(lat,
		   lon,
		   _lastLat,
		   _lastLon,
		   &r, &theta);
  if (_params->considerVerticalDistance){

    double v_dist = fabs(_lastAlt - alt)/1000.0;
    dist = sqrt(r*r + v_dist*v_dist);
  } else {
    dist = r;
  }
  //
  if (dist > _params->maxAllowedDist){
    //
    // We will not add this point but it is possible that we will
    // add one still.
    //
    return 1;
  }

  //
  // Do similar checks against the first entry, is requested.
  //

  if (_params->totalMaxAllowedTime.applyLimit){
    //
    // If we are out of temporal range, return -1.
    //
    if (t - _firstT > _params->totalMaxAllowedTime.limit){
      return -1;
    }
  }

  if (_params->totalMaxAllowedDist.applyLimit){
    //
    // If we are within spatial range, add the point and return 0.
    // Requires that the distances be calculated.
    //
    double r,theta, dist;
    PJGLatLon2RTheta(lat,
		     lon,
		     _firstLat,
		     _firstLon,
		     &r, &theta);
    if (_params->considerVerticalDistance){

      double v_dist = fabs(_firstAlt - alt)/1000.0;
      dist = sqrt(r*r + v_dist*v_dist);
    } else {
      dist = r;
    }
    //
    if (dist > _params->totalMaxAllowedDist.limit){
      //
      // We will not add this point but it is possible that we will
      // add one still.
      //
      return 1;
    }
  }

  //
  // Add the point.
  //
  _lastLat = lat;   _lastLon = lon;   _lastAlt = alt;   _lastT = t;
  if (alt > _maxAlt) _maxAlt = alt;
  if (alt < _minAlt) _minAlt = alt;
  _altVector.push_back(alt);
  _numPoints++;
  //
  return 0;
  //
}
//
// Save out the data for the set of points to an SPDB database.
// Returns time of save, or 0 if no data were saved.
//
time_t setOfPoints::addChunk(DsSpdb *Out, DsSpdb *OutLtg){
  //
  //
  //
  if (_numPoints < _params->minNumEntries) return 0L; // fugghedabaddit
  //
  // Calculate the first to last distance.
  //
  double r,theta, dist, dist2D, dist3D;
  PJGLatLon2RTheta(_firstLat,
		   _firstLon,
		   _lastLat,
		   _lastLon,
		   &r, &theta);
  dist2D = r;
  double v_dist = (_maxAlt - _minAlt)/1000.0;
  dist3D = sqrt(r*r + v_dist*v_dist);

  if (_params->considerVerticalDistance){
    dist = dist3D;
  } else {
    dist = dist2D;
  }
  //
  // Calculate the mean altitude.
  //
  double total=0.0;
  for (unsigned i=0; i < _altVector.size(); i++){
    total += _altVector[i];
  }
  //
  double meanAlt = total / double(_altVector.size());
  //
  //
  // Second loop to get SD
  //
  double sumsqr=0.0;
  for (unsigned i=0; i < _altVector.size(); i++){
    sumsqr += (_altVector[i] - meanAlt) * (_altVector[i] - meanAlt);
  }
  //
  double var = sumsqr / double(_altVector.size());
  double sd = sqrt( var );
  //
  // Do some debugging.
  //
  if (_params->debug){
    cerr << "NUM=" << _numPoints << endl;
    cerr << "FIRST POINT :" << endl;
    cerr << " LAT=" << _firstLat;
    cerr << " LON=" << _firstLon; 
    cerr << " ALT=" << _firstAlt;  
    cerr << " TIM=" << utimstr((time_t)rint(_firstT)) << endl;
    cerr << "LAST POINT :" << endl;
    cerr << " LAT=" << _lastLat;
    cerr << " LON=" << _lastLon; 
    cerr << " ALT=" << _lastAlt;  
    cerr << " TIM=" << utimstr((time_t)rint(_lastT)) << endl;
    
    cerr << " DURATION=" << _lastT - _firstT;
    //
    cerr << endl << "ALTS : " << endl;
    int pcount = 0;
    for (unsigned i=0; i < _altVector.size(); i++){
      cerr << _altVector[i];
      pcount++;
      if (pcount < 10){
	cerr << "\t";
      } else {
	cerr << endl;
	pcount = 0;
      }
    }
    
    cerr << endl << "ALT : MEAN=" << meanAlt << " SD=" << sd;
    cerr << " MAXALT=" << _maxAlt;
    cerr << " MINALT=" << _minAlt << endl;
    //
    cerr << " DIST=" << dist << endl;
    cerr << endl << endl << endl;
    //
  }
  //
  // Save this out as a GenPoint.
  //
  GenPt G;
  G.setName("Ltg point with altitude");
  G.setId( 0 );
  time_t saveTime;
  int dataType;
  if (_params->saveBasedOnFirstPoint){
    saveTime = (time_t)rint( _firstT );
    dataType = (int)rint((_firstT - floor(_firstT))*100000.0);
    G.setTime( saveTime );
    G.setLat( _firstLat );
    G.setLon( _firstLon );
  } else {
    dataType = (int)rint((_lastT - floor(_lastT))*100000.0);
    saveTime = (time_t)rint( _lastT );
    G.setTime( saveTime );
    G.setLat( _lastLat );
    G.setLon( _lastLon );
  }
  G.setNLevels( 1 );
  //
  G.addVal( double(_numPoints) );
  G.addFieldInfo( "numLmaEntries", "count");
  //
  G.addVal( _firstLat );
  G.addFieldInfo( "firstLat", "deg");
  G.addVal( _firstLon );
  G.addFieldInfo( "firstLon", "deg");
  G.addVal( _firstT );
  G.addFieldInfo( "firstT", "sec");
  G.addVal( _firstAlt );
  G.addFieldInfo( "firstAlt", "m");
  //
  G.addVal( _lastLat );
  G.addFieldInfo( "lastLat", "deg");
  G.addVal( _lastLon );
  G.addFieldInfo( "lastLon", "deg");
  G.addVal( _lastT );
  G.addFieldInfo( "lastT", "sec");
  G.addVal( _lastAlt );
  G.addFieldInfo( "lastAlt", "m");
  //
  G.addVal( _minAlt );
  G.addFieldInfo( "minAltitude", "m");
  G.addVal( _maxAlt );
  G.addFieldInfo( "maxAltitude", "m");
  G.addVal( meanAlt );
  G.addFieldInfo( "meanAltitude", "m");
  G.addVal( sd );
  G.addFieldInfo( "sdAltitude", "m");
  //
  G.addVal( _lastT - _firstT );
  G.addFieldInfo( "duration", "sec");
  //  
  G.addVal( dist2D );
  G.addFieldInfo( "horDist", "Km");
  //
  G.addVal( dist );
  G.addFieldInfo( "flashLenght", "Km");
  //
  if (!(G.check())){
    fprintf(stderr, "GenPt check failed.\n");
    exit(-1);
  }
  
  if ( G.assemble() != 0 ) {
    fprintf(stderr, "Failed to assemble GenPt.\n");
    exit(-1);
  }
  //
  // Write the point to the database
  //
  Out->addPutChunk( 0, G.getTime(),
		    G.getTime() + _params->Expiry,
		    G.getBufLen(), G.getBufPtr() );
    
  //
  // If requested, save the data out in the older ltg struct.
  //
  if (_params->saveTraditional){
    LTG_extended_t L;
    memset(&L, 0, sizeof(L));
    if (_params->saveBasedOnFirstPoint){
      L.latitude = _firstLat;
      L.longitude = _firstLon;
      L.altitude = _firstAlt;
    } else {
      L.latitude = _lastLat;
      L.longitude = _lastLon;
      L.altitude = _lastAlt;
    }
    L.cookie = LTG_EXTENDED_COOKIE;
    L.time = saveTime;

    L.amplitude = LTG_MISSING_FLOAT;
    L.type = LTG_TYPE_UNKNOWN;

    L.nanosecs = 0;
    L.n_sensors = LTG_MISSING_INT;
    L.degrees_freedom = LTG_MISSING_INT;
    L.ellipse_angle = LTG_MISSING_FLOAT;
    L.semi_major_axis = LTG_MISSING_FLOAT;
    L.semi_minor_axis = LTG_MISSING_FLOAT;
    L.chi_sq = LTG_MISSING_FLOAT;
    L.rise_time = LTG_MISSING_FLOAT;
    L.peak_to_zero_time = LTG_MISSING_FLOAT;
    L.max_rate_of_rise = LTG_MISSING_FLOAT;
    L.angle_flag = LTG_MISSING_INT;
    L.signal_flag = LTG_MISSING_INT;
    L.timing_flag = LTG_MISSING_INT;
    
    L.residual = LTG_MISSING_FLOAT;
    for (int i = 0; i < 3; ++i)
      L.spare[i] = LTG_MISSING_FLOAT;
    
    LTG_extended_to_BE( &L );

    OutLtg->addPutChunk( 0, saveTime,
			 saveTime + _params->Expiry,
			 sizeof(LTG_extended_t), &L);
  }
  
  //
  // Save ASCII ltg files for research, if requested.
  //
  if (_params->saveAltFiles){
    char altFileName[MAX_PATH_LEN];
    sprintf(altFileName,"%05d_%d_%d_%ld.alt",
	    _altVector.size(), (int)rint(_firstLat*100.0),
	    (int)rint(_firstLon*100.0), saveTime);
    FILE *ofp = fopen(altFileName, "w");
    if (ofp == NULL) return saveTime;
    for (unsigned i=0; i < _altVector.size(); i++){
      fprintf(ofp,"%f\n", _altVector[i]);
    }
    fclose(ofp);
  }
  //
  return saveTime;
  //
}
//
// Return the number of points.
//
int setOfPoints::getSize(){
  return _numPoints;
}
//
// Return the duration.
//
double setOfPoints::getDuration(){
  return _lastT - _firstT;
}
//
// Destructor. Does nothing.
//
setOfPoints::~setOfPoints(){

}

