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

#include <iostream>
#include <vector>
#include <string>

#include <toolsa/umisc.h>
#include <physics/physics.h>
#include <physics/thermo.h>     
#include <Spdb/SoundingPut.hh> 
#include <string.h>

#include <cstdio>

#include "Params.hh"
using namespace std;

int main(int argc, char *argv[]){
  //
  for (int i=1; i < argc; i++){
    if (!(strcmp(argv[i],"-h"))){
      cerr << "-print_params explains all." << endl;
      exit(-1);
    }
  }

  //
  // Load TDRP params.
  //
  Params *P = new  Params();
 
  if (P->loadFromArgs(argc, argv, NULL, NULL)){
    cerr << "Specify params file with -params option." << endl;
    exit(-1);
  }                  
  //
  // Check that arrays are the same size.
  //
  if (
      (P->lats_n != P->lons_n) ||
      (P->lats_n != P->alts_n) ||
      (P->lats_n != P->ids_n)
      ){
    cerr << "ERROR - location arrays differ in size." << endl;
    exit(-1);
  }

  if (
      (P->heights_n != P->temp_n) ||
      (P->heights_n != P->u_n) ||
      (P->heights_n != P->v_n) ||
      (P->heights_n != P->relHum_n)
      ){
    cerr << "ERROR - data arrays differ in size." << endl;
    exit(-1);
  }

  date_time_t start, end;

  if ( 6!= sscanf(P->Start,"%d %d %d %d %d %d",
		  &start.year, &start.month, &start.day,
		  &start.hour, &start.min, &start.sec)){
    cerr << "Format of start time not correct : " << P->Start << endl;
    exit(-1);
  }

  if ( 6!= sscanf(P->End,"%d %d %d %d %d %d",
		  &end.year, &end.month, &end.day,
		  &end.hour, &end.min, &end.sec)){
    cerr << "Format of end time not correct : " << P->End << endl;
    exit(-1);
  }

  uconvert_to_utime( & start ); uconvert_to_utime( &end );

  if (end.unix_time < start.unix_time){
    cerr << "Times backwards." << endl;
    exit(-1);
  }
  //
  // Set up array of standard pressures for these heights.
  //
  double *press = (double *) malloc(sizeof(double) * P->heights_n);
  if (press == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1); // Highly unlikely.
  }
  //
  double *adjHts = (double *) malloc(sizeof(double) * P->heights_n);
  if (adjHts == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1); // Highly unlikely.
  }
  //  
  //
  // Loop through the times.
  //
  time_t dataTime = start.unix_time;
  do {

    cerr << "Processing for " << utimstr( dataTime ) << endl;

    //
    // Loop through the locations at this time.
    //
    for (int i=0; i < P->lats_n; i++){

      double alt = P->_alts[i];
      double lat = P->_lats[i];
      double lon = P->_lons[i];
      int id = P->_ids[i];

      cerr << "Adding ID " << id << endl;
      //
      // Set up adjusted heights.
      //
      //  
      for (int k=0; k< P->heights_n; k++){
	if (P->_heights[k] == P->badVal){
	  adjHts[k]=P->badVal;
	} else {
	  adjHts[k] = P->_heights[k] + alt;
	}
      }
      //
      // And pressures.
      //
      for (int k=0; k< P->heights_n; k++){
	if (adjHts[k] == P->badVal){
	  press[k]=P->badVal;
	} else {
	  press[k]=PHYmeters2mb( adjHts[k] );
	}
      }
      vector< string* > urlVec;
      string Url( P->OutUrl );
      urlVec.push_back( &Url );
      
      SoundingPut S;
    
      char siteName[16];
      sprintf(siteName,"%d",id);

      S.init(urlVec, 
	     Sounding::DEFAULT_ID, 
	     "Synthetic", 
	     id,
	     siteName,
	     lat, lon, alt, P->badVal );
 
      S.set(dataTime,
	    P->heights_n,
	    adjHts,
	    P->_u,
	    P->_v,
	    NULL, // No W information.
	    press,
	    P->_relHum,
	    P->_temp);


      S.writeSounding( dataTime, dataTime + P->expiry );

    }// End of spatial loop.

    dataTime = dataTime + P->TimeStep*3600;

  } while(dataTime <= end.unix_time);

  free(press); free (adjHts);

  return 0;

}

