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
int Process::Derive(Params *TDRP_params, TriggerInfo T){

  OutputUrl = STRdup(TDRP_params->OutUrl);

  if (TDRP_params->Debug){
    cerr << "Data at " << utimstr(T.getIssueTime()) << endl;
  }


  //
  // Set up for the new data.
  //
  DsMdvx New;
  DsMdvx NewOut;
  NewOut.clear();
  New.clear();

  New.setDebug( TDRP_params->Debug);
  New.setReadTime(Mdvx::READ_SPECIFIED_FORECAST, TDRP_params->TriggerUrl, 0,
                  T.getIssueTime(), (T.getForecastTime() - T.getIssueTime()));
  New.setReadEncodingType(Mdvx::ENCODING_FLOAT32);
  New.setReadCompressionType(Mdvx::COMPRESSION_NONE);


  if (New.readVolume()){
    cerr << "Read failed at " << utimstr(T.getForecastTime()) << " from ";
    cerr << TDRP_params->TriggerUrl  << endl;
    return -1;
  }     

  //
  // Set up the output.
  //
  Mdvx::master_header_t InMhdr = New.getMasterHeader();
  Mdvx::master_header_t OutMhdr = InMhdr;


  NewOut.setMasterHeader(OutMhdr);
  NewOut.clearFields(); 
  NewOut.clearWrite();    
  NewOut.setWriteAsForecast();

  //
  // Commence loop through fields.
  //
  for (int i=0; i< TDRP_params->InFieldName_n; i++){
    //
    // Get the desired field.
    //
    MdvxField *InField;

    InField = New.getFieldByName( TDRP_params->_InFieldName[i] );

    if (InField == NULL){
      cerr << "New field " << TDRP_params->_InFieldName[i] << " not found." << endl;
      return -1;
    }
    Mdvx::field_header_t InFhdr = InField->getFieldHeader();
    Mdvx::vlevel_header_t InVhdr = InField->getVlevelHeader();

    fl32 *InData = (fl32 *) InField->getVol();

  
    for(int iz=0; iz < InFhdr.nz; iz++){
      for(int iy=0; iy < InFhdr.ny; iy++){
	for(int ix=0; ix < InFhdr.nx; ix++){
	  int index = ix + iy*InFhdr.nx + iz*InFhdr.nx*InFhdr.ny;
	  //
	  // If this data point is bad, leave it.
	  //
	  if (InData[index] == InFhdr.missing_data_value) {
	    InData[index] = TDRP_params->MissingReplaceValue;
	  }

	  if (InData[index] == InFhdr.bad_data_value) {
	    InData[index] = TDRP_params->BadReplaceValue;
	  }
	
	}
      }
    }

    MdvxField *fld = new MdvxField(InFhdr, InVhdr, (void *)InData);
    
    if (fld->convertRounded(Mdvx::ENCODING_INT8,
			    Mdvx::COMPRESSION_ZLIB)){
      cerr << "convertRounded failed - I cannot go on." << endl;
      exit(-1);
    }  
    
    NewOut.addField(fld);
    
  } // End of loop through the fields.

  //
  // Only do the write if fields were added.
  //

  Mdvx::master_header_t Mhdr = NewOut.getMasterHeader();

  if (Mhdr.n_fields > 0){
    if (NewOut.writeToDir(OutputUrl)) {
      cerr << "Failed to wite to " << OutputUrl << endl;
      exit(-1);
    }      
  }
  free(OutputUrl);

  if (TDRP_params->Debug){
    cerr << "Finished data processing for this time." << endl << endl;
  }    

  return 0;

}

////////////////////////////////////////////////////
//
// Destructor
//
Process::~Process(){
}










