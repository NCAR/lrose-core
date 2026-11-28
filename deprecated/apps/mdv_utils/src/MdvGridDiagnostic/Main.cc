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
// Small program for Andrew Crook. See paramdef file or
// use -print_params option for help.
//
//
// The program runs in archive mode only and does not
// check in with procmap. Niles Oien.
//
//
//

int Compare(const void *a, const void *b);  

#include <iostream>
#include <toolsa/umisc.h>
#include <Mdv/DsMdvx.hh>
#include <Mdv/DsMdvxTimes.hh>
#include <Mdv/MdvxField.hh>    
#include <cstdio>
#include <math.h>
#include <cstdlib>

#include "Params.hh"                  
using namespace std;

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

  //
  //
  // Get TDRP params
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }         
  //
  // Parse the start and end times.
  //
  date_time_t start, end;
 
  if (6!=sscanf(P.StartTime,"%d %d %d %d %d %d",
                &start.year, &start.month, &start.day,
                &start.hour, &start.min, &start.sec)){
    cerr << "Cannot decode start time in " << P.StartTime << endl;
    exit(-1);
  }
  uconvert_to_utime( &start );
                                        
  if (6!=sscanf(P.EndTime,"%d %d %d %d %d %d",
                &end.year, &end.month, &end.day,
                &end.hour, &end.min, &end.sec)){
    cerr << "Cannot decode end time in " << P.EndTime << endl;
    exit(-1);
  }
  uconvert_to_utime( &end );
  //
  // Set up time range
  //
  DsMdvxTimes InTimes;
  //
  //
  if (InTimes.setArchive(P.InUrl, start.unix_time, end.unix_time)){
    cerr << "Failed to get archive mode from " << P.InUrl;
    cerr << " with times " << utimstr (start.unix_time);
    cerr << " to " << utimstr(end.unix_time) << endl;
  }
  //
  // Open output file.
  //
  FILE *fp = fopen(P.OutFile,"wt");
  if (fp == NULL){
    cerr << "Failed to create " << P.OutFile << endl;
    exit(-1);
  }
  //
  // Loop through these times.
  //

  do{

    time_t Time;
    
    InTimes.getNext( Time );
    if ((Time == (time_t)NULL) ) break; // End of time list.
    //
    // Set up a DsMdvx object
    //               
    DsMdvx M;
    M.addReadField( P.InFieldName ); // Only read that one field.           
    M.setReadTime(Mdvx::READ_FIRST_BEFORE, P.InUrl, 0, Time);
    M.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
    M.setReadCompressionType(Mdvx::COMPRESSION_NONE);


    if (M.readVolume()){
      cerr << "Read volume from " << P.InUrl << " at " << utimstr(Time);
      cerr << " failed." << endl;
      exit (-1);
    }

    //
    // Get the data and field header.
    //
    MdvxField *InField = M.getFieldByName(P.InFieldName);
    fl32 *InData = (fl32 *) InField->getVol();
    Mdvx::field_header_t Fhdr = InField->getFieldHeader();

    //
    // Initialize for stats calc.
    //
    
    int Num=0;
    double Min= P.BadValue;
    double Max= P.BadValue;
    double Mean=P.BadValue;
    double TopTen = P.BadValue;
    double BotTen = P.BadValue;


    double Total = 0.0;
    int first=1;
    int outside = 0;
    //
    // Loop through subgrid.
    //
    int MaxSize = (P.MaxX-P.MinX+1)*(P.MaxY-P.MinY+1)*(P.MaxZ-P.MinZ+1);

    fl32 *data = (float *)malloc(MaxSize*sizeof(float));
    if (data==NULL){
      fprintf(stderr,"Malloc failed.\n");
      exit(-1);
    }

    for (int ix=P.MinX; ix <= P.MaxX; ix++){
      for (int iy=P.MinY; iy <= P.MaxY; iy++){
	for (int iz=P.MinZ; iz <= P.MaxZ; iz++){   

	  //
	  // Only accept values inside the main grid.
	  //
	  if (
	      (ix >= 0) &&
	      (iy >= 0) &&
	      (iz >= 0) &&
	      (ix < Fhdr.nx) &&
	      (ix < Fhdr.ny) &&
	      (iz < Fhdr.nz)
	      ){ 
	    //
	    // ix and iy point inside grid.
	    //
	    int index = iz*Fhdr.nx*Fhdr.ny + iy*Fhdr.nx +ix;
	    //
	    // Ignore missing/bad values.
	    //
	    if (
		(InData[index] != Fhdr.missing_data_value) &&
		(InData[index] != Fhdr.bad_data_value)
		) {
	      data[Num]=InData[index];
	      Num++;
	      Total = Total + InData[index];
	      if (first){
		first=0; Min=InData[index]; Max=Min;
	      } else {
		if (Min > InData[index]) Min = InData[index];
		if (Max < InData[index]) Max = InData[index];
	      }
	    }
	  } else { // ix and iy point outside grid.
	    outside = 1;
	  }

	}
      }
    }

    if (outside){
      cerr << "WARNING! Subgrid exceeds main grid." << endl;
      cerr << "Main grid : " << Fhdr.nx << " by " << Fhdr.ny << endl;
      cerr << "Sub grid : X " << P.MinX << " to " << P.MaxX << endl;
      cerr << "Sub grid : Y " << P.MinY << " to " << P.MaxY << endl;
      cerr << "Sub grid regions outside main region have been ignored." << endl;
    }

    if (Num > 0){
      Mean = Total / double(Num);
      int BotTenIndex = (int)rint((Num-1) * 0.1);
      int TopTenIndex = (int)rint((Num-1) * 0.9);

      qsort(data, Num, sizeof(float), Compare);

      TopTen = data[TopTenIndex];
      BotTen = data[BotTenIndex];


    }

    free( data );


    //
    // Generate output.
    //
    date_time_t DataTime;
    DataTime.unix_time = Time;
    uconvert_from_utime( &DataTime );
    //
    // Add the date.
    //
    fprintf(fp,"%8ld  %d\t%02d\t%02d\t%02d\t%02d\t%02d\t",
	    (long)(Time - start.unix_time),
	    DataTime.year, DataTime.month, DataTime.day,
	    DataTime.hour, DataTime.min, DataTime.sec);
    //
    // Then the data.
    //
    fprintf(fp,"%d\t%6.10f\t%6.10f\t%6.10f\t%6.10f\t%6.10f\t%6.10f\n",
	    Num, Total, Min, Mean,Max,BotTen,TopTen);
	    
  } while (1); // Loop exits with break statement inside it.

  fclose(fp);
  return 0;

}

//////////////////////////////////////////
//
// Comparison routine for ascending order.
//
int Compare(const void *a, const void *b){

  float *x = (float *)a;  float *y = (float *)b;

  if (*x >  *y) return 1;
  if (*x <  *y) return -1;
  return 0;

}
