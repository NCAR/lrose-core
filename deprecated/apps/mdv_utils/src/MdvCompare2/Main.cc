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
    cerr << "USAGE : MdvCompare file1 file2  U_fieldname_file1 V_fieldname_file1 U_fieldname_file2 V_fieldname_file2 -params User.params" << endl;
    cerr << "eg. MdvCompare SomeDir/20000926/013000.mdv SomeOtherDir/20000926/013000.mdv U V U_FCST V_FCST -params User.params" << endl;
    exit(-1);
  }
  ///////////////////////////////////////////////////////////////
  //
  // Read in MDV data into object A
  //
  //
  DsMdvx A;
  A.setReadPath( argv[1] ); // Set MDV filename (note : not URL)
  A.addReadField( argv[3] );
  A.addReadField( argv[4] ); // Only read U and V fields.

  A.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  A.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (A.readVolume()){
    cerr << "Failed to read field " << argv[2] << " from file " << argv[1] << endl;
    exit (-1);
  }
  //
  // Get the A data and field header for U
  //
  MdvxField *AUField = A.getFieldByName( argv[3] );
  if (AUField == NULL){
    cerr << "Field " << argv[3]  << " not found." << endl;
    exit(-1);
  }
  fl32 *AUData = (fl32 *) AUField->getVol();
  Mdvx::field_header_t AUFhdr = AUField->getFieldHeader();
  //
  // Ditto V
  //
  MdvxField *AVField = A.getFieldByName( argv[4] );
  if (AVField == NULL){
    cerr << "Field " << argv[4]  << " not found." << endl;
    exit(-1);
  }
  fl32 *AVData = (fl32 *) AVField->getVol();
  Mdvx::field_header_t AVFhdr = AVField->getFieldHeader();
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
  B.setReadPath( argv[2] ); // Set MDV filename (note : not URL)     
  B.addReadField( argv[5] );
  B.addReadField( argv[6] ); // Only read U and V fields.

  B.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  B.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (B.readVolume()){
    cerr << "Failed to read field " << argv[4] << " from file " << argv[3] << endl;
    exit (-1);
  }
  //
  // Get the B data and field header for U
  //
  MdvxField *BUField = B.getFieldByName( argv[5] );
  if (BUField == NULL){
    cerr << "Field " << argv[5]  << " not found." << endl;
    exit(-1);
  }
  fl32 *BUData = (fl32 *) BUField->getVol();
  Mdvx::field_header_t BUFhdr = BUField->getFieldHeader();
  //
  // Ditto V
  //
  MdvxField *BVField = B.getFieldByName( argv[6] );
  if (BVField == NULL){
    cerr << "Field " << argv[6]  << " not found." << endl;
    exit(-1);
  }
  fl32 *BVData = (fl32 *) BVField->getVol();
  Mdvx::field_header_t BVFhdr = BVField->getFieldHeader();
  //
  // Gross check of geometry
  //
  if (
      (BUFhdr.nx != AUFhdr.nx) ||
      (BUFhdr.ny != AUFhdr.ny) ||
      (BUFhdr.nz != AUFhdr.nz) ||
      (BVFhdr.nx != AVFhdr.nx) ||
      (BVFhdr.ny != AVFhdr.ny) ||
      (BVFhdr.nz != AVFhdr.nz) 
      ){
    cerr << argv[1] << " and " << argv[2] << "have different grid sizes.";
    cerr << "Program terminated." << endl;
    exit(-1);
  }
  //
  // Initialize for stats calc.
  //
  int Num=0;
  double Result = P.BadValue;

  int outside = 0;
  //
  // Loop through subgrid.
  //
  int MaxSize = (P.MaxX-P.MinX+1)*(P.MaxY-P.MinY+1)*(P.MaxZ-P.MinZ+1);

  fl32 *UDiff = (float *)malloc(MaxSize*sizeof(float));
  fl32 *VDiff = (float *)malloc(MaxSize*sizeof(float));

  if ((UDiff==NULL) ||(VDiff == NULL)){
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
	    (ix < AUFhdr.nx) &&
	    (ix < AUFhdr.ny) &&
	    (iz < AUFhdr.nz)
	    ){ 
	  //
	  // ix and iy point inside grid.
	  //
	  int index = iz*AUFhdr.nx*AUFhdr.ny + iy*AUFhdr.nx +ix;
	  //
	  // Ignore missing/bad values.
	  //
	  if (
	      (AUData[index] != AUFhdr.missing_data_value) &&
	      (AUData[index] != AUFhdr.bad_data_value) &&
	      (BUData[index] != BUFhdr.missing_data_value) &&
	      (BUData[index] != BUFhdr.bad_data_value) &&
	      (AVData[index] != AVFhdr.missing_data_value) &&
	      (AVData[index] != AVFhdr.bad_data_value) &&
	      (BVData[index] != BVFhdr.missing_data_value) &&
	      (BVData[index] != BVFhdr.bad_data_value)
	      ) {
	    UDiff[Num]=AUData[index] - BUData[index];
	    VDiff[Num]=AVData[index] - BVData[index];

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
    cerr << "Main grid : " << AUFhdr.nx << " by " << AUFhdr.ny << endl;
    cerr << "Sub grid : X " << P.MinX << " to " << P.MaxX << endl;
    cerr << "Sub grid : Y " << P.MinY << " to " << P.MaxY << endl;
    cerr << "Sub grid regions outside main region have been ignored." << endl;
  }

  //
  // If we have data, do statistics.
  //
  if (Num > 0){
    //
    // COMPUTE THE RESULT.
    //
    double Total = 0.0;
    for (int i=0; i < Num; i++){
      Total=Total+sqrt(UDiff[i]*UDiff[i] * VDiff[i]*VDiff[i]);
    }
    Result = Total / double( Num );
  }

  free( UDiff ); free(VDiff);

  // Column oriented output
  printf("\t%10.4f\n",Result);
  //
  // Print to cout.
  //

  /*
  cout << AMhdr.time_centroid << " ";
  cout << Result;
  cout << endl;
  */
 
  return 0;

}



