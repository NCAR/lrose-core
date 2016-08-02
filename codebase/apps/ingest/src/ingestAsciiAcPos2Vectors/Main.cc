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

#include <Spdb/DsSpdb.hh>
#include <cstdio>
#include <rapformats/acPosVector.hh>
#include <toolsa/umisc.h>
#include <toolsa/pmu.h>

#include "Params.hh"

int main(int argc, char *argv[]){


  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }                       
  
  PMU_auto_init("ingestAsciiAcPos2Vectors", P.instance,
                PROCMAP_REGISTER_INTERVAL);

  for (int iFile=0; iFile < P.inFilenames_n; iFile++){

    FILE *ifp = fopen(P._inFilenames[iFile], "r");
    if (ifp == NULL){
      fprintf(stderr,"%s does not exist.\n", P._inFilenames[iFile]);
      continue;
    }

    if (P.debug){
      cerr << "Processing " << P._inFilenames[iFile] << endl;
    }
 
    char Line[1024];
    int first = 1;
    
    DsSpdb out;

    time_t lastT=0L;
    double lastLat=0.0, lastLon=0.0, lastAlt=0.0;

    while( NULL != fgets(Line, 1024, ifp)){
      
      PMU_auto_register("processing...");
      
      char callSign[4];
      date_time_t T;
      double lat, lon, alt;
            
      if ( 9 == sscanf(Line+3,"%d,%d,%d,%d,%d,%d,%lf,%lf,%lf",
		       &T.year, &T.month, &T.day, 
		       &T.hour, &T.min, &T.sec,
		       &lat, &lon, &alt)){
	
	if (P.debug){
	  cerr << Line;
	}
	
	callSign[0] = Line[0];
	callSign[1] = Line[1];
	callSign[2] = char(0);
	callSign[3] = char(0);
	
	uconvert_to_utime( &T );
	
	if (first){
	  first = 0;
	  lastLat = lat; lastLon = lon; lastAlt = alt; lastT = T.unix_time;
	} else {
	  acPosVector PV(lastT, lastLat, lastLon, lastAlt,
			 T.unix_time, lat, lon, alt, callSign);

	  if (P.debug){
	    cerr << utimstr(lastT) << " : ";
	    cerr << lastLat << ", ";
	    cerr << lastLon << ", ";
	    cerr << lastAlt << " -> ";
	    cerr << utimstr(T.unix_time) << " : ";
	    cerr << lat << ", ";
	    cerr << lon << ", ";
	    cerr << alt << endl;
	    PV.print( cerr );
	  }
	  if (PV.assemble()){
	    cerr << "Assembly failed!" << endl;
	    return -1;
	  }
	  
	  int dataType = Spdb::hash4CharsToInt32( callSign );
	  
	  out.addPutChunk(dataType,
			  T.unix_time,
			  T.unix_time,
			  PV.getBufLen(),
			  PV.getBufPtr());
	  
	  lastLat = lat; lastLon = lon; lastAlt = alt; lastT = T.unix_time;
	}
      }
    }
    fclose(ifp);
    out.put( P.outUrl, SPDB_AC_VECTOR_ID, SPDB_AC_VECTOR_LABEL);
  }
  return 0;

}
