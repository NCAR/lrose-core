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

  New.setReadVlevelLimits(P->minVlevel, P->maxVlevel);

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


  _Out.setMasterHeader(OutMhdr);
  _Out.clearFields();     

  //
  // Get the desired field.
  //
  MdvxField *InField;
  InField = New.getFieldByName( P->CinFieldName );

  if (InField == NULL){
    cerr << "New field " << P->CinFieldName << " not found." << endl;
    return -1;
  }

  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

  MdvxProj Proj(InMhdr, InFhdr);

  fl32 *InData = (fl32 *) InField->getVol();

  fl32 *OutData = (fl32 *) malloc(sizeof(float) * 
				  InFhdr.nx *
				  InFhdr.ny);

  if (OutData == NULL){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // Init to missing.
  //
  for(int l=0; l < InFhdr.nx*InFhdr.ny; l++){
    OutData[l]=InFhdr.missing_data_value;
  }

  vector<int>missing_index;
  
  for(int ix=0; ix < InFhdr.nx; ix++){
    for(int iy=0; iy < InFhdr.ny; iy++){

      double total = 0.0; int num = 0;

      for(int iz=0; iz < InFhdr.nz; iz++){

	  int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	  //
	  // If this data point is bad, or outside our thresholds, leave it.
	  //
	  if (
	      (InData[index] == InFhdr.missing_data_value) ||
	      (InData[index] == InFhdr.bad_data_value) ||
	      (InData[index] > P->maxValidCINvalue) ||
	      (InData[index] < P->minValidCINvalue)
	      ) {
	    continue;
	  }

	  num++; total += InData[index];
      }
      if (num){
	OutData[iy*InFhdr.nx + ix] = total / double(num);
      }
      else
      {
	// keeping track of the points that still have
	// missing data values after procesing through
	// the vertical levels
	missing_index.push_back(ix + iy*InFhdr.nx);
      }
    }
  }

  // fill in missing data values with closest vertical point 
  // that is not a missing data value
  if(P->fill_missing && missing_index.size() > 0)
  {

    if(P->level_increases_with_height)
      New.setReadVlevelLimits(P->maxVlevel, 10000.0);
    else
      New.setReadVlevelLimits(0.0,P->minVlevel);

    if (New.readVolume()){
      cerr << "Read failed at " << utimstr(T) << " from ";
      cerr << P->TriggerUrl  << endl;
      return -1;
    }     

    InField = New.getFieldByName( P->CinFieldName );
    InFhdr = InField->getFieldHeader();

    if (InField == NULL){
      cerr << "New field " << P->CinFieldName << " not found." << endl;
      return -1;
    }

    InData = (fl32 *) InField->getVol();

    for(vector<int>::iterator it = missing_index.begin(); it != missing_index.end(); ++it) 
    {
      for(int iz=0; iz < InFhdr.nz; iz++){

	int index = *it + iz*InFhdr.nx*InFhdr.ny;
	//
	// If this data point is bad, or outside our thresholds, leave it.
	//
	if (
	    (InData[index] == InFhdr.missing_data_value) ||
	    (InData[index] == InFhdr.bad_data_value) ||
	    (InData[index] > P->maxValidCINvalue) ||
	    (InData[index] < P->minValidCINvalue)
	    ) {
	  continue;
	}
	OutData[*it] =  InData[index];
	break;
      }
    }
  }
  
  //
  // Overwrite the input headers so they can be used for output.
  //
  InFhdr.nz = 1;
  InFhdr.volume_size = InFhdr.nx * InFhdr.ny * InFhdr.nz * sizeof(fl32);

  InVhdr.level[0] = P->outputVlevel;

  MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)OutData);
  
  free(OutData);
  
  if (fld->convertRounded(Mdvx::ENCODING_INT16,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  _Out.addField(fld);

  if (P->rename_field) 
  {
    MdvxField *field = _Out.getField(0);
      
    field->setFieldNameLong("CIN_layer_mean");

    if (strcmp(field->getFieldName(),
	       P->new_name.old_field_name)==0) 
    {
      field->setFieldName(P->new_name.new_field_name);
    }
  }
  
  if (_Out.writeToDir(P->OutUrl)) {
    cerr << "Failed to wite to " << P->OutUrl << endl;
    exit(-1);
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
  return;
}
