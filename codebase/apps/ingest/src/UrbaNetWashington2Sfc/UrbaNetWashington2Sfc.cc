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

#include <cstdio>
#include <string.h>
#include <math.h>
#include <toolsa/pmu.h>
#include <physics/physics.h>
#include <stdlib.h>
#include <toolsa/file_io.h>
#include <unistd.h>

#include "UrbaNetWashington2Sfc.hh"

//
// Constructor. Copies parameters.
//
UrbaNetWashington2Sfc::UrbaNetWashington2Sfc(Params *TDRP_params){
  //
  //
  // Point to the TDRP parameters.
  //
  _params = TDRP_params;
}



//
// Destructor. Does nothing.
//
UrbaNetWashington2Sfc::~UrbaNetWashington2Sfc(){
  return;
}



//
// Main routine.
//
void UrbaNetWashington2Sfc::UrbaNetWashington2SfcFile( char *filename ){
  //
  // Initialize variables.
  //

  date_time_t T;
  T.unix_time = time(NULL);
  uconvert_from_utime( &T );

  if (_params->debug){
    fprintf(stderr,"Processing file %s at %s\n", 
	    filename, utimstr(T.unix_time));
  }

  //
  // Make sure output directory exists.
  //
  if (ta_makedir_recurse(_params->outDir)){
    fprintf(stderr,"Failed to create output directory %s\n",
	    _params->outDir);
    return;
  }


  //
  // Open the input filename
  //
  FILE *fp = fopen(filename,"r");
  if (fp == NULL){
    fprintf(stderr, "%s not found.\n", filename);
    return;
  }

  const int lineLen = 1024;
  char Line[lineLen];


  FILE *ofp = NULL; // Will open this if we need it
  char outFileName[1024];

  int numRead = 0;
  while (NULL!=fgets(Line, lineLen, fp)){

    double lat, lon, temp, rh, pres, spd, dir;
    
    if (7 == sscanf(Line + strlen("ANPHS,"), "%lf,%lf,%lf,%lf,%lf,%lf,%lf",
		     &lat, &lon, &temp, &rh, &pres, &spd, &dir)){

      if (numRead == 0){
	//
	// Open the output file, put the header on it.
	//
	sprintf(outFileName,"%s/%d%02d%02d_%02d%02d%02d.sfc",
		_params->outDir, T.year, T.month, T.day, T.hour, T.min, T.sec);

	ofp = fopen(outFileName, "w");
	if (ofp == NULL){
	  fprintf(stderr,"Failed to create %s\n", outFileName);
	  return;
	}

	fprintf(ofp,"# CREATOR:       UrbaNetWashington2Sfc\n");
	fprintf(ofp,"# DATE:          %d-%02d-%02d %02d:%02d\n", 
		T.year, T.month, T.day, T.hour, T.min);
	fprintf(ofp,"# SOURCE:        OBS\n");
	fprintf(ofp,"# EDITED:        YES\n");
	fprintf(ofp,"# REFERENCE:     AGL\n");
	fprintf(ofp,"# TYPE:          OBSERVATION\n");
	fprintf(ofp,"# TIMEREFERENCE: UTC\n");
	fprintf(ofp,"# MODE:          OBS ALL\n");
	fprintf(ofp,"SURFACE\n");
	fprintf(ofp,"8\n");
	fprintf(ofp,"ID      YYYYMMDDHOUR    LAT     LON     Z       WDIR    WSPD\n");
	fprintf(ofp,"                HOURS   N       E       M       DEG     M/S\n");
	fprintf(ofp,"999.0000\n");


      }

      if (_params->debug >=Params::DEBUG_DATA)
	fprintf(stderr,"%s", Line);

      double decimalHours = T.hour + double(T.min)/60.0 + double(T.sec)/3600.0;

      numRead++;

      Line[5]=char(0); // Leave only the ID in the line

      fprintf(ofp, "%s %d%02d%02d %12.4f %12.4f %12.4f %12.4f %12.4f %12.4f\n",
              Line,
              T.year, T.month, T.day, decimalHours,
              lat, lon, _params->alt, dir, spd);


    }

  }

  if (_params->debug){
    fprintf(stderr,"%d station entries read.\n", numRead);
  }

  fclose( fp );
  if (ofp != NULL) fclose(ofp);

  if (_params->deleteWhenDone){
    if (_params->debug){
      fprintf(stderr,"Deleting %s\n", filename);
    }
    unlink(filename);
  }

  //
  // Run a script on the output file.
  //
  if (strlen(_params->swimScript) > 0){
    char com[1024];
    sprintf (com, "%s %s", _params->swimScript, outFileName);
    if (_params->debug){
      fprintf(stderr,"Running : %s\n", com);
    }
    system( com );
  }

  return;

}

