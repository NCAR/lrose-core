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
#include <physics/physics.h>

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

}
//
// Destructor. Does nothing.
//
LMAingest::~LMAingest(){
 
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
  

  FILE *fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", filename);
    return;
  }
  
  const int lineLen = 1024;
  char Line[lineLen];
  
  LTG_strike_t L;
  
  int numRead = 0;
  date_time_t T;
  bool gotDate = false;
  //
  DsSpdb OutGenPts; OutGenPts.clearPutChunks();
  DsSpdb OutLtg;    OutLtg.clearPutChunks();
  //
  while (NULL!=fgets(Line, lineLen, fp)){
    //
    // See if we have the date line.
    //
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
    //
    // See if we have data, after we have the date.
    //
    if (gotDate){
      memset(&L, 0, sizeof(L));
      double nSecs, alt, chi2, power;
      //
      if ( (4 == sscanf(Line, "%lf %f %f %lf", &nSecs, &L.latitude, &L.longitude, &alt) 
	    && !_params->isColoradoLma) ||  
	   (6 == sscanf(Line, "%lf %f %f %lf %lf %lf", &nSecs, &L.latitude, &L.longitude, &alt,
			&chi2, &power) 
	    && _params->isColoradoLma)){
	//
	// Forget it if it's out of range.
	//
	if (
	    (L.latitude < _params->minLat) ||
	    (L.latitude > _params->maxLat) ||
	    (L.longitude < _params->minLon) ||
	    (L.longitude > _params->maxLon) ||
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
	  PJGLatLon2RTheta(L.latitude,
			   L.longitude,
			   _params->centralPoint.lat,
			   _params->centralPoint.lon,
			   &r, &theta);
	  if (r >  _params->centralPoint.maxRangeKm){
	    continue;
	  }
	}
	
	//
	// Thresh on Chi^2 value if relevant
	//
	if(_params->isColoradoLma && _params->threshOnChi2){
	  
	  if (chi2 > _params->chi2Thresh)
	    continue;
	}
	
	//
	// Thresh on power value if relevant
	//
	if(_params->isColoradoLma && _params->threshOnPower){
	  
	  if (power < _params->powerThresh)
	    continue;
	}


        //
        // Thresh on height value if relevant
        //
        if(_params->isColoradoLma && _params->threshOnHeight){

          if (alt > _params->heightThresh)
            continue;
        }


	//
	// Get the fractional part of the number of seconds, and
	// use it to get the datatype.
	//
	double fracSecs = nSecs - floor(nSecs);
	int dataType = (int)rint(100000.0*fracSecs);
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

	L.time = T.unix_time;
      
	L.type = LTG_TYPE_UNKNOWN;
	L.amplitude = 0;
	numRead++;
	//
	// Save the data out. This can be done as a ltg
	// struct or as a GenPt, the latter preserving the
	// altitude and enabling an altitude encoded display.
	//
	//
	if (_params->saveGenPtFormatData){
	  //
	  // Set up a GenPt and save.
	  //
	  GenPt G;
	  G.setName("Ltg point with altitude");
	  G.setId( 0 );
	  G.setTime( L.time );
	  G.setLat( L.latitude );
	  G.setLon( L.longitude );
	  G.setNLevels( 1 );
	  G.addVal( alt );
	  G.addFieldInfo( "Altitude", "m");
	  
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
	  OutGenPts.addPutChunk( dataType,
				 T.unix_time,
				 T.unix_time + _params->Expiry,
				 G.getBufLen(), G.getBufPtr(),
				 0 );
	}
	//
	// Save the traditional ltg struct, if requested.
	//
	if (_params->saveLtgFormatData){
	  //
	  LTG_to_BE( &L );
	  //
	  // Save the point out.
	  //
	  OutLtg.addPutChunk( dataType,
		      T.unix_time,
		      T.unix_time + _params->Expiry,
		      sizeof(LTG_strike_t), &L, 0);
	}
      }
    }
  }
  //
  // Close the input file.
  //
  fclose( fp );
  //
  // Save out the chunks we have added.
  //
  if (_params->saveLtgFormatData){
    if (OutLtg.put( _params->ltgOutUrl,
		    SPDB_LTG_ID,
		    SPDB_LTG_LABEL)){
      fprintf(stderr,"Failed to put ltg data\n");
      exit(-1);
    }
  }

  if (_params->saveGenPtFormatData){
    if (OutGenPts.put( _params->genPtOutUrl,
		      SPDB_GENERIC_POINT_ID,
		      SPDB_GENERIC_POINT_LABEL)){
      fprintf(stderr,"Failed to put GenPt data\n");
      exit(-1);
    }
  }

  if (_params->debug){
    fprintf(stderr,"%d strikes written.\n", numRead);
  }
  
  return;

}
