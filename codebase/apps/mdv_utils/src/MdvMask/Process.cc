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
using namespace std;

fl32  GetMedian(fl32 *Buffer, int num,
		Mdvx::field_header_t InFhdr);


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
int Process::Derive(Params *P, const time_t trigger_time){
  return Derive(P, trigger_time, 0 );
}


int Process::Derive(Params *P, const time_t trigger_time, const int leadTime){

  OutputUrl = STRdup(P->OutUrl);

  if (P->Debug){
    cerr << "Trigger time " << utimstr(trigger_time) << ", lead secs " << leadTime << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;


  if( P->Mode == Params::REALTIME_FCST_DATA)
  {
    New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, 
		    P->InputUrl,
		    P->maxMaskValidSecs, 
		    trigger_time, leadTime );
  }
  else
  {
    New.setReadTime(Mdvx::READ_FIRST_BEFORE, 
		    P->InputUrl,
		    P->maxMaskValidSecs, trigger_time);
  }
  

  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);
  New.setReadFieldFileHeaders();

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
    cerr << "Read failed at " << utimstr(trigger_time) << " from ";
    cerr << P->InputUrl  << endl;
    return -1;
  }     

  if (P->Debug)
  {
    cerr << "---> Input file: " << endl;
    if (New.getMasterHeader().data_collection_type == Mdvx::DATA_FORECAST)
    {
      cerr << "       fcst valid time = " << DateTime::str(New.getMasterHeader().time_centroid) << endl;
      cerr << "       fcst gen time = " << DateTime::str(New.getMasterHeader().time_gen) << endl;
    }
    else
    {
      cerr << "       valid time = " << DateTime::str(New.getMasterHeader().time_centroid) << endl;
    }
  }
  
  
  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;

  if (P->output_with_trigger_time)
  {
      time_t offset = trigger_time - InMhdr.time_centroid;
      OutMhdr.time_begin = InMhdr.time_begin + offset;
      OutMhdr.time_end = InMhdr.time_end + offset;
      OutMhdr.time_centroid = trigger_time;
  }
  

  Out.setMasterHeader(OutMhdr);
  Out.clearFields();     

  //
  // Read mask data.
  //

  DsMdvx Mask;

  Mask.clearRead();

  if (P->ReadFromPath){
    Mask.setReadPath(P->MaskPath);
  } else {
    //
    // Set the mask URL up for a realtime read.
    //
    Mask.setReadTime(Mdvx::READ_FIRST_BEFORE, P->MaskUrl,
		     P->lookbackForMask, trigger_time);
  }

  Mask.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  Mask.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      Mask.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      Mask.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      Mask.setReadRemapLc2(P->grid_nx, P->grid_ny,
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

  //
  // Read the mask - this is done in a loop to allow the
  // mask URL a chance to catch up.
  //
  int go = 1;
  int retVal = 0;
  int numTries = 0;
  do {
    retVal = Mask.readVolume();
    if (retVal){
      //
      // The read has failed - sleep for 5 seconds, PMU register,
      // and then try again - let the mask URL catch up.
      //
      numTries++;
      if (numTries > P->max_mask_read_attempts){
	//
	// Tried enough times - give up.
	//
	go = 0;
      } else {
	//
	// Sleep and try again.
	//
	sleep(5);
	PMU_auto_register("Mask read failed - sleeping for 5, trying again ...");
	if (P->Debug){
	  cerr << "Mask read attempt " << numTries << " failed." << endl;
	}
      }
    } else {
      // We succeeded.
      go = 0;
    }
  } while (go);

  if (retVal && P->ProcessIfNoMask == 0 ){
    cerr << "Mask read failed for data at " << utimstr(trigger_time) << endl;

    return -1;
  }

  MdvxField *MaskField;
  fl32 *MaskData = NULL;
  Mdvx::field_header_t MaskFhdr;
  MEM_zero(MaskFhdr);

  if( ! retVal) // Mask available for processing.
  {
  
    if (P->Debug)
    {
      cerr << "     Mask file: " << endl;
      if (Mask.getMasterHeader().data_collection_type == Mdvx::DATA_FORECAST)
      {
	cerr << "       fcst valid time = " << DateTime::str(Mask.getMasterHeader().time_centroid) << endl;
	cerr << "       fcst gen time = " << DateTime::str(Mask.getMasterHeader().time_gen) << endl;
      }
      else
      {
	cerr << "       valid time = " << DateTime::str(Mask.getMasterHeader().time_centroid) << endl;
      }
    }
  
    MaskField = Mask.getFieldByName( P->MaskFieldName );
  
    if (MaskField == NULL){
      cerr << "Mask field " << P->MaskFieldName << " not found." << endl;
      return -1;
    }
    MaskFhdr = MaskField->getFieldHeader();
  
    MaskData = (fl32 *)MaskField->getVol();
  }
  else	// requested to pass data through when mask is not available
  {
    cerr << "Mask read failed for data at " << utimstr(trigger_time) << endl;
    cerr << "Passing data through unchanged\n";
  }
  
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////


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
      if (P->Debug){
	cerr << "Working with field " << Fnum << endl;
      }
    } else {
      InField = New.getFieldByName( P->_InFieldName[i] );
      if (P->Debug){
	cerr << "Working with field " << P->_InFieldName[i] << endl;
      }
    }

    if (InField == NULL){
      cerr << "New field " << P->_InFieldName[i] << " not found in URL "
	   << P->InputUrl << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    float input_scale = InField->getFieldHeaderFile()->scale;
    float input_bias = InField->getFieldHeaderFile()->bias;
    
    fl32 *InData = (fl32 *) InField->getVol();

    if (
	( (InFhdr.nx != MaskFhdr.nx ) ||
	  (InFhdr.ny != MaskFhdr.ny ) ||
	  (InFhdr.nz != MaskFhdr.nz ) ) &&
	( P->ProcessIfNoMask == 0 ) ) {
      cerr << "Image size mismatch!" << endl;
      cerr << "Select image remapping in param file." << endl;
      exit(-1);
    }

  
    int dataSize = InFhdr.nx * InFhdr.ny * InFhdr.nz;

    int numMasked = 0;
    for (int k=0; k < dataSize; k++){
      if ( retVal) // No Mask data but want to pass input data through
      {
	continue;
      }
      if (P->ReverseMask){
	if (MaskData[k] != MaskFhdr.bad_data_value &&
	    MaskData[k] != MaskFhdr.missing_data_value){
	  
	  if((! P->use_less_than_threshold) &&
	     (P->ApplyMaskThreshold) &&
	     (MaskData[k] < P->MaskThresholdValue)){
	    continue;
	  }
	  else if(( P->use_less_than_threshold) &&
	     (P->ApplyMaskThreshold) &&
	     (MaskData[k] >= P->MaskThresholdValue)){
	    continue;
	  }
	  else {
	    InData[k] = InFhdr.bad_data_value;
	    numMasked++;
	  }
	}
      }
      else {
	if ( 
	    (MaskData[k] == MaskFhdr.bad_data_value) ||
	    (MaskData[k] == MaskFhdr.missing_data_value)
     	   )
	{
	  InData[k]=InFhdr.bad_data_value;
	  numMasked++;
	}
	else if (
	  (! P->use_less_than_threshold) &&
	  (P->ApplyMaskThreshold) &&
	  (MaskData[k] <= P->MaskThresholdValue)
	  )
	{
	  InData[k]=InFhdr.bad_data_value;
	  numMasked++;
	}
	else if (
	  (P->use_less_than_threshold) &&
	  (P->ApplyMaskThreshold) &&
	  (MaskData[k] > P->MaskThresholdValue)
	  )
	{
	  InData[k]=InFhdr.bad_data_value;
	  numMasked++;
	}
	else if (
	  (P->ApplyMaskSingleValue) &&
	  (MaskData[k] == P->MaskSingleValue)
	  ){
	  InData[k]=InFhdr.bad_data_value;
	  numMasked++;
	}
	
      }
      
    }

    if (P->Debug){
      cerr << "Masked out " << numMasked << " of " << dataSize << endl;
    }

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)InData);
    
    fld->setFieldName(P->_OutFieldName[i]);
    fld->setFieldNameLong(P->_OutFieldName[i]);
    
    if (fld->convertType(Mdvx::ENCODING_ASIS,
			 Mdvx::COMPRESSION_GZIP,
			 Mdvx::SCALING_SPECIFIED,
			 input_scale, input_bias)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through the fields.

  if (P->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }

  Mdvx::master_header_t Mhdr = Out.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (P->write_as_forecast)
      Out.setWriteAsForecast();

    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wright to " << OutputUrl << endl;
      exit(-1);
    }
  }
  free(OutputUrl);

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
}


fl32 GetMedian(fl32 *Buffer, int num,
		Mdvx::field_header_t InFhdr){

  //
  // Pick off the non-missing data into another array.
  // Return missing as the median if no data found.
  //
  fl32 *GoodBuf = (fl32 *) umalloc(sizeof(fl32) * num);
  
  if (GoodBuf == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  
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
    free(GoodBuf);
    return InFhdr.missing_data_value;
  }

  //
  // Sort them into order. Simple bubble sort.
  //
  int go;
  do {
    go = 0;
    for (int p=0; p < nGood - 1; p++){
      if (GoodBuf[p] < GoodBuf[p+1]){
	fl32 temp = GoodBuf[p];
	GoodBuf[p] = GoodBuf[p+1];
	GoodBuf[p+1] = temp;
	go=1;
      }
    }
  } while(go);

  int index = (int)floor(double(nGood) / 2.0);
  fl32 ans = GoodBuf[index];
  free(GoodBuf);

  return ans;

}








