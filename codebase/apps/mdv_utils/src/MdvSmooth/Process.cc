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
// This class does the actual spatial smoothing for the
// MdvSmooth application.
//
#include "Process.hh"

#include <iostream>
#include <Mdv/MdvxField.hh>
#include <Mdv/MdvxChunk.hh>
#include <math.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <toolsa/TaArray.hh>
using namespace std;


//
// Constructor
//
Process::Process(){
  _OutputUrl = NULL;
  return;
}

////////////////////////////////////////////////////
//
// Main method - process data at a given time.
//
int Process::Derive(Params *TDRP, time_t T, int leadTime){
  //
  // Make a copy of the output URL for use in the destructor.
  //
  _OutputUrl = STRdup(TDRP->OutUrl);

  //
  // Print some debugging, if requested.
  //
  if (TDRP->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }

  //
  // Set up for reading the new input MDV data. This
  // is done with the 'New' object.
  //
  DsMdvx New;

  New.setDebug( TDRP->Debug);

  //
  // Set up to read an uncompressed grid of FLOAT32 values.
  //
  if (TDRP->Mode == Params::ARCHIVE_FCST || TDRP->Mode == Params::REALTIME_FCST)
    New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, TDRP->TriggerUrl, 0, T, leadTime);
  else
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, TDRP->TriggerUrl, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  //
  // Set up to read specific fields
  //
  for (int i = 0; i < TDRP->InFieldName_n; i++)
     New.addReadField( TDRP->_InFieldName[i]);

  //
  // If we want to remap the grid onto another geometry rather
  // than just using the one that was on disk, then do so.
  //
  if (TDRP->RemapGrid){
    switch ( TDRP->grid_projection){

    case Params::FLAT:
      New.setReadRemapFlat(TDRP->grid_nx, TDRP->grid_ny,
			 TDRP->grid_minx, TDRP->grid_miny,
			 TDRP->grid_dx, TDRP->grid_dy,
			 TDRP->grid_origin_lat, TDRP->grid_origin_lon,
			 TDRP->grid_rotation);

      break;
                                   
    case Params::LATLON:
      New.setReadRemapLatlon(TDRP->grid_nx, TDRP->grid_ny,
			   TDRP->grid_minx, TDRP->grid_miny,
			   TDRP->grid_dx, TDRP->grid_dy);

      break;            

    case Params::LAMBERT:
      New.setReadRemapLc2(TDRP->grid_nx, TDRP->grid_ny,
			TDRP->grid_minx, TDRP->grid_miny,
			TDRP->grid_dx, TDRP->grid_dy,
			TDRP->grid_origin_lat, 
			TDRP->grid_origin_lon,
			TDRP->grid_lat1,  TDRP->grid_lat2);
      
      break;
      
    default:
      cerr << "Unsupported projection." << endl;
      return -1;
      break;
      
    }               
  }
  //
  // Read the MDV data for this time.
  //
  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << TDRP->TriggerUrl  << endl;
    return -1;
  }     
  
  //
  // Initialize the _Out DsMdvx object and get ready to write the
  // output data.
  //
  const Mdvx::master_header_t &InMhdr = New.getMasterHeader();
  _Out.setMasterHeader(InMhdr);
  _Out.clearFields();
  _Out.clearChunks();
  _Out.setAppName("MdvSmooth");
  //
  // Copy and MdvChunks that were in the input to the output.
  // These chunks are added to an MDV file to carry excess
  // information, such as the setup parameters for a radar.
  // Here, we just pass them on from input to output.
  //
  for (int ii = 0; ii < New.getNChunks(); ii++) {
    MdvxChunk *chunk = new MdvxChunk(*New.getChunkByNum(ii));
    _Out.addChunk(chunk);
  }

  //
  // Commence loop through the list of fields that we want to smooth.
  //
  for (int i=0; i< TDRP->InFieldName_n; i++){

    //
    // See if this is a field for which we want
    // to undo the DBZ relationship and do our smoothing in Z.
    //
    bool undoDBZ = false;
    for (int k=0; k < TDRP->undoLogDbzNames_n; k++){
      if (0==strcmp(TDRP->_InFieldName[i], TDRP->_undoLogDbzNames[k])){
	undoDBZ = true;
	break;
      }
    }

    //
    // Get the desired field.
    //
    MdvxField *InField;
    //
    // If the fieldname starts with the '#' sign, then
    // decode the field number, ie. #3 means 'get me
    // field number three'. Otherwise, get the field
    // by its name.
    //
    if (!(strncmp(TDRP->_InFieldName[i],"#",1))){
      //
      // The fieldname with a '#', get the field number using atoi
      // and use that number to get the field.
      //
      int Fnum = atoi(TDRP->_InFieldName[i] + 1);
      InField = New.getFieldByNum( Fnum ); 
    } else {
      //
      // It does not start with a '#' so just get the field by name.
      //
      InField = New.getFieldByName( TDRP->_InFieldName[i] );
    }
    //
    // However we got the field, check that we were successful.
    //
    if (InField == NULL){
      cerr << "New field " << TDRP->_InFieldName[i] << " not found." << endl;
      return -1;
    }
    //
    // Get the field header and vlevel header for this field.
    //
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    //
    // Get a pointer to the input data.
    //
    fl32 *InData = (fl32 *) InField->getVol();

    //
    // Go from DBZ to Z if desired.
    //
    if (undoDBZ){
      if (TDRP->Debug){
	cerr << "Converting from dbz to Z..." << endl;
      }
      for (int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
	if (
	    (InData[k] != InFhdr.missing_data_value) &&
	    (InData[k] != InFhdr.bad_data_value)
	    ){
	  InData[k] = pow(10.0, InData[k]/10.0);
	}
      }
    }

    //
    // And allocate space for the output grid.
    //
    fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				    InFhdr.nx *
				    InFhdr.ny *
				    InFhdr.nz);

    if (OutData == NULL){
      cerr << "Malloc failed." << endl; // Highly unlikely.
      exit(-1);
    }
    //
    // Initialize the output grid to missing values.
    //
    for(int l=0; l < InFhdr.nx*InFhdr.ny*InFhdr.nz; l++){
      OutData[l]=InFhdr.missing_data_value;
    }

    //
    // Allocate a buffer that we will use to hold the values
    // that we are smoothing.
    //
    fl32 *Buffer = (fl32 *) malloc(sizeof(float) * 
				    (2*TDRP->HalfWin+1) * 
				    (2*TDRP->HalfWin+1) *
				    (2*TDRP->VertHalfWin+1));
    if (Buffer == NULL){
      cerr << "Malloc failed." << endl;
      exit(-1);
    }

    //
    // Loop through the vertical planes of the volume.
    //
    for(int iz=0; iz < InFhdr.nz; iz++){
      //
      // Loop through the X and Y directions.
      //
      for(int iy=0; iy < InFhdr.ny; iy++){
	for(int ix=0; ix < InFhdr.nx; ix++){
	  //
	  // Gather points to smooth, based at ix, iy.
	  // Store them in 'Buffer'.
	  //
	  int num=0;
	  for (int izz = -TDRP->VertHalfWin; izz <= TDRP->VertHalfWin; izz++){
 	    for (int ixx = -TDRP->HalfWin; ixx <= TDRP->HalfWin; ixx++){
	      for (int iyy = -TDRP->HalfWin; iyy <= TDRP->HalfWin; iyy++){
	        int ixxx = ix + ixx;
	        int iyyy = iy + iyy;
		int izzz = iz + izz;

	        if (
		    (ixxx > -1) &&
		    (iyyy > -1) &&
		    (iyyy < InFhdr.ny) &&
		    (ixxx < InFhdr.nx) &&
		    (izzz > -1) &&
		    (izzz < InFhdr.nz)
		    ){
		  Buffer[num] = InData[ixxx +
				    iyyy*InFhdr.nx +
				    izzz*InFhdr.nx*InFhdr.ny];
		  num ++;

	        }
	      }
	    }
	  }

	  int NumGood = 0;
	  double Total=0.0,Min=0.0,Max=0.0;
	  int First = 1;
	  for (int q=0; q < num; q++){
	    if (
		(Buffer[q] != InFhdr.missing_data_value) &&
		(Buffer[q] != InFhdr.bad_data_value)
		){
	      NumGood++;
	      Total = Total + Buffer[q];
	      //
	      // Record the minimum and maximum values.
	      //
	      if (First){
		Max = Buffer[q];
		Min = Max;
		First = 0;
	      } else {
		if (Min > Buffer[q]) Min = Buffer[q];
		if (Max < Buffer[q]) Max = Buffer[q];
	      }
	    }
	  }

	  //
	  // Calculate the index for the output grid.
	  //
	  int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	  //
	  // If we have no non-missing data poits then set the output
	  // to missing, otherwise set it according to the smoothing method.
	  //
	  if (NumGood < TDRP->MinWinData){
	    OutData[index]=InFhdr.missing_data_value;
	  } else {

	    switch (TDRP->SmoothingMethod){

	    case Params::MIN :
	      OutData[index]=Min;
	      break;

	    case Params::MAX :
	      OutData[index]=Max;
	      break;

	    case Params::MEAN :
	      {
		fl32 Mean = Total / double(NumGood);
		OutData[index]=Mean;
	      }
	      break;

	    case Params::MEDIAN :
	      {
		fl32 Median = _GetMedian(Buffer,num,InFhdr);
		OutData[index]=Median;
	      }
	      break;

	    default :
	      cerr << "Unsupported smoothing option. I cannot cope." << endl;
	      exit(-1);
	      break;

	    }
	  }

	}
      }
    }

    free(Buffer);


    //
    // Go back from Z to DBZ if we did the inverse of that
    // before smoothing this field.
    //
    if (undoDBZ){
      if (TDRP->Debug){
	cerr << "Converting back from Z to dbz..." << endl;
      }
      for (int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
	if (
	    (OutData[k] != InFhdr.missing_data_value) &&
	    (OutData[k] != InFhdr.bad_data_value)
	    ){
	  if (OutData[k] < 0.0){
	    OutData[k] = InFhdr.missing_data_value; // I don't _think_ this can happen, but why not check?
	  } else {
	    OutData[k] = 10.0*log10(OutData[k]);
	  }
	}
      }
    }

    //
    // Do a loop to get the min,max. Useful as a diagnostic,
    // and allows us to set the bad and missing values of the
    // output.
    //
    int first = 1;
    double min=0.0,max=0.0,mean,total=0.0;
    long Numgood = 0;
    for(int k=0; k < InFhdr.nx*InFhdr.ny*InFhdr.nz; k++){
      if (
	  (OutData[k] == InFhdr.missing_data_value) ||
	  (OutData[k] == InFhdr.bad_data_value)
	  ) {
	continue;
      } else {
	Numgood++;
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
    
    if (Numgood == 0){
      if (TDRP->Debug){
	cerr << "All output data are missing." << endl;
      }
    } else {
      mean = total / double(Numgood);
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

      if (TDRP->Debug){
	cerr << "Data range from " << min << " to " << max;
	cerr << " with mean " << mean << endl;
      }
    }
    //
    // Take the input field header, the input vlevel header
    // and the output data and make an MdvxField from them.
    //
    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
    
    free(OutData);
    //
    // Set the output field name.
    //
    fld->setFieldName(TDRP->_OutFieldName[i]);
    fld->setFieldNameLong(TDRP->_OutFieldName[i]);
    //
    // Set encoding and compression on the output field.
    //    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_GZIP)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    //
    // Add this field to the list of stuff that we have
    // to output.
    //    
    _Out.addField(fld);
    
    if (TDRP->Mode == Params::ARCHIVE_FCST or TDRP->Mode == Params::REALTIME_FCST)
      _Out.setWriteAsForecast();


  } // End of loop through the fields.

  if (TDRP->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }
  //
  // The data will be written by the destructor.
  //
  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  //
  // Write the MDV data if the fields were added.
  //
  Mdvx::master_header_t Mhdr = _Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (_Out.writeToDir(_OutputUrl)) {
      cerr << "Failed to wite to " << _OutputUrl << endl;
      exit(-1);
    }      
  }
  free(_OutputUrl);
}
////////////////////////////////////////////////////
//
// Small routine to get the median value.
//
fl32 Process::_GetMedian(fl32 *Buffer, int num,
		Mdvx::field_header_t InFhdr){

  //
  // Pick off the non-missing data into another array.
  // Return missing as the median if no data found.
  //
  TaArray<fl32> GoodBuf_;
  fl32 *GoodBuf = GoodBuf_.alloc(num);
  
  int nGood = 0;
  for(int l=0; l < num; l++){
    if (
	(Buffer[l] != InFhdr.missing_data_value) &&
	(Buffer[l] != InFhdr.bad_data_value)
	) {   
      GoodBuf[nGood] = Buffer[l];
      nGood++;
    }
  }

  if (nGood == 0){
    return InFhdr.missing_data_value;
  }

  //
  // Sort them into order.
  //
  qsort(GoodBuf, num, sizeof(fl32), _Fl32Compare);

  // pick median

  int index = (int)floor(double(nGood) / 2.0);
  fl32 ans = GoodBuf[index];

  return ans;

}

//////////////////////////////////////////////////
//
// Comparison routine for fl32 in ascending order.
//
int Process::_Fl32Compare(const void *a, const void *b){

  fl32 *x = (fl32 *)a;  fl32 *y = (fl32 *)b;

  if (*x >  *y) return 1;
  if (*x <  *y) return -1;
  return 0;

}
