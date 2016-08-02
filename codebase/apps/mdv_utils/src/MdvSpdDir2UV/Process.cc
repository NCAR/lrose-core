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
#include <physics/physics.h>
#include <toolsa/str.h>
#include <toolsa/pjg_flat.h>
#include <Mdv/MdvxProj.hh>

/**
 *
 * @file Process.cc
 *
 * @class Process
 *
 * This is the class that actually processes the data, generating
 * the output U,V fields from the input speed, direction.
 *
 * @todo Eliminate the use of the OutputUrl variable and move
 * the writing of the MDV output from the destructor into the main
 * function.
 *
 * @author Niles Oien oien@ucar.edu
 *
 */


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

  New.addReadField(P->speedFieldName);
  New.addReadField(P->dirFieldName);

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
  // Get the desired field.
  //
  MdvxField *speedField, *dirField;

 
  speedField = New.getFieldByName( P->speedFieldName );
  dirField = New.getFieldByName( P->dirFieldName );
    

  if ((speedField == NULL) || (dirField == NULL)){
    cerr << "Required fields not found." << endl;
    return -1;
  }

  Mdvx::field_header_t speedFhdr = speedField->getFieldHeader();
  Mdvx::field_header_t dirFhdr   = dirField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr   = speedField->getVlevelHeader();

  fl32 *speedData = (fl32 *) speedField->getVol();
  fl32 *dirData = (fl32 *) dirField->getVol();
 

  fl32 *uData = (fl32 *) malloc(sizeof(float) * 
				dirFhdr.nx *
				dirFhdr.ny *
				dirFhdr.nz);

  fl32 *vData = (fl32 *) malloc(sizeof(float) * 
				dirFhdr.nx *
				dirFhdr.ny *
				dirFhdr.nz);


  if ((vData == NULL) || ( uData == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // Init to missing.
  //
  for(int l=0; l < speedFhdr.nx * speedFhdr.ny * speedFhdr.nz; l++){
    if (
	(speedData[l] == speedFhdr.bad_data_value) ||
	(speedData[l] == speedFhdr.missing_data_value) ||
	(dirData[l] == dirFhdr.bad_data_value) ||
	(dirData[l] == dirFhdr.missing_data_value)
	){
      uData[l] = speedFhdr.bad_data_value;
      vData[l] = speedFhdr.bad_data_value;
    } else {
      uData[l] = PHYwind_u(speedData[l] * P->conversionFactor, dirData[l] );
      vData[l] = PHYwind_v(speedData[l] * P->conversionFactor, dirData[l] );
    }
  }

  Mdvx::field_header_t uFhdr = speedFhdr;
  Mdvx::field_header_t vFhdr = speedFhdr;
 
  MdvxField *ufld = new MdvxField(uFhdr, InVhdr, uData);
  ufld->setFieldName( "U" );
  ufld->setUnits( "m/s" );
  ufld->setFieldNameLong( "U component of wind" );
  
  if (ufld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  Out.addField(ufld);
  

  MdvxField *vfld = new MdvxField(vFhdr, InVhdr, vData);
  vfld->setFieldName( "V" );
  vfld->setUnits( "m/s" );
  vfld->setFieldNameLong( "V component of wind" );
  
  if (vfld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  Out.addField(vfld);
  
  free(uData); free(vData);


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










