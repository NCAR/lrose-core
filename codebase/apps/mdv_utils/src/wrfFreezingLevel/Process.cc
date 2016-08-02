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
  // Set up for the new data.
  //
  DsMdvx New;


  New.addReadField(P->field_spec.heightFieldName);
  New.addReadField(P->field_spec.tempFieldName);

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
  OutMhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  OutMhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  sprintf(OutMhdr.data_set_info, "Derived freezing level");

  Out.setMasterHeader(OutMhdr);
  Out.clearFields();

  //
  // Get the desired fields.
  //
  MdvxField *hgtField = New.getFieldByName(P->field_spec.heightFieldName);
  if (hgtField == NULL){
    cerr << "Height field " << P->field_spec.heightFieldName << " not found." << endl;
    return -1;
  }

  MdvxField *tempField = New.getFieldByName(P->field_spec.tempFieldName);
  if (tempField == NULL){
    cerr << "Temperature field " << P->field_spec.tempFieldName << " not found." << endl;
    return -1;
  }

  Mdvx::field_header_t tempFhdr = tempField->getFieldHeader();
  Mdvx::field_header_t hgtFhdr = hgtField->getFieldHeader();
  //
  // Do rudimentary check on field sizes.
  //
  if ((hgtFhdr.nx != tempFhdr.nx) ||
      (hgtFhdr.ny != tempFhdr.ny) ||
      (hgtFhdr.nz != tempFhdr.nz)){
    cerr << "Height and temp different size - I cannot cope" << endl;
    return -1;
  }
  
  Mdvx::field_header_t fzgLvlFhdr = hgtFhdr;
  sprintf(fzgLvlFhdr.field_name,"fzgLevel");
  sprintf(fzgLvlFhdr.field_name_long,"Freezing Level");
  sprintf(fzgLvlFhdr.units, P->field_spec.outUnits);
  fzgLvlFhdr.bad_data_value = -1.0;
  fzgLvlFhdr.missing_data_value = -1.0;
  fzgLvlFhdr.nz = 1;
  fzgLvlFhdr.volume_size = fzgLvlFhdr.nx*fzgLvlFhdr.ny*sizeof(fl32);
  fzgLvlFhdr.native_vlevel_type = Mdvx::VERT_TYPE_SURFACE;
  fzgLvlFhdr.vlevel_type = Mdvx::VERT_TYPE_SURFACE;


  fl32 *OutData = (fl32 *) malloc(sizeof(float) * hgtFhdr.nx * hgtFhdr.ny);
  fl32 *hgtData = (fl32 *) hgtField->getVol();
  fl32 *tempData = (fl32 *) tempField->getVol();

  if (OutData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // Init to missing, set up a vhdr for output.
  //
  for(int l=0; l < fzgLvlFhdr.nx*fzgLvlFhdr.ny; l++){
    OutData[l]=fzgLvlFhdr.missing_data_value;
  }

  Mdvx::vlevel_header_t fzgVhdr;
  fzgVhdr.type[0] = Mdvx::VERT_TYPE_SURFACE;
  fzgVhdr.level[0] = 0.0;

  //
  // Do the work - find freezing level.
  //
  for (int ix=0; ix < hgtFhdr.nx; ix++){
    for (int iy=0; iy < hgtFhdr.ny; iy++){

      //
      // Go up through col. and find first level at or below freezing
      //
      int hgtIndex = -1;
      for (int iz=0; iz < hgtFhdr.nz; iz++){

	int index = ix + iy*hgtFhdr.nx + iz*hgtFhdr.nx*hgtFhdr.ny;

	if ((tempData[index] == tempFhdr.missing_data_value) ||
	    (tempData[index] == tempFhdr.bad_data_value) ||
	    (hgtData[index] == hgtFhdr.missing_data_value) ||
	    (hgtData[index] == hgtFhdr.bad_data_value)){
	  continue;
	}

	if (tempData[index] > P->field_spec.freezingLevel) continue;

	hgtIndex = index;
	break;
      }

      if (hgtIndex != -1){
	OutData[ix + iy*hgtFhdr.nx] = P->field_spec.scaleOutput*hgtData[hgtIndex] + P->field_spec.biasOutput;
      }
    }
  }


  MdvxField *fld = new MdvxField(fzgLvlFhdr, fzgVhdr, OutData);

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

  if (P->writeAsForecast) Out.setWriteAsForecast();

  if (Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    exit(-1);
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










