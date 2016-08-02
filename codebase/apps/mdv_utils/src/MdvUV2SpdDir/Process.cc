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
  forecastMode = P->forecastMode;

  if (P->Debug){
    cerr << "Data at " << utimstr(T) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;

  New.addReadField(P->UfieldName);
  New.addReadField(P->VfieldName);

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
  MdvxField *UField, *VField;

 
  UField = New.getFieldByName( P->UfieldName );
  VField = New.getFieldByName( P->VfieldName );
    

  if ((UField == NULL) || (VField == NULL)){
    cerr << "Required fields not found." << endl;
    return -1;
  }

  Mdvx::field_header_t UFhdr = UField->getFieldHeader();
  Mdvx::field_header_t VFhdr = VField->getFieldHeader();
  Mdvx::vlevel_header_t InVhdr = UField->getVlevelHeader();

  fl32 *UData = (fl32 *) UField->getVol();
  fl32 *VData = (fl32 *) VField->getVol();
 

  fl32 *spdData = (fl32 *) malloc(sizeof(float) * 
				  UFhdr.nx *
				  UFhdr.ny *
				  UFhdr.nz);

  fl32 *dirData = (fl32 *) malloc(sizeof(float) * 
				  UFhdr.nx *
				  UFhdr.ny *
				  UFhdr.nz);


  if ((spdData == NULL) || ( dirData == NULL)){
    cerr << "Malloc failed." << endl;
    exit(-1);
  }
  //
  // 
  //
  for(int l=0; l < UFhdr.nx * UFhdr.ny * UFhdr.nz; l++){
    if (
	(UData[l] == UFhdr.bad_data_value) ||
	(UData[l] == UFhdr.missing_data_value) ||
	(VData[l] == VFhdr.bad_data_value) ||
	(VData[l] == VFhdr.missing_data_value)
	){
      spdData[l] = UFhdr.bad_data_value;
      dirData[l] = UFhdr.bad_data_value;
    } else {
      spdData[l] = PHYwind_speed( P->conversionFactor*UData[l], 
				  P->conversionFactor*VData[l] );

      dirData[l] = PHYwind_dir( P->conversionFactor*UData[l],
				P->conversionFactor*VData[l] );
    }
  }

  Mdvx::field_header_t spdFhdr = UFhdr;
  Mdvx::field_header_t dirFhdr = UFhdr;
 
  MdvxField *ufld = new MdvxField(spdFhdr, InVhdr, spdData);
  ufld->setFieldName( "Wind Speed" );
  ufld->setUnits( "m/s" );
  ufld->setFieldNameLong( "Wind Speed" );
  
  if (ufld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  Out.addField(ufld);
  

  MdvxField *vfld = new MdvxField(dirFhdr, InVhdr, dirData);
  vfld->setFieldName( "dir" );
  vfld->setUnits( "deg" );
  vfld->setFieldNameLong( "Wind direction" );
  
  if (vfld->convertRounded(Mdvx::ENCODING_INT8,
			  Mdvx::COMPRESSION_ZLIB)){
    cerr << "convertRounded failed - I cannot go on." << endl;
    exit(-1);
  }  
  
  Out.addField(vfld);
  
  free(spdData); free(dirData);


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
    if (forecastMode) Out.setWriteAsForecast();
    if (Out.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);
}










