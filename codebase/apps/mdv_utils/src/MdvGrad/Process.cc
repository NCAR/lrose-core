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


  New.setDebug( P->Debug);

  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(P->grid_nx, P->grid_ny,
			P->grid_minx, P->grid_miny,
			P->grid_dx, P->grid_dy,
			P->grid_origin_lat, 
			P->grid_origin_lon,
			P->grid_lat1,  P->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }

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
  for (int i=0; i< P->InFieldName_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    if (!(strncmp(P->_InFieldName[i],"#",1))){
      int Fnum = atoi(P->_InFieldName[i] + 1);
      InField = New.getFieldByNum( Fnum ); 
    } else {
      InField = New.getFieldByName( P->_InFieldName[i] );
    }

    if (InField == NULL){
      cerr << "New field " << P->_InFieldName[i] << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    MdvxProj Proj(InMhdr, InFhdr);

    fl32 *InData = (fl32 *) InField->getVol();

    fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				    InFhdr.nx *
				    InFhdr.ny *
				    InFhdr.nz);

    if (OutData == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }
    //
    // Init to missing.
    //
    for(int l=0; l < InFhdr.nx*InFhdr.ny*InFhdr.nz; l++){
      OutData[l]=InFhdr.missing_data_value;
    }
    //
    // Set up this array, which we will use later.
    //
    fl32 *SurroundVal = (fl32 *)malloc(sizeof(fl32)*P->NumRingPoints);
    if (SurroundVal == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    for(int iz=0; iz < InFhdr.nz; iz++){
      for(int iy=0; iy < InFhdr.ny; iy++){
	for(int ix=0; ix < InFhdr.nx; ix++){
	  int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	  //
	  // If this data point is bad, leave it.
	  //
	  if (
	      (InData[index] == InFhdr.missing_data_value) ||
	      (InData[index] == InFhdr.bad_data_value)
	      ) {
	    continue;
	  }
	  //
	  // Get the central lat/lon.
	  //
	  double LatC, LonC;
	  Proj.xyIndex2latlon(ix, iy, LatC, LonC);
	  //
	  // Move the specified distance away, equispaced directions.
	  // Exit if we hit a bad/missing. Store the data values in
	  // the SurroundVal array.
	  //

	  bool GotBad = false;
	  for (int id=0; ((id<P->NumRingPoints) && (GotBad == false)); id++){

	    double dir = id*360.0/P->NumRingPoints;
	    double Lat2, Lon2;
	    PJGLatLonPlusRTheta(LatC, LonC, P->GradDist, 
				dir, &Lat2, &Lon2);
	    //
	    // Get the indicies at the new point.
	    // Then get the data value.
	    //
	    int ixx,iyy;
	    if (Proj.latlon2xyIndex(Lat2, Lon2, ixx, iyy)){
	      SurroundVal[id] = InFhdr.missing_data_value; 
	    } else {
	      int newIndex;
	      newIndex = ixx + iyy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	      SurroundVal[id] = InData[newIndex];
	      if ((ixx==ix) && (iyy == iy)){
		if (P->Debug){
		  cerr << "WARNING : Array inicies did not change!" << endl;
		  cerr << "Increase GradDist in param file?" << endl;
		}
	      }
	    }
	    if (
		(SurroundVal[id] == InFhdr.bad_data_value) ||
		(SurroundVal[id] == InFhdr.missing_data_value)
		){
	      GotBad = true;
	    }
	  }
	  //
	  // If one of the points was bad, forget it.
	  //
	  if (GotBad) continue;
	  //

	  double MaxGrad = 0.0;
	  for (int id=0; id < P->NumRingPoints / 2; id++){
	    double Grad = (SurroundVal[id] -
			   SurroundVal[id+ (P->NumRingPoints/2)])/(2.0*P->GradDist);
	    if (Grad < 0.0) Grad = -1.0*Grad;
	    if (MaxGrad < Grad) MaxGrad = Grad;
	  }

	  OutData[index] = MaxGrad;

	}
      }
    }
    free(SurroundVal);

    //
    // Do a loop to get the min,max. Useful as a diagnostic,
    // and allows us to set the bad and missing values of the
    // output.
    //
    int first = 1;
    double min,max,mean,total=0.0;
    long NumGood = 0;
    for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
      if (
	  (OutData[k] == InFhdr.missing_data_value) ||
	  (OutData[k] == InFhdr.bad_data_value)
	  ) {
	continue;
      } else {
	NumGood++;
	total = total + OutData[k];
	if (first){
	  min = OutData[k];
	  max = min;
	  first = 0;
	} else {
	  if (OutData[k] < min) min = OutData[k];
	  if (OutData[k] > max) max = OutData[k];
	}
      }
    }
    
    if (NumGood == 0){
      if (P->Debug){
	cerr << "All output data are missing." << endl;
      }
    } else {
      mean = total / double(NumGood);
      fl32 newBad = max + 1.0;
      for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
	if (
	    (OutData[k] == InFhdr.missing_data_value) ||
	    (OutData[k] == InFhdr.bad_data_value)
	    ) {   
	  OutData[k] = newBad;
	}
      }

      InFhdr.missing_data_value = newBad;
      InFhdr.bad_data_value = newBad;

      if (P->Debug){
	cerr << "Data range from " << min << " to " << max;
	cerr << " with mean " << mean << endl;
      }
    }

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
    
    free(OutData);

    fld->setFieldName(P->_OutFieldName[i]);
    fld->setUnits(P->_Units[i]);
    fld->setFieldNameLong(P->_OutFieldName[i]);
    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through the fields.

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










