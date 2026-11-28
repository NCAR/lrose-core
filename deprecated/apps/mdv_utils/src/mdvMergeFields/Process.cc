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
  // Put all the fields we have into the output object.
  //
  for (int i=0; i< InMhdr.n_fields; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField = New.getFieldByNum( i ); 
    if (InField == NULL){
      cerr << "Problems with field number " << i+1 << endl;
      exit(-1); // Unlikely.
    }

    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();
    fl32 *InData = (fl32 *) InField->getVol();

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)InData);
    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(fld);
    
  } // End of loop through the original fields.


  //
  // Set up for the add in data.
  //
  DsMdvx addIn;

  addIn.setReadTime(Mdvx::READ_CLOSEST, P->addInUrl, 
		    P->MaxRealtimeValidAge, T);
  addIn.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  addIn.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  if (P->RemapGrid){
    switch ( P->grid_projection){

    case Params::FLAT:
      addIn.setReadRemapFlat(P->grid_nx, P->grid_ny,
			 P->grid_minx, P->grid_miny,
			 P->grid_dx, P->grid_dy,
			 P->grid_origin_lat, P->grid_origin_lon,
			 P->grid_rotation);

      break;
                                   
    case Params::LATLON:
      addIn.setReadRemapLatlon(P->grid_nx, P->grid_ny,
			   P->grid_minx, P->grid_miny,
			   P->grid_dx, P->grid_dy);

      break;            

    case Params::LAMBERT:
      addIn.setReadRemapLc2(P->grid_nx, P->grid_ny,
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


  for (int j=0; j < P->addInFieldnames_n; j++){
    addIn.addReadField( P->_addInFieldnames[j] );
  }

  if (addIn.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->TriggerUrl  << endl;
    return -1;
  }     

  for (int j=0; j < P->addInFieldnames_n; j++){
    //
    // Get the desired field.
    //
    MdvxField *InAddField = addIn.getFieldByName( P->_addInFieldnames[j] ); 
    if (InAddField == NULL){
      cerr << "Problems with field name " << P->_addInFieldnames[j] << endl;
      exit(-1); // Unlikely.
    }

    Mdvx::field_header_t InAddFhdr = InAddField->getFieldHeader();
    Mdvx::vlevel_header_t InAddVhdr = InAddField->getVlevelHeader();
    fl32 *InAddData = (fl32 *) InAddField->getVol();

    MdvxField *addFld = new MdvxField(InAddFhdr, InAddVhdr, 
				      (void *)InAddData);
    
    if (addFld->convertRounded(Mdvx::ENCODING_INT8,
			       Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    Out.addField(addFld);

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










