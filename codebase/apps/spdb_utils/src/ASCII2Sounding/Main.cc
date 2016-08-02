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
#include <cstdlib>

#include <toolsa/umisc.h>
#include <Spdb/SoundingPut.hh>
#include <toolsa/umisc.h>

#include "Params.hh"


int main(int argc, char *argv[]){


  if (
      (argc < 2) ||
      (strlen(argv[1]) < strlen("2005051305.prof"))
      ){
    fprintf(stderr,"I need a filename, ie. 2005051305.prof\n");
    exit(-1);
  }
  
  date_time_t T; T.min = 0; T.sec = 0;
  char *p = argv[1]+ strlen(argv[1]) - strlen("2005051305.prof");
  
  if (4 != sscanf(p,"%4d%2d%2d%2d",
		  &T.year, &T.month, &T.day,
		  &T.hour)){
    fprintf(stderr,"Failed to parse time from %s\n", argv[1]);
    exit(-1);
  }
  uconvert_to_utime( &T );

  FILE *ifp = fopen(argv[1],"r");
  if (ifp == NULL){
    fprintf(stderr,"Failed to open %s\n", argv[1]);
    exit(-1);
  }

  int id;
  double lat, lon, alt;

  if (4 != fscanf(ifp,"%d %lf %lf %lf",
		  &id, &lat, &lon, &alt)){
    fprintf(stderr,"Failed to read first line of file.\n");
    exit(-1);
  }

  const int maxPoints = 1000;
  double  Height[maxPoints];
  double  u[maxPoints];
  double  v[maxPoints];
  double  Temp[maxPoints];
  double Pres[maxPoints];
  double rh[maxPoints];

  const double badVal = -9999.0;

  int go = 1; int numPoints = 0;
  do {
    if (6 == fscanf(ifp, "%lf %lf %lf %lf %lf %lf",
		    &Height[numPoints],
		    &Pres[numPoints],
		    &Temp[numPoints], 
		    &rh[numPoints],
		    &u[numPoints], 
		    &v[numPoints])){
      numPoints++;
      if (numPoints > maxPoints){
	fprintf(stderr,"Number of points exceeded.\n");
	exit(-1);
      }
    } else {
      go = 0;
    }
  } while ( go );

  fclose(ifp);
  fprintf(stderr,"%d points read.\n", numPoints);

  Params P;

  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }

  //
  // Write the data out. Have to push back url to do so.
  //
  vector< string* > urlVec;
  string Url( P.OutUrl );
  urlVec.push_back( &Url );

  SoundingPut S;

  S.init(urlVec,
         Sounding::DEFAULT_ID,
         "Model",
         id,
         "Model",
         lat, lon, alt, badVal );

  S.set(T.unix_time,
        numPoints,
        Height,
        u,
        v,
        NULL,
        Pres,
        rh,
        Temp);

  S.writeSounding( T.unix_time, T.unix_time + P.Expiry );

  return 0;

}
