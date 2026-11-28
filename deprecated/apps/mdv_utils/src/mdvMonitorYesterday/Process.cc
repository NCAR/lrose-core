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
double Process::Derive(Params *P, time_t T){

  //
  // Set up for the new data.
  //
  DsMdvx New;


  New.setReadTime(Mdvx::READ_FIRST_BEFORE, P->url, 0, T);
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);

  New.addReadField(P->fieldName);

  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T) << " from ";
    cerr << P->url  << endl;
    return 110.0;
  }     


  //
  // Get the desired field.
  //
  MdvxField *InField = New.getFieldByName( P->fieldName );
  
  if (InField == NULL){
    cerr << "Field " << P->fieldName << " not found." << endl;
    return 120.0;
  }
  Mdvx::field_header_t InFhdr = InField->getFieldHeader();
  
  fl32 *InData = (fl32 *) InField->getVol();
  
  //
  // Calculate percent bad.
  //
  unsigned long numBad = 0;
  for(unsigned long l=0; l < InFhdr.nx*InFhdr.ny*InFhdr.nz; l++){
    if (
	(InData[l]==InFhdr.missing_data_value) ||
	(InData[l]==InFhdr.bad_data_value)){
      numBad++;
    }
  }
  
  return 100.0*double(numBad)/double(InFhdr.nx*InFhdr.ny*InFhdr.nz);
  
}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
  return;
}










