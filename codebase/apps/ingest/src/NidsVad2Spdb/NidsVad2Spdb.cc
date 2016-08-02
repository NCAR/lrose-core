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
#include <iostream>
#include <cstring>
#include <vector>
#include <physics/physics.h>

#include <toolsa/umisc.h>

#include <Spdb/SoundingPut.hh>

#include "NidsVad2Spdb.hh"
using namespace std;
//
// constructor - does everything.
//
NidsVad2Spdb::NidsVad2Spdb(char *Filename, Params *P){

  if (P->debug){
    cerr << "Processing file " << Filename << endl;
  }
  //
  // See if we can get the three letter location
  // code and time from the filename.
  //
  if (strlen(Filename) < strlen("FTG/19990525/19990525.0055.VAD")){
    cerr << "Filename is not in format FTG/19990525/19990525.0055.VAD" << endl;
    cerr << "Will not process this file." << endl;
    return;
  }

  char *parse = Filename + strlen(Filename) - 
    strlen("FTG/19990525/19990525.0055.VAD");

  char LocationCode[4];
  date_time_t T;
  T.sec = 0;
  long dummy;

  if (7!= sscanf(parse,
		 "%3s/%8ld/%4d%2d%2d.%2d%2d.VAD",
		 LocationCode, &dummy, &T.year, &T.month, &T.day,
		 &T.hour, &T.min)){
    cerr << "Cannot parse requisite information from ";
    cerr << "filename " << Filename << " skipping this file" << endl;
    return;
  }
  uconvert_to_utime( &T );
  if (P->debug){
    cerr << "Decoded time is " << utimstr(T.unix_time) << endl;
    cerr << "Location code " << LocationCode << endl;
  }
  //
  //
  // See if we can look up this station code.
  // Set these to 0 to avoid compiler warning.
  //
  double siteLat=0.0, siteLon=0.0, siteAlt=0.0;
  char *siteName = NULL;
  int siteId = 0;
  int gotSite=0;
  for (int i=0; i<P->siteMap_n; i++){
    if (!(strcmp(LocationCode,P->_siteMap[i].vadID))){
      gotSite = 1;
      siteLat = P->_siteMap[i].lat;
      siteLon = P->_siteMap[i].lon;
      siteAlt = P->_siteMap[i].alt;
      siteId  = P->_siteMap[i].soundingSiteId;
      siteName = P->_siteMap[i].vadID;
    }
  }

  if (!(gotSite)){
    cerr << "Location " << LocationCode << " is not listed in parameters.";
    cerr << " Skipping this file." << endl;
    return;
  }

  if (P->debug){
    cerr << "Location lat, lon, alt : ";
    cerr << siteLat << ", " << siteLon << ", " << siteAlt;
    cerr << endl;
  }


  FILE *fp = fopen(Filename,"rt");
  if (fp == NULL){
    cerr << "Could not find file " << Filename << endl;
    return;
  }
  //
  // Read the file twice - first to find the number
  // of points, then to actually process it.
  //
  int NumPoints = 0;

  const int LineLen = 1024;
  char Line[LineLen];

  double alt_single, dir_single, speed_single;
  int time_single;

  while(NULL!=fgets(Line,LineLen,fp)){
    if (4==sscanf(Line,"%lf %lf %lf %d",
		  &alt_single, &dir_single, 
		  &speed_single, &time_single )){
      NumPoints++;
    }
  }

  rewind(fp);

  if (P->debug){
    cerr << NumPoints << " points in " << Filename << endl;
  }
  //
  // Allocate space and read into arrays.
  //
  double *alt = (double *)malloc(sizeof(double)*NumPoints);
  double *dir = (double *)malloc(sizeof(double)*NumPoints);
  double *speed = (double *)malloc(sizeof(double)*NumPoints);

  double *u = (double *)malloc(sizeof(double)*NumPoints);
  double *v = (double *)malloc(sizeof(double)*NumPoints);

  if (
      (u==NULL) || (v==NULL) || (speed == NULL) ||
      (dir == NULL) || (alt == NULL)
      ){
    cerr << "Malloc failure." << endl;
    exit(-1);
  }

  int index=0;
  while(NULL!=fgets(Line,LineLen,fp)){
    if (4==sscanf(Line,"%lf %lf %lf %d",
		  &alt[index], &dir[index], 
		  &speed[index], &time_single )){

      if (index >= NumPoints){
	cerr << "Array lengths not calculated correctly!!" << endl;
	exit(-1);
      }
      index++;
    }
  }

  if (index != NumPoints){
    cerr << "Array lengths not calculated correctly!!" << endl;
    exit(-1);
  }

  fclose(fp);
  //
  // Convert speed, dir to U,V
  //
  for(int i=0; i<NumPoints; i++){
    u[i]=PHYwind_u(speed[i], dir[i]);
    v[i]=PHYwind_v(speed[i], dir[i]);
  }

  //
  // Done with input file. Print out what we read, if requested.
  //
  if (P->debug){
    for(int i=0; i<NumPoints; i++){
      cerr << "alt=" << alt[i];
      cerr << "  dir=" << dir[i];
      cerr << "  speed=" << speed[i];
      cerr << " u=" << u[i];
      cerr << " v=" << v[i] << endl;
    }
    cerr << endl;
  }

  SoundingPut S;
  //
  // Have to push back Url into vector.
  //
  vector< string* > urlVec;
  string Url( P->OutUrl );
  urlVec.push_back( &Url );


  S.init(urlVec, 
	 Sounding::VAD_ID, 
	 "NidsVad2Spdb", 
	 0, siteName,
	 siteLat, siteLon, 
	 siteAlt, -999.0 );

  S.setSiteId( siteId );
  S.set( T.unix_time, NumPoints, alt, u, v);
  S.writeSounding( T.unix_time, T.unix_time + P->expiry);

  //
  // Free work arrays.
  //
  free(u); free(v); free(speed); free(dir); free( alt );

}
//
//
// destructor - does nothing.
//
NidsVad2Spdb::~NidsVad2Spdb (){
}

