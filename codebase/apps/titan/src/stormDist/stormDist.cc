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
///////////////////////////////////////////////////////////////
// stormDist.cc
//
// stormDist object
//
// The One, RAP, NCAR
// P.O.Box 3000, Boulder, CO, 80307-3000, USA
//
//
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////

#include "stormDist.hh"

#include <vector>
#include <string>
#include <toolsa/file_io.h>
#include <toolsa/str.h>
#include <toolsa/Path.hh>
#include <toolsa/DateTime.hh>
#include <toolsa/pjg_flat.h>
#include <titan/DsTitan.hh>
#include <cerrno>
#include <titan/TitanComplexTrack.hh>
#include <titan/TitanSimpleTrack.hh>
#include <titan/TitanTrackEntry.hh>
#include <rapformats/GenPt.hh>
#include<Spdb/DsSpdb.hh>
#include <toolsa/pmu.h>
using namespace std;

// Constructor

stormDist::stormDist(Params *TDRP_params)
  
{
  //
  // Make copy of the input parameters.
  //
  _params = TDRP_params;
  return;

}

// destructor

stormDist::~stormDist()

{
  return;
}

//////////////////////////////////////////////////
// Run

int stormDist::Run(time_t start, time_t end )
{
  //
  if (_params->debug){
    cerr << "Running from " << utimstr(start);
    cerr << " to " << utimstr(end) << endl;
  }
  
  // set up the DsTitan server object and get a vector of times.
  
  _titan.clearArrays();  
  _titan.clearRead();
  
  time_t margin = end-start;
  time_t goTime = end;
  
  vector<time_t> scanTimes;

  while(true) {

    _titan.setReadFirstBefore(goTime, margin);
    _titan.setReadCurrentEntries();

    // do the read

    PMU_auto_register("Reading data....");
    if (_titan.read(_params->titanUrl)) {
      //
      // This means there are no more storms - hardly a fatal
      // error condition.
      //
      break;
    }

    goTime = _titan.getTimeInUse();

    //
    // Save this time.
    //
    if ((goTime >= start) && (goTime <= end))
      scanTimes.push_back(goTime);

    //
    // Get ready for next read.
    //
    goTime--;
    margin = goTime - start + 1;
    
    PMU_auto_register("Read data....");
    
  }

  if (_params->debug) {
    cerr << scanTimes.size() << " scans between " << utimstr(start) << " and " << utimstr(end) << endl;
  }


  vector<time_t>::reverse_iterator i;
  //
  // Go through the vector backwards (that's how we loaded it).
  //
  int iscan=0;
  for (i=scanTimes.rbegin(); i != scanTimes.rend(); i++){

    iscan++;
    if (_params->debug){
      cerr << endl << endl << "Data for scan " << iscan << " at time " << utimstr(*i) << endl;
    }
    
    _titan.clearArrays();  
    _titan.clearRead();
    _titan.setReadFirstBefore(*i, 0);
    _titan.setReadCurrentEntries();
    
    PMU_auto_register("Reading data....");
    if (_titan.read(_params->titanUrl)) {
      //
      // This is an error, since we checked that there was data here.
      //
      cerr << "ERROR - stormDist::Run()" << endl;
      cerr << _titan.getErrStr() << endl;
      cerr << "For time " << utimstr(*i) << endl;
      exit(-1);
    }

    //
    // Get the vector of track entries.
    //
    const vector<TitanTrackEntry *> &TrackEntries = _titan.current_entries();
    int numEntries = TrackEntries.size();
    //
    
    if (_params->debug) {
      cerr <<  numEntries << " track entries found." << endl;
    }

    //
    // Allocate enough space for the storm data.
    //
    stormData_t *stormData = (stormData_t *)malloc(numEntries*sizeof(stormData_t));
    if (stormData == NULL){
      cerr << "Malloc failed!" << endl; // Unlikely.
      exit(-1);
    }
    
    for (int te=0; te < numEntries; te++){
      PMU_auto_register("Processing track entries....");
      const TitanTrackEntry &TrackEntry = *TrackEntries[te];
      //
      //
      //
      const storm_file_scan_header_t &Scan = TrackEntry.scan();
      
      const track_file_entry_t &trackFileEntry = TrackEntry.entry();
      
      double latOrg = Scan.grid.proj_origin_lat;
      double lonOrg = Scan.grid.proj_origin_lon;
      double dx = Scan.grid.dx;
      double dy = Scan.grid.dy;

      const storm_file_global_props_t &Gp = TrackEntry.gprops();
      double X = Gp.proj_area_centroid_x;
      double Y = Gp.proj_area_centroid_y;
	  
      double Area = Gp.area_mean;
      long Duration = trackFileEntry.history_in_secs;

      int sNum = trackFileEntry.simple_track_num;
      int cNum = trackFileEntry.complex_track_num;

      double latCent, lonCent;

      if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
	latCent = Y; lonCent = X;
      } else {
	PJGLatLonPlusDxDy(latOrg, lonOrg, X, Y, &latCent, &lonCent);
      }
	
      if (_params->debug) {
	cerr << "    Track entry " << te;
	cerr << " Simple, complex numbers : " << sNum << ", " << cNum;
	cerr << " Lon : " << lonCent;
	cerr << " Lat : " << latCent;
	cerr << " Mean area : " << Area;
	cerr << " Duration : " << Duration;
	cerr << endl;
	
	if (_params->debug >= Params::DEBUG_VERBOSE){
	  for (int ir=0; ir < N_POLY_SIDES; ir++){
	    
	    double dir = double(ir)*(360.0/double(N_POLY_SIDES));
	    double latEnd, lonEnd;
	    
	    cerr << "      Radial " << ir+1;
	    cerr << " at direction " << dir << " deg ";
	    cerr << " : " << Gp.proj_area_polygon[ir];
	    if (Scan.grid.proj_type == TITAN_PROJ_LATLON){
	      cerr << " Deg ";
	      latEnd = latCent + Gp.proj_area_polygon[ir] * cos(3.1415927*dir/180.0) * dy;
	      lonEnd = lonCent + Gp.proj_area_polygon[ir] * sin(3.1415927*dir/180.0) * dx;
	    } else {
	      cerr << " Km ";
	      PJGLatLonPlusRTheta(latCent, lonCent, 
				  Gp.proj_area_polygon[ir], dir, &latEnd, &lonEnd);
	    }
	    cerr << "ends at " << latEnd << ", " << lonEnd << endl;
	  }
	    cerr << endl;
	}
      }

      //
      // Load up the storm data array.
      //
      stormData[te].simpleNum = sNum;
      stormData[te].complexNum = cNum;
      stormData[te].latCent = latCent;
      stormData[te].lonCent = lonCent;
      stormData[te].area = Area;
      stormData[te].duration = Duration;
      stormData[te].dx = dx;
      stormData[te].dy = dy;
      

      
      stormData[te].isFlat = true;
      if (Scan.grid.proj_type == TITAN_PROJ_LATLON) 
	stormData[te].isFlat = false;

      for (int ir=0; ir < N_POLY_SIDES; ir++){
	stormData[te].radials[ir] = Gp.proj_area_polygon[ir];
      }
    }

    for (int ie=0; ie < numEntries-1; ie++){
      for (int je=ie+1; je < numEntries; je++){
	_compare(stormData[ie], stormData[je], *i);
      }
    }
    free(stormData);
  }

  return 0;

}

//
// Compare two storms. Output goes to stdout.
//
void stormDist::_compare(stormData_t s1, stormData_t s2, time_t t){

  if ((s1.area < _params->areaThreshold) || (s2.area < _params->areaThreshold)){
    return;
  }

  if ((s1.duration < _params->durationThreshold) || (s2.duration < _params->durationThreshold)){
    return;
  }

  date_time_t T;
  T.unix_time = t;
  uconvert_from_utime( &T );

  //
  // Get the total distance and the direction from storm one to storm two,
  // centroid to centroid.
  //
  double totalDist;
  double dirS1_s2;

  PJGLatLon2RTheta(s1.latCent, s1.lonCent, s2.latCent, s2.lonCent, &totalDist, &dirS1_s2);

  //
  // See if the storms are too far apart to deal with.
  //
  if (!(_params->maxCentroidDist < 0.0)){
    if (totalDist > _params->maxCentroidDist){
      return;
    }
  }

  //
  // Get the direction from storm two to storm one.
  //
  double dirS2_s1 = dirS1_s2 - 180.0;

  //
  // Get the mean radius in that direction for storm one.
  //
  double meanRadKmS1 = _getMeanRad(s1.radials, s1.isFlat, dirS1_s2, s1.latCent, s1.lonCent, s1.dx, s1.dy);

  //
  // ditto storm two.
  //
  double meanRadKmS2 = _getMeanRad(s2.radials, s2.isFlat, dirS2_s1, s2.latCent, s2.lonCent, s2.dx, s2.dy);

  //
  // Get the edge-to-edge distance.
  //
  double distEdgeToEdge = totalDist - meanRadKmS1 - meanRadKmS2;

  if (_params->annotateOutput){

    cout << "Year=" << T.year << " Month=" << T.month << " Day=" << T.day << " Hour=";
    cout << T.hour << " Minute=" << T.min << " Second=" << T.sec << " Storm1ComplexNum=";
    
    cout << s1.complexNum << " Storm1SimpleNum=" << s1.simpleNum << " Storm2ComplexNum=";
    cout << s2.complexNum << " Storm2SimpleNum=" << s2.simpleNum << " Storm1Area=";
    
    cout << s1.area << " Storm2Area=" << s2.area << " Storm1Duration=";
    cout << s1.duration << " Storm2Duration=" << s2.duration << " Storm1LatCent=";
    
    cout << s1.latCent << " Storm1LonCent=" << s1.lonCent << " Storm2LatCent=";
    cout << s2.latCent << " Storm2LonCent=" << s2.lonCent << " distCent=";
    
    cout << totalDist << " distEdges=" << distEdgeToEdge;

    cout << endl;

  } else {

    cout << T.year << " " << T.month << " " << T.day << " ";
    cout << T.hour << " " << T.min << " " << T.sec << " ";
    
    cout << s1.complexNum << " " << s1.simpleNum << " ";
    cout << s2.complexNum << " " << s2.simpleNum << " ";
    
    cout << s1.area << " " << s2.area << " ";
    cout << s1.duration << " " << s2.duration << " ";
    
    cout << s1.latCent << " " << s1.lonCent << " ";
    cout << s2.latCent << " " << s2.lonCent << " ";
    
    cout << totalDist << " " << distEdgeToEdge;

    cout << endl;

  }

  return;
}


double stormDist::_getMeanRad(double *radials, bool isFlat, double dir, double latCent, double lonCent,
			      double dx, double dy){

  //
  // Make sure dir is 0..360
  //
  dir = _polarWrapDir(dir);

  //
  // Get the index of the closest radius for this direction.
  //
  const double dirInc = 360.0/N_POLY_SIDES;
  int centerIndex = (int)rint(dir/dirInc);

  if (_params->debug >= Params::DEBUG_VERBOSE){
    cerr << "     Using central radius index " << centerIndex << " for direction " << dir << endl;
  }

  
  //
  // Get the mean radius.
  //
  double total = 0.0;
  int num = 0;
  for (int ri=centerIndex - _params->radialSpread; ri <= centerIndex + _params->radialSpread; ri++){
    total +=radials[_polarWrapIndex(ri)]; num++;
    if (_params->debug >= Params::DEBUG_VERBOSE){
      cerr << "     Radius index " << ri << " is " << radials[_polarWrapIndex(ri)] << endl;
    }
  }
  double meanRad = total/double(num);
  

  if ((isFlat) && (_params->debug >= Params::DEBUG_VERBOSE)){
    cerr << "     Mean radius (flat earth) : " << meanRad << endl;
  }
  
  if (isFlat) return meanRad;

  if (_params->debug >= Params::DEBUG_VERBOSE){
    cerr << "     Mean radius (latlon space) : " << meanRad << " in direction " << dir << endl;
  }

  //
  // On a lat/lon grid, have to do some math to get
  // the lat/lon of the end point, and hence the distance in Km.
  //
  double delLat = meanRad * cos(3.1415927*dir/180.0) * dy;
  double delLon = meanRad * sin(3.1415927*dir/180.0) * dx;
  double latEnd = latCent + delLat;
  double lonEnd = lonCent + delLon;

  if (_params->debug >= Params::DEBUG_VERBOSE){
    cerr << "     Start  lat, lon : " << latCent << ", " << lonCent << endl;
    cerr << "     Delta  lat, lon : " << delLat << ", " << delLon << endl;
    cerr << "     Ending lat, lon : " << latEnd << ", " << lonEnd << endl;
  }
  
  double dist, theta;
  
  PJGLatLon2RTheta(latCent, lonCent, latEnd, lonEnd, &dist, &theta);

  if (_params->debug >= Params::DEBUG_VERBOSE){
    cerr << "     Mean radius (latlon) : " << dist << " in direction " << _polarWrapDir(theta) << endl << endl;
  }
  
  return dist;
  
}


int stormDist::_polarWrapIndex(int index){

  if ((index > -1) && (index < N_POLY_SIDES)) return index;

  if (index < 0) index+=N_POLY_SIDES;
  if (index > N_POLY_SIDES-1) index -=N_POLY_SIDES;

  //
  // Recurse.
  //
  return (_polarWrapIndex(index));

}

double stormDist::_polarWrapDir(double dir){

  if (!(dir < 0.0)){
    if (dir < 360.0){
      return dir;
    }
  }

  if (dir < 0.0) dir += 360.0;
  if (dir > 360.0) dir -= 360.0;

  return _polarWrapDir(dir);

}
