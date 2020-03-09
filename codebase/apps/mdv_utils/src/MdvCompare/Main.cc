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
  // Get TDRP params. Do this before checking number of cli args
  // so that -print_params option can be processed.
  //
  Params P;
  if (P.loadFromArgs(argc,argv,NULL,NULL)){
    cerr << "Specify params file with -params option." << endl ;
    return(-1);
  }         
  //
  // See if we have enough cli parameters.
  //
  if (argc < 7){
    cerr << "USAGE : MdvCompare file1 field1 file2 field2 -params User.params" << endl;
    cerr << "eg. MdvCompare SomeDir/20000926/013000.mdv div SomeOtherDir/20000926/013000.mdv div -parapms User.params" << endl;
    exit(-1);
  }
  ///////////////////////////////////////////////////////////////
  //
  // Read in MDV data into object A
  //
  //
  DsMdvx A;
  A.setReadPath( argv[1] ); // Set MDV filename (note : not URL)
  A.addReadField( argv[2] ); // Only read that one field.           

  A.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  A.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (A.readVolume()){
    cerr << "Failed to read field " << argv[2] << " from file " << argv[1] << endl;
    exit (-1);
  }
  
  //
  // Get the data and field header.
  //
  MdvxField *AField = A.getFieldByName( argv[2] );
  fl32 *AData = (fl32 *) AField->getVol();
  Mdvx::field_header_t AFhdr = AField->getFieldHeader();
  //
  // Get master header for A so we can get time_centroid
  //
  Mdvx::master_header_t AMhdr = A.getMasterHeader();
  

  ///////////////////////////////////////////////////////////////
  //
  // Read in MDV data into object B
  //
  //
  DsMdvx B;
  B.setReadPath( argv[3] ); // Set MDV filename (note : not URL)
  B.addReadField( argv[4] ); // Only read that one field.           

  B.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  B.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (B.readVolume()){
    cerr << "Failed to read field " << argv[4] << " from file " << argv[3] << endl;
    exit (-1);
  }
  //
  // Get the data and field header.
  //
  MdvxField *BField = B.getFieldByName( argv[4] );
  fl32 *BData = (fl32 *) BField->getVol();
  Mdvx::field_header_t BFhdr = BField->getFieldHeader();
  
  //
  // Gross check of geometry
  //

  if (
      (BFhdr.nx != AFhdr.nx) ||
      (BFhdr.ny != AFhdr.ny) ||
      (BFhdr.nz != AFhdr.nz)
      ){
    cerr << argv[1] << " and " << argv[3] << "have different grid sizes.";
    cerr << "time centroid: " << AMhdr.time_centroid << endl;
    cerr << "Program terminated." << endl;
    exit(-1);
  }


  //
  // Initialize for stats calc.
  //
    
  int Num=0;
  double Min= P.BadValue;
  double Max= P.BadValue;
  double Mean=P.BadValue;
  double TopTen = P.BadValue;
  double Median = P.BadValue;
  double BotTen = P.BadValue;
  double STD = P.BadValue;
  double RMS = P.BadValue;


  double Total = 0.0;
  int outside = 0;
  //
  // Loop through subgrid.
  //
  int MaxSize = (P.MaxX-P.MinX+1)*(P.MaxY-P.MinY+1)*(P.MaxZ-P.MinZ+1);

  fl32 *Diff = (float *)malloc(MaxSize*sizeof(float));
  if (Diff==NULL){
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
	    (ix < AFhdr.nx) &&
	    (ix < AFhdr.ny) &&
	    (iz < AFhdr.nz)
	    ){ 
	  //
	  // ix and iy point inside grid.
	  //
	  int index = iz*AFhdr.nx*AFhdr.ny + iy*AFhdr.nx +ix;
	  //
	  // Ignore missing/bad values.
	  //
	  if (
	      (AData[index] != AFhdr.missing_data_value) &&
	      (AData[index] != AFhdr.bad_data_value) &&
	      (BData[index] != BFhdr.missing_data_value) &&
	      (BData[index] != BFhdr.bad_data_value)
	      ) {
	    Diff[Num]=AData[index] - BData[index];
	    Total = Total + Diff[Num];
	    Num++; // Get ready for the next one.
	  }
	} else { // ix and iy point outside grid.
	  outside = 1;
	}
	
      }
    }
  }
  
  if (outside){
    cerr << "WARNING! Subgrid exceeds main grid." << endl;
    cerr << "Main grid : " << AFhdr.nx << " by " << AFhdr.ny << endl;
    cerr << "Sub grid : X " << P.MinX << " to " << P.MaxX << endl;
    cerr << "Sub grid : Y " << P.MinY << " to " << P.MaxY << endl;
    cerr << "Sub grid regions outside main region have been ignored." << endl;
  }

  //
  // If we have data, do statistics.
  //
  if (Num > 0){
    //
    // Compute the mean
    //
    Mean = Total / double(Num);
    //
    // Then the standard deviation.
    //
    double D=0.0;
    for (int i=0; i< Num; i++){
      D=D+(Diff[i]-Mean)*(Diff[i]-Mean);
    }
    double MD = D/Num;
    STD = sqrt( MD );
    //
    // Then the RMS error
    //
    double S=0.0;
    for (int i=0; i< Num; i++){
      S=S+Diff[i]*Diff[i];
    }
    double MS = S / Num;
    RMS = sqrt( MS );
    //
    // And then the 10 percentile, median, and 90 percentile.
    //
    int BotTenIndex = (int)rint((Num-1) * 0.1);
    int MidIndex =    (int)rint((Num-1) * 0.5);
    int TopTenIndex = (int)rint((Num-1) * 0.9);
    //
    // Sort the data into order.
    //
    qsort(Diff, Num, sizeof(float), Compare);
    //
    // Pick off min, bottom tenth percentile, median, 
    // top tenth percentile,
    // and max.
    //
    TopTen = Diff[TopTenIndex];
    Median = Diff[MidIndex];
    BotTen = Diff[BotTenIndex];
    Min = Diff[0]; Max = Diff[Num-1];

  }

  free( Diff );

  //
  // Print to cout.
  //

  printf("%10.4f %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f %10.4f\n",
		 Min,Mean,Max,BotTen,Median,TopTen,RMS,STD);

  /*
  cout << AMhdr.time_centroid << " ";
  cout << Min << " ";
  cout << Mean << " ";
  cout << Max << " ";

  cout << BotTen << " ";
  cout << Median << " ";
  cout << TopTen << " ";

  cout << RMS << " ";
  cout << STD << " ";

  cout << endl;
  */ 
 
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



