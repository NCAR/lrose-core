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
#include <iostream>
#include <rapformats/ltg.h>
#include <toolsa/umisc.h>
#include <Spdb/DsSpdb.hh>


#include "Process.hh"
using namespace std;

int Process(char *Filename, Params *P){


  //
  // Open the file.
  //
  FILE *fp = fopen(Filename,"rt");
  if (fp == NULL){
    cerr << "Input file " << Filename << " not found." << endl;
    return -1;
  }


  //
  // Loop through, line by line, reading strikes and
  // adding them to the database.
  //
  char Line[1024];

  DsSpdb OutSpdb;

  OutSpdb.clearPutChunks();
  OutSpdb.clearUrls();
  OutSpdb.addUrl(P->OutputUrl);      

  while (NULL!=(fgets(Line, 1024, fp))){
    LTG_strike_t strk;
    strk.type =  LTG_TYPE_UNKNOWN;

    int amp;
    long t;
    if (4 == sscanf(Line,"%ld %f %f %d",
		    &t, &strk.longitude,
		    &strk.latitude, &amp)){

      strk.time = t;
      strk.amplitude = amp;

      if (P->Debug){
	cerr << "Time      : " << utimstr(strk.time) << endl;
	cerr << "Latitude  : " << strk.latitude << endl;
	cerr << "Longitude : " << strk.longitude << endl;
	cerr << "Amplitude : " << strk.amplitude << endl;
      }
      //
      // Check if we are inside the region.
      //
      if (
	  (strk.longitude < P->MinLon) ||
	  (strk.longitude > P->MaxLon) ||
	  (strk.latitude < P->MinLat) ||
	  (strk.latitude > P->MaxLat) 
	  ){

	if (P->Debug) cerr << "Strike outside of region" << endl << endl;

      } else {
	if (P->Debug) cerr << "Adding strike to database." << endl << endl;

	//
	// Try to get unique dataType and dataType2 so that strikes at
	// the same time do not overwrite each other.
	//
	int dataType, dataType2;
	dataType = (int) rint( (strk.latitude + 90.0) * 100.0);
	dataType2 = (int) rint( (strk.longitude + 180.0) * 100.0);

	LTG_to_BE( &strk );

	OutSpdb.addPutChunk( dataType,
			    t, t + P->ExpireSecs,
			    sizeof(strk),
			    (void *) &strk,
			    dataType2
			    );  
      }
      
    } else {
      if (P->Debug){
	cerr << "WARNING : Could not decode the following line :" << endl;
	cerr << Line;
	cerr << "This may well not be a fatal error." << endl << endl;
      }
    }

  }

  fclose(fp);

  OutSpdb.put( SPDB_LTG_ID, SPDB_LTG_LABEL);

  return 0;

}


