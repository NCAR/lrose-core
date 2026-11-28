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
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *P, time_t T){

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Read in new data at the time specified.
  //
  DsMdvx New;

  New.setDebug( P->Debug);

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

  DsMdvx Out;

  Out.setMasterHeader(InMhdr);
  Out.clearFields();     

  //
  // Commence loop through fields.
  //
  for (int i=0; i< InMhdr.n_fields; i++){
    //
    //
    //
    MdvxField *InField = New.getFieldByNum( i ); 

    if (InField == NULL){
      cerr << "Error reading field " << i+1 << " of " << InMhdr.n_fields << endl;
      continue;
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();

    if (InFhdr.proj_type != Mdvx::PROJ_POLAR_RADAR){
      cerr << "Field " << i+1 << " named " << InFhdr.field_name << " is not polar projection, skipping." << endl;
      continue;
    }

    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    fl32 *InData = (fl32 *) InField->getVol();

    fl32 *OutData = (fl32 *) malloc(sizeof(fl32) * 
				    P->outGrid.nGates *
				    P->outGrid.numBeams *
				    InFhdr.nz);

    if (OutData == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    //
    // Calculate the integer index range over which we are averaging in the existing grid.
    //
    int iDx = (int)rint(P->avSpace.averagingSpaceAlongBeamKm / InFhdr.grid_dx );
    int iDy = (int)rint(P->avSpace.averagingAzimuthAngleDeg / InFhdr.grid_dy );

    if (P->Debug){
      cerr << "iDx is " << iDx << ", iDy is " << iDy << " for field " << InFhdr.field_name << endl;
    }

    //
    // Loop through all the planes in the output (same number as in input).
    //
    for(int iz=0; iz < InFhdr.nz; iz++){
      //
      // Loop through the beams in the output.
      //
      for(int iy=0; iy < P->outGrid.numBeams; iy++){
	//
	// Get az and old iy (that is, iy index in existing grid).
	//
	double az = 360.0*double(iy)/double(P->outGrid.numBeams);
	int oldIy = (int)rint((az-InFhdr.grid_miny)/InFhdr.grid_dy);
	//
	// Loop through the gates - along the beam - in the output.
	//
	for(int ix=0; ix < P->outGrid.nGates; ix++){
	  //
	  // Get r and old ix (that is, ix index in existing grid).
	  //
	  double r = P->outGrid.firstGateRangeKm + ix * P->outGrid.gateSpacingKm;
	  int oldIx = (int)rint((r-InFhdr.grid_minx)/InFhdr.grid_dx);
	  //
	  // If we are avoiding "blob expansion" and the data in the input
	  // grid are bad/missing, then set the output value to bad/missing and
	  // have done with it.
	  //
	  if (P->noBlobExpansion){
	    int id = oldIx + oldIy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	    if ((InData[id]==InFhdr.bad_data_value) || (InData[id]==InFhdr.missing_data_value)){
	      OutData[ix + iy*P->outGrid.nGates + iz*P->outGrid.nGates*P->outGrid.numBeams] = InFhdr.missing_data_value;
	      continue;
	    }
	  }

	  //
	  // Loop over averaging space getting mean, min and max
	  //
	  double total = 0.0; int num = 0;
	  double min = 0; double max = 0; // Set to 0 to avoid compiler warnings

	  for (int ixx = oldIx - iDx; ixx <= oldIx + iDx; ixx++){
	    //
	    // ixx has to be in range 0..InFhdr.nx-1
	    //
	    if ((ixx < 0) || (ixx > InFhdr.nx-1)) continue;
	    //
	    for (int iyy = oldIy - iDy; iyy <= oldIy + iDy; iyy++){
	      //
	      // Since iyy is azimuthal, if it is out of range then it wraps.
	      //
	      int wrapIyy = iyy;

	      do {
		if ( wrapIyy < 0 )  wrapIyy += InFhdr.ny;
		if ( wrapIyy > InFhdr.ny-1 )  wrapIyy = wrapIyy - InFhdr.ny;
	      } while (( wrapIyy < 0 ) || ( wrapIyy > InFhdr.ny-1 ));

	      int index = ixx + wrapIyy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	      //
	      // Do we have valid data here?
	      //
	      if ((InData[index] == InFhdr.bad_data_value) ||
		  (InData[index] == InFhdr.missing_data_value)) continue;
	      //
	      // Consider data in the processing.
	      //
	      if (num == 0){
		min = InData[index]; max = InData[index];
	      } else {
		if (InData[index] < min) min = InData[index];
		if (InData[index] > max) max = InData[index];
	      }
	      num++; total += InData[index];

	    } // End of azimuthal averaging loop
	  } // End of range averaging loop

	  int newIndex = ix + iy*P->outGrid.nGates + iz*P->outGrid.nGates*P->outGrid.numBeams;

	  if (num == 0){
	    OutData[newIndex] = InFhdr.bad_data_value; // No data were found in averaging space
	  } else {

	    switch ( P->method){

	    case Params::METHOD_AVERAGE :
	      OutData[newIndex] = total/double(num);
	      break;

	    case Params::METHOD_MAX :
	      OutData[newIndex] = max;
	      break;

	    case Params::METHOD_MIN :
	      OutData[newIndex] = min;
	      break;
	      
	    default :
	      cerr << "Unrecognized processing method : " << P->method << endl;
	      exit(-1);
	      break;

	    }
	  }
	}
      }
    }
    
    //
    // Now have to muck with grid specified by InFhdr to get it ready for output.
    //
    InFhdr.nx = P->outGrid.nGates;
    InFhdr.ny = P->outGrid.numBeams;
    InFhdr.grid_minx = P->outGrid.firstGateRangeKm;
    InFhdr.grid_dy = 360.0 / double(P->outGrid.numBeams);
    InFhdr.grid_dx = P->outGrid.gateSpacingKm;
    InFhdr.volume_size = InFhdr.nx * InFhdr.ny * InFhdr.nz * sizeof(fl32);

    //
    // Create field and add it to output DsMdvx object.
    //
    MdvxField *fld = new MdvxField(InFhdr, InVhdr, OutData);

    if (fld->convertRounded(Mdvx::ENCODING_INT16,
                            Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl; // Unlikely
      exit(-1);
    }

    Out.addField(fld);
    free(OutData);
    
  } // End of loop through the fields.

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    return -1;
  }      

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










