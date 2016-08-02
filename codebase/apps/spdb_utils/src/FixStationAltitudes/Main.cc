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
//
// This is a small program that reads in SPDB METAR data, and
// also reads in a list of station locations and altitudes.
// The station data then have their lat, lon and altitude
// replaced by that specified in the file.
//
// Program was occasioned by a station location file that
// had the altiudes all wrong, thus requiring a fix.
//
// The program runs in archive mode only and does not
// check in with procmap. Niles Oien.
//
//
//
#include <iostream>
#include <Spdb/DsSpdb.hh>
#include <toolsa/umisc.h>
#include <vector>
#include <rapformats/station_reports.h>

#include "Params.hh"
#include "StationLocate.hh"



int main(int argc, char *argv[]){      
  //
  // Is this a cry for help?
  //
  for (int i=0; i < argc; i++){
    if (
	(!(strcmp(argv[i], "-h"))) ||
	(!(strcmp(argv[i], "--"))) ||
	(!(strcmp(argv[i], "-?")))
	){
      cerr << "For help, try the -print_params option and read the comments." << endl;
      exit(0);
    }
  }

  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }        


  date_time_t start, end;

  if (6!=sscanf(P.StartTimeString,"%d %d %d %d %d %d",
		&start.year, &start.month, &start.day,
		&start.hour, &start.min, &start.sec)){
    cerr << "Cannot decode start time in " << P.StartTimeString << endl;
    exit(-1);
  }
  uconvert_to_utime( &start );

  if (6!=sscanf(P.EndTimeString,"%d %d %d %d %d %d",
		&end.year, &end.month, &end.day,
		&end.hour, &end.min, &end.sec)){
    cerr << "Cannot decode end time in " << P.EndTimeString << endl;
    exit(-1);
  }
  uconvert_to_utime( &end );


  StationLocate L;
  if (L.load(P.StationLocationFile)){
    cerr << "Failed to load station locations from " << P.StationLocationFile << endl;
    exit(-1);
  }

  DsSpdb In, Out;
  
  Out.setAppName("FixStationAltitudes");
  Out.clearPutChunks();
  Out.clearUrls();
  Out.addUrl(P.OutputUrl);
 
  if (In.getInterval(P.InputUrl, start.unix_time, end.unix_time)){
    cerr << "Problem getting metars from " << P.InputUrl << endl;
    exit(-1);
  }
  cerr <<  In.getNChunks() << " stations read from " << P.InputUrl << endl;

  const vector <Spdb::chunk_t> &chunks = In.getChunks();
  long numOut = 0;

  for (int i=0; i < In.getNChunks(); i++){

    station_report_t *S;

    S = (station_report_t *)chunks[i].data;

    station_report_from_be( S );

    double newLat, newLon, newAlt;
    string stationID(Spdb::dehashInt32To4Chars(chunks[i].data_type));


    bool Found = true;
    if (L.getPos(stationID, newLat, newLon, newAlt)){
      Found = false;
      if (P.Debug){
	cerr << "WARNING : station " << stationID;
	cerr << " found in data but not in " << P.StationLocationFile << endl;
	  }
    } else {
      if (P.Debug){
	cerr << "Alt : " << S->alt << " to " << newAlt << endl;
	cerr << "Lat : " << S->lat << " to " << newLat << endl;
	cerr << "Lon : " << S->lon << " to " << newLon << endl << endl;
	
      }
      S->alt = newAlt;
      S->lon = newLon;
      S->lat = newLat;
    }

    if ((Found) || (P.PassUnlocatedStations)){

      station_report_to_be( S );

      Out.addPutChunk(chunks[i].data_type,
		      chunks[i].valid_time,
		      chunks[i].expire_time,
		      sizeof(station_report_t),
		      (void *) S,
		      chunks[i].data_type2);
      numOut++;
    }
  }
  if (Out.put(SPDB_STATION_REPORT_ID,
	      SPDB_STATION_REPORT_LABEL)) {
    cerr << "ERROR - writing data" << endl;
    cerr << "  Cannot put output to url: " << P.OutputUrl << endl;
    return -1;
  }

  cerr << numOut << " stations written to " << P.OutputUrl << endl;

  return 0;

}
