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
#include <cstdio>

#include "Params.hh"



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



  DsSpdb In;

  vector<int> doneStations;
 
  if (In.getInterval(P.InputUrl, start.unix_time, end.unix_time)){
    cerr << "Problem getting metars from " << P.InputUrl << endl;
    exit(-1);
  }
  cerr <<  In.getNChunks() << " stations read from " << P.InputUrl << endl;

  const vector <Spdb::chunk_t> &chunks = In.getChunks();


  FILE *fp = fopen(P.fileName,"w");
  if (fp == NULL){
    cerr << "Failed to create " << P.fileName << endl;
    exit(-1);
  }

  unsigned long numOut = 0;

  for (int i=0; i < In.getNChunks(); i++){

    station_report_t *S;

    S = (station_report_t *)chunks[i].data;
    station_report_from_be( S );
    //
    // See if we have already done this station.
    //
    int dataType = chunks[i].data_type;

    bool doneThis = false;
    for (unsigned j=0; j < doneStations.size(); j++){
      if (dataType ==  doneStations[j]){
	doneThis = true;
	break;
      }
    }

    if (doneThis) continue;

    doneStations.push_back( dataType );

    string stationID(Spdb::dehashInt32To4Chars(chunks[i].data_type));

    fprintf(fp,"%s, %f, %f, %f\n",
	    stationID.c_str(), S->lat, S->lon, S->alt);

    numOut++;

    if (P.Debug){
      fprintf(stderr,"%s, %f, %f, %f\n",
	      stationID.c_str(), S->lat, S->lon, S->alt);
    }
 
  }

  fclose(fp);

  if (P.Debug){
    cerr << numOut << " stations listed in " << P.fileName << endl;
  }

  return 0;

}
