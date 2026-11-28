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


#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>
using namespace std;

//
// Constructor
//
Process::Process(){
  OutputUrl = (char *)NULL;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){
  
  OutputUrl = STRdup(P->OutUrl);
  
  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  
  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     
  
  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;

  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  //
  // Get the desired field - just take field number 0, there's only one.
  //
  MdvxField *InField;

  InField = New.getFieldByNum( 0 ); 

  if (InField == NULL){
    cerr << "No input fields found!!" << endl;
    return -1;
  }

  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
  
  fl32 *InData = (fl32 *) InField->getVol();

  fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				  InFhdr.nx *
				  InFhdr.ny *
				  InFhdr.nz);

  if (OutData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }

  int startX = (int)rint(P->avRange.rangeStartKm / InFhdr.grid_dx);
  int endX = (int)rint(P->avRange.rangeEndKm / InFhdr.grid_dx);
  endX++;

  if (endX > InFhdr.nx) endX = InFhdr.nx;  if (endX < 0) endX = 0;
  if (startX > InFhdr.nx) startX = InFhdr.nx;  if (startX < 0) startX = 0;

  int iDelY = (int) rint(P->avRange.elevationDegEitherSide / InFhdr.grid_dy );

  if (P->Debug){
    cerr << "Beam spacing is " << InFhdr.grid_dy << " deg, ";
    cerr << "considering " << iDelY << " beams either side." << endl;
  }

  // Loop through planes
  for (int iz=0; iz < InFhdr.nz; iz++){

    // Loop through beams
    for (int iy=0; iy < InFhdr.ny; iy++){

      // Get the total mean, all beams in range.

      int startY = iy - iDelY;
      int endY = iy + iDelY;
      endY++;

      if (endY > InFhdr.ny) endY = InFhdr.ny;  if (endY < 0) endY = 0;
      if (startY > InFhdr.ny) startY = InFhdr.ny;  if (startY < 0) startY = 0;

      double bigTotal = 0.0;
      unsigned long bigNum = 0;

      double littleTotal = 0.0;
      unsigned long littleNum = 0;

      for (int iyy = startY; iyy < endY; iyy++){
	for (int ixx = startX; ixx < endX; ixx++){
	  int index = iz*InFhdr.nx*InFhdr.ny + iyy * InFhdr.nx + ixx;
	  if (
	      (InData[index] != InFhdr.bad_data_value) &&
	      (InData[index] != InFhdr.missing_data_value)
	      ){
	    bigNum++; bigTotal += InData[index];
	    if (iyy == iy){
	       littleNum++; littleTotal += InData[index];
	    }
	  }
	}
      }

      if (bigNum == 0) continue; // Unlikely
      if (littleNum == 0) continue; // Unlikely

      double bigMean = bigTotal / double(bigNum);
      double littleMean = littleTotal / double(littleNum);

      if (P->Debug){
	cerr << "Regional mean " << bigMean << " beam mean " << littleMean <<  " diff " << bigMean - littleMean << endl;
      }

      for (int ix=0; ix < InFhdr.nx; ix++){
	int index = iz*InFhdr.nx*InFhdr.ny + iy * InFhdr.nx + ix;
	if (
	    (InData[index] != InFhdr.bad_data_value) &&
	    (InData[index] != InFhdr.missing_data_value)
	    ){
	  OutData[index] = InData[index] - littleMean + bigMean;
	} else {
	  OutData[index] = InFhdr.missing_data_value;
	}
      }
    }
  }
  
  MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
  
  free(OutData);
  
  if (fld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  Out.addField(fld);

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










