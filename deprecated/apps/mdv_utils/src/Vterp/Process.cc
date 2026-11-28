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
  // Commence loop through fields.
  //
  for (int i=0; i< P->fields_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;
    InField = New.getFieldByName( P->_fields[i].name );

    if (InField == NULL){
      cerr << "New field " << P->_fields[i].name << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    if (P->Debug){
      cerr << "Processing field " <<  P->_fields[i].name << endl;
    }

    MdvxProj Proj(InMhdr, InFhdr);

    fl32 *InData = (fl32 *) InField->getVol();

    fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				    InFhdr.nx *
				    InFhdr.ny *
				    P->desiredHeights_n );

    if (OutData == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }
    //
    // Init to missing.
    //
    for(int l=0; l < InFhdr.nx*InFhdr.ny*P->desiredHeights_n; l++){
      OutData[l]=InFhdr.missing_data_value;
    }
    //
    // Find the existing heights that are closest to
    // our target height.
    //
    for (int iz=0; iz < P->desiredHeights_n; iz++){
      //
      double targetHeight = P->_desiredHeights[iz];
      int closestHtIndex = 0;
      //
      // Find the closest height.
      //
      double minDist = 0.0;
      int first = 1;
      for (int iez=0; iez < InFhdr.nz; iez++){
	if (first){
	  first = 0;
	  closestHtIndex = iez;
	  minDist = fabs( InVhdr.level[iez] - targetHeight );
	} else {
	  if (minDist > fabs( InVhdr.level[iez] - targetHeight )){
	    minDist = fabs( InVhdr.level[iez] - targetHeight );
	    closestHtIndex = iez;
	  }
	}
      }

      int nextClosestHtIndex = 0;

      //
      // Find next the closest height.
      //

      minDist = 0.0;
      first = 1;
      for (int iez=0; iez < InFhdr.nz; iez++){
	if (iez == closestHtIndex) continue;
	if (first){
	  first = 0;
	  nextClosestHtIndex = iez;
	  minDist = fabs( InVhdr.level[iez] - targetHeight );
	} else {
	  if (minDist > fabs( InVhdr.level[iez] - targetHeight )){
	    minDist = fabs( InVhdr.level[iez] - targetHeight );
	    nextClosestHtIndex = iez;
	  }
	}
      }
      //
      // Get t : when target height is closest height,
      // t == 1, when target height is next closest height, t
      // is 0.
      //
      double t = (targetHeight-InVhdr.level[nextClosestHtIndex])/
	(InVhdr.level[closestHtIndex]-InVhdr.level[nextClosestHtIndex]);

      if (P->Debug){
	cerr << "The closest existing heights to target " << targetHeight;
	cerr << " are " << InVhdr.level[closestHtIndex] << " and ";
	cerr << InVhdr.level[nextClosestHtIndex] << "(t is ";
	cerr << t << ")" << endl;
      }
      //
      // OK - fill this plane.
      //
      for (int ix = 0; ix < InFhdr.nx; ix++){
	for (int iy = 0; iy < InFhdr.ny; iy++){
	  //
	  int index = iz*InFhdr.ny*InFhdr.nx + iy*InFhdr.nx + ix;
	  //
	  int indexClosest = closestHtIndex*InFhdr.ny*InFhdr.nx +
	    iy*InFhdr.nx + ix;
	  //
	  int indexNextClosest = nextClosestHtIndex*InFhdr.ny*InFhdr.nx +
	    iy*InFhdr.nx + ix;
	  //
	  //
	  // If we have data, interpolate.
	  //
	  if (P->_fields[i].interpMethod == Params::INTERP_LINEAR){

	    if (
		(InData[indexClosest] == InFhdr.bad_data_value) ||
		(InData[indexClosest] == InFhdr.missing_data_value) ||
		(InData[indexNextClosest] == InFhdr.bad_data_value) ||
		(InData[indexNextClosest] == InFhdr.missing_data_value)
		){
	      OutData[index] = InFhdr.missing_data_value;
	    } else {
	      OutData[index] = t*InData[indexClosest] + (1.0-t)*InData[indexNextClosest];
	    }
	  }
	  if (P->_fields[i].interpMethod == Params::INTERP_POWER){

	    if (
		(InData[indexClosest] == InFhdr.bad_data_value) ||
		(InData[indexClosest] == InFhdr.missing_data_value)
		){
	      OutData[index] = InFhdr.missing_data_value;
	    } else {
	      double h0 = P->surfaceHeight;
	      double u0 = P->surfaceSpeed;
	      
	      double h1 = InVhdr.level[closestHtIndex];
	      double u1 = InData[indexClosest];
	      //
	      // Make sure u1 and u0 are of the same sign.
	      //
	      if (u1*u0 < 0.0){
		u0 = -u0;
	      }

	      //
	      // Make sure u0 is smaller in magnitude than u1.
	      //
	      if (fabs(u0) > fabs(u1)/10.0){
		u0 = u1 /1000.0;
	      }

	      double logA = (u1 * log(h0) - u0 * log(h1)) / (u0 - u1);

	      if (logA < 0.0){
		cerr << "logA is negative, I cannot cope." << endl;
		cerr << "U0 : " << u0 << endl;
		cerr << "U1 : " << u1 << endl;
		cerr << "H0 : " << h0 << endl;
		cerr << "H0 : " << h1 << endl;
		cerr << "logA : " << logA << endl;
		exit(-1);
	      }

	      double a = exp(logA);
	      double k = u1 / log(a*h1);
	      
	      if ((u1 == 0.0) && (u0 == 0.0)){
		OutData[index] = 0.0;
	      } else {
		OutData[index] = k*log(a*targetHeight);
	      }
	    }
	  }
	}
      }
    }
    //
    // Overwrite the appropriate parts of the input headers
    // for output.
    //
    InFhdr.nz = P->desiredHeights_n;
    InFhdr.volume_size = InFhdr.nx*InFhdr.ny*InFhdr.nz*sizeof(float);
    for (int iz=0; iz < P->desiredHeights_n; iz++){
      InVhdr.level[iz] = P->_desiredHeights[iz];
    }
    
    MdvxField *fld = new MdvxField(InFhdr, InVhdr, OutData);
    
    free(OutData);
    
    if (fld->convertRounded(Mdvx::ENCODING_INT16,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    Out.addField(fld);  

  }

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










