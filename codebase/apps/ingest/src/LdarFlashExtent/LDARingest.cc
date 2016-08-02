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


#include "LDARingest.hh"

//
// Constructor. Copies parameters.
//
LDARingest::LDARingest(Params *TDRP_params){
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
LDARingest::~LDARingest(){
  delete _setMgr;
}
//
// Main routine.
//
void LDARingest::LDARingestFile( char *filename ){
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
  // 20141090300.txt
  //
  bool gotDate = false;
  //
  date_time_t fileNameTime;
  char *p = strstr(filename, _params->fileExtension);

  int jday;
  if (p != NULL){
    char *q = p - 11;
    if (
        (4 == sscanf(q,"%4d%3d%2d%2d.txt",
                     &fileNameTime.year, &jday,
                     &fileNameTime.hour, &fileNameTime.min)) &&
        (fileNameTime.hour > -1) && (fileNameTime.hour < 24) &&
        (fileNameTime.min > -1) && (fileNameTime.min < 60))
    {
      fileNameTime.sec = 0;
      
      // calculate julian date up to last day of previous year
      long jdate = ujulian_date(31, 12, fileNameTime.year - 1);
      // add the day of the year to the julian date and find the date
      ucalendar_date(jdate + jday, &fileNameTime.day, &fileNameTime.month, &fileNameTime.year);
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
  //
  date_time_t strokeTime;
  strokeTime.year = fileNameTime.year;
  strokeTime.month = fileNameTime.month;
  strokeTime.day = fileNameTime.day;

  //
  //
  while (NULL!=fgets(Line, lineLen, fp)){

    double microSecs, SrcLat, SrcLon, X, Y, Z, lat , lon, alt;
    char *tstring;
    tstring = new char[10];
      
    char *ETYPE;
    ETYPE = new char[10];
    
    SrcLat = _params-> LDAR_Central_Site_Loc.latOrigin;
    SrcLon = _params-> LDAR_Central_Site_Loc.lonOrigin;
    
    switch( _params->format ){

    case Params::FORMAT_ONE :

      if ( sscanf(Line, "%3d%2d:%2d:%2d:%6lf%8lf%8lf%6lf%s%s",
		  &jday, &strokeTime.hour, &strokeTime.min, &strokeTime.sec, &microSecs, 
			  &X, &Y, &Z, ETYPE, tstring) == 10)
      {

	//
	// Convert x,y to lat lon using the SrcLat, SrcLon
	//
	PJGLatLonPlusDxDy(SrcLat, SrcLon, 
			  X * .001, Y * .001,
			  &lat, &lon);
	/*
	cerr << jday << ", ";
	cerr << strokeTime.hour << ", ";
	cerr << strokeTime.min << ", ";
	cerr << strokeTime.sec << ", ";
	cerr << microSecs << ", ";
	cerr << X << ", ";
	cerr << Y << ", ";
	cerr << Z << ", ";
	cerr << lat << ", ";
	cerr << lon << ", ";
	cerr << ETYPE << endl;
	*/

	//
	// alt in meters so no conversion needed
	//
	alt = Z;

	//
	// Do a set of QC checks, and if it passes all of them then
	// then feed the data into the grid.
	// Increment numReject - decrement it later if the point
	// is accepted.
	//
	numReject++;
	//
	bool reject=false;
	
	if ((lat < _params->minLat) || (lat > _params->maxLat) ||
	    (lon < _params->minLon) || (lon > _params->maxLon) ||
	    (alt < _params->minAlt) || (alt > _params->maxAlt)){
	  reject=true;
	}
	
	if (reject){
	  if (_params->debugRejects){
	    cerr << "Point rejected :" << endl;
	    
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
	
	uconvert_to_utime( &strokeTime );
	  
	double fracSecs = microSecs * .000001;
	double doubleTime = strokeTime.unix_time + fracSecs;
	  
	if( strcmp(ETYPE,"4DLSS") == 0)
	{
	  _setMgr->addPoint(lat, lon, alt, doubleTime);
	}
	else
	{
	  if(_params->debug)
	  {
	    cerr << "Skipping Event Type " << ETYPE << endl;
	  }
	}
	
	//
	// Keep track of the last time for the flush method.
	//
	_lastTime = doubleTime;
      }
      delete [] tstring;
      delete [] ETYPE;

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
void LDARingest::flush(){
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

