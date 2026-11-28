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
#include <toolsa/pmu.h>
#include <toolsa/umisc.h>
#include <physics/physics.h>

#include <rapformats/ltg.h>
#include <rapformats/GenPt.hh>
#include <toolsa/pjg_flat.h>
#include <toolsa/file_io.h>


#include "LMAingest.hh"

//
// Constructor. Copies parameters.
//
LMAingest::LMAingest(Params *TDRP_params){
  //
  // Init the _lastTime variable.
  //
  _lastTime = 0;
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
  //
  // Set up the setOfPointsMgr object.
  //
  _setMgr = new setOfPointsMgr(_params);
}
//
// Destructor. Does nothing.
//
LMAingest::~LMAingest(){
  delete _setMgr;
}
//
// Main routine.
//
void LMAingest::LMAingestFile( char *filename ){
  //
  // Say hello, and open the file.
  //
  if (_params->debug){
    time_t now = time(NULL);
    fprintf(stderr,"Processing file %s at %s\n", 
	    filename, utimstr(now));
  }
  
  //
  // See if we can parse the date out of the filename.
  // Naming convention I'm seeing is :
  //
  // LYLOUT_050714_200000_3600.dat
  // LYLOUT_COLMA_150922_200100_0060.dat
  //
  bool gotDate = false;
  //
  date_time_t fileNameTime;
  char *p = strstr(filename, "LYLOUT_COLMA_");
  int strLength;  
  if (p == NULL) 
  {
    p = strstr(filename, "LYLOUT_");
    strLength =  strlen("LYLOUT_");
  } 
  else
     strLength = strlen("LYLOUT_COLMA_");
  if (p != NULL){
    char *q = p + strLength;
    if (
        (6 == sscanf(q,"%2d%2d%2d_%2d%2d%2d", 
		     &fileNameTime.year, &fileNameTime.month, &fileNameTime.day,
		     &fileNameTime.hour, &fileNameTime.min, &fileNameTime.sec)) &&
        (fileNameTime.month > 0) && (fileNameTime.month < 13) &&
        (fileNameTime.day > 0) && (fileNameTime.day < 32) &&
        (fileNameTime.hour > -1) && (fileNameTime.hour < 24) &&
        (fileNameTime.min > -1) && (fileNameTime.min < 60) &&
        (fileNameTime.sec > -1) && (fileNameTime.sec < 60)
        ){
      fileNameTime.year += 2000;
      uconvert_to_utime( &fileNameTime );
      gotDate = true;
      if (_params->debug){
	fprintf(stderr,"Date parsed from filename is %s\n",
		utimstr(fileNameTime.unix_time));
      }
    }
  }

  if (!(gotDate)){
    fprintf(stderr,"Failed to parse time from %s\n", filename);
    return;
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
  T.year = fileNameTime.year;
  T.month = fileNameTime.month;
  T.day = fileNameTime.day;

  //
  //
  while (NULL!=fgets(Line, lineLen, fp)){


    double nSecs, lat, lon, alt, chiSqrd, power;
    int numStations;

    switch( _params->format ){

    case Params::FORMAT_ONE :

      //
      if (6 == sscanf(Line, "%lf %lf %lf %lf %lf %d",
		      &nSecs, &lat, &lon, &alt, &chiSqrd, &numStations)){
	//
	// Do a set of QC checks, and if it passes all of them then
	// then feed the data into the grid.
	// Increment numReject - decrement it later if the point
	// is accepted.
	//
	numReject++;
	//
	bool reject=false;
	if (
	    (chiSqrd > _params->maxChiSqrd) ||
	    (numStations < _params->minNumStations)
	    ){
	  reject=true;
	}
	
	if ((lat < _params->minLat) || (lat > _params->maxLat) ||
	    (lon < _params->minLon) || (lon > _params->maxLon) ||
	    (alt < _params->minAlt) || (alt > _params->maxAlt)){
	  reject=true;
	}
	
	if (reject){
	  if (_params->debugRejects){
	    cerr << "Point rejected :" << endl;
	    
	    if (chiSqrd > _params->maxChiSqrd)
	      cerr << " chiSqrd of " << chiSqrd << " greater than " << _params->maxChiSqrd << endl;
	    
	    if (numStations < _params->minNumStations)
	      cerr << " Numstations is " << numStations << ", need " << _params->minNumStations << endl;
	    
	    if ((lat < _params->minLat) || (lat > _params->maxLat))
	      cerr << " Latitude " << lat << " outside " <<  _params->minLat  << " to " <<  _params->maxLat << " range" << endl;

	    if ((lon < _params->minLon) || (lon > _params->maxLon))
	      cerr << " Longitude " << lon << " outside " <<  _params->minLon  << " to " <<  _params->maxLon << " range" << endl;

	    if ((alt < _params->minAlt) || (lat > _params->maxAlt))
	      cerr << " Altitude " << alt << " outside " <<  _params->minAlt  << " to " <<  _params->maxAlt << " range" << endl;
	    
	    cerr << endl;
	  }
	  continue;
	}

	//
	// Passed QC checks - decrement numReject and increment numAccept.
	//
	numReject--;
	numAccept++;
	//
	//
	// Fill out the time with the nSecs variable - the number of
	// seconds in the day, UTC.
	//
	
	
	T.hour = (int)floor(nSecs/3600.0);
	nSecs = nSecs - 3600*T.hour;
	
	T.min = (int)floor(nSecs/60.0);
	nSecs = nSecs - 60*T.min;
	
	T.sec = (int)rint(floor(nSecs));
	  
	uconvert_to_utime( &T );
	  
	double fracSecs = nSecs - floor(nSecs);
	double doubleTime = T.unix_time + fracSecs;
	  
	_setMgr->addPoint(lat, lon, alt, doubleTime);
	//
	// Keep track of the last time for the flush method.
	//
	_lastTime = doubleTime;
      }

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
	  continue;
	}
	//

	//
	// Passed QC checks - decrement numReject and increment numAccept.
	//
	numReject--;
	numAccept++;
	//
	//
	// Fill out the time with the nSecs variable - the number of
	// seconds in the day, UTC.
	//
	
	
	T.hour = (int)floor(nSecs/3600.0);
	nSecs = nSecs - 3600*T.hour;
	
	T.min = (int)floor(nSecs/60.0);
	nSecs = nSecs - 60*T.min;
	
	T.sec = (int)rint(floor(nSecs));
	
	uconvert_to_utime( &T );
	
	double fracSecs = nSecs - floor(nSecs);
	double doubleTime = T.unix_time + fracSecs;
	
	_setMgr->addPoint(lat, lon, alt, doubleTime);
	//
	// Keep track of the last time for the flush method.
	//
	_lastTime = doubleTime;
      }      
      break;

    case Params::FORMAT_THREE :

      //
      char one[6];
      char two[2];
      char three[6];
      char four[9];
      int active_stations;
    
      if (5 == sscanf(Line, "%6s %2s %6s %9s %d",
		      one, two, three, four, &active_stations))
      {
	if (strcmp(three,"active") == 0 )
	{
	  numStations = active_stations;
	}
      }

      if (6 == sscanf(Line, "%lf %lf %lf %lf %lf %lf",
		      &nSecs, &lat, &lon, &alt, &chiSqrd, &power)){
	//
	// Do a set of QC checks, and if it passes all of them then
	// then feed the data into the grid.
	// Increment numReject - decrement it later if the point
	// is accepted.
	//
	numReject++;
	//
	bool reject=false;
	if (
	    (chiSqrd > _params->maxChiSqrd) ||
	    (numStations < _params->minNumStations) ||
	    (power < _params->minPower)
	    ){
	  reject=true;
	}
	
	if ((lat < _params->minLat) || (lat > _params->maxLat) ||
	    (lon < _params->minLon) || (lon > _params->maxLon) ||
	    (alt < _params->minAlt) || (alt > _params->maxAlt)){
	  reject=true;
	}
	
	if (reject){
	  if (_params->debugRejects){
	    cerr << "Point rejected :" << endl;
	    
	    if (chiSqrd > _params->maxChiSqrd)
	      cerr << " chiSqrd of " << chiSqrd << " greater than " << _params->maxChiSqrd << endl;
	    
	    if (numStations < _params->minNumStations)
	      cerr << " Numstations is " << numStations << ", need " << _params->minNumStations << endl;

	    if (power < _params->minPower)
	      cerr << " Power is " << power << ", need " << _params->minPower << endl;
	    
	    if ((lat < _params->minLat) || (lat > _params->maxLat))
	      cerr << " Latitude " << lat << " outside " <<  _params->minLat  << " to " <<  _params->maxLat << " range" << endl;

	    if ((lon < _params->minLon) || (lon > _params->maxLon))
	      cerr << " Longitude " << lon << " outside " <<  _params->minLon  << " to " <<  _params->maxLon << " range" << endl;

	    if ((alt < _params->minAlt) || (lat > _params->maxAlt))
	      cerr << " Altitude " << alt << " outside " <<  _params->minAlt  << " to " <<  _params->maxAlt << " range" << endl;
	    
	    cerr << endl;
	  }
	  continue;
	}

	//
	// Passed QC checks - decrement numReject and increment numAccept.
	//
	numReject--;
	numAccept++;
	//
	//
	// Fill out the time with the nSecs variable - the number of
	// seconds in the day, UTC.
	//
	
	
	T.hour = (int)floor(nSecs/3600.0);
	nSecs = nSecs - 3600*T.hour;
	
	T.min = (int)floor(nSecs/60.0);
	nSecs = nSecs - 60*T.min;
	
	T.sec = (int)rint(floor(nSecs));
	  
	uconvert_to_utime( &T );
	  
	double fracSecs = nSecs - floor(nSecs);
	double doubleTime = T.unix_time + fracSecs;
	  
	_setMgr->addPoint(lat, lon, alt, doubleTime);
	//
	// Keep track of the last time for the flush method.
	//
	_lastTime = doubleTime;
      }

      break;
    }

  }
  //
  // Close the input file.
  //
  fclose( fp );

  //
  // Flush if we need to
  //
  if (fileNameTime.unix_time - _lastTime >= _params->maxAllowedTime){
    flush();
    _lastTime = fileNameTime.unix_time;
    //    flush();
  }

  if (_params->debug){
    fprintf(stderr,"%ld entries accepted, %ld rejected.\n",
	    numAccept, numReject);
  }
  
  return;

}

/////////////////////////////////////////////////////
//
// Flush out buffers at end of archive mode.
//
void LMAingest::flush(){
  //
  // Send a dummy point late enough to flush the buffers,
  // if we have set any points at all.
  //
  if (_lastTime == 0) return; // No points.
  //
  // Send a point late enough to cause a buffer flush.
  //
  _setMgr->addPoint(-90.0, 180.0, 0.0, 
		    _lastTime + _params->maxAllowedTime + 0.5);
  //
  return;
}

