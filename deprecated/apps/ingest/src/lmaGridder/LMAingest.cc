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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <toolsa/pmu.h>
#include <physics/physics.h>
#include <toolsa/file_io.h>

#include <rapformats/ltg.h>
#include <rapformats/GenPt.hh>
#include <toolsa/pjg_flat.h>
#include <Spdb/DsSpdb.hh>

#include "LMAingest.hh"

//
// Constructor. Copies parameters.
//
LMAingest::LMAingest(Params *TDRP_params){
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  //
  // Set up the lmaGrid object.
  //
  _lmaGrid = new lmaGridder(_params);

  return;

}
//
// Destructor. Does nothing.
//
LMAingest::~LMAingest(){
  delete _lmaGrid;

  return;

}
//
// Main routine.
//
void LMAingest::LMAingestFile( char *filename ){
  //
  // Say hello, and open the file.
  //
  time_t now = time(NULL);
  fprintf(stderr,"Processing file %s at %s\n", 
	  filename, utimstr(now));
  //
  // See if we can parse the date out of the filename.
  // Naming convention I'm seeing is :
  //
  // LYLOUT_050714_200000_3600.dat
  //
  bool gotDate = false;
  //
  int fileYear=0, fileMonth=0, fileDay=0;
  char *p = strstr(filename, "LYLOUT_");
  if (p != NULL){
    char *q = p + strlen("LYLOUT_");
    if (
	(3 == sscanf(q,"%2d%2d%2d", &fileYear, &fileMonth, &fileDay)) &&
	(fileMonth > 0) && (fileMonth < 13) &&
	(fileDay > 0) && (fileDay < 32)
	){
      fileYear += 2000;
      gotDate = true;
      if (_params->debug){
	cerr << "Date parsed from filename : " << fileYear;
	cerr << "/" << fileMonth << "/" << fileDay << endl;
      }
    }
  }

  FILE *fp = ta_fopen_uncompress(filename,"r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", filename);
    return;
  }
  
  const int lineLen = 1024;
  char Line[lineLen];
  //  
  long int numAccept = 0;
  long int numReject = 0;
  date_time_t T;
  //
  if (gotDate){
    T.year = fileYear;
    T.month = fileMonth;
    T.day = fileDay;
  }
  //
  while (NULL!=fgets(Line, lineLen, fp)){

    switch( _params->format ){

    case Params::FORMAT_ONE :

      //
      // See if we have the date line.
      //
      if (!gotDate){
	if (strlen(Line) > strlen("Data start time:")){
	  if (!(strncmp("Data start time:",
			Line,
			strlen("Data start time:")))){
	    
	    char *p=Line + strlen("Data start time:");
	    if (3 == sscanf(p,"%d/%d/%d",&T.month,&T.day,&T.year)){
	      T.year = T.year + 2000;
	      gotDate = true;
	    }
	  }
	}
      }
      //
      // See if we have data, after we have the date.
      //
      if (gotDate){
	double nSecs, lat, lon, alt, chiSqrd;
	int numStations;
	//
	if (6 != sscanf(Line, "%lf %lf %lf %lf %lf %d",
			&nSecs, &lat, &lon, &alt, &chiSqrd, &numStations)){

	  if (_params->debug){
	    cerr << "Read failed for line : " << Line;
	  }

	} else {
	  /*------------------------
	  if (_params->debug){
	    cerr << "Lat : " << lat << endl;
	    cerr << "Lon : " << lon << endl;
	    cerr << "Alt : " << alt << endl;
	    cerr << numStations << endl;
	    cerr << endl;
	  }
          ------------------------*/
	  //
	  //
	  // Fill out the time with the nSecs variable - the number of
	  // seconds in the day, UTC.
	  //
	  T.hour = (int)floor(nSecs/3600.0);
	  nSecs = nSecs - 3600*T.hour;
	  
	  T.min = (int)floor(nSecs/60.0);
	  nSecs = nSecs - 60*T.min;
	  
	  T.sec = (int)rint(nSecs);
	  
	  uconvert_to_utime( &T );

	  //
	  // Do a set of QC checks, and if it passes all of them then
	  // then feed the data into the grid.
	  // Increment numReject - decrement it later if the point
	  // is accepted.
	  //
	  numReject++;
	  //
	  if (
	      (chiSqrd > _params->maxChiSqrd) ||
	      (numStations < _params->minNumStations)
	      ){
	    if (_params->debug){
	      cerr << "Failed QC test, chiSqrd= ";
	      cerr << chiSqrd << " max is " << _params->maxChiSqrd;
	      cerr << "    numstations= " << numStations;
	      cerr << " min required is " << _params->minNumStations << endl;
	    }

	    continue;
	  }
	  //
	  //
	  if (
	      (lat < _params->minLat) ||
	      (lat > _params->maxLat) ||
	      (lon < _params->minLon) ||
	      (lon > _params->maxLon) ||
	      (alt > _params->maxAlt) ||
	      (alt < _params->minAlt)
	      ){

	    if (_params->debug){
	      cerr << "Failed max/min lat/lon test." << endl;
	    }

	    continue;
	  }
	  //
	  // Apply closeness criteria, if desired.
	  //
	  if (_params->applyClosenessCriteria){
	    double r,theta;
	    PJGLatLon2RTheta(lat,
			     lon,
			     _params->centralPoint.lat,
			     _params->centralPoint.lon,
			     &r, &theta);
	    if (r >  _params->centralPoint.maxRangeKm){
	      if (_params->debug){
		cerr << "Failed range test." << endl;
	      }
	      continue;
	    }
	  }

	  //
	  // Passed QC checks - decrement numReject and increment numAccept.
	  //
	  numReject--;
	  numAccept++;

	  if (_params->debug){
	    cerr << "Adding to grid at time " << utimstr(T.unix_time) << endl;
	  }
	  
	  _lmaGrid->addToGrid(lat, lon, alt, T.unix_time);

	  break;

	case Params::FORMAT_TWO :

	  if (9 != sscanf(Line, "%2d/%2d/%2d %2d:%2d:%lf %lf %lf %lf",
			  &T.month, &T.day, &T.year,
			  &T.hour, &T.min, &nSecs,
			  &lat, &lon, &alt)){

	    if ( _params->debug ){
	      cerr << "WARNING - failed to decode line " << Line;
	    }

	  } else {

	    T.sec = (int)rint(nSecs);
	    T.year += 2000;

	    uconvert_to_utime ( &T );

	    numReject++;
	    //
	    //
	    //
	    if (
		(lat < _params->minLat) ||
		(lat > _params->maxLat) ||
		(lon < _params->minLon) ||
		(lon > _params->maxLon) ||
		(alt > _params->maxAlt) ||
		(alt < _params->minAlt)
		){
	      continue;
	    }
	    //
	    // Apply closeness criteria, if desired.
	    //
	    if (_params->applyClosenessCriteria){
	      double r,theta;
	      PJGLatLon2RTheta(lat,
			       lon,
			       _params->centralPoint.lat,
			       _params->centralPoint.lon,
			       &r, &theta);
	      if (r >  _params->centralPoint.maxRangeKm){
		continue;
	      }
	    }

	    //
	    // Passed QC checks - decrement numReject and increment numAccept.
	    //
	    numReject--;
	    numAccept++;
	    
	    _lmaGrid->addToGrid(lat, lon, alt, T.unix_time);

	  }

	  break;

	case Params::FORMAT_THREE :

	char one[6];
	char two[2];
	char three[6];
	char four[9];
	int active_stations;
	double nSecs, lat, lon, alt, chiSqrd, power;
	int numStations;
    
	if (5 == sscanf(Line, "%6s %2s %6s %9s %d",
			one, two, three, four, &active_stations))
	{
	  if (strcmp(three,"active") == 0 )
	  {
	    numStations = active_stations;
	  }
	}
	
	if (6 != sscanf(Line, "%lf %lf %lf %lf %lf %lf",
			&nSecs, &lat, &lon, &alt, &chiSqrd, &power)){

	  //	  if ( _params->debug ){
	  //	    cerr << "WARNING - failed to decode line " << Line;
	  //	  }

	} else {

	  T.hour = (int)floor(nSecs/3600.0);
	  nSecs = nSecs - 3600*T.hour;
	
	  T.min = (int)floor(nSecs/60.0);
	  nSecs = nSecs - 60*T.min;
	
	  T.sec = (int)rint(floor(nSecs));
	  
	  uconvert_to_utime( &T );
	  
	  //	  double fracSecs = nSecs - floor(nSecs);
	  //	  double doubleTime = T.unix_time + fracSecs;

	  numReject++;

	  if (
	      (chiSqrd > _params->maxChiSqrd) ||
	      (numStations < _params->minNumStations) ||
	      (power < _params->minPower)
	      ){
	    if (_params->debug){
	      cerr << endl;
	      cerr << "Failed QC test: \n";
	      cerr << " chiSqrd= " << chiSqrd << " max is " << _params->maxChiSqrd << endl;
	      cerr << " numstations= " << numStations << " min required is " << _params->minNumStations << endl;
	      cerr << " power= " << power << " min required is " << _params->minPower << endl;
	      cerr << endl;
	    }

	    continue;
	  }

	  if (
	      (lat < _params->minLat) ||
	      (lat > _params->maxLat) ||
	      (lon < _params->minLon) ||
	      (lon > _params->maxLon) ||
	      (alt > _params->maxAlt) ||
	      (alt < _params->minAlt)
	      ){
	    if (_params->debug){
	      cerr << endl;
	      cerr << "Failed lat/lon/alt limits:\n";
	      cerr << "lat = " << lat;
	      cerr << ", minLat = " << _params->minLat;
	      cerr << ", maxLat = " << _params->maxLat << endl;
	      cerr << "lon = " << lon;
	      cerr << ", minLon = " << _params->minLon;
	      cerr << ", maxLon = " << _params->maxLon << endl;
	      cerr << "alt = " << alt;
	      cerr << ", minAlt = " << _params->minAlt;
	      cerr << ", maxAlt = " << _params->maxAlt << endl;
	      cerr << endl;
	    }
	    
	    continue;
	  }
	  //
	  // Apply closeness criteria, if desired.
	  //
	  
	  if (_params->applyClosenessCriteria){
	    double r,theta;
	    PJGLatLon2RTheta(lat,
			     lon,
			     _params->centralPoint.lat,
			     _params->centralPoint.lon,
			     &r, &theta);
	    cerr << "HERE\n";
	    if (r >  _params->centralPoint.maxRangeKm){
	      continue;
	    }
	  }

	  //
	  // Passed QC checks - decrement numReject and increment numAccept.
	  //
	  numReject--;
	  numAccept++;

	  _lmaGrid->addToGrid(lat, lon, alt, T.unix_time);
	  
	}

	break;

	default :

	cerr << "Unsupported input format" << endl;
	exit(-1);
	break;

	}
	
      }
      
    }
    
  }
  
  //
  // Close the input file.
  //
  fclose( fp );
  //

  if (_params->debug){
    fprintf(stderr,"%ld entries accepted, %ld rejected in %s.\n",
	    numAccept, numReject, filename);
  }

  //
  // In REALTIME mode, we send a dummy point, just so that empty
  // grids get output regularly - if we have not sent any other points.
  //
  if ((_params->mode == Params::REALTIME) && (numAccept == 0)){

    if (_params->takeZeroAsBadValue){
      _lmaGrid->addToGrid(0.0, 0.0, 0.0, time(NULL));
    }
    else
    {
      _lmaGrid->addToGrid(-1.0, -1.0, -1.0, time(NULL));
    }
    
  } else if (numAccept == 0) { // ARCHIVE or TIME_INTERVAL
    
    if (_params->takeZeroAsBadValue){
      _lmaGrid->addToGrid(0.0, 0.0, 0.0, T.unix_time);
    }
    else
    {
      _lmaGrid->addToGrid(-1.0, -1.0, -1.0, T.unix_time);
    }
  }
  
  return;

}
